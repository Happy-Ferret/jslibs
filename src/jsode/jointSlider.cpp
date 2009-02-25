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
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( JointSlider )

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
	ode::dJointID jointId = ode::dJointCreateSlider(worldId, 0); // The joint group ID is 0 to allocate the joint normally.
	JS_SetPrivate(cx, obj, jointId);
	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Properties ===
**/

/**doc
 * $TYPE vec3 $INAME
  TBD
**/
DEFINE_PROPERTY( axisSetter ) {

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(jointId); // (TBD) check if NULL is meaningful for joints !
	ode::dVector3 vector;
//	FloatArrayToVector(cx, 3, vp, vector);
	size_t length;
	J_CHK( JsvalToFloatVector(cx, *vp, vector, 3, &length) );
	J_S_ASSERT( length == 3, "Invalid array size." );
	ode::dJointSetSliderAxis( jointId, vector[0], vector[1], vector[2] );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( axisGetter ) {

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(jointId);
	ode::dVector3 vector;
	ode::dJointGetSliderAxis(jointId,vector);
	//FloatVectorToArray(cx, 3, vector, vp);
	J_CHK( FloatVectorToJsval(cx, vector, 3, vp) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
 * $REAL $INAME $READONLY
  TBD
**/
DEFINE_PROPERTY( position ) {

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(jointId);
	JS_NewDoubleValue(cx, ode::dJointGetSliderPosition(jointId), vp);
	return JS_TRUE;
	JL_BAD;
}

/**doc
 * $REAL $INAME $READONLY
  TBD
**/

DEFINE_PROPERTY( positionRate ) {

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(jointId);
	JS_NewDoubleValue(cx, ode::dJointGetSliderPositionRate(jointId), vp);
	return JS_TRUE;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(SvnRevToInt("$Revision$"))
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
