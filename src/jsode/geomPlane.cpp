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
BEGIN_CLASS( GeomPlane )

DEFINE_FINALIZE() {

	ode::dGeomID geomId = (ode::dGeomID)JS_GetPrivate(cx, obj);
	if ( geomId != NULL )
		ode::dGeomSetData(geomId, NULL);
}

/**doc
$TOC_MEMBER $INAME
 $INAME( space )
  TBD
**/
DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();
	ode::dSpaceID space = 0;
	if ( argc >= 1 ) // place it in a space ?
		J_CHK( ValToSpaceID(cx, argv[0], &space) );
	ode::dGeomID geomId = ode::dCreatePlane(space, 0,0,1,0); // default lengths are 1
	JS_SetPrivate(cx, obj, geomId);
	SetupReadMatrix(cx, obj); // (TBD) check return status
	ode::dGeomSetData(geomId, obj); // 'obj' do not need to be rooted because Goem's data is reset to NULL when 'obj' is finalized.
	return JS_TRUE;
	JL_BAD;
}

/*
DEFINE_PROPERTY( lengthsSetter ) {

	ode::dGeomID geom = (ode::dGeomID)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( geom );
	//J_S_ASSERT_NUMBER( *vp );
	ode::dVector3 vector;
//	FloatArrayToVector(cx, 3, vp, vector);
	size_t length;
	J_CHK( JsvalToFloatVector(cx, *vp, vector, 3, &length) );
	J_S_ASSERT( length == 3, "Invalid array size." );
	ode::dGeomPlaneSetLengths(geom, vector[0], vector[1], vector[2]);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( lengthsGetter ) {

	ode::dGeomID geom = (ode::dGeomID)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( geom );
	ode::dVector3 result;
	ode::dGeomPlaneGetLengths(geom, result);
	//FloatVectorToArray(cx, 3, result, vp);
	J_CHK( FloatVectorToJsval(cx, result, 3, vp) );
	return JS_TRUE;
	JL_BAD;
}
*/


CONFIGURE_CLASS

	REVISION(SvnRevToInt("$Revision$"))
	HAS_FINALIZE

	HAS_PROTOTYPE( prototypeGeom )
	HAS_CONSTRUCTOR

	HAS_PRIVATE

END_CLASS
