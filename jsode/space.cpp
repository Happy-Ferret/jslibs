/*

check this:
	void dSpaceSetCleanup (dSpaceID space, int mode);
	int dSpaceGetCleanup (dSpaceID space);
*/

#include "stdafx.h"
#include "space.h"

BEGIN_CLASS


/* This class cannot have a Finalize ( see readme.txt )
DEFINE_FINALIZE() {

	ode::dSpaceID spaceId = (ode::dSpaceID)JS_GetPrivate(cx,obj);
	if ( spaceId != NULL )
		ode::dSpaceDestroy(spaceId);
}
*/



DEFINE_FUNCTION( ClassConstruct ) {

	RT_ASSERT_CONSTRUCTING(&classSpace);
	ode::dSpaceID parentSpace = 0;
	if ( argc >= 1 )
		if ( ValToSpaceID(cx, argv[0], &parentSpace) == JS_FALSE )
			return JS_FALSE;
	JS_SetPrivate(cx, obj, ode::dHashSpaceCreate(parentSpace)); // dSimpleSpaceCreate
	return JS_TRUE;
}


BEGIN_FUNCTION_MAP
END_MAP

BEGIN_PROPERTY_MAP
END_MAP


NO_STATIC_FUNCTION_MAP
NO_STATIC_PROPERTY_MAP

//NO_CLASS_CONSTRUCT
NO_OBJECT_CONSTRUCT
NO_FINALIZE
NO_CALL
NO_PROTOTYPE
NO_CONSTANT_MAP
NO_INITCLASSAUX

END_CLASS( Space, HAS_PRIVATE, NO_RESERVED_SLOT)