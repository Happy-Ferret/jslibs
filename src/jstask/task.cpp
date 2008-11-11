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

#include <jsxdrapi.h>

#include "../host/host.h"

typedef JSXDRState* Serialized;

void SerializerInit( Serialized *xdr ) {
	
	*xdr = NULL;
}

void SerializerFree( Serialized *xdr ) {

	if ( *xdr != NULL ) {

		JS_XDRMemSetData(*xdr, NULL, 0);
		JS_XDRDestroy(*xdr);
		*xdr = NULL;
	}
}

bool SerializerEmpty( const Serialized *xdr ) {

	return *xdr == NULL;
}

JSBool SerializeJsval( JSContext *cx, Serialized *xdr, jsval *val ) {

	if ( *xdr != NULL )
		SerializerFree(xdr);
	*xdr = JS_XDRNewMem(cx, JSXDR_ENCODE);
	J_CHK( JS_XDRValue(*xdr, val) );
	return JS_TRUE;
}

JSBool UnserializeJsval( JSContext *cx, const Serialized *xdr, jsval *rval ) {

	JSXDRState *xdrDecoder = JS_XDRNewMem(cx, JSXDR_DECODE);
	uint32 length;
	void *data = JS_XDRMemGetData(*xdr, &length);
	JS_XDRMemSetData(xdrDecoder, data, length);
	J_CHK( JS_XDRValue(xdrDecoder, rval) );
	JS_XDRMemSetData(xdrDecoder, NULL, 0);
	JS_XDRDestroy(xdrDecoder);
	return JS_TRUE;
}



struct Private {

	JLMutexHandler mutex;
	JLThreadHandler threadHandle;
	bool end;

	Serialized serializedCode;

	JLSemaphoreHandler requestSem;
	jl::Queue requestList;
	size_t requestCount;

	JLSemaphoreHandler responseSem;
	jl::Queue responseList;
	size_t responseCount;

	jl::Queue exceptionList;
};


/**doc
$CLASS_HEADER
**/
BEGIN_CLASS( Task )

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

		while ( !jl::QueueIsEmpty(&pv->responseList) ) {

			Serialized xdr = (Serialized)jl::QueueShift(&pv->responseList);
			SerializerFree(&xdr);
		}

		while ( !jl::QueueIsEmpty(&pv->requestList) ) {

			Serialized xdr = (Serialized)jl::QueueShift(&pv->requestList);
			SerializerFree(&xdr);
		}

		while ( !jl::QueueIsEmpty(&pv->exceptionList) ) {

			Serialized xdr = (Serialized)jl::QueueShift(&pv->exceptionList);
			SerializerFree(&xdr);
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
			if ( jl::QueueIsEmpty(&pv->requestList) || pv->end )
				goto end;

			Serialized serializedRequest = (Serialized)jl::QueueShift(&pv->requestList);
			pv->requestCount--;
			jsval request;
			J_CHK( UnserializeJsval(cx, &serializedRequest, &request) );
			SerializerFree(&serializedRequest);
			JLReleaseMutex(pv->mutex);

			jsval argv[] = { request, INT_TO_JSVAL(index++) };
	//		J_CHK( J_ADD_ROOT(cx, &value) );
			JSBool status = JS_CallFunction(cx, globalObj, fun, COUNTOF(argv), argv, &rval);
	//		J_CHK( J_REMOVE_ROOT(cx, &value) );

			JLAcquireMutex(pv->mutex);
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
				SerializerInit(&serializedException);
				J_CHK( SerializeJsval(cx, &serializedException, &ex) );
				jl::QueuePush(&pv->exceptionList, serializedException);

				jl::QueuePush(&pv->responseList, NULL); // signals an exception
				pv->responseCount++;
				JS_ClearPendingException(cx);
			} else {

				Serialized serializedResponse;
				SerializerInit(&serializedResponse);
				J_CHK( SerializeJsval(cx, &serializedResponse, &rval) );
				jl::QueuePush(&pv->responseList, serializedResponse);
				pv->responseCount++;
			}
			JLReleaseMutex(pv->mutex);
			JLReleaseSemaphore(pv->responseSem); // signals a response
		}
	}
end:
	return JS_TRUE;
}


JLThreadFuncDecl Thread( void *arglist ) {

	JSContext *cx = CreateHost(-1, -1, 0);
	if ( cx == NULL )
		return -1;
	JSBool status = InitHost(cx, _unsafeMode, NULL, NULL);
	if ( status == JS_TRUE ) {

		JS_ToggleOptions(cx, JS_GetOptions(cx) | JSOPTION_DONT_REPORT_UNCAUGHT);

		Private *pv = (Private*)arglist;
		JSBool status = Task(cx, pv); // (TBD) manage fatal errors
		JLReleaseMutex(pv->mutex);
/*
		JLAcquireMutex(pv->mutex);

		pv->end = true;

		if ( status == JS_FALSE ) {

			jsval ex;
			if ( JS_IsExceptionPending(cx) ) { // manageable error

				J_CHK( JS_GetPendingException(cx, &ex) );
				JSString *jsstr = JS_ValueToString(cx, ex); // transform the exception into a string
				ex = STRING_TO_JSVAL(jsstr);


		Serialized serializedException;
		SerializerInit(&serializedException);
		J_CHK( SerializeJsval(cx, &serializedException, &ex) );
		jl::QueuePush(&pv->exceptionList, serializedException);
		JLReleaseMutex(pv->mutex);
*/
		JLReleaseSemaphore(pv->responseSem);
	}
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
	J_CHK( JS_SetPrivate(cx, obj, pv) );

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

	pv->mutex = JLCreateMutex();
	pv->end = false;

	jl::QueueInitialize(&pv->requestList);
	pv->requestSem = JLCreateSemaphore(0, 1);
	pv->requestCount = 0;

	jl::QueueInitialize(&pv->responseList);
	pv->responseSem = JLCreateSemaphore(0, 1);
	pv->responseCount = 0;

	jl::QueueInitialize(&pv->exceptionList);

	SerializerInit(&pv->serializedCode);
	J_CHK( SerializeJsval(cx, &pv->serializedCode, &J_ARG(1)) );

	pv->threadHandle = JLStartThread(Thread, pv);

	J_S_ASSERT( JLThreadOk(pv->threadHandle), "Unable to create the thread." );

	return JS_TRUE;
}



DEFINE_FUNCTION_FAST( Request ) {

	J_S_ASSERT_ARG_MIN(1);
	Private *pv = (Private*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(pv);

	Serialized serializedRequest;
	SerializerInit(&serializedRequest);
	J_CHK( SerializeJsval(cx, &serializedRequest, &J_FARG(1)) );
	JLAcquireMutex(pv->mutex);
	jl::QueuePush(&pv->requestList, serializedRequest);
	pv->requestCount++;
	JLReleaseMutex(pv->mutex);
	JLReleaseSemaphore(pv->requestSem); // signals a request

	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Response ) {

	Private *pv = (Private*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(pv);

	bool hasNoResponse;
	JLAcquireMutex(pv->mutex);
	hasNoResponse = jl::QueueIsEmpty(&pv->responseList);

	if ( pv->end && jl::QueueIsEmpty(&pv->responseList) && !jl::QueueIsEmpty(&pv->exceptionList) ) {

		Serialized serializedException = (Serialized)jl::QueueShift(&pv->exceptionList);
		JLReleaseMutex(pv->mutex);

		jsval exception;
		J_CHK( UnserializeJsval(cx, &serializedException, &exception) );
		JS_SetPendingException(cx, exception);
		return JS_FALSE;
	}

	JLReleaseMutex(pv->mutex);

	if ( hasNoResponse )
		JLAcquireSemaphore(pv->responseSem); // wait for a response

	JLAcquireMutex(pv->mutex);

	if ( jl::QueueIsEmpty(&pv->responseList) ) { // || !JLThreadIsActive( pv->threadHandle )

		JLReleaseMutex(pv->mutex);
		*J_FRVAL = JSVAL_VOID;
		return JS_TRUE;
	}

	Serialized serializedResponse = (Serialized)jl::QueueShift(&pv->responseList);
	pv->responseCount--;

	if ( SerializerEmpty( &serializedResponse ) ) { // an exception is signaled

		Serialized serializedException = (Serialized)jl::QueueShift(&pv->exceptionList);
		jsval exception;
		J_CHK( UnserializeJsval(cx, &serializedException, &exception) );
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
	J_CHK( UIntToJsval(cx, pv->requestCount, vp) );
	JLReleaseMutex(pv->mutex);
	return JS_TRUE;
}


DEFINE_PROPERTY( pendingResponseCount ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(pv);
	JLAcquireMutex(pv->mutex); // (TBD) needed ?
	J_CHK( UIntToJsval(cx, pv->responseCount, vp) );
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
