#include "stdafx.h"
#include "space.h"
#include "geom.h"

BEGIN_CLASS( GeomBox )

DEFINE_CONSTRUCTOR() {

	RT_ASSERT_CONSTRUCTING(&classGeomBox);
	ode::dSpaceID space = 0;
	if ( argc >= 1 ) // place it in a space ?
		if ( ValToSpaceID(cx, argv[0], &space) == JS_FALSE )
			return JS_FALSE;
	ode::dGeomID geomId = ode::dCreateBox(space, 1,1,1); // default lengths are 1
	JS_SetPrivate(cx, obj, geomId);
	return JS_TRUE;
}

DEFINE_PROPERTY( lengthsSetter ) {
	
	ode::dGeomID geom = (ode::dGeomID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE( geom );
	//RT_ASSERT_NUMBER( *vp );
	ode::dVector3 vector;
	FloatArrayToVector(cx, 3, vp, vector);
	ode::dGeomBoxSetLengths(geom, vector[0], vector[1], vector[2]);
	return JS_TRUE;
}

DEFINE_PROPERTY( lengthsGetter ) {

	ode::dGeomID geom = (ode::dGeomID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE( geom );
	ode::dVector3 result;
	ode::dGeomBoxGetLengths(geom, result);
	FloatVectorToArray(cx, 3, result, vp);
	return JS_TRUE;
}

CONFIGURE_CLASS

	HAS_PROTOTYPE( prototypeGeom )
//	CONSTRUCT_PROTOTYPE
	HAS_CONSTRUCTOR

	BEGIN_PROPERTY_SPEC
		PROPERTY( lengths )
	END_PROPERTY_SPEC

	HAS_PRIVATE

END_CLASS
