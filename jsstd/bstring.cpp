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
#include <jsobj.h>

#include "bstring.h"

#include "../common/jsNativeInterface.h"


inline JSBool LengthSet( JSContext *cx, JSObject *obj, int bufferLength ) {

	RT_CHECK_CALL( JS_SetReservedSlot(cx, obj, SLOT_BSTRING_LENGTH, INT_TO_JSVAL( bufferLength )) );
	return JS_TRUE;
}


inline JSBool LengthGet( JSContext *cx, JSObject *obj, int *bufferLength ) {

	jsval bufferLengthVal;
	RT_CHECK_CALL( JS_GetReservedSlot(cx, obj, SLOT_BSTRING_LENGTH, &bufferLengthVal) );
	if ( JSVAL_IS_INT(bufferLengthVal) )
		*bufferLength = JSVAL_TO_INT( bufferLengthVal );
	else
		*bufferLength = 0;
	return JS_TRUE;
}


BEGIN_CLASS( BString )


DEFINE_FINALIZE() {

	char *pv = (char*)JS_GetPrivate(cx, obj);
	if (pv != NULL)
		JS_free(cx, pv);
//	JS_SetPrivate(cx, J_FOBJ, NULL);
}


DEFINE_CONSTRUCTOR() {

	RT_ASSERT_CONSTRUCTING( _class );
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Clear ) {
	
	JS_ClearScope(cx, J_FOBJ);
	char *pv = (char*)JS_GetPrivate(cx, J_FOBJ);
	if (pv != NULL)
		JS_free(cx, pv);
	RT_CHECK_CALL( LengthSet(cx, J_FOBJ, 0) );
	RT_CHECK_CALL( JS_SetPrivate(cx, J_FOBJ, NULL) );
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Set ) {
	
	RT_ASSERT_ARGC( 1 );

	JS_ClearScope(cx, J_FOBJ);

	size_t strLen;
	char* str;
	RT_JSVAL_TO_STRING_AND_LENGTH( J_FARG(1), str, strLen );

	RT_CHECK_CALL( LengthSet(cx, J_FOBJ, strLen) );

	char *pv = (char*)JS_GetPrivate(cx, J_FOBJ);

	if ( strLen == 0 && pv != NULL ) {

		JS_free(cx, pv);
		RT_CHECK_CALL( JS_SetPrivate(cx, J_FOBJ, NULL) );
		return JS_TRUE;
	}

	if (pv == NULL)
		pv = (char*)JS_malloc(cx, strLen);
	else
		pv = (char*)JS_realloc(cx, pv, strLen);

	RT_ASSERT_ALLOC( pv );

	memcpy( pv, str, strLen );

	RT_CHECK_CALL( JS_SetPrivate(cx, J_FOBJ, pv) );
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Add ) {
	
	RT_ASSERT_ARGC( 1 );
	char *pv = (char*)JS_GetPrivate(cx, J_FOBJ);

	size_t strLen;
	char* str;
	RT_JSVAL_TO_STRING_AND_LENGTH( J_FARG(1), str, strLen );

	int length;
	RT_CHECK_CALL( LengthGet(cx, J_FOBJ, &length) );

	if (pv == NULL)
		pv = (char*)JS_malloc(cx, strLen + length);
	else
		pv = (char*)JS_realloc(cx, pv, strLen + length);
	RT_ASSERT_ALLOC( pv );

	memcpy( pv + length, str, strLen );

	length += strLen;
	RT_CHECK_CALL( LengthSet(cx, J_FOBJ, length) );
	RT_CHECK_CALL( JS_SetPrivate(cx, J_FOBJ, pv) );
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

		*J_FRVAL = JS_GetEmptyStringValue(cx);
		return JS_TRUE;
	}

	if ( start < 0 ) 
		start = dataLength + start;

	if ( start < 0 || start >= dataLength )
		start = 0;

	int length;
	if ( J_FARG_ISDEF(2) ) {

		RT_JSVAL_TO_INT32( J_FARG(2), length );
		if ( length <= 0 ) {

			*J_FRVAL = JS_GetEmptyStringValue(cx);
			return JS_TRUE;
		}

		if ( start + length > dataLength )
			length = dataLength - start;

	} else
		length = dataLength - start; // ??

	JSString *jsstr = JS_NewStringCopyN(cx, pv + start, length);
	*J_FRVAL = STRING_TO_JSVAL( jsstr );

	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( toString ) {

	char *pv = (char*)JS_GetPrivate(cx, J_FOBJ);
	int length;
	RT_CHECK_CALL( LengthGet(cx, J_FOBJ, &length) );
	if ( pv == NULL || length == 0 ) {

		*J_FRVAL = JS_GetEmptyStringValue(cx);
	} else {

		JSString *jsstr = JS_NewStringCopyN(cx, pv, length);
		*J_FRVAL = STRING_TO_JSVAL( jsstr );
	}
	return JS_TRUE;
}


DEFINE_PROPERTY( length ) {

	RT_CHECK_CALL( JS_GetReservedSlot(cx, obj, SLOT_BSTRING_LENGTH, vp ) );
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

	JSString *str1 = JS_NewStringCopyN(cx, pv + slot, 1);
	RT_ASSERT_ALLOC( str1 );

	JS_DefineProperty(cx, obj, (char*)slot, STRING_TO_JSVAL(str1), NULL, NULL, JSPROP_READONLY | JSPROP_INDEX );

	*objp = obj;

	return JS_TRUE;
}

DEFINE_SET_PROPERTY() {
		
	RT_ASSERT_STRING(*vp);

	jsint slot = JSVAL_TO_INT( id );

	char *pv = (char*)JS_GetPrivate(cx, obj);
	if ( pv == NULL )
		return JS_TRUE;

	int length;
	RT_CHECK_CALL( LengthGet(cx, obj, &length) );

	if ( slot < 0 || slot >= length )
		REPORT_ERROR("Out of range.");

	JSString *str1 = JS_NewStringCopyN(cx, pv + slot, 1);
	RT_ASSERT_ALLOC( str1 );

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
		FUNCTION_FAST(Clear)
		FUNCTION_FAST(Substr)
		FUNCTION_FAST(toString)
//		FUNCTION_ALIAS(valueOf, toString)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
//		PROPERTY(content)
		PROPERTY_READ(length)
	END_PROPERTY_SPEC

	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1)

END_CLASS

