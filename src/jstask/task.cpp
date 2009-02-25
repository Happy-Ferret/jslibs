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

#include "../common/buffer.h"

#include <errno.h>

using namespace jl;

struct Private {

	JLMutexHandler mutex;
	JLThreadHandler threadHandle;
	bool end;

	Serialized serializedCode;

	JLSemaphoreHandler requestSem;
	jl::Queue requestList;
	size_t pendingRequestCount;

	size_t processingRequestCount;

	JLSemaphoreHandler responseSem;
	jl::Queue responseList;
	size_t pendingResponseCount;

	jl::Queue exceptionList;
};


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
 With multicore CPUs becoming prevalent, splitting computationally expensive tasks is a good way to obtain better over-all performance.
 Task like:
 * Decoding a SVG image.
 * Generate a public/private key pair.
 * Decoding compressed Audio data.
 * Complex query on large database.

 $H note
  Creating new tasks are expensive operating system calls. Tasks may have to be reused over the time.
**/
BEGIN_CLASS( Task )


//void TaskCleanupLists(Private *pv) {
//}

DEFINE_FINALIZE() {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	if ( pv ) {

		JLAcquireMutex(pv->mutex); // --
		pv->end = true;
		JLReleaseMutex(pv->mutex); // ++

		JLReleaseSemaphore(pv->requestSem); // +1 // unlock the thread an let it manage the "end"

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

		pv->processingRequestCount = 0;

		JS_free(cx, pv);
	}
}


JSBool Task(JSContext *cx, Private *pv) {

	jsval code, rval;

	J_CHK( UnserializeJsval(cx, &pv->serializedCode, &code) ); // no need to mutex this because this is the only place that access pv->serializedCode
	SerializerFree(&pv->serializedCode);

	JSFunction *fun;
	fun = JS_ValueToFunction(cx, code);
	JSObject *funObj;
	funObj = JS_GetFunctionObject(fun);
	JSObject *globalObj;
	globalObj = JS_GetGlobalObject(cx);
	J_CHK( JS_SetParent(cx, funObj, globalObj) ); // re-scope the function

	size_t index;
	index = 0;

	for (;;) {

		JLAcquireSemaphore(pv->requestSem); // -1 // wait for a request

		JLAcquireMutex(pv->mutex); // --
		if ( pv->end ) { // manage the end of the thread

			JLReleaseMutex(pv->mutex); // ++
			break;
		}

		Serialized serializedRequest = (Serialized)QueueShift(&pv->requestList);
		pv->pendingRequestCount--;
		pv->processingRequestCount = 1;
		JLReleaseMutex(pv->mutex); // ++

		jsval request;
		J_CHK( UnserializeJsval(cx, &serializedRequest, &request) );
		SerializerFree(&serializedRequest);

		jsval argv[2]; // (TBD) root something ?
		argv[0] = request;
		argv[1] = INT_TO_JSVAL(index++);
		JSBool status = JS_CallFunction(cx, globalObj, fun, COUNTOF(argv), argv, &rval);

		if ( !status ) {

			jsval ex;
			if ( JS_IsExceptionPending(cx) ) { // manageable error

				J_CHK( JS_GetPendingException(cx, &ex) );
				if ( !IsSerializable(ex) ) {

					JSString *jsstr = JS_ValueToString(cx, ex); // transform the exception into a string
					ex = STRING_TO_JSVAL(jsstr);
				}
			} else {

				ex = JSVAL_VOID; // unknown exception
			}

			Serialized serializedException;
			SerializerCreate(&serializedException);
			J_CHK( SerializeJsval(cx, &serializedException, &ex) );

			JLAcquireMutex(pv->mutex); // --
			QueuePush(&pv->exceptionList, serializedException);
			QueuePush(&pv->responseList, NULL); // signals an exception
			pv->pendingResponseCount++;
			pv->processingRequestCount = 0;
			JLReleaseMutex(pv->mutex); // ++

			JS_ClearPendingException(cx);
		} else {

			Serialized serializedResponse;
			SerializerCreate(&serializedResponse);
			J_CHK( SerializeJsval(cx, &serializedResponse, &rval) ); // (TBD) need a better exception management

			JLAcquireMutex(pv->mutex); // --
			QueuePush(&pv->responseList, serializedResponse);
			pv->pendingResponseCount++;
			pv->processingRequestCount = 0;
			JLReleaseMutex(pv->mutex); // ++
		}

		JLReleaseSemaphore(pv->responseSem); // +1 // signals a response
	}

	return JS_TRUE;
	JL_BAD;
}


int TaskStdErrHostOutput( void *privateData, const char *buffer, size_t length ) {

	Buffer *eb = (Buffer*)privateData;
	memcpy(BufferNewChunk(eb, length), buffer, length);
	BufferConfirm(eb, length);
	return 0;
}


JLThreadFuncDecl ThreadProc( void *threadArg ) {

	Buffer errBuffer;
	BufferInitialize(&errBuffer, bufferTypeRealloc, bufferGrowTypeDouble);

	JSContext *cx = CreateHost(-1, -1, 0);
	if ( cx == NULL )
		return 0;

	J_CHK( InitHost(cx, _unsafeMode, NULL, TaskStdErrHostOutput, &errBuffer) );
	
	JS_SetOptions(cx, JS_GetOptions(cx) | JSOPTION_DONT_REPORT_UNCAUGHT);

	Private *pv;
	pv = (Private*)threadArg;

	JSBool status;
	status = Task(cx, pv);

	if ( status != JS_TRUE ) { // fatal errors

		jsval ex;
		if ( JS_IsExceptionPending(cx) ) {

			J_CHK( JS_GetPendingException(cx, &ex) );

			if ( !IsSerializable(ex) ) {

				JSString *jsstr = JS_ValueToString(cx, ex); // transform the exception into a string
				ex = STRING_TO_JSVAL(jsstr);
			}
		} else {

			J_CHK( StringAndLengthToJsval(cx, &ex, BufferGetData(&errBuffer), BufferGetLength(&errBuffer)) );
			//ex = JSVAL_VOID; // unknown exception
		}

		Serialized serializedException;
		SerializerCreate(&serializedException);
		J_CHK( SerializeJsval(cx, &serializedException, &ex) );

		JLAcquireMutex(pv->mutex); // --
		pv->end = true;
		QueuePush(&pv->exceptionList, serializedException);
		JLReleaseMutex(pv->mutex); // ++

		JLReleaseSemaphore(pv->responseSem); // +1
	}

bad:
	BufferFinalize(&errBuffer);
	if ( cx )
		DestroyHost(cx);
	return 0;
}


/**doc
 * $INAME( taskFunc [ , priority = 0 ] )
  Creates a new Task object from the given function.
  $H arguments
   $ARG function taskFunc:
   $ARG integer priority:
   The _taskFunc_ prototype is: `function( request, index )`.
**/
DEFINE_CONSTRUCTOR() {

	Private *pv = NULL; // keep on top
	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_CLASS( obj, _class );
	J_S_ASSERT_ARG_MIN(1);
	J_S_ASSERT_FUNCTION( J_ARG(1) );

	pv = (Private*)JS_malloc(cx, sizeof(Private));
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

	J_CHK( JS_SetPrivate(cx, obj, pv) );

	pv->mutex = JLCreateMutex();
	pv->end = false;

	QueueInitialize(&pv->requestList);
	pv->requestSem = JLCreateSemaphore(0);
	pv->pendingRequestCount = 0;

	pv->processingRequestCount = 0;

	QueueInitialize(&pv->responseList);
	pv->responseSem = JLCreateSemaphore(0);
	pv->pendingResponseCount = 0;

	QueueInitialize(&pv->exceptionList);

	SerializerCreate(&pv->serializedCode);
	J_CHK( SerializeJsval(cx, &pv->serializedCode, &J_ARG(1)));

	pv->threadHandle = JLThreadStart(ThreadProc, pv);

	J_S_ASSERT( JLThreadOk(pv->threadHandle), "Unable to create the thread." );

	return JS_TRUE;
bad:
	if ( pv )
		JS_free(cx, pv);
	return JS_FALSE;
}



/**doc
 * $VOID $INAME( data )
  Send data to the task. This function do not block. If the task is already processing a request, next requests are automatically queued.
**/
DEFINE_FUNCTION_FAST( Request ) {

	J_S_ASSERT_CLASS( J_FOBJ, _class );
	J_S_ASSERT_ARG_MIN(1);
	Private *pv;
	pv = (Private*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(pv);

	Serialized serializedRequest;
	SerializerCreate(&serializedRequest);
	J_CHK( SerializeJsval(cx, &serializedRequest, &J_FARG(1)) );
	JLAcquireMutex(pv->mutex); // --
	QueuePush(&pv->requestList, serializedRequest);
	pv->pendingRequestCount++;
	JLReleaseMutex(pv->mutex); // ++
	JLReleaseSemaphore(pv->requestSem); // +1 // signals a request
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
 * data $INAME()
  Read a response from the task. If no response is pending, the function wait until a response is available.
**/
DEFINE_FUNCTION_FAST( Response ) {

	J_S_ASSERT_CLASS( J_FOBJ, _class );
	Private *pv;
	pv = (Private*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(pv);

	bool hasNoResponse;
	JLAcquireMutex(pv->mutex); // --
	hasNoResponse = QueueIsEmpty(&pv->responseList);

	if ( QueueIsEmpty(&pv->responseList) && !QueueIsEmpty(&pv->exceptionList) ) {

		Serialized serializedException = (Serialized)QueueShift(&pv->exceptionList);
		JLReleaseMutex(pv->mutex); // ++

		jsval exception;
		J_CHK( UnserializeJsval(cx, &serializedException, &exception) );
		JS_SetPendingException(cx, exception);
		return JS_FALSE;
	}

	if ( pv->end ) {

		JLReleaseMutex(pv->mutex); // ++
		jsval exception = JSVAL_VOID; // throw a StopTask exception
		JS_SetPendingException(cx, exception);
		return JS_FALSE;
	}

	JLReleaseMutex(pv->mutex); // ++

	JLAcquireSemaphore(pv->responseSem); // -1 // wait for a response

	JLAcquireMutex(pv->mutex); // --

	if ( QueueIsEmpty(&pv->responseList) ) { // || !JLThreadIsActive( pv->threadHandle )

		JLReleaseMutex(pv->mutex); // ++
		*J_FRVAL = JSVAL_VOID;
		return JS_TRUE;
	}

	Serialized serializedResponse;
	serializedResponse = (Serialized)QueueShift(&pv->responseList);
	pv->pendingResponseCount--;

	if ( SerializerIsEmpty( &serializedResponse ) ) { // an exception is signaled

		Serialized serializedException = (Serialized)QueueShift(&pv->exceptionList);
		JLReleaseMutex(pv->mutex); // ++
		jsval exception;
		J_CHK( UnserializeJsval(cx, &serializedException, &exception) ); // (TBD) throw a TaskException ?
		JS_SetPendingException(cx, exception);
		return JS_FALSE;
	} else {

		J_CHK( UnserializeJsval(cx, &serializedResponse, J_FRVAL) );
		SerializerFree(&serializedResponse);
	}
	JLReleaseMutex(pv->mutex); // ++

	return JS_TRUE;
	JL_BAD;
}


/**doc
 * $INT $INAME
  Is the number of requests that haven't be processed by the task yet. The request being processed is not included in this count.
**/
DEFINE_PROPERTY( pendingRequestCount ) {

	J_S_ASSERT_CLASS( obj, _class );
	Private *pv;
	pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(pv);
	JLAcquireMutex(pv->mutex); // --
	J_CHK( UIntToJsval(cx, pv->pendingRequestCount, vp) );
//	J_CHK( UIntToJsval(cx, pv->end ? 0 : pv->pendingRequestCount, vp) );
	JLReleaseMutex(pv->mutex); // ++
	return JS_TRUE;
	JL_BAD;
}


/**doc
 * $INT $INAME
  Is the number of available responses that has already been processed by the task.
**/
DEFINE_PROPERTY( pendingResponseCount ) {

	J_S_ASSERT_CLASS( obj, _class );
	Private *pv;
	pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(pv);
	JLAcquireMutex(pv->mutex); // --
	J_CHK( UIntToJsval(cx, pv->pendingResponseCount, vp) );
//	J_CHK( UIntToJsval(cx, pv->pendingResponseCount ? pv->pendingResponseCount : QueueIsEmpty(&pv->exceptionList) ? 0 : 1 , vp) );
	JLReleaseMutex(pv->mutex); // ++
	return JS_TRUE;
	JL_BAD;
}


/**doc
 * $INT $INAME
  Is the current state of the task. true if there is no request being processed, if request and response queues are empty and if there is no pending error.
**/
DEFINE_PROPERTY( idle ) {

	J_S_ASSERT_CLASS( obj, _class );
	Private *pv;
	pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(pv);
	JLAcquireMutex(pv->mutex); // --
	J_CHK( BoolToJsval(cx, pv->pendingRequestCount + pv->processingRequestCount + pv->pendingResponseCount == 0 || pv->end, vp) );
	JLReleaseMutex(pv->mutex); // ++
	return JS_TRUE;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(SvnRevToInt("$Revision$"))
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
		PROPERTY_READ(idle)
	END_PROPERTY_SPEC

	BEGIN_STATIC_FUNCTION_SPEC
	END_STATIC_FUNCTION_SPEC

END_CLASS


/**doc
=== Examples ===
 $H example 1
 {{{
 LoadModule('jsstd');
 LoadModule('jstask');

 function MyTask( request ) {

  var sum = 0;
  for ( var i = 0; i < request; i++) {
   
   sum = sum + i;
  }
  return sum;
 }

 var myTask = new Task(MyTask);

 for ( var i = 0; i < 100; i++ ) {
  
  myTask.Request(i);
 }

 while ( !myTask.idle )
 Print( myTask.Response(), '\n' );
 }}}
 **/
