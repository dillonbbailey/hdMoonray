// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <pxr/imaging/hd/types.h>
#include <pxr/usd/sdf/path.h>

namespace hdMoonray {

void hdmLogSyncStart(const std::string& type, const pxr::SdfPath& id, pxr::HdDirtyBits *dirtyBits);
void hdmLogSyncEnd(const pxr::SdfPath& id);

void hdmLogRenderBuffer(const std::string& msg,const pxr::SdfPath& id);

void hdmLogArras(const std::string& msg);

// True when HDM_LOG_FILE is set; check before building expensive messages.
bool hdmLogEnabled();
// One line, as given.
void hdmLogMessage(const std::string& msg);

}

