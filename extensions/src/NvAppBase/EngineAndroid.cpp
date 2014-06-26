//----------------------------------------------------------------------------------
// File:        NvAppBase/EngineAndroid.cpp
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

#ifdef ANDROID

//BEGIN_INCLUDE(all)
#include "EngineAndroid.h"
#include "NvAppBase/NvGLAppContext.h"
#include "NvGamepad/NvGamepadAndroid.h"

#include "NvAndroidNativeAppGlue.h"
#include <NvEGLUtil/NvEGLUtil.h>
#include "NV/NvLogs.h"

bool NvEGLAppContext::isExtensionSupported(const char* ext)
{
    return strstr((const char*)glGetString(GL_EXTENSIONS), ext) != NULL;
}

Engine::Engine(struct android_app* app) {
    mApp = app;

    mForceRender = 0;
    mResizePending = false;
    
    mEGL = NULL;

    memset( &mState, 0, sizeof(struct saved_state));

    mApp->userData = this;
    mApp->onAppCmd = handleCmdThunk;
    mApp->onInputEvent = handleInputThunk;

    mGamepad = new NvGamepadAndroid;

    if (mApp->savedState != NULL) {
        // We are starting with a previous saved state; restore from it.
        mState = *(struct saved_state*)mApp->savedState;
    }

}

Engine::~Engine()
{
    delete mEGL;
}

bool Engine::isAppRunning()
{
    return nv_app_status_running(mApp);
}

void Engine::requestExit()
{
    ANativeActivity_finish(mApp->activity);
}

bool Engine::pollEvents(NvInputCallbacks* callbacks)
{
    // Read all pending events.
    int32_t ident;
    int32_t events;
    struct android_poll_source* source;

    mCallbacks = callbacks;
    mPadChangedMask = 0;

    // If not animating, we will block forever waiting for events.
    // If animating, we loop until all events are read, then continue
    // to draw the next frame of animation.
    while ((ident=ALooper_pollAll((nv_app_status_focused(mApp)) ? 1 : 250, NULL, &events,
            (void**)&source)) >= 0) {

        // Process this event.
        if (source != NULL) {
            source->process(mApp, source);
        }
    }

    if (mCallbacks && mPadChangedMask)
        mCallbacks->gamepadChanged(mPadChangedMask);

    mCallbacks = NULL;
}

bool Engine::shouldRender()
{
    if (nv_app_status_interactable(mApp))
    {
        if (!mEGL->isReadyToRender(true))
            return false;

        return true;
    }
    else
    {
        // Even if we are not interactible, we may be visible, so we
        // HAVE to do any forced renderings if we can.  We must also
        // check for resize, since that may have been the point of the
        // forced render request in the first place!
        if ((mForceRender > 0) && mEGL->isReadyToRender(false)) 
        {
            return true;
        }
    }

    if (mForceRender > 0) {
        mForceRender--;
        return true;
    }

    return false;
}

bool Engine::isContextLost()
{
    return !mEGL->hasContext();
}

bool Engine::isContextBound()
{
    return mEGL->isBound();
}

bool Engine::hasWindowResized()
{
    if (mEGL->checkWindowResized())
    {
        requestForceRender();
        return true;
    }

    return false;
}


/**
 * Process the next input event.
 */
int32_t Engine::handleInputThunk(struct android_app* app, AInputEvent* event)
{
    Engine* engine = (Engine*)app->userData;
    if (!engine)
        return 0;

    return engine->handleInput(event);
}

/**
 * Process the next main command.
 */
void Engine::handleCmdThunk(struct android_app* app, int32_t cmd)
{
    Engine* engine = (Engine*)app->userData;
    if (!engine)
        return;

    engine->handleCommand(cmd);
}

static uint8_t mapAndroidCodeToChar(int32_t code) {
    // Only map alphanumeric characters (and only to caps) for now
    if (code >= AKEYCODE_A && code <= AKEYCODE_Z)
        return code - AKEYCODE_A + 'A';
    if (code >= AKEYCODE_0 && code <= AKEYCODE_9)
        return code - AKEYCODE_0 + '0';
    return 0;
}

/**
 * Helpers to match missing, newer motion processing functions in java.
 */
int32_t amotion_getActionMasked(AInputEvent* event) {
    int32_t action_raw = AMotionEvent_getAction(event);
    return (action_raw & AMOTION_EVENT_ACTION_MASK);
}

int32_t amotion_getActionIndex(AInputEvent* event) {
    int32_t action_raw = AMotionEvent_getAction(event);
       int32_t pointerIndex = (action_raw & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK);
    pointerIndex >>= AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
    return pointerIndex;
}

/**
 * Process the next input event.
 */
int32_t Engine::handleInput(AInputEvent* event) {
    if (mCallbacks && mGamepad->pollGamepads(event, mPadChangedMask))
        return true;
    else if (NULL==mCallbacks)
        return false;

    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        int32_t pointerCount = AMotionEvent_getPointerCount(event);

        int32_t action = amotion_getActionMasked(event);
        int32_t pointerIndex = amotion_getActionIndex(event);

        bool handled = false;

        float x=0, y=0;
        NvPointerEvent p[20]; // !!!!!TBD  should use linkedlist or app-member struct or something?  TODO
        // loop over pointercount and copy data
        // !!!!TBD TODO, might need to ensure that p[0] is always the data from the first-finger-touched ID...
        for (int i=0; i<pointerCount; i++) {
            x = AMotionEvent_getX(event, i);
            y = AMotionEvent_getY(event, i);
            p[i].m_x = x;
            p[i].m_y = y;
            p[i].m_id = AMotionEvent_getPointerId(event, i);;
        }

        mState.x = x;
        mState.y = y;
    
        NvInputDeviceType::Enum dev = NvInputDeviceType::TOUCH;
        NvPointerActionType::Enum pact;

        switch(action)
        {
        case AMOTION_EVENT_ACTION_DOWN:
            pact = NvPointerActionType::DOWN;
            break;
        case AMOTION_EVENT_ACTION_POINTER_DOWN:
            pact = NvPointerActionType::EXTRA_DOWN;
            break;
        case AMOTION_EVENT_ACTION_UP:
            pact = NvPointerActionType::UP;
            break;
        case AMOTION_EVENT_ACTION_POINTER_UP:
            pact = NvPointerActionType::EXTRA_UP;
            break;
        case AMOTION_EVENT_ACTION_MOVE:
            pact = NvPointerActionType::MOTION;
            break;
        case AMOTION_EVENT_ACTION_CANCEL:
            pact = NvPointerActionType::UP;
            pointerCount = 0; // clear.
            break;
        }

        handled = mCallbacks->pointerInput(dev, pact, 0, pointerCount, p);
        // return code handling...
        if (pact==NvPointerActionType::UP)
            return 1;
        return handled ? 1 : 0;
    } else if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY) {
        bool handled = false;
        int32_t source = AInputEvent_getSource(event);
        int32_t code = AKeyEvent_getKeyCode((const AInputEvent*)event);
        int32_t action = AKeyEvent_getAction((const AInputEvent*)event);

        bool down = (action != AKEY_EVENT_ACTION_UP) ? true : false;

        if (mCallbacks) {
            handled = mCallbacks->keyInput(code, 
                down ? NvKeyActionType::DOWN : NvKeyActionType::UP);

            if (!handled && down) {
                uint8_t c = mapAndroidCodeToChar(code);
                if (c)
                    handled = mCallbacks->characterInput(c);
            }
        }

        return handled ? 1 : 0;
    }

    return 0;
}

/**
 * Process the next main command.
 */
void Engine::handleCommand(int32_t cmd) {
    switch (cmd) {
        case APP_CMD_START:
            break;

        // The window is being shown, get it ready.
        // Note that on ICS, the EGL size will often be correct for the new size here
        // But on HC it will not be.  We need to defer checking the new res until the
        // first render with the new surface!
        case APP_CMD_INIT_WINDOW:
        case APP_CMD_WINDOW_RESIZED:
            mEGL->setWindow(mApp->window);
            requestForceRender();
            break;

        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
            mEGL->setWindow(NULL);
            break;

        case APP_CMD_GAINED_FOCUS:
            requestForceRender();
            break;

        case APP_CMD_LOST_FOCUS:
        case APP_CMD_PAUSE:
            requestForceRender();
            break;

        // ICS does not appear to require this, but on GB phones,
        // not having this causes rotation changes to be delayed or
        // ignored when we're in a non-rendering mode like autopause.
        // The forced renders appear to allow GB to process the rotation
        case APP_CMD_CONFIG_CHANGED:
            requestForceRender();
            break;

        case APP_CMD_SAVE_STATE:
            // The system has asked us to save our current state.  Do so.
            mApp->savedState = malloc(sizeof(struct saved_state));
            *((struct saved_state*)mApp->savedState) = mState;
            mApp->savedStateSize = sizeof(struct saved_state);
            break;

        case APP_CMD_DESTROY:
            break;
    }
}

#endif // ANDROID
