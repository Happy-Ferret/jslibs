typedef void (*NativeResourceFunction)( void *pv, unsigned char *data, unsigned int *length );

// setup NativeResourceRead system:
//   this system is a shortcut to read datas from this resource

JSBool SetNativeResource( JSContext *cx, JSObject *obj, void *pv, NativeResourceFunction read, NativeResourceFunction write );
JSBool GetNativeResource( JSContext *cx, JSObject *obj, void **pv, NativeResourceFunction *read, NativeResourceFunction *write );


