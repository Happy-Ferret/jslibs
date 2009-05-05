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

//
// RULES:
// - the buffer MUST NOT contains an empty chunk ( else memory leak during bugger life time )
// - reading a greater amount that the buffer length make 'onunderflow' property to be called until the size of the buffer do not grows any more.
// - reading only one chunk of data ( with .Read(undefined); ) on an empty buffer just make one call to 'onunderflow' property.
//

#include "stdafx.h"
#include <cstring>

#include "buffer.h"

#include "../common/queue.h"
#include "../jslang/blobPub.h"

#define SLOT_SOURCE 0


struct BufferPrivate {

	size_t length;
	jl::Queue *queue;
};


static JSBool NativeInterfaceStreamRead( JSContext *cx, JSObject *obj, char *buf, size_t *amount ) {

	return ReadRawAmount(cx, obj, amount, buf);
}


inline JSBool PushJsval( JSContext *cx, jl::Queue *queue, jsval value ) {

	jsval *pItem = (jsval*)malloc(sizeof(jsval));
	J_S_ASSERT_ALLOC( pItem );
	*pItem = value;
	jl::QueuePush( queue, pItem ); // no need to JS_AddRoot *pItem, see Tracer callback
	return JS_TRUE;
	JL_BAD;
}


inline JSBool UnshiftJsval( JSContext *cx, jl::Queue *queue, jsval value ) {

	jsval *pItem = (jsval*)malloc(sizeof(jsval));
	J_S_ASSERT_ALLOC( pItem );
	*pItem = value;
	jl::QueueUnshift( queue, pItem ); // no need to JS_AddRoot *pItem, see Tracer callback
	return JS_TRUE;
	JL_BAD;
}


inline JSBool ShiftJsval( JSContext *cx, jl::Queue *queue, jsval *value ) {

	jsval *pItem = (jsval*)QueueShift(queue);  // no need to JS_RemoveRoot *pItem, see Tracer callback
	if ( value != NULL )
		*value = *pItem;
	free(pItem);
	return JS_TRUE;
}

inline JSBool PeekJsval( JSContext *cx, jl::QueueCell *cell, jsval *value ) {

	*value = *(jsval*)QueueGetData(cell);  // no need to JS_RemoveRoot *pItem, see Tracer callback
	return JS_TRUE;
}


JSBool WriteChunk( JSContext *cx, JSObject *obj, jsval chunk ) {

	BufferPrivate *pv;
	pv = (BufferPrivate*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );

	if ( !JSVAL_IS_STRING(chunk) && !JsvalIsBlob(cx, chunk) ) {

		JSString *jsstr = JS_ValueToString(cx, chunk);
		J_S_ASSERT( jsstr != NULL, "Unable to convert the chunk into a string." );
		chunk = STRING_TO_JSVAL(jsstr);
	}

	// here, chunk is a JSString or a Blob

	size_t strLen;
	J_CHK( JsvalToStringLength(cx, chunk, &strLen) );

//	J_CHK( JsvalToStringLength(cx, chunk, &strLen) );
	if ( strLen == 0 ) // optimization & RULES
		return JS_TRUE;
	J_CHK( PushJsval(cx, pv->queue, chunk) );
	pv->length += strLen;
	return JS_TRUE;
	JL_BAD;
}


JSBool WriteRawChunk( JSContext *cx, JSObject *obj, size_t amount, const char *str ) {

	BufferPrivate *pv = (BufferPrivate*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	jsval bstr;
	J_CHK( J_NewBlobCopyN(cx, str, amount, &bstr) );
	J_CHK( PushJsval(cx, pv->queue, bstr) );
	pv->length += amount;
	return JS_TRUE;
	JL_BAD;
}


JSBool UnReadChunk( JSContext *cx, JSObject *obj, jsval chunk ) {

	BufferPrivate *pv = (BufferPrivate*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );

	if ( !JSVAL_IS_STRING(chunk) && !JsvalIsBlob(cx, chunk) ) {

		JSString *jsstr = JS_ValueToString(cx, chunk);
		J_S_ASSERT( jsstr != NULL, "Unable to convert the chunk into a string." );
		chunk = STRING_TO_JSVAL(jsstr);
	}
	// here, chunk is a JSString or a Blob

	size_t strLen;
	J_CHK( JsvalToStringLength(cx, chunk, &strLen) );

	if ( strLen == 0 ) // optimization && RULES
		return JS_TRUE;
	J_CHK( UnshiftJsval(cx, pv->queue, chunk) );
	pv->length += strLen;
	return JS_TRUE;
	JL_BAD;
}




/*
inline JSBool BufferRefill( JSContext *cx, JSObject *obj, size_t amount ) { // amount = total needed amount

	BufferPrivate *pv = (BufferPrivate*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	jsval rval, fctVal;
	size_t bufferLengthBeforeRefillRequest;
	J_CHK( JS_GetProperty(cx, obj, "onunderflow", &fctVal) );
	if ( !JsvalIsFunction(cx, fctVal) )
		return JS_TRUE; // cannot refil, onunderflow is not defined
	do {

		bufferLengthBeforeRefillRequest = pv->length;
		jsval argv[] = { OBJECT_TO_JSVAL(obj), INT_TO_JSVAL(amount - pv->length) }; // missing amount of data to complete the request at once.
		J_CHK( JS_CallFunctionValue(cx, obj, fctVal, sizeof(argv)/sizeof(*argv), argv, &rval) );
	} while( pv->length < amount && pv->length > bufferLengthBeforeRefillRequest ); // see RULES ( at the top of this file )
	return JS_TRUE;
}
*/

inline JSBool BufferRefill( JSContext *cx, JSObject *obj, size_t amount ) { // amount = total needed amount

	BufferPrivate *pv = (BufferPrivate*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );

	jsval srcVal;
//	J_CHK( JS_GetProperty(cx, obj, "source", &srcVal) );
	J_CHK( JS_GetReservedSlot(cx, obj, SLOT_SOURCE, &srcVal) );

	if ( JSVAL_IS_VOID( srcVal ) ) // no source for refill
		return JS_TRUE;

	J_S_ASSERT_OBJECT(srcVal);

	JSObject *srcObj;
	srcObj = JSVAL_TO_OBJECT( srcVal );

	NIStreamRead nisr;
	nisr = StreamReadInterface(cx, srcObj); // get native or non-native StreamRead function
	J_S_ASSERT( nisr != NULL, "Invalid source object" );

	size_t len, prevBufferLength;

	do {

		prevBufferLength = pv->length;
		len = amount - pv->length;

		char *buf;
		buf = (char*)JS_malloc(cx, len);
		J_CHK( buf );

		J_CHK( nisr(cx, srcObj, buf, &len) );

		if ( len == 0 ) {

			JS_free(cx, buf);
		} else {

			if ( MaybeRealloc(amount, len) )
				buf = (char*)JS_realloc(cx, buf, len);
			J_CHK( WriteRawChunk(cx, obj, len, buf) );
		}

	} while( pv->length < amount && pv->length > prevBufferLength ); // see RULES ( at the top of this file )

	return JS_TRUE;
	JL_BAD;
}



JSBool UnReadRawChunk( JSContext *cx, JSObject *obj, char *data, size_t length ) {

	if ( length == 0 ) // optimization & RULES
		return JS_TRUE;
	jsval bstr;
	J_CHK( J_NewBlobCopyN(cx, data, length, &bstr) );
	J_CHK( UnReadChunk(cx, obj, bstr) );
	return JS_TRUE;
	JL_BAD;
}


JSBool ReadChunk( JSContext *cx, JSObject *obj, jsval *rval ) {

	BufferPrivate *pv = (BufferPrivate*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );

	if ( pv->length == 0 ) { // if buffer is empty, try to refill it.

		J_CHK( BufferRefill(cx, obj, 1) ); // at least 1 more byte
		if ( pv->length == 0 ) { // the data are definitively exhausted

			*rval = JS_GetEmptyStringValue(cx);
			return JS_TRUE;
		}
	}

	J_CHK( ShiftJsval(cx, pv->queue, rval) );
	size_t len;
	J_CHK( JsvalToStringLength(cx, *rval, &len) );
	pv->length -= len;
	return JS_TRUE;
	JL_BAD;
}


JSBool ReadRawAmount( JSContext *cx, JSObject *obj, size_t *amount, char *str ) { // this form allows one to read into a static buffer

	if ( *amount == 0 ) // optimization
		return JS_TRUE;

	J_S_ASSERT( *amount > 0, "Invalid amount requested." );

	BufferPrivate *pv;
	pv = (BufferPrivate*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );

	if ( pv->length < *amount )
		J_CHK( BufferRefill(cx, obj, *amount) );

	if ( *amount == 0 ) // another optimization
		return JS_TRUE;

	char *ptr;
	ptr = str;

	*amount = J_MIN( *amount, pv->length );

	size_t remainToRead;
	remainToRead = *amount;

	while ( remainToRead > 0 ) { // while there is something to read,

		jsval item;
		J_CHK( ShiftJsval(cx, pv->queue, &item) );

		size_t chunkLen;
		const char *chunk;
		J_CHK( JsvalToStringAndLength(cx, &item, &chunk, &chunkLen) ); // (TBD) GC protect item ? (can be a JSString or a NIBufferGet)

		if ( chunkLen <= remainToRead ) {

			memcpy(ptr, chunk, chunkLen);
			ptr += chunkLen;
			remainToRead -= chunkLen; // adjust remaining required data length
		} else { // chunkLen > remain: this is the last chunk we have to manage. we only get a part of it chunk and we 'unread' the remaining.

			memcpy(ptr, chunk, remainToRead);
			jsval bstr;
			J_CHK( J_NewBlobCopyN(cx, chunk + remainToRead, chunkLen - remainToRead, &bstr) );
			UnshiftJsval(cx, pv->queue, bstr);
			remainToRead = 0; // adjust remaining required data length
		}
	}
	pv->length -= *amount;
	return JS_TRUE;
	JL_BAD;
}


JSBool BufferSkipAmount( JSContext *cx, JSObject *obj, size_t amount ) {

	if ( amount == 0 ) // optimization
		return JS_TRUE;

	BufferPrivate *pv = (BufferPrivate*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );

	if ( pv->length < amount )
		J_CHK( BufferRefill(cx, obj, amount) );

	if ( pv->length < amount ) {

		amount = pv->length - amount;
		while ( !QueueIsEmpty(pv->queue) )
			J_CHK( ShiftJsval(cx, pv->queue, NULL) );
		pv->length = 0;
		return JS_TRUE;
	}

	size_t remainToRead;
	remainToRead = amount;
	while ( remainToRead > 0 ) { // while there is something to read,

		jsval item;
		J_CHK( ShiftJsval(cx, pv->queue, &item) );

		size_t chunkLen;
		J_CHK( JsvalToStringLength(cx, item, &chunkLen) );

		if ( chunkLen <= remainToRead ) {

			remainToRead -= chunkLen;
		} else {

			size_t chunkLen;
			const char *chunk;
			J_CHK( JsvalToStringAndLength(cx, &item, &chunk, &chunkLen) );

			jsval bstr;
			J_CHK( J_NewBlobCopyN(cx, chunk + remainToRead, chunkLen - remainToRead, &bstr) );
			UnshiftJsval(cx, pv->queue, bstr);
			remainToRead = 0; // adjust remaining required data length
		}
	}
	pv->length -= amount;
	return JS_TRUE;
	JL_BAD;
}


JSBool ReadAmount( JSContext *cx, JSObject *obj, size_t amount, jsval *rval ) {

	if ( amount == 0 ) { // optimization

		*rval = JS_GetEmptyStringValue(cx);
		return JS_TRUE;
	}

	char *str;
	str = (char*)JS_malloc(cx, amount +1); // (TBD) memory leak if ReadRawAmount failed
	J_CHK( str );

	// (TBD) IMPORTANT: here, amount should be MIN( amount, buffer_size ). This can avoid an useless memory allocation.
	int requestedAmount;
	requestedAmount = amount;
	J_CHK( ReadRawAmount(cx, obj, &amount, str) );

	if ( amount == 0 ) { // optimization

		JS_free(cx, str);
		*rval = JS_GetEmptyStringValue(cx);
		return JS_TRUE;
	}

	str[amount] = '\0'; // (TBD) explain this

	if ( MaybeRealloc( requestedAmount, amount ) ) {

		str = (char*)JS_realloc(cx, str, amount +1);
		J_CHK( str );
	}

	J_CHK( J_NewBlob(cx, str, amount, rval) );
	return JS_TRUE;
	JL_BAD;
}


JSBool FindInBuffer( JSContext *cx, JSObject *obj, const char *needle, size_t needleLength, int *foundAt ) {

	// (TBD) optimise this function for needleLength == 1 (eg. '\0' in a string)
	BufferPrivate *pv = (BufferPrivate*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );

	size_t pos;
	pos = 0;

	char staticBuffer[128];
	char *buf;
	buf = needleLength <= sizeof(staticBuffer) ? staticBuffer : (char*)malloc(needleLength); // the "ring buffer"

	size_t chunkLength;
	const char *chunk;
	size_t i, j;

	for ( jl::QueueCell *it = jl::QueueBegin(pv->queue); it; it = jl::QueueNext(it) ) {

		jsval *pNewStr = (jsval*)QueueGetData(it);
		J_CHK( JsvalToStringAndLength(cx, pNewStr, &chunk, &chunkLength) );
		for ( i = 0; i < chunkLength; i++ ) {

			buf[pos++ % needleLength] = chunk[i]; // store one more char of the chunk in the ring buffer
			if ( pos >= needleLength ) { // if we have enough data in the ring buffer to start the search

				for ( j = 0; j < needleLength && needle[j] == buf[(pos+j) % needleLength]; j++ ); // search the 'needle' starting at the right position in the ring buffer.
				if( j == needleLength ) { // if all chars of the 'needle' are found

					*foundAt = pos-needleLength;
					goto end; // this is a cheap way to break all these nested loops
				}
			}
		}
	}

	*foundAt = -1;
end:
	if ( buf != staticBuffer )
		free(buf); // free the "ring buffer"
	return JS_TRUE;
	JL_BAD;
}


JSBool AddBuffer( JSContext *cx, JSObject *destBuffer, JSObject *srcBuffer ) {

	J_S_ASSERT_CLASS( destBuffer, classBuffer );
	BufferPrivate *dpv;
	dpv = (BufferPrivate*)JS_GetPrivate(cx, destBuffer);
	J_S_ASSERT_RESOURCE( dpv );

	J_S_ASSERT_CLASS( srcBuffer, classBuffer );
	BufferPrivate *spv;
	spv = (BufferPrivate*)JS_GetPrivate(cx, srcBuffer);
	J_S_ASSERT_RESOURCE( spv );

	for ( jl::QueueCell *it = jl::QueueBegin(spv->queue); it; it = jl::QueueNext(it) )
		J_CHK( PushJsval(cx, dpv->queue, *(jsval*)QueueGetData(it)) );
	dpv->length += spv->length;

	return JS_TRUE;
	JL_BAD;
}


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
 Buffer class is a simple buffer that allows arbitrary length input and output.
**/
BEGIN_CLASS( Buffer )

DEFINE_FINALIZE() {

	BufferPrivate *pv = (BufferPrivate*)JS_GetPrivate(cx, obj);
	if ( pv != NULL ) {

		while ( !QueueIsEmpty(pv->queue) )
			ShiftJsval(cx, pv->queue, NULL);
		QueueDestruct(pv->queue);
		JS_free(cx, pv);
		JS_SetPrivate(cx, obj, NULL); // optional ?
	}
}

/* prev doc:
  If a string is given as argument, the buffer is initialized with this string.
  If buffer object is given, the buffer is initialized with this existing buffer object (kind of copy constructor).
*/

/**doc
$TOC_MEMBER $INAME
 $INAME( [source] )
  Constructs a Buffer object.
  $H arguments
   $ARG streamObject source: any object that supports NIStreamRead interface. The Buffer uses this object when its length is less than the requested amount of data.
   $H beware
    When used by the Buffer, the source MUST return the exact or less data than the required size else the remaining is lost (not stored in the buffer).
   $H note
    You can use the Write() function in the source like:
    {{{
    var buf = new Buffer({ Read:function(count) { buf.Write('some data') } });
    }}}

  $H example 1
  {{{
  var buf1 = new Buffer(Stream('456'));
  buf1.Write('123');
  Print( buf1.Read(6) ); // prints: '123456'
  }}}

  $H example 2
  {{{
  var buf1 = new Buffer(Stream('123'));
  Print( buf1.Read(1) ,'\n'); // prints: '1'
  Print( buf1.Read(1) ,'\n'); // prints: '2'
  Print( buf1.Read(1) ,'\n'); // prints: '3'
  }}}

  $H example 3
  {{{
  var buf2 = new Buffer(new function() {
   this.Read = function(count) StringRepeat('x',count);
  });
  Print( buf2.Read(6) ); // prints: 'xxxxxx'
  }}}

  $H example 4
   Create a long chain ( pack << buffer << buffer << stream )
  {{{
  var p = new Pack(new Buffer(new Buffer(Stream('\x12\x34'))));
  Print( (p.ReadInt(2, false, true)).toString(16) ); // prints: '1234'
  }}}
**/
DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();
	J_CHK( SetStreamReadInterface(cx, obj, NativeInterfaceStreamRead) );

	BufferPrivate *pv;
	pv = (BufferPrivate *)JS_malloc(cx, sizeof(BufferPrivate));
	J_CHK( pv );
	J_CHK( JS_SetPrivate(cx, obj, pv) );
	pv->queue = jl::QueueConstruct();
	J_S_ASSERT_ALLOC(pv->queue);
	pv->length = 0;

	if ( J_ARG_ISDEF(1) ) {

/*
		// (TBD) loop over all args
		if ( JsvalIsClass(J_ARG(1), _class) ) {

			return AddBuffer(cx, obj, JSVAL_TO_OBJECT( J_ARG(1) ));
		} else {

			size_t length;
			J_CHK( JsvalToStringLength(cx, J_ARG(1), &length) );
			if ( length == 0 )
				return JS_TRUE;
			pv->length += length;
			return PushJsval(cx, pv->queue, J_ARG(1));
		}
*/

		J_S_ASSERT_OBJECT( J_ARG(1) );
		J_CHK( JS_SetReservedSlot(cx, obj, SLOT_SOURCE, J_ARG(1)) );

	}
	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Methods ===
**/

/*
DEFINE_FUNCTION( Clone ) {

		// (TBD) loop over all args
		if ( JsvalIsClass(J_ARG(1), _class) ) {

			return AddBuffer(cx, obj, JSVAL_TO_OBJECT( J_ARG(1) ));
		} else {

			size_t length;
			J_CHK( JsvalToStringLength(cx, J_ARG(1), &length) );
			if ( length == 0 )
				return JS_TRUE;
			pv->length += length;
			return PushJsval(cx, pv->queue, J_ARG(1));
		}

	return JS_TRUE;
	JL_BAD;
}
*/


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Empty the whole buffer.
**/
DEFINE_FUNCTION_FAST( Clear ) {

	J_S_ASSERT_CLASS(J_FOBJ, _class);
	BufferPrivate *pv;
	pv = (BufferPrivate*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE( pv );
	while ( !QueueIsEmpty(pv->queue) )
		J_CHK( ShiftJsval(cx, pv->queue, NULL) );
	pv->length = 0;
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INAME( data [, length] )
 * $VOID $INAME( buffer )
  Add _data_ in the buffer. If _length_ is used, only the first _length_ bytes of _data_ are added.
  The second form allow to add another whole buffer in the current buffer.
**/
DEFINE_FUNCTION_FAST( Write ) {

	J_S_ASSERT_CLASS(J_FOBJ, _class);
	BufferPrivate *pv;
	pv = (BufferPrivate*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE( pv );
	J_S_ASSERT_ARG_MIN( 1 );

	if ( JsvalIsClass(J_FARG(1), _class ) )
		return AddBuffer(cx, J_FOBJ, JSVAL_TO_OBJECT( J_FARG(1) ));

	*J_FRVAL = JSVAL_VOID;
	if ( J_FARG_ISDEF(2) ) {

		size_t amount, strLen;
		const char *buf;
		J_CHK( JsvalToStringAndLength(cx, &J_FARG(1), &buf, &strLen) ); // warning: GC on the returned buffer !

		if ( strLen == 0 )
			return JS_TRUE;

		J_CHK( JsvalToUInt(cx, J_FARG(2), &amount) );
		if ( amount > strLen )
			amount = strLen;

		return WriteRawChunk(cx, J_FOBJ, amount, buf);
	} else {

		return WriteChunk(cx, J_FOBJ, J_FARG(1));
	}
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME( str [, consume = false ] )
  Check if the given string _str_ matchs to the next data in the buffer.
  $H arguments
   $ARG $STR str
   $ARG $BOOL consume: if false, just check if it match without consuming data, else, read and check.
  $H return value
   true if it matchs, else false.
**/
DEFINE_FUNCTION_FAST( Match ) {

	J_S_ASSERT_CLASS(J_FOBJ, _class);
	J_S_ASSERT_ARG_MIN(1);

	const char *str;
	size_t len;
	J_CHK( JsvalToStringAndLength(cx, &J_FARG(1), &str, &len) ); // warning: GC on the returned buffer !

	char *src;
	src = (char *)malloc(len);
	size_t amount;
	amount = len;
	JSBool st;
	st = ReadRawAmount(cx, J_FOBJ, &amount, src);
	if ( st != JS_TRUE )
		goto err;

	if ( amount != len )
		*J_FRVAL = JSVAL_FALSE;
	else
		*J_FRVAL = strncmp( str, src, len ) == 0 ? JSVAL_TRUE : JSVAL_FALSE;

	bool consume;
	if ( J_FARG_ISDEF(2) )
		J_CHK( JsvalToBool(cx, J_FARG(2), &consume) );
	else
		consume = false;

	if ( !consume )
		J_CHK( UnReadRawChunk(cx, J_FOBJ, src, amount) );

err:
	free(src);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( [ amount ] )
  Read _amount_ data in the buffer. If _amount_ is omited, The whole buffer is returned.
  $H beware
   This function returns a Blob or a string literal as empty string.
  $LF
  If _amount_ == undefined, an arbitrary (ideal) amount of data is returned. Use this when you don't know how many data you have to read.
  $H example
  {{{
  var chunk = buffer.Read(undefined);
  }}}
  $H note
  The read operation never blocks, even if the requested amount of data is greater than the buffer length.
**/
DEFINE_FUNCTION( Read ) { // Read( [ amount | <undefined> ] )

	J_S_ASSERT_THIS_CLASS();
	BufferPrivate *pv;
	pv = (BufferPrivate*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );

	if ( J_ARGC == 1 && JSVAL_IS_VOID( J_ARG(1) ) ) // read the next chunk (of an unknown length) (read something as fast as possible)
		return ReadChunk(cx, obj, rval);

	size_t amount;
	if ( J_ARG_ISDEF(1) )
		J_CHK( JsvalToUInt(cx, J_ARG(1), &amount) );
	else
		amount = pv->length;

	J_S_ASSERT( amount >= 0, "Invalid amount" );
	J_CHK( ReadAmount(cx, obj, amount, rval) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INAME( length )
  Skip _length_ bytes of data from the buffer.
**/
DEFINE_FUNCTION( Skip ) { // Skip( amount )

	J_S_ASSERT_THIS_CLASS();
	BufferPrivate *pv;
	pv = (BufferPrivate*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	J_S_ASSERT_ARG_MIN( 1 );
	size_t amount;
	J_CHK( JsvalToUInt(cx, J_ARG(1), &amount) );
	J_S_ASSERT( amount >= 0, "Invalid amount" );
	size_t prevBufferLength;
	prevBufferLength = pv->length;
	J_CHK( BufferSkipAmount(cx, obj, amount) ); // (TBD) optimization: skip without reading the data.
	*rval = BOOLEAN_TO_JSVAL( pv->length == prevBufferLength - amount ); // function returns true on sucessful skip operation
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( boundaryString [, skip] )
  Reads the buffer until it match the _boundaryString_, else it returns $UNDEF.
  If _skip_ argument is $TRUE, the _boundaryString_ is skiped from the buffer.
**/
DEFINE_FUNCTION( ReadUntil ) {

	J_S_ASSERT_THIS_CLASS();
	J_S_ASSERT_ARG_MIN( 1 );
	const char *boundary;
	size_t boundaryLength;

	J_CHK( JsvalToStringAndLength(cx, &J_ARG(1), &boundary, &boundaryLength) ); // warning: GC on the returned buffer !

	bool skip;
	if ( J_ARG_ISDEF(2) )
		J_CHK( JsvalToBool(cx, J_ARG(2), &skip) );
	else
		skip = true;
	int found;
	J_CHK( FindInBuffer(cx, obj, boundary, boundaryLength, &found) );
	if ( found != -1 ) {

		J_CHK( ReadAmount(cx, obj, found, rval) );
		if ( skip ) {

			jsval tmp;
			J_CHK( ReadAmount(cx, obj, boundaryLength, &tmp) ); // (TBD) optimization: skip without reading the data.
		}
	}
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME( string )
  Find _string_ in the buffer and returns the offset of the first letter. If not found, this function returns -1.
**/
DEFINE_FUNCTION_FAST( IndexOf ) {

	J_S_ASSERT_CLASS(J_FOBJ, _class);
	J_S_ASSERT_ARG_MIN( 1 );
	const char *boundary;
	size_t boundaryLength;
	J_CHK( JsvalToStringAndLength(cx, &J_FARG(1), &boundary, &boundaryLength) ); // warning: GC on the returned buffer !
	int found;
	J_CHK( FindInBuffer(cx, J_FOBJ, boundary, boundaryLength, &found) );
	*J_FRVAL = INT_TO_JSVAL(found);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( _data_ )
  Insert _data_ at the begining of the buffer. This function can undo a read operation. The returned value is _data_.
  $H example
  {{{
  function Peek(len) {

    return buffer.Unread( buffer.Read(len) );
  }
  }}}
**/
DEFINE_FUNCTION_FAST( Unread ) {

	J_S_ASSERT_CLASS(J_FOBJ, _class);
	J_S_ASSERT_ARG_MIN( 1 );
	J_S_ASSERT_ARG_MAX( 1 ); // discourages one to use Unread like Write
	J_CHK( UnReadChunk(cx, J_FOBJ, J_FARG(1)) );
	*J_FRVAL = J_FARG(1);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME()
  Converts the whole content of the buffer to a string.
  $H note
   The buffer is not modified.
**/

// Note:  String( { toString:function() { return [1,2,3]} } );  throws the following error: "can't convert Object to string"
DEFINE_FUNCTION( toString ) {

	J_S_ASSERT_THIS_CLASS();
	BufferPrivate *pv;
	pv = (BufferPrivate*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );

	if ( pv->length == 0 ) {

		*rval = JS_GetEmptyStringValue(cx);
		return JS_TRUE;
	}

	char *buffer;
	buffer = (char*)JS_malloc(cx, pv->length +1);
	J_CHK( buffer );
	buffer[pv->length] = '\0';

	size_t pos;
	pos = 0;
//	while ( !QueueIsEmpty(pv->queue) ) {
	for ( jl::QueueCell *it = jl::QueueBegin(pv->queue); it; it = jl::QueueNext(it) ) {

//		J_CHK( ShiftJsval(cx, pv->queue, rval) );
		J_CHK( PeekJsval(cx, it, rval) );

		const char *chunkBuf;
		size_t chunkLen;
		J_CHK( JsvalToStringAndLength(cx, rval, &chunkBuf, &chunkLen) );
		memcpy(buffer + pos, chunkBuf, chunkLen);
		pos += chunkLen;
	}

	JSString *str;
	str = JS_NewString(cx, buffer, pv->length);
	J_CHK( str );
	*rval = STRING_TO_JSVAL(str);
//	pv->length = 0;


	// we have to return a real string !?
	//char *bstrBuf = (char*)JS_malloc(cx, pv->length);
	//J_CHK( bstrBuf );
	//size_t amount = pv->length;
	//J_CHK( ReadRawAmount(cx, obj, &amount, bstrBuf) );
	//JSObject *bstrObj = NewBlob(cx, bstrBuf, pv->length);
	//J_S_ASSERT( bstrObj != NULL, "Unable to create the Blob." );
	//*J_RVAL = OBJECT_TO_JSVAL(bstrObj);

	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $TYPE char $INAME $READONLY
  Used to access the character in the _N_th position where _N_ is a positive integer between 0 and one less than the value of length.
**/
DEFINE_GET_PROPERTY() {

	J_S_ASSERT_THIS_CLASS();
	if ( !JSVAL_IS_INT(id) )
		return JS_TRUE;

	long slot;
	slot = JSVAL_TO_INT( id );

	BufferPrivate *pv;
	pv = (BufferPrivate*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );

	if ( slot >= 0 && (unsigned)slot < pv->length ) {

		size_t offset = 0;

		size_t chunkLength;
		const char *chunk;

		for ( jl::QueueCell *it = jl::QueueBegin(pv->queue); it; it = jl::QueueNext(it) ) {

			jsval *pNewStr = (jsval*)QueueGetData(it);
			J_CHK( JsvalToStringLength(cx, *pNewStr, &chunkLength) );

			if ( (unsigned)slot >= offset && (unsigned)slot < offset + chunkLength ) {

				J_CHK( JsvalToString(cx, pNewStr, &chunk) ); // items in the queue are GC protected.

				jschar chr = ((char*)chunk)[slot - offset];
				JSString *str1 = JS_NewUCStringCopyN(cx, &chr, 1);
				J_CHK( str1 );
				*vp = STRING_TO_JSVAL(str1);
				return JS_TRUE;
			}
			offset += chunkLength;
		}
	}

	*vp = JS_GetEmptyStringValue(cx);
	return JS_TRUE;
	JL_BAD;
}


DEFINE_SET_PROPERTY() {

	J_S_ASSERT( !JSVAL_IS_NUMBER(id), "Operation not allowed." );
	return JS_TRUE;
	JL_BAD;
}




/**doc
=== Properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  Is the current length of the buffer.
**/
DEFINE_PROPERTY( length ) {

	J_S_ASSERT_THIS_CLASS();
	BufferPrivate *pv;
	pv = (BufferPrivate*)JS_GetPrivate(cx, obj);
	*vp = INT_TO_JSVAL(pv->length);
	return JS_TRUE;
	JL_BAD;
}


//DEFINE_PROPERTY( source ) { // do not support: delete buf.source
//
//	J_CHK( JS_SetReservedSlot(cx, obj, SLOT_SOURCE, *vp) );
//	return JS_TRUE;
//}


DEFINE_TRACER() {

	BufferPrivate *pv = (BufferPrivate*)JS_GetPrivate(trc->context, obj);
	if ( pv )
		for ( jl::QueueCell *it = jl::QueueBegin(pv->queue); it; it = jl::QueueNext(it) )
			JS_CALL_VALUE_TRACER(trc, *(jsval*)QueueGetData(it), "jsstd/Buffer:bufferQueueItem");
}


/* previous doc:
 * *onunderflow*( bufferObject, missingAmount )
  This function is called by the Buffer when its length is less than the requested amount of data.
  $H arguments
   $ARG Buffer bufferObject: is the buffer object that caused the call.
   $ARG $INT missingAmount: is the  missing amount of data to complete the request at once.
  This function is called until the buffer size do not grow any more.
*/

/** prev doc
$TOC_MEMBER $INAME
 $OBJ *source*
  The source property can contains any NIStreamRead compatible object. The Buffer uses this object when its length is less than the requested amount of data. Any extra data returned by the NIStreamRead object is keept in the buffer and will be used at the next read operation.
  $H beware
   The .source function called by the Buffer MUST return the exact or less than the  required size else the remaining is lost (not stored in the buffer).
  $H note
   You can use the Write() function in .source like:
	{{{
   buf.source = { Read:function(count) { times++; buf.Write(toto) } };
   }}}

	$H example 1
   {{{
	var buf1 = new Buffer('123');
	buf1.source = Stream('456');
	QA.ASSERT( buf1.length, 3, 'length' );
	Print( buf2.Read(6) ); // prints: '123456'
	}}}

	$H example 2
	{{{
	var buf2 = new Buffer('123');
	buf2.source = new function() {
		this.Read = function(count) StringRepeat('x',count);
	}
	Print( buf2.Read(6) ); // prints: '123xxx'
	}}}

	$H example 3
	{{{
	var first = new Buffer();
	first.source = new Buffer();
	first.source.source = new Buffer('123');
	Print( first.Read(4) ); // prints: '123'
	}}}

	$H example 4
	 Create a long chain ( pack << buffer << buffer << stream )
	{{{
	var p = new Pack(new Buffer());
	p.buffer.source = new Buffer();
	p.buffer.source.source = Stream('\x12\x34');
	Print( (p.ReadInt(2, false, true)).toString(16) ); // prints: '1234'
	}}}
**/

/**doc
=== Native Interface ===
 * *NIStreamRead*
**/

CONFIGURE_CLASS

	REVISION(SvnRevToInt("$Revision$"))

	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1)

	HAS_CONSTRUCTOR
	HAS_FINALIZE
	HAS_TRACER

	HAS_GET_PROPERTY
	HAS_SET_PROPERTY

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST(Clear)
		FUNCTION_FAST(Write)
		FUNCTION_FAST(Match)
		FUNCTION(Read)
		FUNCTION_FAST(Unread)
		FUNCTION(ReadUntil)
		FUNCTION_FAST(IndexOf)
		FUNCTION(Skip)
		FUNCTION(toString) // used when the buffer has to be transformed into a javascript string
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ(length)
//		PROPERTY_WRITE_STORE(source)
	END_PROPERTY_SPEC

END_CLASS


/**doc
=== example 1 ===
 {{{
 var buf = new Buffer();
 buf.Write('1234');
 buf.Write('5');
 buf.Write('');
 buf.Write('6789');
 Print( buf.Read() );
 }}}

=== example 2 ===
 {{{
 var buf = new Buffer();
 buf.Write('0123456789');
 Print( buf.Read(4) );
 Print( buf.Read(1) );
 Print( buf.Read(1) );
 Print( buf.Read(4) );
 }}}


=== example 3 ===
 Buffered read from a stream.
 {{{
 function ReadFromFile() {

  Print('*** read from the file\n');
  return StringRepeat('x',5);
 }

 var buf = new Buffer({ Read:function() { buf.Write(ReadFromFile()); }})

 for ( var i=0; i<15; i++ )
  Print( buf.Read(1), '\n' )
 }}}
**/
