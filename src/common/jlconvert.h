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

inline JSBool JL_JsvalToSInt8( JSContext *cx, jsval val, int8_t *result, bool *outOfRange ) {

	if ( JSVAL_IS_INT( val ) ) {

		int v = JSVAL_TO_INT( val );
		*outOfRange = v < (-128) || v > (127);
		*result = (int8_t)v;
	} else if ( JSVAL_IS_DOUBLE( val ) ) {

		double d = JSVAL_TO_DOUBLE(val);
		*outOfRange = d < (double)(-128) || d > (double)(127);
		*result = (int8_t)d;
	} else if ( JSVAL_IS_STRING( val ) ) { // using system byte order

		JLStr str;
		if ( !JL_JsvalToNative(cx, val, &str) || !str.IsSet() || str.Length() < sizeof(int8_t) )
			return JS_FALSE;
		const char *s = str.GetConstStr();
		*result = s[0];
		*outOfRange = false;
	} else {
		
		return JS_FALSE;
	}
	return JS_TRUE;
}

inline JSBool JL_JsvalToUInt8( JSContext *cx, jsval val, uint8_t *result, bool *outOfRange ) {

	if ( JSVAL_IS_INT( val ) ) {

		int v = JSVAL_TO_INT( val );
		*outOfRange = v < (0) || v > (0xff);
		*result = (uint8_t)v;
	} else if ( JSVAL_IS_DOUBLE( val ) ) {

		double d = JSVAL_TO_DOUBLE(val);
		*outOfRange = d < (0) || d > (double)(0xff);
		*result = (uint8_t)d;
	} else if ( JSVAL_IS_STRING( val ) ) { // using system byte order

		JLStr str;
		if ( !JL_JsvalToNative(cx, val, &str) || !str.IsSet() || str.Length() < sizeof(uint8_t) )
			return JS_FALSE;
		const char *s = str.GetConstStr();
		*result = (uint8_t)s[0];
		*outOfRange = *result < 0;
	} else {
		
		return JS_FALSE;
	}
	return JS_TRUE;
}


inline JSBool JL_JsvalToSInt16( JSContext *cx, jsval val, int16_t *result, bool *outOfRange ) {

	if ( JSVAL_IS_INT( val ) ) {

		int v = JSVAL_TO_INT( val );
		*outOfRange = v < (-0x7FFF - 1) || v > (0x7FFF);
		*result = (int16_t)v;
	} else if ( JSVAL_IS_DOUBLE( val ) ) {

		double d = JSVAL_TO_DOUBLE(val);
		*outOfRange = d < (double)(-0x7FFF - 1) || d > (double)(0x7FFF);
		*result = (int16_t)d;
	} else if ( JSVAL_IS_STRING( val ) ) { // using system byte order

		JLStr str;
		if ( !JL_JsvalToNative(cx, val, &str) || !str.IsSet() || str.Length() < sizeof(int16_t) )
			return JS_FALSE;
		const char *s = str.GetConstStr();
		*result = s[0]<<8 | s[1];
		*outOfRange = false;
	} else {
		
		return JS_FALSE;
	}
	return JS_TRUE;
}

inline JSBool JL_JsvalToUInt16( JSContext *cx, jsval val, uint16_t *result, bool *outOfRange ) {

	if ( JSVAL_IS_INT( val ) ) {

		int v = JSVAL_TO_INT( val );
		*outOfRange = v < (0) || v > 0xffff;
		*result = (uint16_t)v;
	} else if ( JSVAL_IS_DOUBLE( val ) ) {

		double d = JSVAL_TO_DOUBLE(val);
		*outOfRange = d < (0) || d > (double)(0xffff);
		*result = (uint16_t)d;
	} else if ( JSVAL_IS_STRING( val ) ) { // using system byte order

		JLStr str;
		if ( !JL_JsvalToNative(cx, val, &str) || !str.IsSet() || str.Length() < sizeof(uint16_t) )
			return JS_FALSE;
		const char *s = str.GetConstStr();
		*result = s[0]<<8 | s[1];
		*outOfRange = *result < 0;
	} else {
		
		return JS_FALSE;
	}
	return JS_TRUE;
}


inline JSBool JL_JsvalToSInt24( JSContext *cx, jsval val, int32_t *result, bool *outOfRange ) {

	if ( JSVAL_IS_INT( val ) ) {

		int v = JSVAL_TO_INT( val );
		*outOfRange = v < (-0x7FFFFFL - 1) || v > (0x7FFFFFL);
		*result = (int32_t)v;
	} else if ( JSVAL_IS_DOUBLE( val ) ) {

		double d = JSVAL_TO_DOUBLE(val);
		*outOfRange = d < (double)(-0x7FFFFFL - 1) || d > (double)(0x7FFFFFL);
		*result = (int32_t)d;
	} else if ( JSVAL_IS_STRING( val ) ) { // using system byte order

		JLStr str;
		if ( !JL_JsvalToNative(cx, val, &str) || !str.IsSet() || str.Length() < 3 )
			return JS_FALSE;
		const char *s = str.GetConstStr();
		*result = ((s[0]<<8 | s[1])<<8) | s[2];
		*outOfRange = false;
	} else {
		
		return JS_FALSE;
	}
	return JS_TRUE;
}


inline JSBool JL_JsvalToUInt24( JSContext *cx, jsval val, uint32_t *result, bool *outOfRange ) {

	if ( JSVAL_IS_INT( val ) ) {

		unsigned int v = JSVAL_TO_INT( val );
		*outOfRange = v < 0 || v > (0xFFFFFFUL);
		*result = (uint32_t)v;
	} else if ( JSVAL_IS_DOUBLE( val ) ) {

		double d = JSVAL_TO_DOUBLE(val);
		*outOfRange = d < 0 || d > (double)(0xFFFFFFUL) || d != (double)(int32_t)d;
		*result = (uint32_t)d;
	} else if ( JSVAL_IS_STRING( val ) ) { // using system byte order

		JLStr str;
		if ( !JL_JsvalToNative(cx, val, &str) || !str.IsSet() || str.Length() < 3 )
			return JS_FALSE;
		const char *s = str.GetConstStr();
		*result = ((s[0]<<8 | s[1])<<8) | s[2];
		*outOfRange = *result < 0;
	} else {
		
		return JS_FALSE;
	}
	return JS_TRUE;
}


inline JSBool JL_JsvalToSInt32( JSContext *cx, jsval val, int32_t *result, bool *outOfRange ) {

	if ( JSVAL_IS_INT( val ) ) {

		int v = JSVAL_TO_INT( val );
		*outOfRange = v < (-0x7FFFFFFFL - 1) || v > (0x7FFFFFFFL);
		*result = (int32_t)v;
	} else if ( JSVAL_IS_DOUBLE( val ) ) {

		double d = JSVAL_TO_DOUBLE(val);
		*outOfRange = d < (double)(-0x7FFFFFFFL - 1) || d > (double)(0x7FFFFFFFL);
		*result = (int32_t)d;
	} else if ( JSVAL_IS_STRING( val ) ) { // using system byte order

		JLStr str;
		if ( !JL_JsvalToNative(cx, val, &str) || !str.IsSet() || str.Length() < sizeof(int32_t) )
			return JS_FALSE;
		const char *s = str.GetConstStr();
		*result = (((s[0]<<8 | s[1])<<8) | s[2])<<8 | s[3];
		*outOfRange = false;
	} else {
		
		return JS_FALSE;
	}
	return JS_TRUE;
}

inline JSBool JL_JsvalToUInt32( JSContext *cx, jsval val, uint32_t *result, bool *outOfRange ) {

	if ( JSVAL_IS_INT( val ) ) {

		long v = JSVAL_TO_INT( val ); // beware: int31 ! not int32
		*outOfRange = v < (0);// || v > ULONG_MAX; <- this case is impossible
		*result = (uint32_t)v;
	} else if ( JSVAL_IS_DOUBLE( val ) ) {

		double d = JSVAL_TO_DOUBLE(val);
		*outOfRange = d < (0) || d > (double)(0xFFFFFFFFUL);
		*result = (uint32_t)d;
	} else if ( JSVAL_IS_STRING( val ) ) { // using system byte order

		JLStr str;
		if ( !JL_JsvalToNative(cx, val, &str) || !str.IsSet() || str.Length() < sizeof(uint32_t) )
			return JS_FALSE;
		const char *s = str.GetConstStr();
		*result = (((s[0]<<8 | s[1])<<8) | s[2])<<8 | s[3];
		*outOfRange = *result < 0;
	} else {
		
		return JS_FALSE;
	}
	return JS_TRUE;
}


// range if jsval is a jsdouble: +/-2^53
inline JSBool JL_JsvalToSInt64( JSContext *cx, jsval val, int64_t *result, bool *outOfRange ) {

	if ( JSVAL_IS_INT( val ) ) {

		int v = JSVAL_TO_INT( val );
		*outOfRange = false;
		*result = (int64_t)v;
	} else if ( JSVAL_IS_DOUBLE( val ) ) {

		double d = JSVAL_TO_DOUBLE(val);
		*outOfRange = d < -MAX_INT_TO_DOUBLE || d > MAX_INT_TO_DOUBLE || d != (double)(int64_t)d;
		*result = (int64_t)d;
	} else if ( JSVAL_IS_STRING( val ) ) { // using system byte order

		JLStr str;
		if ( !JL_JsvalToNative(cx, val, &str) || !str.IsSet() || str.Length() < sizeof(int64_t) )
			return JS_FALSE;
		const char *s = str.GetConstStr();
		*result = (((((((s[0]<<8 | s[1])<<8) | s[2])<<8 | s[3]<<8) | s[4]<<8) | s[5]<<8) | s[6]<<8) | s[7];
		*outOfRange = false;
	} else {
		
		return JS_FALSE;
	}
	return JS_TRUE;
}

inline JSBool JL_JsvalToUInt64( JSContext *cx, jsval val, uint64_t *result, bool *outOfRange ) {

	if ( JSVAL_IS_INT( val ) ) {

		int v = JSVAL_TO_INT( val );
		*outOfRange = false;
		*result = (uint64_t)v;
	} else if ( JSVAL_IS_DOUBLE( val ) ) {

		double d = JSVAL_TO_DOUBLE(val);
		*outOfRange = d < 0 || d > MAX_INT_TO_DOUBLE || d != (double)(int64_t)d;
		*result = (uint64_t)d;
	} else if ( JSVAL_IS_STRING( val ) ) { // using system byte order

		JLStr str;
		if ( !JL_JsvalToNative(cx, val, &str) || !str.IsSet() || str.Length() < sizeof(int64_t) )
			return JS_FALSE;
		const char *s = str.GetConstStr();
		*result = (((((((s[0]<<8 | s[1])<<8) | s[2])<<8 | s[3]<<8) | s[4]<<8) | s[5]<<8) | s[6]<<8) | s[7];
		*outOfRange = *result < 0;
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

		JL_SAFE( JL_NewNumberValue(cx, vector[i], &value) ); // (TBD) useful ??
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

		JL_NewNumberValue(cx, vector[i], &value); // JL_NativeToJsval(cx, vector[i], &value);
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
