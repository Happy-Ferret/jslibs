/*
Collision callback:
	http://opende.sourceforge.net/wiki/index.php/Collision_callback_member_function

*/

#include "stdafx.h"
#include "geom.h"

BEGIN_CLASS


DEFINE_FINALIZE() {}


//DEFINE_FUNCTION( ClassConstruct ) {
//	return JS_TRUE;
//}


DEFINE_PROPERTY( bodySetter ) {

	void dGeomSetBody(dGeomID, dBodyID);

	return JS_TRUE;
}


DEFINE_PROPERTY( bodyGetter ) {

	void dGeomGetBody(dGeomID, dBodyID);

	return JS_TRUE;
}



BEGIN_FUNCTION_MAP
END_MAP

BEGIN_PROPERTY_MAP
	READWRITE( body )
END_MAP


NO_STATIC_FUNCTION_MAP
NO_STATIC_PROPERTY_MAP

NO_CLASS_CONSTRUCT // the aim of this class is not to be construct, it is to be a prototype class
NO_OBJECT_CONSTRUCT
//NO_FINALIZE
NO_CALL
NO_PROTOTYPE
NO_CONSTANT_MAP
NO_INITCLASSAUX

END_CLASS( Geom, NO_PRIVATE, NO_RESERVED_SLOT)

