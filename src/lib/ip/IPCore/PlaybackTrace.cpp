//******************************************************************************
// Copyright (c) 2026 Autodesk Inc. All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0
//
//******************************************************************************
#include <IPCore/PlaybackTrace.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <deque>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <system_error>
#include <vector>

namespace IPCore
{
    using namespace std;
    namespace fs = std::filesystem;

    bool PlaybackTrace::m_enabled = false;
    PlaybackTrace::Record PlaybackTrace::m_current;

    namespace
    {
        //  Seconds of playback between summaries.
        constexpr double summaryPeriod = 5.0;

        //  Loop intervals kept to estimate the presentation refresh period.
        constexpr size_t intervalHistory = 240;

        //  Loops after play starts that are excluded from the analysis.
        constexpr int warmupLoops = 3;

        //  Loops printed in full before and after every stop.
        constexpr size_t preStopLines = 6;
        constexpr int postStopLines = 4;

        //  Seconds after a stop during which a frame change is flagged.
        constexpr double afterStopWindow = 2.0;

        //  A top-level flush longer than this during playback is flagged.
        constexpr double mainFlushFlagMs = 4.0;

        //  Longest pending marker text attached to one CSV row.
        constexpr size_t maxMarkerChars = 600;

        constexpr double defaultCsvMaxMB = 1024.0;

        struct Summary
        {
            double start = 0.0;
            int loops = 0;
            int framesShown = 0;
            int skipEvents = 0;
            int skippedFrames = 0;
            int judders = 0;
            int lateLoops = 0;
            int misses = 0;
            int buffering = 0;
            int mainFlushes = 0;
            int slowMainFlushes = 0;
            double mainFlushMaxMs = 0.0;
            double worstIntervalMs = 0.0;
            int worstIntervalFrame = 0;
            double deviceHz = 0.0;
            map<string, int> lateCulprits;
            map<string, int> evalPaths;
        };

        //  Longest top-level events kept per loop window.
        constexpr size_t topEventCount = 3;

        struct EventSample
        {
            double ms = 0.0;
            double start = 0.0;
            string description;
        };

        //  Top-level Qt events completed since the last beginLoop().
        struct EventWindow
        {
            double busyMs = 0.0;
            int count = 0;
            vector<EventSample> top;

            void add(double start, double ms, const string& description)
            {
                busyMs += ms;
                ++count;
                if (description.empty())
                    return;
                if (top.size() == topEventCount && ms <= top.back().ms)
                    return;
                EventSample e;
                e.ms = ms;
                e.start = start;
                e.description = description;
                top.insert(upper_bound(top.begin(), top.end(), e, [](const EventSample& a, const EventSample& b) { return a.ms > b.ms; }),
                           e);
                if (top.size() > topEventCount)
                    top.pop_back();
            }
        };

        //  Widget paints and geometry changes (nested inside top-level
        //  events) since the last beginLoop().
        struct WidgetWindow
        {
            static constexpr size_t maxPaints = 5;
            static constexpr size_t maxChanges = 8;

            struct Paint
            {
                int count = 0;
                double ms = 0.0;
            };

            map<string, Paint> paints;
            double paintMs = 0.0;
            int paintCount = 0;

            vector<string> changes;
            int changeCount = 0;

            void addPaint(const string& description, double ms)
            {
                Paint& p = paints[description];
                ++p.count;
                p.ms += ms;
                paintMs += ms;
                ++paintCount;
            }

            void addChange(const string& description)
            {
                ++changeCount;
                if (changes.size() < maxChanges && find(changes.begin(), changes.end(), description) == changes.end())
                    changes.push_back(description);
            }

            string paintSummary() const
            {
                vector<pair<string, Paint>> v(paints.begin(), paints.end());
                sort(v.begin(), v.end(), [](const auto& a, const auto& b) { return a.second.ms > b.second.ms; });
                ostringstream o;
                o << fixed << setprecision(2);
                for (size_t i = 0; i < v.size() && i < maxPaints; ++i)
                {
                    if (i)
                        o << "; ";
                    o << v[i].first << " " << v[i].second.ms << "ms";
                    if (v[i].second.count > 1)
                        o << "x" << v[i].second.count;
                }
                return o.str();
            }

            string changeSummary() const
            {
                ostringstream o;
                for (size_t i = 0; i < changes.size(); ++i)
                    o << (i ? "; " : "") << changes[i];
                if (changeCount > int(changes.size()))
                    o << "; ...(" << changeCount << " total)";
                return o.str();
            }
        };

        struct State
        {
            bool initialized = false;

            ofstream csv;
            ofstream log;
            string csvPath;
            string logPath;
            string backend;

            double csvBytes = 0.0;
            double csvMaxBytes = defaultCsvMaxMB * 1024.0 * 1024.0;
            bool csvCapped = false;

            //  Qt event accounting
            bool inEvent = false;
            double eventStart = 0.0;
            const char* eventType = "";
            bool eventContainsLoop = false;
            EventWindow events;
            WidgetWindow widgets;

            //  Accumulated until the next endLoop()
            string pendingMarkers;
            int pendingPointerMoves = 0;
            double flushMs = 0.0;
            double flushMaxMs = 0.0;
            int flushCount = 0;
            string flushWorst;

            //  The previous loop, for attributing the interval that ends at
            //  the current loop.
            PlaybackTrace::Record prevRecord;

            double lastHeartbeat = 0.0;
            double prevLoopStart = 0.0;
            double prevLoopEnd = 0.0;
            deque<double> intervals;

            bool wasPlaying = false;
            int loopsSincePlay = 0;
            int lastShownFrame = 0;
            double frameShownSince = 0.0;
            bool haveShownFrame = false;

            //  Any row, playing or not: to catch the frame moving after stop.
            bool haveRowFrame = false;
            int lastRowFrame = 0;
            int lastPlayInc = 1;
            double lastStopTime = -1.0;
            int postStopRemaining = 0;
            deque<string> recentLines;

            Summary summary;
        };

        State& state()
        {
            static State s;
            return s;
        }

        void writeLine(State& s, const string& line)
        {
            cout << line << endl;
            if (s.log.is_open())
            {
                s.log << line << "\n";
                s.log.flush();
            }
        }

        string wallClock(const char* format)
        {
            const time_t t = time(nullptr);
            tm local{};
#ifdef _WIN32
            localtime_s(&local, &t);
#else
            localtime_r(&t, &local);
#endif
            char buffer[64];
            strftime(buffer, sizeof(buffer), format, &local);
            return buffer;
        }

        string envOrEmpty(const char* name)
        {
            const char* v = getenv(name);
            return v ? string(v) : string();
        }

        //  <Documents>/RV_playback_trace, or the home or temp directory.
        fs::path defaultFolder()
        {
            vector<fs::path> candidates;
#ifdef _WIN32
            const string home = envOrEmpty("USERPROFILE");
#else
            const string home = envOrEmpty("HOME");
#endif
            if (!home.empty())
            {
                candidates.push_back(fs::path(home) / "Documents");
                candidates.push_back(fs::path(home));
            }
            for (const char* name : {"TEMP", "TMP", "TMPDIR"})
            {
                const string dir = envOrEmpty(name);
                if (!dir.empty())
                    candidates.push_back(fs::path(dir));
            }
            candidates.push_back(fs::path("/tmp"));

            for (const fs::path& base : candidates)
            {
                error_code ec;
                if (!fs::is_directory(base, ec))
                    continue;
                const fs::path folder = base / "RV_playback_trace";
                fs::create_directories(folder, ec);
                if (fs::is_directory(folder, ec))
                    return folder;
            }
            return fs::path(".");
        }

        //  Deletes the oldest rv_trace_* runs so at most keep remain.
        void pruneOldRuns(const fs::path& folder, int keep)
        {
            error_code ec;
            map<string, vector<fs::path>> runs;
            for (fs::directory_iterator it(folder, ec), end; !ec && it != end; it.increment(ec))
            {
                const fs::path p = it->path();
                const string name = p.filename().string();
                const string ext = p.extension().string();
                if (name.rfind("rv_trace_", 0) != 0 || (ext != ".csv" && ext != ".log"))
                    continue;
                runs[p.stem().string()].push_back(p);
            }

            int excess = int(runs.size()) - keep;
            for (auto& run : runs)
            {
                if (excess-- <= 0)
                    break;
                for (const fs::path& p : run.second)
                    fs::remove(p, ec);
            }
        }

        double medianInterval(const deque<double>& intervals)
        {
            if (intervals.empty())
                return 0.0;
            vector<double> v(intervals.begin(), intervals.end());
            nth_element(v.begin(), v.begin() + v.size() / 2, v.end());
            return v[v.size() / 2];
        }

        //  Text for a double-quoted CSV field.
        string csvText(string s)
        {
            replace(s.begin(), s.end(), '"', '\'');
            replace(s.begin(), s.end(), '\n', ' ');
            return s;
        }

        void writeHeader(ostream& o)
        {
            o << "t,backend,playing,realtime,audio,fps,deviceHz,syncIntervalMs,refreshMs,"
                 "intervalMs,loopMs,outsideMs,heartbeats,maxHeartbeatGapMs,redrawRequests,requestToPaintMs,"
                 "elapsed,predictedSyncMs,prevFrame,targetFrame,shownFrame,skipped,shiftBefore,shiftAfter,"
                 "frameCached,evalPath,cacheLockMs,bufferWait,bufferStop,bufferRefill,wrapped,"
                 "preRenderEventMs,evaluateMs,waitUploadMs,preEvalMs,rendererMs,uploadMs,userRenderMs,frameChangedMs,"
                 "presentMs,fenceWaitMs,acquireMs,outputPresentMs,postRenderMs,holdMs,"
                 "eventsBusyMs,eventCount,eventsIdleMs,paintEventType,paintDispatchMs,topEvents,"
                 "widgetPaintMs,widgetPaintCount,widgetPaints,widgetChanges,"
                 "mainFlushMs,mainFlushMaxMs,mainFlushCount,mainFlushWorst,pointerMoves,markers,flags\n";
        }

        void printSummary(State& s, double t, const char* why)
        {
            Summary& m = s.summary;
            if (m.loops == 0)
                return;

            const double secs = max(t - m.start, 1e-6);
            ostringstream line;
            line << "PLAYBACK-TRACE: ---- summary (" << why << ") over " << fixed << setprecision(2) << secs << "s: loops=" << m.loops
                 << " (" << (m.loops / secs) << "/s) framesShown=" << m.framesShown << " (" << (m.framesShown / secs) << " fps)"
                 << " deviceHz=" << m.deviceHz << " measuredRefresh=" << medianInterval(s.intervals) << "ms"
                 << " skipEvents=" << m.skipEvents << " skippedFrames=" << m.skippedFrames << " judders=" << m.judders
                 << " lateLoops=" << m.lateLoops << " notCached=" << m.misses << " buffering=" << m.buffering
                 << " mainFlushes=" << m.mainFlushes << " slowMainFlushes=" << m.slowMainFlushes << " mainFlushMax=" << m.mainFlushMaxMs
                 << "ms worstInterval=" << m.worstIntervalMs << "ms@frame" << m.worstIntervalFrame;

            if (!m.lateCulprits.empty())
            {
                line << " lateCulprits{";
                for (const auto& c : m.lateCulprits)
                    line << " " << c.first << "=" << c.second;
                line << " }";
            }

            line << " evalPaths{";
            for (const auto& p : m.evalPaths)
                line << " " << p.first << "=" << p.second;
            line << " }";

            writeLine(s, line.str());

            if (s.csv.is_open())
                s.csv.flush();

            m = Summary();
            m.start = t;
        }
    } // namespace

    void PlaybackTrace::initFromEnvironment()
    {
        State& s = state();
        if (m_enabled || s.initialized)
            return;
        s.initialized = true;

        const string value = envOrEmpty("RV_PLAYBACK_TRACE");
        if (value == "0")
        {
            cout << "INFO: PlaybackTrace disabled (RV_PLAYBACK_TRACE=0)" << endl;
            return;
        }

        setEnabled(true, value);
    }

    void PlaybackTrace::setEnabled(bool b, const string& path)
    {
        State& s = state();
        s.initialized = true;

        if (s.csv.is_open())
            s.csv.close();
        if (s.log.is_open())
            s.log.close();

        m_enabled = b;
        m_current = Record();
        s.inEvent = false;
        s.eventContainsLoop = false;
        s.events = EventWindow();
        s.widgets = WidgetWindow();
        s.pendingMarkers.clear();
        s.csvBytes = 0.0;
        s.csvCapped = false;

        if (!b)
            return;

        const string maxMB = envOrEmpty("RV_PLAYBACK_TRACE_MAX_MB");
        if (!maxMB.empty() && atof(maxMB.c_str()) > 0.0)
            s.csvMaxBytes = atof(maxMB.c_str()) * 1024.0 * 1024.0;

        fs::path csvFile;
        error_code ec;
        const bool explicitCsv = !path.empty() && path != "1" && fs::path(path).extension() == ".csv";

        if (explicitCsv)
        {
            csvFile = fs::path(path);
        }
        else
        {
            fs::path folder;
            if (path.empty() || path == "1")
            {
                folder = defaultFolder();
            }
            else
            {
                folder = fs::path(path);
                fs::create_directories(folder, ec);
                if (!fs::is_directory(folder, ec))
                    folder = defaultFolder();
            }

            pruneOldRuns(folder, maxKeptRuns - 1);

            const string stem = "rv_trace_" + wallClock("%Y%m%d_%H%M%S");
            csvFile = folder / (stem + ".csv");
            for (int i = 2; fs::exists(csvFile, ec) && i < 100; ++i)
                csvFile = folder / (stem + "_" + to_string(i) + ".csv");
        }

        fs::path logFile = csvFile;
        logFile.replace_extension(".log");

        s.csvPath = csvFile.string();
        s.logPath = logFile.string();

        s.log.open(s.logPath.c_str(), ios::out | ios::trunc);
        s.csv.open(s.csvPath.c_str(), ios::out | ios::trunc);

        if (s.csv.is_open())
            writeHeader(s.csv);

        cout << "INFO: PlaybackTrace enabled (set RV_PLAYBACK_TRACE=0 to disable)" << endl;
        if (!s.log.is_open())
            cout << "WARNING: PlaybackTrace: cannot open " << s.logPath << ", console only" << endl;
        if (!s.csv.is_open())
            cout << "WARNING: PlaybackTrace: cannot open " << s.csvPath << ", no CSV" << endl;

        info("started " + wallClock("%Y-%m-%d %H:%M:%S") + " (trace clock t=0 is the first trace call)");
        info("log: " + s.logPath);
        info("csv: " + s.csvPath);
        for (const char* name :
             {"RV_PLAYBACK_TRACE", "RV_AVPLAYBACK_VERSION", "RV_VSYNC_RATE_FORCE", "RV_VSYNC_IGNORE_DEVICE", "RV_VSYNC_ADD_OFFSET",
              "RV_VSYNC_MAX_INTERVAL", "RV_VSYNC_MIN_INTERVAL", "QSG_RHI_BACKEND", "QT_QUICK_BACKEND", "QT_OPENGL", "SESSIONNAME"})
        {
            const string v = envOrEmpty(name);
            if (!v.empty())
                info(string("env ") + name + "=" + v);
        }
    }

    const string& PlaybackTrace::logPath() { return state().logPath; }

    const string& PlaybackTrace::csvPath() { return state().csvPath; }

    double PlaybackTrace::now()
    {
        using namespace std::chrono;
        static const steady_clock::time_point origin = steady_clock::now();
        return duration<double>(steady_clock::now() - origin).count();
    }

    void PlaybackTrace::info(const string& line)
    {
        if (!m_enabled)
            return;
        writeLine(state(), "PLAYBACK-TRACE-INFO: " + line);
    }

    void PlaybackTrace::mark(const string& what)
    {
        if (!m_enabled)
            return;

        State& s = state();
        const double t = now();

        ostringstream line;
        line << fixed << setprecision(4) << "PLAYBACK-TRACE: t=" << t << " MARK " << what;
        writeLine(s, line.str());

        if (s.pendingMarkers.size() < maxMarkerChars)
        {
            ostringstream m;
            m << fixed << setprecision(4) << (s.pendingMarkers.empty() ? "" : "; ") << what << "@" << t;
            s.pendingMarkers += m.str();
        }
    }

    void PlaybackTrace::pointerMove()
    {
        if (m_enabled)
            ++state().pendingPointerMoves;
    }

    void PlaybackTrace::mainWindowFlush(const string& description, double ms)
    {
        if (!m_enabled)
            return;

        State& s = state();
        s.flushMs += ms;
        ++s.flushCount;
        if (ms > s.flushMaxMs)
        {
            s.flushMaxMs = ms;
            s.flushWorst = description;
        }
    }

    void PlaybackTrace::heartbeat(bool redraw)
    {
        if (!m_enabled)
            return;

        State& s = state();
        const double t = now();

        if (s.lastHeartbeat > 0.0)
            m_current.maxHeartbeatGapMs = max(m_current.maxHeartbeatGapMs, (t - s.lastHeartbeat) * 1000.0);
        s.lastHeartbeat = t;
        ++m_current.heartbeats;

        if (redraw)
        {
            if (m_current.redrawRequests == 0)
                m_current.firstRedrawRequest = t;
            ++m_current.redrawRequests;
        }
    }

    void PlaybackTrace::beginLoop(const char* backend)
    {
        if (!m_enabled)
            return;

        State& s = state();
        s.backend = backend;
        m_current.loopStart = now();

        //  Snapshot the Qt events delivered since the previous loop. The event
        //  carrying this paint is still open; its dispatch time up to here is
        //  reported separately, and its remainder after endLoop() goes to the
        //  next window.
        Record& r = m_current;
        r.eventsBusyMs = s.events.busyMs;
        r.eventCount = s.events.count;

        ostringstream top;
        top << fixed << setprecision(2);
        const double origin = s.prevLoopEnd > 0.0 ? s.prevLoopEnd : r.loopStart;
        for (const EventSample& e : s.events.top)
        {
            if (top.tellp() > 0)
                top << "; ";
            top << e.description << " " << e.ms << "ms@+" << max(0.0, (e.start - origin) * 1000.0);
        }
        r.topEvents = top.str();

        if (s.inEvent)
        {
            r.paintEventType = s.eventType;
            r.paintDispatchMs = (r.loopStart - s.eventStart) * 1000.0;
            s.eventContainsLoop = true;
        }
        else
        {
            r.paintEventType = "none";
        }

        s.events = EventWindow();

        r.widgetPaintMs = s.widgets.paintMs;
        r.widgetPaintCount = s.widgets.paintCount;
        r.widgetPaints = s.widgets.paintSummary();
        r.widgetChanges = s.widgets.changeSummary();
        s.widgets = WidgetWindow();
    }

    void PlaybackTrace::widgetEvent(bool paint, const string& description, double ms)
    {
        if (!m_enabled)
            return;

        WidgetWindow& w = state().widgets;
        if (paint)
            w.addPaint(description, ms);
        else
            w.addChange(description);
    }

    void PlaybackTrace::eventBegin(double start, const char* typeName)
    {
        if (!m_enabled)
            return;

        State& s = state();
        s.inEvent = true;
        s.eventStart = start;
        s.eventType = typeName;
        s.eventContainsLoop = false;
    }

    void PlaybackTrace::eventEnd(double end, const string& description)
    {
        if (!m_enabled)
            return;

        State& s = state();
        if (!s.inEvent)
            return;
        s.inEvent = false;

        if (s.eventContainsLoop)
        {
            //  Only the part after the loop ended belongs to the next window.
            const double tailStart = max(s.eventStart, s.prevLoopEnd);
            const double tailMs = max(0.0, (end - tailStart) * 1000.0);
            s.events.add(tailStart, tailMs, tailMs >= eventDetailMs ? string(s.eventType) + "(after-render)" : string());
        }
        else
        {
            s.events.add(s.eventStart, (end - s.eventStart) * 1000.0, description);
        }
        s.eventContainsLoop = false;
    }

    void PlaybackTrace::endLoop()
    {
        if (!m_enabled)
            return;

        State& s = state();
        Record& r = m_current;
        r.loopEnd = now();

        //  Take what accumulated since the previous loop.
        r.markers.swap(s.pendingMarkers);
        s.pendingMarkers.clear();
        r.pointerMoves = s.pendingPointerMoves;
        s.pendingPointerMoves = 0;
        r.mainFlushMs = s.flushMs;
        r.mainFlushMaxMs = s.flushMaxMs;
        r.mainFlushCount = s.flushCount;
        const string flushWorst = s.flushWorst;
        s.flushMs = 0.0;
        s.flushMaxMs = 0.0;
        s.flushCount = 0;
        s.flushWorst.clear();

        const bool playStart = r.playing && !s.wasPlaying;
        const bool playStop = !r.playing && s.wasPlaying;

        if (playStart)
        {
            s.loopsSincePlay = 0;
            s.haveShownFrame = false;
            s.intervals.clear();
            s.recentLines.clear();
            s.summary = Summary();
            s.summary.start = r.loopStart;
            ostringstream line;
            line << "PLAYBACK-TRACE: ---- play start at frame " << r.shownFrame << " fps=" << r.fps << " realtime=" << r.realtime
                 << " audio=" << r.hasAudio << " deviceHz=" << r.deviceHz << " backend=" << s.backend;
            writeLine(s, line.str());
        }

        const double intervalMs = s.prevLoopStart > 0.0 ? (r.loopStart - s.prevLoopStart) * 1000.0 : 0.0;
        const double outsideMs = s.prevLoopEnd > 0.0 ? (r.loopStart - s.prevLoopEnd) * 1000.0 : 0.0;
        const double loopMs = (r.loopEnd - r.loopStart) * 1000.0;
        const double requestToPaintMs = r.redrawRequests ? (r.loopStart - r.firstRedrawRequest) * 1000.0 : -1.0;

        //  Time between loops spent neither in a Qt event nor dispatching
        //  the paint: the thread was waiting, or busy outside Qt event
        //  delivery (native message handling, OS).
        const double eventsIdleMs = max(0.0, outsideMs - r.eventsBusyMs - r.paintDispatchMs);

        if (r.playing && s.loopsSincePlay > 0 && intervalMs > 0.0)
        {
            s.intervals.push_back(intervalMs);
            if (s.intervals.size() > intervalHistory)
                s.intervals.pop_front();
        }

        const double measuredMs = medianInterval(s.intervals);
        double refreshMs = r.deviceHz > 0.0 ? 1000.0 / r.deviceHz : measuredMs;
        if (refreshMs <= 0.0)
            refreshMs = 1000.0 / 60.0;

        const bool analyze = r.playing && s.loopsSincePlay >= warmupLoops;
        ostringstream flags;
        flags << fixed << setprecision(1);
        double holdMs = 0.0;

        //  Frame cadence: how long the previous frame stayed on screen.
        if (r.playing && (!s.haveShownFrame || r.shownFrame != s.lastShownFrame))
        {
            if (s.haveShownFrame)
            {
                holdMs = (r.loopStart - s.frameShownSince) * 1000.0;
                const int advanced = r.inc ? abs((r.shownFrame - s.lastShownFrame) / r.inc) : 1;
                const double expectedMs = r.fps > 0.0 ? 1000.0 / r.fps : holdMs;

                if (analyze && !r.wrapped && advanced >= 1 && fabs(holdMs - expectedMs) > 0.75 * refreshMs)
                {
                    flags << " JUDDER(hold=" << holdMs << "ms expected=" << expectedMs << "ms)";
                    ++s.summary.judders;
                }
            }

            s.lastShownFrame = r.shownFrame;
            s.frameShownSince = r.loopStart;
            s.haveShownFrame = true;
            ++s.summary.framesShown;
        }

        //  The frame moving against the play direction while playing, or
        //  changing at all shortly after a stop.
        if (s.haveRowFrame && !r.wrapped && r.shownFrame != s.lastRowFrame)
        {
            const int dir = r.playing ? (r.inc < 0 ? -1 : 1) : s.lastPlayInc;
            const bool backward = (r.shownFrame - s.lastRowFrame) * dir < 0;
            const bool afterStop = !r.playing && s.lastStopTime >= 0.0 && r.loopStart - s.lastStopTime < afterStopWindow;

            if (afterStop)
                flags << " FRAME-CHANGED-AFTER-STOP(" << s.lastRowFrame << "->" << r.shownFrame << (backward ? " backward" : "") << ")";
            else if (r.playing && backward)
                flags << " BACKWARD(" << s.lastRowFrame << "->" << r.shownFrame << ")";
        }

        if (analyze)
        {
            ++s.summary.loops;
            s.summary.deviceHz = r.deviceHz;
            s.summary.mainFlushes += r.mainFlushCount;
            s.summary.mainFlushMaxMs = max(s.summary.mainFlushMaxMs, r.mainFlushMaxMs);

            if (intervalMs > s.summary.worstIntervalMs)
            {
                s.summary.worstIntervalMs = intervalMs;
                s.summary.worstIntervalFrame = r.shownFrame;
            }

            if (r.skipped > 0)
            {
                flags << " SKIP(" << r.skipped << ")";
                ++s.summary.skipEvents;
                s.summary.skippedFrames += r.skipped;
            }

            if (r.evalPath[0] && !r.frameCached)
            {
                flags << " NOT-CACHED";
                ++s.summary.misses;
            }

            if (r.bufferStop || r.bufferRefill || r.bufferWait)
            {
                flags << " BUFFERING(stop=" << r.bufferStop << " refill=" << r.bufferRefill << " wait=" << r.bufferWait << ")";
                ++s.summary.buffering;
            }

            if (r.mainFlushMaxMs > mainFlushFlagMs)
            {
                flags << " MAIN-WINDOW-FLUSH(max=" << r.mainFlushMaxMs << "ms n=" << r.mainFlushCount << " " << flushWorst << ")";
                ++s.summary.slowMainFlushes;
            }

            if (r.evalPath[0])
                ++s.summary.evalPaths[r.evalPath];

            if (intervalMs > 1.5 * refreshMs)
            {
                //  Attribute the late loop to the largest contributor. The
                //  interval runs from the previous loop's start to this
                //  loop's start, so it is the previous loop's stages plus
                //  the time between the loops; this loop's own stages are
                //  not part of it. The renderer total includes upload and
                //  user render (Mu/Python render hooks), so those are split
                //  out. On GL, present includes the buffer swap (vsync).
                const Record& p = s.prevRecord;
                const double rendererOther = max(0.0, p.rendererMs - p.uploadMs - p.userRenderMs);
                const double presentOther = max(0.0, p.presentMs - p.fenceWaitMs - p.acquireMs);
                const pair<const char*, double> stages[] = {
                    {"outside-render(event-loop)", outsideMs},
                    {"prev-loop/pre-render-event", p.preRenderEventMs},
                    {"prev-loop/evaluate", p.evaluateMs},
                    {"prev-loop/wait-upload", p.waitUploadMs},
                    {"prev-loop/pre-eval", p.preEvalMs},
                    {"prev-loop/texture-upload", p.uploadMs},
                    {"prev-loop/user-render(Mu/Py)", p.userRenderMs},
                    {"prev-loop/renderer-other", rendererOther},
                    {"prev-loop/frame-changed-event", p.frameChangedMs},
                    {"prev-loop/present-fence-wait", p.fenceWaitMs},
                    {"prev-loop/present-acquire(vsync)", p.acquireMs},
                    {"prev-loop/present(swap)", presentOther},
                    {"prev-loop/output-present", p.outputPresentMs},
                    {"prev-loop/post-render", p.postRenderMs},
                };

                const auto* worst =
                    max_element(begin(stages), end(stages), [](const auto& a, const auto& b) { return a.second < b.second; });

                string culprit = worst->first;
                if (worst->second == outsideMs)
                {
                    if (r.maxHeartbeatGapMs > 1.5 * refreshMs)
                        culprit += "/heartbeat-starved";
                    else if (requestToPaintMs > refreshMs)
                        culprit += "/paint-delivery-delay";

                    //  Was the main thread busy in Qt events (name the
                    //  longest), or idle / outside Qt event delivery?
                    if (r.mainFlushMs >= 0.5 * outsideMs)
                        culprit += "/main-window-flush";
                    else if (r.eventsBusyMs >= 0.5 * outsideMs && !r.topEvents.empty())
                        culprit += "/qt-event:" + r.topEvents.substr(0, r.topEvents.find(" "));
                    else if (eventsIdleMs >= 0.5 * outsideMs)
                        culprit += "/not-in-qt-events";
                    else if (r.paintDispatchMs >= 0.5 * outsideMs)
                        culprit += "/paint-dispatch";
                }

                flags << " LATE(interval=" << intervalMs << "ms refresh=" << refreshMs << "ms culprit=" << culprit << " " << worst->second
                      << "ms)";
                ++s.summary.lateLoops;
                ++s.summary.lateCulprits[culprit];
            }
        }

        const string flagStr = flags.str();

        if (s.csv.is_open() && !s.csvCapped)
        {
            ostringstream o;
            o << fixed << setprecision(4) << r.loopStart << "," << s.backend << "," << r.playing << "," << r.realtime << "," << r.hasAudio
              << "," << r.fps << "," << r.deviceHz << "," << r.syncIntervalMs << "," << refreshMs << "," << intervalMs << "," << loopMs
              << "," << outsideMs << "," << r.heartbeats << "," << r.maxHeartbeatGapMs << "," << r.redrawRequests << "," << requestToPaintMs
              << "," << r.elapsed << "," << (r.predictedSync * 1000.0) << "," << r.prevFrame << "," << r.targetFrame << "," << r.shownFrame
              << "," << r.skipped << "," << r.shiftBefore << "," << r.shiftAfter << "," << r.frameCached << "," << r.evalPath << ","
              << r.cacheLockMs << "," << r.bufferWait << "," << r.bufferStop << "," << r.bufferRefill << "," << r.wrapped << ","
              << r.preRenderEventMs << "," << r.evaluateMs << "," << r.waitUploadMs << "," << r.preEvalMs << "," << r.rendererMs << ","
              << r.uploadMs << "," << r.userRenderMs << "," << r.frameChangedMs << "," << r.presentMs << "," << r.fenceWaitMs << ","
              << r.acquireMs << "," << r.outputPresentMs << "," << r.postRenderMs << "," << holdMs << "," << r.eventsBusyMs << ","
              << r.eventCount << "," << eventsIdleMs << "," << r.paintEventType << "," << r.paintDispatchMs << ",\"" << csvText(r.topEvents)
              << "\"," << r.widgetPaintMs << "," << r.widgetPaintCount << ",\"" << csvText(r.widgetPaints) << "\",\""
              << csvText(r.widgetChanges) << "\"," << r.mainFlushMs << "," << r.mainFlushMaxMs << "," << r.mainFlushCount << ",\""
              << csvText(flushWorst) << "\"," << r.pointerMoves << ",\"" << csvText(r.markers) << "\"," << flagStr << "\n";

            const string row = o.str();
            s.csv << row;
            s.csvBytes += double(row.size());

            if (s.csvBytes > s.csvMaxBytes)
            {
                s.csvCapped = true;
                s.csv.flush();
                writeLine(s, "PLAYBACK-TRACE-INFO: CSV size limit reached (RV_PLAYBACK_TRACE_MAX_MB), no more CSV rows; the log continues");
            }
        }

        //  Full description of this loop, printed for flagged loops and
        //  around stops.
        ostringstream line;
        line << fixed << setprecision(2) << "PLAYBACK-TRACE: t=" << setprecision(4) << r.loopStart << setprecision(2)
             << (r.playing ? " play" : " idle") << " frame " << r.prevFrame << "->" << r.shownFrame << " (target " << r.targetFrame << ")"
             << flagStr << " | interval=" << intervalMs << " outside=" << outsideMs << " loop=" << loopMs << " hb=" << r.heartbeats
             << " hbGap=" << r.maxHeartbeatGapMs << " reqToPaint=" << requestToPaintMs << " sync=" << (r.predictedSync * 1000.0)
             << " syncInterval=" << r.syncIntervalMs << " eval=" << r.evaluateMs << "(" << r.evalPath << " lock=" << r.cacheLockMs
             << ") render=" << r.rendererMs << "(upload=" << r.uploadMs << " user=" << r.userRenderMs << ") fchg=" << r.frameChangedMs
             << " present=" << r.presentMs << "(fence=" << r.fenceWaitMs << " acq=" << r.acquireMs << ") out=" << r.outputPresentMs
             << " post=" << r.postRenderMs << " | events: busy=" << r.eventsBusyMs << "ms n=" << r.eventCount << " idle=" << eventsIdleMs
             << "ms paintVia=" << r.paintEventType << " dispatch=" << r.paintDispatchMs << "ms";
        if (!r.topEvents.empty())
            line << " top{ " << r.topEvents << " }";
        if (r.mainFlushCount > 0)
            line << " | main flush: " << r.mainFlushMs << "ms n=" << r.mainFlushCount << " max=" << r.mainFlushMaxMs << "ms " << flushWorst;
        if (r.widgetPaintCount > 0)
            line << " | widget paints: " << r.widgetPaintMs << "ms n=" << r.widgetPaintCount << " top{ " << r.widgetPaints << " }";
        if (!r.widgetChanges.empty())
            line << " | widget changes{ " << r.widgetChanges << " }";
        if (r.pointerMoves > 0)
            line << " | pointerMoves=" << r.pointerMoves;
        if (!r.markers.empty())
            line << " | markers{ " << r.markers << " }";

        if (playStop)
        {
            //  The loops leading up to the stop, which were not printed.
            writeLine(s, "PLAYBACK-TRACE: ---- play stop; last loops before it:");
            for (const string& l : s.recentLines)
                writeLine(s, "PLAYBACK-TRACE: PRE-STOP " + l.substr(16));
            s.recentLines.clear();
            s.lastStopTime = r.loopStart;
            s.postStopRemaining = postStopLines;
        }

        if (!flagStr.empty() || s.postStopRemaining > 0)
        {
            writeLine(s, s.postStopRemaining > 0 ? "PLAYBACK-TRACE: POST-STOP " + line.str().substr(16) : line.str());
            if (s.postStopRemaining > 0)
                --s.postStopRemaining;
        }
        else if (r.playing)
        {
            s.recentLines.push_back(line.str());
            if (s.recentLines.size() > preStopLines)
                s.recentLines.pop_front();
        }

        if (analyze && r.loopStart - s.summary.start >= summaryPeriod)
            printSummary(s, r.loopStart, "periodic");

        if (playStop)
            printSummary(s, r.loopStart, "play stop");

        if (r.playing)
            s.lastPlayInc = r.inc < 0 ? -1 : 1;
        s.haveRowFrame = true;
        s.lastRowFrame = r.shownFrame;

        s.wasPlaying = r.playing;
        ++s.loopsSincePlay;
        s.prevLoopStart = r.loopStart;
        s.prevLoopEnd = r.loopEnd;
        s.prevRecord = r;
        r = Record();
    }

} // namespace IPCore
