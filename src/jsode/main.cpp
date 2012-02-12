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
#include <jlhelper.cpp>
#include <jslibsModule.cpp>

#include "mass.h"
#include "space.h"
#include "world.h"
#include "body.h"
#include "joint.h"
#include "geom.h"


DECLARE_CLASS( Vector )



#ifdef XP_WIN
// The following avoid the need for ODE to be linked with User32.lib ( MessageBox* symbol is used in ../ode/src/ode/src/error.cpp )
	#pragma warning( push )
	#pragma warning(disable : 4273)
	int WINAPI MessageBoxA( HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType) { return IDOK; }
	#pragma warning( pop )
#endif


void messageHandler(int errnum, const char *msg, va_list ap) {

	char text[1024];
	int len = vsprintf(text, msg, ap);
	//vprintf(msg, ap);
	write(2, text, len); // (TBD) enhance this !

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

//typedef void * dAllocFunction (size_t size);
//typedef void * dReallocFunction (void *ptr, size_t oldsize, size_t newsize);
//typedef void dFreeFunction (void *ptr, size_t size);

void* odeReallocFunction (void *ptr, size_t oldsize, size_t newsize) {
	return jl_realloc(ptr, newsize);
}
void odeFreeFunction (void *ptr, size_t size) {
	jl_free(ptr);
}

JSBool
ModuleInit(JSContext *cx, JSObject *obj, uint32_t id) {

//	put_StackCommitSize

	JL_CHK( InitJslibsModule(cx, id) );

	ode::dSetAllocHandler(jl_malloc); // do not need an intermediate function because prototype match.
	ode::dSetReallocHandler(odeReallocFunction);
	ode::dSetFreeHandler(odeFreeFunction);

	ode::dSetErrorHandler(messageHandler);
	ode::dSetDebugHandler(messageHandler);
	ode::dSetMessageHandler(messageHandler);

	int status = ode::dInitODE2(/*ode::dAllocateFlagCollisionData*/0  /*| ode::dInitFlagManualThreadCleanup*/);
	JL_ASSERT( status != 0, E_LIB, E_STR("ODE"), E_INIT );


	ASSERT( !finite(dInfinity) );
	ASSERT( !finite(-dInfinity) );

	ASSERT( !finite((jsdouble)dInfinity) );
	ASSERT( !finite((jsdouble)-dInfinity) );

	ASSERT( JSVAL_TO_DOUBLE(JS_GetPositiveInfinityValue(cx)) == dInfinity );
	ASSERT( JSVAL_TO_DOUBLE(JS_GetNegativeInfinityValue(cx)) == -dInfinity );

	ASSERT( JS_GetPositiveInfinityValue(cx) == DOUBLE_TO_JSVAL(dInfinity) );
	ASSERT( JS_GetNegativeInfinityValue(cx) == DOUBLE_TO_JSVAL(-dInfinity) );


//	INIT_CLASS( Vector );
	INIT_CLASS( JointGroup );
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
	INIT_CLASS( GeomConvex );
	INIT_CLASS( GeomTrimesh );
	INIT_CLASS( World );
	INIT_CLASS( SurfaceParameters );

	return JS_TRUE;
	JL_BAD;
}

JSBool
ModuleRelease(JSContext *cx) {

	JL_IGNORE(cx);
	return JS_TRUE;
}

void
ModuleFree() {

	ode::dCloseODE();

	ode::dSetErrorHandler(0);
	ode::dSetDebugHandler(0);
	ode::dSetMessageHandler(0);

	ode::dSetAllocHandler(0);
	ode::dSetReallocHandler(0);
	ode::dSetFreeHandler(0);
}

// User guide: http://www.ode.org/ode-latest-userguide.html
