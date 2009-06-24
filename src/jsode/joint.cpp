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



void FinalizeJoint(JSContext *cx, JSObject *obj) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	if ( !jointId )
		return;
	JS_free(cx, ode::dJointGetFeedback(jointId)); // NULL is supported
	ode::dJointSetFeedback(jointId, NULL);
	ode::dJointSetData(jointId, NULL);
	if ( ode::dJointGetNumBodies(jointId) == 0 /*|| _odeFinalization*/ ) // geom is lost (limbo).
		ode::dJointDestroy(jointId);
}


JSBool ReconstructJoint( JSContext *cx, ode::dJointID jointId, JSObject **obj ) {

	JL_S_ASSERT( ode::dJointGetData(jointId) == NULL, "Invalid case (object not finalized)." );
	JL_S_ASSERT( jointId != NULL, "Invalid ode object." );

	switch( ode::dJointGetType(jointId) ) {
		case ode::dJointTypeBall:
			*obj = JS_NewObject(cx, classJointBall, NULL, NULL);
			break;
		case ode::dJointTypeHinge:
			*obj = JS_NewObject(cx, classJointHinge, NULL, NULL);
			break;
		case ode::dJointTypeSlider:
			*obj = JS_NewObject(cx, classJointSlider, NULL, NULL);
			break;
		case ode::dJointTypeUniversal:
			*obj = JS_NewObject(cx, classJointUniversal, NULL, NULL);
			break;
		case ode::dJointTypePiston:
			*obj = JS_NewObject(cx, classJointPiston, NULL, NULL);
			break;
		case ode::dJointTypeFixed:
			*obj = JS_NewObject(cx, classJointFixed, NULL, NULL);
			break;
		case ode::dJointTypeAMotor:
			*obj = JS_NewObject(cx, classJointAMotor, NULL, NULL);
			break;
		case ode::dJointTypeLMotor:
			*obj = JS_NewObject(cx, classJointLMotor, NULL, NULL);
			break;
		case ode::dJointTypePlane2D:
			*obj = JS_NewObject(cx, classJointPlane, NULL, NULL);
			break;
		default:
			JL_REPORT_ERROR("Unable to reconstruct the joint.");
	}

	ode::dJointSetData(jointId, *obj);
	JL_SetPrivate(cx, *obj, jointId);

	return JS_TRUE;
	JL_BAD;
}



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
	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( jointId );
	JS_free(cx, ode::dJointGetFeedback(jointId)); // NULL is supported
	JL_SetPrivate(cx, obj, NULL);
	ode::dJointDestroy(jointId);
	return JS_TRUE;
	JL_BAD;
}

/*
DEFINE_FUNCTION_FAST( GetBody ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE( jointId );

	JL_S_ASSERT_ARG_MIN(1);
	int index;
	JL_CHK( JsvalToInt(cx, JL_FARG(1), &index) );
	if ( index < 0 || index >= ode::dJointGetNumBodies(jointId) ) {
		
		*JL_FRVAL = JSVAL_VOID;
		return JS_TRUE;
	}

	ode::dBodyID bodyId;
	bodyId = ode::dJointGetBody(jointId, index);

	JSObject *jsBody = BodyToJSObject(bodyId);

	// (TBD)! construct a new wrapper if it has been finalized !

	*JL_FRVAL = jsBody ? OBJECT_TO_JSVAL( jsBody ) : JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}
*/


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

ALWAYS_INLINE void JointSetParam( ode::dJointID jointId, int parameter, ode::dReal value ) {

	switch( ode::dJointGetType(jointId) ) {
		case ode::dJointTypeBall:
			ode::dJointSetBallParam(jointId, parameter, value);
			return;
		case ode::dJointTypeHinge:
			ode::dJointSetHingeParam(jointId, parameter, value);
			return;
		case ode::dJointTypeSlider:
			ode::dJointSetSliderParam(jointId, parameter, value);
			return;
		case ode::dJointTypeUniversal:
			ode::dJointSetUniversalParam(jointId, parameter, value);
			return;
		case ode::dJointTypePiston:
			ode::dJointSetPistonParam(jointId, parameter, value);
			return;
		case ode::dJointTypePlane2D:
			ode::dJointSetPlane2DXParam(jointId, parameter, value); // (TBD) split them
			ode::dJointSetPlane2DYParam(jointId, parameter, value);
//			ode::dJointSetPlane2DAngleParam(jointId, parameter, value);
			return;
		case ode::dJointTypeLMotor:
			ode::dJointSetLMotorParam(jointId, parameter, value);
			return;
		case ode::dJointTypeAMotor:
			ode::dJointSetAMotorParam(jointId, parameter, value);
			return;
	}
}


ALWAYS_INLINE ode::dReal JointGetParam( ode::dJointID jointId, int parameter ) {

	switch( ode::dJointGetType(jointId) ) {
		case ode::dJointTypeBall:
			return ode::dJointGetBallParam(jointId, parameter);
		case ode::dJointTypeHinge:
			return ode::dJointGetHingeParam(jointId, parameter);
		case ode::dJointTypeSlider:
			return ode::dJointGetSliderParam(jointId, parameter);
		case ode::dJointTypeUniversal:
			return ode::dJointGetUniversalParam(jointId, parameter);
		case ode::dJointTypePiston:
			return ode::dJointGetPistonParam(jointId, parameter);
		case ode::dJointTypePlane2D:
			return 0; // (TBD)
		case ode::dJointTypeLMotor:
			return ode::dJointGetLMotorParam(jointId, parameter);
		case ode::dJointTypeAMotor:
			return ode::dJointGetAMotorParam(jointId, parameter);
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
DEFINE_PROPERTY_SETTER( body1 ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( jointId );
	ode::dBodyID bodyId;
	JL_CHK( JsvalToBody(cx, *vp, &bodyId) );
	ode::dJointAttach(jointId, bodyId, ode::dJointGetBody(jointId, 1));
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( body1 ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( jointId );
	ode::dBodyID bodyId = ode::dJointGetBody(jointId, 0);
	if ( bodyId )
		return BodyToJsval(cx, bodyId, vp);
	*vp = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE Body $INAME
  Set the second body of the joint.
**/
DEFINE_PROPERTY_SETTER( body2 ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( jointId );
	ode::dBodyID bodyId;
	JL_CHK( JsvalToBody(cx, *vp, &bodyId) );
	ode::dJointAttach(jointId, ode::dJointGetBody(jointId, 0), bodyId);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( body2 ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( jointId );
	ode::dBodyID bodyId = ode::dJointGetBody(jointId, 1);
	if ( bodyId )
		return BodyToJsval(cx, bodyId, vp);
	*vp = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME
  (TBD)
**/
DEFINE_PROPERTY( disabledSetter ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( jointId );
	bool disabled;
	JL_CHK( JsvalToBool(cx, *vp, &disabled) );
	if ( disabled )
		ode::dJointDisable(jointId);
	else
		ode::dJointEnable(jointId);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( disabledGetter ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( jointId );
	JL_CHK( BoolToJsval(cx, ode::dJointIsEnabled(jointId) == 0, vp) );
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
DEFINE_PROPERTY( useFeedbackSetter ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( jointId );
	bool b;
	JL_CHK( JsvalToBool(cx, *vp, &b) );
	ode::dJointFeedback *currentFeedback = ode::dJointGetFeedback(jointId);
	if ( !currentFeedback == !b ) // no changes
		return JS_TRUE;

	if ( b ) {

		ode::dJointFeedback *fb = (ode::dJointFeedback*)JS_malloc(cx, sizeof(ode::dJointFeedback));
		JL_CHK(fb);
		ode::dJointSetFeedback(jointId, fb);
	} else {

		ode::dJointSetFeedback(jointId, NULL);
		JS_free(cx, currentFeedback);
	}

	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY( useFeedbackGetter ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( jointId );
	JL_CHK( BoolToJsval(cx, ode::dJointGetFeedback(jointId) != NULL, vp) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE vec3 *body1Force*
  Is the current force vector that applies to the body1 if feedback is activated.

$TOC_MEMBER $INAME
 $TYPE vec3 *body1Torque*
  Is the current torque vector that applies to the body1 if feedback is activated.

$TOC_MEMBER $INAME
 $TYPE vec3 *body2Force*
  Is the current force vector that applies to the body2 if feedback is activated.

$TOC_MEMBER $INAME
 $TYPE vec3 *body2Torque*
  Is the current torque vector that applies to the body2 if feedback is activated.
**/
enum { body1Force, body1Torque, body2Force, body2Torque };

DEFINE_PROPERTY( feedbackVectorSetter ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( jointId );
	ode::dJointFeedback *feedback = ode::dJointGetFeedback(jointId);
	JL_S_ASSERT( feedback != NULL, "Feedback is disabled." );
	size_t length;
	switch(JSVAL_TO_INT(id)) {
		case body1Force:
			JL_CHK( JsvalToFloatVector(cx, *vp, feedback->f1, 3, &length) );
			break;
		case body1Torque:
			JL_CHK( JsvalToFloatVector(cx, *vp, feedback->t1, 3, &length) );
			break;
		case body2Force:
			JL_CHK( JsvalToFloatVector(cx, *vp, feedback->f2, 3, &length) );
			break;
		case body2Torque:
			JL_CHK( JsvalToFloatVector(cx, *vp, feedback->t2, 3, &length) );
			break;
	}
	JL_S_ASSERT( length >= 3, "Invalid array size." );
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY( feedbackVectorGetter ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( jointId );
	ode::dJointFeedback *feedback = ode::dJointGetFeedback(jointId);
	JL_S_ASSERT( feedback != NULL, "Feedback is disabled." );
	switch(JSVAL_TO_INT(id)) {
		case body1Force:
			JL_CHK( FloatVectorToJsval(cx, feedback->f1, 3, vp) );
			break;
		case body1Torque:
			JL_CHK( FloatVectorToJsval(cx, feedback->t1, 3, vp) );
			break;
		case body2Force:
			JL_CHK( FloatVectorToJsval(cx, feedback->f2, 3, vp) );
			break;
		case body2Torque:
			JL_CHK( FloatVectorToJsval(cx, feedback->t2, 3, vp) );
			break;
	}
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $REAL *loStop*

$TOC_MEMBER $INAME
 $REAL *hiStop*

$TOC_MEMBER $INAME
 $REAL *velocity*

$TOC_MEMBER $INAME
 $REAL *maxForce*

$TOC_MEMBER $INAME
 $REAL *fudgeFactor*

$TOC_MEMBER $INAME
 $REAL *bounce*

$TOC_MEMBER $INAME
 $REAL *CFM*

$TOC_MEMBER $INAME
 $REAL *stopERP*

$TOC_MEMBER $INAME
 $REAL *stopCFM*
**/
enum { 
	loStop = ode::dParamLoStop,
	hiStop = ode::dParamHiStop,
	velocity = ode::dParamVel,
	maxForce = ode::dParamFMax,
	fudgeFactor = ode::dParamFudgeFactor,
	bounce = ode::dParamBounce,
	CFM = ode::dParamCFM,
	stopERP = ode::dParamStopERP,
	stopCFM = ode::dParamStopCFM,

	loStop2 = ode::dParamLoStop,
	hiStop2 = ode::dParamHiStop,
	velocity2 = ode::dParamVel,
	maxForce2 = ode::dParamFMax,
	fudgeFactor2 = ode::dParamFudgeFactor,
	bounce2 = ode::dParamBounce,
	CFM2 = ode::dParamCFM,
	stopERP2 = ode::dParamStopERP,
	stopCFM2 = ode::dParamStopCFM,

	loStop3 = ode::dParamLoStop,
	hiStop3 = ode::dParamHiStop,
	velocity3 = ode::dParamVel,
	maxForce3 = ode::dParamFMax,
	fudgeFactor3 = ode::dParamFudgeFactor,
	bounce3 = ode::dParamBounce,
	CFM3 = ode::dParamCFM,
	stopERP3 = ode::dParamStopERP,
	stopCFM3 = ode::dParamStopCFM,
};


DEFINE_PROPERTY( jointParam1Setter ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(jointId);
	JL_S_ASSERT_NUMBER( *vp );
	JointSetParam(jointId, JSVAL_TO_INT(id), JSValToODEReal(cx, *vp));
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( jointParam1Getter ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(jointId);
	*vp = ODERealToJsval(cx, JointGetParam(jointId, JSVAL_TO_INT(id)));
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY( jointParam2Setter ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(jointId);
	JL_S_ASSERT_NUMBER( *vp );
	JointSetParam(jointId, JSVAL_TO_INT(id) + ode::dParamGroup2, JSValToODEReal(cx, *vp));
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( jointParam2Getter ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(jointId);
	*vp = ODERealToJsval(cx, JointGetParam(jointId, JSVAL_TO_INT(id) + ode::dParamGroup2));
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY( jointParam3Setter ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(jointId);
	JL_S_ASSERT_NUMBER( *vp );
	JointSetParam(jointId, JSVAL_TO_INT(id) + ode::dParamGroup3, JSValToODEReal(cx, *vp));
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( jointParam3Getter ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(jointId);
	*vp = ODERealToJsval(cx, JointGetParam(jointId, JSVAL_TO_INT(id) + ode::dParamGroup3));
	return JS_TRUE;
	JL_BAD;
}

CONFIGURE_CLASS
	REVISION(JL_SvnRevToInt("$Revision$"))

	BEGIN_FUNCTION_SPEC
		FUNCTION( Destroy )
//		FUNCTION( GetBody )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_STORE( body1 ) // store it to keep a reference (GC protection)
		PROPERTY_STORE( body2 ) // store it to keep a reference (GC protection)

		PROPERTY( disabled )
		PROPERTY( useFeedback )

		PROPERTY_SWITCH( body1Force , feedbackVector )
		PROPERTY_SWITCH( body1Torque, feedbackVector )
		PROPERTY_SWITCH( body2Force , feedbackVector )
		PROPERTY_SWITCH( body2Torque, feedbackVector )

		// group 1
		PROPERTY_SWITCH( loStop, jointParam1 )
		PROPERTY_SWITCH( hiStop, jointParam1 )
		PROPERTY_SWITCH( velocity, jointParam1 )
		PROPERTY_SWITCH( maxForce, jointParam1 )
		PROPERTY_SWITCH( fudgeFactor, jointParam1 )
		PROPERTY_SWITCH( bounce, jointParam1 )
		PROPERTY_SWITCH( CFM, jointParam1 )
		PROPERTY_SWITCH( stopERP, jointParam1 )
		PROPERTY_SWITCH( stopCFM, jointParam1 )

		// group 2
		PROPERTY_SWITCH( loStop2, jointParam2 )
		PROPERTY_SWITCH( hiStop2, jointParam2 )
		PROPERTY_SWITCH( velocity2, jointParam2 )
		PROPERTY_SWITCH( maxForce2, jointParam2 )
		PROPERTY_SWITCH( fudgeFactor2, jointParam2 )
		PROPERTY_SWITCH( bounce2, jointParam2 )
		PROPERTY_SWITCH( CFM2, jointParam2 )
		PROPERTY_SWITCH( stopERP2, jointParam2 )
		PROPERTY_SWITCH( stopCFM2, jointParam2 )

		// group 3
		PROPERTY_SWITCH( loStop3, jointParam3 )
		PROPERTY_SWITCH( hiStop3, jointParam3 )
		PROPERTY_SWITCH( velocity3, jointParam3 )
		PROPERTY_SWITCH( maxForce3, jointParam3 )
		PROPERTY_SWITCH( fudgeFactor3, jointParam3 )
		PROPERTY_SWITCH( bounce3, jointParam3 )
		PROPERTY_SWITCH( CFM3, jointParam3 )
		PROPERTY_SWITCH( stopERP3, jointParam3 )
		PROPERTY_SWITCH( stopCFM3, jointParam3 )

		END_PROPERTY_SPEC
END_CLASS
