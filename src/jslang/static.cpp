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
#include "../jslang/handlePub.h"
#include "jslang.h"

using namespace jl;

DECLARE_CLASS(Handle)


/**doc fileIndex:topmost **/

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_STATIC

/**doc
=== Static functions ==
**/


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( value )
  This function converts any value into a floating point value.
**/
DEFINE_FUNCTION( real ) {

	JL_S_ASSERT_ARG(1);
	jsdouble val;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &val) );
	*JL_RVAL = DOUBLE_TO_JSVAL(val);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( value )
  This function converts any value of stream into a string.
**/
DEFINE_FUNCTION( Stringify ) {

	JL_S_ASSERT_ARG(1);

	if ( !JSVAL_IS_PRIMITIVE(JL_ARG(1)) ) {

		JSObject *sobj = JSVAL_TO_OBJECT( JL_ARG(1) );

		NIStreamRead read = StreamReadInterface(cx, sobj);
		if ( read ) {

			Buffer buf;
			BufferInitialize(&buf, bufferTypeAuto, bufferGrowTypeAuto, NULL, NULL, NULL, NULL);

			size_t length;
			do {
				length = 4096;
				JL_CHKB( read(cx, sobj, BufferNewChunk(&buf, length), &length), bad_freeBuffer );
				BufferConfirm(&buf, length);
			} while ( length != 0 );

			size_t total;
			total = BufferGetLength(&buf);
			char *newBuffer;
			newBuffer = (char*)JS_malloc(cx, total +1);
			JL_CHK( newBuffer );
			newBuffer[total] = '\0';
			BufferCopyData(&buf, newBuffer, total);
			*JL_RVAL = STRING_TO_JSVAL( JLStr(newBuffer, total, true).GetJSString(cx) );

			BufferFinalize(&buf);
			return JS_TRUE;

		bad_freeBuffer:
			BufferFinalize(&buf);
			return JS_FALSE;
		}
	}

	// fallback:
	{
	JLStr str;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &str) );
	*JL_RVAL = STRING_TO_JSVAL( str.GetJSString(cx) );
	}

	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $INT $INAME( eventsHandler1 [, ... [, eventsHandler30]] )
  Passive event waiting.
  Returns the bit field that represents the index (in arguments order) of the triggered events. eg. if eventsHandler1 event is triggered, bit 1 is set.
  $H example 1
{{{
LoadModule('jsstd');

function onTimeout() {

  Print('.');
}

function onEndSignal() {

  Print('end signal detected\n');
  throw 0;
}

for (;;)
  ProcessEvents( TimeoutEvents(500, onTimeout), EndSignalEvents(onEndSignal) );
}}}
**/
JLThreadFuncDecl ProcessEventThread( void *data ) {

	ProcessEventThreadInfo *ti = (ProcessEventThreadInfo*)data;
	int st;
	for (;;) {

		st = JLSemaphoreAcquire(ti->startSem, JLINFINITE);
		JL_ASSERT(st);
		if ( ti->isEnd )
			break;
		JL_ASSERT( ti->peSlot != NULL );
		ti->peSlot->startWait(ti->peSlot);
		ti->peSlot = NULL;
		JLSemaphoreRelease(ti->signalEventSem);
	}
	JLThreadExit(0);
	return 0;
}

DEFINE_FUNCTION( ProcessEvents ) {

	int st;
	ModulePrivate *mpv = (ModulePrivate*)JL_GetModulePrivate(cx, jslangModuleId);

	JL_S_ASSERT_ARG_RANGE( 1, (int)COUNTOF(mpv->processEventThreadInfo) );
	ProcessEvent *peList[COUNTOF(mpv->processEventThreadInfo)]; // cache to avoid calling GetHandlePrivate() too often.

	uintN i;
	for ( i = 0; i < argc; ++i ) {

		JL_S_ASSERT( IsHandleType(cx, JL_ARGV[i], JL_CAST_CSTR_TO_UINT32("pev")), "Invalid event handle." );
		ProcessEvent *pe = (ProcessEvent*)GetHandlePrivate(cx, JL_ARGV[i]);
		//JL_S_ASSERT_RESOURCE( pe );
		if ( pe == NULL )
			JL_REPORT_ERROR_NUM(cx, JLSMSG_INVALID_RESOURCE);
		JL_ASSERT( pe->startWait );
		JL_ASSERT( pe->cancelWait );
		JL_ASSERT( pe->endWait );
		peList[i] = pe;
	}

	for ( i = 0; i < argc; ++i ) {

		ProcessEventThreadInfo *ti = &mpv->processEventThreadInfo[i];
		if ( ti->thread == 0 ) { // create the thread stuff, see jl_cmalloc in jslangModuleInit()

			ti->startSem = JLSemaphoreCreate(0);
			JL_ASSERT( JLSemaphoreOk(ti->startSem) );
			ti->thread = JLThreadStart(ProcessEventThread, ti);
			JL_ASSERT( JLThreadOk(ti->thread) );
			JLThreadPriority(ti->thread, JL_THREAD_PRIORITY_HIGHEST);
			ti->signalEventSem = mpv->processEventSignalEventSem;
			ti->isEnd = false;
		}
		JL_ASSERT( ti->peSlot == NULL );
		JL_ASSERT( ti->isEnd == false );

		ti->peSlot = peList[i];
		JLSemaphoreRelease(ti->startSem);
	}

	JLSemaphoreAcquire(mpv->processEventSignalEventSem, JLINFINITE); // wait for an event (timeout can also be managed here)
	JLSemaphoreRelease(mpv->processEventSignalEventSem);

	for ( i = 0; i < argc; ++i ) {

		volatile ProcessEvent *peSlot = mpv->processEventThreadInfo[i].peSlot; // avoids to mutex ti->mpSlot access.
		if ( peSlot != NULL ) { // see ProcessEventThread(). if peSlot is null this mean that peSlot->startWait() has returned.

			if ( !peSlot->cancelWait(peSlot) ) { // if the thread cannot be gracefully canceled then kill it.

				ProcessEventThreadInfo *ti = &mpv->processEventThreadInfo[i];
				ti->peSlot = NULL;
				JLSemaphoreRelease(ti->signalEventSem); // see ProcessEventThread()
				JLThreadCancel(ti->thread);
				JLThreadWait(ti->thread, NULL); // (TBD) needed ?
				JLSemaphoreFree(&ti->startSem);
				JLThreadFree(&ti->thread);
				ti->thread = 0; // mean that "the thread is free/unused" (see thread creation place)
			}
		}
	}

	for ( i = 0; i < argc; ++i ) {

		st = JLSemaphoreAcquire(mpv->processEventSignalEventSem, JLINFINITE);
		JL_ASSERT( st );
	}

	JL_ASSERT( argc <= JSVAL_INT_BITS ); // bits
	unsigned int events;
	events = 0;
	bool hasEvent;
	JSBool ok;
	ok = JS_TRUE;
	for ( i = 0; i < argc; ++i ) {

		ProcessEvent *pe = peList[i];

		JSExceptionState *exState = NULL;
		if ( JL_IsExceptionPending(cx) ) {

			exState = JS_SaveExceptionState(cx);
			JS_ClearPendingException(cx);
		}

		if ( pe->endWait(pe, &hasEvent, cx, JSVAL_TO_OBJECT(JL_ARGV[i])) != JS_TRUE ) //
			ok = JS_FALSE;

		if ( exState )
			JS_RestoreExceptionState(cx, exState);

		if ( hasEvent )
			events |= 1 << i;
		JL_CHK( HandleClose(cx, JL_ARGV[i]) );
	}

#ifdef DEBUG
	for ( i = 0; i < argc; ++i )
		JL_ASSERT( mpv->processEventThreadInfo[i].peSlot == NULL );
	JL_ASSERT( JLSemaphoreAcquire(mpv->processEventSignalEventSem, 0) == JLTIMEOUT ); // else invalid state
#endif // DEBUG

	*JL_RVAL = INT_TO_JSVAL(events);
	return ok;
	JL_BAD;
}




/**doc
$TOC_MEMBER $INAME
 $TYPE id $INAME( msTimeout [, onTimeout] )
 Passively waits for a timeout through the ProcessEvents function.
**/
struct UserProcessEvent {

	ProcessEvent pe;
	unsigned int timeout;
	JLEventHandler cancel;
	bool canceled;
	jsval callbackFunction;
};

JL_STATIC_ASSERT( offsetof(UserProcessEvent, pe) == 0 );

void TimeoutStartWait( volatile ProcessEvent *pe ) {

	UserProcessEvent *upe = (UserProcessEvent*)pe;

	if ( upe->timeout > 0 ) {

		int st = JLEventWait(upe->cancel, upe->timeout);
		upe->canceled = (st == JLOK);
	} else {

		upe->canceled = false;
	}
}

bool TimeoutCancelWait( volatile ProcessEvent *pe ) {

	UserProcessEvent *upe = (UserProcessEvent*)pe;

	JLEventTrigger(upe->cancel);
	return true;
}

JSBool TimeoutEndWait( volatile ProcessEvent *pe, bool *hasEvent, JSContext *cx, JSObject *obj ) {

	JL_USE(obj);

	UserProcessEvent *upe = (UserProcessEvent*)pe;

	JLEventFree(&upe->cancel);
	*hasEvent = !upe->canceled;
	if ( !*hasEvent )
		return JS_TRUE;
	if ( JSVAL_IS_VOID( upe->callbackFunction ) )
		return JS_TRUE;
	jsval rval;
	JL_CHK( JS_CallFunctionValue(cx, JL_GetGlobalObject(cx), upe->callbackFunction, 0, NULL, &rval) );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_FUNCTION( TimeoutEvents ) {

	JL_S_ASSERT_ARG_RANGE( 1, 2 );

	unsigned int timeout;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &timeout) );
	if ( JL_ARG_ISDEF(2) )
		JL_S_ASSERT_FUNCTION( JL_ARG(2) );

	UserProcessEvent *upe;
	JL_CHK( HandleCreate(cx, JLHID(pev), sizeof(UserProcessEvent), (void**)&upe, NULL, JL_RVAL) );
	upe->pe.startWait = TimeoutStartWait;
	upe->pe.cancelWait = TimeoutCancelWait;
	upe->pe.endWait = TimeoutEndWait;
	upe->timeout = timeout;
	upe->cancel = JLEventCreate(false);
	JL_ASSERT( JLEventOk(upe->cancel) );

	if ( JL_ARG_ISDEF(2) && JL_IsFunction(cx, JL_ARG(2)) ) {

		SetHandleSlot(cx, *JL_RVAL, 0, JL_ARG(2));
		JL_CHK( SetHandleSlot(cx, *JL_RVAL, 0, JL_ARG(2)) ); // GC protection only
		upe->callbackFunction = JL_ARG(2);
	} else {

		upe->callbackFunction = JSVAL_VOID;
	}

	return JS_TRUE;
	JL_BAD;
}


#ifdef DEBUG
//#include "../jslang/blobPub.h"
DEFINE_FUNCTION( jslang_test ) {

	JL_USE(argc);

/*
	float nvec[10];
	jsuint realLen;
	JL_CHK( JL_JsvalToNativeVector(cx, JL_ARG(1), nvec, COUNTOF(nvec), &realLen) );
*/

	bool b;

//	jsval val = JL_ARG(1);
	JSObject *obj = JS_THIS_OBJECT(cx,vp);

	size_t __err = JLGetEIP(); size_t __pos = JLGetEIP(); ////////////////////////////////////////

	//b = JL_JsidToJsval(cx, (jsid)vp, &val);
	//b = JL_IsObjectObject(cx, obj);
	//JL_NewBlobCopyN(cx, "123", 3, vp);
	//b = JL_GetObjectProtoKey(cx, obj ) != 0;

	_asm int 3;
	JL_IsStringObject(cx, obj);

	bad: ///////////////////////////////////////////////////////////////////////////////////
	printf("code length: %d\n", JLGetEIP() - __pos - (__pos-__err));

	*JL_RVAL = BOOLEAN_TO_JSVAL( b );

//	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
//	JL_BAD;
}
#endif // DEBUG


CONFIGURE_STATIC

//	REVISION(JL_SvnRevToInt("$Revision$")) // avoid to set a revision to the global context
	BEGIN_STATIC_FUNCTION_SPEC

		#ifdef DEBUG
		FUNCTION( jslang_test )
		#endif // DEBUG

		FUNCTION_ARGC( real, 1 )

		FUNCTION_ARGC( Stringify, 1 )
		FUNCTION_ARGC( ProcessEvents, 4 ) // (just a guess)
		FUNCTION_ARGC( TimeoutEvents, 2 )
	END_STATIC_FUNCTION_SPEC

END_STATIC
