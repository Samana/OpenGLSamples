//----------------------------------------------------------------------------------
// File:        NvUI/NvUIGraphicFrame.cpp
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

typedef struct
{
    float x, y;     // position
    float s, t;     // texture coordinate
    float bx, by;   // whether or not this vertex is a border vertex in x and y
} NvFrameVertex;

static int32_t s_staticCount = 0;

static NvGraphicFrameShader ms_shader;

static GLuint ms_gfvbo = 0;
static GLuint ms_gfibo = 0;

static float s_gfpixelToClipMatrix[4][4];
static float s_gfpixelScaleFactorX = 2.0f / 800.0f;
static float s_gfpixelScaleFactorY = 2.0f / 480.0f;
static int32_t s_gfpixelXLast = 800;
static int32_t s_gfpixelYLast = 480;


//======================================================================
// ----- NvUIGraphicFrame -----
//======================================================================

const static char s_frameVertShader[] =
"#version 100\n"
"// this is set from higher level.  think of it as the upper model matrix\n"
"uniform mat4 pixelToClipMat;\n"
"uniform vec2 thickness;\n"
"uniform vec2 texBorder;\n"
"attribute vec2 border;\n"
"attribute vec2 position;\n"
"attribute vec2 tex;\n"
"varying vec2 tex_coord;\n"
"void main()\n"
"{\n"
"    vec2 invBorder = vec2(1,1) - border;\n"
"    vec2 shiftedPosition = (position-thickness*invBorder*position);\n"
"    // we need to convert from -1,1 coords into 0,1 coords before xform.\n"
"    shiftedPosition *= 0.5;\n"
"    shiftedPosition += 0.5;\n"
"    // then we multiply like uigraphic normally would\n"
"    gl_Position = pixelToClipMat * vec4(shiftedPosition, 0, 1);\n"
"    tex_coord = tex + invBorder * -position * texBorder;\n"
"}\n";

// note this is same as uigraphic's frag shader, minus colorization removed.
const static char s_frameFragShader[] =
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
void NvGraphicFrameShader::Load(const char* vs, const char* fs)
{
    INHERITED::Load(vs, fs);

    // inherited Load doesn't keep program enabled for 'safety', so we
    // re-enable here so we can reference ourselves...
    m_program->enable();

    m_borderIndex = m_program->getAttribLocation("border");

    glUniform1i(m_program->getUniformLocation("sampler"), 0); // texunit index zero.

    m_thicknessIndex = m_program->getUniformLocation("thickness");
    m_texBorderIndex = m_program->getUniformLocation("texBorder");

    m_program->disable();
}


//======================================================================
//======================================================================
NvUIGraphicFrame::NvUIGraphicFrame(const std::string& texname, float border)
    : NvUIGraphic(texname), m_drawCenter(true)
{
    m_texBorder.x = border;
    m_texBorder.y = border;
    m_borderThickness.x = border;
    m_borderThickness.y = border;
    StaticInit();
}

//======================================================================
//======================================================================
NvUIGraphicFrame::NvUIGraphicFrame(const std::string& texname, float borderX, float borderY)
    : NvUIGraphic(texname), m_drawCenter(true)
{
    m_texBorder.x = borderX;
    m_texBorder.y = borderY;
    m_borderThickness.x = borderX;
    m_borderThickness.y = borderY;
    StaticInit();
}


//======================================================================
//======================================================================
NvUIGraphicFrame::NvUIGraphicFrame(NvUITexture *uiTex, float border)
    : NvUIGraphic(uiTex), m_drawCenter(true)
{
    m_texBorder.x = border;
    m_texBorder.y = border;
    m_borderThickness.x = border;
    m_borderThickness.y = border;
    StaticInit();
}


//======================================================================
//======================================================================
NvUIGraphicFrame::~NvUIGraphicFrame()
{
    StaticCleanup();
}

void NvUIGraphicFrame::StaticCleanup()
{
    if (--s_staticCount == 0)
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glUseProgram(0);

        delete ms_shader.m_program;
        ms_shader.m_program = 0;

        glDeleteBuffers(1, &ms_gfvbo);
        glDeleteBuffers(1, &ms_gfibo);
        ms_gfvbo = 0;
        ms_gfibo = 0;
    }
}

//======================================================================
//======================================================================
bool NvUIGraphicFrame::LoadTexture(const std::string& texname, bool resetDimensions/*==true*/)
{
    // We pass false to inherited call to be explicit to leave our dimensions alone, and
    // not change them to match the texture's size, as frames inherently 'stretch' at draw
    // time to match the destination size.
    if (INHERITED::LoadTexture(texname, false))
        return true;    
    return false;
}


//======================================================================
//======================================================================
bool NvUIGraphicFrame::StaticInit()
{
    s_staticCount++;
    
    if (!ms_gfvbo)
    {
        // TODO: disable drawing of the center piece?
        NvFrameVertex vert[4*4];
        NvFrameVertex temp[4] =
        {
            {-1,  1, 0, 1, 0, 0},
            { 1,  1, 1, 1, 0, 0},
            {-1, -1, 0, 0, 0, 0},
            { 1, -1, 1, 0, 0, 0}
        };

        for (int32_t y = 0; y < 4; y++)
        {
            for (int32_t x = 0; x < 4; x++)
            {
                int32_t src = ((y >> 1)*2) + (x >> 1);  //0,0,1,1,0,0,1,1,2,2,3,3,2,2,3,3
                int32_t dst = y*4+x;                    //0,1,2,3,4,5,6,7...
                memcpy(&vert[dst], &temp[src], sizeof(NvFrameVertex));
                if (y == 0 || y == 3)
                    vert[dst].by = 1;
                if (x == 0 || x == 3)
                    vert[dst].bx = 1;
            }
        }

        uint16_t indices[30+6] = {
                  0, 4, 1, 5, 2, 6, 3, 7,         7, // first row
          4,      4, 8, 5, 9,                     9, // left panel
          6,      6, 10, 7, 11, 11, // right panel
          8,      8, 12, 9, 13, 10, 14, 11, 15,   15, // bottom
          5,      5, 9, 6, 10 // center piece
        };

        ms_shader.Load(s_frameVertShader, s_frameFragShader);

        CHECK_GL_ERROR();

        glGenBuffers(1, &ms_gfibo);
        glGenBuffers(1, &ms_gfvbo);

        glBindBuffer(GL_ARRAY_BUFFER, ms_gfvbo);
        glBufferData(GL_ARRAY_BUFFER, 4 * 4 * sizeof(NvFrameVertex), vert, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ms_gfibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (30+6) * sizeof(uint16_t),
            indices, GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        // The following entries are const
        // so we set them up now and never change
        s_gfpixelToClipMatrix[2][0] = 0.0f;
        s_gfpixelToClipMatrix[2][1] = 0.0f;

        s_gfpixelToClipMatrix[0][2] = 0.0f;
        s_gfpixelToClipMatrix[1][2] = 0.0f;
        s_gfpixelToClipMatrix[2][2] = 1.0f;
        s_gfpixelToClipMatrix[3][2] = 0.0f;

        s_gfpixelToClipMatrix[0][3] = 0.0f;
        s_gfpixelToClipMatrix[1][3] = 0.0f;
        s_gfpixelToClipMatrix[2][3] = 0.0f;
        s_gfpixelToClipMatrix[3][3] = 1.0f;
    }

    CHECK_GL_ERROR();

    return true;
}

//======================================================================
//======================================================================
void NvUIGraphicFrame::SetBorderThickness(float thickness)
{
    m_borderThickness.x = thickness;
    m_borderThickness.y = thickness;
}

//======================================================================
//======================================================================
void NvUIGraphicFrame::SetBorderThickness(float width, float height)
{
    m_borderThickness.x = width;
    m_borderThickness.y = height;
}


//======================================================================
//======================================================================
void NvUIGraphicFrame::GetBorderThickness(float *x, float *y)
{
    if (x)
        *x = m_borderThickness.x;
    if (y)
        *y = m_borderThickness.y;
}


//======================================================================
//======================================================================
void NvUIGraphicFrame::SetDrawCenter(bool drawCenter)
{
    m_drawCenter = drawCenter;
}

//======================================================================
//======================================================================
void NvUIGraphicFrame::Draw(const NvUIDrawState &drawState)
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
    if (s_gfpixelXLast != designWidth)
    {
        s_gfpixelXLast = (int32_t)designWidth;
        s_gfpixelScaleFactorX = 2.0f / s_gfpixelXLast;
    }
    if (s_gfpixelYLast != designHeight)
    {
        s_gfpixelYLast = (int32_t)designHeight;
        s_gfpixelScaleFactorY = 2.0f / s_gfpixelYLast;
    }

    float rad = (float)(drawState.rotation / 180.0f * 3.14159f); // [-1,2]=>[-90,180] in radians...
    float cosf = (float)cos(rad);
    float sinf = (float)sin(rad);

    const float wNorm = s_gfpixelScaleFactorX;
    const float hNorm = s_gfpixelScaleFactorY;

    s_gfpixelToClipMatrix[0][0] = wNorm * m_rect.width  * cosf;
    s_gfpixelToClipMatrix[1][0] = hNorm * m_rect.height * -sinf;
    s_gfpixelToClipMatrix[0][1] = wNorm * m_rect.width  * sinf;
    s_gfpixelToClipMatrix[1][1] = hNorm * m_rect.height * cosf;

    s_gfpixelToClipMatrix[3][0] = ( wNorm * m_rect.left - 1) * cosf
                              - ( 1 - hNorm * (m_rect.top + m_rect.height))  * sinf;
    s_gfpixelToClipMatrix[3][1] = ( wNorm * m_rect.left - 1 ) * sinf
                              + ( 1 - hNorm * (m_rect.top + m_rect.height))  * cosf;

    glUniformMatrix4fv(ms_shader.m_matrixIndex, 1, GL_FALSE, &(s_gfpixelToClipMatrix[0][0]));

    nv::vec2<float> thickness;
    thickness.x = m_borderThickness.x; 
    thickness.y = m_borderThickness.y;
    if (thickness.x > m_rect.width / 2)
        thickness.x = m_rect.width / 2;
    if (thickness.y > m_rect.height / 2)
        thickness.y = m_rect.height / 2;
    thickness.x /= m_rect.width/2;
    thickness.y /= m_rect.height/2;

    glUniform2f(ms_shader.m_texBorderIndex,
                    m_texBorder.x / m_tex->GetWidth(),
                    m_texBorder.y / m_tex->GetHeight());
    glUniform2f(ms_shader.m_thicknessIndex, thickness.x, thickness.y);

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

//    glUniform1i(shader.m_samplerIndex, 0); // texunit index zero.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_tex->GetGLTex());

    // setup data buffers/attribs.
    glBindBuffer(GL_ARRAY_BUFFER, ms_gfvbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ms_gfibo);

    glVertexAttribPointer(ms_shader.m_positionIndex, 2, GL_FLOAT, 0, sizeof(NvFrameVertex), 0);
    glEnableVertexAttribArray(ms_shader.m_positionIndex);
    glVertexAttribPointer(ms_shader.m_uvIndex, 2, GL_FLOAT, 0, sizeof(NvFrameVertex), (void*) (2* 4));
    glEnableVertexAttribArray(ms_shader.m_uvIndex);
    glVertexAttribPointer(ms_shader.m_borderIndex, 2, GL_FLOAT, 0, sizeof(NvFrameVertex), (void*) (2*2* 4));
    glEnableVertexAttribArray(ms_shader.m_borderIndex);

    // draw it already!
    glDrawElements(GL_TRIANGLE_STRIP, m_drawCenter ? (30+6) : 30, GL_UNSIGNED_SHORT, 0);

    //nv_flush_tracked_attribs();
    glDisableVertexAttribArray(ms_shader.m_positionIndex);
    glDisableVertexAttribArray(ms_shader.m_uvIndex);
    glDisableVertexAttribArray(ms_shader.m_borderIndex);

    if (ae)
        glDisable(GL_BLEND);
}
