#include "stdafx.h"
#include "joint.h"
#include "body.h"
#include "world.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool joint_get_body(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	JS_GetReservedSlot(cx, obj, JSVAL_TO_INT(id), vp);
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSPropertySpec joint_PropertySpec[] = { // *name, tinyid, flags, getter, setter
	{ "body1"   , JOINT_SLOT_BODY1, JSPROP_PERMANENT|JSPROP_SHARED|JSPROP_READONLY, joint_get_body, NULL },
	{ "body2"   , JOINT_SLOT_BODY2, JSPROP_PERMANENT|JSPROP_SHARED|JSPROP_READONLY, joint_get_body, NULL },
	{ 0 }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool joint_destroy(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT_CLASS(JS_GetPrototype(cx,JS_GetPrototype(cx, obj)), &joint_class);
	ode::dJointID jointID = (ode::dJointID)JS_GetPrivate( cx, obj );
	RT_ASSERT( jointID != NULL, RT_ERROR_NOT_INITIALIZED );

	JS_SetReservedSlot(cx, obj, JOINT_SLOT_BODY1, JSVAL_VOID);
	JS_SetReservedSlot(cx, obj, JOINT_SLOT_BODY2, JSVAL_VOID);

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
	RT_ASSERT_CLASS(body1Object, &classBody);
	JS_SetReservedSlot(cx, obj, JOINT_SLOT_BODY1, argv[0]);
	ode::dBodyID bodyID1 = (ode::dBodyID)JS_GetPrivate(cx, body1Object);
//	RT_ASSERT(bodyID != NULL, RT_ERROR_NOT_INITIALIZED);

	JS_ValueToObject(cx, argv[1], &body2Object);
	RT_ASSERT_CLASS(body2Object, &classBody);
	JS_SetReservedSlot(cx, obj, JOINT_SLOT_BODY2, argv[1]);
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

//==============================================================================================================================================

/*
struct jointStruct {
	enum type;
	ode::dJointID (*create)(ode::dWorldID, ode::dJointGroupID);
};
ode::dJointID (*create)(ode::dWorldID, ode::dJointGroupID);
typedef void (*setVector)(ode::dJointID, ode::dReal x, ode::dReal y, ode::dReal z);
setVectorFunctions[] = {
*/


//{ ode::dJointTypeBall, ode::dJointCreateBall 




enum { 
	ballAnchor, ballAnchor2, 
	hingeAnchor, hingeAnchor2, hingeAxis, hingeAngle, hingeAngleRate, 
	sliderAxis, sliderPosition, sliderPositionRate
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool joint_get_vector(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	ode::dJointID jointID = (ode::dJointID)JS_GetPrivate(cx, obj);
	RT_ASSERT(jointID != NULL, RT_ERROR_NOT_INITIALIZED);
	ode::dVector3 vector;
	switch(JSVAL_TO_INT(id)) {
		case ballAnchor:
			ode::dJointGetBallAnchor(jointID,vector);
			break;
		case ballAnchor2:
			ode::dJointGetBallAnchor2(jointID,vector);
			break;
		case hingeAnchor:
			ode::dJointGetHingeAnchor(jointID,vector);
			break;
		case hingeAxis:
			ode::dJointGetHingeAxis(jointID,vector);
			break;
		case sliderAxis:
			ode::dJointGetSliderAxis(jointID,vector);
			break;
	}
	FloatVectorToArray(cx, 3, vector, vp);
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool joint_set_vector(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	ode::dJointID jointID = (ode::dJointID)JS_GetPrivate( cx, obj );
	RT_ASSERT( jointID != NULL, RT_ERROR_NOT_INITIALIZED );
	ode::dVector3 vector;
	FloatArrayToVector(cx, 3, vp, vector);
	switch(JSVAL_TO_INT(id)) {
		case ballAnchor:
			ode::dJointSetBallAnchor( jointID, vector[0], vector[1], vector[2] );
			break;
		case hingeAnchor:
			ode::dJointSetHingeAnchor( jointID, vector[0], vector[1], vector[2] );
			break;
		case hingeAxis:
			ode::dJointSetHingeAxis( jointID, vector[0], vector[1], vector[2] );
			break;
		case sliderAxis:
			ode::dJointSetSliderAxis( jointID, vector[0], vector[1], vector[2] );
			break;
	}
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool joint_get_real(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	ode::dJointID jointID = (ode::dJointID)JS_GetPrivate(cx, obj);
	RT_ASSERT(jointID != NULL, RT_ERROR_NOT_INITIALIZED);
	ode::dReal real;
	switch(JSVAL_TO_INT(id)) {
		case hingeAngle:
			real = ode::dJointGetHingeAngle(jointID);
			break;
		case hingeAngleRate:
			real = ode::dJointGetHingeAngleRate(jointID);
			break;
		case sliderPosition:
			real = ode::dJointGetSliderPosition(jointID);
			break;
		case sliderPositionRate:
			real = ode::dJointGetSliderPositionRate(jointID);
			break;
	}
	JS_NewDoubleValue(cx, real, vp);
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool joint_set_real(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	ode::dJointID jointID = (ode::dJointID)JS_GetPrivate( cx, obj );
	RT_ASSERT( jointID != NULL, RT_ERROR_NOT_INITIALIZED );

	jsdouble real;
	JS_ValueToNumber(cx, *vp, &real);

	switch(JSVAL_TO_INT(id)) {

	}
	return JS_TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool jointBall_construct(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT_CONSTRUCTING(&jointBall_class);
	RT_ASSERT_ARGC(1);
	JSObject *worldObject;
	JS_ValueToObject(cx, argv[0], &worldObject);
	RT_ASSERT_CLASS(worldObject,&classWorld);
	ode::dWorldID worldID = (ode::dWorldID)JS_GetPrivate(cx,worldObject);
	RT_ASSERT_1(worldID != NULL, "%s object not initialized", classWorld.name );
//	JS_SetReservedSlot(cx, obj, 0, argv[0]);
	ode::dJointID jointID = ode::dJointCreateBall(worldID, 0); // The joint group ID is 0 to allocate the joint normally.
	JS_SetPrivate(cx, obj, jointID);
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSPropertySpec jointBall_PropertySpec[] = { // *name, tinyid, flags, getter, setter
	{ "anchor"   , ballAnchor, JSPROP_PERMANENT|JSPROP_SHARED, joint_get_vector, joint_set_vector },
	{ "anchor2"  , ballAnchor2, JSPROP_PERMANENT|JSPROP_SHARED|JSPROP_READONLY, joint_get_vector, NULL },
	{ 0 }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSClass jointBall_class = { "JointBall", JSCLASS_HAS_PRIVATE | JSCLASS_HAS_RESERVED_SLOTS(2),
	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool jointHinge_construct(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT_CONSTRUCTING(&jointHinge_class);
	RT_ASSERT_ARGC(1);
	JSObject *worldObject;
	JS_ValueToObject(cx, argv[0], &worldObject);
	RT_ASSERT_CLASS(worldObject,&classWorld);
	ode::dWorldID worldID = (ode::dWorldID)JS_GetPrivate(cx,worldObject);
	RT_ASSERT_1(worldID != NULL, "%s object not initialized", classWorld.name );
//	JS_SetReservedSlot(cx, obj, 0, argv[0]);
	ode::dJointID jointID = ode::dJointCreateHinge(worldID, 0); // The joint group ID is 0 to allocate the joint normally.
	JS_SetPrivate(cx, obj, jointID);
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSPropertySpec jointHinge_PropertySpec[] = { // *name, tinyid, flags, getter, setter
	{ "anchor"     , hingeAnchor    , JSPROP_PERMANENT | JSPROP_SHARED                   , joint_get_vector , joint_set_vector },
	{ "anchor2"    , hingeAnchor2   , JSPROP_PERMANENT | JSPROP_SHARED | JSPROP_READONLY , joint_get_vector , NULL },
	{ "axis"       , hingeAxis      , JSPROP_PERMANENT | JSPROP_SHARED                   , joint_get_vector , joint_set_vector },
	{ "angle"      , hingeAngle     , JSPROP_PERMANENT | JSPROP_SHARED | JSPROP_READONLY , joint_get_real   , NULL },
	{ "angleRate"  , hingeAngleRate , JSPROP_PERMANENT | JSPROP_SHARED | JSPROP_READONLY , joint_get_real   , NULL },
	{ 0 }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSClass jointHinge_class = { "JointHinge", JSCLASS_HAS_PRIVATE | JSCLASS_HAS_RESERVED_SLOTS(2),
	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool jointSlider_construct(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT_CONSTRUCTING(&jointSlider_class);
	RT_ASSERT_ARGC(1);
	JSObject *worldObject;
	JS_ValueToObject(cx, argv[0], &worldObject);
	RT_ASSERT_CLASS(worldObject,&classWorld);
	ode::dWorldID worldID = (ode::dWorldID)JS_GetPrivate(cx,worldObject);
	RT_ASSERT_1(worldID != NULL, "%s object not initialized", classWorld.name );
//	JS_SetReservedSlot(cx, obj, 0, argv[0]);
	ode::dJointID jointID = ode::dJointCreateSlider(worldID, 0); // The joint group ID is 0 to allocate the joint normally.
	JS_SetPrivate(cx, obj, jointID);
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSPropertySpec jointSlider_PropertySpec[] = { // *name, tinyid, flags, getter, setter
	{ "axis"          , sliderAxis         , JSPROP_PERMANENT | JSPROP_SHARED                   , joint_get_vector , joint_set_vector },
	{ "position"      , sliderPosition     , JSPROP_PERMANENT | JSPROP_SHARED | JSPROP_READONLY , joint_get_real   , NULL },
	{ "positionRate"  , sliderPositionRate , JSPROP_PERMANENT | JSPROP_SHARED | JSPROP_READONLY , joint_get_real   , NULL },
	{ 0 }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSClass jointSlider_class = { "JointSlider", JSCLASS_HAS_PRIVATE | JSCLASS_HAS_RESERVED_SLOTS(2),
	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool jointFixed_construct(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT_CONSTRUCTING(&jointFixed_class);
	RT_ASSERT_ARGC(1);
	JSObject *worldObject;
	JS_ValueToObject(cx, argv[0], &worldObject);
	RT_ASSERT_CLASS(worldObject,&classWorld);
	ode::dWorldID worldID = (ode::dWorldID)JS_GetPrivate(cx,worldObject);
	RT_ASSERT_1(worldID != NULL, "%s object not initialized", classWorld.name );
//	JS_SetReservedSlot(cx, obj, 0, argv[0]);
	ode::dJointID jointID = ode::dJointCreateFixed(worldID, 0); // The joint group ID is 0 to allocate the joint normally.
	JS_SetPrivate(cx, obj, jointID);
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool jointFixed_set(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT_CLASS(obj, &jointFixed_class);
	ode::dJointID jointID = (ode::dJointID)JS_GetPrivate(cx, obj);
	RT_ASSERT(jointID != NULL, RT_ERROR_NOT_INITIALIZED);
	ode::dJointSetFixed(jointID);
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSFunctionSpec jointFixed_FunctionSpec[] = { // *name, tinyid, flags, getter, setter
	{ "Set"    , jointFixed_set },
	{ 0 }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSClass jointFixed_class = { "JointFixed", JSCLASS_HAS_PRIVATE | JSCLASS_HAS_RESERVED_SLOTS(2),
	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSObject *jointInitClass( JSContext *cx, JSObject *obj ) {

	JSObject *joint = JS_InitClass( cx, obj, NULL, &joint_class, NULL, 0, joint_PropertySpec, joint_FunctionSpec, NULL, NULL );

	JS_InitClass( cx, obj, joint, &jointBall_class   , jointBall_construct   , 0, jointBall_PropertySpec   , NULL, NULL, NULL );
	JS_InitClass( cx, obj, joint, &jointHinge_class  , jointHinge_construct  , 0, jointHinge_PropertySpec  , NULL, NULL, NULL );
	JS_InitClass( cx, obj, joint, &jointSlider_class , jointSlider_construct , 0, jointSlider_PropertySpec , NULL, NULL, NULL );
	JS_InitClass( cx, obj, joint, &jointFixed_class  , jointFixed_construct  , 0, NULL                     , jointFixed_FunctionSpec, NULL, NULL );

	return joint;
}
