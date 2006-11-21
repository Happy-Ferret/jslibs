#include "stdafx.h"
#include "space.h"
#include "geom.h"

BEGIN_CLASS( GeomSphere )

DEFINE_CONSTRUCTOR() {

	RT_ASSERT_CONSTRUCTING(&classGeomSphere);
	ode::dSpaceID space = 0;
	if ( argc >= 1 ) // place it in a space ?
		if ( ValToSpaceID(cx, argv[0], &space) == JS_FALSE )
			return JS_FALSE;
	ode::dGeomID geomId = ode::dCreateSphere(space, 1); // default radius is 1
	JS_SetPrivate(cx, obj, geomId);
	return JS_TRUE;
}

DEFINE_PROPERTY( radiusSetter ) {
	
	ode::dGeomID geom = (ode::dGeomID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE( geom );
	RT_ASSERT_NUMBER( *vp );
	jsdouble radius;
	JS_ValueToNumber(cx, *vp, &radius);
	ode::dGeomSphereSetRadius(geom, radius);
	return JS_TRUE;
}

DEFINE_PROPERTY( radiusGetter ) {

	ode::dGeomID geom = (ode::dGeomID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE( geom );
	JS_NewDoubleValue(cx, ode::dGeomSphereGetRadius(geom), vp); // see JS_NewNumberValue and JS_NewDouble
	return JS_TRUE;
}


CONFIGURE_CLASS

	HAS_PROTOTYPE( prototypeGeom )
	CONSTRUCT_PROTOTYPE
	HAS_CONSTRUCTOR

	BEGIN_PROPERTY_SPEC
		PROPERTY( radius )
	END_PROPERTY_SPEC

	HAS_PRIVATE

END_CLASS
