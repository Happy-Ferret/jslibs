#include "stdafx.h"
#include "world.h"
#include "joint.h"

BEGIN_CLASS( JointPlane )

DEFINE_CONSTRUCTOR() {

	RT_ASSERT_CONSTRUCTING(_class);
	RT_ASSERT_ARGC(1);
	ode::dWorldID worldId;
	if ( ValToWorldID( cx, argv[0], &worldId) == JS_FALSE )
		return JS_FALSE;
	ode::dJointID jointId = ode::dJointCreatePlane2D(worldId, 0); // The joint group ID is 0 to allocate the joint normally.
	JS_SetPrivate(cx, obj, jointId);
	return JS_TRUE;
}


CONFIGURE_CLASS
	
	HAS_CONSTRUCTOR

	HAS_PROTOTYPE( prototypeJoint )
	HAS_PRIVATE
END_CLASS
