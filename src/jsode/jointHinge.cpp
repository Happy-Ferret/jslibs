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
BEGIN_CLASS( JointHinge )


DEFINE_FINALIZE() {

	FinalizeJoint(cx, obj);
}

/**doc
$TOC_MEMBER $INAME
 $INAME( world, [ jointGroup ] )
  TBD
**/
DEFINE_CONSTRUCTOR() {

	JL_S_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

	JL_S_ASSERT_ARG_RANGE(1,2);

	ode::dJointGroupID groupId;
	if ( JL_ARG_ISDEF(2) ) {

		JL_S_ASSERT_OBJECT( JL_ARG(2) );
		JL_S_ASSERT_CLASS( JSVAL_TO_OBJECT( JL_ARG(2) ), JL_CLASS(JointGroup) );
		groupId = (ode::dJointGroupID)JL_GetPrivate(cx, JSVAL_TO_OBJECT(JL_ARG(2)));
	} else {

		groupId = 0;
	}
	
	ode::dWorldID worldId;
	JL_CHK( JL_JsvalToWorldID( cx, JL_ARG(1), &worldId) );
	ode::dJointID jointId = ode::dJointCreateHinge(worldId, groupId); // The joint group ID is 0 to allocate the joint normally.
	ode::dJointSetData(jointId, obj);
	ode::dJointSetFeedback(jointId, NULL);
	JL_SetPrivate(cx, obj, jointId);
	return JS_TRUE;
	JL_BAD;
}


/**doc
=== Methods ===
**/

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( torque )
  TBD
**/
DEFINE_FUNCTION( AddTorque ) {

	JL_DEFINE_FUNCTION_OBJ;

	JL_S_ASSERT_ARG_MIN(1);
	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, JL_OBJ);
	JL_S_ASSERT_RESOURCE(jointId);
	jsdouble torque;
	JL_CHK( JL_JsvalToCVal(cx, JL_ARG(1), &torque) );
	ode::dJointAddHingeTorque(jointId, (ode::dReal)torque);

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
=== Properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $TYPE vec3 $INAME
  TBD
**/
DEFINE_PROPERTY( anchorSetter ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(jointId); // (TBD) check if NULL is meaningful for joints !
	ode::dVector3 vector;
//	FloatArrayToVector(cx, 3, vp, vector);
	uint32 length;
	JL_CHK( JL_JsvalToODERealVector(cx, *vp, vector, 3, &length) );
	JL_S_ASSERT( length >= 3, "Invalid array size." );
	ode::dJointSetHingeAnchor( jointId, vector[0], vector[1], vector[2] );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( anchorGetter ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(jointId);
	ode::dVector3 vector;
	ode::dJointGetHingeAnchor(jointId,vector);
	JL_CHK( ODERealVectorToJsval(cx, vector, 3, vp) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $TYPE vec3 $INAME $READONLY
  TBD
**/
DEFINE_PROPERTY( anchor2 ) { // read only

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(jointId);
	ode::dVector3 vector;
	ode::dJointGetHingeAnchor2(jointId,vector);
	JL_CHK( ODERealVectorToJsval(cx, vector, 3, vp) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $TYPE vec3 $INAME
  Get or set the axis of the joint.
**/
DEFINE_PROPERTY( axisSetter ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(jointId); // (TBD) check if NULL is meaningful for joints !
	ode::dVector3 vector;
//	FloatArrayToVector(cx, 3, vp, vector);
	uint32 length;
	JL_CHK( JL_JsvalToODERealVector(cx, *vp, vector, 3, &length) );
	JL_S_ASSERT( length >= 3, "Invalid array size." );
	ode::dJointSetHingeAxis( jointId, vector[0], vector[1], vector[2] );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( axisGetter ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(jointId);
	ode::dVector3 vector;
	ode::dJointGetHingeAxis(jointId,vector);
	//FloatVectorToArray(cx, 3, vector, vp);
	JL_CHK( ODERealVectorToJsval(cx, vector, 3, vp) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $REAL $INAME $READONLY
  Get the current angle.
**/
DEFINE_PROPERTY( angle ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(jointId);
	JL_CHK( JL_CValToJsval(cx, ode::dJointGetHingeAngle(jointId), vp) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $REAL $INAME $READONLY
  Get the current rotation speed.
**/
DEFINE_PROPERTY( angleRate ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(jointId);
	JL_CHK( JL_CValToJsval(cx, ode::dJointGetHingeAngleRate(jointId), vp) );
	return JS_TRUE;
	JL_BAD;
}

CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_PROTOTYPE( Joint )
	HAS_CONSTRUCTOR
	HAS_FINALIZE
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(2) // body1, body2

	BEGIN_FUNCTION_SPEC
		FUNCTION_ARGC( AddTorque, 1 )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY( anchor )
		PROPERTY_READ( anchor2 )
		PROPERTY( axis )
		PROPERTY_READ( angle )
		PROPERTY_READ( angleRate )
	END_PROPERTY_SPEC

END_CLASS
