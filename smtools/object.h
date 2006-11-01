bool IsInstanceOf( JSContext *cx, JSObject *obj, JSClass *clasp );
JSBool GetNamedPrivate( JSContext *cx, JSObject *obj, const char *name, void **pv );
JSBool SetNamedPrivate( JSContext *cx, JSObject *obj, const char *name, const void *pv );

