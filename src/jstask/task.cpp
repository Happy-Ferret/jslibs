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
#include <buffer.h>
//#include <jsvalserializer.h>

#include "../host/host.h"
#include "../jslang/handlePub.h"

#include <errno.h>

using namespace jl;


typedef struct SerializedData {
	void *data;
	size_t length;
} SerializedData;


ALWAYS_INLINE void SerializerCreate( SerializedData **ser ) {

	*ser = (SerializedData*)jl_malloc(sizeof(SerializedData));
	(*ser)->length = 0;
}

ALWAYS_INLINE void SerializerFree( SerializedData **ser ) {

	jl_free(*ser);
	*ser = NULL;
}

ALWAYS_INLINE JSBool SerializeJsval( JSContext *cx, SerializedData *ser, jsval *val ) {

	jl::Serializer serializer(JSVAL_VOID);
	JL_CHK( serializer.Write(cx, *val) );
	JL_CHK( serializer.GetBufferOwnership(&ser->data, &ser->length) );

	// (TBD) big strange crash when we return JS_FALSE here.
	// stacktrace:
	// 	ntdll.dll!7c90100b()
	// 	[Frames below may be incorrect and/or missing, no symbols loaded for ntdll.dll]
	//	>	jstask.dll!JLMutexAcquire(__JSMUtexHandler * mutex=0x7c840c5b)  Line 1733 + 0xc bytes	C++
	//	 	ffffffff()
	return JS_TRUE;
	JL_BAD;
}

ALWAYS_INLINE JSBool UnserializeJsval( JSContext *cx, const SerializedData *ser, jsval *rval ) {

	jl::Unserializer unserializer(JSVAL_VOID, ser->data, ser->length);
	JL_CHK( unserializer.Read(cx, *rval) );
	return JS_TRUE;
	JL_BAD;
}

ALWAYS_INLINE bool IsSerializable( jsval val ) {
	return true;
}

ALWAYS_INLINE bool SerializerIsEmpty( const SerializedData *ser ) {

	return ser->length == 0;
}



struct TaskPrivate : CppAllocators {

	JLMutexHandler mutex;
	JLThreadHandler threadHandle;
	bool end;

	SerializedData * serializedCode;

	JLSemaphoreHandler requestSem;
	jl::Queue requestList;
	volatile size_t pendingRequestCount;

	volatile size_t processingRequestCount;

	JLEventHandler responseEvent; // (TBD) replace responseSem by this signal (like a conditional variable).

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
    loadModule('jsio');
   }
   ...
  }
 }}}
**/
BEGIN_CLASS( Task )


DEFINE_FINALIZE() {

	TaskPrivate *pv = (TaskPrivate*)JL_GetPrivate(cx, obj);
	if ( !pv )
		return;

	JLMutexAcquire(pv->mutex); // --
	pv->end = true;
	JLMutexRelease(pv->mutex); // ++

	JLSemaphoreRelease(pv->requestSem); // +1 // unlock the thread an let it manage the "end"

	JLThreadWait(pv->threadHandle, NULL); // wait for the end of the thread
	JLThreadFree(&pv->threadHandle);

	JLSemaphoreFree(&pv->requestSem);
	JLSemaphoreFree(&pv->responseSem);
	JLEventFree(&pv->responseEvent);

	JLMutexFree(&pv->mutex);

	while ( !QueueIsEmpty(&pv->requestList) ) {

		SerializedData * ser = (SerializedData *)QueueShift(&pv->requestList);
		SerializerFree(&ser);
	}

	delete pv;
	return;

bad:
	delete pv;
	// (TBD) report a warning.
	return;
}


JSBool TheTask(JSContext *cx, TaskPrivate *pv) {

	jsval argv[3] = { JSVAL_NULL }; // argv[0] is rval and code

	// no need to mutex this because this and the constructor are the only places that access pv->serializedCode.
	JL_CHK( UnserializeJsval(cx, pv->serializedCode, &argv[0]) );
	SerializerFree(&pv->serializedCode);

	JSFunction *fun;
	fun = JS_ValueToFunction(cx, argv[0]);
	JSObject *funObj;
	funObj = JS_GetFunctionObject(fun);
	JSObject *globalObj;
	globalObj = JL_GetGlobal(cx);
	ASSERT( funObj != NULL );
	JL_CHK( JS_SetParent(cx, funObj, globalObj) ); // re-scope the function

	size_t index;
	index = 0;

	for (;;) {

		JL_CHK( JLSemaphoreAcquire(pv->requestSem, JLINFINITE) ); // -1 // wait for a request

		JLMutexAcquire(pv->mutex); // --
		if ( pv->end ) { // manage the end of the thread

			JLMutexRelease(pv->mutex); // ++
			break;
		}

		SerializedData * serializedRequest = (SerializedData *)QueueShift(&pv->requestList);
		pv->pendingRequestCount--;
		pv->processingRequestCount++; // = 1;
		JLMutexRelease(pv->mutex); // ++

		JL_CHK( UnserializeJsval(cx, serializedRequest, &argv[1]) );
		SerializerFree(&serializedRequest);
		argv[2] = INT_TO_JSVAL(index++);
		JSBool status = JS_CallFunction(cx, globalObj, fun, COUNTOF(argv)-1, argv+1, argv);

		if ( !status ) {

			jsval ex;
			if ( JL_IsExceptionPending(cx) ) { // manageable error

				JL_CHK( JS_GetPendingException(cx, &ex) );
				if ( !IsSerializable(ex) ) {

					JSString *jsstr = JS_ValueToString(cx, ex); // transform the exception into a string
					ex = STRING_TO_JSVAL(jsstr);
				}
			} else {

				ex = JSVAL_VOID; // unknown exception
			}

			SerializedData * serializedException;
			SerializerCreate(&serializedException);
			JL_CHK( SerializeJsval(cx, serializedException, &ex) );

			JLMutexAcquire(pv->mutex); // --
			QueuePush(&pv->exceptionList, serializedException);
			QueuePush(&pv->responseList, NULL); // signals an exception
			pv->pendingResponseCount++;
			pv->processingRequestCount--;
			JLMutexRelease(pv->mutex); // ++

			JS_ClearPendingException(cx);
		} else {

			SerializedData * serializedResponse;
			SerializerCreate(&serializedResponse);
			JL_CHK( SerializeJsval(cx, serializedResponse, &argv[0]) ); // (TBD) need a better exception management

			JLMutexAcquire(pv->mutex); // --
			QueuePush(&pv->responseList, serializedResponse);
			pv->pendingResponseCount++;
			pv->processingRequestCount = 0;
			JLMutexRelease(pv->mutex); // ++
		}

		JLSemaphoreRelease(pv->responseSem); // +1 // signals a response
		JLEventTrigger(pv->responseEvent);
	}

	return JS_TRUE;
bad:
	return JS_FALSE;
}


int TaskStdErrHostOutput( void *privateData, const char *buffer, size_t length ) {

	Buffer *eb = (Buffer*)privateData;
	jl_memcpy(BufferNewChunk(eb, length), buffer, length);
	BufferConfirm(eb, length);
	return 0;
}


JLThreadFuncDecl TaskThreadProc( void *threadArg ) {

	TaskPrivate *pv = NULL;

	Buffer errBuffer;
	BufferInitialize(&errBuffer, bufferTypeRealloc, bufferGrowTypeDouble, NULL, NULL, NULL, NULL);

	JSContext *cx = CreateHost((uint32_t)-1, (uint32_t)-1, 0);
	JL_CHK( cx != NULL );

	HostPrivate *hpv;
	hpv = JL_GetHostPrivate(cx);

// allocator must be threadsafe !
	hpv->alloc.malloc = jl_malloc;
	hpv->alloc.calloc = jl_calloc;
	hpv->alloc.memalign = jl_memalign;
	hpv->alloc.realloc = jl_realloc;
	hpv->alloc.msize = jl_msize;
	hpv->alloc.free = jl_free;

	JL_CHK( InitHost(cx, _unsafeMode != 0, NULL, NULL, TaskStdErrHostOutput, &errBuffer) );

	JS_SetOptions(cx, JS_GetOptions(cx) | JSOPTION_DONT_REPORT_UNCAUGHT);

	pv = (TaskPrivate*)threadArg;

	JSBool status;
	status = TheTask(cx, pv);
	if ( status != JS_TRUE ) { // fatal errors

		jsval ex;
		if ( JL_IsExceptionPending(cx) ) {

			JL_CHK( JS_GetPendingException(cx, &ex) );

			if ( !IsSerializable(ex) ) {

				JSString *jsstr = JS_ValueToString(cx, ex); // transform the exception into a string
				ex = STRING_TO_JSVAL(jsstr);
			}
		} else {

			//JL_CHK( StringAndLengthToJsval(cx, &ex, BufferGetData(&errBuffer), BufferGetLength(&errBuffer)) );
			JL_CHK( JL_NativeToJsval(cx, BufferGetData(&errBuffer), BufferGetLength(&errBuffer), &ex) );

			//ex = JSVAL_VOID; // unknown exception
		}

		SerializedData * serializedException;
		SerializerCreate(&serializedException);
		JL_CHK( SerializeJsval(cx, serializedException, &ex) );

		JLMutexAcquire(pv->mutex); // --
		pv->end = true;
		QueuePush(&pv->exceptionList, serializedException);
		JLMutexRelease(pv->mutex); // ++

		JLSemaphoreRelease(pv->responseSem); // +1
	}

bad:
	BufferFinalize(&errBuffer);

	// These queues must be destroyed before cx because SerializedData * *ser hold a reference to the context that created the value.
	JLMutexAcquire(pv->mutex); // --
	while ( !QueueIsEmpty(&pv->exceptionList) ) {

		SerializedData * ser = (SerializedData *)QueueShift(&pv->exceptionList);
		SerializerFree(&ser);
	}
	while ( !QueueIsEmpty(&pv->responseList) ) {

		SerializedData * ser = (SerializedData *)QueueShift(&pv->responseList);
		SerializerFree(&ser);
	}
	JLMutexRelease(pv->mutex); // ++

	if ( cx )
		DestroyHost(cx, true);
	JLThreadExit(0);
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

	TaskPrivate *pv = NULL; // keep on top

	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT_ARG_IS_CALLABLE(1);

	pv = new TaskPrivate;
	JL_CHK( pv );

	JLThreadPriorityType priority;
	if ( JL_ARG_ISDEF(2) ) {

		int p;
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &p) );
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
				JL_ASSERT_ARG_VAL_RANGE( p, -1, 1, 2 );
		}
	}

	JL_SetPrivate(cx, obj, pv);

	pv->mutex = JLMutexCreate();
	JL_CHKM( JLMutexOk(pv->mutex), E_THISOBJ, E_CREATE ); // "Unable to create the mutex."
	pv->end = false;

	QueueInitialize(&pv->requestList);
	pv->requestSem = JLSemaphoreCreate(0);
	pv->pendingRequestCount = 0;

	pv->processingRequestCount = 0;

	QueueInitialize(&pv->responseList);
	pv->responseSem = JLSemaphoreCreate(0);
	pv->responseEvent = JLEventCreate(false);
	ASSERT( JLEventOk(pv->responseEvent) );
	pv->pendingResponseCount = 0;

	QueueInitialize(&pv->exceptionList);

	SerializerCreate(&pv->serializedCode);
	JL_CHK( SerializeJsval(cx, pv->serializedCode, &JL_ARG(1)));

	pv->threadHandle = JLThreadStart(TaskThreadProc, pv);
	JL_CHKM( JLThreadOk(pv->threadHandle), E_THISOBJ, E_CREATE ); // "Unable to create the thread."
	return JS_TRUE;
bad:
	delete pv;
	return JS_FALSE;
}



/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( [data] )
  Send data to the task. This function do not block. If the task is already processing a request, next requests are automatically queued.
**/
DEFINE_FUNCTION( request ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();

	TaskPrivate *pv;
	pv = (TaskPrivate*)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	SerializedData * serializedRequest;
	SerializerCreate(&serializedRequest);
	if ( JL_ARG_ISDEF(1) )
		JL_CHK( SerializeJsval(cx, serializedRequest, &JL_ARG(1)) ); // leak ???
	else {
		jsval arg = JSVAL_VOID;
		JL_CHK( SerializeJsval(cx, serializedRequest, &arg) );
	}
	JLMutexAcquire(pv->mutex); // --
	QueuePush(&pv->requestList, serializedRequest);
	pv->pendingRequestCount++;
	JLMutexRelease(pv->mutex); // ++
	JLSemaphoreRelease(pv->requestSem); // +1 // signals a request

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 data $INAME()
  Read a response from the task. If no response is pending, the function wait until a response is available.
**/
DEFINE_FUNCTION( response ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();

	TaskPrivate *pv;
	pv = (TaskPrivate*)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	bool hasNoResponse;
	JLMutexAcquire(pv->mutex); // --
	hasNoResponse = QueueIsEmpty(&pv->responseList);

	if ( QueueIsEmpty(&pv->responseList) && !QueueIsEmpty(&pv->exceptionList) ) {

		SerializedData * serializedException = (SerializedData *)QueueShift(&pv->exceptionList);
		JLMutexRelease(pv->mutex); // ++

		jsval exception;
		JL_CHK( UnserializeJsval(cx, serializedException, &exception) );
		JS_SetPendingException(cx, exception);

		return JS_FALSE;
	}

	if ( pv->end ) {

		JLMutexRelease(pv->mutex); // ++
		jsval exception = JSVAL_VOID; // throw a StopTask exception
		JS_SetPendingException(cx, exception);
		return JS_FALSE;
	}

	JLMutexRelease(pv->mutex); // ++

	JL_CHK( JLSemaphoreAcquire(pv->responseSem, JLINFINITE) ); // -1 // wait for a response

	JLMutexAcquire(pv->mutex); // --

	if ( QueueIsEmpty(&pv->responseList) ) { // || !JLThreadIsActive( pv->threadHandle )

		JLMutexRelease(pv->mutex); // ++
		*JL_RVAL = JSVAL_VOID;
		return JS_TRUE;
	}

	SerializedData * serializedResponse;
	serializedResponse = (SerializedData *)QueueShift(&pv->responseList);
	pv->pendingResponseCount--;

	if ( SerializerIsEmpty( serializedResponse ) ) { // an exception is signaled

		SerializedData * serializedException = (SerializedData *)QueueShift(&pv->exceptionList);
		JLMutexRelease(pv->mutex); // ++
		jsval exception;
		JL_CHK( UnserializeJsval(cx, serializedException, &exception) ); // (TBD) throw a TaskException ?
		JS_SetPendingException(cx, exception);
		SerializerFree(&serializedException);
		return JS_FALSE;
	} else {

		JL_CHK( UnserializeJsval(cx, serializedResponse, JL_RVAL) );
		SerializerFree(&serializedResponse);
	}
	JLMutexRelease(pv->mutex); // ++

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME
  Is the number of requests that haven't be processed by the task yet. The request being processed is not included in this count.
**/
DEFINE_PROPERTY_GETTER( pendingRequestCount ) {

	JL_ASSERT_THIS_INSTANCE();

	TaskPrivate *pv;
	pv = (TaskPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JLMutexAcquire(pv->mutex); // --
	JL_CHK( JL_NativeToJsval(cx, (size_t)pv->pendingRequestCount, vp) );
//	JL_CHK( JL_NativeToJsval(cx, pv->end ? 0 : pv->pendingRequestCount, vp) );
	JLMutexRelease(pv->mutex); // ++
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME
  Is the number of requests that are currently processed by the task.
**/
DEFINE_PROPERTY_GETTER( processingRequestCount ) {

	JL_ASSERT_THIS_INSTANCE();

	TaskPrivate *pv;
	pv = (TaskPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JLMutexAcquire(pv->mutex); // --
	JL_CHK( JL_NativeToJsval(cx, (size_t)pv->processingRequestCount, vp) );
	JLMutexRelease(pv->mutex); // ++
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME
  Is the number of available responses that has already been processed by the task.
**/
DEFINE_PROPERTY_GETTER( pendingResponseCount ) {

	JL_ASSERT_THIS_INSTANCE();

	TaskPrivate *pv;
	pv = (TaskPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JLMutexAcquire(pv->mutex); // --
	JL_CHK( JL_NativeToJsval(cx, (size_t)pv->pendingResponseCount, vp) );
//	JL_CHK( JL_NativeToJsval(cx, pv->pendingResponseCount ? pv->pendingResponseCount : QueueIsEmpty(&pv->exceptionList) ? 0 : 1 , vp) );
	JLMutexRelease(pv->mutex); // ++
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME
  Is the current state of the task. true if there is no request being processed, if request and response queues are empty and if there is no pending error.
**/
DEFINE_PROPERTY_GETTER( idle ) {

	JL_ASSERT_THIS_INSTANCE();

	TaskPrivate *pv;
	pv = (TaskPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JLMutexAcquire(pv->mutex); // --
	JL_CHK(JL_NativeToJsval(cx, pv->pendingRequestCount + pv->processingRequestCount + pv->pendingResponseCount == 0 || pv->end, vp) );
	JLMutexRelease(pv->mutex); // ++
	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $TYPE Handle $INAME
 Passively waits for a response through the processEvents function. When a new response is available, the onResponse function of the Task object is called.
 $H example
{{{
loadModule('jstask');
loadModule('jsstd');

var t = new Task(function(data) {

	loadModule('jsstd');
	sleep(100);
	return data+1;
});

t.onResponse = function(t) {

	var v = t.response();
	t.request( v );
	print(v, '\n');
}

t.request(0);

while ( !endSignal )
	processEvents(t.event(), endSignalEvent());
}}}
**/
struct UserProcessEvent {

	ProcessEvent pe;

	bool canceled;
	JSObject *obj;
	TaskPrivate *pv;
};

S_ASSERT( offsetof(UserProcessEvent, pe) == 0 );

void TaskStartWait( volatile ProcessEvent *pe ) {

	UserProcessEvent *upe = (UserProcessEvent*)pe;

	while ( upe->pv->pendingResponseCount == 0 && !upe->canceled )
		JLEventWait(upe->pv->responseEvent, JLINFINITE);
	JLEventReset(upe->pv->responseEvent);
}

bool TaskCancelWait( volatile ProcessEvent *pe ) {

	UserProcessEvent *upe = (UserProcessEvent*)pe;

	upe->canceled = true;
	JLEventTrigger(upe->pv->responseEvent);
	return true;
}

JSBool TaskEndWait( volatile ProcessEvent *pe, bool *hasEvent, JSContext *cx, JSObject *obj ) {

	UserProcessEvent *upe = (UserProcessEvent*)pe;

	*hasEvent = upe->pv->pendingResponseCount > 0;
	if ( !*hasEvent )
		return JS_TRUE;

	jsval fct, argv[2];
	argv[1] = OBJECT_TO_JSVAL(upe->obj); // already rooted

	JL_CHK( JS_GetProperty(cx, upe->obj, "onResponse", &fct) );
	if ( JL_ValueIsCallable(cx, fct) )
		JL_CHK( JS_CallFunctionValue(cx, upe->obj, fct, COUNTOF(argv)-1, argv+1, argv) );

	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION( events ) {

	JL_DEFINE_FUNCTION_OBJ;

	JL_ASSERT_ARGC(0);
	JL_ASSERT_THIS_INSTANCE();

	TaskPrivate *pv;
	pv = (TaskPrivate*)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	UserProcessEvent *upe;
	JL_CHK( HandleCreate(cx, JLHID(pev), sizeof(UserProcessEvent), (void**)&upe, NULL, JL_RVAL) );
	upe->pe.startWait = TaskStartWait;
	upe->pe.cancelWait = TaskCancelWait;
	upe->pe.endWait = TaskEndWait;

	upe->canceled = false;
	upe->obj = JL_OBJ;
	upe->pv = pv;

	JL_CHK( SetHandleSlot(cx, *JL_RVAL, 0, OBJECT_TO_JSVAL(upe->obj)) ); // GC protection

	return JS_TRUE;
	JL_BAD;
}



CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_PRIVATE

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION(request)
		FUNCTION(response)
		FUNCTION(events)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_GETTER(pendingRequestCount)
		PROPERTY_GETTER(processingRequestCount)
		PROPERTY_GETTER(pendingResponseCount)
		PROPERTY_GETTER(idle)
	END_PROPERTY_SPEC

END_CLASS


/**doc
=== Examples ===
 $H example 1
 {{{
 loadModule('jsstd');
 loadModule('jstask');

 function MyTask( request ) {

  var sum = 0;
  for ( var i = 0; i < request; i++) {

   sum = sum + i;
  }
  return sum;
 }

 var myTask = new Task(MyTask);

 for ( var i = 0; i < 100; i++ ) {

  myTask.request(i);
 }

 while ( !myTask.idle )
  print( myTask.response(), '\n' );
 }}}
 **/
