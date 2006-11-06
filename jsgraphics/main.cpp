#include "stdafx.h"
#include "jswindow.h"
#include "jsgl.h"
#include "jstransformation.h"
#include "../configuration/configuration.h"

DEFINE_UNSAFE_MODE

extern "C" __declspec(dllexport) JSBool ModuleInit(JSContext *cx, JSObject *obj) {

	jsval value;
	if ( GetConfigurationValue(cx, "unsafeMode", &value) == JS_FALSE )
		return JS_FALSE;
	SET_UNSAFE_MODE( value != JSVAL_VOID && JSVAL_IS_BOOLEAN(value) && JSVAL_TO_BOOLEAN(value) == JS_TRUE );

	if ( InitClassTransformation( cx, obj ) == JS_FALSE ) 
		return JS_FALSE;
	if ( InitClassWindow( cx, obj ) == JS_FALSE ) 
		return JS_FALSE;
	if ( InitClassGl( cx, obj ) == JS_FALSE ) 
		return JS_FALSE;
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
