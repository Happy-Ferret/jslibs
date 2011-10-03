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

#include <jstypedarray.h>


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



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME()
  Returns $TRUE if the value is a boolean value or object.
**/
DEFINE_FUNCTION( IsBoolean ) {

	JL_ASSERT_ARGC(1);

	if ( JSVAL_IS_BOOLEAN(JL_ARG(1)) ) {

		*JL_RVAL = JSVAL_TRUE;
		return JS_TRUE;
	}

	if ( JSVAL_IS_PRIMITIVE(JL_ARG(1)) ) {

		*JL_RVAL = JSVAL_FALSE;
		return JS_TRUE;
	}

	*JL_RVAL = BOOLEAN_TO_JSVAL( JL_GetClass(JSVAL_TO_OBJECT(JL_ARG(1))) == JL_GetStandardClassByKey(cx, JSProto_Boolean) );

	return JS_TRUE;
	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME()
  Returns $TRUE if the value is a number value or object.
**/
DEFINE_FUNCTION( IsNumber ) {

	JL_ASSERT_ARGC(1);

	if ( JSVAL_IS_NUMBER(JL_ARG(1)) ) {

		*JL_RVAL = JSVAL_TRUE;
		return JS_TRUE;
	}

	if ( JSVAL_IS_PRIMITIVE(JL_ARG(1)) ) {

		*JL_RVAL = JSVAL_FALSE;
		return JS_TRUE;
	}

	*JL_RVAL = BOOLEAN_TO_JSVAL( JL_GetClass(JSVAL_TO_OBJECT(JL_ARG(1))) == JL_GetStandardClassByKey(cx, JSProto_Number) );

	return JS_TRUE;
	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME()
  Returns $TRUE if the value is a primitive ( null or not an object ).
**/
DEFINE_FUNCTION( IsPrimitive ) {

	JL_IGNORE(cx);
	JL_ASSERT_ARGC(1);

	*JL_RVAL = BOOLEAN_TO_JSVAL( JSVAL_IS_PRIMITIVE(JL_ARG(1)) );

	return JS_TRUE;
	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME()
  Returns $TRUE if the value is a function.
**/
DEFINE_FUNCTION( IsFunction ) {

	JL_ASSERT_ARGC(1);
	*JL_RVAL = BOOLEAN_TO_JSVAL( VALUE_IS_FUNCTION(cx, JL_ARG(1)) );
	return JS_TRUE;
	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME()
  Returns $TRUE if the value is a generator.
**/
DEFINE_FUNCTION( IsGeneratorFunction ) {

	JL_ASSERT_ARGC(1);
	*JL_RVAL = BOOLEAN_TO_JSVAL( JL_IsGeneratorFunction(cx, JL_ARG(1)) );
	return JS_TRUE;
	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME()
  Returns $TRUE if the value is a generator.
**/
DEFINE_FUNCTION( IsGeneratorObject ) {

	JL_ASSERT_ARGC(1);

	*JL_RVAL = BOOLEAN_TO_JSVAL( JL_IsGeneratorObject(cx, JL_ARG(1)) );

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( value )
  This function converts any value into a floating point value.
**/
DEFINE_FUNCTION( Real ) {

	JL_ASSERT_ARGC(1);

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

	JL_ASSERT_ARGC(1);

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

			*JL_RVAL = STRING_TO_JSVAL( JLStr(BufferGetDataOwnership(&buf), BufferGetLength(&buf), false).GetJSString(cx) );
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
		ASSERT(st);
		if ( ti->isEnd )
			break;
		ASSERT( ti != NULL );
		ASSERT( ti->peSlot != NULL );
		ASSERT( ti->peSlot->startWait != NULL );
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

	JL_ASSERT_ARGC_MAX( COUNTOF(mpv->processEventThreadInfo) );
	ProcessEvent *peList[COUNTOF(mpv->processEventThreadInfo)]; // cache to avoid calling GetHandlePrivate() too often.

	if ( JL_ARGC == 0 ) {
		
		*JL_RVAL = INT_TO_JSVAL(0);
		return JS_TRUE;
	}

	uintN i;
	for ( i = 0; i < argc; ++i ) {

		JL_ASSERT_ARG_TYPE( IsHandle(cx, JL_ARGV[i]), i+1, "(pev) Handle" );
		JSObject *pevObj = JSVAL_TO_OBJECT(JL_ARGV[i]);
		JL_ASSERT_ARG_TYPE( IsHandleType(cx, pevObj, JL_CAST_CSTR_TO_UINT32("pev")), i+1, "(pev) Handle" );
		ProcessEvent *pe = (ProcessEvent*)GetHandlePrivate(cx, JL_ARGV[i]);
		JL_ASSERT(  pe != NULL, E_ARG, E_NUM(i+1), E_STATE ); //JL_ASSERT( pe != NULL, E_ARG, E_NUM(i+1), E_ANINVALID, E_NAME("pev Handle") );

		ASSERT( pe->startWait );
		ASSERT( pe->cancelWait );
		ASSERT( pe->endWait );
		peList[i] = pe;
	}

	for ( i = 0; i < argc; ++i ) {

		ProcessEventThreadInfo *ti = &mpv->processEventThreadInfo[i];
		if ( ti->thread == 0 ) { // create the thread stuff, see jl_cmalloc in jslangModuleInit()

			ti->startSem = JLSemaphoreCreate(0);
			ASSERT( JLSemaphoreOk(ti->startSem) );
			ti->thread = JLThreadStart(ProcessEventThread, ti);
			ASSERT( JLThreadOk(ti->thread) );
			JLThreadPriority(ti->thread, JL_THREAD_PRIORITY_HIGHEST);
			ti->signalEventSem = mpv->processEventSignalEventSem;
			ti->isEnd = false;
		}
		ASSERT( ti->peSlot == NULL );
		ASSERT( ti->isEnd == false );

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
		ASSERT( st );
	}

	ASSERT( argc <= JSVAL_INT_BITS ); // bits
	unsigned int eventsMask;
	eventsMask = 0;
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
			eventsMask |= 1 << i;
		JL_CHK( HandleClose(cx, JL_ARGV[i]) );
	}

#ifdef DEBUG
	for ( i = 0; i < argc; ++i )
		ASSERT( mpv->processEventThreadInfo[i].peSlot == NULL );
	ASSERT( JLSemaphoreAcquire(mpv->processEventSignalEventSem, 0) == JLTIMEOUT ); // else invalid state
#endif // DEBUG

	*JL_RVAL = INT_TO_JSVAL(eventsMask);
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

S_ASSERT( offsetof(UserProcessEvent, pe) == 0 );

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

	JL_IGNORE(obj);

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

	JL_ASSERT_ARGC_RANGE(1, 2);

	unsigned int timeout;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &timeout) );
	if ( JL_ARG_ISDEF(2) )
		JL_ASSERT_ARG_IS_FUNCTION(2);

	UserProcessEvent *upe;
	JL_CHK( HandleCreate(cx, JLHID(pev), sizeof(UserProcessEvent), (void**)&upe, NULL, JL_RVAL) );
	upe->pe.startWait = TimeoutStartWait;
	upe->pe.cancelWait = TimeoutCancelWait;
	upe->pe.endWait = TimeoutEndWait;
	upe->timeout = timeout;
	upe->cancel = JLEventCreate(false);
	ASSERT( JLEventOk(upe->cancel) );

	if ( JL_ARG_ISDEF(2) && JL_ValueIsFunction(cx, JL_ARG(2)) ) {

		SetHandleSlot(cx, *JL_RVAL, 0, JL_ARG(2));
		JL_CHK( SetHandleSlot(cx, *JL_RVAL, 0, JL_ARG(2)) ); // GC protection only
		upe->callbackFunction = JL_ARG(2);
	} else {

		upe->callbackFunction = JSVAL_VOID;
	}

	return JS_TRUE;
	JL_BAD;
}



/*
var obj = {__proto__:null};
Print(typeof obj.__proto__, '\n' ); // -> undefined
var obj2 = Deserialize(Serialize(obj));
Print(typeof obj2.__proto__, '\n' ); // -> object
// cannot use StructuredClone since { __proto__:null } is not supported.

JSObject *ReadStructuredClone(JSContext *cx, JSStructuredCloneReader *r, uint32 tag, uint32 data, void *closure) {

	void *buffer = jl_malloc(data);
	JL_CHK( JS_ReadBytes(r, buffer, data) );

	//JSObject *obj = JL_NewJslibsObject(cx, "Blob");
	jsval blob;
	JL_CHK( JL_NewBlob(cx, buffer, data-1, &blob) ); // -1 : the '\0' terminator is not a part of the data

	return JSVAL_TO_OBJECT(blob);

bad:
	jl_free(buffer);
	return NULL;
}

JSBool WriteStructuredClone(JSContext *cx, JSStructuredCloneWriter *w, JSObject *obj, void *closure) {

	JL_CHK( JS_WriteUint32Pair(w, JS_SCTAG_USER_MIN + 1, 7) );
	JL_CHK( JS_WriteBytes(w, "123456", 7) );
	return JS_TRUE;
	JL_BAD;
}

const JSStructuredCloneCallbacks structuredClone = { ReadStructuredClone, WriteStructuredClone, NULL };


DEFINE_FUNCTION( Serialize ) {

    jsval v = argc > 0 ? JS_ARGV(cx, vp)[0] : JSVAL_VOID;
    uint64 *datap;
    size_t nbytes;
    if (!JS_WriteStructuredClone(cx, v, &datap, &nbytes, &structuredClone, JL_GetHostPrivate(cx)))
        return false;

	JSObject *arrayobj = js_CreateTypedArray(cx, js::TypedArray::TYPE_UINT8, nbytes);
    if (!arrayobj) {
        JS_free(cx, datap);
        return false;
    }
    js::TypedArray *array = js::TypedArray::fromJSObject(arrayobj);
    JS_ASSERT((uintptr_t(array->data) & 7) == 0);
    memcpy(array->data, datap, nbytes);
    JS_free(cx, datap);
    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(arrayobj));
    return true;
}


DEFINE_FUNCTION( Deserialize ) {

    jsval v = argc > 0 ? JS_ARGV(cx, vp)[0] : JSVAL_VOID;
    JSObject *obj;
    if (JSVAL_IS_PRIMITIVE(v) || !js_IsTypedArray((obj = JSVAL_TO_OBJECT(v)))) {

        //JS_ReportErrorNumber(cx, js_GetErrorMessage, NULL, JSSMSG_INVALID_ARGS, "deserialize");
		 JL_ERR( E_DATA, E_INVALID );
        return false;
    }
    js::TypedArray *array = js::TypedArray::fromJSObject(obj);
    if ((array->byteLength & 7) != 0) {

        //JS_ReportErrorNumber(cx, js_GetErrorMessage, NULL, JSSMSG_INVALID_ARGS, "deserialize");
		 JL_ERR( E_DATA, E_INVALID );
        return false;
    }
    if ((uintptr_t(array->data) & 7) != 0) {

        //JS_ReportErrorNumber(cx, js_GetErrorMessage, NULL, JSSMSG_BAD_ALIGNMENT);
		 JL_ERR( E_DATA, E_INVALID );
        return false;
    }

    if (!JS_ReadStructuredClone(cx, (uint64 *) array->data, array->byteLength, JS_STRUCTURED_CLONE_VERSION, &v, &structuredClone, JL_GetHostPrivate(cx))) {

		 return false;
    }
    JS_SET_RVAL(cx, vp, v);
    return true;
	 JL_BAD;
}

*/


#ifdef DEBUG

DEFINE_FUNCTION( _jsapiTests ) {

	JL_IGNORE(cx);
	JL_IGNORE(argc);
	JL_IGNORE(vp);

	///////////////////////////////////////////////////////////////
	// check JL_JsvalToJsid -> JL_JsidToJsval
	//
	JSObject *o = JL_NewObj(cx);
	jsid id;
	jsval s = OBJECT_TO_JSVAL(o);
	JL_CHK( JL_JsvalToJsid(cx, &s, &id) );
	ASSERT( JSID_IS_OBJECT(id) );
	jsval r;
	JL_CHK( JL_JsidToJsval(cx, id, &r) );
	ASSERT( JSVAL_TO_OBJECT(r) == o );

	JL_CHK( JS_ValueToId(cx, OBJECT_TO_JSVAL(o), &id) );
	ASSERT( !JSID_IS_OBJECT(id) );

	JSBool found;
	JL_CHK( JS_DefineProperty(cx, o, "test", JSVAL_ONE, NULL, NULL, JSPROP_PERMANENT) );
	JL_CHK( JS_HasProperty(cx, o, "test", &found) );
	ASSERT( found );

	JSString *jsstr = JS_NewUCStringCopyZ(cx, L"testtesttesttesttesttesttesttesttesttesttesttest");
	jsid pid;
	pid = JL_StringToJsid(cx, jsstr);

	ASSERT( JS_GetParent(cx, JS_NewObject(cx, NULL, NULL, NULL)) != NULL );
	ASSERT( JS_GetParent(cx, JS_NewObjectWithGivenProto(cx, NULL, NULL, NULL)) != NULL );


/*
	JL_CHK( JS_SetPropertyAttributes(cx, o, "test", 0, &found) );

//	jsval ok;
//	jsid pid;
//	pid = JL_StringToJsid(cx, L"test");
//	JS_DeletePropertyById2(cx, o, pid, &ok);

//	JL_RemovePropertyById(cx, o, JL_StringToJsid(cx, L"test"));

	JL_CHK( JS_HasProperty(cx, o, "test", &found) );
	ASSERT( !found );
*/


	return JS_TRUE;
	JL_BAD;
}

#endif // DEBUG





#define JSLANG_TEST DEBUG //|| true

#ifdef JSLANG_TEST


DEFINE_FUNCTION( jslangTest ) {

	JL_IGNORE(cx);
	JL_IGNORE(argc);
	JL_IGNORE(vp);

	JSObject *o = JL_NewObj(cx);
	jsid t;
	jsval s = OBJECT_TO_JSVAL(o);
	JL_CHK( JL_JsvalToJsid(cx, &s, &t) );
	jsval r;
	JL_JsidToJsval(cx, t, &r);
	ASSERT( JSVAL_TO_OBJECT(r) == o );


	return JS_TRUE;
	JL_BAD;
}

#endif // JSLANG_TEST


CONFIGURE_STATIC

//	REVISION(JL_SvnRevToInt("$Revision$")) // avoid to set a revision to the global context
	BEGIN_STATIC_FUNCTION_SPEC

		FUNCTION_ARGC( IsBoolean, 1 )
		FUNCTION_ARGC( IsNumber, 1 )
		FUNCTION_ARGC( IsPrimitive, 1 )
		FUNCTION_ARGC( IsFunction, 1 )
		FUNCTION_ARGC( IsGeneratorFunction, 1 )
		FUNCTION_ARGC( IsGeneratorObject, 1 )


		FUNCTION_ARGC( Real, 1 )

		FUNCTION_ARGC( Stringify, 1 )
		FUNCTION_ARGC( ProcessEvents, 4 ) // (just a guess)
		FUNCTION_ARGC( TimeoutEvents, 2 )

//		FUNCTION_ARGC( Serialize, 1 )
//		FUNCTION_ARGC( Deserialize, 1 )

		#ifdef DEBUG
		FUNCTION( _jsapiTests )
		#endif // DEBUG

		#ifdef JSLANG_TEST
		FUNCTION( jslangTest )
		#endif // JSLANG_TEST

	END_STATIC_FUNCTION_SPEC

END_STATIC
