#include <windows.h>
#include <jsapi.h>
#include "ffi.h"


extern "C" __declspec(dllexport) JSBool ModuleInit(JSContext *cx, JSObject *obj) {

	Init_JSNI(cx, obj);
	return JS_TRUE;
}

extern "C" __declspec(dllexport) JSBool ModuleRelease(JSContext *cx, JSObject *obj) {

	// [TBD] cleanup the global object
	Release_JSNI(cx);
	Destroy_JSNI();
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

