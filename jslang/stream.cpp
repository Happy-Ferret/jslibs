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

#define SLOT_STREAM_SOURCE 0
#define SLOT_STREAM_POSITION 1


inline JSBool PositionSet( JSContext *cx, JSObject *obj, int position ) {

	jsval tmp;
	J_CHECK_CALL( IntToJsval(cx, position, &tmp) );
	return JS_SetReservedSlot(cx, obj, SLOT_STREAM_POSITION, tmp);
}


inline JSBool PositionGet( JSContext *cx, JSObject *obj, int *position ) {

	jsval tmp;
	J_CHECK_CALL( JS_GetReservedSlot(cx, obj, SLOT_STREAM_POSITION, &tmp) );
	J_CHECK_CALL( JsvalToInt(cx, tmp, position) );
	return JS_TRUE;
}


JSBool StreamRead( JSContext *cx, JSObject *obj, char *buf, unsigned int *amount ) {

	int position;
	J_CHECK_CALL( PositionGet(cx, obj, &position) );
	jsval source;
	J_CHECK_CALL( JS_GetReservedSlot(cx, obj, SLOT_STREAM_SOURCE, &source) );
	size_t length;
	const char *buffer;
	J_CHECK_CALL( JsvalToStringAndLength(cx, source, &buffer, &length) );

	if ( length - position <= 0 ) { // position >= length

		*amount = 0; // EOF
		return JS_TRUE;
	}

	if ( position + *amount > length )
		*amount = length - position;
	memcpy( buf, buffer+position, *amount );
	position += *amount;
	PositionSet(cx, obj, position);
	return JS_TRUE;
}


BEGIN_CLASS( Stream )


DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_ARG_MIN( 1 );

	if ( JS_IsConstructing(cx) == JS_FALSE ) { // supports this form (w/o new operator) : result.param1 = Blob('Hello World');

		obj = JS_NewObject(cx, _class, NULL, NULL);
		J_S_ASSERT_ALLOC( obj );
		*rval = OBJECT_TO_JSVAL(obj);
	} else {

		J_S_ASSERT_THIS_CLASS();
	}

	J_CHECK_CALL( JS_SetReservedSlot(cx, obj, SLOT_STREAM_SOURCE, J_ARG(1)) );
	J_CHECK_CALL( PositionSet(cx, obj, 0) );
	J_CHECK_CALL( SetStreamReadInterface(cx, obj, StreamRead) );
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Read ) {

	J_S_ASSERT_ARG_MIN( 1 );

	int amount;
	J_CHECK_CALL( JsvalToInt(cx, J_FARG(1), &amount) );

	char *buffer = (char*)JS_malloc(cx, amount +1);
	J_S_ASSERT_ALLOC(buffer);

	size_t readAmount = amount;
	J_CHECK_CALL( StreamRead(cx, J_FOBJ, buffer, &readAmount ) );

	if ( MaybeRealloc(amount, readAmount) )
		buffer = (char*)JS_realloc(cx, buffer, readAmount +1);

	buffer[readAmount] = '\0';

	JSObject *bstrObj = NewBString(cx, buffer, readAmount);
	J_S_ASSERT( bstrObj != NULL, "Unable to create a BString object." );
	*J_FRVAL = OBJECT_TO_JSVAL(bstrObj);

	return JS_TRUE;
}


DEFINE_PROPERTY( positionGetter ) {

	int position;
	J_CHECK_CALL( PositionGet(cx, obj, &position) );
	*vp = INT_TO_JSVAL( position );
	return JS_TRUE;
}

DEFINE_PROPERTY( positionSetter ) {

	int position;
	J_CHECK_CALL( JsvalToInt(cx, *vp, &position) );
	J_S_ASSERT( position >= 0, "Invalid stream position." );
	J_CHECK_CALL( PositionSet(cx, obj, position) );
	return JS_TRUE;
}

DEFINE_PROPERTY( available ) {

	J_CHECK_CALL( JS_GetReservedSlot(cx, obj, SLOT_STREAM_SOURCE, vp) ); // use vp as a tmp variable
	JSObject *srcObj;
	if ( JSVAL_IS_OBJECT( *vp ) )
		srcObj = JSVAL_TO_OBJECT( *vp );
	else
		J_CHECK_CALL( JS_ValueToObject(cx, *vp, &srcObj) );
	int length, position;
	J_CHECK_CALL( PositionGet(cx, obj, &position) );
	JS_GetProperty(cx, srcObj, "length", vp); // use vp as a tmp variable
	if ( *vp == JSVAL_VOID )
		return JS_TRUE; // if length is not defined, the returned value is undefined
	J_CHECK_CALL( JsvalToInt(cx, *vp, &length) );
	J_CHECK_CALL( IntToJsval(cx, length - position, vp ) );
	return JS_TRUE;
}


CONFIGURE_CLASS

	HAS_CONSTRUCTOR
	HAS_RESERVED_SLOTS(2)

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST(Read)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY(position)
		PROPERTY_READ(available)
	END_PROPERTY_SPEC

END_CLASS

/*
	Adapter pattern:
		http://en.wikipedia.org/wiki/Adapter_pattern

*/