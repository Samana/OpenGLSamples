//----------------------------------------------------------------------------------
// File:        NvAppBase/NvGLAppContext.h
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

#ifndef NV_GLAPPCONTEXT_H
#define NV_GLAPPCONTEXT_H

#include <NvFoundation.h>
#include <NV/NvPlatformGL.h>

/// \file
/// Cross-platform OpenGL Context APIs and information

/// [E]GL config representation.
struct NvEGLConfiguration {

    /// Inline all-elements constructor.
    /// \param[in] _apiVer the API and version information
    /// \param[in] r the red color depth in bits
    /// \param[in] g the green color depth in bits
    /// \param[in] b the blue color depth in bits
    /// \param[in] a the alpha color depth in bits
    /// \param[in] d the depth buffer depth in bits
    /// \param[in] s the stencil buffer depth in bits
    NvEGLConfiguration(const NvGfxAPIVersion& _apiVer = NvGfxAPIVersionES2(),
        uint32_t r = 8, uint32_t g = 8, 
        uint32_t b = 8, uint32_t a = 8,
        uint32_t d = 24, uint32_t s = 0) : apiVer(_apiVer),
    redBits(r), greenBits(g), blueBits(b), alphaBits(a),
        depthBits(d), stencilBits(s) {}

    NvGfxAPIVersion apiVer; ///< API and version
    uint32_t redBits; ///< red color channel depth in bits
    uint32_t greenBits; ///< green color channel depth in bits
    uint32_t blueBits; ///< blue color channel depth in bits
    uint32_t alphaBits; ///< alpha color channel depth in bits
    uint32_t depthBits; ///< depth buffer depth in bits
    uint32_t stencilBits; ///< stencil buffer depth in bits
};

/// GPU platform category (high-level).
struct NvGLPlatformCategory {
    enum Enum {
        PLAT_MOBILE, ///< Mobile/handheld platform
        PLAT_DESKTOP ///< Desktop/laptop-class platform
    };
};

/// Platform OS.
struct NvGLPlatformOS {
    enum Enum {
        OS_ANDROID, ///< Android
        OS_WINDOWS, ///< Windows
        OS_LINUX, ///< "Desktop" Linux
        OS_MACOSX ///< MacOS
    };
};

/// Basic, combined platform info.
struct NvGLPlatformInfo {
    /// Inline, all-members constructor
    /// \param[in] category the platform category
    /// \param[in] os the platform OS
    NvGLPlatformInfo(NvGLPlatformCategory::Enum category, NvGLPlatformOS::Enum os) :
       mCategory(category)
       , mOS(os)
       { }

    NvGLPlatformCategory::Enum mCategory; ///< Platform GPU category
    NvGLPlatformOS::Enum mOS; ///< Platform OS
};

/// OpenGL[ES] Context wrapper.
class NvGLAppContext : public NvGLExtensionsAPI {
public:
    virtual ~NvGLAppContext() {}

    /// Bind the GL context (and current surface) to the current thread, creating if needed.
    /// \return true on success, false on failure
    virtual bool bindContext() = 0;

    /// Unbind the GL context from the current thread
    /// \return true on success, false on failure
    virtual bool unbindContext() = 0;

    /// Swap the rendering buffers (ie present).
    /// \return true on success, false on failure
    virtual bool swap() = 0;

    /// Set the swap interval if supported by the platform.
    /// \param[in] interval the number of VSYNCs to wait between swaps (0 == never wait)
    /// \return true if the platform can support swap interval, false if not
    virtual bool setSwapInterval(int32_t interval) = 0;

    /// Surface width.
    /// \return the surface width in pixels
    virtual int32_t width() = 0;

    /// Surface height.
    /// \return the surface height in pixels
    virtual int32_t height() = 0;

    /// The selected [E]GL configuration.
    /// \return the selected configuration information for the platform
    const NvEGLConfiguration& getConfiguration() const { return mConfig; }


    /// The platform description.
    /// \return the platform information for the system
    const NvGLPlatformInfo& getPlatformInfo() const { return mPlatformInfo; }

    /// Cross-platform extension function retrieval.
    /// \return the named extension function if available.  Note that on
    /// some platforms, non-NULL return does NOT indicate support for the
    /// extension.  The only safe way to know if an extension is supported is
    /// via the extension string.
    virtual GLproc getGLProcAddress(const char* procname) { return NULL; }

    /// Extension support query.
    /// \return true if the given string is found in the extension set for the
    /// context, false if not.  Should only be called with a bound context for
    /// safety across all platforms
    virtual bool isExtensionSupported(const char* ext) { return false; }
    
    /// Force context reset.
    /// Optional per-platform function to request that the GL context be
    /// shut down and restarted on demand.  Used to test an app's implementation
    /// of the initRendering/shutdownRendering sequence
    /// \return true if the feature is supported and the context has been reset,
    /// false if not supported or could not be completed
    virtual bool requestResetContext() { return false; }

    /// Get platform-specific context.
    /// This function is for use in special circumstances where the WGL, EGL, GLX, etc
    /// context is required by the application.  Most applications should avoid this function.
    /// \return the platform-specific context handle, cast to void* or NULL if not supported
    virtual void* getCurrentPlatformContext() { return NULL; }

    /// Get platform-specific display.
    /// This function is for use in special circumstances where the WGL, EGL, GLX, etc
    /// display is required by the application.  Most applications should avoid this function.
    /// \return the platform-specific display handle, cast to void* or NULL if not supported
    virtual void* getCurrentPlatformDisplay() { return NULL; }

protected:
    /// \privatesection
    NvGLAppContext(NvGLPlatformInfo info) : 
         mPlatformInfo(info) {}
    NvEGLConfiguration mConfig;
    NvGLPlatformInfo mPlatformInfo;
};

#endif
