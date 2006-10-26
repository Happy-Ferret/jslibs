#include "stdafx.h"
#include "smtools.h"

JSBool IntArrayToVector( JSContext *cx, int count, const jsval *vp, int *vector ) {

	JSObject *jsArray;
	RT_SAFE( JS_ValueToObject(cx, *vp, &jsArray) );
	RT_UNSAFE( jsArray = JSVAL_TO_OBJECT(*vp) );
	RT_ASSERT( JS_IsArrayObject(cx,jsArray), "value must be an array." );
	jsval value;
	int32 i32;
	for (int i=0; i<count; ++i) {

		JS_GetElement(cx, jsArray, i, &value );
		RT_SAFE( jsdouble d; JS_ValueToNumber(cx, value, &d); i32=d; );
		RT_UNSAFE( i32 = JSVAL_TO_INT(value) );
		vector[i] = i32;
	}
	return JS_TRUE;
}

JSBool IntVectorToArray( JSContext *cx, int count, const int *vector, jsval *vp ) {

	JSObject *jsArray = JS_NewArrayObject(cx, 0, NULL);
	*vp = OBJECT_TO_JSVAL(jsArray);
	jsval value;
	int32 i32;
	for (int i=0; i<count; ++i) {

		RT_SAFE( JS_NewNumberValue(cx, vector[i], &value) );
		RT_UNSAFE( value = INT_TO_JSVAL(vector[i]) );
		JS_SetElement(cx, jsArray, i, &value);
	}
	return JS_TRUE;
}


JSBool FloatArrayToVector( JSContext *cx, int count, const jsval *vp, float *vector ) {

	JSObject *jsArray;
	JS_ValueToObject(cx, *vp, &jsArray );
	RT_ASSERT( JS_IsArrayObject(cx,jsArray), "value must be an array." );
	jsval value;
	jsdouble d;
	for (int i=0; i<count; ++i) {

		JS_GetElement(cx, jsArray, i, &value );
		JS_ValueToNumber(cx, value, &d);
		vector[i] = d;
	}
	return JS_TRUE;
}

JSBool FloatVectorToArray( JSContext *cx, int count, const float *vector, jsval *vp ) {

	JSObject *jsArray = JS_NewArrayObject(cx, 0, NULL);
	*vp = OBJECT_TO_JSVAL(jsArray);
	jsval value;
	for (int i=0; i<count; ++i) {

		JS_NewDoubleValue(cx, vector[i], &value);
		JS_SetElement(cx, jsArray, i, &value);
	}
	return JS_TRUE;
}
