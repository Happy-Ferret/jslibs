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

#include "blobPub.h"

#define SLOT_STREAM_SOURCE 0
#define SLOT_STREAM_POSITION 1

DECLARE_CLASS( Stream );

inline JSBool PositionSet( JSContext *cx, JSObject *obj, int position ) {

	jsval tmp;
	JL_CHK( IntToJsval(cx, position, &tmp) );
	return JS_SetReservedSlot(cx, obj, SLOT_STREAM_POSITION, tmp);
	JL_BAD;
}


inline JSBool PositionGet( JSContext *cx, JSObject *obj, int *position ) {

	jsval tmp;
	JL_CHK( JS_GetReservedSlot(cx, obj, SLOT_STREAM_POSITION, &tmp) );
	JL_CHK( JsvalToInt(cx, tmp, position) );
	return JS_TRUE;
	JL_BAD;
}


JSBool StreamRead( JSContext *cx, JSObject *obj, char *buf, size_t *amount ) {

	JL_S_ASSERT_CLASS(obj, classStream);

	int position;
	JL_CHK( PositionGet(cx, obj, &position) );
	jsval source;
	JL_CHK( JS_GetReservedSlot(cx, obj, SLOT_STREAM_SOURCE, &source) );

	const char *buffer;
	size_t length;
	JL_CHK( JsvalToStringAndLength(cx, &source, &buffer, &length) ); // (TBD) GC protect source

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

	JL_S_ASSERT_ARG_MIN( 1 );

	if ( JS_IsConstructing(cx) == JS_FALSE ) { // supports this form (w/o new operator) : result.param1 = Blob('Hello World');

		obj = JS_NewObject(cx, _class, NULL, NULL);
		JL_CHK( obj );
		*rval = OBJECT_TO_JSVAL(obj);
	} else {

		JL_S_ASSERT_THIS_CLASS();
	}

	JL_S_ASSERT( !JSVAL_IS_VOID(JL_ARG(1)) && !JSVAL_IS_NULL(JL_ARG(1)), "Invalid stream source." );

	JL_CHK( JS_SetReservedSlot(cx, obj, SLOT_STREAM_SOURCE, JL_ARG(1)) );
	JL_CHK( PositionSet(cx, obj, 0) );

	JL_CHK( ReserveStreamReadInterface(cx, obj) );
	JL_CHK( SetStreamReadInterface(cx, obj, StreamRead) );

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE Blob $INAME( amount )
  Read _amount_ of data into the stream.
**/
DEFINE_FUNCTION_FAST( Read ) {

	JSObject *obj = JL_FOBJ;
	JL_S_ASSERT_CLASS(obj, _class);
	JL_S_ASSERT_ARG_MIN( 1 );

	int amount;
	JL_CHK( JsvalToInt(cx, JL_FARG(1), &amount) );

	char *buffer;
	buffer = (char*)JS_malloc(cx, amount +1);
	JL_CHK(buffer);

	size_t readAmount;
	readAmount = amount;
	JL_CHK( StreamRead(cx, obj, buffer, &readAmount ) );

	if ( JL_MaybeRealloc(amount, readAmount) )
		buffer = (char*)JS_realloc(cx, buffer, readAmount +1);

	buffer[readAmount] = '\0';

	JL_CHK( JL_NewBlob(cx, buffer, readAmount, JL_FRVAL) );

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME
  Get or set the stream pointer position.
**/
DEFINE_PROPERTY( positionGetter ) {

	JL_S_ASSERT_CLASS(obj, _class);
	int position;
	JL_CHK( PositionGet(cx, obj, &position) );
	*vp = INT_TO_JSVAL( position );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( positionSetter ) {

	JL_S_ASSERT_CLASS(obj, _class);
	int position;
	JL_CHK( JsvalToInt(cx, *vp, &position) );
	JL_S_ASSERT( position >= 0, "Invalid stream position." );
	JL_CHK( PositionSet(cx, obj, position) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  The remaining data from the stream pointer position to the end of the stream.
**/
DEFINE_PROPERTY( available ) {

	JL_S_ASSERT_CLASS(obj, _class);
	JL_CHK( JS_GetReservedSlot(cx, obj, SLOT_STREAM_SOURCE, vp) ); // use vp as a tmp variable
	JSObject *srcObj;
	if ( JSVAL_IS_OBJECT( *vp ) )
		srcObj = JSVAL_TO_OBJECT( *vp );
	else
		JL_CHK( JS_ValueToObject(cx, *vp, &srcObj) );
	int length, position;
	JL_CHK( PositionGet(cx, obj, &position) );
	JL_CHK( JS_GetProperty(cx, srcObj, "length", vp) ); // use vp as a tmp variable
	if ( JSVAL_IS_VOID( *vp ) )
		return JS_TRUE; // if length is not defined, the returned value is undefined
	JL_CHK( JsvalToInt(cx, *vp, &length) );
	JL_CHK( IntToJsval(cx, length - position, vp ) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $OBJ $INAME
  The object used to create the steam.
**/
DEFINE_PROPERTY( source ) {

	JL_S_ASSERT_CLASS(obj, _class);
	JL_CHK( JS_GetReservedSlot(cx, obj, SLOT_STREAM_SOURCE, vp) );
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
		FUNCTION_FAST(Read)
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
