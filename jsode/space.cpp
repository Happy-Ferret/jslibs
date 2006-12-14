/*

check this:
	void dSpaceSetCleanup (dSpaceID space, int mode);
	int dSpaceGetCleanup (dSpaceID space);
*/

#include "stdafx.h"
#include "space.h"

BEGIN_CLASS( Space )


/* This class cannot have a Finalize ( see readme.txt )
DEFINE_FINALIZE() {

	ode::dSpaceID spaceId = (ode::dSpaceID)JS_GetPrivate(cx,obj);
	if ( spaceId != NULL )
		ode::dSpaceDestroy(spaceId);
}
*/



DEFINE_CONSTRUCTOR() {

	RT_ASSERT_CONSTRUCTING(&classSpace);
	ode::dSpaceID parentSpace = 0;
	if ( argc >= 1 )
		if ( ValToSpaceID(cx, argv[0], &parentSpace) == JS_FALSE )
			return JS_FALSE;
	ode::dSpaceID spaceId = ode::dSimpleSpaceCreate(parentSpace);
	JS_SetPrivate(cx, obj, spaceId); // dSimpleSpaceCreate / dHashSpaceCreate / dQuadTreeSpaceCreate
	// ode::dHashSpaceSetLevels(spaceId, 
	// (TBD) use this
	return JS_TRUE;
}


CONFIGURE_CLASS

	HAS_CONSTRUCTOR
//	HAS_FINALIZE
	HAS_PRIVATE

END_CLASS
