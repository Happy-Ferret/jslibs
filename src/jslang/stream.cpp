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

#define SLOT_STREAM_SOURCE 0
#define SLOT_STREAM_POSITION 1

DECLARE_CLASS( Stream );


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 3533 $
**/
BEGIN_CLASS( Stream )


ALWAYS_INLINE JSBool
GetStreamSource(JSContext *, JSObject *obj, JSObject **srcObj) {

	jsval val;
	JL_CHK( JL_GetReservedSlot( obj, SLOT_STREAM_SOURCE, &val) );
	ASSERT( JSVAL_IS_OBJECT( val ) );
	*srcObj = JSVAL_TO_OBJECT( val );
	return JS_TRUE;
	JL_BAD;
}

ALWAYS_INLINE JSBool
SetStreamSource(JSContext *cx, JSObject *obj, jsval *srcVal) {

	if ( JSVAL_IS_PRIMITIVE( *srcVal ) ) {

		JSObject *tmpObj;
		JL_CHK( JS_ValueToObject(cx, *srcVal, &tmpObj) );
		*srcVal = OBJECT_TO_JSVAL(tmpObj);
	}

	JL_CHK( JL_SetReservedSlot( obj, SLOT_STREAM_SOURCE, *srcVal) );
	return JS_TRUE;
	JL_BAD;
}

ALWAYS_INLINE JSBool
SetPosition(JSContext *cx, JSObject *obj, size_t position) {

	JL_CHK( JL_NativeToReservedSlot(cx, JL_OBJ, SLOT_STREAM_POSITION, position) );
	return JS_TRUE;
	JL_BAD;
}

ALWAYS_INLINE JSBool
GetPosition(JSContext *cx, JSObject *obj, size_t *position) {

	JL_CHK( JL_ReservedSlotToNative(cx, JL_OBJ, SLOT_STREAM_POSITION, position) );
	return JS_TRUE;
	JL_BAD;
}


ALWAYS_INLINE JSBool
GetAvailable(JSContext *cx, JSObject *obj, size_t *available) {

	size_t position, length;
	JSObject *srcObj;
	JL_CHK( GetPosition(cx, obj, &position) );
	JL_CHK( GetStreamSource(cx, obj, &srcObj) );

	JSBool found;
	JL_CHK( JS_HasPropertyById(cx, obj, JLID(cx, length), &found) );

	jsval val;
	JL_CHK( JS_GetPropertyByIdDefault(cx, obj, JLID(cx, length), JSVAL_VOID, &val) );

	if ( !JSVAL_IS_VOID(val) ) {

		JL_CHK( JL_JsvalToNative(cx, val, &length) );
	} else {

		JLData data;
		val = OBJECT_TO_JSVAL(srcObj);
		JL_CHK( JL_JsvalToNative(cx, val, &data) );
		JL_CHK( data.IsSet() );
		length = data.Length();
	}

	*available = length - position;
	return JS_TRUE;
	JL_BAD;
}


ALWAYS_INLINE JSBool
StreamRead( JSContext *cx, JSObject *streamObj, char *buf, size_t *amount ) {

	JLData data;
	size_t position;
	jsval source;

	JL_ASSERT_INSTANCE(streamObj, JL_THIS_CLASS);

	JL_CHK( GetPosition(cx, streamObj, &position) );
	JL_CHK( JL_GetReservedSlot( streamObj, SLOT_STREAM_SOURCE, &source) );
	JL_CHK( JL_JsvalToNative(cx, source, &data) );

	size_t length = data.Length();
	if ( length - position <= 0 ) { // position >= length

		*amount = 0; // EOF
		return JS_TRUE;
	}

	if ( position + *amount > length )
		*amount = length - position;

	jl::memcpy( buf, data.GetConstStr() + position, *amount ); // (TBD) possible optimization. see JLData::CopyTo() ?
	JL_CHK( SetPosition(cx, streamObj, position + *amount) );
	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $INAME( bufferObject )
  Creates an object that transforms any buffer-like objects into a stream.
  $LF
  buffer-like objects are: string, Blob, and any objects that implements NIBufferGet native interface.
  $LF
  $H note
  When called in a non-constructor context, Object behaves identically.
**/
DEFINE_CONSTRUCTOR() {

	JL_DEFINE_CONSTRUCTOR_OBJ;
	// JL_ASSERT_CONSTRUCTING(); // supports this form (w/o new operator) : result.param1 = Blob('Hello World');
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN( 1 );

	JL_CHK( SetStreamSource(cx, JL_OBJ, &JL_ARG(1)) );
	JL_CHK( SetPosition(cx, JL_OBJ, 0) );
	JL_CHK( SetStreamReadInterface(cx, JL_OBJ, StreamRead) );

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE Blob $INAME( amount )
  Read _amount_ of data into the stream.
**/
DEFINE_FUNCTION( read ) {

	uint8_t *buffer = NULL;

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_RANGE(0, 1);

	size_t amount, available;

	JL_CHK( GetAvailable(cx, obj, &available) );

	if ( JL_ARG_ISDEF(1) ) {

		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &amount) );
		if ( amount == 0 ) {

			JL_CHK( JL_NewEmptyBuffer(cx, JL_RVAL) );
			return JS_TRUE;
		}
		if ( available < amount )
			amount = available;
	} else {

		amount = available;
	}

	if ( available == 0 ) {

		*JL_RVAL = JSVAL_VOID;
		return JS_TRUE;
	}

	buffer = JL_NewBuffer(cx, amount, JL_RVAL);
	JL_CHK( buffer );

	size_t readAmount;
	readAmount = amount;

	JL_CHK( StreamRead(cx, JL_OBJ, (char*)buffer, &readAmount) );

	if ( readAmount == 0 ) {

		*JL_RVAL = JSVAL_VOID;
		return JS_TRUE;
	}

	if ( readAmount != amount )
		JL_ChangeBufferLength(cx, JL_RVAL, readAmount);

	return JS_TRUE;
bad:
	JL_DataBufferFree(cx, buffer);
	return JS_FALSE;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME
  Get or set the stream pointer position.
**/
DEFINE_PROPERTY_GETTER( position ) {

	JL_IGNORE(id);
	JL_ASSERT_INSTANCE(obj, JL_THIS_CLASS);
	size_t position;
	JL_CHK( GetPosition(cx, obj, &position) );
	JL_CHK( JL_NativeToJsval(cx, position, vp) );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( position ) {

	JL_IGNORE(id, strict);
	JL_ASSERT_INSTANCE(obj, JL_THIS_CLASS);
	size_t position;
	JL_CHK( JL_JsvalToNative(cx, *vp, &position) );
	JL_CHK( SetPosition(cx, obj, position) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  The remaining data from the stream pointer position to the end of the stream.
**/
/**qa
	QA.ASSERTOP( Stream(stringify('1234ABCDEF', true)), 'has', 'available' );
	QA.ASSERTOP( Stream(stringify('1234ABCDEF', true)).available, '===', 10 );
	QA.ASSERTOP( Stream(Int8Array(stringify("1234ABCDEF", true))).available, '==', 10 );
**/
DEFINE_PROPERTY_GETTER( available ) {

	JL_IGNORE(id);
	JL_ASSERT_INSTANCE(obj, JL_THIS_CLASS);
	size_t available;
	JL_CHK( GetAvailable(cx, obj, &available) );
	JL_CHK( JL_NativeToJsval(cx, available, vp) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $OBJ $INAME
  The object used to create the steam.
**/
DEFINE_PROPERTY_GETTER( source ) {

	JL_IGNORE(id);
	JL_ASSERT_INSTANCE(obj, JL_THIS_CLASS);
	JL_CHK( JL_GetReservedSlot( obj, SLOT_STREAM_SOURCE, vp) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
=== note ===
 Basically, a Stream is nothing else that a buffer with a stream pointer position.
**/


/**doc
=== Native Interface ===
 * *NIStreamRead*
**/

CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision: 3533 $"))
	HAS_CONSTRUCTOR
	HAS_RESERVED_SLOTS(2)

	BEGIN_FUNCTION_SPEC
		FUNCTION(read)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY(position)
		PROPERTY_GETTER(available)
		PROPERTY_GETTER(source)
	END_PROPERTY_SPEC

END_CLASS

/*
	Adapter pattern:
		http://en.wikipedia.org/wiki/Adapter_pattern

*/
