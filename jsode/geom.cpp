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

/*
Collision callback:
	http://opende.sourceforge.net/wiki/index.php/Collision_callback_member_function

check:

	dGeomGetCategoryBits (o1);
	dGeomGetCollideBits (o1);
*/

#include "stdafx.h"
#include "body.h"
#include "geom.h"
#include "../common/jsNativeInterface.h"

int ReadMatrix(void *pv, float **pm) { // Doc: __declspec(noinline) tells the compiler to never inline a particular function.

	ode::dGeomID geomID = (ode::dGeomID)pv;

	// read LOCAL position and rotation
	const ode::dReal* pos = ode::dGeomGetPosition(geomID);
	const ode::dReal* m43 = ode::dGeomGetRotation(geomID);

	float *m = *pm;
	m[0]  = m43[0];
	m[1]  = m43[4];
	m[2]  = m43[8];
	m[3]  = 0;
	m[4]  = m43[1];
	m[5]  = m43[5];
	m[6]  = m43[9];
	m[7]  = 0;
	m[8]  = m43[2];
	m[9]  = m43[6];
	m[10] = m43[10];
	m[11] = 0;
	m[12] = pos[0];
	m[13] = pos[1];
	m[14] = pos[2];
	m[15] = 1;
	return true;
}


JSBool SetupReadMatrix(JSContext *cx, JSObject *obj, ode::dGeomID geomId) {

	return SetNativeInterface(cx, obj, NI_READ_MATRIX44, (FunctionPointer)ReadMatrix, geomId);
}


BEGIN_CLASS( Geom )

DEFINE_FUNCTION( Destroy ) {

	ode::dGeomID geomId = (ode::dGeomID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE( geomId );
	ode::dGeomSetData(geomId, NULL); // perhaps useless
	ode::dGeomDestroy(geomId);
	JS_SetPrivate(cx, obj, NULL);
	return JS_TRUE;
}


DEFINE_PROPERTY( enableSetter ) {

	ode::dGeomID geom = (ode::dGeomID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(geom);
	JSBool enableState;
	JS_ValueToBoolean(cx, *vp, &enableState);
	if ( enableState )
		ode::dGeomEnable(geom);
	else
		ode::dGeomDisable(geom);
	return JS_TRUE;
}


DEFINE_PROPERTY( enableGetter ) {

	ode::dGeomID geom = (ode::dGeomID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE( geom );
	*vp = ode::dGeomIsEnabled(geom) == 1 ? JSVAL_TRUE : JSVAL_FALSE;
	return JS_TRUE;
}


DEFINE_PROPERTY( body ) {

	// (TBD) check if the obj's private data is the right body. else ERROR
	//ode::dGeomID geom = (ode::dGeomID)JS_GetPrivate(cx, obj);
	//RT_ASSERT_RESOURCE( geom );
	//ode::dBodyID bodyId = dGeomGetBody(geom);

	ode::dGeomID geom = (ode::dGeomID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE( geom );
	ode::dBodyID bodyId;
	if ( ValToBodyID(cx, *vp, &bodyId) == JS_FALSE )
		return JS_FALSE;
	ode::dGeomSetBody(geom, bodyId);
	return JS_TRUE;
}


// setting undefined means clear the offset
DEFINE_PROPERTY( offset ) {

	ode::dGeomID geom = (ode::dGeomID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(geom);
	if ( *vp == JSVAL_VOID ) {
		ode::dGeomClearOffset(geom);
	} else {
		float tmp[16], *m = tmp;
		NIMatrix44Read ReadMatrix;
		void *descriptor;
		GetNativeInterface(cx, obj, NI_READ_MATRIX44, (FunctionPointer*)&ReadMatrix, &descriptor);
		RT_ASSERT( ReadMatrix != NULL && descriptor != NULL, "Invalid matrix interface." );
		ReadMatrix(descriptor, (float**)&m);
		RT_ASSERT( *m != NULL, "Invalid matrix." );
		ode::dMatrix3 m3 = { m[0], m[4], m[8], 0, m[1], m[5], m[9], 0, m[2], m[6], m[10], 0 }; // (TBD) check
		ode::dGeomSetOffsetRotation(geom, m3);
		ode::dGeomSetOffsetPosition(geom, m[3], m[7], m[11]);
	}
	return JS_TRUE;
}



DEFINE_PROPERTY( tansformation ) {

	ode::dGeomID geom = (ode::dGeomID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(geom);
	float tmp[16], *m = tmp;
	NIMatrix44Read ReadMatrix;
	void *descriptor;
	GetNativeInterface(cx, obj, NI_READ_MATRIX44, (FunctionPointer*)&ReadMatrix, &descriptor);
	RT_ASSERT( ReadMatrix != NULL && descriptor != NULL, "Invalid matrix interface." );
	ReadMatrix(descriptor, (float**)&m);
	RT_ASSERT( *m != NULL, "Invalid matrix." );
	ode::dMatrix3 m3 = { m[0], m[4], m[8], 0, m[1], m[5], m[9], 0, m[2], m[6], m[10], 0 }; // (TBD) check
	ode::dGeomSetRotation(geom, m3);
	ode::dGeomSetPosition(geom, m[3], m[7], m[11]);
	return JS_TRUE;
}


DEFINE_PROPERTY( positionGetter ) {

	ode::dGeomID geom = (ode::dGeomID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(geom);
	const ode::dReal *vector = ode::dGeomGetPosition(geom);
	FloatVectorToArray(cx, 3, vector, vp);
	return JS_TRUE;
}


DEFINE_PROPERTY( positionSetter ) {

	ode::dGeomID geom = (ode::dGeomID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(geom);
	ode::dVector3 vector;
	FloatArrayToVector(cx, 3, vp, vector);
	ode::dGeomSetPosition( geom, vector[0], vector[1], vector[2] );
	return JS_TRUE;
}


/*

DEFINE_PROPERTY( offsetPositionGetter ) {

	ode::dGeomID geom = (ode::dGeomID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(geom);
	const ode::dReal *vector = ode::dGeomGetOffsetPosition(geom); // (TBD) dGeomGetOffsetRotation
	FloatVectorToArray(cx, 3, vector, vp);
	return JS_TRUE;
}


DEFINE_PROPERTY( offsetPositionSetter ) {

	ode::dGeomID geom = (ode::dGeomID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(geom);
	ode::dVector3 vector;
	FloatArrayToVector(cx, 3, vp, vector);
	ode::dGeomSetOffsetPosition( geom, vector[0], vector[1], vector[2] ); // (TBD) dGeomSetOffsetWorldRotation
	return JS_TRUE;
}
*/


CONFIGURE_CLASS

	BEGIN_FUNCTION_SPEC
		FUNCTION( Destroy )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_WRITE_STORE( body )
		PROPERTY_WRITE( tansformation )
		PROPERTY_WRITE( offset )
		PROPERTY( enable )
		PROPERTY( position )
//		PROPERTY( offsetPosition )
	END_PROPERTY_SPEC

	// (TBD) explain why HAS_PRIVATE is needed in Geom
	HAS_PRIVATE // needed because Finalize use JS_GetPrivate

END_CLASS

