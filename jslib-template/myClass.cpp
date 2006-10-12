#include "stdafx.h"

#define XP_WIN
#include <jsapi.h>
#include "../common/jshelper.h"

#include "myClass.h"

BEGIN_CLASS

void Finalize(JSContext *cx, JSObject *obj) {
}

JSBool Construct(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	if ( !JS_IsConstructing(cx) || JS_GetClass(obj) != thisClass ) {
		// error
		return JS_FALSE;
	}
	return JS_TRUE;
}

JSBool Call(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	return JS_TRUE;
}


JSBool Property(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
  return JS_TRUE;
}

JSBool Function(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  return JS_TRUE;
}


BEGIN_FUNCTION_MAP
	FUNCTION(Function)
END_MAP
BEGIN_PROPERTY_MAP
	READONLY(Property)
END_MAP
BEGIN_STATIC_FUNCTION_MAP
END_MAP
BEGIN_STATIC_PROPERTY_MAP
END_MAP
//	NO_CONSTRUCT
//	NO_FINALIZE
//	NO_CALL
NO_PROTOTYPE

END_CLASS(Test, NO_RESERVED_SLOTS)
