#include "stdafx.h"

#define XP_WIN
#include <jsapi.h>

#include "myClassClass.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void myClass_Finalize(JSContext *cx, JSObject *obj) {

}


JSClass myClass_class = {
  "myClass", JSCLASS_HAS_PRIVATE,
  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
  JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, myClass_Finalize
};


JSBool myClass_construct(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

//	if ( !JS_IsConstructing(cx) ) { ... return JS_FALSE; }
	return JS_TRUE;
}


JSBool myClass_myFunction(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	return JS_TRUE;
}


JSFunctionSpec myClass_FunctionSpec[] = { // *name, call, nargs, flags, extra
 { "myFunction"     , myClass_myFunction     , 0, 0, 0 },
 { 0 }
};


JSBool myClass_getter_myProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

  return JS_TRUE;
}


JSPropertySpec myClass_PropertySpec[] = { // *name, tinyid, flags, getter, setter
	{ "myProperty" , 0, JSPROP_PERMANENT|JSPROP_READONLY, myClass_getter_myProperty, NULL },
  { 0 }
};


JSBool myClass_static_getter_myStatic(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

  return JS_TRUE;
}


JSPropertySpec myClass_static_PropertySpec[] = { // *name, tinyid, flags, getter, setter
	{ "myStatic" , 0, JSPROP_PERMANENT|JSPROP_READONLY, myClass_static_getter_myStatic, NULL },
  { 0 }
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


JSObject *myClassInitClass( JSContext *cx, JSObject *obj ) {

	return JS_InitClass( cx, obj, NULL, &myClass_class, myClass_construct, 1, myClass_PropertySpec, myClass_FunctionSpec, myClass_static_PropertySpec, NULL );
}
