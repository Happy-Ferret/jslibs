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
#include "mass.h"
#include "body.h"
#include "world.h"
#include "geom.h"
#include "joint.h"

//#include "vector.h"

static JSBool ReadMatrix( JSContext *cx, JSObject *obj, float **pm) { // Doc: __declspec(noinline) tells the compiler to never inline a particular function.

	ode::dBodyID bodyId = (ode::dBodyID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( bodyId );

	const ode::dReal *m43 = ode::dBodyGetRotation(bodyId);
	const ode::dReal *pos = ode::dBodyGetPosition(bodyId);
// (TBD) need center of mass ajustement ?
	float *m = *pm;
	m[0]  = m43[0];
	m[1]  = m43[4];
	m[2]  = m43[8];
	m[3]  = 0;
	m[4]  = m43[1];
	m[5]  = m43[5];
	m[6]  = m43[9];
	m[7]  = 0;
	m[8]  = m43[2];
	m[9]  = m43[6];
	m[10] = m43[10];
	m[11] = 0;
	m[12] = pos[0];// - comx;
	m[13] = pos[1];// - comy;
	m[14] = pos[2];// - comz;
	m[15] = 1;
	return JS_TRUE;
	JL_BAD;
}


JSBool ReconstructBody(JSContext *cx, ode::dBodyID bodyId, JSObject **obj) { // (TBD) JSObject** = Conservative Stack Scanning issue ?

	if (unlikely( bodyId == (ode::dBodyID)0 )) { // bodyId may be null if body is world.env

		*obj = JS_NewObjectWithGivenProto(cx, JL_CLASS(Body), JL_PROTOTYPE(cx, Body), NULL);
		JL_CHK( *obj );
	} else {

		JL_S_ASSERT( ode::dBodyGetData(bodyId) == NULL, "Invalid case (object not finalized)." );
		JL_S_ASSERT( bodyId != NULL, "Invalid ode object." );

		*obj = JS_NewObjectWithGivenProto(cx, JL_CLASS(Body), JL_PROTOTYPE(cx, Body), NULL);
		JL_CHK( *obj );
//		BodyPrivate *bodypv = (BodyPrivate*)jl_malloc(sizeof(BodyPrivate));
//		JL_S_ASSERT_ALLOC( bodypv );
//		bodypv->obj = *obj;
//		ode::dBodySetData(bodyId, bodypv);
		ode::dBodySetData(bodyId, *obj);
	}

	JL_CHK( SetMatrix44GetInterface(cx, *obj, ReadMatrix) );
	JL_SetPrivate(cx, *obj, bodyId);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( Body )

DEFINE_FINALIZE() {

	ode::dBodyID bodyId = (ode::dBodyID)JL_GetPrivate(cx, obj);
	if ( !bodyId )
		return;
//	if ( _odeFinalization )
//		ode::dBodyDestroy(bodyId);
//	else
		ode::dBodySetMovedCallback(bodyId, NULL);
		ode::dBodySetData(bodyId, NULL);
}

/**doc
$TOC_MEMBER $INAME
 $INAME( _world_ )
  TBD
**/

DEFINE_CONSTRUCTOR() {

	JL_S_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

	JL_S_ASSERT_ARG_MIN(1);

	ode::dWorldID worldId;
	JL_CHK( JL_JsvalToWorldID(cx, JL_ARG(1), &worldId) );

//	BodyPrivate *bodypv = (BodyPrivate*)jl_malloc(sizeof(BodyPrivate));
//	JL_S_ASSERT_ALLOC( bodypv );
//	bodypv->obj

	ode::dBodyID bodyId = ode::dBodyCreate(worldId);
	JL_S_ASSERT( bodyId != NULL, "unable to create the body." );
	JL_CHK( SetMatrix44GetInterface(cx, obj, ReadMatrix) );
	JL_SetPrivate(cx, obj, bodyId);
	ode::dBodySetData(bodyId, obj);
	return JS_TRUE;
	JL_BAD;
}



/**doc
=== Methods ===
**/

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  TBD dBodyDestroy
**/
DEFINE_FUNCTION( Destroy ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_CLASS(obj, JL_CLASS(Body));
	ode::dBodyID bodyId = (ode::dBodyID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( bodyId ); // (TBD) manage world-connected ( when bodyId == 0 )
//	JL_SetReservedSlot(cx, obj, BODY_SLOT_WORLD, JSVAL_VOID);
	JL_SetPrivate(cx, obj, NULL);
/*
	ode::dGeomID geomId;
	for ( geomId = ode::dBodyGetFirstGeom(bodyId); geomId; geomId = ode::dBodyGetNextGeom(geomId) ) {

		JSObject *jsGeom = ode::dGeomGetData(geomId);
	}
*/
	ode::dBodyDestroy(bodyId);
	JL_CHK( SetMatrix44GetInterface(cx, obj, NULL) );

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME( _body_ )
  TBD dAreConnected
**/
DEFINE_FUNCTION( IsConnectedTo ) {
	
	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT_CLASS(obj, JL_CLASS(Body));
	ode::dBodyID thisBodyID = (ode::dBodyID)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( thisBodyID );
	ode::dBodyID otherBodyId;
	JL_CHK( JL_JsvalToBody(cx, JL_ARG(1), &otherBodyId) );
	ode::dAreConnected(thisBodyID, otherBodyId);

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $OBJ $INAME( index )
**/
DEFINE_FUNCTION( GetGeom ) {

	JL_DEFINE_FUNCTION_OBJ;

	ode::dBodyID bodyId = (ode::dBodyID)JL_GetPrivate( cx, JL_OBJ );
	JL_S_ASSERT_RESOURCE( bodyId );

	JL_S_ASSERT_ARG_MIN(1);
	int index;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &index) );
	ode::dGeomID geomId;
	for ( geomId = ode::dBodyGetFirstGeom(bodyId); geomId && index--; geomId = ode::dBodyGetNextGeom(geomId) );
	if ( geomId == NULL ) {
		
		*JL_RVAL = JSVAL_VOID;
		return JS_TRUE;
	}
	return GeomToJsval(cx, geomId, JL_RVAL);
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $OBJ $INAME( index )
**/
DEFINE_FUNCTION( GetJoint ) {

	JL_DEFINE_FUNCTION_OBJ;

	ode::dBodyID bodyId = (ode::dBodyID)JL_GetPrivate( cx, JL_OBJ );
	JL_S_ASSERT_RESOURCE( bodyId );

	JL_S_ASSERT_ARG_MIN(1);
	int index;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &index) );
	if ( index < 0 || index >= ode::dBodyGetNumJoints(bodyId) ) {
		
		*JL_RVAL = JSVAL_VOID;
		return JS_TRUE;
	}
	ode::dJointID jointId;
	jointId = ode::dBodyGetJoint(bodyId, index);
	if ( jointId == NULL ) {

		*JL_RVAL = JSVAL_VOID;
		return JS_TRUE;
	}
	JSObject *jsJoint;
	JL_CHK( JointToJSObject(cx, jointId, &jsJoint) );
	*JL_RVAL = OBJECT_TO_JSVAL( jsJoint );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( $TYPE vec3 force [, $TYPE vec3 pos] )
**/
DEFINE_FUNCTION( AddForce ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT_CLASS(obj, JL_CLASS(Body));
	ode::dBodyID thisBodyID = (ode::dBodyID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( thisBodyID );
	uint32 length;
	ode::dVector3 forceVec;
//	JL_CHK( JL_JsvalToODERealVector(cx, JL_ARG(1), forceVec, 3, &length) );
	JL_CHK( JL_JsvalToODERealVector(cx, JL_ARG(1), forceVec, 3, &length) );
	JL_S_ASSERT( length >= 3, "Invalid array size." );
	if ( JL_ARG_ISDEF(2) ) {

		ode::dVector3 posVec;
		JL_CHK( JL_JsvalToODERealVector(cx, JL_ARG(2), posVec, 3, &length) );
		JL_S_ASSERT( length >= 3, "Invalid array size." );
		ode::dBodyAddForceAtPos(thisBodyID, forceVec[0], forceVec[1], forceVec[2], posVec[0], posVec[1], posVec[2] );
		return JS_TRUE;
	}
	ode::dBodyAddForce(thisBodyID, forceVec[0], forceVec[1], forceVec[2] );

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( $TYPE vec3 torque )
**/
DEFINE_FUNCTION( AddTorque ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT_CLASS(obj, JL_CLASS(Body));
	ode::dBodyID thisBodyID = (ode::dBodyID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( thisBodyID );
	ode::dVector3 vector;
	uint32 length;
	JL_CHK( JL_JsvalToODERealVector(cx, JL_ARG(1), vector, 3, &length) );
	JL_S_ASSERT( length >= 3, "Invalid array size." );
	ode::dBodyAddTorque(thisBodyID, vector[0], vector[1], vector[2] );

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
**/
DEFINE_FUNCTION( SetDampingDefaults ) {

	JL_DEFINE_FUNCTION_OBJ;

	ode::dBodyID bodyId = (ode::dBodyID)JL_GetPrivate( cx, JL_OBJ );
	JL_S_ASSERT_RESOURCE( bodyId );
	ode::dBodySetDampingDefaults(bodyId);

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE real $INAME( point )
**/
DEFINE_FUNCTION( GetRelativeVelocity ) {

	JL_DEFINE_FUNCTION_OBJ;

	ode::dBodyID bodyId = (ode::dBodyID)JL_GetPrivate( cx, JL_OBJ );
	JL_S_ASSERT_RESOURCE( bodyId );

	Vector3 pt;
	uint32 len;
	JL_CHK( JL_JsvalToODERealVector(cx, JL_ARG(1), pt.raw, 3, &len) );
	JL_S_ASSERT( len >= 3, "Unsupported vector length (%d).", len );

	Vector3 vel, pos;
	Vector3LoadPtr(&vel, ode::dBodyGetLinearVel(bodyId));
	Vector3LoadPtr(&pos, ode::dBodyGetPosition(bodyId));

	ode::dReal velocity;
	Vector3SubVector3(&pt, &pt, &pos);
	if ( !Vector3IsNull(&pt) ) {

		Vector3Normalize(&pt, &pt);
		velocity = Vector3Dot(&pt, &vel);
//		Vector3Mult(&pt, &pt, Vector3Dot(&pt, &vel));
	} else {

		velocity = Vector3Length(&vel);
	}
//	*JL_RVAL = JL_ARG(JL_ARG_ISDEF(2) ? 2 : 1);
//	return ODERealVectorToJsval(cx, pt.raw, 3, JL_RVAL, true);
	return JL_NativeToJsval(cx, velocity, JL_RVAL);
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( point [ , dest ] )
**/
DEFINE_FUNCTION( GetRelPointVel ) {

	JL_DEFINE_FUNCTION_OBJ;

	ode::dBodyID bodyId = (ode::dBodyID)JL_GetPrivate( cx, JL_OBJ );
	JL_S_ASSERT_RESOURCE( bodyId );

	ode::dReal pt[3];
	uint32 len;
	JL_CHK( JL_JsvalToODERealVector(cx, JL_ARG(1), pt, 3, &len) );

	ode::dVector3 result;
	ode::dBodyGetRelPointVel(bodyId, pt[0], pt[1], pt[2], result);

	*JL_RVAL = JL_ARG(JL_ARG_ISDEF(2) ? 2 : 1);
	return ODERealVectorToJsval(cx, result, 3, JL_RVAL, true);
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( vector [ , dest ] )
**/
DEFINE_FUNCTION( Vector3ToWorld ) {

	JL_DEFINE_FUNCTION_OBJ;

	ode::dBodyID bodyId = (ode::dBodyID)JL_GetPrivate( cx, JL_OBJ );
	JL_S_ASSERT_RESOURCE( bodyId );

	ode::dReal v[3];
	uint32 len;
	JL_CHK( JL_JsvalToODERealVector(cx, JL_ARG(1), v, 3, &len) );
	JL_S_ASSERT( len >= 3, "Unsupported vector length (%d).", len );
	ode::dVector3 result;
	ode::dBodyVectorToWorld(bodyId, v[0], v[1], v[2], result);

//	ode::dBodyGetRelPointPos(bodyId, v.x, v.y, v.z, result);
	*JL_RVAL = JL_ARG(JL_ARG_ISDEF(2) ? 2 : 1);
	return ODERealVectorToJsval(cx, result, 3, JL_RVAL, true);
	JL_BAD;
}



/**doc
=== Properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME
  (TBD)
**/
DEFINE_PROPERTY_SETTER( kinematic ) {

	ode::dBodyID bodyId = (ode::dBodyID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( bodyId );
	bool kinematic;
	JL_CHK( JL_JsvalToNative(cx, *vp, &kinematic) );
	if ( kinematic )
		ode::dBodySetKinematic(bodyId);
	else
		ode::dBodySetDynamic(bodyId);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( kinematic ) {

	ode::dBodyID bodyId = (ode::dBodyID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( bodyId );
	JL_CHK( JL_NativeToJsval(cx, ode::dBodyIsKinematic(bodyId) != 0, vp) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME
  (TBD)
**/
DEFINE_PROPERTY_SETTER( autoDisable ) {

	ode::dBodyID bodyId = (ode::dBodyID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( bodyId );
	bool autoDisable;
	JL_CHK( JL_JsvalToNative(cx, *vp, &autoDisable) );
	ode::dBodySetAutoDisableFlag(bodyId, autoDisable ? 1 : 0);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( autoDisable ) {

	ode::dBodyID bodyId = (ode::dBodyID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( bodyId );
	JL_CHK( JL_NativeToJsval(cx, ode::dBodyGetAutoDisableFlag(bodyId) != 0, vp) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE real $INAME
  (TBD)
**/
DEFINE_PROPERTY_SETTER( autoDisableLinearThreshold ) {

	ode::dBodyID bodyId = (ode::dBodyID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( bodyId );
	ode::dReal threshold;
	JL_CHK( JL_JsvalToODEReal(cx, *vp, &threshold) );
	ode::dBodySetAutoDisableLinearThreshold(bodyId, threshold);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( autoDisableLinearThreshold ) {

	ode::dBodyID bodyId = (ode::dBodyID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( bodyId );
	ode::dReal threshold;
	threshold = ode::dBodyGetAutoDisableLinearThreshold(bodyId);
	JL_CHK( ODERealToJsval(cx, threshold, vp) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE real $INAME
  (TBD)
**/
DEFINE_PROPERTY_SETTER( autoDisableAngularThreshold ) {

	ode::dBodyID bodyId = (ode::dBodyID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( bodyId );
	ode::dReal threshold;
	JL_CHK( JL_JsvalToODEReal(cx, *vp, &threshold) );
	ode::dBodySetAutoDisableAngularThreshold(bodyId, threshold);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( autoDisableAngularThreshold ) {

	ode::dBodyID bodyId = (ode::dBodyID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( bodyId );
	ode::dReal threshold;
	threshold = ode::dBodyGetAutoDisableAngularThreshold(bodyId);
	JL_CHK( ODERealToJsval(cx, threshold, vp) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME
  (TBD)
**/
DEFINE_PROPERTY( disabledSetter ) {

	ode::dBodyID bodyId = (ode::dBodyID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( bodyId );
	bool disabled;
	JL_CHK( JL_JsvalToNative(cx, *vp, &disabled) );
	if ( disabled )
		ode::dBodyDisable(bodyId);
	else
		ode::dBodyEnable(bodyId);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( disabledGetter ) {

	ode::dBodyID bodyId = (ode::dBodyID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( bodyId );
	JL_CHK( JL_NativeToJsval(cx, ode::dBodyIsEnabled(bodyId) == 0, vp) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME
  (TBD)
**/
DEFINE_PROPERTY( gravityModeSetter ) {

	ode::dBodyID bodyId = (ode::dBodyID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( bodyId );
	bool gravityMode;
	JL_CHK( JL_JsvalToNative(cx, *vp, &gravityMode) );
	ode::dBodySetGravityMode(bodyId, gravityMode);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( gravityModeGetter ) {

	ode::dBodyID bodyId = (ode::dBodyID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( bodyId );
	JL_CHK( JL_NativeToJsval(cx, ode::dBodyGetGravityMode(bodyId) != 0, vp) );
	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME
  (TBD)
**/
DEFINE_PROPERTY( gyroscopicModeSetter ) {

	ode::dBodyID bodyId = (ode::dBodyID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( bodyId );
	bool gravityMode;
	JL_CHK( JL_JsvalToNative(cx, *vp, &gravityMode) );
	ode::dBodySetGyroscopicMode(bodyId, gravityMode);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( gyroscopicModeGetter ) {

	ode::dBodyID bodyId = (ode::dBodyID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( bodyId );
	JL_CHK( JL_NativeToJsval(cx, ode::dBodyGetGyroscopicMode(bodyId) != 0, vp) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME
  (TBD)
**/
DEFINE_PROPERTY( finiteRotationModeSetter ) {

	ode::dBodyID bodyId = (ode::dBodyID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( bodyId );
	bool mode;
	JL_CHK( JL_JsvalToNative(cx, *vp, &mode) );
	ode::dBodySetFiniteRotationMode(bodyId, mode);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( finiteRotationModeGetter ) {

	ode::dBodyID bodyId = (ode::dBodyID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( bodyId );
	JL_CHK( JL_NativeToJsval(cx, ode::dBodyGetFiniteRotationMode(bodyId) != 0, vp) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE real $INAME
  (TBD)
**/
DEFINE_PROPERTY( finiteRotationAxisSetter ) {

	ode::dBodyID bodyId = (ode::dBodyID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( bodyId );
	ode::dReal vec[3];
	uint32 len;
	if ( *vp == JSVAL_VOID ) {

		vec[0] = 0;
		vec[1] = 0;
		vec[2] = 0;
	} else {
		
		JL_CHK( JL_JsvalToODERealVector(cx, *vp, vec, 3, &len) );
	}
	JL_S_ASSERT( len >= 3, "Invalid array size." );
	ode::dBodySetFiniteRotationAxis(bodyId, vec[0], vec[1], vec[2]);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( finiteRotationAxisGetter ) {

	ode::dBodyID bodyId = (ode::dBodyID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( bodyId );
	ode::dVector3 vec;
	ode::dBodyGetFiniteRotationAxis(bodyId, vec);
	JL_CHK( ODERealVectorToJsval(cx, vec, 3, vp) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $TYPE real $INAME
  (TBD)
**/
DEFINE_PROPERTY( maxAngularSpeedGetter ) {

	ode::dBodyID bodyId = (ode::dBodyID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( bodyId );
	JL_CHK( JL_NativeToJsval(cx, ode::dBodyGetMaxAngularSpeed(bodyId), vp) );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( maxAngularSpeedSetter ) {

	ode::dBodyID bodyId = (ode::dBodyID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( bodyId );
	ode::dReal maxAngularSpeed;
	JL_CHK( JL_JsvalToNative(cx, *vp, &maxAngularSpeed) );
	ode::dBodySetMaxAngularSpeed(bodyId, maxAngularSpeed);
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $TYPE real $INAME
  (TBD)
**/
DEFINE_PROPERTY( linearDampingGetter ) {

	ode::dBodyID bodyId = (ode::dBodyID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( bodyId );
	JL_CHK( JL_NativeToJsval(cx, ode::dBodyGetLinearDamping(bodyId), vp) );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( linearDampingSetter ) {

	ode::dBodyID bodyId = (ode::dBodyID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( bodyId );
	ode::dReal scale;
	JL_CHK( JL_JsvalToNative(cx, *vp, &scale) );
	ode::dBodySetLinearDamping(bodyId, scale);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE real $INAME
  (TBD)
**/
DEFINE_PROPERTY( linearDampingThresholdGetter ) {

	ode::dBodyID bodyId = (ode::dBodyID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( bodyId );
	JL_CHK( JL_NativeToJsval(cx, ode::dBodyGetLinearDampingThreshold(bodyId), vp) );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( linearDampingThresholdSetter ) {

	ode::dBodyID bodyId = (ode::dBodyID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( bodyId );
	ode::dReal threshold;
	JL_CHK( JL_JsvalToNative(cx, *vp, &threshold) );
	ode::dBodySetLinearDampingThreshold(bodyId, threshold);
	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $TYPE real $INAME
  (TBD)
**/
DEFINE_PROPERTY( angularDampingGetter ) {

	ode::dBodyID bodyId = (ode::dBodyID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( bodyId );
	JL_CHK( JL_NativeToJsval(cx, ode::dBodyGetAngularDamping(bodyId), vp) );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( angularDampingSetter ) {

	ode::dBodyID bodyId = (ode::dBodyID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( bodyId );
	ode::dReal scale;
	JL_CHK( JL_JsvalToNative(cx, *vp, &scale) );
	ode::dBodySetAngularDamping(bodyId, scale);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE real $INAME
  (TBD)
**/
DEFINE_PROPERTY( angularDampingThresholdGetter ) {

	ode::dBodyID bodyId = (ode::dBodyID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( bodyId );
	JL_CHK( JL_NativeToJsval(cx, ode::dBodyGetAngularDampingThreshold(bodyId), vp) );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( angularDampingThresholdSetter ) {

	ode::dBodyID bodyId = (ode::dBodyID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( bodyId );
	ode::dReal threshold;
	JL_CHK( JL_JsvalToNative(cx, *vp, &threshold) );
	ode::dBodySetAngularDampingThreshold(bodyId, threshold);
	return JS_TRUE;
	JL_BAD;
}

/*
/ **doc
$TOC_MEMBER $INAME
 $TYPE real $INAME
  (TBD)
** /
DEFINE_PROPERTY( jointsForce ) {

	ode::dBodyID body = (ode::dBodyID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( body );

	ode::dVector3 force = {0,0,0};

	for ( int i = 0; i < ode::dBodyGetNumJoints(body); ++i ) {
	
		ode::dJointID jointId;
		jointId = ode::dBodyGetJoint(body, i);
		ode::dJointFeedback *feedback = ode::dJointGetFeedback(jointId);
		if ( feedback ) {
			
			ode::dBodyID body1 = ode::dJointGetBody(jointId, 0);
			if ( body == body1 ) {

				force[0] += feedback->f1[0];
				force[1] += feedback->f1[1];
				force[2] += feedback->f1[2];
			} else {

				ode::dBodyID body2 = ode::dJointGetBody(jointId, 1);
				if ( body == body2 ) {
				
					force[0] += feedback->f2[0];
					force[1] += feedback->f2[1];
					force[2] += feedback->f2[2];
				}
			}
		}
	}

	JL_CHK( ODERealVectorToJsval(cx, force, 3, vp) );
	return JS_TRUE;
	JL_BAD;
}


/ **doc
$TOC_MEMBER $INAME
 $TYPE real $INAME
  (TBD)
** /
DEFINE_PROPERTY( jointsTorque ) {

	return JS_TRUE;
	JL_BAD;
}
*/


/*
void BodyPositionSet(void *userData, int index, float value) {
	
	ode::dxBody* body = (ode::dxBody*)userData;
	body->posr.pos[index] = value;
}

float BodyPositionGet(void *userData, int index) {
	
	return ode::dBodyGetPosition((ode::dxBody*)userData)[index];
}

DEFINE_PROPERTY( position ) {

	ode::dBodyID bodyId = (ode::dBodyID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( bodyId );
	if ( JSVAL_IS_VOID(*vp) )
		return CreateVector(cx, obj, bodyId, BodyPositionSet, BodyPositionGet, vp);
	return JS_TRUE;
	JL_BAD;
}
*/


/*
/ **doc
$TOC_MEMBER $INAME
 $BOOL $INAME
  (TBD)
** /
DEFINE_PROPERTY( isMoving ) {

	ode::dBodyID bodyId = (ode::dBodyID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( bodyId );

	const ode::dReal *vector;
	ode::dReal mov;
	vector = ode::dBodyGetLinearVel(bodyId);
	mov = abs(vector[0]) + abs(vector[1]) + abs(vector[2]);

	vector = ode::dBodyGetAngularVel(bodyId);
	mov += abs(vector[0]) + abs(vector[1]) + abs(vector[2]);
	JL_CHK( ODERealToJsval(cx, mov, vp) );

	return JS_TRUE;
	JL_BAD;
}
*/

/**doc
$TOC_MEMBER $INAME
 $TYPE vec3 *position*
  dBodySetPosition

 * $TYPE vec4 *quaternion*
  dBodySetQuaternion
 
 * $TYPE vec3 *linearVel*
  dBodySetLinearVel
  
 * $TYPE vec3 *angularVel*
  dBodySetAngularVel
 
 * $TYPE vec3 *force*
  dBodySetForce
 
 * $TYPE vec3 *torque*
  dBodySetTorque
**/
enum { position, quaternion, linearVel, angularVel, force, torque };

DEFINE_PROPERTY( vectorGetter ) {

	ode::dBodyID bodyID = (ode::dBodyID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( bodyID );
	//if ( bodyID == 0 ) {
	//	*vp = JSVAL_VOID;
	//	return JS_TRUE;
	//}

	const ode::dReal *vector;
	int dim;
	switch ( JSID_TO_INT(id) ) {
		case position:
			vector = ode::dBodyGetPosition(bodyID);
			dim = 3;
			break;
		case quaternion:
			vector = ode::dBodyGetQuaternion(bodyID);
			dim = 4;
			break;
		case linearVel:
			vector = ode::dBodyGetLinearVel(bodyID);
			dim = 3;
			break;
		case angularVel:
			vector = ode::dBodyGetAngularVel(bodyID);
			dim = 3;
			break;
		case force:
			vector = ode::dBodyGetForce(bodyID);
			dim = 3;
			break;
		case torque:
			vector = ode::dBodyGetTorque(bodyID);
			dim = 3;
			break;
	}
	//ODERealVectorToArray(cx, dim, vector, vp);
	JL_CHK( ODERealVectorToJsval(cx, vector, dim, vp) );
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY( vectorSetter ) {

	ode::dBodyID bodyID = (ode::dBodyID)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( bodyID );
	ode::dVector3 vector;
	ode::dVector4 quatern;
	uint32 length;
	switch ( JSID_TO_INT(id) ) {
		case position:
			JL_CHK( JL_JsvalToODERealVector(cx, *vp, vector, 3, &length) );
			JL_S_ASSERT( length >= 3, "Invalid array size." );
			ode::dBodySetPosition( bodyID, vector[0], vector[1], vector[2] );
			break;
		case quaternion:
			JL_CHK( JL_JsvalToODERealVector(cx, *vp, quatern, 4, &length) );
			JL_S_ASSERT( length >= 4, "Invalid array size." );
			ode::dBodySetQuaternion( bodyID, quatern );
			break;
		case linearVel:
			JL_CHK( JL_JsvalToODERealVector(cx, *vp, vector, 3, &length) );
			JL_S_ASSERT( length >= 3, "Invalid array size." );
			ode::dBodySetLinearVel( bodyID, vector[0], vector[1], vector[2] );
			break;
		case angularVel:
			JL_CHK( JL_JsvalToODERealVector(cx, *vp, vector, 3, &length) );
			JL_S_ASSERT( length >= 3, "Invalid array size." );
			ode::dBodySetAngularVel( bodyID, vector[0], vector[1], vector[2] );
			break;
		case force:
			JL_CHK( JL_JsvalToODERealVector(cx, *vp, vector, 3, &length) );
			JL_S_ASSERT( length >= 3, "Invalid array size." );
			ode::dBodySetForce( bodyID, vector[0], vector[1], vector[2] );
			break;
		case torque:
			JL_CHK( JL_JsvalToODERealVector(cx, *vp, vector, 3, &length) );
			JL_S_ASSERT( length >= 3, "Invalid array size." );
			ode::dBodySetTorque( bodyID, vector[0], vector[1], vector[2] );
			break;
	}
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Is the mass object (jsode::Mass) of the body.
**/
DEFINE_PROPERTY( mass ) {

	JSObject *massObject = JS_NewObjectWithGivenProto(cx, JL_CLASS(Mass), JL_PROTOTYPE(cx, Mass), NULL);
	JL_S_ASSERT(massObject != NULL, "Unable to create the Mass object.");
	*vp = OBJECT_TO_JSVAL(massObject);
	JL_CHK( JL_SetReservedSlot(cx, massObject, MASS_SLOT_BODY, OBJECT_TO_JSVAL(obj)) );
	return JL_StoreProperty(cx, obj, id, vp, true);
	JL_BAD;
}



/**doc
=== Static Functions ===
**/

/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME()
**/
DEFINE_FUNCTION( AreConnected ) {

	JL_S_ASSERT_ARG_MIN(2);
	ode::dBodyID body1, body2;
	JL_CHK( JL_JsvalToBody(cx, JL_ARG(1), &body1) );
	JL_CHK( JL_JsvalToBody(cx, JL_ARG(1), &body2) );
	return JL_NativeToJsval(cx, ode::dAreConnected(body1, body2) != 0, JL_RVAL);
	JL_BAD;
}


//JSBool body_set_mass(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
//
//	ode::dBodyID bodyID = (ode::dBodyID)JL_GetPrivate( cx, obj );
//	JL_S_ASSERT( bodyID != NULL, RT_ERROR_NOT_INITIALIZED );
//	JSObject *massObject;
//	JS_ValueToObject(cx, *vp, &massObject);
//	JL_S_ASSERT_CLASS(massObject, &mass_class);
//	ode::dMass *mass = (ode::dMass*)JL_GetPrivate(cx, massObject);
//	JL_S_ASSERT(mass != NULL, RT_ERROR_NOT_INITIALIZED);
//
//	ode::dBodySetMass(bodyID, mass);
//	return JS_TRUE;
//}

/**doc
=== Callback ===
**/

/**doc
$TOC_MEMBER $INAME
 $FUNCTION $INAME ,,not implemented yet,,
**/
void moveCallback(ode::dBodyID bodyId) {

	ode::dWorldID worldId = ode::dBodyGetWorld(bodyId);
	JL_ASSERT( worldId != NULL );
	JSObject *obj = (JSObject*)ode::dBodyGetData(bodyId);
	if ( obj == NULL )
		return;



//	WorldPrivate *pv = (WorldPrivate*)JL_GetPrivate(cx, obj);

	// no way to access the JSContext
}

DEFINE_PROPERTY( onMove ) {

	ode::dBodyID bodyId = (ode::dBodyID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( bodyId );
	if ( JSVAL_IS_VOID(*vp) )
		ode::dBodySetMovedCallback(bodyId, NULL);
	else
		ode::dBodySetMovedCallback(bodyId, moveCallback);
	return JL_StoreProperty(cx, obj, id, vp, false);
	JL_BAD;
}


/**doc
=== Native Interface ===
 * *NIMatrix44Read*
  Is the current body's position.
**/

CONFIGURE_CLASS
	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_PRIVATE
	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION( Destroy )
		FUNCTION_ARGC( IsConnectedTo, 1 )
		FUNCTION_ARGC( GetGeom, 1 )
		FUNCTION_ARGC( GetJoint, 1 )
		FUNCTION_ARGC( AddForce, 1 )
		FUNCTION_ARGC( AddTorque, 1 )
		FUNCTION_ARGC( SetDampingDefaults, 0 )
		FUNCTION_ARGC( GetRelativeVelocity, 1 )
		FUNCTION_ARGC( GetRelPointVel, 2 )
		FUNCTION_ARGC( Vector3ToWorld, 2 )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC

		PROPERTY( kinematic )

		PROPERTY( autoDisable )
		PROPERTY( autoDisableLinearThreshold )
		PROPERTY( autoDisableAngularThreshold )
		PROPERTY( disabled )
		PROPERTY( gravityMode )
		PROPERTY( gyroscopicMode )

		PROPERTY( finiteRotationMode )
		PROPERTY( finiteRotationAxis )

		PROPERTY( maxAngularSpeed )
		PROPERTY( linearDamping )
		PROPERTY( linearDampingThreshold )
		PROPERTY( angularDamping )
		PROPERTY( angularDampingThreshold )

//		PROPERTY_READ( jointsForce )
//		PROPERTY_READ( jointsTorque )

//		PROPERTY_READ( isMoving )

		PROPERTY_SWITCH( position  , vector )
		PROPERTY_SWITCH( quaternion, vector )
		PROPERTY_SWITCH( linearVel , vector )
		PROPERTY_SWITCH( angularVel, vector )
		PROPERTY_SWITCH( force     , vector )
		PROPERTY_SWITCH( torque    , vector )
		PROPERTY_READ( mass ) // mass is only a wrapper to dBodyGetMass and dBodySetMass

//		PROPERTY_READ_STORE( position )

		PROPERTY_WRITE( onMove )

	END_PROPERTY_SPEC

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION_ARGC( AreConnected, 2 )
	END_STATIC_FUNCTION_SPEC

END_CLASS


/****************************************************************


// with new operator
//  argv[-1] : Body
//  argv[-2] : Function
//  argv[-3] : global
//  JS_GetParent : global


// without new operator
//  argv[-1] : World
//  argv[-2] : Function
//  argv[-3] : global

JSObject *o;
jsval *sp = cx->fp->sp;
JS_ValueToObject(cx, *(sp-1) , &o);
JSClass *cl = JL_GetClass(o);


//	cx.lastInternalResult
//	JSObject *o;
//	JS_ValueToObject(cx, cx->lastInternalResult, &o);
//	JSClass *cl = JL_GetClass(o);

*/
