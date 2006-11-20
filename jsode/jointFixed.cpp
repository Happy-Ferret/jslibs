#include "stdafx.h"
#include "world.h"
#include "joint.h"

BEGIN_CLASS( JointFixed )

DEFINE_CONSTRUCTOR() {

	RT_ASSERT_CONSTRUCTING(_class);
	RT_ASSERT_ARGC(1);
	ode::dWorldID worldId;
	if ( ValToWorldID( cx, argv[0], &worldId) == JS_FALSE )
		return JS_FALSE;
	ode::dJointID jointId = ode::dJointCreateFixed(worldId, 0); // The joint group ID is 0 to allocate the joint normally.
	JS_SetPrivate(cx, obj, jointId);
	return JS_TRUE;
}

DEFINE_FUNCTION( Set ) {

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(jointId);
	ode::dJointSetFixed(jointId);
	return JS_TRUE;
}


CONFIGURE_CLASS

	HAS_CONSTRUCTOR

	BEGIN_FUNCTION_SPEC
		FUNCTION( Set )
	END_FUNCTION_SPEC

	HAS_PROTOTYPE( prototypeJoint )
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(2)

END_CLASS
