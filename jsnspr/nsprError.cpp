#define XP_WIN
#include <jsapi.h>
#include <nspr.h>

#include "nsprError.h"

JSClass NSPRError_class = { 
  "NSPRError", JSCLASS_HAS_PRIVATE, 
  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, 
  JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub
};


JSBool NSPRError_getter_code(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	PRErrorCode errorCode = (PRErrorCode)JS_GetPrivate( cx, obj );
	*vp = INT_TO_JSVAL( errorCode ); //	JS_NewNumberValue( cx, 
  return JS_TRUE;
}

JSBool NSPRError_getter_text(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	PRErrorCode errorCode = (PRErrorCode)JS_GetPrivate( cx, obj );

	JSString *str = JS_NewStringCopyZ( cx, PR_ErrorToString( errorCode, PR_LANGUAGE_EN ) );
	
	*vp = STRING_TO_JSVAL( str );
  return JS_TRUE;
}


JSPropertySpec NSPRError_PropertySpec[] = { // *name, tinyid, flags, getter, setter
  { "code",  0, JSPROP_READONLY|JSPROP_PERMANENT, NSPRError_getter_code, NULL },
  { "text",  0, JSPROP_READONLY|JSPROP_PERMANENT, NSPRError_getter_text, NULL },
  { 0 }
};

JSBool NSPRError_construct(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	return JS_TRUE;
}


JSBool ThrowNSPRError( JSContext *cx, PRErrorCode errorCode ) {

	JS_ReportError( cx, "error" );	return JS_FALSE;

	JSObject *error = JS_NewObject( cx, &NSPRError_class, NULL, NULL );
	JS_SetPrivate( cx, error, (void*)errorCode );
	JS_SetPendingException( cx, OBJECT_TO_JSVAL( error ) );
  return JS_FALSE;
}

JSObject *InitErrorClass( JSContext *cx, JSObject *obj ) {

	return JS_InitClass( cx, obj, NULL, &NSPRError_class, NSPRError_construct, 0, NSPRError_PropertySpec, NULL, NULL, NULL );
}
