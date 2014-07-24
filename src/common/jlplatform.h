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


// platform: http://nadeausoftware.com/articles/2012/01/c_c_tip_how_use_compiler_predefined_macros_detect_operating_system
#ifdef _WIN64
	#define WIN
#elif _WIN32
	#define WIN
#elif defined(__APPLE__) && defined(__MACH__)
    #include "TargetConditionals.h"
    #if TARGET_OS_MAC == 1
		#define MAC
    #else
        #error Unsupported OS
    #endif
#elif __linux
	#define UNIX
#elif __unix
	#define UNIX
#else
	#error Unsupported OS
#endif

// platform bits: http://nadeausoftware.com/articles/2012/02/c_c_tip_how_detect_processor_type_using_compiler_predefined_macros
#if defined(_WIN64) || defined(_M_X64) || defined(__x86_64__) || defined(__ppc64__)
	#define BITS64
#elif defined(_WIN32) || defined(_M_IX86) || defined(__i386) || defined(__ppc__)
	#define BITS32
#else
	#error Unsupported architecture
#endif

#define PLATFORM_BITS \
	(sizeof(ptrdiff_t) * 8)


// compiler: http://nadeausoftware.com/articles/2012/10/c_c_tip_how_detect_compiler_name_and_version_using_compiler_predefined_macros
#ifdef _MSC_VER
	#define MSC
#elif  defined(__GNUC__) || defined(__GNUG__) || defined(__clang__)
	#define GCC
#elif defined(__MINGW32__)
	#define GCC
#else
	#error Unsupported compiler
#endif

// debug
#if defined(_DEBUG) && !defined(DEBUG)
	#define DEBUG
#endif



///////////////////////////////////////////////////////////////////////////////
// Miscellaneous

#define JL_BEGIN_NAMESPACE namespace jl {
#define JL_END_NAMESPACE }


#define JL_STRINGIFY(x) #x
#define JL_TOSTRING(x) JL_STRINGIFY(x)

#define JL__CONCAT(X,Y) X##Y
#define JL_CONCAT(X,Y) JL__CONCAT(X,Y)


// from jstypes.h
#define JL_MACRO_BEGIN do {

#if defined(_MSC_VER) && _MSC_VER >= 1400
	#define JL_MACRO_END } __pragma(warning(push)) __pragma(warning(disable:4127)) while (0) __pragma(warning(pop))
#else
	#define JL_MACRO_END   } while (0)
#endif

#define JL_CODE_LOCATION __FILE__ ":" JL_TOSTRING(__LINE__)

#define __DATE__YEAR ((((__DATE__ [7] - '0') * 10 + (__DATE__ [8] - '0')) * 10 + (__DATE__ [9] - '0')) * 10 + (__DATE__ [10] - '0'))
#define __DATE__MONTH ( (__DATE__[2] == 'n') ? (__DATE__[1] == 'a' ? 1 : 6) : (__DATE__[2] == 'b') ? 2 : (__DATE__[2] == 'r') ? (__DATE__[1] == 'a' ? 3 : 4) : (__DATE__[2] == 'y') ? 5 : (__DATE__[2] == 'l') ? 7 : (__DATE__[2] == 'g') ? 8 : (__DATE__[2] == 'p') ? 9 : (__DATE__[2] == 't') ? 10 : (__DATE__[2] == 'v') ? 11 : 12)
#define __DATE__DAY ((__DATE__ [4] == ' ' ? 0 : __DATE__ [4] - '0') * 10 + (__DATE__ [5] - '0'))
#define __DATE__HOUR (((__TIME__[0]-'0')*10) + (__TIME__[1]-'0'))
#define __DATE__MINUTE (((__TIME__[3]-'0')*10) + (__TIME__[4]-'0'))
#define __DATE__SECOND (((__TIME__[6]-'0')*10) + (__TIME__[7]-'0'))
#define __DATE__EPOCH_DAYS (( __DATE__MONTH > 2 ? (__DATE__YEAR*365 + __DATE__YEAR/4 - __DATE__YEAR/100 + __DATE__YEAR/400 + (__DATE__MONTH+1) * 306001 / 10000 + __DATE__DAY) : ((__DATE__YEAR-1)*365 + (__DATE__YEAR-1)/4 - (__DATE__YEAR-1)/100 + (__DATE__YEAR-1)/400 + (__DATE__MONTH+13) * 306001 / 10000 + __DATE__DAY) ) - 719591)
#define __DATE__EPOCH (__DATE__EPOCH_DAYS * 86400 + __DATE__HOUR * 3600 + __DATE__MINUTE * 60 + __DATE__SECOND)



///////////////////////////////////////////////////////////////////////////////
// Compiler specific configuration

#if defined(MSC)
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
	#pragma warning(error : 4172) // returning address of local variable or temporary
	#ifdef DEBUG
		#pragma warning(error : 4701) // potentially uninitialized local variable 'XXX' used
	#endif
#elif defined(GCC)
	// # pragma GCC diagnostic ignored "-Wformat"  /* Ignore Warning about printf format /
	// # pragma GCC diagnostic ignored "-Wunused-parameter"  / Ignore Warning about unused function parameter */
	#pragma GCC diagnostic error "-Wdiv-by-zero"
#endif


/*
#if defined(MSC)

#ifndef IN
	#define IN _In_
#endif

#ifndef OUT
	#define OUT _Out_
#endif

#ifndef INOUT
	#define INOUT _Inout_
#endif

#ifndef OPTIONAL
	#define OPTIONAL
#endif

else

#endif

*/

#ifdef DEBUG
	#define IFDEBUG(expr) expr
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
//template <class F> NEVER_INLINE F NOIL( F f ) { return f; }
#define NOIL(f) f
#endif

/*
#ifndef ASSUME
# if defined _MSC_VER
#  define ASSUME(expr) (__assume(expr))
# else
#  define ASSUME(expr) ((void)0)
# endif
#endif
*/
#define ASSUME(expr) ((void)0)


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
	#define DLLIMPORT __declspec(dllimport)
	#define DLLEXPORT __declspec(dllexport)
	#define DLLLOCAL
#elif defined( __GNUC__ )
	// info. http://gcc.gnu.org/wiki/Visibility
	// Support of visibility attribute is mandatory to manage _moduleId scope (must be private but global for the .so).
	// If visibility hidden is not set for _moduleId,
	// DLLLOCAL uint32_t _moduleId = 0;
	// nm build/default/src/jsstd/jsstd | grep moduleId
	//   000168a8 B _moduleId   => uppercase B = Global "bss" (that is, uninitialized data space) symbol.
	// and should be:
	//   000168a8 b _moduleId   => lowercase b = Local bss symbol.
	#define DLLIMPORT
	#define DLLEXPORT __attribute__ ((visibility("default")))
	#define DLLLOCAL __attribute__ ((visibility("hidden")))
#else
	#error NOT IMPLEMENTED YET	// (TBD)
	#define DLLIMPORT
	#define DLLEXPORT
	#define DLLLOCAL
#endif

#if defined( BUILD_DLL )
	#define DLLAPI DLLEXPORT
#else
	#define DLLAPI DLLIMPORT
#endif




#if defined(_MSC_VER) && _MSC_VER >= 1100
#define NOVTABLE __declspec(novtable)
#else
#define NOVTABLE
#endif


#define NOTHROW throw()

#if defined(MSC)

#if defined(DEBUG)

		#include <rtcapi.h>

		#define DISABLE_SMALLER_TYPE_CHECK \
			int _prev_rtc_cvrt_loss_info = _RTC_SetErrorType(_RTC_CVRT_LOSS_INFO, _RTC_ERRTYPE_IGNORE); __pragma(warning(push)) __pragma(warning(disable:4244))

		#define RESTORE_SMALLER_TYPE_CHECK \
			_RTC_SetErrorType(_RTC_CVRT_LOSS_INFO, _prev_rtc_cvrt_loss_info); __pragma(warning(pop))

	#else

		#define DISABLE_SMALLER_TYPE_CHECK ((void)0); __pragma(warning(push)) __pragma(warning(disable:4244))
		#define RESTORE_SMALLER_TYPE_CHECK ((void)0);  __pragma(warning(pop))

	#endif

#endif

// convert a number of TCHARs into a size in bytes
#define TSIZE(length) ((length)*sizeof(TCHAR))

/*
template <size_t MAXLEN>
ALWAYS_INLINE const char *
tstrToStr( const wchar_t *src, char *dst ) {

	const char *max = dst + MAXLEN;
	char *it = dst;
	while (it != max) {

		if ( *src & 0xff00 )
			*it = '~';
		else
			*it = *src & 0xff;
		if (*src == 0)
			break;
		++src;
		++it;
	}
	return dst;
}


template <size_t MAXLEN>
ALWAYS_INLINE const char *
tstrToStr( const char *src, char * ) {

	return src;
}
*/


///////////////////////////////////////////////////////////////////////////////

#include <sys/types.h>
#include <float.h>
#include <wchar.h>
//#include <limits.h>
#include <limits>
#include <cstddef>
#include <stdarg.h>
#include <errno.h>
#include <malloc.h>
#include <fcntl.h> // _O_RDONLY, _O_WRONLY
#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <cctype>
#include <type_traits>
#include <tchar.h>

#define _USE_MATH_DEFINES
#include <math.h>

#if defined(_MSC_VER)
#include <intrin.h> // __cpuid()
#include <io.h> // _open_osfhandle()
#endif

#include <sys/stat.h>



///////////////////////////////////////////////////////////////////////////////
// Platform specific configuration

#if defined(WIN)

#if defined(REPORT_MEMORY_LEAKS)
	// the following code make issue with jstl.h (js\src\jstl.h(244) : error C2039: '_malloc_dbg' : is not a member of 'JSContext')
	#ifdef _DEBUG
	# define _CRTDBG_MAP_ALLOC
	# include <crtdbg.h>
	#endif // _DEBUG
#endif // REPORT_MEMORY_LEAKS


	// http://msdn.microsoft.com/en-us/library/windows/desktop/aa383745%28v=vs.85%29.aspx#setting_winver_or__win32_winnt

	// doc: _WIN32_WINNT_WS03 = 0x0502 = Windows Server 2003 with SP1, Windows XP with SP2.
	// note: SpiderMionkey compilation option: --with-windows-version=502

	#ifndef WINVER         // Allow use of features specific to Windows 95 and Windows NT 4 or later.
	#define WINVER _WIN32_WINNT_WIN7  // Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
	#endif

	#ifndef _WIN32_WINNT         // Allow use of features specific to Windows NT 4 or later.
	#define _WIN32_WINNT _WIN32_WINNT_WIN7  // Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
	#endif

	#ifndef _WIN32_WINDOWS        // Allow use of features specific to Windows 98 or later.
	#define _WIN32_WINDOWS _WIN32_WINNT_WIN7 // Change this to the appropriate value to target Windows Me or later. 0x501 = XP SP1.
	#endif

	#undef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN   // Exclude rarely-used stuff from Windows headers

	// Using STRICT to Improve Type Checking (http://msdn.microsoft.com/en-us/library/aa280394%28v=VS.60%29.aspx)
	#define STRICT 1

	// see WinDef.h
	#define NOMINMAX

	#include <windows.h>
//	#include <Mmsystem.h> // timeGetTime()

//	#include <direct.h> // function declarations for directory handling/creation
	#include <process.h> // threads, ...
	#define __STDC_LIMIT_MACROS
	#include <stdint.h>
	typedef float float32_t;
	typedef double float64_t;


	#define PATH_MAX (MAX_PATH > _MAX_DIR ? MAX_PATH : _MAX_DIR)
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


#elif defined(MAC) // MacosX platform

	#ifndef UNIX
	#define UNIX // used by SpiderMonkey and jslibs
	#endif

	#include <unistd.h>

	typedef float float32_t;
	typedef double float64_t;

	#define DLL_EXT ".dylib"
	#define PATH_SEPARATOR_STRING "/"
	#define PATH_SEPARATOR '/'
	#define LIST_SEPARATOR_STRING ":"
	#define LIST_SEPARATOR ':'

	#error TBD

#else // Linux platform

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

#define S_ASSERT(cond) \
	extern void STATIC_ASSERT_DUMMY(int arg[(cond) ? 1 : -1])

#ifndef PTRDIFF_MAX
#define PTRDIFF_MIN ((ptrdiff_t)(-PTRDIFF_MAX - (ptrdiff_t)1))
#define PTRDIFF_MAX ((ptrdiff_t)(SIZE_MAX / 2))
#endif

#ifdef MSC

#define UNLIKELY_SPLIT_BEGIN(...) { struct { INLINE NEVER_INLINE bool FASTCALL operator()( ##__VA_ARGS__ ) {
#define UNLIKELY_SPLIT_END(...) } } inner; if ( inner( ##__VA_ARGS__ ) ) return true; else goto bad; JL_BAD; }

#else

#define UNLIKELY_SPLIT_BEGIN(...)
#define UNLIKELY_SPLIT_END(...)

#endif



#ifdef WIN

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
// see UNREFERENCED_PARAMETER in winNT.h
#define JL_IGNORE(...) \
	((void)(__VA_ARGS__))



#if defined(WIN32)

#define JL_ASSERT_FAILURE( message, location ) \
	( \
	_ftprintf(stderr, TEXT("JL Assertion failure:  %s  @%s\n"), message, location), \
	fflush(stderr), \
	__debugbreak(), \
	(*((volatile int*) NULL) = 123), \
	::TerminateProcess(::GetCurrentProcess(), 3) \
	)

#elif defined(__APPLE__)

#define JL_ASSERT_FAILURE( message, location ) \
	( \
	fprintf(stderr, "JL Assertion failure: %s @%s\n", message, location), \
	fflush(stderr), \
	__debugbreak(), \
	(*((volatile int*) NULL) = 123), \
	raise(SIGABRT) \
	)

#else

#define JL_ASSERT_FAILURE( message, location ) \
	( \
	fprintf(stderr, "JL Assertion failure: %s @%s\n", message, location), \
	fflush(stderr), \
	raise(SIGABRT) \
	)

#endif



#ifdef DEBUG

#define ASSERT(expr) \
    ( (expr) ? (void)0 : JL_ASSERT_FAILURE(#expr, JL_CODE_LOCATION) )

#define ASSERT_IF(cond, expr) \
    ( (!(cond) || (expr)) ? (void)0 : JL_ASSERT_FAILURE(#expr, JL_CODE_LOCATION) )

#else // DEBUG

// beware. Use __assume in an ASSERT only when the assert is not recoverable.
#define ASSERT(expr) \
	((void)0) // (ASSUME(expr))

#define ASSERT_IF(cond, expr) \
	((void)0)

#endif // DEBUG


/*
#include "mozilla/Assertions.h"
#define ASSERT(expr)           MOZ_ASSERT(expr)
#define ASSERT_IF(cond, expr)  MOZ_ASSERT_IF(cond, expr)
*/


///////////////////////////////////////////////////////////////////////////////
// Platform tools

#define CREATE_MEMBER_DETECTOR(X)                                                   \
template<typename T> class Detect_##X {                                             \
    struct Fallback { int X; };                                                     \
    struct Derived : T, Fallback { };                                               \
                                                                                    \
    template<typename U, U> struct Check;                                           \
                                                                                    \
    typedef char ArrayOfOne[1];                                                     \
    typedef char ArrayOfTwo[2];                                                     \
                                                                                    \
    template<typename U> static ArrayOfOne & func(Check<int Fallback::*, &U::X> *); \
    template<typename U> static ArrayOfTwo & func(...);                             \
  public:                                                                           \
    typedef Detect_##X type;                                                        \
    enum { value = sizeof(func<Derived>(0)) == 2 };                                 \
};


/* SFINAE

1/ has member function

template <typename Type>
class HasGetter {
   class yes { char m; };
   class no { yes m[2]; };
   struct BaseMixin { void Getter(){} };
   struct Base : public Type, public BaseMixin {};
   template <typename T, T t>  class Helper{};
   template <typename U>
   static no deduce(U*, Helper<void (BaseMixin::*)(), &U::Getter>* = 0);
   static yes deduce(...);
public:
   static const bool result = sizeof(yes) == sizeof(deduce((Base*)(0)));
};

---

2/ has member variable

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
	typedef T Type;
	T _item;
	ALWAYS_INLINE explicit Wrap( T& item )
	: _item(item) {
	}
	ALWAYS_INLINE operator T() {

		return _item;
	}
};


template<class T>
struct DummyAlignStruct {
  uint8_t first;
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
	uint8_t bytes[4];
	uint32_t value;
} JLHostEndianType = { { 0, 1, 2, 3 } };


#define JLHostEndian \
	(JLHostEndianType.value)



JL_BEGIN_NAMESPACE
	

template<typename A, typename B>
ALWAYS_INLINE A
min(const A a, const B b) {
	
	return a < static_cast<A>(b) ? a : static_cast<A>(b);
}

template<typename A, typename B>
ALWAYS_INLINE A
max(const A a, const B b) {
	
	return a > static_cast<A>(b) ? a : static_cast<A>(b);
}

template<typename V, typename A, typename B>
ALWAYS_INLINE V
minmax(const V v, const A a, const B b) {
	
	return v > static_cast<V>(b) ? static_cast<V>(b) : v < static_cast<V>(a) ? static_cast<V>(a) : v;
}



template<class T, class U>
ALWAYS_INLINE bool
isInRange(T val, U vmin, U vmax) {

	return val >= vmin && val <= vmax; // note: unsigned(val - vmin) <= unsigned(vmax - vmin) is 10 cycles faster.
}


template <typename T, typename T1>
ALWAYS_INLINE bool
isIn(T val, T1 v1) {
	
	return val == v1;
}

template <typename T, typename T1, typename T2>
ALWAYS_INLINE bool
isIn(T val, T1 v1, T2 v2) {
	
	return val == v1 || val == v2;
}

template <typename T, typename T1, typename T2, typename T3>
ALWAYS_INLINE bool
isIn(T val, T1 v1, T2 v2, T2 v3) {
	
	return val == v1 || val == v2 || val == v3;
}




ALWAYS_INLINE NOALIAS int
DoubleIsNegZero(const double &d) {
#ifdef WIN
	return (d == 0 && (_fpclass(d) & _FPCLASS_NZ));
#elif defined(SOLARIS)
	return (d == 0 && copysign(1, d) < 0);
#else
	return (d == 0 && signbit(d));
#endif
}


ALWAYS_INLINE NOALIAS bool
DoubleIsNeg(const double &d) {
#ifdef WIN
	return d < 0 || DoubleIsNegZero(d);
#elif defined(SOLARIS)
	return copysign(1, d) < 0;
#else
	return signbit(d);
#endif
}


template <typename T>
ALWAYS_INLINE bool
IsIntegerValue(T num) {

	//return ::std::modf(num, 0) == 0.0;
	return num == floor(num);
}



template <class T> ALWAYS_INLINE bool isTypeFloat64(T) { return false; }
ALWAYS_INLINE bool isTypeFloat64(double) { return true; }

template <class T> ALWAYS_INLINE bool isTypeFloat32(T) { return false; }
ALWAYS_INLINE bool isTypeFloat32(float) { return true; }


// 2^53 = 9007199254740992. since double(9007199254740992) == double(9007199254740993), and double(-9007199254740992) == double(-9007199254740993)  we must subtract 1.
// see also std::numeric_limits<double>::digits
//
// failed on linux because + Standard 2003, 5.19 "Constant expressions", paragraph 1.
// ... Floating literals (2.13.3) can appear only if they are cast to integral or enumeration types.
//S_ASSERT( MAX_INT_TO_DOUBLE != MAX_INT_TO_DOUBLE+(double)1 );
//S_ASSERT( MAX_INT_TO_DOUBLE+(double)1 == MAX_INT_TO_DOUBLE+(double)2 );


template<typename Source>
struct SignificandStringValue {
	
	static const char * min() {
	
		static char buffer[(::std::numeric_limits<Source>::is_signed ? ::std::numeric_limits<Source>::digits10 + 1 : 0) + 2];
		static char* str = jl::itoa10( ::std::numeric_limits<Source>::min(), buffer );
		return str;
	}

	static const char * max() {
	
		static char buffer[::std::numeric_limits<Source>::digits10 + 2];
		static char* str = jl::itoa10( ::std::numeric_limits<Source>::max(), buffer );
		return str;
	}
};

template<>
struct SignificandStringValue<int32_t> {
	static const char * min() {
		return ( "-2^31" );
	}
	static const char * max() {
		return ( "2^31-1" );
	}
};

template<>
struct SignificandStringValue<uint32_t> {
	static const char * min() {
		return ( "0" );
	}
	static const char * max() {
		return ( "2^32" );
	}
};

template<>
struct SignificandStringValue<int64_t> {
	static const char * min() {
		return ( "2^63" );
	}
	static const char * max() {
		return ( "2^63-1" );
	}
};

template<>
struct SignificandStringValue<uint64_t> {
	static const char * min() {
		return ( "0" );
	}
	static const char * max() {
		return ( "2^64" );
	}
};

template<>
struct SignificandStringValue<float> {
	static const char * min() {
		return ( "-2^24" );
	}
	static const char * max() {
		return ( "2^24" );
	}
};

template<>
struct SignificandStringValue<double> {
	static const char * min() {
		return ( "-2^53" );
	}
	static const char * max() {
		return ( "2^53" );
	}
};

////

template<typename T>
int significandSizeOf() {

	return ::std::numeric_limits<T>::digits;
}

template<typename Source> struct SignificandValue {
	static Source max() { return ::std::numeric_limits<Source>::max(); }
	static Source min() { return ::std::numeric_limits<Source>::min(); }
};

S_ASSERT( ::std::numeric_limits<float32_t>::digits < 32 );
template<> struct SignificandValue<float32_t> {
	static float32_t max() { return (uint32_t(1)<<::std::numeric_limits<float32_t>::digits)-1; }
	static float32_t min() { return -max(); }
};

S_ASSERT( ::std::numeric_limits<float64_t>::digits < 64 );
template<> struct SignificandValue<float64_t> {
	static float64_t max() { return (uint64_t(1)<<::std::numeric_limits<float64_t>::digits)-1; }
	static float64_t min() { return -max(); }
};

namespace pv {

// Helper class used as a base for various type traits, exposed publicly because <type_traits> exposes it as well.
template<typename T, T Value>
struct IntegralConstant {
    static const T value = Value;
    typedef T ValueType;
    typedef IntegralConstant<T, Value> Type;
};

// Convenient aliases.
typedef IntegralConstant<bool, true> TrueType;
typedef IntegralConstant<bool, false> FalseType;

template<typename T, typename U>
struct IsSame;

template<typename T, typename U>
struct IsSame : FalseType {};

template<typename T>
struct IsSame<T, T> : TrueType {};


template <typename Target, typename Source, bool targetSigned = ::std::numeric_limits<Target>::is_signed, bool sourceSigned = ::std::numeric_limits<Source>::is_signed> struct BoundsChecker;
template <typename Target, typename Source> struct BoundsChecker<Target, Source, false, false> {
    static bool inBounds(Source value) {
        // Same signedness so implicit type conversion will always increase precision to widest type
        return value <= SignificandValue<Target>::max();
    }
};

template <typename Target, typename Source> struct BoundsChecker<Target, Source, true, true> {
    static bool inBounds(Source value) {
        // Same signedness so implicit type conversion will always increase precision to widest type
        return SignificandValue<Target>::min() <= value && value <= SignificandValue<Target>::max();
    }
};

template <typename Target, typename Source> struct BoundsChecker<Target, Source, false, true> {
    static bool inBounds(Source value) {
        // Target is unsigned so any value less than zero is clearly unsafe
        if (value < 0)
            return false;
        // If our (unsigned) Target is the same or greater width we can convert value to type Target without losing precision
        if (significandSizeOf<Target>() >= significandSizeOf<Source>())
            return static_cast<Target>(value) <= SignificandValue<Target>::max();
        // The signed Source type has greater precision than the target so max(Target) -> Source will widen.
        return value <= static_cast<Source>(SignificandValue<Target>::max());
    }
};

template <typename Target, typename Source> struct BoundsChecker<Target, Source, true, false> {
    static bool inBounds(Source value) {
        // Signed target with an unsigned source
        if (significandSizeOf<Target>() <= significandSizeOf<Source>())
            return value <= static_cast<Source>(SignificandValue<Target>::max());
        // Target is Wider than Source so we're guaranteed to fit any value in unsigned Source
        return true;
    }
};

template <typename Target, typename Source, bool SameType = IsSame<Target, Source>::value> struct BoundsCheckElider;
template <typename Target, typename Source> struct BoundsCheckElider<Target, Source, true> {
    static bool inBounds(Source) {
		return true;
	}
};

template <typename Target, typename Source> struct BoundsCheckElider<Target, Source, false> : public BoundsChecker<Target, Source> {
};

}; // pv namespace

template <typename Target, typename Source> static inline bool isInBounds(Source value) {
    return pv::BoundsCheckElider<Target, Source>::inBounds(value);
}

//mozilla::IsUnsigned

/*
Unsigned
*/
//template<class T> struct Unsigned { typedef T Type; static const bool isUnsigned = false; };
//template<class T> struct Unsigned <unsigned T> { typedef T Type; static const bool isUnsigned = true; };

/*
Const
*/
/*
template<class T> struct Const { typedef T Type; static const bool isConst = false; };
template<class T> struct Const <const T> { typedef T Type; static const bool isConst = true; };
template<class T> struct Const <const T[]> { typedef T Type[]; static const bool isConst = true; };
template<class T, unsigned int N> struct Const <const T[N]> { typedef T Type[N]; static const bool isConst = true; };
*/

/*
Pointer
*/
/*
template<class T> struct Pointer { typedef T Type; static const bool isPointer = false; };
template<class T> struct Pointer <T*> { typedef T Type; static const bool isPointer = true; };
*/


//

#define MakeUnsigned(T) \
	std::make_unsigned<T>::type

#define MakeSigned(T) \
	std::make_signed<T>::type

template<class T>
const T *
constPtr( T *v ) {
	
	return v;
}

#define IsConst(T) \
	mozilla::IsConst<T>::value

#define RemoveConst(T) \
	mozilla::RemoveConst<T>::Type

//

template<class T> struct RemovePointer { typedef T Type; };
template<class T> struct RemovePointer <T*> { typedef T Type; };

#define IsPointer(T) \
	mozilla::IsPointer<T>::value

#define RemovePointer(T) \
	jl::RemovePointer<T>::Type



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
		!cstr[2] ? (cstr[1] <<  8) | cstr[0] :
		!cstr[3] ? (cstr[2] << 16) | (cstr[1] <<  8) | cstr[0] :
		           (cstr[3] << 24) | (cstr[2] << 16) | (cstr[1] <<  8) | cstr[0];
}

ALWAYS_INLINE NOALIAS int
CastUint32ToCStr( uint32_t val, char *cstr ) {

	cstr[0] = (val & 0x000000FF) >> 0  & 0xFF;
	cstr[1] = (val & 0x0000FF00) >> 8  & 0xFF;
	cstr[2] = (val & 0x00FF0000) >> 16 & 0xFF;
	cstr[3] = (val & 0xFF000000) >> 24 & 0xFF;
	return cstr[0] ? cstr[1] ? cstr[2] ? cstr[3] ? 4 : 3 : 2 : 1 : 0;
}


ALWAYS_INLINE void *
memcpy(void *dst_, const void *src_, size_t len) {
    
	uint8_t *dst = (uint8_t*)dst_;
	const uint8_t *src = (const uint8_t*)src_;
    ASSERT_IF(dst >= src, (size_t) (dst - src) >= len);
    ASSERT_IF(src >= dst, (size_t) (src - dst) >= len);
	return ::memcpy(dst, src, len);
	//  Use memmove if the memory areas do overlap.
}


ALWAYS_INLINE void*
zeromem(void *dst, size_t length) {

#ifdef WIN
	return ::ZeroMemory(dst, length);
#else
	return ::zeromem(dst, length);
#endif
	
}


/*
ALWAYS_INLINE void *
memset(void *dst, size_t len, uint8_t val) {
    
	return ::memchr(dst, val, len);
}
*/


template<class D, class S>
ALWAYS_INLINE void
reinterpretBuffer(void* d, void* s, size_t length) {

	D *dst = reinterpret_cast<D*>(d);
	S *src = reinterpret_cast<S*>(s);

	DISABLE_SMALLER_TYPE_CHECK;
	if ( sizeof(D) < sizeof(S) ) {

		S* end = src + length;
		while ( src != end )
			*(dst++) = *(src++);
	} else {

		S* end = src;
		dst += length;
		src += length;
		while ( src != end )
			*(--dst) = *(--src);
	}
	RESTORE_SMALLER_TYPE_CHECK;
}

template<class D, class S>
ALWAYS_INLINE void
reinterpretBufferUnsigned( void* d, void* s, size_t length ) {

	reinterpretBuffer<MakeUnsigned( D ), MakeUnsigned( S )>( d, s, length );
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

	#ifdef MSC
		#pragma warning(push)
		#pragma warning(disable : 4146) // warning C4146: unary minus operator applied to unsigned type, result still unsigned
	#endif // WIN

	return x & -x;

	#ifdef MSC
		#pragma warning(pop)
	#endif // WIN
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
#if defined(WIN)
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
#elif defined(UNIX)
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


#ifdef UNIX

INLINE void GetAbsoluteModulePath( TCHAR* moduleFileName, size_t size, TCHAR* modulePath ) {

	if ( modulePath[0] == PATH_SEPARATOR ) { //  /jshost

		jl::strcpy(moduleFileName, modulePath);
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

				jl::strcpy(moduleFileName, envPath);
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

#endif //UNIX


ALWAYS_INLINE bool
GetModuleFileName(TCHAR *hostFullPath) {

#if defined(WIN)

// get hostpath and hostname
	return GetModuleFileName((HINSTANCE)GetModuleHandle(NULL), hostFullPath, PATH_MAX) != 0;

#elif defined(UNIX)

//	jl::GetAbsoluteModulePath(hostFullPath, PATH_MAX, argv0);
	int len = readlink("/proc/self/exe", moduleFileName, sizeof(moduleFileName)); // doc: readlink does not append a NUL character to buf.
	moduleFileName[len] = '\0';
//	strcpy(hostFullPath, argv[0]);
	return true;

#else

	#error NOT IMPLEMENTED YET	// (TBD)

#endif

}


#if defined(WIN)

// from: http://alter.org.ua/en/docs/win/args/

ALWAYS_INLINE
PWCHAR*
CommandLineToArgvW( PWCHAR CmdLine, int* _argc ) {

	PWCHAR* argv;
	PWCHAR  _argv;
	ULONG   len;
	ULONG   argc;
	WCHAR   a;
	ULONG   i, j;

	BOOLEAN  in_QM;
	BOOLEAN  in_TEXT;
	BOOLEAN  in_SPACE;

	len = wcslen( CmdLine );
	i = ((len + 2) / 2)*sizeof( PVOID ) + sizeof( PVOID );

	//argv = (PWCHAR*)GlobalAlloc( GMEM_FIXED, i + (len + 2)*sizeof( WCHAR ) );
	argv = (PWCHAR*)::malloc( i + (len + 2)*sizeof( WCHAR ) );

	_argv = (PWCHAR)(((PUCHAR)argv) + i);

	argc = 0;
	argv[argc] = _argv;
	in_QM = FALSE;
	in_TEXT = FALSE;
	in_SPACE = TRUE;
	i = 0;
	j = 0;

	while ( (a = CmdLine[i]) != 0 ) {
		if ( in_QM ) {
			if ( a == '\"' ) {
				in_QM = FALSE;
			} else {
				_argv[j] = a;
				j++;
			}
		} else {
			switch ( a ) {
			case '\"':
				in_QM = TRUE;
				in_TEXT = TRUE;
				if ( in_SPACE ) {
					argv[argc] = _argv + j;
					argc++;
				}
				in_SPACE = FALSE;
				break;
			case ' ':
			case '\t':
			case '\n':
			case '\r':
				if ( in_TEXT ) {
					_argv[j] = '\0';
					j++;
				}
				in_TEXT = FALSE;
				in_SPACE = TRUE;
				break;
			default:
				in_TEXT = TRUE;
				if ( in_SPACE ) {
					argv[argc] = _argv + j;
					argc++;
				}
				_argv[j] = a;
				j++;
				in_SPACE = FALSE;
				break;
			}
		}
		i++;
	}
	_argv[j] = '\0';
	argv[argc] = NULL;

	(*_argc) = argc;
	return argv;
}

#endif

///////////////////////////////////////////////////////////////////////////////
// Platform tools

// cf. _swab() -or- _rotl();
// 16 bits: #define SWAP_BYTES(X) ((X & 0xff) << 8) | (X >> 8)
// 32 bits swap: #define SWAP_BYTE(x) ((x<<24) | (x>>24) | ((x&0xFF00)<<8) | ((x&0xFF0000)>>8))
//#define BytesSwap(ptr,a,b) { register int8_t tmp = ((int8_t*)ptr)[a]; ((int8_t*)ptr)[a] = ((int8_t*)ptr)[b]; ((int8_t*)ptr)[b] = tmp; }

ALWAYS_INLINE NOALIAS void
BytesSwap(void *ptr, size_t a, size_t b) {

	register int8_t tmp = ((int8_t*)ptr)[a];
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

////

// http://www.i18nguy.com/unicode/c-unicode.html

ALWAYS_INLINE void FASTCALL
strncpy(char *dst, const char *src, size_t nelem) {

    //jl::memcpy(dst, src, nelem * sizeof(char));
	::strncpy( dst, src, nelem );
}

ALWAYS_INLINE void FASTCALL
strncpy(wchar_t *dst, const wchar_t *src, size_t nelem) {

    //jl::memcpy(dst, src, nelem * sizeof(wchar_t)); // wcsncpy ?
	::wcsncpy( dst, src, nelem );
}

////

template <size_t MAX>
ALWAYS_INLINE errno_t FASTCALL
strcpy_s( char *dst, const char *src ) {

	return ::strcpy_s( dst, MAX, src );
}

template <size_t MAX>
ALWAYS_INLINE errno_t FASTCALL
strcpy_s( wchar_t *dst, const wchar_t *src ) {

	return ::wcscpy_s( dst, MAX, src );
}

////

ALWAYS_INLINE const char* FASTCALL
strchr( const char *s, char c ) {

	return ::strchr(s, c);
}

ALWAYS_INLINE char* FASTCALL
strchr( char *s, char c ) {

	return ::strchr(s, c);
}

//

ALWAYS_INLINE const wchar_t* FASTCALL
strchr(const wchar_t *s, wchar_t c) {

	return ::wcschr(s, c);
}

ALWAYS_INLINE wchar_t* FASTCALL
strchr(wchar_t *s, wchar_t c) {

	return ::wcschr(s, c);
}

////



ALWAYS_INLINE const char* FASTCALL
strrchr( const char *s, char c ) {

	return ::strrchr( s, c );
}

ALWAYS_INLINE char* FASTCALL
strrchr( char *s, char c ) {

	return ::strrchr( s, c );
}

//

ALWAYS_INLINE const wchar_t* FASTCALL
strrchr( const wchar_t *s, wchar_t c ) {

	return ::wcsrchr( s, c );
}

ALWAYS_INLINE wchar_t* FASTCALL
strrchr( wchar_t *s, wchar_t c ) {

	return ::wcsrchr( s, c );
}


////


ALWAYS_INLINE char* FASTCALL
strchr_limit(const char *s, char c, const char *limit) {

    while (s < limit) {
        if (*s == c)
            return (char *)s;
        s++;
    }
    return NULL;
}

ALWAYS_INLINE wchar_t* FASTCALL
strchr_limit(const wchar_t *s, wchar_t c, const wchar_t *limit) {

    while (s < limit) {
        if (*s == c)
            return (wchar_t *)s;
        s++;
    }
    return NULL;
}


////

ALWAYS_INLINE size_t FASTCALL
strlen( const char *str ) { return ::strlen(str); }

ALWAYS_INLINE size_t FASTCALL
strlen( const wchar_t *str ) { return ::wcslen(str); }

////

ALWAYS_INLINE char * FASTCALL
strcat( char *dst, const char *str ) { return ::strcat( dst, str ); }

ALWAYS_INLINE wchar_t * FASTCALL
strcat( wchar_t *dst, const wchar_t *str ) { return ::wcscat( dst, str ); }

////

ALWAYS_INLINE char * FASTCALL
strcpy( char *dst, const char *str ) { return ::strcpy( dst, str ); }

ALWAYS_INLINE wchar_t * FASTCALL
strcpy( wchar_t *dst, const wchar_t *str ) { return ::wcscpy( dst, str ); }

////

ALWAYS_INLINE int FASTCALL
strcmp( const char *str1, const char *str2 ) {
	return ::strcmp( str1, str2 );
}

ALWAYS_INLINE int FASTCALL
strcmp( const wchar_t *str1, const wchar_t *str2 ) {
	return ::wcscmp( str1, str2 );
}

////


ALWAYS_INLINE int FASTCALL
stricmp( const char *str1, const char *str2 ) {
	return ::_stricmp( str1, str2 );
}

ALWAYS_INLINE int FASTCALL
stricmp( const wchar_t *str1, const wchar_t *str2 ) {
	return ::_wcsicmp( str1, str2 );
}


////

ALWAYS_INLINE int FASTCALL
strncmp( const char *str1, const char *str2, size_t maxCount ) {
	return ::strncmp( str1, str2, maxCount );
}

ALWAYS_INLINE int FASTCALL
strncmp( const wchar_t *str1, const wchar_t *str2, size_t maxCount ) {
	return ::wcsncmp( str1, str2, maxCount );
}

////



ALWAYS_INLINE int FASTCALL
open( const char * filename, int openFlag, int permissionMode = 0 ) {

	return _open( filename, openFlag, permissionMode );
}

ALWAYS_INLINE int FASTCALL
open( const wchar_t * filename, int openFlag, int permissionMode = 0 ) {

	return _wopen( filename, openFlag, permissionMode );
}

////


ALWAYS_INLINE int FASTCALL
stat( const char * filename, struct _stat * st ) {

	return _stat( filename, st );
}

ALWAYS_INLINE int FASTCALL
stat( const wchar_t * filename, struct _stat * st ) {

	return _wstat( filename, st );
}

////

// on win32, see ::GetTempPath() and ::GetTempFileName();

ALWAYS_INLINE char * FASTCALL
tempFilename( char *buffer ) { // PATH_MAX

	char *tmp = tempnam( NULL, "jltmp" );
	if ( tmp == NULL )
		return NULL;
	jl::strcpy( buffer, tmp );
	return buffer;
	free( tmp );
}

ALWAYS_INLINE wchar_t * FASTCALL
tempFilename( wchar_t *buffer ) { // PATH_MAX

	wchar_t *tmp = _wtempnam( NULL, L( "jltmp" ) );
	if ( tmp == NULL )
		return NULL;
	jl::strcpy( buffer, tmp );
	return buffer;
	free( tmp );
}

////


template <typename T>
ALWAYS_INLINE size_t FASTCALL
tstrlen(const T* s) {

	const T *t;
    for (t = s; *t != 0; t++)
        continue;
    return (size_t)(t - s);
}


template <typename T, typename U>
int32_t
tstrcmp( const T *lhs, const U *rhs ) {

	while ( true ) {

		//if ( *lhs != static_cast<T>(*rhs) )
		if ( *lhs != *rhs )
			return static_cast<int32_t>(*lhs) - static_cast<int32_t>(*rhs);
		if ( *lhs == 0 )
			return 0;
		++lhs, ++rhs;
	}
}

template <typename T, typename U>
int32_t
tstrcmpUnsigned( const T *lhs, const U *rhs ) {

	return tstrcmp( reinterpret_cast<const MakeUnsigned( T )*>(lhs), reinterpret_cast<const MakeUnsigned( U )*>(rhs) );
}


template <typename T, typename U>
int32_t
tstrncmp(const T *lhs, const U *rhs, size_t max ) {

	const T *limit = lhs + max;
	while ( lhs < limit ) {

		if ( *lhs != static_cast<T>(*rhs) )
			return static_cast<int32_t>(*lhs) - static_cast<int32_t>(*rhs);
		if ( *lhs == 0 )
			return 0;
		++lhs, ++rhs;
	}
	return 0;
}

template <typename T, typename U>
int32_t
tstrncmpUnsigned( const T *lhs, const U *rhs, size_t max ) {

	return tstrncmp( reinterpret_cast<const MakeUnsigned( T )*>(lhs), reinterpret_cast<const MakeUnsigned( U )*>(rhs), max );
}


////


INLINE NEVER_INLINE long FASTCALL
atoi(const char *buf, int base) {

	return ::strtol(buf, NULL, base);
}

INLINE NEVER_INLINE long FASTCALL
atoi( const wchar_t *buf, int base ) {

	return ::wcstol( buf, NULL, base );
}

////

INLINE NEVER_INLINE double FASTCALL
atof( const char *buf ) {

	return ::strtod( buf, NULL );
}

INLINE NEVER_INLINE double FASTCALL
atof( const wchar_t *buf ) {

	return ::wcstod( buf, NULL );
}

////


// \0 not included !
#define IToA10MaxDigits(TYPE) \
	( (::std::numeric_limits<TYPE>::is_signed ? 1 : 0) + ::std::numeric_limits<TYPE>::digits10 + 1 )


template <typename T>
INLINE NEVER_INLINE char* FASTCALL
itoa( T val, char *buf, uint8_t base ) {

	char *p = buf;
	T prev;
	do {
		prev = val;
		val /= base;
		*p++ = ("zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"+35)[prev - val * base];
	} while ( val );
	if ( ::std::numeric_limits<T>::is_signed && prev < 0 )
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

template <typename T>
INLINE NEVER_INLINE wchar_t* FASTCALL
itoa( T val, wchar_t *buf, uint8_t base ) {

	wchar_t *p = buf;
	T prev;
	do {
		prev = val;
		val /= base;
		*p++ = (L"zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" + 35)[prev - val * base];
	} while ( val );
	if ( ::std::numeric_limits<T>::is_signed && prev < 0 )
		*p++ = '-';
	*p-- = '\0';
	wchar_t *p1 = buf;
	wchar_t chr;
	while ( p1 < p ) {

		chr = *p;
		*p-- = *p1;
		*p1++ = chr;
	}
	return buf;
}




template <typename T>
INLINE NEVER_INLINE char* FASTCALL
itoa10(T val, char *buf) {

	char *tmp = buf + (::std::numeric_limits<T>::is_signed ? 1 : 0) + 1 + ::std::numeric_limits<T>::digits10 + 1;
	*--tmp = '\0';
	if ( ::std::numeric_limits<T>::is_signed && val < 0 ) {

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

template <typename T>
INLINE NEVER_INLINE wchar_t* FASTCALL
itoa10( T val, wchar_t *buf ) {

	wchar_t *tmp = buf + (::std::numeric_limits<T>::is_signed ? 1 : 0) + 1 + ::std::numeric_limits<T>::digits10 + 1;
	*--tmp = L'\0';
	if ( ::std::numeric_limits<T>::is_signed && val < 0 ) {

		do {
			*--tmp = L'0' + val % 10;
			val /= 10;
		} while ( val );
	} else {

		do {
			*--tmp = L'0' - val % 10;
			val /= 10;
		} while ( val );
		*--tmp = '-';
	}
	return tmp;
}




// ReaD Time Stamp Counter (rdtsc)

#if defined(WIN)

INLINE uint64_t
rdtsc(void) {

  return __rdtsc();
}

#elif defined(UNIX)

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
#if defined(WIN)
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
// Programs that use the QueryPerformanceCounter function may perform poorly in Windows Server 2000, in Windows Server 2003, and in Windows XP: http://support.microsoft.com/kb/895980
// Win32 Performance Measurement Options: QueryPerformanceCounter is fine for individual short-interval timing.
// http://www.drdobbs.com/article/print?articleId=184416651
// (TBD) see GetTickCount() and CurrentClockTickTime()
// -QueryPerformanceCounter is acurate to less than 1ms.
// -GetTickCount() seems to vary. But has a max resolution of 1ms, but is often several ms.
// -timeGetTime() can be forced to have a 1ms resolution. 

INLINE double FASTCALL
AccurateTimeCounter() {

#if defined(WIN)

	// see article http://www.virtualdub.org/blog/pivot/entry.php?id=106
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
	return (double)1000 * (performanceCount.QuadPart - initTime) / (double)frequency.QuadPart;

#elif defined(UNIX)

	static volatile long initTime = 0; // initTime helps in avoiding precision waste.
	struct timeval tv;
	gettimeofday(&tv, NULL);
	if ( initTime == 0 )
		initTime = tv.tv_sec;
	return (double)(tv.tv_sec - initTime) * (double)1000 + tv.tv_usec / (double)1000;

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

ALWAYS_INLINE void
SleepMilliseconds(uint32_t ms) {

#if defined(WIN)

	Sleep(ms); // winbase.h

#elif defined(UNIX)

	usleep(ms * 1000); // unistd.h // (TBD) obsolete, use nanosleep() instead.

#else

	#error NOT IMPLEMENTED YET	// (TBD)

#endif

}


ALWAYS_INLINE void
SleepMillisecondsAccurate(uint32_t ms) {

	if ( ms == 0 )
		return;

#if defined(WIN)

	double aim = AccurateTimeCounter() + (double)ms;
	Sleep(ms-1); // winbase.h
	while ( AccurateTimeCounter() < aim )
		Sleep(0);

#elif defined(UNIX)

	usleep(ms * 1000); // unistd.h // (TBD) obsolete, use nanosleep() instead.

#else

	#error NOT IMPLEMENTED YET	// (TBD)

#endif

}


#define JL_PAGESIZE 4096

ALWAYS_INLINE size_t
JLPageSize() {

	static size_t pageSize = 0;
	if ( pageSize )
		return pageSize;
#if defined(WIN)
	SYSTEM_INFO siSysInfo;
	GetSystemInfo(&siSysInfo); // WinBase.h
	pageSize = siSysInfo.dwPageSize;
#elif defined(UNIX)
	pageSize = sysconf(_SC_PAGESIZE); // unistd.h
#elif defined(__i386__) || defined(__x86_64__)
    pageSize = 4096;
#else
	#error Unable to detect system page size
#endif
	return pageSize;
}


#if defined(MSC)
INLINE NEVER_INLINE __declspec(naked) size_t 
GetEIP() {

	__asm pop eax;
	__asm jmp eax;
}
#endif


ALWAYS_INLINE void
cpuid( int info[4], int type ) {

#if defined(MSC)
	__cpuid(info, type);
#elif defined(GCC)
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


#if defined(WIN)

ALWAYS_INLINE HMODULE
GetCurrentModule() {

	// see also:
	//   http://blogs.msdn.com/b/oldnewthing/archive/2004/10/25/247180.aspx
	//   http://www.codeguru.com/Cpp/W-P/dll/tips/article.php/c3635/
	HMODULE handle;
	GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, TEXT(""), &handle); // requires XP or 2003
	return handle;
	// or:
	// static int dummy;
	//	MEMORY_BASIC_INFORMATION mbi;
	//	VirtualQuery( &dummy, &mbi, sizeof(mbi) );
	//	return reinterpret_cast<HMODULE>(mbi.AllocationBase);

}

#elif defined(UNIX)

	int dladdr(void *addr, Dl_info *info);
	#error TBD

#endif


ALWAYS_INLINE int
ProcessId() {

#if defined(WIN)
	return getpid();
#elif defined(UNIX)
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
//#if defined(WIN)
////	r ^= (u_int32_t)GetModuleHandle(NULL);
//	MEMORYSTATUS status;
//	GlobalMemoryStatus( &status );
//	r ^= (uint32_t)status.dwAvailPhys;
//#endif // WIN
	return r ? r : 1; // avoid returning 0
}


ALWAYS_INLINE size_t
RemainingStackSize() {
#if defined(WIN)
/*
	#pragma warning(push)
	#pragma warning(disable : 4312) // warning C4312: 'operation' : conversion from 'type1' to 'type2' of greater size
	NT_TIB *tib = (NT_TIB*)__readfsdword(0x18); // http://en.wikipedia.org/wiki/Win32_Thread_Information_Block
	#pragma warning(pop)
*/
	NT_TIB *tib = (NT_TIB*)NtCurrentTeb();
	volatile BYTE *currentSP;
	__asm mov [currentSP], esp;
	return currentSP - (BYTE*)tib->StackLimit;

#elif defined(UNIX)

	return (size_t)-1;
	#error TBD

#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif
}

/*
ALWAYS_INLINE size_t
ThreadStackSize() {
#if defined(WIN)

	//#pragma warning(push)
	//#pragma warning(disable : 4312) // warning C4312: 'operation' : conversion from 'type1' to 'type2' of greater size
	//NT_TIB *tib = (NT_TIB*)__readfsdword(0x18); // http://en.wikipedia.org/wiki/Win32_Thread_Information_Block
	//#pragma warning(pop)

	//volatile BYTE *currentSP;
	//__asm mov [currentSP], esp;
	//return currentSP - (BYTE*)tib->StackLimit;

	NT_TIB *tib = (NT_TIB*)NtCurrentTeb();
	DWORD stackBase = (DWORD)tib->StackBase;
	DWORD stackLimit = (DWORD)tib->StackLimit;
	return stackLimit - stackBase;

#elif defined(UNIX)

	struct rlimit limit;
	getrlimit (RLIMIT_STACK, &limit);
	return (size_t)limit.rlim_cur;

#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif
}
*/



// see http://unicode.org/faq/utf_bom.html#BOM
enum EncodingType {
	ENC_UNKNOWN,
	ENC_LATIN1, // (ISO-8859-1)
	ENC_UTF8,
	ENC_UTF16le,
	ENC_UTF16be,
	ENC_UTF32le,
	ENC_UTF32be
};

ALWAYS_INLINE EncodingType
parseEncodingName( const TCHAR *str ) {

	static const struct {
		const TCHAR *encName;
		EncodingType enc;
	} encMap[] = {
			{ L( "LATIN-1" ), ENC_LATIN1 },
			{ L( "UTF-8" ), ENC_UTF8 },
			{ L( "UTF-16be" ), ENC_UTF16be },
			{ L( "UTF-16le" ), ENC_UTF16le },
			{ L( "UTF-32be" ), ENC_UTF32be },
			{ L( "UTF-32le" ), ENC_UTF32le }
	};

	for ( size_t i = 0; i < COUNTOF( encMap ); ++i ) {

		if ( jl::stricmp( str, encMap[i].encName ) == 0 ) {

			return encMap[i].enc;
		}
	}
	
	return ENC_UNKNOWN;
}

/*
INLINE bool
IsASCII(const uint8_t *buf, size_t len) {

	for ( const uint8_t *end = buf + len; buf != end; ++buf ) {

		if ( (*buf & 0x80) || !(*buf & 0xF8) ) // > 127 || ! < 8
			return false;
	}
	return true;
}
*/

INLINE bool
IsLatin1Text( const uint8_t *buf, size_t len ) {

	for ( const uint8_t *end = buf + len; buf != end; ++buf ) {

		if ( !(*buf & 0xF8) ) // ! < 8
			return false;
	}
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
DetectEncoding( uint8_t *buf, size_t size, OUT size_t *hdrSize ) {

	if ( size >= 3 && buf[0] == 0xEF && buf[1] == 0xBB && buf[2] == 0xBF ) {

		*hdrSize = 3;
		return ENC_UTF8;
	}

	if ( size >= 2 && buf[0] == 0xFF && buf[1] == 0xFE ) {

		*hdrSize = 2;
		return ENC_UTF16le;
	}

	if ( size >= 2 && buf[0] == 0xFE && buf[1] == 0xFF ) {

		*hdrSize = 2;
		return ENC_UTF16be;
	}

	if ( size >= 4 && buf[0] == 0xFF && buf[1] == 0xFE && buf[2] == 0x00 && buf[3] == 0x00 ) {

		*hdrSize = 4;
		return ENC_UTF32le;
	}

	if ( size >= 4 && buf[0] == 0x00 && buf[1] == 0x00 && buf[2] == 0xFE && buf[3] == 0xFF ) {

		*hdrSize = 4;
		return ENC_UTF32be;
	}

	*hdrSize = 0;

	size_t scanLen = jl::min(size, 1024);
	
	if ( IsLatin1Text(buf, scanLen) )
		return ENC_LATIN1;

	if ( IsUTF8(buf, scanLen) )
		return ENC_UTF8;


	// no BOM, then guess // (TBD) find a better heuristic

	if ( size >= 2 && buf[0] != 0 && buf[1] == 0 )
		return ENC_UTF16le;

	if ( size >= 2 && buf[0] == 0 && buf[1] != 0 )
		return ENC_UTF16be;

	return ENC_UNKNOWN;
}


#ifdef MSC
	#pragma warning( push )
	#pragma warning( disable : 4244 ) // warning C4244: '=' : conversion from 'unsigned int' to 'unsigned short', possible loss of data
#endif // MSC

#define xmlLittleEndian (JLHostEndian == JLLittleEndian)


// source: libxml2 - encoding.c - MIT License - dl: https://git.gnome.org/browse/libxml2/
// changes: outlen and inlen from integer to size_t
INLINE int NOALIAS FASTCALL
UTF8ToUTF16LE( unsigned char* outb, size_t *outlen, const unsigned char* in, size_t *inlen ) {

	unsigned short* out = (unsigned short*)outb;
	const unsigned char* processed = in;
	const unsigned char *const instart = in;
	unsigned short* outstart = out;
	unsigned short* outend;
	const unsigned char* inend;
	unsigned int c, d;
	int trailing;
	unsigned char *tmp;
	unsigned short tmp1, tmp2;

	/* UTF16LE encoding has no BOM */
	if ( (out == NULL) || (outlen == NULL) || (inlen == NULL) ) return(-1);
	if ( in == NULL ) {
		*outlen = 0;
		*inlen = 0;
		return(0);
	}
	inend = in + *inlen;
	outend = out + (*outlen / 2);
	while ( in < inend ) {
		d = *in++;
		if ( d < 0x80 ) {
			c = d; trailing = 0;
		} else if ( d < 0xC0 ) {
			/* trailing byte in leading position */
			*outlen = (out - outstart) * 2;
			*inlen = processed - instart;
			return(-2);
		} else if ( d < 0xE0 ) {
			c = d & 0x1F; trailing = 1;
		} else if ( d < 0xF0 ) {
			c = d & 0x0F; trailing = 2;
		} else if ( d < 0xF8 ) {
			c = d & 0x07; trailing = 3;
		} else {
			/* no chance for this in UTF-16 */
			*outlen = (out - outstart) * 2;
			*inlen = processed - instart;
			return(-2);
		}

		if ( inend - in < trailing ) {
			break;
		}

		for ( ; trailing; trailing-- ) {
			if ( (in >= inend) || (((d = *in++) & 0xC0) != 0x80) )
				break;
			c <<= 6;
			c |= d & 0x3F;
		}

		/* assertion: c is a single UTF-4 value */
		if ( c < 0x10000 ) {
			if ( out >= outend )
				break;
			if ( xmlLittleEndian ) {
				*out++ = c;
			} else {
				tmp = (unsigned char *)out;
				*tmp = c;
				*(tmp + 1) = c >> 8;
				out++;
			}
		} else if ( c < 0x110000 ) {
			if ( out + 1 >= outend )
				break;
			c -= 0x10000;
			if ( xmlLittleEndian ) {
				*out++ = 0xD800 | (c >> 10);
				*out++ = 0xDC00 | (c & 0x03FF);
			} else {
				tmp1 = 0xD800 | (c >> 10);
				tmp = (unsigned char *)out;
				*tmp = (unsigned char)tmp1;
				*(tmp + 1) = tmp1 >> 8;
				out++;

				tmp2 = 0xDC00 | (c & 0x03FF);
				tmp = (unsigned char *)out;
				*tmp = (unsigned char)tmp2;
				*(tmp + 1) = tmp2 >> 8;
				out++;
			}
		} else
			break;
		processed = in;
	}
	*outlen = (out - outstart) * 2;
	*inlen = processed - instart;
	return(*outlen);
}

INLINE int NOALIAS FASTCALL
UTF16LEToUTF8( unsigned char* out, size_t *outlen, const unsigned char* inb, size_t *inlenb ) {

	unsigned char* outstart = out;
	const unsigned char* processed = inb;
	unsigned char* outend = out + *outlen;
	unsigned short* in = (unsigned short*)inb;
	unsigned short* inend;
	unsigned int c, d, inlen;
	unsigned char *tmp;
	int bits;

	if ( (*inlenb % 2) == 1 )
		(*inlenb)--;
	inlen = *inlenb / 2;
	inend = in + inlen;
	while ( (in < inend) && (out - outstart + 5 < (ptrdiff_t)*outlen) ) {
		if ( xmlLittleEndian ) {
			c = *in++;
		} else {
			tmp = (unsigned char *)in;
			c = *tmp++;
			c = c | (((unsigned int)*tmp) << 8);
			in++;
		}
		if ( (c & 0xFC00) == 0xD800 ) {    /* surrogates */
			if ( in >= inend ) {           /* (in > inend) shouldn't happens */
				break;
			}
			if ( xmlLittleEndian ) {
				d = *in++;
			} else {
				tmp = (unsigned char *)in;
				d = *tmp++;
				d = d | (((unsigned int)*tmp) << 8);
				in++;
			}
			if ( (d & 0xFC00) == 0xDC00 ) {
				c &= 0x03FF;
				c <<= 10;
				c |= d & 0x03FF;
				c += 0x10000;
			} else {
				*outlen = out - outstart;
				*inlenb = processed - inb;
				return(-2);
			}
		}

		/* assertion: c is a single UTF-4 value */
		if ( out >= outend )
			break;
		if ( c < 0x80 ) {
			*out++ = c;                bits = -6;
		} else if ( c < 0x800 ) {
			*out++ = ((c >> 6) & 0x1F) | 0xC0;  bits = 0;
		} else if ( c < 0x10000 ) {
			*out++ = ((c >> 12) & 0x0F) | 0xE0;  bits = 6;
		} else {
			*out++ = ((c >> 18) & 0x07) | 0xF0;  bits = 12;
		}

		for ( ; bits >= 0; bits -= 6 ) {
			if ( out >= outend )
				break;
			*out++ = ((c >> bits) & 0x3F) | 0x80;
		}
		processed = (const unsigned char*)in;
	}
	*outlen = out - outstart;
	*inlenb = processed - inb;
	return(*outlen);
}


#undef xmlLittleEndian


#ifdef MSC
	#pragma warning( pop )
#endif // MSC


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
	#if !defined(UNIX)
		patlen > 128 ? UnrolledMatch< MemCmp<T> >(text, textlen, pat, patlen) :
	#endif
		UnrolledMatch< ManualCmp<T> >(text, textlen, pat, patlen);
}


JL_END_NAMESPACE


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if defined(WIN)

#elif defined(UNIX)
	#include <pthread.h>
	#include <sched.h>
	#include <semaphore.h>
	#include <error.h>
#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif

///////////////////////////////////////////////////////////////////////////////
// memory
//

ALWAYS_INLINE bool
JL_enableLowFragmentationHeap() {

#ifdef WIN
	// enable low fragmentation heap
	HANDLE heap = GetProcessHeap();
	ULONG enable = 2;
	return HeapSetInformation(heap, HeapCompatibilityInformation, &enable, sizeof(enable)) == TRUE;
#endif // WIN

	return true; // not implemented
}



///////////////////////////////////////////////////////////////////////////////
// cpu
//

ALWAYS_INLINE bool
JL_setMonoCPU() {

#ifdef WIN

	return SetProcessAffinityMask(GetCurrentProcess(), 1) != FALSE;

#elif UNIX

	cpu_set_t  mask;
	CPU_ZERO(&mask);
	CPU_SET(0, &mask);
	return sched_setaffinity(0, sizeof(mask), &mask) == 0;

#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif

}


///////////////////////////////////////////////////////////////////////////////
// system interrupt
//

///////////////////////////////////////////////////////////////////////////////
// user locale
//

#if defined(WIN)
	// already defined
#elif defined(UNIX)
	#define LOCALE_NAME_MAX_LENGTH 256
#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif

ALWAYS_INLINE
void
JLGetUserLocaleName(wchar_t *localeName, size_t localeNameMaxLength) {

#if defined(WIN)

	::GetUserDefaultLocaleName(localeName, localeNameMaxLength);

#elif defined(UNIX)

	ASSERT( localeNameMaxLength >= 1 );
	char *locale = setLocale(LC_MESSAGES, NULL);
	if ( locale != NULL ) {

		const wchar_t *localeNameEnd = localeName + localeNameMaxLength;
		while ( localeName != localeNameEnd ) {

			*localeName = static_cast<wchar_t>(*locale);
			if ( *locale == 0 )
				break;
			++locale;
			++localeName;
		}
	} else {

		*locale = 0;
	}
#else

	#error NOT IMPLEMENTED YET	// (TBD)

#endif

}


///////////////////////////////////////////////////////////////////////////////
// system errors
//

#if defined(WIN)
	typedef DWORD JLSystemErrorCode;
#elif defined(UNIX)
	typedef int JLSystemErrorCode;
#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif


INLINE NEVER_INLINE void NOALIAS FASTCALL
JLSysetmErrorMessage( TCHAR *message, size_t maxLength, JLSystemErrorCode errorCode, const TCHAR *moduleName ) {

 #if defined(WIN)
	
	LPCVOID source;
	DWORD flags;

	if ( moduleName == NULL ) {
		
		flags = FORMAT_MESSAGE_FROM_SYSTEM;
		source = NULL;
	} else {
	
		flags = FORMAT_MESSAGE_FROM_HMODULE;
		source = ::LoadLibrary(moduleName);
	}

	DWORD chars = ::FormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK | flags, source, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), message, maxLength, NULL);
	if ( chars == 0 ) {

		*message = '\0';
	}

 #elif defined(UNIX)
	
	const TCHAR *msgBuf = strerror(errorCode); // wcserror
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


INLINE NEVER_INLINE void NOALIAS FASTCALL
JLLastSysetmErrorMessage( TCHAR *message, size_t maxLength ) {

 #if defined(WIN)
	
	JLSysetmErrorMessage(message, maxLength, ::GetLastError(), NULL);

 #elif defined(UNIX)
	
	JLSysetmErrorMessage(message, maxLength, errno, NULL);

 #else

	#error NOT IMPLEMENTED YET	// (TBD)

 #endif
	
}



///////////////////////////////////////////////////////////////////////////////
// system misc

ALWAYS_INLINE bool FASTCALL
JLDisableThreadNotifications() {

#ifdef WIN
	// doc:
	//   Disables the DLL_THREAD_ATTACH and DLL_THREAD_DETACH notifications for the specified dynamic-link library (DLL).
	//   This can reduce the size of the working set for some applications.
	BOOL st = ::DisableThreadLibraryCalls(jl::GetCurrentModule());
	return !!st;
#endif // WIN
	return true;
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
#if defined(WIN)
	return InterlockedExchange(ptr, val);
#elif defined(UNIX) // #elif ((__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1)) && ... //  #if defined(HAVE_GCC_ATOMIC32)
	return __sync_lock_test_and_set(ptr, val);
#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif
}

ALWAYS_INLINE long JLAtomicIncrement(volatile int32_t *ptr) {
#if defined(WIN)
	return _InterlockedIncrement((volatile LONG*)ptr); // Increments the value of the specified 32-bit variable as an atomic operation.
#elif defined(UNIX)
	return __sync_add_and_fetch(ptr, 1);
#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif
}

ALWAYS_INLINE long JLAtomicDecrement(volatile int32_t *ptr) {
#if defined(WIN)
	return _InterlockedDecrement((volatile LONG*)ptr);
#elif defined(UNIX)
	return __sync_sub_and_fetch(ptr, 1);
#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif
}

ALWAYS_INLINE int JLAtomicAdd(volatile int32_t *ptr, int32_t val) {
#if defined(WIN)
	return _InterlockedExchangeAdd((volatile LONG*)ptr, val) + val; // Performs an atomic addition of two 32-bit values and returns the initial value of the Addend parameter.
#elif defined(UNIX)
	return __sync_add_and_fetch(ptr, val);
#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif
}



///////////////////////////////////////////////////////////////////////////////
// semaphores
//

#if defined(WIN)
	typedef HANDLE JLSemaphoreHandler;
#elif defined(UNIX)
	typedef sem_t* JLSemaphoreHandler;
#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif


	ALWAYS_INLINE JLSemaphoreHandler JLSemaphoreCreate( int initCount ) {

	#if defined(WIN)
		return CreateSemaphore(NULL, initCount, LONG_MAX, NULL);
	#elif defined(UNIX)
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

	#if defined(WIN)
		LONG count;
		ReleaseSemaphore(semaphore, 0, &count); // INVALID
		return count;
	#elif defined(UNIX)
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
	INLINE int JLSemaphoreAcquire( JLSemaphoreHandler semaphore, int msTimeout = JLINFINITE ) {

		ASSERT( JLSemaphoreOk(semaphore) );
	#if defined(WIN)
		DWORD status = WaitForSingleObject(semaphore, msTimeout == JLINFINITE ? INFINITE : msTimeout);
		ASSERT( status != WAIT_FAILED );
		if ( status == WAIT_TIMEOUT )
			return JLTIMEOUT;
		return JLOK;
	#elif defined(UNIX)
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
	#if defined(WIN)
		BOOL st = ReleaseSemaphore(semaphore, 1, NULL);
		ASSERT( st != FALSE );
	#elif defined(UNIX)
		int st = sem_post(semaphore);
		ASSERT( st == 0 );
		JL_IGNORE( st );
	#else
		#error NOT IMPLEMENTED YET	// (TBD)
	#endif
	}

	ALWAYS_INLINE void JLSemaphoreFree( JLSemaphoreHandler *pSemaphore ) {

		ASSERT( pSemaphore != NULL && JLSemaphoreOk(*pSemaphore) );
	#if defined(WIN)
		BOOL st = CloseHandle(*pSemaphore);
		ASSERT( st != FALSE );
	#elif defined(UNIX)
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

#if defined(WIN)
	typedef CRITICAL_SECTION* JLMutexHandler; // doc. critical section objects provide a slightly faster, more efficient mechanism for mutual-exclusion synchronization.
#elif defined(UNIX)
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
	#if defined(WIN)
		InitializeCriticalSection(mutex);
	#elif defined(UNIX)
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
	#if defined(WIN)
		DeleteCriticalSection(*pMutex);
	#elif defined(UNIX)
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
	#if defined(WIN)
		EnterCriticalSection(mutex);
	#elif defined(UNIX)
		int st = pthread_mutex_lock(mutex); // doc. shall not return an error code of [EINTR].
		ASSERT( st == 0 );
		JL_IGNORE( st );
	#else
		#error NOT IMPLEMENTED YET	// (TBD)
	#endif
	}

	ALWAYS_INLINE void JLMutexRelease( JLMutexHandler mutex ) {

		ASSERT( JLMutexOk(mutex) );
	#if defined(WIN)
		LeaveCriticalSection(mutex);
	#elif defined(UNIX)
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

#if defined(WIN)

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

#elif defined(UNIX)

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
	#if defined(WIN)
		HANDLE hEvent;
		LONG waitingThreadCount;
		CRITICAL_SECTION cs;
	#elif defined(UNIX)
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
	#if defined(WIN)
		InitializeCriticalSection(&ev->cs);
		ev->waitingThreadCount = 0;
		ev->hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		if ( ev->hEvent == NULL )
			return NULL;
	#elif defined(UNIX)
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
	#if defined(WIN)
		DeleteCriticalSection(&(*ev)->cs);
		BOOL st = CloseHandle((*ev)->hEvent);
		ASSERT( st != FALSE );
	#elif defined(UNIX)
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
	#if defined(WIN)
		EnterCriticalSection(&ev->cs);
		if ( ev->waitingThreadCount != 0 || !ev->autoReset ) {

			BOOL st = SetEvent(ev->hEvent);
			ASSERT( st != FALSE );
			JL_IGNORE(st);
		}
		LeaveCriticalSection(&ev->cs);
	#elif defined(UNIX)
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
	#if defined(WIN)
		EnterCriticalSection(&ev->cs);
		BOOL st = ResetEvent(ev->hEvent);
		ASSERT( st != FALSE );
		JL_IGNORE(st);
		LeaveCriticalSection(&ev->cs);
	#elif defined(UNIX)
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
	#if defined(WIN)

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

	#elif defined(UNIX)
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

	#if defined(WIN)
		#define JL_THREAD_PRIORITY_LOWEST THREAD_PRIORITY_LOWEST
		#define JL_THREAD_PRIORITY_LOW THREAD_PRIORITY_BELOW_NORMAL
		#define JL_THREAD_PRIORITY_NORMAL THREAD_PRIORITY_NORMAL
		#define JL_THREAD_PRIORITY_HIGH THREAD_PRIORITY_ABOVE_NORMAL
		#define JL_THREAD_PRIORITY_HIGHEST THREAD_PRIORITY_HIGHEST
		typedef HANDLE JLThreadHandler;
		typedef int JLThreadPriorityType;
//		#define JLThreadFuncDecl unsigned __stdcall
		#define JLThreadFuncDecl DWORD WINAPI
//		typedef unsigned (__stdcall *JLThreadRoutine)(void *);
		typedef PTHREAD_START_ROUTINE JLThreadRoutine;
	#elif defined(UNIX)
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


	ALWAYS_INLINE JLThreadHandler JLThreadStart( JLThreadRoutine threadRoutine, void *pv = NULL ) {

	#if defined(WIN)
		// The new thread handle is created with the THREAD_ALL_ACCESS access right.
		// If a security descriptor is not provided when the thread is created, a default security descriptor is constructed for the new thread using the primary token of the process that is creating the thread.
		// When a caller attempts to access the thread with the OpenThread function, the effective token of the caller is evaluated against this security descriptor to grant or deny access.
		return CreateThread(NULL, 0, threadRoutine, pv, 0, NULL);
//		return (JLThreadHandler)_beginthreadex(NULL, 0, threadRoutine, pv, 0, NULL);
	#elif defined(UNIX)
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
	#if defined(WIN)
		DWORD status;
//		status = WaitForSingleObject(thread, 0);
//		ASSERT( status != WAIT_FAILED );
//		return status == WAIT_TIMEOUT; // else != WAIT_OBJECT_0 ?
		DWORD exitValue;
		status = GetExitCodeThread(thread, &exitValue);
		ASSERT( status != FALSE );
		return exitValue == STILL_ACTIVE;
	#elif defined(UNIX)
		int policy;
		struct sched_param param;
		return pthread_getschedparam(*thread, &policy, &param) != ESRCH; // errno.h
	#else
		#error NOT IMPLEMENTED YET	// (TBD)
	#endif
	}


	ALWAYS_INLINE void JLThreadFree( JLThreadHandler *pThread ) {

		ASSERT( pThread != NULL && JLThreadOk(*pThread) );
	#if defined(WIN)
		BOOL st = CloseHandle(*pThread);
		ASSERT( st != FALSE );
		JL_IGNORE(st);
	#elif defined(UNIX)
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

	#if defined(WIN)
		ASSERT( exitValue != STILL_ACTIVE ); // see JLThreadIsActive(), doc: GetExitCodeProcess function (http://msdn.microsoft.com/en-us/library/windows/desktop/ms683189(v=vs.85).aspx)
		ExitThread(exitValue);
//		_endthreadex(exitValue); // doc: _endthreadex does not close the thread handle.
	#elif defined(UNIX)
		pthread_exit((void*)exitValue);
	#else
		#error NOT IMPLEMENTED YET	// (TBD)
	#endif
	}


	ALWAYS_INLINE void JLThreadCancel( JLThreadHandler thread ) {

		ASSERT( JLThreadOk(thread) );
	#if defined(WIN)
		// yes, using TerminateThread can lead to memory leaks and worse.
		// doc. The handle must have the THREAD_TERMINATE access right. ... Use the GetExitCodeThread function to retrieve a thread's exit value.
		BOOL st = TerminateThread(thread, 0);
		ASSERT( st != 0 );
	#elif defined(UNIX)
		int st = pthread_cancel(*thread);
		ASSERT( st == 0 );
		JL_IGNORE( st );
	#else
		#error NOT IMPLEMENTED YET	// (TBD)
	#endif
	}


	ALWAYS_INLINE void JLThreadPriority( JLThreadHandler thread, JLThreadPriorityType priority ) {

		ASSERT( JLThreadOk(thread) );
	#if defined(WIN)
		BOOL st = SetThreadPriority(thread, priority);
		ASSERT( st != FALSE );
	#elif defined(UNIX)
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


	ALWAYS_INLINE void JLThreadWait( JLThreadHandler thread, unsigned int *exitValue = NULL ) {

		ASSERT( JLThreadOk(thread) );
	#if defined(WIN)
		BOOL st = WaitForSingleObject(thread, INFINITE);
		ASSERT( st == WAIT_OBJECT_0 );
		if ( exitValue != NULL )
			GetExitCodeThread(thread, (DWORD*)exitValue);
	#elif defined(UNIX)
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

#if defined(WIN)
	typedef DWORD JLTLSKey;
#elif defined(UNIX)
	typedef pthread_key_t JLTLSKey;
#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif

#define JLTLSInvalidKey 0

ALWAYS_INLINE JLTLSKey JLTLSAllocKey() {
	JLTLSKey key;
#if defined(WIN)
	key = TlsAlloc();
	ASSERT( key != TLS_OUT_OF_INDEXES );
	key++;
#elif defined(UNIX)
	int st = pthread_key_create(&key, NULL);
	ASSERT( st == 0 );
	JL_IGNORE( st );
#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif
	return key;
}


ALWAYS_INLINE void JLTLSFreeKey( JLTLSKey key ) {
#if defined(WIN)
	ASSERT( key != 0 );
	key--;
	ASSERT( key >= 0 && key < TLS_MINIMUM_AVAILABLE );
	BOOL st = TlsFree(key);
	ASSERT( st != FALSE );
#elif defined(UNIX)
	int st = pthread_key_delete(key);
	ASSERT( st == 0 );
#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif
	JL_IGNORE( st );
}


ALWAYS_INLINE void JLTLSSet( JLTLSKey key, void *value ) {
#if defined(WIN)
	ASSERT( key != 0 );
	key--;
	ASSERT( key >= 0 && key < TLS_MINIMUM_AVAILABLE );
	BOOL st = TlsSetValue(key, value);
	ASSERT( st != FALSE );
#elif defined(UNIX)
	int st = pthread_setspecific(key, value);
	ASSERT( st == 0 );
#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif
	JL_IGNORE( st );
}


ALWAYS_INLINE void* JLTLSGet( JLTLSKey key ) {
#if defined(WIN)
	ASSERT( key != 0 );
	key--;
	void *value;
	ASSERT( key >= 0 && key < TLS_MINIMUM_AVAILABLE );
	value = TlsGetValue(key);
	ASSERT( value != 0 || GetLastError() == ERROR_SUCCESS  );
	return value;
#elif defined(UNIX)
	return pthread_getspecific(key);
#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif

}


///////////////////////////////////////////////////////////////////////////////
// dynamic libraries
//

#if defined(WIN)
	typedef HMODULE JLLibraryHandler;
#elif defined(UNIX)
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

	#if defined(WIN)
		// GetErrorMode() only exists on Vista and higher,
		// call SetErrorMode() twice to achieve the same effect.
		// see also SetThreadErrorMode()
		UINT oldErrorMode = ::SetErrorMode(SEM_FAILCRITICALERRORS);
		::SetErrorMode( oldErrorMode | SEM_FAILCRITICALERRORS ); // avoid error popups
		HMODULE hModule = ::LoadLibraryA(filename); // If the function fails, the return value is NULL. 
		// Restore previous error mode.
		::SetErrorMode(oldErrorMode);
		return hModule;
	#elif defined(UNIX)
		dlerror(); // Resets the error indicator.
		return dlopen(filename, RTLD_LAZY | RTLD_LOCAL); // shall return NULL on error. // see. RTLD_NOW / RTLD_GLOBAL
	#else
		#error NOT IMPLEMENTED YET	// (TBD)
	#endif
	}

	ALWAYS_INLINE void JLDynamicLibraryClose( JLLibraryHandler *libraryHandler ) {

	#if defined(WIN)
		BOOL st = FreeLibrary(*libraryHandler);
		ASSERT( st != FALSE );
	#elif defined(UNIX)
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

	#if defined(WIN)
		DWORD errorCode = ::GetLastError();
		LPVOID lpMsgBuf;
		DWORD result = ::FormatMessageA(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK,
			NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&lpMsgBuf, 0, NULL );
		if ( result != 0 ) {

			jl::strncpy( message, (LPSTR)lpMsgBuf, maxLength - 1 );
			message[maxLength-1] = '\0';
		} else
			*message = '\0';
	#elif defined(UNIX)
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

	ALWAYS_INLINE void *JLDynamicLibrarySymbol( JLLibraryHandler libraryHandler, const char *symbolName ) { // symbolName cannot be wchar_t

	#if defined(WIN)
		return (void*)::GetProcAddress(libraryHandler, symbolName);
	#elif defined(UNIX)
		dlerror(); // Resets the error indicator.
		return dlsym(libraryHandler, symbolName);
	#else
		#error NOT IMPLEMENTED YET	// (TBD)
	#endif
	}

	ALWAYS_INLINE void JLDynamicLibraryName( void *addr, TCHAR *fileName, size_t maxFileNameLength ) {

		// DWORD st = GetModuleFileName(libraryHandler, (LPCH)fileName, (DWORD)fileNameSize); ASSERT( st != ERROR_INSUFFICIENT_BUFFER );
		ASSERT( maxFileNameLength > 0 );
	#if defined(WIN)
		HMODULE libraryHandler;
		if ( !::GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (const TCHAR *)addr, &libraryHandler) ) { // requires XP or 2003
			
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
		::GetModuleFileName(libraryHandler, (TCHAR*)fileName, maxFileNameLength);
	#elif defined(UNIX)
		Dl_info info;
		if ( !dladdr(addr, &info) || !info.dli_fbase || !info.dli_fname ) {

			fileName[0] = '\0';
			return;
		}
		int len = jl::min(strlen(info.dli_fname), maxFileNameLength-1);
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
#if defined(WIN)
	typedef struct {
		HANDLE mutex;
		HANDLE event;
	} *JLCondHandler;
#elif defined(UNIX)
	typedef struct {
		pthread_mutex_t mutex;
		pthread_cond_t cond;
	} *JLCondHandler;
#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif


	#if defined(WIN)
	ALWAYS_INLINE JLCondHandler JLCreateCond() {

		JLCondHandler cond = (JLCondHandler)malloc(sizeof(*cond));
		cond->mutex = CreateMutex(NULL, FALSE, NULL); // lpMutexAttributes, bInitialOwner, lpName
		cond->event = CreateEvent(NULL, TRUE, FALSE, NULL); // lpEventAttributes, bManualReset, bInitialState, lpName
		return cond;
	}
	#elif defined(UNIX)
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

