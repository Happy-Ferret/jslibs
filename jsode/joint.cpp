#include "stdafx.h"
#include "joint.h"
#include "body.h"
#include "world.h"

BEGIN_CLASS

DEFINE_PROPERTY( body1 ) {

	JS_GetReservedSlot(cx, obj, JOINT_SLOT_BODY1, vp);
	return JS_TRUE;
}

DEFINE_PROPERTY( body2 ) {

	JS_GetReservedSlot(cx, obj, JOINT_SLOT_BODY2, vp);
	return JS_TRUE;
}

DEFINE_FUNCTION( Destroy ) {

	RT_ASSERT( IsInstanceOf(cx, obj, thisClass), RT_ERROR_INVALID_CLASS );
	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( jointId );
	JS_SetReservedSlot(cx, obj, JOINT_SLOT_BODY1, JSVAL_VOID);
	JS_SetReservedSlot(cx, obj, JOINT_SLOT_BODY2, JSVAL_VOID);
	JS_SetPrivate(cx, obj, NULL); 
	ode::dJointDestroy(jointId);
	return JS_TRUE;
}

DEFINE_FUNCTION( Attach ) {

	RT_ASSERT_ARGC(2);
	RT_ASSERT( IsInstanceOf(cx, obj, thisClass), RT_ERROR_INVALID_CLASS );

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

BEGIN_FUNCTION_MAP
	FUNCTION( Attach )
	FUNCTION( Destroy )
END_MAP

BEGIN_PROPERTY_MAP
	READONLY( body1 )
	READONLY( body2 )
END_MAP

NO_STATIC_FUNCTION_MAP
NO_STATIC_PROPERTY_MAP
NO_CLASS_CONSTRUCT
NO_OBJECT_CONSTRUCT
NO_FINALIZE
NO_CALL
NO_PROTOTYPE
NO_CONSTANT_MAP
NO_INITCLASSAUX

END_CLASS( Joint, NO_PRIVATE, NO_RESERVED_SLOT );




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


/*

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
*/
