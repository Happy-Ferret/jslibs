#define XP_WIN
#include <jsapi.h>
//#include <jsdbgapi.h>
//#include <jscntxt.h>
//#include <jsscript.h>

#include <nspr.h>

#include "nsprError.h"

JSClass NSPRError_class = { 
  "NSPRError", JSCLASS_HAS_RESERVED_SLOTS(1), 
  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, 
  JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub
};


JSBool NSPRError_getter_code(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	JS_GetReservedSlot( cx, obj, 0, vp );
  return JS_TRUE;
}

JSBool NSPRError_getter_text(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	JS_GetReservedSlot( cx, obj, 0, vp );
	PRErrorCode errorCode = JSVAL_TO_INT(*vp);
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

/*
	const char * filename = NULL;
	uintN lineno;
  for (JSStackFrame *fp = cx->fp; fp; fp = fp->down)
      if (fp->script && fp->pc) {

          filename = fp->script->filename;
          lineno = JS_PCToLineNumber(cx, fp->script, fp->pc);
          break;
      }
	JS_ReportWarning( cx, "ThrowNSPRError %s:%d", filename, lineno );
*/

	JS_ReportWarning( cx, "NSPRError exception" );

	JSObject *error = JS_NewObject( cx, &NSPRError_class, NULL, NULL );
	JS_SetReservedSlot( cx, error, 0, INT_TO_JSVAL(errorCode) );
	JS_SetPendingException( cx, OBJECT_TO_JSVAL( error ) );
  return JS_FALSE;
}

JSObject *InitErrorClass( JSContext *cx, JSObject *obj ) {

	return JS_InitClass( cx, obj, NULL, &NSPRError_class, NSPRError_construct, 0, NSPRError_PropertySpec, NULL, NULL, NULL );
}
