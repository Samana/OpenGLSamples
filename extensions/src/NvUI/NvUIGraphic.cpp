//----------------------------------------------------------------------------------
// File:        NvUI/NvUIGraphic.cpp
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
/*
 * library for implementing base classes for NV UI framework
 * used for building Widgets and Applications on common code.
 */

#include "NvUI/NvUI.h"

#include "NvAssetLoader/NvAssetLoader.h"
#include "NvGLUtils/NvImage.h"
#include "NV/NvPlatformGL.h"
#include <NvGLUtils/NvGLSLProgram.h>
#include "NV/NvLogs.h"

#include <math.h>


//======================================================================
//======================================================================
static NvGraphicShader ms_shader;

GLuint NvUIGraphic::ms_vbo = 0;
GLuint NvUIGraphic::ms_vboFlip = 0;
GLuint NvUIGraphic::ms_ibo = 0;

float NvUIGraphic::s_pixelToClipMatrix[4][4];
float NvUIGraphic::s_pixelScaleFactorX = 0.5f;
float NvUIGraphic::s_pixelScaleFactorY = 0.5f;
int32_t NvUIGraphic::s_pixelXLast = 1;
int32_t NvUIGraphic::s_pixelYLast = 1;
float NvUIGraphic::s_graphicWidth=0, NvUIGraphic::s_graphicHeight=0;
//static int32_t s_pixelOrientLast = -2; // an invalid value..

//======================================================================
// ----- NvUIGraphic -----
//======================================================================

/** This is a helper structure for rendering sets of 2D textured vertices. */
typedef struct
{
    nv::vec2<float> position; /**< 2d vertex coord */
    nv::vec2<float> uv; /**< vertex texturing position */
} NvTexturedVertex;


const static char s_graphicVertShader[] =
"#version 100\n"
"// this is set from higher level.  think of it as the upper model matrix\n"
"uniform mat4 pixelToClipMat;\n"
"attribute vec2 position;\n"
"attribute vec2 tex;\n"
"varying vec2 tex_coord;\n"
"void main()\n"
"{\n"
"    gl_Position = pixelToClipMat * vec4(position, 0, 1);\n"
"    tex_coord = tex;\n"
"}\n";

const static char s_graphicFragShader[] =
"#version 100\n"
"precision mediump float;\n"
"varying vec2 tex_coord;\n"
"uniform sampler2D sampler;\n"
"uniform float alpha;\n"
"uniform vec4 color;\n"
"void main()\n"
"{\n"
"    gl_FragColor = texture2D(sampler, tex_coord) * vec4(color.r,color.g,color.b,alpha);\n"
"}\n";



//======================================================================
//======================================================================
void NvGraphicShader::Load(const char* vs, const char* fs)
{
    NvGLSLProgram *prog = NvGLSLProgram::createFromStrings(vs, fs);
    CHECK_GL_ERROR();

    if (prog==NULL)
    {
        // !!!!TBD TODO
        return;
    }

    m_program = prog;
    prog->enable();

    m_positionIndex = prog->getAttribLocation("position");
    m_uvIndex = prog->getAttribLocation("tex");

    prog->setUniform1i(prog->getUniformLocation("sampler"), 0); // texunit index zero.

    m_matrixIndex = prog->getUniformLocation("pixelToClipMat");
    m_alphaIndex = prog->getUniformLocation("alpha");
    m_colorIndex = prog->getUniformLocation("color");

    prog->disable();

    CHECK_GL_ERROR();
}


//======================================================================
//======================================================================
void NvUIGraphic::PrivateInit(void)
{
    m_tex = NULL;
    m_scale = false;
    m_vFlip = false;
    m_color = NV_PC_PREDEF_WHITE;
};    

//======================================================================
//======================================================================
NvUIGraphic::NvUIGraphic(const std::string& texname, float dstw/*==0*/, float dsth/*==0*/)
{
    StaticInit();
    PrivateInit();
    LoadTexture(texname);
    if (dstw!=0)
        SetDimensions(dstw, dsth);
}


//======================================================================
//======================================================================
NvUIGraphic::NvUIGraphic(uint32_t texId, bool alpha,
                            uint32_t srcw, uint32_t srch,
                            float dstw/*==0*/, float dsth/*==0*/)
{
    StaticInit();
    PrivateInit();
    SetTextureID(texId, alpha, srcw, srch);
    if (dstw!=0)
        SetDimensions(dstw, dsth);
}


//======================================================================
//======================================================================
NvUIGraphic::NvUIGraphic(NvUITexture *uiTex, float dstw/*==0*/, float dsth/*==0*/)
{
    StaticInit();
    PrivateInit();
    m_tex = uiTex;
    m_tex->AddRef();
    if (dstw!=0)
        SetDimensions(dstw, dsth);
    else
        SetDimensions((float)m_tex->GetWidth(), (float)m_tex->GetHeight());
}


//======================================================================
//======================================================================
NvUIGraphic::~NvUIGraphic()
{
    FlushTexture();
    StaticCleanup();
}


//======================================================================
//======================================================================
void NvUIGraphic::FlushTexture()
{
    if (m_tex)
    {
        m_tex->DelRef();
        m_tex = NULL;
    }
    m_scale = false;
}


//======================================================================
//======================================================================
bool NvUIGraphic::LoadTexture(const std::string& texname, bool resetDimensions/*==true*/)
{
    FlushTexture(); // in case we're being use to re-load new texture.

    CHECK_GL_ERROR();
    glActiveTexture(GL_TEXTURE0);
    m_tex = NvUITexture::CacheTexture(texname);
    CHECK_GL_ERROR();

    if (m_tex && resetDimensions)
    {
        // set screenrect and dest dimensions to match the raw texel size of the texture.
        SetDimensions((float)m_tex->GetWidth(), (float)m_tex->GetHeight());
    }

    return(m_tex && m_tex->GetGLTex()!=0);
}


//======================================================================
// call Flush manually if you want to re-set same texID...
//======================================================================
void NvUIGraphic::SetTextureID(const uint32_t texID, bool alpha,
                                uint32_t srcw, uint32_t srch)
{
    if (m_tex)
    {
        // optimization. !!!!TBD TODO? WORKING PROPERLY??
        if (texID == m_tex->GetGLTex()) // we've got this already.  punt if match
        {
            if (srcw == (uint32_t)(m_tex->GetWidth())
            &&  srch == (uint32_t)(m_tex->GetHeight()))
                return;
        }

        FlushTexture(); // anything bound...
    }
    
    m_tex = new NvUITexture(texID, alpha, srcw, srch);
   
    // and push dest dimensions into screenRect and texDestZZZ
    SetDimensions((float)srcw, (float)srch);
}

void NvUIGraphic::SetTexture(NvUITexture *tex)
{
    FlushTexture();
    m_tex = tex;
    m_tex->AddRef();

    // TODO - Does this really make sense??  We rarely use texel-to-pixel
    // graphics, do we?  We tend to have set the scale manually.
    // And since SetTextureID doesn't rescale, I think this should not, either
//    SetDimensions((float)m_tex->GetWidth(), (float)m_tex->GetHeight());
}


//======================================================================
//======================================================================
void NvUIGraphic::SetTextureFiltering(uint32_t minFilter, uint32_t magFilter)
{
    if (m_tex && m_tex->GetGLTex()
        && (minFilter || magFilter))
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_tex->GetGLTex());

        if (minFilter)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
        if (magFilter)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

        glBindTexture(GL_TEXTURE_2D, 0); // unbind at end!
    }
}


//======================================================================
//======================================================================
static int32_t s_initCount;
bool NvUIGraphic::StaticInit()
{
    if (s_initCount++ == 0)
    {
        NvTexturedVertex    vert[4];
        uint16_t            indices[6];

        ms_shader.Load(s_graphicVertShader, s_graphicFragShader);

        uint16_t pos = 0;
        int32_t ipos = 0;

        indices[ipos+0] = pos;
        indices[ipos+1] = pos+1;
        indices[ipos+2] = pos+3;
        indices[ipos+3] = pos+3;
        indices[ipos+4] = pos+1;
        indices[ipos+5] = pos+2;

        vert[pos].position.x = 0; 
        vert[pos].position.y = 1;
        vert[pos].uv.x = 0; 
        vert[pos].uv.y = 1;
        pos++;

        vert[pos].position.x = 0; 
        vert[pos].position.y = 0;
        vert[pos].uv.x = 0; 
        vert[pos].uv.y = 0;
        pos++;

        vert[pos].position.x = 1; 
        vert[pos].position.y = 0;
        vert[pos].uv.x = 1; 
        vert[pos].uv.y = 0;
        pos++;

        vert[pos].position.x = 1; 
        vert[pos].position.y = 1;
        vert[pos].uv.x = 1; 
        vert[pos].uv.y = 1;
        pos++;

        glGenBuffers(1, &ms_ibo);
        glGenBuffers(1, &ms_vbo);

        glBindBuffer(GL_ARRAY_BUFFER, ms_vbo);
        glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(NvTexturedVertex), vert, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ms_ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(uint16_t), indices, GL_STATIC_DRAW);

        CHECK_GL_ERROR();

        // make a texture-Y-flipped vbo...
        glGenBuffers(1, &ms_vboFlip);
        pos = 0;
        
        vert[pos].position.x = 0; 
        vert[pos].position.y = 1;
        vert[pos].uv.x = 0; 
        vert[pos].uv.y = 0;
        pos++;

        vert[pos].position.x = 0; 
        vert[pos].position.y = 0;
        vert[pos].uv.x = 0; 
        vert[pos].uv.y = 1;
        pos++;

        vert[pos].position.x = 1; 
        vert[pos].position.y = 0;
        vert[pos].uv.x = 1; 
        vert[pos].uv.y = 1;
        pos++;

        vert[pos].position.x = 1; 
        vert[pos].position.y = 1;
        vert[pos].uv.x = 1; 
        vert[pos].uv.y = 0;
        pos++;

        glBindBuffer(GL_ARRAY_BUFFER, ms_vboFlip);
        glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(NvTexturedVertex), vert, GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        // The following entries are const
        // so we set them up now and never change
        s_pixelToClipMatrix[2][0] = 0.0f;
        s_pixelToClipMatrix[2][1] = 0.0f;

        s_pixelToClipMatrix[0][2] = 0.0f;
        s_pixelToClipMatrix[1][2] = 0.0f;
        s_pixelToClipMatrix[2][2] = 1.0f;
        s_pixelToClipMatrix[3][2] = 0.0f;

        s_pixelToClipMatrix[0][3] = 0.0f;
        s_pixelToClipMatrix[1][3] = 0.0f;
        s_pixelToClipMatrix[2][3] = 0.0f;
        s_pixelToClipMatrix[3][3] = 1.0f;
    }

    CHECK_GL_ERROR();
    
    return true;
}


//======================================================================
//======================================================================
void NvUIGraphic::StaticCleanup()
{
    if (--s_initCount == 0)
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glUseProgram(0);

        delete ms_shader.m_program;
        ms_shader.m_program = 0;

        glDeleteBuffers(1, &ms_vbo);
        glDeleteBuffers(1, &ms_vboFlip);
        glDeleteBuffers(1, &ms_ibo);
        ms_vbo = 0;
        ms_vboFlip = 0;
        ms_ibo = 0;
    }
}


//======================================================================
// set the color to white to disable colorization
//======================================================================
void NvUIGraphic::SetColor(NvPackedColor color)
{
    m_color = color;
}


//======================================================================
//======================================================================
void NvUIGraphic::Draw(const NvUIDrawState &drawState)
{
    if (!m_isVisible) return;
    if (!m_tex) return;

    // calculate internal alpha value...
    float myAlpha = m_alpha;
    if (drawState.alpha != 1.0f)
        myAlpha *= drawState.alpha;

    // pick correct shader based on alpha...
    ms_shader.m_program->enable();

    // then if alpha shader, set alpha uniform...
    if (ms_shader.m_alphaIndex >= 0)
        glUniform1f(ms_shader.m_alphaIndex, myAlpha);
    
    // then if colorizing shader, set color uniform...
    if (ms_shader.m_colorIndex >= 0)
    {   // optimize it a bit...  // !!!!TBD alpha in color not just sep value?
        if (NV_PC_IS_WHITE(m_color))
            glUniform4f(ms_shader.m_colorIndex, 1,1,1,1);
        else
            glUniform4f(ms_shader.m_colorIndex,
                            NV_PC_RED_FLOAT(m_color),
                            NV_PC_GREEN_FLOAT(m_color),
                            NV_PC_BLUE_FLOAT(m_color),
                            1); // !!!!TBD
    }
    
    // update the transform matrix.
    int32_t designWidth, designHeight;
    if (drawState.designWidth)
    {
        designWidth = drawState.designWidth;
        designHeight = drawState.designHeight;
    }
    else
    {
        designWidth = drawState.width;
        designHeight = drawState.height;
    }
    
    // update the scale factors ONLY IF cached design size changed.
    bool scaleFactorChanged = false;
    if (s_pixelXLast != designWidth)
    {
        s_pixelXLast = (int32_t)designWidth;
        s_pixelScaleFactorX = 2.0f / s_pixelXLast;
        scaleFactorChanged = true;
    }
    if (s_pixelYLast != designHeight)
    {
        s_pixelYLast = (int32_t)designHeight;
        s_pixelScaleFactorY = 2.0f / s_pixelYLast;
        scaleFactorChanged = true;
    }

    float rad = (float)(drawState.rotation / 180.0f * 3.14159f); // [-1,2]=>[-90,180] in radians...
    float cosf = cos(rad);
    float sinf = sin(rad);

    const float wNorm = s_pixelScaleFactorX;
    const float hNorm = s_pixelScaleFactorY;

    if (s_graphicWidth != m_rect.width)
    {
        s_graphicWidth = m_rect.width;
        scaleFactorChanged = true;
    }
    if (s_graphicHeight != m_rect.height)
    {
        s_graphicHeight = m_rect.height;
        scaleFactorChanged = true;
    }

    //if (1) //(scaleFactorChanged) // scale factor OR w/h is different this call
    {
        s_pixelToClipMatrix[0][0] = wNorm * m_rect.width  * cosf;
        s_pixelToClipMatrix[1][0] = hNorm * m_rect.height * -sinf;
        s_pixelToClipMatrix[0][1] = wNorm * m_rect.width  * sinf;
        s_pixelToClipMatrix[1][1] = hNorm * m_rect.height * cosf;
    }
    
    s_pixelToClipMatrix[3][0] = ( wNorm * m_rect.left - 1) * cosf
                              - ( 1 - hNorm * (m_rect.top + m_rect.height))  * sinf;
    s_pixelToClipMatrix[3][1] = ( wNorm * m_rect.left - 1 ) * sinf
                              + ( 1 - hNorm * (m_rect.top + m_rect.height))  * cosf;

    glUniformMatrix4fv(ms_shader.m_matrixIndex, 1, GL_FALSE, &(s_pixelToClipMatrix[0][0]));

    // set up texturing.
    bool ae = false;
    if (m_tex->GetHasAlpha() || (myAlpha<1.0f))
    {
        ae = true;
        glEnable(GL_BLEND);
        // Alpha sums in the destination channel to ensure that
        // partially-opaque items do not decrease the destination
        // alpha and thus "cut holes" in the backdrop
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
            GL_ONE, GL_ONE);
    }
    else
        glDisable(GL_BLEND);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_tex->GetGLTex());
    glBindBuffer(GL_ARRAY_BUFFER, m_vFlip?ms_vboFlip:ms_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ms_ibo);

    glVertexAttribPointer(ms_shader.m_positionIndex, 2, GL_FLOAT, 0, sizeof(NvTexturedVertex), 0);
    glEnableVertexAttribArray(ms_shader.m_positionIndex);
    glVertexAttribPointer(ms_shader.m_uvIndex, 2, GL_FLOAT, 0, sizeof(NvTexturedVertex), (void*)sizeof(nv::vec2<float>));
    glEnableVertexAttribArray(ms_shader.m_uvIndex);

    // draw it already!
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

    //nv_flush_tracked_attribs();
    glDisableVertexAttribArray(ms_shader.m_positionIndex);
    glDisableVertexAttribArray(ms_shader.m_uvIndex);

    if (ae)
        glDisable(GL_BLEND);
}
