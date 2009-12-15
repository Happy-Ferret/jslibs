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

#include "limits.h"

inline JSBool JsvalToSInt8( JSContext *cx, jsval val, int8_t *result, bool *outOfRange ) {

	if ( JSVAL_IS_INT( val ) ) {

		int v = JSVAL_TO_INT( val );
		*outOfRange = v < (-128) || v > (127);
		*result = (int8_t)v;
	} else if ( JSVAL_IS_DOUBLE( val ) ) {

		double d = *JSVAL_TO_DOUBLE(val);
		*outOfRange = d < (double)(-128) || d > (double)(127);
		*result = (int8_t)(uint8_t)d;
	} else if ( JSVAL_IS_STRING( val ) ) { // using system byte order

		if ( JL_GetStringLength(JSVAL_TO_STRING( val )) < sizeof(int8_t) )
			return JS_FALSE;
		*outOfRange = false;
		*result = *(int8_t*)JS_GetStringBytes(JSVAL_TO_STRING( val ));
	} else
		return JS_FALSE;
	return JS_TRUE;
}

inline JSBool JsvalToUInt8( JSContext *cx, jsval val, uint8_t *result, bool *outOfRange ) {

	if ( JSVAL_IS_INT( val ) ) {

		int v = JSVAL_TO_INT( val );
		*outOfRange = v < (0) || v > (0xff);
		*result = (uint8_t)v;
	} else if ( JSVAL_IS_DOUBLE( val ) ) {

		double d = *JSVAL_TO_DOUBLE(val);
		*outOfRange = d < (0) || d > (double)(0xff);
		*result = (uint8_t)d;
	} else if ( JSVAL_IS_STRING( val ) ) { // using system byte order

		if ( JL_GetStringLength(JSVAL_TO_STRING( val )) < sizeof(uint8_t) )
			return JS_FALSE;
		int8_t c = *(uint8_t*)JS_GetStringBytes(JSVAL_TO_STRING( val ));
		*outOfRange = c < 0;
		*result = (uint8_t)c;
	} else
		return JS_FALSE;
	return JS_TRUE;
}


inline JSBool JsvalToSInt16( JSContext *cx, jsval val, int16_t *result, bool *outOfRange ) {

	if ( JSVAL_IS_INT( val ) ) {

		int v = JSVAL_TO_INT( val );
		*outOfRange = v < (-32768) || v > (32767);
		*result = (int16_t)v;
	} else if ( JSVAL_IS_DOUBLE( val ) ) {

		double d = *JSVAL_TO_DOUBLE(val);
		*outOfRange = d < (double)(-32768) || d > (double)(32767);
		*result = (int16_t)(uint16_t)d;
	} else if ( JSVAL_IS_STRING( val ) ) { // using system byte order

		if ( JL_GetStringLength(JSVAL_TO_STRING( val )) < sizeof(int16_t) )
			return JS_FALSE;
		*outOfRange = false;
		*result = *(int16_t*)JS_GetStringBytes(JSVAL_TO_STRING( val ));
	} else
		return JS_FALSE;
	return JS_TRUE;
}

inline JSBool JsvalToUInt16( JSContext *cx, jsval val, uint16_t *result, bool *outOfRange ) {

	if ( JSVAL_IS_INT( val ) ) {

		int v = JSVAL_TO_INT( val );
		*outOfRange = v < (0) || v > 0xffff;
		*result = (uint16_t)v;
	} else if ( JSVAL_IS_DOUBLE( val ) ) {

		double d = *JSVAL_TO_DOUBLE(val);
		*outOfRange = d < (0) || d > (double)(0xffff);
		*result = (uint16_t)d;
	} else if ( JSVAL_IS_STRING( val ) ) { // using system byte order

		if ( JL_GetStringLength(JSVAL_TO_STRING( val )) < sizeof(uint16_t) )
			return JS_FALSE;
		int16_t s = *(int16_t*)JS_GetStringBytes(JSVAL_TO_STRING( val ));
		*outOfRange = s < 0;
		*result = (uint16_t)s;
	} else
		return JS_FALSE;
	return JS_TRUE;
}


inline JSBool JsvalToSInt32( JSContext *cx, jsval val, int32_t *result, bool *outOfRange ) {

	if ( JSVAL_IS_INT( val ) ) {

		int v = JSVAL_TO_INT( val );
		*outOfRange = v < (-2147483647L - 1) || v > (2147483647L);
		*result = (int32_t)v;
	} else if ( JSVAL_IS_DOUBLE( val ) ) {

		double d = *JSVAL_TO_DOUBLE(val);
		*outOfRange = d < (double)(-2147483647L - 1) || d > (double)(2147483647L);
		*result = (int32_t)(uint32_t)d;
	} else if ( JSVAL_IS_STRING( val ) ) { // using system byte order

		if ( JL_GetStringLength(JSVAL_TO_STRING( val )) < sizeof(int32_t) )
			return JS_FALSE;
		*outOfRange = false;
		*result = *(int32_t*)JS_GetStringBytes(JSVAL_TO_STRING( val ));
	} else
		return JS_FALSE;
	return JS_TRUE;
}

inline JSBool JsvalToUInt32( JSContext *cx, jsval val, uint32_t *result, bool *outOfRange ) {

	if ( JSVAL_IS_INT( val ) ) {

		long v = JSVAL_TO_INT( val ); // beware: int31 ! not int32
		*outOfRange = v < (0);// || v > ULONG_MAX; <- this case is impossible
		*result = (uint32_t)v;
	} else if ( JSVAL_IS_DOUBLE( val ) ) {

		double d = *JSVAL_TO_DOUBLE(val);
		*outOfRange = d < (0) || d > (double)(0xffffffffUL);
		*result = (uint32_t)d;
	} else if ( JSVAL_IS_STRING( val ) ) { // using system byte order

		if ( JL_GetStringLength(JSVAL_TO_STRING( val )) < sizeof(uint32_t) )
			return JS_FALSE;
		long s = *(int32_t*)JS_GetStringBytes(JSVAL_TO_STRING( val ));
		*outOfRange = s < 0;
		*result = (uint32_t)s;
	} else
		return JS_FALSE;
	return JS_TRUE;
}


// range if jsval is a jsdouble: +/-2^53
inline JSBool JsvalToSInt64( JSContext *cx, jsval val, int64_t *result, bool *outOfRange ) {

	if ( JSVAL_IS_INT( val ) ) {

		int v = JSVAL_TO_INT( val );
		*outOfRange = false;
		*result = (int64_t)v;
	} else if ( JSVAL_IS_DOUBLE( val ) ) {

		double d = *JSVAL_TO_DOUBLE(val);
		*outOfRange = d < -MAX_INTDOUBLE || d > MAX_INTDOUBLE || d != (double)(int64_t)d;
		*result = (int64_t)d;
	} else if ( JSVAL_IS_STRING( val ) ) { // using system byte order

		if ( JL_GetStringLength(JSVAL_TO_STRING( val )) < sizeof(int64_t) )
			return JS_FALSE;
		*outOfRange = false;
		*result = *(int64_t*)JS_GetStringBytes(JSVAL_TO_STRING( val ));
	} else
		return JS_FALSE;
	return JS_TRUE;
}

inline JSBool JsvalToUInt64( JSContext *cx, jsval val, uint64_t *result, bool *outOfRange ) {

	if ( JSVAL_IS_INT( val ) ) {

		int v = JSVAL_TO_INT( val );
		*outOfRange = false;
		*result = (uint64_t)v;
	} else if ( JSVAL_IS_DOUBLE( val ) ) {

		double d = *JSVAL_TO_DOUBLE(val);
		*outOfRange = d < 0 || d > MAX_INTDOUBLE || d != (double)(int64_t)d;
		*result = (uint64_t)d;
	} else if ( JSVAL_IS_STRING( val ) ) { // using system byte order

		if ( JL_GetStringLength(JSVAL_TO_STRING( val )) < sizeof(uint64_t) )
			return JS_FALSE;
		*outOfRange = false;
		*result = *(uint64_t*)JS_GetStringBytes(JSVAL_TO_STRING( val ));
	} else
		return JS_FALSE;
	return JS_TRUE;
}


/*
inline JSBool IntArgvToVector( JSContext *cx, int count, const jsval *argv, int *vector ) {

	for (int i=0; i<count; ++i) {

		JL_S_ASSERT( JSVAL_IS_INT(argv[i]), "Must be an integer." );
		JL_SAFE( jsdouble d; JS_ValueToNumber(cx, argv[i], &d); vector[i] = (int)d; );
		JL_UNSAFE( vector[i] = JSVAL_TO_INT(argv[i]) );
	}
	return JS_TRUE;
}
*/

/*
inline JSBool ArrayLength( JSContext *cx, int *count, const jsval jsvalArray ) {

	JSObject *jsArray;
	JL_CHK( JS_ValueToObject(cx, jsvalArray, &jsArray) );
	JL_S_ASSERT( jsArray != NULL && JS_IsArrayObject(cx,jsArray), "value must be an array." );
	jsuint arraySize;
	JL_CHK( JS_GetArrayLength(cx, jsArray, &arraySize) );
	*count = arraySize;
	return JS_TRUE;
}
*/

/*
inline JSBool ArrayArrayToVector( JSContext *cx, int count, const jsval *vp, jsval *vector ) {

	JSObject *jsArray;
	JL_CHK( JS_ValueToObject(cx, *vp, &jsArray) );
	JL_S_ASSERT( jsArray != NULL && JS_IsArrayObject(cx,jsArray), "value must be an array." );
	jsval value; // sub-array
	JSBool status;
	for (int i=0; i<count; ++i) {

		status = JS_GetElement(cx, jsArray, i, &value);
		JL_S_ASSERT( status && !JSVAL_IS_VOID( value ), "Invalid array value." );
		vector[i] = value;
	}
	return JS_TRUE;
}
*/

/*
inline JSBool IntArrayToVector( JSContext *cx, int count, const jsval *vp, int *vector ) {

	JSObject *jsArray;
	JL_CHK( JS_ValueToObject(cx, *vp, &jsArray) );
	JL_S_ASSERT( jsArray != NULL && JS_IsArrayObject(cx,jsArray), "value must be an array." );
	jsval value;
	JSBool status;
	for (int i=0; i<count; ++i) {

		status = JS_GetElement(cx, jsArray, i, &value);
		JL_S_ASSERT( status && !JSVAL_IS_VOID( value ), "Invalid array value." );
		JL_SAFE( jsdouble d; JS_ValueToNumber(cx, value, &d); vector[i] = (int)d; );
		JL_UNSAFE( vector[i] = JSVAL_TO_INT(value) );
	}
	return JS_TRUE;
}
*/

/*
inline JSBool IntVectorToArray( JSContext *cx, int count, const int *vector, jsval *vp ) {

	JSObject *jsArray = JS_NewArrayObject(cx, 0, NULL);
	*vp = OBJECT_TO_JSVAL(jsArray);
	jsval value;
	for (jsint i=0; i<count; ++i) {

		JL_SAFE( JS_NewNumberValue(cx, vector[i], &value) ); // (TBD) useful ??
		JL_UNSAFE( value = INT_TO_JSVAL(vector[i]) );
		JS_SetElement(cx, jsArray, i, &value);
	}
	return JS_TRUE;
}
*/

/*
inline JSBool FloatArrayToVector( JSContext *cx, int count, const jsval *vp, float *vector ) {

	JSObject *jsArray;
	JS_ValueToObject(cx, *vp, &jsArray);
	JL_S_ASSERT( JS_IsArrayObject(cx,jsArray), "value must be an array." );
	jsval value;
	jsdouble d;
	JSBool status;
	for (jsint i=0; i<count; ++i) {

		status = JS_GetElement(cx, jsArray, i, &value );
		JL_S_ASSERT( status && !JSVAL_IS_VOID( value ), "Invalid array value." );
		JS_ValueToNumber(cx, value, &d);
		vector[i] = d;
	}
	return JS_TRUE;
}
*/

/*
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
*/

/*
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
*/

/*
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
*/

/*
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
*/


#endif // _JSCONVERSIONHELPER_H_
