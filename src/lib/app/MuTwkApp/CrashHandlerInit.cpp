//
// Copyright (C) 2026 Autodesk, Inc. All Rights Reserved.
//
// SPDX-License-Identifier: Apache-2.0
//
#include <MuTwkApp/CrashHandlerInit.h>
#include <MuTwkApp/MuInterface.h>
#include <TwkUtil/CrashHandler.h>
#include <iostream>

namespace TwkApp
{

    bool initializeCrashHandler(const std::string& appName, const std::string& appVersion, const std::string& executableDir)
    {
        // The handler is always the platform wrapper (never the bare
        // crashpad_handler) so the log redirection - and on Linux the ulimit
        // safety cap - always apply. Naming is fixed (contract C5).
#ifdef PLATFORM_WINDOWS
        const std::string handlerName = "crashpad_handler.exe";
#elif defined(PLATFORM_LINUX)
        const std::string handlerName = "crashpad_handler_linux.sh";
#else
        const std::string handlerName = "crashpad_handler_macos.sh";
#endif
        const std::string handlerPath = executableDir + "/" + handlerName;

        const bool initialized = TwkUtil::CrashHandler::instance().initialize(appName, appVersion, handlerPath);

        if (initialized)
        {
            // Enable Mu debugging so crash dumps include script source context.
            setDebugging(true);
            std::cout << "INFO: Automatically enabled -debug mu for crash dumps" << std::endl;
        }

        return initialized;
    }

} // namespace TwkApp
