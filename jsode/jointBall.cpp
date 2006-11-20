#include "stdafx.h"
#include "world.h"
#include "joint.h"

BEGIN_CLASS( JointBall )

DEFINE_CONSTRUCTOR() {

	RT_ASSERT_CONSTRUCTING(_class);
	RT_ASSERT_ARGC(1);
	ode::dWorldID worldId;
	if ( ValToWorldID( cx, argv[0], &worldId) == JS_FALSE )
		return JS_FALSE;
	ode::dJointID jointId = ode::dJointCreateBall(worldId, 0); // The joint group ID is 0 to allocate the joint normally.
	JS_SetPrivate(cx, obj, jointId);
	return JS_TRUE;
}

DEFINE_PROPERTY( anchorSetter ) {

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(jointId); // [TBD] check if NULL is meaningful for joints !
	ode::dVector3 vector;
	FloatArrayToVector(cx, 3, vp, vector);
	ode::dJointSetBallAnchor( jointId, vector[0], vector[1], vector[2] );
	return JS_TRUE;
}

DEFINE_PROPERTY( anchorGetter ) {

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(jointId);
	ode::dVector3 vector;
	ode::dJointGetBallAnchor(jointId,vector);
	FloatVectorToArray(cx, 3, vector, vp);
	return JS_TRUE;
}

DEFINE_PROPERTY( anchor2 ) { // read only

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(jointId);
	ode::dVector3 vector;
	ode::dJointGetBallAnchor2(jointId,vector);
	FloatVectorToArray(cx, 3, vector, vp);
	return JS_TRUE;
}

CONFIGURE_CLASS

	HAS_CONSTRUCTOR

	BEGIN_PROPERTY_SPEC
		PROPERTY( anchor )
		PROPERTY_READ( anchor2 )
	END_PROPERTY_SPEC

	HAS_PROTOTYPE( prototypeJoint )
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(2)

END_CLASS
