#include "stdafx.h"
#include "jsimage.h"

#include <stdio.h>

BEGIN_CLASS

	static void Finalize(JSContext *cx, JSObject *obj) {
	}

	static JSBool ClassConstruct(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

		RT_ASSERT_CONSTRUCTING(thisClass);
		RT_ASSERT_ARGC(1);



		return JS_TRUE;
	}

//	JSBool Call(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
//		return JS_TRUE;
//	}

//	JSBool prop(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
//		return JS_TRUE;
//	}

//	JSBool Func(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
//		return JS_TRUE;
//	}


	BEGIN_FUNCTION_MAP
//		FUNCTION(Func)
	END_MAP
	BEGIN_PROPERTY_MAP
//		READONLY(prop)
	END_MAP
	NO_STATIC_FUNCTION_MAP
	//BEGIN_STATIC_FUNCTION_MAP
	//END_MAP
	NO_STATIC_PROPERTY_MAP
	//BEGIN_STATIC_PROPERTY_MAP
	//END_MAP
//	NO_CLASS_CONSTRUCT
	NO_OBJECT_CONSTRUCT
//	NO_FINALIZE
	NO_CALL
	NO_PROTOTYPE
	NO_CONSTANT_MAP
	NO_INITCLASSAUX

END_CLASS(Png, NO_PRIVATE, NO_RESERVED_SLOT)

