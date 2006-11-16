#include "stdafx.h"
#include "joint.h"
#include "body.h"
#include "world.h"

BEGIN_CLASS

	// Api: dBodyID dJointGetBody (dJointID, int index);
	// Api: int dAreConnected (dBodyID b1, dBodyID b2);

	
	// [TBD] replace Attach with body1 and body2 setter

/*
DEFINE_PROPERTY( body1 ) {

	JS_GetReservedSlot(cx, obj, JOINT_SLOT_BODY1, vp);
	return JS_TRUE;
}
DEFINE_PROPERTY( body2 ) {

	JS_GetReservedSlot(cx, obj, JOINT_SLOT_BODY2, vp);
	return JS_TRUE;
}
*/

inline JSBool SetJoint( JSContext *cx, JSObject *obj, jsval *b1, jsval *b2 ) {

	ode::dJointID jointID = (ode::dJointID)JS_GetPrivate( cx, obj );
	RT_ASSERT( jointID != NULL, RT_ERROR_NOT_INITIALIZED );

	if ( *b1 == JSVAL_VOID || *b2 == JSVAL_VOID )
		ode::dJointAttach(jointID, 0, 0); // detach it. The only way to attach it to the world environment is to use World.env

	ode::dBodyID bId1 = 0;
	ode::dBodyID bId2 = 0;

	if ( *b1 != JSVAL_VOID )
		if ( ValToBodyID(cx, *b1, &bId1) == JS_FALSE )
			return JS_FALSE;
	
	if ( *b2 != JSVAL_VOID )
		if ( ValToBodyID(cx, *b2, &bId2) == JS_FALSE )
			return JS_FALSE;

	ode::dJointAttach(jointID, bId1, bId2);
	return JS_TRUE;
}


DEFINE_PROPERTY( body1 ) {

	jsval b2;
	JS_GetProperty(cx, obj, "body2", &b2);
	if ( SetJoint(cx, obj, vp, &b2) == JS_FALSE )
		return JS_FALSE;
	return JS_TRUE;
}


DEFINE_PROPERTY( body2 ) {

	jsval b1;
	JS_GetProperty(cx, obj, "body1", &b1);
	if ( SetJoint(cx, obj, &b1, vp) == JS_FALSE )
		return JS_FALSE;
	return JS_TRUE;
}


DEFINE_FUNCTION( Destroy ) {

	RT_ASSERT( IsInstanceOf(cx, obj, thisClass), RT_ERROR_INVALID_CLASS );
	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( jointId );
	// remove references to bodies
	jsval val = JSVAL_VOID;
	JS_SetProperty(cx, obj, "body1", &val); // [TBD] find why to not use JS_DeleteProperty
	JS_SetProperty(cx, obj, "body2", &val);
	JS_SetPrivate(cx, obj, NULL); 
	ode::dJointDestroy(jointId);
	return JS_TRUE;
}

/*
DEFINE_FUNCTION( Attach ) {

	RT_ASSERT_ARGC(2);
	RT_ASSERT( IsInstanceOf(cx, obj, thisClass), RT_ERROR_INVALID_CLASS );

	ode::dJointID jointID = (ode::dJointID)JS_GetPrivate( cx, obj );
	RT_ASSERT( jointID != NULL, RT_ERROR_NOT_INITIALIZED );

	JSObject *body1Object, *body2Object;

	JS_ValueToObject(cx, argv[0], &body1Object);
	RT_ASSERT_CLASS(body1Object, &classBody);
	JS_SetReservedSlot(cx, obj, JOINT_SLOT_BODY1, argv[0]);
	ode::dBodyID bodyID1 = (ode::dBodyID)JS_GetPrivate(cx, body1Object);
//	RT_ASSERT(bodyID != NULL, RT_ERROR_NOT_INITIALIZED);

	JS_ValueToObject(cx, argv[1], &body2Object);
	RT_ASSERT_CLASS(body2Object, &classBody);
	JS_SetReservedSlot(cx, obj, JOINT_SLOT_BODY2, argv[1]);
	ode::dBodyID bodyID2 = (ode::dBodyID)JS_GetPrivate(cx, body2Object);
//	RT_ASSERT(bodyID != NULL, RT_ERROR_NOT_INITIALIZED);

	ode::dJointAttach(jointID, bodyID1, bodyID2);
	return JS_TRUE;
}
*/

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
		case ode::dJointTypeBall:
//			ode::dJointSetBallParam(jointId, parameter, value);
			break;
		case ode::dJointTypeHinge:
			ode::dJointSetHingeParam(jointId, parameter, value);
			break;
		case ode::dJointTypeSlider:
			ode::dJointSetSliderParam(jointId, parameter, value);
			break;
		case ode::dJointTypeFixed:
//			ode::dJointSetFixedParam(jointId, parameter, value);
			break;
	}
}


ode::dReal JointGetParam( ode::dJointID jointId, int parameter ) {

	switch( ode::dJointGetType(jointId) ) {
		case ode::dJointTypeBall:
//			ode::dJointGetBallParam(jointId, parameter);
			break;
		case ode::dJointTypeHinge:
			ode::dJointGetHingeParam(jointId, parameter);
			break;
		case ode::dJointTypeSlider:
			ode::dJointGetSliderParam(jointId, parameter);
			break;
		case ode::dJointTypeFixed:
//			ode::dJointGetFixedParam(jointId, parameter);
			break;
	}
	return 0;
}



// http://opende.sourceforge.net/wiki/index.php/Manual_%28Joint_Types_and_Functions%29
DEFINE_PROPERTY( loStopSetter ) {

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(jointId);
	jsdouble value;
	if ( *vp == JSVAL_VOID )
		value = -dInfinity;
	else
		JS_ValueToNumber(cx, *vp, &value);
	JointSetParam(jointId, ode::dParamLoStop, value);
	return JS_TRUE;
}

DEFINE_PROPERTY( loStopGetter ) {

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(jointId);
	JS_NewDoubleValue(cx, JointGetParam(jointId, ode::dParamLoStop), vp);
	return JS_TRUE;
}


DEFINE_PROPERTY( hiStopSetter ) {

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(jointId);
	jsdouble value;
	if ( *vp == JSVAL_VOID )
		value = dInfinity;
	else
		JS_ValueToNumber(cx, *vp, &value);
	JointSetParam(jointId, ode::dParamLoStop, value);
	return JS_TRUE;
}

DEFINE_PROPERTY( hiStopGetter ) {

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(jointId);
	JS_NewDoubleValue(cx, JointGetParam(jointId, ode::dParamLoStop), vp);
	return JS_TRUE;
}


DEFINE_PROPERTY( bounceSetter ) {

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(jointId);
	jsdouble value;
	JS_ValueToNumber(cx, *vp, &value);
	JointSetParam(jointId, ode::dParamBounce, value);
	return JS_TRUE;
}

DEFINE_PROPERTY( bounceGetter ) {

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(jointId);
	JS_NewDoubleValue(cx, JointGetParam(jointId, ode::dParamBounce), vp);
	return JS_TRUE;
}


DEFINE_PROPERTY( CFMSetter ) {

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(jointId);
	jsdouble value;
	JS_ValueToNumber(cx, *vp, &value);
	JointSetParam(jointId, ode::dParamCFM, value);
	return JS_TRUE;
}

DEFINE_PROPERTY( CFMGetter ) {

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(jointId);
	JS_NewDoubleValue(cx, JointGetParam(jointId, ode::dParamCFM), vp);
	return JS_TRUE;
}



DEFINE_PROPERTY( stopERPSetter ) {

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(jointId);
	jsdouble value;
	JS_ValueToNumber(cx, *vp, &value);
	JointSetParam(jointId, ode::dParamStopERP, value);
	return JS_TRUE;
}

DEFINE_PROPERTY( stopERPGetter ) {

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(jointId);
	JS_NewDoubleValue(cx, JointGetParam(jointId, ode::dParamStopERP), vp);
	return JS_TRUE;
}



DEFINE_PROPERTY( stopCFMSetter ) {

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(jointId);
	jsdouble value;
	JS_ValueToNumber(cx, *vp, &value);
	JointSetParam(jointId, ode::dParamStopCFM, value);
	return JS_TRUE;
}

DEFINE_PROPERTY( stopCFMGetter ) {

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(jointId);
	JS_NewDoubleValue(cx, JointGetParam(jointId, ode::dParamStopCFM), vp);
	return JS_TRUE;
}

//dParamSuspensionERP 	 Suspension error reduction parameter (ERP). Currently this is only implemented on the hinge-2 joint.
//dParamSuspensionCFM 	Suspension constraint force mixing (CFM) value. Currently this is only implemented on the hinge-2 joint.



DEFINE_PROPERTY( velocitySetter ) {

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(jointId);
	jsdouble value;
	JS_ValueToNumber(cx, *vp, &value);
	JointSetParam(jointId, ode::dParamVel, value);
	return JS_TRUE;
}

DEFINE_PROPERTY( velocityGetter ) {

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(jointId);
	JS_NewDoubleValue(cx, JointGetParam(jointId, ode::dParamFMax), vp);
	return JS_TRUE;
}


DEFINE_PROPERTY( maxForceSetter ) {

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(jointId);
	jsdouble value;
	JS_ValueToNumber(cx, *vp, &value);
	JointSetParam(jointId, ode::dParamVel, value);
	return JS_TRUE;
}

DEFINE_PROPERTY( maxForceGetter ) {

	ode::dJointID jointId = (ode::dJointID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(jointId);
	JS_NewDoubleValue(cx, JointGetParam(jointId, ode::dParamFMax), vp);
	return JS_TRUE;
}



BEGIN_FUNCTION_MAP
//	FUNCTION( Attach )
	FUNCTION( Destroy )
END_MAP

BEGIN_PROPERTY_MAP
//	PROPERTY_READONLY_STORE( body1 )
//	PROPERTY_READONLY_STORE( body2 )
	PROPERTY_STORE( body1 )
	PROPERTY_STORE( body2 )

	READWRITE( loStop )
	READWRITE( hiStop )
	READWRITE( bounce )

	READWRITE( CFM )
	READWRITE( stopERP )
	READWRITE( stopCFM )

	READWRITE( velocity )
	READWRITE( maxForce )
END_MAP

NO_STATIC_FUNCTION_MAP
NO_STATIC_PROPERTY_MAP
NO_CLASS_CONSTRUCT
NO_OBJECT_CONSTRUCT
NO_FINALIZE
NO_CALL
NO_PROTOTYPE
NO_CONSTANT_MAP
NO_INITCLASSAUX

END_CLASS( Joint, NO_PRIVATE, NO_RESERVED_SLOT );
