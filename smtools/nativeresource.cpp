#include "stdafx.h"
#include "smtools.h"
#include "nativeresource.h"

#define NATIVE_RESOURCE_PRIVATE "_nativeResourcePrivate"
#define NATIVE_RESOURCE_READ_FUNCTION "_nativeResourceReadFunction"
#define NATIVE_RESOURCE_WRITE_FUNCTION "_nativeResourceWriteFunction"

JSBool SetNativeResource( JSContext *cx, JSObject *obj, void *pv, NativeResourceFunction read, NativeResourceFunction write ) {

	JSBool status;
	jsval tmp;
	tmp = PRIVATE_TO_JSVAL(pv);
	status = JS_SetProperty(cx, obj, NATIVE_RESOURCE_PRIVATE, &tmp); // [TBD] set hidden + readonly
	if ( status == JS_FALSE )
			return JS_FALSE;
	tmp = PRIVATE_TO_JSVAL(read);
	status = JS_SetProperty(cx, obj, NATIVE_RESOURCE_READ_FUNCTION, &tmp); // [TBD] set hidden + readonly
	if ( status == JS_FALSE )
			return JS_FALSE;
	tmp = PRIVATE_TO_JSVAL(write);
	status = JS_SetProperty(cx, obj, NATIVE_RESOURCE_WRITE_FUNCTION, &tmp); // [TBD] set hidden + readonly
	if ( status == JS_FALSE )
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
