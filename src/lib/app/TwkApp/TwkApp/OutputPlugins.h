//
// Copyright (C) 2023  Autodesk, Inc. All Rights Reserved. 
// 
// SPDX-License-Identifier: Apache-2.0 
//

#pragma once

#include <string>
#include <unordered_set>

namespace TwkApp {

class OutputPlugin;

//
//  OutputPlugins
//
//  Manages the audio/video output plugins
//

class OutputPlugins
{
  public:

    OutputPlugins();
    virtual ~OutputPlugins();

    typedef std::unordered_set<OutputPlugin*> Plugins;

    //
    //  Load all on-disk plugins
    //

    static void loadPlugins(const std::string& envVar);

    //
    //  Unload all on-disk plugins
    //

    static void unloadPlugins();

    //
    //  Returns an unordered set of all the currently loaded plugins
    //

    static Plugins& plugins();

  private:

    static void addPlugin(TwkApp::OutputPlugin*);

    static bool m_loadedAll;
    static Plugins* m_plugins;
};

} // TwkApp
