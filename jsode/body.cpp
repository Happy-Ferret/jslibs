#include "stdafx.h"
#include "mass.h"
#include "body.h"
#include "world.h"

#include "tools.h"

JSObject *body;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void body_Finalize(JSContext *cx, JSObject *obj) {

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

	RT_ASSERT_CONSTRUCTING(&body_class);
	RT_ASSERT_ARGC(1);
	RT_ASSERT_CLASS(JSVAL_TO_OBJECT(argv[0]),&world_class);
	JSObject *worldObject = JSVAL_TO_OBJECT(argv[0]);
	ode::dWorldID worldID = (ode::dWorldID)JS_GetPrivate(cx,worldObject);
	RT_ASSERT_1(worldID != NULL, "%s object not initialized", world_class.name );

	ode::dBodyID bodyID = ode::dBodyCreate(worldID);
	RT_ASSERT( bodyID != NULL, "unable to create the body." );
	JS_SetPrivate(cx, obj, bodyID);
	JS_SetParent(cx, obj, worldObject); // protect from GC
//	ode::dBodySetData(bodyID,worldObject);
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
		JS_SetParent(cx, obj, NULL); // no more protection from GC needed
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
	VectorToArray(cx, 3, vector, vp);
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool body_setter_vector(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	ode::dBodyID bodyID = (ode::dBodyID)JS_GetPrivate( cx, obj );
	RT_ASSERT( bodyID != NULL, RT_ERROR_NOT_INITIALIZED );
	ode::dVector3 vector;
	switch(JSVAL_TO_INT(id)) {
		case position:
			ArrayToVector(cx, 3, vp, vector);
			ode::dBodySetPosition( bodyID, vector[0], vector[1], vector[2] );
			break;
		case quaternion:
			ArrayToVector(cx, 4, vp, vector);
			ode::dBodySetQuaternion( bodyID, vector );
			break;
		case linearVel:
			ArrayToVector(cx, 3, vp, vector);
			ode::dBodySetLinearVel( bodyID, vector[0], vector[1], vector[2] );
			break;
		case angularVel:
			ArrayToVector(cx, 3, vp, vector);
			ode::dBodySetAngularVel( bodyID, vector[0], vector[1], vector[2] );
			break;
		case force:
			ArrayToVector(cx, 3, vp, vector);
			ode::dBodySetForce( bodyID, vector[0], vector[1], vector[2] );
			break;
		case torque:
			ArrayToVector(cx, 3, vp, vector);
			ode::dBodySetTorque( bodyID, vector[0], vector[1], vector[2] );
			break;
	}
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool body_get_mass(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	ode::dBodyID bodyID = (ode::dBodyID)JS_GetPrivate( cx, obj );
	RT_ASSERT( bodyID != NULL, RT_ERROR_NOT_INITIALIZED );

	JSObject *massObject = JS_NewObject(cx, &mass_class, NULL, NULL);
	RT_ASSERT(massObject != NULL, "unable to construct Mass object.");
	
	ode::dMass *mass = (ode::dMass*)malloc(sizeof(ode::dMass));
	RT_ASSERT_ALLOC(mass);
	ode::dBodyGetMass(bodyID, mass);
	JS_SetPrivate(cx, massObject, mass);
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSPropertySpec body_PropertySpec[] = { // *name, tinyid, flags, getter, setter
	{ "position"   , position   , JSPROP_PERMANENT|JSPROP_SHARED, body_getter_vector, body_setter_vector },
	{ "quaternion" , quaternion , JSPROP_PERMANENT|JSPROP_SHARED, body_getter_vector, body_setter_vector },
	{ "linearVel"  , linearVel  , JSPROP_PERMANENT|JSPROP_SHARED, body_getter_vector, body_setter_vector },
	{ "angularVel" , angularVel , JSPROP_PERMANENT|JSPROP_SHARED, body_getter_vector, body_setter_vector },
	{ "force"      , force      , JSPROP_PERMANENT|JSPROP_SHARED, body_getter_vector, body_setter_vector },
	{ "torque"     , torque     , JSPROP_PERMANENT|JSPROP_SHARED, body_getter_vector, body_setter_vector },

	{ "mass"       , 0 , JSPROP_PERMANENT|JSPROP_SHARED, body_get_mass, body_set_mass },
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
JSClass body_class = { "Body", JSCLASS_HAS_PRIVATE,
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