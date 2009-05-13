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
	volatile size_t pendingRequestCount;

	volatile size_t processingRequestCount;

	JLSemaphoreHandler responseSem;
	jl::Queue responseList;
	volatile size_t pendingResponseCount;

	jl::Queue exceptionList;
};


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
 With multicore CPUs becoming prevalent, splitting computationally expensive tasks is a good way to obtain better over-all performance.
 The aim of the Task class is to make use of CPU resources for doing computation task like decompressing data, decoding/computing images, create cryptographic keys, processing audio data, complex query on large database ...
 A task can also manage asynchronous I/O, but it is not recommended. Poll() function is a preferred way to manage asynchronous I/O.

 The "new Task(taskFunc);" expression creates a new thread (or more), and a new JavaScript runtime that runs in the thread, then the thread is waiting for a request.
 Each time the Request(req) function is called, the request req is stored in a queue and the thread is unlocked and call the function taskFunc with the first queued request.
 When the taskFunc has finish, its return value is stored in the response queue and the thread returns in the "waiting for a request" state.
 Responses are retrieved from the response queue using the Response() function.

 $H notes
  Creating new tasks are expensive operating system calls. Tasks may have to be reused over the time.
  The function taskFunc is completely isolated from the main program, Request()/Response() API is the only way to exchange data.
  If no response are pending, the Response() function will block until a response is available.
  The request/response queue size is only limited by the available amount of memory.
  If an error or an exception occurs while a request is processed, the exception is stored as the response and raised again when the Response() function is called.
  The execution context of the taskFunc function is reused, this mean that the global object can be used to store data.
  The second argument of the taskFunc(req, index) is the index of the current request. This value can be used for initialization purpose.
  eg.
  {{{
  function MyTask( req, idx ) {
   
   if ( idx == 0 ) {
    LoadModule('jsio');
   }
   ...
  }
 }}}
**/
BEGIN_CLASS( Task )


DEFINE_FINALIZE() {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	if ( !pv )
		return;

	J_CHK( JLAcquireMutex(pv->mutex) ); // --
	pv->end = true;
	J_CHK( JLReleaseMutex(pv->mutex) ); // ++

	JLReleaseSemaphore(pv->requestSem); // +1 // unlock the thread an let it manage the "end"

	J_CHK( JLWaitThread(pv->threadHandle) ); // wait for the end of the thread
	J_CHK( JLFreeThread(&pv->threadHandle) );

	J_CHK( JLFreeSemaphore(&pv->requestSem) );
	J_CHK( JLFreeSemaphore(&pv->responseSem) );

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
	return;

bad:
	JS_free(cx, pv);
	// (TBD) report a warning.
	return;
}


JSBool Task(JSContext *cx, Private *pv) {

	jsval argv[3] = { JSVAL_NULL }; // argv[0] is rval and code
	JSTempValueRooter tvr;
	JS_PUSH_TEMP_ROOT(cx, COUNTOF(argv), argv, &tvr);

	// no need to mutex this because this and the constructor are the only places that access pv->serializedCode.
	J_CHK( UnserializeJsval(cx, &pv->serializedCode, &argv[0]) );
	SerializerFree(&pv->serializedCode);

	JSFunction *fun;
	fun = JS_ValueToFunction(cx, argv[0]);
	JSObject *funObj;
	funObj = JS_GetFunctionObject(fun);
	JSObject *globalObj;
	globalObj = JS_GetGlobalObject(cx);
	J_CHK( JS_SetParent(cx, funObj, globalObj) ); // re-scope the function

	size_t index;
	index = 0;

	for (;;) {

		J_CHK( JLAcquireSemaphore(pv->requestSem) ); // -1 // wait for a request

		J_CHK( JLAcquireMutex(pv->mutex) ); // --
		if ( pv->end ) { // manage the end of the thread

			J_CHK( JLReleaseMutex(pv->mutex) ); // ++
			break;
		}

		Serialized serializedRequest = (Serialized)QueueShift(&pv->requestList);
		pv->pendingRequestCount--;
		pv->processingRequestCount++; // = 1;
		J_CHK( JLReleaseMutex(pv->mutex) ); // ++

		J_CHK( UnserializeJsval(cx, &serializedRequest, &argv[1]) );
		SerializerFree(&serializedRequest);
		argv[2] = INT_TO_JSVAL(index++);
		JSBool status = JS_CallFunction(cx, globalObj, fun, COUNTOF(argv)-1, argv+1, argv);

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

			J_CHK( JLAcquireMutex(pv->mutex) ); // --
			QueuePush(&pv->exceptionList, serializedException);
			QueuePush(&pv->responseList, NULL); // signals an exception
			pv->pendingResponseCount++;
			pv->processingRequestCount--;
			J_CHK( JLReleaseMutex(pv->mutex) ); // ++

			JS_ClearPendingException(cx);
		} else {

			Serialized serializedResponse;
			SerializerCreate(&serializedResponse);
			J_CHK( SerializeJsval(cx, &serializedResponse, &argv[0]) ); // (TBD) need a better exception management

			J_CHK( JLAcquireMutex(pv->mutex) ); // --
			QueuePush(&pv->responseList, serializedResponse);
			pv->pendingResponseCount++;
			pv->processingRequestCount = 0;
			J_CHK( JLReleaseMutex(pv->mutex) ); // ++
		}

		JLReleaseSemaphore(pv->responseSem); // +1 // signals a response
	}

	JS_POP_TEMP_ROOT(cx, &tvr);
	return JS_TRUE;
bad:
	JS_POP_TEMP_ROOT(cx, &tvr);
	return JS_FALSE;
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

		J_CHK( JLAcquireMutex(pv->mutex) ); // --
		pv->end = true;
		QueuePush(&pv->exceptionList, serializedException);
		J_CHK( JLReleaseMutex(pv->mutex) ); // ++

		JLReleaseSemaphore(pv->responseSem); // +1
	}

bad:
	BufferFinalize(&errBuffer);
	if ( cx )
		DestroyHost(cx);
	return 0;
}


/**doc
$TOC_MEMBER $INAME
 $INAME( taskFunc [ , priority = 0 ] )
  Creates a new Task object from the given function.
  $H arguments
   $ARG $FUN taskFunc: the JavaScrip function that will bi run as thread.
   $ARG $INT priority: 0 is normal, -1 is low, 1 is high.
   The _taskFunc_ prototype is: `function( request, index )`.
**/
DEFINE_CONSTRUCTOR() {

	Private *pv = NULL; // keep on top
	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_CLASS( obj, _class );
	J_S_ASSERT_ARG_MIN(1);
	J_S_ASSERT_FUNCTION( J_ARG(1) );

	pv = (Private*)JS_malloc(cx, sizeof(Private));
	J_CHK( pv );

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
	J_CHKM( JLMutexOk(pv->mutex), "Unable to create the mutex." );
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
	J_CHKM( JLThreadOk(pv->threadHandle), "Unable to create the thread." );

	return JS_TRUE;
bad:
	if ( pv )
		JS_free(cx, pv);
	return JS_FALSE;
}



/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( [data] )
  Send data to the task. This function do not block. If the task is already processing a request, next requests are automatically queued.
**/
DEFINE_FUNCTION_FAST( Request ) {

	J_S_ASSERT_CLASS( J_FOBJ, _class );
	Private *pv;
	pv = (Private*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(pv);

	Serialized serializedRequest;
	SerializerCreate(&serializedRequest);
	if ( J_FARG_ISDEF(1) )
		J_CHK( SerializeJsval(cx, &serializedRequest, &J_FARG(1)) ); // leak ???
	else {
		jsval arg = JSVAL_VOID;
		J_CHK( SerializeJsval(cx, &serializedRequest, &arg) );
	}
	J_CHK( JLAcquireMutex(pv->mutex) ); // --
	QueuePush(&pv->requestList, serializedRequest);
	pv->pendingRequestCount++;
	J_CHK( JLReleaseMutex(pv->mutex) ); // ++
	JLReleaseSemaphore(pv->requestSem); // +1 // signals a request
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 data $INAME()
  Read a response from the task. If no response is pending, the function wait until a response is available.
**/
DEFINE_FUNCTION_FAST( Response ) {

	J_S_ASSERT_CLASS( J_FOBJ, _class );
	Private *pv;
	pv = (Private*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(pv);

	bool hasNoResponse;
	J_CHK( JLAcquireMutex(pv->mutex) ); // --
	hasNoResponse = QueueIsEmpty(&pv->responseList);

	if ( QueueIsEmpty(&pv->responseList) && !QueueIsEmpty(&pv->exceptionList) ) {

		Serialized serializedException = (Serialized)QueueShift(&pv->exceptionList);
		J_CHK( JLReleaseMutex(pv->mutex) ); // ++

		jsval exception;
		J_CHK( UnserializeJsval(cx, &serializedException, &exception) );
		JS_SetPendingException(cx, exception);
		
		return JS_FALSE;
	}

	if ( pv->end ) {

		J_CHK( JLReleaseMutex(pv->mutex) ); // ++
		jsval exception = JSVAL_VOID; // throw a StopTask exception
		JS_SetPendingException(cx, exception);
		return JS_FALSE;
	}

	J_CHK( JLReleaseMutex(pv->mutex) ); // ++

	J_CHK( JLAcquireSemaphore(pv->responseSem) ); // -1 // wait for a response

	J_CHK( JLAcquireMutex(pv->mutex) ); // --

	if ( QueueIsEmpty(&pv->responseList) ) { // || !JLThreadIsActive( pv->threadHandle )

		J_CHK( JLReleaseMutex(pv->mutex) ); // ++
		*J_FRVAL = JSVAL_VOID;
		return JS_TRUE;
	}

	Serialized serializedResponse;
	serializedResponse = (Serialized)QueueShift(&pv->responseList);
	pv->pendingResponseCount--;

	if ( SerializerIsEmpty( &serializedResponse ) ) { // an exception is signaled

		Serialized serializedException = (Serialized)QueueShift(&pv->exceptionList);
		J_CHK( JLReleaseMutex(pv->mutex) ); // ++
		jsval exception;
		J_CHK( UnserializeJsval(cx, &serializedException, &exception) ); // (TBD) throw a TaskException ?
		JS_SetPendingException(cx, exception);
		SerializerFree(&serializedException);
		return JS_FALSE;
	} else {

		J_CHK( UnserializeJsval(cx, &serializedResponse, J_FRVAL) );
		SerializerFree(&serializedResponse);
	}
	J_CHK( JLReleaseMutex(pv->mutex) ); // ++

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME
  Is the number of requests that haven't be processed by the task yet. The request being processed is not included in this count.
**/
DEFINE_PROPERTY( pendingRequestCount ) {

	J_S_ASSERT_CLASS( obj, _class );
	Private *pv;
	pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(pv);
	J_CHK( JLAcquireMutex(pv->mutex) ); // --
	J_CHK( UIntToJsval(cx, pv->pendingRequestCount, vp) );
//	J_CHK( UIntToJsval(cx, pv->end ? 0 : pv->pendingRequestCount, vp) );
	J_CHK( JLReleaseMutex(pv->mutex) ); // ++
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME
  Is the number of requests that are currently processed by the task.
**/
DEFINE_PROPERTY( processingRequestCount ) {

	J_S_ASSERT_CLASS( obj, _class );
	Private *pv;
	pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(pv);
	J_CHK( JLAcquireMutex(pv->mutex) ); // --
	J_CHK( UIntToJsval(cx, pv->processingRequestCount, vp) );
	J_CHK( JLReleaseMutex(pv->mutex) ); // ++
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME
  Is the number of available responses that has already been processed by the task.
**/
DEFINE_PROPERTY( pendingResponseCount ) {

	J_S_ASSERT_CLASS( obj, _class );
	Private *pv;
	pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(pv);
	J_CHK( JLAcquireMutex(pv->mutex) ); // --
	J_CHK( UIntToJsval(cx, pv->pendingResponseCount, vp) );
//	J_CHK( UIntToJsval(cx, pv->pendingResponseCount ? pv->pendingResponseCount : QueueIsEmpty(&pv->exceptionList) ? 0 : 1 , vp) );
	J_CHK( JLReleaseMutex(pv->mutex) ); // ++
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME
  Is the current state of the task. true if there is no request being processed, if request and response queues are empty and if there is no pending error.
**/
DEFINE_PROPERTY( idle ) {

	J_S_ASSERT_CLASS( obj, _class );
	Private *pv;
	pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(pv);
	J_CHK( JLAcquireMutex(pv->mutex) ); // --
	J_CHK( BoolToJsval(cx, pv->pendingRequestCount + pv->processingRequestCount + pv->pendingResponseCount == 0 || pv->end, vp) );
	J_CHK( JLReleaseMutex(pv->mutex) ); // ++
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
		PROPERTY_READ(processingRequestCount)
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
