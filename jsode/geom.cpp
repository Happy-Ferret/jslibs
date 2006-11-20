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

/*
static int ReadMatrix(void *pv, float **pm) { // Doc: __declspec(noinline) tells the compiler to never inline a particular function.

	ode::dBodyID id = (ode::dBodyID)pv;
	const ode::dReal * m43 = dBodyGetRotation( id );
	const ode::dReal * pos = dBodyGetPosition( id );
// [TBD] center of mass ajustement ?
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
	m[12] = pos[0];// - comx;
	m[13] = pos[1];// - comy;
	m[14] = pos[2];// - comz;
	m[15] = 1;
	return true;
}
... DEFINE_FUNCTION( ClassConstruct ) -- JSCLASS_CONSTRUCT_PROTOTYPE
	SetNativeInterface(cx, obj, NI_READ_MATRIX44, (FunctionPointer)ReadMatrix, bodyID); // [TBD] check return status
*/

BEGIN_CLASS( Geom )


DEFINE_FUNCTION( Destroy ) {

	ode::dGeomID geom = (ode::dGeomID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE( geom );
	ode::dGeomDestroy(geom);
	JS_SetPrivate(cx, obj, NULL);
	return JS_TRUE;
}


DEFINE_FUNCTION( ClearOffset ) {

	ode::dGeomID geom = (ode::dGeomID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE( geom );
	ode::dGeomClearOffset(geom);
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

	// [TBD] check if the obj's private data is the right body. else ERROR
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


DEFINE_PROPERTY( tansformation ) {

	ode::dGeomID geom = (ode::dGeomID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(geom);
	float tmp[16], *m = tmp;
	NIMatrix44Read ReadMatrix;
	void *descriptor;
	GetNativeInterface(cx, obj, NI_READ_MATRIX44, (FunctionPointer*)&ReadMatrix, &descriptor);
	RT_ASSERT( ReadMatrix != NULL && descriptor != NULL, "Invalid matrix interface." );
	ReadMatrix(descriptor, (float**)&m);
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


DEFINE_PROPERTY( offsetPositionGetter ) {

	ode::dGeomID geom = (ode::dGeomID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(geom);
	const ode::dReal *vector = ode::dGeomGetOffsetPosition(geom);
	FloatVectorToArray(cx, 3, vector, vp);
	return JS_TRUE;
}


DEFINE_PROPERTY( offsetPositionSetter ) {

	ode::dGeomID geom = (ode::dGeomID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(geom);
	ode::dVector3 vector;
	FloatArrayToVector(cx, 3, vp, vector);
	ode::dGeomSetOffsetPosition( geom, vector[0], vector[1], vector[2] );
	return JS_TRUE;
}



CONFIGURE_CLASS

	BEGIN_FUNCTION_SPEC
		FUNCTION( Destroy )
		FUNCTION( ClearOffset )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_WRITE_STORE( body )
		PROPERTY_WRITE( tansformation )
		PROPERTY( enable )
		PROPERTY( position )
		PROPERTY( offsetPosition )
	END_PROPERTY_SPEC

END_CLASS

