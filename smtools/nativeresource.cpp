#include "stdafx.h"
#include "smtools.h"
#include "nativeresource.h"

#define NATIVE_RESOURCE_PRIVATE "_nativeResourcePrivate"
#define NATIVE_RESOURCE_READ_FUNCTION "_nativeResourceReadFunction"
#define NATIVE_RESOURCE_WRITE_FUNCTION "_nativeResourceWriteFunction"

#define PROPERTIES_FLAGS JSPROP_READONLY | JSPROP_PERMANENT

JSBool SetNativeResource( JSContext *cx, JSObject *obj, void *pv, NativeResourceFunction read, NativeResourceFunction write ) {

	if ( JS_DefineProperty(cx, obj, NATIVE_RESOURCE_PRIVATE, PRIVATE_TO_JSVAL(pv), NULL, NULL, PROPERTIES_FLAGS ) == JS_FALSE )
		return JS_FALSE;
	if ( JS_DefineProperty(cx, obj, NATIVE_RESOURCE_READ_FUNCTION, PRIVATE_TO_JSVAL(read), NULL, NULL, PROPERTIES_FLAGS ) == JS_FALSE )
		return JS_FALSE;
	if ( JS_DefineProperty(cx, obj, NATIVE_RESOURCE_WRITE_FUNCTION, PRIVATE_TO_JSVAL(write), NULL, NULL, PROPERTIES_FLAGS ) == JS_FALSE )
		return JS_FALSE;
	return JS_TRUE;
}


JSBool GetNativeResource( JSContext *cx, JSObject *obj, void **pv, NativeResourceFunction *read, NativeResourceFunction *write ) {

	jsval tmp;
	if ( pv ) {
		JS_GetProperty(cx, obj, NATIVE_RESOURCE_PRIVATE, &tmp);
		if ( tmp == JSVAL_VOID )
			return JS_FALSE;
		*pv = JSVAL_TO_PRIVATE(tmp);
	}
	if ( read ) {
		JS_GetProperty(cx, obj, NATIVE_RESOURCE_READ_FUNCTION, &tmp);
		if ( tmp == JSVAL_VOID )
			return JS_FALSE;
		*read = (NativeResourceFunction)JSVAL_TO_PRIVATE(tmp);
	}
	if ( write ) {
		JS_GetProperty(cx, obj, NATIVE_RESOURCE_WRITE_FUNCTION, &tmp);
		if ( tmp == JSVAL_VOID )
			return JS_FALSE;
		*write = (NativeResourceFunction)JSVAL_TO_PRIVATE(tmp);
	}
	return JS_TRUE;
}
