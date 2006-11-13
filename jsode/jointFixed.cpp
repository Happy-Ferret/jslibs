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

BEGIN_FUNCTION_MAP
	FUNCTION( Set )
END_MAP

NO_PROPERTY_MAP
NO_STATIC_FUNCTION_MAP
NO_STATIC_PROPERTY_MAP
//NO_CLASS_CONSTRUCT
NO_OBJECT_CONSTRUCT
NO_FINALIZE
NO_CALL
PROTOTYPE( classObjectJoint )
NO_CONSTANT_MAP
NO_INITCLASSAUX

END_CLASS( JointFixed, HAS_PRIVATE, 2 );
