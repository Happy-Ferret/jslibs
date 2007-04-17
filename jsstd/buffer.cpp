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

	jsval rval, argv[] = { INT_TO_JSVAL(*amount) };
	if ( JS_CallFunctionName(cx, obj, "Read", 1, argv, &rval) == JS_FALSE )
		return false;
	if ( !JSVAL_IS_STRING(rval) )
		return false;
	JSString *jsStr = JSVAL_TO_STRING(rval);
	char * str = JS_GetStringBytes(jsStr);
	size_t len = JS_GetStringLength(jsStr);
	memcpy(buf, str, len);
	*amount = len;

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

inline JSBool BufferRefillRequest( JSContext *cx, JSObject *obj, size_t *bufferLength  ) {

	jsval fctVal;
	JS_GetProperty(cx, obj, "onunderflow", &fctVal);
	if ( fctVal != JSVAL_VOID ) {

		RT_ASSERT_FUNCTION(fctVal);
		jsval retVal;
		RT_CHECK_CALL( CallFunction(cx, obj, fctVal, &retVal, 1, OBJECT_TO_JSVAL(obj)) );
	}
	BufferGetLength(cx, obj, bufferLength); // read it again because it may have changed
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


DEFINE_FUNCTION( Unread ) {

	RT_ASSERT_ARGC( 1 );
	RT_ASSERT_ARGC_MAX( 1 ); // discourages one to use Unread like Write
	RT_ASSERT_STRING( argv[0] );
	Queue *queue = (Queue*)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE( queue );
	JSString **pNewStr = (JSString**)malloc(sizeof(JSString*));
	RT_ASSERT_ALLOC( pNewStr );
	*pNewStr = JSVAL_TO_STRING(argv[0]); // no need to use JS_NewDependentString (see js_NewDependentString in jsstr.c)
	RT_CHECK_CALL( JS_AddRoot(cx, pNewStr) );
	QueueUnshift( queue, pNewStr );
	RT_CHECK_CALL( BufferLengthAdd(cx, obj, JS_GetStringLength(*pNewStr)) );
	*rval = argv[0];
	return JS_TRUE;
}


DEFINE_FUNCTION( Read ) { // Read( [amount | <undefined> ] )

	Queue *queue = (Queue*)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(queue);
	size_t bufferLength;
	RT_CHECK_CALL( BufferGetLength(cx, obj, &bufferLength) );
	size_t amount;

	if ( argc == 0 )
		amount = bufferLength; // read the whole buffer
	else if ( argv[0] == JSVAL_VOID ) { // read the next chunk (read something as fast as possible)

		if ( bufferLength == 0 ) {

			RT_CHECK_CALL( BufferRefillRequest(cx, obj, &bufferLength) );
			if ( bufferLength == 0 ) { // the data are definitively exhausted
				
				*rval = JS_GetEmptyStringValue(cx);
				return JS_TRUE;
			}
		}
		JSString **pNewStr = (JSString**)QueueShift(queue);
		bufferLength -= JS_GetStringLength(*pNewStr);
		RT_CHECK_CALL( BufferSetLength(cx, obj, bufferLength) ); // update buffer size
		RT_CHECK_CALL( JS_RemoveRoot(cx, pNewStr) ); // removeRoot
		*rval = STRING_TO_JSVAL(*pNewStr); // result (Rooted)
		return JS_TRUE;
	} else
		RT_JSVAL_TO_INT32( argv[0], amount );

	if ( amount <= 0 ) { // optimization
		
		*rval = JS_GetEmptyStringValue(cx);
		return JS_TRUE;
	}

	if ( bufferLength < amount || bufferLength == 0 ) {

		RT_CHECK_CALL( BufferRefillRequest(cx, obj, &bufferLength) );
		if ( amount > bufferLength )
			amount = bufferLength; // we definitively cannot read the required amount of data, then read the whole buffer.
		}

	// at this point, amound must contain the exact amount of data we will return
	char *str = (char*)JS_malloc(cx, amount +1);
	RT_ASSERT_ALLOC(str);
	str[amount] = '\0'; // (TBD) explain this
	char *ptr = str;
	size_t remainToRead = amount;

	while ( remainToRead > 0 ) {

		JSString **pNewStr = (JSString**)QueueGetData(QueueBegin(queue)); // just get the data, do not shift the queue
		char *chunk = JS_GetStringBytes(*pNewStr);
		size_t chunkLen = JS_GetStringLength(*pNewStr);

		if ( chunkLen <= remainToRead ) {

			memcpy(ptr, chunk, chunkLen);
			ptr += chunkLen;
			remainToRead -= chunkLen; // adjust remaining required data length
			RT_CHECK_CALL( JS_RemoveRoot(cx, pNewStr) );
			QueueShift(queue);
		} else { // chunkLen > remain ( this is the last chunk we have to manage )

			memcpy(ptr, chunk, remainToRead);
			*pNewStr = JS_NewStringCopyN(cx, chunk + remainToRead, chunkLen - remainToRead); // now, we have to store the unwanted data of this chunk. note that pNewStr is already rooted
			RT_ASSERT_ALLOC( *pNewStr );
			remainToRead = 0; // adjust remaining required data length
		}
	}
	*rval = STRING_TO_JSVAL(JS_NewString(cx, str, amount));
	bufferLength -= amount;
	RT_CHECK_CALL( BufferSetLength(cx, obj, bufferLength) ); // update buffer size
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
		FUNCTION(Write)
		FUNCTION(Unread)
		FUNCTION(Read)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ(length)
	END_PROPERTY_SPEC

	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1)

END_CLASS
