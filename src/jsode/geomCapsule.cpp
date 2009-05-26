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
BEGIN_CLASS( GeomCapsule )

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
	ode::dGeomID geomId = ode::dCreateCapsule(space, 1, 1); // default radius and length are 1
	JL_SetPrivate(cx, obj, geomId);
	SetupReadMatrix(cx, obj); // (TBD) check return status
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
	ode::dGeomCapsuleGetParams(geom, &radius, &length);
	jsdouble value;
	JS_ValueToNumber(cx, *vp, &value);
	ode::dGeomCapsuleSetParams(geom, value, length);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( radiusGetter ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( geom );
	ode::dReal radius, length;
	ode::dGeomCapsuleGetParams(geom, &radius, &length);
	JS_NewDoubleValue(cx, radius, vp); // see JS_NewNumberValue and JS_NewDouble
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $REAL $INAME
  Is the length of the capsule.
**/
DEFINE_PROPERTY( lengthSetter ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( geom );
	JL_S_ASSERT_NUMBER( *vp );
	ode::dReal radius, length;
	ode::dGeomCapsuleGetParams(geom, &radius, &length);
	jsdouble value;
	JS_ValueToNumber(cx, *vp, &value);
	ode::dGeomCapsuleSetParams(geom, radius, value);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( lengthGetter ) {

	ode::dGeomID geom = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( geom );
	ode::dReal radius, length;
	ode::dGeomCapsuleGetParams(geom, &radius, &length);
	JS_NewDoubleValue(cx, length, vp); // see JS_NewNumberValue and JS_NewDouble
	return JS_TRUE;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(SvnRevToInt("$Revision$"))
	HAS_FINALIZE

	HAS_PROTOTYPE( prototypeGeom )
	HAS_CONSTRUCTOR

	BEGIN_PROPERTY_SPEC
		PROPERTY( radius )
		PROPERTY( length )
	END_PROPERTY_SPEC

	HAS_PRIVATE

END_CLASS
