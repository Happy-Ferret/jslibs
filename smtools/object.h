bool IsInstanceOf( JSContext *cx, JSObject *obj, JSClass *clasp );
JSBool GetNamedPrivate( JSContext *cx, JSObject *obj, const char *name, void **pv );
JSBool SetNamedPrivate( JSContext *cx, JSObject *obj, const char *name, const void *pv );

inline JSBool GetIntProperty( JSContext *cx, JSObject *obj, const char *propertyName, int *value ) {

	jsval tmp;
	int32 int32Value;
	if ( JS_GetProperty(cx, obj, propertyName, &tmp) == JS_FALSE )
		return JS_FALSE;
	JS_ValueToInt32(cx, tmp, &int32Value);
	*value = int32Value;
	return JS_TRUE;
}
