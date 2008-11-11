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


void XdrInit( JSXDRState **xdr ) {
	
	*xdr = NULL;
}


void XdrFree( JSXDRState **xdr ) {

	if ( *xdr != NULL ) {

		JS_XDRMemSetData(*xdr, NULL, 0);
		JS_XDRDestroy(*xdr);
		*xdr = NULL;
	}
}


JSBool XdrEncodeJsval( JSContext *cx, JSXDRState **xdr, jsval *val ) {

	if ( *xdr != NULL )
		XdrFree(xdr);
	*xdr = JS_XDRNewMem(cx, JSXDR_ENCODE);
	J_CHK( JS_XDRValue(*xdr, val) );
	return JS_TRUE;
}


JSBool XdrDecodeJsval( JSContext *cx, JSXDRState *xdr, jsval *rval ) {

	JSXDRState *xdrDecoder = JS_XDRNewMem(cx, JSXDR_DECODE);
	uint32 length;
	void *data = JS_XDRMemGetData(xdr, &length);
	JS_XDRMemSetData(xdrDecoder, data, length);
	J_CHK( JS_XDRValue(xdrDecoder, rval) );
	JS_XDRMemSetData(xdrDecoder, NULL, 0);
	JS_XDRDestroy(xdrDecoder);
	return JS_TRUE;
}


struct Private {

	JLMutexHandler mutex;

	JSXDRState *xdrCode;
	JLThreadHandler threadHandle;
	bool end;

	size_t requestCount;
	JLSemaphoreHandler requestSem;
	jl::Queue requestList;

	size_t responseCount;
	JLSemaphoreHandler responseSem;
	jl::Queue responseList;

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

		XdrFree(&pv->xdrCode);


		while ( !jl::QueueIsEmpty(&pv->responseList) ) {

			JSXDRState *xdr = (JSXDRState*)jl::QueueShift(&pv->responseList);
			XdrFree(&xdr);
		}

		while ( !jl::QueueIsEmpty(&pv->requestList) ) {

			JSXDRState *xdr = (JSXDRState*)jl::QueueShift(&pv->requestList);
			XdrFree(&xdr);
		}

		while ( !jl::QueueIsEmpty(&pv->exceptionList) ) {

			JSXDRState *xdr = (JSXDRState*)jl::QueueShift(&pv->exceptionList);
			XdrFree(&xdr);
		}

		JS_free(cx, pv);
	}
}


JSBool Task(JSContext *cx, Private *pv) {

	jsval code, rval;

	J_CHK( XdrDecodeJsval(cx, pv->xdrCode, &code) );
	XdrFree(&pv->xdrCode);

	JSFunction *fun = JS_ValueToFunction(cx, code);
	JSObject *funObj = JS_GetFunctionObject(fun);
	JSObject *globalObj = JS_GetGlobalObject(cx);
	J_CHK( JS_SetParent(cx, funObj, globalObj) ); // re-scope the function

	size_t index = 0;

	for (;;) {

		JLAcquireSemaphore(pv->requestSem); // wait for a request

		JLAcquireMutex(pv->mutex);
		if ( pv->end ) {

			JLReleaseMutex(pv->mutex);
			break;
		}
		JLReleaseMutex(pv->mutex);

		for (;;) {

			JLAcquireMutex(pv->mutex);
			if ( jl::QueueIsEmpty(&pv->requestList) || pv->end ) {

				JLReleaseMutex(pv->mutex);
				break;
			}
			JSXDRState *xdrRequest = (JSXDRState*)jl::QueueShift(&pv->requestList);
			pv->requestCount--;
			jsval request;
			XdrDecodeJsval(cx, xdrRequest, &request);
			XdrFree(&xdrRequest);
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

				JSXDRState *xdrException;
				XdrInit(&xdrException);
				XdrEncodeJsval(cx, &xdrException, &ex);
				jl::QueuePush(&pv->exceptionList, xdrException);

				jl::QueuePush(&pv->responseList, NULL); // signals an exception
				pv->responseCount++;
				JS_ClearPendingException(cx);

			} else {

				JSXDRState *xdrResponse;
				XdrInit(&xdrResponse);
				XdrEncodeJsval(cx, &xdrResponse, &rval);
				jl::QueuePush(&pv->responseList, xdrResponse);
				pv->responseCount++;
			}
			
			JLReleaseMutex(pv->mutex);

			JLReleaseSemaphore(pv->responseSem); // signals a response
		}

	}
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
		Task(cx, pv); // (TBD) manage fatal errors
		
		JLReleaseMutex(pv->mutex);
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

	XdrInit(&pv->xdrCode);
	J_CHK( XdrEncodeJsval(cx, &pv->xdrCode, &J_ARG(1)) );

	pv->threadHandle = JLStartThread(Thread, pv);

	J_S_ASSERT( JLThreadOk(pv->threadHandle), "Unable to create the thread." );

	return JS_TRUE;
}



DEFINE_FUNCTION_FAST( Request ) {

	J_S_ASSERT_ARG_MIN(1);
	Private *pv = (Private*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(pv);

	JLAcquireMutex(pv->mutex);
	JSXDRState *xdrRequest;
	XdrInit(&xdrRequest);
	J_CHK( XdrEncodeJsval(cx, &xdrRequest, &J_FARG(1)) );
	jl::QueuePush(&pv->requestList, xdrRequest);
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
	JLReleaseMutex(pv->mutex);

	if ( hasNoResponse )
		JLAcquireSemaphore(pv->responseSem);

	JLAcquireMutex(pv->mutex);
	JSXDRState *xdrResponse = (JSXDRState*)jl::QueueShift(&pv->responseList);
	pv->responseCount--;

	if ( xdrResponse == NULL ) { // an exception is signaled

		JSXDRState *xdrException = (JSXDRState*)jl::QueueShift(&pv->exceptionList);
		jsval exception;
		J_CHK( XdrDecodeJsval(cx, xdrException, &exception) );
		JS_SetPendingException(cx, exception);
		JLReleaseMutex(pv->mutex);
		return JS_FALSE;

	} else {

		XdrDecodeJsval(cx, xdrResponse, J_FRVAL);
		XdrFree(&xdrResponse);
	}
	JLReleaseMutex(pv->mutex);

	return JS_TRUE;
}


DEFINE_PROPERTY( pendingRequestCount ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(pv);
	JLAcquireMutex(pv->mutex);
	J_CHK( UIntToJsval(cx, pv->requestCount, vp) );
	JLReleaseMutex(pv->mutex);
	return JS_TRUE;
}


DEFINE_PROPERTY( pendingResponseCount ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(pv);
	JLAcquireMutex(pv->mutex);
	J_CHK( UIntToJsval(cx, pv->responseCount, vp) );
	JLReleaseMutex(pv->mutex);
	return JS_TRUE;
}



/*
DEFINE_PROPERTY( result ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(pv);
	JLAcquireMutex(pv->mutex);
	if ( pv->hasError ) {
		
		jsval ex;
		J_CHK( XdrDecodeJsval(cx, pv->xdrError, &ex) );
		JS_SetPendingException(cx, ex);
		JLReleaseMutex(pv->mutex);
		return JS_FALSE;
	}
	if ( pv->hasResult )
		J_CHK( XdrDecodeJsval(cx, pv->xdrValue, vp) );
	else
		*vp = JSVAL_VOID;
	JLReleaseMutex(pv->mutex);
	return JS_TRUE;
}


DEFINE_PROPERTY( resultWait ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(pv);

	JLAcquireSemaphore(pv->resultSem);

	JLAcquireMutex(pv->mutex);
	if ( pv->hasError ) {
		
		jsval ex;
		J_CHK( XdrDecodeJsval(cx, pv->xdrError, &ex) );
		JS_SetPendingException(cx, ex);
		JLReleaseMutex(pv->mutex);
		return JS_FALSE;
	}
	if ( pv->hasResult )
		J_CHK( XdrDecodeJsval(cx, pv->xdrValue, vp) );
	else
		*vp = JSVAL_VOID;
	JLReleaseMutex(pv->mutex);
	return JS_TRUE;
}
*/

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
