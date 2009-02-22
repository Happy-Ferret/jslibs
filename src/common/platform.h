/* ***** BEGIN LICENSE BLOCK *****
 * Version: GNU GPL 2.0
 *
 * The contents of this file are subject to the
 * GNU General Public License Version 2.0; you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at
 * http://www.gnu.org/licenses/gpl.html
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 * ***** END LICENSE BLOCK ***** */

#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#if defined __cplusplus
	#define EXTERN_C extern "C"
#else
	#define EXTERN_C
#endif // __cplusplus

/*
#if defined _DEBUG
	#define DEBUG
#endif // _DEBUG
*/

///////////////////////////////////////////////////////////////////////////////
// Compiler specific configuration

#if defined(__GNUC__) && (__GNUC__ > 2)
	#define likely(expr)	__builtin_expect((expr), !0)
	#define unlikely(expr)	__builtin_expect((expr), 0)
#else
	#define likely(expr)	!!(expr)
	#define unlikely(expr)	!!(expr)
#endif

#if defined(_MSC_VER)

	#define DLLEXPORT __declspec(dllexport)
	#define DLLLOCAL
	#define FASTCALL __fastcall
	#define ALWAYS_INLINE __forceinline

#elif defined(__GNUC__)

	#if defined HAVE_GCCVISIBILITYPATCH
		#define DLLEXPORT __attribute__ ((visibility("default")))
		#define DLLLOCAL __attribute__ ((visibility("hidden")))
	#else
		#define DLLEXPORT
		#define DLLLOCAL
	#endif

	#if defined(__i386__) && ((__GNUC__ >= 4) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4))
		#define FASTCALL __attribute__((fastcall))
	#else
		#define FASTCALL
	#endif

	#define ALWAYS_INLINE __attribute__((always_inline))

#else

	#define DLLEXPORT
	#define FASTCALL
	#define DLLLOCAL
	#define ALWAYS_INLINE

#endif


#if defined _MSC_VER
	// disable warnings:
	#pragma warning(disable : 4244 4305)  // for VC++, no precision loss complaints
	#pragma warning(disable : 4127)  // no "conditional expression is constant" complaints
	#pragma warning(disable : 4311) // warning C4311: 'variable' : pointer truncation from 'type' to 'type'
	#pragma warning(disable : 4312) // warning C4312: 'operation' : conversion from 'type1' to 'type2' of greater size
	#pragma warning(disable : 4267) // warning C4267: 'var' : conversion from 'size_t' to 'type', possible loss of data
	#pragma warning(disable : 4996) // warning C4996: 'function': was declared deprecated
	#pragma warning(disable : 4100) // warning C4100: 'xxx' : unreferenced formal parameter
	#pragma warning(disable : 4102) // warning C4102: 'xxx' : unreferenced label
	// force warning to error:
	#pragma warning(error : 4715) // not all control paths return a value
	#pragma warning(error : 4018) // warning C4018: '<' : signed/unsigned mismatch
	#pragma warning(error : 4309) // warning C4309: 'initializing' : truncation of constant value
	#pragma warning(error : 4700) // warning C4700: uninitialized local variable 'XXX' used
	#pragma warning(error : 4533) // warning C4533: initialization of 'xxx' is skipped by 'goto YYY'

#endif // #if defined WIN32

#include <limits.h>
#include <sys/types.h>


///////////////////////////////////////////////////////////////////////////////
// Platform specific configuration

#if defined(_WINDOWS) || defined(WIN32) // Windows platform

	#define XP_WIN // used by SpiderMonkey and jslibs

	#ifndef WINVER         // Allow use of features specific to Windows 95 and Windows NT 4 or later.
	#define WINVER 0x0501  // Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
	#endif

	#ifndef _WIN32_WINNT         // Allow use of features specific to Windows NT 4 or later.
	#define _WIN32_WINNT 0x0501  // Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
	#endif                       

	#ifndef _WIN32_WINDOWS        // Allow use of features specific to Windows 98 or later.
	#define _WIN32_WINDOWS 0x0501 // Change this to the appropriate value to target Windows Me or later.
	#endif

	#define WIN32_LEAN_AND_MEAN   // Exclude rarely-used stuff from Windows headers
	#include <windows.h>

	#include <direct.h> // function declarations for directory handling/creation
	#include <process.h> // threads, ...

/*
	#define int8_t   INT8
	#define int16_t  INT16
	#define int32_t  INT32
	#define int64_t  INT64

	#define u_int8_t  UINT8
	#define u_int16_t UINT16
	#define u_int32_t UINT32
	#define u_int64_t UINT64
*/

	#define LLONG __int64

	#define PATH_MAX MAX_PATH
	#define DLL_EXT ".dll"
	#define PATH_SEPARATOR_STRING "\\"
	#define PATH_SEPARATOR '\\'
	#define LIST_SEPARATOR_STRING ";"
	#define LIST_SEPARATOR ';'

	#define strcasecmp stricmp

#elif defined(_MACOSX) // MacosX platform
	
	#define XP_UNIX // used by SpiderMonkey and jslibs

	#include <unistd.h>
	
	#define LLONG long long

	#define DLL_EXT ".dylib"
	#define PATH_SEPARATOR_STRING "/"
	#define PATH_SEPARATOR '/'
	#define LIST_SEPARATOR_STRING ":"
	#define LIST_SEPARATOR ':'

#else // Linux platform
	
	#define XP_UNIX // used by SpiderMonkey and jslibs

	#include <unistd.h>
	#include <sys/time.h>
	#include <dlfcn.h>

	#define LLONG long long

	#define DLL_EXT ".so"
	#define PATH_SEPARATOR_STRING "/"
	#define PATH_SEPARATOR '/'
	#define LIST_SEPARATOR_STRING ":"
	#define LIST_SEPARATOR ':'

#endif // Windows/MacosX/Linux platform

// MS specific ?
//#ifndef O_BINARY
//	#define O_BINARY 0
//#endif


///////////////////////////////////////////////////////////////////////////////
// Miscellaneous

#define COUNTOF(vector) (sizeof(vector)/sizeof(*vector))


///////////////////////////////////////////////////////////////////////////////
// Platform tools

enum Endian {
	BigEndian,
	LittleEndian,
	MiddleEndian,
	UnknownEndian
};

inline Endian DetectSystemEndianType() {

	switch ( *(unsigned long*)"\3\2\1" ) { // 03020100
		case 0x03020100: return BigEndian;
		case 0x00010203: return LittleEndian;
		case 0x02030001: return MiddleEndian;
	}
	return UnknownEndian;
}


inline char* IntegerToString(int val, int base) {

	static char buf[64]; // (TBD) multithread warning !
	buf[63] = '\0';
	int i = 62;
	for(; val && i ; --i, val /= base)
		buf[i] = "0123456789abcdefghijklmnopqrstuvwxyz"[val % base];
	return &buf[i+1];
}


inline void SleepMilliseconds(unsigned int ms) {
#if defined XP_WIN
	Sleep(ms); // winbase.h
#elif defined XP_UNIX
	usleep(ms * 1000); // unistd.h
#endif // XP_UNIX
}

inline double AccurateTimeCounter() {

#if defined XP_WIN
	LARGE_INTEGER frequency, performanceCount;
	BOOL result = ::QueryPerformanceFrequency(&frequency);
	result = ::QueryPerformanceCounter(&performanceCount);
	return 1000 * performanceCount.QuadPart / (double)frequency.QuadPart;
#elif defined XP_UNIX
	struct timeval time;
	struct timezone tz;
	gettimeofday(&time, &tz);
	return (double)time.tv_sec / 1000L*1000L;
#endif
	return -1; // (TBD) see. JS_Now() ? no, it could be expensive and is not suitable for calls when a GC lock is held.
}


inline int JLProcessId() {

#if defined XP_WIN
	return getpid();
#elif defined XP_UNIX
	return getpid();
#endif // XP_UNIX
	return -1; // (TBD)
}


inline unsigned int JLSessionId() {

	unsigned int r = 0x12345678;
	r ^= (unsigned int)AccurateTimeCounter();
	r ^= (unsigned int)JLProcessId();
#if defined XP_WIN
//	r ^= (u_int32_t)GetModuleHandle(NULL);
	MEMORYSTATUS status;
	GlobalMemoryStatus( &status );
	r ^= (unsigned int)status.dwAvailPhys;
#endif // XP_WIN
	return r ? r : 1; // avoid returning 0
}


// Atomic operations
	// MS doc: http://msdn.microsoft.com/en-us/library/ms686360.aspx

	#if defined XP_WIN
	typedef volatile LONG * JLAtomicPtr;
	#elif defined XP_UNIX
	typedef volatile long * JLAtomicPtr;
	#endif // XP_UNIX

	inline bool JLAtomicInc( JLAtomicPtr ptr ) {

		//InterlockedIncrement( atomic );
		// int atomic_inc_and_test(atomic_t * v);
		// __sync_fetch_and_add
		return ++*ptr != 0;
	}

	inline bool JLAtomicDec( JLAtomicPtr ptr ) {

		// int atomic_dec_and_test(atomic_t * v);
		return --*ptr != 0;
	}


#if defined XP_WIN
#elif defined XP_UNIX
#include <stdlib.h> // malloc()
#include <pthread.h>
#include <sched.h>
#include <semaphore.h>
#include <errno.h>
#endif


// semaphores
	#if defined XP_WIN
	typedef HANDLE JLSemaphoreHandler;
	#elif defined XP_UNIX
	typedef sem_t* JLSemaphoreHandler;
	#endif


	inline JLSemaphoreHandler JLCreateSemaphore( int initCount ) {

		#if defined XP_WIN
		return CreateSemaphore(NULL, initCount, LONG_MAX, NULL);
		#elif defined XP_UNIX
		sem_t *sem = (sem_t*)malloc(sizeof(sem_t)); // (TBD) max ???
		sem_init(sem, 0, initCount);
		return sem;
		#endif
	}

	inline bool JLSemaphoreOk( JLSemaphoreHandler semaphore ) {
		
		return semaphore != (JLSemaphoreHandler)0;
	}

	inline void JLAcquireSemaphore( JLSemaphoreHandler semaphore ) {

		if ( !JLSemaphoreOk(semaphore) )
			return;
		#if defined XP_WIN
		WaitForSingleObject(semaphore, INFINITE);
		#elif defined XP_UNIX
		sem_wait(semaphore);
		#endif
	}

	inline void JLReleaseSemaphore( JLSemaphoreHandler semaphore ) {

		if ( !JLSemaphoreOk(semaphore) )
			return;
		#if defined XP_WIN
		ReleaseSemaphore(semaphore, 1, NULL);
		#elif defined XP_UNIX
		sem_post(semaphore);
		#endif
	}
	
	inline void JLFreeSemaphore( JLSemaphoreHandler *pSemaphore ) {
		
		if ( !pSemaphore || !JLSemaphoreOk(*pSemaphore) )
			return;
		#if defined XP_WIN
		CloseHandle(*pSemaphore);
		#elif defined XP_UNIX
		sem_destroy(*pSemaphore);
		#endif
		*pSemaphore = (JLSemaphoreHandler)0;
	}


// mutex
/* notes:
	A normal mutex cannot be locked repeatedly by the owner. 
	Attempts by a thread to relock an already held mutex, 
	or to lock a mutex that was held by another thread when that thread terminated result in a deadlock condition.
	PTHREAD_MUTEX_NORMAL
	A recursive mutex can be locked repeatedly by the owner. 
	The mutex doesn't become unlocked until the owner has called pthread_mutex_unlock() for
	each successful lock request that it has outstanding on the mutex.
	PTHREAD_MUTEX_RECURSIVE
*/
	#if defined XP_WIN
	typedef HANDLE JLMutexHandler;
	#elif defined XP_UNIX	
	typedef pthread_mutex_t* JLMutexHandler;
	#endif

	inline JLMutexHandler JLCreateMutex() {
		
		#if defined XP_WIN
		return CreateMutex(NULL, FALSE, NULL);
		#elif defined XP_UNIX
		pthread_mutex_t *mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
		pthread_mutex_init(mutex, NULL);
		return mutex;
		#endif
	}

	inline bool JLMutexOk( JLMutexHandler mutex ) {
		
		return mutex != (JLMutexHandler)0;
	}

	inline void JLAcquireMutex( JLMutexHandler mutex ) {

		if ( !JLMutexOk(mutex) )
			return;
		#if defined XP_WIN
		WaitForSingleObject(mutex, INFINITE);
		#elif defined XP_UNIX
		pthread_mutex_lock(mutex);
		#endif
	}

	inline void JLReleaseMutex( JLMutexHandler mutex ) {
	
		if ( !JLMutexOk(mutex) )
			return;
		#if defined XP_WIN
		ReleaseMutex(mutex);
		#elif defined XP_UNIX
		pthread_mutex_unlock(mutex);
		#endif
	}

	inline void JLFreeMutex( JLMutexHandler *pMutex ) {
		
		if ( !pMutex || !JLMutexOk(*pMutex) )
			return;
		#if defined XP_WIN
		CloseHandle(*pMutex);
		#elif defined XP_UNIX
		pthread_mutex_destroy(*pMutex);
		#endif
		*pMutex = (JLMutexHandler)0;
	}

// thread
//   Linux: https://computing.llnl.gov/tutorials/pthreads/#PthreadsAPI

	#if defined XP_WIN
		#define JL_THREAD_PRIORITY_LOWEST THREAD_PRIORITY_LOWEST
		#define JL_THREAD_PRIORITY_LOW THREAD_PRIORITY_BELOW_NORMAL
		#define JL_THREAD_PRIORITY_NORMAL THREAD_PRIORITY_NORMAL
		#define JL_THREAD_PRIORITY_HIGH THREAD_PRIORITY_ABOVE_NORMAL
		typedef HANDLE JLThreadHandler;
		typedef int JLThreadPriorityType;
		#define JLThreadFuncDecl DWORD WINAPI
		typedef PTHREAD_START_ROUTINE JLThreadRoutine;
	#elif defined XP_UNIX
		#define JL_THREAD_PRIORITY_LOWEST 0
		#define JL_THREAD_PRIORITY_LOW 15
		#define JL_THREAD_PRIORITY_NORMAL 31
		#define JL_THREAD_PRIORITY_HIGH 47
		typedef pthread_t* JLThreadHandler;
		typedef int JLThreadPriorityType;
		#define JLThreadFuncDecl void*
		typedef void*(*JLThreadRoutine)(void *);
	#endif


	inline bool JLThreadOk( JLThreadHandler thread ) {

		return thread != (JLThreadHandler)0;
	}

	inline JLThreadHandler JLThreadStart( JLThreadRoutine threadRoutine, void *pv ) {

		#if defined XP_WIN
		return (JLThreadHandler)CreateThread(NULL, 0, threadRoutine, pv, 0, NULL);
		#elif defined XP_UNIX
		pthread_t *thread = (pthread_t*)malloc(sizeof(pthread_t));
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE); // optional ?
		int rc;
		rc = pthread_create(thread, &attr, threadRoutine, pv);
		pthread_attr_destroy(&attr);
		return rc ? 0 : thread;
		#endif
	}

	inline void JLThreadExit() {

		#if defined XP_WIN
		ExitThread(0);
		#elif defined XP_UNIX
		pthread_exit(NULL);
		#endif
	}

	inline void JLThreadCancel( JLThreadHandler thread ) {

		#if defined XP_WIN
		TerminateThread(thread, 0); // The handle must have the THREAD_TERMINATE access right.
		#elif defined XP_UNIX
		pthread_cancel(*thread);
		#endif
	}


	inline void JLThreadPriority( JLThreadHandler thread, JLThreadPriorityType priority ) {

		#if defined XP_WIN
		SetThreadPriority(thread, priority);
		#elif defined XP_UNIX
		int policy;
		struct sched_param param;
		int rv;
		rv = pthread_getschedparam(*thread, &policy, &param);
		int max = sched_get_priority_max(policy);
		int min = sched_get_priority_min(policy);
		param.sched_priority = min + priority * (max - min) / 128;
		rv = pthread_setschedparam(*thread, policy, &param);
		#endif
	}

	inline bool JLThreadIsActive( JLThreadHandler thread ) {

		if ( !JLThreadOk(thread) )
			return false;
		#if defined XP_WIN
		DWORD result = WaitForSingleObject( thread, 0 );
		return result != WAIT_OBJECT_0; // else WAIT_TIMEOUT ?
		#elif defined XP_UNIX
		int policy;
		struct sched_param param;
		int rv = pthread_getschedparam(*thread, &policy, &param);
		return rv != ESRCH; // errno.h
		#endif
	}

	inline void JLWaitThread( JLThreadHandler thread ) {
		
		if ( !JLThreadOk(thread) )
			return;
		#if defined XP_WIN
		WaitForSingleObject( thread, INFINITE ); // WAIT_OBJECT_0
		#elif defined XP_UNIX
		void *status;
		int rc;
		rc = pthread_join(*thread, &status);
		#endif
	}

	inline void JLFreeThread( JLThreadHandler *pThread ) {

		if ( !pThread || !JLThreadOk(*pThread) )
			return;
		#if defined XP_WIN
		CloseHandle(*pThread);
		#elif defined XP_UNIX
		pthread_detach(**pThread);
		free(*pThread);
		#endif
		*pThread = (JLThreadHandler)0;
	}


// dynamic libraries
	#if defined XP_WIN
	typedef HMODULE JLLibraryHandler;
	#elif defined XP_UNIX
	typedef void* JLLibraryHandler;
	#endif

	inline JLLibraryHandler JLDynamicLibraryOpen( const char *filename ) {

		#if defined XP_WIN
		return LoadLibrary(filename);
		#elif defined XP_UNIX
		dlerror();
		return dlopen( filename, RTLD_LAZY ); // RTLD_NOW
		#endif
	}

	inline bool JLDynamicLibraryOk( JLLibraryHandler libraryHandler ) {
		
		return libraryHandler != (JLLibraryHandler)0;
	}

	inline void JLDynamicLibraryLastErrorMessage( char *message, size_t maxLength ) {
		
		#if defined XP_WIN
		DWORD errorCode = ::GetLastError();
		LPVOID lpMsgBuf;
		DWORD result = ::FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK,
			NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL );
		if ( result != 0 ) {

			strncpy(message, (char*)lpMsgBuf, maxLength-1);
			message[maxLength-1] = '\0';
		} else
			*message = '\0';
		#elif defined XP_UNIX
		const char *msgBuf = dlerror();
		if ( msgBuf != NULL ) {

			strncpy(message, msgBuf, maxLength-1);
			message[maxLength-1] = '\0';
		} else 
			*message = '\0';
		#endif
	}

	inline void *JLDynamicLibrarySymbol( JLLibraryHandler libraryHandler, const char *symbolName ) {

		#if defined XP_WIN
		return (void*)GetProcAddress(libraryHandler, symbolName);
		#elif defined XP_UNIX
		dlerror();
		return dlsym(libraryHandler, symbolName);
		#endif
	}

	inline void JLDynamicLibraryClose( JLLibraryHandler *libraryHandler ) {

		#if defined XP_WIN
		FreeLibrary(*libraryHandler);
		#elif defined XP_UNIX
		dlerror();
		dlclose(*libraryHandler);
		#endif
		*libraryHandler = (JLLibraryHandler)0;
	}


/* (TBD) manage error
#if defined XP_UNIX
	J_S_ASSERT_2( id != 0, "Unable to load the module \"%s\": %s", libFileName, dlerror() );
#else // XP_UNIX
	J_S_ASSERT_2( id != 0, "Unable to load the module \"%s\": %x", libFileName, GetLastError() );
#endif // XP_UNIX
*/



#endif // _PLATFORM_H_

/*
Inline Functions In C
	http://www.greenend.org.uk/rjk/2003/03/inline.html
*/

/*
#ifndef BOOL
	#define BOOL int
#endif

#ifndef TRUE
	#define TRUE (1)
#endif

#ifndef FALSE
	#define FALSE (0)
#endif
*/
