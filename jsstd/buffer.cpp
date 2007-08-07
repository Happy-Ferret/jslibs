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
#include "buffer.h"

#include "../common/jsNativeInterface.h"
#include "../common/queue.h"

#include <jscntxt.h>

inline JSBool ReadRawAmount( JSContext *cx, JSObject *obj, size_t *amount, char *str );

typedef struct JsCntxt {
	JSRuntime *rt;
	JSObject *obj;
} JsCntxt;


static bool NativeInterfaceReadBuffer( void *pv, unsigned char *buf, unsigned int *amount ) {

	JsCntxt *cntxt = (JsCntxt*)pv;
	JSContext *cx = NULL;
	JS_ContextIterator(cntxt->rt, &cx); // (TBD) find a better way to get a suitable cx ( beware: cx & threads )
	JSObject *obj = cntxt->obj;
	RT_SAFE( if ( obj == NULL || cx == NULL ) return false );

	#ifdef JS_THREADSAFE
		JS_BeginRequest( cx ); // http://developer.mozilla.org/en/docs/JS_BeginRequest
	#endif

	JSBool status = ReadRawAmount(cx, obj, amount, (char*)buf);
	if ( status != JS_TRUE )
		return false;
/*
	jsval rval, argv[] = { INT_TO_JSVAL(*amount) };
	if ( JS_CallFunctionName(cx, obj, "Read", 1, argv, &rval) == JS_FALSE ) // (TBD) use a lower level function like ReadRawAmount
		return false;
	if ( !JSVAL_IS_STRING(rval) )
		return false;
	JSString *jsStr = JSVAL_TO_STRING(rval);
	char * str = JS_GetStringBytes(jsStr);
	size_t len = JS_GetStringLength(jsStr);
	memcpy(buf, str, len);
	*amount = len;
*/
	#ifdef JS_THREADSAFE
		JS_EndRequest( cx );
	#endif

	return true;
}



inline JSBool BufferSetLength( JSContext *cx, JSObject *obj, size_t bufferLength ) {

	RT_CHECK_CALL( JS_SetReservedSlot(cx, obj, SLOT_BUFFER_LENGTH, INT_TO_JSVAL( bufferLength )) );
	return JS_TRUE;
}

inline JSBool BufferGetLength( JSContext *cx, JSObject *obj, size_t *bufferLength ) {

	jsval bufferLengthVal;
	RT_CHECK_CALL( JS_GetReservedSlot(cx, obj, SLOT_BUFFER_LENGTH, &bufferLengthVal) );
	*bufferLength = JSVAL_TO_INT(bufferLengthVal);
	return JS_TRUE;
}

inline JSBool BufferLengthAdd( JSContext *cx, JSObject *obj, size_t amount ) {
	
	size_t bufferLength;
	RT_CHECK_CALL( BufferGetLength(cx, obj, &bufferLength) );
	bufferLength += amount;
	RT_CHECK_CALL( BufferSetLength(cx, obj, bufferLength) );
	return JS_TRUE;
}

inline JSBool BufferRefillRequest( JSContext *cx, JSObject *obj, size_t missing ) { // missing = missing amount of data to complete the request

	jsval fctVal;
	JS_GetProperty(cx, obj, "onunderflow", &fctVal);
	if ( fctVal != JSVAL_VOID ) {

		RT_ASSERT_FUNCTION(fctVal);
		jsval retVal;
		RT_CHECK_CALL( CallFunction(cx, obj, fctVal, &retVal, 2, OBJECT_TO_JSVAL(obj), INT_TO_JSVAL(missing) ) );
	}

	return JS_TRUE;
}


BEGIN_CLASS( Buffer )

DEFINE_FINALIZE() {

	Queue *queue = (Queue*)JS_GetPrivate(cx, obj);
	if ( queue != NULL ) {
		
		while ( !QueueIsEmpty(queue) ) {
			
			JSString **pNewStr = (JSString**)QueueShift(queue);
			JS_RemoveRoot(cx, pNewStr);
			free(pNewStr);
		}
		QueueDestruct(queue);
		JS_SetPrivate(cx, obj, NULL);
	}
}


DEFINE_CONSTRUCTOR() {

	RT_ASSERT_CONSTRUCTING( _class );
	// prepare NativeInterface compatibility
	JsCntxt *cntxt = (JsCntxt*)malloc(sizeof(JsCntxt));
	cntxt->rt = JS_GetRuntime(cx); // beware: cx must exist during the life of this object !
	cntxt->obj = obj;
	SetNativeInterface(cx, obj, NI_READ_RESOURCE, (FunctionPointer)NativeInterfaceReadBuffer, cntxt);
	Queue *queue = QueueConstruct();
	RT_ASSERT_ALLOC(queue);
	JS_SetPrivate(cx, obj, queue);
	BufferSetLength(cx, obj, 0);
	return JS_TRUE;
}

DEFINE_FUNCTION( Clear ) {

	Queue *queue = (Queue*)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(queue);

	while ( !QueueIsEmpty(queue) ) {
		
		JSString **pNewStr = (JSString**)QueueShift(queue);
		JS_RemoveRoot(cx, pNewStr);
		free(pNewStr);
	}
	BufferSetLength(cx, obj, 0);
	return JS_TRUE;
}


DEFINE_FUNCTION( Write ) {

	RT_ASSERT_ARGC(1);
	RT_ASSERT_STRING(argv[0]);
	Queue *queue = (Queue*)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(queue);

	JSString *str = JSVAL_TO_STRING(argv[0]);
	size_t strLen = JS_GetStringLength(str);

	size_t amount;
	if ( argc >= 2 ) {
		RT_JSVAL_TO_INT32( argv[1], amount );
		if ( amount > strLen )
			amount = strLen;
	} else
		amount = strLen;

	JSString **pNewStr = (JSString**)malloc(sizeof(JSString*));
	*pNewStr = JS_NewDependentString(cx, str, 0, amount);
	RT_ASSERT( pNewStr != NULL, "Unable to create a NewDependentString." );
	RT_CHECK_CALL( JS_AddRoot(cx, pNewStr) );
	QueuePush( queue, pNewStr );
	RT_CHECK_CALL( BufferLengthAdd(cx, obj, amount) );
	return JS_TRUE;
}


inline JSBool ReadOneChunk( JSContext *cx, JSObject *obj, jsval *rval ) {

  	size_t bufferLength;
	RT_CHECK_CALL( BufferGetLength(cx, obj, &bufferLength) );

	if ( bufferLength == 0 ) { // if buffer is empty, try to refill it.

		RT_CHECK_CALL( BufferRefillRequest(cx, obj, 0) );
		RT_CHECK_CALL( BufferGetLength(cx, obj, &bufferLength) ); // read it again because it may have changed
		if ( bufferLength == 0 ) { // the data are definitively exhausted
			
			*rval = JS_GetEmptyStringValue(cx);
			return JS_TRUE;
		}
	}
	Queue *queue = (Queue*)JS_GetPrivate(cx, obj);
	JSString **pNewStr = (JSString**)QueueShift(queue);
	bufferLength -= JS_GetStringLength(*pNewStr);
	RT_CHECK_CALL( BufferSetLength(cx, obj, bufferLength) ); // update buffer size
	RT_CHECK_CALL( JS_RemoveRoot(cx, pNewStr) ); // removeRoot
	*rval = STRING_TO_JSVAL(*pNewStr); // result (Rooted)
	return JS_TRUE;
}


inline JSBool ReadRawAmount( JSContext *cx, JSObject *obj, size_t *amount, char *str ) { // this form allows one to read into a static buffer
	
	if ( amount == 0 ) // optimization
		return JS_TRUE;

	size_t bufferLength;
	RT_CHECK_CALL( BufferGetLength(cx, obj, &bufferLength) );

	if ( bufferLength < *amount || bufferLength == 0 ) { // more that the available data is required, then try to refill the buffer

		RT_CHECK_CALL( BufferRefillRequest(cx, obj, *amount - bufferLength) );
		RT_CHECK_CALL( BufferGetLength(cx, obj, &bufferLength) ); // read it again because it may have changed
		if ( *amount > bufferLength ) // we definitively cannot read the required amount of data, then read the whole buffer.
			*amount = bufferLength;
	}

	char *ptr = str;
	size_t remainToRead = *amount;

	Queue *queue = (Queue*)JS_GetPrivate(cx, obj);

	while ( remainToRead > 0 ) { // while there is something to read,

		JSString **pNewStr = (JSString**)QueueGetData(QueueBegin(queue)); // just get the data, do not shift the queue
		char *chunk = JS_GetStringBytes(*pNewStr);
		size_t chunkLen = JS_GetStringLength(*pNewStr);

		if ( chunkLen <= remainToRead ) {

			memcpy(ptr, chunk, chunkLen);
			ptr += chunkLen;
			remainToRead -= chunkLen; // adjust remaining required data length
			RT_CHECK_CALL( JS_RemoveRoot(cx, pNewStr) );
			QueueShift(queue);
		} else { // chunkLen > remain: this is the last chunk we have to manage. we only get a part of it chunk and we 'unread' the remaining.
			
			memcpy(ptr, chunk, remainToRead);
			if ( true ) { // (TBD) compute the condition to make 'conservative chunk'

				*pNewStr = JS_NewDependentString(cx, *pNewStr, remainToRead, chunkLen - remainToRead);
			} else {

				*pNewStr = JS_NewStringCopyN(cx, chunk + remainToRead, chunkLen - remainToRead); // now, we have to store the unwanted data of this chunk. note that pNewStr is already rooted
				RT_ASSERT_ALLOC( *pNewStr );
			}
			remainToRead = 0; // adjust remaining required data length
		}
	}
	bufferLength -= *amount;
	RT_CHECK_CALL( BufferSetLength(cx, obj, bufferLength) ); // update buffer size
	return JS_TRUE;
}


inline JSBool ReadAmount( JSContext *cx, JSObject *obj, size_t amount, jsval *rval ) {
	
	if ( amount == 0 ) { // optimization

		*rval = JS_GetEmptyStringValue(cx);
		return JS_TRUE;
	}
	char *str = (char*)JS_malloc(cx, amount + 1);
	RT_ASSERT_ALLOC(str);
	str[amount] = '\0'; // (TBD) explain this
	RT_CHECK_CALL( ReadRawAmount(cx, obj, &amount, str) );
	// (TBD) if amount after calling ReadRawAmount is smaller than before, realloc the buffer 
	*rval = STRING_TO_JSVAL(JS_NewString(cx, str, amount));
	return JS_TRUE;
}


inline JSBool UnReadRawChunk( JSContext *cx, JSObject *obj, char *data, size_t length ) {

	Queue *queue = (Queue*)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE( queue );
	JSString **pNewStr = (JSString**)malloc(sizeof(JSString*));
	RT_ASSERT_ALLOC( pNewStr );
	*pNewStr = JS_NewStringCopyN(cx, data, length);
	RT_ASSERT_ALLOC( *pNewStr );
	RT_CHECK_CALL( JS_AddRoot(cx, pNewStr) );
	QueueUnshift( queue, pNewStr );
	RT_CHECK_CALL( BufferLengthAdd(cx, obj, JS_GetStringLength(*pNewStr)) );
	return JS_TRUE;
}


inline JSBool UnReadChunk( JSContext *cx, JSObject *obj, JSString *chunk ) {

	Queue *queue = (Queue*)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE( queue );
	JSString **pNewStr = (JSString**)malloc(sizeof(JSString*));
	RT_ASSERT_ALLOC( pNewStr );
	*pNewStr = chunk; // no need to use JS_NewDependentString (see js_NewDependentString in jsstr.c)
	RT_CHECK_CALL( JS_AddRoot(cx, pNewStr) );
	QueueUnshift( queue, pNewStr );
	RT_CHECK_CALL( BufferLengthAdd(cx, obj, JS_GetStringLength(*pNewStr)) );
	return JS_TRUE;
}


DEFINE_FUNCTION( ReadReal ) {

	RT_ASSERT_ARGC(1);
	unsigned char data[16];
	size_t size = JSVAL_TO_INT( argv[0] );
	size_t amount = size;
	RT_CHECK_CALL( ReadRawAmount(cx, obj, &amount, (char*)data) );
	if ( amount < size ) { // not enough data to complete the requested operation, then unread the few data we read

		RT_CHECK_CALL( UnReadRawChunk(cx, obj, (char*)data, amount) );
		*rval = JSVAL_VOID;
		return JS_TRUE;
	}

	switch (size) {
		case sizeof(float):
			RT_CHECK_CALL( JS_NewNumberValue(cx, *((float*)data), rval) );
			break;
		case sizeof(double):
			RT_CHECK_CALL( JS_NewNumberValue(cx, *((double*)data), rval) );
			break;
		default:
			*rval = JSVAL_VOID;
	}
	return JS_TRUE;

}


DEFINE_FUNCTION( ReadInt ) {
		
	unsigned char data[8]; // int 64
	
	size_t size = argc >= 1 ? JSVAL_TO_INT(argv[0]) : sizeof(int);
	size_t amount = size;
	RT_CHECK_CALL( ReadRawAmount(cx, obj, &amount, (char*)data) );

	if ( amount < size ) { // not enough data to complete the requested operation, then unread the few data we read

		RT_CHECK_CALL( UnReadRawChunk(cx, obj, (char*)data, amount) );
		*rval = JSVAL_VOID;
		return JS_TRUE;
	}

	switch (size) {
		case sizeof(signed char):
			*rval = INT_TO_JSVAL( *(signed char*)data );
			break;
		case sizeof(signed short):
			*rval = INT_TO_JSVAL( *(signed short*)data );
			break;
		case sizeof(signed long): {
			unsigned long val = *(signed long*)data;
			if ( INT_FITS_IN_JSVAL( val ) )
				*rval = INT_TO_JSVAL( val );
			else
				RT_CHECK_CALL( JS_NewNumberValue(cx, val, rval) );
			break;
		}
		case sizeof(signed __int64): {
			unsigned long val = *(signed __int64*)data;
			if ( INT_FITS_IN_JSVAL( val ) )
				*rval = INT_TO_JSVAL( val );
			else
				RT_CHECK_CALL( JS_NewNumberValue(cx, val, rval) );
			break;
		}
		default:
			*rval = JSVAL_VOID;
	}
	return JS_TRUE;
}


DEFINE_FUNCTION( ReadStringZ ) {
	
	// get a chunk
	// check if it contains '\0'
	// else store it and get a new chunk...
	// if it contains '\0',
	// create a buffer of the right size,
	// strcat all stored chunks, and unread the remainder if any
	// else if no '\0' and the buffer is empty, unread all stored chunks

	return JS_TRUE;
}

DEFINE_FUNCTION( ReadUInt ) {
		
	unsigned char data[8]; // int 64
	
	size_t size = argc >= 1 ? JSVAL_TO_INT(argv[0]) : sizeof(unsigned int);
	size_t amount = size;
	RT_CHECK_CALL( ReadRawAmount(cx, obj, &amount, (char*)data) );

	if ( amount < size ) { // not enough data to complete the requested operation, then unread the few data we read

		RT_CHECK_CALL( UnReadRawChunk(cx, obj, (char*)data, amount) );
		*rval = JSVAL_VOID;
		return JS_TRUE;
	}

	switch (size) {
		case sizeof(unsigned char):
			*rval = INT_TO_JSVAL( *(unsigned char*)data );
			break;
		case sizeof(unsigned short):
			*rval = INT_TO_JSVAL( *(unsigned short*)data );
			break;
		case sizeof(unsigned long): {
			unsigned long val = *(unsigned long*)data;
			if ( INT_FITS_IN_JSVAL( val ) )
				*rval = INT_TO_JSVAL( val );
			else
				RT_CHECK_CALL( JS_NewNumberValue(cx, val, rval) );
			break;
		}
		case sizeof(unsigned __int64): {
			unsigned long val = *((unsigned __int64*)data);
			if ( INT_FITS_IN_JSVAL( val ) )
				*rval = INT_TO_JSVAL( val );
			else
				RT_CHECK_CALL( JS_NewNumberValue(cx, val, rval) );
			break;
		}
		default:
			*rval = JSVAL_VOID;
	}
	return JS_TRUE;
}


DEFINE_FUNCTION( Read ) { // Read( [amount | <undefined> ] )

	RT_ASSERT_RESOURCE( JS_GetPrivate(cx, obj) ); // first, ensure that the object is valid
	if ( argc >= 1 && argv[0] == JSVAL_VOID ) { // read the next chunk (of an unknown length) (read something as fast as possible)
		
		RT_CHECK_CALL( ReadOneChunk(cx, obj, rval) );
		return JS_TRUE;
	}
	size_t amount;
	if ( argc == 0 )
		RT_CHECK_CALL( BufferGetLength(cx, obj, &amount) ) // no arguments then read the whole buffer
	else
		RT_JSVAL_TO_INT32( argv[0], amount );
	RT_ASSERT( amount >= 0, "Invalid amount" );
	RT_CHECK_CALL( ReadAmount(cx, obj, amount, rval) );
	return JS_TRUE;
}


DEFINE_FUNCTION( Unread ) {

	RT_ASSERT_ARGC( 1 );
	RT_ASSERT_ARGC_MAX( 1 ); // discourages one to use Unread like Write
	RT_ASSERT_STRING( argv[0] );
	RT_CHECK_CALL( UnReadChunk(cx, obj, JSVAL_TO_STRING(argv[0])) ); // no need to use JS_NewDependentString (see js_NewDependentString in jsstr.c)
	*rval = argv[0];
	return JS_TRUE;
}


DEFINE_PROPERTY( length ) {

	JS_GetReservedSlot(cx, obj, SLOT_BUFFER_LENGTH, vp );
	return JS_TRUE;
}


CONFIGURE_CLASS

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION(Clear)
		FUNCTION(Write)
		FUNCTION(Unread)
		FUNCTION(Read)
		FUNCTION(ReadInt)
		FUNCTION(ReadUInt)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ(length)
	END_PROPERTY_SPEC

	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1)

END_CLASS
