//******************************************************************************
//
// Copyright (C) 2026 Autodesk, Inc. All Rights Reserved.
//
// SPDX-License-Identifier: Apache-2.0
//
//******************************************************************************

#ifndef __stream__StreamCachePath__h__
#define __stream__StreamCachePath__h__

#include <string>

//
// Returns the FFmpeg stream media cache directory.
//
// Uses RV_STREAM_CACHE_PATH when set; otherwise picks a per-user platform
// default. Creates the directory if needed and logs the resolved path once.
//

std::string streamCachePath();

#endif // __stream__StreamCachePath__h__
