//******************************************************************************
//
// Copyright (C) 2026 Autodesk, Inc. All Rights Reserved.
//
// SPDX-License-Identifier: Apache-2.0
//
//******************************************************************************
#include <stream/StreamCachePath.h>

#include <TwkUtil/EnvVar.h>
#include <TwkUtil/File.h>

#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <string>

#ifdef PLATFORM_WINDOWS
#include <direct.h>
#else
#include <sys/stat.h>
#endif

static ENVVAR_STRING(evStreamCachePath, "RV_STREAM_CACHE_PATH", "");

namespace
{
    std::string joinPath(const std::string& base, const std::string& child)
    {
        if (base.empty())
        {
            return child;
        }

        const char last = base.back();
        if (last == '/' || last == '\\')
        {
            return base + child;
        }

        return base + "/" + child;
    }

    std::string defaultPlatformStreamCachePath()
    {
#if defined(PLATFORM_WINDOWS)
        if (const char* base = std::getenv("LOCALAPPDATA"))
        {
            return joinPath(base, "RV/stream");
        }
        if (const char* temp = std::getenv("TEMP"))
        {
            return joinPath(temp, "RV/stream");
        }
        return "C:/Temp/RV/stream";
#elif defined(PLATFORM_DARWIN)
        if (const char* home = std::getenv("HOME"))
        {
            return joinPath(home, "Library/Caches/RV/stream");
        }
        return "/tmp/RV/stream";
#else
        if (const char* xdg = std::getenv("XDG_CACHE_HOME"))
        {
            if (xdg[0] != '\0')
            {
                return joinPath(xdg, "RV/stream");
            }
        }
        if (const char* home = std::getenv("HOME"))
        {
            return joinPath(home, ".cache/RV/stream");
        }
        return "/tmp/RV/stream";
#endif
    }

    bool ensureDirectoryExists(const std::string& path)
    {
        if (path.empty())
        {
            return false;
        }

        if (TwkUtil::fileExists(path.c_str()))
        {
            return true;
        }

        const std::string parent = TwkUtil::dirname(path);
        if (!parent.empty() && parent != "." && parent != path)
        {
            if (!ensureDirectoryExists(parent))
            {
                return false;
            }
        }

#ifdef PLATFORM_WINDOWS
        return _mkdir(path.c_str()) == 0 || errno == EEXIST;
#else
        return mkdir(path.c_str(), 0755) == 0 || errno == EEXIST;
#endif
    }
} // namespace

std::string streamCachePath()
{
    static std::once_flag initialized;
    static std::string cachedPath;

    std::call_once(initialized,
                   []()
                   {
                       const std::string configured = evStreamCachePath.getValue();
                       cachedPath = configured.empty() ? defaultPlatformStreamCachePath() : configured;

                       if (!ensureDirectoryExists(cachedPath))
                       {
                           std::cerr << "WARNING: could not create FFmpeg stream cache directory: " << cachedPath << std::endl;
                       }
                       else
                       {
                           std::cerr << "INFO: FFmpeg stream cache path: " << cachedPath << std::endl;
                       }
                   });

    return cachedPath;
}
