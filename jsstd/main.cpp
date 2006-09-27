#include "stdafx.h"

#define XP_WIN
#include <jsapi.h>



extern "C" __declspec(dllexport) JSBool ModuleInit(JSContext *cx, JSObject *obj) {



	JSObject *global = JS_GetGlobalObject(cx);
	if ( global != NULL )
		return JS_TRUE;

	jsval jsvalConfig;
	JS_GetProperty( cx, global, 'configuration', &jsvalConfig );
	if ( jsvalConfig == JSVAL_VOID )
		return  JS_TRUE;
	JSObject *configuration = JSVAL_TO_OBJECT(jsvalConfig);

	GetPrivate




  return JS_TRUE;
}


BOOL APIENTRY DllMain( HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved ) {

  switch (ul_reason_for_call) {

	  case DLL_PROCESS_ATTACH:
	  case DLL_THREAD_ATTACH:
	  case DLL_THREAD_DETACH:
	  case DLL_PROCESS_DETACH:
		  break;
  }
  return TRUE;
}

