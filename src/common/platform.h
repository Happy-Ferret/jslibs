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

#ifdef __cplusplus
	#define EXTERN_C extern "C"
#else
	#define EXTERN_C
#endif // __cplusplus

#ifdef _DEBUG
	#define DEBUG
#endif // _DEBUG


///////////////////////////////////////////////////////////////////////////////
// Compiler specific configuration

#if defined(__GNUC__) && (__GNUC__ > 2)
	#define likely(expr)	__builtin_expect((expr), !0)
	#define unlikely(expr)	__builtin_expect((expr), 0)
#else
	#define likely(expr)	!!(expr)
	#define unlikely(expr)	!!(expr)
#endif

#define COUNTOF(vector) (sizeof(vector)/sizeof(*vector))

#ifdef _MSC_VER
	#pragma warning(disable : 4244 4305)  // for VC++, no precision loss complaints
	#pragma warning(disable : 4127)  // no "conditional expression is constant" complaints
	#pragma warning(disable : 4311) // warning C4311: 'variable' : pointer truncation from 'type' to 'type'
	#pragma warning(disable : 4312) // warning C4312: 'operation' : conversion from 'type1' to 'type2' of greater size
	#pragma warning(disable : 4267) // warning C4267: 'var' : conversion from 'size_t' to 'type', possible loss of data
	#pragma warning(disable : 4996) // warning C4996: 'function': was declared deprecated
	#pragma warning(disable : 4100) // warning C4100: 'xxx' : unreferenced formal parameter
	#pragma warning(error : 4715) // force error for: not all control paths return a value
	#pragma warning(error : 4018) // force error for: warning C4018: '<' : signed/unsigned mismatch
	#pragma warning(error : 4309) // force error for: warning C4309: 'initializing' : truncation of constant value
#endif // #ifdef WIN32

#include <limits.h>
#include <sys/types.h>


///////////////////////////////////////////////////////////////////////////////
// Platform specific configuration

#if defined(_WINDOWS) || defined(WIN32) // Windows platform

	#ifndef WINVER                // Allow use of features specific to Windows 95 and Windows NT 4 or later.
	#define WINVER 0x0501        // Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
	#endif

	#ifndef _WIN32_WINNT        // Allow use of features specific to Windows NT 4 or later.
	#define _WIN32_WINNT 0x0501        // Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
	#endif                       

	#ifndef _WIN32_WINDOWS        // Allow use of features specific to Windows 98 or later.
	#define _WIN32_WINDOWS 0x0501 // Change this to the appropriate value to target Windows Me or later.
	#endif

	#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

	#include <windows.h>
	#include <direct.h> // function declarations for directory handling/creation
	#include <process.h> // threads, ...

	#define int8_t   INT8
	#define int16_t  INT16
	#define int32_t  INT32
	#define int64_t  INT64

	#define u_int8_t  UINT8
	#define u_int16_t UINT16
	#define u_int32_t UINT32
	#define u_int64_t UINT64

	#define XP_WIN

	#define PATH_MAX MAX_PATH
	#define DLL_EXT ".dll"
	#define PATH_SEPARATOR_STRING "\\"
	#define PATH_SEPARATOR '\\'
	#define LLONG __int64

	#define DLLEXPORT __declspec(dllexport)
	#define DLLLOCAL

	#define strcasecmp stricmp


// semaphores
	typedef HANDLE JLSemaphoreHandler;

	inline JLSemaphoreHandler JLCreateSemaphore( int initCount, int maxCount ) {
		
		return CreateSemaphore(NULL, initCount, maxCount, NULL);
	}

	inline bool JLSemaphoreOk( JLSemaphoreHandler semaphore ) {
		
		return semaphore != NULL;
	}

	inline void JLAcquireSemaphore( JLSemaphoreHandler semaphore ) {

		if ( JLSemaphoreOk(semaphore) )
			WaitForSingleObject(semaphore, INFINITE);
	}

	inline void JLReleaseSemaphore( JLSemaphoreHandler semaphore ) {

		if ( JLSemaphoreOk(semaphore) )
			ReleaseSemaphore(semaphore, 1, NULL);
	}
	
	inline void JLFreeSemaphore( JLSemaphoreHandler *pSemaphore ) {
		
		if ( JLSemaphoreOk(pSemaphore) ) {
			
			CloseHandle(*pSemaphore);
			*pSemaphore = NULL;
		}
	}


// mutex
	typedef HANDLE JLMutexHandler;

	inline JLMutexHandler JLCreateMutex() {
		
		return CreateMutex(NULL, FALSE, NULL);
	}

	inline bool JLMutexOk( JLMutexHandler mutex ) {
		
		return mutex != NULL;
	}

	inline void JLAcquireMutex( JLMutexHandler mutex ) {

		if ( JLMutexOk(mutex) )
			WaitForSingleObject(mutex, INFINITE);
	}

	inline void JLReleaseMutex( JLMutexHandler mutex ) {
	
		if ( JLMutexOk(mutex) )
			ReleaseMutex(mutex);
	}

	inline void JLFreeMutex( JLMutexHandler *pMutex ) {
		
		if ( pMutex && JLMutexOk(*pMutex) ) {

			CloseHandle(*pMutex);
			*pMutex = (JLMutexHandler)0;
		}
	}

// thread
	#define JL_THREAD_PRIORITY_LOWEST THREAD_PRIORITY_LOWEST
	#define JL_THREAD_PRIORITY_LOW THREAD_PRIORITY_BELOW_NORMAL
	#define JL_THREAD_PRIORITY_NORMAL THREAD_PRIORITY_ABOVE_NORMAL
	#define JL_THREAD_PRIORITY_HIGH THREAD_PRIORITY_NORMAL

	typedef HANDLE JLThreadHandler;
	typedef int JLThreadPriorityType;

	#define JLThreadFuncDecl DWORD WINAPI

	inline bool JLThreadPriority( JLThreadHandler thread, JLThreadPriorityType priority ) {

		SetThreadPriority(thread, priority);
	}

	inline JLThreadHandler JLStartThread( LPTHREAD_START_ROUTINE threatFunction, void *pv ) {

		return (JLThreadHandler)CreateThread(NULL, 0, threatFunction, pv, 0, NULL);
	}

	inline bool JLThreadOk( JLThreadHandler thread ) {

		return thread != (JLThreadHandler)0;
	}

	inline void JLWaitThread( JLThreadHandler thread ) {
		
		if ( JLThreadOk(thread) )
			WaitForSingleObject( thread, INFINITE ); // WAIT_OBJECT_0
	}

	inline void JLFreeThread( JLThreadHandler *pThread ) {

		if ( JLThreadOk(*pThread) ) {

			CloseHandle(*pThread);
			*pThread = (JLThreadHandler)0;
		}
	}

// dynamic libraries
	typedef HMODULE JLLibraryHandler;

	inline JLLibraryHandler JLDynamicLibraryOpen( const char *filename ) {

		return LoadLibrary(filename);
	}

	inline void *JLDynamicLibrarySymbol( JLLibraryHandler libraryHandler, const char *symbolName ) {

		return (void*)GetProcAddress(libraryHandler, symbolName);
	}

	inline void JLDynamicLibraryClose( JLLibraryHandler *libraryHandler ) {

		FreeLibrary(*libraryHandler);
		*libraryHandler = NULL;
	}


/* (TBD) manage error
#ifdef XP_UNIX
	J_S_ASSERT_2( id != 0, "Unable to load the module \"%s\": %s", libFileName, dlerror() );
#else // XP_UNIX
	J_S_ASSERT_2( id != 0, "Unable to load the module \"%s\": %x", libFileName, GetLastError() );
#endif // XP_UNIX
*/


#elif defined(_MACOSX) // MacosX platform
	
	#include <unistd.h>
	
	#define XP_UNIX

	#define DLL_EXT ".dylib"
	#define PATH_SEPARATOR_STRING "/"
	#define PATH_SEPARATOR '/'
	#define LLONG long long

	#ifdef HAVE_GCCVISIBILITYPATCH
		#define DLLEXPORT __attribute__ ((visibility("default")))
		#define DLLLOCAL __attribute__ ((visibility("hidden")))
	#else
		#define DLLEXPORT
		#define DLLLOCAL
	#endif


#else // Linux platform
	
	#include <unistd.h>
	#include <sys/time.h>
	#include <dlfcn.h>

	#define XP_UNIX

	#define DLL_EXT ".so"
	#define PATH_SEPARATOR_STRING "/"
	#define PATH_SEPARATOR '/'
	#define LLONG long long

	#ifdef HAVE_GCCVISIBILITYPATCH
		#define DLLEXPORT __attribute__ ((visibility("default")))
		#define DLLLOCAL __attribute__ ((visibility("hidden")))
	#else
		#define DLLEXPORT
		#define DLLLOCAL
	#endif


	typedef void *JLLibraryHandler;

	inline JLLibraryHandler JLDynamicLibraryOpen( const char *filename ) {

		dlerror();
		return dlopen( filename, RTLD_NOW );
	}

	inline void JLDynamicLibraryClose( JLLibraryHandler *libraryHandler ) {

		dlerror();
		dlclose( libraryHandler );
	}

	inline void *JLDynamicLibrarySymbol( JLLibraryHandler libraryHandler, const char *symbolName ) {

		dlerror();
		return dlsym( libraryHandler, symbolName );
	}

#endif // Windows/MacosX/Linux platform

// MS specific ?
//#ifndef O_BINARY
//	#define O_BINARY 0
//#endif


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

	static char buf[32] = {0};
	int i = 30;
	for(; val && i ; --i, val /= base)
		buf[i] = "0123456789abcdef"[val % base];
	return &buf[i+1];
}

inline double AccurateTimeCounter() {

#ifdef XP_WIN
	LARGE_INTEGER frequency, performanceCount;
	BOOL result = ::QueryPerformanceFrequency(&frequency);
	result = ::QueryPerformanceCounter(&performanceCount);
	return 1000 * performanceCount.QuadPart / (double)frequency.QuadPart;
#endif // XP_WIN

#ifdef XP_UNIX

	struct timeval time;
	struct timezone tz;
	gettimeofday(&time, &tz);
	return (double)time.tv_sec / 1000*1000;
#endif // XP_UNIX

	// see. JS_Now()
	return -1; // (TBD)
}

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
