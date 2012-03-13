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


void FinalizeJoint(JSContext *cx, JSObject *obj) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	if ( !jointId )
		return;
	JS_free(cx, ode::dJointGetFeedback(jointId)); // NULL is supported
	ode::dJointSetFeedback(jointId, NULL);
	ode::dJointSetData(jointId, NULL);
	if ( ode::dJointGetNumBodies(jointId) == 0 ) // joint is lost (limbo).
		ode::dJointDestroy(jointId);
}


JSBool ReconstructJoint( JSContext *cx, ode::dJointID jointId, JSObject **obj ) { // (TBD) JSObject** = Conservative Stack Scanning issue ?

	JL_ASSERT( jointId != NULL && ode::dJointGetData(jointId) == NULL, E_MODULE, E_INTERNAL, E_SEP, E_STR(JL_CLASS_NAME(Joint)), E_STATE );

	switch( ode::dJointGetType(jointId) ) {
		case ode::dJointTypeBall:
			*obj = JL_NewObjectWithGivenProto(cx, JL_CLASS(JointBall), JL_PROTOTYPE(cx, JointBall), NULL);
			break;
		case ode::dJointTypeHinge:
			*obj = JL_NewObjectWithGivenProto(cx, JL_CLASS(JointHinge), JL_PROTOTYPE(cx, JointHinge), NULL);
			break;
		case ode::dJointTypeSlider:
			*obj = JL_NewObjectWithGivenProto(cx, JL_CLASS(JointSlider), JL_PROTOTYPE(cx, JointSlider), NULL);
			break;
		case ode::dJointTypeUniversal:
			*obj = JL_NewObjectWithGivenProto(cx, JL_CLASS(JointUniversal), JL_PROTOTYPE(cx, JointUniversal), NULL);
			break;
		case ode::dJointTypePiston:
			*obj = JL_NewObjectWithGivenProto(cx, JL_CLASS(JointPiston), JL_PROTOTYPE(cx, JointPiston), NULL);
			break;
		case ode::dJointTypeFixed:
			*obj = JL_NewObjectWithGivenProto(cx, JL_CLASS(JointFixed), JL_PROTOTYPE(cx, JointFixed), NULL);
			break;
		case ode::dJointTypeAMotor:
			*obj = JL_NewObjectWithGivenProto(cx, JL_CLASS(JointAMotor), JL_PROTOTYPE(cx, JointAMotor), NULL);
			break;
		case ode::dJointTypeLMotor:
			*obj = JL_NewObjectWithGivenProto(cx, JL_CLASS(JointLMotor), JL_PROTOTYPE(cx, JointLMotor), NULL);
			break;
		case ode::dJointTypePlane2D:
			*obj = JL_NewObjectWithGivenProto(cx, JL_CLASS(JointPlane), JL_PROTOTYPE(cx, JointPlane), NULL);
			break;
		default:
			ASSERT(false);
	}

	ode::dJointSetData(jointId, *obj);
	JL_SetPrivate(cx, *obj, jointId);

	return JS_TRUE;
	JL_BAD;
}



/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 3533 $
**/
BEGIN_CLASS( Joint )


	// Api: dBodyID dJointGetBody (dJointID, int index);
	// Api: int dAreConnected (dBodyID b1, dBodyID b2);


	// (TBD) replace Attach with body1 and body2 setter

/*
DEFINE_PROPERTY( body1 ) {

	JL_GetReservedSlot(cx, obj, JOINT_SLOT_BODY1, vp);
	return JS_TRUE;
	JL_BAD;
}
DEFINE_PROPERTY( body2 ) {

	JL_GetReservedSlot(cx, obj, JOINT_SLOT_BODY2, vp);
	return JS_TRUE;
	JL_BAD;
}
*/


/**doc
=== Methods ===
**/

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  TBD
**/
DEFINE_FUNCTION( destroy ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INHERITANCE();

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( jointId );
	JS_free(cx, ode::dJointGetFeedback(jointId)); // NULL is supported
	JL_SetPrivate(cx, obj, NULL);
	ode::dJointDestroy(jointId);
	
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

/*
DEFINE_FUNCTION( getBody ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( jointId );

	JL_ASSERT_ARGC_MIN(1);
	int index;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &index) );
	if ( index < 0 || index >= ode::dJointGetNumBodies(jointId) ) {
		
		*JL_RVAL = JSVAL_VOID;
		return JS_TRUE;
	}

	ode::dBodyID bodyId;
	bodyId = ode::dJointGetBody(jointId, index);

	JSObject *jsBody = BodyToJSObject(bodyId);

	// (TBD)! construct a new wrapper if it has been finalized !

	*JL_RVAL = jsBody ? OBJECT_TO_JSVAL( jsBody ) : JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}
*/


/*
DEFINE_FUNCTION( attach ) {

	JL_ASSERT_ARGC_MIN(2);
	JL_ASSERT( IsInstanceOf(cx, obj, thisClass), RT_ERROR_INVALID_CLASS );

	ode::dJointID jointID = (ode::dJointID)JL_GetPrivate( cx, obj );
	JL_ASSERT( jointID != NULL, RT_ERROR_NOT_INITIALIZED );

	JSObject *body1Object, *body2Object;

	JS_ValueToObject(cx, argv[0], &body1Object);
	JL_ASSERT_INSTANCE(body1Object, &classBody);
	JL_SetReservedSlot(cx, obj, JOINT_SLOT_BODY1, argv[0]);
	ode::dBodyID bodyID1 = (ode::dBodyID)JL_GetPrivate(cx, body1Object);
//	JL_ASSERT(bodyID != NULL, RT_ERROR_NOT_INITIALIZED);

	JS_ValueToObject(cx, argv[1], &body2Object);
	JL_ASSERT_INSTANCE(body2Object, &classBody);
	JL_SetReservedSlot(cx, obj, JOINT_SLOT_BODY2, argv[1]);
	ode::dBodyID bodyID2 = (ode::dBodyID)JL_GetPrivate(cx, body2Object);
//	JL_ASSERT(bodyID != NULL, RT_ERROR_NOT_INITIALIZED);

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
		case ode::dJointTypePlane2D: // (TBD)
			ode::dJointSetPlane2DXParam(jointId, parameter, value); // (TBD) split them ? see http://opende.sourceforge.net/wiki/index.php/HOWTO_constrain_objects_to_2d
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

	JL_ASSERT_THIS_INHERITANCE();

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( jointId );
	ode::dBodyID bodyId;
	JL_CHK( JL_JsvalToBody(cx, *vp, &bodyId) );
	ode::dJointAttach(jointId, bodyId, ode::dJointGetBody(jointId, 1));
	return JL_StoreProperty(cx, obj, id, vp, false);
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( body1 ) {

	JL_ASSERT_THIS_INHERITANCE();

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( jointId );
	ode::dBodyID bodyId = ode::dJointGetBody(jointId, 0);
	if ( bodyId )
		BodyToJsval(cx, bodyId, vp);
	else
		*vp = JSVAL_VOID;
	return JL_StoreProperty(cx, obj, id, vp, false);
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE Body $INAME
  Set the second body of the joint.
**/
DEFINE_PROPERTY_SETTER( body2 ) {

	JL_ASSERT_THIS_INHERITANCE();

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate( cx, obj );
	JL_ASSERT_THIS_OBJECT_STATE( jointId );
	ode::dBodyID bodyId;
	JL_CHK( JL_JsvalToBody(cx, *vp, &bodyId) );
	ode::dJointAttach(jointId, ode::dJointGetBody(jointId, 0), bodyId);
	return JL_StoreProperty(cx, obj, id, vp, false);
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( body2 ) {

	JL_ASSERT_THIS_INHERITANCE();

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate( cx, obj );
	JL_ASSERT_THIS_OBJECT_STATE( jointId );
	ode::dBodyID bodyId = ode::dJointGetBody(jointId, 1);
	if ( bodyId )
		BodyToJsval(cx, bodyId, vp);
	else
		*vp = JSVAL_VOID;
	return JL_StoreProperty(cx, obj, id, vp, false);
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME
  (TBD)
**/
DEFINE_PROPERTY_SETTER( disabled ) {

	JL_ASSERT_THIS_INHERITANCE();

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate( cx, obj );
	JL_ASSERT_THIS_OBJECT_STATE( jointId );
	bool disabled;
	JL_CHK( JL_JsvalToNative(cx, *vp, &disabled) );
	if ( disabled )
		ode::dJointDisable(jointId);
	else
		ode::dJointEnable(jointId);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( disabled ) {

	JL_ASSERT_THIS_INHERITANCE();

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate( cx, obj );
	JL_ASSERT_THIS_OBJECT_STATE( jointId );
	JL_CHK( JL_NativeToJsval(cx, ode::dJointIsEnabled(jointId) == 0, vp) );
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
DEFINE_PROPERTY_SETTER( useFeedback ) {

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate( cx, obj );
	JL_ASSERT_THIS_OBJECT_STATE( jointId );
	bool b;
	JL_CHK( JL_JsvalToNative(cx, *vp, &b) );
	ode::dJointFeedback *currentFeedback = ode::dJointGetFeedback(jointId);
	if ( !currentFeedback == !b ) // no changes
		return JS_TRUE;

	if ( b ) {

		ode::dJointFeedback *fb = (ode::dJointFeedback*)JS_malloc(cx, sizeof(ode::dJointFeedback));
		JL_CHK(fb);
		memset(fb, 0, sizeof(ode::dJointFeedback));
		ode::dJointSetFeedback(jointId, fb);
	} else {

		ode::dJointSetFeedback(jointId, NULL);
		JS_free(cx, currentFeedback);
	}

	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( useFeedback ) {

	JL_ASSERT_THIS_INHERITANCE();

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( jointId );
	JL_CHK( JL_NativeToJsval(cx, ode::dJointGetFeedback(jointId) != NULL, vp) );
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

DEFINE_PROPERTY_SETTER( feedbackVector ) {

	JL_ASSERT_THIS_INHERITANCE();

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( jointId );
	ode::dJointFeedback *feedback = ode::dJointGetFeedback(jointId);
	JL_ASSERT( feedback != NULL, E_STR("feedback"), E_DISABLED );
	uint32_t length;
	IFDEBUG( length = 0 ); // avoid "potentially uninitialized local variable" warning

	ode::dReal *vector;
	switch ( JSID_TO_INT(id) ) {
		case body1Force:
			//JL_CHK( JsvalToODERealVector(cx, *vp, feedback->f1, 3, &length) );
			vector = feedback->f1;
			break;
		case body1Torque:
			//JL_CHK( JsvalToODERealVector(cx, *vp, feedback->t1, 3, &length) );
			vector = feedback->t1;
			break;
		case body2Force:
			//JL_CHK( JsvalToODERealVector(cx, *vp, feedback->f2, 3, &length) );
			vector = feedback->f1;
			break;
		case body2Torque:
			//JL_CHK( JsvalToODERealVector(cx, *vp, feedback->t2, 3, &length) );
			vector = feedback->t2;
			break;
		default:
			ASSERT(false);
			IFDEBUG( vector = NULL );
	}
	JL_CHK( JsvalToODERealVector(cx, *vp, vector, 3, &length) );

	JL_ASSERT( length >= 3, E_VALUE, E_TYPE, E_TY_NVECTOR(3) );
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( feedbackVector ) {

	JL_ASSERT_THIS_INHERITANCE();

	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( jointId );
	ode::dJointFeedback *feedback = ode::dJointGetFeedback(jointId);
	JL_ASSERT( feedback != NULL, E_STR("feedback"), E_DISABLED );

	ode::dReal *vector;
	switch ( JSID_TO_INT(id) ) {
		case body1Force:
			//JL_CHK( ODERealVectorToJsval(cx, feedback->f1, 3, vp) );
			vector = feedback->f1;
			break;
		case body1Torque:
			//JL_CHK( ODERealVectorToJsval(cx, feedback->t1, 3, vp) );
			vector = feedback->t1;
			break;
		case body2Force:
			//JL_CHK( ODERealVectorToJsval(cx, feedback->f2, 3, vp) );
			vector = feedback->f2;
			break;
		case body2Torque:
			//JL_CHK( ODERealVectorToJsval(cx, feedback->t2, 3, vp) );
			vector = feedback->t2;
			break;
		default:
			ASSERT(false);
			IFDEBUG( vector = NULL );
	}
	JL_CHK( ODERealVectorToJsval(cx, vector, 3, vp) );

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
	suspensionERP = ode::dParamSuspensionERP,
	suspensionCFM = ode::dParamSuspensionCFM,
	ERP = ode::dParamERP,

	loStop1 = ode::dParamLoStop,
	hiStop1 = ode::dParamHiStop,
	velocity1 = ode::dParamVel,
	maxForce1 = ode::dParamFMax,
	fudgeFactor1 = ode::dParamFudgeFactor,
	bounce1 = ode::dParamBounce,
	CFM1 = ode::dParamCFM,
	stopERP1 = ode::dParamStopERP,
	stopCFM1 = ode::dParamStopCFM,
	suspensionERP1 = ode::dParamSuspensionERP,
	suspensionCFM1 = ode::dParamSuspensionCFM,
	ERP1 = ode::dParamERP,

	loStop2 = ode::dParamLoStop,
	hiStop2 = ode::dParamHiStop,
	velocity2 = ode::dParamVel,
	maxForce2 = ode::dParamFMax,
	fudgeFactor2 = ode::dParamFudgeFactor,
	bounce2 = ode::dParamBounce,
	CFM2 = ode::dParamCFM,
	stopERP2 = ode::dParamStopERP,
	stopCFM2 = ode::dParamStopCFM,
	suspensionERP2 = ode::dParamSuspensionERP,
	suspensionCFM2 = ode::dParamSuspensionCFM,
	ERP2 = ode::dParamERP,
};


DEFINE_PROPERTY_SETTER( jointParam ) {

	JL_ASSERT_THIS_INHERITANCE();
	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(jointId);
	ode::dReal value;
	JL_CHK( JsvalToODEReal(cx, *vp, &value) );
	JointSetParam(jointId, JSID_TO_INT(id), value);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( jointParam ) {

	JL_ASSERT_THIS_INHERITANCE();
	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(jointId);
	JL_CHK( ODERealToJsval(cx, JointGetParam(jointId, JSID_TO_INT(id)), vp) );
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY_SETTER( jointParam1 ) {

	JL_ASSERT_THIS_INHERITANCE();
	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(jointId);
	ode::dReal real;
	JL_CHK( JsvalToODEReal(cx, *vp, &real) );
	JointSetParam(jointId, JSID_TO_INT(id) + ode::dParamGroup2, real);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( jointParam1 ) {

	JL_ASSERT_THIS_INHERITANCE();
	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(jointId);
	JL_CHK( ODERealToJsval(cx, JointGetParam(jointId, JSID_TO_INT(id) + ode::dParamGroup2), vp) );
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY_SETTER( jointParam2 ) {

	JL_ASSERT_THIS_INHERITANCE();
	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(jointId);
	ode::dReal real;
	JL_CHK( JsvalToODEReal(cx, *vp, &real) );
	JointSetParam(jointId, JSID_TO_INT(id) + ode::dParamGroup3, real);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( jointParam2 ) {

	JL_ASSERT_THIS_INHERITANCE();
	ode::dJointID jointId = (ode::dJointID)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(jointId);
	JL_CHK( ODERealToJsval(cx, JointGetParam(jointId, JSID_TO_INT(id) + ode::dParamGroup3), vp) );
	return JS_TRUE;
	JL_BAD;
}

CONFIGURE_CLASS
	REVISION(JL_SvnRevToInt("$Revision: 3533 $"))

	BEGIN_FUNCTION_SPEC
		FUNCTION( destroy )
//		FUNCTION( getBody )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY( body1 ) // store it to keep a reference (GC protection)
		PROPERTY( body2 ) // store it to keep a reference (GC protection)

		PROPERTY( disabled )
		PROPERTY( useFeedback )

		PROPERTY_SWITCH( body1Force , feedbackVector )
		PROPERTY_SWITCH( body1Torque, feedbackVector )
		PROPERTY_SWITCH( body2Force , feedbackVector )
		PROPERTY_SWITCH( body2Torque, feedbackVector )

		// group 1
		PROPERTY_SWITCH( loStop, jointParam )
		PROPERTY_SWITCH( hiStop, jointParam )
		PROPERTY_SWITCH( velocity, jointParam )
		PROPERTY_SWITCH( maxForce, jointParam )
		PROPERTY_SWITCH( fudgeFactor, jointParam )
		PROPERTY_SWITCH( bounce, jointParam )
		PROPERTY_SWITCH( CFM, jointParam )
		PROPERTY_SWITCH( stopERP, jointParam )
		PROPERTY_SWITCH( stopCFM, jointParam )
		PROPERTY_SWITCH( suspensionERP, jointParam )
		PROPERTY_SWITCH( suspensionCFM, jointParam )
		PROPERTY_SWITCH( ERP, jointParam )

		// group 2
		PROPERTY_SWITCH( loStop1, jointParam1 )
		PROPERTY_SWITCH( hiStop1, jointParam1 )
		PROPERTY_SWITCH( velocity1, jointParam1 )
		PROPERTY_SWITCH( maxForce1, jointParam1 )
		PROPERTY_SWITCH( fudgeFactor1, jointParam1 )
		PROPERTY_SWITCH( bounce1, jointParam1 )
		PROPERTY_SWITCH( CFM1, jointParam1 )
		PROPERTY_SWITCH( stopERP1, jointParam1 )
		PROPERTY_SWITCH( stopCFM1, jointParam1 )
		PROPERTY_SWITCH( suspensionERP1, jointParam1 )
		PROPERTY_SWITCH( suspensionCFM1, jointParam1 )
		PROPERTY_SWITCH( ERP1, jointParam1 )

		// group 3
		PROPERTY_SWITCH( loStop2, jointParam2 )
		PROPERTY_SWITCH( hiStop2, jointParam2 )
		PROPERTY_SWITCH( velocity2, jointParam2 )
		PROPERTY_SWITCH( maxForce2, jointParam2 )
		PROPERTY_SWITCH( fudgeFactor2, jointParam2 )
		PROPERTY_SWITCH( bounce2, jointParam2 )
		PROPERTY_SWITCH( CFM2, jointParam2 )
		PROPERTY_SWITCH( stopERP2, jointParam2 )
		PROPERTY_SWITCH( stopCFM2, jointParam2 )
		PROPERTY_SWITCH( suspensionERP2, jointParam2 )
		PROPERTY_SWITCH( suspensionCFM2, jointParam2 )
		PROPERTY_SWITCH( ERP2, jointParam2 )

		END_PROPERTY_SPEC
END_CLASS
