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
$SVN_REVISION $Revision: 2614 $
**/
BEGIN_CLASS( GeomCylinder )

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
	ode::dGeomID geomId = ode::dCreateCylinder(space, 1, 1); // default radius and length are 1
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
DEFINE_PROPERTY( radiusSetter ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( geom );
	JL_S_ASSERT_NUMBER( *vp );
	ode::dReal radius, length;
	ode::dGeomCylinderGetParams(geom, &radius, &length);
	jsdouble value;
	JS_ValueToNumber(cx, *vp, &value);
	ode::dGeomCylinderSetParams(geom, value, length);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( radiusGetter ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( geom );
	ode::dReal radius, length;
	ode::dGeomCylinderGetParams(geom, &radius, &length);
	JS_NewDoubleValue(cx, radius, vp); // see JS_NewNumberValue and JS_NewDouble
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $REAL $INAME
  Is the length of the Cylinder.
**/
DEFINE_PROPERTY( lengthSetter ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( geom );
	JL_S_ASSERT_NUMBER( *vp );
	ode::dReal radius, length;
	ode::dGeomCylinderGetParams(geom, &radius, &length);
	jsdouble value;
	JS_ValueToNumber(cx, *vp, &value);
	ode::dGeomCylinderSetParams(geom, radius, value);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( lengthGetter ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( geom );
	ode::dReal radius, length;
	ode::dGeomCylinderGetParams(geom, &radius, &length);
	JS_NewDoubleValue(cx, length, vp); // see JS_NewNumberValue and JS_NewDouble
	return JS_TRUE;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision: 2614 $"))
	HAS_PROTOTYPE( prototypeGeom )
	HAS_CONSTRUCTOR
	HAS_FINALIZE
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(2)

	BEGIN_PROPERTY_SPEC
		PROPERTY( radius )
		PROPERTY( length )
	END_PROPERTY_SPEC

END_CLASS