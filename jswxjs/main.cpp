#include "stdafx.h"

#define JSHELPER_UNSAFE_DEFINED
#include "../common/jshelper.h"
#include "../common/jsclass.h"

#define MAX_WXJS_MODULES 32
#define WXJS_INIT_PROC_NAME "wxJS_Init"
#define WXJS_DESTROY_PROC_NAME "wxJS_Destroy"

#ifdef WIN32
	#define DLL_EXT ".dll"
#else
	#define DLL_EXT ".so"
#endif

static HMODULE _moduleList[MAX_WXJS_MODULES] = { NULL };

DEFINE_FUNCTION( LoadWXJSModule ) {

	RT_ASSERT_ARGC(1);
	char *fileName;
	RT_JSVAL_TO_STRING( argv[0], fileName );
	char libFileName[MAX_PATH];
	strcpy( libFileName, fileName );
	strcat( libFileName, DLL_EXT );
	HMODULE module = ::LoadLibrary(libFileName);
	RT_ASSERT_2( module != NULL, "Unable to load the library %s (error:%d).", libFileName, GetLastError() );
	int i;
	for ( i = 0; _moduleList[i] != NULL; ++i ); // find a free module slot
	RT_ASSERT( i < 32, "unable to load more libraries" );
	_moduleList[i] = module;
	typedef bool (*ModuleInitFunction)(JSContext *, JSObject *);
	ModuleInitFunction moduleInit = (ModuleInitFunction)::GetProcAddress( module, WXJS_INIT_PROC_NAME );
	RT_ASSERT_1( moduleInit != NULL, "Module initialization function not found in %s.", libFileName );
	*rval = moduleInit( cx, obj ) ? JSVAL_TRUE : JSVAL_FALSE;
	return JS_TRUE;
}


DEFINE_FUNCTION( UnloadWXJSModule ) {

	return JS_TRUE;
}


CONFIGURE_STATIC

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION( LoadWXJSModule )
		FUNCTION( UnloadWXJSModule )
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
	END_STATIC_PROPERTY_SPEC

END_STATIC



extern "C" __declspec(dllexport) JSBool ModuleInit(JSContext *cx, JSObject *obj) {

	INIT_STATIC(cx, obj);
/*
	jsval unsafeModeValue;
	JSBool jsStatus = GetConfigurationValue(cx, "unsafeMode", &unsafeModeValue);
	RT_ASSERT( jsStatus != JS_FALSE, "Unable to read unsafeMode state from configuration object." );
	if ( unsafeModeValue != JSVAL_VOID && JSVAL_IS_BOOLEAN(unsafeModeValue) )
		SET_UNSAFE_MODE( JSVAL_TO_BOOLEAN(unsafeModeValue) == JS_TRUE );
*/
	return JS_TRUE;
}

extern "C" __declspec(dllexport) JSBool ModuleRelease(JSContext *cx, JSObject *obj) {

// free used modules
	typedef void (*ModuleFreeFunction)(void);
	for ( int i = sizeof(_moduleList) / sizeof(*_moduleList) - 1; i >= 0; --i ) // beware: 'i' must be signed
		if ( _moduleList[i] != NULL ) {

			ModuleFreeFunction moduleFree = (ModuleFreeFunction)::GetProcAddress( _moduleList[i], WXJS_DESTROY_PROC_NAME );
			if ( moduleFree != NULL )
				moduleFree();
			::FreeLibrary(_moduleList[i]);
			_moduleList[i] = NULL;
		}

	return JS_TRUE;
}


BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved ) {

  switch (ul_reason_for_call) {

	  case DLL_PROCESS_ATTACH:
	  case DLL_THREAD_ATTACH:
	  case DLL_THREAD_DETACH:
	  case DLL_PROCESS_DETACH:
		  break;
  }
  return TRUE;
}

