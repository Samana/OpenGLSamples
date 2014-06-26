//----------------------------------------------------------------------------------
// File:        NV/NvPlatformGL.h
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

/// \file
/// Platform-independent OpenGL[ES] header.
/// Includes the correct GL headers depending on platform and Regal settings
/// Supports Android, Linux, MacOS, Win32 and Regal

#ifndef NV_PLATFORM_GL_H
#define NV_PLATFORM_GL_H

#include <NvFoundation.h>


#if defined(_WIN32)

#define GLEW_STATIC
#include <GL/glew.h>
#define GLFW_INCLUDE_ES2
#include <GLFW/glfw3.h>

#elif defined(USE_REGAL)

#include <Regal/GL/Regal.h>

#elif defined(LINUX)

#ifndef GLEW_NO_GLU
#define GLEW_NO_GLU
#endif

#define GLEW_STATIC
#include <GL/glew.h>
#define GLFW_INCLUDE_ES2
#include <GLFW/glfw3.h>

#elif defined(ANDROID)

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#elif defined(__APPLE__)
// apple DEFINES all these all the time and uses 0/1 to differentiate.
#if TARGET_OS_IPHONE

#error "Reached NvPlatformGL.h on an unsupported platform."

#else // assume #elif TARGET_OS_MAC

#define GLEW_STATIC
#include <GL/glew.h>
#define GLFW_INCLUDE_ES2
#include <GLFW/glfw3.h>

#endif // valid mac

#else // not a platform we recognize by #defs

#error "Reached NvPlatformGL.h with no platform #defines we know about.  ERROR."

#endif

#include <NV/NvGfxAPI.h>

class NvGLExtensionsAPI {
public:
    virtual ~NvGLExtensionsAPI() { }

    /// Cross-platform extension function pointer type.
    typedef void (*GLproc)(void);

    /// Cross-platform extension function retrieval.
    /// \return the named extension function if available.  Note that on
    /// some platforms, non-NULL return does NOT indicate support for the
    /// extension.  The only safe way to know if an extension is supported is
    /// via the extension string.
    virtual GLproc getGLProcAddress(const char* procname) = 0;

    /// Extension support query.
    /// \return true if the given string is found in the extension set for the
    /// context, false if not.  Should only be called with a bound context for
    /// safety across all platforms
    virtual bool isExtensionSupported(const char* ext) = 0;
};


#endif
