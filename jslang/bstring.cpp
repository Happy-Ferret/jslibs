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

#include "stdafx.h"
#include <cstring>

#include "bstringapi.h"

#include "../common/jsNativeInterface.h"




inline JSBool LengthSet( JSContext *cx, JSObject *obj, size_t bufferLength ) {

	return JS_SetReservedSlot(cx, obj, SLOT_BSTRING_LENGTH, INT_TO_JSVAL( bufferLength ));
}


inline JSBool LengthGet( JSContext *cx, JSObject *obj, size_t *bufferLength ) {

	jsval bufferLengthVal;
	J_CHECK_CALL( JS_GetReservedSlot(cx, obj, SLOT_BSTRING_LENGTH, &bufferLengthVal) );
	*bufferLength = JSVAL_IS_INT(bufferLengthVal) ? JSVAL_TO_INT( bufferLengthVal ) : 0;
	return JS_TRUE;
}

inline JSBool BufferGet( JSContext *cx, JSObject *obj, const char **buf ) {

	*buf = (const char*)JS_GetPrivate(cx, obj);
	return JS_TRUE;
}


JSBool NativeInterfaceBufferRead( JSContext *cx, JSObject *obj, const char **buf, size_t *size ) {

	J_CHECK_CALL( BufferGet(cx, obj, buf) );
	J_CHECK_CALL( LengthGet(cx, obj, size) );
	return JS_TRUE;
}


BEGIN_CLASS( BString )

/*
inline bool JsvalIsBString( JSContext *cx, jsval val ) {

	return JSVAL_IS_OBJECT(val) && !JSVAL_IS_NULL(val) && JS_GET_CLASS(cx, JSVAL_TO_OBJECT(val)) == _class;
}
*/

inline JSBool JsvalToBString( JSContext *cx, JSObject *obj, jsval val ) {

	size_t srcLen;
	void *src, *dst = NULL;

	if ( JsvalIsBString(cx, val) ) {
		
		BStringGetDataAndLength(cx, JSVAL_TO_OBJECT( val ), &src, &srcLen);
		if ( srcLen > 0 ) {

			dst = JS_malloc(cx, srcLen +1);
			J_S_ASSERT_ALLOC( dst );
			((char*)dst)[srcLen] = '\0';
			memcpy(dst, src, srcLen);
		}
	} else {
		
		JSString *jsstr = JS_ValueToString(cx, val);
		srcLen = JS_GetStringLength(jsstr);
		if ( srcLen > 0 ) {

			dst = JS_malloc(cx, srcLen +1);
			J_S_ASSERT_ALLOC( dst );
			((char*)dst)[srcLen] = '\0';

			jschar *chars = JS_GetStringChars(jsstr);
			for ( size_t i = 0; i < srcLen; i++ )
				((char*)dst)[i] = (uint8)chars[i];
		}
	}

	void *pv = JS_GetPrivate(cx, obj);
	if ( pv != NULL )
		JS_free(cx, pv);

	J_CHECK_CALL( JS_SetPrivate(cx, obj, dst) );
	J_CHECK_CALL( LengthSet(cx, obj, srcLen) );

	return JS_TRUE;
}


JSBool BStringToJsval( JSContext *cx, JSObject *obj, jsval *val ) {

	void *pv = JS_GetPrivate(cx, obj);
	size_t length;
	J_CHECK_CALL( LengthGet(cx, obj, &length) );
	if ( pv == NULL || length == 0 ) {

		*val = JS_GetEmptyStringValue(cx);
	} else {

		jschar *ucStr = (jschar*)JS_malloc(cx, (length + 1) * sizeof(jschar));
		ucStr[length] = 0;
		for ( size_t i = 0; i < length; i++ )
			ucStr[i] = ((char*)pv)[i];
		JSString *jsstr = JS_NewUCString(cx, ucStr, length);
		*val = STRING_TO_JSVAL( jsstr );
	}
	return JS_TRUE;
}



DEFINE_FINALIZE() {

	void *pv = JS_GetPrivate(cx, obj);
	if (pv != NULL)
		JS_free(cx, pv);
}

DEFINE_CONSTRUCTOR() {

	if ( JS_IsConstructing(cx) == JS_FALSE ) { // supports this form (w/o new operator) : result.param1 = Blob('Hello World');

		obj = JS_NewObject(cx, _class, NULL, NULL);
		J_S_ASSERT( obj != NULL, "BString construction failed." );
		*rval = OBJECT_TO_JSVAL(obj);
	} else
		J_S_ASSERT_THIS_CLASS();

	if ( J_ARG_ISDEF(1) ) {

//		J_CHECK_CALL( JsvalToBString(cx, obj, J_ARG(1)) );

		size_t length;
		const char *sBuffer;
		J_CHECK_CALL( JsvalToStringAndLength(cx, J_ARG(1), &sBuffer, &length) );
		J_CHECK_CALL( LengthSet(cx, obj, length) );
		void *dBuffer = JS_malloc(cx, length +1);
		((char*)dBuffer)[length] = '\0';
		memcpy(dBuffer, sBuffer, length);
		JS_SetPrivate(cx, obj, dBuffer);
	}

	J_CHECK_CALL( SetBufferReadInterface(cx, obj, NativeInterfaceBufferRead) );

	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Set ) {
	
	JS_ClearScope(cx, J_FOBJ);

	*J_FRVAL = OBJECT_TO_JSVAL( J_FOBJ );

	if ( !J_FARG_ISDEF(1) ) { // clear

		void *pv = JS_GetPrivate(cx, J_FOBJ);
		if (pv != NULL)
			JS_free(cx, pv);
		J_CHECK_CALL( JS_SetPrivate(cx, J_FOBJ, NULL) );
		J_CHECK_CALL( LengthSet(cx, J_FOBJ, 0) );
		return JS_TRUE;
	}

	J_CHECK_CALL( JsvalToBString(cx, J_FOBJ, J_FARG(1)) );
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Add ) {
	
	J_S_ASSERT_ARG_MIN( 1 );

	size_t length;
	J_CHECK_CALL( LengthGet(cx, J_FOBJ, &length) );

	size_t srcLen;
	void *src, *dst;

	if ( JsvalIsBString(cx, J_FARG(1)) ) {
		
		BStringGetDataAndLength(cx, JSVAL_TO_OBJECT( J_FARG(1) ), &src, &srcLen);
		if ( srcLen > 0 ) {

			dst = JS_malloc(cx, srcLen + length +1);
			J_S_ASSERT_ALLOC( dst );
			((char*)dst)[srcLen + length] = '\0';
			memcpy(((int8_t*)dst) + length, src, srcLen);
		} else {

			dst = NULL;
		}
	} else {
		
		JSString *jsstr = JS_ValueToString(cx, J_FARG(1));
		J_FARG(1) = STRING_TO_JSVAL(jsstr);

		srcLen = JS_GetStringLength(jsstr);
		if ( srcLen > 0 ) {

			dst = JS_malloc(cx, srcLen + length +1);
			J_S_ASSERT_ALLOC( dst );
			((char*)dst)[srcLen + length] = '\0';
			jschar *chars = JS_GetStringChars(jsstr);
			for ( size_t i = 0; i < srcLen; i++ )
				((char*)dst)[i + length] = (uint8)chars[i];
		} else {
			
			dst = NULL;
		}
	}

	void *pv = JS_GetPrivate(cx, J_FOBJ);
	if ( pv != NULL ) {
		
		memcpy(dst, pv, length);
		JS_free(cx, pv);
	}

	J_CHECK_CALL( LengthSet(cx, J_FOBJ, srcLen + length) );
	J_CHECK_CALL( JS_SetPrivate(cx, J_FOBJ, dst) );

	*J_FRVAL = OBJECT_TO_JSVAL( J_FOBJ );
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Substr ) { // http://developer.mozilla.org/en/docs/Core_JavaScript_1.5_Reference:Global_Objects:String:substr

	J_S_ASSERT_ARG_MIN(1);
	void *pv = JS_GetPrivate(cx, J_FOBJ);
	
	size_t dataLength;
	J_CHECK_CALL( LengthGet(cx, J_FOBJ, &dataLength) );

	int start;
	J_JSVAL_TO_INT32( J_FARG(1), start );

	if ( start >= (int)dataLength || start < -(int)dataLength ) {

		*J_FRVAL = OBJECT_TO_JSVAL( NewEmptyBString(cx) );
		return JS_TRUE;
	}

	if ( start < 0 ) 
		start = dataLength + start;

	if ( start < 0 || start >= (int)dataLength )
		start = 0;

	// now 0 <= start < dataLength

	int length;
	if ( J_FARG_ISDEF(2) ) {

		J_JSVAL_TO_INT32( J_FARG(2), length );
		if ( length <= 0 ) {

			*J_FRVAL = OBJECT_TO_JSVAL( NewEmptyBString(cx) );
			return JS_TRUE;
		}

		if ( start + length > (int)dataLength )
			length = dataLength - start;

	} else
		length = dataLength - start;

	// now 0 <= length < dataLength - start

	void *buffer = JS_malloc(cx, length +1);
	J_S_ASSERT_ALLOC( buffer );
	((char*)buffer)[length] = '\0';

	memcpy(buffer, ((int8_t*)pv) + start, length);

	*J_FRVAL = OBJECT_TO_JSVAL( NewBString(cx, buffer, length) );
	return JS_TRUE;
}


/*
DEFINE_FUNCTION_FAST( IndexOf ) {

	RT_ASSERT_ARGC(1);

	int start = 0;
	if ( J_FARG_ISDEF(2) )
		RT_JSVAL_TO_INT32( J_FARG(2), start );
}
*/


DEFINE_FUNCTION_FAST( valueOf ) {

	J_CHECK_CALL( BStringToJsval(cx, J_FOBJ, J_FRVAL) );
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( toString ) {

	J_CHECK_CALL( BStringToJsval(cx, J_FOBJ, J_FRVAL) );
	return JS_TRUE;
}


DEFINE_PROPERTY( length ) {

	size_t length;
	J_CHECK_CALL( LengthGet(cx, obj, &length) );
	*vp = INT_TO_JSVAL( length );
	return JS_TRUE;
}

/*
DEFINE_NEW_RESOLVE() { // support of data[n]

	if (!JSVAL_IS_INT(id) || (flags & JSRESOLVE_ASSIGNING))
		return JS_TRUE;
	jsint slot = JSVAL_TO_INT( id );

	void *pv = JS_GetPrivate(cx, obj);
	if ( pv == NULL )
		return JS_TRUE;

	int length;
	J_CHECK_CALL( LengthGet(cx, obj, &length) );

	if ( slot < 0 || slot >= length )
		return JS_TRUE;

	jschar chr = ((char*)pv)[slot];
	JSString *str1 = JS_NewUCStringCopyN(cx, &chr, 1);
	J_S_ASSERT_ALLOC( str1 );

	JS_DefineProperty(cx, obj, (char*)slot, STRING_TO_JSVAL(str1), NULL, NULL, JSPROP_INDEX );

	*objp = obj;

	return JS_TRUE;
}
*/


DEFINE_GET_PROPERTY() {

	if ( !JSVAL_IS_INT(id) )
		return JS_TRUE;
	
	jsint slot = JSVAL_TO_INT( id );

	void *pv = JS_GetPrivate(cx, obj);
	if ( pv == NULL )
		return JS_TRUE;

	size_t length;
	J_CHECK_CALL( LengthGet(cx, obj, &length) );

	if ( slot < 0 || slot >= (int)length )
		return JS_TRUE;

	jschar chr = ((char*)pv)[slot];
	JSString *str1 = JS_NewUCStringCopyN(cx, &chr, 1);
	J_S_ASSERT_ALLOC( str1 );

	*vp = STRING_TO_JSVAL(str1);

	return JS_TRUE;
}


DEFINE_SET_PROPERTY() {
		
	void *pv = JS_GetPrivate(cx, obj);
	if ( pv == NULL )
		J_REPORT_ERROR("Out of range.");

	size_t length;
	J_CHECK_CALL( LengthGet(cx, obj, &length) );

	J_S_ASSERT_INT(id);
	jsint slot = JSVAL_TO_INT( id );

	if ( slot < 0 || slot >= (int)length )
		J_REPORT_ERROR("Out of range.");

	jschar chr = ((char*)pv)[slot];
	JSString *str1 = JS_NewUCStringCopyN(cx, &chr, 1);
	J_S_ASSERT_ALLOC( str1 );

	J_S_ASSERT_STRING(*vp);
	if ( JS_GetStringLength( JSVAL_TO_STRING(*vp) ) != 1 )
		J_REPORT_ERROR("Invalid char.");

	((char*)pv)[slot] = *JS_GetStringBytes( JSVAL_TO_STRING(*vp) );

	return JS_TRUE;
}


CONFIGURE_CLASS

	HAS_CONSTRUCTOR
	HAS_FINALIZE
//	HAS_NEW_RESOLVE
	HAS_GET_PROPERTY
	HAS_SET_PROPERTY

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST(Add)
		FUNCTION_FAST(Set)
		FUNCTION_FAST(Substr)
		FUNCTION_FAST(valueOf)
		FUNCTION_FAST(toString)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ(length)
	END_PROPERTY_SPEC

	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1)

END_CLASS
