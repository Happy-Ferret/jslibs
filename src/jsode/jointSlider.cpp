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
BEGIN_CLASS( JointSlider )

/**doc
=== Methods ===
**/

/**doc
 * *_Constructor_*( world )
  TBD
**/
DEFINE_CONSTRUCTOR() {

	RT_ASSERT_CONSTRUCTING(_class);
	RT_ASSERT_ARGC(1);
	ode::dWorldID worldId;
	if ( ValToWorldID( cx, argv[0], &worldId) == JS_FALSE )
		return JS_FALSE;
	ode::dJointID jointId = ode::dJointCreateSlider(worldId, 0); // The joint group ID is 0 to allocate the joint normally.
	JS_SetPrivate(cx, obj, jointId);
	return JS_TRUE;
}

/**doc
=== Properties ===
**/

/**doc
 * $RET vec3 *axis*
  TBD
**/
DEFINE_PROPERTY( axisSetter ) {

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(jointId); // (TBD) check if NULL is meaningful for joints !
	ode::dVector3 vector;
	FloatArrayToVector(cx, 3, vp, vector);
	ode::dJointSetSliderAxis( jointId, vector[0], vector[1], vector[2] );
	return JS_TRUE;
}

DEFINE_PROPERTY( axisGetter ) {

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(jointId);
	ode::dVector3 vector;
	ode::dJointGetSliderAxis(jointId,vector);
	FloatVectorToArray(cx, 3, vector, vp);
	return JS_TRUE;
}

/**doc
 * $REAL *position* $READONLY
  TBD
**/
DEFINE_PROPERTY( position ) {

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(jointId);
	JS_NewDoubleValue(cx, ode::dJointGetSliderPosition(jointId), vp);
	return JS_TRUE;
}

/**doc
 * $REAL *positionRate* $READONLY
  TBD
**/

DEFINE_PROPERTY( positionRate ) {

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(jointId);
	JS_NewDoubleValue(cx, ode::dJointGetSliderPositionRate(jointId), vp);
	return JS_TRUE;
}


CONFIGURE_CLASS

	HAS_CONSTRUCTOR

	BEGIN_PROPERTY_SPEC
		PROPERTY( axis )
		PROPERTY_READ( position )
		PROPERTY_READ( positionRate )
	END_PROPERTY_SPEC

	HAS_PROTOTYPE( prototypeJoint )
	HAS_PRIVATE
//	HAS_RESERVED_SLOTS(3)

END_CLASS
