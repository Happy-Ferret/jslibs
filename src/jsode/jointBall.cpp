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
BEGIN_CLASS( JointBall )

DEFINE_FINALIZE() {

	FinalizeJoint(cx, obj);
}

/**doc
$TOC_MEMBER $INAME
 $INAME( world, [ jointGroup ] )
**/
DEFINE_CONSTRUCTOR() {

	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

	JL_ASSERT_ARGC_RANGE(1,2);

	ode::dJointGroupID groupId;
	if ( JL_ARG_ISDEF(2) ) {
		
		JL_ASSERT_ARG_IS_OBJECT(2);
		JL_ASSERT_CLASS( JSVAL_TO_OBJECT( JL_ARG(2) ), JL_CLASS(JointGroup) );
		groupId = (ode::dJointGroupID)JL_GetPrivate(cx, JSVAL_TO_OBJECT(JL_ARG(2)));
	} else {
		
		groupId = 0;
	}
	
	ode::dWorldID worldId;
	JL_CHK( JL_JsvalToWorldID( cx, JL_ARG(1), &worldId) );
	ode::dJointID jointId = ode::dJointCreateBall(worldId, groupId); // The joint group ID is 0 to allocate the joint normally.
	ode::dJointSetData(jointId, obj);
	ode::dJointSetFeedback(jointId, NULL);
	JL_SetPrivate(cx, obj, jointId);
	return JS_TRUE;
	JL_BAD;
}


/**doc
=== Properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $TYPE vec3 $INAME
**/
DEFINE_PROPERTY_SETTER( anchor ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(jointId); // (TBD) check if NULL is meaningful for joints !
	ode::dVector3 vector;
	//FloatArrayToVector(cx, 3, vp, vector);
	uint32 length;
	JL_CHK( JL_JsvalToODERealVector(cx, *vp, vector, 3, &length) );
	JL_ASSERT( length >= 3, E_VALUE, E_TYPE, E_TY_NARRAY(3) );
	ode::dJointSetBallAnchor( jointId, vector[0], vector[1], vector[2] );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( anchor ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(jointId);
	ode::dVector3 vector;
	ode::dJointGetBallAnchor(jointId,vector);
	JL_CHK( ODERealVectorToJsval(cx, vector, 3, vp) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE vec3 $INAME
**/
DEFINE_PROPERTY_SETTER( anchor2 ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(jointId); // (TBD) check if NULL is meaningful for joints !
	ode::dVector3 vector;
	//FloatArrayToVector(cx, 3, vp, vector);
	uint32 length;
	JL_CHK( JL_JsvalToODERealVector(cx, *vp, vector, 3, &length) );
	JL_ASSERT( length >= 3, E_VALUE, E_TYPE, E_TY_NARRAY(3) );
	ode::dJointSetBallAnchor2( jointId, vector[0], vector[1], vector[2] );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( anchor2 ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(jointId);
	ode::dVector3 vector;
	ode::dJointGetBallAnchor2(jointId,vector);
	JL_CHK( ODERealVectorToJsval(cx, vector, 3, vp) );
	return JS_TRUE;
	JL_BAD;
}


/** doc
$TOC_MEMBER $INAME
 $TYPE vec3 $INAME $READONLY
**/
/*
DEFINE_PROPERTY( anchor2 ) { // read only

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(jointId);
	ode::dVector3 vector;
	ode::dJointGetBallAnchor2(jointId,vector);
	//FloatVectorToArray(cx, 3, vector, vp);
	JL_CHK( FloatVectorToJsval(cx, vector, 3, vp) );
	return JS_TRUE;
	JL_BAD;
}
*/

CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_PROTOTYPE( Joint )
	HAS_CONSTRUCTOR
	HAS_FINALIZE
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(2) // body1, body2

	BEGIN_PROPERTY_SPEC
		PROPERTY( anchor )
		PROPERTY( anchor2 )
//		PROPERTY_READ( anchor2 )
	END_PROPERTY_SPEC

END_CLASS
