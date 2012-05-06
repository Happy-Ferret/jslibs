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

#pragma once


///////////////////////////////////////////////////////////////////////////////
// Miscellaneous

#define JL_BEGIN_NAMESPACE namespace jl {
#define JL_END_NAMESPACE }


#define JL_STRINGIFY(x) #x
#define JL_TOSTRING(x) JL_STRINGIFY(x)


// from jstypes.h
#define JL_MACRO_BEGIN do {

#if defined(_MSC_VER) && _MSC_VER >= 1400
	#define JL_MACRO_END } __pragma(warning(push)) __pragma(warning(disable:4127)) while (0) __pragma(warning(pop))
#else
	#define JL_MACRO_END   } while (0)
#endif

#define JL_CODE_LOCATION __FILE__ ":" JL_TOSTRING(__LINE__)

#define __DATE__YEAR ((((__DATE__ [7] - '0') * 10 + (__DATE__ [8] - '0')) * 10 + (__DATE__ [9] - '0')) * 10 + (__DATE__ [10] - '0'))
#define __DATE__MONTH ((__DATE__ [2] == 'n' ? 0 : __DATE__ [2] == 'b' ? 1 : __DATE__ [2] == 'r' ? (__DATE__ [0] == 'M' ? 2 : 3) : __DATE__ [2] == 'y' ? 4 : __DATE__ [2] == 'n' ? 5 : __DATE__ [2] == 'l' ? 6 : __DATE__ [2] == 'g' ? 7 : __DATE__ [2] == 'p' ? 8 : __DATE__ [2] == 't' ? 9 : __DATE__ [2] == 'v' ? 10 : 11)+1)
#define __DATE__DAY ((__DATE__ [4] == ' ' ? 0 : __DATE__ [4] - '0') * 10 + (__DATE__ [5] - '0'))
#define __DATE__HOUR (((__TIME__[0]-'0')*10) + (__TIME__[1]-'0'))
#define __DATE__MINUTE (((__TIME__[3]-'0')*10) + (__TIME__[4]-'0'))
#define __DATE__SECOND (((__TIME__[6]-'0')*10) + (__TIME__[7]-'0'))
#define __DATE__EPOCH_DAYS (( __DATE__MONTH > 2 ? (__DATE__YEAR*365 + __DATE__YEAR/4 - __DATE__YEAR/100 + __DATE__YEAR/400 + (__DATE__MONTH+1) * 306001 / 10000 + __DATE__DAY) : ((__DATE__YEAR-1)*365 + (__DATE__YEAR-1)/4 - (__DATE__YEAR-1)/100 + (__DATE__YEAR-1)/400 + (__DATE__MONTH+13) * 306001 / 10000 + __DATE__DAY) ) - 719591)
#define __DATE__EPOCH (__DATE__EPOCH_DAYS * 86400 + __DATE__HOUR * 3600 + __DATE__MINUTE * 60 + __DATE__SECOND)



///////////////////////////////////////////////////////////////////////////////
// Compiler specific configuration

#if defined(_MSC_VER)
	// disable warnings:
	#pragma warning(disable : 4127) // no "conditional expression is constant" complaints
	#pragma warning(disable : 4996) // 'function': was declared deprecated
	#pragma warning(disable : 4201) // nonstandard extension used : nameless struct/union
	#pragma warning(disable : 4102) // unreferenced label
	#pragma warning(disable : 4702) // unreachable code
	#pragma warning(disable : 4227) // anachronism used : qualifiers on reference are ignored
	#pragma warning(disable : 4521) // 'XXX' : multiple copy constructors specified
	#ifndef DEBUG
		#pragma warning(disable : 4701) // potentially uninitialized local variable 'XXX' used
	#endif
	// force warning to error:
	#pragma warning(error : 4715) // not all control paths return a value
	#pragma warning(error : 4018) // '<' : signed/unsigned mismatch
	#pragma warning(error : 4309) // 'initializing' : truncation of constant value
	#pragma warning(error : 4700) // uninitialized local variable 'XXX' used
	#pragma warning(error : 4533) // initialization of 'xxx' is skipped by 'goto YYY'
	#pragma warning(error : 4002) // too many actual parameters for macro 'XXX'
	#pragma warning(error : 4003) // not enough actual parameters for macro 'XXX'
	#pragma warning(error : 4239) // nonstandard extension used
	#pragma warning(error : 4005) // 'XXX' : macro redefinition
	#pragma warning(error : 4717) // 'XXX' : recursive on all control paths, function will cause runtime stack overflow
	#pragma warning(error : 4508) // 'XXX' : function should return a value; 'void' return type assumed
	#pragma warning(error : 4800) // 'XXX' : forcing value to bool 'true' or 'false' (performance warning)
	#pragma warning(error : 4101) // 'XXX' : unreferenced local variable
	#ifdef DEBUG
		#pragma warning(error : 4701) // potentially uninitialized local variable 'XXX' used
	#endif
#endif // _MSC_VER


#if defined(__GNUC__)
	// # pragma GCC diagnostic ignored "-Wformat"  /* Ignore Warning about printf format /
	// # pragma GCC diagnostic ignored "-Wunused-parameter"  / Ignore Warning about unused function parameter */
	#pragma GCC diagnostic error "-Wdiv-by-zero"
#endif // __GNUC__



#if defined(_DEBUG)
	#if !defined(DEBUG)
		#define DEBUG
	#endif
#endif


#ifdef DEBUG
	#define IFDEBUG(expr) expr;
#else
	#define IFDEBUG(expr) 0
#endif // DEBUG


#ifdef DEBUG
	#define IS_DEBUG true
#else
	#define IS_DEBUG false
#endif // DEBUG


#if defined(__cplusplus)
	#define EXTERN_C extern "C"
#else
	#define EXTERN_C
#endif // __cplusplus


#if defined(__GNUC__) && (__GNUC__ > 2)
	#define likely(expr) __builtin_expect((expr), 1)
	#define unlikely(expr) __builtin_expect((expr), 0)
#else
	#define likely(expr) (expr)
	#define unlikely(expr) (expr)
#endif


// from jstypes.h
#if defined(_MSC_VER) && defined(_M_IX86)
# define FASTCALL __fastcall
#elif defined(__GNUC__) && defined(__i386__) && ((__GNUC__ >= 4) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4))
# define FASTCALL __attribute__((fastcall))
#else
# define FASTCALL
#endif


// trick:
//   When an inline function is not static, then the compiler must assume that there may be calls from other source files; since a global symbol can be defined only once in any program,
//   the function must not be defined in the other source files, so the calls therein cannot be integrated. Therefore, a non-static inline function is always compiled on its own in the usual fashion.

// from jstypes.h
#if defined __cplusplus
# define INLINE inline
#elif defined _MSC_VER
# define INLINE __inline
#elif defined __GNUC__
# define INLINE __inline__
#else
# define INLINE inline
#endif
// note: The compiler treats the inline expansion options and keywords as suggestions.


// from jstypes.h
#ifndef ALWAYS_INLINE
# if defined DEBUG
#  define ALWAYS_INLINE INLINE
# elif defined _MSC_VER
#  define ALWAYS_INLINE __forceinline /* __inline */
# elif defined __GNUC__
#  define ALWAYS_INLINE __attribute__((always_inline)) INLINE
# else
#  define ALWAYS_INLINE JS_INLINE
# endif
#endif


// from jstypes.h
// using  INLINE NEVER_INLINE void foo() {...}  is permitted.
#ifndef NEVER_INLINE
# if defined _MSC_VER
#  define NEVER_INLINE __declspec(noinline)
# elif defined __GNUC__
#  define NEVER_INLINE __attribute__((noinline))
# else
#  define NEVER_INLINE
# endif
#endif


#if defined DEBUG
#define NOIL(f) f
#else
template <class F> NEVER_INLINE F NOIL( F f ) { return f; }
#endif


#ifndef ASSUME
# if defined _MSC_VER
#  define ASSUME(expr) (__assume(expr))
# else
#  define ASSUME(expr) ((void)0)
# endif
#endif


// restrict says that the pointer is the only thing that accesses the underlying object. 
// see also. http://cellperformance.beyond3d.com/articles/2006/05/demystifying-the-restrict-keyword.html
// I certify that writes through this pointer will not effect the values read through any other pointer available in the same context which is also declared as restricted.
#ifndef RESTRICT
# if defined _MSC_VER
// msdn doc. the __restrict keyword indicates that a symbol is not aliased in the current scope
#  define RESTRICT __restrict
# else
// or. restrict is basically a promise to the compiler that for the scope of the pointer, the target of the pointer will only be accessed through that pointer (and pointers copied from it).
#  define RESTRICT __restrict__
# endif
#endif


#ifndef RESTRICT_DECL
# if defined _MSC_VER
#  define RESTRICT_DECL __declspec(restrict)
# else
#  define RESTRICT_DECL
# endif
#endif


// msdn doc. noalias means that a function call does not modify or reference visible global state and only modifies the memory pointed to directly by pointer parameters (first-level indirections).
#ifndef NOALIAS
# if defined _MSC_VER && _MSC_VER >= 1400
#  define NOALIAS __declspec(noalias)
# else
#  define NOALIAS
# endif
#endif


#if defined(_MSC_VER)
	#define DLLEXPORT __declspec(dllexport)
	#define DLLLOCAL
#elif defined(__GNUC__)
	// info. http://gcc.gnu.org/wiki/Visibility
	// Support of visibility attribute is mandatory to manage _moduleId scope (must be private but global for the .so).
	// If visibility hidden is not set for _moduleId,
	// DLLLOCAL uint32_t _moduleId = 0;
	// nm build/default/src/jsstd/jsstd | grep moduleId
	//   000168a8 B _moduleId   => uppercase B = global
	// and should be:
	//   000168a8 b _moduleId   => lowercase b = local
	#define DLLEXPORT __attribute__ ((visibility("default")))
	#define DLLLOCAL __attribute__ ((visibility("hidden")))
#else
	#error NOT IMPLEMENTED YET	// (TBD)
	#define DLLEXPORT
	#define DLLLOCAL
#endif


#if defined(_MSC_VER) && _MSC_VER >= 1100
#define NOVTABLE __declspec(novtable)
#else
#define NOVTABLE
#endif


#define NOTHROW throw()


///////////////////////////////////////////////////////////////////////////////

#include <sys/types.h>
#include <float.h>
#include <wchar.h>
#include <limits.h>
#include <cstddef>
#include <stdarg.h>
#include <errno.h>
#include <malloc.h>
#include <fcntl.h> // _O_RDONLY, _O_WRONLY
#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <cctype>

#define _USE_MATH_DEFINES
#include <math.h>

#if defined(_MSC_VER)
#include <intrin.h> // __cpuid()
#include <io.h> // _open_osfhandle()
#endif



///////////////////////////////////////////////////////////////////////////////
// Platform specific configuration

#if defined(_WINDOWS) || defined(WIN32) // Windows platform

#if defined(REPORT_MEMORY_LEAKS)
	// the following code make issue with jstl.h (js\src\jstl.h(244) : error C2039: '_malloc_dbg' : is not a member of 'JSContext')
	#ifdef _DEBUG
	# define _CRTDBG_MAP_ALLOC
	# include <crtdbg.h>
	#endif // _DEBUG
#endif // REPORT_MEMORY_LEAKS

	#ifndef XP_WIN
	#define XP_WIN // used by SpiderMonkey and jslibs
	#endif

	// doc: _WIN32_WINNT_WS03 = 0x0502 = Windows Server 2003 with SP1, Windows XP with SP2.
	// note: SpiderMionkey compilation option: --with-windows-version=502

	#ifndef WINVER         // Allow use of features specific to Windows 95 and Windows NT 4 or later.
	#define WINVER _WIN32_WINNT_WS03  // Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
	#endif

	#ifndef _WIN32_WINNT         // Allow use of features specific to Windows NT 4 or later.
	#define _WIN32_WINNT _WIN32_WINNT_WS03  // Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
	#endif

	#ifndef _WIN32_WINDOWS        // Allow use of features specific to Windows 98 or later.
	#define _WIN32_WINDOWS _WIN32_WINNT_WS03 // Change this to the appropriate value to target Windows Me or later. 0x501 = XP SP1.
	#endif

	#undef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN   // Exclude rarely-used stuff from Windows headers

	// Using STRICT to Improve Type Checking (http://msdn.microsoft.com/en-us/library/aa280394%28v=VS.60%29.aspx)
	#define STRICT 1

	// see WinDef.h
	#define NOMINMAX

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

	#define INT8_MIN     ((int8_t)_I8_MIN)
	#define INT8_MAX     _I8_MAX
	#define INT16_MIN    ((int16_t)_I16_MIN)
	#define INT16_MAX    _I16_MAX
	#define INT32_MIN    ((int32_t)_I32_MIN)
	#define INT32_MAX    _I32_MAX
	#define INT64_MIN    ((int64_t)_I64_MIN)
	#define INT64_MAX    _I64_MAX
	#define UINT8_MAX    _UI8_MAX
	#define UINT16_MAX   _UI16_MAX
	#define UINT32_MAX   _UI32_MAX
	#define UINT64_MAX   _UI64_MAX

	typedef float float32_t;
	typedef double float64_t;


	#define PATH_MAX MAX_PATH
	#define DLL_EXT ".dll"
	#define PATH_SEPARATOR_STRING "\\"
	#define PATH_SEPARATOR (PATH_SEPARATOR_STRING[0])
	#define LIST_SEPARATOR_STRING ";"
	#define LIST_SEPARATOR (LIST_SEPARATOR_STRING[0])

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

	#define snprintf _snprintf


#elif defined(_MACOSX) // MacosX platform

	#ifndef XP_UNIX
	#define XP_UNIX // used by SpiderMonkey and jslibs
	#endif

	#include <unistd.h>

	typedef float float32_t;
	typedef double float64_t;

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
	#include <time.h>
	#include <dlfcn.h>
	#define __STDC_LIMIT_MACROS
	#include <stdint.h>
	#include <signal.h>
	#include <sys/statvfs.h>

	#ifndef O_BINARY
	#define O_BINARY 0
	#endif // O_BINARY

	#ifndef O_SEQUENTIAL
	#define O_SEQUENTIAL 0
	#endif // O_SEQUENTIAL

	typedef float float32_t;
	typedef double float64_t;


	#define DLL_EXT ".so"
	#define PATH_SEPARATOR_STRING "/"
	#define PATH_SEPARATOR (PATH_SEPARATOR_STRING[0])
	#define LIST_SEPARATOR_STRING ":"
	#define LIST_SEPARATOR (LIST_SEPARATOR_STRING[0])

	static ALWAYS_INLINE size_t msize( void *ptr ) {

		return malloc_usable_size(ptr); // NULL-check of ptr seems not needed
		return 0;
	}

	// GCC 4.4.5+ has memalign
	//static ALWAYS_INLINE void* memalign( size_t alignment, size_t size ) {
	//
	//	void *ptr;
	//	posix_memalign(&ptr, alignment, size);
	//	return ptr;
	//}

#endif // Windows/MacosX/Linux platform


///////////////////////////////////////////////////////////////////////////////
// Miscellaneous

#define PLATFORM_BITS \
	(sizeof(ptrdiff_t) * 8)

#define S_ASSERT(cond) \
	extern void STATIC_ASSERT_DUMMY(int arg[(cond) ? 1 : -1])

#ifndef PTRDIFF_MAX
#define PTRDIFF_MIN ((ptrdiff_t)(-PTRDIFF_MAX - (ptrdiff_t)1))
#define PTRDIFF_MAX ((ptrdiff_t)(SIZE_MAX / 2))
#endif

#ifdef _MSC_VER
#define UNLIKELY_SPLIT_BEGIN(...) { struct { INLINE NEVER_INLINE JSBool FASTCALL operator()( ##__VA_ARGS__ ) {
#define UNLIKELY_SPLIT_END(...) } } inner; if ( inner( ##__VA_ARGS__ ) ) return JS_TRUE; else goto bad; JL_BAD; }
#else
#define UNLIKELY_SPLIT_BEGIN(...)
#define UNLIKELY_SPLIT_END(...)
#endif


#ifdef WIN32

#define L(CSTRING) (L##CSTRING)

#else

S_ASSERT(sizeof(wchar_t) == sizeof(uint16_t)); // see -fshort-wchar

ALWAYS_INLINE const uint16_t
LL(const wchar_t c) {
	return (const uint16_t)c;
}

ALWAYS_INLINE const uint16_t*
LL(const wchar_t *s) {
	return (const uint16_t*)s;
}

#define L(CSTRING) (LL(L##CSTRING))

#endif


//template<class T>
//static inline void JL_IGNORE(T) {};
//#define JL_IGNORE(x) x __attribute__((unused))
//#define JL_IGNORE(x) ((x) = (x))
#define JL_IGNORE(...) \
	((void)(__VA_ARGS__))


// see CrashInJS() in jsutil.cpp
ALWAYS_INLINE void
JL_Break() {
#if defined(WIN32)
# if defined(DEBUG)
	_asm { int 3 }
# endif // DEBUG
	*((volatile int *) NULL) = 123;
	exit(3);
#elif defined(__APPLE__)
	*((volatile int *) NULL) = 123;
	raise(SIGABRT);
#else
	raise(SIGABRT);
#endif
}


// from jsutil.cpp
ALWAYS_INLINE void
JL_AssertFailure( const char *message, const char *location ) {
	
	fprintf(stderr, "JL Assertion failure: %s @%s\n", message, location);
	fflush(stderr);
	JL_Break();
}


#ifdef DEBUG
#define ASSERT(expr) \
    ( (expr) ? (void)0 : JL_AssertFailure(#expr, JL_CODE_LOCATION) )
#define ASSERT_IF(cond, expr) \
    ( (!(cond) || (expr)) ? (void)0 : JL_AssertFailure(#expr, JL_CODE_LOCATION) )
#else // DEBUG
// beware. Use __assume in an ASSERT only when the assert is not recoverable.
#define ASSERT(expr) (ASSUME(expr))
#define ASSERT_IF(cond, expr) ((void)0)
#endif // DEBUG


///////////////////////////////////////////////////////////////////////////////
// Platform tools

/* SFINAE
template<typename T>
struct HasX {
	struct Fallback {
		int jldata;
	};
	struct Derived : T, Fallback {};
	template<typename C, C> struct ChT;    
	template<typename C> static char (&f(ChT<int Fallback::*, &C::jldata>*))[1];
	
	template<typename C> static char (&f(...))[2];
	static bool const value = sizeof(f<Derived>(0)) == 2;
};

struct A {
int jldatad;
};
	int b = HasX<A>::value;
	printf("has%d \n", b);
*/


template <class T>
struct Wrap {
	T _item;
	ALWAYS_INLINE explicit Wrap( T item ) : _item(item) {}
	ALWAYS_INLINE operator T() { return _item; }
};


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


// 2^53 = 9007199254740992
// since double(9007199254740992) == double(9007199254740993), and double(-9007199254740992) == double(-9007199254740993)  we must subtract 1.
// see also std::numeric_limits<double>::digits
S_ASSERT( DBL_MANT_DIG < 64 );

#define MAX_INT_TO_DOUBLE \
	((double)((((uint64_t)1)<<DBL_MANT_DIG)-1))

#define MIN_INT_TO_DOUBLE \
	(-MAX_INT_TO_DOUBLE)

// failed on linux because + Standard 2003, 5.19 "Constant expressions", paragraph 1.
// ... Floating literals (2.13.3) can appear only if they are cast to integral or enumeration types.
//S_ASSERT( MAX_INT_TO_DOUBLE != MAX_INT_TO_DOUBLE+(double)1 );
//S_ASSERT( MAX_INT_TO_DOUBLE+(double)1 == MAX_INT_TO_DOUBLE+(double)2 );



#define JL_MIN(a,b) ((a) < (b) ? (a) : (b))

#define JL_MAX(a,b) ((a) > (b) ? (a) : (b))

#define JL_MINMAX(v,a,b) ((v) > (b) ? (b) : (v) < (a) ? (a) : (v))


JL_BEGIN_NAMESPACE


ALWAYS_INLINE NOALIAS int
DoubleIsNegZero(const double &d) {
#ifdef WIN32
	return (d == 0 && (_fpclass(d) & _FPCLASS_NZ));
#elif defined(SOLARIS)
	return (d == 0 && copysign(1, d) < 0);
#else
	return (d == 0 && signbit(d));
#endif
}


ALWAYS_INLINE NOALIAS bool
DoubleIsNeg(const double &d) {
#ifdef WIN32
	return DoubleIsNegZero(d) || d < 0;
#elif defined(SOLARIS)
	return copysign(1, d) < 0;
#else
	return signbit(d);
#endif
}


ALWAYS_INLINE bool
IsInteger(double d) {

	return d == floor(d);
}


template<class T>
ALWAYS_INLINE T
IsSigned(T a) {

	return a > (T)-1;
}


template<class T, class U>
ALWAYS_INLINE bool
IsInRange(T val, U vmin, U vmax) {

	return val >= vmin && val <= vmax; // note: unsigned(val - vmin) <= unsigned(vmax - vmin) is 10 cycles faster.
}


// eg. if ( IsSafeCast<int>(size_t(12345)) ) ...
template <class D, class S>
ALWAYS_INLINE bool
IsSafeCast(S src) {

	D dsrc = static_cast<D>(src);
	return static_cast<S>(dsrc) == src && (src < 0) == (dsrc < 0); // compare converted value and sign.
}


// eg. int i; if ( IsSafeCast(size_t(12345), i) ) ...
template <class D, class S>
ALWAYS_INLINE bool
IsSafeCast(S src, D) {

	D dsrc = static_cast<D>(src);
	return static_cast<S>(dsrc) == src && (src < 0) == (dsrc < 0); // compare converted value and sign.
}


// eg. int i = SafeCast<int>(size_t(12345));
template <class D, class S>
ALWAYS_INLINE D
SafeCast(S src) {

	ASSERT( (IsSafeCast<D>(src)) );
	return static_cast<D>(src);
}


// eg. int i; i = SafeCast(size_t(12345), i);
template <class D, class S>
ALWAYS_INLINE D
SafeCast(S src, D) {

	ASSERT( (IsSafeCast<D>(src)) );
	return static_cast<D>(src);
}


//Macro that avoid multicharacter constant: From gcc page:
//`-Wno-multichar'
//     Do not warn if a multicharacter constant (`'FOOF'') is used.
//     Usually they indicate a typo in the user's code, as they have
//     implementation-defined values, and should not be used in portable
//     code.

ALWAYS_INLINE NOALIAS uint32_t
CastCStrToUint32( const char *cstr ) {

	ASSERT( cstr != NULL );
	ASSERT( !(cstr[0] && cstr[1] && cstr[2] && cstr[3] && cstr[4]) );
	return
		!cstr[0] ? 0 :
		!cstr[1] ? cstr[0] :
		!cstr[2] ? (cstr[0] <<  8) | cstr[1] :
		!cstr[3] ? (cstr[0] << 16) | (cstr[1] <<  8) | cstr[2] :
		           (cstr[0] << 24) | (cstr[1] << 16) | (cstr[2] <<  8) | cstr[3];
}


ALWAYS_INLINE void *
memcpy(void *dst_, const void *src_, size_t len) {
    
	char *dst = (char *) dst_;
    const char *src = (const char *) src_;
    ASSERT_IF(dst >= src, (size_t) (dst - src) >= len);
    ASSERT_IF(src >= dst, (size_t) (src - dst) >= len);
	return ::memcpy(dst, src, len);
}


INLINE unsigned long FASTCALL
IntSqrt(unsigned long x) {

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


INLINE int FASTCALL
IntPow(int base, int exp) {

	int result = 1;
    while (exp) {

        if (exp & 1)
            result *= base;
        exp >>= 1;
        base *= base;
    }
    return result;
}


ALWAYS_INLINE bool
IsPow2(uint32_t v) {

	return (v & (v - 1)) == 0;
}


ALWAYS_INLINE uint32_t
NextPow2(uint32_t v) {

	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	return v+1;
}


ALWAYS_INLINE int
CountSetBits(int32_t v) {

	v = v - ((v >> 1) & 0x55555555);
	v = (v & 0x33333333) + ((v >> 2) & 0x33333333);
	return (((v + (v >> 4)) & 0xF0F0F0F) * 0x1010101) >> 24;
}


ALWAYS_INLINE int
Parity(uint32_t v) {

    v ^= v >> 1;
    v ^= v >> 2;
    v = (v & 0x11111111U) * 0x11111111U;
    return (v >> 28) & 1;
}


ALWAYS_INLINE uint8_t
ReverseBits(uint8_t b) {

	return (((b * 0x0802LU & 0x22110LU) | (b * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16) & 0xFF;
}


ALWAYS_INLINE uint32_t
MostSignificantBit(register uint32_t x) {

	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);
	return (x & ~(x >> 1));
}


template <class T>
ALWAYS_INLINE T
LeastSignificantBit(register T x) {

	#ifdef XP_WIN
	#pragma warning(push)
	#pragma warning(disable : 4146) // warning C4146: unary minus operator applied to unsigned type, result still unsigned
	#endif // XP_WIN
	return x & -x;
	#ifdef XP_WIN
	#pragma warning(pop)
	#endif // XP_WIN
}


ALWAYS_INLINE NOALIAS uint32_t
SvnRevToInt(const char *r) { // supports 9 digits revision number, NULL and empty and "$Revision$" strings.

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



/*
int posix_memalign(void **memptr, size_t alignment, size_t size) {

	if (alignment % sizeof(void *) != 0)
		return EINVAL;
	*memptr = memalign(alignment, size);
	return (*memptr != NULL ? 0 : ENOMEM);
}
*/


/* unused
INLINE bool FASTCALL
fpipe( FILE **read, FILE **write ) {

	int readfd, writefd;
#if defined(XP_WIN)
	HANDLE readPipe, writePipe;
	if ( CreatePipe(&readPipe, &writePipe, NULL, 65536) == 0 )
		return false;
	// doc: The underlying handle is also closed by a call to _close,
	//      so it is not necessary to call the Win32 function CloseHandle on the original handle.
	readfd = _open_osfhandle((intptr_t)readPipe, _O_RDONLY);
	if ( readfd == -1 )
		return false;
	writefd = _open_osfhandle((intptr_t)writePipe, _O_WRONLY);
	if ( writefd == -1 )
		return false;
#elif defined(XP_UNIX)
	int fd[2];
	if ( pipe(fd) == -1 )
		return false;
	readfd = fd[0];
	writefd = fd[1];
#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif
	*read = fdopen(readfd, "r");
	if ( *read == NULL )
		return false;
	*write = fdopen(writefd, "w");
	if ( *write == NULL )
		return false;
	return true;
}
*/


#ifdef XP_UNIX
INLINE void GetAbsoluteModulePath( char* moduleFileName, size_t size, char *modulePath ) {

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
//#define BytesSwap(ptr,a,b) { register char tmp = ((int8_t*)ptr)[a]; ((int8_t*)ptr)[a] = ((int8_t*)ptr)[b]; ((int8_t*)ptr)[b] = tmp; }

ALWAYS_INLINE NOALIAS void
BytesSwap(void *ptr, size_t a, size_t b) {

	register char tmp = ((int8_t*)ptr)[a];
	((int8_t*)ptr)[a] = ((int8_t*)ptr)[b];
	((int8_t*)ptr)[b] = tmp;
}


ALWAYS_INLINE NOALIAS void
Host16ToNetwork16( void *pval ) {

	if ( JLHostEndian == JLLittleEndian )
		BytesSwap( pval, 0, 1 );
}


ALWAYS_INLINE NOALIAS void
Host24ToNetwork24( void *pval ) {

	if ( JLHostEndian == JLLittleEndian )
		BytesSwap( pval, 0, 2 );
}


ALWAYS_INLINE NOALIAS void
Host32ToNetwork32( void *pval ) {

	if ( JLHostEndian == JLLittleEndian ) {

		BytesSwap( pval, 0, 3 );
		BytesSwap( pval, 1, 2 );
	}
}


ALWAYS_INLINE NOALIAS void
Host64ToNetwork64( void *pval ) {

	if ( JLHostEndian == JLLittleEndian ) {

		BytesSwap( pval, 0, 7 );
		BytesSwap( pval, 1, 6 );
		BytesSwap( pval, 2, 5 );
		BytesSwap( pval, 3, 4 );
	}
}


ALWAYS_INLINE NOALIAS void
Network16ToHost16( void *pval ) {

	if ( JLHostEndian == JLLittleEndian )
		BytesSwap( pval, 0, 1 );
}


ALWAYS_INLINE NOALIAS void
Network24ToHost24( void *pval ) {

	if ( JLHostEndian == JLLittleEndian )
		BytesSwap( pval, 0, 2 );
}


ALWAYS_INLINE NOALIAS void
Network32ToHost32( void *pval ) {

	if ( JLHostEndian == JLLittleEndian ) {

		BytesSwap( pval, 0, 3 );
		BytesSwap( pval, 1, 2 );
	}
}


ALWAYS_INLINE NOALIAS void
Network64ToHost64( void *pval ) {

	if ( JLHostEndian == JLLittleEndian ) {

		BytesSwap( pval, 0, 7 );
		BytesSwap( pval, 1, 6 );
		BytesSwap( pval, 2, 5 );
		BytesSwap( pval, 3, 4 );
	}
}


INLINE NEVER_INLINE long FASTCALL
atoi(const char *buf, int base) {

	return strtol(buf, NULL, base);
}


INLINE NEVER_INLINE char* FASTCALL
itoa(long val, char *buf, int base) {

	char *p = buf;
	long prev;
	do {

		prev = val;
		val /= base;
		*p++ = ("zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"+35)[prev - val * base];
	} while ( val );
	if ( prev < 0 )
		*p++ = '-';
	*p-- = '\0';
	char *p1 = buf;
	char chr;
	while ( p1 < p ) {

		chr = *p;
		*p-- = *p1;
		*p1++ = chr;
	}
	return buf;
}


#define JL_ITOA10_MAX_DIGITS (12) // 12 = sign + base-10 max int32_t + '\0'

INLINE NEVER_INLINE char* FASTCALL
itoa10(uint32_t val, char *buf) {

	char *tmp = buf + JL_ITOA10_MAX_DIGITS;
	*--tmp = '\0';
	do {
		*--tmp = '0' + val % 10;
		val /= 10;
	} while ( val );
	return tmp;
}


INLINE NEVER_INLINE char* FASTCALL
itoa10(int32_t val, char *buf) {

	char *tmp = buf + JL_ITOA10_MAX_DIGITS;
	*--tmp = '\0';
	if ( val >= 0 ) {
		
		do {
			*--tmp = '0' + val % 10;
			val /= 10;
		} while ( val );
	} else {

		do {
			*--tmp = '0' - val % 10;
			val /= 10;
		} while ( val );
		*--tmp = '-';
	}
	return tmp;
}


ALWAYS_INLINE void
SleepMilliseconds(uint32_t ms) {

#if defined(XP_WIN)

	Sleep(ms); // winbase.h
#elif defined(XP_UNIX)
	usleep(ms * 1000); // unistd.h // (TBD) obsolete, use nanosleep() instead.
#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif
}


// ReaD Time Stamp Counter (rdtsc)

#if defined(XP_WIN)
INLINE uint64_t
rdtsc(void) {

  return __rdtsc();
}
#elif defined(XP_UNIX)
#ifdef __i386
INLINE uint64_t
rdtsc() {
  uint64_t x;
  __asm__ volatile ("rdtsc" : "=A" (x));
  return x;
}
#elif defined __amd64
INLINE uint64_t
rdtsc() {
  uint64_t a, d;
  __asm__ volatile ("rdtsc" : "=a" (a), "=d" (d));
  return (d<<32) | a;
}
#endif
#else
	#error NOT IMPLEMENTED YET
#endif

/*
#if defined(XP_WIN)
INLINE DWORD
GetCurrentProcessor(void) {
#if defined(__i386__) || defined(__x86_64__)
	_asm { mov eax, 1 }
	_asm { cpuid }
	_asm { shr ebx, 24 }
	_asm { mov eax, ebx }
#else
	#error not implemented
#endif
}
#else
#endif
*/


// Accurate FPS Limiting / High-precision 'Sleeps': see. http://www.geisswerks.com/ryan/FAQS/timing.html

INLINE double FASTCALL
AccurateTimeCounter() {

#if defined(XP_WIN)
	// beware: rdtsc is a per-cpu operation. On multiprocessor systems, be careful that calls to rdtsc are actually executing on the same cpu.

	BOOL result;
	static volatile LONGLONG initTime = 0; // initTime helps in avoiding precision waste by having a relative time.
	static volatile DWORD_PTR cpuMask = 0;
	
	if ( cpuMask == 0 ) {

		DWORD_PTR processAffinityMask = 0;
		DWORD_PTR systemAffinityMask = 0;
		result = ::GetProcessAffinityMask(::GetCurrentProcess(), &processAffinityMask, &systemAffinityMask);
		ASSERT( result );
		ASSERT( processAffinityMask );
		cpuMask = LeastSignificantBit(processAffinityMask);
	}
	LARGE_INTEGER frequency, performanceCount;
	HANDLE thread = ::GetCurrentThread();
	DWORD_PTR oldmask = ::SetThreadAffinityMask(thread, cpuMask);
	result = ::QueryPerformanceFrequency(&frequency);
	ASSERT( result );
	result = ::QueryPerformanceCounter(&performanceCount);
	ASSERT( result );
	if ( initTime == 0 )
		initTime = performanceCount.QuadPart;
	if ( oldmask )
		::SetThreadAffinityMask(thread, oldmask);
	JL_IGNORE( result );
	return (double)1000 * (performanceCount.QuadPart-initTime) / (double)frequency.QuadPart;
#elif defined(XP_UNIX)
	static volatile long initTime = 0; // initTime helps in avoiding precision waste.
	struct timeval tv;
	gettimeofday(&tv, NULL);
	if ( initTime == 0 )
		initTime = tv.tv_sec;
	return (double)(tv.tv_sec-initTime) * (double)1000 + tv.tv_usec / (double)1000;
#else
	#error NOT IMPLEMENTED YET
#endif
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


#define JL_PAGESIZE 4096

ALWAYS_INLINE size_t
JLPageSize() {

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


#if defined(XP_WIN)
INLINE NEVER_INLINE __declspec(naked) size_t 
GetEIP() {

	__asm pop eax;
	__asm jmp eax;
}
#endif


ALWAYS_INLINE void
cpuid( int info[4], int type ) {

#if defined(XP_WIN)
	__cpuid(info, type);
#elif defined(XP_UNIX)
	asm("cpuid":"=a" (info[0]), "=b" (info[1]), "=c" (info[2]), "=d" (info[3]) : "a" (type));
#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif
}


typedef char CpuInfo_t[128];

ALWAYS_INLINE void
CPUInfo( CpuInfo_t info ) {

	// see. http://msdn.microsoft.com/en-us/library/hskdteyh(v=vs.80).aspx
	// see. http://faydoc.tripod.com/cpu/cpuid.htm

	IFDEBUG( char *tmp = (char*)info );

	cpuid((int*)info, 0);
	info += 16;
	cpuid((int*)info, 1);
	info[7] = 0; // CPUInfo[1] &= 0x00ffffff; // remove "Initial APIC ID"
	info += 16;
	cpuid((int*)info, 2);
	info += 16;
	cpuid((int*)info, 0x80000001);
	info += 16;
	cpuid((int*)info, 0x80000002);
	info += 16;
	cpuid((int*)info, 0x80000003);
	info += 16;
	cpuid((int*)info, 0x80000004);
	info += 16;
	cpuid((int*)info, 0x80000006);

	IFDEBUG( ASSERT( (info+16) - (char*)tmp == sizeof(CpuInfo_t) ) );
}


#if defined(XP_WIN)
ALWAYS_INLINE HMODULE
GetCurrentModule() {

	// see also:
	//   http://blogs.msdn.com/b/oldnewthing/archive/2004/10/25/247180.aspx
	//   http://www.codeguru.com/Cpp/W-P/dll/tips/article.php/c3635/
	HMODULE handle;
	GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, "", &handle); // requires XP or 2003
	return handle;
	// or:
	// static int dummy;
	//	MEMORY_BASIC_INFORMATION mbi;
	//	VirtualQuery( &dummy, &mbi, sizeof(mbi) );
	//	return reinterpret_cast<HMODULE>(mbi.AllocationBase);
}
#endif


ALWAYS_INLINE int
ProcessId() {

#if defined(XP_WIN)
	return getpid();
#elif defined(XP_UNIX)
	return getpid();
#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif
}


ALWAYS_INLINE uint32_t
SessionId() {

	uint32_t r = 0x12345678;
	r ^= (uint32_t)AccurateTimeCounter();
	r ^= (uint32_t)ProcessId();
#if defined(XP_WIN)
//	r ^= (u_int32_t)GetModuleHandle(NULL);
	MEMORYSTATUS status;
	GlobalMemoryStatus( &status );
	r ^= (uint32_t)status.dwAvailPhys;
#endif // XP_WIN
	return r ? r : 1; // avoid returning 0
}


ALWAYS_INLINE size_t
RemainingStackSize() {
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

#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif
}


// see http://unicode.org/faq/utf_bom.html#BOM
enum EncodingType {
	ENC_UNKNOWN,
	ENC_UTF32be,
	ENC_UTF32le,
	ENC_UTF16be,
	ENC_UTF16le,
	ENC_UTF8,
	ENC_ASCII
};


INLINE bool
IsASCII(const uint8_t *buf, size_t len) {

	for ( const uint8_t *end = buf + len; buf != end; ++buf )
		if ( (*buf & 0x80) || !(*buf & 0xF8) )
			return false;
	return true;
}


INLINE bool
IsUTF8(const uint8_t *str, size_t len) {

	int n;
	for ( size_t i = 0; i < len; ++i ) {

		if ( str[i] < 0x80 ) continue;
		else if ( (str[i] & 0xE0) == 0xC0 ) n = 1;
		else if ( (str[i] & 0xF0) == 0xE0 ) n = 2;
		else if ( (str[i] & 0xF8) == 0xF0 ) n = 3;
		else if ( (str[i] & 0xFC) == 0xF8 ) n = 4;
		else if ( (str[i] & 0xFE) == 0xFC ) n = 5;
		else return false;
		for ( int j = 0; j < n; ++j ) {
		
			if ( (++i == len) || ((str[i] & 0xC0) != 0x80) )
				return false;
		}
	}
	return true;
}


INLINE EncodingType NOALIAS FASTCALL
DetectEncoding(uint8_t **buf, size_t *size) {

	if ( *size >= 3 && (*buf)[0] == 0xEF && (*buf)[1] == 0xBB && (*buf)[2] == 0xBF ) {

		*buf += 3;
		*size -= 3;
		return ENC_UTF8;
	}

	if ( *size > 2 && (*buf)[0] == 0xFF && (*buf)[1] == 0xFE ) {

		*buf += 2;
		*size -= 2;
		return ENC_UTF16le;
	}

	if ( *size > 2 && (*buf)[0] == 0xFE && (*buf)[1] == 0xFF ) {

		*buf += 2;
		*size -= 2;
		return ENC_UTF16be;
	}

	size_t scanLen = JL_MIN(*size, 1024);
	
	if ( IsASCII(*buf, scanLen) )
		return ENC_ASCII;

	if ( IsUTF8(*buf, scanLen) )
		return ENC_UTF8;

/* no BOM, then guess // (TBD) find a better heuristic
	if ( (*buf)[0] > 0 && (*buf)[1] > 0 )
		return ASCII;

	if ( (*buf)[0] != 0 && (*buf)[1] == 0 )
		return UTF16le;

	if ( (*buf)[0] == 0 && (*buf)[1] != 0 )
		return UTF16be;
*/

	return ENC_UNKNOWN;
}


#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4244 ) // warning C4244: '=' : conversion from 'unsigned int' to 'unsigned short', possible loss of data
#endif // _MSC_VER

#define xmlLittleEndian (JLHostEndian == JLLittleEndian)

// source: libxml2 - encoding.c - MIT License
// changes: outlen and inlen from integer to size_t, and the last redurned value
INLINE int NOALIAS FASTCALL
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


template <typename T>
static inline bool
MemCompare(T *a, T *b, size_t c) {
	size_t n = (c + size_t(7)) / size_t(8);
	switch (c % 8) {
		case 0: do { if (*a++ != *b++) return false;
		case 7:      if (*a++ != *b++) return false;
		case 6:      if (*a++ != *b++) return false;
		case 5:      if (*a++ != *b++) return false;
		case 4:      if (*a++ != *b++) return false;
		case 3:      if (*a++ != *b++) return false;
		case 2:      if (*a++ != *b++) return false;
		case 1:      if (*a++ != *b++) return false;
		        } while (--n > 0);
	}
	return true;
}


template <class T>
struct MemCmp { // from jsstr.cpp
    typedef size_t Extent;
    static ALWAYS_INLINE Extent computeExtent(const T *, size_t patlen) {
        return (patlen - 1) * sizeof(T);
    }
    static ALWAYS_INLINE bool match(const T *p, const T *t, Extent extent) {
        return memcmp(p, t, extent) == 0;
    }
};


template <class T>
struct ManualCmp { // from jsstr.cpp
    typedef const T *Extent;
	 static ALWAYS_INLINE Extent computeExtent(const T *pat, size_t patlen) {
        return pat + patlen;
    }
    static ALWAYS_INLINE bool match(const T *p, const T *t, Extent extent) {
        for (; p != extent; ++p, ++t) {
            if (*p != *t)
                return false;
        }
        return true;
    }
};


template <class InnerMatch, class T> // from jsstr.cpp
ALWAYS_INLINE int32_t
UnrolledMatch(const T *text, size_t textlen, const T *pat, size_t patlen)
{
    ASSERT(patlen > 0 && textlen > 0);
    const T *textend = text + textlen - (patlen - 1);
    const T p0 = *pat;
    const T *const patNext = pat + 1;
    const typename InnerMatch::Extent extent = InnerMatch::computeExtent(pat, patlen);
    T fixup;

    const T *t = text;
    switch ((textend - t) & 7) {
      case 0: if (*t++ == p0) { fixup = 8; goto match; }
      case 7: if (*t++ == p0) { fixup = 7; goto match; }
      case 6: if (*t++ == p0) { fixup = 6; goto match; }
      case 5: if (*t++ == p0) { fixup = 5; goto match; }
      case 4: if (*t++ == p0) { fixup = 4; goto match; }
      case 3: if (*t++ == p0) { fixup = 3; goto match; }
      case 2: if (*t++ == p0) { fixup = 2; goto match; }
      case 1: if (*t++ == p0) { fixup = 1; goto match; }
    }
    while (t != textend) {
      if (t[0] == p0) { t += 1; fixup = 8; goto match; }
      if (t[1] == p0) { t += 2; fixup = 7; goto match; }
      if (t[2] == p0) { t += 3; fixup = 6; goto match; }
      if (t[3] == p0) { t += 4; fixup = 5; goto match; }
      if (t[4] == p0) { t += 5; fixup = 4; goto match; }
      if (t[5] == p0) { t += 6; fixup = 3; goto match; }
      if (t[6] == p0) { t += 7; fixup = 2; goto match; }
      if (t[7] == p0) { t += 8; fixup = 1; goto match; }
        t += 8;
        continue;
        do {
            if (*t++ == p0) {
              match:
                if (!InnerMatch::match(patNext, t, extent))
                    goto failed_match;
                return t - text - 1;
            }
          failed_match:;
        } while (--fixup > 0);
    }
    return -1;
}


template <class T>
ALWAYS_INLINE int32_t
Match(const T *text, size_t textlen, const T *pat, size_t patlen) {

    if (patlen == 0)
        return 0;
    if (textlen < patlen)
        return -1;

	return
	#if !defined(__linux__)
		patlen > 128 ? UnrolledMatch< MemCmp<T> >(text, textlen, pat, patlen) :
	#endif
		UnrolledMatch< ManualCmp<T> >(text, textlen, pat, patlen);
}


JL_END_NAMESPACE


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if defined(XP_WIN)
#elif defined(XP_UNIX)
	#include <pthread.h>
	#include <sched.h>
	#include <semaphore.h>
	#include <error.h>
#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif


///////////////////////////////////////////////////////////////////////////////
// system errors
//

INLINE NEVER_INLINE void NOALIAS FASTCALL
JLLastSysetmErrorMessage( char *message, size_t maxLength ) {

 #if defined(XP_WIN)
	DWORD errorCode = ::GetLastError();
	LPVOID lpMsgBuf;
	DWORD result = ::FormatMessage( // FormatMessageW
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
	const char *msgBuf = strerror(errno); // wcserror
	if ( msgBuf != NULL ) {

		strncpy(message, msgBuf, maxLength-1);
		message[maxLength-1] = '\0';
	} else {

		*message = '\0';
	}
 #else
	#error NOT IMPLEMENTED YET	// (TBD)
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

ALWAYS_INLINE int JLAtomicExchange(volatile long *ptr, long val) {
#if defined(XP_WIN)
	return InterlockedExchange(ptr, val);
#elif defined(XP_UNIX) // #elif ((__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1)) && ... //  #if defined(HAVE_GCC_ATOMIC32)
	return __sync_lock_test_and_set(ptr, val);
#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif
}

ALWAYS_INLINE long JLAtomicIncrement(volatile int32_t *ptr) {
#if defined(XP_WIN)
	return _InterlockedIncrement((volatile LONG*)ptr); // Increments the value of the specified 32-bit variable as an atomic operation.
#elif defined(XP_UNIX)
	return __sync_add_and_fetch(ptr, 1);
#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif
}

ALWAYS_INLINE long JLAtomicDecrement(volatile int32_t *ptr) {
#if defined(XP_WIN)
	return _InterlockedDecrement((volatile LONG*)ptr);
#elif defined(XP_UNIX)
	return __sync_sub_and_fetch(ptr, 1);
#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif
}

ALWAYS_INLINE int JLAtomicAdd(volatile int32_t *ptr, int32_t val) {
#if defined(XP_WIN)
	return _InterlockedExchangeAdd((volatile LONG*)ptr, val) + val; // Performs an atomic addition of two 32-bit values and returns the initial value of the Addend parameter.
#elif defined(XP_UNIX)
	return __sync_add_and_fetch(ptr, val);
#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif
}



///////////////////////////////////////////////////////////////////////////////
// semaphores
//

#if defined(XP_WIN)
	typedef HANDLE JLSemaphoreHandler;
#elif defined(XP_UNIX)
	typedef sem_t* JLSemaphoreHandler;
#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif


	ALWAYS_INLINE JLSemaphoreHandler JLSemaphoreCreate( int initCount ) {

	#if defined(XP_WIN)
		return CreateSemaphore(NULL, initCount, LONG_MAX, NULL);
	#elif defined(XP_UNIX)
		sem_t *sem = (sem_t*)malloc(sizeof(sem_t));
		if ( sem == NULL )
			return NULL;
		sem_init(sem, 0, initCount);
		return sem;
	#else
		#error NOT IMPLEMENTED YET	// (TBD)
	#endif
	}

	ALWAYS_INLINE bool JLSemaphoreOk( JLSemaphoreHandler semaphore ) {

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
	#else
		#error NOT IMPLEMENTED YET	// (TBD)
	#endif
	}
*/

	// msTimeout = JLINFINITE : no timeout
	// returns true on a sucessfuly semaphore locked.
	INLINE int JLSemaphoreAcquire( JLSemaphoreHandler semaphore, int msTimeout ) {

		ASSERT( JLSemaphoreOk(semaphore) );
	#if defined(XP_WIN)
		DWORD status = WaitForSingleObject(semaphore, msTimeout == JLINFINITE ? INFINITE : msTimeout);
		ASSERT( status != WAIT_FAILED );
		if ( status == WAIT_TIMEOUT )
			return JLTIMEOUT;
		return JLOK;
	#elif defined(XP_UNIX)
		int st;
		if ( msTimeout == JLINFINITE ) {

			// st = sem_wait(semaphore);
			while ((st = sem_wait(semaphore)) == -1 && errno == EINTR)
				continue; // Restart if interrupted by handler
			ASSERT( st == 0 );
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
			ASSERT( st == 0 );
			JL_IGNORE( st );
			return JLOK;
		}
		return JLERROR;
	#else
		#error NOT IMPLEMENTED YET	// (TBD)
	#endif
	}

	ALWAYS_INLINE void JLSemaphoreRelease( JLSemaphoreHandler semaphore ) {

		ASSERT( JLSemaphoreOk(semaphore) );
	#if defined(XP_WIN)
		BOOL st = ReleaseSemaphore(semaphore, 1, NULL);
		ASSERT( st != FALSE );
	#elif defined(XP_UNIX)
		int st = sem_post(semaphore);
		ASSERT( st == 0 );
		JL_IGNORE( st );
	#else
		#error NOT IMPLEMENTED YET	// (TBD)
	#endif
	}

	ALWAYS_INLINE void JLSemaphoreFree( JLSemaphoreHandler *pSemaphore ) {

		ASSERT( pSemaphore != NULL && JLSemaphoreOk(*pSemaphore) );
	#if defined(XP_WIN)
		BOOL st = CloseHandle(*pSemaphore);
		ASSERT( st != FALSE );
	#elif defined(XP_UNIX)
		int st = sem_destroy(*pSemaphore);
		ASSERT( st == 0 );
		free(*pSemaphore);
	#else
		#error NOT IMPLEMENTED YET	// (TBD)
	#endif
		*pSemaphore = (JLSemaphoreHandler)0;
		JL_IGNORE( st );
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

#if defined(XP_WIN)
	typedef CRITICAL_SECTION* JLMutexHandler; // doc. critical section objects provide a slightly faster, more efficient mechanism for mutual-exclusion synchronization.
#elif defined(XP_UNIX)
	typedef pthread_mutex_t* JLMutexHandler;
#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif


	#define JLMutexInvalidHandler ((JLMutexHandler)0)

	ALWAYS_INLINE bool JLMutexOk( JLMutexHandler mutex ) {

		return mutex != JLMutexInvalidHandler;
	}

	ALWAYS_INLINE JLMutexHandler JLMutexCreate() {

		JLMutexHandler mutex = (JLMutexHandler)malloc(sizeof(*mutex));
		if ( mutex == NULL )
			return NULL;
	#if defined(XP_WIN)
		InitializeCriticalSection(mutex);
	#elif defined(XP_UNIX)
		int st = pthread_mutex_init(mutex, NULL);
		ASSERT( st == 0 );
		JL_IGNORE( st );
	#else
		#error NOT IMPLEMENTED YET	// (TBD)
	#endif
		return mutex;
	}

	ALWAYS_INLINE void JLMutexFree( JLMutexHandler *pMutex ) {

		ASSERT( *pMutex != NULL && JLMutexOk(*pMutex) );
	#if defined(XP_WIN)
		DeleteCriticalSection(*pMutex);
	#elif defined(XP_UNIX)
		int st = pthread_mutex_destroy(*pMutex);
		ASSERT( st == 0 );
		JL_IGNORE( st );
	#else
		#error NOT IMPLEMENTED YET	// (TBD)
	#endif
		free(*pMutex);
		*pMutex = JLMutexInvalidHandler;
	}

	ALWAYS_INLINE void JLMutexAcquire( JLMutexHandler mutex ) {

		ASSERT( JLMutexOk(mutex) );
	#if defined(XP_WIN)
		EnterCriticalSection(mutex);
	#elif defined(XP_UNIX)
		int st = pthread_mutex_lock(mutex); // doc. shall not return an error code of [EINTR].
		ASSERT( st == 0 );
		JL_IGNORE( st );
	#else
		#error NOT IMPLEMENTED YET	// (TBD)
	#endif
	}

	ALWAYS_INLINE void JLMutexRelease( JLMutexHandler mutex ) {

		ASSERT( JLMutexOk(mutex) );
	#if defined(XP_WIN)
		LeaveCriticalSection(mutex);
	#elif defined(XP_UNIX)
		int st = pthread_mutex_unlock(mutex);
		ASSERT( st == 0 );
		JL_IGNORE( st );
	#else
		#error NOT IMPLEMENTED YET	// (TBD)
	#endif
	}



///////////////////////////////////////////////////////////////////////////////
// Condition Variables
//
// see http://www.cs.wustl.edu/~schmidt/win32-cv-1.html (Strategies for Implementing POSIX Condition Variables on Win32)

#define JLCondInvalideHandler ((JLCondHandler)0)

#if defined(XP_WIN)

typedef struct __JLCondHandler {
	unsigned int waiters_count;
	// doc. waiters_count_lock_ protects the waiters_count_ from being corrupted by race conditions.
	//      This is not necessary as long as pthread_cond_signal and pthread_cond_broadcast are always called by a thread that has locked the same external_mutex used by pthread_cond_wait.
	// CRITICAL_SECTION waiters_count_lock;
	HANDLE events[2]; // [signal, broadcast]
} *JLCondHandler;


ALWAYS_INLINE bool JLCondOk( JLCondHandler cv ) {

	return cv != JLCondInvalideHandler;
}

ALWAYS_INLINE JLCondHandler JLCondCreate() {

	JLCondHandler cv = (JLCondHandler)malloc(sizeof(*cv));
	if ( cv == NULL )
		return NULL;
	cv->waiters_count = 0;
//	InitializeCriticalSection(&cv->waiters_count_lock);
	cv->events[0] = CreateEvent(NULL, FALSE, FALSE, NULL);
	ASSERT( cv->events[0] != NULL );
	cv->events[1] = CreateEvent(NULL, TRUE, FALSE, NULL);
	ASSERT( cv->events[1] != NULL );
	return cv;
}

ALWAYS_INLINE void JLCondFree( JLCondHandler *cv ) {

	ASSERT( cv != NULL && JLCondOk(*cv) );
	BOOL st = CloseHandle((*cv)->events[1]);
	ASSERT( st != FALSE );
	st = CloseHandle((*cv)->events[0]);
	ASSERT( st != FALSE );
	JL_IGNORE( st );
//	DeleteCriticalSection(&(*cv)->waiters_count_lock);
	free(*cv);
	*cv = NULL;
}

INLINE int JLCondWait( JLCondHandler cv, JLMutexHandler external_mutex ) {

	ASSERT( JLCondOk(cv) );
//	EnterCriticalSection(&cv->waiters_count_lock);
	cv->waiters_count++;
//	LeaveCriticalSection(&cv->waiters_count_lock);
	JLMutexRelease(external_mutex);
	int result = WaitForMultipleObjects(2, cv->events, FALSE, INFINITE);
	ASSERT( result != WAIT_FAILED );
//	EnterCriticalSection(&cv->waiters_count_lock);
	cv->waiters_count--;
	int last_waiter = result == WAIT_OBJECT_0 + 1 && cv->waiters_count == 0;
//	LeaveCriticalSection(&cv->waiters_count_lock);
	if ( last_waiter ) {

		BOOL st = ResetEvent(cv->events[1]);
		ASSERT( st != FALSE );
		JL_IGNORE(st);
	}
	JLMutexAcquire(external_mutex);
	return JLOK;
}

ALWAYS_INLINE void JLCondBroadcast( JLCondHandler cv ) {

	ASSERT( JLCondOk(cv) );
//	EnterCriticalSection(&cv->waiters_count_lock);
	int have_waiters = cv->waiters_count > 0;
//	LeaveCriticalSection(&cv->waiters_count_lock);
	if ( have_waiters ) {

		BOOL st = SetEvent(cv->events[1]);
		ASSERT( st != FALSE );
		JL_IGNORE(st);
	}
}

ALWAYS_INLINE void JLCondSignal( JLCondHandler cv ) {

	ASSERT( JLCondOk(cv) );
//	EnterCriticalSection(&cv->waiters_count_lock);
	int have_waiters = cv->waiters_count > 0;
//	LeaveCriticalSection(&cv->waiters_count_lock);
	if ( have_waiters ) {

		BOOL st = SetEvent(cv->events[0]);
		ASSERT( st != FALSE );
		JL_IGNORE(st);
	}
}

#elif defined(XP_UNIX)

typedef struct __JLCondHandler {
	pthread_cond_t cond;
} *JLCondHandler;

ALWAYS_INLINE bool JLCondOk( JLCondHandler cv ) {

	return cv != JLCondInvalideHandler;
}

ALWAYS_INLINE JLCondHandler JLCondCreate() {

	JLCondHandler cv = (JLCondHandler)malloc(sizeof(*cv));
//	cv->cond = PTHREAD_COND_INITIALIZER;
	pthread_cond_init(&cv->cond, NULL);
	return cv;
}

ALWAYS_INLINE void JLCondFree( JLCondHandler *cv ) {

	ASSERT( cv != NULL && JLCondOk(*cv) );
	pthread_cond_destroy(&(*cv)->cond);
	free(*cv);
	*cv = NULL;
}

ALWAYS_INLINE int JLCondWait( JLCondHandler cv, JLMutexHandler external_mutex ) {

	ASSERT( JLCondOk(cv) );
	pthread_cond_wait(&cv->cond, external_mutex); // doc. shall not return an error code of [EINTR].
	return JLOK;
}

ALWAYS_INLINE void JLCondBroadcast( JLCondHandler cv ) {

	ASSERT( JLCondOk(cv) );
	pthread_cond_broadcast(&cv->cond);
}

ALWAYS_INLINE void JLCondSignal( JLCondHandler cv ) {

	ASSERT( JLCondOk(cv) );
	pthread_cond_signal(&cv->cond);
}

#else
	#error NOT IMPLEMENTED YET	// (TBD)
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
	#else
		#error NOT IMPLEMENTED YET	// (TBD)
	#endif
		bool autoReset;
	} *JLEventHandler;

	ALWAYS_INLINE bool JLEventOk( JLEventHandler ev ) {

		return ev != (JLEventHandler)0;
	}

	ALWAYS_INLINE JLEventHandler JLEventCreate( bool autoReset ) {

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
	#else
		#error NOT IMPLEMENTED YET	// (TBD)
	#endif
		ev->autoReset = autoReset;
		return ev;
	}

	ALWAYS_INLINE void JLEventFree( JLEventHandler *ev ) {

		ASSERT( ev != NULL && JLEventOk(*ev) );
	#if defined(XP_WIN)
		DeleteCriticalSection(&(*ev)->cs);
		BOOL st = CloseHandle((*ev)->hEvent);
		ASSERT( st != FALSE );
	#elif defined(XP_UNIX)
		int st = pthread_cond_destroy(&(*ev)->cond);
		ASSERT( st == 0 );
		st = pthread_mutex_destroy(&(*ev)->mutex);
		ASSERT( st == 0 );
	#else
		#error NOT IMPLEMENTED YET	// (TBD)
	#endif
		free(*ev);
		*ev = NULL;
	}

	INLINE void JLEventTrigger( JLEventHandler ev ) {

		ASSERT( JLEventOk(ev) );
	#if defined(XP_WIN)
		EnterCriticalSection(&ev->cs);
		if ( ev->waitingThreadCount != 0 || !ev->autoReset ) {

			BOOL st = SetEvent(ev->hEvent);
			ASSERT( st != FALSE );
			JL_IGNORE(st);
		}
		LeaveCriticalSection(&ev->cs);
	#elif defined(XP_UNIX)
		pthread_mutex_lock(&ev->mutex);
		ev->triggered = true;
		pthread_cond_broadcast(&ev->cond); // pthread_cond_signal
		if ( ev->autoReset )
			ev->triggered = false;
		pthread_mutex_unlock(&ev->mutex);
	#else
		#error NOT IMPLEMENTED YET	// (TBD)
	#endif
	}

	ALWAYS_INLINE void JLEventReset( JLEventHandler ev ) {

		ASSERT( JLEventOk(ev) );
	#if defined(XP_WIN)
		EnterCriticalSection(&ev->cs);
		BOOL st = ResetEvent(ev->hEvent);
		ASSERT( st != FALSE );
		JL_IGNORE(st);
		LeaveCriticalSection(&ev->cs);
	#elif defined(XP_UNIX)
		pthread_mutex_lock(&ev->mutex);
		ev->triggered = false;
		pthread_mutex_unlock(&ev->mutex);
	#else
		#error NOT IMPLEMENTED YET	// (TBD)
	#endif
	}

	// msTimeout = JLINFINITE : no timeout
	INLINE int JLEventWait( JLEventHandler ev, int msTimeout ) {

		ASSERT( JLEventOk(ev) );
	#if defined(XP_WIN)

		EnterCriticalSection(&ev->cs);
		ev->waitingThreadCount++;
		LeaveCriticalSection(&ev->cs);

		DWORD status = WaitForSingleObject(ev->hEvent, msTimeout == JLINFINITE ? INFINITE : msTimeout);
		ASSERT( status != WAIT_FAILED );

		EnterCriticalSection(&ev->cs);
		ev->waitingThreadCount--;
		if ( ev->waitingThreadCount == 0 && ev->autoReset ) {

			BOOL st = ResetEvent(ev->hEvent);
			ASSERT( st == TRUE );
			JL_IGNORE(st);
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
			ASSERT( st == 0 );
			JL_IGNORE(st);
			return JLOK;
		}
		return JLERROR;
	#else
		#error NOT IMPLEMENTED YET	// (TBD)
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
	#else
		#error NOT IMPLEMENTED YET	// (TBD)
	#endif

	#define JLThreadInvalidHandler ((JLThreadHandler)0)


	ALWAYS_INLINE bool JLThreadOk( JLThreadHandler thread ) {

		return thread != JLThreadInvalidHandler;
	}


	ALWAYS_INLINE JLThreadHandler JLThreadStart( JLThreadRoutine threadRoutine, void *pv ) {

	#if defined(XP_WIN)
		// The new thread handle is created with the THREAD_ALL_ACCESS access right.
		// If a security descriptor is not provided when the thread is created, a default security descriptor is constructed for the new thread using the primary token of the process that is creating the thread.
		// When a caller attempts to access the thread with the OpenThread function, the effective token of the caller is evaluated against this security descriptor to grant or deny access.
		return CreateThread(NULL, 0, threadRoutine, pv, 0, NULL);
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
	#else
		#error NOT IMPLEMENTED YET	// (TBD)
	#endif
	}

	ALWAYS_INLINE bool JLThreadIsActive( JLThreadHandler thread ) { // (TBD) how to manage errors ?

		ASSERT( JLThreadOk(thread) );
	#if defined(XP_WIN)
		DWORD status = WaitForSingleObject(thread, 0);
		ASSERT( status != WAIT_FAILED );
		return status == WAIT_TIMEOUT; // else != WAIT_OBJECT_0 ?
	#elif defined(XP_UNIX)
		int policy;
		struct sched_param param;
		return pthread_getschedparam(*thread, &policy, &param) != ESRCH; // errno.h
	#else
		#error NOT IMPLEMENTED YET	// (TBD)
	#endif
	}


	ALWAYS_INLINE void JLThreadFree( JLThreadHandler *pThread ) {

		ASSERT( pThread != NULL && JLThreadOk(*pThread) );
	#if defined(XP_WIN)
		BOOL st = CloseHandle(*pThread);
		ASSERT( st != FALSE );
		JL_IGNORE(st);
	#elif defined(XP_UNIX)
		if ( JLThreadIsActive( *pThread ) ) {

			int st = pthread_detach(**pThread);
			ASSERT( st == 0 );
			JL_IGNORE(st);
		}
		free(*pThread);
	#else
		#error NOT IMPLEMENTED YET	// (TBD)
	#endif
		*pThread = JLThreadInvalidHandler;
	}


	ALWAYS_INLINE void JLThreadExit( unsigned int exitValue ) {

	#if defined(XP_WIN)
		ExitThread(exitValue);
	#elif defined(XP_UNIX)
		pthread_exit((void*)exitValue);
	#else
		#error NOT IMPLEMENTED YET	// (TBD)
	#endif
	}


	ALWAYS_INLINE void JLThreadCancel( JLThreadHandler thread ) {

		ASSERT( JLThreadOk(thread) );
	#if defined(XP_WIN)
		BOOL st = TerminateThread(thread, 0); // doc. The handle must have the THREAD_TERMINATE access right. ... Use the GetExitCodeThread function to retrieve a thread's exit value.
		ASSERT( st != 0 );
	#elif defined(XP_UNIX)
		int st = pthread_cancel(*thread);
		ASSERT( st == 0 );
		JL_IGNORE( st );
	#else
		#error NOT IMPLEMENTED YET	// (TBD)
	#endif
	}


	ALWAYS_INLINE void JLThreadPriority( JLThreadHandler thread, JLThreadPriorityType priority ) {

		ASSERT( JLThreadOk(thread) );
	#if defined(XP_WIN)
		BOOL st = SetThreadPriority(thread, priority);
		ASSERT( st != FALSE );
	#elif defined(XP_UNIX)
		int st;
		int policy;
		struct sched_param param;
		st = pthread_getschedparam(*thread, &policy, &param);
		ASSERT( st == 0 );
		int max = sched_get_priority_max(policy);
		int min = sched_get_priority_min(policy);
		param.sched_priority = min + (max - min) * priority / 128;
		st = pthread_setschedparam(*thread, policy, &param);
		ASSERT( st == 0 );
		JL_IGNORE( st );
	#else
		#error NOT IMPLEMENTED YET	// (TBD)
	#endif
	}


	ALWAYS_INLINE void JLThreadWait( JLThreadHandler thread, unsigned int *exitValue ) {

		ASSERT( JLThreadOk(thread) );
	#if defined(XP_WIN)
		BOOL st = WaitForSingleObject(thread, INFINITE);
		ASSERT( st == WAIT_OBJECT_0 );
		if ( exitValue )
			GetExitCodeThread(thread, (DWORD*)exitValue);
	#elif defined(XP_UNIX)
		int st = pthread_join(*thread, (void**)exitValue); // doc. The thread exit status returned by pthread_join() on a canceled thread is PTHREAD_CANCELED. pthread_join shall not return an error code of [EINTR].
		ASSERT( st == 0 );
		JL_IGNORE( st );
	#else
		#error NOT IMPLEMENTED YET	// (TBD)
	#endif
	}



///////////////////////////////////////////////////////////////////////////////
// thread-local storage
//

#if defined(XP_WIN)
	typedef DWORD JLTLSKey;
#elif defined(XP_UNIX)
	typedef pthread_key_t JLTLSKey;
#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif

#define JLTLSInvalidKey 0

ALWAYS_INLINE JLTLSKey JLTLSAllocKey() {
	JLTLSKey key;
#if defined(XP_WIN)
	key = TlsAlloc();
	ASSERT( key != TLS_OUT_OF_INDEXES );
	key++;
#elif defined(XP_UNIX)
	int st = pthread_key_create(&key, NULL);
	ASSERT( st == 0 );
	JL_IGNORE( st );
#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif
	return key;
}


ALWAYS_INLINE void JLTLSFreeKey( JLTLSKey key ) {
#if defined(XP_WIN)
	ASSERT( key != 0 );
	key--;
	ASSERT( key >= 0 && key < TLS_MINIMUM_AVAILABLE );
	BOOL st = TlsFree(key);
	ASSERT( st != FALSE );
#elif defined(XP_UNIX)
	int st = pthread_key_delete(key);
	ASSERT( st == 0 );
#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif
	JL_IGNORE( st );
}


ALWAYS_INLINE void JLTLSSet( JLTLSKey key, void *value ) {
#if defined(XP_WIN)
	ASSERT( key != 0 );
	key--;
	ASSERT( key >= 0 && key < TLS_MINIMUM_AVAILABLE );
	BOOL st = TlsSetValue(key, value);
	ASSERT( st != FALSE );
#elif defined(XP_UNIX)
	int st = pthread_setspecific(key, value);
	ASSERT( st == 0 );
#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif
	JL_IGNORE( st );
}


ALWAYS_INLINE void* JLTLSGet( JLTLSKey key ) {
#if defined(XP_WIN)
	ASSERT( key != 0 );
	key--;
	void *value;
	ASSERT( key >= 0 && key < TLS_MINIMUM_AVAILABLE );
	value = TlsGetValue(key);
	ASSERT( value != 0 || GetLastError() == ERROR_SUCCESS  );
	return value;
#elif defined(XP_UNIX)
	return pthread_getspecific(key);
#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif

}


///////////////////////////////////////////////////////////////////////////////
// dynamic libraries
//

#if defined(XP_WIN)
	typedef HMODULE JLLibraryHandler;
#elif defined(XP_UNIX)
	typedef void* JLLibraryHandler;
#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif

	#define JLDynamicLibraryNullHandler ((JLLibraryHandler)0)

	ALWAYS_INLINE bool JLDynamicLibraryOk( JLLibraryHandler libraryHandler ) {

		return libraryHandler != JLDynamicLibraryNullHandler;
	}

	ALWAYS_INLINE uintptr_t JLDynamicLibraryId( JLLibraryHandler libraryHandler ) {

		return (uint32_t)( ((uintptr_t)libraryHandler >> ALIGNOF(void*)) & 0xffffffff ); // shift useless and keep 32bits.
	}

	ALWAYS_INLINE JLLibraryHandler JLDynamicLibraryOpen( const char *filename ) {

	#if defined(XP_WIN)
		// GetErrorMode() only exists on Vista and higher,
		// call SetErrorMode() twice to achieve the same effect.
		// see also SetThreadErrorMode()
//		UINT oldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
//		SetErrorMode( oldErrorMode | SEM_FAILCRITICALERRORS ); // avoid error popups
		HMODULE hModule = LoadLibrary(filename); // If the function fails, the return value is NULL. 
		// Restore previous error mode.
//		SetErrorMode(oldErrorMode);
		return hModule;
	#elif defined(XP_UNIX)
		dlerror(); // Resets the error indicator.
		return dlopen(filename, RTLD_LAZY | RTLD_LOCAL); // shall return NULL on error. // see. RTLD_NOW / RTLD_GLOBAL
	#else
		#error NOT IMPLEMENTED YET	// (TBD)
	#endif
	}

	ALWAYS_INLINE void JLDynamicLibraryClose( JLLibraryHandler *libraryHandler ) {

	#if defined(XP_WIN)
		BOOL st = FreeLibrary(*libraryHandler);
		ASSERT( st != FALSE );
	#elif defined(XP_UNIX)
		dlerror(); // Resets the error indicator.
		int st = dlclose(*libraryHandler);
		ASSERT( st == 0 );
	#else
		#error NOT IMPLEMENTED YET	// (TBD)
	#endif
		*libraryHandler = (JLLibraryHandler)0;
		JL_IGNORE( st );
	}

	INLINE NEVER_INLINE void FASTCALL
	JLDynamicLibraryLastErrorMessage( char *message, size_t maxLength ) {

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
	#else
		#error NOT IMPLEMENTED YET	// (TBD)
	#endif
	}

	ALWAYS_INLINE void *JLDynamicLibrarySymbol( JLLibraryHandler libraryHandler, const char *symbolName ) {

	#if defined(XP_WIN)
		return (void*)GetProcAddress(libraryHandler, symbolName);
	#elif defined(XP_UNIX)
		dlerror(); // Resets the error indicator.
		return dlsym(libraryHandler, symbolName);
	#else
		#error NOT IMPLEMENTED YET	// (TBD)
	#endif
	}

	ALWAYS_INLINE void JLDynamicLibraryName( void *addr, char *fileName, size_t maxFileNameLength ) {

		// DWORD st = GetModuleFileName(libraryHandler, (LPCH)fileName, (DWORD)fileNameSize); ASSERT( st != ERROR_INSUFFICIENT_BUFFER );
		ASSERT( maxFileNameLength > 0 );
	#if defined(XP_WIN)
		HMODULE libraryHandler;
		if ( !GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCSTR)addr, &libraryHandler) ) { // requires XP or 2003
			
			fileName[0] = '\0';
			return;
		}
		//alternative:
		//	MEMORY_BASIC_INFORMATION mem;
		//	if ( !VirtualQuery(addr, &mem, sizeof(mem)) || mem.AllocationBase == NULL ) {
		//
		//		fileName[0] = '\0';
		//		return;
		//	}
		//	HMODULE libraryHandler = (HMODULE)mem.AllocationBase;
		GetModuleFileName(libraryHandler, (LPCH)fileName, maxFileNameLength);
	#elif defined(XP_UNIX)
		Dl_info info;
		if ( !dladdr(addr, &info) || !info.dli_fbase || !info.dli_fname ) {

			fileName[0] = '\0';
			return;
		}
		int len = JL_MIN(strlen(info.dli_fname), maxFileNameLength-1);
		jl::memcpy(fileName, info.dli_fname, len);
		fileName[len] = '\0';
	#else
		#error NOT IMPLEMENTED YET	// (TBD)
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
#else
	#error NOT IMPLEMENTED YET	// (TBD)
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
	#else
		#error NOT IMPLEMENTED YET	// (TBD)
	#endif

	ALWAYS_INLINE JLCondWait( JLCondHandler cond, int timeout ) {
	}
*/

// Pthreads-w32: http://sourceware.org/pthreads-win32/





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

	return cv != JLCondHandler;
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

	ASSERT( JLCondOk(*cv) );
	BOOL st = CloseHandle((*cv)->sema_);
	ASSERT( st );
	JL_IGNORE( st );
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

