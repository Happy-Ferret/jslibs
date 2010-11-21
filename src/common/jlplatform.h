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
//


///////////////////////////////////////////////////////////////////////////////
// Miscellaneous

#define J__STRINGIFY(x) #x
#define J__TOSTRING(x) J__STRINGIFY(x)

#define JL_MACRO_BEGIN do {
#define JL_MACRO_END } while(0)

#define __DATE__YEAR ((((__DATE__ [7] - '0') * 10 + (__DATE__ [8] - '0')) * 10 + (__DATE__ [9] - '0')) * 10 + (__DATE__ [10] - '0'))
#define __DATE__MONTH (__DATE__ [2] == 'n' ? (__DATE__ [1] == 'a' ? 0 : 5) : __DATE__ [2] == 'l' ? 6 : __DATE__ [2] == 'g' ? 7 : __DATE__ [2] == 'p' ? 8 : __DATE__ [2] == 't' ? 9 : __DATE__ [2] == 'v' ? 10 : 11)
#define __DATE__DAY ((__DATE__ [4] == ' ' ? 0 : __DATE__ [4] - '0') * 10 + (__DATE__ [5] - '0'))
#define __DATE__HOUR (((__TIME__[0]-'0')*10) + (__TIME__[1]-'0'))
#define __DATE__MINUTE (((__TIME__[3]-'0')*10) + (__TIME__[4]-'0'))
#define __DATE__SECOND (((__TIME__[6]-'0')*10) + (__TIME__[7]-'0'))

#define JL_BUILD ( (((__DATE__YEAR * 12 + __DATE__MONTH) * 31 + __DATE__DAY) * 24 + __DATE__HOUR) - (((2006*12 + 6)*31 + 22)*24 + 0) ) // - Aug 22, 2006

#define JL_CODE_LOCATION __FILE__ ":" J__TOSTRING(__LINE__)


///////////////////////////////////////////////////////////////////////////////
// Compiler specific configuration


#if defined(__cplusplus)
	#define EXTERN_C extern "C"
#else
	#define EXTERN_C
#endif // __cplusplus


#if defined(_DEBUG)
	#if !defined(DEBUG)
		#define DEBUG
	#endif
#endif


#ifdef DEBUG
	#define IFDEBUG(expr) expr
#else
	#define IFDEBUG(expr)
#endif // DEBUG


#if defined(__GNUC__) && (__GNUC__ > 2)
	#define likely(expr)	__builtin_expect((expr), !0)
	#define unlikely(expr)	__builtin_expect((expr), 0)
#else
	#define likely(expr)	(!!(expr))
	#define unlikely(expr)	(!!(expr))
	// see __assume keyword for MSVC
#endif


// trick. When an inline function is not static, then the compiler must assume that there may be calls from other source files; since a global symbol can be defined only once in any program,
//        the function must not be defined in the other source files, so the calls therein cannot be integrated. Therefore, a non-static inline function is always compiled on its own in the usual fashion.

#if defined(_MSC_VER)

	#define DLLEXPORT __declspec(dllexport)
	#define DLLLOCAL
	#define FASTCALL __fastcall
	#define ALWAYS_INLINE __forceinline
	#define INLINE inline

#elif defined(__GNUC__)

	// # pragma GCC diagnostic ignored "-Wformat"  /* Ignore Warning about printf format /
	// # pragma GCC diagnostic ignored "-Wunused-parameter"  / Ignore Warning about unused function parameter */
	#pragma GCC diagnostic error "-Wdiv-by-zero"

	// info. http://gcc.gnu.org/wiki/Visibility
	// support of visibility attribute is mandatory to manage _moduleId scope (must be private but global for the .so)
	//
	//	without #define DLLLOCAL __attribute__ ((visibility("hidden")))
	// DLLLOCAL uint32_t _moduleId = 0;
	// if __attribute__ ((visibility("hidden"))) is not set for _moduleId,
	// nm build/default/src/jsstd/jsstd | grep moduleId
	// 000168a8 B _moduleId => uppercase B = global
	// and should be:
	// 000168a8 b _moduleId => lowercase b = local

	#define DLLEXPORT __attribute__ ((visibility("default")))
	#define DLLLOCAL __attribute__ ((visibility("hidden")))

	#if defined(__i386__) && ((__GNUC__ >= 4) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4))
		#define FASTCALL __attribute__((fastcall))
	#else
		#define FASTCALL
	#endif

	#ifdef DEBUG
		#define ALWAYS_INLINE __inline__
		#define INLINE __inline__
	#else
		#define ALWAYS_INLINE __attribute__((always_inline)) __inline__
		#define INLINE __inline__
	#endif

#else

	#define DLLEXPORT
	#define FASTCALL
	#define DLLLOCAL
	#define ALWAYS_INLINE

#endif


#if defined(_MSC_VER)
	// disable warnings:
	#pragma warning(disable : 4127)  // no "conditional expression is constant" complaints
	#pragma warning(disable : 4996) // warning C4996: 'function': was declared deprecated

	// force warning to error:
	#pragma warning(error : 4715) // not all control paths return a value
	#pragma warning(error : 4018) // warning C4018: '<' : signed/unsigned mismatch
	#pragma warning(error : 4309) // warning C4309: 'initializing' : truncation of constant value
	#pragma warning(error : 4700) // warning C4700: uninitialized local variable 'XXX' used
	#pragma warning(error : 4533) // warning C4533: initialization of 'xxx' is skipped by 'goto YYY'
	#pragma warning(error : 4002) // too many actual parameters for macro 'XXX'

	// Using STRICT to Improve Type Checking (http://msdn.microsoft.com/en-us/library/aa280394%28v=VS.60%29.aspx)
	#define STRICT

	// see WinDef.h
	#define NOMINMAX

//warning: must be defined on cmd line !
//	#if !defined(DEBUG)
//		// Don's use secure standard library from VC++
//		#define _SECURE_SCL_THROWS 0
//		#define _SECURE_SCL 0
//		#define _HAS_ITERATOR_DEBUGGING 0
//	#endif // #DEBUG


//	#define _CRT_SECURE_NO_DEPRECATE
//	#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1

	#define _NOTHROW throw()
	#define _NOVTABLE __declspec(novtable)

	#include <intrin.h>
	#pragma intrinsic(_ReturnAddress)

#else // _MSC_VER

	#define _NOTHROW
	#define _NOVTABLE

#endif // _MSC_VER



#include <limits>

#include <math.h>
#include <float.h>
#include <limits.h>
#include <sys/types.h>
#include <malloc.h>
#include <stdlib.h>
#include <errno.h>

#include <fcntl.h> // _O_RDONLY, _O_WRONLY
#include <stdio.h>
#include <cstddef>
//#include <varargs.h>
#include <stdarg.h>
#include <string.h>


#if CHAR_BIT != 8
#error "unsupported char size"
#endif


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

	#ifndef XP_WIN
	#define XP_WIN // used by SpiderMonkey and jslibs
	#endif

	#ifndef WINVER         // Allow use of features specific to Windows 95 and Windows NT 4 or later.
	#define WINVER 0x0501  // Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
	#endif

	#ifndef _WIN32_WINNT         // Allow use of features specific to Windows NT 4 or later.
	#define _WIN32_WINNT 0x0501  // Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
	#endif

	#ifndef _WIN32_WINDOWS        // Allow use of features specific to Windows 98 or later.
	#define _WIN32_WINDOWS 0x0501 // Change this to the appropriate value to target Windows Me or later. 0x501 = XP SP1.
	#endif

	#undef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN   // Exclude rarely-used stuff from Windows headers
	#include <windows.h>

//	#include <direct.h> // function declarations for directory handling/creation
	#include <process.h> // threads, ...

	typedef INT8  int8_t;
	typedef INT16 int16_t;
	typedef INT32 int32_t;
	typedef INT64 int64_t;

	typedef UINT8 uint8_t;
	typedef UINT16 uint16_t;
	typedef UINT32 uint32_t;
	typedef UINT64 uint64_t;

	typedef __int64 LLONG;

	#ifndef _SSIZE_T_DEFINED
	#ifdef  _WIN64
	typedef signed __int64    ssize_t;
	#else
	typedef _W64 signed int   ssize_t;
	#endif
	#define _SSIZE_T_DEFINED
	#endif


	#define PATH_MAX MAX_PATH
	#define DLL_EXT ".dll"
	#define PATH_SEPARATOR_STRING "\\"
	#define PATH_SEPARATOR '\\'
	#define LIST_SEPARATOR_STRING ";"
	#define LIST_SEPARATOR ';'

	#define finite _finite
	#define isnan _isnan

	#define strcasecmp stricmp

	static ALWAYS_INLINE size_t msize( void *ptr ) {

		if ( ptr != NULL )
			return _msize(ptr);
		return 0;
	}

	static ALWAYS_INLINE void* memalign( size_t alignment, size_t size ) {

		return _aligned_malloc(size, alignment);
	}

#elif defined(_MACOSX) // MacosX platform

	#ifndef XP_UNIX
	#define XP_UNIX // used by SpiderMonkey and jslibs
	#endif

	#include <unistd.h>

	#define LLONG long long

	#define DLL_EXT ".dylib"
	#define PATH_SEPARATOR_STRING "/"
	#define PATH_SEPARATOR '/'
	#define LIST_SEPARATOR_STRING ":"
	#define LIST_SEPARATOR ':'

#else // Linux platform

	#ifndef XP_UNIX
	#define XP_UNIX // used by SpiderMonkey and jslibs
	#endif

	#include <unistd.h>
	#include <sys/time.h>
	#include <dlfcn.h>
	#include <stdint.h>

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

	static ALWAYS_INLINE size_t msize( void *ptr ) {

		if ( ptr != NULL ) // (TBD) check if it is needed
			return malloc_usable_size(ptr);
		return 0;
	}

	static ALWAYS_INLINE void* memalign( size_t alignment, size_t size ) {

		void *ptr;
		posix_memalign(&ptr, alignment, size);
		return ptr;
	}

#endif // Windows/MacosX/Linux platform


// #define SIZE_T_MIN (static_cast<size_t>(0))
// #define SIZE_T_MAX (static_cast<size_t>(-1))
//	see SIZE_MAX

#define SSIZE_T_MAX (static_cast<ssize_t>(SIZE_T_MAX / 2))
#define SSIZE_T_MIN (static_cast<ssize_t>(-SSIZE_T_MAX - 1L))


#if defined(XP_WIN)
#include <io.h> // _open_osfhandle()
#endif

// MS specific ?
//#ifndef O_BINARY
//	#define O_BINARY 0
//#endif



///////////////////////////////////////////////////////////////////////////////
// Miscellaneous


//template<class T>
//static inline void JL_UNUSED(T) {};
//#define JL_UNUSED(x) x __attribute__((unused))
//#define JL_UNUSED(x) ((x) = (x))
#define JL_UNUSED(x) ((void)(x))

#define JL_STATIC_ASSERT(cond) \
	extern void jl_static_assert(int arg[(cond) ? 1 : -1])

/*
#if defined(WIN32)
#define JL_BREAK_HALT JL_MACRO_BEGIN   __debugbreak(); abort();   JL_MACRO_END
#elif defined(XP_OS2) || (defined(__GNUC__) && defined(__i386))
#define JL_BREAK_HALT JL_MACRO_BEGIN   asm("int $3"); abort();   JL_MACRO_END
#else
#define JL_BREAK_HALT JL_MACRO_BEGIN   abort();   JL_MACRO_END
#endif // platforms
*/

// see JS_Assert in jsutil.cpp
static ALWAYS_INLINE void
JL_AssertFailure( const char *message, const char *location ) {
	
	fprintf(stderr, "Assertion failure: %s @%s\n", message, location);
	fflush(stderr);
#if defined(WIN32)
    *((int *) NULL) = 0;
    exit(3);
#elif defined(__APPLE__)
    *((int *) NULL) = 0;  /* To continue from here in GDB: "return" then "continue". */
    raise(SIGABRT);  /* In case above statement gets nixed by the optimizer. */
#else
    raise(SIGABRT);  /* To continue from here in GDB: "signal 0". */
#endif
}


#ifdef DEBUG

#define JL_ASSERT(expr) \
    ( (expr) ? (void)0 : JL_AssertFailure(#expr, JL_CODE_LOCATION) )

#define JL_ASSERT_IF(cond, expr) \
    ( (!(cond) || (expr)) ? (void)0 : JL_AssertFailure(#expr, JL_CODE_LOCATION) )

#else // DEBUG

#define JL_ASSERT(expr) ((void)0)
#define JL_ASSERT_IF(cond,expr) ((void) 0)

#endif // DEBUG

///////////////////////////////////////////////////////////////////////////////
// Platform tools

template<class T>
struct DummyAlignStruct {
  unsigned char first;
  T second;
};

#define ALIGNOF(type) \
	(offsetof(DummyAlignStruct<type>, second))

#define COUNTOF(vector) \
	(sizeof(vector)/sizeof(*vector))


enum {
    JLBigEndian = 0x00010203ul,
    JLLittleEndian = 0x03020100ul,
    JLMiddleEndian = 0x01000302ul // or PDP endian
};

static const union {
	unsigned char bytes[4];
	uint32_t value;
} JLHostEndianType = { { 0, 1, 2, 3 } };

#define JLHostEndian \
	(JLHostEndianType.value)


// since 9007199254740992 == 9007199254740993, we must subtract 1.
// see also std::numeric_limits<double>::digits
JL_STATIC_ASSERT( DBL_MANT_DIG < 64 );

#define MAX_INT_TO_DOUBLE \
	((double)(((uint64_t)1<<DBL_MANT_DIG)-1))

// (TBD) fix needed on Linux
//JL_STATIC_ASSERT( MAX_INTDOUBLE != MAX_INTDOUBLE+1 );
//JL_STATIC_ASSERT( MAX_INTDOUBLE+1. == MAX_INTDOUBLE+2. );


static ALWAYS_INLINE int DOUBLE_IS_NEGZERO(const double &d) {
#ifdef WIN32
	return (d == 0 && (_fpclass(d) & _FPCLASS_NZ));
#elif defined(SOLARIS)
	return (d == 0 && copysign(1, d) < 0);
#else
	return (d == 0 && signbit(d));
#endif
}

static ALWAYS_INLINE bool DOUBLE_IS_NEG(const double &d) {
#ifdef WIN32
	return DOUBLE_IS_NEGZERO(d) || d < 0;
#elif defined(SOLARIS)
	return copysign(1, d) < 0;
#else
	return signbit(d);
#endif
}

static ALWAYS_INLINE bool JL_DOUBLE_IS_INTEGER(const double &d) {

	//	return d == double(int64_t(d));
	return floor(d) == d;
}


template<class T>
static ALWAYS_INLINE T JL_MIN(T a, T b) {
	return (a) < (b) ? (a) : (b);
}

template<class T>
static ALWAYS_INLINE T JL_MAX(T a, T b) {
	return (a) > (b) ? (a) : (b);
}


namespace jl {

	// eg. if ( IsSafeCast<int>(size_t(12345)) ) ...
	template <class D, class S>
	bool IsSafeCast(S src) {

		D dsrc = static_cast<D>(src);
		return static_cast<S>(dsrc) == src && (src < 0) == (dsrc < 0); // compare converted value and sign.
	}

	// eg. int i; if ( IsSafeCast(size_t(12345), i) ) ...
	template <class D, class S>
	bool IsSafeCast(S src, D) {

		D dsrc = static_cast<D>(src);
		return static_cast<S>(dsrc) == src && (src < 0) == (dsrc < 0); // compare converted value and sign.
	}

	// eg. int i = SafeCast<int>(size_t(12345));
	template <class D, class S>
	D SafeCast(S src) {

		JL_ASSERT( (IsSafeCast<D>(src)) );
		return static_cast<D>(src);
	}

	// eg. int i; i = SafeCast(size_t(12345), i);
	template <class D, class S>
	D SafeCast(S src, D) {

		JL_ASSERT( (IsSafeCast<D>(src)) );
		return static_cast<D>(src);
	}

}



//Macro that avoid multicharacter constant: From gcc page:
//`-Wno-multichar'
//     Do not warn if a multicharacter constant (`'FOOF'') is used.
//     Usually they indicate a typo in the user's code, as they have
//     implementation-defined values, and should not be used in portable
//     code.

// rise a "division by zero" if x is not a 5-char string.
//#define JL_CAST_CSTR_TO_UINT32(x) ( jl_unused(0/(sizeof(x) == 5 && x[3] == 0 ? 1 : 0)), (x[0]<<24) | (x[1]<<16) | (x[2]<<8) | (x[3]) )
//#define JL_CAST_CSTR_TO_UINT32(x) ( (x[0]<<24) | (x[1]<<16) | (x[2]<<8) | (x[3]) )
//	return (cstr[0]<<24) | (cstr[1]<<16) | (cstr[2]<<8) | (cstr[3]);
//	return *(uint32_t*)cstr;


//static ALWAYS_INLINE uint32_t JL_CAST_CSTR_TO_UINT32( const char cstr[5] ) {
//
////	JL_ASSERT(strlen(cstr) == 4);
//	if ( cstr[0] == '\0' )
//		return 0;
//	else
//	if ( cstr[1] == '\0' )
//		return cstr[0];
//	else
//	if ( cstr[2] == '\0' )
//		return (cstr[0] << 8) | cstr[1];
//	else
//	if ( cstr[3] == '\0' )
//		return (cstr[0] << 16) | (cstr[1] << 8) | cstr[2];
//
//	return *(uint32_t*)cstr;
//}

static ALWAYS_INLINE uint32_t JL_CAST_CSTR_TO_UINT32( const char *cstr ) {

	JL_ASSERT( cstr != NULL && !(cstr[0] && cstr[1] && cstr[2] && cstr[3] && cstr[4]) );
	return
		!cstr[0] ? 0 :
		!cstr[1] ? cstr[0] :
		!cstr[2] ? (cstr[0] <<  8) | cstr[1] :
		!cstr[3] ? (cstr[0] << 16) | (cstr[1] <<  8) | cstr[2] :
		           (cstr[0] << 24) | (cstr[1] << 16) | (cstr[2] <<  8) | cstr[3];
}


static INLINE unsigned long int_sqrt(unsigned long x) {

    register unsigned long op, res, one;

    op = x;
    res = 0;

    one = 1 << 30;
    while (one > op) one >>= 2;

    while (one != 0) {

        if (op >= res + one) {
            op -= res + one;
            res += one << 1;
        }
        res >>= 1;
        one >>= 2;
    }
    return res;
}


static INLINE int int_pow(int base, int exp) {

	int result = 1;
    while (exp) {

        if (exp & 1)
            result *= base;
        exp >>= 1;
        base *= base;
    }
    return result;
}


static ALWAYS_INLINE uint32_t JL_SvnRevToInt(const char *r) { // supports 9 digits revision number, NULL and empty and "$Revision$" strings.

	if ( r == NULL || r[0] == '\0' || r[10] == '\0' || r[11] == '\0' || r[12] == '\0' || r[13] == '\0' )
		return 0;

	const uint32_t count =
		  r[11] == ' ' ? 1
		: r[12] == ' ' ? 10
		: r[13] == ' ' ? 100
		: r[14] == ' ' ? 1000
		: r[15] == ' ' ? 10000
		: r[16] == ' ' ? 100000
		: r[17] == ' ' ? 1000000
		: r[18] == ' ' ? 10000000
		: r[19] == ' ' ? 100000000
		: 0;

	return
		(r[11] == ' ' ? 0 : (r[11]-'0') * (count/10) +
		(r[12] == ' ' ? 0 : (r[12]-'0') * (count/100) +
		(r[13] == ' ' ? 0 : (r[13]-'0') * (count/1000) +
		(r[14] == ' ' ? 0 : (r[14]-'0') * (count/10000) +
		(r[15] == ' ' ? 0 : (r[15]-'0') * (count/100000) +
		(r[16] == ' ' ? 0 : (r[16]-'0') * (count/1000000) +
		(r[17] == ' ' ? 0 : (r[17]-'0') * (count/10000000) +
		(r[18] == ' ' ? 0 : (r[18]-'0') * (count/100000000) +
		(r[19] == ' ' ? 0 : (r[19]-'0') * (count/1000000000) +
		0)))))))));
}


//int posix_memalign(void **memptr, size_t alignment, size_t size) {
//	if (alignment % sizeof(void *) != 0)
//		return EINVAL;
//	*memptr = memalign(alignment, size);
//	return (*memptr != NULL ? 0 : ENOMEM);
//}


INLINE void fpipe( FILE **read, FILE **write ) {

	int readfd, writefd;
#if defined(XP_WIN)
	HANDLE readPipe, writePipe;
	CreatePipe(&readPipe, &writePipe, NULL, 65536);
	// doc: The underlying handle is also closed by a call to _close,
	//      so it is not necessary to call the Win32 function CloseHandle on the original handle.
	readfd = _open_osfhandle((intptr_t)readPipe, _O_RDONLY);
	writefd = _open_osfhandle((intptr_t)writePipe, _O_WRONLY);
#elif defined(XP_UNIX)
	int fd[2];
	pipe(fd); // (TBD) check return value
	readfd = fd[0];
	writefd = fd[1];
#endif
	*read = fdopen(readfd, "r");
	*write = fdopen(writefd, "w");
}


#ifdef XP_UNIX
#include <stdlib.h>
static INLINE void JLGetAbsoluteModulePath( char* moduleFileName, size_t size, char *modulePath ) {

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


// cf. _swab() -or- _rotl();
// 16 bits: #define SWAP_BYTES(X) ((X & 0xff) << 8) | (X >> 8)
// 32 bits swap: #define SWAP_BYTE(x) ((x<<24) | (x>>24) | ((x&0xFF00)<<8) | ((x&0xFF0000)>>8))
//#define JL_BYTESWAP(ptr,a,b) { register char tmp = ((int8_t*)ptr)[a]; ((int8_t*)ptr)[a] = ((int8_t*)ptr)[b]; ((int8_t*)ptr)[b] = tmp; }

static ALWAYS_INLINE void JL_BYTESWAP(void *ptr, size_t a, size_t b) {

	register char tmp = ((int8_t*)ptr)[a];
	((int8_t*)ptr)[a] = ((int8_t*)ptr)[b];
	((int8_t*)ptr)[b] = tmp;
}

static ALWAYS_INLINE void Host16ToNetwork16( void *pval ) {

	if ( JLHostEndian == JLLittleEndian )
		JL_BYTESWAP( pval, 0, 1 );
}

static ALWAYS_INLINE void Host24ToNetwork24( void *pval ) {

	if ( JLHostEndian == JLLittleEndian )
		JL_BYTESWAP( pval, 0, 2 );
}

static ALWAYS_INLINE void Host32ToNetwork32( void *pval ) {

	if ( JLHostEndian == JLLittleEndian ) {

		JL_BYTESWAP( pval, 0, 3 );
		JL_BYTESWAP( pval, 1, 2 );
	}
}

static ALWAYS_INLINE void Host64ToNetwork64( void *pval ) {

	if ( JLHostEndian == JLLittleEndian ) {

		JL_BYTESWAP( pval, 0, 7 );
		JL_BYTESWAP( pval, 1, 6 );
		JL_BYTESWAP( pval, 2, 5 );
		JL_BYTESWAP( pval, 3, 4 );
	}
}


static ALWAYS_INLINE void Network16ToHost16( void *pval ) {

	if ( JLHostEndian == JLLittleEndian )
		JL_BYTESWAP( pval, 0, 1 );
}

static ALWAYS_INLINE void Network24ToHost24( void *pval ) {

	if ( JLHostEndian == JLLittleEndian )
		JL_BYTESWAP( pval, 0, 2 );
}


static ALWAYS_INLINE void Network32ToHost32( void *pval ) {

	if ( JLHostEndian == JLLittleEndian ) {

		JL_BYTESWAP( pval, 0, 3 );
		JL_BYTESWAP( pval, 1, 2 );
	}
}

static ALWAYS_INLINE void Network64ToHost64( void *pval ) {

	if ( JLHostEndian == JLLittleEndian ) {

		JL_BYTESWAP( pval, 0, 7 );
		JL_BYTESWAP( pval, 1, 6 );
		JL_BYTESWAP( pval, 2, 5 );
		JL_BYTESWAP( pval, 3, 4 );
	}
}


static INLINE char* IntegerToString(int val, int base) {

	bool neg;
	static char buf[64]; // (TBD) overflow warning !
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


static ALWAYS_INLINE void SleepMilliseconds(uint32_t ms) {

#if defined(XP_WIN)
	Sleep(ms); // winbase.h
#elif defined(XP_UNIX)
	usleep(ms * 1000); // unistd.h
#endif // XP_UNIX
}


static INLINE double AccurateTimeCounter() {

#if defined(XP_WIN)
	static volatile LONGLONG initTime = 0; // initTime helps in avoiding precision waste.
	LARGE_INTEGER frequency, performanceCount;
	BOOL result = ::QueryPerformanceFrequency(&frequency);
	JL_ASSERT( result );
	DWORD_PTR oldmask = ::SetThreadAffinityMask(::GetCurrentThread(), 0); // manage bug in BIOS or HAL
	result = ::QueryPerformanceCounter(&performanceCount);
	JL_ASSERT( result );
	if ( initTime == 0 )
		initTime = performanceCount.QuadPart;
	::SetThreadAffinityMask(::GetCurrentThread(), oldmask);
	JL_UNUSED( result );
	return (double)1000 * (performanceCount.QuadPart-initTime) / (double)frequency.QuadPart;
#elif defined(XP_UNIX)
	static long initTime = 0; // initTime helps in avoiding precision waste.
	struct timeval tv;
	gettimeofday(&tv, NULL);
	if ( initTime == 0 )
		initTime = tv.tv_sec;
	return (double)(tv.tv_sec-initTime) * (double)1000 + tv.tv_usec / (double)1000;
#endif
	return -1; // (TBD) see. js_IntervalNow() or JS_Now() ? no, it could be expensive and is not suitable for calls when a GC lock is held.
}
// see also:
//__int64 GetTime() {
//    __int64 clock;
//    __asm {
//        rdtsc                        // Resad the RDTSC Timer
//        mov    dword ptr[clock], eax // Store the value in EAX and EDX Registers
//        mov    dword ptr[clock+4], edx
//    }
//    return clock;
//}



static ALWAYS_INLINE int JLProcessId() {

#if defined(XP_WIN)
	return getpid();
#elif defined(XP_UNIX)
	return getpid();
#endif // XP_UNIX
	return -1; // (TBD)
}


#define JL_PAGESIZE 4096

static ALWAYS_INLINE size_t JLPageSize() {

	static size_t pageSize = 0;
	if ( pageSize )
		return pageSize;
#if defined(XP_WIN)
	SYSTEM_INFO siSysInfo;
	GetSystemInfo(&siSysInfo); // WinBase.h
	pageSize = siSysInfo.dwPageSize;
#elif defined(XP_UNIX)
	pageSize = sysconf(_SC_PAGESIZE); // unistd.h
#elif defined(__i386__) || defined(__x86_64__)
    pageSize = 4096;
#else
	#error Unable to detect system page size
#endif
	return pageSize;
}


static ALWAYS_INLINE uint32_t JLSessionId() {

	uint32_t r = 0x12345678;
	r ^= (uint32_t)AccurateTimeCounter();
	r ^= (uint32_t)JLProcessId();
#if defined(XP_WIN)
//	r ^= (u_int32_t)GetModuleHandle(NULL);
	MEMORYSTATUS status;
	GlobalMemoryStatus( &status );
	r ^= (uint32_t)status.dwAvailPhys;
#endif // XP_WIN
	return r ? r : 1; // avoid returning 0
}


/*
static __declspec(naked) __declspec(noinline) size_t JLGetEIP() {

	__asm pop eax;
	__asm jmp eax;
}
*/

/*
static size_t JLIP() {
#if defined(XP_WIN)
	return (size_t)_ReturnAddress();
#elif defined(XP_UNIX)
	return (size_t)__builtin_return_address(0);
#endif
}
*/

static ALWAYS_INLINE size_t JLRemainingStackSize() {
#if defined(XP_WIN)

	#pragma warning(push)
	#pragma warning(disable : 4312) // warning C4312: 'operation' : conversion from 'type1' to 'type2' of greater size
	NT_TIB *tib = (NT_TIB*)__readfsdword(0x18); // http://en.wikipedia.org/wiki/Win32_Thread_Information_Block
	#pragma warning(pop)

	volatile BYTE *currentSP;
	__asm mov [currentSP], esp;
	return currentSP - (BYTE*)tib->StackLimit;
#elif defined(XP_UNIX)

	return (size_t)-1;
#endif
}


// see http://unicode.org/faq/utf_bom.html#BOM
enum JLEncodingType {
	unknown,
	UTF32be,
	UTF32le,
	UTF16be,
	UTF16le,
	UTF8,
	ASCII
};

static INLINE JLEncodingType JLDetectEncoding(char **buf, size_t *size) {

	if ( *size < 2 )
		return ASCII;
	if ( (*buf)[0] == '\xFF' && (*buf)[1] == '\xFE' ) {

		*buf += 2;
		*size -= 2;
		return UTF16le;
	}
	if ( (*buf)[0] == '\xFE' && (*buf)[1] == '\xFF' ) {

		*buf += 2;
		*size -= 2;
		return UTF16be;
	}
	if ( *size >= 3 && (*buf)[0] == '\xEF' && (*buf)[1] == '\xBB' && (*buf)[2] == '\xBF' ) {

		*buf += 3;
		*size -= 3;
		return UTF8;
	}
	// no BOM, then guess
	if ( (*buf)[0] > 0 && (*buf)[1] > 0 )
		return ASCII;
	if ( (*buf)[0] != 0 && (*buf)[1] == 0 )
		return UTF16le;
	if ( (*buf)[0] == 0 && (*buf)[1] != 0 )
		return UTF16be;
	return unknown;
}

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4244 ) // warning C4244: '=' : conversion from 'unsigned int' to 'unsigned short', possible loss of data
#endif // _MSC_VER

#define xmlLittleEndian (JLHostEndian == JLLittleEndian)

// source: libxml2 - encoding.c - MIT License
// changes: outlen and inlen from integer to size_t, and the last redurned value
static INLINE int
UTF8ToUTF16LE(unsigned char* outb, size_t *outlen,
            const unsigned char* in, size_t *inlen)
{
    unsigned short* out = (unsigned short*) outb;
    const unsigned char* processed = in;
    const unsigned char *const instart = in;
    unsigned short* outstart= out;
    unsigned short* outend;
    const unsigned char* inend;
    unsigned int c, d;
    int trailing;
    unsigned char *tmp;
    unsigned short tmp1, tmp2;

    /* UTF16LE encoding has no BOM */
    if ((out == NULL) || (outlen == NULL) || (inlen == NULL)) return(-1);
    if (in == NULL) {
	*outlen = 0;
	*inlen = 0;
	return(0);
    }
    inend= in + *inlen;
    outend = out + (*outlen / 2);
    while (in < inend) {
      d= *in++;
      if      (d < 0x80)  { c= d; trailing= 0; }
      else if (d < 0xC0) {
          /* trailing byte in leading position */
	  *outlen = (out - outstart) * 2;
	  *inlen = processed - instart;
	  return(-2);
      } else if (d < 0xE0)  { c= d & 0x1F; trailing= 1; }
      else if (d < 0xF0)  { c= d & 0x0F; trailing= 2; }
      else if (d < 0xF8)  { c= d & 0x07; trailing= 3; }
      else {
	/* no chance for this in UTF-16 */
	*outlen = (out - outstart) * 2;
	*inlen = processed - instart;
	return(-2);
      }

      if (inend - in < trailing) {
          break;
      }

      for ( ; trailing; trailing--) {
          if ((in >= inend) || (((d= *in++) & 0xC0) != 0x80))
	      break;
          c <<= 6;
          c |= d & 0x3F;
      }

      /* assertion: c is a single UTF-4 value */
        if (c < 0x10000) {
            if (out >= outend)
	        break;
	    if (xmlLittleEndian) {
		*out++ = c;
	    } else {
		tmp = (unsigned char *) out;
		*tmp = c ;
		*(tmp + 1) = c >> 8 ;
		out++;
	    }
        }
        else if (c < 0x110000) {
            if (out+1 >= outend)
	        break;
            c -= 0x10000;
	    if (xmlLittleEndian) {
		*out++ = 0xD800 | (c >> 10);
		*out++ = 0xDC00 | (c & 0x03FF);
	    } else {
		tmp1 = 0xD800 | (c >> 10);
		tmp = (unsigned char *) out;
		*tmp = (unsigned char) tmp1;
		*(tmp + 1) = tmp1 >> 8;
		out++;

		tmp2 = 0xDC00 | (c & 0x03FF);
		tmp = (unsigned char *) out;
		*tmp  = (unsigned char) tmp2;
		*(tmp + 1) = tmp2 >> 8;
		out++;
	    }
        }
        else
	    break;
	processed = in;
    }
    *outlen = (out - outstart) * 2;
    *inlen = processed - instart;
//    return(*outlen);
	return(1);
}
#undef xmlLittleEndian

#ifdef _MSC_VER
#pragma warning( pop )
#endif // _MSC_VER


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if defined(XP_WIN)
#include <malloc.h> // malloc()
#elif defined(XP_UNIX)
#include <stdlib.h> // malloc()
#include <time.h>
#include <pthread.h>
#include <sched.h>
#include <semaphore.h>
#include <error.h>
#endif


///////////////////////////////////////////////////////////////////////////////
// system errors
//
	static INLINE void JLLastSysetmErrorMessage( char *message, size_t maxLength ) {

	#if defined(XP_WIN)
		DWORD errorCode = ::GetLastError();
		LPVOID lpMsgBuf;
		DWORD result = ::FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK,
			NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL );
		if ( result != 0 ) {

			strncpy(message, (char*)lpMsgBuf, maxLength-1);
			LocalFree(lpMsgBuf);
			message[maxLength-1] = '\0';
		} else {

			*message = '\0';
		}
	#elif defined(XP_UNIX)
		const char *msgBuf = strerror(errno);
		if ( msgBuf != NULL ) {

			strncpy(message, msgBuf, maxLength-1);
			message[maxLength-1] = '\0';
		} else {

			*message = '\0';
		}
	#endif
	}

///////////////////////////////////////////////////////////////////////////////

// status
#define JLERROR (0) // (int)false
#define JLOK (1)
#define JLTIMEOUT (-2)

// param
#define JLINFINITE (-1)

///////////////////////////////////////////////////////////////////////////////
// atomic operations
//
// MS doc: http://msdn.microsoft.com/en-us/library/ms686360.aspx
//         http://msdn.microsoft.com/en-us/library/ms683590%28VS.85%29.aspx
// Linux: http://gcc.gnu.org/onlinedocs/gcc-4.1.2/gcc/Atomic-Builtins.html

static ALWAYS_INLINE int JLAtomicExchange(volatile long *ptr, long val) {
#if defined(XP_WIN)
	return InterlockedExchange(ptr, val);
#elif defined(XP_UNIX) // #elif ((__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1)) && ... //  #if defined(HAVE_GCC_ATOMIC32)
	return __sync_lock_test_and_set(ptr, val);
#endif
}

static ALWAYS_INLINE long JLAtomicIncrement(volatile long *ptr) {
#if defined(XP_WIN)
	return InterlockedIncrement(ptr);
#elif defined(XP_UNIX)
	return __sync_add_and_fetch(ptr, 1);
#endif
}

static ALWAYS_INLINE long JLAtomicDecrement(volatile long *ptr) {
#if defined(XP_WIN)
	return InterlockedDecrement(ptr);
#elif defined(XP_UNIX)
	return __sync_sub_and_fetch(ptr, 1);
#endif
}

static ALWAYS_INLINE int JLAtomicAdd(volatile long *ptr, long val) {
#if defined(XP_WIN)
	return InterlockedExchangeAdd(ptr, val) + val;
#elif defined(XP_UNIX)
	return __sync_add_and_fetch(ptr, val);
#endif
}

/*
///////////////////////////////////////////////////////////////////////////////
// condvar
//
#if defined(XP_WIN)
	typedef struct {
		HANDLE mutex;
		HANDLE event;
	} *JLCondHandler;
#elif defined(XP_UNIX)
	typedef struct {
		pthread_mutex_t mutex;
		pthread_cond_t cond;
	} *JLCondHandler;
#endif

	#if defined(XP_WIN)
	ALWAYS_INLINE JLCondHandler JLCreateCond() {

		JLCondHandler cond = (JLCondHandler)malloc(sizeof(*cond));
		cond->mutex = CreateMutex(NULL, FALSE, NULL); // lpMutexAttributes, bInitialOwner, lpName
		cond->event = CreateEvent(NULL, TRUE, FALSE, NULL); // lpEventAttributes, bManualReset, bInitialState, lpName
		return cond;
	}
	#elif defined(XP_UNIX)
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

// Pthreads-w32: http://sourceware.org/pthreads-win32/

///////////////////////////////////////////////////////////////////////////////
// semaphores
//

#if defined(XP_WIN)
	typedef HANDLE JLSemaphoreHandler;
#elif defined(XP_UNIX)
	typedef sem_t* JLSemaphoreHandler;
#endif


	static INLINE JLSemaphoreHandler JLSemaphoreCreate( int initCount ) {

	#if defined(XP_WIN)
		return CreateSemaphore(NULL, initCount, LONG_MAX, NULL);
	#elif defined(XP_UNIX)
		sem_t *sem = (sem_t*)malloc(sizeof(sem_t)); // (TBD) max ???
		if ( sem == NULL )
			return NULL;
		sem_init(sem, 0, initCount);
		return sem;
	#endif
	}

	static ALWAYS_INLINE bool JLSemaphoreOk( JLSemaphoreHandler semaphore ) {

		return semaphore != (JLSemaphoreHandler)0;
	}

/*
	ALWAYS_INLINE int JLSemaphoreGetValue( JLSemaphoreHandler semaphore ) {

	#if defined(XP_WIN)
		LONG count;
		ReleaseSemaphore(semaphore, 0, &count); // INVALID
		return count;
	#elif defined(XP_UNIX)
		int value;
		sem_getvalue(semaphore, &value);
		return value;
	#endif
	}
*/

	// msTimeout = JLINFINITE : no timeout
	// returns true on a sucessfuly semaphore locked.
	static INLINE int JLSemaphoreAcquire( JLSemaphoreHandler semaphore, int msTimeout ) {

		JL_ASSERT( JLSemaphoreOk(semaphore) );
	#if defined(XP_WIN)
		DWORD status = WaitForSingleObject(semaphore, msTimeout == JLINFINITE ? INFINITE : msTimeout);
		JL_ASSERT( status != WAIT_FAILED );
		if ( status == WAIT_TIMEOUT )
			return JLTIMEOUT;
		return JLOK;
	#elif defined(XP_UNIX)
		int st;
		if ( msTimeout == JLINFINITE ) {

			// st = sem_wait(semaphore);
			while ((st = sem_wait(semaphore)) == -1 && errno == EINTR)
				continue; // Restart if interrupted by handler
			JL_ASSERT( st == 0 );
			return JLOK;
		} else {

			struct timespec ts;
			if ( msTimeout == 0 ) {

				ts.tv_sec = 0;
				ts.tv_nsec = 0;
			} else {

				// see also: struct timespec ts; clock_gettime(CLOCK_REALTIME, &ts); then link with -lrt
				//clock_gettime(CLOCK_REALTIME, &now)
				//reltime = sleep_til_this_absolute_time -now;
				//cond_relative_timed_wait(c, m, &reltime);

				struct timeval tv;
				if ( gettimeofday(&tv, NULL) != 0 ) // function shall return 0 and no value shall be reserved to indicate an error.
					return JLERROR;
				ts.tv_nsec = tv.tv_usec * 1000UL + (msTimeout % 1000)*1000000UL;
				ts.tv_sec = tv.tv_sec + msTimeout / 1000UL + ts.tv_nsec / 1000000000UL;
				ts.tv_nsec %= 1000000000UL;
			}
			// st = sem_timedwait(semaphore, &ts);
			while ((st = sem_timedwait(semaphore, &ts)) == -1 && errno == EINTR)
				continue; // Restart if interrupted by handler
			if ( st == -1 && errno == ETIMEDOUT )
				return JLTIMEOUT;
			JL_ASSERT( st == 0 );
			JL_UNUSED( st );
			return JLOK;
		}
		return JLERROR;
	#endif
	}

	static ALWAYS_INLINE void JLSemaphoreRelease( JLSemaphoreHandler semaphore ) {

		JL_ASSERT( JLSemaphoreOk(semaphore) );
	#if defined(XP_WIN)
		BOOL st = ReleaseSemaphore(semaphore, 1, NULL);
		JL_ASSERT( st != FALSE );
	#elif defined(XP_UNIX)
		int st = sem_post(semaphore);
		JL_ASSERT( st == 0 );
	#endif
		JL_UNUSED( st );
	}

	static ALWAYS_INLINE void JLSemaphoreFree( JLSemaphoreHandler *pSemaphore ) {

		JL_ASSERT( pSemaphore != NULL && JLSemaphoreOk(*pSemaphore) );
	#if defined(XP_WIN)
		BOOL st = CloseHandle(*pSemaphore);
		JL_ASSERT( st != FALSE );
	#elif defined(XP_UNIX)
		int st = sem_destroy(*pSemaphore);
		JL_ASSERT( st == 0 );
		free(*pSemaphore);
	#endif
		*pSemaphore = (JLSemaphoreHandler)0;
		JL_UNUSED(st);
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


	typedef struct __JSMUtexHandler {
	#if defined(XP_WIN)
		CRITICAL_SECTION cs;
		// HANDLE mx;
	#elif defined(XP_UNIX)
		pthread_mutex_t mx;
	#endif
	} *JLMutexHandler;

	#define JLMutexInvalidHandler ((JLMutexHandler)0)

	static ALWAYS_INLINE bool JLMutexOk( JLMutexHandler mutex ) {

		return mutex != (JLMutexHandler)0;
	}

	static ALWAYS_INLINE JLMutexHandler JLMutexCreate() {

		JLMutexHandler mutex = (JLMutexHandler)malloc(sizeof(*mutex));
		if ( mutex == NULL )
			return NULL;
	#if defined(XP_WIN)
		InitializeCriticalSection(&mutex->cs);
//		mutex->mx = CreateMutex(NULL, FALSE, NULL);
//		if ( mutex->mx == NULL )
//			return NULL;
	#elif defined(XP_UNIX)
		int st = pthread_mutex_init(&mutex->mx, NULL);
		JL_ASSERT( st == 0 );
		JL_UNUSED( st );
	#endif
		return mutex;
	}

	static ALWAYS_INLINE void JLMutexFree( JLMutexHandler *pMutex ) {

		JL_ASSERT( *pMutex != NULL && JLMutexOk(*pMutex) );
	#if defined(XP_WIN)
		DeleteCriticalSection(&(*pMutex)->cs);
//		CloseHandle((*mutex)->mx);
	#elif defined(XP_UNIX)
		int st = pthread_mutex_destroy(&(*pMutex)->mx);
		JL_ASSERT( st == 0 );
		JL_UNUSED( st );
	#endif
		free(*pMutex);
		*pMutex = (JLMutexHandler)0;
	}

	static ALWAYS_INLINE void JLMutexAcquire( JLMutexHandler mutex ) {

		JL_ASSERT( JLMutexOk(mutex) );
	#if defined(XP_WIN)
		EnterCriticalSection(&mutex->cs);
//		WaitForSingleObject(mutex->mx, INFINITE);
	#elif defined(XP_UNIX)
		int st = pthread_mutex_lock(&mutex->mx); // doc. shall not return an error code of [EINTR].
		JL_ASSERT( st == 0 );
		JL_UNUSED( st );
	#endif
	}

	static ALWAYS_INLINE void JLMutexRelease( JLMutexHandler mutex ) {

		JL_ASSERT( JLMutexOk(mutex) );
	#if defined(XP_WIN)
		LeaveCriticalSection(&mutex->cs);
//		ReleaseMutex(mutex->mx);
	#elif defined(XP_UNIX)
		int st = pthread_mutex_unlock(&mutex->mx);
		JL_ASSERT( st == 0 );
		JL_UNUSED( st );
	#endif
	}


///////////////////////////////////////////////////////////////////////////////
// Condition Variables
//
// see http://www.cs.wustl.edu/~schmidt/win32-cv-1.html (Strategies for Implementing POSIX Condition Variables on Win32)


#if defined(XP_WIN)

typedef struct __JLCondHandler {
	unsigned int waiters_count;
	// doc. waiters_count_lock_ protects the waiters_count_ from being corrupted by race conditions.
	//      This is not necessary as long as pthread_cond_signal and pthread_cond_broadcast are always called by a thread that has locked the same external_mutex used by pthread_cond_wait.
	// CRITICAL_SECTION waiters_count_lock;
	HANDLE events[2]; // [signal, broadcast]
} *JLCondHandler;


static ALWAYS_INLINE bool JLCondOk( JLCondHandler cv ) {

	return cv != (JLCondHandler)0;
}

static INLINE JLCondHandler JLCondCreate() {

	JLCondHandler cv = (JLCondHandler)malloc(sizeof(*cv));
	if ( cv == NULL )
		return NULL;
	cv->waiters_count = 0;
//	InitializeCriticalSection(&cv->waiters_count_lock);
	cv->events[0] = CreateEvent(NULL, FALSE, FALSE, NULL);
	JL_ASSERT( cv->events[0] != NULL );
	cv->events[1] = CreateEvent(NULL, TRUE, FALSE, NULL);
	JL_ASSERT( cv->events[1] != NULL );
	return cv;
}

static INLINE void JLCondFree( JLCondHandler *cv ) {

	JL_ASSERT( cv != NULL && JLCondOk(*cv) );
	BOOL st = CloseHandle((*cv)->events[1]);
	JL_ASSERT( st != FALSE );
	st = CloseHandle((*cv)->events[0]);
	JL_ASSERT( st != FALSE );
	JL_UNUSED( st );
//	DeleteCriticalSection(&(*cv)->waiters_count_lock);
	free(*cv);
	*cv = NULL;
}

static INLINE int JLCondWait( JLCondHandler cv, JLMutexHandler external_mutex ) {

	JL_ASSERT( JLCondOk(cv) );
//	EnterCriticalSection(&cv->waiters_count_lock);
	cv->waiters_count++;
//	LeaveCriticalSection(&cv->waiters_count_lock);
	JLMutexRelease(external_mutex);
	int result = WaitForMultipleObjects(2, cv->events, FALSE, INFINITE);
	JL_ASSERT( result != WAIT_FAILED );
//	EnterCriticalSection(&cv->waiters_count_lock);
	cv->waiters_count--;
	int last_waiter = result == WAIT_OBJECT_0 + 1 && cv->waiters_count == 0;
//	LeaveCriticalSection(&cv->waiters_count_lock);
	if ( last_waiter ) {

		BOOL st = ResetEvent(cv->events[1]);
		JL_ASSERT( st != FALSE );
		JL_UNUSED(st);
	}
	JLMutexAcquire(external_mutex);
	return JLOK;
}

static ALWAYS_INLINE void JLCondBroadcast( JLCondHandler cv ) {

	JL_ASSERT( JLCondOk(cv) );
//	EnterCriticalSection(&cv->waiters_count_lock);
	int have_waiters = cv->waiters_count > 0;
//	LeaveCriticalSection(&cv->waiters_count_lock);
	if ( have_waiters ) {

		BOOL st = SetEvent(cv->events[1]);
		JL_ASSERT( st != FALSE );
		JL_UNUSED(st);
	}
}

static ALWAYS_INLINE void JLCondSignal( JLCondHandler cv ) {

	JL_ASSERT( JLCondOk(cv) );
//	EnterCriticalSection(&cv->waiters_count_lock);
	int have_waiters = cv->waiters_count > 0;
//	LeaveCriticalSection(&cv->waiters_count_lock);
	if ( have_waiters ) {

		BOOL st = SetEvent(cv->events[0]);
		JL_ASSERT( st != FALSE );
		JL_UNUSED(st);
	}
}



/*
typedef struct {
	int waiters_count_;
	// Number of waiting threads.

//	CRITICAL_SECTION global_lock;
	CRITICAL_SECTION waiters_count_lock_;
	// Serialize access to <waiters_count_>.

	HANDLE sema_;
	// Semaphore used to queue up threads waiting for the condition to
	// become signaled.

	HANDLE waiters_done_;
	// An auto-reset event used by the broadcast/signal thread to wait
	// for all the waiting thread(s) to wake up and be released from the
	// semaphore.

	size_t was_broadcast_;
	// Keeps track of whether we were broadcasting or signaling.  This
	// allows us to optimize the code if we're just signaling.
} *JLCondHandler;


ALWAYS_INLINE bool JLCondOk( JLCondHandler cv ) {

	return cv != (JLCondHandler)0;
}

ALWAYS_INLINE JLCondHandler JLCondCreate() {

	JLCondHandler cv = (JLCondHandler)malloc(sizeof(*cv));
	cv->waiters_count_ = 0;
	cv->was_broadcast_ = 0;
	cv->sema_ = CreateSemaphore(NULL, 0, 0x7fffffff, NULL);
	InitializeCriticalSection (&cv->waiters_count_lock_);
//	InitializeCriticalSection (&cv->global_lock);
	cv->waiters_done_ = CreateEvent (NULL, FALSE, FALSE, NULL);
	return cv;
}


ALWAYS_INLINE void JLCondFree( JLCondHandler *cv ) {

	JL_ASSERT( JLCondOk(*cv) );
	BOOL st = CloseHandle((*cv)->sema_);
	JL_ASSERT( st );
	JL_UNUSED( st );
	DeleteCriticalSection(&(*cv)->waiters_count_lock_);
//	DeleteCriticalSection(&(*cv)->global_lock);
	free(*cv);
	*cv = NULL;
}


ALWAYS_INLINE int JLCondWait( JLCondHandler cv, JLMutexHandler external_mutex ) {

//  EnterCriticalSection (&cv->global_lock);

  EnterCriticalSection (&cv->waiters_count_lock_);
  cv->waiters_count_++;
  LeaveCriticalSection (&cv->waiters_count_lock_);

  SignalObjectAndWait(external_mutex->mx, cv->sema_, INFINITE, FALSE);

  // Reacquire lock to avoid race conditions.
  EnterCriticalSection (&cv->waiters_count_lock_);

  // We're no longer waiting...
  cv->waiters_count_--;

  // Check to see if we're the last waiter after <pthread_cond_broadcast>.
  int last_waiter = cv->was_broadcast_ && cv->waiters_count_ == 0;

  LeaveCriticalSection (&cv->waiters_count_lock_);

  // If we're the last waiter thread during this particular broadcast
  // then let all the other threads proceed.
  if (last_waiter)
    // This call atomically signals the <waiters_done_> event and waits until
    // it can acquire the <external_mutex>.  This is required to ensure fairness.
	 SignalObjectAndWait (cv->waiters_done_, external_mutex->mx, INFINITE, FALSE);
  else
    // Always regain the external mutex since that's the guarantee we
    // give to our callers.
	 WaitForSingleObject(external_mutex->mx, INFINITE);

//  LeaveCriticalSection (&cv->global_lock);

  return JLOK;
}


ALWAYS_INLINE void JLCondSignal( JLCondHandler cv ) {

//  EnterCriticalSection (&cv->global_lock);

  EnterCriticalSection (&cv->waiters_count_lock_);
  int have_waiters = cv->waiters_count_ > 0;
  LeaveCriticalSection (&cv->waiters_count_lock_);

  // If there aren't any waiters, then this is a no-op.
  if (have_waiters)
    ReleaseSemaphore (cv->sema_, 1, 0);

//  LeaveCriticalSection (&cv->global_lock);
}


ALWAYS_INLINE void JLCondBroadcast( JLCondHandler cv ) {

//  EnterCriticalSection (&cv->global_lock);

  // This is needed to ensure that <waiters_count_> and <was_broadcast_> are
  // consistent relative to each other.
  EnterCriticalSection (&cv->waiters_count_lock_);
  int have_waiters = 0;

  if (cv->waiters_count_ > 0) {
    // We are broadcasting, even if there is just one waiter...
    // Record that we are broadcasting, which helps optimize
    // <pthread_cond_wait> for the non-broadcast case.
    cv->was_broadcast_ = 1;
    have_waiters = 1;
  }

  if (have_waiters) {
    // Wake up all the waiters atomically.
    ReleaseSemaphore (cv->sema_, cv->waiters_count_, 0);

    LeaveCriticalSection (&cv->waiters_count_lock_);

    // Wait for all the awakened threads to acquire the counting
    // semaphore.
    WaitForSingleObject (cv->waiters_done_, INFINITE);
    // This assignment is okay, even without the <waiters_count_lock_> held
    // because no other waiter threads can wake up to access it.
    cv->was_broadcast_ = 0;
  }
  else
    LeaveCriticalSection (&cv->waiters_count_lock_);

//  LeaveCriticalSection (&cv->global_lock);
}
*/

#elif defined(XP_UNIX)

typedef struct __JLCondHandler {
	pthread_cond_t cond;
} *JLCondHandler;

static ALWAYS_INLINE bool JLCondOk( JLCondHandler cv ) {

	return cv != (JLCondHandler)0;
}

static ALWAYS_INLINE JLCondHandler JLCondCreate() {

	JLCondHandler cv = (JLCondHandler)malloc(sizeof(*cv));
//	cv->cond = PTHREAD_COND_INITIALIZER;
	pthread_cond_init(&cv->cond, NULL);
	return cv;
}

static ALWAYS_INLINE void JLCondFree( JLCondHandler *cv ) {

	JL_ASSERT( cv != NULL && JLCondOk(*cv) );
	pthread_cond_destroy(&(*cv)->cond);
	free(*cv);
	*cv = NULL;
}

static ALWAYS_INLINE int JLCondWait( JLCondHandler cv, JLMutexHandler external_mutex ) {

	JL_ASSERT( JLCondOk(cv) );
	pthread_cond_wait(&cv->cond, &external_mutex->mx); // doc. shall not return an error code of [EINTR].
	return JLOK;
}

static ALWAYS_INLINE void JLCondBroadcast( JLCondHandler cv ) {

	JL_ASSERT( JLCondOk(cv) );
	pthread_cond_broadcast(&cv->cond);
}

static ALWAYS_INLINE void JLCondSignal( JLCondHandler cv ) {

	JL_ASSERT( JLCondOk(cv) );
	pthread_cond_signal(&cv->cond);
}

#endif


///////////////////////////////////////////////////////////////////////////////
// event
//
// 5. Implementing Events on non-Win32 Platforms ( http://www.cs.wustl.edu/~schmidt/win32-cv-2.html )

	typedef struct __JLEventHandler {
	#if defined(XP_WIN)
		HANDLE hEvent;
		LONG waitingThreadCount;
		CRITICAL_SECTION cs;
	#elif defined(XP_UNIX)
		pthread_mutex_t mutex;
		pthread_cond_t cond;
		bool triggered;
	#endif
		bool autoReset;
	} *JLEventHandler;

	static ALWAYS_INLINE bool JLEventOk( JLEventHandler ev ) {

		return ev != (JLEventHandler)0;
	}

	static ALWAYS_INLINE JLEventHandler JLEventCreate( bool autoReset ) {

		JLEventHandler ev = (JLEventHandler)malloc(sizeof(*ev));
		if ( ev == NULL )
			return NULL;
	#if defined(XP_WIN)
		InitializeCriticalSection(&ev->cs);
		ev->waitingThreadCount = 0;
		ev->hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		if ( ev->hEvent == NULL )
			return NULL;
	#elif defined(XP_UNIX)
		pthread_mutex_init(&ev->mutex, 0);
		pthread_cond_init(&ev->cond, 0);
		ev->triggered = false;
	#endif
		ev->autoReset = autoReset;
		return ev;
	}

	static ALWAYS_INLINE void JLEventFree( JLEventHandler *ev ) {

		JL_ASSERT( ev != NULL && JLEventOk(*ev) );
	#if defined(XP_WIN)
		DeleteCriticalSection(&(*ev)->cs);
		BOOL st = CloseHandle((*ev)->hEvent);
		JL_ASSERT( st != FALSE );
	#elif defined(XP_UNIX)
		int st = pthread_cond_destroy(&(*ev)->cond);
		JL_ASSERT( st == 0 );
		st = pthread_mutex_destroy(&(*ev)->mutex);
		JL_ASSERT( st == 0 );
	#endif
		free(*ev);
		*ev = NULL;
		JL_UNUSED(st);
	}

	static INLINE void JLEventTrigger( JLEventHandler ev ) {

		JL_ASSERT( JLEventOk(ev) );
	#if defined(XP_WIN)

		EnterCriticalSection(&ev->cs);
		if ( ev->waitingThreadCount != 0 || !ev->autoReset ) {

			BOOL st = SetEvent(ev->hEvent);
			JL_ASSERT( st != FALSE );
			JL_UNUSED(st);
		}
		LeaveCriticalSection(&ev->cs);

	#elif defined(XP_UNIX)
		pthread_mutex_lock(&ev->mutex);
		ev->triggered = true;
		pthread_cond_broadcast(&ev->cond); // pthread_cond_signal
		if ( ev->autoReset )
			ev->triggered = false;
		pthread_mutex_unlock(&ev->mutex);
	#endif
	}

	static ALWAYS_INLINE void JLEventReset( JLEventHandler ev ) {

		JL_ASSERT( JLEventOk(ev) );
	#if defined(XP_WIN)

		EnterCriticalSection(&ev->cs);
		BOOL st = ResetEvent(ev->hEvent);
		JL_ASSERT( st != FALSE );
		JL_UNUSED(st);
		LeaveCriticalSection(&ev->cs);

	#elif defined(XP_UNIX)
		pthread_mutex_lock(&ev->mutex);
		ev->triggered = false;
		pthread_mutex_unlock(&ev->mutex);
	#endif
	}

	// msTimeout = JLINFINITE : no timeout
	static INLINE int JLEventWait( JLEventHandler ev, int msTimeout ) {

		JL_ASSERT( JLEventOk(ev) );
	#if defined(XP_WIN)

		EnterCriticalSection(&ev->cs);
		ev->waitingThreadCount++;
		LeaveCriticalSection(&ev->cs);

		DWORD status = WaitForSingleObject(ev->hEvent, msTimeout == JLINFINITE ? INFINITE : msTimeout);
		JL_ASSERT( status != WAIT_FAILED );

		EnterCriticalSection(&ev->cs);
		ev->waitingThreadCount--;
		if ( ev->waitingThreadCount == 0 && ev->autoReset ) {

			BOOL st = ResetEvent(ev->hEvent);
			JL_ASSERT( st == TRUE );
			JL_UNUSED(st);
		}
		LeaveCriticalSection(&ev->cs);
		if ( status == WAIT_TIMEOUT )
			return JLTIMEOUT;
		return JLOK;

	#elif defined(XP_UNIX)
		int st;
		if ( msTimeout == JLINFINITE ) {

		  pthread_mutex_lock(&ev->mutex);
		  while ( !ev->triggered )
				pthread_cond_wait(&ev->cond, &ev->mutex); // doc. shall not return an error code of [EINTR].
		  pthread_mutex_unlock(&ev->mutex);
		  return JLOK;
		} else {

			struct timespec ts;
			if ( msTimeout == 0 ) {

				ts.tv_sec = 0;
				ts.tv_nsec = 0;
			} else {

				struct timeval tv;
				if ( gettimeofday(&tv, NULL) != 0 )
					return JLERROR;
				ts.tv_nsec = tv.tv_usec * 1000UL + (msTimeout % 1000)*1000000UL;
				ts.tv_sec = tv.tv_sec + msTimeout / 1000UL + ts.tv_nsec / 1000000000UL;
				ts.tv_nsec %= 1000000000UL;
			}

			st = 0;
			pthread_mutex_lock(&ev->mutex);
			while ( !ev->triggered && st == 0 )
				st = pthread_cond_timedwait(&ev->cond, &ev->mutex, &ts); // doc. shall not return an error code of [EINTR].
			pthread_mutex_unlock(&ev->mutex);
			if ( st == -1 && errno == ETIMEDOUT )
				return JLTIMEOUT;
			JL_ASSERT( st == 0 );
			JL_UNUSED(st);
			return JLOK;
		}
		return JLERROR;
	#endif
	}


///////////////////////////////////////////////////////////////////////////////
// thread
//   Linux: https://computing.llnl.gov/tutorials/pthreads/#PthreadsAPI

	#if defined(XP_WIN)
		#define JL_THREAD_PRIORITY_LOWEST THREAD_PRIORITY_LOWEST
		#define JL_THREAD_PRIORITY_LOW THREAD_PRIORITY_BELOW_NORMAL
		#define JL_THREAD_PRIORITY_NORMAL THREAD_PRIORITY_NORMAL
		#define JL_THREAD_PRIORITY_HIGH THREAD_PRIORITY_ABOVE_NORMAL
		#define JL_THREAD_PRIORITY_HIGHEST THREAD_PRIORITY_HIGHEST
		typedef HANDLE JLThreadHandler;
		typedef int JLThreadPriorityType;
		#define JLThreadFuncDecl DWORD WINAPI
		typedef PTHREAD_START_ROUTINE JLThreadRoutine;
	#elif defined(XP_UNIX)
		#define JL_THREAD_PRIORITY_LOWEST 32
		#define JL_THREAD_PRIORITY_LOW 48
		#define JL_THREAD_PRIORITY_NORMAL 64
		#define JL_THREAD_PRIORITY_HIGH 80
		#define JL_THREAD_PRIORITY_HIGHEST 96
		typedef pthread_t* JLThreadHandler;
		typedef int JLThreadPriorityType;
		#define JLThreadFuncDecl void*
		typedef void*(*JLThreadRoutine)(void *);
	#endif


	static ALWAYS_INLINE bool JLThreadOk( JLThreadHandler thread ) {

		return thread != (JLThreadHandler)0;
	}


	static ALWAYS_INLINE JLThreadHandler JLThreadStart( JLThreadRoutine threadRoutine, void *pv ) {

	#if defined(XP_WIN)
		return CreateThread(NULL, 0, threadRoutine, pv, 0, NULL); // (TBD) need THREAD_TERMINATE ?
	#elif defined(XP_UNIX)
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


	static ALWAYS_INLINE bool JLThreadIsActive( JLThreadHandler thread ) {  // (TBD) how to manage errors ?

		JL_ASSERT( JLThreadOk(thread) );
	#if defined(XP_WIN)
		DWORD status = WaitForSingleObject(thread, 0);
		JL_ASSERT( status != WAIT_FAILED );
		return status == WAIT_TIMEOUT; // else != WAIT_OBJECT_0 ?
	#elif defined(XP_UNIX)
		int policy;
		struct sched_param param;
		return pthread_getschedparam(*thread, &policy, &param) != ESRCH; // errno.h
	#endif
	}


	static ALWAYS_INLINE void JLThreadFree( JLThreadHandler *pThread ) {

		JL_ASSERT( pThread != NULL && JLThreadOk(*pThread) );
	#if defined(XP_WIN)
		BOOL st = CloseHandle(*pThread);
		JL_ASSERT( st != FALSE );
		JL_UNUSED(st);
	#elif defined(XP_UNIX)
		if ( JLThreadIsActive( *pThread ) ) {

			int st = pthread_detach(**pThread);
			JL_ASSERT( st == 0 );
			JL_UNUSED(st);
		}
		free(*pThread);
	#endif
		*pThread = (JLThreadHandler)0;
	}


	static ALWAYS_INLINE void JLThreadExit( unsigned int exitValue ) {

	#if defined(XP_WIN)
		ExitThread(exitValue);
	#elif defined(XP_UNIX)
		pthread_exit((void*)exitValue);
	#endif
	}


	static ALWAYS_INLINE void JLThreadCancel( JLThreadHandler thread ) {

		JL_ASSERT( JLThreadOk(thread) );
	#if defined(XP_WIN)
		BOOL st = TerminateThread(thread, 0); // doc. The handle must have the THREAD_TERMINATE access right. ... Use the GetExitCodeThread function to retrieve a thread's exit value.
		JL_ASSERT( st != 0 );
	#elif defined(XP_UNIX)
		int st = pthread_cancel(*thread);
		JL_ASSERT( st == 0 );
	#endif
		JL_UNUSED(st);
	}


	static ALWAYS_INLINE void JLThreadPriority( JLThreadHandler thread, JLThreadPriorityType priority ) {

		JL_ASSERT( JLThreadOk(thread) );
	#if defined(XP_WIN)
		BOOL st = SetThreadPriority(thread, priority);
		JL_ASSERT( st != FALSE );
	#elif defined(XP_UNIX)
		int st;
		int policy;
		struct sched_param param;
		st = pthread_getschedparam(*thread, &policy, &param);
		JL_ASSERT( st == 0 );
		int max = sched_get_priority_max(policy);
		int min = sched_get_priority_min(policy);
		param.sched_priority = min + (max - min) * priority / 128;
		st = pthread_setschedparam(*thread, policy, &param);
		JL_ASSERT( st == 0 );
	#endif
		JL_UNUSED(st);
	}


	static ALWAYS_INLINE void JLThreadWait( JLThreadHandler thread, unsigned int *exitValue ) {

		JL_ASSERT( JLThreadOk(thread) );
	#if defined(XP_WIN)
		BOOL st = WaitForSingleObject(thread, INFINITE);
		JL_ASSERT( st == WAIT_OBJECT_0 );
		if ( exitValue )
			GetExitCodeThread(thread, (DWORD*)exitValue);
	#elif defined(XP_UNIX)
		int st = pthread_join(*thread, (void**)exitValue); // doc. The thread exit status returned by pthread_join() on a canceled thread is PTHREAD_CANCELED. pthread_join shall not return an error code of [EINTR].
		JL_ASSERT( st == 0 );
	#endif
		JL_UNUSED(st);
	}

///////////////////////////////////////////////////////////////////////////////
// thread-local storage
//

#if defined(XP_WIN)
	typedef DWORD JLTLSKey;
#elif defined(XP_UNIX)
	typedef pthread_key_t JLTLSKey;
#endif

#define JLTLSInvalidKey 0

static ALWAYS_INLINE JLTLSKey JLTLSAllocKey() {
	JLTLSKey key;
#if defined(XP_WIN)
	key = TlsAlloc();
	JL_ASSERT( key != TLS_OUT_OF_INDEXES );
	key++;
#elif defined(XP_UNIX)
	int st = pthread_key_create(&key, NULL);
	JL_UNUSED( st );
	JL_ASSERT( st == 0 );
#endif
	return key;
}


static ALWAYS_INLINE void JLTLSFreeKey( JLTLSKey key ) {
#if defined(XP_WIN)
	JL_ASSERT( key != 0 );
	key--;
	JL_ASSERT( key >= 0 && key < TLS_MINIMUM_AVAILABLE );
	BOOL st = TlsFree(key);
	JL_ASSERT( st != FALSE );
#elif defined(XP_UNIX)
	int st = pthread_key_delete(key);
	JL_ASSERT( st == 0 );
#endif
	JL_UNUSED(st);
}


static ALWAYS_INLINE void JLTLSSet( JLTLSKey key, void *value ) {
#if defined(XP_WIN)
	JL_ASSERT( key != 0 );
	key--;
	JL_ASSERT( key >= 0 && key < TLS_MINIMUM_AVAILABLE );
	BOOL st = TlsSetValue(key, value);
	JL_ASSERT( st != FALSE );
#elif defined(XP_UNIX)
	int st = pthread_setspecific(key, value);
	JL_ASSERT( st == 0 );
#endif
	JL_UNUSED(st);
}


static ALWAYS_INLINE void* JLTLSGet( JLTLSKey key ) {
#if defined(XP_WIN)
	JL_ASSERT( key != 0 );
	key--;
	void *value;
	JL_ASSERT( key >= 0 && key < TLS_MINIMUM_AVAILABLE );
	value = TlsGetValue(key);
	JL_ASSERT( value != 0 || GetLastError() == ERROR_SUCCESS  );
	return value;
#elif defined(XP_UNIX)
	return pthread_getspecific(key);
#endif
}


///////////////////////////////////////////////////////////////////////////////
// dynamic libraries
//

#if defined(XP_WIN)
	typedef HMODULE JLLibraryHandler;
#elif defined(XP_UNIX)
	typedef void* JLLibraryHandler;
#endif

	static ALWAYS_INLINE bool JLDynamicLibraryOk( JLLibraryHandler libraryHandler ) {

		return libraryHandler != (JLLibraryHandler)0;
	}

	static ALWAYS_INLINE uintptr_t JLDynamicLibraryId( JLLibraryHandler libraryHandler ) {

		return (uint32_t)( ((uintptr_t)libraryHandler >> ALIGNOF(void*)) & 0xffffffff ); // shift useless and keep 32bits.
	}

	static ALWAYS_INLINE JLLibraryHandler JLDynamicLibraryOpen( const char *filename ) {

	#if defined(XP_WIN)
		// GetErrorMode() only exists on Vista and higher,
		// call SetErrorMode() twice to achieve the same effect.
		// see also SetThreadErrorMode()
//		UINT oldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
//		SetErrorMode( oldErrorMode | SEM_FAILCRITICALERRORS ); // avoid error popups
		HMODULE hModule = LoadLibrary(filename);
		// Restore previous error mode.
//		SetErrorMode(oldErrorMode);
		return hModule;
	#elif defined(XP_UNIX)
		dlerror(); // Resets the error indicator.
		return dlopen(filename, RTLD_LAZY | RTLD_LOCAL); // RTLD_NOW / RTLD_GLOBAL
	#endif
	}

	static ALWAYS_INLINE void JLDynamicLibraryClose( JLLibraryHandler *libraryHandler ) {

	#if defined(XP_WIN)
		BOOL st = FreeLibrary(*libraryHandler);
		JL_ASSERT( st != FALSE );
	#elif defined(XP_UNIX)
		dlerror(); // Resets the error indicator.
		int st = dlclose(*libraryHandler);
		JL_ASSERT( st == 0 );
	#endif
		*libraryHandler = (JLLibraryHandler)0;
		JL_UNUSED(st);
	}

	static INLINE void JLDynamicLibraryLastErrorMessage( char *message, size_t maxLength ) {

	#if defined(XP_WIN)
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
	#elif defined(XP_UNIX)
		const char *msgBuf = dlerror();
		if ( msgBuf != NULL ) {

			strncpy(message, msgBuf, maxLength-1);
			message[maxLength-1] = '\0';
		} else
			*message = '\0';
	#endif
	}

	static ALWAYS_INLINE void *JLDynamicLibrarySymbol( JLLibraryHandler libraryHandler, const char *symbolName ) {

	#if defined(XP_WIN)
		return (void*)GetProcAddress(libraryHandler, symbolName);
	#elif defined(XP_UNIX)
		dlerror(); // Resets the error indicator.
		return dlsym(libraryHandler, symbolName);
	#endif
	}

#endif // _JLPLATFORM_H_
