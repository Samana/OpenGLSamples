//----------------------------------------------------------------------------------
// File:        NvAppBase/NvSampleApp.h
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

// simple SDK app interface
#ifndef NV_SAMPLE_APP_H
#define NV_SAMPLE_APP_H

#include <NvFoundation.h>

#include "NvAppBase.h"
#include "NV/NvStopWatch.h"
#include "NvGLAppContext.h"
#include "NvPlatformContext.h"
#include "NvGamepad/NvGamepad.h"
#include "NvUI/NvUI.h"
#include "NvUI/NvTweakVar.h"
#include <map>

/// \file
/// Sample app base class.

class NvFramerateCounter;
class NvInputTransformer;
class NvSimpleFBO;
class NvTweakBar;

/// Base class for sample apps.
/// Adds numerous features to NvAppBase that are of use to most or all sample apps
class NvSampleApp : public NvAppBase 
{
public:
    /// Constructor
    /// Do NOT make rendering API calls in the constructor
    /// The rendering context is not bound until the entry into initRendering
    /// \param[in] platform the platform context representing the system, normally
    /// passed in from the #NvAppFactory
    /// \param[in] appTitle the null-terminated string title of the application
    NvSampleApp(NvPlatformContext* platform, const char* appTitle=NULL);

    /// Destructor
    virtual ~NvSampleApp();

    /// UI init callback.
    /// Called after rendering is initialized, to allow preparation of overlaid
    /// UI elements
    virtual void initUI(void) { }

    /// App-specific UI drawing callback.
    /// Called to request the app render any UI elements over the frame.
    virtual void drawUI(void) { }

    /// The base class provides an implementation of the mainloop that
    /// calls the virtual "callbacks" .  Leaving this function as implemented in the
    /// App base class allows the application to simply override the individual
    /// callbacks to implement their app behavior.
    /// However, apps can still copy the source of the App::mainLoop into their
    /// own override of the function and either modify it slightly or completely
    /// replace it.
    virtual void mainLoop();

    /// Get UI window.
    /// \return a pointer to the UI window
    NvUIWindow *getUIWindow() { return mUIWindow; }

    /// Get the framerate counter.
    /// The NvSampleApp initializes and updates an NvFramerateCounter in its
    /// mainloop implementation.  It also draws it to the screen.  The application
    /// may gain access if it wishes to get the data for its own use.
    /// \return a pointer to the framerate counter object
    NvFramerateCounter *getFramerate() { return mFramerate; }

    /// Extension requirement declaration.
    /// Allow an app to declare an extension as "required".
    /// \param[in] ext the extension name to be required
    /// \param[in] exitOnFailure if true,
    /// then #errorExit is called to indicate the issue and exit
    /// \return true if the extension string is exported and false
    /// if it is not
    bool requireExtension(const char* ext, bool exitOnFailure = true);

    /// GL Minimum API requirement declaration.
    /// \param[in] minApi the minimum API that is required
    /// \param[in] exitOnFailure if true,
    /// then errorExit is called to indicate the issue and exit
    /// \return true if the platform's GL[ES] API version is at least
    /// as new as the given minApi.  Otherwise, returns false
    bool requireMinAPIVersion(const NvGfxAPIVersion& minApi, bool exitOnFailure = true);

    /// Exit with indication of an error
    /// Exits in all cases; in normal mode it shows the string in a dialog and exits
    /// In test mode, it writes the string to the log file and exits
    /// \param[in] errorString a null-terminated string indicating the error
    void errorExit(const char* errorString);

    /// Frame delta time.
    /// \return the time since the last frame in seconds
    float getFrameDeltaTime() { return mFrameDelta; }

    /// Key binding.
    /// Adds a key binding.
    /// \param[in] var the tweak variable to be bound
    /// \param[in] incKey the key to be bound to increment the tweak variable
    /// \param[in] decKey the key to be bound to decrement the tweak variable
    void addTweakKeyBind(NvTweakVarBase *var, uint32_t incKey, uint32_t decKey=0);

    /// Gamepad Button binding.
    /// Adds a button binding.
    /// \param[in] var the tweak variable to be bound
    /// \param[in] incKey the button to be bound to increment the tweak variable
    /// \param[in] decKey the button to be bound to decrement the tweak variable
    void addTweakButtonBind(NvTweakVarBase *var, uint32_t incBtn, uint32_t decBtn=0);

    /// Window size request.
    /// Allows the app to change the default window size.
    /// While an app can override this, it is NOT recommended, as the base class
    /// parses the command line arguments to set the window size.  Applications
    /// wishing to override this should call the base class version and return
    /// without changing the values if the base class returns true.
    /// Application must return true if it changes the width or height passed in
    /// Not all platforms can support setting the window size.  These platforms
    /// will not call this function
    /// Most apps should be resolution-agnostic and be able to run at a given
    /// resolution
    /// \param[in,out] width the default width is passed in.  If the application wishes
    /// to reuqest it be changed, it should change the value before returning true
    /// \param[in,out] height the default height is passed in.  If the application wishes
    /// to reuqest it be changed, it should change the value before returning true
    /// \return whether the value has been changed.  true if changed, false if not
    virtual bool getRequestedWindowSize(int32_t& width, int32_t& height);

    virtual NvUIEventResponse handleReaction(const NvUIReaction& react) { return nvuiEventNotHandled; }

    /// Request to update any UI related to a given NvTweakVarBase
    /// Allows the framework to abstract the process by which we
    /// call HandleReaction to notify all the UI elements that a
    /// particular variable being tracked has had some kind of update.
    /// \param[in] var the variable that changed
    void syncValue(NvTweakVarBase *var);

    /// Retrieve the main "onscreen" framebuffer; this may actually
    /// be an offscreen FBO that the framework uses internally, and then
    /// resolves to the window.  Apps should ALWAYS use this value when
    /// binding the onscreen FBO and NOT use FBO ID 0 in order to ensure
    /// that they are compatible with test mode, etc.
    /// This should be queried on a per-frame basis.  It may change every frame
    /// \return the GL ID of the main, "onscreen" FBO
    GLuint getMainFBO() const { return mMainFBO; }

protected:
    /// Test mode query.
    /// \return true if the app is running in a timed test harness
    bool isTestMode() { return mTestMode; }

    // Do not override these virtuals - overide the "handle" ones above
    bool pointerInput(NvInputDeviceType::Enum device, NvPointerActionType::Enum action, 
        uint32_t modifiers, int32_t count, NvPointerEvent* points); // we have base impl.
    bool keyInput(uint32_t code, NvKeyActionType::Enum action);
    bool characterInput(uint8_t c);
    bool gamepadChanged(uint32_t changedPadFlags);
    bool gamepadButtonChanged(uint32_t button, bool down);

    NvFramerateCounter *mFramerate;
    float mFrameDelta;
    NvStopWatch* mFrameTimer;

    NvStopWatch* mAutoRepeatTimer;
    uint32_t     mAutoRepeatButton;
    bool         mAutoRepeatTriggered;

    NvUIWindow *mUIWindow;
    NvUIValueText *mFPSText;
    NvTweakBar *mTweakBar;
    NvUIButton *mTweakTab;

    NvInputTransformer* m_transformer;

    typedef std::map<uint32_t, NvTweakBind> NvAppKeyBind;
    NvAppKeyBind mKeyBinds;
    typedef std::map<uint32_t, NvTweakBind> NvAppButtonBind;
    NvAppButtonBind mButtonBinds;

    NvGamepad::State mLastPadState[NvGamepad::MAX_GAMEPADS];

    /// \privatesection
    void baseInitRendering(void);
    void baseShutdownRendering(void);
    void baseInitUI(void);
    void baseReshape(int32_t w, int32_t h);
    void baseUpdate(void);
    void baseDraw(void);
    void baseDrawUI(void);
    void baseFocusChanged(bool focused);
    void baseHandleReaction(void);
    void logTestResults(float frameRate, int32_t frames);

    void SwapBuffers();

private:
    GLuint mMainFBO;
    bool mUseFBOPair;
    int32_t mCurrentFBOIndex;
    NvSimpleFBO* mFBOPair[2];
    int32_t m_fboWidth;
    int32_t m_fboHeight;

    int32_t m_windowWidth;
    int32_t m_windowHeight;

    int32_t m_desiredWidth;
    int32_t m_desiredHeight;
    bool mTestMode;
    float mTestDuration;
    int32_t mTestRepeatFrames;
    std::string mTestName;

    enum {
        TEST_MODE_ISSUE_NONE = 0x00000000,
        TEST_MODE_FBO_ISSUE = 0x00000001,
    };

    uint32_t m_testModeIssues;

    const static int32_t TESTMODE_WARMUP_FRAMES = 10;
};

#endif
