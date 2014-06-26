//----------------------------------------------------------------------------------
// File:        NvModel/NvGLModel.h
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

#ifndef NVGLMODEL_H_
#define NVGLMODEL_H_

#include <NvFoundation.h>


#include "NV/NvPlatformGL.h"
#include "NV/NvMath.h"

/// \file
/// OpenGL geometric model handing and rendering

class NvModel;

/// Drawable geometric model using GL VBOs.  Supports loading from OBJ file data.
/// Contains positions as well as optional normals, UVs, and tangent vectors
class NvGLModel
{
public:
    NvGLModel();
    ~NvGLModel();

	/// Initialize internal model with passed in ptr
	/// \param[in] pModel pointer to an NvModel to use for mesh data.  
	/// WARNING!!! This pointer is cached in the object, and must not be freed after this call returns WARNING!!!
	NvGLModel(NvModel *pModel);

    /// Load the model from OBJ format data
    /// Loads a model from OBJ-formatted file data
    /// \param[in] fileData pointer to the OBJ file loaded into memory.  This pointer
    /// is not cached in the object, and may be freed after this call returns
    void loadModelFromObjData(char *fileData);

    /// Rescale the model geometry.
    /// Rescales the model geometry and centers it around the origin.  Does NOT update 
    /// the vertex buffers.  Applications should update the VBOs via #initBuffers
    /// \param[in] radius the desired new radius.  The model geometry will be rescaled to
    /// fit this radius
    void rescaleModel(float radius);

    /// Initialize or update the model geometry VBOs
    /// \param[in] computeTangents if set to true, then tangent vectors will be computed
    /// to be in the S texture coordinate direction.  This may require vertices to be
    /// duplicated in order to allow multiple tangents at a point.  This can cause model
    /// size explosion, and should be done only if required.
    void initBuffers(bool computeTangents = false);

    /// Draw the model using the current shader (positions)
    /// Binds the vertex position array to the given attribute array index and draws the
    /// model with the currently bound shader.
    /// \param[in] positionHandle the vertex attribute array index that represents position in the current shader
    void drawElements(GLint positionHandle);

    /// Draw the model using the current shader (positions and normals)
    /// Binds the vertex position and normal arrays to the given attribute array indices and draws the
    /// model with the currently bound shader.
    /// \param[in] positionHandle the vertex attribute array index that represents position in the current shader
    /// \param[in] normalHandle the vertex attribute array index that represents normals in the current shader
    void drawElements(GLint positionHandle, GLint normalHandle);

    /// Draw the model using the current shader (positions, UVs and normals)
    /// Binds the vertex position, UV and normal arrays to the given attribute array indices and draws the
    /// model with the currently bound shader.
    /// \param[in] positionHandle the vertex attribute array index that represents position in the current shader
    /// \param[in] normalHandle the vertex attribute array index that represents normals in the current shader
    /// \param[in] texcoordHandle the vertex attribute array index that represents UVs in the current shader
    void drawElements(GLint positionHandle, GLint normalHandle, GLint texcoordHandle);

    /// Draw the model using the current shader (positions, UVs, normals and tangents)
    /// Binds the vertex position, UV, normal and tangent arrays to the given attribute array indices and draws the
    /// model with the currently bound shader.
    /// \param[in] positionHandle the vertex attribute array index that represents position in the current shader
    /// \param[in] normalHandle the vertex attribute array index that represents normals in the current shader
    /// \param[in] texcoordHandle the vertex attribute array index that represents UVs in the current shader
    /// \param[in] tangentHandle the vertex attribute array index that represents tangents in the current shader
    void drawElements(GLint positionHandle, GLint normalHandle, GLint texcoordHandle, GLint tangentHandle);

    /// Get the low-level geometry data.
    /// Returns the underlying geometry model data instance
    /// \return a pointer to the #NvModel instance that holds the client-memory data
    NvModel *getModel();

    /// Updates the center point of the model
    void computeCenter();

    nv::vec3f m_center; ///< The computed center of the bounding box of the app

    nv::vec3f GetMinExt()
    {
        return m_minExtent;
    }

    nv::vec3f GetMaxExt()
    {
        return m_maxExtent;
    }

private:
    NvModel *model;
    GLuint model_vboID, model_iboID;
    nv::vec3f m_minExtent, m_maxExtent, m_radius;

    inline void bindBuffers();
    inline void unbindBuffers();
};


#endif /* NVGLMODEL_H_ */
