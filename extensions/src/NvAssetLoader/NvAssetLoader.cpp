//----------------------------------------------------------------------------------
// File:        NvAssetLoader/NvAssetLoader.cpp
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
#include "NvAssetLoader/NvAssetLoader.h"
#include "NV/NvLogs.h"

#include <string>

#ifdef ANDROID

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

static AAssetManager* s_assetManager = NULL;

bool NvAssetLoaderInit(void* platform)
{
    if (!platform)
        return false;

    s_assetManager = (AAssetManager*)platform;

    return true;
}

bool NvAssetLoaderShutdown()
{
    s_assetManager = NULL;
    return true;
}

bool NvAssetLoaderAddSearchPath(const char *)
{
    return true;
}

bool NvAssetLoaderRemoveSearchPath(const char *)
{
    return true;
}

char *NvAssetLoaderRead(const char *filePath, int32_t &length)
{
    char *buff = NULL;

    if (!s_assetManager)
        return NULL;

    AAsset *fileAsset = AAssetManager_open(s_assetManager, filePath, AASSET_MODE_BUFFER);

    if(fileAsset != NULL)
    {
        length = AAsset_getLength(fileAsset);
        buff = new char[length+1];
        int32_t numBytes = AAsset_read(fileAsset, buff, length);
        buff[length] = '\0';

        LOGI("Read asset '%s', %d bytes", filePath, numBytes);
        //LOGI(" %s\n", buff);

        AAsset_close(fileAsset);
    }

    return buff;
}

bool NvAssetLoaderFree(char* asset)
{
    delete[] asset;
    return true;
}

#elif defined(WIN32)

#include <stdio.h>
#include <vector>

static std::vector<std::string> s_searchPath;

bool NvAssetLoaderInit(void*)
{
    return true;
}

bool NvAssetLoaderShutdown()
{
    s_searchPath.clear();
    return true;
}

bool NvAssetLoaderAddSearchPath(const char *path)
{
    std::vector<std::string>::iterator src = s_searchPath.begin();

    while (src != s_searchPath.end()) {
        if (!(*src).compare(path))
            return true;
        src++;
    }

    s_searchPath.push_back(path);
    return true;
}

bool NvAssetLoaderRemoveSearchPath(const char *path)
{
    std::vector<std::string>::iterator src = s_searchPath.begin();

    while (src != s_searchPath.end()) {
        if (!(*src).compare(path)) {
            s_searchPath.erase(src);
            return true;
        }
        src++;
    }
    return true;
}

char *NvAssetLoaderRead(const char *filePath, int32_t &length)
{
    FILE *fp = NULL;
    // loop N times up the hierarchy, testing at each level
    std::string upPath;
    std::string fullPath;
    for (int32_t i = 0; i < 10; i++) {
        std::vector<std::string>::iterator src = s_searchPath.begin();
        bool looping = true;
        while(looping) {
            fullPath.assign(upPath);  // reset to current upPath.
            if (src != s_searchPath.end()) {
                //sprintf_s(fullPath, "%s%s/assets/%s", upPath, *src, filePath);
                fullPath.append(*src);
                fullPath.append("/assets/");
                src++;
            } else {
                //sprintf_s(fullPath, "%sassets/%s", upPath, filePath);
                fullPath.append("assets/");
                looping = false;
            }
            fullPath.append(filePath);

#ifdef DEBUG
            fprintf(stderr, "Trying to open %s\n", fullPath.c_str());
#endif
            if ((fopen_s(&fp, fullPath.c_str(), "rb") != 0) || (fp == NULL))
                fp = NULL;
            else
                looping = false;
        }

        if (fp)
            break;

        upPath.append("../");
    }

    if (!fp) {
        fprintf(stderr, "Error opening file '%s'\n", filePath);
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    length = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *data = new char [length + 1];
    fread(data, 1, length, fp);
    data[length] = '\0';

    fclose(fp);

#ifdef DEBUG
    fprintf(stderr, "Read file '%s', %d bytes\n", filePath, length);
#endif
    return data;
}

bool NvAssetLoaderFree(char* asset)
{
    delete[] asset;
    return true;
}

#elif defined(LINUX) || defined(MACOSX) // have mac and linux share ftm.

#include <stdio.h>
#include <vector>

static std::vector<std::string> s_searchPath;

bool NvAssetLoaderInit(void*)
{
    return true;
}

bool NvAssetLoaderShutdown()
{
    s_searchPath.clear();
    return true;
}

bool NvAssetLoaderAddSearchPath(const char *path)
{
    std::vector<std::string>::iterator src = s_searchPath.begin();

    while (src != s_searchPath.end()) {
        if (!(*src).compare(path))
            return true;
        src++;
    }

    s_searchPath.push_back(path);
    return true;
}

bool NvAssetLoaderRemoveSearchPath(const char *path)
{
    std::vector<std::string>::iterator src = s_searchPath.begin();

    while (src != s_searchPath.end()) {
        if (!(*src).compare(path)) {
            s_searchPath.erase(src);
            return true;
        }
        src++;
    }
    return true;
}

char *NvAssetLoaderRead(const char *filePath, int32_t &length)
{
    FILE *fp = NULL;
    // loop N times up the hierarchy, testing at each level
    std::string upPath;
    std::string fullPath;
    for (int32_t i = 0; i < 10; i++) {
        std::vector<std::string>::iterator src = s_searchPath.begin();
        bool looping = true;
        while(looping) {
            fullPath.assign(upPath);  // reset to current upPath.
            if (src != s_searchPath.end()) {
                //sprintf_s(fullPath, "%s%s/assets/%s", upPath, *src, filePath);
                fullPath.append(*src);
                fullPath.append("/assets/");
                src++;
            } else {
                //sprintf_s(fullPath, "%sassets/%s", upPath, filePath);
                fullPath.append("assets/");
                looping = false;
            }
            fullPath.append(filePath);

#ifdef DEBUG
            fprintf(stderr, "Trying to open %s\n", fullPath.c_str());
#endif
            fp = fopen(fullPath.c_str(), "rb");
            if (fp != NULL)
                looping = false;
        }

        if (fp)
            break;

        upPath.append("../");
    }

    if (!fp) {
        fprintf(stderr, "Error opening file '%s'\n", filePath);
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    length = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *data = new char [length + 1];
    int32_t size = fread(data, 1, length, fp);
    data[length] = '\0';

    fclose(fp);

#ifdef DEBUG
    fprintf(stderr, "Read file '%s', %d bytes\n", filePath, length);
#endif
    return data;
}

bool NvAssetLoaderFree(char* asset)
{
    delete[] asset;
    return true;
}

#else

#error "No asset loader library defined for this platform!"

#endif

