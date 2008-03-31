#ifndef _JSLANGAPI_H_
#define _JSLANGAPI_H_


inline JSClass* BStringJSClass( JSContext *cx ) {

	static JSClass *jsClass = NULL;
	if ( jsClass == NULL )
		jsClass = GetClassByName(cx, "BString");
	return jsClass;
}



// NewBString takes ownership of jsMallocatedBuffer on success. Allocation must be done with JS_malloc
inline JSObject* NewBString( JSContext *cx, char *jsMallocatedBuffer, size_t bufferLength ) {
		
	JSClass *bstringClass = BStringJSClass(cx);
	if ( bstringClass == NULL )
		return NULL;
	JSObject *obj = JS_NewObject(cx, BStringJSClass(cx), NULL, NULL);
	if ( obj == NULL )
		return NULL;
	JS_SetReservedSlot(cx, obj, SLOT_BSTRING_LENGTH, INT_TO_JSVAL( bufferLength ));
	JS_SetPrivate(cx, obj, jsMallocatedBuffer);
	return obj;
}


inline JSObject* EmptyBString( JSContext *cx ) {

	JSClass *bstringClass = BStringJSClass(cx);
	if ( bstringClass == NULL )
		return NULL;
	JSObject *obj = JS_NewObject(cx, bstringClass, NULL, NULL);
	if ( obj == NULL )
		return NULL;
	return obj;
}


inline size_t BStringLength( JSContext *cx, JSObject *bStringObject ) {

	RT_SAFE(
		if ( JS_GET_CLASS( cx, bStringObject ) != BStringJSClass( cx ) )
			return 0;
	);
	jsval lengthVal;
	RT_CHECK_CALL( JS_GetReservedSlot(cx, bStringObject, SLOT_BSTRING_LENGTH, &lengthVal) );
	return JSVAL_IS_INT(lengthVal) ? JSVAL_TO_INT( lengthVal ) : 0;
}


inline char* BStringData( JSContext *cx, JSObject *bStringObject ) {

	RT_SAFE(
		if ( JS_GET_CLASS( cx, bStringObject ) != BStringJSClass( cx ) )
			return NULL;
	);
	return (char*)JS_GetPrivate(cx, bStringObject);
}


inline JSBool BStringGetDataAndLength( JSContext *cx, JSObject *bStringObject, char **data, size_t *dataLength ) {

	RT_SAFE(
		if ( JS_GET_CLASS( cx, bStringObject ) != BStringJSClass( cx ) )
			return JS_FALSE;
	);
	jsval lengthVal;
	RT_CHECK_CALL( JS_GetReservedSlot(cx, bStringObject, SLOT_BSTRING_LENGTH, &lengthVal) );
	*dataLength = JSVAL_IS_INT(lengthVal) ? JSVAL_TO_INT( lengthVal ) : 0;
	*data = (char*)JS_GetPrivate(cx, bStringObject);
	return JS_TRUE;
}


#endif // _JSLANGAPI_H_
