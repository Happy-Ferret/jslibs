#include "stdafx.h"

#include "../configuration/Configuration.h"

#include "error.h"
#include "database.h"
#include "result.h"


extern "C" __declspec(dllexport) JSBool ModuleInit(JSContext *cx, JSObject *obj) {

	jsval unsafeModeValue;
	JSBool jsStatus = GetConfigurationValue(cx, "unsafeMode", &unsafeModeValue);
	RT_ASSERT( jsStatus != JS_FALSE, "Unable to read unsafeMode state from configuration object." );
	if ( JSVAL_IS_BOOLEAN(unsafeModeValue) )
		SET_UNSAFE_MODE( JSVAL_TO_BOOLEAN(unsafeModeValue) == JS_TRUE );

	INIT_CLASS( SqliteError )
	INIT_CLASS( Result )
	INIT_CLASS( Database )

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

