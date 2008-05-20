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

DEFINE_UNSAFE_MODE;

// the following avoid ODE to be linked with User32.lib ( MessageBox* symbol is used in ../ode/src/ode/src/error.cpp )


int WINAPI MessageBoxA(__in_opt HWND hWnd, __in_opt LPCSTR lpText, __in_opt LPCSTR lpCaption, __in UINT uType) {

	return IDCANCEL;
}

void messageHandler(int errnum, const char *msg, va_list ap) {

//	abort(); // http://msdn2.microsoft.com/en-us/library/k089yyh0(VS.80).aspx
}

EXTERN_C DLLEXPORT JSBool ModuleInit(JSContext *cx, JSObject *obj) {

	ode::dSetErrorHandler(messageHandler);
	ode::dSetDebugHandler(messageHandler);
	ode::dSetMessageHandler(messageHandler);

	SET_UNSAFE_MODE( GetConfigurationValue(cx, "unsafeMode" ) == JSVAL_TRUE );

	INIT_CLASS( Space );
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
	INIT_CLASS( GeomPlane );
	INIT_CLASS( GeomCapsule );
	INIT_CLASS( GeomRay );
	INIT_CLASS( World );
	INIT_CLASS( SurfaceParameters );

//	JSObject *p = JS_GetPrototype(cx, classObjectJointHinge);
//	JSObject *q = JS_GetPrototype(cx, p);

	return JS_TRUE;
}

EXTERN_C DLLEXPORT JSBool ModuleRelease(JSContext *cx, JSObject *obj) {

	ode::dCloseODE();
	return JS_TRUE;
}


/*
User guide: http://www.ode.org/ode-latest-userguide.html


var world = new ode.World;
world.gravity = [0,0,-9.81];


new world.Body;
-or-
new ode.Body(world);


*/