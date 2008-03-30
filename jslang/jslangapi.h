
inline JSClass* BStringClasp( JSContext *cx ) {

	static JSClass *clasp = NULL; // cache
	if ( clasp == NULL )
		clasp = GetClassByName(cx, "BString");
	return clasp;
}

inline JSObject* NewBString( JSContext *cx, char *jsMallocatedBuffer, size_t bufferLength ) {
		
	JSClass *bstringClass = BStringClasp(cx);
	if ( bstringClass == NULL )
		REPORT_ERROR("BString class is not defined");
	JSObject *obj = JS_NewObject(cx, BStringClasp(cx), NULL, NULL);
	RT_ASSERT_ALLOC( obj );
	RT_CHECK_CALL( JS_SetReservedSlot(cx, obj, SLOT_BSTRING_LENGTH, INT_TO_JSVAL( bufferLength )) );
	RT_CHECK_CALL( JS_SetPrivate(cx, obj, jsMallocatedBuffer) );
	return obj;
}


inline JSObject* EmptyBString( JSContext *cx ) {

	JSObject *obj = JS_NewObject(cx, BStringClasp(cx), NULL, NULL);
	RT_ASSERT_ALLOC( obj );
	return obj;
}


inline size_t BStringLength( JSContext *cx, JSObject *bStringObject ) {

	RT_ASSERT_CLASS_NAME( bStringObject, "BString" );
	jsval lengthVal;
	RT_CHECK_CALL( JS_GetReservedSlot(cx, bStringObject, SLOT_BSTRING_LENGTH, &lengthVal) );
	return JSVAL_IS_INT(lengthVal) ? JSVAL_TO_INT( lengthVal ) : 0;
}


inline char* BStringData( JSContext *cx, JSObject *bStringObject ) {

	RT_ASSERT_CLASS_NAME( bStringObject, "BString" );
	return (char*)JS_GetPrivate(cx, bStringObject);
}


inline JSBool BStringGetDataAndLength( JSContext *cx, JSObject *bStringObject, char **data, size_t *dataLength ) {

	RT_ASSERT_CLASS_NAME( bStringObject, "BString" );
	jsval lengthVal;
	RT_CHECK_CALL( JS_GetReservedSlot(cx, bStringObject, SLOT_BSTRING_LENGTH, &lengthVal) );
	*dataLength = JSVAL_IS_INT(lengthVal) ? JSVAL_TO_INT( lengthVal ) : 0;
	*data = (char*)JS_GetPrivate(cx, bStringObject);
	return JS_TRUE;
}
