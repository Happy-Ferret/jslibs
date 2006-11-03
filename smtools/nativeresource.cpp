#include "stdafx.h"
#include "smtools.h"
#include "nativeresource.h"

/*
#define PROPERTIES_FLAGS JSPROP_READONLY | JSPROP_PERMANENT

JSBool SetNativeResource( JSContext *cx, JSObject *obj, void *pv, NativeResourceFunction read, NativeResourceFunction write ) {

	if ( JS_DefineProperty(cx, obj, NATIVE_RESOURCE_PRIVATE_STRING, PRIVATE_TO_JSVAL(pv), NULL, NULL, PROPERTIES_FLAGS ) == JS_FALSE )
		return JS_FALSE;
	if ( JS_DefineProperty(cx, obj, NATIVE_RESOURCE_READ_FUNCTION_STRING, PRIVATE_TO_JSVAL(read), NULL, NULL, PROPERTIES_FLAGS ) == JS_FALSE )
		return JS_FALSE;
	if ( JS_DefineProperty(cx, obj, NATIVE_RESOURCE_WRITE_FUNCTION_STRING, PRIVATE_TO_JSVAL(write), NULL, NULL, PROPERTIES_FLAGS ) == JS_FALSE )
		return JS_FALSE;
	return JS_TRUE;
}


JSBool GetNativeResource( JSContext *cx, JSObject *obj, void **pv, NativeResourceFunction *read, NativeResourceFunction *write ) {

	jsval tmp;
	if ( pv ) {
		JS_GetProperty(cx, obj, NATIVE_RESOURCE_PRIVATE_STRING, &tmp);
		if ( tmp == JSVAL_VOID )
			return JS_FALSE;
		*pv = JSVAL_TO_PRIVATE(tmp);
	}
	if ( read ) {
		JS_GetProperty(cx, obj, NATIVE_RESOURCE_READ_FUNCTION_STRING, &tmp);
		if ( tmp == JSVAL_VOID )
			return JS_FALSE;
		*read = (NativeResourceFunction)JSVAL_TO_PRIVATE(tmp);
	}
	if ( write ) {
		JS_GetProperty(cx, obj, NATIVE_RESOURCE_WRITE_FUNCTION_STRING, &tmp);
		if ( tmp == JSVAL_VOID )
			return JS_FALSE;
		*write = (NativeResourceFunction)JSVAL_TO_PRIVATE(tmp);
	}
	return JS_TRUE;
}

void UseObjectReadFunction( void *pv, unsigned char *data, unsigned int *length ) {


}
*/