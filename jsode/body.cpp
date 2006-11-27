#include "stdafx.h"
//#include <jscntxt.h>
#include "mass.h"
#include "body.h"
#include "world.h"
#include "../common/jsNativeInterface.h"

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

BEGIN_CLASS( Body )


DEFINE_FINALIZE() {

	// [TBD] manage Mass object (body.mass)
/*
	ode::dBodyID bodyId = (ode::dBodyID)JS_GetPrivate( cx, obj );
//	JSObject *parent = JS_GetParent(cx,obj); // If the object does not have a parent, or the object finalize function is active, JS_GetParent returns NULL.
	if ( bodyId != NULL ) {
		dBodyDestroy(bodyId);
		JS_SetPrivate(cx, obj, NULL);
	}
*/
}


DEFINE_CONSTRUCTOR() {

	RT_ASSERT_CONSTRUCTING(&classBody);
	RT_ASSERT_ARGC(1);
	
	ode::dWorldID worldId;
	ValToWorldID(cx, argv[0], &worldId);

	ode::dBodyID bodyID = ode::dBodyCreate(worldId);
	RT_ASSERT( bodyID != NULL, "unable to create the body." );

	JS_SetPrivate(cx, obj, bodyID);
	JS_SetReservedSlot(cx, obj, BODY_SLOT_WORLD, argv[0]);

//	JS_AddRoot(cx, obj); // Doc: The pointer or jsval whose address is passed as rp must live in storage that remains allocated until the balancing JS_RemoveRoot call is made.
//	ode::dBodySetData(bodyID, obj);

//	ode::dBodySetData(bodyID,worldObject);
	SetNativeInterface(cx, obj, NI_READ_MATRIX44, (FunctionPointer)ReadMatrix, bodyID); // [TBD] check return status
	return JS_TRUE;
}


DEFINE_FUNCTION( Destroy ) {

	RT_ASSERT_CLASS(obj, &classBody);
	ode::dBodyID bodyId = (ode::dBodyID)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( bodyId ); // [TBD] manage world-connected ( when bodyId == 0 )
	dBodyDestroy(bodyId);
	JS_SetPrivate(cx, obj, NULL);
	JS_SetReservedSlot(cx, obj, BODY_SLOT_WORLD, JSVAL_VOID);
	return JS_TRUE;
}


DEFINE_FUNCTION( IsConnectedTo ) {

	RT_ASSERT_ARGC(1);
	RT_ASSERT_CLASS(obj, &classBody);
	ode::dBodyID thisBodyID = (ode::dBodyID)JS_GetPrivate( cx, obj );
	RT_ASSERT( thisBodyID != NULL, RT_ERROR_NOT_INITIALIZED );
	ode::dBodyID bodyId;
	ValToBodyID(cx, argv[0], &bodyId);
	ode::dAreConnected(thisBodyID, bodyId);
	return JS_TRUE;
}


enum { position, quaternion, linearVel, angularVel, force, torque };

DEFINE_PROPERTY( vectorGetter ) {

	ode::dBodyID bodyID = (ode::dBodyID)JS_GetPrivate(cx, obj);
	RT_ASSERT(bodyID != NULL, RT_ERROR_NOT_INITIALIZED);
	const ode::dReal *vector;
	int dim;
	switch(JSVAL_TO_INT(id)) {
		case position:
			vector = ode::dBodyGetPosition(bodyID);
			dim = 3;
			break;
		case quaternion:
			vector = ode::dBodyGetQuaternion(bodyID);
			dim = 4;
			break;
		case linearVel:
			vector = ode::dBodyGetLinearVel(bodyID);
			dim = 3;
			break;
		case angularVel:
			vector = ode::dBodyGetAngularVel(bodyID);
			dim = 3;
			break;
		case force:
			vector = ode::dBodyGetForce(bodyID);
			dim = 3;
			break;
		case torque:
			vector = ode::dBodyGetTorque(bodyID);
			dim = 3;
			break;
	}
	FloatVectorToArray(cx, dim, vector, vp);
	return JS_TRUE;
}


DEFINE_PROPERTY( vectorSetter ) {

	ode::dBodyID bodyID = (ode::dBodyID)JS_GetPrivate( cx, obj );
	RT_ASSERT( bodyID != NULL, RT_ERROR_NOT_INITIALIZED );
	ode::dVector3 vector;
	switch(JSVAL_TO_INT(id)) {
		case position:
			FloatArrayToVector(cx, 3, vp, vector);
			ode::dBodySetPosition( bodyID, vector[0], vector[1], vector[2] );
			break;
		case quaternion:
			FloatArrayToVector(cx, 4, vp, vector);
			ode::dBodySetQuaternion( bodyID, vector );
			break;
		case linearVel:
			FloatArrayToVector(cx, 3, vp, vector);
			ode::dBodySetLinearVel( bodyID, vector[0], vector[1], vector[2] );
			break;
		case angularVel:
			FloatArrayToVector(cx, 3, vp, vector);
			ode::dBodySetAngularVel( bodyID, vector[0], vector[1], vector[2] );
			break;
		case force:
			FloatArrayToVector(cx, 3, vp, vector);
			ode::dBodySetForce( bodyID, vector[0], vector[1], vector[2] );
			break;
		case torque:
			FloatArrayToVector(cx, 3, vp, vector);
			ode::dBodySetTorque( bodyID, vector[0], vector[1], vector[2] );
			break;
	}
	return JS_TRUE;
}


DEFINE_PROPERTY( mass ) {

	if ( *vp == JSVAL_VOID ) { // if mass do not exist, we have to create it and store it

		JSObject *massObject = JS_NewObject(cx, &classMass, NULL, NULL);
		RT_ASSERT(massObject != NULL, "unable to construct Mass object.");
		JS_SetReservedSlot(cx, massObject, MASS_SLOT_BODY, OBJECT_TO_JSVAL(obj));
		*vp = OBJECT_TO_JSVAL(massObject);
	}
	return JS_TRUE;
}


//JSBool body_set_mass(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
//
//	ode::dBodyID bodyID = (ode::dBodyID)JS_GetPrivate( cx, obj );
//	RT_ASSERT( bodyID != NULL, RT_ERROR_NOT_INITIALIZED );
//	JSObject *massObject;
//	JS_ValueToObject(cx, *vp, &massObject);
//	RT_ASSERT_CLASS(massObject, &mass_class);
//	ode::dMass *mass = (ode::dMass*)JS_GetPrivate(cx, massObject);
//	RT_ASSERT(mass != NULL, RT_ERROR_NOT_INITIALIZED);
//
//	ode::dBodySetMass(bodyID, mass);
//	return JS_TRUE;
//}

CONFIGURE_CLASS

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION( Destroy )
		FUNCTION( IsConnectedTo )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_SWITCH( position  , vector )
		PROPERTY_SWITCH( quaternion, vector )
		PROPERTY_SWITCH( linearVel , vector )
		PROPERTY_SWITCH( angularVel, vector )
		PROPERTY_SWITCH( force     , vector )
		PROPERTY_SWITCH( torque    , vector )
		PROPERTY_READ_STORE( mass ) // mass is only a wrapper to dBodyGetMass and dBodySetMass
	END_PROPERTY_SPEC

	HAS_PRIVATE  // private: BodyID
	HAS_RESERVED_SLOTS(1)

END_CLASS


/****************************************************************


// with new operator
//  argv[-1] : Body
//  argv[-2] : Function
//  argv[-3] : global
//  JS_GetParent : global


// without new operator
//  argv[-1] : World
//  argv[-2] : Function
//  argv[-3] : global

JSObject *o;
jsval *sp = cx->fp->sp;
JS_ValueToObject(cx, *(sp-1) , &o);
JSClass *cl = JS_GetClass(o);


//	cx.lastInternalResult
//	JSObject *o;
//	JS_ValueToObject(cx, cx->lastInternalResult, &o);
//	JSClass *cl = JS_GetClass(o);

*/