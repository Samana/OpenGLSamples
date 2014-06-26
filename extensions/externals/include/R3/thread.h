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

#ifndef __R3_THREAD_H__
#define __R3_THREAD_H__

//#define USE_WIN32_THREADS 1
#define USE_WIN32_THREADS _WIN32

#if USE_WIN32_THREADS
# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN 1
# endif
# include <windows.h>
# include <process.h>	// for _beginthreadex()
#else
# include <pthread.h>
#endif



namespace r3 {
	
	void InitThread();

	class Thread {
#if USE_WIN32_THREADS
		HANDLE threadHandle;
		unsigned int threadId;
#else
		pthread_t threadId;
#endif
	public:
		Thread() : running( false ) {
		}
		bool running;
		void Start();
		void WaitForExit();
		virtual void Run() = 0;
	};

	int getNumRunningThreads();

	int getNumCPUCores();

#if USE_WIN32_THREADS
#if 0
	class Mutex {
		HANDLE mutex;
	public:
		Mutex() {
			mutex = CreateMutex( 0, FALSE, 0 ); 
		}
		~Mutex() {
			CloseHandle( mutex );
		}
		void Acquire() {
			WaitForSingleObject( mutex, INFINITE );
		}
		void Release() {
			ReleaseMutex( mutex );
		}
	};
#else
	// critical sections are faster than mutexes, apparently
	class Mutex {
    protected:
		CRITICAL_SECTION criticalSection;
	public:
		Mutex() {
			//InitializeCriticalSectionAndSpinCount(&criticalSection, 0x80000400);
            InitializeCriticalSection(&criticalSection);
		}
		~Mutex() {
			DeleteCriticalSection(&criticalSection);
		}
		void Acquire() {
			EnterCriticalSection(&criticalSection); 
		}
		void Release() {
			LeaveCriticalSection(&criticalSection);
		}
	};    
#endif

    // condition variable
    class Condition : public Mutex {
        CONDITION_VARIABLE cond;
    public:
        Condition() {
            InitializeConditionVariable(&cond);
        }
        ~Condition() {
        }
		// wait on condition
		// - releases mutex, causes calling thread to block until condition is signalled
		// - returns with mutex locked
        void Wait() {
            SleepConditionVariableCS(&cond, &criticalSection, INFINITE);
            //SleepConditionVariableSRW(&cond, &lock, INFINITE, 0);
            //SleepConditionVariableSRW(&cond, &lock, INFINITE, CONDITION_VARIABLE_LOCKMODE_SHARED);
        }
        void Signal() {
            WakeConditionVariable(&cond);
        }
        void Broadcast() {
            WakeAllConditionVariable(&cond);
        }
    };

#else
	// Pthread versions

	class Mutex {
	protected:
		pthread_mutex_t mutex;
	public:
		Mutex() {
			pthread_mutexattr_t attr;
			pthread_mutexattr_init(&attr);
			pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

			pthread_mutex_init( &mutex, &attr );
			//pthread_mutex_init( &mutex, NULL );

			pthread_mutexattr_destroy(&attr);
		}
        ~Mutex() {
            pthread_mutex_destroy( &mutex );
        }
		void Acquire() {
			pthread_mutex_lock( &mutex );
		}
		void Release() {
			pthread_mutex_unlock( &mutex );
		}
	};

    class Condition : public Mutex {
        pthread_cond_t cond;
    public:
        Condition() {
            pthread_cond_init(&cond, NULL);
        }
        ~Condition() {
            pthread_cond_destroy(&cond);
        }
        void Wait() {
			pthread_cond_wait(&cond, &mutex);
        }
        void Signal() {
            pthread_cond_signal(&cond);
        }
        void Broadcast() {
            pthread_cond_broadcast(&cond);
        }
    };
#endif

	// higher-level objects
	class ScopedMutex {
		Mutex & m;
	public:
		ScopedMutex( Mutex &inMutex ) : m( inMutex ) {
			m.Acquire();
		}
		~ScopedMutex() {
			m.Release();
		}
	protected:
        ScopedMutex & operator=( const ScopedMutex & ) {}
	};
	
	class ScopedMutexReverse {
		Mutex & m;
	public:
		ScopedMutexReverse( Mutex &inMutex ) : m( inMutex ) {
			m.Release();
		}
		~ScopedMutexReverse() {
			m.Acquire();
		}
	protected:
        ScopedMutexReverse & operator=( const ScopedMutexReverse & ) {}
	};

    class ThreadBarrier
    {
    private:
        Condition cv;	// condition variable and mutex
        int maxcnt;		// number of threads to wait for
		int cnt;		// number of waiting threads
		int phase;		// flag to seperate two barriers
    public:
        ThreadBarrier(int numThreads) {
            maxcnt = numThreads;
            cnt = 0;
			phase = 0;
        }

        ~ThreadBarrier() {
        }

		// waits until all threads have reached barrier
		void Wait() {
			cv.Acquire();
		    int my_phase = phase;
            cnt++;
			if (cnt == maxcnt) {		// I am the last one
				cnt = 0;                // reset for next use
				phase = 1 - my_phase;   // toggle phase
				cv.Broadcast();
			} else {
				while (phase == my_phase) {				
					cv.Wait();
				}
			}
			cv.Release();
        }

		void Reset() {
			cnt = 0;
			phase = 0;
		}
    };
}

#endif // __R3_THREAD_H__
