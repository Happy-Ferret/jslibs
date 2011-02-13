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
#include "space.h"

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
}


JSBool SetupReadMatrix(JSContext *cx, JSObject *obj) {

	return SetMatrix44GetInterface(cx, obj, ReadMatrix);
}


void FinalizeGeom(JSContext *cx, JSObject *obj) {

	ode::dGeomID geomId = (ode::dGeomID)JL_GetPrivate(cx, obj);
	if ( !geomId )
		return;
	ode::dGeomSetData(geomId, NULL);
}


JSBool ReconstructGeom(JSContext *cx, ode::dGeomID geomId, JSObject **obj) { // (TBD) JSObject** = Conservative Stack Scanning issue ?

	JL_S_ASSERT( ode::dGeomGetData(geomId) == NULL, "Invalid case (object not finalized)." );
	JL_S_ASSERT( geomId != NULL, "Invalid ode object." );

	switch( ode::dGeomGetClass(geomId) ) {
		case ode::dSphereClass:
			*obj = JS_NewObjectWithGivenProto(cx, JL_CLASS(GeomSphere), JL_PROTOTYPE(cx, GeomSphere), NULL);
			break;
		case ode::dBoxClass:
			*obj = JS_NewObjectWithGivenProto(cx, JL_CLASS(GeomBox), JL_PROTOTYPE(cx, GeomBox), NULL);
			break;
		case ode::dCapsuleClass:
			*obj = JS_NewObjectWithGivenProto(cx, JL_CLASS(GeomCapsule), JL_PROTOTYPE(cx, GeomCapsule), NULL);
			break;
		case ode::dCylinderClass:
			*obj = JS_NewObjectWithGivenProto(cx, JL_CLASS(GeomCylinder), JL_PROTOTYPE(cx, GeomCylinder), NULL);
			break;
		case ode::dPlaneClass:
			*obj = JS_NewObjectWithGivenProto(cx, JL_CLASS(GeomPlane), JL_PROTOTYPE(cx, GeomPlane), NULL);
			break;
		case ode::dRayClass:
			*obj = JS_NewObjectWithGivenProto(cx, JL_CLASS(GeomRay), JL_PROTOTYPE(cx, GeomRay), NULL);
			break;
		case ode::dConvexClass:
			*obj = JS_NewObjectWithGivenProto(cx, JL_CLASS(GeomConvex), JL_PROTOTYPE(cx, GeomConvex), NULL);
			break;
		case ode::dTriMeshClass:
			*obj = JS_NewObjectWithGivenProto(cx, JL_CLASS(GeomTrimesh), JL_PROTOTYPE(cx, GeomTrimesh), NULL);
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
 $VOID $INAME()
  dGeomSetData NULL, dGeomDestroy
**/
DEFINE_FUNCTION( Destroy ) {

	JL_DEFINE_FUNCTION_OBJ;

	ode::dGeomID geomId = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( geomId );
	ode::dGeomDestroy(geomId);
	JL_SetPrivate(cx, obj, NULL);
	SetMatrix44GetInterface(cx, obj, NULL);
	
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $REAL $INAME( $TYPE vec3 point )
**/
DEFINE_FUNCTION( PointDepth ) {

	JL_DEFINE_FUNCTION_OBJ;

	ode::dGeomID geomId = (ode::dGeomID)JL_GetPrivate(cx, JL_OBJ);
	JL_S_ASSERT_RESOURCE( geomId );
	JL_S_ASSERT_ARRAY( JL_ARG(1) );
	ode::dReal depth, point[3];
	uint32 len;
	JL_CHK( JL_JsvalToODERealVector(cx, JL_ARG(1), point, 3, &len) );
	JL_S_ASSERT( len >= 3, "Invalid array size." );

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

	JL_CHK( JL_NativeToJsval(cx, depth, JL_RVAL) );
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

DEFINE_PROPERTY_SETTER( disabled ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(geom);
	bool disabled;
	JL_CHK( JL_JsvalToNative(cx, *vp, &disabled) );
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

DEFINE_PROPERTY_SETTER( temporalCoherence ) {

	ode::dGeomID geomId = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(geomId);
	bool enableState;
	JL_CHK( JL_JsvalToNative(cx, *vp, &enableState) );
	ode::dGeomTriMeshEnableTC(geomId, ode::dGeomGetClass(geomId), enableState ? 1 : 0 );
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( temporalCoherence ) {

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
DEFINE_PROPERTY_SETTER( body ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( geom );
	ode::dBodyID bodyId;
	JL_CHK( JL_JsvalToBody(cx, *vp, &bodyId) );
	ode::dGeomSetBody(geom, bodyId);
	return JL_StoreProperty(cx, obj, id, vp, false);
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( body ) {

	ode::dGeomID geomId = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( geomId );
	JL_CHK( BodyToJsval(cx, ode::dGeomGetBody(geomId), vp) );
	return JL_StoreProperty(cx, obj, id, vp, false);
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 *offset*
  Sets the position and rotation of the geometry to its center of mass.
  $LF
  Use $UNDEF value to reset the geometry offset.
**/

// setting undefined means clear the offset
DEFINE_PROPERTY_SETTER( offset ) {

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
DEFINE_PROPERTY_SETTER( tansformation ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(geom);

	if ( !JSVAL_IS_PRIMITIVE(*vp) ) {

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
		return JL_StoreProperty(cx, obj, id, vp, false);
	}
	JL_REPORT_ERROR("Invalid source.");
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
  Is the current position of the geometry.
**/
DEFINE_PROPERTY_GETTER( space ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(geom);
/*
	ode::dSpaceID space = ode::dGeomGetSpace(geom);
	ode::dGeomSetData((ode::dGeomID)space, obj);
*/
	
	JL_CHK( JL_GetReservedSlot(cx, obj, SLOT_GEOM_SPACEOBJECT, vp) );
	if ( JSVAL_IS_VOID( *vp ) )
		return JS_TRUE;

//	JL_S_ASSERT_OBJECT( *vp );
//	JSObject *spaceObj = JSVAL_TO_OBJECT( *vp );
//	JL_S_ASSERT_CLASS( spaceObj, JL_CLASS(Space) );
//	ode::dSpaceID spaceId = (ode::dSpaceID)JL_GetPrivate(cx, spaceObj);

	ode::dSpaceID spaceId;
	JL_CHK( JL_JsvalToSpaceID(cx, *vp, &spaceId) );
	JL_ASSERT( spaceId == ode::dGeomGetSpace(geom) );

	if ( spaceId == NULL ) {
	
		*vp = JSVAL_VOID;
		return JL_SetReservedSlot(cx, obj, SLOT_GEOM_SPACEOBJECT, *vp);
	}

	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $TYPE vec3 *position*
  Is the current position of the geometry.
**/
DEFINE_PROPERTY_GETTER( position ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(geom);
	const ode::dReal *vector = ode::dGeomGetPosition(geom);
	JL_CHK( ODERealVectorToJsval(cx, vector, 3, vp) );
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY_SETTER( position ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(geom);
	ode::dVector3 vector;
	uint32 length;
	JL_CHK( JL_JsvalToODERealVector(cx, *vp, vector, 3, &length) );
	JL_S_ASSERT( length >= 3, "Invalid array size." );
	ode::dGeomSetPosition( geom, vector[0], vector[1], vector[2] );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $TYPE vec6 $INAME
  Is the axis-aligned bounding box [minx, miny, minz,  maxx, maxy, maxz].
**/
DEFINE_PROPERTY_GETTER( aabb ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(geom);
	ode::dReal aabb[6];
	ode::dGeomGetAABB(geom, aabb);

	ode::dReal tmp[6];
	tmp[0] = aabb[0];
	tmp[1] = aabb[2];
	tmp[2] = aabb[4];
	tmp[3] = aabb[1];
	tmp[4] = aabb[3];
	tmp[5] = aabb[5];

	JL_CHK( ODERealVectorToJsval(cx, tmp, 6, vp) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE vec4 $INAME
  Is the bounding sphere [x,y,z, radius]
**/
DEFINE_PROPERTY_GETTER( boundarySphere ) {

	ode::dGeomID geomId = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(geomId);
	ode::dReal aabb[6];
	ode::dGeomGetAABB(geomId, aabb);

	Vector3 v1, v2, center;
	Vector3Set(&v1, aabb[0], aabb[2], aabb[4] );
	Vector3Set(&v2, aabb[1], aabb[3], aabb[5] );

	Vector3SubVector3(&center, &v1, &v2);
	Vector3Div(&center, &center, 2);
	ode::dReal radius = Vector3Length(&center);
	Vector3AddVector3(&center, &center, &v2);
	
	JL_CHK( ODERealVectorToJsval(cx, center.raw, 3, vp) );
	jsval tmpVal;
	JL_CHK( JL_NativeToJsval(cx, radius, &tmpVal) );
	JL_CHK( JS_SetElement(cx, JSVAL_TO_OBJECT(*vp), 3, &tmpVal) );

	return JS_TRUE;
	JL_BAD;
}


/*

DEFINE_PROPERTY( offsetPositionGetter ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(geom);
	const ode::dReal *vector = ode::dGeomGetOffsetPosition(geom); // (TBD) dGeomGetOffsetRotation
	//ODERealVectorToArray(cx, 3, vector, vp);
	JL_CHK( ODERealVectorToJsval(cx, vector, 3, vp) );

	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY( offsetPositionSetter ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(geom);
	ode::dVector3 vector;
//	FloatArrayToVector(cx, 3, vp, vector);
	size_t length;
	JL_CHK( JL_JsvalToODERealVector(cx, *vp, vector, 3, &length) );
	JL_S_ASSERT( length >= 3, "Invalid array size." );
	ode::dGeomSetOffsetPosition( geom, vector[0], vector[1], vector[2] ); // (TBD) dGeomSetOffsetWorldRotation
	return JS_TRUE;
	JL_BAD;
}
*/


/**doc
=== Callback functions ===
 * *contact*(thisGeom, againstGeom, contactVelocity, contactX, contactY, contactZ, side1, side2);
  This function is called each time two geometries collide together.
  _thisGeom_ is the geometry that is colliding (usualy, `this` object).
  $LF
  _againstGeom_ is the geometry against with this geometry is colliding (the other Geom).
**/
DEFINE_PROPERTY_SETTER( contact ) {
	
//	JL_S_ASSERT( JL_IsFunction(cx, *vp) || JSVAL_IS_VOID(*vp), "Invalid type." );
	if ( !JSVAL_IS_VOID(*vp) )
		JL_S_ASSERT_FUNCTION(*vp);
	return JL_SetReservedSlot(cx, obj, SLOT_GEOM_CONTACT_FUNCTION, *vp);
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( contact ) {

	return JL_GetReservedSlot(cx, obj, SLOT_GEOM_CONTACT_FUNCTION, vp);
}


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
		FUNCTION( PointDepth )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY( contact )
		PROPERTY( body ) // store it to keep a reference (GC protection)
		PROPERTY_WRITE( tansformation )
		PROPERTY_WRITE( offset )
		PROPERTY( disabled )
		PROPERTY( temporalCoherence )
		PROPERTY( position )
		PROPERTY_READ( aabb )
		PROPERTY_READ( boundarySphere )
		PROPERTY_READ( space )
//		PROPERTY( offsetPosition )
	END_PROPERTY_SPEC

END_CLASS

