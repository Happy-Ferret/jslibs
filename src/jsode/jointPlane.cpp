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
BEGIN_CLASS( JointPlane )


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
	JL_CHK( JsvalToWorldID( cx, JL_ARG(1), &worldId) );
	ode::dJointID jointId = ode::dJointCreatePlane2D(worldId, groupId); // The joint group ID is 0 to allocate the joint normally.
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

**/
DEFINE_FUNCTION( ResetBody ) {

	// fc. http://opende.sourceforge.net/wiki/index.php/Manual_(Joint_Types_and_Functions)#Plane_2D
	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(jointId); // (TBD) check if NULL is meaningful for joints !

	ode::dBodyID bodyId = ode::dJointGetBody(jointId, 0);

	const ode::dReal *rot = ode::dBodyGetAngularVel(bodyId);
	const ode::dReal *quat_ptr;
	ode::dReal quat[4], quat_len;
	quat_ptr = ode::dBodyGetQuaternion(bodyId);
	quat[0] = quat_ptr[0];
	quat[1] = 0;
	quat[2] = 0; 
	quat[3] = quat_ptr[3]; 
	quat_len = sqrt(quat[0] * quat[0] + quat[3] * quat[3]);
	quat[0] /= quat_len;
	quat[3] /= quat_len;
	ode::dBodySetQuaternion(bodyId, quat);
	ode::dBodySetAngularVel(bodyId, 0, 0, rot[2]);

	return JS_TRUE;
	JL_BAD;
}



// dJointSetPlane2DXParam
// dJointSetPlane2DYParam
// dJointSetPlane2DAngleParam

//dParamFMax
/*
DEFINE_PROPERTY( x ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(jointId);

	ode::dJointSetPlane2DXParam( jointId, 

	return JS_TRUE;
	JL_BAD;
}
*/

CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_PROTOTYPE( prototypeJoint )
	HAS_CONSTRUCTOR
	HAS_FINALIZE
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(2) // body1, body2

	BEGIN_FUNCTION_SPEC
		FUNCTION_ARGC( ResetBody, 0 )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
//		PROPERTY_WRITE_STORE( x )
//		PROPERTY_WRITE_STORE( y )
//		PROPERTY_WRITE_STORE( angle )
	END_PROPERTY_SPEC

END_CLASS
