#include "stdafx.h"

#include "misc.h"

#include "rsa.h"
#include "prng.h"
#include "hash.h"
#include "crypt.h"

#include "../configuration/configuration.h"

DEFINE_UNSAFE_MODE;

extern "C" __declspec(dllexport) JSBool ModuleInit(JSContext *cx, JSObject *obj) {

	SET_UNSAFE_MODE( JSVAL_TO_BOOLEAN(GetConfigurationValue(cx, "unsafeMode")) == JS_TRUE );

	ltc_mp = ltm_desc;

	InitErrorClass( cx, obj );
	miscInitClass( cx, obj );
	rsaInitClass( cx, obj );
	prngInitClass( cx, obj );
	hashInitClass( cx, obj );
//	cipherInitClass( cx, obj );
	cryptInitClass( cx, obj );
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

