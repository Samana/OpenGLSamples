//----------------------------------------------------------------------------------
// File:        ParticleUpsampling/SceneRenderer.h
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

#ifndef SCENE_RENDERER_H
#define SCENE_RENDERER_H

//#define ENABLE_GPU_TIMERS 1
//#define ENABLE_CPU_TIMERS 1

#include "NvFoundation.h"
#include "NV/NvPlatformGL.h"
#include "ParticleRenderer.h"
#include "Upsampler.h"
#include "SceneInfo.h"
#include "NvGLUtils/NvGLSLProgram.h"
#include "NvGLUtils/NvSimpleFBO.h"

class OpaqueColorProgram;
class OpaqueDepthProgram;
class NvGLModel;

class SceneRenderer
{
public:
    struct Params
    {
        Params()
        : drawModel(true)
        , useDepthPrepass(false)
        , backgroundColor(0.5f, 0.8f, 1.0f)
        {
        }
        bool drawModel;
        bool useDepthPrepass;
        nv::vec3f backgroundColor;
    };

    SceneRenderer(bool isGL);
    ~SceneRenderer();

    void initTimers();
    void loadModelFromData(char *fileData);
    void loadModel();

    void drawScene(GLint positionAttrib, GLint normalAttrib = -1);
    void drawModel(GLint positionAttrib, GLint normalAttrib = -1);
    void drawFloor(GLint positionAttrib, GLint normalAttrib = -1);

    void downsampleSceneDepth(NvSimpleFBO *srcFbo, NvSimpleFBO *dstFbo);
    void renderSceneDepth(NvSimpleFBO* depthFbo);
    void renderLowResSceneDepth();
    void renderFullResSceneColor();
    void renderFrame();

    void reshapeWindow(int32_t w, int32_t h)
    {
        m_scene.setScreenSize(w, h);
        createScreenBuffers();
    }

    void createScreenBuffers()
    {
        m_fbos->createScreenBuffers(m_scene.m_screenWidth, m_scene.m_screenHeight);
    }

    void createLightBuffer()
    {
        m_fbos->createLightBuffer();
    }

    void setEyeViewMatrix(nv::matrix4f viewMatrix)
    {
        m_scene.m_eyeView = viewMatrix;
    }

    ParticleRenderer::Params *getParticleParams()
    {
        return &m_particles->getParams();
    }

    Upsampler::Params *getUpsamplingParams()
    {
        return &m_upsampler->getParams();
    }

    SceneFBOs::Params *getSceneFBOParams()
    {
        return &m_fbos->m_params;
    }

    SceneRenderer::Params *getSceneParams()
    {
        return &m_params;
    }

    int32_t getScreenWidth()
    {
        return m_scene.m_screenWidth;
    }

    int32_t getScreenHeight()
    {
        return m_scene.m_screenHeight;
    }

protected:
    Params m_params;
    NvGLModel *m_model;
    ParticleRenderer *m_particles;
    Upsampler *m_upsampler;
    SceneInfo m_scene;

#if ENABLE_GPU_TIMERS
    GPUTimer m_GPUTimers[GPU_TIMER_COUNT];
#endif
#if ENABLE_CPU_TIMERS
    StopWatch m_CPUTimers[CPU_TIMER_COUNT];
#endif

    int32_t m_screenWidth;
    int32_t m_screenHeight;
    SceneFBOs *m_fbos;

    OpaqueColorProgram *m_opaqueColorProg;
    OpaqueDepthProgram *m_opaqueDepthProg;
};

#endif // SCENE_RENDERER_H
