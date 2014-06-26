//----------------------------------------------------------------------------------
// File:        NvGLUtils/NvImageGL.cpp
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
#include "NV/NvPlatformGL.h"
#include "NvAssetLoader/NvAssetLoader.h"
#include "NvGLUtils/NvImage.h"

NvImage* NvImage::CreateFromDDSFile(const char* filename) {
    int32_t len;
    char* ddsData = NvAssetLoaderRead(filename, len);

    if (!ddsData)
        return NULL;

    NvImage* image = new NvImage;
    bool result = image->loadImageFromFileData((const uint8_t*)ddsData, len, "dds");

    NvAssetLoaderFree(ddsData);
    if (!result) {
        delete image;
        image = NULL;
    }

    return image;
}

uint32_t NvImage::UploadTextureFromDDSFile(const char* filename) {
    int32_t len;
    char* ddsData = NvAssetLoaderRead(filename, len);

    if (!ddsData)
        return 0;

    GLuint result = NvImage::UploadTextureFromDDSData(ddsData, len);

    NvAssetLoaderFree(ddsData);

    return result;
}

uint32_t NvImage::UploadTextureFromDDSData(const char* ddsData, int32_t length) {
    GLuint texID = 0;
    NvImage* image = new NvImage;

    if (image->loadImageFromFileData((const uint8_t*)ddsData, length, "dds")) {
        texID = NvImage::UploadTexture(image);
    }

    delete image;

    return texID;
}

uint32_t NvImage::UploadTexture(NvImage* image) {
    GLuint texID = 0;

    if (!image) 
        return 0;

    glGenTextures(1, &texID);

    const NvGfxAPIVersion& api = NvImage::getAPIVersion();

    uint32_t internalFormat = (api.api == NvGfxAPI::GLES) ? image->getFormat() : image->getInternalFormat();

    if (image->isCubeMap()) {
        int32_t error = glGetError();
        glBindTexture(GL_TEXTURE_CUBE_MAP, texID);
        error = glGetError();
        for (int32_t f = GL_TEXTURE_CUBE_MAP_POSITIVE_X; f <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z; f++) {
            int32_t w = image->getWidth();
            int32_t h = image->getHeight();
            for (int32_t l = 0; l < image->getMipLevels(); l++) {
                if (image->isCompressed()) {
                    glCompressedTexImage2D( f, l, internalFormat, w, h,
                        0, image->getImageSize(l), image->getLevel(l, f));
                } else {
                    glTexImage2D( f, l, internalFormat, w, h, 0,
                        image->getFormat(), image->getType(), image->getLevel(l, f));
                }
                error = glGetError();
                w >>= 1;
                h >>= 1;
                w = w ? w : 1;
                h = h ? h : 1;
            }
        }
    } else {
        glBindTexture(GL_TEXTURE_2D, texID);

        int32_t w = image->getWidth();
        int32_t h = image->getHeight();
        for (int32_t l = 0; l < image->getMipLevels(); l++) {
            if (image->isCompressed()) {
                glCompressedTexImage2D( GL_TEXTURE_2D, l, internalFormat, w, h,
                    0, image->getImageSize(l), image->getLevel(l));
            } else {
                glTexImage2D( GL_TEXTURE_2D, l, internalFormat, w, h, 0,
                    image->getFormat(), image->getType(), image->getLevel(l));
            }
            w >>= 1;
            h >>= 1;
            w = w ? w : 1;
            h = h ? h : 1;
        }
    }

    return texID;
}

