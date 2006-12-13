#include "stdafx.h"
#include "template.h"

extern "C" __declspec(dllexport) JSBool ModuleInit(JSContext *cx, JSObject *obj) {

	INIT_CLASS( Template );

	return JS_TRUE;
}

extern "C" __declspec(dllexport) JSBool ModuleRelease(JSContext *cx, JSObject *obj) {

	return JS_TRUE;
}

extern "C" __declspec(dllexport) void ModuleRelease(JSContext *cx) {
}

extern "C" __declspec(dllexport) void ModuleFree() {
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

