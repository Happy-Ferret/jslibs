#include "stdafx.h"

#include "objex.h"

extern "C" DLLEXPORT JSBool ModuleInit(JSContext *cx, JSObject *obj) {

	objexInitClass( cx, obj );
	return JS_TRUE;
}

/*
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
*/
