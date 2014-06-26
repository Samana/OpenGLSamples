//----------------------------------------------------------------------------------
// File:        ParticleUpsampling/ParticleUpsampling.cpp
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
#include "ParticleUpsampling.h"
#include "NvAppBase/NvFramerateCounter.h"
#include "NvAppBase/NvInputTransformer.h"
#include "NvAssetLoader/NvAssetLoader.h"
#include "NvUI/NvTweakBar.h"
#include "NV/NvLogs.h"

#include "SceneRenderer.h"

void printGLString(const char *name, GLenum s)
{
    char *v = (char *) glGetString(s);
    LOGI("GL %s: %s\n", name, v);
}


ParticleUpsampling::ParticleUpsampling(NvPlatformContext* platform) : 
    NvSampleApp(platform, "Particle Upsampling Sample")
{
    // Required in all subclasses to avoid silent link issues
    forceLinkHack();
}

ParticleUpsampling::~ParticleUpsampling()
{
    LOGI("ParticleUpsampling: destroyed\n");
}

void ParticleUpsampling::configurationCallback(NvEGLConfiguration& config)
{ 
    config.depthBits = 24; 
    config.stencilBits = 0; 
    config.apiVer = NvGfxAPIVersionGL4();
}

enum TweakBarActionCodes
{
    REACT_UPDATE_SCREEN_BUFFERS = 1,
    REACT_UPDATE_LIGHT_BUFFERS,
};

void ParticleUpsampling::initUI(void)
{
    if (mTweakBar)
    {
        mTweakBar->addPadding();
        mTweakBar->addValue("renderShadows", m_sceneRenderer->getParticleParams()->renderShadows);
        mTweakBar->addValue("drawModel", m_sceneRenderer->getSceneParams()->drawModel);
        mTweakBar->addValue("useDepthPrepass", m_sceneRenderer->getSceneParams()->useDepthPrepass);

        mTweakBar->addPadding();
        NvTweakEnum<uint32_t> shadowSliceModes[] = {
            {"16",  16},
            {"32",  32},
            {"64",  64}
        };
        mTweakBar->addEnum("shadowSlices", m_sceneRenderer->getParticleParams()->numSlices, shadowSliceModes, TWEAKENUM_ARRAYSIZE(shadowSliceModes));

        mTweakBar->addPadding();
        NvTweakEnum<uint32_t> particleDownsampleModes[] = {
            {"Full-Res",    1},
            {"Half-Res",    2},
            {"Quarter-Res", 4}
        };
        mTweakBar->addEnum("particleDownsample", m_sceneRenderer->getSceneFBOParams()->particleDownsample, particleDownsampleModes, TWEAKENUM_ARRAYSIZE(particleDownsampleModes), REACT_UPDATE_SCREEN_BUFFERS);

        NvTweakEnum<uint32_t> lightBufferSizeModes[] = {
            {"64x64",   64},
            {"128x128",  128},
            {"256x256",  256}
        };
        mTweakBar->addEnum("lightBufferSize", m_sceneRenderer->getSceneFBOParams()->lightBufferSize, lightBufferSizeModes, TWEAKENUM_ARRAYSIZE(lightBufferSizeModes), REACT_UPDATE_LIGHT_BUFFERS);
    }
}

NvUIEventResponse ParticleUpsampling::handleReaction(const NvUIReaction& react)
{
    switch(react.code)
    {
    case REACT_UPDATE_SCREEN_BUFFERS:
        m_sceneRenderer->createScreenBuffers();
        return nvuiEventHandled;
    case REACT_UPDATE_LIGHT_BUFFERS:
        m_sceneRenderer->createLightBuffer();
        return nvuiEventHandled;
    default:
        break;
    }
    return nvuiEventNotHandled;
}

void ParticleUpsampling::initRendering(void)
{
    printGLString("Version",    GL_VERSION);
    printGLString("Vendor",     GL_VENDOR);
    printGLString("Renderer",   GL_RENDERER);
    printGLString("Extensions", GL_EXTENSIONS);

    GLint depthBits;
    glGetIntegerv(GL_DEPTH_BITS, &depthBits);
    LOGI("depth bits = %d\n", depthBits);

    //glClearColor(0.5f, 0.5f, 0.5f, 1.0f); 
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);    

    NvAssetLoaderAddSearchPath("ParticleUpsampling");

    m_sceneRenderer = new SceneRenderer(requireMinAPIVersion(NvGfxAPIVersionGL4(), false));

    CHECK_GL_ERROR();
}

void ParticleUpsampling::reshape(int32_t width, int32_t height)
{
    glViewport( 0, 0, (GLint) width, (GLint) height );

    m_sceneRenderer->reshapeWindow(width, height);

    CHECK_GL_ERROR();
}

void ParticleUpsampling::draw(void)
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    matrix4f rotationMatrix, translationMatrix;
    nv::rotationY(rotationMatrix, m_transformer->getRotationVec().y);
    nv::translation(translationMatrix, 0.f, 0.f, -5.f);
    translationMatrix.set_scale(vec3f(-1.0, 1.0, -1.0));
    m_sceneRenderer->setEyeViewMatrix(translationMatrix * rotationMatrix);

    m_sceneRenderer->renderFrame();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, m_width, m_height);
}

bool ParticleUpsampling::handleGamepadChanged(uint32_t changedPadFlags) {
    if (changedPadFlags) {
        NvGamepad* pad = getPlatformContext()->getGamepad();
        if (!pad) return false;

        LOGI("gamepads: 0x%08x", changedPadFlags);
    }

    return false;
}


NvAppBase* NvAppFactory(NvPlatformContext* platform) {
    return new ParticleUpsampling(platform);
}
