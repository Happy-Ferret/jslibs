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
BEGIN_CLASS( JointAMotor )


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
	JL_S_ASSERT_THIS_CLASS();
	JL_S_ASSERT_ARG_RANGE(1,2);

	ode::dJointGroupID groupId;
	JL_S_ASSERT_OBJECT( JL_ARG(2) );
	JL_S_ASSERT_CLASS( JSVAL_TO_OBJECT( JL_ARG(2) ), JL_CLASS(JointGroup) );
	if ( JL_ARG_ISDEF(2) )
		groupId = (ode::dJointGroupID)JL_GetPrivate(cx, JSVAL_TO_OBJECT(JL_ARG(2)));
	else
		groupId = 0;

	ode::dWorldID worldId;
	JL_CHK( JsvalToWorldID( cx, JL_ARG(1), &worldId) );
	ode::dJointID jointId = ode::dJointCreateAMotor(worldId, groupId);
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
 $INAME()
  dJointAddAMotorTorques
**/
DEFINE_FUNCTION( AddTorque0 ) {

	JL_S_ASSERT_ARG_MIN( 1 );
	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(jointId); // (TBD) check if NULL is meaningful for joints !
	float torque;
	JL_CHK( JsvalToFloat(cx, JL_ARG(1), &torque) );
	ode::dJointAddAMotorTorques(jointId, torque,0,0);
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INAME()
  dJointAddAMotorTorques
**/
DEFINE_FUNCTION( AddTorque1 ) {

	JL_S_ASSERT_ARG_MIN( 1 );
	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(jointId); // (TBD) check if NULL is meaningful for joints !
	float torque;
	JL_CHK( JsvalToFloat(cx, JL_ARG(1), &torque) );
	ode::dJointAddAMotorTorques(jointId, 0,torque,0);
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INAME()
  dJointAddAMotorTorques
**/
DEFINE_FUNCTION( AddTorque2 ) {

	JL_S_ASSERT_ARG_MIN( 1 );
	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(jointId); // (TBD) check if NULL is meaningful for joints !
	float torque;
	JL_CHK( JsvalToFloat(cx, JL_ARG(1), &torque) );
	ode::dJointAddAMotorTorques(jointId, 0,0,torque);
	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $INAME()
  (TBD)
**/
DEFINE_FUNCTION( SetAxis ) {
	
	JL_S_ASSERT_ARG_MIN( 3 );
	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(jointId); // (TBD) check if NULL is meaningful for joints !
	int anum, rel;
	JsvalToInt(cx, JL_ARG(1), &anum);
	JsvalToInt(cx, JL_ARG(2), &rel);
	ode::dVector3 vector;
	uint32 length;
	JL_CHK( JsvalToFloatVector(cx, JL_ARG(3), vector, 3, &length) );
	JL_S_ASSERT( length >= 3, "Invalid array size." );

	if ( anum+1 > ode::dJointGetAMotorNumAxes(jointId) )
		ode::dJointSetAMotorNumAxes(jointId, anum+1);
	
	ode::dJointSetAMotorAxis(jointId, anum, rel, vector[0], vector[1], vector[2]);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INAME( axisIndex, angle )
  (TBD)
**/
DEFINE_FUNCTION( SetAngle ) {
	
	JL_S_ASSERT_ARG(2);
	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(jointId); // (TBD) check if NULL is meaningful for joints !
	
	int anum;
	float angle;
	JsvalToInt(cx, JL_ARG(1), &anum);
	JsvalToFloat(cx, JL_ARG(2), &angle);

	if ( anum+1 > ode::dJointGetAMotorNumAxes(jointId) )
		ode::dJointSetAMotorNumAxes(jointId, anum+1);

	ode::dJointSetAMotorAngle(jointId, anum, angle);
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
DEFINE_PROPERTY( eulerModeSetter ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(jointId); // (TBD) check if NULL is meaningful for joints !
	bool eulerMode;
	JL_CHK( JsvalToBool(cx, *vp, &eulerMode) );
	ode::dJointSetAMotorMode(jointId, eulerMode ?  ode::dAMotorEuler :  ode::dAMotorUser);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( eulerModeGetter ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(jointId);
	bool eulerMode = ode::dJointGetAMotorMode(jointId) == ode::dAMotorEuler;
	JL_CHK( BoolToJsval(cx, eulerMode, vp) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $REAL $INAME $READONLY
  In euler mode this is the corresponding euler angle rate of axis 1.
**/
DEFINE_PROPERTY( angle0Rate ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(jointId);
	return FloatToJsval(cx, ode::dJointGetAMotorAngleRate(jointId, 0), vp);
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $REAL $INAME $READONLY
  In euler mode this is the corresponding euler angle rate of axis 2.
**/
DEFINE_PROPERTY( angle1Rate ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(jointId);
	return FloatToJsval(cx, ode::dJointGetAMotorAngleRate(jointId, 1), vp);
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $REAL $INAME $READONLY
  In euler mode this is the corresponding euler angle rate of axis 3.
**/
DEFINE_PROPERTY( angle2Rate ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(jointId);
	return FloatToJsval(cx, ode::dJointGetAMotorAngleRate(jointId, 2), vp);
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_PROTOTYPE( prototypeJoint )
	HAS_CONSTRUCTOR
	HAS_FINALIZE
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(2) // body1, body2

	BEGIN_FUNCTION_SPEC
		FUNCTION_ARGC( SetAxis, 3 )
		FUNCTION_ARGC( SetAngle, 2 )
		FUNCTION_ARGC( AddTorque0, 1 )
		FUNCTION_ARGC( AddTorque1, 1 )
		FUNCTION_ARGC( AddTorque2, 1 )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY( eulerMode )
		PROPERTY_READ( angle0Rate )
		PROPERTY_READ( angle1Rate )
		PROPERTY_READ( angle2Rate )
	END_PROPERTY_SPEC

END_CLASS

