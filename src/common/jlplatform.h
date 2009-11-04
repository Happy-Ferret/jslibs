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

#ifndef _JLPLATFORM_H_
#define _JLPLATFORM_H_


///////////////////////////////////////////////////////////////////////////////
// Miscellaneous

#define JL_MACRO_BEGIN do {
#define JL_MACRO_END } while(0)

#define COUNTOF(vector) (sizeof(vector)/sizeof(*vector))


///////////////////////////////////////////////////////////////////////////////
// Compiler specific configuration

#if defined __cplusplus
	#define EXTERN_C extern "C"
#else
	#define EXTERN_C
#endif // __cplusplus

#if defined(_DEBUG)
	#if !defined(DEBUG)
		#define DEBUG
	#endif
#endif

#if defined(__GNUC__) && (__GNUC__ > 2)
	#define likely(expr)	__builtin_expect((expr), !0)
	#define unlikely(expr)	__builtin_expect((expr), 0)
#else
	#define likely(expr)	!!(expr)
	#define unlikely(expr)	!!(expr)
	// see __assume keyword for MSVC
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

	#ifdef DEBUG
		#define ALWAYS_INLINE __inline__
	#else
		#define ALWAYS_INLINE __attribute__((always_inline)) __inline__
	#endif

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
//	#pragma warning(disable : 4125) // warning C4125: decimal digit terminates octal escape sequence
	// force warning to error:
	#pragma warning(error : 4715) // not all control paths return a value
	#pragma warning(error : 4018) // warning C4018: '<' : signed/unsigned mismatch
	#pragma warning(error : 4309) // warning C4309: 'initializing' : truncation of constant value
	#pragma warning(error : 4700) // warning C4700: uninitialized local variable 'XXX' used
	#pragma warning(error : 4533) // warning C4533: initialization of 'xxx' is skipped by 'goto YYY'

#endif // #if defined WIN32

#include <limits.h>
#include <sys/types.h>
#include <malloc.h>
#include <stdlib.h>
#include <errno.h>

///////////////////////////////////////////////////////////////////////////////
// Platform specific configuration

#if defined(_WINDOWS) || defined(WIN32) // Windows platform

#if defined(REPORT_MEMORY_LEAKS)
	// the following code make issue with jstl.h (js\src\jstl.h(244) : error C2039: '_malloc_dbg' : is not a member of 'JSContext')
	#ifdef _DEBUG
	# define _CRTDBG_MAP_ALLOC
	# include <stdlib.h>
	# include <crtdbg.h>
	#endif // _DEBUG
#endif // REPORT_MEMORY_LEAKS

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

	#define int8_t   INT8
	#define int16_t  INT16
	#define int32_t  INT32
	#define int64_t  INT64

	#define uint8_t  UINT8
	#define uint16_t UINT16
	#define uint32_t UINT32
	#define uint64_t UINT64

	#define LLONG __int64

	#define PATH_MAX MAX_PATH
	#define DLL_EXT ".dll"
	#define PATH_SEPARATOR_STRING "\\"
	#define PATH_SEPARATOR '\\'
	#define LIST_SEPARATOR_STRING ";"
	#define LIST_SEPARATOR ';'

	#define __THROW throw()

	#define strcasecmp stricmp

	inline size_t msize( void *ptr ) {

		if ( ptr != NULL )
			return _msize(ptr);
		return 0;
	}

	inline void* memalign( size_t alignment, size_t size ) {
		
		return _aligned_malloc(alignment, size);
	}

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

	#ifndef O_BINARY
	#define O_BINARY 0
	#endif // O_BINARY

	#ifndef O_SEQUENTIAL
	#define O_SEQUENTIAL 0
	#endif // O_SEQUENTIAL

	#define LLONG long long

	#define DLL_EXT ".so"
	#define PATH_SEPARATOR_STRING "/"
	#define PATH_SEPARATOR '/'
	#define LIST_SEPARATOR_STRING ":"
	#define LIST_SEPARATOR ':'

	inline size_t msize( void *ptr ) {

		if ( ptr != NULL ) // (TBD) check if it is needed
			return malloc_usable_size(ptr);
		return 0;
	}

	inline void* memalign( size_t alignment, size_t size ) {
		
		void *ptr;
		posix_memalign(&ptr, alignment, size);
		return ptr;
	}

#endif // Windows/MacosX/Linux platform

/*
	int posix_memalign(void **memptr, size_t alignment, size_t size) {
		if (alignment % sizeof(void *) != 0)
			return EINVAL;
		*memptr = memalign(alignment, size);
		return (*memptr != NULL ? 0 : ENOMEM);
	}
*/

// MS specific ?
//#ifndef O_BINARY
//	#define O_BINARY 0
//#endif

#if defined XP_WIN
	#include <io.h>
#endif
#include <fcntl.h>
#include <stdio.h>


#ifdef DEBUG

inline void JL_Assert(const char *s, const char *file, unsigned int ln) {

    fprintf(stderr, "Jslibs assertion failure: %s, at %s:%d\n", s, file, ln);
#if defined(WIN32)
    DebugBreak();
    exit(3);
#elif defined(XP_OS2) || (defined(__GNUC__) && defined(__i386))
    asm("int $3");
#endif
    abort();
}

#define JL_ASSERT(expr) \
    ((expr) ? (void)0 : JS_Assert(#expr, __FILE__, __LINE__))

#else

#define JL_ASSERT(expr) ((void) 0)

#endif // DEBUG


inline void fpipe( FILE **read, FILE **write ) {

	int readfd, writefd;
#if defined XP_WIN
	HANDLE readPipe, writePipe;
	CreatePipe(&readPipe, &writePipe, NULL, 65536);
	// doc: The underlying handle is also closed by a call to _close,
	//      so it is not necessary to call the Win32 function CloseHandle on the original handle. 
	readfd = _open_osfhandle((intptr_t)readPipe, _O_RDONLY);
	writefd = _open_osfhandle((intptr_t)writePipe, _O_WRONLY);
#elif defined XP_UNIX
	int fd[2];
	pipe(fd); // (TBD) check return value
	readfd = fd[0];
	writefd = fd[1];
#endif
	*read = fdopen(readfd, "r");
	*write = fdopen(writefd, "w");
}


#ifdef XP_UNIX
#include <string.h>
#include <stdlib.h>
ALWAYS_INLINE void JLGetAbsoluteModulePath( char* moduleFileName, size_t size, char *modulePath ) {

	if ( modulePath[0] == PATH_SEPARATOR ) { //  /jshost

		strcpy(moduleFileName, modulePath);
		return;
	}

	if ( modulePath[0] == '.' && modulePath[1] == PATH_SEPARATOR ) { //  ./jshost

		getcwd(moduleFileName, size);
		strcat(moduleFileName, modulePath + 1 );
		return;
	}

	if ( modulePath[0] == '.' && modulePath[1] == '.' && modulePath[2] == PATH_SEPARATOR ) { //  ../jshost

		getcwd(moduleFileName, size);
		strcat(moduleFileName, PATH_SEPARATOR_STRING);
		strcat(moduleFileName, modulePath);
		return;
	}

	if ( strchr( modulePath, PATH_SEPARATOR ) != NULL ) { //  xxx/../jshost

		getcwd(moduleFileName, size);
		strcat(moduleFileName, PATH_SEPARATOR_STRING);
		strcat(moduleFileName, modulePath);
		return;
	}

	char *envPath = getenv("PATH");
	char *pos;

	do {

		pos = strchr( envPath, ':' );

		if ( envPath[0] == PATH_SEPARATOR ) {

			if ( pos == NULL ) {

				strcpy(moduleFileName, envPath);
			} else {

				strncpy(moduleFileName, envPath, pos-envPath);
				moduleFileName[pos-envPath] = '\0';
			}

			strcat(moduleFileName, PATH_SEPARATOR_STRING);
			strcat(moduleFileName, modulePath);

			if (access(moduleFileName, R_OK | X_OK ) == 0) // If the requested access is permitted, it returns 0.
				return;
		}

		envPath = pos+1;

	} while (pos != NULL);

	moduleFileName[0] = '\0';
	return;
}
#endif //XP_UNIX


///////////////////////////////////////////////////////////////////////////////
// Platform tools

enum Endian {
	BigEndian,
	LittleEndian,
	MiddleEndian,
	UnknownEndian
};

ALWAYS_INLINE Endian DetectSystemEndianType() {

	switch ( *(unsigned long*)"\3\2\1" ) { // 03020100
		case 0x03020100: return BigEndian;
		case 0x00010203: return LittleEndian;
		case 0x02030001: return MiddleEndian;
	}
	return UnknownEndian;
}


ALWAYS_INLINE char* IntegerToString(int val, int base) {

	bool neg;
	static char buf[64]; // (TBD) threadsafe and overflow warning !
	buf[63] = '\0';
	if ( val < 0 ) {

		val = -val;
		neg = true;
	} else {

		neg = false;
	}
	int i = 62;
	for(; val && i ; --i, val /= base)
		buf[i] = "0123456789abcdefghijklmnopqrstuvwxyz"[val % base];
	if ( neg )
		buf[i--] = '-';
	return &buf[i+1];
}


ALWAYS_INLINE void SleepMilliseconds(unsigned int ms) {

#if defined XP_WIN
	Sleep(ms); // winbase.h
#elif defined XP_UNIX
	usleep(ms * 1000); // unistd.h
#endif // XP_UNIX
}

ALWAYS_INLINE double AccurateTimeCounter() {

#if defined XP_WIN
	static LONGLONG initTime = 0; // initTime helps in avoiding precision waste.
	LARGE_INTEGER frequency, performanceCount;
	BOOL result = ::QueryPerformanceFrequency(&frequency);
	DWORD_PTR oldmask = ::SetThreadAffinityMask(::GetCurrentThread(), 0); // manage bug in BIOS or HAL
	result = ::QueryPerformanceCounter(&performanceCount);
	if ( initTime == 0 )
		initTime = performanceCount.QuadPart;
	::SetThreadAffinityMask(::GetCurrentThread(), oldmask);
	return (double)1000 * (performanceCount.QuadPart-initTime) / (double)frequency.QuadPart;
#elif defined XP_UNIX
	static long initTime = 0; // initTime helps in avoiding precision waste.
	struct timeval tv;
	gettimeofday(&tv, NULL);
	if ( initTime == 0 )
		initTime = tv.tv_sec;
	return (double)(tv.tv_sec-initTime) * (double)1000 + tv.tv_usec / (double)1000;
#endif
	return -1; // (TBD) see. js_IntervalNow() or JS_Now() ? no, it could be expensive and is not suitable for calls when a GC lock is held.
/* see also:
__int64 GetTime() {
    __int64 clock;
    __asm {
        rdtsc                        // Resad the RDTSC Timer
        mov    dword ptr[clock], eax // Store the value in EAX and EDX Registers
        mov    dword ptr[clock+4], edx
    }
    return clock;
}
*/
}


ALWAYS_INLINE int JLProcessId() {

#if defined XP_WIN
	return getpid();
#elif defined XP_UNIX
	return getpid();
#endif // XP_UNIX
	return -1; // (TBD)
}


ALWAYS_INLINE unsigned int JLSessionId() {

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


#if defined XP_WIN
#include <malloc.h> // malloc()
#elif defined XP_UNIX
#include <stdlib.h> // malloc()
#include <time.h>
#include <pthread.h>
#include <sched.h>
#include <semaphore.h>
#include <errno.h>
#include <string.h>
#include <error.h>
#endif


///////////////////////////////////////////////////////////////////////////////
// system errors
//
	ALWAYS_INLINE void JLLastSysetmErrorMessage( char *message, size_t maxLength ) {

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
		const char *msgBuf = strerror(errno);
		if ( msgBuf != NULL ) {

			strncpy(message, msgBuf, maxLength-1);
			message[maxLength-1] = '\0';
		} else
			*message = '\0';
#endif
	}

///////////////////////////////////////////////////////////////////////////////

#define JLERROR (0) // (int)false
#define JLOK (1)
#define JLTIMEOUT (-2)


///////////////////////////////////////////////////////////////////////////////
// atomic operations
// 
// MS doc: http://msdn.microsoft.com/en-us/library/ms686360.aspx
//         http://msdn.microsoft.com/en-us/library/ms683590%28VS.85%29.aspx
// Linux: http://gcc.gnu.org/onlinedocs/gcc-4.1.2/gcc/Atomic-Builtins.html

	ALWAYS_INLINE int JLAtomicExchange(volatile long *ptr, long val) {
	#if defined XP_WIN
		return InterlockedExchange(ptr, val);
	#elif defined XP_UNIX // #elif ((__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1)) && ... //  #if defined(HAVE_GCC_ATOMIC32)
		return __sync_lock_test_and_set(ptr, val);
	#endif
	}

	ALWAYS_INLINE void JLAtomicIncrement(volatile long *ptr) {
	#if defined XP_WIN
		InterlockedIncrement(ptr);
	#elif defined XP_UNIX
		__sync_add_and_fetch(ptr, 1);
	#endif
	}

	ALWAYS_INLINE int JLAtomicAdd(volatile long *ptr, long val) {
	#if defined XP_WIN
		return InterlockedExchangeAdd(ptr, val) + val;
	#elif defined XP_UNIX
		return __sync_add_and_fetch(ptr, val);
	#endif
	}

/*
///////////////////////////////////////////////////////////////////////////////
// condvar
//
#if defined XP_WIN
	typedef struct {
		HANDLE mutex;
		HANDLE event;
	} *JLCondHandler;
#elif defined XP_UNIX
	typedef struct {
		pthread_mutex_t mutex;
		pthread_cond_t cond;
	} *JLCondHandler;
#endif

	#if defined XP_WIN
	ALWAYS_INLINE JLCondHandler JLCreateCond() {

		JLCondHandler cond = (JLCondHandler)malloc(sizeof(*cond));
		cond->mutex = CreateMutex(NULL, FALSE, NULL); // lpMutexAttributes, bInitialOwner, lpName
		cond->event = CreateEvent(NULL, TRUE, FALSE, NULL); // lpEventAttributes, bManualReset, bInitialState, lpName
		return cond;
	}
	#elif defined XP_UNIX
	ALWAYS_INLINE JLCondHandler JLCreateCond() {

		JLCondHandler cond = (JLCondHandler)malloc(sizeof(*cond));
		pthread_mutex_init(cond->mutex, NULL);
		pthread_cond_init(&cond->cond, NULL);
		return cond;
	}
	#endif

	ALWAYS_INLINE JLCondWait( JLCondHandler cond, int timeout ) {
	}
*/



///////////////////////////////////////////////////////////////////////////////
// semaphores
//

#if defined XP_WIN
	typedef HANDLE JLSemaphoreHandler;
#elif defined XP_UNIX
	typedef sem_t* JLSemaphoreHandler;
#endif


	ALWAYS_INLINE JLSemaphoreHandler JLCreateSemaphore( int initCount ) {

	#if defined XP_WIN
		return CreateSemaphore(NULL, initCount, LONG_MAX, NULL);
	#elif defined XP_UNIX
		sem_t *sem = (sem_t*)malloc(sizeof(sem_t)); // (TBD) max ???
		if ( !sem )
			return NULL;
		sem_init(sem, 0, initCount);
		return sem;
	#endif
	}

	ALWAYS_INLINE bool JLSemaphoreOk( JLSemaphoreHandler semaphore ) {

		return semaphore != (JLSemaphoreHandler)0;
	}

	// msTimeout = -1 : no timeout
	// returns true on a sucessfuly semaphore locked.
	ALWAYS_INLINE int JLAcquireSemaphore( JLSemaphoreHandler semaphore, int msTimeout ) {

		if ( !JLSemaphoreOk(semaphore) )
			return JLERROR;
	#if defined XP_WIN
		switch ( WaitForSingleObject(semaphore, msTimeout == -1 ? INFINITE : msTimeout) ) {
			case WAIT_TIMEOUT:
				return JLTIMEOUT;
			case WAIT_OBJECT_0:
				return JLOK;
		}
	#elif defined XP_UNIX
		if ( msTimeout == -1 ) {
			
			if ( sem_wait(semaphore) == 0 )
				return JLOK;
		} else {

			// see also: struct timespec ts; clock_gettime(CLOCK_REALTIME, &ts); link with -lrt
			struct timespec ts;
			struct timeval tv;
			gettimeofday(&tv, NULL);
			ts.tv_nsec = tv.tv_usec * 1000UL + (msTimeout % 1000)*1000000UL;
			ts.tv_sec = tv.tv_sec + msTimeout / 1000UL + ts.tv_nsec / 1000000000UL;
			ts.tv_nsec %= 1000000000UL;
			switch ( sem_timedwait(semaphore, &ts) ) {
				case ETIMEDOUT:
					return JLTIMEOUT;
				case 0:
					return JLOK;
			}
		}
	#endif
		return JLERROR;
	}

	ALWAYS_INLINE int JLReleaseSemaphore( JLSemaphoreHandler semaphore ) {

		if ( !JLSemaphoreOk(semaphore) )
			return JLERROR;
	#if defined XP_WIN
		if ( ReleaseSemaphore(semaphore, 1, NULL) == 0 )
			return JLERROR;
	#elif defined XP_UNIX
		if ( sem_post(semaphore) != 0 )
			return JLERROR;
	#endif
		return JLOK;
	}

	ALWAYS_INLINE int JLFreeSemaphore( JLSemaphoreHandler *pSemaphore ) {

		if ( !pSemaphore || !JLSemaphoreOk(*pSemaphore) )
			return JLERROR;
	#if defined XP_WIN
		if ( CloseHandle(*pSemaphore) == 0 )
			return JLERROR;
	#elif defined XP_UNIX
		if ( sem_destroy(*pSemaphore) != 0 )
			return JLERROR;
		free(*pSemaphore);
	#endif
		*pSemaphore = (JLSemaphoreHandler)0;
		return JLOK;
	}


///////////////////////////////////////////////////////////////////////////////
// mutex
//
//notes:
//  A normal mutex cannot be locked repeatedly by the owner.
//  Attempts by a thread to relock an already held mutex,
//  or to lock a mutex that was held by another thread when that thread terminated result in a deadlock condition.
//  PTHREAD_MUTEX_NORMAL
//  A recursive mutex can be locked repeatedly by the owner.
//  The mutex doesn't become unlocked until the owner has called pthread_mutex_unlock() for
//  each successful lock request that it has outstanding on the mutex.
//  PTHREAD_MUTEX_RECURSIVE

#if defined XP_WIN
	typedef HANDLE JLMutexHandler;
#elif defined XP_UNIX
	typedef pthread_mutex_t* JLMutexHandler;
#endif

	ALWAYS_INLINE JLMutexHandler JLCreateMutex() {

	#if defined XP_WIN
		return CreateMutex(NULL, FALSE, NULL);
	#elif defined XP_UNIX
		pthread_mutex_t *mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
		pthread_mutex_init(mutex, NULL);
		return mutex;
	#endif
	}

	ALWAYS_INLINE bool JLMutexOk( JLMutexHandler mutex ) {

		return mutex != (JLMutexHandler)0;
	}

	ALWAYS_INLINE int JLAcquireMutex( JLMutexHandler mutex ) {

		if ( !JLMutexOk(mutex) )
			return JLERROR;
	#if defined XP_WIN
		if ( WaitForSingleObject(mutex, INFINITE) != WAIT_OBJECT_0 )
			return JLERROR;
	#elif defined XP_UNIX
		if ( pthread_mutex_lock(mutex) != 0 )
			return JLERROR;
	#endif
		return JLOK;
	}

	ALWAYS_INLINE int JLReleaseMutex( JLMutexHandler mutex ) {

		if ( !JLMutexOk(mutex) )
			return JLERROR;
	#if defined XP_WIN
		if ( ReleaseMutex(mutex) == 0 )
			return JLERROR;
	#elif defined XP_UNIX
		if ( pthread_mutex_unlock(mutex) != 0 )
			return JLERROR;
	#endif
		return JLOK;
	}

	ALWAYS_INLINE int JLFreeMutex( JLMutexHandler *pMutex ) {

		if ( !pMutex || !JLMutexOk(*pMutex) )
			return JLERROR;
	#if defined XP_WIN
		if ( CloseHandle(*pMutex) == 0 )
			return JLERROR;
	#elif defined XP_UNIX
		if ( pthread_mutex_destroy(*pMutex) != 0 )
			return JLERROR;
		free(*pMutex);
	#endif
		*pMutex = (JLMutexHandler)0;
		return JLOK;
	}


///////////////////////////////////////////////////////////////////////////////
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
		#define JL_THREAD_PRIORITY_LOW 40
		#define JL_THREAD_PRIORITY_NORMAL 64
		#define JL_THREAD_PRIORITY_HIGH 88
		typedef pthread_t* JLThreadHandler;
		typedef int JLThreadPriorityType;
		#define JLThreadFuncDecl void*
		typedef void*(*JLThreadRoutine)(void *);
	#endif


	ALWAYS_INLINE bool JLThreadOk( JLThreadHandler thread ) {

		return thread != (JLThreadHandler)0;
	}

	ALWAYS_INLINE JLThreadHandler JLThreadStart( JLThreadRoutine threadRoutine, void *pv ) {

	#if defined XP_WIN
		return CreateThread(NULL, 0, threadRoutine, pv, 0, NULL); // (TBD) need THREAD_TERMINATE ?
	#elif defined XP_UNIX
		pthread_t *thread = (pthread_t*)malloc(sizeof(pthread_t));
		if ( thread == NULL )
			return NULL;
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE); // optional ?
		int rc;
		rc = pthread_create(thread, &attr, threadRoutine, pv);
		pthread_attr_destroy(&attr);
		if ( rc == 0 ) // if no error
			return thread;
		free(thread);
		return NULL;
	#endif
	}

	ALWAYS_INLINE void JLThreadExit() {

	#if defined XP_WIN
		ExitThread(0);
	#elif defined XP_UNIX
		pthread_exit(NULL);
	#endif
	}

	ALWAYS_INLINE int JLThreadCancel( JLThreadHandler thread ) {

	#if defined XP_WIN
		if ( TerminateThread(thread, 0) == 0 ) // doc. The handle must have the THREAD_TERMINATE access right. ... Use the GetExitCodeThread function to retrieve a thread's exit value.
			return JLERROR;
	#elif defined XP_UNIX
		if ( pthread_cancel(*thread) != 0 )
			return JLERROR;
	#endif
		return JLOK;
	}


	ALWAYS_INLINE int JLThreadPriority( JLThreadHandler thread, JLThreadPriorityType priority ) {

	#if defined XP_WIN
		if ( SetThreadPriority(thread, priority) != 0 )
			return JLOK;
	#elif defined XP_UNIX
		int policy;
		struct sched_param param;
		if ( pthread_getschedparam(*thread, &policy, &param) != 0 )
			return JLERROR;
		int max = sched_get_priority_max(policy);
		int min = sched_get_priority_min(policy);
		param.sched_priority = min + (max - min) * priority / 128;
		if ( pthread_setschedparam(*thread, policy, &param) == 0 )
			return JLOK;
	#endif
		return JLERROR;
	}

	ALWAYS_INLINE bool JLThreadIsActive( JLThreadHandler thread ) {  // (TBD) how to manage errors ?

		if ( !JLThreadOk(thread) )
			return false;
	#if defined XP_WIN
		DWORD result = WaitForSingleObject(thread, 0);
		return result == WAIT_TIMEOUT; // else != WAIT_OBJECT_0 ?
	#elif defined XP_UNIX
		int policy;
		struct sched_param param;
		return pthread_getschedparam(*thread, &policy, &param) != ESRCH; // errno.h
	#endif
	}

	ALWAYS_INLINE int JLWaitThread( JLThreadHandler thread ) {

		if ( !JLThreadOk(thread) )
			return JLERROR;
	#if defined XP_WIN
		if ( WaitForSingleObject(thread, INFINITE) != WAIT_OBJECT_0 )
			return JLERROR;
	#elif defined XP_UNIX
		void *status;
		if ( pthread_join(*thread, &status) != 0 ) // doc. The thread exit status returned by pthread_join() on a canceled thread is PTHREAD_CANCELED.
			return JLERROR;
	#endif
		return JLOK;
	}

	ALWAYS_INLINE int JLFreeThread( JLThreadHandler *pThread ) {

		if ( !pThread || !JLThreadOk(*pThread) )
			return JLERROR;
	#if defined XP_WIN
		if ( CloseHandle(*pThread) == 0 )
			return JLERROR;
	#elif defined XP_UNIX
		if ( JLThreadIsActive( *pThread ) )
			if ( pthread_detach(**pThread) != 0 )
				return JLERROR;
		free(*pThread);
	#endif
		*pThread = (JLThreadHandler)0;
		return JLOK;
	}

///////////////////////////////////////////////////////////////////////////////
// dynamic libraries
//

#if defined XP_WIN
	typedef HMODULE JLLibraryHandler;
#elif defined XP_UNIX
	typedef void* JLLibraryHandler;
#endif

	ALWAYS_INLINE JLLibraryHandler JLDynamicLibraryOpen( const char *filename ) {

	#if defined XP_WIN
		return LoadLibrary(filename);
	#elif defined XP_UNIX
		dlerror(); // Resets the error indicator.
		return dlopen(filename, RTLD_LAZY | RTLD_LOCAL); // RTLD_NOW / RTLD_GLOBAL
	#endif
	}

	ALWAYS_INLINE bool JLDynamicLibraryOk( JLLibraryHandler libraryHandler ) {

		return libraryHandler != (JLLibraryHandler)0;
	}

	ALWAYS_INLINE void JLDynamicLibraryLastErrorMessage( char *message, size_t maxLength ) {

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

	ALWAYS_INLINE void *JLDynamicLibrarySymbol( JLLibraryHandler libraryHandler, const char *symbolName ) {

	#if defined XP_WIN
		return (void*)GetProcAddress(libraryHandler, symbolName);
	#elif defined XP_UNIX
		dlerror(); // Resets the error indicator.
		return dlsym(libraryHandler, symbolName);
	#endif
	}

	ALWAYS_INLINE int JLDynamicLibraryClose( JLLibraryHandler *libraryHandler ) {

	#if defined XP_WIN
		if ( FreeLibrary(*libraryHandler) == 0 )
			return JLERROR;
	#elif defined XP_UNIX
		dlerror(); // Resets the error indicator.
		if ( dlclose(*libraryHandler) != 0 )
			return JLERROR;
	#endif
		*libraryHandler = (JLLibraryHandler)0;
		return JLOK;
	}

#endif // _JLPLATFORM_H_
