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
//#include "blobPub.h"

#define SLOT_STREAM_SOURCE 0
#define SLOT_STREAM_POSITION 1

DECLARE_CLASS( Stream );

inline JSBool PositionSet( JSContext *cx, JSObject *streamObj, size_t position ) {

	jsval tmp;
	JL_CHK( JL_NativeToJsval(cx, position, &tmp) );
	return JL_SetReservedSlot(cx, streamObj, SLOT_STREAM_POSITION, tmp);
	JL_BAD;
}


inline JSBool PositionGet( JSContext *cx, JSObject *streamObj, size_t *position ) {

	jsval tmp;
	JL_CHK( JL_GetReservedSlot(cx, streamObj, SLOT_STREAM_POSITION, &tmp) );
	JL_CHK( JL_JsvalToNative(cx, tmp, position) );
	return JS_TRUE;
	JL_BAD;
}


JSBool StreamRead( JSContext *cx, JSObject *streamObj, char *buf, size_t *amount ) {

	JL_ASSERT_INSTANCE(streamObj, JL_CLASS(Stream));

	size_t position;
	JL_CHK( PositionGet(cx, streamObj, &position) );
	jsval source;
	JL_CHK( JL_GetReservedSlot(cx, streamObj, SLOT_STREAM_SOURCE, &source) );

	{
	JLStr str;
	JL_CHK( JL_JsvalToNative(cx, source, &str) ); // (TBD) GC protect source

	size_t length = str.Length();
	if ( length - position <= 0 ) { // position >= length

		*amount = 0; // EOF
		return JS_TRUE;
	}

	if ( position + *amount > length )
		*amount = length - position;

	memcpy( buf, str.GetConstStr() + position, *amount );
	position += *amount;
	PositionSet(cx, streamObj, position);
	}
	return JS_TRUE;
	JL_BAD;
}


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( Stream )

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
//	JL_ASSERT_ARG_IS_OBJECT(1);

	JL_CHK( JL_SetReservedSlot(cx, JL_OBJ, SLOT_STREAM_SOURCE, JL_ARG(1)) );
	JL_CHK( PositionSet(cx, JL_OBJ, 0) );

	// JL_CHK( ReserveStreamReadInterface(cx, JL_OBJ) );
	JL_CHK( SetStreamReadInterface(cx, JL_OBJ, StreamRead) );

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE Blob $INAME( amount )
  Read _amount_ of data into the stream.
**/
DEFINE_FUNCTION( Read ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN( 1 );

	int amount;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &amount) );

	char *buffer;
	buffer = (char*)JS_malloc(cx, amount +1);
	JL_CHK(buffer);

	size_t readAmount;
	readAmount = amount;
	JL_CHK( StreamRead(cx, JL_OBJ, buffer, &readAmount ) );

	if ( JL_MaybeRealloc(amount, readAmount) )
		buffer = (char*)JS_realloc(cx, buffer, readAmount +1);

	buffer[readAmount] = '\0';
	JL_CHK( JL_NewBlob(cx, buffer, readAmount, JL_RVAL) );

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME
  Get or set the stream pointer position.
**/
DEFINE_PROPERTY_GETTER( position ) {

	JL_USE(id);

	JL_ASSERT_INSTANCE(obj, JL_THIS_CLASS);
	size_t position;
	JL_CHK( PositionGet(cx, obj, &position) );
	return JL_NativeToJsval(cx, position, vp);
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( position ) {

	JL_USE(id);
	JL_USE(strict);

	JL_ASSERT_INSTANCE(obj, JL_THIS_CLASS);
	size_t position;
	JL_CHK( JL_JsvalToNative(cx, *vp, &position) );
	JL_ASSERT( position >= 0, E_VALUE, E_MIN, E_NUM(0) );

	JL_CHK( PositionSet(cx, obj, position) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  The remaining data from the stream pointer position to the end of the stream.
**/
DEFINE_PROPERTY_GETTER( available ) {

	JL_USE(id);

	JL_ASSERT_INSTANCE(obj, JL_THIS_CLASS);
	JL_CHK( JL_GetReservedSlot(cx, obj, SLOT_STREAM_SOURCE, vp) ); // use vp as a tmp variable
	JSObject *srcObj;
	if ( JSVAL_IS_OBJECT( *vp ) )
		srcObj = JSVAL_TO_OBJECT( *vp );
	else
		JL_CHK( JS_ValueToObject(cx, *vp, &srcObj) );
	size_t length, position;
	JL_CHK( PositionGet(cx, obj, &position) );
	JL_CHK( JS_GetProperty(cx, srcObj, "length", vp) ); // use vp as a tmp variable
	if ( JSVAL_IS_VOID( *vp ) )
		return JS_TRUE; // if length is not defined, the returned value is undefined
	JL_CHK( JL_JsvalToNative(cx, *vp, &length) );
	JL_CHK( JL_NativeToJsval(cx, length - position, vp ) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $OBJ $INAME
  The object used to create the steam.
**/
DEFINE_PROPERTY_GETTER( source ) {

	JL_USE(id);

	JL_ASSERT_INSTANCE(obj, JL_THIS_CLASS);
	JL_CHK( JL_GetReservedSlot(cx, obj, SLOT_STREAM_SOURCE, vp) );
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

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_CONSTRUCTOR
	HAS_RESERVED_SLOTS(2)

	BEGIN_FUNCTION_SPEC
		FUNCTION(Read)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY(position)
		PROPERTY_READ(available)
		PROPERTY_READ(source)
	END_PROPERTY_SPEC

END_CLASS

/*
	Adapter pattern:
		http://en.wikipedia.org/wiki/Adapter_pattern

*/
