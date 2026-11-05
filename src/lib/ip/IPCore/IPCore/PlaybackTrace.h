//******************************************************************************
// Copyright (c) 2026 Autodesk Inc. All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0
//
//******************************************************************************
#ifndef __IPCore__PlaybackTrace__h__
#define __IPCore__PlaybackTrace__h__

#include <string>

namespace IPCore
{

    //
    //  PlaybackTrace: per-frame playback timing trace used to find the origin
    //  of stutter / skipped frames. Unlike -debug gpu (60-frame averages)
    //  every render loop is recorded, so single-frame hitches are visible.
    //
    //  ON BY DEFAULT in this instrumented build. Controlled by
    //  RV_PLAYBACK_TRACE:
    //
    //      (unset) or 1                  trace to the default folder
    //      0                             disabled
    //      C:/some/folder                trace to that folder
    //      C:/tmp/trace.csv              CSV at that path, log next to it
    //
    //  Default folder: <Documents>/RV_playback_trace (falls back to the home
    //  or temp directory). Each run writes two files with the same stem:
    //
    //      rv_trace_<date>_<time>.log    readable log: system info, markers
    //                                    (input, play/stop, setFrame),
    //                                    anomalies, summaries
    //      rv_trace_<date>_<time>.csv    one row per render loop, playing
    //                                    or not
    //
    //  Only the newest maxKeptRuns runs are kept in the default folder. The
    //  CSV stops growing after RV_PLAYBACK_TRACE_MAX_MB (default 1024 MB);
    //  the log continues.
    //
    //  All calls are made from the main (GUI) thread.
    //

    class PlaybackTrace
    {
    public:
        struct Record
        {
            //  Timeline, absolute seconds (trace clock)
            double loopStart = 0.0;
            double loopEnd = 0.0;

            //  Heartbeat (QTimer -> Session::update) between two loops
            int heartbeats = 0;
            double maxHeartbeatGapMs = 0.0;
            int redrawRequests = 0;
            double firstRedrawRequest = 0.0;

            //  Session state
            bool playing = false;
            bool realtime = false;
            bool hasAudio = false;
            bool bufferWait = false;
            bool bufferStop = false;
            bool bufferRefill = false;
            bool wrapped = false;
            int inc = 1;
            double fps = 0.0;
            double deviceHz = 0.0;
            double syncIntervalMs = 0.0;

            //  Frame decision
            double elapsed = 0.0;
            double predictedSync = 0.0;
            int prevFrame = 0;
            int targetFrame = 0;
            int shownFrame = 0;
            int skipped = 0;
            int shiftBefore = 0;
            int shiftAfter = 0;

            //  Cache / evaluation (IPGraph::evaluateAtFrame)
            bool frameCached = false;
            const char* evalPath = "";
            double cacheLockMs = 0.0;

            //  Stage durations, ms
            double preRenderEventMs = 0.0;
            double evaluateMs = 0.0;
            double waitUploadMs = 0.0;
            double preEvalMs = 0.0;
            double rendererMs = 0.0;
            double uploadMs = 0.0;
            double userRenderMs = 0.0;
            double frameChangedMs = 0.0;
            double presentMs = 0.0;
            double outputPresentMs = 0.0;
            double fenceWaitMs = 0.0;
            double acquireMs = 0.0;
            double postRenderMs = 0.0;

            //  Main-thread Qt events delivered between the previous loop's
            //  end and this loop's start (see eventBegin/eventEnd).
            double eventsBusyMs = 0.0;
            int eventCount = 0;
            std::string topEvents;
            const char* paintEventType = "";
            double paintDispatchMs = 0.0;

            //  QWidget paints and geometry changes in the same window (see
            //  widgetEvent): which widgets repainted, and which changed
            //  size or visibility (what triggers a relayout).
            double widgetPaintMs = 0.0;
            int widgetPaintCount = 0;
            std::string widgetPaints;
            std::string widgetChanges;

            //  Top-level (main window, panels, tooltips) repaint/flush
            //  deliveries in the same window (see mainWindowFlush).
            double mainFlushMs = 0.0;
            double mainFlushMaxMs = 0.0;
            int mainFlushCount = 0;

            //  Pointer moves delivered in the same window (hover rate).
            int pointerMoves = 0;

            //  Markers (input, play/stop, setFrame) since the previous loop.
            std::string markers;
        };

        //  Runs kept in the default folder (older ones are deleted).
        static constexpr int maxKeptRuns = 10;

        static bool enabled() { return m_enabled; }

        //  Enables or disables the trace according to RV_PLAYBACK_TRACE (on
        //  when unset). Does nothing if already enabled. Safe to call more
        //  than once.
        static void initFromEnvironment();

        //  path: "" or "1" for the default folder, a folder, or a .csv path.
        static void setEnabled(bool b, const std::string& path = "");

        static const std::string& logPath();
        static const std::string& csvPath();

        static double now();

        static Record& current() { return m_current; }

        //  A line of configuration / system information (printed and logged).
        static void info(const std::string& line);

        //  A timestamped marker: printed, logged and attached to the next
        //  CSV row. Use for rare events (input, play/stop, setFrame).
        static void mark(const std::string& what);

        //  A pointer move was delivered (counted only).
        static void pointerMove();

        //  A top-level window repaint/flush (QEvent::UpdateRequest to a
        //  window) took ms. description must not contain spaces.
        static void mainWindowFlush(const std::string& description, double ms);

        //  Session::update(): QTimer heartbeat; redraw = a repaint was requested
        static void heartbeat(bool redraw);

        //  Top and bottom of the view's per-frame render (VulkanWindow,
        //  GLWindow). endLoop() writes the CSV row and analyzes the frame.
        static void beginLoop(const char* backend);
        static void endLoop();

        //  Main-thread Qt event accounting, called by the application's
        //  QApplication::notify() override for top-level (non-nested)
        //  deliveries only. typeName must have static storage. The caller
        //  builds a description (receiver class/name, parent) only when the
        //  delivery took at least eventDetailMs, otherwise passes "".
        static constexpr double eventDetailMs = 0.25;
        static void eventBegin(double start, const char* typeName);
        static void eventEnd(double end, const std::string& description);

        //  A nested QWidget event: Paint (with its duration), or a geometry
        //  or visibility change (Resize, Show, Hide, LayoutRequest; ms is
        //  the handling time). Description must not contain spaces.
        static void widgetEvent(bool paint, const std::string& description, double ms);

        //  Accumulates the scope's duration (ms) into a Record field.
        class Scope
        {
        public:
            Scope(double& accumulatorMs)
                : m_accumulatorMs(m_enabled ? &accumulatorMs : nullptr)
                , m_start(m_enabled ? now() : 0.0)
            {
            }

            ~Scope()
            {
                if (m_accumulatorMs)
                    *m_accumulatorMs += (now() - m_start) * 1000.0;
            }

            Scope(const Scope&) = delete;
            Scope& operator=(const Scope&) = delete;

        private:
            double* m_accumulatorMs;
            double m_start;
        };

    private:
        static bool m_enabled;
        static Record m_current;
    };

} // namespace IPCore

#endif // __IPCore__PlaybackTrace__h__
