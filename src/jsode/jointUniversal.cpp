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
$SVN_REVISION $Revision: 2620 $
**/
BEGIN_CLASS( JointUniversal )

DEFINE_FINALIZE() {

	FinalizeJoint(cx, obj);
}

/**doc
$TOC_MEMBER $INAME
 $INAME( world, [ jointGroup ] )
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
	ode::dJointID jointId = ode::dJointCreateUniversal(worldId, groupId); // The joint group ID is 0 to allocate the joint normally.
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
DEFINE_PROPERTY( anchorSetter ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(jointId); // (TBD) check if NULL is meaningful for joints !
	ode::dVector3 vector;
	//FloatArrayToVector(cx, 3, vp, vector);
	size_t length;
	JL_CHK( JsvalToFloatVector(cx, *vp, vector, 3, &length) );
	JL_S_ASSERT( length >= 3, "Invalid array size." );
	ode::dJointSetUniversalAnchor( jointId, vector[0], vector[1], vector[2] );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( anchorGetter ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(jointId);
	ode::dVector3 vector;
	ode::dJointGetUniversalAnchor(jointId,vector);
	//FloatVectorToArray(cx, 3, vector, vp);
	JL_CHK( FloatVectorToJsval(cx, vector, 3, vp) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE vec3 $INAME $READONLY
**/
DEFINE_PROPERTY( anchor2 ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(jointId);
	ode::dVector3 vector;
	ode::dJointGetUniversalAnchor2(jointId,vector);
	JL_CHK( FloatVectorToJsval(cx, vector, 3, vp) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE vec3 $INAME
  Get or set the axis of the joint.
**/
DEFINE_PROPERTY( axis1Setter ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(jointId); // (TBD) check if NULL is meaningful for joints !
	ode::dVector3 vector;
	size_t length;
	JL_CHK( JsvalToFloatVector(cx, *vp, vector, 3, &length) );
	JL_S_ASSERT( length >= 3, "Invalid array size." );
	ode::dJointSetUniversalAxis1( jointId, vector[0], vector[1], vector[2] );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( axis1Getter ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(jointId);
	ode::dVector3 vector;
	ode::dJointGetUniversalAxis1(jointId,vector);
	JL_CHK( FloatVectorToJsval(cx, vector, 3, vp) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE vec3 $INAME
  Get or set the axis of the joint.
**/
DEFINE_PROPERTY( axis2Setter ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(jointId); // (TBD) check if NULL is meaningful for joints !
	ode::dVector3 vector;
	size_t length;
	JL_CHK( JsvalToFloatVector(cx, *vp, vector, 3, &length) );
	JL_S_ASSERT( length >= 3, "Invalid array size." );
	ode::dJointSetUniversalAxis2( jointId, vector[0], vector[1], vector[2] );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( axis2Getter ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(jointId);
	ode::dVector3 vector;
	ode::dJointGetUniversalAxis2(jointId,vector);
	JL_CHK( FloatVectorToJsval(cx, vector, 3, vp) );
	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $REAL $INAME $READONLY
**/
DEFINE_PROPERTY( angle1 ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(jointId);
	return FloatToJsval(cx, ode::dJointGetUniversalAngle1(jointId), vp);
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $REAL $INAME $READONLY
**/
DEFINE_PROPERTY( angle2 ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(jointId);
	return FloatToJsval(cx, ode::dJointGetUniversalAngle2(jointId), vp);
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $REAL $INAME $READONLY
**/
DEFINE_PROPERTY( angle1Rate ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(jointId);
	return FloatToJsval(cx, ode::dJointGetUniversalAngle1Rate(jointId), vp);
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $REAL $INAME $READONLY
**/
DEFINE_PROPERTY( angle2Rate ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(jointId);
	return FloatToJsval(cx, ode::dJointGetUniversalAngle2Rate(jointId), vp);
	JL_BAD;
}

CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision: 2620 $"))
	HAS_PROTOTYPE( prototypeJoint )
	HAS_CONSTRUCTOR
	HAS_FINALIZE
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(2) // body1, body2

	BEGIN_PROPERTY_SPEC
		PROPERTY( anchor )
		PROPERTY_READ( anchor2 )
		PROPERTY( axis1 )
		PROPERTY( axis2 )
		PROPERTY_READ( angle1 )
		PROPERTY_READ( angle2 )
		PROPERTY_READ( angle1Rate )
		PROPERTY_READ( angle2Rate )
	END_PROPERTY_SPEC

END_CLASS
