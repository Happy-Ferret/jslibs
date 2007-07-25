/* ***** BEGIN LICENSE BLOCK *****
 * Version: GNU GPL 2.0
 *
 * The contents of this file are subject to the
 * GNU General Public License Version 2.0; you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at
 * http://www.gnu.org/licenses/gpl.html
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 * ***** END LICENSE BLOCK ***** */

#ifndef _JSCONVERSIONHELPER_H_
#define _JSCONVERSIONHELPER_H_

inline JSBool IntArgvToVector( JSContext *cx, int count, const jsval *argv, int *vector ) {

	for (int i=0; i<count; ++i) {

		RT_ASSERT( JSVAL_IS_INT(argv[i]), "Must be an integer." );
		RT_SAFE( jsdouble d; JS_ValueToNumber(cx, argv[i], &d); vector[i] = d; );
		RT_UNSAFE( vector[i] = JSVAL_TO_INT(argv[i]) );
	}
	return JS_TRUE;
}

inline JSBool ArrayLength( JSContext *cx, int *count, const jsval jsvalArray ) {

	JSObject *jsArray;
	RT_CHECK_CALL( JS_ValueToObject(cx, jsvalArray, &jsArray) );
	RT_ASSERT( jsArray != NULL && JS_IsArrayObject(cx,jsArray), "value must be an array." );
	jsuint arraySize;
	RT_CHECK_CALL( JS_GetArrayLength(cx, jsArray, &arraySize) );
	*count = arraySize;
	return JS_TRUE;
}

inline JSBool ArrayArrayToVector( JSContext *cx, int count, const jsval *vp, jsval *vector ) {

	JSObject *jsArray;
	RT_CHECK_CALL( JS_ValueToObject(cx, *vp, &jsArray) )
	RT_ASSERT( jsArray != NULL && JS_IsArrayObject(cx,jsArray), "value must be an array." );
	jsval value; // sub-array
	JSBool status;
	for (int i=0; i<count; ++i) {

		status = JS_GetElement(cx, jsArray, i, &value);
		RT_ASSERT( status && value != JSVAL_VOID, "Invalid array value." );
		vector[i] = value;
	}
	return JS_TRUE;
}

inline JSBool IntArrayToVector( JSContext *cx, int count, const jsval *vp, int *vector ) {

	JSObject *jsArray;
	RT_CHECK_CALL( JS_ValueToObject(cx, *vp, &jsArray) )
	RT_ASSERT( jsArray != NULL && JS_IsArrayObject(cx,jsArray), "value must be an array." );
	jsval value;
	JSBool status;
	for (int i=0; i<count; ++i) {

		status = JS_GetElement(cx, jsArray, i, &value);
		RT_ASSERT( status && value != JSVAL_VOID, "Invalid array value." );
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

		RT_SAFE( JS_NewNumberValue(cx, vector[i], &value) ); // (TBD) useful ??
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
	for (jsint i=0; i<count; ++i) {

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




inline int GetInt( JSContext *cx, jsval objVal, const char *propertyName, int defaultValue ) {

	int32 value;
	jsval tmp;
	if ( !JSVAL_IS_OBJECT(objVal) || JSVAL_IS_NULL(objVal) )
		return defaultValue;
	if ( JS_GetProperty(cx, JSVAL_TO_OBJECT(objVal), propertyName, &tmp) == JS_FALSE )
		return defaultValue;
	if ( JS_ValueToInt32(cx, tmp, &value) == JS_FALSE )
		return defaultValue;
	return value;
}

inline float GetFloat( JSContext *cx, jsval objVal, const char *propertyName, float defaultValue ) {

	jsdouble value;
	jsval tmp;
	if ( !JSVAL_IS_OBJECT(objVal) || JSVAL_IS_NULL(objVal) )
		return defaultValue;
	if ( JS_GetProperty(cx, JSVAL_TO_OBJECT(objVal), propertyName, &tmp) == JS_FALSE )
		return defaultValue;
	if ( JS_ValueToNumber(cx, tmp, &value) == JS_FALSE )
		return defaultValue;
	return value;
}

inline float GetBool( JSContext *cx, jsval objVal, const char *propertyName, bool defaultValue ) {

	JSBool value;
	jsval tmp;
	if ( !JSVAL_IS_OBJECT(objVal) || JSVAL_IS_NULL(objVal) )
		return defaultValue;
	if ( JS_GetProperty(cx, JSVAL_TO_OBJECT(objVal), propertyName, &tmp) == JS_FALSE )
		return defaultValue;
	if ( JS_ValueToBoolean(cx, tmp, &value) == JS_FALSE )
		return defaultValue;
	return value == JS_TRUE;
}

#endif // _JSCONVERSIONHELPER_H_
