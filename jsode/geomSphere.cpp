#include "stdafx.h"
#include "space.h"
#include "geom.h"
#include "geomSphere.h"

BEGIN_CLASS

DEFINE_FINALIZE() {
	 // [TBD] manage destruction dependency: geom - space, body - world, but how to ?

	// read the next comment !!
//	ode::dGeomID GeomId = (ode::dGeomID)JS_GetPrivate(cx,obj);
//	if ( GeomId != NULL )
//		ode::dGeomDestroy(GeomId);

	// [TBD] really destroy the geom when finalise ? think that even if there is no more references to this object, 
	//       the geom remain managed by ODE, so what to do ??? ...
	//       in the case we use geom as a property of Body ??
}

DEFINE_FUNCTION( ClassConstruct ) {

	RT_ASSERT_CONSTRUCTING(&classGeomSphere);
	ode::dSpaceID space = 0;
	if ( argc >= 1 )
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

BEGIN_FUNCTION_MAP
END_MAP

BEGIN_PROPERTY_MAP
	READWRITE( radius )
END_MAP

NO_STATIC_FUNCTION_MAP
NO_STATIC_PROPERTY_MAP

PROTOTYPE( classObjectGeom ) //NO_PROTOTYPE

//NO_CLASS_CONSTRUCT
NO_OBJECT_CONSTRUCT
//NO_FINALIZE
NO_CALL
NO_CONSTANT_MAP
NO_INITCLASSAUX

END_CLASS( GeomSphere, HAS_PRIVATE, NO_RESERVED_SLOT )
