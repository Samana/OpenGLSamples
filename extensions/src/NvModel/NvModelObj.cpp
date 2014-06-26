//----------------------------------------------------------------------------------
// File:        NvModel/NvModelObj.cpp
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

#include "NvModel/NvModel.h"
#include <assert.h>
#include <iostream>
#include <sstream>
#include <stdio.h>

#include <NV/NvTokenizer.h>

using std::vector;

bool NvModel::loadObjFromFileData( char *fileData, NvModel &m)
{
    NvTokenizer tok(fileData, "/");
    
    float val[4];
    int32_t idx[3][3];
    int32_t match;
    char format = 0;
    bool vtx4Comp = false;
    bool tex3Comp = false;
    bool hasTC = false;
    bool hasNormals = false;

#if 0 //def _DEBUG
    tok.setVerbose();
    LOGI("NvModel::loadObjFromFileData..");
#endif

    while ( !tok.atEOF() )
    {
        if (!tok.readToken()) {
            tok.consumeToEOL();
            continue; // likely EOL we didn't explicitly handle?
        }

        const char* tmp = tok.getLastTokenPtr();
        switch (tmp[0]) {
            case '#':
                //comment line, eat the remainder
                tok.consumeToEOL();
                break;

            case 'v':
                switch (tmp[1]) {
                    case '\0':
                        //vertex, 3 or 4 components
                        val[3] = 1.0f;  //default w coordinate
                        match = tok.getTokenFloatArray(val, 4);
                        m._positions.push_back( val[0]);
                        m._positions.push_back( val[1]);
                        m._positions.push_back( val[2]);
                        m._positions.push_back( val[3]);
                        vtx4Comp |= ( match == 4);
                        assert( match > 2 && match < 5);
                        break;

                    case 'n':
                        //normal, 3 components
                        match = tok.getTokenFloatArray(val, 3);
                        m._normals.push_back( val[0]);
                        m._normals.push_back( val[1]);
                        m._normals.push_back( val[2]);
                        assert( match == 3);
                        break;

                    case 't':
                        //texcoord, 2 or 3 components
                        val[2] = 0.0f;  //default r coordinate
                        match = tok.getTokenFloatArray(val, 3);
                        m._texCoords.push_back( val[0]);
                        m._texCoords.push_back( val[1]);
                        m._texCoords.push_back( val[2]);
                        tex3Comp |= ( match == 3);
                        assert( match > 1 && match < 4);
                        break;
                }
                break;

            case 'f':
            {
                //face
                // determine the type, and read the initial vertex, all entries in a face must have the same format
                // formats are:
                // 1  #
                // 2  #/#
                // 3  #/#/#
                // 4  #//#

                // we need to 'hand read' the first run, to decode the formatting.
                format = 0;
                if (!tok.getTokenInt(idx[0][0])) {
                    assert(0);
                    return false;
                }
                // on our way.
                format = 1;
                if (tok.consumeOneDelim()) {
                    if (tok.consumeOneDelim()) {
                        // automatically format 4.
                        format = 4;
                    }
                    if (!tok.getTokenInt(idx[0][1])) {
                        assert(0);
                        return false;
                    }
                    format = 2; // at least format 2.
                    tok.setConsumeWS(false);
                    if (tok.consumeOneDelim()) {
                        if (tok.getTokenInt(idx[0][2])) {
                            // automatically format 3
                            format = 3;
                        }
                        // else remain format 2, in case of "#/#/" wacky format.
                    }
                    tok.setConsumeWS(true);
                }

                switch (format) {
                    case 1: // #
                    { //This face has only vertex indices
                        //remap them to the right spot
                        idx[0][0] = (idx[0][0] > 0) ? (idx[0][0] - 1) : ((int32_t)m._positions.size() - idx[0][0]);

                        //grab the second vertex to prime
                        tok.getTokenInt(idx[1][0]);

                        //remap them to the right spot
                        idx[1][0] = (idx[1][0] > 0) ? (idx[1][0] - 1) : ((int32_t)m._positions.size() - idx[1][0]);

                        while ( tok.getTokenInt(idx[2][0]) ) {
                            //remap them to the right spot
                            idx[2][0] = (idx[2][0] > 0) ? (idx[2][0] - 1) : ((int32_t)m._positions.size() - idx[2][0]);

                            //add the indices
                            for (int32_t ii = 0; ii < 3; ii++) {
                                m._pIndex.push_back( idx[ii][0]);
                                m._tIndex.push_back( 0); //dummy index to keep things in synch
                                m._nIndex.push_back( 0); //dummy normal index to keep everything in synch
                            }

                            //prepare for the next iteration
                            idx[1][0] = idx[2][0];
                        }

                        break;
                    }

                    case 2: // #/#
                    { //This face has vertex and texture coordinate indices
                        //remap them to the right spot
                        idx[0][0] = (idx[0][0] > 0) ? (idx[0][0] - 1) : ((int32_t)m._positions.size() - idx[0][0]);
                        idx[0][1] = (idx[0][1] > 0) ? (idx[0][1] - 1) : ((int32_t)m._texCoords.size() - idx[0][1]);

                        //grab the second vertex to prime
                        tok.getTokenIntArray(idx[1], 2);

                        //remap them to the right spot
                        idx[1][0] = (idx[1][0] > 0) ? (idx[1][0] - 1) : ((int32_t)m._positions.size() - idx[1][0]);
                        idx[1][1] = (idx[1][1] > 0) ? (idx[1][1] - 1) : ((int32_t)m._texCoords.size() - idx[1][1]);

                        while ( tok.getTokenIntArray(idx[2], 2) == 2) {
                            //remap them to the right spot
                            idx[2][0] = (idx[2][0] > 0) ? (idx[2][0] - 1) : ((int32_t)m._positions.size() - idx[2][0]);
                            idx[2][1] = (idx[2][1] > 0) ? (idx[2][1] - 1) : ((int32_t)m._texCoords.size() - idx[2][1]);

                            //add the indices
                            for (int32_t ii = 0; ii < 3; ii++) {
                                m._pIndex.push_back( idx[ii][0]);
                                m._tIndex.push_back( idx[ii][1]);
                                m._nIndex.push_back( 0); //dummy normal index to keep everything in synch
                            }

                            //prepare for the next iteration
                            idx[1][0] = idx[2][0];
                            idx[1][1] = idx[2][1];
                        }

                        hasTC = true;
                        break;
                    }

                    case 3: // #/#/#
                    { //This face has vertex, texture coordinate, and normal indices
                        //remap them to the right spot
                        idx[0][0] = (idx[0][0] > 0) ? (idx[0][0] - 1) : ((int32_t)m._positions.size() - idx[0][0]);
                        idx[0][1] = (idx[0][1] > 0) ? (idx[0][1] - 1) : ((int32_t)m._texCoords.size() - idx[0][1]);
                        idx[0][2] = (idx[0][2] > 0) ? (idx[0][2] - 1) : ((int32_t)m._normals.size() - idx[0][2]);

                        //grab the second vertex to prime
                        tok.getTokenIntArray(idx[1], 3);

                        //remap them to the right spot
                        idx[1][0] = (idx[1][0] > 0) ? (idx[1][0] - 1) : ((int32_t)m._positions.size() - idx[1][0]);
                        idx[1][1] = (idx[1][1] > 0) ? (idx[1][1] - 1) : ((int32_t)m._texCoords.size() - idx[1][1]);
                        idx[1][2] = (idx[1][2] > 0) ? (idx[1][2] - 1) : ((int32_t)m._normals.size() - idx[1][2]);

                        //create the fan
                        while ( tok.getTokenIntArray(idx[2], 3) == 3) {
                            //remap them to the right spot
                            idx[2][0] = (idx[2][0] > 0) ? (idx[2][0] - 1) : ((int32_t)m._positions.size() - idx[2][0]);
                            idx[2][1] = (idx[2][1] > 0) ? (idx[2][1] - 1) : ((int32_t)m._texCoords.size() - idx[2][1]);
                            idx[2][2] = (idx[2][2] > 0) ? (idx[2][2] - 1) : ((int32_t)m._normals.size() - idx[2][2]);

                            //add the indices
                            for (int32_t ii = 0; ii < 3; ii++) {
                                m._pIndex.push_back( idx[ii][0]);
                                m._tIndex.push_back( idx[ii][1]);
                                m._nIndex.push_back( idx[ii][2]);
                            }

                            //prepare for the next iteration
                            idx[1][0] = idx[2][0];
                            idx[1][1] = idx[2][1];
                            idx[1][2] = idx[2][2];
                        }

                        hasTC = true;
                        hasNormals = true;
                        break;
                    }

                    case 4: // #//#
                    { //This face has vertex and normal indices
                        //remap them to the right spot
                        idx[0][0] = (idx[0][0] > 0) ? (idx[0][0] - 1) : ((int32_t)m._positions.size() - idx[0][0]);
                        idx[0][1] = (idx[0][1] > 0) ? (idx[0][1] - 1) : ((int32_t)m._normals.size() - idx[0][1]);

                        //grab the second vertex to prime
                        tok.getTokenIntArray(idx[1], 2);

                        //remap them to the right spot
                        idx[1][0] = (idx[1][0] > 0) ? (idx[1][0] - 1) : ((int32_t)m._positions.size() - idx[1][0]);
                        idx[1][1] = (idx[1][1] > 0) ? (idx[1][1] - 1) : ((int32_t)m._normals.size() - idx[1][1]);

                        //create the fan
                        while ( tok.getTokenIntArray(idx[2], 2) == 2) {
                            //remap them to the right spot
                            idx[2][0] = (idx[2][0] > 0) ? (idx[2][0] - 1) : ((int32_t)m._positions.size() - idx[2][0]);
                            idx[2][1] = (idx[2][1] > 0) ? (idx[2][1] - 1) : ((int32_t)m._normals.size() - idx[2][1]);

                            //add the indices
                            for (int32_t ii = 0; ii < 3; ii++) {
                                m._pIndex.push_back( idx[ii][0]);
                                m._nIndex.push_back( idx[ii][1]);
                                m._tIndex.push_back(0); // dummy index, to ensure that the buffers are of identical size
                            }

                            //prepare for the next iteration
                            idx[1][0] = idx[2][0];
                            idx[1][1] = idx[2][1];
                        }

                        hasNormals = true;
                        break;
                    }

                    default:
                        assert(0);
                        return false;
                }

/*
                if ( sscanf( buf2, "%d//%d", &idx[0][0], &idx[0][1]) == 2) 
                { //This face has vertex and normal indices
                    //remap them to the right spot
                    idx[0][0] = (idx[0][0] > 0) ? (idx[0][0] - 1) : ((int32_t)m._positions.size() - idx[0][0]);
                    idx[0][1] = (idx[0][1] > 0) ? (idx[0][1] - 1) : ((int32_t)m._normals.size() - idx[0][1]);

                    //grab the second vertex to prime
                    buf2[0] = '\0';
                    faceStringStream>>buf2;
                    //LOGI(" %s", buf2);
                    sscanf(buf2, "%d//%d", &idx[1][0], &idx[1][1]);

                    //remap them to the right spot
                    idx[1][0] = (idx[1][0] > 0) ? (idx[1][0] - 1) : ((int32_t)m._positions.size() - idx[1][0]);
                    idx[1][1] = (idx[1][1] > 0) ? (idx[1][1] - 1) : ((int32_t)m._normals.size() - idx[1][1]);

                    buf2[0] = '\0';
                    faceStringStream>>buf2;
                    //LOGI(" %s", buf2);
                    //create the fan
                    while ( sscanf(buf2, "%d//%d", &idx[2][0], &idx[2][1]) == 2) {
                        //remap them to the right spot
                        idx[2][0] = (idx[2][0] > 0) ? (idx[2][0] - 1) : ((int32_t)m._positions.size() - idx[2][0]);
                        idx[2][1] = (idx[2][1] > 0) ? (idx[2][1] - 1) : ((int32_t)m._normals.size() - idx[2][1]);

                        //add the indices
                        for (int32_t ii = 0; ii < 3; ii++) {
                            m._pIndex.push_back( idx[ii][0]);
                            m._nIndex.push_back( idx[ii][1]);
                            m._tIndex.push_back(0); // dummy index, to ensure that the buffers are of identical size
                        }

                        //prepare for the next iteration
                        idx[1][0] = idx[2][0];
                        idx[1][1] = idx[2][1];

                        buf2[0] = '\0';
                        faceStringStream>>buf2;
                        //LOGI("* %s\n", buf2);
                    }
                    hasNormals = true;
                }
                else if ( sscanf( buf2, "%d/%d/%d", &idx[0][0], &idx[0][1], &idx[0][2]) == 3)
                { //This face has vertex, texture coordinate, and normal indices
                    //remap them to the right spot
                    idx[0][0] = (idx[0][0] > 0) ? (idx[0][0] - 1) : ((int32_t)m._positions.size() - idx[0][0]);
                    idx[0][1] = (idx[0][1] > 0) ? (idx[0][1] - 1) : ((int32_t)m._texCoords.size() - idx[0][1]);
                    idx[0][2] = (idx[0][2] > 0) ? (idx[0][2] - 1) : ((int32_t)m._normals.size() - idx[0][2]);

                    //grab the second vertex to prime
                    buf2[0] = '\0';
                    faceStringStream>>buf2;
                    //LOGI(" %s\n", buf2);
                    sscanf(buf2, "%d/%d/%d", &idx[1][0], &idx[1][1], &idx[1][2]);
                    //fscanf( fp, "%d/%d/%d", &idx[1][0], &idx[1][1], &idx[1][2]);

                    //remap them to the right spot
                    idx[1][0] = (idx[1][0] > 0) ? (idx[1][0] - 1) : ((int32_t)m._positions.size() - idx[1][0]);
                    idx[1][1] = (idx[1][1] > 0) ? (idx[1][1] - 1) : ((int32_t)m._texCoords.size() - idx[1][1]);
                    idx[1][2] = (idx[1][2] > 0) ? (idx[1][2] - 1) : ((int32_t)m._normals.size() - idx[1][2]);

                    buf2[0] = '\0';
                    faceStringStream>>buf2;
                    //LOGI(" %s\n", buf2);
                    //create the fan
                    while ( sscanf( buf2, "%d/%d/%d", &idx[2][0], &idx[2][1], &idx[2][2]) == 3) {
                        //remap them to the right spot
                        idx[2][0] = (idx[2][0] > 0) ? (idx[2][0] - 1) : ((int32_t)m._positions.size() - idx[2][0]);
                        idx[2][1] = (idx[2][1] > 0) ? (idx[2][1] - 1) : ((int32_t)m._texCoords.size() - idx[2][1]);
                        idx[2][2] = (idx[2][2] > 0) ? (idx[2][2] - 1) : ((int32_t)m._normals.size() - idx[2][2]);

                        //add the indices
                        for (int32_t ii = 0; ii < 3; ii++) {
                            m._pIndex.push_back( idx[ii][0]);
                            m._tIndex.push_back( idx[ii][1]);
                            m._nIndex.push_back( idx[ii][2]);
                        }

                        //prepare for the next iteration
                        idx[1][0] = idx[2][0];
                        idx[1][1] = idx[2][1];
                        idx[1][2] = idx[2][2];

                        buf2[0] = '\0';
                        faceStringStream>>buf2;
                        //LOGI(" %s\n", buf2);
                    }

                    hasTC = true;
                    hasNormals = true;
                }
                else if ( sscanf( buf2, "%d/%d", &idx[0][0], &idx[0][1]) == 2)
                { //This face has vertex and texture coordinate indices
                    //remap them to the right spot
                    idx[0][0] = (idx[0][0] > 0) ? (idx[0][0] - 1) : ((int32_t)m._positions.size() - idx[0][0]);
                    idx[0][1] = (idx[0][1] > 0) ? (idx[0][1] - 1) : ((int32_t)m._texCoords.size() - idx[0][1]);

                    //grab the second vertex to prime
                    buf2[0] = '\0';
                    faceStringStream>>buf2;
                    //LOGI(" %s\n", buf2);
                    sscanf( buf2, "%d/%d", &idx[1][0], &idx[1][1]);

                    //remap them to the right spot
                    idx[1][0] = (idx[1][0] > 0) ? (idx[1][0] - 1) : ((int32_t)m._positions.size() - idx[1][0]);
                    idx[1][1] = (idx[1][1] > 0) ? (idx[1][1] - 1) : ((int32_t)m._texCoords.size() - idx[1][1]);

                    buf2[0] = '\0';
                    faceStringStream>>buf2;
                    //LOGI(" %s\n", buf2);
                    //create the fan
                    while ( sscanf( buf2, "%d/%d", &idx[2][0], &idx[2][1]) == 2) {
                        //remap them to the right spot
                        idx[2][0] = (idx[2][0] > 0) ? (idx[2][0] - 1) : ((int32_t)m._positions.size() - idx[2][0]);
                        idx[2][1] = (idx[2][1] > 0) ? (idx[2][1] - 1) : ((int32_t)m._texCoords.size() - idx[2][1]);

                        //add the indices
                        for (int32_t ii = 0; ii < 3; ii++) {
                            m._pIndex.push_back( idx[ii][0]);
                            m._tIndex.push_back( idx[ii][1]);
                            m._nIndex.push_back( 0); //dummy normal index to keep everything in synch
                        }

                        //prepare for the next iteration
                        idx[1][0] = idx[2][0];
                        idx[1][1] = idx[2][1];

                        buf2[0] = '\0';
                        faceStringStream>>buf2;
                        //LOGI(" %s\n", buf2);
                    }
                    hasTC = true;
                }
                else if ( sscanf( buf2, "%d", &idx[0][0]) == 1)
                {
                    //This face has only vertex indices

                    //remap them to the right spot
                    idx[0][0] = (idx[0][0] > 0) ? (idx[0][0] - 1) : ((int32_t)m._positions.size() - idx[0][0]);

                    //grab the second vertex to prime
                    buf2[0] = '\0';
                    faceStringStream>>buf2;
                    sscanf( buf2, "%d", &idx[1][0]);

                    //remap them to the right spot
                    idx[1][0] = (idx[1][0] > 0) ? (idx[1][0] - 1) : ((int32_t)m._positions.size() - idx[1][0]);

                    buf2[0] = '\0';
                    faceStringStream>>buf2;
                    //LOGI(" %s\n", buf2);
                    //create the fan
                    while ( sscanf( buf2, "%d", &idx[2][0]) == 1) {
                        //remap them to the right spot
                        idx[2][0] = (idx[2][0] > 0) ? (idx[2][0] - 1) : ((int32_t)m._positions.size() - idx[2][0]);

                        //add the indices
                        for (int32_t ii = 0; ii < 3; ii++) {
                            m._pIndex.push_back( idx[ii][0]);
                            m._tIndex.push_back( 0); //dummy index to keep things in synch
                            m._nIndex.push_back( 0); //dummy normal index to keep everything in synch
                        }

                        //prepare for the next iteration
                        idx[1][0] = idx[2][0];

                        buf2[0] = '\0';
                        faceStringStream>>buf2;
                        //LOGI(" %s\n", buf2);
                    }
                }
                else {
                    //bad format
                    assert(0);
                    tok.consumeToEOL();
                }
*/
            }
            break;

            case 's':
            case 'g':
            case 'u':
                //all presently ignored
            default:
                tok.consumeToEOL();
        };
    }

    //post-process data

    //free anything that ended up being unused
    if (!hasNormals) {
        m._normals.clear();
        m._nIndex.clear();
    }

    if (!hasTC) {
        m._texCoords.clear();
        m._tIndex.clear();
    }

    //set the defaults as the worst-case for an obj file
    m._posSize = 4;
    m._tcSize = 3;

    //compact to 3 component vertices if possible
    if (!vtx4Comp) {
        vector<float>::iterator src = m._positions.begin();
        vector<float>::iterator dst = m._positions.begin();

        for ( ; src < m._positions.end(); ) {
            *(dst++) = *(src++);
            *(dst++) = *(src++);
            *(dst++) = *(src++);
            src++;
        }

        m._positions.resize( (m._positions.size() / 4) * 3);

        m._posSize = 3;
    }

    //compact to 2 component tex coords if possible
    if (!tex3Comp) {
        vector<float>::iterator src = m._texCoords.begin();
        vector<float>::iterator dst = m._texCoords.begin();

        for ( ; src < m._texCoords.end(); ) {
            *(dst++) = *(src++);
            *(dst++) = *(src++);
            src++;
        }

        m._texCoords.resize( (m._texCoords.size() / 3) * 2);

        m._tcSize = 2;
    }

//    LOGI("Positions \n");
//
//    std::vector<float>::iterator it;
//    for(it = m._positions.begin(); it<m._positions.end(); it++)
//    {
//        LOGI("%f\n", *(it));
//    }

    return true;
}
