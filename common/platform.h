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
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4311)
#pragma warning(disable : 4312)
#pragma warning(disable : 4267)
#pragma warning(disable : 4996)
#endif // #ifdef WIN32


#if defined(_WINDOWS) || defined(WIN32) // Windows platform
	#define XP_WIN

	#define DLL_EXT ".dll"
	#define PATH_SEPARATOR '\\'
	#define LLONG __int64

	#define DLLEXPORT __declspec(dllexport)
	#define DLLLOCAL


#elif defined(_MACOSX) // MacosX platform
	#define XP_UNIX

	#define DLL_EXT ".dylib"
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
	#define XP_UNIX

	#define DLL_EXT ".so"
	#define PATH_SEPARATOR '/'
	#define MAX_PATH PATH_MAX
	#define LLONG long long

	#ifdef HAVE_GCCVISIBILITYPATCH
		#define DLLEXPORT __attribute__ ((visibility("default")))
		#define DLLLOCAL __attribute__ ((visibility("hidden")))
	#else
		#define DLLEXPORT
		#define DLLLOCAL
	#endif

#endif // Windows/MacosX/Linux platform

// MS specific ?
//#ifndef O_BINARY
//	#define O_BINARY 0
//#endif


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
