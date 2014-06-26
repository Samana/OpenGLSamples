//----------------------------------------------------------------------------------
// File:        NvAppBase/NvAppBase.cpp
// SDK Version: v1.2 
// Email:       gameworks@nvidia.com
// Site:        http://developer.nvidia.com/
//
// Copyright (c) 2014, NVIDIA CORPORATION. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of NVIDIA CORPORATION nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//----------------------------------------------------------------------------------
#include "NvAppBase/NvAppBase.h"

#include "NV/NvLogs.h"
#include "NV/NvStopWatch.h"
#include "NvGLUtils/NvImage.h"

#include <stdio.h>

NvAppBase::NvAppBase(NvPlatformContext* platform, const char *appTitle/*==NULL*/)
    : mPlatform(platform)
    , m_width(0)
    , m_height(0)
    , m_requestedExit(false)

{
    if (appTitle!=NULL && appTitle[0]!=0) { // something there..
        mAppTitle.assign(appTitle);
    }
}

NvAppBase::~NvAppBase() {
}

void NvAppBase::mainLoop() {
    bool hasInitializedGL = false;

    while (getPlatformContext()->isAppRunning() && !isExiting()) {
        bool needsReshape = false;

        getPlatformContext()->pollEvents(this);

        NvPlatformContext* ctx = getPlatformContext();

        update();

        // If the context has been lost and graphics resources are still around,
        // signal for them to be deleted
        if (ctx->isContextLost()) {
            if (hasInitializedGL) {
                shutdownRendering();
                hasInitializedGL = false;
            }
        }

        // If we're ready to render (i.e. the GL is ready and we're focused), then go ahead
        if (ctx->shouldRender()) {
            // If we've not (re-)initialized the resources, do it
            if (!hasInitializedGL) {
                NvImage::setAPIVersion(getGLContext()->getConfiguration().apiVer);

                initRendering();
                hasInitializedGL = true;
                needsReshape = true;
            } else if (ctx->hasWindowResized()) {
                needsReshape = true;
            }

            if (needsReshape) {
                reshape(getGLContext()->width(), getGLContext()->height());
            }

            if (!isExiting()) {
                draw();

                getGLContext()->swap();
            }
        }
    }

    if (hasInitializedGL) {
        shutdownRendering();
        hasInitializedGL = false;
    }
}

void NvAppBase::appRequestExit() {
    getPlatformContext()->requestExit();
    m_requestedExit = true; 
}

