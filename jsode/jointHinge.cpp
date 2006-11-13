#include "stdafx.h"
#include "world.h"
#include "joint.h"

BEGIN_CLASS

DEFINE_FUNCTION( ClassConstruct ) {

	RT_ASSERT_CONSTRUCTING(thisClass);
	RT_ASSERT_ARGC(1);
	ode::dWorldID worldId;
	if ( ValToWorldID( cx, argv[0], &worldId) == JS_FALSE )
		return JS_FALSE;
	ode::dJointID jointId = ode::dJointCreateHinge(worldId, 0); // The joint group ID is 0 to allocate the joint normally.
	JS_SetPrivate(cx, obj, jointId);
	return JS_TRUE;
}

DEFINE_PROPERTY( anchorSetter ) {

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(jointId); // [TBD] check if NULL is meaningful for joints !
	ode::dVector3 vector;
	FloatArrayToVector(cx, 3, vp, vector);
	ode::dJointSetHingeAnchor( jointId, vector[0], vector[1], vector[2] );
	return JS_TRUE;
}

DEFINE_PROPERTY( anchorGetter ) {

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(jointId);
	ode::dVector3 vector;
	ode::dJointGetHingeAnchor(jointId,vector);
	FloatVectorToArray(cx, 3, vector, vp);
	return JS_TRUE;
}

DEFINE_PROPERTY( anchor2 ) { // read only

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(jointId);
	ode::dVector3 vector;
	ode::dJointGetHingeAnchor2(jointId,vector);
	FloatVectorToArray(cx, 3, vector, vp);
	return JS_TRUE;
}

DEFINE_PROPERTY( axisSetter ) {

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(jointId); // [TBD] check if NULL is meaningful for joints !
	ode::dVector3 vector;
	FloatArrayToVector(cx, 3, vp, vector);
	ode::dJointSetHingeAxis( jointId, vector[0], vector[1], vector[2] );
	return JS_TRUE;
}

DEFINE_PROPERTY( axisGetter ) {

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(jointId);
	ode::dVector3 vector;
	ode::dJointGetHingeAxis(jointId,vector);
	FloatVectorToArray(cx, 3, vector, vp);
	return JS_TRUE;
}

DEFINE_PROPERTY( angle ) {

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(jointId);
	JS_NewDoubleValue(cx, ode::dJointGetHingeAngle(jointId), vp);
	return JS_TRUE;
}

DEFINE_PROPERTY( angleRate ) {

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(jointId);
	JS_NewDoubleValue(cx, ode::dJointGetHingeAngleRate(jointId), vp);
	return JS_TRUE;
}

BEGIN_PROPERTY_MAP
	READWRITE( anchor )
	READONLY( anchor2 )
	READWRITE( axis )
	READONLY( angle )
	READONLY( angleRate )
END_MAP

NO_FUNCTION_MAP
NO_STATIC_FUNCTION_MAP
NO_STATIC_PROPERTY_MAP
//NO_CLASS_CONSTRUCT
NO_OBJECT_CONSTRUCT
NO_FINALIZE
NO_CALL
PROTOTYPE( classObjectJoint )
NO_CONSTANT_MAP
NO_INITCLASSAUX

END_CLASS( JointHinge, HAS_PRIVATE, 2 );
