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
// - buffer length is stored in SLOT_BUFFER_LENGTH, and MUST always be up to date.
// - each buffer chunk is a JSString** that is rooted before being stored in the chunk queue.
//

#include "stdafx.h"
#include <cstring>

#include "buffer.h"

#include "../common/queue.h"
#include "../jslang/bstringapi.h"


static JSBool NativeInterfaceStreamRead( JSContext *cx, JSObject *obj, char *buf, size_t *amount ) {

	#ifdef JS_THREADSAFE
		JS_BeginRequest( cx ); // http://developer.mozilla.org/en/docs/JS_BeginRequest
	#endif

	J_CHK( ReadRawAmount(cx, obj, amount, buf) );

	#ifdef JS_THREADSAFE
		JS_EndRequest( cx );
	#endif
	return JS_TRUE;
}


inline JSBool BufferLengthSet( JSContext *cx, JSObject *obj, size_t bufferLength ) {

	J_CHK( JS_SetReservedSlot(cx, obj, SLOT_BUFFER_LENGTH, INT_TO_JSVAL( bufferLength )) );
	return JS_TRUE;
}


inline JSBool BufferLengthGet( JSContext *cx, JSObject *obj, size_t *bufferLength ) {

	jsval bufferLengthVal;
	J_CHK( JS_GetReservedSlot(cx, obj, SLOT_BUFFER_LENGTH, &bufferLengthVal) );
	*bufferLength = JSVAL_TO_INT(bufferLengthVal);
	return JS_TRUE;
}


inline JSBool BufferLengthAdd( JSContext *cx, JSObject *obj, size_t amount ) {

	size_t bufferLength;
	J_CHK( BufferLengthGet(cx, obj, &bufferLength) );
	bufferLength += amount;
	J_CHK( BufferLengthSet(cx, obj, bufferLength) );
	return JS_TRUE;
}


inline JSBool PushJsval( JSContext *cx, Queue *queue, jsval value ) {

	jsval *pItem = (jsval*)malloc(sizeof(jsval));
	J_S_ASSERT_ALLOC( pItem );
	*pItem = value;
//	if ( JSVAL_IS_GCTHING(*pItem) )
//		J_CHK( J_ADD_ROOT(cx, pItem) ); // cf. Trace callback
	QueuePush( queue, pItem );
	return JS_TRUE;
}


inline JSBool UnshiftJsval( JSContext *cx, Queue *queue, jsval value ) {

	jsval *pItem = (jsval*)malloc(sizeof(jsval));
	J_S_ASSERT_ALLOC( pItem );
	*pItem = value;
//	if ( JSVAL_IS_GCTHING(*pItem) )
//		J_CHK( J_ADD_ROOT(cx, pItem) ); // cf. Trace callback
	QueueUnshift( queue, pItem );
	return JS_TRUE;
}


inline JSBool ShiftJsval( JSContext *cx, Queue *queue, jsval *value ) {

	jsval *pItem = (jsval*)QueueShift(queue);
//	if ( JSVAL_IS_GCTHING(*pItem) )
//		J_REMOVE_ROOT(cx, pItem); // cf. Trace callback
	if ( value != NULL )
		*value = *pItem;
	free(pItem);
	return JS_TRUE;
}


inline JSBool BufferRefillRequest( JSContext *cx, JSObject *obj, size_t missing ) { // missing = missing amount of data to complete the request

	jsval rval, fctVal;
	J_CHK( JS_GetProperty(cx, obj, "onunderflow", &fctVal) );
	jsval argv[] = { OBJECT_TO_JSVAL(obj), INT_TO_JSVAL(missing) };
	if ( JsvalIsFunction(cx, fctVal) )
		JS_CallFunctionValue(cx, obj, fctVal, sizeof(argv)/sizeof(*argv), argv, &rval);
	return JS_TRUE;
}



JSBool AddBuffer( JSContext *cx, JSObject *destBuffer, JSObject *srcBuffer ) {

	J_S_ASSERT_CLASS( destBuffer, &classBuffer );
	Queue *destQueue = (Queue*)JS_GetPrivate(cx, destBuffer);
	J_S_ASSERT_RESOURCE( destQueue );

	J_S_ASSERT_CLASS( srcBuffer, &classBuffer );
	Queue *srcQueue = (Queue*)JS_GetPrivate(cx, srcBuffer);
	J_S_ASSERT_RESOURCE( srcQueue );

	for ( QueueCell *it = QueueBegin(srcQueue); it; it = QueueNext(it) ) {

		J_CHK( PushJsval(cx, destQueue, *(jsval*)QueueGetData(it)) );
	}
	size_t bufLength;
	J_CHK( BufferLengthGet(cx, srcBuffer, &bufLength) );
	J_CHK( BufferLengthAdd(cx, destBuffer, bufLength) );
	return JS_TRUE;
}


JSBool WriteRawData( JSContext *cx, JSObject *obj, size_t amount, char *str ) {

	Queue *queue = (Queue*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(queue);
	JSObject* bstrObj;
	J_CHK( NewBStringCopyN(cx, str, amount, &bstrObj) );
	J_CHK( PushJsval(cx, queue, OBJECT_TO_JSVAL(bstrObj)) );
	J_CHK( BufferLengthAdd(cx, obj, amount) );
	return JS_TRUE;
}


JSBool ReadOneChunk( JSContext *cx, JSObject *obj, jsval *rval ) {

  	size_t bufferLength;
	J_CHK( BufferLengthGet(cx, obj, &bufferLength) );
	if ( bufferLength == 0 ) { // if buffer is empty, try to refill it.

		J_CHK( BufferRefillRequest(cx, obj, 0) );
		J_CHK( BufferLengthGet(cx, obj, &bufferLength) ); // read it again because it may have changed
		if ( bufferLength == 0 ) { // the data are definitively exhausted

			*rval = JS_GetEmptyStringValue(cx);
			return JS_TRUE;
		}
	}
	Queue *queue = (Queue*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( queue );
	J_CHK( ShiftJsval(cx, queue, rval) );
	size_t len;
	J_CHK( JsvalToStringLength(cx, *rval, &len) );
	J_CHK( BufferLengthSet(cx, obj, bufferLength - len) ); // update buffer size

	return JS_TRUE;
}


JSBool ReadRawAmount( JSContext *cx, JSObject *obj, size_t *amount, char *str ) { // this form allows one to read into a static buffer

	if ( *amount == 0 ) { // optimization

		return JS_TRUE;
	}

	size_t bufferLength;
	J_CHK( BufferLengthGet(cx, obj, &bufferLength) );

	if ( bufferLength < *amount || bufferLength == 0 ) { // more that the available data is required, then try to refill the buffer

		size_t bufferLengthBeforeRefillRequest;
		do {

			bufferLengthBeforeRefillRequest = bufferLength;
			J_CHK( BufferRefillRequest(cx, obj, *amount - bufferLength) );
			J_CHK( BufferLengthGet(cx, obj, &bufferLength) ); // read it again because it may have changed
		} while( *amount > bufferLength && bufferLength > bufferLengthBeforeRefillRequest ); // see RULES ( at the top of this file )

		if ( *amount > bufferLength ) // we definitively cannot read the required amount of data, then read the whole buffer.
			*amount = bufferLength;
	}

	if ( *amount == 0 ) { // another optimization

		return JS_TRUE;
	}


	char *ptr = str;
	size_t remainToRead = *amount;

	Queue *queue = (Queue*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( queue );

	while ( remainToRead > 0 ) { // while there is something to read,

//		JSString **pNewStr = (JSString**)QueueGetData(QueueBegin(queue)); // just get the data, do not shift the queue

//		jsval *pNewStr = (jsval*)QueueGetData(QueueBegin(queue)); // just get the data, do not shift the queue

		jsval item;
		J_CHK( ShiftJsval(cx, queue, &item) );


//		char *chunk = JS_GetStringBytes(*pNewStr);
//		size_t chunkLen = J_STRING_LENGTH(*pNewStr);
//		RT_JSVAL_TO_STRING_AND_LENGTH( *pNewStr, chunk, chunkLength );

		size_t chunkLen;
		const char *chunk;
		J_CHK( JsvalToStringAndLength(cx, item, &chunk, &chunkLen) );

		if ( chunkLen <= remainToRead ) {

			memcpy(ptr, chunk, chunkLen);
			ptr += chunkLen;
			remainToRead -= chunkLen; // adjust remaining required data length
//			J_CHK( ShiftJsval(cx, queue, NULL) );

		} else { // chunkLen > remain: this is the last chunk we have to manage. we only get a part of it chunk and we 'unread' the remaining.

			memcpy(ptr, chunk, remainToRead);
			JSObject *bstrObj;
			J_CHK( NewBStringCopyN(cx, chunk + remainToRead, chunkLen - remainToRead, &bstrObj) );
			UnshiftJsval(cx, queue, OBJECT_TO_JSVAL( bstrObj ));
			remainToRead = 0; // adjust remaining required data length
		}
	}
	bufferLength -= *amount;
	J_CHK( BufferLengthSet(cx, obj, bufferLength) ); // update buffer size
	return JS_TRUE;
}



JSBool ReadAmount( JSContext *cx, JSObject *obj, size_t amount, jsval *rval ) {

	if ( amount == 0 ) { // optimization

		*rval = JS_GetEmptyStringValue(cx);
		return JS_TRUE;
	}

	char *str = (char*)JS_malloc(cx, amount + 1); // (TBD) memory leak if ReadRawAmount failed
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

	// (TBD) if amount after calling ReadRawAmount is smaller than before, realloc the buffer
	if ( MaybeRealloc( requestedAmount, amount ) ) {

		str = (char*)JS_realloc(cx, str, amount + 1);
		J_S_ASSERT_ALLOC(str);
	}

//	*rval = STRING_TO_JSVAL(JS_NewString(cx, str, amount));
	JSObject *bstr = NewBString(cx, str, amount);
	J_S_ASSERT( bstr != NULL, "Unable to create the BString." );
	*rval = OBJECT_TO_JSVAL(bstr);

	return JS_TRUE;
}


JSBool UnReadChunk( JSContext *cx, JSObject *obj, jsval chunk ) {

	Queue *queue = (Queue*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( queue );
	size_t length;
	J_CHK( JsvalToStringLength(cx, chunk, &length) );
	if ( length == 0 ) // optimization && RULES
		return JS_TRUE;
	J_CHK( UnshiftJsval(cx, queue, chunk) );
	J_CHK( BufferLengthAdd(cx, obj, length) );
	return JS_TRUE;
}


JSBool UnReadRawChunk( JSContext *cx, JSObject *obj, char *data, size_t length ) {

	if ( length == 0 ) // optimization & RULES
		return JS_TRUE;
	JSObject* bstrObj;
	J_CHK( NewBStringCopyN(cx, data, length, &bstrObj) );
	J_CHK( UnReadChunk(cx, obj, OBJECT_TO_JSVAL(bstrObj)) );
	return JS_TRUE;
}


JSBool FindInBuffer( JSContext *cx, JSObject *obj, const char *needle, size_t needleLength, int *foundAt ) {

	// (TBD) optimise this function for needleLength == 1 (eg. '\0' in a string)
	Queue *queue = (Queue*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( queue );

	size_t pos = 0;
	char *buf = (char*)malloc(needleLength); // the "ring buffer"

	size_t chunkLength;
	const char *chunk;
	size_t i, j;

	for ( QueueCell *it = queue->begin; it; it = it->next ) {

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
	free(buf); // free the "ring buffer"
	return JS_TRUE;
}


BEGIN_CLASS( Buffer )

DEFINE_FINALIZE() {

	Queue *queue = (Queue*)JS_GetPrivate(cx, obj);
	if ( queue != NULL ) {

		while ( !QueueIsEmpty(queue) )
			ShiftJsval(cx, queue, NULL);
		QueueDestruct(queue);
		JS_SetPrivate(cx, obj, NULL); // optional ?
	}
}


DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();
	J_CHK( SetStreamReadInterface(cx, obj, NativeInterfaceStreamRead) );
	Queue *queue = QueueConstruct();
	J_S_ASSERT_ALLOC(queue);
	J_CHK( JS_SetPrivate(cx, obj, queue) );
	BufferLengthSet(cx, obj, 0);

	if ( J_ARG_ISDEF(1) ) {

		if ( JSVAL_IS_OBJECT(J_ARG(1)) && !JSVAL_IS_NULL(J_ARG(1)) && J_JSVAL_IS_CLASS(J_ARG(1), _class) ) {

			J_CHK( AddBuffer(cx, obj, JSVAL_TO_OBJECT( J_ARG(1) )) );
		} else {

			size_t length;
			J_CHK( JsvalToStringLength(cx, J_ARG(1), &length) );
			if ( length == 0 )
				return JS_TRUE;
			J_CHK( BufferLengthAdd(cx, obj, length) );
			J_CHK( PushJsval(cx, queue, J_ARG(1)) );
		}
	}

	return JS_TRUE;
}


DEFINE_FUNCTION( Clear ) {

	Queue *queue = (Queue*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(queue);
	while ( !QueueIsEmpty(queue) )
		J_CHK( ShiftJsval(cx, queue, NULL) );
	BufferLengthSet(cx, obj, 0);
	return JS_TRUE;
}


DEFINE_FUNCTION( Write ) {

	J_S_ASSERT_ARG_MIN( 1 );
	Queue *queue = (Queue*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(queue);

	size_t strLen;
	J_CHK( JsvalToStringLength(cx, J_ARG(1), &strLen) );

	if ( strLen == 0 )
		return JS_TRUE;

	jsval item;

	size_t amount;
	if ( J_ARG_ISDEF(2) ) {

		RT_JSVAL_TO_INT32( J_ARG(2), amount );
		if ( amount > strLen )
			amount = strLen;

		const char *buf;
		J_CHK( JsvalToString(cx, J_ARG(1), &buf) );

		//	(TBD) use JS_NewDependentString if arg 1 is a JSString

		JSObject* bstrObj;
		NewBStringCopyN(cx, buf, amount, &bstrObj);
		item = OBJECT_TO_JSVAL(bstrObj);

	} else {

		item = J_ARG(1);
		amount = strLen;
	}

	J_CHK( PushJsval(cx, queue, item) );
	J_CHK( BufferLengthAdd(cx, obj, amount) );
	return JS_TRUE;
}


DEFINE_FUNCTION( Match ) {

	J_S_ASSERT_ARG_MIN( 1 );
	J_S_ASSERT_RESOURCE( JS_GetPrivate(cx, obj) ); // first, ensure that the object is valid

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

err:
	free(src);
	return JS_TRUE;
}


DEFINE_FUNCTION( Read ) { // Read( [ amount | <undefined> ] )

	J_S_ASSERT_RESOURCE( JS_GetPrivate(cx, obj) ); // first, ensure that the object is valid

	if ( J_ARGC == 1 && J_ARG(1) == JSVAL_VOID ) { // read the next chunk (of an unknown length) (read something as fast as possible)

		J_CHK( ReadOneChunk(cx, obj, rval) );
		return JS_TRUE;
	}

	size_t amount;
	if ( J_ARG_ISDEF(1) )
		RT_JSVAL_TO_INT32( J_ARG(1), amount );
	else
		J_CHK( BufferLengthGet(cx, obj, &amount) ); // no arguments then read the whole buffer

	J_S_ASSERT( amount >= 0, "Invalid amount" );
	J_CHK( ReadAmount(cx, obj, amount, rval) );
	return JS_TRUE;
}


DEFINE_FUNCTION( Skip ) { // Skip( amount )

	J_S_ASSERT_ARG_MIN( 1 );
	J_S_ASSERT_RESOURCE( JS_GetPrivate(cx, obj) ); // first, ensure that the object is valid
	size_t amount;
	RT_JSVAL_TO_INT32( J_ARG(1), amount );
	J_S_ASSERT( amount >= 0, "Invalid amount" );
	J_CHK( ReadAmount(cx, obj, amount, rval) ); // (TBD) optimization: skip without reading the data.
	*rval = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION( ReadUntil ) {

	J_S_ASSERT_ARG_MIN( 1 );
	const char *boundary;
	size_t boundaryLength;

	J_CHK( JsvalToStringAndLength(cx, J_ARG(1), &boundary, &boundaryLength) );

	bool skip;
	if ( J_ARG_ISDEF(2) )
		RT_JSVAL_TO_BOOL(J_ARG(2), skip);
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


DEFINE_FUNCTION( Unread ) {

	J_S_ASSERT_ARG_MIN( 1 );
	J_S_ASSERT_ARG_MAX( 1 ); // discourages one to use Unread like Write
	J_CHK( UnReadChunk(cx, obj, J_ARG(1)) ); // no need to use JS_NewDependentString (see js_NewDependentString in jsstr.c)
	*rval = J_ARG(1);
	return JS_TRUE;
}


DEFINE_FUNCTION( toString ) {

	Queue *queue = (Queue*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( queue );

	size_t bufferLength;
	J_CHK( BufferLengthGet(cx, obj, &bufferLength) );
	if ( bufferLength == 0 ) {

		*rval = JS_GetEmptyStringValue(cx);
		return JS_TRUE;
	}

	char *buffer = (char*)JS_malloc(cx, bufferLength +1);
	J_S_ASSERT_ALLOC( buffer );
	buffer[bufferLength] = '\0';

	size_t pos = 0;

	while ( !QueueIsEmpty(queue) ) {

		J_CHK( ShiftJsval(cx, queue, rval) );
		const char *chunkBuf;
		size_t chunkLen;
		J_CHK( JsvalToStringAndLength(cx, *rval, &chunkBuf, &chunkLen) );
		memcpy(buffer + pos, chunkBuf, chunkLen);
		pos += chunkLen;
	}

	J_CHK( BufferLengthSet(cx, obj, 0) );

	JSString *str = JS_NewString(cx, buffer, bufferLength);
	J_S_ASSERT_ALLOC( str );
	*rval = STRING_TO_JSVAL(str);

	return JS_TRUE;
}


DEFINE_PROPERTY( length ) {

	J_CHK( JS_GetReservedSlot(cx, obj, SLOT_BUFFER_LENGTH, vp) );
	return JS_TRUE;
}


DEFINE_TRACE() {

	Queue *queue = (Queue*)JS_GetPrivate(trc->context, obj);
	if ( queue )
		for ( QueueCell *it = QueueBegin(queue); it; it = QueueNext(it) )
			JS_CALL_VALUE_TRACER(trc, *(jsval*)QueueGetData(it), "jsstd/Buffer/bufferQueueItem");
}


CONFIGURE_CLASS

	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1)

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	HAS_TRACE

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
	END_PROPERTY_SPEC

END_CLASS
