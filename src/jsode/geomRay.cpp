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
//#include "../common/jsNativeInterface.h"

/**doc
$CLASS_HEADER Geom
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( GeomRay )

DEFINE_FINALIZE() {

	ode::dGeomID geomId = (ode::dGeomID)JL_GetPrivate(cx, obj);
	if ( geomId != NULL )
		ode::dGeomSetData(geomId, NULL);
}

/**doc
$TOC_MEMBER $INAME
 $INAME( space )
  TBD
**/
DEFINE_CONSTRUCTOR() {

	JL_S_ASSERT_CONSTRUCTING();
	JL_S_ASSERT_THIS_CLASS();
	ode::dSpaceID space = 0;
	if ( argc >= 1 ) // place it in a space ?
		JL_CHK( ValToSpaceID(cx, argv[0], &space) );
	ode::dGeomID geomId = ode::dCreateRay(space, 1); // default ray length is 1
	JL_SetPrivate(cx, obj, geomId);
	SetupReadMatrix(cx, obj); // (TBD) check return status
	ode::dGeomSetData(geomId, obj); // 'obj' do not need to be rooted because Goem's data is reset to NULL when 'obj' is finalized.
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $REAL $INAME
  TBD
**/
DEFINE_PROPERTY( lengthSetter ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( geom );
	JL_S_ASSERT_NUMBER( *vp );
	jsdouble radius;
	JS_ValueToNumber(cx, *vp, &radius);
	ode::dGeomRaySetLength(geom, radius);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( lengthGetter ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( geom );
	JS_NewDoubleValue(cx, ode::dGeomRayGetLength(geom), vp); // see JS_NewNumberValue and JS_NewDouble
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE vec3 $INAME
  TBD
**/
DEFINE_PROPERTY( startSetter ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( geom );
	ode::dVector3 start, dir;
	ode::dGeomRayGet(geom, start, dir);
//	FloatArrayToVector(cx, 3, vp, start);
	size_t length;
	JL_CHK( JsvalToFloatVector(cx, *vp, start, 3, &length) );
	JL_S_ASSERT( length == 3, "Invalid array size." );
	ode::dGeomRaySet(geom, start[0], start[1], start[2], dir[0], dir[1], dir[2]);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( startGetter ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( geom );
	ode::dVector3 start, dir;
	ode::dGeomRayGet(geom, start, dir);
	//FloatVectorToArray(cx, 3, start, vp);
	JL_CHK( FloatVectorToJsval(cx, start, 3, vp) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE vec3 $INAME
  TBD
**/
DEFINE_PROPERTY( directionSetter ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( geom );
	ode::dVector3 start, dir;
	ode::dGeomRayGet(geom, start, dir);
//	FloatArrayToVector(cx, 3, vp, dir);
	size_t length;
	JL_CHK( JsvalToFloatVector(cx, *vp, dir, 3, &length) );
	JL_S_ASSERT( length == 3, "Invalid array size." );
	ode::dGeomRaySet(geom, start[0], start[1], start[2], dir[0], dir[1], dir[2]);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( directionGetter ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( geom );
	ode::dVector3 start, dir;
	ode::dGeomRayGet(geom, start, dir);
	//FloatVectorToArray(cx, 3, dir, vp);
	JL_CHK( FloatVectorToJsval(cx, dir, 3, vp) );
	return JS_TRUE;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(SvnRevToInt("$Revision$"))
	HAS_FINALIZE
//	CONSTRUCT_PROTOTYPE

	HAS_PROTOTYPE( prototypeGeom )
	HAS_CONSTRUCTOR

	BEGIN_PROPERTY_SPEC
		PROPERTY( length )
		PROPERTY( start )
		PROPERTY( direction )
	END_PROPERTY_SPEC

	HAS_PRIVATE

END_CLASS
