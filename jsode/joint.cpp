#include "stdafx.h"

#include "joint.h"
#include "body.h"
#include "world.h"

#include "tools.h"

JSObject *joint;
JSObject *jointBall;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum { body1=0, body2=1 };

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool joint_get_body(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	JS_GetReservedSlot(cx, obj, JSVAL_TO_INT(id), vp);
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSPropertySpec joint_PropertySpec[] = { // *name, tinyid, flags, getter, setter
	{ "body1"   , body1, JSPROP_PERMANENT|JSPROP_SHARED|JSPROP_READONLY, joint_get_body, NULL },
	{ "body2"   , body2, JSPROP_PERMANENT|JSPROP_SHARED|JSPROP_READONLY, joint_get_body, NULL },
	{ 0 }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool joint_destroy(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT_CLASS(JS_GetPrototype(cx,JS_GetPrototype(cx, obj)), &joint_class);

	ode::dJointID jointID = (ode::dJointID)JS_GetPrivate( cx, obj );
	RT_ASSERT( jointID != NULL, RT_ERROR_NOT_INITIALIZED );

	JS_SetReservedSlot(cx, obj, body1, JSVAL_VOID);
	JS_SetReservedSlot(cx, obj, body2, JSVAL_VOID);

	JS_SetPrivate(cx, obj, NULL); 

	ode::dJointDestroy(jointID);
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool joint_attach(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT_ARGC(2);
//	RT_ASSERT_CLASS(obj, &class_joint); ???

	ode::dJointID jointID = (ode::dJointID)JS_GetPrivate( cx, obj );
	RT_ASSERT( jointID != NULL, RT_ERROR_NOT_INITIALIZED );

	JSObject *body1Object, *body2Object;

	JS_ValueToObject(cx, argv[0], &body1Object);
	RT_ASSERT_CLASS(body1Object, &body_class);
	JS_SetReservedSlot(cx, obj, body1, argv[0]);
	ode::dBodyID bodyID1 = (ode::dBodyID)JS_GetPrivate(cx, body1Object);
//	RT_ASSERT(bodyID != NULL, RT_ERROR_NOT_INITIALIZED);

	JS_ValueToObject(cx, argv[1], &body2Object);
	RT_ASSERT_CLASS(body2Object, &body_class);
	JS_SetReservedSlot(cx, obj, body2, argv[1]);
	ode::dBodyID bodyID2 = (ode::dBodyID)JS_GetPrivate(cx, body2Object);
//	RT_ASSERT(bodyID != NULL, RT_ERROR_NOT_INITIALIZED);

	dJointAttach(jointID, bodyID1, bodyID2);
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSFunctionSpec joint_FunctionSpec[] = { // *name, tinyid, flags, getter, setter
	{ "Destroy", joint_destroy },
	{ "Attach" , joint_attach },
	{ 0 }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSClass joint_class = { "Joint", 0,
	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub
};




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void jointBall_Finalize(JSContext *cx, JSObject *obj) {
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool jointBall_construct(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT_CONSTRUCTING(&jointBall_class);
	RT_ASSERT_ARGC(1);

	JSObject *worldObject;
	
	RT_SAFE(JS_ValueToObject(cx, argv[0], &worldObject));
	RT_UNSAFE(worldObject = JSVAL_TO_OBJECT(argv[0]));

	RT_ASSERT_CLASS(worldObject,&world_class);
	ode::dWorldID worldID = (ode::dWorldID)JS_GetPrivate(cx,worldObject);
	RT_ASSERT_1(worldID != NULL, "%s object not initialized", world_class.name );

	ode::dJointID jointID = ode::dJointCreateBall(worldID, 0); // The joint group ID is 0 to allocate the joint normally.

//	RT_ASSERT( bodyID != NULL, "unable to create the body." );
	JS_SetPrivate(cx, obj, jointID);
//	JS_SetParent(cx, obj, worldObject); // protect from GC
//	ode::dBodySetData(bodyID,worldObject);
	return JS_TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSClass jointBall_class = { "JointBall", JSCLASS_HAS_PRIVATE | JSCLASS_HAS_RESERVED_SLOTS(2),
	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, jointBall_Finalize
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSObject *jointInitClass( JSContext *cx, JSObject *obj ) {

	joint = JS_InitClass( cx, obj, NULL, &joint_class, NULL, 0, joint_PropertySpec, joint_FunctionSpec, NULL, NULL );

	jointBall = JS_InitClass( cx, obj, joint, &jointBall_class, jointBall_construct, 0, NULL, joint_FunctionSpec, NULL, NULL );

	return joint;
}
