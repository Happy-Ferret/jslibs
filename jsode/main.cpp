#include "stdafx.h"
#include "mass.h"
#include "world.h"
#include "body.h"
#include "joint.h"


extern "C" void messageHandler(int errnum, const char *msg, va_list ap) {
}

extern "C" __declspec(dllexport) JSBool ModuleInit(JSContext *cx, JSObject *obj) {

//	ode::dSetErrorHandler(messageHandler);
//	ode::dSetDebugHandler(messageHandler);
//	ode::dSetMessageHandler(messageHandler);

	jointInitClass( cx, obj );
	massInitClass( cx, obj );
	bodyInitClass( cx, obj );
	worldInitClass( cx, obj );
	return JS_TRUE;
}

extern "C" __declspec(dllexport) JSBool ModuleFinalize(JSContext *cx, JSObject *obj) {

	ode::dCloseODE();
	return JS_TRUE;
}


BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved ) {

//	configuration = GetConfiguration( cx );

  switch (ul_reason_for_call) {

	  case DLL_PROCESS_ATTACH:
	  case DLL_THREAD_ATTACH:
	  case DLL_THREAD_DETACH:
	  case DLL_PROCESS_DETACH:
		  break;
  }
  return TRUE;
}

/*
User guide: http://www.ode.org/ode-latest-userguide.html


var world = new ode.World;
world.gravity = [0,0,-9.81];


new world.Body;
-or-
new ode.Body(world);


*/