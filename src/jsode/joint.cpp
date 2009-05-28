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
#include "joint.h"
#include "body.h"
#include "world.h"

#include "math.h"

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( Joint )

	// Api: dBodyID dJointGetBody (dJointID, int index);
	// Api: int dAreConnected (dBodyID b1, dBodyID b2);


	// (TBD) replace Attach with body1 and body2 setter

/*
DEFINE_PROPERTY( body1 ) {

	JS_GetReservedSlot(cx, obj, JOINT_SLOT_BODY1, vp);
	return JS_TRUE;
	JL_BAD;
}
DEFINE_PROPERTY( body2 ) {

	JS_GetReservedSlot(cx, obj, JOINT_SLOT_BODY2, vp);
	return JS_TRUE;
	JL_BAD;
}
*/

inline JSBool SetJoint( JSContext *cx, JSObject *obj, jsval *b1, jsval *b2 ) {

	ode::dJointID jointID = (ode::dJointID)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( jointID );

	if ( JSVAL_IS_VOID( *b1 ) || JSVAL_IS_VOID( *b2 ) )
		ode::dJointAttach(jointID, 0, 0); // detach it. The only way to attach it to the world environment is to use World.env

	ode::dBodyID bId1 = 0;
	ode::dBodyID bId2 = 0;

	if ( !JSVAL_IS_VOID( *b1 ) )
		JL_CHK( ValToBodyID(cx, *b1, &bId1) );

	if ( !JSVAL_IS_VOID( *b2 ) )
		JL_CHK( ValToBodyID(cx, *b2, &bId2) );

	ode::dJointAttach(jointID, bId1, bId2);
	return JS_TRUE;
	JL_BAD;
}


/**doc
=== Methods ===
**/

/**doc
$TOC_MEMBER $INAME
 $INAME()
  TBD
**/
DEFINE_FUNCTION( Destroy ) {

	JL_S_ASSERT( JL_InheritFrom(cx, obj, _class), J__ERRMSG_INVALID_CLASS );
	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( jointId );

	ode::dJointFeedback *currentFeedback = ode::dJointGetFeedback(jointId);
	if ( currentFeedback != NULL )
		free(currentFeedback);
	// remove references to bodies
	jsval val = JSVAL_VOID;
	JS_SetProperty(cx, obj, "body1", &val); // (TBD) find why to not use JS_DeleteProperty
	JS_SetProperty(cx, obj, "body2", &val);
	JL_SetPrivate(cx, obj, NULL);
	ode::dJointDestroy(jointId);
	return JS_TRUE;
	JL_BAD;
}

/*
DEFINE_FUNCTION( Attach ) {

	JL_S_ASSERT_ARG_MIN(2);
	JL_S_ASSERT( IsInstanceOf(cx, obj, thisClass), RT_ERROR_INVALID_CLASS );

	ode::dJointID jointID = (ode::dJointID)JL_GetPrivate( cx, obj );
	JL_S_ASSERT( jointID != NULL, RT_ERROR_NOT_INITIALIZED );

	JSObject *body1Object, *body2Object;

	JS_ValueToObject(cx, argv[0], &body1Object);
	JL_S_ASSERT_CLASS(body1Object, &classBody);
	JS_SetReservedSlot(cx, obj, JOINT_SLOT_BODY1, argv[0]);
	ode::dBodyID bodyID1 = (ode::dBodyID)JL_GetPrivate(cx, body1Object);
//	JL_S_ASSERT(bodyID != NULL, RT_ERROR_NOT_INITIALIZED);

	JS_ValueToObject(cx, argv[1], &body2Object);
	JL_S_ASSERT_CLASS(body2Object, &classBody);
	JS_SetReservedSlot(cx, obj, JOINT_SLOT_BODY2, argv[1]);
	ode::dBodyID bodyID2 = (ode::dBodyID)JL_GetPrivate(cx, body2Object);
//	JL_S_ASSERT(bodyID != NULL, RT_ERROR_NOT_INITIALIZED);

	ode::dJointAttach(jointID, bodyID1, bodyID2);
	return JS_TRUE;
	JL_BAD;
}
*/

// http://opende.sourceforge.net/wiki/index.php/Manual_%28Joint_Types_and_Functions%29

	//dJointTypeBall 	A ball-and-socket joint.
	//dJointTypeHinge 	A hinge joint.
	//dJointTypeSlider 	A slider joint.
	//dJointTypeContact 	A contact joint.
	//dJointTypeUniversal 	A universal joint.
	//dJointTypeHinge2 	A hinge-2 joint.
	//dJointTypeFixed 	A fixed joint.
	//dJointTypeAMotor 	An angular motor joint.
	//dJointTypePlane2D 	A Plane 2D joint.
	//dJointTypePR 	A Rotoide and Prismatic joint.

void JointSetParam( ode::dJointID jointId, int parameter, ode::dReal value ) {

	switch( ode::dJointGetType(jointId) ) {
		case ode::dJointTypeHinge:
			ode::dJointSetHingeParam(jointId, parameter, value);
			break;
		case ode::dJointTypeSlider:
			ode::dJointSetSliderParam(jointId, parameter, value);
			break;
		case ode::dJointTypePlane2D:
			ode::dJointSetPlane2DXParam(jointId, parameter, value); // (TBD) split them
			ode::dJointSetPlane2DYParam(jointId, parameter, value);
//			ode::dJointSetPlane2DAngleParam(jointId, parameter, value);
			break;
	}
}


ode::dReal JointGetParam( ode::dJointID jointId, int parameter ) {

	switch( ode::dJointGetType(jointId) ) {
		case ode::dJointTypeHinge:
			ode::dJointGetHingeParam(jointId, parameter);
			break;
		case ode::dJointTypeSlider:
			ode::dJointGetSliderParam(jointId, parameter);
			break;
	}
	return 0;
}

/**doc
=== Properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $TYPE Body $INAME
  Set the first body of the joint.
**/
DEFINE_PROPERTY( body1 ) {

	jsval b2;
	JS_GetProperty(cx, obj, "body2", &b2);
	JL_CHK( SetJoint(cx, obj, vp, &b2) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE Body $INAME
  Set the second body of the joint.
**/
DEFINE_PROPERTY( body2 ) {

	jsval b1;
	JS_GetProperty(cx, obj, "body1", &b1);
	JL_CHK( SetJoint(cx, obj, &b1, vp) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME
  Set to $TRUE activates the feedback, and $FALSE to desactivates it.
  $LF
  Using feedback will allows body1Force, body1Torque, body2Force and body2Torque to be used.
**/
DEFINE_PROPERTY( useFeedback ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( jointId );

	JSBool b;
	JS_ValueToBoolean(cx, *vp, &b);

	ode::dJointFeedback *currentFeedback = ode::dJointGetFeedback(jointId);

	if ( currentFeedback == NULL && b == JS_TRUE ) {

		ode::dJointFeedback *fb = (ode::dJointFeedback*)malloc(sizeof(ode::dJointFeedback));
		JL_S_ASSERT_ALLOC(fb);
		ode::dJointSetFeedback(jointId, fb);
	} else if ( currentFeedback != NULL && b == JS_FALSE ) {

		ode::dJointSetFeedback(jointId, NULL);
		free(currentFeedback);
	}

	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $TYPE vec3 *body1Force*
  Is the current force vector that applies to the body1 if feedback is activated.

 * $TYPE vec3 *body1Torque*
  Is the current torque vector that applies to the body1 if feedback is activated.

  * $TYPE vec3 *body2Force*
  Is the current force vector that applies to the body2 if feedback is activated.

 * $TYPE vec3 *body2Torque*
  Is the current torque vector that applies to the body2 if feedback is activated.
**/
enum { body1Force, body1Torque, body2Force, body2Torque };

DEFINE_PROPERTY( feedbackVectorSetter ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( jointId );
	ode::dJointFeedback *feedback = ode::dJointGetFeedback(jointId);
	JL_S_ASSERT( feedback != NULL, "Feedback is disabled." );
	size_t length;
	switch(JSVAL_TO_INT(id)) {
		case body1Force:
			//FloatArrayToVector(cx, 3, vp, feedback->f1);
			JL_CHK( JsvalToFloatVector(cx, *vp, feedback->f1, 3, &length) );
			JL_S_ASSERT( length == 3, "Invalid array size." );
			break;
		case body1Torque:
			//FloatArrayToVector(cx, 3, vp, feedback->t1);
			JL_CHK( JsvalToFloatVector(cx, *vp, feedback->t1, 3, &length) );
			JL_S_ASSERT( length == 3, "Invalid array size." );
			break;
		case body2Force:
			//FloatArrayToVector(cx, 3, vp, feedback->f2);
			JL_CHK( JsvalToFloatVector(cx, *vp, feedback->f2, 3, &length) );
			JL_S_ASSERT( length == 3, "Invalid array size." );
			break;
		case body2Torque:
			//FloatArrayToVector(cx, 3, vp, feedback->t2);
			JL_CHK( JsvalToFloatVector(cx, *vp, feedback->t2, 3, &length) );
			JL_S_ASSERT( length == 3, "Invalid array size." );
			break;
	}
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY( feedbackVectorGetter ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( jointId );
	ode::dJointFeedback *feedback = ode::dJointGetFeedback(jointId);
	JL_S_ASSERT( feedback != NULL, "Feedback is disabled." );
	switch(JSVAL_TO_INT(id)) {
		case body1Force:
			//FloatVectorToArray(cx, 3, feedback->f1, vp);
			JL_CHK( FloatVectorToJsval(cx, feedback->f1, 3, vp) );
			break;
		case body1Torque:
			//FloatVectorToArray(cx, 3, feedback->t1, vp);
			JL_CHK( FloatVectorToJsval(cx, feedback->t1, 3, vp) );
			break;
		case body2Force:
			//FloatVectorToArray(cx, 3, feedback->f2, vp);
			JL_CHK( FloatVectorToJsval(cx, feedback->f2, 3, vp) );
			break;
		case body2Torque:
			//FloatVectorToArray(cx, 3, feedback->t2, vp);
			JL_CHK( FloatVectorToJsval(cx, feedback->t2, 3, vp) );
			break;
	}
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $REAL *loStop*

 * $REAL *hiStop*

 * $REAL *bounce*

 * $REAL *CFM*

 * $REAL *stopERP*

 * $REAL *stopCFM*

 * $REAL *velocity*

 * $REAL *maxForce*
**/
enum { loStop, hiStop, bounce, CFM, stopERP, stopCFM, velocity, maxForce };

DEFINE_PROPERTY( jointParamSetter ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(jointId);
	JL_S_ASSERT_NUMBER( *vp );

	int parameter;
	switch(JSVAL_TO_INT(id)) {
		case loStop:
			parameter = ode::dParamLoStop;
			break;
		case hiStop:
			parameter = ode::dParamHiStop;
			break;
		case bounce:
			parameter = ode::dParamBounce;
			break;
		case CFM:
			parameter = ode::dParamCFM;
			break;
		case stopERP:
			parameter = ode::dParamStopERP;
			break;
		case stopCFM:
			parameter = ode::dParamStopCFM;
			break;
		case velocity:
			parameter = ode::dParamVel;
			break;
		case maxForce:
			parameter = ode::dParamFMax;
			break;
	}
	JointSetParam(jointId, parameter, JSValToODEReal(cx, *vp));
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY( jointParamGetter ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(jointId);
	int parameter;
	switch(JSVAL_TO_INT(id)) {
		case loStop:
			parameter = ode::dParamLoStop;
			break;
		case hiStop:
			parameter = ode::dParamHiStop;
			break;
		case bounce:
			parameter = ode::dParamBounce;
			break;
		case CFM:
			parameter = ode::dParamCFM;
			break;
		case stopERP:
			parameter = ode::dParamStopERP;
			break;
		case stopCFM:
			parameter = ode::dParamStopCFM;
			break;
		case velocity:
			parameter = ode::dParamVel;
			break;
		case maxForce:
			parameter = ode::dParamFMax;
			break;
	}
	JS_NewDoubleValue(cx, JointGetParam(jointId, parameter), vp);
	return JS_TRUE;
	JL_BAD;
}




CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
	BEGIN_FUNCTION_SPEC
		FUNCTION( Destroy )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_WRITE_STORE( body1 )
		PROPERTY_WRITE_STORE( body2 )

		PROPERTY_WRITE_STORE( useFeedback )

		PROPERTY_SWITCH( body1Force , feedbackVector )
		PROPERTY_SWITCH( body1Torque, feedbackVector )
		PROPERTY_SWITCH( body2Force , feedbackVector )
		PROPERTY_SWITCH( body2Torque, feedbackVector )

		PROPERTY_SWITCH( loStop  , jointParam )
		PROPERTY_SWITCH( hiStop  , jointParam )
		PROPERTY_SWITCH( bounce  , jointParam )
		PROPERTY_SWITCH( CFM     , jointParam )
		PROPERTY_SWITCH( stopERP , jointParam )
		PROPERTY_SWITCH( stopCFM , jointParam )
		PROPERTY_SWITCH( velocity, jointParam )
		PROPERTY_SWITCH( maxForce, jointParam )

/*
		PROPERTY( loStop )
		PROPERTY( hiStop )
		PROPERTY( bounce )

		PROPERTY( CFM )
		PROPERTY( stopERP )
		PROPERTY( stopCFM )

		PROPERTY( velocity )
		PROPERTY( maxForce )
*/
	END_PROPERTY_SPEC

END_CLASS;
