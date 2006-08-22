#include <windows.h>

#define XP_WIN
#include <jsapi.h>
#include <nspr.h>

#include "nsprError.h"
#include "nsprSocket.h"
#include "nsprGlobal.h"


//////////////////////////////////////////////////////////////////////////////////////////////////


extern "C" __declspec(dllexport) JSBool ModuleInit(JSContext *cx, JSObject *obj) {

	InitSocketClass(cx, obj);
	InitErrorClass(cx, obj);
	InitGlobal(cx, obj);

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

