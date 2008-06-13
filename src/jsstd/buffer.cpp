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


// missing = missing amount of data to complete the request
inline JSBool BufferRefillRequest( JSContext *cx, JSObject *obj, size_t missing ) {

	jsval rval, fctVal;
	J_CHK( JS_GetProperty(cx, obj, "onunderflow", &fctVal) );
	jsval argv[] = { OBJECT_TO_JSVAL(obj), INT_TO_JSVAL(missing) };
	if ( JsvalIsFunction(cx, fctVal) )
		JS_CallFunctionValue(cx, obj, fctVal, sizeof(argv)/sizeof(*argv), argv, &rval);
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
	JSObject* bstrObj;
	J_CHK( NewBStringCopyN(cx, str, amount, &bstrObj) );
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


JSBool UnReadRawChunk( JSContext *cx, JSObject *obj, char *data, size_t length ) {

	if ( length == 0 ) // optimization & RULES
		return JS_TRUE;
	JSObject* bstrObj;
	J_CHK( NewBStringCopyN(cx, data, length, &bstrObj) );
	J_CHK( UnReadChunk(cx, obj, OBJECT_TO_JSVAL(bstrObj)) );
	return JS_TRUE;
}


JSBool ReadChunk( JSContext *cx, JSObject *obj, jsval *rval ) {

	BufferPrivate *pv = (BufferPrivate*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );

	if ( pv->length == 0 ) { // if buffer is empty, try to refill it.

		J_CHK( BufferRefillRequest(cx, obj, 0) );
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

	BufferPrivate *pv = (BufferPrivate*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );

	if ( pv->length < *amount || pv->length == 0 ) { // more that the available data is required, then try to refill the buffer

		size_t bufferLengthBeforeRefillRequest;
		do {

			bufferLengthBeforeRefillRequest = pv->length;
			J_CHK( BufferRefillRequest(cx, obj, *amount - pv->length) );
		} while( pv->length < *amount && pv->length > bufferLengthBeforeRefillRequest ); // see RULES ( at the top of this file )

		if ( pv->length < *amount ) // we definitively cannot read the required amount of data, then read the whole buffer.
			*amount = pv->length;
	}

	if ( *amount == 0 ) // another optimization
		return JS_TRUE;

	char *ptr = str;
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
			JSObject *bstrObj;
			J_CHK( NewBStringCopyN(cx, chunk + remainToRead, chunkLen - remainToRead, &bstrObj) );
			UnshiftJsval(cx, pv->queue, OBJECT_TO_JSVAL( bstrObj ));
			remainToRead = 0; // adjust remaining required data length
		}
	}
	pv->length -= *amount;
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

	JSObject *bstr = NewBString(cx, str, amount);
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



DEFINE_FUNCTION( Clear ) {

	BufferPrivate *pv = (BufferPrivate*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	while ( !QueueIsEmpty(pv->queue) )
		J_CHK( ShiftJsval(cx, pv->queue, NULL) );
	pv->length = 0;
	return JS_TRUE;
}


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

		RT_JSVAL_TO_INT32( J_ARG(2), amount );
		if ( amount > strLen )
			amount = strLen;

		return WriteRawChunk(cx, obj, amount, buf);
	} else {
		
		return WriteChunk(cx, obj, J_ARG(1));
	}
}


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

err:
	free(src);
	return JS_TRUE;
}


DEFINE_FUNCTION( Read ) { // Read( [ amount | <undefined> ] )

	BufferPrivate *pv = (BufferPrivate*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );

	if ( J_ARGC == 1 && J_ARG(1) == JSVAL_VOID ) // read the next chunk (of an unknown length) (read something as fast as possible)
		return ReadChunk(cx, obj, rval);

	size_t amount;
	if ( J_ARG_ISDEF(1) )
		RT_JSVAL_TO_INT32( J_ARG(1), amount );
	else
		amount = pv->length;

	J_S_ASSERT( amount >= 0, "Invalid amount" );
	J_CHK( ReadAmount(cx, obj, amount, rval) );
	return JS_TRUE;
}


DEFINE_FUNCTION( Skip ) { // Skip( amount )

	J_S_ASSERT_ARG_MIN( 1 );

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


DEFINE_PROPERTY( length ) {

	BufferPrivate *pv = (BufferPrivate*)JS_GetPrivate(cx, obj);
	*vp = INT_TO_JSVAL(pv->length);
	return JS_TRUE;
}


DEFINE_TRACER() {

	BufferPrivate *pv = (BufferPrivate*)JS_GetPrivate(trc->context, obj);
	if ( pv )
		for ( QueueCell *it = QueueBegin(pv->queue); it; it = QueueNext(it) )
			JS_CALL_VALUE_TRACER(trc, *(jsval*)QueueGetData(it), "jsstd/Buffer:bufferQueueItem");
}


CONFIGURE_CLASS

	HAS_PRIVATE

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
	END_PROPERTY_SPEC

END_CLASS
