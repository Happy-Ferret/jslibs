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

/*
Collision callback:
	http://opende.sourceforge.net/wiki/index.php/Collision_callback_member_function

check:

	dGeomGetCategoryBits (o1);
	dGeomGetCollideBits (o1);
*/

#include "stdafx.h"
#include "body.h"
#include "geom.h"
//#include "../common/jsNativeInterface.h"


JSBool ReadMatrix(JSContext *cx, JSObject *obj, float **pm) { // Doc: __declspec(noinline) tells the compiler to never inline a particular function.

	ode::dGeomID geomID = (ode::dGeomID)JL_GetPrivate(cx, obj);

	// read LOCAL position and rotation
	const ode::dReal* pos = ode::dGeomGetPosition(geomID);
	const ode::dReal* m43 = ode::dGeomGetRotation(geomID);

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
	m[12] = pos[0];
	m[13] = pos[1];
	m[14] = pos[2];
	m[15] = 1;
	return JS_TRUE;
	JL_BAD;
}


JSBool SetupReadMatrix(JSContext *cx, JSObject *obj) {

	return SetMatrix44GetInterface(cx, obj, ReadMatrix);
}


void FinalizeGeom(JSContext *cx, JSObject *obj) {

	ode::dGeomID geomId = (ode::dGeomID)JL_GetPrivate(cx, obj);
	if ( !geomId )
		return;
//	if ( !ode::dGeomGetBody(geomId) || _odeFinalization ) // geom is lost
//		ode::dGeomDestroy(geomId);
//	else
		ode::dGeomSetData(geomId, NULL);
}


JSBool ReconstructGeom(JSContext *cx, ode::dGeomID geomId, JSObject **obj) {

	JL_S_ASSERT( ode::dGeomGetData(geomId) == NULL, "Invalid case (object not finalized)." );
	JL_S_ASSERT( geomId != NULL, "Invalid ode object." );

	switch( ode::dGeomGetClass(geomId) ) {
		case ode::dSphereClass:
			*obj = JS_NewObject(cx, classGeomSphere, NULL, NULL);
			break;
		case ode::dBoxClass:
			*obj = JS_NewObject(cx, classGeomBox, NULL, NULL);
			break;
		case ode::dCapsuleClass:
			*obj = JS_NewObject(cx, classGeomCapsule, NULL, NULL);
			break;
		case ode::dCylinderClass:
			*obj = JS_NewObject(cx, classGeomCylinder, NULL, NULL);
			break;
		case ode::dPlaneClass:
			*obj = JS_NewObject(cx, classGeomPlane, NULL, NULL);
			break;
		case ode::dRayClass:
			*obj = JS_NewObject(cx, classGeomRay, NULL, NULL);
			break;
		case ode::dConvexClass:
			*obj = JS_NewObject(cx, classGeomConvex, NULL, NULL);
			break;
		case ode::dTriMeshClass:
			*obj = JS_NewObject(cx, classGeomTrimesh, NULL, NULL);
			break;
		default:
			JL_REPORT_ERROR("Unable to reconstruct the geom.");
	}
	JL_CHK( *obj );

	ode::dGeomSetData(geomId, *obj);
	JL_SetPrivate(cx, *obj, geomId);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( Geom )

/**doc
=== Methods ===
**/

/**doc
$TOC_MEMBER $INAME
 $INAME()
  dGeomSetData NULL, dGeomDestroy
**/
DEFINE_FUNCTION( Destroy ) {

	ode::dGeomID geomId = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( geomId );
	ode::dGeomDestroy(geomId);
	JL_SetPrivate(cx, obj, NULL);
	SetMatrix44GetInterface(cx, obj, NULL);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $REAL $INAME( $TYPE vec3 point )
**/
DEFINE_FUNCTION_FAST( PointDepth ) {

	ode::dGeomID geomId = (ode::dGeomID)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE( geomId );
	JL_S_ASSERT_ARRAY( JL_FARG(1) );
	float depth, point[3];
	size_t len;
	JL_CHK( JsvalToFloatVector(cx, *vp, point, 3, &len) );
	JL_S_ASSERT( len == 3, "Invalid array size." );

	switch( ode::dGeomGetClass(geomId) ) {
		case ode::dSphereClass:
			depth = ode::dGeomSpherePointDepth(geomId, point[0], point[1], point[2]);
			break;
		case ode::dBoxClass:
			depth = ode::dGeomBoxPointDepth(geomId, point[0], point[1], point[2]);
			break;
		case ode::dCapsuleClass:
			depth = ode::dGeomCapsulePointDepth(geomId, point[0], point[1], point[2]);
			break;
		case ode::dPlaneClass:
			depth = ode::dGeomPlanePointDepth(geomId, point[0], point[1], point[2]);
			break;
		default:
			JL_REPORT_ERROR("Not support for this geometry.");
	}

	JL_CHK( FloatToJsval(cx, depth, JL_FRVAL) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
=== Properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $BOOL *enable*
  Is the status of the geometry.
**/

DEFINE_PROPERTY( disabledSetter ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(geom);
	bool disabled;
	JL_CHK( JsvalToBool(cx, *vp, &disabled) );
	if ( disabled )
		ode::dGeomDisable(geom);
	else
		ode::dGeomEnable(geom);
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY( disabledGetter ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( geom );
	*vp = ode::dGeomIsEnabled(geom) != 1 ? JSVAL_TRUE : JSVAL_FALSE;
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $BOOL *temporalCoherence*
  Is the status of the geometry.
**/

DEFINE_PROPERTY( temporalCoherenceSetter ) {

	ode::dGeomID geomId = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(geomId);
	bool enableState;
	JL_CHK( JsvalToBool(cx, *vp, &enableState) );
	ode::dGeomTriMeshEnableTC(geomId, ode::dGeomGetClass(geomId), enableState ? 1 : 0 );
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY( temporalCoherenceGetter ) {

	ode::dGeomID geomId = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( geomId );
	*vp = ode::dGeomTriMeshIsTCEnabled(geomId, ode::dGeomGetClass(geomId)) == 1 ? JSVAL_TRUE : JSVAL_FALSE;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE body *body*
  Bind the current geometry to the given body object.
**/
DEFINE_PROPERTY( bodySetter ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( geom );
	ode::dBodyID bodyId;
	JL_CHK( JsvalToBody(cx, *vp, &bodyId) );
	ode::dGeomSetBody(geom, bodyId);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( bodyGetter ) {

	ode::dGeomID geomId = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( geomId );
	return BodyToJsval(cx, ode::dGeomGetBody(geomId), vp);
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $FUNCTION *impact*
  The impact callback.
**/
DEFINE_PROPERTY( impactSetter ) {

	JL_S_ASSERT( JsvalIsFunction(cx, *vp) || JSVAL_IS_VOID(*vp), "Invalid type." );
	return JS_SetReservedSlot(cx, obj, SLOT_GEOM_IMPACT_FUNCTION, *vp);
	JL_BAD;
}

DEFINE_PROPERTY( impactGetter ) {

	return JS_GetReservedSlot(cx, obj, SLOT_GEOM_IMPACT_FUNCTION, vp);
}



/**doc
$TOC_MEMBER $INAME
 *offset*
  Sets the position and rotation of the geometry to its center of mass.
  $LF
  Use $UNDEF value to reset the geometry offset.
**/

// setting undefined means clear the offset
DEFINE_PROPERTY( offset ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(geom);
	if ( JSVAL_IS_VOID( *vp ) ) {

		ode::dGeomClearOffset(geom);
		return JS_TRUE;
	}

	if ( JSVAL_IS_OBJECT(*vp) && !JSVAL_IS_NULL(*vp) ) {

		JSObject *srcObj = JSVAL_TO_OBJECT(*vp);
		float tmp[16], *m = tmp;

		NIMatrix44Get matrixGet = Matrix44GetInterface(cx, srcObj);
		JL_S_ASSERT( matrixGet != NULL, "Invalid matrix interface." );
		matrixGet(cx, srcObj, &m);

//		NIMatrix44Get GetMatrix;
//		JL_CHK( GetMatrix44GetInterface(cx, srcObj, &GetMatrix) );
//		JL_S_ASSERT( ReadMatrix != NULL, "Invalid matrix interface." );
//		ReadMatrix( cx, srcObj, (float**)&m);

		JL_S_ASSERT( *m != NULL, "Invalid matrix." );
		ode::dMatrix3 m3 = { m[0], m[4], m[8], 0, m[1], m[5], m[9], 0, m[2], m[6], m[10], 0 }; // (TBD) check
		ode::dGeomSetOffsetRotation(geom, m3);
		ode::dGeomSetOffsetPosition(geom, m[3], m[7], m[11]);
		return JS_TRUE;
	}
	JL_REPORT_ERROR("Invalid source.");
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 *tansformation*
  Sets the position and rotation of the geometry to its center of mass.
**/
DEFINE_PROPERTY( tansformation ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(geom);

	if ( JSVAL_IS_OBJECT(*vp) && !JSVAL_IS_NULL(*vp) ) {

		JSObject *srcObj = JSVAL_TO_OBJECT(*vp);
		float tmp[16], *m = tmp;

		NIMatrix44Get matrixGet = Matrix44GetInterface(cx, srcObj);
		JL_S_ASSERT( matrixGet != NULL, "Invalid matrix interface." );
		matrixGet(cx, srcObj, &m);
/*
		NIMatrix44Read ReadMatrix;
		JL_CHK( GetMatrix44ReadInterface(cx, srcObj, &ReadMatrix) );
		JL_S_ASSERT( ReadMatrix != NULL, "Invalid matrix interface." );
		ReadMatrix( cx, srcObj, (float**)&m);
		JL_S_ASSERT( *m != NULL, "Invalid matrix." );
*/
		ode::dMatrix3 m3 = { m[0], m[4], m[8], 0, m[1], m[5], m[9], 0, m[2], m[6], m[10], 0 }; // (TBD) check
		ode::dGeomSetOffsetRotation(geom, m3);
		ode::dGeomSetOffsetPosition(geom, m[3], m[7], m[11]);
		return JS_TRUE;
	}
	JL_REPORT_ERROR("Invalid source.");
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $TYPE vec3 *position*
  Is the current position of the geometry.
**/
DEFINE_PROPERTY( positionGetter ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(geom);
	const ode::dReal *vector = ode::dGeomGetPosition(geom);
	//FloatVectorToArray(cx, 3, vector, vp);
	JL_CHK( FloatVectorToJsval(cx, vector, 3, vp) );
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY( positionSetter ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(geom);
	ode::dVector3 vector;
//	FloatArrayToVector(cx, 3, vp, vector);
	size_t length;
	JL_CHK( JsvalToFloatVector(cx, *vp, vector, 3, &length) );
	JL_S_ASSERT( length == 3, "Invalid array size." );
	ode::dGeomSetPosition( geom, vector[0], vector[1], vector[2] );
	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $TYPE vec3 *position*
  Is the axis-aligned bounding box [minx, maxx, miny, maxy, minz, maxz].
**/
DEFINE_PROPERTY( AABB ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(geom);
	ode::dReal aabb[6];
	ode::dGeomGetAABB(geom, aabb);
	JL_CHK( FloatVectorToJsval(cx, aabb, 6, vp) );
	return JS_TRUE;
	JL_BAD;
}


/*

DEFINE_PROPERTY( offsetPositionGetter ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(geom);
	const ode::dReal *vector = ode::dGeomGetOffsetPosition(geom); // (TBD) dGeomGetOffsetRotation
	//FloatVectorToArray(cx, 3, vector, vp);
	JL_CHK( FloatVectorToJsval(cx, vector, 3, vp) );

	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY( offsetPositionSetter ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(geom);
	ode::dVector3 vector;
//	FloatArrayToVector(cx, 3, vp, vector);
	size_t length;
	JL_CHK( JsvalToFloatVector(cx, *vp, vector, 3, &length) );
	JL_S_ASSERT( length == 3, "Invalid array size." );
	ode::dGeomSetOffsetPosition( geom, vector[0], vector[1], vector[2] ); // (TBD) dGeomSetOffsetWorldRotation
	return JS_TRUE;
	JL_BAD;
}
*/


/**doc
=== Callback functions ===
 * *impact*(index, thisGeom, againstGeom, position);
  This function is called each time two geometries collide together.
  _index_ is the index of the collision between step and step+1.
  $LF
  _thisGeom_ is the geometry that is colliding (usualy, `this` object).
  $LF
  _againstGeom_ is the geometry against with this geometry is colliding (the other Geom).
  $LF
  $TYPE vec3 _position_ is the position of the impact point in world position.
**/

/**doc
=== Native Interface ===
 * *NIMatrix44Read*
  Is the current geometry's position.
**/

CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
//	HAS_PRIVATE

	BEGIN_FUNCTION_SPEC
		FUNCTION( Destroy )
		FUNCTION_FAST( PointDepth )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY( impact )
		PROPERTY_STORE( body ) // store it to keep a reference (GC protection)
		PROPERTY_WRITE( tansformation )
		PROPERTY_WRITE( offset )
		PROPERTY( disabled )
		PROPERTY( temporalCoherence )
		PROPERTY( position )
		PROPERTY_READ( AABB )
//		PROPERTY( offsetPosition )
	END_PROPERTY_SPEC

END_CLASS

