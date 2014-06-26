//----------------------------------------------------------------------------------
// File:        NV/NvLogs.h
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

#ifndef NV_LOGS_H
#define NV_LOGS_H

#include <NvFoundation.h>

/// \file
/// Cross-platform application logging to file and console.
/// - LOGI(...) printf-style "info" logging
/// - LOGE(...) printf-style "error" logging
/// - CHECK_GL_ERROR() check the current GL error status and log any error

#define  LOG_TAG    "NVSDK"

#ifdef _WIN32

#include <stdio.h>

extern void NVWindowsLog(const char* fmt, ...);

#define LOGI(...) { NVWindowsLog(__VA_ARGS__); }
#define LOGE(...) { NVWindowsLog(__VA_ARGS__); }

#elif ANDROID

#include <stdlib.h> // for exit()
#include <android/log.h>

#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  { __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__); }

#elif defined(LINUX) || defined(MACOSX)

#include <stdlib.h> // for exit()
#include <stdio.h>

#define LOGI(...) { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); }
#define LOGE(...) { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); }

#else

#error "No supported platform specified for NvLogs.h"

#endif

void checkGLError(const char* file, int32_t line);

#ifndef CHECK_GL_ERROR
#define CHECK_GL_ERROR() checkGLError(__FILE__, __LINE__)
#endif
#endif
