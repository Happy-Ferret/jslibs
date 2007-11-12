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


/*
// used by NativeInterface 
typedef struct {
	JSRuntime *rt;
	JSObject *obj;
} JsCntxt;

static bool NativeInterfaceReadBuffer( void *pv, unsigned char *buf, unsigned int *amount ) {

	RT_SAFE( if ( pv == NULL ) return false );
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

	#ifdef JS_THREADSAFE
		JS_EndRequest( cx );
	#endif
	return true;
}
*/

inline JSBool BufferLengthSet( JSContext *cx, JSObject *obj, size_t bufferLength ) {

	RT_CHECK_CALL( JS_SetReservedSlot(cx, obj, SLOT_BUFFER_LENGTH, INT_TO_JSVAL( bufferLength )) );
	return JS_TRUE;
}

inline JSBool BufferLengthGet( JSContext *cx, JSObject *obj, size_t *bufferLength ) {

	jsval bufferLengthVal;
	RT_CHECK_CALL( JS_GetReservedSlot(cx, obj, SLOT_BUFFER_LENGTH, &bufferLengthVal) );
	*bufferLength = JSVAL_TO_INT(bufferLengthVal);
	return JS_TRUE;
}

inline JSBool BufferLengthAdd( JSContext *cx, JSObject *obj, size_t amount ) {
	
	size_t bufferLength;
	RT_CHECK_CALL( BufferLengthGet(cx, obj, &bufferLength) );
	bufferLength += amount;
	RT_CHECK_CALL( BufferLengthSet(cx, obj, bufferLength) );
	return JS_TRUE;
}

inline JSBool BufferRefillRequest( JSContext *cx, JSObject *obj, size_t missing ) { // missing = missing amount of data to complete the request

	jsval fctVal;
	RT_CHECK_CALL( JS_GetProperty(cx, obj, "onunderflow", &fctVal) );
	if ( fctVal != JSVAL_VOID )
		RT_CHECK_CALL( CallFunction(cx, obj, fctVal, NULL, 2, OBJECT_TO_JSVAL(obj), INT_TO_JSVAL(missing) ) );
	return JS_TRUE;
}

JSBool FindInBuffer( JSContext *cx, JSObject *obj, char *needle, int needleLength, int *foundAt ) {

	// (TBD) optimise this function for needleLength == 1 (eg. '\0' in a string)
	Queue *queue = (Queue*)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE( queue );
	
	int pos = 0;
	char *buf = (char*)malloc(needleLength); // the ring buffer

	int chunkLength;
	char *chunk;
	int i, j;

	for ( QueueCell *it = queue->begin; it; it = it->next ) {

		JSString **pNewStr = (JSString**)QueueGetData(it);
		chunkLength = JS_GetStringLength(*pNewStr);
		chunk = JS_GetStringBytes(*pNewStr);

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
	free(buf);
	return JS_TRUE;
}


JSBool AddBuffer( JSContext *cx, JSObject *destBuffer, JSObject *srcBuffer ) {

	RT_ASSERT_CLASS( destBuffer, &classBuffer );
	Queue *destQueue = (Queue*)JS_GetPrivate(cx, destBuffer);
	RT_ASSERT_RESOURCE( destQueue );

	RT_ASSERT_CLASS( srcBuffer, &classBuffer );
	Queue *srcQueue = (Queue*)JS_GetPrivate(cx, srcBuffer);
	RT_ASSERT_RESOURCE( srcQueue );

	for ( QueueCell *it = QueueBegin(srcQueue); it; it = QueueNext(it) ) {
		
		JSString **pNewStr = (JSString**)malloc(sizeof(JSString*));
		*pNewStr = *(JSString **)QueueGetData(it);
		RT_CHECK_CALL( JS_AddRoot(cx, pNewStr) );
		QueuePush( destQueue, pNewStr );
	}
	size_t bufLength;
	RT_CHECK_CALL( BufferLengthGet(cx, srcBuffer, &bufLength) );
	RT_CHECK_CALL( BufferLengthAdd(cx, destBuffer, bufLength) );
	return JS_TRUE;
}


JSBool WriteRawData( JSContext *cx, JSObject *obj, size_t amount, char *str ) {

	Queue *queue = (Queue*)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(queue);
	JSString **pNewStr = (JSString**)malloc(sizeof(JSString*));
	*pNewStr = JS_NewStringCopyN(cx, str, amount);
	RT_ASSERT( pNewStr != NULL, "Unable to create a NewDependentString." );
	RT_CHECK_CALL( JS_AddRoot(cx, pNewStr) );
	QueuePush( queue, pNewStr );
	RT_CHECK_CALL( BufferLengthAdd(cx, obj, amount) );
	return JS_TRUE;
}


JSBool ReadOneChunk( JSContext *cx, JSObject *obj, jsval *rval ) {

  	size_t bufferLength;
	RT_CHECK_CALL( BufferLengthGet(cx, obj, &bufferLength) );

	if ( bufferLength == 0 ) { // if buffer is empty, try to refill it.

		RT_CHECK_CALL( BufferRefillRequest(cx, obj, 0) );
		RT_CHECK_CALL( BufferLengthGet(cx, obj, &bufferLength) ); // read it again because it may have changed
		if ( bufferLength == 0 ) { // the data are definitively exhausted
			
			*rval = JS_GetEmptyStringValue(cx);
			return JS_TRUE;
		}
	}
	Queue *queue = (Queue*)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE( queue );
	JSString **pNewStr = (JSString**)QueueShift(queue);
	bufferLength -= JS_GetStringLength(*pNewStr);
	RT_CHECK_CALL( BufferLengthSet(cx, obj, bufferLength) ); // update buffer size
	RT_CHECK_CALL( JS_RemoveRoot(cx, pNewStr) ); // removeRoot
	*rval = STRING_TO_JSVAL(*pNewStr); // result (Rooted)
	return JS_TRUE;
}


JSBool ReadRawAmount( JSContext *cx, JSObject *obj, size_t *amount, char *str ) { // this form allows one to read into a static buffer
	
	if ( amount == 0 ) // optimization
		return JS_TRUE;

	size_t bufferLength;
	RT_CHECK_CALL( BufferLengthGet(cx, obj, &bufferLength) );

	if ( bufferLength < *amount || bufferLength == 0 ) { // more that the available data is required, then try to refill the buffer

		size_t bufferLengthBeforeRefillRequest;
		do {

			bufferLengthBeforeRefillRequest = bufferLength;
			RT_CHECK_CALL( BufferRefillRequest(cx, obj, *amount - bufferLength) );
			RT_CHECK_CALL( BufferLengthGet(cx, obj, &bufferLength) ); // read it again because it may have changed
		} while( *amount > bufferLength && bufferLength != bufferLengthBeforeRefillRequest ); // if bufferLength == bufferLengthBeforeRefillRequest nothing has been added in the buffer

		if ( *amount > bufferLength ) // we definitively cannot read the required amount of data, then read the whole buffer.
			*amount = bufferLength;
	}

	char *ptr = str;
	size_t remainToRead = *amount;

	Queue *queue = (Queue*)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE( queue );

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
	RT_CHECK_CALL( BufferLengthSet(cx, obj, bufferLength) ); // update buffer size
	return JS_TRUE;
}


JSBool ReadAmount( JSContext *cx, JSObject *obj, size_t amount, jsval *rval ) {
	
	if ( amount == 0 ) { // optimization

		*rval = JS_GetEmptyStringValue(cx);
		return JS_TRUE;
	}
	char *str = (char*)JS_malloc(cx, amount + 1);
	RT_ASSERT_ALLOC(str);
	str[amount] = '\0'; // (TBD) explain this

	int requestedAmount = amount;
	RT_CHECK_CALL( ReadRawAmount(cx, obj, &amount, str) );
	// (TBD) if amount after calling ReadRawAmount is smaller than before, realloc the buffer 

	if ( MaybeRealloc( requestedAmount, amount ) ) {

		str = (char*)JS_realloc(cx, str, amount + 1);
		RT_ASSERT_ALLOC(str);
	}
	*rval = STRING_TO_JSVAL(JS_NewString(cx, str, amount));
	return JS_TRUE;
}


JSBool UnReadRawChunk( JSContext *cx, JSObject *obj, char *data, size_t length ) {

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


JSBool UnReadChunk( JSContext *cx, JSObject *obj, JSString *chunk ) {

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



BEGIN_CLASS( Buffer )

DEFINE_FINALIZE() {

	Queue *queue = (Queue*)JS_GetPrivate(cx, obj);
	if ( queue != NULL ) {

/*
// remove NativeInterface 
// unable to call GetNativeInterface in Finalize !!!
//		JsCntxt *cntxt;
//		FunctionPointer fp;
//		GetNativeInterface(cx, obj, NI_READ_RESOURCE, &fp, (void**)&cntxt);
//		free(cntxt);
//		RemoveNativeInterface(cx, obj, NI_READ_RESOURCE);
*/

//		jsval tmp;
//		JS_GetReservedSlot(cx, obj, 0, &tmp );

		while ( !QueueIsEmpty(queue) ) {
			
			JSString **pNewStr = (JSString**)QueueShift(queue);
			JS_RemoveRoot(cx, pNewStr);
			free(pNewStr);
		}
		QueueDestruct(queue);
		JS_SetPrivate(cx, obj, NULL); // optional ?
	}
}


DEFINE_CONSTRUCTOR() {

	RT_ASSERT_CONSTRUCTING( _class );
/*
	// prepare NativeInterface compatibility
	JsCntxt *cntxt = (JsCntxt*)malloc(sizeof(JsCntxt)); // (TBD) fix this memory leak
	cntxt->rt = JS_GetRuntime(cx); // beware: cx must exist during the life of this object !
	cntxt->obj = obj;
	SetNativeInterface(cx, obj, NI_READ_RESOURCE, (FunctionPointer)NativeInterfaceReadBuffer, cntxt);
*/
	Queue *queue = QueueConstruct();
	RT_ASSERT_ALLOC(queue);
	JS_SetPrivate(cx, obj, queue);
	BufferLengthSet(cx, obj, 0);

	if ( argc >= 1 ) {
		
		if ( JSVAL_IS_STRING( argv[0] ) ) {

			JSString **pNewStr = (JSString**)malloc(sizeof(JSString*));
			*pNewStr = JSVAL_TO_STRING( argv[0] );
			RT_CHECK_CALL( JS_AddRoot(cx, pNewStr) );
			QueuePush( queue, pNewStr );
			RT_CHECK_CALL( BufferLengthAdd(cx, obj, JS_GetStringLength(*pNewStr)) );
		} else if ( JSVAL_IS_OBJECT( argv[0] ) ) {

			RT_CHECK_CALL( AddBuffer(cx, obj, JSVAL_TO_OBJECT( argv[0] )) );
		} else
			REPORT_ERROR( RT_ERROR_INVALID_ARGUMENT );
	}

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
	BufferLengthSet(cx, obj, 0);
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


DEFINE_FUNCTION( Match ) {
	
	RT_ASSERT_ARGC( 1 );
	RT_ASSERT_RESOURCE( JS_GetPrivate(cx, obj) ); // first, ensure that the object is valid
	RT_ASSERT_STRING( argv[0] );
		
	char *str;
	int len;
	RT_JSVAL_TO_STRING_AND_LENGTH( argv[0], str, len );

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

	RT_ASSERT_RESOURCE( JS_GetPrivate(cx, obj) ); // first, ensure that the object is valid
	if ( argc >= 1 && argv[0] == JSVAL_VOID ) { // read the next chunk (of an unknown length) (read something as fast as possible)
		
		RT_CHECK_CALL( ReadOneChunk(cx, obj, rval) );
		return JS_TRUE;
	}
	size_t amount;
	if ( argc == 0 )
		RT_CHECK_CALL( BufferLengthGet(cx, obj, &amount) ); // no arguments then read the whole buffer
	else
		RT_JSVAL_TO_INT32( argv[0], amount );
	RT_ASSERT( amount >= 0, "Invalid amount" );
	RT_CHECK_CALL( ReadAmount(cx, obj, amount, rval) );
	return JS_TRUE;
}


DEFINE_FUNCTION( Skip ) { // Skip( amount )

	RT_ASSERT_ARGC( 1 );
	RT_ASSERT_RESOURCE( JS_GetPrivate(cx, obj) ); // first, ensure that the object is valid
	size_t amount;
	RT_JSVAL_TO_INT32( argv[0], amount );
	RT_ASSERT( amount >= 0, "Invalid amount" );
	RT_CHECK_CALL( ReadAmount(cx, obj, amount, rval) ); // (TBD) optimization: skip without reading the data.
	*rval = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION( ReadUntil ) {

	RT_ASSERT_ARGC( 1 );
	char *boundary;
	int boundaryLength;
	RT_JSVAL_TO_STRING_AND_LENGTH( argv[0], boundary, boundaryLength );
	int found;
	RT_CHECK_CALL( FindInBuffer(cx, obj, boundary, boundaryLength, &found) );
	if ( found != -1 ) {

		ReadAmount(cx, obj, found, rval);
		jsval tmp;
		RT_CHECK_CALL( ReadAmount(cx, obj, boundaryLength, &tmp) ); // (TBD) optimization: skip without reading the data.
	}
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

	RT_CHECK_CALL( JS_GetReservedSlot(cx, obj, SLOT_BUFFER_LENGTH, vp ) );
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
		FUNCTION(ReadUntil)
		FUNCTION(Match)
		FUNCTION(Skip)
		FUNCTION_ALIAS(toString, Read) // ised when the buffer has to be transformed into a string
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ(length)
	END_PROPERTY_SPEC

	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1)

END_CLASS
