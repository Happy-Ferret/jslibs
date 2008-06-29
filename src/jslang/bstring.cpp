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

#include "../common/errors.h"

#include <cstring>

#include "../common/jsHelper.h"

#include "bstringapi.h"


inline JSBool LengthSet( JSContext *cx, JSObject *obj, size_t bufferLength ) {

	return JS_SetReservedSlot(cx, obj, SLOT_BSTRING_LENGTH, INT_TO_JSVAL( bufferLength ));
}


JSBool NativeInterfaceBufferGet( JSContext *cx, JSObject *obj, const char **buf, size_t *size ) {

	J_CHK( BStringLength(cx, obj, size) );
	J_CHK( BStringBuffer(cx, obj, (const void **)buf) );
	return JS_TRUE;
}


/**doc
$CLASS_HEADER
**/
BEGIN_CLASS( BString )


inline JSBool JsvalToBString( JSContext *cx, jsval val, JSObject **obj ) {

	size_t srcLen;
	void *src, *dst = NULL;

	if ( JsvalIsBString(cx, val) ) {

		BStringGetBufferAndLength(cx, JSVAL_TO_OBJECT( val ), &src, &srcLen);
		if ( srcLen > 0 ) {

			dst = JS_malloc(cx, srcLen +1);
			J_S_ASSERT_ALLOC( dst );
			((char*)dst)[srcLen] = '\0';
			memcpy(dst, src, srcLen);
		}
	} else {

		JSString *jsstr = JS_ValueToString(cx, val);
		J_S_ASSERT( jsstr != NULL, "Unable to convert to string." );
		srcLen = J_STRING_LENGTH(jsstr);
		if ( srcLen > 0 ) {

			dst = JS_malloc(cx, srcLen +1);
			J_S_ASSERT_ALLOC( dst );
			((char*)dst)[srcLen] = '\0';

			// (TBD) try to know if the string is deflated befor using JS_GetStringChars ??
			jschar *chars = JS_GetStringChars(jsstr);
			for ( size_t i = 0; i < srcLen; i++ )
				((char*)dst)[i] = (uint8)chars[i];
		}
	}
	*obj = NewBString(cx, dst, srcLen);
	return JS_TRUE;
}


JSBool BStringToJSString( JSContext *cx, JSObject *obj, JSString **jsstr ) {

	void *pv = JS_GetPrivate(cx, obj);
	size_t length;
	J_CHECK_CALL( BStringLength(cx, obj, &length) );
	if ( pv == NULL || length == 0 ) {

		*jsstr = JSVAL_TO_STRING( JS_GetEmptyStringValue(cx) );
	} else {

		jschar *ucStr = (jschar*)JS_malloc(cx, (length + 1) * sizeof(jschar));
		ucStr[length] = 0;
		for ( size_t i = 0; i < length; i++ )
			ucStr[i] = ((unsigned char*)pv)[i]; // see js_InflateString in jsstr.c
		*jsstr = JS_NewUCString(cx, ucStr, length);
	}
	return JS_TRUE;
}



DEFINE_FINALIZE() {

	void *pv = JS_GetPrivate(cx, obj);
	if (pv != NULL)
		JS_free(cx, pv);
}

/**doc
 * $INAME()
  Creates an object that can contain binary data.
  $H note
  When called in a non-constructor context, Object behaves identically.
**/
DEFINE_CONSTRUCTOR() {

	if ( JS_IsConstructing(cx) == JS_FALSE ) { // supports this form (w/o new operator) : result.param1 = Blob('Hello World');

		obj = JS_NewObject(cx, _class, NULL, NULL);
		J_S_ASSERT( obj != NULL, "BString construction failed." );
		*rval = OBJECT_TO_JSVAL(obj);
	} else
		J_S_ASSERT_THIS_CLASS();

	if ( J_ARG_ISDEF(1) ) {

		size_t length;
		const char *sBuffer;
		J_CHECK_CALL( JsvalToStringAndLength(cx, J_ARG(1), &sBuffer, &length) );
		J_CHECK_CALL( LengthSet(cx, obj, length) );
		void *dBuffer = JS_malloc(cx, length +1);
		((char*)dBuffer)[length] = '\0';
		memcpy(dBuffer, sBuffer, length);
		JS_SetPrivate(cx, obj, dBuffer);
	}

	J_CHK( InitBufferGetInterface(cx, obj) );
	J_CHK( SetBufferGetInterface(cx, obj, NativeInterfaceBufferGet) );

	return JS_TRUE;
}

/*
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
*/

/**doc
 * $TYPE BString $INAME( ??? )
  TBD
**/
DEFINE_FUNCTION_FAST( Add ) {

	J_S_ASSERT_ARG_MIN( 1 );
	J_S_ASSERT_ARG_MAX( 1 );

	size_t length;
	J_CHECK_CALL( BStringLength(cx, J_FOBJ, &length) );

	size_t srcLen;
	void *src, *dst;

	if ( JsvalIsBString(cx, J_FARG(1)) ) {

		BStringGetBufferAndLength(cx, JSVAL_TO_OBJECT( J_FARG(1) ), &src, &srcLen);
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

		srcLen = J_STRING_LENGTH(jsstr);
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

/* mutable BString are no more supported
	void *pv = JS_GetPrivate(cx, J_FOBJ);
	if ( pv != NULL ) {

		memcpy(dst, pv, length);
		JS_free(cx, pv);
	}

	J_CHECK_CALL( LengthSet(cx, J_FOBJ, srcLen + length) );
	J_CHECK_CALL( JS_SetPrivate(cx, J_FOBJ, dst) );

	*J_FRVAL = OBJECT_TO_JSVAL( J_FOBJ );
*/

// copy the begining
	void *pv = JS_GetPrivate(cx, J_FOBJ);
	if ( pv != NULL )
		memcpy(dst, pv, length);


	JSObject *newBStrObj = JS_NewObject(cx, _class, NULL, NULL);
	J_S_ASSERT( newBStrObj != NULL, "Unable to create the new BString" );

	J_CHECK_CALL( LengthSet(cx, newBStrObj, srcLen + length) );
	J_CHECK_CALL( JS_SetPrivate(cx, newBStrObj, dst) );

	*J_FRVAL = OBJECT_TO_JSVAL( newBStrObj );

	return JS_TRUE;
}



/**doc
 * $TYPE ??? $INAME
  TBD
**/
DEFINE_FUNCTION_FAST( Substr ) { // http://developer.mozilla.org/en/docs/Core_JavaScript_1.5_Reference:Global_Objects:String:substr

	J_S_ASSERT_ARG_MIN(1);
	void *pv = JS_GetPrivate(cx, J_FOBJ);

	size_t dataLength;
	J_CHECK_CALL( BStringLength(cx, J_FOBJ, &dataLength) );

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


/**doc
 * $STR ??? $INAME
  TBD
**/
DEFINE_FUNCTION_FAST( toString ) { // and valueOf

	JSString *jsstr;
	J_CHK( BStringToJSString(cx, J_FOBJ, &jsstr) );
	J_S_ASSERT( jsstr != NULL, "Unable to convert BString to String." );
	*J_FRVAL = STRING_TO_JSVAL(jsstr);
	return JS_TRUE;
}


/**doc
 * $INT ??? $INAME
  TBD
**/
DEFINE_PROPERTY( length ) {

	size_t length;
	J_CHECK_CALL( BStringLength(cx, obj, &length) );
	*vp = INT_TO_JSVAL( length );
	return JS_TRUE;
}


/*
DEFINE_NEW_RESOLVE() { // support of data[n]  and  n in data

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


/**doc
 * $TYPE char *_[] operator_*
  TBD
**/

DEFINE_GET_PROPERTY() {

	if ( !JSVAL_IS_INT(id) )
		return JS_TRUE;

	jsint slot = JSVAL_TO_INT( id );

	void *pv = JS_GetPrivate(cx, obj);
	if ( pv == NULL )
		return JS_TRUE;

	size_t length;
	J_CHECK_CALL( BStringLength(cx, obj, &length) );

	if ( slot < 0 || slot >= (int)length )
		return JS_TRUE;

	jschar chr = ((char*)pv)[slot];
	JSString *str1 = JS_NewUCStringCopyN(cx, &chr, 1);
	J_S_ASSERT_ALLOC( str1 );

	*vp = STRING_TO_JSVAL(str1);

	return JS_TRUE;
}


DEFINE_SET_PROPERTY() {

	J_S_ASSERT( !JSVAL_IS_NUMBER(id), "Cannot modify immutable string" );

/* mutable BString are no more supported

	void *pv = JS_GetPrivate(cx, obj);

	if ( pv == NULL )
		return J_ReportError(cx, JSSMSG_OUT_OF_RANGE);
	size_t length;
	J_CHECK_CALL( LengthGet(cx, obj, &length) );

	J_S_ASSERT_INT(id);
	jsint slot = JSVAL_TO_INT( id );

	if ( slot < 0 || slot >= (int)length )
		return J_ReportError(cx, JSSMSG_OUT_OF_RANGE);

	jschar chr = ((char*)pv)[slot];
	JSString *str1 = JS_NewUCStringCopyN(cx, &chr, 1);
	J_S_ASSERT_ALLOC( str1 );

	J_S_ASSERT_STRING(*vp);
	if ( J_STRING_LENGTH( JSVAL_TO_STRING(*vp) ) != 1 )
		J_REPORT_ERROR("Invalid char.");

	((char*)pv)[slot] = *JS_GetStringBytes( JSVAL_TO_STRING(*vp) );
*/
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
//		FUNCTION_FAST(Set)
		FUNCTION_FAST(Substr)
		FUNCTION_FAST(toString)
		FUNCTION_FAST_ALIAS(valueOf, toString)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ(length)
	END_PROPERTY_SPEC

	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1)

END_CLASS
