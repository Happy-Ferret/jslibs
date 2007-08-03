static const char *_revision = "$Rev:$";

#include "stdafx.h"

#define XP_WIN
#include <jsapi.h>


#include "jsobj.h"
#include "jsatom.h"

#include "../common/jsclass.h"
#include "../common/jsconfiguration.h"

#define USE_UNSAFE_MODE

#include "../common/jshelper.h"
#include "jstest.h"
#include "jstest1.h"

DEFINE_UNSAFE_MODE;

extern "C" __declspec(dllexport) JSBool ModuleInit(JSContext *cx, JSObject *obj) {

	SET_UNSAFE_MODE( GetConfigurationValue(cx, "unsafeMode" ) == JSVAL_TRUE );

	JSFunctionSpec _tmp[] = {{0}};
	JSBool st;

	st = INIT_CLASS( Body );
	st = INIT_CLASS( World );



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
