typedef bool (*NativeResourceFunction)( void *pv, unsigned char *data, unsigned int *length );

// setup NativeResourceRead system:
//   this system is a shortcut to read datas from this resource

//JSBool SetNativeResource( JSContext *cx, JSObject *obj, void *pv, NativeResourceFunction read, NativeResourceFunction write );
//JSBool GetNativeResource( JSContext *cx, JSObject *obj, void **pv, NativeResourceFunction *read, NativeResourceFunction *write );

#define NATIVE_RESOURCE_PRIVATE_STRING "_nativeResourcePrivate"
#define NATIVE_RESOURCE_READ_FUNCTION_STRING "_nativeResourceReadFunction"
#define NATIVE_RESOURCE_WRITE_FUNCTION_STRING "_nativeResourceWriteFunction"
