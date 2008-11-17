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

#include "../host/host.h"

struct Private {

	JLMutexHandler mutex;
	JLThreadHandler threadHandle;
	bool end;

	Serialized serializedCode;

	JLSemaphoreHandler requestSem;
	jl::Queue requestList;
	size_t pendingRequestCount;

	JLSemaphoreHandler responseSem;
	jl::Queue responseList;
	size_t pendingResponseCount;

	jl::Queue exceptionList;
};


/**doc
$CLASS_HEADER
**/
BEGIN_CLASS( Task )


//void TaskCleanupLists(Private *pv) {
//}

DEFINE_FINALIZE() {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	if ( pv ) {

		JLAcquireMutex(pv->mutex);
		pv->end = true;
		JLReleaseMutex(pv->mutex);

		JLReleaseSemaphore(pv->requestSem); // unlock the thread an let it manage the "end"

		JLWaitThread(pv->threadHandle); // wait for the end of the thread
		JLFreeThread(&pv->threadHandle);

		JLFreeSemaphore(&pv->requestSem);
		JLFreeSemaphore(&pv->responseSem);

		JLFreeMutex(&pv->mutex);

		// cleanup lists
		while ( !QueueIsEmpty(&pv->responseList) ) {

			Serialized ser = (Serialized)QueueShift(&pv->responseList);
			SerializerFree(&ser);
		}
		pv->pendingResponseCount = 0;

		while ( !QueueIsEmpty(&pv->requestList) ) {

			Serialized ser = (Serialized)QueueShift(&pv->requestList);
			SerializerFree(&ser);
		}
		pv->pendingRequestCount = 0;

		while ( !QueueIsEmpty(&pv->exceptionList) ) {

			Serialized ser = (Serialized)QueueShift(&pv->exceptionList);
			SerializerFree(&ser);
		}

		JS_free(cx, pv);
	}
}


JSBool Task(JSContext *cx, Private *pv) {

	jsval code, rval;

	J_CHK( UnserializeJsval(cx, &pv->serializedCode, &code) ); // no need to mutex this because this is the only place that access pv->serializedCode
	SerializerFree(&pv->serializedCode);

	JSFunction *fun = JS_ValueToFunction(cx, code);
	JSObject *funObj = JS_GetFunctionObject(fun);
	JSObject *globalObj = JS_GetGlobalObject(cx);
	J_CHK( JS_SetParent(cx, funObj, globalObj) ); // re-scope the function

	size_t index = 0;

	for (;;) {

		JLAcquireSemaphore(pv->requestSem); // wait for a request


		for (;;) {

			JLAcquireMutex(pv->mutex);
			if ( pv->end ) {

				JLReleaseMutex(pv->mutex);
				goto end;
			}
			if ( QueueIsEmpty(&pv->requestList) ) {

				JLReleaseMutex(pv->mutex);
				break;
			}

			Serialized serializedRequest = (Serialized)QueueShift(&pv->requestList);
			pv->pendingRequestCount--;
			JLReleaseMutex(pv->mutex);
			jsval request;
			J_CHK( UnserializeJsval(cx, &serializedRequest, &request) );
			SerializerFree(&serializedRequest);

			jsval argv[] = { request, INT_TO_JSVAL(index++) }; // (TBD) root something ?
			JSBool status = JS_CallFunction(cx, globalObj, fun, COUNTOF(argv), argv, &rval);

			if ( !status ) {

				jsval ex;
				if ( JS_IsExceptionPending(cx) ) { // manageable error

					J_CHK( JS_GetPendingException(cx, &ex) );
					JSString *jsstr = JS_ValueToString(cx, ex); // transform the exception into a string
					ex = STRING_TO_JSVAL(jsstr);
				} else {

					ex = JSVAL_VOID; // unknown exception
				}

				Serialized serializedException;
				SerializerCreate(&serializedException);
				J_CHK( SerializeJsval(cx, &serializedException, &ex) );

				JLAcquireMutex(pv->mutex);
				QueuePush(&pv->exceptionList, serializedException);

				QueuePush(&pv->responseList, NULL); // signals an exception
				pv->pendingResponseCount++;
				JLReleaseMutex(pv->mutex);
	
				JS_ClearPendingException(cx);
			} else {

				Serialized serializedResponse;
				SerializerCreate(&serializedResponse);
				J_CHK( SerializeJsval(cx, &serializedResponse, &rval) ); // (TBD) need a better exception management

				JLAcquireMutex(pv->mutex);
				QueuePush(&pv->responseList, serializedResponse);
				pv->pendingResponseCount++;
				JLReleaseMutex(pv->mutex);
			}
			JLReleaseSemaphore(pv->responseSem); // signals a response
		}
	}
end:

	return JS_TRUE;
}


struct ErrorBuffer {
	char *buffer;
	size_t length;
	size_t maxLength;
};


int TaskStdErrHostOutput( void *privateData, const char *buffer, size_t length ) {

	ErrorBuffer *eb = (ErrorBuffer*)privateData;
	if ( eb->length + length > eb->maxLength ) {
	
		eb->maxLength = eb->length + length + 1024;
		eb->buffer = eb->buffer == NULL ? (char*)malloc(eb->maxLength) : (char*)realloc(eb->buffer, eb->maxLength);
	}
	memcpy(eb->buffer + eb->length, buffer, length);
	eb->length += length;
	return 0;
}


JLThreadFuncDecl ThreadProc( void *threadArg ) {

	JSContext *cx = CreateHost(-1, -1, 0);
	if ( cx == NULL )
		return -1;

	ErrorBuffer errorBuffer = { NULL, 0, 0 };

	JSBool status = InitHost(cx, _unsafeMode, NULL, TaskStdErrHostOutput, &errorBuffer);
	// (TBD) need a better error management when we cannot create the context
	if ( status == JS_TRUE ) {

		Private *pv = (Private*)threadArg;

		status = Task(cx, pv);

		if ( status != JS_TRUE ) { // fatal errors

			jsval ex;
			if ( JS_IsExceptionPending(cx) ) {

				J_CHK( JS_GetPendingException(cx, &ex) );
				JSString *jsstr = JS_ValueToString(cx, ex); // transform the exception into a string
				ex = STRING_TO_JSVAL(jsstr);
			} else {
				
				StringAndLengthToJsval(cx, &ex, errorBuffer.buffer, errorBuffer.length);
				//ex = JSVAL_VOID; // unknown exception
			}

			Serialized serializedException;
			SerializerCreate(&serializedException);
			SerializeJsval(cx, &serializedException, &ex);

			JLAcquireMutex(pv->mutex);
			pv->end = true;
			QueuePush(&pv->exceptionList, serializedException);

			JLReleaseMutex(pv->mutex);
			JLReleaseSemaphore(pv->responseSem);
		}
	}

	if ( errorBuffer.buffer != NULL )
		free(errorBuffer.buffer);

	DestroyHost(cx);
	return 0;
}


/**doc
 * $INAME( func [ , priority = 0 ] )
  $H arguments
   $ARG function func:
   $ARG integer priority:
  Creates a new Task object from the given function.
**/
DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();
	J_S_ASSERT_ARG_MIN(1);
	J_S_ASSERT_FUNCTION( J_ARG(1) );

	Private *pv = (Private*)JS_malloc(cx, sizeof(Private));
	J_S_ASSERT_ALLOC(pv);

	JLThreadPriorityType priority;
	if ( J_ARG_ISDEF(2) ) {

		int p;
		J_CHK( JsvalToInt(cx, J_ARG(2), &p) );
		switch ( p ) {
			case 0:
				priority = JL_THREAD_PRIORITY_NORMAL;
				break;
			case -1:
				priority = JL_THREAD_PRIORITY_LOW;
				break;
			case 1:
				priority = JL_THREAD_PRIORITY_HIGH;
				break;
			default:
				J_REPORT_ERROR("Invalid thread priority.");
		}
	}

	J_CHKB( JS_SetPrivate(cx, obj, pv), bad1 );

	pv->mutex = JLCreateMutex();
	pv->end = false;

	QueueInitialize(&pv->requestList);
	pv->requestSem = JLCreateSemaphore(0);
	pv->pendingRequestCount = 0;

	QueueInitialize(&pv->responseList);
	pv->responseSem = JLCreateSemaphore(0);
	pv->pendingResponseCount = 0;

	QueueInitialize(&pv->exceptionList);

	SerializerCreate(&pv->serializedCode);
	J_CHKB( SerializeJsval(cx, &pv->serializedCode, &J_ARG(1)), bad1 );

	pv->threadHandle = JLStartThread(ThreadProc, pv);

	J_S_ASSERT( JLThreadOk(pv->threadHandle), "Unable to create the thread." );

	return JS_TRUE;
bad1:
	JS_free(cx, pv);
	return JS_FALSE;
}



DEFINE_FUNCTION_FAST( Request ) {

	J_S_ASSERT_ARG_MIN(1);
	Private *pv = (Private*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(pv);

	Serialized serializedRequest;
	SerializerCreate(&serializedRequest);
	J_CHK( SerializeJsval(cx, &serializedRequest, &J_FARG(1)) );
	JLAcquireMutex(pv->mutex);
	QueuePush(&pv->requestList, serializedRequest);
	if ( pv->pendingRequestCount++ == 0 )
		JLReleaseSemaphore(pv->requestSem); // signals a request
	JLReleaseMutex(pv->mutex);
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Response ) {

	Private *pv = (Private*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(pv);

	bool hasNoResponse;
	JLAcquireMutex(pv->mutex);
	hasNoResponse = QueueIsEmpty(&pv->responseList);

	if ( QueueIsEmpty(&pv->responseList) && !QueueIsEmpty(&pv->exceptionList) ) {

		Serialized serializedException = (Serialized)QueueShift(&pv->exceptionList);
		JLReleaseMutex(pv->mutex);

		jsval exception;
		J_CHK( UnserializeJsval(cx, &serializedException, &exception) );
		JS_SetPendingException(cx, exception);
		return JS_FALSE;
	}

	if ( pv->end ) {

		JLReleaseMutex(pv->mutex);
		jsval exception = JSVAL_VOID; // throw a StopTask exception
		JS_SetPendingException(cx, exception);
		return JS_FALSE;
	}

	JLReleaseMutex(pv->mutex);

	if ( hasNoResponse )
		JLAcquireSemaphore(pv->responseSem); // wait for a response

	JLAcquireMutex(pv->mutex);

	if ( QueueIsEmpty(&pv->responseList) ) { // || !JLThreadIsActive( pv->threadHandle )

		JLReleaseMutex(pv->mutex);
		*J_FRVAL = JSVAL_VOID;
		return JS_TRUE;
	}

	Serialized serializedResponse = (Serialized)QueueShift(&pv->responseList);
	pv->pendingResponseCount--;

	if ( SerializerIsEmpty( &serializedResponse ) ) { // an exception is signaled

		Serialized serializedException = (Serialized)QueueShift(&pv->exceptionList);
		jsval exception;
		J_CHK( UnserializeJsval(cx, &serializedException, &exception) ); // (TBD) throw a TaskException ?
		JS_SetPendingException(cx, exception);
		JLReleaseMutex(pv->mutex);
		return JS_FALSE;
	} else {

		J_CHK( UnserializeJsval(cx, &serializedResponse, J_FRVAL) );
		SerializerFree(&serializedResponse);
	}
	JLReleaseMutex(pv->mutex);

	return JS_TRUE;
}


DEFINE_PROPERTY( pendingRequestCount ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(pv);
	JLAcquireMutex(pv->mutex); // (TBD) needed ?
	J_CHK( UIntToJsval(cx, pv->end ? 0 : pv->pendingRequestCount, vp) );
	JLReleaseMutex(pv->mutex);
	return JS_TRUE;
}


DEFINE_PROPERTY( pendingResponseCount ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(pv);
	JLAcquireMutex(pv->mutex); // (TBD) needed ?
	J_CHK( UIntToJsval(cx, pv->pendingResponseCount ? pv->pendingResponseCount : QueueIsEmpty(&pv->exceptionList) ? 0 : 1 , vp) );
	JLReleaseMutex(pv->mutex);
	return JS_TRUE;
}


CONFIGURE_CLASS

	HAS_PRIVATE

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST(Request)
		FUNCTION_FAST(Response)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ(pendingRequestCount)
		PROPERTY_READ(pendingResponseCount)
	END_PROPERTY_SPEC

	BEGIN_STATIC_FUNCTION_SPEC
	END_STATIC_FUNCTION_SPEC

END_CLASS
