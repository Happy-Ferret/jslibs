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
BEGIN_CLASS( JointFixed )


DEFINE_FINALIZE() {

	FinalizeJoint(obj);
}

/**doc
$TOC_MEMBER $INAME
 $INAME( world, [ jointGroup ] )
**/
DEFINE_CONSTRUCTOR() {

	JL_ASSERT_ARGC_RANGE(1,2);
	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

	ode::dJointGroupID groupId;
	if ( JL_ARG_ISDEF(2) ) {
	
		JL_ASSERT_ARG_IS_OBJECT(2);
		JL_ASSERT_INSTANCE( JSVAL_TO_OBJECT( JL_ARG(2) ), JL_CLASS(JointGroup) );
		groupId = (ode::dJointGroupID)JL_GetPrivate(JSVAL_TO_OBJECT(JL_ARG(2)));
	} else {

		groupId = 0;
	}
	
	ode::dWorldID worldId;
	JL_CHK( JL_JsvalToWorldID( cx, JL_ARG(1), &worldId) );
	ode::dJointID jointId = ode::dJointCreateFixed(worldId, groupId); // The joint group ID is 0 to allocate the joint normally.
	ode::dJointSetData(jointId, obj);
	ode::dJointSetFeedback(jointId, NULL);
	
	JL_SetPrivate(obj, jointId);
	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Methods ===
**/

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Set the current position of body1 and body2 as fixed.
**/
DEFINE_FUNCTION( set ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(jointId);
	ode::dJointSetFixed(jointId);

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision: 3533 $"))
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(2) // body1, body2

	HAS_PROTOTYPE( Joint )
	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION( set )
	END_FUNCTION_SPEC

END_CLASS
