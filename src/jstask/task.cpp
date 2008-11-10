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

	bool end;
	JLMutexHandler mutex;
	JLSemaphoreHandler runSem;
	JLSemaphoreHandler resultSem;

	JLThreadHandler threadHandle;
	JSXDRState *xdrCode;
	bool hasResult;
	JSXDRState *xdrValue;
	bool hasError;
	JSXDRState *xdrError;
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
		JLReleaseSemaphore(pv->runSem); // unlock the thread
		JLWaitThread(pv->threadHandle); // wait for the end of the thread
		JLFreeThread(&pv->threadHandle);

		JLFreeSemaphore(&pv->resultSem);
		JLFreeSemaphore(&pv->runSem);
		JLFreeMutex(&pv->mutex);

		XdrFree(&pv->xdrCode);
		XdrFree(&pv->xdrValue);
		XdrFree(&pv->xdrError);
		JS_free(cx, pv);
	}
}


JSBool Task(JSContext *cx, Private *pv) {

	jsval code, value, rval;

	J_CHK( XdrDecodeJsval(cx, pv->xdrCode, &code) );
	XdrFree(&pv->xdrCode);

	JSFunction *fun = JS_ValueToFunction(cx, code);
	JSObject *funObj = JS_GetFunctionObject(fun);
	JSObject *globalObj = JS_GetGlobalObject(cx);
	J_CHK( JS_SetParent(cx, funObj, globalObj) ); // re-scope the function

	for ( size_t round = 0 ; ; round++ ) {

		JLAcquireSemaphore(pv->runSem); // wait for Run()

		JLAcquireMutex(pv->mutex);
		if ( pv->end ) {

			JLReleaseMutex(pv->mutex);
			break;
		}
		J_CHK( XdrDecodeJsval(cx, pv->xdrValue, &value) );
		JLReleaseMutex(pv->mutex);

		jsval argv[] = { value, INT_TO_JSVAL(round) };
//		J_CHK( J_ADD_ROOT(cx, &value) );
		JSBool status = JS_CallFunction(cx, globalObj, fun, COUNTOF(argv), argv, &rval);
//		J_CHK( J_REMOVE_ROOT(cx, &value) );

		JLAcquireMutex(pv->mutex);
		if ( !status ) {

			pv->hasError = true;
			jsval ex;
			if ( JS_IsExceptionPending(cx) ) { // manageable error

				J_CHK( JS_GetPendingException(cx, &ex) );
				JSString *jsstr = JS_ValueToString(cx, ex); // transform the exception into a string
				ex = STRING_TO_JSVAL(jsstr);
			} else {

				ex = JSVAL_VOID; // unknown exception
			}
			J_CHK( XdrEncodeJsval(cx, &pv->xdrError, &ex) );
		} else {

			J_CHK( XdrEncodeJsval(cx, &pv->xdrValue, &rval) );
			pv->hasResult = true;
		}

		JLReleaseMutex(pv->mutex);
		JLReleaseSemaphore(pv->resultSem); // signals resultWait
	}
	return JS_TRUE;
}


DWORD WINAPI Thread( void *arglist ) {

	JSContext *cx = CreateHost(-1, -1, 0);
	if ( cx == NULL )
		return -1;
	JSBool status = InitHost(cx, _unsafeMode, NULL, NULL);
	if ( status == JS_TRUE ) {

		JS_ToggleOptions(cx, JS_GetOptions(cx) | JSOPTION_DONT_REPORT_UNCAUGHT);

		Private *pv = (Private*)arglist;
		Task(cx, pv);
		// (TBD) manage fatal errors
		JLReleaseMutex(pv->mutex);
		JLReleaseSemaphore(pv->resultSem); // signals resultWait
	}
	DestroyHost(cx);
	return 0;
}


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
	pv->runSem = JLCreateSemaphore(0, 1);

	pv->hasError = false;
	XdrInit(&pv->xdrError);

	pv->hasResult = false;
	XdrInit(&pv->xdrValue); // [in]argument/[out]result
	pv->resultSem = JLCreateSemaphore(0, 1);

	XdrInit(&pv->xdrCode);
	J_CHK( XdrEncodeJsval(cx, &pv->xdrCode, &J_ARG(1)) );

	pv->threadHandle = JLStartThread(Thread, pv);

	J_S_ASSERT( JLThreadOk(pv->threadHandle), "Unable to create the thread." );

	return JS_TRUE;
}



DEFINE_FUNCTION_FAST( Run ) {

	J_S_ASSERT_ARG_MIN(1);
	Private *pv = (Private*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(pv);

	JLAcquireMutex(pv->mutex);
	pv->hasResult = false;
	J_CHK( XdrEncodeJsval(cx, &pv->xdrValue, &J_FARG(1)) );
	JLReleaseMutex(pv->mutex);

	JLReleaseSemaphore(pv->runSem);

	return JS_TRUE;
}


DEFINE_PROPERTY( hasResult ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(pv);
	JLAcquireMutex(pv->mutex);
	J_CHK( BoolToJsval(cx, pv->hasResult, vp) );
	JLReleaseMutex(pv->mutex);
	return JS_TRUE;
}


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


CONFIGURE_CLASS

	HAS_PRIVATE

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST(Run)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ(hasResult)
		PROPERTY_READ(result)
		PROPERTY_READ(resultWait)
	END_PROPERTY_SPEC

	BEGIN_STATIC_FUNCTION_SPEC
	END_STATIC_FUNCTION_SPEC

END_CLASS
