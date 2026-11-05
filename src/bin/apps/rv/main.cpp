//******************************************************************************
// Copyright (c) 2001-2005 Tweak Inc. All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0
//
//******************************************************************************
#ifdef _MSC_VER
//
//  We are targetting at least XP, (IE no windows95, etc).
//
#define WINVER 0x0501
#define _WIN32_WINNT 0x0501
#include <winsock2.h>
#include <windows.h>
#include <winnt.h>
#include <wincon.h>
#include <pthread.h>
//
//  NOTE: win_pthreads, which supplies implement.h, seems
//  targetted at an earlier version of windows (pre-XP).  If you
//  include implement.h here, it won't compile.  But as far as I
//  can tell, it's not needed, so just leave it out.
//
//  #include <implement.h>
#endif

#ifdef PLATFORM_LINUX
#include <sched.h>
#endif
#ifndef PLATFORM_LINUX
#include <GL/glew.h>
#endif
#include <RvCommon/GLView.h>
#include "../../utf8Main.h"
#include <RvCommon/QTUtils.h>
#include <RvCommon/RvApplication.h>
#include <RvCommon/RvConsoleWindow.h>
#include <RvCommon/MuUICommands.h>
#include <RvCommon/PyUICommands.h>
#include <RvCommon/RvPreferences.h>
#include <IOproxy/IOproxy.h>
#include <MovieProxy/MovieProxy.h>
#include <MovieSideCar/MovieSideCarIO.h>
#include <ImfHeader.h>
#include <ImfThreading.h>
#include <MovieFB/MovieFB.h>
#include <MovieProcedural/MovieProcedural.h>
#include <MuGL/GLModule.h>
#include <MuIO/IOModule.h>
#include <MuTwkApp/CrashHandlerInit.h>
#include <MuTwkApp/MuInterface.h>
#include <PyTwkApp/PyInterface.h>
#include <IPCore/Application.h>
#include <IPCore/NodeDefinition.h>
#include <RvApp/CommandsModule.h>
#include <RvApp/PyCommandsModule.h>
#include <RvApp/Options.h>
#include <RvPackage/PackageManager.h>
#include <TwkApp/Document.h>
#include <TwkDeploy/Deploy.h>
#include <QTBundle/QTBundle.h>
#include <TwkQtCoreUtil/QtConvert.h>
#include <TwkExc/TwkExcException.h>
#include <TwkFB/IO.h>
#include <TwkFB/TwkFBThreadPool.h>
#include <TwkMovie/MovieIO.h>
#include <TwkUtil/FrameUtils.h>
#include <TwkUtil/SystemInfo.h>
#include <TwkUtil/Daemon.h>
#include <TwkUtil/File.h>
#include <TwkUtil/PathConform.h>
#include <TwkUtil/FourCC.h>
#include <TwkUtil/ThreadName.h>
#include <TwkUtil/MemPool.h>
#include <TwkGLF/GLPixelBufferObjectPool.h>
#include <arg.h>
#include <fstream>
#include <iostream>
#include <iterator>
#include <thread>
#include <sched.h>
#include <sstream>
#include <stdlib.h>
#include <stl_ext/thread_group.h>
#include <signal.h>
#include <string.h>
#include <vector>

#ifndef PLATFORM_WINDOWS
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>
#else
#include <TwkGLFFBO/FBOVideoDevice.h>
#include <IPCore/ImageRenderer.h>
#endif

#if defined(RV_VFX_CY2023)
#include <QTextCodec>
#endif
#include <QtWidgets/QtWidgets>
#include <QtGui/QtGui>
#ifdef PLATFORM_WINDOWS
// For QQuickWindow::setGraphicsApi(); see the call site in utf8Main().
#include <QtQuick/QQuickWindow>
#include <QtQuick/QSGRendererInterface>
#endif
#include <RvCommon/RvDocument.h>
#include <IPCore/PlaybackTrace.h>
#include <algorithm>
#include <map>

namespace
{
    //
    //  QApplication that times main-thread event delivery for the playback
    //  trace (RV_PLAYBACK_TRACE). It shows which Qt events (timers, queued
    //  calls, input, plugin work) run between a redraw request and the paint
    //  when a frame is late. Without the trace this is one flag check per
    //  event.
    //
    class TracingApplication : public QApplication
    {
    public:
        using QApplication::QApplication;

        bool notify(QObject* receiver, QEvent* e) override
        {
            //  notify() also runs on other threads for their objects' events;
            //  only the main (GUI) thread is traced, and m_depth is only
            //  touched there.
            if (!IPCore::PlaybackTrace::enabled() || QThread::currentThread() != thread())
                return QApplication::notify(receiver, e);

            //  Nested delivery (sendEvent from inside an event, or a nested
            //  event loop) is part of the enclosing top-level event.
            if (m_depth > 0)
            {
                ++m_depth;
                DepthGuard guard(m_depth);

                //  Widget paints and geometry/visibility changes are traced
                //  individually: they show which widgets a main-window
                //  repaint covers, and which change triggered the relayout.
                const QEvent::Type type = e->type();
                const bool paint = type == QEvent::Paint;
                const bool change = type == QEvent::Resize || type == QEvent::Show || type == QEvent::Hide || type == QEvent::LayoutRequest;
                if (!(paint || change) || !receiver || !receiver->isWidgetType())
                    return QApplication::notify(receiver, e);

                std::string description = eventTypeName(type);
                description += ":" + widgetPath(receiver);
                if (paint)
                {
                    const QRect rect = static_cast<QPaintEvent*>(e)->rect();
                    description += "[" + std::to_string(rect.width()) + "x" + std::to_string(rect.height()) + "]";
                }
                else if (type == QEvent::Resize)
                {
                    const QResizeEvent* re = static_cast<QResizeEvent*>(e);
                    description += "[" + std::to_string(re->oldSize().width()) + "x" + std::to_string(re->oldSize().height()) + "->"
                                   + std::to_string(re->size().width()) + "x" + std::to_string(re->size().height()) + "]";
                }
                std::replace(description.begin(), description.end(), ' ', '_');

                const double start = IPCore::PlaybackTrace::now();
                const bool result = QApplication::notify(receiver, e);
                IPCore::PlaybackTrace::widgetEvent(paint, description, (IPCore::PlaybackTrace::now() - start) * 1000.0);
                return result;
            }

            //  Captured before delivery: the receiver may be deleted by it.
            const QEvent::Type type = e->type();
            const char* typeName = eventTypeName(type);
            const QMetaObject* receiverMeta = receiver ? receiver->metaObject() : nullptr;
            const QString receiverName = receiver ? receiver->objectName() : QString();
            const QObject* parent = receiver ? receiver->parent() : nullptr;
            const QMetaObject* parentMeta = parent ? parent->metaObject() : nullptr;
            const QString parentName = parent ? parent->objectName() : QString();

            //  Posted LayoutRequest/Show/Hide arrive as top-level events.
            std::string changeDescription;
            if (receiver && receiver->isWidgetType()
                && (type == QEvent::LayoutRequest || type == QEvent::Show || type == QEvent::Hide || type == QEvent::Resize))
            {
                changeDescription = std::string(typeName) + ":" + widgetPath(receiver);
                std::replace(changeDescription.begin(), changeDescription.end(), ' ', '_');
            }

            //  Input: pointer moves are counted; clicks and keys are markers,
            //  so a stop can be told apart as play button vs. spacebar.
            if (type == QEvent::MouseMove)
                IPCore::PlaybackTrace::pointerMove();
            else if (type == QEvent::MouseButtonPress || type == QEvent::MouseButtonRelease || type == QEvent::MouseButtonDblClick
                     || type == QEvent::KeyPress || type == QEvent::KeyRelease)
                markInput(receiver, e);

            //  A repaint/flush of a top-level window other than the viewport
            //  (the main window, a floating panel, a tooltip).
            std::string flushDescription;
            if (type == QEvent::UpdateRequest && receiver)
            {
                const bool widgetWindow = receiver->isWidgetType() && static_cast<QWidget*>(receiver)->isWindow();
                if (widgetWindow || receiver->inherits("QWidgetWindow"))
                {
                    flushDescription = widgetPath(receiver);
                    std::replace(flushDescription.begin(), flushDescription.end(), ' ', '_');
                    noteSurfaceType(receiver, flushDescription);
                }
            }

            const double start = IPCore::PlaybackTrace::now();
            IPCore::PlaybackTrace::eventBegin(start, typeName);

            bool result = false;
            {
                ++m_depth;
                DepthGuard guard(m_depth);
                EndGuard end(start, typeName, receiverMeta, receiverName, parentMeta, parentName);
                result = QApplication::notify(receiver, e);
            }

            if (!changeDescription.empty())
                IPCore::PlaybackTrace::widgetEvent(false, changeDescription, (IPCore::PlaybackTrace::now() - start) * 1000.0);
            if (!flushDescription.empty())
                IPCore::PlaybackTrace::mainWindowFlush(flushDescription, (IPCore::PlaybackTrace::now() - start) * 1000.0);
            return result;
        }

    private:
        struct DepthGuard
        {
            explicit DepthGuard(int& depth)
                : m_depth(depth)
            {
            }

            ~DepthGuard() { --m_depth; }

            int& m_depth;
        };

        //  Ends the trace event even if delivery throws.
        struct EndGuard
        {
            EndGuard(double start, const char* typeName, const QMetaObject* receiverMeta, const QString& receiverName,
                     const QMetaObject* parentMeta, const QString& parentName)
                : m_start(start)
                , m_typeName(typeName)
                , m_receiverMeta(receiverMeta)
                , m_receiverName(receiverName)
                , m_parentMeta(parentMeta)
                , m_parentName(parentName)
            {
            }

            ~EndGuard()
            {
                const double end = IPCore::PlaybackTrace::now();
                std::string description;
                if ((end - m_start) * 1000.0 >= IPCore::PlaybackTrace::eventDetailMs)
                    description = describe();
                IPCore::PlaybackTrace::eventEnd(end, description);
            }

            //  e.g. Timer:QTimer'RvApplication::m_timer'<-Rv::RvApplication
            //  No spaces, so the trace can use it as a culprit key.
            std::string describe() const
            {
                std::string d = m_typeName;
                d += ":";
                d += m_receiverMeta ? m_receiverMeta->className() : "null";
                if (!m_receiverName.isEmpty())
                    d += "'" + m_receiverName.toStdString() + "'";
                if (m_parentMeta)
                {
                    d += "<-";
                    d += m_parentMeta->className();
                    if (!m_parentName.isEmpty())
                        d += "'" + m_parentName.toStdString() + "'";
                }
                std::replace(d.begin(), d.end(), ' ', '_');
                return d;
            }

            double m_start;
            const char* m_typeName;
            const QMetaObject* m_receiverMeta;
            const QString& m_receiverName;
            const QMetaObject* m_parentMeta;
            const QString& m_parentName;
        };

        //  Class'objectName' of the widget and up to two ancestors, innermost
        //  first, e.g. QLabel<-QToolBar'timeline'<-Rv::RvDocument.
        static std::string widgetPath(const QObject* o)
        {
            std::string path;
            for (int level = 0; o && level < 3; ++level, o = o->parent())
            {
                if (level)
                    path += "<-";
                path += o->metaObject()->className();
                const QString name = o->objectName();
                if (!name.isEmpty())
                    path += "'" + name.toStdString() + "'";
            }
            return path;
        }

        //  e.g. "MouseButtonPress left on QToolButton<-Rv::RvBottomViewToolBar"
        //  or "KeyPress Space on Rv::GLWindow". Called before delivery.
        static void markInput(QObject* receiver, QEvent* e)
        {
            const QEvent::Type type = e->type();
            std::string what = eventTypeName(type);

            if (type == QEvent::KeyPress || type == QEvent::KeyRelease)
            {
                const QKeyEvent* ke = static_cast<QKeyEvent*>(e);
                if (ke->isAutoRepeat())
                    return;
                const QString key = QKeySequence(ke->key()).toString();
                what += " " + (key.isEmpty() ? std::to_string(ke->key()) : key.toStdString());
                what += " on " + widgetPath(receiver);
            }
            else
            {
                const QMouseEvent* me = static_cast<QMouseEvent*>(e);
                const Qt::MouseButton button = me->button();
                what += button == Qt::LeftButton     ? " left"
                        : button == Qt::RightButton  ? " right"
                        : button == Qt::MiddleButton ? " middle"
                                                     : " other";

                //  Top-level mouse events go to the window; name the widget
                //  under the pointer instead.
                const QWidget* under = QApplication::widgetAt(QCursor::pos());
                what += " on " + widgetPath(under ? static_cast<const QObject*>(under) : receiver);
            }

            std::replace(what.begin(), what.end(), '\n', ' ');
            IPCore::PlaybackTrace::mark(what);
        }

        //  Logs how each top-level window is composited (raster or OpenGL),
        //  once per window and whenever it changes. An OpenGL-composited
        //  main window swaps, and may wait for vsync, on every repaint.
        static void noteSurfaceType(QObject* receiver, const std::string& description)
        {
            const QWindow* window = nullptr;
            if (receiver->isWidgetType())
                window = static_cast<QWidget*>(receiver)->windowHandle();
            else
                window = qobject_cast<QWindow*>(receiver);
            if (!window)
                return;

            static std::map<const QWindow*, int> seen;
            const int surfaceType = int(window->surfaceType());
            auto it = seen.find(window);
            if (it != seen.end() && it->second == surfaceType)
                return;
            seen[window] = surfaceType;

            std::string name;
            switch (window->surfaceType())
            {
            case QSurface::RasterSurface:
                name = "RasterSurface";
                break;
            case QSurface::OpenGLSurface:
                name = "OpenGLSurface";
                break;
            case QSurface::VulkanSurface:
                name = "VulkanSurface";
                break;
            case QSurface::MetalSurface:
                name = "MetalSurface";
                break;
            default:
                name = std::to_string(surfaceType);
                break;
            }
            IPCore::PlaybackTrace::info("top-level window " + description + " surface type " + name);
        }

        static const char* eventTypeName(QEvent::Type type)
        {
            static const QMetaEnum e = QMetaEnum::fromType<QEvent::Type>();
            const char* name = e.valueToKey(type);
            return name ? name : (type >= QEvent::User ? "User" : "Unknown");
        }

        int m_depth = 0;
    };

    //
    //  Playback trace: one-time system information (RV, Qt, OS, remote
    //  session, GPUs, screens, RV preferences), plus screen changes.
    //

    //  Values are truncated so a long list (paths, layouts) stays readable.
    std::string traceVariantText(const QVariant& v)
    {
        constexpr int maxChars = 300;
        QString s;
        if (v.userType() == QMetaType::QStringList)
            s = v.toStringList().join(",");
        else if (v.canConvert<QString>())
            s = v.toString();
        else
            s = QString("<") + v.typeName() + ">";
        s.replace('\n', ' ');
        if (s.size() > maxChars)
            s = s.left(maxChars) + "...";
        return s.toStdString();
    }

    std::string traceScreenText(const QScreen* screen)
    {
        std::ostringstream o;
        const QRect g = screen->geometry();
        o << "'" << screen->name().toStdString() << "' " << screen->manufacturer().toStdString() << " " << screen->model().toStdString()
          << " geometry=" << g.width() << "x" << g.height() << "+" << g.x() << "+" << g.y() << " dpr=" << screen->devicePixelRatio()
          << " refreshRate=" << screen->refreshRate() << "Hz depth=" << screen->depth()
          << (screen == QGuiApplication::primaryScreen() ? " primary" : "");
        return o.str();
    }

    void traceWatchScreen(QScreen* screen)
    {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QObject::connect(screen, &QScreen::refreshRateChanged,
                         [screen](qreal) { IPCore::PlaybackTrace::info("screen refresh rate changed: " + traceScreenText(screen)); });
#endif
        QObject::connect(screen, &QScreen::geometryChanged,
                         [screen](const QRect&) { IPCore::PlaybackTrace::info("screen geometry changed: " + traceScreenText(screen)); });
    }

    void traceStartupInfo()
    {
        using IPCore::PlaybackTrace;
        if (!PlaybackTrace::enabled())
            return;

        {
            std::ostringstream o;
            o << "RV " << MAJOR_VERSION << "." << MINOR_VERSION << "." << REVISION_NUMBER << " (" << RELEASE_DESCRIPTION << ")"
              << " HEAD=" << GIT_HEAD;
            PlaybackTrace::info(o.str());
        }

        PlaybackTrace::info(std::string("Qt ") + qVersion() + ", platform " + QGuiApplication::platformName().toStdString());
        PlaybackTrace::info("OS " + QSysInfo::prettyProductName().toStdString() + ", kernel " + QSysInfo::kernelVersion().toStdString()
                            + ", " + QSysInfo::currentCpuArchitecture().toStdString() + ", " + std::to_string(QThread::idealThreadCount())
                            + " threads");

#ifdef PLATFORM_WINDOWS
        PlaybackTrace::info(std::string("remote session (SM_REMOTESESSION): ") + (GetSystemMetrics(SM_REMOTESESSION) ? "yes" : "no"));

        //  GPUs as Windows sees them. In a Remote Desktop session the
        //  attached adapter is usually "Microsoft Remote Display Adapter".
        for (DWORD i = 0;; ++i)
        {
            DISPLAY_DEVICEW device{};
            device.cb = sizeof(device);
            if (!EnumDisplayDevicesW(nullptr, i, &device, 0))
                break;

            std::ostringstream o;
            o << "GPU adapter " << i << ": " << QString::fromWCharArray(device.DeviceString).toStdString() << " ["
              << QString::fromWCharArray(device.DeviceID).toStdString() << "] " << QString::fromWCharArray(device.DeviceName).toStdString()
              << ((device.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) ? " attached" : "")
              << ((device.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) ? " primary" : "");

            DEVMODEW mode{};
            mode.dmSize = sizeof(mode);
            if (EnumDisplaySettingsW(device.DeviceName, ENUM_CURRENT_SETTINGS, &mode))
                o << " mode=" << mode.dmPelsWidth << "x" << mode.dmPelsHeight << "@" << mode.dmDisplayFrequency << "Hz";

            PlaybackTrace::info(o.str());
        }
#endif

        for (QScreen* screen : QGuiApplication::screens())
        {
            PlaybackTrace::info("screen: " + traceScreenText(screen));
            traceWatchScreen(screen);
        }

        QObject::connect(qApp, &QGuiApplication::screenAdded,
                         [](QScreen* screen)
                         {
                             PlaybackTrace::info("screen added: " + traceScreenText(screen));
                             traceWatchScreen(screen);
                         });
        QObject::connect(qApp, &QGuiApplication::screenRemoved,
                         [](QScreen* screen) { PlaybackTrace::info("screen removed: '" + screen->name().toStdString() + "'"); });

        //  RV preferences, read from the same file through a separate
        //  read-only QSettings. Recent-file lists and anything that looks
        //  like a credential are left out.
        const QString prefsFile = Rv::RvSettings::globalSettings().fileName();
        PlaybackTrace::info("preferences file: " + prefsFile.toStdString());
        for (const char* name : {"RV_PREFS_OVERRIDE_PATH", "RV_PREFS_CLOBBER_PATH"})
        {
            if (const char* v = getenv(name))
                PlaybackTrace::info(std::string("env ") + name + "=" + v);
        }

#ifdef PLATFORM_WINDOWS
        QSettings prefs(prefsFile, QSettings::IniFormat);
#else
        QSettings prefs(prefsFile, QSettings::NativeFormat);
#endif
        const QStringList skipped = {"recent", "password", "token", "secret", "credential", "login", "cookie"};
        int count = 0;
        for (const QString& key : prefs.allKeys())
        {
            const QString lower = key.toLower();
            if (std::any_of(skipped.begin(), skipped.end(), [&lower](const QString& word) { return lower.contains(word); }))
                continue;
            PlaybackTrace::info("pref " + key.toStdString() + "=" + traceVariantText(prefs.value(key)));
            ++count;
        }
        PlaybackTrace::info("preferences: " + std::to_string(count) + " values logged");
    }
} // namespace

// TODO_QT: Remove if everything works.
// Assuming it is not needed: qtGlobals.cpp stopped being generated since Qt 4.6
// #include <QtGlobal>

extern const char* rv_linux_dark;

// RV third party optional customization
#if defined(RV_THIRD_PARTY_CUSTOMIZATION)
extern void rvThirdPartyCustomization(TwkApp::Bundle& bundle, char* licarg);
#endif

//
//  Check that a version actually exists. If we're compiling opt error
//  out in cpp. Otherwise, just note the lack of version info.
//

#if MAJOR_VERSION == 99
#if NDEBUG
// #error ********* NO VERSION INFORMATION ***********
#else
#ifndef _MSC_VER
#warning********* NO VERSION INFORMATION ***********
#endif
#endif
#endif

#ifdef WIN32
#define FILE_SEP ";"
#else
#define FILE_SEP ":"
#endif

using namespace std;
using namespace TwkFB;
using namespace TwkMovie;
using namespace TwkUtil;

extern const char* TweakDark;

#if !defined(PLATFORM_WINDOWS)
static void control_c_handler(int sig)
{
    cout << "INFO: stopped by user" << endl;
    exit(1);
}
#endif

void setEnvVar(const string& var, const string& val)
{
#ifdef WIN32
    ostringstream str;
    str << var << "=" << val;
    putenv(str.str().c_str());
#else
    setenv(var.c_str(), val.c_str(), 1);
#endif
}

void setEnvVar(const string& var, const QFileInfo& val) { setEnvVar(var, val.absoluteFilePath().toUtf8().data()); }

void addToEnvVar(const string& var, const string& val)
{
    if (getenv(var.c_str()))
    {
        ostringstream str;
        str << getenv(var.c_str()) << FILE_SEP << val;
        setEnvVar(var, str.str());
    }
    else
    {
        setEnvVar(var, val);
    }
}

string scarfFile(const string& fileName)
{
    ifstream in(fileName.c_str(), ios::binary);
    stringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

#define EXECUTABLE_SHORT_NAME "rv"
#define EXECUTABLE_SHORT_NAME_CAPS "RV"

TwkApp::QTBundle bundle(EXECUTABLE_SHORT_NAME, MAJOR_VERSION, MINOR_VERSION, REVISION_NUMBER);

#ifdef PLATFORM_LINUX
extern "C" int XInitThreads();
#endif

//
// This function is an attempt to prevent QCoreApplication::initLocale() method
// to complain that an invalid locale is currently set.
//
// Before this bugfix, RV's init code explicitely set the current locale to "C".
// At the same time, this locale is now explicitely unsupported by Qt6, and
// instead will instead switch to "C.UTF-8" under Linux, or "UTF-8" under Darwin
// (macOS)
//
// For more information about how Qt initializes the locale, look at
// initLocale() in the file:
//
// QT_HOME/Qt6.x.x/Src/qtbase/src/corelib/kernel/qcoreapplication.cpp
//
static void setPlatformSpecificLocale()
{
    std::string locale = "";

#if defined(PLATFORM_DARWIN)
    // In Darwin, "C.UTF-8" isn't directly supported, but we can use any
    // "<region>.UTF-8" locale.  However, as per Qt's initLocale() method
    // for in macOS, there is...
    //
    //    ..."no need to try language or region specific CTYPEs, as they
    //        all point back to the same generic UTF-8 CTYPE."
    //
    // NOTE: Leaving this here for completeness for the curious minded,
    // but the main.cpp file for macOS is actually src/bin/nsapps/RV/main.cpp

    locale = "UTF-8";
#elif defined(PLATFORM_LINUX)
    // this is valid for Rocky Linux. As per the Qt code, some other Unixes may
    // also support "C.utf-8" as an alias to C.UTF-8", but for Rocky Linux there
    // is no need.
    locale = "C.UTF-8";
#elif defined(WIN32)
    // Under Windows, it seems Qt does not try to enfore switching to "C.UTF-8",
    // (as this function only seems to be compiled for unix-variants).
    // Therefore, we leave it as the "C" locale.
    locale = "C";
#endif

    setEnvVar("LANG", locale);
    setEnvVar("LC_ALL", locale);
    // cout << "Set locale to " << locale.c_str() << endl; // for debugging at
    // console
}

int utf8Main(int argc, char* argv[])
{
#ifdef PLATFORM_LINUX
    XInitThreads();
#endif

#ifdef PLATFORM_WINDOWS
    // High DPI support is disabled by default on Windows.
    // This is a temporary measure due to reported crashes with
    // QtWebEngineWidgets in Qt 6.5.3 on high DPI displays. To enable High DPI
    // support, set the environment variable "RV_QT_HDPI_SUPPORT". Note: The
    // "RV_QT_HDPI_SUPPORT" environment variable was also used in previous
    // versions of RV.
    const bool noHighDPISupport = getenv("RV_QT_HDPI_SUPPORT") == nullptr;
#else
    // High DPI support is enabled by default on non-Windows platforms.
    // To disable High DPI support, set the environment variable
    // "RV_NO_QT_HDPI_SUPPORT".
    const bool noHighDPISupport = getenv("RV_NO_QT_HDPI_SUPPORT") != nullptr;
#endif

    if (noHighDPISupport)
    {
        qunsetenv("QT_SCALE_FACTOR");
        qunsetenv("QT_SCREEN_SCALE_FACTORS");
        qunsetenv("QT_AUTO_SCREEN_SCALE_FACTOR");
        qputenv("QT_ENABLE_HIGHDPI_SCALING", "0");
        qunsetenv("QT_SCALE_FACTOR_ROUNDING_POLICY");
        qunsetenv("QT_DEVICE_PIXEL_RATIO");
    }

    const char* pythonPath = getenv("PYTHONPATH");
    const char* pythonHome = getenv("PYTHONHOME");

    if (pythonPath)
        pythonPath = strdup(pythonPath);
    if (pythonHome)
        pythonHome = strdup(pythonHome);

#ifndef PLATFORM_WINDOWS
    //
    //  Check the per-process limit on open file descriptors and
    //  reset the soft limit to the hard limit.
    //
    struct rlimit rlim;
    getrlimit(RLIMIT_NOFILE, &rlim);
    rlim.rlim_cur = rlim.rlim_max;
    setrlimit(RLIMIT_NOFILE, &rlim);
    getrlimit(RLIMIT_NOFILE, &rlim);
    if (rlim.rlim_cur < rlim.rlim_max)
    {
        cerr << "WARNING: unable to increase open file limit above " << rlim.rlim_cur << endl;
    }
#else
    _setmaxstdio(2048);
#endif

#ifdef PLATFORM_WINDOWS
    //
    //  RV is a "GUI" app, so by default we get no output in the
    //  console, so "rv.exe -help" doesn't work, etc.  Here we
    //  reattach to the starting console, so we do get that
    //  output.
    //
    HANDLE hand = GetStdHandle(STD_OUTPUT_HANDLE);

    if ((hand == INVALID_HANDLE_VALUE || GetFileType(hand) == FILE_TYPE_UNKNOWN) && AttachConsole(ATTACH_PARENT_PROCESS))
    {
        freopen("CON", "w", stdout);
        freopen("CON", "w", stderr);
    }
#endif

    //
    //  Check for a valid HOME. If we are on Windows and HOME is not set then
    //  see if we can create one from concatenating HOMEDRIVE and HOMEPATH.
    //
    if (!getenv("HOME"))
    {
#ifdef PLATFORM_WINDOWS
        if (getenv("HOMEDRIVE") && getenv("HOMEPATH"))
        {
            string home = string(getenv("HOMEDRIVE")) + getenv("HOMEPATH");
            setEnvVar("HOME", home.c_str());
            cout << "INFO: Using '" << home << "' for $HOME." << endl;
        }
        else
#endif
        {
            cerr << "ERROR: $HOME is not set in the environment and is required." << endl;
            exit(-1);
        }
    }

    if (getenv("LANG"))
    {
        std::string originalLocale = getenv("LANG");
        setEnvVar("ORIGINALLOCAL", originalLocale.c_str());
    }
    else
    {
        setEnvVar("ORIGINALLOCAL", "en");
    }

    setPlatformSpecificLocale();

    // Refresh the locale using the environment variable set by
    // setPlatformSpecificLocale.
    QLocale::setDefault(QLocale::system());

    TwkFB::ThreadPool::initialize();

#if defined(PLATFORM_LINUX)
    // Work around for Wacom Tablet issue on linux
    // Note: This is a Qt 5.12.4 regression
    // (https://bugreports.qt.io/browse/QTBUG-77826)
    setEnvVar("QT_XCB_TABLET_LEGACY_COORDINATES", "1");
#endif

    // Prevent crash at startup when multithreaded upload is enabled
    // (RV Preferences/Rendering/Multithread GPU Upload)
    QApplication::setAttribute(Qt::AA_DontCheckOpenGLContextThreadAffinity);

    // Share GL resources across every QOpenGLContext in the process. This is a
    // documented QtWebEngine requirement, and it lets RV's auxiliary GL
    // surfaces -- the second-output ScreenView and the multithreaded-upload
    // worker device -- share textures/FBOs with the main viewport context
    // without an explicit, ordering-sensitive setShareContext() call. Must be
    // set before the QApplication is constructed.
    //
    // The Vulkan path also relies on it: QTVulkanVideoDevice::ensureGLContext()
    // has no GLView to share with, so it joins this global group.
    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

#ifdef PLATFORM_WINDOWS
    // Put Qt Quick on the same graphics API RV's windows composite with.
    //
    // Qt Quick's RHI backend defaults to Direct3D 11 on Windows, while RV's
    // top-level windows composite with OpenGL. A QQuickWidget cannot obtain a
    // QRhi from a window using a different API, so anything Quick-based inside
    // an RV window -- most visibly a QWebEngineView, whose page is rendered by a
    // QQuickWidget -- draws nothing and logs "The top-level window is not using
    // the expected graphics API for composition" followed by "Attempted to
    // render scene with no rhi".
    //
    // Windows only: Quick already defaults to OpenGL on Linux. macOS defaults to
    // Metal and so may need the same treatment, but that is untested here.
    //
    // Setting QSG_RHI_BACKEND from this point does not work -- by the time any
    // code in main() runs, the backend has already been resolved, so only the
    // environment RV was launched with is honoured. setGraphicsApi() is the
    // documented way to choose it from the application, and is specified to work
    // when called before the first QQuickWindow exists. It is skipped when the
    // environment already chooses a backend, so QSG_RHI_BACKEND still overrides.
    if (!getenv("QSG_RHI_BACKEND"))
    {
        QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
    }
#endif

    TwkUtil::MemPool::initialize();

    string altPrefsPath;
    for (size_t i = 0; i < argc; i++)
    {
        if (i < argc - 1 && !strcmp(argv[i], "-prefsPath"))
            altPrefsPath = argv[i + 1];
    }
    Rv::RvApplication::initializeQSettings(altPrefsPath);

    bool noPrefs = false;
    bool resetPrefs = false;
    Q_INIT_RESOURCE(rv);

    //
    //  Expand any rvlink urls in argv
    //

    vector<char*> newArgv;

    for (int i = 0; i < argc; ++i)
    {
        if (0 == strncmp("rvlink://", argv[i], 9))
        {
            Rv::RvApplication::parseURL(argv[i], newArgv);
        }
        else
            newArgv.push_back(argv[i]);
    }
    argv = &(newArgv[0]);
    argc = newArgv.size();

    for (size_t i = 0; i < argc; i++)
    {
        if (!strcmp(argv[i], "-noPrefs"))
            noPrefs = true;
        else if (!strcmp(argv[i], "-resetPrefs"))
            resetPrefs = true;
    }
    for (size_t i = 0; i < argc; i++)
    {
        if (!strcmp(argv[i], "-bakeURL"))
        {
            string url = Rv::RvApplication::bakeCommandLineURL(argc, argv);
            cerr << "Baked URL: " << url << endl;
            exit(0);
        }
        if (!strcmp(argv[i], "-encodeURL"))
        {
            string url = Rv::RvApplication::encodeCommandLineURL(argc, argv);
            cerr << "Encoded URL: " << url << endl;
            exit(0);
        }
    }

    Rv::Options& opts = Rv::Options::sharedOptions();
    if (resetPrefs)
        Rv::RvPreferences::resetPreferencesFile();
    if (!noPrefs)
        Rv::RvPreferences::loadSettingsIntoOptions(opts);
    else
        Rv::PackageManager::setIgnorePrefs(true);
    Rv::Options::manglePerSourceArgs(argv, argc);
    Rv::Options prefOpts = opts;

    //
    //  Additional options. These are QT UI look related.
    //

    //
    //  Call the deploy functions
    //

    TWK_DEPLOY_APP_OBJECT dobj(MAJOR_VERSION, MINOR_VERSION, REVISION_NUMBER, argc, argv, RELEASE_DESCRIPTION, "HEAD=" GIT_HEAD);

    Imf::staticInitialize();

    TwkFB::GenericIO::init();    // Initialize TwkFB::GenericIO plugins statics
    TwkMovie::GenericIO::init(); // Initialize TwkMovie::GenericIO plugins
                                 // statics

    IPCore::Application::cacheEnvVars();

    //
    // We handle the '--help' flag by changing it to '-help' for
    // argparse to work.
    //
    for (int i = 0; i < argc; ++i)
    {
        if (!strcmp("--help", argv[i]))
        {
            strcpy(argv[i], "-help");
            break;
        }
    }

    //
    //  Parse cmd line args
    //

    int strictlicense = 0;
    char* prefsPath = 0;
    int sleepTime = 0;

    if (arg_parse(argc, argv, "", "", RV_ARG_EXAMPLES, "", "", RV_ARG_SEQUENCE_HELP, "", "", RV_ARG_SOURCE_OPTIONS(opts), "", "", "",
                  ARG_SUBR(Rv::RvApplication::parseInFiles), "Input sequence patterns, images, movies, or directories ",
                  RV_ARG_PARSE_OPTIONS(opts), "-strictlicense", ARG_FLAG(&strictlicense),
                  "Exit rather than consume an RV license if no rvsolo licenses are "
                  "available",
                  "-prefsPath %S", &prefsPath, "Alternate path to preferences directory",
#if defined(PLATFORM_LINUX)
                  "-scheduler %S", &opts.schedulePolicy, "Thread scheduling policy (may require root)", "-priorities %d %d",
                  &opts.displayPriority, &opts.audioPriority, "Set display and audio thread priorities (may require root)",
#endif
                  "-sleep %d", &sleepTime, "Sleep (in seconds) before starting to allow attaching debugger", NULL)
        < 0)
    {
        exit(-1);
    }

    //
    //  Qt itself (on linux) will consider the -geometry flag to be
    //  a traditional X11 geometry flag, which it's not, so prevent
    //  this by switching it to "geometri" if it's in argv.
    //
    for (int i = 0; i < argc; ++i)
        if (strcmp(argv[i], "-geometry") == 0)
            argv[i][8] = 'i';

    if (sleepTime > 0)
    {
        cout << "INFO: sleeping " << sleepTime << " seconds" << endl;
        std::this_thread::sleep_for(std::chrono::seconds(sleepTime));
        cout << "INFO: continuing after sleep" << endl;
    }

    if (opts.showVersion)
    {
        cout << MAJOR_VERSION << "." << MINOR_VERSION << "." << REVISION_NUMBER << endl;
        exit(0);
    }

    //
    //  Desktop aware: being "desktop aware" may be causing problems
    //  on some systems
    //

    QApplication::setDesktopSettingsAware(opts.qtdesktop == 1);

    //
    //  Set scheduling parameters for display thread
    //

    if (opts.schedulePolicy)
    {
#if defined(PLATFORM_LINUX)
        sched_param sp;
        memset(&sp, 0, sizeof(sched_param));
        sp.sched_priority = opts.displayPriority;

        unsigned int policy = SCHED_OTHER;

        if (!strcmp(opts.schedulePolicy, "SCHED_RR"))
            policy = SCHED_RR;
        else if (!strcmp(opts.schedulePolicy, "SCHED_FIFO"))
            policy = SCHED_FIFO;

        if (policy != SCHED_OTHER)
        {
            if (sched_setscheduler(0, policy, &sp))
            {
                cout << "ERROR: can't set thread priority" << endl;
            }
        }
        else
        {
            if (setpriority(PRIO_PROCESS, 0, opts.displayPriority))
            {
                cout << "ERROR: can't set thread priority" << endl;
            }
        }
#elif defined(PLATFORM_DARWIN)
        // don't bother here ... use RV or RV64 instead
#else
        // Uh... windoz
#endif
    }

    //
    //  Banners
    //

    TWK_DEPLOY_SHOW_PROGRAM_BANNER(cout);
    TWK_DEPLOY_SHOW_COPYRIGHT_BANNER(cout);
    TWK_DEPLOY_SHOW_LOCAL_BANNER(cout);

    //
    //  Get CPU info and tell the EXR library to use them all
    //

    if (opts.exrcpus > 0)
    {
        Imf::setGlobalThreadCount(opts.exrcpus);
    }
    else
    {
        Imf::setGlobalThreadCount(Rv::automaticExrThreadCount());
    }

    //
    //  Additional options. These are QT UI look  related.
    //

    // Required by the Live Review RV plugin React component's scroll bars
    std::vector<char*> arguments(argv, argv + argc);
    static char enableOverlayScrollbar[] = "--enable-features=OverlayScrollbar";
    arguments.emplace_back(enableOverlayScrollbar);
    argc = static_cast<int>(arguments.size());
    argv = &arguments[0];

    //
    //  Application
    //

    QApplication* app = new TracingApplication(argc, argv);

    //
    // Initialize crash handler early to catch crashes during startup
    // (Must be after QApplication is created to use QCoreApplication::applicationDirPath)
    //
    {
        TwkApp::initializeCrashHandler("RV", QCoreApplication::applicationDirPath().toStdString());
    }

    QTranslator* translator = new QTranslator();
    QLocale locale = QLocale(getenv("ORIGINALLOCAL"));
    if (translator->load(locale, QLatin1String("i18n"), "_", QLatin1String(":/translations")))
    {
        app->installTranslator(translator);
    }

    QDir dir(QCoreApplication::applicationDirPath());
    dir.cdUp();
    dir.cd("plugins");
    dir.cd("Qt");
    QApplication::addLibraryPath(dir.absolutePath());

    Rv::RvApplication* rvapp = new Rv::RvApplication(argc, argv);

    //  Instrumented build: the playback trace is on unless
    //  RV_PLAYBACK_TRACE=0. Enabled here, before any session exists, so the
    //  log starts with the system information.
    IPCore::PlaybackTrace::initFromEnvironment();
    traceStartupInfo();

    rvapp->setExecutableNameCaps("RV");

    app->installEventFilter(rvapp);
    app->setQuitOnLastWindowClosed(true);

    //
    //  Get the bundle info so we can find resource files
    //
    //  init priorities are:
    //
    //      1) -init script
    //      2) $RV_INIT
    //      3) $HOME/.rvrc.mu
    //      4) $RV_HOME/scripts/rv/rvrc.mu
    //      5) ./rvrc.mu
    //

    if (bundle.top() == "")
    {
        cerr << "ERROR: can't locate RV home directory (is it installed in "
                "root?)"
             << endl;

        exit(-1);
    }

    bundle.setEnvVar("RV_PYTHONPATH_EXTERNAL", (pythonPath) ? pythonPath : "");
    bundle.setEnvVar("RV_PYTHONHOME_EXTERNAL", (pythonHome) ? pythonHome : "");

    if (getenv("RV_APP_RVIO"))
        bundle.setEnvVar("RV_APP_RVIO_SET_BY_USER", "true");
    else
        bundle.setEnvVar("RV_APP_RVIO", bundle.executableFile("rvio"));

    bundle.setEnvVar("RV_APP_RV_SHORT_NAME", EXECUTABLE_SHORT_NAME);
    bundle.setEnvVar("RV_APP_RV", bundle.executableFile(EXECUTABLE_SHORT_NAME));
    bundle.setEnvVar("RV_APP_MANUAL", bundle.resource("rv_manual", "pdf"));
    bundle.setEnvVar("RV_APP_MANUAL_HTML", bundle.resource("rv_manual", "html"));
    bundle.setEnvVar("RV_APP_SDI_MANUAL", bundle.resource("rvsdi_manual", "pdf"));
    bundle.setEnvVar("RV_APP_SDI_MANUAL_HTML", bundle.resource("rvsdi_manual", "html"));
    bundle.setEnvVar("RV_APP_REFERENCE_MANUAL", bundle.resource("rv_reference", "pdf"));
    bundle.setEnvVar("RV_APP_REFERENCE_MANUAL_HTML", bundle.resource("rv_reference", "html"));
    bundle.setEnvVar("RV_APP_MU_MANUAL", bundle.resource("mu", "pdf"));
    bundle.setEnvVar("RV_APP_GTO_REFERENCE", bundle.resource("gto", "pdf"));
    bundle.setEnvVar("RV_APP_RELEASE_NOTES", bundle.resource("rv_release_notes", "html"));
    bundle.setEnvVar("RV_APP_LICENSES_NOTES", bundle.resource("rv_client_licenses", "html"));
    bundle.addPathToEnvVar("OIIO_LIBRARY_PATH", bundle.appPluginPath("OIIO"));

    //
    //  Find the init file
    //

    string muInitFile = bundle.rcfile("rvrc", "mu", "RV_INIT");
    string pyInitFile = bundle.rcfile("rvrc", "py", "RV_PYINIT");
    bundle.setEnvVar("RV_APP_INIT", muInitFile.c_str());
    bundle.setEnvVar("RV_APP_PYINIT", pyInitFile.c_str());

    if (opts.initscript)
        muInitFile = opts.initscript;

    // RV third party optional customization
#if defined(RV_THIRD_PARTY_CUSTOMIZATION)
    rvThirdPartyCustomization(bundle, opts.licarg);
#endif

    try
    {
        TwkFB::loadProxyPlugins("TWK_FB_PLUGIN_PATH");
        TwkMovie::loadProxyPlugins("TWK_MOVIE_PLUGIN_PATH");
    }
    catch (...)
    {
        cerr << "WARNING: a problem occured while loading plugins." << endl;
        cerr << "         some plugins may not have been loaded." << endl;
    }

    TwkMovie::GenericIO::addPlugin(new MovieFBIO());
    TwkMovie::GenericIO::addPlugin(new MovieProceduralIO());

    //
    //  Compile the list of sequencable file extensions
    //

    TwkFB::GenericIO::compileExtensionSet(predicateFileExtensions());

#ifdef PLATFORM_WINDOWS
    DWORD targetClass = HIGH_PRIORITY_CLASS;
    if (getenv("RV_REALTIME_PRIORITY_CLASS"))
        targetClass = REALTIME_PRIORITY_CLASS;
    DWORD proClass1 = GetPriorityClass(GetCurrentProcess());
    if (0 == SetPriorityClass(GetCurrentProcess(), targetClass))
    {
        cerr << "ERROR: SetPriorityClass failed, error " << GetLastError() << endl;
    }
    DWORD proClass2 = GetPriorityClass(GetCurrentProcess());
    if (proClass2 != targetClass)
    {
        cerr << "WARNING: failed to set Priority Class, class " << hex << showbase << int(proClass1) << " -> " << int(proClass2)
             << noshowbase << dec << endl;
    }
#endif

    //
    //  QT starts here
    //

#ifdef PTW32_STATIC_LIB
    pthread_win32_process_attach_np();
    pthread_win32_process_attach_np();
#endif

    // Rv::dumpApplicationPalette();
    Rv::initializeDefaultPalette();

    if (!getenv("RV_DARK"))
        bundle.setEnvVar("RV_DARK", "", true);

    QString csstext;

    if (opts.qtcss)
    {
        string s = scarfFile(opts.qtcss);
        QString qcss = QString(s.c_str());
        // Always apply font size parameters - Qt handles missing placeholders
        // safely
        csstext = qcss.arg(opts.fontSize1).arg(opts.fontSize2).arg(opts.fontSize2 - 1);
    }
    else
    {
        csstext = QString(rv_linux_dark).arg(opts.fontSize1).arg(opts.fontSize2).arg(opts.fontSize2 - 1);
    }

    if (!opts.qtstyle || !strcmp(opts.qtstyle, "RV"))
    {
        bundle.setEnvVar("RV_DARK", "1", true);
        app->setStyle(QStyleFactory::create("Cleanlooks"));
    }
    else
    {
        app->setStyle(QStyleFactory::create(opts.qtstyle));
    }

    app->setStyleSheet(csstext);

    try
    {
        TwkApp::initMu(0);
        TwkApp::initPython(argc, argv);
        Rv::initUICommands();
        Rv::initCommands();

        if (!opts.initializeAfterParsing((noPrefs) ? 0 : &prefOpts))
        {
            rvapp->console()->processLastTextBuffer();
            exit(-1);
        }

        TwkApp::initWithFile(TwkApp::muContext(), TwkApp::muProcess(), TwkApp::muModuleList(), muInitFile.c_str());

        TwkApp::pyInitWithFile(pyInitFile.c_str(), Rv::pyRvAppCommands(), Rv::pyUICommands());
    }
    catch (const exception& e)
    {
        cerr << "ERROR: during initialization: " << e.what() << endl;
        rvapp->console()->processLastTextBuffer();
        exit(-1);
    }

    //
    //  XXX dummyDev is leaking here.  Best would be to pass it to the App so
    //  that it could delete it after startup, since it's no longer needed at
    //  that point.
    //

#ifdef PLATFORM_WINDOWS
    TwkGLF::FBOVideoDevice* dummyDev = new TwkGLF::FBOVideoDevice(0, 10, 10, false);
    IPCore::ImageRenderer::queryGL();
    const char* glVersion = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
    IPCore::Shader::Function::useShadingLanguageVersion(glVersion);
#endif

    //
    //  Organize the input file list
    //

    bool dorun = true;

    rvapp->createNewSessionFromFiles(opts.inputFiles);

    rvapp->processNetworkOpts();

    int rval = 0;

    rvapp->console()->processTextBuffer();

#if !defined(PLATFORM_WINDOWS)
    //
    //  Overwrite python's signal handler
    //

    if (signal(SIGINT, control_c_handler) == SIG_ERR)
    {
        cout << "ERROR: failed to install SIGINT signal handler" << endl;
    }
#endif

    TwkUtil::setThreadName("RV Main");

    if (dorun)
    {
        try
        {
            rval = app->exec();
        }
        catch (TwkExc::Exception& exc)
        {
            cerr << exc;
            exit(-1);
        }
        catch (const exception& e)
        {
            cerr << "ERROR: Unhandled exception during execution: " << e.what() << endl;

            exit(-1);
        }
        catch (...)
        {
            cerr << "ERROR: Unhandled unknown exception" << endl;
            exit(-1);
        }
    }

#ifdef PTW32_STATIC_LIB
    pthread_win32_thread_detach_np();
    pthread_win32_process_detach_np();
#endif

    TwkFB::ThreadPool::shutdown();
    TwkMovie::GenericIO::shutdown(); // Shutdown TwkMovie::GenericIO plugins
    TwkFB::GenericIO::shutdown();    // Shutdown TwkFB::GenericIO plugins

    TwkGLF::UninitPBOPools();

    // Ensure to delete the QApplication before calling finalizePython
    delete rvapp;
    delete app;
    TwkApp::finalizePython();

    return rval;
}
