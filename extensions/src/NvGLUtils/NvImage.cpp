//----------------------------------------------------------------------------------
// File:        NvGLUtils/NvImage.cpp
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

#include <string.h>
#include <algorithm>

#include "NvGLUtils/NvImage.h"
#include "BlockDXT.h"

#include "NvGLEnums.h"

using std::vector;
using std::max;

#ifdef WIN32
#define strcasecmp _stricmp
#endif

NvImage::FormatInfo NvImage::formatTable[] = {
    { "dds", NvImage::readDDS, 0}
};

bool NvImage::upperLeftOrigin = true;
NvGfxAPIVersion NvImage::m_gfxAPIVersion = NvGfxAPIVersionGL4_3();
bool NvImage::m_expandDXT = true;

//
//
////////////////////////////////////////////////////////////
void NvImage::UpperLeftOrigin( bool ul) {
    upperLeftOrigin = ul;
}

//
//
////////////////////////////////////////////////////////////
NvImage::NvImage() : _width(0), _height(0), _depth(0), _levelCount(0), _layers(0), _format(GL_RGBA),
    _internalFormat(GL_RGBA8), _type(GL_UNSIGNED_BYTE), _elementSize(0), _cubeMap(false) {
}

//
//
////////////////////////////////////////////////////////////
NvImage::~NvImage() {
    freeData();
}

//
//
////////////////////////////////////////////////////////////
void NvImage::freeData() {
    for (vector<uint8_t*>::iterator it = _data.begin(); it != _data.end(); it++) {
        delete []*it;
    }
    _data.clear();
}

//
//
////////////////////////////////////////////////////////////
int32_t NvImage::getImageSize( int32_t level) const {
    bool compressed = isCompressed();
    int32_t w = _width >> level;
    int32_t h = _height >> level;
    int32_t d = _depth >> level;
    w = (w) ? w : 1;
    h = (h) ? h : 1;
    d = (d) ? d : 1;
    int32_t bw = (compressed) ? ( w + 3 ) / 4 : w;
    int32_t bh = (compressed) ? ( h + 3 ) / 4 : h;
    int32_t elementSize = _elementSize;

    return bw*bh*d*elementSize;
}


//
//
////////////////////////////////////////////////////////////
const void* NvImage::getLevel( int32_t level) const {
    return getLevel(level, GL_TEXTURE_CUBE_MAP_POSITIVE_X);
}

//
//
////////////////////////////////////////////////////////////
const void* NvImage::getLevel( int32_t level, uint32_t face) const {
    assert( level < _levelCount);
    assert( face >= GL_TEXTURE_CUBE_MAP_POSITIVE_X && face <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
    assert( face == GL_TEXTURE_CUBE_MAP_POSITIVE_X || _cubeMap);

    face = face - GL_TEXTURE_CUBE_MAP_POSITIVE_X;

    assert( (face*_levelCount + level) < (int32_t)_data.size());

    // make sure we don't hand back a garbage pointer
    if (level >=_levelCount || (int32_t)face >= _layers)
        return NULL;

    return _data[ face*_levelCount + level];
}

//
//
////////////////////////////////////////////////////////////
void* NvImage::getLevel( int32_t level) {
    return getLevel(level, GL_TEXTURE_CUBE_MAP_POSITIVE_X);
}

//
//
////////////////////////////////////////////////////////////
void* NvImage::getLevel( int32_t level, uint32_t face) {
    assert( level < _levelCount);
    assert( face >= GL_TEXTURE_CUBE_MAP_POSITIVE_X && face <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
    assert( face == GL_TEXTURE_CUBE_MAP_POSITIVE_X || _cubeMap);

    face = face - GL_TEXTURE_CUBE_MAP_POSITIVE_X;

    assert( (face*_levelCount + level) < (int32_t)_data.size());

    // make sure we don't hand back a garbage pointer
    if (level >=_levelCount || (int32_t)face >= _layers)
        return NULL;

    return _data[ face*_levelCount + level];
}

//
//
////////////////////////////////////////////////////////////
const void* NvImage::getLayerLevel( int32_t level, int32_t layer) const {
    assert( level < _levelCount);
    assert( layer < _layers);


    assert( (layer*_levelCount + level) < (int32_t)_data.size());

    // make sure we don't hand back a garbage pointer
    if (level >=_levelCount || layer >= _layers)
        return NULL;

    return _data[ layer*_levelCount + level];
}

//
//
////////////////////////////////////////////////////////////
void* NvImage::getLayerLevel( int32_t level, int32_t layer) {
    assert( level < _levelCount);
    assert( layer < _layers);


    assert( (layer*_levelCount + level) < (int32_t)_data.size());

    // make sure we don't hand back a garbage pointer
    if (level >=_levelCount || layer >= _layers)
        return NULL;

    return _data[ layer*_levelCount + level];
}

//
//
////////////////////////////////////////////////////////////
bool NvImage::loadImageFromFileData(const uint8_t* fileData, size_t size, const char* fileExt) {
    int32_t formatCount = sizeof(NvImage::formatTable) / sizeof(NvImage::FormatInfo);

    //try to match by format first
    for ( int32_t ii = 0; ii < formatCount; ii++) {
        if ( ! strcasecmp( formatTable[ii].extension, fileExt)) {
            //extension matches, load it
            return formatTable[ii].reader( fileData, size, *this);
        }
    }


    return false;
}

//
//
////////////////////////////////////////////////////////////
void NvImage::flipSurface(uint8_t *surf, int32_t width, int32_t height, int32_t depth)
{
    uint32_t lineSize;

    depth = (depth) ? depth : 1;

    if (!isCompressed()) {
        lineSize = _elementSize * width;
        uint32_t sliceSize = lineSize * height;

        uint8_t *tempBuf = new uint8_t[lineSize];

        for ( int32_t ii = 0; ii < depth; ii++) {
            uint8_t *top = surf + ii*sliceSize;
            uint8_t *bottom = top + (sliceSize - lineSize);
    
            for ( int32_t jj = 0; jj < (height >> 1); jj++) {
                memcpy( tempBuf, top, lineSize);
                memcpy( top, bottom, lineSize);
                memcpy( bottom, tempBuf, lineSize);

                top += lineSize;
                bottom -= lineSize;
            }
        }

        delete []tempBuf;
    }
    else
    {
        void (*flipblocks)(uint8_t*, uint32_t);
        width = (width + 3) / 4;
        height = (height + 3) / 4;
        uint32_t blockSize = 0;

        switch (_format)
        {
            case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT: 
                blockSize = 8;
                flipblocks = &NvImage::flip_blocks_dxtc1; 
                break;
            case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT: 
                blockSize = 16;
                flipblocks = &NvImage::flip_blocks_dxtc3; 
                break;
            case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT: 
                blockSize = 16;
                flipblocks = &NvImage::flip_blocks_dxtc5; 
                break;
            case GL_COMPRESSED_LUMINANCE_LATC1_EXT:
            case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:
            case GL_COMPRESSED_RED_RGTC1:
            case GL_COMPRESSED_SIGNED_RED_RGTC1:
                blockSize = 8;
                flipblocks = &NvImage::flip_blocks_bc4;
                break;
            case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:
            case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:
            case GL_COMPRESSED_RG_RGTC2:
            case GL_COMPRESSED_SIGNED_RG_RGTC2:
                blockSize = 16;
                flipblocks = &NvImage::flip_blocks_bc5;
                break;
            default:
                return;
        }

        lineSize = width * blockSize;
        uint8_t *tempBuf = new uint8_t[lineSize];

        uint8_t *top = surf;
        uint8_t *bottom = surf + (height-1) * lineSize;

        for (uint32_t j = 0; j < std::max( (uint32_t)height >> 1, (uint32_t)1); j++)
        {
            if (top == bottom)
            {
                flipblocks(top, width);
                break;
            }

            flipblocks(top, width);
            flipblocks(bottom, width);

            memcpy( tempBuf, top, lineSize);
            memcpy( top, bottom, lineSize);
            memcpy( bottom, tempBuf, lineSize);

            top += lineSize;
            bottom -= lineSize;
        }
        delete []tempBuf;
    }
}    

//
//
////////////////////////////////////////////////////////////
void NvImage::componentSwapSurface(uint8_t *surf, int32_t width, int32_t height, int32_t depth)
{
    depth = (depth) ? depth : 1;

    if (_type != GL_UNSIGNED_BYTE)
        return;
    if (isCompressed())
        return;

    if (_format == GL_BGR) {
        for ( int32_t ii = 0; ii < depth; ii++) {    
            for ( int32_t jj = 0; jj < height; jj++) {
                for ( int32_t kk = 0; kk < width; kk++) {
                    uint8_t tmp = surf[0];
                    surf[0] = surf[2];
                    surf[2] = tmp;
                    surf += 3;
                }
            }
        }
        _format = GL_RGB;
    } else if (_format == GL_BGRA) {
        for ( int32_t ii = 0; ii < depth; ii++) {    
            for ( int32_t jj = 0; jj < height; jj++) {
                for ( int32_t kk = 0; kk < width; kk++) {
                    uint8_t tmp = surf[0];
                    surf[0] = surf[2];
                    surf[2] = tmp;
                    surf += 4;
                }
            }
        }
        _format = GL_RGBA;
    }

}    

static inline int32_t min(int32_t a, int32_t b) {
    return (a <= b) ? a : b;
}

//
//
////////////////////////////////////////////////////////////
uint8_t* NvImage::expandDXT(uint8_t *surf, int32_t width, int32_t height, int32_t depth)
{
    depth = (depth) ? depth : 1;

    uint32_t* dest = new uint32_t[width * height * depth];
    uint32_t* plane = dest;

    int32_t bh = (height + 3) / 4;
    int32_t bw = (width + 3) / 4;

    if (_format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) {
        for (int32_t k = 0; k < depth; k++) {

            for (int32_t j = 0; j < bh; j++) {
                int32_t yBlockSize = min(4, height - 4 * j);

                for (int32_t i = 0; i < bw; i++) {
                    int32_t xBlockSize = min(4, width - 4 * i);
                    nv::BlockDXT1* block = (nv::BlockDXT1*)surf;
                    nv::ColorBlock color;

                    block->decodeBlock(&color);

                    // Write color block.
                    for (int32_t y = 0; y < yBlockSize; y++) {
                        for (int32_t x = 0; x < xBlockSize; x++) {
                            plane[4*i+x + (4*j+y)*width] = (uint32_t)color.color(x, y);
                        }
                    }

                    surf += sizeof(nv::BlockDXT1); // 64bits
                }
            }

            plane += width * height;
        }
    } else if (_format == GL_COMPRESSED_RGBA_S3TC_DXT3_EXT) {
        for (int32_t k = 0; k < depth; k++) {

            for (int32_t j = 0; j < bh; j++) {
                int32_t yBlockSize = min(4, height - 4 * j);

                for (int32_t i = 0; i < bw; i++) {
                    int32_t xBlockSize = min(4, width - 4 * i);
                    nv::BlockDXT3* block = (nv::BlockDXT3*)surf;
                    nv::ColorBlock color;

                    block->decodeBlock(&color);

                    // Write color block.
                    for (int32_t y = 0; y < yBlockSize; y++) {
                        for (int32_t x = 0; x < xBlockSize; x++) {
                            plane[4*i+x + (4*j+y)*width] = (uint32_t)color.color(x, y);
                        }
                    }

                    surf += sizeof(nv::BlockDXT3); // 64bits
                }
            }

            plane += width * height;
        }
    } else {
        for (int32_t k = 0; k < depth; k++) {

            for (int32_t j = 0; j < bh; j++) {
                int32_t yBlockSize = min(4U, height - 4 * j);

                for (int32_t i = 0; i < bw; i++) {
                    int32_t xBlockSize = min(4, width - 4 * i);
                    nv::BlockDXT5* block = (nv::BlockDXT5*)surf;
                    nv::ColorBlock color;

                    block->decodeBlock(&color);

                    // Write color block.
                    for (int32_t y = 0; y < yBlockSize; y++) {
                        for (int32_t x = 0; x < xBlockSize; x++) {
                            plane[4*i+x + (4*j+y)*width] = (uint32_t)color.color(x, y);
                        }
                    }

                    surf += sizeof(nv::BlockDXT5); // 64bits
                }
            }

            plane += width * height;
        }
    }

    return (uint8_t*)dest;
}

//
//
////////////////////////////////////////////////////////////
bool NvImage::convertCrossToCubemap() {
    //can't already be a cubemap
    if (isCubeMap())
        return false;

    //mipmaps are not supported
    if (_levelCount != 1)
        return false;

    //compressed textures are not supported
    if (isCompressed())
        return false;

    //this function only supports vertical cross format for now (3 wide by 4 high)
    if (  (_width / 3 != _height / 4) || (_width % 3 != 0) || (_height % 4 != 0) || (_depth != 0))
        return false;

    //get the source data
    uint8_t *data = _data[0];

    int32_t fWidth = _width / 3;
    int32_t fHeight = _height / 4;

    //remove the old pointer from the vector
    _data.pop_back();
    
    uint8_t *face = new uint8_t[ fWidth * fHeight * _elementSize];
    uint8_t *ptr;

    //extract the faces

    // positive X
    ptr = face;
    for (int32_t j=0; j<fHeight; j++) {
        memcpy( ptr, &data[((_height - (fHeight + j + 1))*_width + 2 * fWidth) * _elementSize], fWidth*_elementSize);
        ptr += fWidth*_elementSize;
    }
    _data.push_back(face);

    // negative X
    face = new uint8_t[ fWidth * fHeight * _elementSize];
    ptr = face;
    for (int32_t j=0; j<fHeight; j++) {
        memcpy( ptr, &data[(_height - (fHeight + j + 1))*_width*_elementSize], fWidth*_elementSize);
        ptr += fWidth*_elementSize;
    }
    _data.push_back(face);

    // positive Y
    face = new uint8_t[ fWidth * fHeight * _elementSize];
    ptr = face;
    for (int32_t j=0; j<fHeight; j++) {
        memcpy( ptr, &data[((4 * fHeight - j - 1)*_width + fWidth)*_elementSize], fWidth*_elementSize);
        ptr += fWidth*_elementSize;
    }
    _data.push_back(face);

    // negative Y
    face = new uint8_t[ fWidth * fHeight * _elementSize];
    ptr = face;
    for (int32_t j=0; j<fHeight; j++) {
        memcpy( ptr, &data[((2*fHeight - j - 1)*_width + fWidth)*_elementSize], fWidth*_elementSize);
        ptr += fWidth*_elementSize;
    }
    _data.push_back(face);

    // positive Z
    face = new uint8_t[ fWidth * fHeight * _elementSize];
    ptr = face;
    for (int32_t j=0; j<fHeight; j++) {
        memcpy( ptr, &data[((_height - (fHeight + j + 1))*_width + fWidth) * _elementSize], fWidth*_elementSize);
        ptr += fWidth*_elementSize;
    }
    _data.push_back(face);

    // negative Z
    face = new uint8_t[ fWidth * fHeight * _elementSize];
    ptr = face;
    for (int32_t j=0; j<fHeight; j++) {
        for (int32_t i=0; i<fWidth; i++) {
            memcpy( ptr, &data[(j*_width + 2 * fWidth - (i + 1))*_elementSize], _elementSize);
            ptr += _elementSize;
        }
    }
    _data.push_back(face);

    //set the new # of faces, width and height
    _layers = 6;
    _width = fWidth;
    _height = fHeight;
    _cubeMap = true;

    //delete the old pointer
    delete []data;

    return true;
}

//
//
////////////////////////////////////////////////////////////
bool NvImage::setImage( int32_t width, int32_t height, uint32_t format, uint32_t type, const void* data){
    //check parameters before destroying the old image
    int32_t elementSize;
    uint32_t internalFormat;

    switch (format) {
        case GL_ALPHA:
            switch (type) {
                case GL_UNSIGNED_BYTE:
                    internalFormat = GL_ALPHA8;
                    elementSize = 1;
                    break;
                case GL_UNSIGNED_SHORT:
                    internalFormat = GL_ALPHA16;
                    elementSize = 2;
                    break;
                case GL_FLOAT:
                    internalFormat = GL_ALPHA32F_ARB;
                    elementSize = 4;
                    break;
                case GL_HALF_FLOAT_ARB:
                    internalFormat = GL_ALPHA16F_ARB;
                    elementSize = 2;
                    break;
                default:
                    return false; //format/type combo not supported
            }
            break;
        case GL_LUMINANCE:
            switch (type) {
                case GL_UNSIGNED_BYTE:
                    internalFormat = GL_LUMINANCE8;
                    elementSize = 1;
                    break;
                case GL_UNSIGNED_SHORT:
                    internalFormat = GL_LUMINANCE16;
                    elementSize = 2;
                    break;
                case GL_FLOAT:
                    internalFormat = GL_LUMINANCE32F_ARB;
                    elementSize = 4;
                    break;
                case GL_HALF_FLOAT_ARB:
                    internalFormat = GL_LUMINANCE16F_ARB;
                    elementSize = 2;
                    break;
                default:
                    return false; //format/type combo not supported
            }
            break;
        case GL_LUMINANCE_ALPHA:
            switch (type) {
                case GL_UNSIGNED_BYTE:
                    internalFormat = GL_LUMINANCE8_ALPHA8;
                    elementSize = 2;
                    break;
                case GL_UNSIGNED_SHORT:
                    internalFormat = GL_LUMINANCE16_ALPHA16;
                    elementSize = 4;
                    break;
                case GL_FLOAT:
                    internalFormat = GL_LUMINANCE_ALPHA32F_ARB;
                    elementSize = 8;
                    break;
                case GL_HALF_FLOAT_ARB:
                    internalFormat = GL_LUMINANCE_ALPHA16F_ARB;
                    elementSize = 4;
                    break;
                default:
                    return false; //format/type combo not supported
            }
            break;
        case GL_RGB:
            switch (type) {
                case GL_UNSIGNED_BYTE:
                    internalFormat = GL_RGB8;
                    elementSize = 3;
                    break;
                case GL_UNSIGNED_SHORT:
                    internalFormat = GL_RGB16;
                    elementSize = 6;
                    break;
                case GL_FLOAT:
                    internalFormat = GL_RGB32F_ARB;
                    elementSize = 12;
                    break;
                case GL_HALF_FLOAT_ARB:
                    internalFormat = GL_RGB16F_ARB;
                    elementSize = 6;
                    break;
                default:
                    return false; //format/type combo not supported
            }
            break;
        case GL_RGBA:
            switch (type) {
                case GL_UNSIGNED_BYTE:
                    internalFormat = GL_RGBA8;
                    elementSize = 4;
                    break;
                case GL_UNSIGNED_SHORT:
                    internalFormat = GL_RGBA16;
                    elementSize = 8;
                    break;
                case GL_FLOAT:
                    internalFormat = GL_RGBA32F_ARB;
                    elementSize = 16;
                    break;
                case GL_HALF_FLOAT_ARB:
                    internalFormat = GL_RGBA16F_ARB;
                    elementSize = 8;
                    break;
                default:
                    return false; //format/type combo not supported
            }
            break;
        default:
            //bad format
            return false;
            break;
    }


    //clear old data
    freeData();

    uint8_t *newImage = new uint8_t[width*height*elementSize];
    memcpy( newImage, data, width*height*elementSize);

    _data.push_back(newImage);

    _width = width;
    _height = height;
    _elementSize = elementSize;
    _internalFormat = internalFormat;
    _levelCount = 1;
    _layers = 1;
    _depth = 0;
    _format = format;
    _type = type;
    _cubeMap = false;

    return true;
}

bool NvImage::isCompressed() const {
    switch(_format) {
        case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
        case GL_COMPRESSED_LUMINANCE_LATC1_EXT:
        case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:
        case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:
        case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:
        case GL_COMPRESSED_RG_RGTC2:
        case GL_COMPRESSED_RED_RGTC1:
        case GL_COMPRESSED_SIGNED_RG_RGTC2:
        case GL_COMPRESSED_SIGNED_RED_RGTC1:
            return true;
    }
    return false;
}

bool NvImage::hasAlpha() const {
    switch(_format) {
        case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
        case GL_COMPRESSED_LUMINANCE_LATC1_EXT:
        case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:
        case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:
        case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:
        case GL_ALPHA:
        case GL_LUMINANCE_ALPHA:
        case GL_RGBA:
        case GL_RGBA_INTEGER:
        case GL_BGRA:
            return true;
    }
    return false;
}
