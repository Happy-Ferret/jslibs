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

inline JSBool LengthSet( JSContext *cx, JSObject *obj, size_t bufferLength ) {

	RT_CHECK_CALL( JS_SetReservedSlot(cx, obj, SLOT_BSTRING_LENGTH, INT_TO_JSVAL( bufferLength )) );
	return JS_TRUE;
}


inline JSBool LengthGet( JSContext *cx, JSObject *obj, size_t *bufferLength ) {

	jsval bufferLengthVal;
	RT_CHECK_CALL( JS_GetReservedSlot(cx, obj, SLOT_BSTRING_LENGTH, &bufferLengthVal) );
	*bufferLength = JSVAL_TO_INT(bufferLengthVal);
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
	RT_CHECK_CALL( LengthSet(cx, obj, 0) );

//	RT_ASSERT_ARGC( 1 );
//	RT_ASSERT_OBJECT( J_ARG(1) );
//	RT_ASSERT_CLASS( JSVAL_TO_OBJECT( J_ARG(1) ), &classBuffer );
//	RT_CHECK_CALL( JS_SetReservedSlot(cx, obj, SLOT_PACK_BUFFEROBJECT, J_ARG(1)) );
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Clear ) {
	
	char *pv = (char*)JS_GetPrivate(cx, J_FOBJ);
	if (pv != NULL)
		JS_free(cx, pv);
	RT_CHECK_CALL( LengthSet(cx, J_FOBJ, 0) );
	RT_CHECK_CALL( JS_SetPrivate(cx, J_FOBJ, NULL) );
	JS_ClearScope(cx, J_FOBJ);
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Set ) {
	
	RT_ASSERT_ARGC( 1 );

	size_t strLen;
	char* str;
	RT_JSVAL_TO_STRING_AND_LENGTH( J_FARG(1), str, strLen );

	char *pv = (char*)JS_GetPrivate(cx, J_FOBJ);
	if (pv == NULL)
		pv = (char*)JS_malloc(cx, strLen);
	else
		pv = (char*)JS_realloc(cx, pv, strLen);
	RT_ASSERT_ALLOC( pv );

	memcpy( pv, str, strLen );

	RT_CHECK_CALL( LengthSet(cx, J_FOBJ, strLen) );
	RT_CHECK_CALL( JS_SetPrivate(cx, J_FOBJ, pv) );
	JS_ClearScope(cx, J_FOBJ);
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Add ) {
	
	RT_ASSERT_ARGC( 1 );
	char *pv = (char*)JS_GetPrivate(cx, J_FOBJ);

	size_t strLen;
	char* str;
	RT_JSVAL_TO_STRING_AND_LENGTH( J_FARG(1), str, strLen );

	size_t length;
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


DEFINE_FUNCTION_FAST( toString ) {

	char *pv = (char*)JS_GetPrivate(cx, J_FOBJ);
	
	size_t length;
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

	size_t length;
	RT_CHECK_CALL( LengthGet(cx, obj, &length) );

	if ( slot >= length )
		return JS_TRUE;

	JSString *str1 = JS_NewStringCopyN(cx, pv + slot, 1);
	RT_ASSERT_ALLOC( str1 );

	JS_DefineProperty(cx, obj, (char*)slot, STRING_TO_JSVAL(str1), NULL, NULL, JSPROP_READONLY | JSPROP_INDEX );

	*objp = obj;

	return JS_TRUE;
}

/*
DEFINE_GET_PROPERTY() { // support of data[-1]

	if (!JSVAL_IS_INT(id))
		return JS_TRUE;
	jsint slot = JSVAL_TO_INT( id );
	if (slot == -1)
		RT_CHECK_CALL( JS_GetReservedSlot(cx, obj, SLOT_BSTRING_LENGTH, vp ) );
	return JS_TRUE;
}
*/

CONFIGURE_CLASS

	HAS_CONSTRUCTOR
	HAS_FINALIZE
//	HAS_GET_PROPERTY
	HAS_NEW_RESOLVE

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST(Add)
		FUNCTION_FAST(Set)
		FUNCTION_FAST(Clear)
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

