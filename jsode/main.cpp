#include "stdafx.h"
#include "mass.h"
#include "world.h"
#include "body.h"
#include "joint.h"

#include "geom.h"

#include "../configuration/configuration.h"

DEFINE_UNSAFE_MODE;


// the following avoid ODE to be linked with User32.lib ( MessageBox* symbol is used in ../ode/src/ode/src/error.cpp )
int WINAPI MessageBoxA(__in_opt HWND hWnd, __in_opt LPCSTR lpText, __in_opt LPCSTR lpCaption, __in UINT uType) {

	return IDCANCEL;
}
 
extern "C" void messageHandler(int errnum, const char *msg, va_list ap) {

//	abort(); // http://msdn2.microsoft.com/en-us/library/k089yyh0(VS.80).aspx
}

extern "C" __declspec(dllexport) JSBool ModuleInit(JSContext *cx, JSObject *obj) {

	ode::dSetErrorHandler(messageHandler);
	ode::dSetDebugHandler(messageHandler);
	ode::dSetMessageHandler(messageHandler);

	jsval unsafeModeValue;
	JSBool jsStatus = GetConfigurationValue(cx, "unsafeMode", &unsafeModeValue);
	RT_ASSERT( jsStatus != JS_FALSE, "Unable to read unsafeMode state from configuration object." );
	if ( unsafeModeValue != JSVAL_VOID && JSVAL_IS_BOOLEAN(unsafeModeValue) )
		SET_UNSAFE_MODE( JSVAL_TO_BOOLEAN(unsafeModeValue) == JS_TRUE );

	INIT_CLASS( Joint );
	INIT_CLASS( JointBall );
	INIT_CLASS( JointHinge );
	INIT_CLASS( JointSlider );
	INIT_CLASS( JointFixed );
	INIT_CLASS( Mass );
	INIT_CLASS( Body );
	INIT_CLASS( Geom );
	INIT_CLASS( GeomSphere );
	INIT_CLASS( GeomBox );
	INIT_CLASS( World );

//	JSObject *p = JS_GetPrototype(cx, classObjectJointHinge);
//	JSObject *q = JS_GetPrototype(cx, p);

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