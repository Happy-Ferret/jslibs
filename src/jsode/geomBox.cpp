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
**/
BEGIN_CLASS( GeomBox )

DEFINE_FINALIZE() {

	ode::dGeomID geomId = (ode::dGeomID)JS_GetPrivate(cx, obj);
	if ( geomId != NULL )
		ode::dGeomSetData(geomId, NULL);
}

/**doc
 * $INAME( space )
  TBD
**/
DEFINE_CONSTRUCTOR() {

	RT_ASSERT_CONSTRUCTING(&classGeomBox);
	ode::dSpaceID space = 0;
	if ( argc >= 1 ) // place it in a space ?
		if ( ValToSpaceID(cx, argv[0], &space) == JS_FALSE )
			return JS_FALSE;
	ode::dGeomID geomId = ode::dCreateBox(space, 1,1,1); // default lengths are 1
	JS_SetPrivate(cx, obj, geomId);
	SetupReadMatrix(cx, obj); // (TBD) check return status
	ode::dGeomSetData(geomId, obj); // 'obj' do not need to be rooted because Goem's data is reset to NULL when 'obj' is finalized.
	return JS_TRUE;
}

/**doc
=== Properties ===
**/

/**doc
 * $TYPE vec3 $INAME
  Is the x, y, z size of the box.
**/
DEFINE_PROPERTY( lengthsSetter ) {

	ode::dGeomID geom = (ode::dGeomID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE( geom );
	RT_ASSERT_NUMBER( *vp );
	ode::dVector3 vector;
	FloatArrayToVector(cx, 3, vp, vector);
	ode::dGeomBoxSetLengths(geom, vector[0], vector[1], vector[2]);
	return JS_TRUE;
}

DEFINE_PROPERTY( lengthsGetter ) {

	ode::dGeomID geom = (ode::dGeomID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE( geom );
	ode::dVector3 result;
	ode::dGeomBoxGetLengths(geom, result);
	FloatVectorToArray(cx, 3, result, vp);
	return JS_TRUE;
}

CONFIGURE_CLASS

	HAS_FINALIZE

//	CONSTRUCT_PROTOTYPE

	HAS_PROTOTYPE( prototypeGeom )
	HAS_CONSTRUCTOR

	BEGIN_PROPERTY_SPEC
		PROPERTY( lengths )
	END_PROPERTY_SPEC

	HAS_PRIVATE

END_CLASS
