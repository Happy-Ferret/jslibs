

#if defined(_WINDOWS) || defined(WIN32)
	#define XP_WIN
	#define DLL_EXT ".dll"
	#define PATH_SEPARATOR '\\'

	#define DLLEXPORT __declspec(dllexport)
	#define DLLLOCAL
#else
	#define XP_UNIX
	#define DLL_EXT ".so"
	#define PATH_SEPARATOR '/'

	#ifdef HAVE_GCCVISIBILITYPATCH
		#define DLLEXPORT __attribute__ ((visibility("default")))
		#define DLLLOCAL __attribute__ ((visibility("hidden")))
	#else
		#define DLLEXPORT
		#define DLLLOCAL
	#endif
#endif


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
