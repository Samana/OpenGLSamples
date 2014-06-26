//----------------------------------------------------------------------------------
// File:        BindlessApp/assets/shaders/simple_vertex.glsl
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
#version 420
#extension GL_NV_shader_buffer_load : require

struct PerMeshUniforms;


// Input attributes
layout(location=0) in vec4             iPos;
layout(location=1) in vec4             iColor;
layout(location=2) in PerMeshUniforms* bindlessPerMeshUniformsPtr;
layout(location=3) in vec4             iAttrib3; 
layout(location=4) in vec4             iAttrib4; 
layout(location=5) in vec4             iAttrib5; 
layout(location=6) in vec4             iAttrib6; 
layout(location=7) in vec4             iAttrib7; 



// Outputs
layout(location=0) smooth out vec4 oColor;



// Uniforms
layout(std140, binding=2) uniform TransformParams
{
    mat4 ModelView;
    mat4 ModelViewProjection;
  bool UseBindlessUniforms;
};

struct PerMeshUniforms
{ 
  float r, g, b, a;
};

layout(std140, binding=3) uniform NonBindlessPerMeshUniforms
{
  PerMeshUniforms nonBindlessPerMeshUniforms;
};


void main() 
{
  float r, g, b;

  if(UseBindlessUniforms)
  {
    // For bindless uniforms, we pass in a pointer in GPU memory to the uniform data through a vertex attribute.
    // We use this pointer to load the uniform data.
    // *** INTERESTING ***
    r = bindlessPerMeshUniformsPtr->r;
    g = bindlessPerMeshUniformsPtr->g;
    b = bindlessPerMeshUniformsPtr->b;
  }
  else
  {
    // For non-bindless uniforms, we directly used the uniforms
    r = nonBindlessPerMeshUniforms.r;
    g = nonBindlessPerMeshUniforms.g;
    b = nonBindlessPerMeshUniforms.b;
  }

  vec4 positionModelSpace;
  positionModelSpace = iPos;
  positionModelSpace.y += sin(positionModelSpace.y * r) * .2f;
  gl_Position = ModelViewProjection * positionModelSpace;
    
  oColor.r = iColor.r * r;
  oColor.g = iColor.g * g;
  oColor.b = iColor.b * b;
  oColor.a = iColor.a;
}
