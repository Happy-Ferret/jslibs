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
BEGIN_CLASS( JointFixed )

/**doc
 * $INAME( world )
**/
DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();
	J_S_ASSERT_ARG_MIN(1);
	ode::dWorldID worldId;
	if ( ValToWorldID( cx, argv[0], &worldId) == JS_FALSE )
		return JS_FALSE;
	ode::dJointID jointId = ode::dJointCreateFixed(worldId, 0); // The joint group ID is 0 to allocate the joint normally.
	JS_SetPrivate(cx, obj, jointId);
	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Methods ===
**/

/**doc
 * $INAME()
  Set the current position of body1 and body2 as fixed.
**/
DEFINE_FUNCTION( Set ) {

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(jointId);
	ode::dJointSetFixed(jointId);
	return JS_TRUE;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(SvnRevToInt("$Revision$"))
	HAS_CONSTRUCTOR

	BEGIN_FUNCTION_SPEC
		FUNCTION( Set )
	END_FUNCTION_SPEC

	HAS_PROTOTYPE( prototypeJoint )
	HAS_PRIVATE
//	HAS_RESERVED_SLOTS(3)

END_CLASS
