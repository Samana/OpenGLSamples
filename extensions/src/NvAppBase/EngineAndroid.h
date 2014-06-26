//----------------------------------------------------------------------------------
// File:        NvAppBase/EngineAndroid.h
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
#ifndef ENGINE_ANDROID_H
#define ENGINE_ANDROID_H

//BEGIN_INCLUDE(all)
#include <NvFoundation.h>


#include <jni.h>
#include <errno.h>
#include <EGL/egl.h>
#include "NvAppBase/NvAppBase.h"
#include "NvEGLUtil/NvEGLUtil.h"

#define EGL_STATUS_LOG(str) \
    LOGD("Success: %s (%s:%d)", str, __FUNCTION__, __LINE__)

#define EGL_ERROR_LOG(str) \
        LOGE("Failure: %s, error = 0x%08x (%s:%d)", str, eglGetError(), __FUNCTION__, __LINE__)

/**
 * Our saved state data.
 */
struct saved_state {
    float x;
    float y;
};

struct AInputEvent;
struct android_app;

class NvEGLAppContext: public NvGLAppContext {
public:
    NvEGLAppContext(NvEGLUtil* egl) : 
        NvGLAppContext(NvGLPlatformInfo(
            NvGLPlatformCategory::PLAT_MOBILE, 
            NvGLPlatformOS::OS_ANDROID))
        , mEGL(egl) {
        EGLDisplay disp = mEGL->getDisplay();
        EGLConfig config = mEGL->getConfig();
        eglGetConfigAttrib(disp, config, EGL_RED_SIZE, (EGLint *)&mConfig.redBits);
        eglGetConfigAttrib(disp, config, EGL_GREEN_SIZE, (EGLint *)&mConfig.greenBits);
        eglGetConfigAttrib(disp, config, EGL_BLUE_SIZE, (EGLint *)&mConfig.blueBits);
        eglGetConfigAttrib(disp, config, EGL_ALPHA_SIZE, (EGLint *)&mConfig.alphaBits);
        eglGetConfigAttrib(disp, config, EGL_DEPTH_SIZE, (EGLint *)&mConfig.depthBits);
        eglGetConfigAttrib(disp, config, EGL_STENCIL_SIZE, (EGLint *)&mConfig.stencilBits);

        mConfig.apiVer.api = (mEGL->getAPI() == EGL_OPENGL_API) 
            ? NvGfxAPI::GL : NvGfxAPI::GLES;
        mConfig.apiVer.majVersion = mEGL->getMajVer();
        mConfig.apiVer.minVersion = mEGL->getMinVer();
    }

    bool bindContext() {
        return mEGL->bind();
    }

    bool unbindContext() {
        return mEGL->unbind();
    }

    bool swap() {
        return mEGL->swap();
        return true;
    }

    bool setSwapInterval(int32_t) {
        return false;
    }

    int32_t width() {
        return mEGL->getWidth();
    }

    int32_t height() {
        return mEGL->getHeight();
    }

    GLproc getGLProcAddress(const char* procname) {
        return eglGetProcAddress(procname);
    }

    bool isExtensionSupported(const char* ext);

    virtual bool requestResetContext() {
        return mEGL->requestResetContext();
    }

    virtual void* getCurrentPlatformContext() { 
        return (void*)mEGL->getContext(); 
    }

    virtual void* getCurrentPlatformDisplay() { 
        return (void*)mEGL->getDisplay(); 
    }


protected:
    NvEGLUtil* mEGL;
};

class NvGamepadAndroid;

/**
 * Shared state for our app.
 */
class Engine : public NvPlatformContext {
public:
    Engine(struct android_app* app);
    ~Engine();

    virtual bool isAppRunning();
    virtual void requestExit();
    virtual bool pollEvents(NvInputCallbacks* callbacks);
    virtual bool isContextLost();
    virtual bool isContextBound();
    virtual bool shouldRender();
    virtual bool hasWindowResized();
    virtual NvGamepad* getGamepad() { return (NvGamepad*)mGamepad; }
    virtual void setAppTitle(const char* title) { /* TODO if anything we can do... */ }
    virtual const std::vector<std::string>& getCommandLine() { return m_commandLine; }

    struct android_app* mApp;

    int32_t mForceRender;
    bool mResizePending;
    NvEGLUtil* mEGL;
    struct saved_state mState;
    NvInputCallbacks* mCallbacks;

    NvGamepadAndroid* mGamepad;
    uint32_t mPadChangedMask;

    static int32_t handleInputThunk(struct android_app* app, AInputEvent* event);
    static void handleCmdThunk(struct android_app* app, int32_t cmd);

    int32_t handleInput(AInputEvent* event);
    void handleCommand(int32_t cmd);

    void requestForceRender() { if (mForceRender < 4) mForceRender = 4; }

    std::vector<std::string> m_commandLine;
};

#endif
