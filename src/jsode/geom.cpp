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

bool ReadMatrix(JSContext *cx, JSObject *obj, float **pm) { // Doc: __declspec(noinline) tells the compiler to never inline a particular function.

	ode::dGeomID geomID = (ode::dGeomID)JL_GetPrivate(obj);

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
	return true;
}


bool SetupReadMatrix(JSContext *cx, JSObject *obj) {

	return SetMatrix44GetInterface(cx, obj, ReadMatrix);
}


void FinalizeGeom(JSObject *obj) {

	ode::dGeomID geomId = (ode::dGeomID)JL_GetPrivate(obj);
	if ( !geomId )
		return;
	ode::dGeomSetData(geomId, NULL);
}


bool ReconstructGeom(JSContext *cx, ode::dGeomID geomId, JSObject **obj) { // (TBD) JSObject** = Conservative Stack Scanning issue ?

	JL_ASSERT( geomId != NULL && ode::dGeomGetData(geomId) == NULL, E_MODULE, E_INTERNAL, E_SEP, E_STR(JL_CLASS_NAME(Geom)), E_STATE, E_INVALID );

	switch( ode::dGeomGetClass(geomId) ) {
		case ode::dSphereClass:
			*obj = jl::newObjectWithGivenProto(cx, JL_CLASS(GeomSphere), JL_CLASS_PROTOTYPE(cx, GeomSphere));
			break;
		case ode::dBoxClass:
			*obj = jl::newObjectWithGivenProto(cx, JL_CLASS(GeomBox), JL_CLASS_PROTOTYPE(cx, GeomBox));
			break;
		case ode::dCapsuleClass:
			*obj = jl::newObjectWithGivenProto(cx, JL_CLASS(GeomCapsule), JL_CLASS_PROTOTYPE(cx, GeomCapsule));
			break;
		case ode::dCylinderClass:
			*obj = jl::newObjectWithGivenProto(cx, JL_CLASS(GeomCylinder), JL_CLASS_PROTOTYPE(cx, GeomCylinder));
			break;
		case ode::dPlaneClass:
			*obj = jl::newObjectWithGivenProto(cx, JL_CLASS(GeomPlane), JL_CLASS_PROTOTYPE(cx, GeomPlane));
			break;
		case ode::dRayClass:
			*obj = jl::newObjectWithGivenProto(cx, JL_CLASS(GeomRay), JL_CLASS_PROTOTYPE(cx, GeomRay));
			break;
		case ode::dConvexClass:
			*obj = jl::newObjectWithGivenProto(cx, JL_CLASS(GeomConvex), JL_CLASS_PROTOTYPE(cx, GeomConvex));
			break;
		case ode::dTriMeshClass:
			*obj = jl::newObjectWithGivenProto(cx, JL_CLASS(GeomTrimesh), JL_CLASS_PROTOTYPE(cx, GeomTrimesh));
			break;
		default:
			ASSERT(false);
	}
	JL_CHK( *obj );

	ode::dGeomSetData(geomId, *obj);
	JL_SetPrivate( *obj, geomId);
	return true;
	JL_BAD;
}


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 3533 $
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
DEFINE_FUNCTION( destroy ) {

		JL_ASSERT_THIS_INHERITANCE();

	ode::dGeomID geomId = (ode::dGeomID)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( geomId );
	ode::dGeomDestroy(geomId);
	JL_SetPrivate( obj, NULL);
	SetMatrix44GetInterface(cx, obj, NULL);
	
	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $REAL $INAME( $TYPE vec3 point )
**/
DEFINE_FUNCTION( pointDepth ) {

		JL_ASSERT_THIS_INHERITANCE();
	JL_ASSERT_ARG_IS_ARRAY(1);

	ode::dGeomID geomId = (ode::dGeomID)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( geomId );

	ode::dReal depth, point[3];
	uint32_t len;
	JL_CHK( JsvalToODERealVector(cx, JL_ARG(1), point, 3, &len) );
	JL_ASSERT( len >= 3, E_ARG, E_NUM(1), E_TYPE, E_TY_NVECTOR(3) );

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
		case ode::dCylinderClass:
			depth = ode::dGeomCCylinderPointDepth(geomId, point[0], point[1], point[2]);
			break;
		case ode::dPlaneClass:
			depth = ode::dGeomPlanePointDepth(geomId, point[0], point[1], point[2]);
			break;
//		case ode::dTriMeshClass:
//			depth = ode::dGeomTrimeshPointDepth(geomId, point[0], point[1], point[2]);
//			break;
		default:
			JL_RVAL.setUndefined();
			JL_ERR( E_STR(JL_THIS_CLASS_NAME), E_NOTSUPPORTED );
			return true;
	}

	JL_CHK( JL_NativeToJsval(cx, depth, JL_RVAL) );
	return true;
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

	JL_ASSERT_THIS_INHERITANCE();
	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(geom);
	bool disabled;
	JL_CHK( jl::getValue(cx, *vp, &disabled) );
	if ( disabled )
		ode::dGeomDisable(geom);
	else
		ode::dGeomEnable(geom);
	return true;
	JL_BAD;
}


DEFINE_PROPERTY( disabledGetter ) {

	JL_ASSERT_THIS_INHERITANCE();
	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( geom );
	*vp = BOOLEAN_TO_JSVAL( ode::dGeomIsEnabled(geom) != 1 );
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $BOOL *temporalCoherence*
  Is the status of the geometry.
**/

DEFINE_PROPERTY_SETTER( temporalCoherence ) {

	JL_ASSERT_THIS_INHERITANCE();
	ode::dGeomID geomId = (ode::dGeomID)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(geomId);
	bool enableState;
	JL_CHK( jl::getValue(cx, *vp, &enableState) );
	ode::dGeomTriMeshEnableTC(geomId, ode::dGeomGetClass(geomId), enableState ? 1 : 0 );
	return true;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( temporalCoherence ) {

	JL_ASSERT_THIS_INHERITANCE();
	ode::dGeomID geomId = (ode::dGeomID)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( geomId );
	*vp = BOOLEAN_TO_JSVAL( ode::dGeomTriMeshIsTCEnabled(geomId, ode::dGeomGetClass(geomId)) == 1 );
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE body *body*
  Bind the current geometry to the given body object.
**/
DEFINE_PROPERTY_SETTER( body ) {

	JL_ASSERT_THIS_INHERITANCE();
	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( geom );
	ode::dBodyID bodyId;
	JL_CHK( JL_JsvalToBody(cx, *vp, &bodyId) );
	ode::dGeomSetBody(geom, bodyId);
//	return jl::StoreProperty(cx, obj, id, vp, false);
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( body ) {

	JL_ASSERT_THIS_INHERITANCE();
	ode::dGeomID geomId = (ode::dGeomID)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( geomId );
	JL_CHK( BodyToJsval(cx, ode::dGeomGetBody(geomId), vp) );
//	return jl::StoreProperty(cx, obj, id, vp, false);
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 *offset*
  Sets the position and rotation of the geometry to its center of mass.
  $LF
  Use $UNDEF value to reset the geometry offset.
**/
DEFINE_PROPERTY_SETTER( offset ) {

	JL_ASSERT_THIS_INHERITANCE();
	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(geom);
	if ( vp.isUndefined() ) {

		ode::dGeomClearOffset(geom);
		return true;
	}

	if ( vp.isObject() ) {

// (TBD)! use jsval to matrix44 helper

		JSObject *srcObj = &vp.toObject();
		float tmp[16], *m = tmp;

		NIMatrix44Get matrixGet = Matrix44GetInterface(cx, srcObj);
		JL_CHKM( matrixGet != NULL, E_VALUE, E_SEP, E_INTERFACE, E_STR("Matrix44Get"), E_NOTSUPPORTED );
		
		matrixGet(cx, srcObj, &m);

//		NIMatrix44Get GetMatrix;
//		JL_CHK( GetMatrix44GetInterface(cx, srcObj, &GetMatrix) );
//		JL_ASSERT( ReadMatrix != NULL, "Invalid matrix interface." );
//		ReadMatrix( cx, srcObj, (float**)&m);

		JL_ASSERT( *m != NULL, E_VALUE, E_INVALID );

		ode::dMatrix3 m3 = { m[0], m[4], m[8], 0, m[1], m[5], m[9], 0, m[2], m[6], m[10], 0 }; // (TBD) check
		ode::dGeomSetOffsetRotation(geom, m3);
		ode::dGeomSetOffsetPosition(geom, m[3], m[7], m[11]);
		return true;
	}

	JL_ERR( E_VALUE, E_TYPE, E_STR("matrix44"), E_OR, E_TY_UNDEFINED );
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 *tansformation*
  Sets the position and rotation of the geometry to its center of mass.
**/
DEFINE_PROPERTY_SETTER( tansformation ) {

	JL_ASSERT_THIS_INHERITANCE();
	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(geom);

	if ( vp.isObject) ) {

// (TBD)! use jsval to matrix44 helper

		JSObject *srcObj = vp.toObject();
		float tmp[16], *m = tmp;

		NIMatrix44Get matrixGet = Matrix44GetInterface(cx, srcObj);
		JL_CHKM( matrixGet != NULL, E_VALUE, E_SEP, E_INTERFACE, E_STR("Matrix44Get"), E_NOTSUPPORTED );

		matrixGet(cx, srcObj, &m);
/*
		NIMatrix44Read ReadMatrix;
		JL_CHK( GetMatrix44ReadInterface(cx, srcObj, &ReadMatrix) );
		JL_ASSERT( ReadMatrix != NULL, "Invalid matrix interface." );
		ReadMatrix( cx, srcObj, (float**)&m);
		JL_ASSERT( *m != NULL, "Invalid matrix." );
*/
		ode::dMatrix3 m3 = { m[0], m[4], m[8], 0, m[1], m[5], m[9], 0, m[2], m[6], m[10], 0 }; // (TBD) check
		ode::dGeomSetOffsetRotation(geom, m3);
		ode::dGeomSetOffsetPosition(geom, m[3], m[7], m[11]);
		return jl::StoreProperty(cx, obj, id, vp, false);
	}

	JL_ERR( E_VALUE, E_TYPE, E_STR("matrix44") );
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
  Is the current position of the geometry.
**/
DEFINE_PROPERTY_GETTER( space ) {

	JL_ASSERT_THIS_INHERITANCE();
	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(geom);
/*
	ode::dSpaceID space = ode::dGeomGetSpace(geom);
	ode::dGeomSetData((ode::dGeomID)space, obj);
*/
	
	JL_CHK( JL_GetReservedSlot( obj, SLOT_GEOM_SPACEOBJECT, vp) );
	if ( vp.isUndefined() )
		return true;

//	JL_ASSERT_OBJECT( *vp );
//	JSObject *spaceObj = JSVAL_TO_OBJECT( *vp );
//	JL_ASSERT_INSTANCE( spaceObj, JL_CLASS(Space) );
//	ode::dSpaceID spaceId = (ode::dSpaceID)JL_GetPrivate(spaceObj);

	ode::dSpaceID spaceId;
	JL_CHK( JL_JsvalToSpaceID(cx, *vp, &spaceId) );
	ASSERT( spaceId == ode::dGeomGetSpace(geom) );

	if ( spaceId == NULL ) {
	
		*vp = JSVAL_VOID;
		return JL_SetReservedSlot( obj, SLOT_GEOM_SPACEOBJECT, *vp);
	}

	return true;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $TYPE vec3 *position*
  Is the current position of the geometry.
**/
DEFINE_PROPERTY_GETTER( position ) {

	JL_ASSERT_THIS_INHERITANCE();
	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(geom);
	const ode::dReal *vector = ode::dGeomGetPosition(geom);
	JL_CHK( ODERealVectorToJsval(cx, vector, 3, vp) );
	return true;
	JL_BAD;
}


DEFINE_PROPERTY_SETTER( position ) {

	JL_ASSERT_THIS_INHERITANCE();
	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(geom);
	ode::dVector3 vector;
	uint32_t length;
	JL_CHK( JsvalToODERealVector(cx, *vp, vector, 3, &length) );
	JL_ASSERT( length >= 3, E_VALUE, E_TYPE, E_TY_NVECTOR(3) );
	ode::dGeomSetPosition( geom, vector[0], vector[1], vector[2] );
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $TYPE vec6 $INAME
  Is the axis-aligned bounding box [minx, miny, minz,  maxx, maxy, maxz].
**/
DEFINE_PROPERTY_GETTER( aabb ) {

	JL_ASSERT_THIS_INHERITANCE();
	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(geom);
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
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE vec4 $INAME
  Is the bounding sphere [x,y,z, radius]
**/
DEFINE_PROPERTY_GETTER( boundarySphere ) {

	JL_ASSERT_THIS_INHERITANCE();
	ode::dGeomID geomId = (ode::dGeomID)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(geomId);
	ode::dReal aabb[6];
	ode::dGeomGetAABB(geomId, aabb);

	Vector3 v1, v2, center;
	Vector3Set(&v1, aabb[0], aabb[2], aabb[4]);
	Vector3Set(&v2, aabb[1], aabb[3], aabb[5]);

	Vector3SubVector3(&center, &v1, &v2);
	Vector3Div(&center, &center, 2);
	ode::dReal radius = Vector3Length(&center);
	Vector3AddVector3(&center, &center, &v2);
	
	JL_CHK( ODERealVectorToJsval(cx, center.raw, 3, vp) );
	jsval tmpVal;
	JL_CHK( JL_NativeToJsval(cx, radius, &tmpVal) );
	JL_CHK( JL_SetElement(cx, &vp.toObject(), 3, &tmpVal) );

	return true;
	JL_BAD;
}


/*

DEFINE_PROPERTY( offsetPositionGetter ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(geom);
	const ode::dReal *vector = ode::dGeomGetOffsetPosition(geom); // (TBD) dGeomGetOffsetRotation
	//ODERealVectorToArray(cx, 3, vector, vp);
	JL_CHK( ODERealVectorToJsval(cx, vector, 3, vp) );

	return true;
	JL_BAD;
}


DEFINE_PROPERTY( offsetPositionSetter ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(geom);
	ode::dVector3 vector;
//	FloatArrayToVector(cx, 3, vp, vector);
	size_t length;
	JL_CHK( JsvalToODERealVector(cx, *vp, vector, 3, &length) );
	JL_ASSERT( length >= 3, "Invalid array size." );
	ode::dGeomSetOffsetPosition( geom, vector[0], vector[1], vector[2] ); // (TBD) dGeomSetOffsetWorldRotation
	return true;
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
	
//	JL_ASSERT( JL_IsCallable(cx, *vp) || JSVAL_IS_VOID(*vp), "Invalid type." );
	if ( !vp.isUndefined() )
		JL_ASSERT_IS_CALLABLE(*vp, "");
	return JL_SetReservedSlot( obj, SLOT_GEOM_CONTACT_FUNCTION, *vp);
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( contact ) {

	return JL_GetReservedSlot( obj, SLOT_GEOM_CONTACT_FUNCTION, vp);
}


/**doc
=== Native Interface ===
 * *NIMatrix44Read*
  Is the current geometry's position.
**/

CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision: 3533 $"))
//	HAS_PRIVATE

	BEGIN_FUNCTION_SPEC
		FUNCTION( destroy )
		FUNCTION( pointDepth )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY( contact )
		PROPERTY( body ) // store it to keep a reference (GC protection)
		PROPERTY_SETTER( tansformation )
		PROPERTY_SETTER( offset )
		PROPERTY( disabled )
		PROPERTY( temporalCoherence )
		PROPERTY( position )
		PROPERTY_GETTER( aabb )
		PROPERTY_GETTER( boundarySphere )
		PROPERTY_GETTER( space )
//		PROPERTY( offsetPosition )
	END_PROPERTY_SPEC

END_CLASS

