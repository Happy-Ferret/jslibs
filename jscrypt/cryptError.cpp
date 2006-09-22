#include "stdafx.h"

#define XP_WIN
#include <jsapi.h>

#include <tomcrypt.h>

#include "cryptError.h"

JSClass CryptError_class = { 
  "CryptError", JSCLASS_HAS_RESERVED_SLOTS(1), 
  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, 
  JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub
};


JSBool CryptError_getter_code(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	JS_GetReservedSlot( cx, obj, 0, vp );

	return JS_TRUE;
}

JSBool CryptError_getter_text(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	JS_GetReservedSlot( cx, obj, 0, vp );
	int errorCode = JSVAL_TO_INT(*vp);

	JSString *str = JS_NewStringCopyZ( cx, error_to_string(errorCode) );
	*vp = STRING_TO_JSVAL( str );

	return JS_TRUE;
}

JSPropertySpec CryptError_PropertySpec[] = { // *name, tinyid, flags, getter, setter
  { "code",  0, JSPROP_READONLY|JSPROP_PERMANENT, CryptError_getter_code, NULL },
  { "text",  0, JSPROP_READONLY|JSPROP_PERMANENT, CryptError_getter_text, NULL },
  { 0 }
};


JSBool CryptError_construct(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	return JS_TRUE;
}


JSBool ThrowCryptError( JSContext *cx, int errorCode ) {

	JS_ReportWarning( cx, "CryptError exception" );

	JSObject *error = JS_NewObject( cx, &CryptError_class, NULL, NULL );

	JS_SetReservedSlot( cx, error, 0, INT_TO_JSVAL(errorCode) );
//	JS_SetReservedSlot( cx, error, 1, errorMessage != NULL ? STRING_TO_JSVAL(JS_NewStringCopyZ( cx, errorMessage )) : JSVAL_VOID );

	JS_SetPendingException( cx, OBJECT_TO_JSVAL( error ) );
  return JS_FALSE;
}

JSObject *InitErrorClass( JSContext *cx, JSObject *obj ) {

	return JS_InitClass( cx, obj, NULL, &CryptError_class, CryptError_construct, 0, CryptError_PropertySpec, NULL, NULL, NULL );
}
