/* 
 Copyright (c) 2010 Cass Everitt
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or
 without modification, are permitted provided that the following
 conditions are met:
 
 * Redistributions of source code must retain the above
 copyright notice, this list of conditions and the following
 disclaimer.
 
 * Redistributions in binary form must reproduce the above
 copyright notice, this list of conditions and the following
 disclaimer in the documentation and/or other materials
 provided with the distribution.
 
 * The names of contributors to this software may not be used
 to endorse or promote products derived from this software
 without specific prior written permission. 
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 POSSIBILITY OF SUCH DAMAGE. 


 Cass Everitt
 */

#include <stdio.h>
#include "R3/thread.h"

using namespace r3;

namespace  {
	Mutex mainThreadMutex;
	int numThreads;
	
}

#if USE_WIN32_THREADS
//DWORD WINAPI r3ThreadStart( LPVOID data )
unsigned __stdcall r3ThreadStart( void *data )
#else
void *r3ThreadStart( void *data )
#endif
{
	Thread * thread = static_cast< Thread * > ( data );

	mainThreadMutex.Acquire();
	numThreads++;
	mainThreadMutex.Release();
	
	thread->Run();
	
	mainThreadMutex.Acquire();
	numThreads--;
	//thread->running = false;
	mainThreadMutex.Release();

#if USE_WIN32_THREADS
	_endthread();
#else
	pthread_exit(NULL);
#endif
	return NULL;
}

namespace r3 {

	void Thread::Start() {
		ScopedMutex m( mainThreadMutex );
		if ( running ) {
			return;
		}
		running = true;
#if USE_WIN32_THREADS
		//threadHandle = CreateThread( NULL, 0, r3ThreadStart, this, 0, NULL );
		threadHandle = (HANDLE) _beginthreadex(NULL, 0, r3ThreadStart, this, 0, &threadId);
		//int status = SetThreadPriority(threadHandle, THREAD_PRIORITY_HIGHEST);
#else
		pthread_create( &threadId, NULL, r3ThreadStart, this );
#endif
	}

	void Thread::WaitForExit()
	{
#if USE_WIN32_THREADS

#else
		pthread_join(threadId, NULL);
#endif
	}

	int getNumRunningThreads()
	{
		int n;
		mainThreadMutex.Acquire();
		n = numThreads;
		mainThreadMutex.Release();
		return n;
	}
}
