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
#include "../jslang/bstringapi.h"

//#define SLOT_SOURCE 0


struct BufferPrivate {

	size_t length;
	Queue *queue;
};


static JSBool NativeInterfaceStreamRead( JSContext *cx, JSObject *obj, char *buf, size_t *amount ) {

	return ReadRawAmount(cx, obj, amount, buf);
}


inline JSBool PushJsval( JSContext *cx, Queue *queue, jsval value ) {

	jsval *pItem = (jsval*)malloc(sizeof(jsval));
	J_S_ASSERT_ALLOC( pItem );
	*pItem = value;
	QueuePush( queue, pItem ); // no need to JS_AddRoot *pItem, see Tracer callback
	return JS_TRUE;
}


inline JSBool UnshiftJsval( JSContext *cx, Queue *queue, jsval value ) {

	jsval *pItem = (jsval*)malloc(sizeof(jsval));
	J_S_ASSERT_ALLOC( pItem );
	*pItem = value;
	QueueUnshift( queue, pItem ); // no need to JS_AddRoot *pItem, see Tracer callback
	return JS_TRUE;
}


inline JSBool ShiftJsval( JSContext *cx, Queue *queue, jsval *value ) {

	jsval *pItem = (jsval*)QueueShift(queue);  // no need to JS_RemoveRoot *pItem, see Tracer callback
	if ( value != NULL )
		*value = *pItem;
	free(pItem);
	return JS_TRUE;
}



JSBool WriteChunk( JSContext *cx, JSObject *obj, jsval chunk ) {

	BufferPrivate *pv = (BufferPrivate*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );

	if ( !JSVAL_IS_STRING(chunk) && !JsvalIsBString(cx, chunk) ) {

		JSString *jsstr = JS_ValueToString(cx, chunk);
		J_S_ASSERT( jsstr != NULL, "Unable to convert the chunk into a string." );
		chunk = STRING_TO_JSVAL(jsstr);
	}

	// here, chunk is a JSString or a BString

	size_t strLen;
	if ( JSVAL_IS_STRING( chunk ) )
		strLen = J_STRING_LENGTH( JSVAL_TO_STRING( chunk ) );
	else
		BStringLength(cx, JSVAL_TO_OBJECT( chunk ), &strLen);
//	J_CHK( JsvalToStringLength(cx, chunk, &strLen) );
	if ( strLen == 0 ) // optimization & RULES
		return JS_TRUE;
	J_CHK( PushJsval(cx, pv->queue, chunk) );
	pv->length += strLen;
	return JS_TRUE;
}


JSBool WriteRawChunk( JSContext *cx, JSObject *obj, size_t amount, const char *str ) {

	BufferPrivate *pv = (BufferPrivate*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	JSObject* bstrObj = J_NewBinaryStringCopyN(cx, str, amount);
	J_S_ASSERT( bstrObj != NULL, "Unable to create the binary string." );
	J_CHK( PushJsval(cx, pv->queue, OBJECT_TO_JSVAL(bstrObj)) );
	pv->length += amount;
	return JS_TRUE;
}


JSBool UnReadChunk( JSContext *cx, JSObject *obj, jsval chunk ) {

	BufferPrivate *pv = (BufferPrivate*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );

	if ( !JSVAL_IS_STRING(chunk) && !JsvalIsBString(cx, chunk) ) {

		JSString *jsstr = JS_ValueToString(cx, chunk);
		J_S_ASSERT( jsstr != NULL, "Unable to convert the chunk into a string." );
		chunk = STRING_TO_JSVAL(jsstr);
	}
	// here, chunk is a JSString or a BString

	size_t strLen;
	if ( JSVAL_IS_STRING( chunk ) )
		strLen = J_STRING_LENGTH( JSVAL_TO_STRING( chunk ) );
	else
		BStringLength(cx, JSVAL_TO_OBJECT( chunk ), &strLen);
//	J_CHK( JsvalToStringLength(cx, chunk, &length) );
	if ( strLen == 0 ) // optimization && RULES
		return JS_TRUE;
	J_CHK( UnshiftJsval(cx, pv->queue, chunk) );
	pv->length += strLen;
	return JS_TRUE;
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

	jsval srcVal;
	J_CHK( JS_GetProperty(cx, obj, "source", &srcVal) );
//	J_CHK( JS_GetReservedSlot(cx, obj, SLOT_SOURCE, &srcVal) );

	if ( srcVal == JSVAL_VOID ) // undefined or <undefined> or deleted, ...
		return JS_TRUE;

	J_S_ASSERT_OBJECT(srcVal);

	JSObject *srcObj = JSVAL_TO_OBJECT( srcVal );

	NIStreamRead nisr = StreamReadInterface(cx, srcObj);
	J_S_ASSERT( nisr != NULL, "Invalid source object" );

	char *buf = (char*)JS_malloc(cx, amount);
	J_S_ASSERT_ALLOC( buf );

	size_t len = amount;
	
	J_CHK( nisr(cx, srcObj, buf, &len) ); // (TBD) loop like with onunderflow

	if ( MaybeRealloc(amount, len) )
		buf = (char*)JS_realloc(cx, buf, len);

	JSObject *chunk = J_NewBinaryString(cx, buf, len);

	J_S_ASSERT( chunk != NULL, "Unable to create a binary string" );

	J_CHK( WriteChunk(cx, obj, OBJECT_TO_JSVAL( chunk )) );

	return JS_TRUE;
}



JSBool UnReadRawChunk( JSContext *cx, JSObject *obj, char *data, size_t length ) {

	if ( length == 0 ) // optimization & RULES
		return JS_TRUE;
	JSObject* bstrObj = J_NewBinaryStringCopyN(cx, data, length);
	J_S_ASSERT( bstrObj != NULL, "Unable to create the binary string." );
	J_CHK( UnReadChunk(cx, obj, OBJECT_TO_JSVAL(bstrObj)) );
	return JS_TRUE;
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
}


JSBool ReadRawAmount( JSContext *cx, JSObject *obj, size_t *amount, char *str ) { // this form allows one to read into a static buffer

	if ( *amount == 0 ) // optimization
		return JS_TRUE;

	J_S_ASSERT( *amount > 0, "Invalid amount requested." );

	BufferPrivate *pv = (BufferPrivate*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );

	if ( pv->length < *amount )
		J_CHK( BufferRefill(cx, obj, *amount) );

	if ( *amount == 0 ) // another optimization
		return JS_TRUE;

	char *ptr = str;

	*amount = J_MIN( *amount, pv->length );

	size_t remainToRead = *amount;

	while ( remainToRead > 0 ) { // while there is something to read,

		jsval item;
		J_CHK( ShiftJsval(cx, pv->queue, &item) );

		size_t chunkLen;
		const char *chunk;
		J_CHK( JsvalToStringAndLength(cx, item, &chunk, &chunkLen) );

		if ( chunkLen <= remainToRead ) {

			memcpy(ptr, chunk, chunkLen);
			ptr += chunkLen;
			remainToRead -= chunkLen; // adjust remaining required data length
		} else { // chunkLen > remain: this is the last chunk we have to manage. we only get a part of it chunk and we 'unread' the remaining.

			memcpy(ptr, chunk, remainToRead);
			JSObject *bstrObj = J_NewBinaryStringCopyN(cx, chunk + remainToRead, chunkLen - remainToRead);
			J_S_ASSERT( bstrObj != NULL, "Unable to create the binary string." );
			UnshiftJsval(cx, pv->queue, OBJECT_TO_JSVAL( bstrObj ));
			remainToRead = 0; // adjust remaining required data length
		}
	}
	pv->length -= *amount;
	return JS_TRUE;
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

	size_t remainToRead = amount;
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
			J_CHK( JsvalToStringAndLength(cx, item, &chunk, &chunkLen) );

			JSObject *bstrObj = J_NewBinaryStringCopyN(cx, chunk + remainToRead, chunkLen - remainToRead);
			J_S_ASSERT( bstrObj != NULL, "Unable to create the binary string." );
			UnshiftJsval(cx, pv->queue, OBJECT_TO_JSVAL( bstrObj ));
			remainToRead = 0; // adjust remaining required data length
		}
	}
	pv->length -= amount;
	return JS_TRUE;
}


JSBool ReadAmount( JSContext *cx, JSObject *obj, size_t amount, jsval *rval ) {

	if ( amount == 0 ) { // optimization

		*rval = JS_GetEmptyStringValue(cx);
		return JS_TRUE;
	}

	char *str = (char*)JS_malloc(cx, amount +1); // (TBD) memory leak if ReadRawAmount failed
	J_S_ASSERT_ALLOC(str);

	// (TBD) IMPORTANT: here, amount should be MIN( amount, buffer_size ). This can avoid an useless memory allocation.
	int requestedAmount = amount;
	J_CHK( ReadRawAmount(cx, obj, &amount, str) );

	if ( amount == 0 ) { // optimization

		JS_free(cx, str);
		*rval = JS_GetEmptyStringValue(cx);
		return JS_TRUE;
	}

	str[amount] = '\0'; // (TBD) explain this

	if ( MaybeRealloc( requestedAmount, amount ) ) {

		str = (char*)JS_realloc(cx, str, amount +1);
		J_S_ASSERT_ALLOC(str);
	}

	JSObject *bstr = J_NewBinaryString(cx, str, amount);
	J_S_ASSERT( bstr != NULL, "Unable to create the BString." );
	*rval = OBJECT_TO_JSVAL(bstr);
	return JS_TRUE;
}


JSBool FindInBuffer( JSContext *cx, JSObject *obj, const char *needle, size_t needleLength, int *foundAt ) {

	// (TBD) optimise this function for needleLength == 1 (eg. '\0' in a string)
	BufferPrivate *pv = (BufferPrivate*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );

	size_t pos = 0;

	char staticBuffer[128];
	char *buf = needleLength <= sizeof(staticBuffer) ? staticBuffer : (char*)malloc(needleLength); // the "ring buffer"

	size_t chunkLength;
	const char *chunk;
	size_t i, j;

	for ( QueueCell *it = pv->queue->begin; it; it = it->next ) {

		jsval *pNewStr = (jsval*)QueueGetData(it);
		J_CHK( JsvalToStringAndLength(cx, *pNewStr, &chunk, &chunkLength) );
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
}


JSBool AddBuffer( JSContext *cx, JSObject *destBuffer, JSObject *srcBuffer ) {

	J_S_ASSERT_CLASS( destBuffer, &classBuffer );
	BufferPrivate *dpv = (BufferPrivate*)JS_GetPrivate(cx, destBuffer);
	J_S_ASSERT_RESOURCE( dpv );

	J_S_ASSERT_CLASS( srcBuffer, &classBuffer );
	BufferPrivate *spv = (BufferPrivate*)JS_GetPrivate(cx, srcBuffer);
	J_S_ASSERT_RESOURCE( spv );

	for ( QueueCell *it = QueueBegin(spv->queue); it; it = QueueNext(it) )
		J_CHK( PushJsval(cx, dpv->queue, *(jsval*)QueueGetData(it)) );
	dpv->length += spv->length;

	return JS_TRUE;
}


/**doc
$CLASS_HEADER
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

/**doc
 * $INAME( [string | Buffer object] )
  Constructs a Buffer object.
  If a string is given as argument, the buffer is initialized with this string.
  If buffer object is given, the buffer is initialized with this existing buffer object (kind of copy constructor).
**/
DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();
	J_CHK( SetStreamReadInterface(cx, obj, NativeInterfaceStreamRead) );

	BufferPrivate *pv = (BufferPrivate *)JS_malloc(cx, sizeof(BufferPrivate));
	J_S_ASSERT_ALLOC(pv);
	J_CHK( JS_SetPrivate(cx, obj, pv) );
	pv->queue = QueueConstruct();
	J_S_ASSERT_ALLOC(pv->queue);
	pv->length = 0;

	if ( J_ARG_ISDEF(1) ) {

		// (TBD) loop over all args
		if ( J_JSVAL_IS_CLASS(J_ARG(1), _class) ) {

			return AddBuffer(cx, obj, JSVAL_TO_OBJECT( J_ARG(1) ));
		} else {

			size_t length;
			J_CHK( JsvalToStringLength(cx, J_ARG(1), &length) );
			if ( length == 0 )
				return JS_TRUE;
			pv->length += length;
			return PushJsval(cx, pv->queue, J_ARG(1));
		}
	}
	return JS_TRUE;
}


/**doc
=== Methods ===
**/

/**doc
 * $VOID $INAME()
  Empty the whole buffer.
**/
DEFINE_FUNCTION( Clear ) {

	BufferPrivate *pv = (BufferPrivate*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	while ( !QueueIsEmpty(pv->queue) )
		J_CHK( ShiftJsval(cx, pv->queue, NULL) );
	pv->length = 0;
	return JS_TRUE;
}


/**doc
 * $INAME( data [, length] )
  Add _data_ in the buffer. If _length_ is used, only the first _length_ bytes of _data_ are added.
**/
DEFINE_FUNCTION( Write ) {

	BufferPrivate *pv = (BufferPrivate*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	J_S_ASSERT_ARG_MIN( 1 );

	if ( J_ARG_ISDEF(2) ) {

		size_t amount, strLen;
		const char *buf;
		J_CHK( JsvalToStringAndLength(cx, J_ARG(1), &buf, &strLen) );

		if ( strLen == 0 )
			return JS_TRUE;

		J_JSVAL_TO_INT32( J_ARG(2), amount );
		if ( amount > strLen )
			amount = strLen;

		return WriteRawChunk(cx, obj, amount, buf);
	} else {

		return WriteChunk(cx, obj, J_ARG(1));
	}
}


/**doc
 * $BOOL $INAME( str [, consume = false ] )
  Check if the given string _str_ matchs to the next data in the buffer. 
  $H arguments
   $ARG string str
   $ARG boolean consume: if false, just check if it match without consuming data, else, read and check.
  $H return value
   true if it matchs, else false.
**/
DEFINE_FUNCTION( Match ) {

	J_S_ASSERT_ARG_MIN( 1 );

	const char *str;
	size_t len;
	J_CHK( JsvalToStringAndLength(cx, J_ARG(1), &str, &len) );

	char *src = (char *)malloc(len);
	size_t amount = len;
	JSBool st = ReadRawAmount(cx, obj, &amount, src);
	if ( st != JS_TRUE )
		goto err;

	if ( amount != len )
		*rval = JSVAL_FALSE;
	else
		*rval = strncmp( str, src, len ) == 0 ? JSVAL_TRUE : JSVAL_FALSE;

	bool consume;
	if ( J_ARG_ISDEF(2) )
		J_JSVAL_TO_BOOL( J_ARG(2), consume );
	else
		consume = false;
	
	if ( !consume )
		J_CHK( UnReadRawChunk(cx, obj, src, amount) );

err:
	free(src);
	return JS_TRUE;
}


/**doc
 * $STR $INAME( [ amount ] )
  Read _amount_ data in the buffer. If _amount_ is omited, The whole buffer is returned.
  = =
  If _amount_ == undefined, an arbitrary (ideal) amount of data is returned. Use this when you don't know how many data you have to read.
  ===== example: =====
  {{{
  var chunk = buffer.Read(undefined);
  }}}
  ===== note: =====
  The read operation never blocks, even if the requested amount of data is greater than the buffer length.
**/
DEFINE_FUNCTION( Read ) { // Read( [ amount | <undefined> ] )

	BufferPrivate *pv = (BufferPrivate*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );

	if ( J_ARGC == 1 && J_ARG(1) == JSVAL_VOID ) // read the next chunk (of an unknown length) (read something as fast as possible)
		return ReadChunk(cx, obj, rval);

	size_t amount;
	if ( J_ARG_ISDEF(1) )
		J_JSVAL_TO_INT32( J_ARG(1), amount );
	else
		amount = pv->length;

	J_S_ASSERT( amount >= 0, "Invalid amount" );
	J_CHK( ReadAmount(cx, obj, amount, rval) );
	return JS_TRUE;
}


/**doc
 * $INAME( length )
  Skip _length_ bytes of data from the buffer.
**/
DEFINE_FUNCTION( Skip ) { // Skip( amount )

	BufferPrivate *pv = (BufferPrivate*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	J_S_ASSERT_ARG_MIN( 1 );
	size_t amount;
	J_JSVAL_TO_INT32( J_ARG(1), amount );
	J_S_ASSERT( amount >= 0, "Invalid amount" );
	size_t prevBufferLength = pv->length;
	J_CHK( BufferSkipAmount(cx, obj, amount) ); // (TBD) optimization: skip without reading the data.
	*rval = BOOLEAN_TO_JSVAL( pv->length == prevBufferLength - amount ); // function returns true on sucessful skip operation
	return JS_TRUE;
}


/**doc
 * $STR $INAME( boundaryString [, skip] )
  Reads the buffer until it match the _boundaryString_, else it returns <undefined>.
  If _skip_ argument is <true>, the _boundaryString_ is skiped from the buffer.
**/
DEFINE_FUNCTION( ReadUntil ) {

	J_S_ASSERT_ARG_MIN( 1 );
	const char *boundary;
	size_t boundaryLength;

	J_CHK( JsvalToStringAndLength(cx, J_ARG(1), &boundary, &boundaryLength) );

	bool skip;
	if ( J_ARG_ISDEF(2) )
		J_JSVAL_TO_BOOL(J_ARG(2), skip);
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
}


/**doc
 * $INT $INAME( string )
  Find _string_ in the buffer and returns the offset of the first letter. If not found, this function returns -1.
**/
DEFINE_FUNCTION( IndexOf ) {

	J_S_ASSERT_ARG_MIN( 1 );
	const char *boundary;
	size_t boundaryLength;
	J_CHK( JsvalToStringAndLength(cx, J_ARG(1), &boundary, &boundaryLength) );
	int found;
	J_CHK( FindInBuffer(cx, obj, boundary, boundaryLength, &found) );
	*rval = INT_TO_JSVAL(found);
	return JS_TRUE;
}


/**doc
 * $STR $INAME( _data_ )
  Insert _data_ at the begining of the buffer. This function can undo a read operation. The returned value is _data_.
  ===== example: =====
  {{{
  function Peek(len) {

    return buffer.Unread( buffer.Read(len) );
  }
  }}}
**/
DEFINE_FUNCTION( Unread ) {

	J_S_ASSERT_ARG_MIN( 1 );
	J_S_ASSERT_ARG_MAX( 1 ); // discourages one to use Unread like Write
	J_CHK( UnReadChunk(cx, obj, J_ARG(1)) ); // no need to use JS_NewDependentString (see js_NewDependentString in jsstr.c)
	*rval = J_ARG(1);
	return JS_TRUE;
}


/**doc
 * $STR $INAME()
  Converts the whole content of the buffer to a string.
**/

// Note:  String( { toString:function() { return [1,2,3]} } );  throws the following error: "can't convert Object to string"
DEFINE_FUNCTION( toString ) {

	BufferPrivate *pv = (BufferPrivate*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );

	if ( pv->length == 0 ) {

		*rval = JS_GetEmptyStringValue(cx);
		return JS_TRUE;
	}

	char *buffer = (char*)JS_malloc(cx, pv->length +1);
	J_S_ASSERT_ALLOC( buffer );
	buffer[pv->length] = '\0';

	size_t pos = 0;
	while ( !QueueIsEmpty(pv->queue) ) {

		J_CHK( ShiftJsval(cx, pv->queue, rval) );
		const char *chunkBuf;
		size_t chunkLen;
		J_CHK( JsvalToStringAndLength(cx, *rval, &chunkBuf, &chunkLen) );
		memcpy(buffer + pos, chunkBuf, chunkLen);
		pos += chunkLen;
	}

	JSString *str = JS_NewString(cx, buffer, pv->length);
	J_S_ASSERT_ALLOC( str );
	*rval = STRING_TO_JSVAL(str);
	pv->length = 0;

	// we have to return a real string !
	//char *bstrBuf = (char*)JS_malloc(cx, pv->length);
	//J_S_ASSERT_ALLOC( bstrBuf );
	//size_t amount = pv->length;
	//J_CHK( ReadRawAmount(cx, obj, &amount, bstrBuf) );
	//JSObject *bstrObj = NewBString(cx, bstrBuf, pv->length);
	//J_S_ASSERT( bstrObj != NULL, "Unable to create the BString." );
	//*J_RVAL = OBJECT_TO_JSVAL(bstrObj);

	return JS_TRUE;
}


/**doc
=== Properties ===
**/

/**doc
 * $INT $INAME $READONLY
  Is the current length of the buffer.
**/
DEFINE_PROPERTY( length ) {

	BufferPrivate *pv = (BufferPrivate*)JS_GetPrivate(cx, obj);
	*vp = INT_TO_JSVAL(pv->length);
	return JS_TRUE;
}


//DEFINE_PROPERTY( source ) { // do not support: delete buf.source
//
//	J_CHK( JS_SetReservedSlot(cx, obj, SLOT_SOURCE, *vp) );
//	return JS_TRUE;
//}


DEFINE_TRACER() {

	BufferPrivate *pv = (BufferPrivate*)JS_GetPrivate(trc->context, obj);
	if ( pv )
		for ( QueueCell *it = QueueBegin(pv->queue); it; it = QueueNext(it) )
			JS_CALL_VALUE_TRACER(trc, *(jsval*)QueueGetData(it), "jsstd/Buffer:bufferQueueItem");
}


/* previous doc:
 * *onunderflow*( bufferObject, missingAmount )
  This function is called by the Buffer when its length is less than the requested amount of data.
  $H arguments
   $ARG Buffer bufferObject: is the buffer object that caused the call.
   $ARG integer missingAmount: is the  missing amount of data to complete the request at once.
  This function is called until the buffer size do not grow any more.
*/

/**doc tab:2
 * $OBJ *source*
  The source property can contains any NIStreamRead compatible object. The Buffer uses this object when its length is less than the requested amount of data. Any extra data returned by the NIStreamRead object is keept in the buffer and will be used at the next read operation.
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
 *NIStreamRead*
**/

CONFIGURE_CLASS

	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1)

	HAS_CONSTRUCTOR
	HAS_FINALIZE
	HAS_TRACER

	BEGIN_FUNCTION_SPEC
		FUNCTION(Clear)
		FUNCTION(Write)
		FUNCTION(Unread)
		FUNCTION(Read)
		FUNCTION(ReadUntil)
		FUNCTION(IndexOf)
		FUNCTION(Match)
		FUNCTION(Skip)
		FUNCTION(toString) // used when the buffer has to be transformed into a javascript string
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ(length)
//		PROPERTY_WRITE_STORE(source)
	END_PROPERTY_SPEC

END_CLASS


/**doc
=== example ===
 {{{
 var buf = new Buffer();
 buf.Write('1234');
 buf.Write('5');
 buf.Write('');
 buf.Write('6789');
 Print( buf.Read() );
 }}}

=== example ===
 {{{
 var buf = new Buffer();
 buf.Write('0123456789');
 Print( buf.Read(4) );
 Print( buf.Read(1) );
 Print( buf.Read(1) );
 Print( buf.Read(4) );
 }}}
**/
