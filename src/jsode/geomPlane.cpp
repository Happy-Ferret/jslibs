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

	FinalizeGeom(cx, obj);
}

/**doc
$TOC_MEMBER $INAME
 $INAME( [ space ] )
  TBD
**/
DEFINE_CONSTRUCTOR() {

	JL_S_ASSERT_CONSTRUCTING();
	JL_S_ASSERT_THIS_CLASS();
	JL_S_ASSERT_ARG_RANGE(0, 1);
	ode::dSpaceID space;
	if ( JL_ARG_ISDEF(1) ) // place it in a space ?
		JL_CHK( JsvalToSpaceID(cx, JL_ARG(1), &space) );
	else
		space = 0;
	ode::dGeomID geomId = ode::dCreatePlane(space, 0,0,1,0); // default lengths are 1
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
**/
DEFINE_PROPERTY( paramsSetter ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( geom );
	JL_S_ASSERT_ARRAY( *vp );
	ode::dVector4 params;
	uint32 length;
	JL_CHK( JsvalToFloatVector(cx, *vp, params, 4, &length) );
	JL_S_ASSERT( length >= 4, "Invalid array size." );
	ode::dGeomPlaneSetParams(geom, params[0], params[1], params[2], params[3]);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( paramsGetter ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( geom );
	ode::dVector4 result;
	ode::dGeomPlaneGetParams(geom, result);
	JL_CHK( FloatVectorToJsval(cx, result, COUNTOF(result), vp) );
	return JS_TRUE;
	JL_BAD;
}




/*
DEFINE_PROPERTY( lengthsSetter ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( geom );
	//JL_S_ASSERT_NUMBER( *vp );
	ode::dVector3 vector;
//	FloatArrayToVector(cx, 3, vp, vector);
	size_t length;
	JL_CHK( JsvalToFloatVector(cx, *vp, vector, 3, &length) );
	JL_S_ASSERT( length >= 3, "Invalid array size." );
	ode::dGeomPlaneSetLengths(geom, vector[0], vector[1], vector[2]);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( lengthsGetter ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( geom );
	ode::dVector3 result;
	ode::dGeomPlaneGetLengths(geom, result);
	//FloatVectorToArray(cx, 3, result, vp);
	JL_CHK( FloatVectorToJsval(cx, result, 3, vp) );
	return JS_TRUE;
	JL_BAD;
}
*/


CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_PROTOTYPE( prototypeGeom )
	HAS_CONSTRUCTOR
	HAS_FINALIZE
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(2)

	BEGIN_PROPERTY_SPEC
		PROPERTY( params )
	END_PROPERTY_SPEC

END_CLASS
