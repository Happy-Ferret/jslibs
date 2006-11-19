//#include "stdafx.h"

#include <windows.h>

#define XP_WIN
#include <jsapi.h>

#include "../common/jsclass.h"
#include "jstest.h"

// =====================================================

BEGIN_CLASS(Toto);

	DEFINE_FUNCTION( Test ) {

		return JS_TRUE;
	}

	DEFINE_FUNCTION( Constructor ) {

		return JS_TRUE;
	}



CONFIGURE_CLASS

	HAS_PRIVATE
	HAS_RESERVED_SLOTS(2)
	HAS_CONSTRUCTOR

	BEGIN_FUNCTION_SPEC
		FUNCTION(Test)
	END_FUNCTION_SPEC

END_CLASS;


// = JSCLASS_CONSTRUCT_PROTOTYPE;


/*

static JSBool _InitClass(JSContext *cx, JSObject *obj) {

	// JSCLASS_CONSTRUCT_PROTOTYPE |
	privateSlot | JSCLASS_HAS_RESERVED_SLOTS(reservedSlotCount)
	*_class = { #className, _classFlags, JS_PropertyStub , JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_EnumerateStub, JS_ResolveStub , JS_ConvertStub , Finalize, 0, 0, Call, ObjectConstruct };
	*_proto = JS_InitClass( cx, obj, *_parentProto, thisClass, ClassConstruct, 0, _propertyMap, _functionMap, _propertyStaticMap, _functionStaticMap );

	RT_ASSERT_1( *_proto != NULL, "Unable to InitClass %s", thisClass->name );
	if ( _constantMap != NULL )
		if ( JS_DefineConstDoubles( cx, thisClassObject, _constantMap ) == JS_FALSE )
			return JS_FALSE;
	InitClassAux(cx, obj);

}

*/