

#if defined(_WINDOWS) || defined(WIN32)
	#define XP_WIN
	#define DLL_EXT ".dll"

	#define DLLEXPORT __declspec(dllexport)
	#define DLLLOCAL
#else
	#define XP_UNIX
	#define DLL_EXT ".so"

	#ifdef HAVE_GCCVISIBILITYPATCH
		#define DLLEXPORT __attribute__ ((visibility("default")))
		#define DLLLOCAL __attribute__ ((visibility("hidden")))
	#else
		#define DLLEXPORT
		#define DLLLOCAL
	#endif
#endif
