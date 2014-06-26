//----------------------------------------------------------------------------------
// File:        ComputeWaterSimulation/WaveSimThread.cpp
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
#include "WaveSimThread.h"
#include "NV/NvStopWatch.h"


//TODO: remove
//#pragma comment(lib, "r3d")


int WaveSimThread::g_runningThreads = 0;
int WaveSimThread::g_threadsCounter = 0;
r3::Condition WaveSimThread::g_globalWaveSimThreadCondition;
NvStopWatch* WaveSimThread::m_threadStopWatch = NULL;

WaveSimThread::WaveSimThread(WaveSim *sim, NvStopWatch* stopWatch)
:m_simulation(sim), m_threadId(++WaveSimThread::g_threadsCounter), m_run(true)
{
	m_threadStopWatch = stopWatch;
}

void WaveSimThread::runSimulation()
{
	m_localStartCondition.Acquire();
	m_localStartCondition.Signal();
	m_localStartCondition.Release();
}

void WaveSimThread::waitForAllThreads()
{
	g_globalWaveSimThreadCondition.Acquire();
	while(g_runningThreads>0)
	{
		g_globalWaveSimThreadCondition.Wait();
	}
	g_globalWaveSimThreadCondition.Release();
}

void WaveSimThread::pauseAllThreads()
{
	g_globalWaveSimThreadCondition.Acquire();
	g_runningThreads = 0;
	g_globalWaveSimThreadCondition.Release();
}

void WaveSimThread::Run()
{
	while(m_run)
	{
		g_globalWaveSimThreadCondition.Acquire();
		g_runningThreads++;
		g_globalWaveSimThreadCondition.Release();

		m_startTime = m_threadStopWatch->getTime();

		//TODO:
		float timestep = 1.0f;

		m_simulation->simulate(timestep);
		m_simulation->calcGradients();

		m_endTime = m_threadStopWatch->getTime();
		//LOGI("Thread %d = [start, end] - Exec Time : [%f, %f] - %f\n", id, startTime, endTime, endTime - startTime);

		g_globalWaveSimThreadCondition.Acquire();
		g_runningThreads--;
		g_globalWaveSimThreadCondition.Signal();
		g_globalWaveSimThreadCondition.Release();

		m_localStartCondition.Acquire();
		m_localStartCondition.Wait();
		m_localStartCondition.Release();
	}
	
	//printf("Thread %d has finished.\n", id);

	g_globalWaveSimThreadCondition.Acquire();
	g_runningThreads--;
	g_globalWaveSimThreadCondition.Signal();
	g_globalWaveSimThreadCondition.Release();
}

void WaveSimThread::runSingleSimulationOnCallingThread()
{
	g_globalWaveSimThreadCondition.Acquire();
	g_runningThreads++;
	g_globalWaveSimThreadCondition.Release();

	m_startTime = m_threadStopWatch->getTime();

	//TODO:
	float timestep = 1.0f;

	m_simulation->simulate(timestep);
	m_simulation->calcGradients();

	m_endTime = m_threadStopWatch->getTime();
		//LOGI("Thread %d = [start, end] - Exec Time : [%f, %f] - %f\n", id, startTime, endTime, endTime - startTime);

	g_globalWaveSimThreadCondition.Acquire();
	g_runningThreads--;
	g_globalWaveSimThreadCondition.Signal();
	g_globalWaveSimThreadCondition.Release();
}

WaveSimThread::~WaveSimThread()
{
	g_globalWaveSimThreadCondition.Acquire();
	g_threadsCounter--;
	g_globalWaveSimThreadCondition.Release();
}
