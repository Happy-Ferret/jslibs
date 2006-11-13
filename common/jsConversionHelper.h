
inline JSBool IntArgvToVector( JSContext *cx, int count, const jsval *argv, int *vector ) {

	for (int i=0; i<count; ++i) {

		RT_ASSERT( JSVAL_IS_INT(argv[i]), "Must be an integer." );
		RT_SAFE( jsdouble d; JS_ValueToNumber(cx, argv[i], &d); vector[i] = d; );
		RT_UNSAFE( vector[i] = JSVAL_TO_INT(argv[i]) );
	}
	return JS_TRUE;
}

inline JSBool IntArrayToVector( JSContext *cx, int count, const jsval *vp, int *vector ) {

	JSObject *jsArray;
	if ( JS_ValueToObject(cx, *vp, &jsArray) == JS_FALSE )
		return JS_FALSE;
	RT_ASSERT( jsArray != NULL && JS_IsArrayObject(cx,jsArray), "value must be an array." );
	jsval value;
	for (int i=0; i<count; ++i) {

		JS_GetElement(cx, jsArray, i, &value );
		RT_SAFE( jsdouble d; JS_ValueToNumber(cx, value, &d); vector[i] = d; );
		RT_UNSAFE( vector[i] = JSVAL_TO_INT(value) );
	}
	return JS_TRUE;
}


inline JSBool IntVectorToArray( JSContext *cx, int count, const int *vector, jsval *vp ) {

	JSObject *jsArray = JS_NewArrayObject(cx, 0, NULL);
	*vp = OBJECT_TO_JSVAL(jsArray);
	jsval value;
	for (jsint i=0; i<count; ++i) {

		RT_SAFE( JS_NewNumberValue(cx, vector[i], &value) ); // [TBD] useful ??
		RT_UNSAFE( value = INT_TO_JSVAL(vector[i]) );
		JS_SetElement(cx, jsArray, i, &value);
	}
	return JS_TRUE;
}


inline JSBool FloatArrayToVector( JSContext *cx, int count, const jsval *vp, float *vector ) {

	JSObject *jsArray;
	JS_ValueToObject(cx, *vp, &jsArray);
	RT_ASSERT( JS_IsArrayObject(cx,jsArray), "value must be an array." );
	jsval value;
	jsdouble d;
	JSBool status;
	for (int i=0; i<count; ++i) {

		status = JS_GetElement(cx, jsArray, i, &value );
		RT_ASSERT( status && value != JSVAL_VOID, "Invalid array value." );
		JS_ValueToNumber(cx, value, &d);
		vector[i] = d;
	}
	return JS_TRUE;
}


inline JSBool FloatVectorToArray( JSContext *cx, int count, const float *vector, jsval *vp ) {

	JSObject *jsArray = JS_NewArrayObject(cx, 0, NULL);
	*vp = OBJECT_TO_JSVAL(jsArray);
	jsval value;
	for (int i=0; i<count; ++i) {

		JS_NewNumberValue(cx, vector[i], &value); // JS_NewDoubleValue(cx, vector[i], &value);
		JS_SetElement(cx, jsArray, i, &value);
	}
	return JS_TRUE;
}
