//----------------------------------------------------------------------------------
// File:        NvGLUtils/NvImage.h
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

#ifndef NV_IMAGE_H
#define NV_IMAGE_H

#include <NvFoundation.h>
#include <vector>
#include <assert.h>
#include <NV/NvGfxAPI.h>

/// \file
/// Image-handling support (loading, editing, GL textures)

class NvImage;

/// GL-based image loading, representation and handling
/// Support loading of images from DDS files and data, including
/// cube maps, arrays mipmap levels, formats, etc.
/// The class does NOT encapsulate a GL texture object, only the
/// client side pixel data that could be used to create such a texture
class NvImage {
public:

    /// Sets the image origin to top or bottom.
    /// Sets the origin to be assumed when loading image data from file or data block
    /// By default, the image library places the origin of images at the
    /// lower-left corner, to make it map exactly to OpenGL screen coords.
    /// This flips the image, and it might make it incompatible with
    /// the texture coordinate conventions of an imported model.
    /// \param[in] ul true if the origin is in the upper left (D3D/DDS) or bottom-left (GL)
    static void UpperLeftOrigin( bool ul);

    /// Create a new GL texture and upload the given image to it
    /// \param[in] image the image to load
    /// \return the GL texture ID on success, 0 on failure
    static uint32_t UploadTexture(NvImage* image);

    /// Create a new GL texture directly from DDS file
    /// Uses #NvAssetLoaderRead for opening the file.  See the documentation for
    /// that package to understand the correct paths
    /// \param[in] filename the image filename (and path) to load
    /// \return the GL texture ID on success, 0 on failure
    static uint32_t UploadTextureFromDDSFile(const char* filename);

    /// Create a new GL texture directly from DDS file-formatted data
    /// \param[in] ddsData the pointer to the DDS file data
    /// \param[in] length the size in bytes of the file block
    /// \return the GL texture ID on success, 0 on failure
    static uint32_t UploadTextureFromDDSData(const char* ddsData, int32_t length);

    /// Create a new NvImage (no texture) directly from DDS file
    /// Uses #NvAssetLoaderRead for opening the file.  See the documentation for
    /// that package to understand the correct paths
    /// \param[in] filename the image filename (and path) to load
    /// \return a pointer to the NvImage representing the file or null on failure
    static NvImage* CreateFromDDSFile(const char* filename);

    NvImage();
    virtual ~NvImage();

    /// The image width in pixels
    /// \return the width of the image in pixels
    int32_t getWidth() const { return _width; }

    /// The image height in pixels
    /// \return the height of the image in pixels
    int32_t getHeight() const { return _height; }

    /// The image depth in pixels.
    /// This is the third dimension of a 3D/volume image, NOT the color-depth
    /// \return the depth of the image (0 for images with no depth)
    int32_t getDepth() const { return _depth; }

    /// The number of miplevels.
    /// \return the number of mipmap levels available for the image
    int32_t getMipLevels() const { return _levelCount; }

    /// The number of cubemap faces.
    /// \return the number of cubemap faces available for the image (0 for non-cubemap images)
    int32_t getFaces() const { return _cubeMap ? _layers : 0; }

    /// The number of layers in a texture array
    /// \return the number of layers for use in texture arrays
    int32_t getLayers() const { return _layers; }

    /// The GL format of the image
    /// \return the format of the image data (GL_RGB, GL_BGR, etc)
    uint32_t getFormat() const { return _format; }

    /// The GL internal format of the image
    /// \return the suggested internal format for the data
    uint32_t getInternalFormat() const { return _internalFormat; }

    /// The GL type of the pixel data
    /// \return the type of the image data
    uint32_t getType() const { return _type; }

    /// The size (in bytes) of a selected mipmap level of the image
    /// \return the Size in bytes of a level of the image 
    /// \param[in] level the mipmap level whose size if to be returned
    int32_t getImageSize(int32_t level = 0) const;

    /// Whether or not the image is compressed
    /// \return boolean whether the data is a crompressed format
    bool isCompressed() const;

    /// Whether or not the image is a cubemap
    /// \return boolean whether the image represents a cubemap
    bool isCubeMap() const { return _cubeMap; }

    /// Whether or not the image's pixel format has an explicit alpha channel
    /// \return boolean whether the image has explicit alpha channel
    bool hasAlpha() const;

    /// Whether or not the image is an array texture
    /// \return boolean whether the image represents a texture array
    bool isArray() const { return _layers > 1; }

    /// Whether or not the image is a volume (3D) image
    /// \return boolean whether the image represents a volume
    bool isVolume() const { return _depth > 0; }

    ///@{
    /// Get a pointer to the pixel data for a given mipmap level.
    /// \param[in] level the mipmap level [0, getMipLevels)
    /// \return a pointer to the data
    const void* getLevel( int32_t level) const;
    void* getLevel( int32_t level);
    ///@}

    ///@{
    /// Get a pointer to the pixel data for a given mipmap level and cubemap face.
    /// \param[in] level the mipmap level [0, getMipLevels)
    /// \param[in] face the cubemap face (GL_TEXTURE_CUBE_MAP_*_*)
    /// \return a pointer to the data
    const void* getLevel( int32_t level, uint32_t face) const;
    void* getLevel( int32_t level, uint32_t face);
    ///@}

    ///@{
    /// Get a pointer to the pixel data for a given mipmap level and array slice.
    /// \param[in] level the mipmap level [0, #getMipLevels)
    /// \param[in] slice the layer index [0, #getLayers)
    /// \return a pointer to the data
    const void* getLayerLevel( int32_t level, int32_t slice) const;
    void* getLayerLevel( int32_t level, int32_t slice);
    ///@}

    /// Loads an image from file-formatted data.
    /// Initialize an image from file-formatted memory; only DDS files are supported
    /// \param[in] fileData the block of memory representing the entire image file
    /// \param[in] size the size of the data block in bytes
    /// \param[in] fileExt the file extension string; must be "dds"
    /// \return true on success, false on failure
    bool loadImageFromFileData(const uint8_t* fileData, size_t size, const char* fileExt);

    /// Convert a flat "cross" image to  a cubemap
    /// Convert a suitable image from a cubemap cross to a cubemap
    /// \return true on success or false for unsuitable source images
    bool convertCrossToCubemap();

    bool setImage( int32_t width, int32_t height, uint32_t format, uint32_t type, const void* data);

    /// Set the API version to be targetted for image loading.
    /// Images may be loaded differently for OpenGL ES and OpenGL.  This function
    /// sends a hint to the loader which allows it to target the desired API level.
    /// \param[in] api the desired target API. Default is GL4 (highest-end features)
    static void setAPIVersion(const NvGfxAPIVersion& api) { m_gfxAPIVersion = api; }

    /// Gets the current API-level for targetting the loading of images
    /// \return the current API level
    static const NvGfxAPIVersion& getAPIVersion() { return m_gfxAPIVersion; }

    /// Enables or disables automatic expansion of DXT images to RGBA
    /// \param[in] expand true enables DXT-to-RGBA expansion.  False passes 
    /// DXT images through as-is
    static void setDXTExpansion(bool expand) { m_expandDXT = expand; }

    /// Gets the status of automatic DXT expansion
    /// \return true if DXT images will be expanded, false if they will be passed through
    static bool getDXTExpansion() { return m_expandDXT; }

protected:
    /// \privatesection

    static NvGfxAPIVersion m_gfxAPIVersion;
    int32_t _width;
    int32_t _height;
    int32_t _depth;
    int32_t _levelCount;
    int32_t _layers;
    uint32_t _format;
    uint32_t _internalFormat;
    uint32_t _type;
    int32_t _elementSize;
    bool _cubeMap;

    //pointers to the levels
    std::vector<uint8_t*> _data;

    void freeData();
    void flipSurface(uint8_t *surf, int32_t width, int32_t height, int32_t depth);
    void componentSwapSurface(uint8_t *surf, int32_t width, int32_t height, int32_t depth);
    uint8_t* expandDXT(uint8_t *surf, int32_t width, int32_t height, int32_t depth);

    //
    // Static elements used to dispatch to proper sub-readers
    //
    //////////////////////////////////////////////////////////////
    struct FormatInfo {
        const char* extension;
        bool (*reader)(const uint8_t* fileData, size_t size, NvImage& i);
        bool (*writer)(uint8_t* fileData, size_t size, NvImage& i);
    };

    static FormatInfo formatTable[]; 
    static bool upperLeftOrigin;
    static bool m_expandDXT;

    static bool readDDS(const uint8_t* fileData, size_t size, NvImage& i);

    static void flip_blocks_dxtc1(uint8_t *ptr, uint32_t numBlocks);
    static void flip_blocks_dxtc3(uint8_t *ptr, uint32_t numBlocks);
    static void flip_blocks_dxtc5(uint8_t *ptr, uint32_t numBlocks);
    static void flip_blocks_bc4(uint8_t *ptr, uint32_t numBlocks);
    static void flip_blocks_bc5(uint8_t *ptr, uint32_t numBlocks);

    friend bool TranslateDX10Format( const void *ptr, NvImage &i, int32_t &bytesPerElement, bool &btcCompressed);
};

#endif //NV_IMAGE_H
