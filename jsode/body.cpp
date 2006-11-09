#include "stdafx.h"
#include <jscntxt.h>
#include "mass.h"
#include "body.h"
#include "world.h"

#include "../common/jsNativeInterface.h"


JSObject *body;

#define SLOT_PARENT 0

static int ReadMatrix(void *pv, float **pm) { // Doc: __declspec(noinline) tells the compiler to never inline a particular function.

	ode::dBodyID id = (ode::dBodyID)pv;
	const ode::dReal * m43 = dBodyGetRotation( id );
	const ode::dReal * pos = dBodyGetPosition( id );

// [TBD] center of mass ajustement ?
	float comx = 0;
	float comy = 0;
	float comz = 0;

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
	m[12] = pos[0] - comx;
	m[13] = pos[1] - comy;
	m[14] = pos[2] - comz;
	m[15] = 1;

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void body_Finalize(JSContext *cx, JSObject *obj) {

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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool body_construct(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

/*
// with new operator
//  argv[-1] : Body
//  argv[-2] : Function
//  argv[-3] : global
//  JS_GetParent : global


// without new operator
//  argv[-1] : World
//  argv[-2] : Function
//  argv[-3] : global


	JSObject *o = JS_GetScopeChain(cx);

	JSClass *cl = JS_GetClass(o);
*/

/*
JSObject *o;
jsval *sp = cx->fp->sp;
JS_ValueToObject(cx, *(sp-1) , &o);
JSClass *cl = JS_GetClass(o);
*/


//	cx.lastInternalResult


//	JSObject *o;
//	JS_ValueToObject(cx, cx->lastInternalResult, &o);
//	JSClass *cl = JS_GetClass(o);

	RT_ASSERT_CONSTRUCTING(&body_class);
	RT_ASSERT_ARGC(1);
	RT_ASSERT_CLASS(JSVAL_TO_OBJECT(argv[0]),&world_class);
	JSObject *worldObject = JSVAL_TO_OBJECT(argv[0]);
	ode::dWorldID worldID = (ode::dWorldID)JS_GetPrivate(cx,worldObject);
	RT_ASSERT_1(worldID != NULL, "%s object not initialized", world_class.name );

	ode::dBodyID bodyID = ode::dBodyCreate(worldID);
	RT_ASSERT( bodyID != NULL, "unable to create the body." );
	JS_SetPrivate(cx, obj, bodyID);
	JS_SetReservedSlot(cx, obj, SLOT_PARENT, OBJECT_TO_JSVAL(worldObject)); //
//	ode::dBodySetData(bodyID,worldObject);
	SetNativeInterface(cx, obj, NI_READ_MATRIX44, ReadMatrix, bodyID); // [TBD] check return status
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool body_call(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool body_destroy(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT_CLASS(obj, &body_class);
	ode::dBodyID bodyId = (ode::dBodyID)JS_GetPrivate( cx, obj );
	RT_ASSERT( bodyId != NULL, RT_ERROR_NOT_INITIALIZED ); // [TBD] manage world-connected

//	if ( bodyId != NULL ) {
		dBodyDestroy(bodyId);
		JS_SetPrivate(cx, obj, NULL);
		JS_SetReservedSlot(cx, obj, SLOT_PARENT, JSVAL_VOID);
//	}
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool body_isConnectedTo(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT_ARGC(1);

	RT_ASSERT_CLASS(obj, &body_class);
	ode::dBodyID thisBodyID = (ode::dBodyID)JS_GetPrivate( cx, obj );
	RT_ASSERT( thisBodyID != NULL, RT_ERROR_NOT_INITIALIZED );

	JSObject *bodyObject;
	JS_ValueToObject(cx, argv[0], &bodyObject);
	RT_ASSERT_CLASS(bodyObject, &body_class);
	ode::dBodyID bodyID = (ode::dBodyID)JS_GetPrivate(cx, bodyObject);

	ode::dAreConnected(thisBodyID, bodyID);

	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSFunctionSpec body_FunctionSpec[] = { // *name, call, nargs, flags, extra
	{ "Destroy" , body_destroy, 0, 0, 0 },
	{ "IsConnectedTo" , body_isConnectedTo, 0, 0, 0 },
	{ 0 }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum { position, quaternion, linearVel, angularVel, force, torque };

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool body_getter_vector(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool body_setter_vector(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool body_get_mass(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	if ( *vp == JSVAL_VOID ) {

		JSObject *massObject = JS_NewObject(cx, &mass_class, NULL, NULL);
		RT_ASSERT(massObject != NULL, "unable to construct Mass object.");
		JS_SetReservedSlot(cx, massObject, MASS_SLOT_BODY, OBJECT_TO_JSVAL(obj));
		*vp = OBJECT_TO_JSVAL(massObject);
	}
	return JS_TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
JSBool body_set_mass(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	ode::dBodyID bodyID = (ode::dBodyID)JS_GetPrivate( cx, obj );
	RT_ASSERT( bodyID != NULL, RT_ERROR_NOT_INITIALIZED );
	JSObject *massObject;
	JS_ValueToObject(cx, *vp, &massObject);
	RT_ASSERT_CLASS(massObject, &mass_class);
	ode::dMass *mass = (ode::dMass*)JS_GetPrivate(cx, massObject);
	RT_ASSERT(mass != NULL, RT_ERROR_NOT_INITIALIZED);

	ode::dBodySetMass(bodyID, mass);
	return JS_TRUE;
}
*/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSPropertySpec body_PropertySpec[] = { // *name, tinyid, flags, getter, setter
	{ "position"   , position   , JSPROP_PERMANENT|JSPROP_SHARED, body_getter_vector, body_setter_vector },
	{ "quaternion" , quaternion , JSPROP_PERMANENT|JSPROP_SHARED, body_getter_vector, body_setter_vector },
	{ "linearVel"  , linearVel  , JSPROP_PERMANENT|JSPROP_SHARED, body_getter_vector, body_setter_vector },
	{ "angularVel" , angularVel , JSPROP_PERMANENT|JSPROP_SHARED, body_getter_vector, body_setter_vector },
	{ "force"      , force      , JSPROP_PERMANENT|JSPROP_SHARED, body_getter_vector, body_setter_vector },
	{ "torque"     , torque     , JSPROP_PERMANENT|JSPROP_SHARED, body_getter_vector, body_setter_vector },

	{ "mass"       , 0 , JSPROP_PERMANENT|JSPROP_READONLY, body_get_mass, NULL },
	{ 0 }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//JSBool body_static_getter_myStatic(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
//
//  return JS_TRUE;
//}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSPropertySpec body_static_PropertySpec[] = { // *name, tinyid, flags, getter, setter
//	{ "myStatic" , 0, JSPROP_PERMANENT|JSPROP_READONLY, body_static_getter_myStatic , NULL },
	{ 0 }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSClass body_class = { "Body", JSCLASS_HAS_PRIVATE | JSCLASS_HAS_RESERVED_SLOTS(1),
	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, body_Finalize,
	NULL, NULL, body_call
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSObject *bodyInitClass( JSContext *cx, JSObject *obj ) {

	body = JS_InitClass( cx, obj, NULL, &body_class, body_construct, 0, body_PropertySpec, body_FunctionSpec, body_static_PropertySpec, NULL );
	return body;
}


/****************************************************************

*/