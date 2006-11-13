/*
Collision callback:
	http://opende.sourceforge.net/wiki/index.php/Collision_callback_member_function

*/

#include "stdafx.h"
#include "body.h"
#include "geom.h"


BEGIN_CLASS

DEFINE_PROPERTY( body ) {

	// [TBD] check if the obj's private data is the right body. else ERROR
	//ode::dGeomID geom = (ode::dGeomID)JS_GetPrivate(cx, obj);
	//RT_ASSERT_RESOURCE( geom );
	//ode::dBodyID bodyId = dGeomGetBody(geom);

	if ( *vp == JSVAL_VOID ) {

		ode::dGeomID geom = (ode::dGeomID)JS_GetPrivate(cx, obj);
		RT_ASSERT_RESOURCE( geom );
		ode::dBodyID bodyId;
		if ( ValToBodyID(cx, *vp, &bodyId) == JS_FALSE )
			return JS_FALSE;
		ode::dGeomSetBody(geom, bodyId);
	}
	return JS_TRUE;
}


DEFINE_FUNCTION( Destroy ) {

	ode::dGeomID geom = (ode::dGeomID)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE( geom );
	ode::dGeomDestroy(geom);
	JS_SetPrivate(cx, obj, NULL);
	return JS_TRUE;
}


BEGIN_FUNCTION_MAP
	FUNCTION( Destroy )
END_MAP

BEGIN_PROPERTY_MAP
	SET_AND_STORE( body )
END_MAP


NO_STATIC_FUNCTION_MAP
NO_STATIC_PROPERTY_MAP

NO_CLASS_CONSTRUCT // the aim of this class is not to be construct, it is to be a prototype class
NO_OBJECT_CONSTRUCT
NO_FINALIZE
NO_CALL
NO_PROTOTYPE
NO_CONSTANT_MAP
NO_INITCLASSAUX

END_CLASS( Geom, NO_PRIVATE, NO_RESERVED_SLOT)

