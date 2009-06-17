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
$SVN_REVISION $Revision: 2555 $
**/
BEGIN_CLASS( JointLMotor )


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
	JL_S_ASSERT_CLASS( JSVAL_TO_OBJECT( JL_ARG(2) ), classJointGroup );
	if ( JL_ARG_ISDEF(2) )
		groupId = (ode::dJointGroupID)JL_GetPrivate(cx, JSVAL_TO_OBJECT(JL_ARG(2)));
	else
		groupId = 0;

	ode::dWorldID worldId;
	JL_CHK( JsvalToWorldID(cx, JL_ARG(1), &worldId) );
	ode::dJointID jointId = ode::dJointCreateLMotor(worldId, groupId);
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
 $INAME( axisIndex, rel, $TYPE vec3 axis )
  (TBD)
**/
DEFINE_FUNCTION( SetAxis ) {
	
	JL_S_ASSERT_ARG_MIN( 3 );
	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(jointId); // (TBD) check if NULL is meaningful for joints !
	int anum, rel;
	JL_CHK( JsvalToInt(cx, JL_ARG(1), &anum) );
	JL_CHK( JsvalToInt(cx, JL_ARG(2), &rel) );
	ode::dVector3 vector;
	size_t length;
	JL_CHK( JsvalToFloatVector(cx, JL_ARG(3), vector, 3, &length) );
	JL_S_ASSERT( length == 3, "Invalid array size." );

	if ( anum+1 > ode::dJointGetLMotorNumAxes(jointId) )
		ode::dJointSetLMotorNumAxes(jointId, anum+1);
	
	ode::dJointSetLMotorAxis(jointId, anum, rel, vector[0], vector[1], vector[2]);
	return JS_TRUE;
	JL_BAD;
}


/**doc
=== Properties ===
**/

CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision: 2555 $"))
	HAS_PROTOTYPE( prototypeJoint )
	HAS_CONSTRUCTOR
	HAS_FINALIZE
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(2) // body1, body2

	BEGIN_FUNCTION_SPEC
		FUNCTION_ARGC( SetAxis, 3 )
	END_FUNCTION_SPEC

END_CLASS

