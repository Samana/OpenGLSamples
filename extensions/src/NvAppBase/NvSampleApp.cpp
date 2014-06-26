//----------------------------------------------------------------------------------
// File:        NvAppBase/NvSampleApp.cpp
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
#include "NvAppBase/NvSampleApp.h"
#include "NV/NvLogs.h"
#include "NV/NvPlatformGL.h"
#include "NvAppBase/NvFramerateCounter.h"
#include "NvAppBase/NvInputTransformer.h"
#include "NvGLUtils/NvImage.h"
#include "NvGLUtils/NvSimpleFBO.h"
#include "NvGLUtils/NvTimers.h"
#include "NvUI/NvTweakBar.h"
#include "NV/NvString.h"
#include "NV/NvTokenizer.h"

#include <stdarg.h>
#include <sstream>

NvSampleApp::NvSampleApp(NvPlatformContext* platform, const char* appTitle) : 
    NvAppBase(platform, appTitle)
    , mFramerate(0L)
    , mFrameDelta(0.0f)
    , mUIWindow(0L)
    , mFPSText(0L)
    , mTweakBar(0L)
    , mTweakTab(0L)
    , mMainFBO(0)
    , mUseFBOPair(false)
    , mCurrentFBOIndex(0)
    , m_fboWidth(0)
    , m_fboHeight(0)
    , m_windowWidth(0)
    , m_windowHeight(0)
    , m_desiredWidth(0)
    , m_desiredHeight(0)
    , mTestMode(false)
    , mTestDuration(0.0f)
    , mTestRepeatFrames(1)
    , m_testModeIssues(TEST_MODE_ISSUE_NONE)
{
    m_transformer = new NvInputTransformer;
    mFrameTimer = createStopWatch();
    memset(mLastPadState, 0, sizeof(mLastPadState));

    mAutoRepeatTimer = createStopWatch();
    mAutoRepeatButton = 0; // none yet! :)
    mAutoRepeatTriggered = false;

    mFBOPair[0] = NULL;
    mFBOPair[1] = NULL;

    const std::vector<std::string>& cmd = platform->getCommandLine();
    std::vector<std::string>::const_iterator iter = cmd.begin();

    while (iter != cmd.end()) {
        if (0==(*iter).compare("-w")) {
            iter++;
            std::stringstream(*iter) >> m_desiredWidth;
        } else if (0==(*iter).compare("-h")) {
            iter++;
            std::stringstream(*iter) >> m_desiredHeight;
        } else if (0==(*iter).compare("-testmode")) {
            mTestMode = true;
            iter++;
            std::stringstream(*iter) >> mTestDuration;
            iter++;
            mTestName = (*iter); // both std::string
        } else if (0==(*iter).compare("-repeat")) {
            iter++;
            std::stringstream(*iter) >> mTestRepeatFrames;
        } else if (0==(*iter).compare("-fbo")) {
            mUseFBOPair = true;
            iter++;
            std::stringstream(*iter) >> m_fboWidth;
            iter++;
            std::stringstream(*iter) >> m_fboHeight;
        }
        iter++;
    }
    NvCPUTimer::globalInit(this);
}

NvSampleApp::~NvSampleApp() 
{ 
    // clean up internal allocs
    delete mFrameTimer;
    delete m_transformer;
}

void NvSampleApp::baseInitRendering(void) {
    LOGI("GL_RENDERER   = %s", (char *) glGetString(GL_RENDERER));
    LOGI("GL_VERSION    = %s", (char *) glGetString(GL_VERSION));
    LOGI("GL_VENDOR     = %s", (char *) glGetString(GL_VENDOR));

    NvGPUTimer::globalInit(*getGLContext());

    if (mUseFBOPair) {
        // clear the main framebuffer to black for later testing
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        mUseFBOPair = false;
        SwapBuffers();
        mUseFBOPair = true;

        NvSimpleFBO::Desc desc;
        desc.width = m_fboWidth;
        desc.height = m_fboHeight;
        desc.color.format = GL_RGBA;
        desc.color.filter = GL_LINEAR;
        desc.color.type = GL_UNSIGNED_BYTE;
        desc.color.wrap = GL_CLAMP_TO_EDGE;
        desc.depth.format = GL_DEPTH_COMPONENT;
        desc.depth.type = GL_UNSIGNED_INT;
        desc.depth.filter = GL_NEAREST;

        mFBOPair[0] = new NvSimpleFBO(desc);
        mFBOPair[1] = new NvSimpleFBO(desc);

        SwapBuffers();
    }

    LOGI("GL_EXTENSIONS =");

    // Break the extensions into lines without breaking extensions
    // (since unbroken line-wrap with extensions hurts search)
    {
        const int32_t lineMaxLen = 80;
        const char *s, *delimStr = " ";
        uint32_t ltl, delimLen = strlen(delimStr);
        std::string exbuf;
        NvTokenizer tok((const char*)glGetString(GL_EXTENSIONS));
        while (tok.readToken()) {
            ltl = tok.getLastTokenLen();
            s = tok.getLastTokenPtr();
            if (exbuf.length()+delimLen+ltl > lineMaxLen) { // adding would exceed max, print and clear
                if (exbuf.length())
                    LOGI("%s", exbuf.c_str());
                exbuf.clear();
                if (ltl > lineMaxLen) { // alone is too long, print alone and continue
                    LOGI("%s", s);
                }
                continue; //in either case, next...
            }
            exbuf.append(delimStr);
            exbuf.append(s);
        }
        if (exbuf.length()) // flush last line
            LOGI("%s\n\n", exbuf.c_str());
    }

    // check extensions and enable DXT expansion if needed
    bool hasDXT = getGLContext()->isExtensionSupported("GL_EXT_texture_compression_s3tc") ||
          getGLContext()->isExtensionSupported("GL_EXT_texture_compression_dxt1");
    if (!hasDXT) {
        LOGI("Device has no DXT texture support - enabling DXT expansion");
        NvImage::setDXTExpansion(true);
    }

    initRendering();
    baseInitUI();
}

void NvSampleApp::baseInitUI(void) {
    // safe to now pass through title to platform layer...
    if (!mAppTitle.empty())
        mPlatform->setAppTitle(mAppTitle.c_str());

    if (!mUIWindow) {
        const int32_t w = getGLContext()->width(), h = getGLContext()->height();
        mUIWindow = new NvUIWindow((float)w, (float)h);
        mFPSText = new NvUIValueText("", NvUIFontFamily::SANS, w/40.0f, NvUITextAlign::RIGHT,
                                    0.0f, 1, NvUITextAlign::RIGHT);
        mFPSText->SetColor(NV_PACKED_COLOR(0x30,0xD0,0xD0,0xB0));
        mFPSText->SetShadow();
        mUIWindow->Add(mFPSText, (float)w-8, 0);

        if (mTweakBar==NULL) {
            mTweakBar = NvTweakBar::CreateTweakBar(mUIWindow); // adds to window internally.
            mTweakBar->SetVisibility(false);

            if (!mAppTitle.empty()) {
                mTweakBar->addLabel(mAppTitle.c_str(), true);
                mTweakBar->addPadding();
            }

            // for now, app will own the tweakbar tab button
            float high = mTweakBar->GetDefaultLineHeight();
            NvUIElement *els[3];
            els[0] = new NvUIGraphic("arrow_blue.dds");
            els[0]->SetDimensions(high/2, high/2);
            els[1] = new NvUIGraphic("arrow_blue_left.dds");
            els[1]->SetDimensions(high/2, high/2);
            els[2] = NULL;

            mTweakTab = new NvUIButton(NvUIButtonType::CHECK, TWEAKBAR_ACTIONCODE_BASE, els);
            mTweakTab->SetHitMargin(high/2, high/2);
            mUIWindow->Add(mTweakTab, high*0.25f, mTweakBar->GetStartOffY()+high*0.125f);
        }

        CHECK_GL_ERROR();
    }

    initUI();
}

void NvSampleApp::baseReshape(int32_t w, int32_t h) {
    m_windowWidth = w;
    m_windowHeight = h;

    if (mUseFBOPair) {
        w = m_fboWidth;
        h = m_fboHeight;
    }

    if ((w == m_width) && (h == m_height))
        return;

    m_width = w;
    m_height = h;

    mUIWindow->HandleReshape((float)w, (float)h);

    m_transformer->setScreenSize(w, h);

    reshape(w, h);
}

void NvSampleApp::baseUpdate(void) {
    update();
}

void NvSampleApp::baseDraw(void) {
    draw();
}

void NvSampleApp::baseDrawUI(void) {

    if (mUIWindow && mUIWindow->GetVisibility()) {
        if (mFPSText) {
            mFPSText->SetValue(mFramerate->getMeanFramerate());
        }
        NvUST time = 0;
        NvUIDrawState ds(time, getGLContext()->width(), getGLContext()->height());
        mUIWindow->Draw(ds);
    }

    drawUI();
}

void NvSampleApp::baseFocusChanged(bool focused) {
    focusChanged(focused);
}

void NvSampleApp::baseHandleReaction() {
    NvUIEventResponse r;
    const NvUIReaction& react = NvUIElement::GetReaction();
    // we let the UI handle any reaction first, in case there
    // are interesting side-effects such as updating variables...
    r = mUIWindow->HandleReaction(react);
    // then the app is always given a look, even if already handled...
    //if (r==nvuiEventNotHandled)
    r = handleReaction(react);
}

void NvSampleApp::syncValue(NvTweakVarBase *var) {
    NvUIReaction &react = mUIWindow->GetReactionEdit(true);
    react.code = var->getActionCode();
    react.flags = NvReactFlag::FORCE_UPDATE;
    baseHandleReaction();
}

bool NvSampleApp::pointerInput(NvInputDeviceType::Enum device, NvPointerActionType::Enum action, 
    uint32_t modifiers, int32_t count, NvPointerEvent* points) {
    NvUST time = 0;
    static bool isDown = false;
    static float startX = 0, startY = 0;
    bool isButtonEvent = (action==NvPointerActionType::DOWN)||(action==NvPointerActionType::UP);
    if (isButtonEvent)
        isDown = (action==NvPointerActionType::DOWN);

    if (mUIWindow!=NULL) {
        NvInputEventClass::Enum giclass = NvInputEventClass::MOUSE; // default to mouse
        NvGestureKind::Enum gikind;
        // override for non-mouse device.
        if (device==NvInputDeviceType::STYLUS)
            giclass = NvInputEventClass::STYLUS;
        else if (device==NvInputDeviceType::TOUCH)
            giclass = NvInputEventClass::TOUCH;
        // since not using a heavyweight gesture detection system,
        // determine reasonable kind/state to pass along here.
        if (isButtonEvent)
            gikind = (isDown ? NvGestureKind::PRESS : NvGestureKind::RELEASE);
        else
            gikind = (isDown ? NvGestureKind::DRAG : NvGestureKind::HOVER);
        float x=0, y=0;
        if (count)
        {
            x = points[0].m_x;
            y = points[0].m_y;
        }
        NvGestureEvent gesture(giclass, gikind, x, y);
        if (isButtonEvent)
        {
            if (isDown)
            {
                startX = x;
                startY = y;
            }
        }
        else if (isDown)
        {
            gesture.x = startX;
            gesture.y = startY;
            gesture.dx = x - startX;
            gesture.dy = y - startY;
        }
        NvUIEventResponse r = mUIWindow->HandleEvent(gesture, time, NULL);
        if (r&nvuiEventHandled) 
        {
            if (r&nvuiEventHadReaction)
                baseHandleReaction();
            return true;
        }
    }

    if (handlePointerInput(device, action, modifiers, count, points))
        return true;
    else
        return m_transformer->processPointer(device, action, modifiers, count, points);
}


void NvSampleApp::addTweakKeyBind(NvTweakVarBase *var, uint32_t incKey, uint32_t decKey/*=0*/) {
    mKeyBinds[incKey] = NvTweakBind(NvTweakCmd::INCREMENT, var);
    if (decKey)
        mKeyBinds[decKey] = NvTweakBind(NvTweakCmd::DECREMENT, var);
}

bool NvSampleApp::keyInput(uint32_t code, NvKeyActionType::Enum action) {
    // only do down and repeat for now.
    if (NvKeyActionType::UP!=action) {
        NvAppKeyBind::const_iterator bind = mKeyBinds.find(code);
        if (bind != mKeyBinds.end()) {
            // we have a binding.  do something with it.
            NvTweakVarBase *var = bind->second.mVar;
            if (var) {
                switch (bind->second.mCmd) {
                    case NvTweakCmd::RESET:
                        var->reset();
                        break;
                    case NvTweakCmd::INCREMENT:
                        var->increment();
                        break;
                    case NvTweakCmd::DECREMENT:
                        var->decrement();
                        break;
                    default:
                        return false;
                }

                syncValue(var);
                // we're done.
                return true;
            }
        }
    }

    if (mTweakBar && NvKeyActionType::UP!=action) // handle down+repeat as needed
    {
        // would be nice if this was some pluggable class we could add/remove more easily like inputtransformer.
        NvUIEventResponse r = nvuiEventNotHandled;
        switch(code)
        {
            case NvKey::K_TAB: {
                if (NvKeyActionType::DOWN!=action) break; // we don't want autorepeat...
                NvUIReaction &react = mUIWindow->GetReactionEdit(true);
                react.code = TWEAKBAR_ACTIONCODE_BASE;
                react.state = mTweakBar->GetVisibility() ? 0 : 1;
                r = nvuiEventHandledReaction;
                break;
            }
            case NvKey::K_ARROW_DOWN: {
                if (NvKeyActionType::DOWN!=action) break; // we don't want autorepeat...
                r = mUIWindow->HandleFocusEvent(NvFocusEvent::MOVE_DOWN);
                break;
            }
            case NvKey::K_ARROW_UP: {
                if (NvKeyActionType::DOWN!=action) break; // we don't want autorepeat...
                r = mUIWindow->HandleFocusEvent(NvFocusEvent::MOVE_UP);
                break;
            }
            case NvKey::K_ENTER: {
                if (NvKeyActionType::DOWN!=action) break; // we don't want autorepeat...
                r = mUIWindow->HandleFocusEvent(NvFocusEvent::ACT_PRESS);
                break;
            }
            case NvKey::K_BACKSPACE: {
                if (NvKeyActionType::DOWN!=action) break; // we don't want autorepeat...
                r = mUIWindow->HandleFocusEvent(NvFocusEvent::FOCUS_CLEAR);
                break;
            }
            case NvKey::K_ARROW_LEFT: {
                r = mUIWindow->HandleFocusEvent(NvFocusEvent::ACT_DEC);
                break;
            }
            case NvKey::K_ARROW_RIGHT: {
                r = mUIWindow->HandleFocusEvent(NvFocusEvent::ACT_INC);
                break;
            }
            default:
                break;
        }

        if (r&nvuiEventHandled) 
        {
            if (r&nvuiEventHadReaction)
                baseHandleReaction();
            return true;
        }
    }
        
    if (handleKeyInput(code, action))
        return true;

    // give last shot to transformer.
    return m_transformer->processKey(code, action);
}

bool NvSampleApp::characterInput(uint8_t c) {
    if (handleCharacterInput(c))
        return true;
    return false;
}

void NvSampleApp::addTweakButtonBind(NvTweakVarBase *var, uint32_t incBtn, uint32_t decBtn/*=0*/) {
    mButtonBinds[incBtn] = NvTweakBind(NvTweakCmd::INCREMENT, var);
    if (decBtn)
        mButtonBinds[decBtn] = NvTweakBind(NvTweakCmd::DECREMENT, var);
}


bool NvSampleApp::gamepadButtonChanged(uint32_t button, bool down) {
    if (mAutoRepeatButton == button && !down) {
        mAutoRepeatButton = 0;
        mAutoRepeatTriggered = false;
        mAutoRepeatTimer->stop();
    }

    // currently, we only react on the button DOWN
    if (down) {
        NvAppButtonBind::const_iterator bind = mButtonBinds.find(button);
        if (bind != mButtonBinds.end()) {
            // we have a binding.  do something with it.
            NvTweakVarBase *var = bind->second.mVar;
            if (var) {
                switch (bind->second.mCmd) {
                    case NvTweakCmd::RESET:
                        var->reset();
                        break;
                    case NvTweakCmd::INCREMENT:
                        var->increment();
                        break;
                    case NvTweakCmd::DECREMENT:
                        var->decrement();
                        break;
                    default:
                        return false;
                }

                syncValue(var);
                // we're done.
                return true;
            }
        }
    }

    if (handleGamepadButtonChanged(button, down))
        return true;

    if (down && mTweakBar) {
        // would be nice if this was some pluggable class we could add/remove more easily like inputtransformer.
        NvUIEventResponse r = nvuiEventNotHandled;
        switch(button) {
            case NvGamepad::BUTTON_START: {
                NvUIReaction &react = mUIWindow->GetReactionEdit(true);
                react.code = TWEAKBAR_ACTIONCODE_BASE;
                react.state = mTweakBar->GetVisibility() ? 0 : 1;
                r = nvuiEventHandledReaction;
                break;
            }
            case NvGamepad::BUTTON_BACK: {
                appRequestExit();
                return true;
            }
            case NvGamepad::BUTTON_DPAD_DOWN: {
                r = mUIWindow->HandleFocusEvent(NvFocusEvent::MOVE_DOWN);
                break;
            }
            case NvGamepad::BUTTON_DPAD_UP: {
                r = mUIWindow->HandleFocusEvent(NvFocusEvent::MOVE_UP);
                break;
            }
            case NvGamepad::BUTTON_A: {
                r = mUIWindow->HandleFocusEvent(NvFocusEvent::ACT_PRESS);
                break;
            }
            case NvGamepad::BUTTON_B: {
                r = mUIWindow->HandleFocusEvent(NvFocusEvent::FOCUS_CLEAR);
                break;
            }
            case NvGamepad::BUTTON_DPAD_LEFT: {
                r = mUIWindow->HandleFocusEvent(NvFocusEvent::ACT_DEC);
                mAutoRepeatTimer->start();
                mAutoRepeatButton = button;
                break;
            }
            case NvGamepad::BUTTON_DPAD_RIGHT: {
                r = mUIWindow->HandleFocusEvent(NvFocusEvent::ACT_INC);
                mAutoRepeatTimer->start();
                mAutoRepeatButton = button;
                break;
            }
            default: break;
        }

        if (r&nvuiEventHandled) 
        {
            if (r&nvuiEventHadReaction)
                baseHandleReaction();
            return true;
        }
    }
    return false;
}

bool NvSampleApp::gamepadChanged(uint32_t changedPadFlags) {

    if (handleGamepadChanged(changedPadFlags))
        return true;

    if (!changedPadFlags)
        return false;

    NvGamepad* pad = getPlatformContext()->getGamepad();
    if (!pad) return false;

    NvGamepad::State state;
    int32_t i, j;
    uint32_t button;
    bool buttonDown;
    for (i = 0; i < NvGamepad::MAX_GAMEPADS; i++) {
        if (changedPadFlags & (1<<i)) {
            pad->getState(i, state);
            if (state.mButtons != mLastPadState[i].mButtons) {
                // parse through the buttons and send events.
                for (j=0; j<32; j++) { // iterate button bits
                    button = 1<<j;
                    buttonDown = (button & state.mButtons)>0;
                    if (buttonDown != ((button & mLastPadState[i].mButtons)>0))
                        gamepadButtonChanged(button, buttonDown);
                }
            }
            // when done processing a gamepad, copy off the state.
            memcpy(mLastPadState+i, &state, sizeof(state));
        }
    }

    // give last shot to transformer.  not sure how we 'consume' input though.
    return m_transformer->processGamepad(changedPadFlags, *pad);
}

void NvSampleApp::mainLoop() {
    bool hasInitializedGL = false;

    NvStopWatch* testModeTimer = createStopWatch();
    int32_t testModeFrames = -TESTMODE_WARMUP_FRAMES;
    float totalTime = -1e6f; // don't exit during startup

    if (mTestMode) {
        writeLogFile(mTestName, false, "*** Starting Test\n");
    }

    mFramerate = new NvFramerateCounter(this);

    mFrameTimer->start();

    while (getPlatformContext()->isAppRunning() && !isExiting()) {
        bool needsReshape = false;

        getPlatformContext()->pollEvents(this);

        NvPlatformContext* ctx = getPlatformContext();

        baseUpdate();

        // If the context has been lost and graphics resources are still around,
        // signal for them to be deleted
        if (ctx->isContextLost()) {
            if (hasInitializedGL) {
                baseShutdownRendering();
                hasInitializedGL = false;
            }
        }

        // If we're ready to render (i.e. the GL is ready and we're focused), then go ahead
        if (ctx->shouldRender()) {
            // If we've not (re-)initialized the resources, do it
            if (!hasInitializedGL) {
                NvImage::setAPIVersion(getGLContext()->getConfiguration().apiVer);

                baseInitRendering();
                hasInitializedGL = true;
                needsReshape = true;

                // In test mode, disable VSYNC if possible
                if (mTestMode)
                    getGLContext()->setSwapInterval(0);
            } else if (ctx->hasWindowResized()) {
                if (mUIWindow) {
                    const int32_t w = getGLContext()->width(), h = getGLContext()->height();
                    mUIWindow->HandleReshape((float)w, (float)h);
                }

                needsReshape = true;
            }

            if (needsReshape) {
                baseReshape(getGLContext()->width(), getGLContext()->height());
            }

            mFrameTimer->stop();

            if (mTestMode) {
                // Simulate 60fps
                mFrameDelta = 1.0f / 60.0f;

                // just an estimate
                totalTime += mFrameTimer->getTime();
            } else {
                mFrameDelta = mFrameTimer->getTime();
                // just an estimate
                totalTime += mFrameDelta;
            }
            m_transformer->update(mFrameDelta);
            mFrameTimer->reset();

            // initialization may cause the app to want to exit
            if (!isExiting()) {
                mFrameTimer->start();

                if (mAutoRepeatButton) {
                    const float elapsed = mAutoRepeatTimer->getTime();
                    if ( (!mAutoRepeatTriggered && elapsed >= 0.5f) ||
                         (mAutoRepeatTriggered && elapsed >= 0.04f) ) { // 25hz repeat
                        mAutoRepeatTriggered = true;
                        gamepadButtonChanged(mAutoRepeatButton, true);
                    }
                }

                baseDraw();
                CHECK_GL_ERROR(); // sanity catch errors
                if (!mTestMode) {
                    baseDrawUI();
                    CHECK_GL_ERROR(); // sanity catch errors
                }

                if (mTestMode && (mTestRepeatFrames > 1)) {
                    // repeat frame so that we can simulate a heavier workload
                    for (int i = 1; i < mTestRepeatFrames; i++) {
                        baseUpdate();
                        m_transformer->update(mFrameDelta);
                        baseDraw();
                    }
                }

                if (mTestMode && mUseFBOPair) {
                    // Check if the app bound FBO 0 in FBO mode
                    GLuint currFBO = 0;
                    // Enum has MANY names based on extension/version
                    // but they all map to 0x8CA6
                    glGetIntegerv(0x8CA6, (GLint*)&currFBO);

                    if (currFBO == 0)
                        m_testModeIssues |= TEST_MODE_FBO_ISSUE;
                }

                SwapBuffers();

                if (mFramerate->nextFrame()) {
                    // for now, disabling console output of fps as we have on-screen.
                    // makes it easier to read USEFUL log output messages.
                    LOGI("fps: %.2f", mFramerate->getMeanFramerate());
                }
            }

            if (mTestMode) {
                testModeFrames++;
                // if we've come to the end of the warm-up, start timing
                if (testModeFrames == 0) {
                    totalTime = 0.0f;
                    testModeTimer->start();
                }

                if (totalTime > mTestDuration) {
                    testModeTimer->stop();
                    double frameRate = testModeFrames / testModeTimer->getTime();
                    logTestResults((float)frameRate, testModeFrames);
                    exit(0);
//                    appRequestExit();
                }
            }
        }
    }

    if (hasInitializedGL) {
        baseShutdownRendering();
        hasInitializedGL = false;
    }

    // mainloop exiting, clean up things created in mainloop lifespan.
    delete mFramerate;
    mFramerate = NULL;
}

bool NvSampleApp::requireExtension(const char* ext, bool exitOnFailure) {
    if (!getGLContext()->isExtensionSupported(ext)) {
        if (exitOnFailure) {
            std::string caption = std::string("The current system does not appear to support the extension ")
                + std::string(ext) + std::string(", which is required by the sample.  "
                "This is likely because the system's GPU or driver does not support the extension.  "
                "Please see the sample's source code for details");
            errorExit(caption.c_str());
        }

        return false;
    }

    return true;
}

bool NvSampleApp::requireMinAPIVersion(const NvGfxAPIVersion& minApi, bool exitOnFailure) {
    const NvGfxAPIVersion api = getGLContext()->getConfiguration().apiVer;
    if (api < minApi) {
        if (exitOnFailure) {
            char caption [1024];
#pragma warning( push )
#pragma warning( disable : 4996 )
            sprintf(caption, "The current system does not appear to support the minimum GL API required "
                "by the sample (requested: %s %d.%d, got: %s %d.%d).  This is likely because the system's GPU or driver "
                "does not support the API.  Please see the sample's source code for details", 
                (minApi.api == NvGfxAPI::GL) ? "GL" : "GLES", 
                minApi.majVersion, minApi.minVersion,
                (api.api == NvGfxAPI::GL) ? "GL" : "GLES", api.majVersion, api.minVersion);
#pragma warning( pop )
            errorExit(caption);
        }

        return false;
    }

    return true;
}

void NvSampleApp::errorExit(const char* errorString) {
    if (mTestMode) {
        writeLogFile(mTestName, true, "Fatal Error from app\n");
        writeLogFile(mTestName, true, errorString);
        appRequestExit();
    } else {
        // we set the flag here manually.  The exit will not happen until
        // the user closes the dialog.  But we want to act as if we are
        // already exiting (which we are), so we do not render
        m_requestedExit = true;
        showDialog("Fatal Error", errorString, true);
    }
}

bool NvSampleApp::getRequestedWindowSize(int32_t& width, int32_t& height) {
    bool changed = false;
    if (m_desiredWidth != 0) {
        width = m_desiredWidth;
        changed = true;
    }

    if (m_desiredHeight != 0) {
        height = m_desiredHeight;
        changed = true;
    }

    return changed;
}

void NvSampleApp::SwapBuffers() {
    if (mUseFBOPair) {
        mCurrentFBOIndex = mCurrentFBOIndex ? 0 : 1;
        mMainFBO = mFBOPair[mCurrentFBOIndex]->fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, mMainFBO);
    } else {
        getGLContext()->swap();
    }
}

void NvSampleApp::baseShutdownRendering(void) {
    delete mFBOPair[0];
    delete mFBOPair[1];

    // clean up UI elements.
    delete mUIWindow; // note it holds all our UI, so just null other ptrs.
    mUIWindow = NULL;
    mFPSText = NULL;
    mTweakBar = NULL;
    mTweakTab = NULL;

    shutdownRendering();
}

void NvSampleApp::logTestResults(float frameRate, int32_t frames) {
    LOGI("Test Frame Rate = %lf (frames = %d)\n", frameRate, frames);
    writeLogFile(mTestName, true, "\n%s %lf fps (%d frames)\n", mTestName.c_str(), frameRate, frames);
    if (mUseFBOPair) {
        writeLogFile(mTestName, true, "\nOffscreen Mode: FBO Size %d x %d\n", m_width, m_height);
    } else {
        writeLogFile(mTestName, true, "\nWindow Size %d x %d\n", m_width, m_height);
    }
    writeLogFile(mTestName, true, "GL_VENDOR %s\n", glGetString(GL_VENDOR));
    writeLogFile(mTestName, true, "GL_RENDERER %s\n", glGetString(GL_RENDERER));
    writeLogFile(mTestName, true, "GL_EXTENSIONS %s\n", glGetString(GL_EXTENSIONS));

    if (m_testModeIssues != TEST_MODE_ISSUE_NONE) {
        writeLogFile(mTestName, true, "\nWARNING - there were potential test mode anomalies\n");

        if (m_testModeIssues & TEST_MODE_FBO_ISSUE) {
            writeLogFile(mTestName, true, "\tThe application appears to have explicitly bound the onscreen framebuffer\n"
                "\tSince the test was being run in offscreen rendering mode, this could invalidate results\n"
                "\tThe application should be checked for glBindFramebuffer of 0\n\n");
        }
    }
    // This above TEST_MODE_FBO_ISSUE only checks the flag from the end of each frame;
    // it only detects if the app left FBO 0 bound at the end of the frame.  We could
    // still miss a mid-frame binding of FBO 0.  The best way to test for that is to
    // read back FBO 0 at the end of the app and test if any pixel is non-zero:
    if (mUseFBOPair) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        const int32_t size = 4 * m_windowWidth * m_windowHeight;
        const uint8_t* onscreenData = new uint8_t[size];

        glReadPixels(0, 0, m_windowWidth, m_windowHeight, 
            GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)onscreenData);

        const uint8_t* ptr = onscreenData;
        for (int i = 0; i < size; i++) {
            if (*(ptr++)) {
                writeLogFile(mTestName, true, "\tThe application appears to have written to the onscreen framebuffer\n"
                    "\tSince the test was being run in offscreen rendering mode, this could invalidate results\n"
                    "\tThe application should be checked for glBindFramebuffer of 0\n\n");
                break;
            }
        }

        delete[] onscreenData;

        glBindFramebuffer(GL_FRAMEBUFFER, getMainFBO());
    }

    const uint8_t* data = new uint8_t[4 * m_width * m_height];

    glReadPixels(0, 0, m_width, m_height, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)data);

    writeScreenShot(m_width, m_height, data, mTestName);
    writeLogFile(mTestName, true, "Test Complete!");

    delete[] data;
}

