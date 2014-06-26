//----------------------------------------------------------------------------------
// File:        NvAppBase/MainWin32.cpp
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
#ifdef WIN32

#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <map>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/wglew.h>
#define GLFW_INCLUDE_ES2
#include <GLFW/glfw3.h>

#include "NvAppBase/NvAppBase.h"
#include "NV/NvStopWatch.h"
#include "NvGamepad/NvGamepadXInput.h"
#include "NvAssetLoader/NvAssetLoader.h"

#include "NV/NvTokenizer.h"
#include "NV/NvLogs.h"

void NVWindowsLog(const char* fmt, ...) {
 
    const int length = 1024;
    char buffer[length];
    va_list ap;
  
    va_start(ap, fmt); 
    vsnprintf_s(buffer, length-1, fmt, ap);
    OutputDebugString(buffer);
    OutputDebugString("\n");
    va_end(ap);
}

class NvWin32StopWatch: public NvStopWatch 
{
public:
    //! Constructor, default
    NvWin32StopWatch() : 
            start_time(),   end_time(),
            diff_time(0.0), freq_set(false)
    {
        if( ! freq_set) {
            // helper variable
            LARGE_INTEGER temp;

            // get the tick frequency from the OS
            QueryPerformanceFrequency((LARGE_INTEGER*) &temp);

            // convert to type in which it is needed
            freq = ((double) temp.QuadPart);

            // rememeber query
            freq_set = true;
        }
    };

    // Destructor
    ~NvWin32StopWatch()
    { };

    //! Start time measurement
    void start() {
        QueryPerformanceCounter((LARGE_INTEGER*) &start_time);
        m_running = true;
    }

    //! Stop time measurement
    void stop() {
        QueryPerformanceCounter((LARGE_INTEGER*) &end_time);
        diff_time = (float) 
            (((double) end_time.QuadPart - (double) start_time.QuadPart) / freq);
        m_running = false;
    }

    //! Reset time counters to zero
    void reset()
    {
        diff_time = 0;
        if( m_running )
            QueryPerformanceCounter((LARGE_INTEGER*) &start_time);
    }

    const float getTime() const {
        if(m_running) {
            return getDiffTime();
        } else {
            // time difference in seconds
            return diff_time;
        }
    }

private:

    // helper functions
      
    //! Get difference between start time and current time
    float getDiffTime() const {
        LARGE_INTEGER temp;
        QueryPerformanceCounter((LARGE_INTEGER*) &temp);
        return (float) 
            (((double) temp.QuadPart - (double) start_time.QuadPart) / freq);
    }

    // member variables

    //! Start of measurement
    LARGE_INTEGER  start_time;
    //! End of measurement
    LARGE_INTEGER  end_time;

    //! Time difference between the last start and stop
    float  diff_time;

    //! tick frequency
    double  freq;

    //! flag if the frequency has been set
    bool  freq_set;
};


static NvAppBase *sApp = NULL;

// this needs to be global so inputcallbacksglfw can access...
NvInputCallbacks* sCallbacks = NULL;
extern void setInputCallbacksGLFW(GLFWwindow *window);

static bool sWindowIsFocused = true;
static bool sHasResized = true;
static int32_t sForcedRenderCount = 0;

class NvGLWin32AppContext: public NvGLAppContext {
public:
    NvGLWin32AppContext(NvEGLConfiguration& config) :
        NvGLAppContext(NvGLPlatformInfo(
            NvGLPlatformCategory::PLAT_DESKTOP, 
            NvGLPlatformOS::OS_LINUX))
    {
        mWindow = NULL;
        // Hack - we can't query most of this back from GLFW, so we assume it all "took"
        mConfig = config;
        glfwWindowHint(GLFW_RED_BITS, config.redBits);
        glfwWindowHint(GLFW_GREEN_BITS, config.greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, config.blueBits);
        glfwWindowHint(GLFW_ALPHA_BITS, config.alphaBits);
        glfwWindowHint(GLFW_DEPTH_BITS, config.depthBits);
        glfwWindowHint(GLFW_STENCIL_BITS, config.stencilBits);
    }

    void setWindow(GLFWwindow* window) {
        mWindow = window;
    }

    bool bindContext() {
        glfwMakeContextCurrent(mWindow);
        return true;
    }

    bool unbindContext() {
        glfwMakeContextCurrent(NULL);
        return true;
    }

    bool swap() {
        glfwSwapBuffers(mWindow);
        return true;
    }

    bool setSwapInterval(int32_t interval) {
        glfwSwapInterval( interval );
        return false;
    }

    int32_t width() {
        int32_t w, h;
        glfwGetFramebufferSize(mWindow, &w, &h);
        return w;
    }

    int32_t height() {
        int32_t w, h;
        glfwGetFramebufferSize(mWindow, &w, &h);
        return h;
    }

    GLproc getGLProcAddress(const char* procname) {
        return glfwGetProcAddress(procname);
    }

    bool isExtensionSupported(const char* ext) {
        return glfwExtensionSupported(ext) ? true : false;
    }

    void setConfiguration(const NvEGLConfiguration& config) { mConfig = config; }

    virtual void* getCurrentPlatformContext() { 
        return (void*)wglGetCurrentContext(); 
    }

    virtual void* getCurrentPlatformDisplay() { 
        return (void*)wglGetCurrentDC(); 
    }

protected:
    GLFWwindow* mWindow;
};

class NvWin32PlatformContext : public NvPlatformContext {
public:
    NvWin32PlatformContext() : mWindow(NULL), mGamepad(new NvGamepadXInput) { }
    ~NvWin32PlatformContext() { delete mGamepad; }

    void setWindow(GLFWwindow* window) { mWindow = window; }

    virtual bool isAppRunning();
    virtual void requestExit() { glfwSetWindowShouldClose(mWindow, 1); }
    virtual bool pollEvents(NvInputCallbacks* callbacks);
    virtual bool isContextLost() { return false; }
    virtual bool isContextBound() { return glfwGetCurrentContext() != NULL; }
    virtual bool shouldRender();
    virtual bool hasWindowResized();
    virtual NvGamepad* getGamepad() { return mGamepad; }
    virtual void setAppTitle(const char* title) { if (mWindow) glfwSetWindowTitle(mWindow, title); }
    virtual const std::vector<std::string>& getCommandLine() { return m_commandLine; }

    std::vector<std::string> m_commandLine;
protected:
    GLFWwindow* mWindow;
    NvGamepadXInput* mGamepad;
};


bool NvWin32PlatformContext::isAppRunning() {
    return !glfwWindowShouldClose(mWindow);
}

bool NvWin32PlatformContext::pollEvents(NvInputCallbacks* callbacks) {
    sCallbacks = callbacks;
    glfwPollEvents();

    uint32_t changed = mGamepad->pollGamepads();
    if (callbacks && changed)
        callbacks->gamepadChanged(changed);

    sCallbacks = NULL;
    
    return true;
}

bool NvWin32PlatformContext::shouldRender() {
    // For now, do not stop rendering when focus is lost; this is screwing up test mode
    /*if (sWindowIsFocused || (sForcedRenderCount > 0))*/ {
        if (sForcedRenderCount > 0)
            sForcedRenderCount--;

        return true;
    }
    //return false;
}

bool NvWin32PlatformContext::hasWindowResized() {
    if (sHasResized) {
        sHasResized = false;
        return true;
    }
    return false;
}

// window size callback
static void reshape( GLFWwindow* window, int32_t width, int32_t height )
{
    //GLfloat aspect = (GLfloat) height / (GLfloat) width;
    //glViewport( 0, 0, (GLint) width, (GLint) height );
    sHasResized = true;
    sForcedRenderCount += 2;
}

static void focus(GLFWwindow*,int32_t focused)
{
    sWindowIsFocused = focused != 0;
    sApp->focusChanged(sWindowIsFocused);
    sForcedRenderCount += 2;
}


// program initialization
static void initGL()
{
    //glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
}


// program entry
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    GLFWwindow* window;

    NvAssetLoaderInit(NULL);

    sWindowIsFocused = true;
    sForcedRenderCount = 0;

    if( !glfwInit() )
    { // !!!!TBD TODO would be nice if showDialog could do these bits.
        LOGE( "Failed to initialize GLFW\n" );
        exit( EXIT_FAILURE );
    }

    NvWin32PlatformContext* platform = new NvWin32PlatformContext;

    // add command line arguments
    {
        NvTokenizer cmdtok(lpCmdLine);
        std::string sarg;
        while (cmdtok.getTokenString(sarg))
            platform->m_commandLine.push_back(sarg);
    }

    sApp = NvAppFactory(platform);

    NvEGLConfiguration config(NvGfxAPIVersionES2(), 8, 8, 8, 8, 16, 0);
    sApp->configurationCallback(config);

    // Does not seem to work...
/*
    if (config.api == GLAppContext::Configuration::API_ES)
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, config.majVer);
    //glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    */

    NvGLWin32AppContext* context = new NvGLWin32AppContext(config);

    // Check for a requested size
    const int32_t defaultWidth = 1280;
    const int32_t defaultHeight = 720;
    int32_t width = defaultWidth;
    int32_t height = defaultHeight;
    if (!sApp->getRequestedWindowSize(width, height)) {
        // for safety, reset the height and width if the app returns false in case
        // they did a bad thing and changed the values but returned false
        width = defaultWidth;
        height = defaultHeight;
    }

    window = glfwCreateWindow( width, height, "Windows SDK Application", NULL, NULL );
    if (!window)
    {
        LOGE( "Failed to open GLFW window\n" );
        glfwTerminate();
        exit( EXIT_FAILURE );
    }

    platform->setWindow(window);

    context->setWindow(window);
    sApp->setGLContext(context);

    // Set callback functions
    glfwSetFramebufferSizeCallback(window, reshape);
    glfwSetWindowFocusCallback(window, focus);
    setInputCallbacksGLFW(window);

    context->bindContext();
    glfwSwapInterval( 1 );

    glfwGetFramebufferSize(window, &width, &height);

    int32_t major = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR);
    int32_t minor = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR);
    config.apiVer = NvGfxAPIVersion(NvGfxAPI::GL, major, minor);

    glGetIntegerv(GL_RED_BITS, (GLint*)&config.redBits);
    glGetIntegerv(GL_GREEN_BITS, (GLint*)&config.greenBits);
    glGetIntegerv(GL_BLUE_BITS, (GLint*)&config.blueBits);
    glGetIntegerv(GL_ALPHA_BITS, (GLint*)&config.alphaBits);
    glGetIntegerv(GL_DEPTH_BITS, (GLint*)&config.depthBits);
    glGetIntegerv(GL_STENCIL_BITS, (GLint*)&config.stencilBits);

    context->setConfiguration(config);

    // initialize extensions framework
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
      // Problem: glewInit failed, something is seriously wrong.
      // TODO: would be nice to use dialog here too....
      LOGE("Error: %s\n", glewGetErrorString(err));
      exit(-1);
    }
    LOGI("Using GLEW %s\n", glewGetString(GLEW_VERSION));

    // Parse command-line options
    initGL();

    reshape(window, width, height);

    sApp->mainLoop();

    // Shut down the app before shutting down GL
    delete sApp;

    // Terminate GLFW
    glfwTerminate();

    NvAssetLoaderShutdown();

    // Exit program
    exit( EXIT_SUCCESS );
}

// Timer and timing functions
NvStopWatch* NvAppBase::createStopWatch() {
    return new NvWin32StopWatch;
}

uint64_t NvAppBase::queryPerformanceCounter() {
    uint64_t temp;
    QueryPerformanceCounter((LARGE_INTEGER*) &temp);
    return temp;
}

uint64_t NvAppBase::queryPerformanceCounterFrequency() {
    uint64_t temp;
    QueryPerformanceFrequency((LARGE_INTEGER*) &temp);
    return temp;
}

bool NvAppBase::showDialog(const char* title, const char * content, bool exitApp) {
    MessageBox(NULL, content, title, MB_OK);
    getPlatformContext()->requestExit();
    return true;
}

bool NvAppBase::writeScreenShot(int32_t width, int32_t height, const uint8_t* data, const std::string& path) {

    std::string filename = path + ".bmp";
    FILE* fp = NULL;
    errno_t errnum = fopen_s(&fp, filename.c_str(), "wb");
    if (!fp || errnum)
        return false;

    uint8_t* dest = new uint8_t[width * height * 4];

    uint8_t* tmpDest = dest;

    int32_t pixels = width * height;

    for (int i = 0; i < pixels; i++) {
        *(tmpDest++) = data[2];
        *(tmpDest++) = data[1];
        *(tmpDest++) = data[0];
        data += 4;
    }

    int8_t header = 'B';
    fwrite(&header, sizeof(header), 1, fp);

    header = 'M';
    fwrite(&header, sizeof(header), 1, fp);

    uint32_t filesize = 14 + 40 + width * height *3;
    fwrite(&filesize, sizeof(filesize), 1, fp);

    uint16_t reserved1 = 0;
    fwrite(&reserved1, sizeof(reserved1), 1, fp);

    uint16_t reserved2 = 0;
    fwrite(&reserved2, sizeof(reserved2), 1, fp);

    uint32_t offset = 14 + 40;
    fwrite(&offset, sizeof(offset), 1, fp);

    uint32_t size = 40;
    fwrite(&size, sizeof(size), 1, fp);

    int32_t w = width;
    fwrite(&w, sizeof(w), 1, fp);

    int32_t h = height;
    fwrite(&h, sizeof(h), 1, fp);

    uint16_t planes = 1;
    fwrite(&planes, sizeof(planes), 1, fp);

    uint16_t bits = 24;
    fwrite(&bits, sizeof(bits), 1, fp);

    uint32_t compression = 0;
    fwrite(&compression, sizeof(compression), 1, fp);

    uint32_t imagesize = w * h * 3;
    fwrite(&imagesize, sizeof(imagesize), 1, fp);

    int32_t xresolution = 0;
    fwrite(&xresolution, sizeof(xresolution), 1, fp);

    int32_t yresolution = 0;
    fwrite(&yresolution, sizeof(yresolution), 1, fp);

    uint32_t ncolours  = 0;
    fwrite(&ncolours, sizeof(ncolours), 1, fp);

    uint32_t importantcolours = 0;
    fwrite(&importantcolours, sizeof(importantcolours), 1, fp);

    fwrite(dest, w * h * 3, 1, fp);

    delete[] dest;

    fclose(fp);

    return true;
}

bool NvAppBase::writeLogFile(const std::string& path, bool append, const char* fmt, ...) {
    va_list ap;
  
    std::string filename = path + ".txt";
    FILE* fp = NULL;
    errno_t errnum = fopen_s(&fp, filename.c_str(), append ? "a" : "w");
    if (!fp || errnum)
        return false;

    va_start(ap, fmt); 
    vfprintf_s(fp, fmt, ap);
    fprintf_s(fp, "\n");
    va_end(ap);

    fclose(fp);
    return true;
}


void NvAppBase::forceLinkHack() {
}

#endif // WIN32
