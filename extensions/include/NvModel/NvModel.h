//----------------------------------------------------------------------------------
// File:        NvModel/NvModel.h
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

#ifndef NV_MODEL_H
#define NV_MODEL_H

#include <NvFoundation.h>


#include <vector>
#include "NV/NvMath.h"

/// \file
/// GL-compatible geometric object loader and optimizer

/// Primitive type mask.
struct NvModelPrimType {
    NvModelPrimType() {}
    enum Enum {
        NONE = 0x0, ///< Not set
        POINTS = 0x1, ///< single-vertex points
        EDGES = 0x2, ///< two vertex edges
        TRIANGLES = 0x4, ///< three-vertex triangles
        TRIANGLES_WITH_ADJACENCY = 0x8, ///< triangles with adjancency info
        ALL = 0xf ///< mask of all values
    };
};

/// Non-rendering geometry model.
/// Graphics-API-agnostic geometric model class, including model loading from
/// OBJ file data, optimization, bounding volumes and rescaling.  
/// Can compute addition items such as facet normals and tangents as needed.
class NvModel {
protected:
    NvModel();

public:

    /// Create a model.
    /// Creates an empty model
    static NvModel* Create();

    virtual ~NvModel();

    /// Load raw model from OBJ data.
    /// Loads a model from the given block of raw OBJ-file data
    /// \param[in] fileData a pointer to the in-memory representation of the OBJ file.
    /// This data is not cached locally and can be freed once the function returns
    /// \return true on success and false on failure
    bool loadModelFromFileDataObj( char* fileData);

    /// Process a model into rendering-friendly form.
    /// This function takes the raw model data in the internal
    ///  structures, and attempts to bring it to a format directly
    ///  accepted for vertex array style rendering. This means that
    ///  a unique compiled vertex will exist for each unique
    ///  combination of position, normal, tex coords, etc that are
    ///  used in the model. The prim parameter, tells the model
    ///  what type of index list to compile. By default it compiles
    ///  a simple triangle mesh with no connectivity. 
    /// \param[in] prim the desired primitive type that will be used for rendering;
    /// the target of the compilation operation
    void compileModel( NvModelPrimType::Enum prim = NvModelPrimType::TRIANGLES);

    ///  Computes an AABB from the data.
    /// This function returns the points defining the axis-
    /// aligned bounding box containing the model.
    /// \param[out] minVal the returned minimum corner of the AABB
    /// \param[out] maxVal the returned maximum corner of the AABB
    void computeBoundingBox( nv::vec3f &minVal, nv::vec3f &maxVal);

    /// Rescale the model to the desired radius.
    /// Rescales object based on bounding box such that the new bounding box
    /// "radius" (half the diagonal of the bounding box) is the given target value
    /// \param[in] radius the desired target radius of the geometry
    void rescale( float radius);

    /// Rescale the model to the desired radius and centers it at the origin.
    /// Rescales object based on bounding box such that the new bounding box
    /// "radius" (half the diagonal of the bounding box) is the given target value and
    /// translates the vertices so that the bounding box is centered at the origin
    /// \param[in] radius the desired target radius of the geometry
    void rescaleToOrigin( float radius);

    /// Rescale the model to the desired radius.
    /// Rescales object about the given center-point, based on bounding box such
    /// that the new bounding box "radius" (half the diagonal of the bounding box) 
    /// is the given target value.  Scaling factor is the ratio of the max component
    /// of the old radius versus the target radius
    /// \param[in] center the centerpoint about which to scale
    /// \param[in] r the old "radius" vector
    /// \param[in] radius the desired target radius of the geometry
    void rescaleWithCenter( nv::vec3f center, nv::vec3f r, float radius);

    /// Translate the model vertices by a given vector.
    ///  Adds the given vector to each vertex.
    /// \param[in] center the vector to add
    void addToAllPositions( nv::vec3f center);


    /// Compute tangent vectors in the "S" UV direction.
    /// This function computes tangents in the s direction on
    /// the model. It operates on the raw data, so it should only
    /// be used before compiling a model into a HW friendly form.
    /// This can cause model expansion, since it can keep vertices
    /// from being shared.  Thus it should be used only when the results
    /// are required by the rendering method
    void computeTangents();

    /// Compute per-vertex normals.
    /// This function computes vertex normals for a model
    /// which did not have them. It computes them on the raw
    /// data, so it should be done before compiling the model
    /// into a HW friendly format.
    void computeNormals();

    /// Remove zero-area/length primitives.
    /// Removes primitives that will add nothing to the rendered result
    void removeDegeneratePrims();

    ///@{
    /// Vertex data existence queries.
    /// \return true if the given vertex attribute exists in the primitive and false if not
    bool hasNormals() const;
    bool hasTexCoords() const;
    bool hasTangents() const;
    bool hasColors() const;
    ///@}

    ///@{
    /// Vertex data size queries.
    /// \return the number of elements per vertex for the given attribute (e.g. xyz positions are 3)
    int32_t getPositionSize() const;
    int32_t getNormalSize() const;
    int32_t getTexCoordSize() const;
    int32_t getTangentSize() const;
    int32_t getColorSize() const;
    ///@}

    ///@{
    /// Vertex data removal functions.
    /// Removes the given per-vertex attributes from all vertices.  Must be called before the
    /// model is compiled for rendering!
    void clearNormals();
    void clearTexCoords();
    void clearTangents();
    void clearColors();
    ///@}

    ///@{
    /// Raw data access functions.
    /// These are to be used to get the raw array data from the file, not the compiled rendering data.
    /// Note that in raw form, each vertex attribute array has its own index array
    /// \return a pointer to the raw float array data.  This must be used in conjunction with the
    /// get*Size functions to determine the number of floats per vertex per attribute
    const float* getPositions() const;
    const float* getNormals() const;
    const float* getTexCoords() const;
    const float* getTangents() const;
    const float* getColors() const;
    ///@}

    ///@{
    /// Raw data index access functions.
    /// These arrays map from primitives to the raw per-attribute arrays.
    /// Note that in raw form, each vertex attribute array has its own index array.
    /// The data in raw form does NOT have the concept of each primitive having indices
    /// so a single vertex with all per-vertex data together
    const uint32_t* getPositionIndices() const;
    const uint32_t* getNormalIndices() const;
    const uint32_t* getTexCoordIndices() const;
    const uint32_t* getTangentIndices() const;
    const uint32_t* getColorIndices() const;
    ///@}

    ///@{
    /// Raw array length functions.
    /// \return the length (in vertices) of each raw data array
    /// Each array has a different length because sharing in the raw data
    /// is done per attribute
    int32_t getPositionCount() const;
    int32_t getNormalCount() const;
    int32_t getTexCoordCount() const;
    int32_t getTangentCount() const;
    int32_t getColorCount() const;
    ///@}

    /// Get the length of the index arrays.
    /// \return the number of indices in each index array
    int32_t getIndexCount() const;

    /// Get the array of compiled vertices.
    /// The array of the optimized, compiled vertices for rendering
    /// \return the pointer to the start of the first vertex
    const float* getCompiledVertices() const;

    /// Get the array of compiled indices for the given prim type.
    /// Computes and returns the array of indices for the given primitive type
    /// \param[in] prim the primitive type for which indices should be generated
    /// \return pointer to the array of indices
    const uint32_t* getCompiledIndices( NvModelPrimType::Enum prim = NvModelPrimType::TRIANGLES) const;

    ///@{
    /// Get the offset within the vertex of each attrib.
    /// \return the offset (in number of floats) of each attrib from the base of the vertex
    int32_t getCompiledPositionOffset() const;
    int32_t getCompiledNormalOffset() const;
    int32_t getCompiledTexCoordOffset() const;
    int32_t getCompiledTangentOffset() const;
    int32_t getCompiledColorOffset() const;
    ///@}

    /// Get the size of a compiled vertex.
    /// \return the size of the merged vertex (in number of floats)
    int32_t getCompiledVertexSize() const;

    /// Get the count of vertices in the compiled array.
    /// \return the vertex count in the compiled (renderable) array
    int32_t getCompiledVertexCount() const;

    /// The rendering index count.
    /// \param[in] prim the primitive type of the array whose length is to be returned
    /// \return the number of indices in the given array
    int32_t getCompiledIndexCount( NvModelPrimType::Enum prim = NvModelPrimType::TRIANGLES) const;

    int32_t getOpenEdgeCount() const;

protected:
    /// \privatesection
    static const int32_t NumPrimTypes = 4;

    //Would all this be better done as a channel abstraction to handle more arbitrary data?

    //data structures for model data, not optimized for rendering
    std::vector<float> _positions;
    std::vector<float> _normals;
    std::vector<float> _texCoords;
    std::vector<float> _sTangents;
    std::vector<float> _colors;
    int32_t _posSize;
    int32_t _tcSize;
    int32_t _cSize;

    std::vector<uint32_t> _pIndex;
    std::vector<uint32_t> _nIndex;
    std::vector<uint32_t> _tIndex;
    std::vector<uint32_t> _tanIndex;
    std::vector<uint32_t> _cIndex;

    //data structures optimized for rendering, compiled model
    std::vector<uint32_t> _indices[NumPrimTypes];
    std::vector<float> _vertices;
    int32_t _pOffset;
    int32_t _nOffset;
    int32_t _tcOffset;
    int32_t _sTanOffset;
    int32_t _cOffset;
    int32_t _vtxSize;

    int32_t _openEdges;

    static bool loadObjFromFileData( char *fileData, NvModel &m);
};

#endif
