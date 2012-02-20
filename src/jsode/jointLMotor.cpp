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
$SVN_REVISION $Revision: 3533 $
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

	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

	JL_ASSERT_ARGC_RANGE(1,2);

	ode::dJointGroupID groupId;
	if ( JL_ARG_ISDEF(2) ) {
		
		JL_ASSERT_ARG_IS_OBJECT(2);
		JL_ASSERT_INSTANCE( JSVAL_TO_OBJECT( JL_ARG(2) ), JL_CLASS(JointGroup) );
		groupId = (ode::dJointGroupID)JL_GetPrivate(cx, JSVAL_TO_OBJECT(JL_ARG(2)));
	} else {

		groupId = 0;
	}

	ode::dWorldID worldId;
	JL_CHK( JL_JsvalToWorldID(cx, JL_ARG(1), &worldId) );
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
 $VOID $INAME( axisIndex, rel [, $TYPE vec3 axis ] )
  (TBD)
  If the axis vecor is ommited, the axis is disabled.
**/
DEFINE_FUNCTION( setAxis ) {
	
	JL_DEFINE_FUNCTION_OBJ;

	JL_ASSERT_ARGC_MIN( 3 );
	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(jointId); // (TBD) check if NULL is meaningful for joints !
	*JL_RVAL = JSVAL_VOID;

	int anum, rel;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &anum) );
	if ( !JL_ARG_ISDEF(3) ) {
		
		ode::dJointSetLMotorNumAxes(jointId, anum+1);
		return JS_TRUE;
	}
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &rel) );
	
	ode::dVector3 vector;
	uint32_t length;
	JL_CHK( JsvalToODERealVector(cx, JL_ARG(3), vector, 3, &length) );
	JL_ASSERT( length >= 3, E_ARG, E_NUM(3), E_TYPE, E_TY_NVECTOR(3) );

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

	REVISION(JL_SvnRevToInt("$Revision: 3533 $"))
	HAS_PROTOTYPE( Joint )
	HAS_CONSTRUCTOR
	HAS_FINALIZE
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(2) // body1, body2

	BEGIN_FUNCTION_SPEC
		FUNCTION_ARGC( setAxis, 3 )
	END_FUNCTION_SPEC

END_CLASS

