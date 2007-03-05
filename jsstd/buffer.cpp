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

/*
typedef struct BufferChunk {
	JSString *str;
	int length;
} BufferChunk;
*/

typedef struct JsCntxt {
	JSRuntime *rt;
	JSObject *obj;
} CxObj;


static bool NativeInterfaceReadBuffer( void *pv, unsigned char *buf, unsigned int *amount ) {

	JsCntxt *cntxt = (CxObj*)pv;
	JSContext *cx = NULL;
	JS_ContextIterator(cntxt->rt, &cx);
	JSObject *obj = cntxt->obj;
	RT_SAFE( if ( obj == NULL || cx == NULL ) return false );

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
	return true;
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


	JsCntxt *cntxt = (JsCntxt*)malloc(sizeof(JsCntxt));
	
	// (TBD) change this:
	cntxt->rt = JS_GetRuntime(cx); // beware: cx must exist during the life of this object !
	cntxt->obj = obj;

	SetNativeInterface(cx, obj, NI_READ_RESOURCE, (FunctionPointer)NativeInterfaceReadBuffer, cntxt);

	Queue *queue = QueueConstruct();
	RT_ASSERT_ALLOC(queue);
	JS_SetPrivate(cx, obj, queue);
	JS_SetReservedSlot(cx, obj, SLOT_BUFFER_LENGTH, JSVAL_ZERO );
	return JS_TRUE;
}


DEFINE_FUNCTION( Write ) {

	RT_ASSERT_ARGC(1);
	RT_ASSERT_STRING(argv[0]);
	Queue *queue = (Queue*)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(queue);

	JSString *str = JSVAL_TO_STRING(argv[0]);
	size_t amount;
	if ( argc >= 2 )
		RT_JSVAL_TO_INT32( argv[1], amount )
	else
		amount = JS_GetStringLength(str);

	JSString **pNewStr = (JSString**)malloc(sizeof(JSString*));
	*pNewStr = JS_NewDependentString(cx, str, 0, amount);
	RT_ASSERT( pNewStr != NULL, "Unable to create a NewDependentString." );
	RT_CHECK_CALL( JS_AddRoot(cx, pNewStr) );
	QueuePush( queue, pNewStr );

	jsval bufferLengthVal;
	JS_GetReservedSlot(cx, obj, SLOT_BUFFER_LENGTH, &bufferLengthVal );
	bufferLengthVal = INT_TO_JSVAL( JSVAL_TO_INT(bufferLengthVal) + amount );
	JS_SetReservedSlot(cx, obj, SLOT_BUFFER_LENGTH, bufferLengthVal );
	return JS_TRUE;
}

//JSBool BufferRead( JSContext *cx, JSObject *obj, size_t amount, )

DEFINE_FUNCTION( Read ) {

	Queue *queue = (Queue*)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(queue);
	jsval bufferLengthVal;
	JS_GetReservedSlot(cx, obj, SLOT_BUFFER_LENGTH, &bufferLengthVal );

	if ( bufferLengthVal == JSVAL_ZERO ) {

		jsval fctVal;
		JS_GetProperty(cx, obj, "onunderflow", &fctVal);
		if ( fctVal != JSVAL_VOID ) {

			RT_ASSERT_FUNCTION(fctVal);
			jsval retVal;
			RT_CHECK_CALL( CallFunction(cx, obj, fctVal, &retVal, 1, OBJECT_TO_JSVAL(obj)) );
		}
		JS_GetReservedSlot(cx, obj, SLOT_BUFFER_LENGTH, &bufferLengthVal );
	}

	int bufferLength = JSVAL_TO_INT(bufferLengthVal);

	if ( argc == 0 ) {

		char *str = (char*)JS_malloc(cx, bufferLength +1);
		RT_ASSERT_ALLOC(str);
		str[bufferLength] = '\0';
		char *tmp = str;

		while ( !QueueIsEmpty(queue) ) {

			JSString **pNewStr = (JSString**)QueueShift(queue);
			RT_CHECK_CALL( JS_RemoveRoot(cx, pNewStr) );

			char *chunk = JS_GetStringBytes(*pNewStr);
			size_t chunkLen = JS_GetStringLength(*pNewStr);
			memcpy(tmp, chunk, chunkLen);
			tmp += chunkLen;
		}

		*rval = STRING_TO_JSVAL(JS_NewString(cx, str, bufferLength));
		bufferLength = 0;
	} else {

		size_t amount;
		RT_JSVAL_TO_INT32( argv[0], amount );
		RT_ASSERT( amount > 0, "The amount of data to read must be >= 0." );
		if ( amount > bufferLength ) // adjust the amount we will read
			amount = bufferLength;
		char *str = (char*)JS_malloc(cx, amount +1);
		RT_ASSERT_ALLOC(str);
		str[amount] = '\0';
		char *ptr = str;
		size_t remain = amount;

		while ( remain > 0 ) {

			JSString **pNewStr = (JSString**)QueueGetData(QueueBegin(queue)); // just get the data, do not shift the queue
			char *chunk = JS_GetStringBytes(*pNewStr);
			size_t chunkLen = JS_GetStringLength(*pNewStr);

			if ( chunkLen <= remain ) {

				memcpy(ptr, chunk, chunkLen);
				ptr += chunkLen;
				remain -= chunkLen; // adjust remaining required data length
				RT_CHECK_CALL( JS_RemoveRoot(cx, pNewStr) );
				QueueShift(queue);
			} else { // chunkLen > remain ( this is the last chunk we have to manage )

				memcpy(ptr, chunk, remain);
				*pNewStr = JS_NewStringCopyN(cx, chunk + remain, chunkLen - remain); // now, we have to store the unwanted data of this chunk. note that pNewStr is already rooted
				RT_ASSERT_ALLOC( *pNewStr );
				remain = 0; // adjust remaining required data length
			}
		}

		*rval = STRING_TO_JSVAL(JS_NewString(cx, str, amount));
		bufferLength -= amount;
	}

	bufferLengthVal = INT_TO_JSVAL( bufferLength );
	JS_SetReservedSlot(cx, obj, SLOT_BUFFER_LENGTH, bufferLengthVal );
	return JS_TRUE;
}

DEFINE_PROPERTY( length ) {

	JS_GetReservedSlot(cx, obj, SLOT_BUFFER_LENGTH, vp );
	return JS_TRUE;
}


CONFIGURE_CLASS

	HAS_CONSTRUCTOR
	HAS_FINALIZE

//DEFINE_FUNCTION( Call ) {
//	return JS_TRUE;
//}

//DEFINE_PROPERTY( prop ) {
//	return JS_TRUE;
//}

//DEFINE_FUNCTION( Func ) {
//	return JS_TRUE;
//}

	BEGIN_FUNCTION_SPEC
		FUNCTION(Write)
		FUNCTION(Read)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
	PROPERTY_READ(length)
	END_PROPERTY_SPEC

	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1)

END_CLASS
