#include "stdafx.h"
#include "space.h"
#include "geom.h"
#include "../common/jsNativeInterface.h"

BEGIN_CLASS( GeomCapsule )


DEFINE_CONSTRUCTOR() {

	RT_ASSERT_CONSTRUCTING(&classGeomCapsule);
	ode::dSpaceID space = 0;
	if ( argc >= 1 ) // place it in a space ?
		RT_ASSERT_RETURN( ValToSpaceID(cx, argv[0], &space) )
	ode::dGeomID geomId = ode::dCreateCapsule(space, 1, 1); // default radius and length are 1
	JS_SetPrivate(cx, obj, geomId);
	SetupReadMatrix(cx,obj,geomId); // [TBD] check return status
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

	HAS_PROTOTYPE( prototypeGeom )
	HAS_CONSTRUCTOR

	BEGIN_PROPERTY_SPEC
		PROPERTY( radius )
		PROPERTY( length )
	END_PROPERTY_SPEC

	HAS_PRIVATE

END_CLASS
