#include "stdafx.h"
#include "blob.h"

BEGIN_CLASS( Blob )

DEFINE_CONSTRUCTOR() {

	RT_ASSERT_ARGC(1);
	if ( JS_IsConstructing(cx) == JS_FALSE ) { // supports this form (w/o new operator) : result.param1 = Blob('Hello World');

		obj = JS_NewObject(cx, &classBlob, NULL, NULL);
		RT_ASSERT( obj != NULL, "Blob construction failed." );
		*rval = OBJECT_TO_JSVAL(obj);
	}
	JS_SetReservedSlot(cx, obj, SLOT_BLOB_DATA, argv[0]);
	return JS_TRUE;
}

DEFINE_CONVERT() {

	JS_GetReservedSlot(cx, obj, SLOT_BLOB_DATA, vp); // (TBD) why we cannot use this: if ( type == JSTYPE_STRING )
	return JS_TRUE;
}

CONFIGURE_CLASS

	HAS_CONSTRUCTOR
	HAS_CONVERT
	HAS_RESERVED_SLOTS(1)

END_CLASS
