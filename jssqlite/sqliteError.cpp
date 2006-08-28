#include "stdafx.h"
#define XP_WIN
#include <jsapi.h>

#include "SqliteError.h"

JSClass SqliteError_class = { 
  "SqliteError", JSCLASS_HAS_PRIVATE, 
  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, 
  JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub
};


JSBool SqliteError_getter_code(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

  return JS_TRUE;
}

JSBool SqliteError_getter_text(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

//	sqlite3ErrStr(1);

  return JS_TRUE;
}


JSPropertySpec SqliteError_PropertySpec[] = { // *name, tinyid, flags, getter, setter
  { "code",  0, JSPROP_READONLY|JSPROP_PERMANENT, SqliteError_getter_code, NULL },
  { "text",  0, JSPROP_READONLY|JSPROP_PERMANENT, SqliteError_getter_text, NULL },
  { 0 }
};

JSBool SqliteError_construct(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	return JS_TRUE;
}


JSBool SqliteThrowError( JSContext *cx, int errorCode ) {

	JS_ReportError( cx, "error" );	return JS_FALSE;

	JSObject *error = JS_NewObject( cx, &SqliteError_class, NULL, NULL );
	JS_SetPrivate( cx, error, (void*)errorCode );
	JS_SetPendingException( cx, OBJECT_TO_JSVAL( error ) );
  return JS_FALSE;
}

JSObject *SqliteInitErrorClass( JSContext *cx, JSObject *obj ) {

	return JS_InitClass( cx, obj, NULL, &SqliteError_class, SqliteError_construct, 0, SqliteError_PropertySpec, NULL, NULL, NULL );
}
