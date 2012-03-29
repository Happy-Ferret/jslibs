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
#include "space.h"
#include "geom.h"

/**doc
$CLASS_HEADER Geom
$SVN_REVISION $Revision: 3524 $
**/
BEGIN_CLASS( GeomRay )

DEFINE_FINALIZE() {

	FinalizeGeom(cx, obj);
}

/**doc
$TOC_MEMBER $INAME
 $INAME( [ space ] )
  TBD
**/
DEFINE_CONSTRUCTOR() {

	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

	JL_ASSERT_ARGC_RANGE(0, 1);
	ode::dSpaceID space;
	if ( JL_ARG_ISDEF(1) ) { // place it in a space ?

		JL_CHK( JL_JsvalToSpaceID(cx, JL_ARG(1), &space) );
		JL_CHK( JL_SetReservedSlot(cx, obj, SLOT_GEOM_SPACEOBJECT, JL_ARG(1)) );
	} else {

		space = 0;
	}
	ode::dGeomID geomId = ode::dCreateRay(space, 1); // default ray length is 1
	JL_SetPrivate(cx, obj, geomId);
	JL_CHK( SetupReadMatrix(cx, obj) );
	ode::dGeomSetData(geomId, obj); // 'obj' do not need to be rooted because Goem's data is reset to NULL when 'obj' is finalized.
	return JS_TRUE;
	JL_BAD;
}


/**doc
=== Properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $REAL $INAME
  TBD
**/
DEFINE_PROPERTY_SETTER( length ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( geom );
	double radius;
	JL_CHK( JL_JsvalToNative(cx, *vp, &radius) );
	ode::dGeomRaySetLength(geom, (ode::dReal)radius);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( length ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( geom );
	JL_CHK( JL_NativeToJsval(cx, ode::dGeomRayGetLength(geom), vp) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE vec3 $INAME
  TBD
**/
DEFINE_PROPERTY_SETTER( start ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( geom );
	ode::dVector3 start, dir;
	ode::dGeomRayGet(geom, start, dir);
//	FloatArrayToVector(cx, 3, vp, start);
	uint32_t length;
	JL_CHK( JsvalToODERealVector(cx, *vp, start, 3, &length) );
	JL_ASSERT( length >= 3, E_VALUE, E_TYPE, E_TY_NVECTOR(3) );
	ode::dGeomRaySet(geom, start[0], start[1], start[2], dir[0], dir[1], dir[2]);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( start ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( geom );
	ode::dVector3 start, dir;
	ode::dGeomRayGet(geom, start, dir);
	JL_CHK( ODERealVectorToJsval(cx, start, 3, vp) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE vec3 $INAME
  TBD
**/
DEFINE_PROPERTY_SETTER( direction ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( geom );
	ode::dVector3 start, dir;
	ode::dGeomRayGet(geom, start, dir);
//	FloatArrayToVector(cx, 3, vp, dir);
	uint32_t length;
	JL_CHK( JsvalToODERealVector(cx, *vp, dir, 3, &length) );
	JL_ASSERT( length >= 3, E_VALUE, E_TYPE, E_TY_NVECTOR(3) );
	ode::dGeomRaySet(geom, start[0], start[1], start[2], dir[0], dir[1], dir[2]);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( direction ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( geom );
	ode::dVector3 start, dir;
	ode::dGeomRayGet(geom, start, dir);
	JL_CHK( ODERealVectorToJsval(cx, dir, 3, vp) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME
**/
DEFINE_PROPERTY_SETTER( firstContact ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( geom );
	JL_ASSERT_IS_INTEGER(*vp, "");
	int firstContact, backfaceCull;
	ode::dGeomRayGetParams(geom, &firstContact, &backfaceCull);
	JL_CHK( JL_JsvalToNative(cx, *vp, &firstContact) );
	ode::dGeomRaySetParams(geom, firstContact, backfaceCull);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( firstContact ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( geom );
	int firstContact, backfaceCull;
	ode::dGeomRayGetParams(geom, &firstContact, &backfaceCull);
	JL_CHK( JL_NativeToJsval(cx, firstContact, vp) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $REAL $INAME
**/
DEFINE_PROPERTY_SETTER( backfaceCull ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( geom );
	JL_ASSERT_IS_INTEGER(*vp, "");
	int firstContact, backfaceCull;
	ode::dGeomRayGetParams(geom, &firstContact, &backfaceCull);
	JL_CHK( JL_JsvalToNative(cx, *vp, &backfaceCull) );
	ode::dGeomRaySetParams(geom, firstContact, backfaceCull);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( backfaceCull ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( geom );
	int firstContact, backfaceCull;
	ode::dGeomRayGetParams(geom, &firstContact, &backfaceCull);
	JL_CHK( JL_NativeToJsval(cx, backfaceCull, vp) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $REAL $INAME
  TBD
**/
DEFINE_PROPERTY_SETTER( closestHit ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( geom );
	int closestHit;
	JL_CHK( JL_JsvalToNative(cx, *vp, &closestHit) );
	ode::dGeomRaySetClosestHit(geom, closestHit);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( closestHit ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( geom );
	int closestHit;
	closestHit = ode::dGeomRayGetClosestHit(geom);
	JL_CHK( JL_NativeToJsval(cx, closestHit, vp) );
	return JS_TRUE;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision: 3524 $"))
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(3)

	HAS_PROTOTYPE( Geom )
	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_PROPERTY_SPEC
		PROPERTY( length )
		PROPERTY( start )
		PROPERTY( direction )
		PROPERTY( firstContact )
		PROPERTY( backfaceCull )
		PROPERTY( closestHit )
	END_PROPERTY_SPEC

END_CLASS
