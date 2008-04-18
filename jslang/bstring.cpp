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
#include "bstring.h"

//#include "../common/jsNativeInterface.h"


inline JSBool LengthSet( JSContext *cx, JSObject *obj, int bufferLength ) {

	RT_CHECK_CALL( JS_SetReservedSlot(cx, obj, SLOT_BSTRING_LENGTH, INT_TO_JSVAL( bufferLength )) );
	return JS_TRUE;
}


inline JSBool LengthGet( JSContext *cx, JSObject *obj, int *bufferLength ) {

	jsval bufferLengthVal;
	RT_CHECK_CALL( JS_GetReservedSlot(cx, obj, SLOT_BSTRING_LENGTH, &bufferLengthVal) );
	*bufferLength = JSVAL_IS_INT(bufferLengthVal) ? JSVAL_TO_INT( bufferLengthVal ) : 0;
	return JS_TRUE;
}


BEGIN_CLASS( BString )

inline bool JsvalIsBString( JSContext *cx, jsval val ) {

	return JSVAL_IS_OBJECT(val) && !JSVAL_IS_NULL(val) && JS_GET_CLASS(cx, JSVAL_TO_OBJECT(val)) == &classBString; // == BStringJSClass(cx);
}


DEFINE_FINALIZE() {

	char *pv = (char*)JS_GetPrivate(cx, obj);
	if (pv != NULL)
		JS_free(cx, pv);
}


DEFINE_CONSTRUCTOR() {

	RT_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Set ) {
	
	JS_ClearScope(cx, J_FOBJ);

	char *pv = (char*)JS_GetPrivate(cx, J_FOBJ);

	if ( !J_FARG_ISDEF(1) ) { // clear

		if (pv != NULL)
			JS_free(cx, pv);
		RT_CHECK_CALL( JS_SetPrivate(cx, J_FOBJ, NULL) );
		RT_CHECK_CALL( LengthSet(cx, J_FOBJ, 0) );
		return JS_TRUE;
	}

	size_t srcLen;
	char *src, *dst;

	if ( JsvalIsBString(cx, J_FARG(1)) ) {
		
		BStringGetDataAndLength(cx, JSVAL_TO_OBJECT( J_FARG(1) ), &src, &srcLen);
		if ( srcLen > 0 ) {

			dst = (char*)JS_malloc(cx, srcLen);
			RT_ASSERT_ALLOC( dst );
			memcpy(dst, src, srcLen);
		} else {

			dst = NULL;
		}
	} else {
		
		JSString *jsstr = JS_ValueToString(cx, J_FARG(1));
		J_FARG(1) = STRING_TO_JSVAL(jsstr);

		srcLen = JS_GetStringLength(jsstr);
		if ( srcLen > 0 ) {

			dst = (char*)JS_malloc(cx, srcLen);
			RT_ASSERT_ALLOC( dst );
			jschar *chars = JS_GetStringChars(jsstr);
			for ( size_t i = 0; i < srcLen; i++ )
				dst[i] = (uint8)chars[i];
		} else {
			
			dst = NULL;
		}
	}

	if ( pv != NULL )
		JS_free(cx, pv);

	RT_CHECK_CALL( JS_SetPrivate(cx, J_FOBJ, dst) );
	RT_CHECK_CALL( LengthSet(cx, J_FOBJ, srcLen) );

	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Add ) {
	
	RT_ASSERT_ARGC( 1 );

	int length;
	RT_CHECK_CALL( LengthGet(cx, J_FOBJ, &length) );

	size_t srcLen;
	char *src, *dst;

	if ( JsvalIsBString(cx, J_FARG(1)) ) {
		
		BStringGetDataAndLength(cx, JSVAL_TO_OBJECT( J_FARG(1) ), &src, &srcLen);
		if ( srcLen > 0 ) {

			dst = (char*)JS_malloc(cx, srcLen + length);
			RT_ASSERT_ALLOC( dst );
			memcpy(dst+length, src, srcLen);
		} else {

			dst = NULL;
		}
	} else {
		
		JSString *jsstr = JS_ValueToString(cx, J_FARG(1));
		J_FARG(1) = STRING_TO_JSVAL(jsstr);

		srcLen = JS_GetStringLength(jsstr);
		if ( srcLen > 0 ) {

			dst = (char*)JS_malloc(cx, srcLen + length);
			RT_ASSERT_ALLOC( dst );
			jschar *chars = JS_GetStringChars(jsstr);
			for ( size_t i = 0; i < srcLen; i++ )
				dst[i+length] = (uint8)chars[i];
		} else {
			
			dst = NULL;
		}
	}

	char *pv = (char*)JS_GetPrivate(cx, J_FOBJ);
	if ( pv != NULL ) {
		
		memcpy(dst, pv, length);
		JS_free(cx, pv);
	}

	RT_CHECK_CALL( LengthSet(cx, J_FOBJ, srcLen + length) );
	RT_CHECK_CALL( JS_SetPrivate(cx, J_FOBJ, dst) );

	*J_FRVAL = OBJECT_TO_JSVAL( J_FOBJ );
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Substr ) { // http://developer.mozilla.org/en/docs/Core_JavaScript_1.5_Reference:Global_Objects:String:substr

	RT_ASSERT_ARGC(1);
	char *pv = (char*)JS_GetPrivate(cx, J_FOBJ);
	
	int dataLength;
	RT_CHECK_CALL( LengthGet(cx, J_FOBJ, &dataLength) );

	int start;
	RT_JSVAL_TO_INT32( J_FARG(1), start );

	if ( start >= dataLength || start < -dataLength ) {

		*J_FRVAL = OBJECT_TO_JSVAL( EmptyBString(cx) );
		return JS_TRUE;
	}

	if ( start < 0 ) 
		start = dataLength + start;

	if ( start < 0 || start >= dataLength )
		start = 0;

	// now 0 <= start < dataLength

	int length;
	if ( J_FARG_ISDEF(2) ) {

		RT_JSVAL_TO_INT32( J_FARG(2), length );
		if ( length <= 0 ) {

			*J_FRVAL = OBJECT_TO_JSVAL( EmptyBString(cx) );
			return JS_TRUE;
		}

		if ( start + length > dataLength )
			length = dataLength - start;

	} else
		length = dataLength - start;

	// now 0 <= length < dataLength - start

	char *buffer = (char*)JS_malloc(cx, length);
	RT_ASSERT_ALLOC( buffer );
	memcpy(buffer, pv + start, length);

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


DEFINE_FUNCTION_FAST( toString ) {

	char *pv = (char*)JS_GetPrivate(cx, J_FOBJ);
	int length;
	RT_CHECK_CALL( LengthGet(cx, J_FOBJ, &length) );
	if ( pv == NULL || length == 0 ) {

		*J_FRVAL = JS_GetEmptyStringValue(cx);
	} else {

		jschar *ucStr = (jschar*)JS_malloc(cx, length * sizeof(jschar));
		for ( int i = 0; i < length; i++ )
			ucStr[i] = pv[i];
		JSString *jsstr = JS_NewUCString(cx, ucStr, length);
		*J_FRVAL = STRING_TO_JSVAL( jsstr );
	}
	return JS_TRUE;
}


DEFINE_PROPERTY( length ) {

	int length;
	RT_CHECK_CALL( LengthGet(cx, obj, &length) );
	*vp = INT_TO_JSVAL( length );
	return JS_TRUE;
}


DEFINE_NEW_RESOLVE() { // support of data[n]

	if (!JSVAL_IS_INT(id) || (flags & JSRESOLVE_ASSIGNING))
		return JS_TRUE;
	jsint slot = JSVAL_TO_INT( id );

	char *pv = (char*)JS_GetPrivate(cx, obj);
	if ( pv == NULL )
		return JS_TRUE;

	int length;
	RT_CHECK_CALL( LengthGet(cx, obj, &length) );

	if ( slot < 0 || slot >= length )
		return JS_TRUE;

	jschar chr = pv[slot];
	JSString *str1 = JS_NewUCStringCopyN(cx, &chr, 1);
	RT_ASSERT_ALLOC( str1 );

	JS_DefineProperty(cx, obj, (char*)slot, STRING_TO_JSVAL(str1), NULL, NULL, JSPROP_INDEX );

	*objp = obj;

	return JS_TRUE;
}


DEFINE_SET_PROPERTY() {
		
	char *pv = (char*)JS_GetPrivate(cx, obj);
	if ( pv == NULL )
		REPORT_ERROR("Out of range.");

	int length;
	RT_CHECK_CALL( LengthGet(cx, obj, &length) );

	RT_ASSERT_INT(id);
	jsint slot = JSVAL_TO_INT( id );

	if ( slot < 0 || slot >= length )
		REPORT_ERROR("Out of range.");

	jschar chr = pv[slot];
	JSString *str1 = JS_NewUCStringCopyN(cx, &chr, 1);
	RT_ASSERT_ALLOC( str1 );

	RT_ASSERT_STRING(*vp);
	if ( JS_GetStringLength( JSVAL_TO_STRING(*vp) ) != 1 )
		REPORT_ERROR("Invalid char.");

	pv[slot] = *JS_GetStringBytes( JSVAL_TO_STRING(*vp) );

	return JS_TRUE;
}


CONFIGURE_CLASS

	HAS_CONSTRUCTOR
	HAS_FINALIZE
	HAS_NEW_RESOLVE
	HAS_SET_PROPERTY

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST(Add)
		FUNCTION_FAST(Set)
		FUNCTION_FAST(Substr)
		FUNCTION_FAST(toString)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ(length)
	END_PROPERTY_SPEC

	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1)

END_CLASS
