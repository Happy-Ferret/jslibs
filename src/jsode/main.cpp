/* ***** BEGIN LICENSE BLOCK *****
 * Version: GNU GPL 2.0
 *
 * The contents of this file are subject to the
 * GNU General Public License Version 2.0; you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at
 * http://www.gnu.org/licenses/gpl.html
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 * ***** END LICENSE BLOCK ***** */

#include "stdafx.h"

#include "mass.h"
#include "space.h"
#include "world.h"
#include "body.h"
#include "joint.h"

#include "geom.h"

extern bool _unsafeMode = false;

extern bool _odeFinalization = false;

// the following avoid ODE to be linked with User32.lib ( MessageBox* symbol is used in ../ode/src/ode/src/error.cpp )

#ifdef XP_WIN
int WINAPI MessageBoxA(__in_opt HWND hWnd, __in_opt LPCSTR lpText, __in_opt LPCSTR lpCaption, __in UINT uType) {

	return IDCANCEL;
}
#endif


void messageHandler(int errnum, const char *msg, va_list ap) {

	char text[1024];
	int len = vsprintf(text, msg, ap);
//	vprintf(msg, ap);

	// ThrowOdeException(cx, ...


//	abort(); // http://msdn2.microsoft.com/en-us/library/k089yyh0(VS.80).aspx
}


/**doc t:header
$MODULE_HEADER
 jsode is a module that manages support to ODE.
 ODE is an open source, high performance library for simulating rigid body dynamics.
 It is fully featured, stable, mature and platform independent with an easy to use C/C++ API.
 It has advanced joint types and integrated collision detection with friction.
 ODE is useful for simulating vehicles, objects in virtual reality environments and virtual creatures.
 It is currently used in many computer games, 3D authoring tools and simulation tools.
 $H note
  In the following API description, ,,vec3,, type is a js 3 dimentions $ARRAY like `[1,3,5]`
$FILE_TOC
**/

/**doc t:footer
$MODULE_FOOTER
**/

EXTERN_C DLLEXPORT JSBool ModuleInit(JSContext *cx, JSObject *obj) {

	_unsafeMode = GetHostPrivate(cx)->unsafeMode;

	int status = ode::dInitODE2(/*ode::dAllocateFlagCollisionData*/0);
	JL_S_ASSERT( status != 0, "Unable to initialize ODE." );

	ode::dSetErrorHandler(messageHandler);
	ode::dSetDebugHandler(messageHandler);
	ode::dSetMessageHandler(messageHandler);

	INIT_CLASS( Space );
	INIT_CLASS( Joint );
	INIT_CLASS( JointBall );
	INIT_CLASS( JointHinge );
	INIT_CLASS( JointSlider );
	INIT_CLASS( JointUniversal );
	INIT_CLASS( JointPiston );
	INIT_CLASS( JointFixed );
	INIT_CLASS( JointAMotor );
	INIT_CLASS( JointLMotor );
	INIT_CLASS( Mass );
	INIT_CLASS( Body );
	INIT_CLASS( Geom );
	INIT_CLASS( GeomSphere );
	INIT_CLASS( GeomBox );
	INIT_CLASS( GeomPlane );
	INIT_CLASS( GeomCapsule );
	INIT_CLASS( GeomCylinder );
	INIT_CLASS( GeomRay );
	INIT_CLASS( GeomTrimesh );
	INIT_CLASS( World );
	INIT_CLASS( SurfaceParameters );

	return JS_TRUE;
	JL_BAD;
}

EXTERN_C DLLEXPORT JSBool ModuleRelease(JSContext *cx, JSObject *obj) {
	
	_odeFinalization = true;

	return JS_TRUE;
}

EXTERN_C DLLEXPORT void ModuleFree() {



	ode::dCloseODE();
}


#ifdef XP_WIN
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {

	if ( fdwReason == DLL_PROCESS_ATTACH )
		DisableThreadLibraryCalls(hinstDLL);
	return TRUE;
}
#endif // XP_WIN


/*
User guide: http://www.ode.org/ode-latest-userguide.html
*/
