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
#include <deque>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

namespace IPCore
{
    using namespace std;

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

        struct Summary
        {
            double start = 0.0;
            int loops = 0;
            int framesShown = 0;
            int skipEvents = 0;
            int skippedFrames = 0;
            int judders = 0;
            int cadenceLocks = 0;
            int lateLoops = 0;
            int misses = 0;
            int buffering = 0;
            double worstIntervalMs = 0.0;
            int worstIntervalFrame = 0;
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
            ofstream csv;
            string path;
            string backend;

            //  Qt event accounting
            bool inEvent = false;
            double eventStart = 0.0;
            const char* eventType = "";
            bool eventContainsLoop = false;
            EventWindow events;
            WidgetWindow widgets;

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

            Summary summary;
        };

        State& state()
        {
            static State s;
            return s;
        }

        string defaultPath()
        {
            const char* dir = getenv("TEMP");
            if (!dir)
                dir = getenv("TMP");
            if (!dir)
                dir = "/tmp";
            return string(dir) + "/rv_playback_trace.csv";
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
            return s;
        }

        void writeHeader(ostream& o)
        {
            o << "t,backend,playing,realtime,audio,fps,deviceHz,refreshMs,"
                 "intervalMs,loopMs,outsideMs,heartbeats,maxHeartbeatGapMs,redrawRequests,requestToPaintMs,"
                 "elapsed,predictedSyncMs,prevFrame,targetFrame,shownFrame,skipped,cadenceLocked,shiftBefore,shiftAfter,"
                 "frameCached,evalPath,cacheLockMs,bufferWait,bufferStop,bufferRefill,wrapped,"
                 "preRenderEventMs,evaluateMs,waitUploadMs,preEvalMs,rendererMs,uploadMs,userRenderMs,frameChangedMs,"
                 "presentMs,fenceWaitMs,acquireMs,outputPresentMs,postRenderMs,holdMs,"
                 "eventsBusyMs,eventCount,eventsIdleMs,paintEventType,paintDispatchMs,topEvents,"
                 "widgetPaintMs,widgetPaintCount,widgetPaints,widgetChanges,flags\n";
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
                 << " skipEvents=" << m.skipEvents << " skippedFrames=" << m.skippedFrames << " judders=" << m.judders
                 << " cadenceLocks=" << m.cadenceLocks << " lateLoops=" << m.lateLoops << " notCached=" << m.misses
                 << " buffering=" << m.buffering << " worstInterval=" << m.worstIntervalMs << "ms@frame" << m.worstIntervalFrame;

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

            line << " csv=" << s.path;
            cout << line.str() << endl;

            if (s.csv.is_open())
                s.csv.flush();

            m = Summary();
            m.start = t;
        }
    } // namespace

    void PlaybackTrace::setEnabled(bool b, const string& path)
    {
        State& s = state();

        if (s.csv.is_open())
            s.csv.close();

        m_enabled = b;
        m_current = Record();
        s.inEvent = false;
        s.eventContainsLoop = false;
        s.events = EventWindow();
        s.widgets = WidgetWindow();

        if (!b)
            return;

        s.path = (path.empty() || path == "1") ? defaultPath() : path;
        s.csv.open(s.path.c_str(), ios::out | ios::trunc);

        if (s.csv.is_open())
        {
            writeHeader(s.csv);
            cout << "INFO: PlaybackTrace enabled, writing " << s.path << endl;
        }
        else
        {
            cout << "WARNING: PlaybackTrace: cannot open " << s.path << ", stdout only" << endl;
        }
    }

    double PlaybackTrace::now()
    {
        using namespace std::chrono;
        static const steady_clock::time_point origin = steady_clock::now();
        return duration<double>(steady_clock::now() - origin).count();
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

        if (!r.playing && !s.wasPlaying)
        {
            s.prevLoopStart = r.loopStart;
            s.prevLoopEnd = r.loopEnd;
            s.prevRecord = r;
            r = Record();
            return;
        }

        if (r.playing && !s.wasPlaying)
        {
            s.loopsSincePlay = 0;
            s.haveShownFrame = false;
            s.intervals.clear();
            s.summary = Summary();
            s.summary.start = r.loopStart;
            cout << "PLAYBACK-TRACE: ---- play start at frame " << r.shownFrame << " fps=" << r.fps << " realtime=" << r.realtime
                 << " audio=" << r.hasAudio << " backend=" << s.backend << endl;
        }

        const double intervalMs = s.prevLoopStart > 0.0 ? (r.loopStart - s.prevLoopStart) * 1000.0 : 0.0;
        const double outsideMs = s.prevLoopEnd > 0.0 ? (r.loopStart - s.prevLoopEnd) * 1000.0 : 0.0;
        const double loopMs = (r.loopEnd - r.loopStart) * 1000.0;
        const double requestToPaintMs = r.redrawRequests ? (r.loopStart - r.firstRedrawRequest) * 1000.0 : -1.0;

        //  Time between loops spent neither in a Qt event nor dispatching
        //  the paint: the thread was waiting, or busy outside Qt event
        //  delivery (native message handling, OS).
        const double eventsIdleMs = max(0.0, outsideMs - r.eventsBusyMs - r.paintDispatchMs);

        if (s.loopsSincePlay > 0 && intervalMs > 0.0)
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

        if (analyze)
        {
            ++s.summary.loops;

            if (intervalMs > s.summary.worstIntervalMs)
            {
                s.summary.worstIntervalMs = intervalMs;
                s.summary.worstIntervalFrame = r.shownFrame;
            }

            if (r.cadenceLocked)
                ++s.summary.cadenceLocks;

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
                    if (r.eventsBusyMs >= 0.5 * outsideMs && !r.topEvents.empty())
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

        if (s.csv.is_open())
        {
            ostream& o = s.csv;
            o << fixed << setprecision(4) << r.loopStart << "," << s.backend << "," << r.playing << "," << r.realtime << "," << r.hasAudio
              << "," << r.fps << "," << r.deviceHz << "," << refreshMs << "," << intervalMs << "," << loopMs << "," << outsideMs << ","
              << r.heartbeats << "," << r.maxHeartbeatGapMs << "," << r.redrawRequests << "," << requestToPaintMs << "," << r.elapsed << ","
              << (r.predictedSync * 1000.0) << "," << r.prevFrame << "," << r.targetFrame << "," << r.shownFrame << "," << r.skipped << ","
              << r.cadenceLocked << "," << r.shiftBefore << "," << r.shiftAfter << "," << r.frameCached << "," << r.evalPath << ","
              << r.cacheLockMs << "," << r.bufferWait << "," << r.bufferStop << "," << r.bufferRefill << "," << r.wrapped << ","
              << r.preRenderEventMs << "," << r.evaluateMs << "," << r.waitUploadMs << "," << r.preEvalMs << "," << r.rendererMs << ","
              << r.uploadMs << "," << r.userRenderMs << "," << r.frameChangedMs << "," << r.presentMs << "," << r.fenceWaitMs << ","
              << r.acquireMs << "," << r.outputPresentMs << "," << r.postRenderMs << "," << holdMs << "," << r.eventsBusyMs << ","
              << r.eventCount << "," << eventsIdleMs << "," << r.paintEventType << "," << r.paintDispatchMs << ",\"" << csvText(r.topEvents)
              << "\"," << r.widgetPaintMs << "," << r.widgetPaintCount << ",\"" << csvText(r.widgetPaints) << "\",\""
              << csvText(r.widgetChanges) << "\"," << flagStr << "\n";
        }

        if (!flagStr.empty())
        {
            ostringstream line;
            line << fixed << setprecision(2) << "PLAYBACK-TRACE: t=" << r.loopStart << " frame " << r.prevFrame << "->" << r.shownFrame
                 << " (target " << r.targetFrame << ")" << flagStr << " | interval=" << intervalMs << " outside=" << outsideMs
                 << " hb=" << r.heartbeats << " hbGap=" << r.maxHeartbeatGapMs << " reqToPaint=" << requestToPaintMs
                 << " sync=" << (r.predictedSync * 1000.0) << " eval=" << r.evaluateMs << "(" << r.evalPath << " lock=" << r.cacheLockMs
                 << ") render=" << r.rendererMs << "(upload=" << r.uploadMs << " user=" << r.userRenderMs << ") fchg=" << r.frameChangedMs
                 << " present=" << r.presentMs << "(fence=" << r.fenceWaitMs << " acq=" << r.acquireMs << ") out=" << r.outputPresentMs
                 << " post=" << r.postRenderMs << " | events: busy=" << r.eventsBusyMs << "ms n=" << r.eventCount
                 << " idle=" << eventsIdleMs << "ms paintVia=" << r.paintEventType << " dispatch=" << r.paintDispatchMs << "ms";
            if (!r.topEvents.empty())
                line << " top{ " << r.topEvents << " }";
            if (r.widgetPaintCount > 0)
                line << " | widget paints: " << r.widgetPaintMs << "ms n=" << r.widgetPaintCount << " top{ " << r.widgetPaints << " }";
            if (!r.widgetChanges.empty())
                line << " | widget changes{ " << r.widgetChanges << " }";
            cout << line.str() << endl;
        }

        if (analyze && r.loopStart - s.summary.start >= summaryPeriod)
            printSummary(s, r.loopStart, "periodic");

        if (!r.playing && s.wasPlaying)
            printSummary(s, r.loopStart, "play stop");

        s.wasPlaying = r.playing;
        ++s.loopsSincePlay;
        s.prevLoopStart = r.loopStart;
        s.prevLoopEnd = r.loopEnd;
        s.prevRecord = r;
        r = Record();
    }

} // namespace IPCore
