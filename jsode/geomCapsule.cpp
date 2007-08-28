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
#include "../common/jsNativeInterface.h"

BEGIN_CLASS( GeomCapsule )

DEFINE_FINALIZE() {

	ode::dGeomID geomId = (ode::dGeomID)JS_GetPrivate(cx, obj);
	if ( geomId != NULL )
		ode::dGeomSetData(geomId, NULL);
}

DEFINE_CONSTRUCTOR() {

	RT_ASSERT_CONSTRUCTING(&classGeomCapsule);
	ode::dSpaceID space = 0;
	if ( argc >= 1 ) // place it in a space ?
		RT_CHECK_CALL( ValToSpaceID(cx, argv[0], &space) );
	ode::dGeomID geomId = ode::dCreateCapsule(space, 1, 1); // default radius and length are 1
	JS_SetPrivate(cx, obj, geomId);
	SetupReadMatrix(cx,obj,geomId); // (TBD) check return status
	ode::dGeomSetData(geomId, obj); // 'obj' do not need to be rooted because Goem's data is reset to NULL when 'obj' is finalized.
	return JS_TRUE;
}


DEFINE_PROPERTY( radiusSetter ) {

	ode::dGeomID geom = (ode::dGeomID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE( geom );
	RT_ASSERT_NUMBER( *vp );
	ode::dReal radius, length;
	ode::dGeomCapsuleGetParams(geom, &radius, &length);
	jsdouble value;
	JS_ValueToNumber(cx, *vp, &value);
	ode::dGeomCapsuleSetParams(geom, value, length);
	return JS_TRUE;
}

DEFINE_PROPERTY( radiusGetter ) {

	ode::dGeomID geom = (ode::dGeomID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE( geom );
	ode::dReal radius, length;
	ode::dGeomCapsuleGetParams(geom, &radius, &length);
	JS_NewDoubleValue(cx, radius, vp); // see JS_NewNumberValue and JS_NewDouble
	return JS_TRUE;
}


DEFINE_PROPERTY( lengthSetter ) {

	ode::dGeomID geom = (ode::dGeomID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE( geom );
	RT_ASSERT_NUMBER( *vp );
	ode::dReal radius, length;
	ode::dGeomCapsuleGetParams(geom, &radius, &length);
	jsdouble value;
	JS_ValueToNumber(cx, *vp, &value);
	ode::dGeomCapsuleSetParams(geom, radius, value);
	return JS_TRUE;
}

DEFINE_PROPERTY( lengthGetter ) {

	ode::dGeomID geom = (ode::dGeomID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE( geom );
	ode::dReal radius, length;
	ode::dGeomCapsuleGetParams(geom, &radius, &length);
	JS_NewDoubleValue(cx, length, vp); // see JS_NewNumberValue and JS_NewDouble
	return JS_TRUE;
}


CONFIGURE_CLASS

	HAS_FINALIZE

	HAS_PROTOTYPE( prototypeGeom )
	HAS_CONSTRUCTOR

	BEGIN_PROPERTY_SPEC
		PROPERTY( radius )
		PROPERTY( length )
	END_PROPERTY_SPEC

	HAS_PRIVATE

END_CLASS
