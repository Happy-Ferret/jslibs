#include "stdafx.h"


JSBool ArrayToVector( JSContext *cx, int dim, jsval *vp, real *vector ) {

	JSObject *jsArray;
	JS_ValueToObject(cx, *vp, &jsArray );
	RT_ASSERT( JS_IsArrayObject(cx,jsArray), "value must be an array." );
	jsval value;
	jsdouble d;
	for (int i=0; i<dim; ++i) {

		JS_GetElement(cx, jsArray, i, &value );
		JS_ValueToNumber(cx, value, &d);
		vector[i] = d;
	}
	return JS_TRUE;
}

JSBool VectorToArray( JSContext *cx, int dim, const real *vector, jsval *vp ) {

	JSObject *jsArray = JS_NewArrayObject(cx, 0, NULL);
	*vp = OBJECT_TO_JSVAL(jsArray);
	jsval value;
	for (int i=0; i<dim; ++i) {

		JS_NewDoubleValue(cx, vector[i], &value);
		JS_SetElement(cx, jsArray, i, &value);
	}
	return JS_TRUE;
}
