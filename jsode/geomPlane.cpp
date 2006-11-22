#include "stdafx.h"
#include "space.h"
#include "geom.h"
#include "../common/jsNativeInterface.h"

BEGIN_CLASS( GeomPlane )


DEFINE_CONSTRUCTOR() {

	RT_ASSERT_CONSTRUCTING(&classGeomPlane);
	ode::dSpaceID space = 0;
	if ( argc >= 1 ) // place it in a space ?
		if ( ValToSpaceID(cx, argv[0], &space) == JS_FALSE )
			return JS_FALSE;
	ode::dGeomID geomId = ode::dCreatePlane(space, 0,0,1,0); // default lengths are 1
	JS_SetPrivate(cx, obj, geomId);
	SetupReadMatrix(cx,obj,geomId); // [TBD] check return status
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


CONFIGURE_CLASS

	HAS_PROTOTYPE( prototypeGeom )
	HAS_CONSTRUCTOR

	HAS_PRIVATE

END_CLASS
