#include "stdafx.h"
#include "space.h"
#include "geom.h"

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

	RT_ASSERT_CONSTRUCTING(&classGeomPlane);
	ode::dSpaceID space = 0;
	if ( argc >= 1 ) // place it in a space ?
		if ( ValToSpaceID(cx, argv[0], &space) == JS_FALSE )
			return JS_FALSE;
	ode::dGeomID geomId = ode::dCreatePlane(space, 0,0,1,0); // default lengths are 1
	JS_SetPrivate(cx, obj, geomId);
	return JS_TRUE;
}

/*
DEFINE_PROPERTY( lengthsSetter ) {
	
	ode::dGeomID geom = (ode::dGeomID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE( geom );
	//RT_ASSERT_NUMBER( *vp );
	ode::dVector3 vector;
	FloatArrayToVector(cx, 3, vp, vector);
	ode::dGeomPlaneSetLengths(geom, vector[0], vector[1], vector[2]);
	return JS_TRUE;
}

DEFINE_PROPERTY( lengthsGetter ) {

	ode::dGeomID geom = (ode::dGeomID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE( geom );
	ode::dVector3 result;
	ode::dGeomPlaneGetLengths(geom, result);
	FloatVectorToArray(cx, 3, result, vp);
	return JS_TRUE;
}
*/

BEGIN_PROPERTY_MAP
//	READWRITE( lengths )
END_MAP

NO_FUNCTION_MAP
NO_STATIC_FUNCTION_MAP
NO_STATIC_PROPERTY_MAP

PROTOTYPE( classObjectGeom ) //NO_PROTOTYPE

//NO_CLASS_CONSTRUCT
NO_OBJECT_CONSTRUCT
//NO_FINALIZE
NO_CALL
NO_CONSTANT_MAP
NO_INITCLASSAUX

END_CLASS( GeomPlane, HAS_PRIVATE, NO_RESERVED_SLOT )
