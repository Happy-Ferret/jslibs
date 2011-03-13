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
BEGIN_CLASS( JointPiston )


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
		JL_ASSERT_CLASS( JSVAL_TO_OBJECT( JL_ARG(2) ), JL_CLASS(JointGroup) );
		groupId = (ode::dJointGroupID)JL_GetPrivate(cx, JSVAL_TO_OBJECT(JL_ARG(2)));
	} else {

		groupId = 0;
	}

	ode::dWorldID worldId;
	JL_CHK( JL_JsvalToWorldID( cx, JL_ARG(1), &worldId) );
	ode::dJointID jointId = ode::dJointCreatePiston(worldId, groupId); // The joint group ID is 0 to allocate the joint normally.
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
 $VOID $INAME( force )
  TBD
**/
DEFINE_FUNCTION( AddForce ) {

	JL_DEFINE_FUNCTION_OBJ;

	JL_ASSERT_ARGC_MIN(1);
	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(jointId);
	ode::dReal real;
	JL_CHK( JL_JsvalToODEReal(cx, JL_ARG(1), &real) );
	ode::dJointAddPistonForce(jointId, real);

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( x, y, z,  dx, dy, dz )
  TBD
**/
DEFINE_FUNCTION( AxisDelta ) {

	JL_DEFINE_FUNCTION_OBJ;

	JL_ASSERT_ARGC_MIN(1);
	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(jointId);
	float x,y,z,  dx, dy, dz;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(0), &x ) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &y ) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &z ) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &dx ) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(4), &dy ) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(5), &dz ) );
	ode::dJointSetPistonAxisDelta(jointId, x,y,z, dx, dy, dz);

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
**/
DEFINE_PROPERTY_SETTER( anchor ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(jointId); // (TBD) check if NULL is meaningful for joints !
	ode::dVector3 vector;
	//FloatArrayToVector(cx, 3, vp, vector);
	uint32 length;
	JL_CHK( JL_JsvalToODERealVector(cx, *vp, vector, 3, &length) );
	JL_ASSERT( length >= 3, E_VALUE, E_TYPE, E_TY_NARRAY(3) );
	ode::dJointSetPistonAnchor( jointId, vector[0], vector[1], vector[2] );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( anchor ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(jointId);
	ode::dVector3 vector;
	ode::dJointGetPistonAnchor(jointId,vector);
	JL_CHK( ODERealVectorToJsval(cx, vector, 3, vp) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE vec3 $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER( anchor2 ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(jointId);
	ode::dVector3 vector;
	ode::dJointGetPistonAnchor2(jointId,vector);
	JL_CHK( ODERealVectorToJsval(cx, vector, 3, vp) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE vec3 $INAME
  TBD
**/
DEFINE_PROPERTY_SETTER( axis ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(jointId); // (TBD) check if NULL is meaningful for joints !
	ode::dVector3 vector;
	uint32 length;
	JL_CHK( JL_JsvalToODERealVector(cx, *vp, vector, 3, &length) );
	JL_ASSERT( length >= 3, E_VALUE, E_TYPE, E_TY_NARRAY(3) );
	ode::dJointSetPistonAxis( jointId, vector[0], vector[1], vector[2] );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( axis ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(jointId);
	ode::dVector3 vector;
	ode::dJointGetPistonAxis(jointId,vector);
	JL_CHK( ODERealVectorToJsval(cx, vector, 3, vp) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $REAL $INAME $READONLY
  TBD
**/
DEFINE_PROPERTY_GETTER( position ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(jointId);
	JL_CHK( JL_NativeToJsval(cx, ode::dJointGetPistonPosition(jointId), vp) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $REAL $INAME $READONLY
  TBD
**/
DEFINE_PROPERTY_GETTER( positionRate ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(jointId);
	JL_CHK( JL_NativeToJsval(cx, ode::dJointGetPistonPositionRate(jointId), vp) );
	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $REAL $INAME $READONLY
  TBD
**/
DEFINE_PROPERTY_GETTER( angle ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(jointId);
	JL_CHK( JL_NativeToJsval(cx, ode::dJointGetPistonAngle(jointId), vp) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $REAL $INAME $READONLY
  TBD
**/
DEFINE_PROPERTY_GETTER( angleRate ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(jointId);
	JL_CHK( JL_NativeToJsval(cx, ode::dJointGetPistonAngleRate(jointId), vp) );
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
		FUNCTION_ARGC( AddForce, 1 )
		FUNCTION_ARGC( AxisDelta, 6 )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY( anchor )
		PROPERTY_READ( anchor2 )
		PROPERTY( axis )
		PROPERTY_READ( position )
		PROPERTY_READ( positionRate )
		PROPERTY_READ( angle )
		PROPERTY_READ( angleRate )
	END_PROPERTY_SPEC

END_CLASS
