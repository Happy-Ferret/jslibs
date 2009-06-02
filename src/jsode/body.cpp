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
//#include <jscntxt.h>
#include "mass.h"
#include "body.h"
#include "world.h"
//#include "../common/jsNativeInterface.h"

static JSBool ReadMatrix( JSContext *cx, JSObject *obj, float **pm) { // Doc: __declspec(noinline) tells the compiler to never inline a particular function.

	//ode::dBodyID id = (ode::dBodyID)pv;
	ode::dBodyID id = (ode::dBodyID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(id);

	const ode::dReal * m43 = dBodyGetRotation( id );
	const ode::dReal * pos = dBodyGetPosition( id );
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

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( Body )


DEFINE_FINALIZE() {

// (TBD) manage Mass object (body.mass)
/*
	ode::dBodyID bodyId = (ode::dBodyID)JL_GetPrivate( cx, obj );
//	JSObject *parent = JS_GetParent(cx,obj); // If the object does not have a parent, or the object finalize function is active, JS_GetParent returns NULL.
	if ( bodyId != NULL ) {
		dBodyDestroy(bodyId);
		JL_SetPrivate(cx, obj, NULL);
	}
*/
}

/**doc
$TOC_MEMBER $INAME
 $INAME( _world_ )
  TBD
**/

DEFINE_CONSTRUCTOR() {

	JL_S_ASSERT_CONSTRUCTING();
//	JL_S_ASSERT_CLASS(&classBody);
	JL_S_ASSERT_THIS_CLASS();

	JL_S_ASSERT_ARG_MIN(1);

	ode::dWorldID worldId;
	ValToWorldID(cx, argv[0], &worldId);

	ode::dBodyID bodyID = ode::dBodyCreate(worldId);
	JL_S_ASSERT( bodyID != NULL, "unable to create the body." );

	JL_SetPrivate(cx, obj, bodyID);
	JS_SetReservedSlot(cx, obj, BODY_SLOT_WORLD, argv[0]);

//	JS_AddRoot(cx, obj); // Doc: The pointer or jsval whose address is passed as rp must live in storage that remains allocated until the balancing JS_RemoveRoot call is made.
//	ode::dBodySetData(bodyID, obj);

//	ode::dBodySetData(bodyID,worldObject);
	JL_CHK( SetMatrix44GetInterface(cx, obj, ReadMatrix) );
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

	JL_S_ASSERT_CLASS(obj, classBody);
	ode::dBodyID bodyId = (ode::dBodyID)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( bodyId ); // (TBD) manage world-connected ( when bodyId == 0 )
	dBodyDestroy(bodyId);
	JL_SetPrivate(cx, obj, NULL);
	JS_SetReservedSlot(cx, obj, BODY_SLOT_WORLD, JSVAL_VOID);
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME( _body_ )
  TBD dAreConnected
**/
DEFINE_FUNCTION( IsConnectedTo ) {

	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT_CLASS(obj, classBody);
	ode::dBodyID thisBodyID = (ode::dBodyID)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( thisBodyID );
	ode::dBodyID bodyId;
	ValToBodyID(cx, argv[0], &bodyId);
	ode::dAreConnected(thisBodyID, bodyId);
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( $TYPE vec3 torque )
**/
DEFINE_FUNCTION( AddTorque ) {

	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT_CLASS(obj, classBody);
	ode::dBodyID thisBodyID = (ode::dBodyID)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( thisBodyID );
	ode::dVector3 vector;
	size_t length;
	JL_CHK( JsvalToFloatVector(cx, JL_ARG(1), vector, 3, &length) );
	JL_S_ASSERT( length == 3, "Invalid array size." );
	ode::dBodyAddTorque(thisBodyID, vector[0], vector[1], vector[2] );
	return JS_TRUE;
	JL_BAD;
}


/**doc
=== Properties ===
**/

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
	switch(JSVAL_TO_INT(id)) {
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
	//FloatVectorToArray(cx, dim, vector, vp);
	JL_CHK( FloatVectorToJsval(cx, vector, dim, vp) );
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY( vectorSetter ) {

	ode::dBodyID bodyID = (ode::dBodyID)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( bodyID );
	ode::dVector3 vector;
	ode::dVector4 quatern;
	size_t length;
	switch(JSVAL_TO_INT(id)) {
		case position:
			//FloatArrayToVector(cx, 3, vp, vector);
			JL_CHK( JsvalToFloatVector(cx, *vp, vector, 3, &length) );
			JL_S_ASSERT( length == 3, "Invalid array size." );
			ode::dBodySetPosition( bodyID, vector[0], vector[1], vector[2] );
			break;
		case quaternion:
			//FloatArrayToVector(cx, 4, vp, vector);
			JL_CHK( JsvalToFloatVector(cx, *vp, quatern, 4, &length) );
			JL_S_ASSERT( length == 4, "Invalid array size." );
			ode::dBodySetQuaternion( bodyID, quatern );
			break;
		case linearVel:
			//FloatArrayToVector(cx, 3, vp, vector);
			JL_CHK( JsvalToFloatVector(cx, *vp, vector, 3, &length) );
			JL_S_ASSERT( length == 3, "Invalid array size." );
			ode::dBodySetLinearVel( bodyID, vector[0], vector[1], vector[2] );
			break;
		case angularVel:
			//FloatArrayToVector(cx, 3, vp, vector);
			JL_CHK( JsvalToFloatVector(cx, *vp, vector, 3, &length) );
			JL_S_ASSERT( length == 3, "Invalid array size." );
			ode::dBodySetAngularVel( bodyID, vector[0], vector[1], vector[2] );
			break;
		case force:
			//FloatArrayToVector(cx, 3, vp, vector);
			JL_CHK( JsvalToFloatVector(cx, *vp, vector, 3, &length) );
			JL_S_ASSERT( length == 3, "Invalid array size." );
			ode::dBodySetForce( bodyID, vector[0], vector[1], vector[2] );
			break;
		case torque:
			//FloatArrayToVector(cx, 3, vp, vector);
			JL_CHK( JsvalToFloatVector(cx, *vp, vector, 3, &length) );
			JL_S_ASSERT( length == 3, "Invalid array size." );
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

	if ( JSVAL_IS_VOID( *vp ) ) { // if mass do not exist, we have to create it and store it

		JSObject *massObject = JS_NewObject(cx, classMass, NULL, NULL);
		JL_S_ASSERT(massObject != NULL, "unable to construct Mass object.");
		JS_SetReservedSlot(cx, massObject, MASS_SLOT_BODY, OBJECT_TO_JSVAL(obj));
		*vp = OBJECT_TO_JSVAL(massObject);
	}
	return JS_TRUE;
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
=== Native Interface ===
 * *NIMatrix44Read*
  Is the current body's position.
**/

CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION( Destroy )
		FUNCTION( IsConnectedTo )
		FUNCTION( AddTorque )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_SWITCH( position  , vector )
		PROPERTY_SWITCH( quaternion, vector )
		PROPERTY_SWITCH( linearVel , vector )
		PROPERTY_SWITCH( angularVel, vector )
		PROPERTY_SWITCH( force     , vector )
		PROPERTY_SWITCH( torque    , vector )
		PROPERTY_READ_STORE( mass ) // mass is only a wrapper to dBodyGetMass and dBodySetMass
	END_PROPERTY_SPEC

	HAS_PRIVATE  // private: BodyID
	HAS_RESERVED_SLOTS(1)

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
