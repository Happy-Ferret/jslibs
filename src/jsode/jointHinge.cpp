/* ***** BEGIN LICENSE BLOCK *****
 * Version: GNU GPL 2.0
 *
 * The contents of this file are subject to the
 * GNU General Public License Version 2.0; you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at
 * http://www.gnu.org/licenses/gpl.html
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 * ***** END LICENSE BLOCK ***** */

#include "stdafx.h"
#include "world.h"
#include "joint.h"

/**doc
$CLASS_HEADER Joint
**/
BEGIN_CLASS( JointHinge )

/**doc
=== Methods ===
**/

/**doc
 * $INAME( world )
  TBD
**/
DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();
	J_S_ASSERT_ARG_MIN(1);
	ode::dWorldID worldId;
	if ( ValToWorldID( cx, argv[0], &worldId) == JS_FALSE )
		return JS_FALSE;
	ode::dJointID jointId = ode::dJointCreateHinge(worldId, 0); // The joint group ID is 0 to allocate the joint normally.
	JS_SetPrivate(cx, obj, jointId);
	return JS_TRUE;
}


/**doc
 * $VOID $INAME( torque )
  TBD
**/
DEFINE_FUNCTION( AddTorque ) {

	J_S_ASSERT_ARG_MIN(1);
	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(jointId);
	jsdouble torque;
	JS_ValueToNumber(cx, argv[0], &torque);
	ode::dJointAddHingeTorque(jointId, torque);
	return JS_TRUE;
}


/**doc
=== Properties ===
**/

/**doc
 * $TYPE vec3 $INAME
  TBD
**/
DEFINE_PROPERTY( anchorSetter ) {

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(jointId); // (TBD) check if NULL is meaningful for joints !
	ode::dVector3 vector;
	FloatArrayToVector(cx, 3, vp, vector);
	ode::dJointSetHingeAnchor( jointId, vector[0], vector[1], vector[2] );
	return JS_TRUE;
}

DEFINE_PROPERTY( anchorGetter ) {

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(jointId);
	ode::dVector3 vector;
	ode::dJointGetHingeAnchor(jointId,vector);
	FloatVectorToArray(cx, 3, vector, vp);
	return JS_TRUE;
}

/**doc
 * $TYPE vec3 $INAME $READONLY
  TBD
**/
DEFINE_PROPERTY( anchor2 ) { // read only

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(jointId);
	ode::dVector3 vector;
	ode::dJointGetHingeAnchor2(jointId,vector);
	FloatVectorToArray(cx, 3, vector, vp);
	return JS_TRUE;
}

/**doc
 * $TYPE vec3 $INAME
  Get or set the axis of the joint.
**/
DEFINE_PROPERTY( axisSetter ) {

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(jointId); // (TBD) check if NULL is meaningful for joints !
	ode::dVector3 vector;
	FloatArrayToVector(cx, 3, vp, vector);
	ode::dJointSetHingeAxis( jointId, vector[0], vector[1], vector[2] );
	return JS_TRUE;
}

DEFINE_PROPERTY( axisGetter ) {

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(jointId);
	ode::dVector3 vector;
	ode::dJointGetHingeAxis(jointId,vector);
	FloatVectorToArray(cx, 3, vector, vp);
	return JS_TRUE;
}

/**doc
 * $REAL $INAME $READONLY
  Get the current angle.
**/
DEFINE_PROPERTY( angle ) {

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(jointId);
	JS_NewDoubleValue(cx, ode::dJointGetHingeAngle(jointId), vp);
	return JS_TRUE;
}

/**doc
 * $REAL $INAME $READONLY
  Get the current rotation speed.
**/
DEFINE_PROPERTY( angleRate ) {

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(jointId);
	JS_NewDoubleValue(cx, ode::dJointGetHingeAngleRate(jointId), vp);
	return JS_TRUE;
}

CONFIGURE_CLASS

	HAS_CONSTRUCTOR

	BEGIN_PROPERTY_SPEC
		PROPERTY( anchor )
		PROPERTY_READ( anchor2 )
		PROPERTY( axis )
		PROPERTY_READ( angle )
		PROPERTY_READ( angleRate )
	END_PROPERTY_SPEC

	HAS_PROTOTYPE( prototypeJoint )
	HAS_PRIVATE
//	HAS_RESERVED_SLOTS(3)

END_CLASS
