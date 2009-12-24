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
BEGIN_CLASS( JointGroup )


/**doc
$TOC_MEMBER $INAME
 $INAME( world, [ jointGroup ] )
**/
DEFINE_CONSTRUCTOR() {

	JL_S_ASSERT_CONSTRUCTING();
	JL_S_ASSERT_THIS_CLASS();
	JL_S_ASSERT_ARG(0);
	ode::dJointGroupID groupId = ode::dJointGroupCreate(0);
	JL_SetPrivate(cx, obj, groupId);
	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Methods ===
**/

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
**/
DEFINE_FUNCTION_FAST( Destroy ) {
	
	JSObject *obj = JL_FOBJ;
	ode::dJointGroupID groupId = (ode::dJointGroupID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(groupId);
	ode::dJointGroupDestroy(groupId);
	JL_SetPrivate(cx, obj, NULL);
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
**/
DEFINE_FUNCTION_FAST( Empty ) {

	ode::dJointGroupID groupId = (ode::dJointGroupID)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE(groupId);
	ode::dJointGroupEmpty(groupId);
	return JS_TRUE;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_CONSTRUCTOR
	HAS_PRIVATE

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST( Destroy )
		FUNCTION_FAST( Empty )
	END_FUNCTION_SPEC

END_CLASS
