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
/**qa
	QA.ASSERT( isBoolean(true), true );
	QA.ASSERT( isBoolean(false), true );
	QA.ASSERT( isBoolean(new Boolean(true)), true );
**/
DEFINE_FUNCTION( isBoolean ) {

	JL_ASSERT_ARGC(1);

	if ( JSVAL_IS_BOOLEAN(JL_ARG(1)) ) {

		*JL_RVAL = JSVAL_TRUE;
		return JS_TRUE;
	}

	if ( JSVAL_IS_PRIMITIVE(JL_ARG(1)) ) {

		*JL_RVAL = JSVAL_FALSE;
		return JS_TRUE;
	}

	*JL_RVAL = BOOLEAN_TO_JSVAL( JL_ValueIsBoolean(cx, JL_ARG(1)) );

	return JS_TRUE;
	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME()
  Returns $TRUE if the value is a number value or object.
**/
DEFINE_FUNCTION( isNumber ) {

	JL_ASSERT_ARGC(1);

	if ( JSVAL_IS_NUMBER(JL_ARG(1)) ) {

		*JL_RVAL = JSVAL_TRUE;
		return JS_TRUE;
	}

	if ( JSVAL_IS_PRIMITIVE(JL_ARG(1)) ) {

		*JL_RVAL = JSVAL_FALSE;
		return JS_TRUE;
	}

	*JL_RVAL = BOOLEAN_TO_JSVAL( JL_ValueIsNumber(cx, JL_ARG(1)) );

	return JS_TRUE;
	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME()
  Returns $TRUE if the value is a primitive ( null or not an object ).
**/
DEFINE_FUNCTION( isPrimitive ) {

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
  Returns $TRUE if the value is callable (either a function or a callable object).
  $H example
  {{{
  var md5 = new Hash('md5');

  print( isCallable(md5) ) // prints true
  print( typeof(md5) == 'function' ) // prints false
  
  // md5 can be called even if it is not a function.
  print( hexEncode( md5('foobar') ) ); // prints: 3858F62230AC3C915F300C664312C63F
  }}}
**/
/**qa
	loadModule('jscrypt');
	QA.ASSERT( isCallable(new Hash('md5')), true );
	QA.ASSERT( isCallable(function()0), true );
**/
DEFINE_FUNCTION( isCallable ) {

	JL_ASSERT_ARGC(1);
	*JL_RVAL = BOOLEAN_TO_JSVAL( JL_ValueIsCallable(cx, JL_ARG(1)) );
	return JS_TRUE;
	JL_BAD;
}


/* use gen.isGenerator() instead
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/ **doc
$TOC_MEMBER $INAME
 $BOOL $INAME()
  Returns $TRUE if the value is a generator function.
** /
DEFINE_FUNCTION( isGeneratorFunction ) {

	JL_ASSERT_ARGC(1);
	*JL_RVAL = BOOLEAN_TO_JSVAL( JL_IsGeneratorFunction(cx, JL_ARG(1)) );
	return JS_TRUE;
	JL_BAD;
}
*/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* <jimb>	Franck: Any object that has a 'next' method can act as an iterator.
/ **doc
$TOC_MEMBER $INAME
 $BOOL $INAME()
  Returns $TRUE if the value is a generator instance.
** /
DEFINE_FUNCTION( isGeneratorObject ) {

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
DEFINE_FUNCTION( real ) {

	JL_ASSERT_ARGC(1);

	jsdouble val;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &val) );
	*JL_RVAL = DOUBLE_TO_JSVAL(val);

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( value [, toArrayBuffer] )
  This function converts any value of stream into a string.
  If _toArrayBuffer_ is given and is true, the result is stored into an ArrayBuffer.
  The _value_ argument may be: StreamRead compatible object, any kind of data value (string, TypedArray, ...)
**/
/**qa
	QA.ASSERT_EQ( '==', toString(), undefined );
	QA.ASSERT_EQ( '==', toString(undefined), undefined );
	QA.ASSERT_EQ( '==', toString('str'), 'str' );
	QA.ASSERT_EQ( 'typeof', typeof toString('str', false), 'string' );
	QA.ASSERT_EQ( 'instanceof', toString('str', true), ArrayBuffer );
	QA.ASSERT( toString(toString(toString(toString('abcd'), true)), true)), 'abcd' );
**/
DEFINE_FUNCTION( toString ) {

	if ( JL_ARGC == 1 && JSVAL_IS_STRING(JL_ARG(1)) ) { // identity
		
		*JL_RVAL = JL_ARG(1);
		return JS_TRUE;
	} else 
	if ( JL_ARGC == 0 || (JL_ARGC == 1 && JSVAL_IS_VOID(JL_ARG(1))) ) { // undefined

		*JL_RVAL = JSVAL_VOID;
		return JS_TRUE;
	}

	bool toArrayBuffer;
	if ( JL_ARG_ISDEF(2) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &toArrayBuffer) );
	else
		toArrayBuffer = false;

	if ( !JSVAL_IS_PRIMITIVE(JL_ARG(1)) ) {

		JSObject *sobj = JSVAL_TO_OBJECT( JL_ARG(1) );

		NIStreamRead read = StreamReadInterface(cx, sobj);
		if ( read ) {

			Buffer buf;
			BufferInitialize(&buf, bufferTypeAuto, bufferGrowTypeAuto, NULL, NULL, NULL, NULL);

			size_t length;
			do {

				length = 4096;
				JL_CHKB( read(cx, sobj, BufferNewChunk(&buf, length), &length), bad_ReadInterface_freeBuffer );
				BufferConfirm(&buf, length);
			} while ( length != 0 );

			if ( toArrayBuffer ) {

				JL_CHKB( JL_NewBufferGetOwnership(cx, BufferGetDataOwnership(&buf), BufferGetLength(&buf), JL_RVAL), bad_ReadInterface_freeBuffer );
			} else {

				JSString *jsstr = JS_NewStringCopyN(cx, BufferGetData(&buf), BufferGetLength(&buf));
				JL_CHKB( jsstr, bad_ReadInterface_freeBuffer );
				*JL_RVAL = STRING_TO_JSVAL( jsstr );
			}
			BufferFinalize(&buf);
			return JS_TRUE;

		bad_ReadInterface_freeBuffer:
			BufferFinalize(&buf);
			goto bad;
		}

	}

	// fallback:

	{ //block
	JLData str;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &str) );
	if ( toArrayBuffer )
		JL_CHK( str.GetArrayBuffer(cx, JL_RVAL) );
	else
		JL_CHK( str.GetJSString(cx, JL_RVAL) ); // (TBD) handle Z ?
	}

	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $STR $INAME( collection [, toArrayBuffer] )
 The _collection_ argument is an array-like of data or a generator instance that returns data.
 If _toArrayBuffer_ is given and is true, the result is stored into an ArrayBuffer instead of a string.
**/
/**qa
	QA.ASSERT(join([1,2,3,4,5]), '12345');
	QA.ASSERT(join([toString('abc', true), toString('def', true)]), 'abcdef');
	QA.ASSERT(join([]), '');
**/
DEFINE_FUNCTION( join ) {

	AutoValueVector avr(cx);
	avr.reserve(16);

	jl::Stack<JLData, jl::StaticAllocMedium> strList;
	size_t length = 0;

	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT_ARG_IS_OBJECT(1);

	JSObject *argObj;
	argObj = JSVAL_TO_OBJECT(JL_ARG(1));

	jsval val;

	if ( JL_ObjectIsArrayLike(cx, argObj) ) {

		uint32_t arrayLen;
		JL_CHK( JS_GetArrayLength(cx, argObj, &arrayLen) );
		for ( jsuint i = 0; i < arrayLen; ++i ) {

			JL_CHK( JL_GetElement(cx, argObj, i, &val) );
			JL_CHK( JL_JsvalToNative(cx, val, &*++strList) );
			length += strList->Length();
			avr.append(val);
		}
	} else {

		jsval nextFct;
		JL_CHK( JS_GetPropertyById(cx, argObj, JLID(cx, next), &nextFct) );

		if ( !JL_ValueIsCallable(cx, nextFct) ) {

			jsval tmp = OBJECT_TO_JSVAL(argObj);
			JL_CHK( js_ValueToIterator(cx, JSITER_FOR_OF, &tmp) ); // or maybe call new Ierator(argObj) ???
			JL_ASSERT_IS_OBJECT(tmp, "iterator");
			argObj = JSVAL_TO_OBJECT(tmp);
			JL_CHK( JS_GetPropertyById(cx, argObj, JLID(cx, next), &nextFct) );
			if ( !JL_ValueIsCallable(cx, nextFct) ) {

				JL_ERR(E_ARG, E_NUM(1), E_INVALID);
			}
		}

		while ( JS_CallFunctionValue(cx, argObj, nextFct, 0, NULL, &val) != JS_FALSE ) { // loop until StopIteration or error

			if ( !JL_JsvalToNative(cx, val, &*++strList) ) {

				JL_CHK( js_CloseIterator(cx, argObj) );
				goto bad;
			}
			length += strList->Length();
			avr.append(val);
		}
		JL_CHK( js_CloseIterator(cx, argObj) );
		JL_CHK( JL_IsStopIterationExceptionPending(cx) );
		JS_ClearPendingException(cx);
	}

	bool toArrayBuffer;
	if ( JL_ARG_ISDEF(2) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &toArrayBuffer) );
	else
		toArrayBuffer = false;

	if ( toArrayBuffer ) {
		
		uint8_t *buf = JL_NewBuffer(cx, length, JL_RVAL);
		JL_CHK( buf );
		buf += length;
		while ( strList ) {

			buf -= strList->Length();
			strList->CopyTo(buf);
			--strList;
		}
	} else {

		jschar *buf = (jschar*)JS_malloc(cx, (length +1) * sizeof(jschar));
		buf += length;
		*buf = 0; // required by JL_NewUCString

		while ( strList ) {

			buf -= strList->Length();
			strList->CopyTo(buf);
			--strList;
		}

		JSString *jsstr = JL_NewUCString(cx, buf, length);
		JL_ASSERT( jsstr != NULL, E_VALUE, E_CONVERT, E_TY_STRING );
		*JL_RVAL = STRING_TO_JSVAL(jsstr);
	}

	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $STR $INAME( data, pattern [, startIndex = 0] )
**/
DEFINE_FUNCTION( indexOf ) {

	JLData srcStr, patStr;
	uint32_t start;
	JL_ASSERT_ARGC_MIN(2);

	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &srcStr) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &patStr) );

	if ( JL_ARG_ISDEF(3) ) {

		JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &start) );
		if ( start > srcStr.Length() - patStr.Length() ) {
			
			*JL_RVAL = INT_TO_JSVAL( -1 );
			return JS_TRUE;
		}
	} else {
	
		start = 0;
	}

	if ( srcStr.IsWide() )
		*JL_RVAL = INT_TO_JSVAL( Match(srcStr.GetConstWStr()+start, srcStr.Length()-start, patStr.GetConstWStr(), patStr.Length()) );
	else
		*JL_RVAL = INT_TO_JSVAL( Match(srcStr.GetConstStr()+start, srcStr.Length()-start, patStr.GetConstStr(), patStr.Length()) );

	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $INT $INAME( eventsHandler1 [, ... [, eventsHandler30]] )
  Passive event waiting.
  Returns the bit field that represents the index (in arguments order) of the triggered events. eg. if eventsHandler1 event is triggered, bit 1 is set.
  $H note
   Calling processEvents() with no argument jsut returns 0.
  $H example 1
{{{
loadModule('jsstd');

function onTimeout() {

  print('.');
}

function onEndSignal() {

  print('end signal detected\n');
  throw 0;
}

for (;;) {

  processEvents( timeoutEvents(500, onTimeout), endSignalEvents(onEndSignal) );
}
}}}

  $H example 2
{{{
function mySleep(timeout) {

	processEvents( timeoutEvents(timeout) );
}
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

DEFINE_FUNCTION( processEvents ) {

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
 Passively waits for a timeout through the processEvents function.
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
	JL_CHK( JS_CallFunctionValue(cx, JL_GetGlobal(cx), upe->callbackFunction, 0, NULL, &rval) );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_FUNCTION( timeoutEvents ) {

	JL_ASSERT_ARGC_RANGE(1, 2);

	unsigned int timeout;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &timeout) );
	if ( JL_ARG_ISDEF(2) )
		JL_ASSERT_ARG_IS_CALLABLE(2);

	UserProcessEvent *upe;
	JL_CHK( HandleCreate(cx, JLHID(pev), sizeof(UserProcessEvent), (void**)&upe, NULL, JL_RVAL) );
	upe->pe.startWait = TimeoutStartWait;
	upe->pe.cancelWait = TimeoutCancelWait;
	upe->pe.endWait = TimeoutEndWait;
	upe->timeout = timeout;
	upe->cancel = JLEventCreate(false);
	ASSERT( JLEventOk(upe->cancel) );

	if ( JL_ARG_ISDEF(2) && JL_ValueIsCallable(cx, JL_ARG(2)) ) {

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

JSObject *ReadStructuredClone(JSContext *cx, JSStructuredCloneReader *r, uint32_t tag, uint32_t data, void *closure) {

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


DEFINE_FUNCTION( serialize ) {

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


DEFINE_FUNCTION( deserialize ) {

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



/* serialization/deserialization test using StructuredClone API:
   - fail to serialize {__proto__:null}
   - fail to serialize custom JS-defined objects

JSObject *ReadStructuredClone(JSContext *cx, JSStructuredCloneReader *r, uint32_t tag, uint32_t data, void *closure) {

	void *buf = NULL;
	jschar *name = NULL;
	uint32_t nameLength, bufLength;
	JS_ReadUint32Pair(r, &nameLength, &bufLength);

	name = (jschar*)jl_malloca(nameLength*2);
	JL_ASSERT_ALLOC(name);
	JL_CHK( JS_ReadBytes(r, name, nameLength*2) );

	buf = jl_malloc(bufLength);
	JL_ASSERT_ALLOC(buf);
	JSObject *arrayobj;
	JL_CHK( JL_NewTypedArrayCopyN(cx, buf, bufLength, &arrayobj) );

	jsval constructor;
	JL_CHK( JS_GetUCProperty(cx, JL_GetGlobal(cx), name, nameLength, &constructor) );

	jsval rval, argv;
	argv = OBJECT_TO_JSVAL(arrayobj);
	JL_CHK( JL_CallFunctionId(cx, JSVAL_TO_OBJECT(constructor), JLID(cx, _deserialize), 1, &argv, &rval) );
	
	return JSVAL_TO_OBJECT(rval);

bad:
	if ( name )
		jl_freea(name);
	if ( buf )
		jl_free(buf);
	return NULL;
}


JSBool WriteStructuredClone(JSContext *cx, JSStructuredCloneWriter *w, JSObject *obj, void *closure) {

	JLData str;
	jsval rval, fname;
	JL_CHK( JL_CallFunctionId(cx, obj, JLID(cx, _serialize), 0, NULL, &rval) );

	JL_CHK( JL_JsvalToNative(cx, rval, &str) );

	JSObject *constructor = JL_GetConstructor(cx, obj);
	JL_CHK( constructor );
	JL_CHK( JS_GetPropertyById(cx, constructor, JLID(cx, name), &fname) );

	const jschar *name;
	uint32_t nameLength;
	name = JS_GetStringCharsAndLength(cx, JSVAL_TO_STRING(fname), &nameLength);

	JS_WriteUint32Pair(w, nameLength, str.Length());
	JS_WriteBytes(w, name, nameLength);
	JS_WriteBytes(w, str.GetConstStr(), str.Length());


//	JL_CHK( JS_WriteUint32Pair(w, JS_SCTAG_USER_MIN + 1, 7) );
//	JL_CHK( JS_WriteBytes(w, "123456", 7) );

	return JS_TRUE;
	JL_BAD;
}

const JSStructuredCloneCallbacks structuredCloneCallbacks = { ReadStructuredClone, WriteStructuredClone, NULL };


// source copied from /js/src/js.cpp (Serialize/Deserialize)

DEFINE_FUNCTION( serialize ) {
	
	jsval v = argc > 0 ? JS_ARGV(cx, vp)[0] : JSVAL_VOID;
    uint64_t *datap;
    size_t nbytes;
    if (!JS_WriteStructuredClone(cx, v, &datap, &nbytes, &structuredCloneCallbacks, NULL))
        return false;

	JSObject *arrayobj = js_CreateTypedArray(cx, js::TypedArray::TYPE_UINT8, nbytes);
    if (!arrayobj) {
        JS_free(cx, datap);
        return false;
    }
    JSObject *array = js::TypedArray::getTypedArray(arrayobj);
    JS_ASSERT((uintptr_t(js::TypedArray::getDataOffset(array)) & 7) == 0);
    js_memcpy(js::TypedArray::getDataOffset(array), datap, nbytes);
    JS_free(cx, datap);
    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(arrayobj));
    return true;
}

DEFINE_FUNCTION( deserialize ) {

    jsval v = argc > 0 ? JS_ARGV(cx, vp)[0] : JSVAL_VOID;
    JSObject *obj;
    if (JSVAL_IS_PRIMITIVE(v) || !js_IsTypedArray((obj = JSVAL_TO_OBJECT(v)))) {
        //JS_ReportErrorNumber(cx, my_GetErrorMessage, NULL, JSSMSG_INVALID_ARGS, "deserialize");
		JL_ERR( E_DATA, E_INVALID );
        return false;
    }
    JSObject *array = js::TypedArray::getTypedArray(obj);
    if ((js::TypedArray::getByteLength(array) & 7) != 0) {
        //JS_ReportErrorNumber(cx, my_GetErrorMessage, NULL, JSSMSG_INVALID_ARGS, "deserialize");
		JL_ERR( E_DATA, E_INVALID );
        return false;
    }
    if ((uintptr_t(js::TypedArray::getDataOffset(array)) & 7) != 0) {
        //JS_ReportErrorNumber(cx, my_GetErrorMessage, NULL, JSSMSG_BAD_ALIGNMENT);
		JL_ERR( E_DATA, E_INVALID );
        return false;
    }

    if (!JS_ReadStructuredClone(cx, (uint64_t *) js::TypedArray::getDataOffset(array), js::TypedArray::getByteLength(array), JS_STRUCTURED_CLONE_VERSION, &v, &structuredCloneCallbacks, NULL)) {

        return false;
    }
    JS_SET_RVAL(cx, vp, v);
    return true;
bad:
	return false;
}

*/


#ifdef DEBUG

DEFINE_FUNCTION( _jsapiTests ) {

	JL_IGNORE(cx, argc, vp);

	///////////////////////////////////////////////////////////////
	// check JL_JsvalToJsid -> JL_JsidToJsval
	//
	JSObject *o = JL_NewObj(cx);
	jsid id;
	jsval s = OBJECT_TO_JSVAL(o);
	ASSERT( JL_JsvalToJsid(cx, s, &id) );
	ASSERT( JSID_IS_OBJECT(id) );
	jsval r;
	ASSERT( JL_JsidToJsval(cx, id, &r) );
	ASSERT( JSVAL_TO_OBJECT(r) == o );

	ASSERT( JS_ValueToId(cx, OBJECT_TO_JSVAL(o), &id) );
	ASSERT( !JSID_IS_OBJECT(id) );

	JSBool found;
	ASSERT( JS_DefineProperty(cx, o, "test", JSVAL_ONE, NULL, NULL, JSPROP_PERMANENT) );
	ASSERT( JS_HasProperty(cx, o, "test", &found) );
	ASSERT( found );

	JSString *jsstr = JS_NewUCStringCopyZ(cx, L"testtesttesttesttesttesttesttesttesttesttesttest");
	jsid pid;
	pid = JL_StringToJsid(cx, jsstr);

	ASSERT( JL_JsvalToJsid(cx, OBJECT_TO_JSVAL(JS_NewObject(cx, NULL, NULL, NULL)), &id) );
	ASSERT( JSID_IS_OBJECT(id) );


//	// see Bug 688510
	ASSERT( JS_GetParent(JS_NewObject(cx, NULL, NULL, NULL)) != NULL );
//	ASSERT( JS_GetParent(JL_NewObjectWithGivenProto(cx, NULL, NULL, NULL)) != NULL );


	JS_ThrowStopIteration(cx);
	ASSERT( JL_IsStopIterationExceptionPending(cx) );
	JS_ClearPendingException(cx);


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


/*
struct FOO {
	explicit FOO( const FOO &b ) {} // non-const copy constructor
	FOO& operator *() { return *this; }
	FOO( int i ) {}
}; // FOO a(FOO(5));
*/


DEFINE_FUNCTION( jslangTest ) {

	JL_IGNORE(cx, argc, vp);

	return 0;
}

#endif // JSLANG_TEST


CONFIGURE_STATIC

//	REVISION(JL_SvnRevToInt("$Revision$")) // avoid to set a revision to the global context
	BEGIN_STATIC_FUNCTION_SPEC

		FUNCTION_ARGC( isBoolean, 1 )
		FUNCTION_ARGC( isNumber, 1 )
		FUNCTION_ARGC( isPrimitive, 1 )
		FUNCTION_ARGC( isCallable, 1 )
//		FUNCTION_ARGC( isGeneratorFunction, 1 )
//		FUNCTION_ARGC( isGeneratorObject, 1 )

		FUNCTION_ARGC( join, 1 )


		FUNCTION_ARGC( real, 1 )

		FUNCTION_ARGC( toString, 1 )
		FUNCTION_ARGC( indexOf, 1 )
		FUNCTION_ARGC( processEvents, 4 ) // (4 is just a guess)
		FUNCTION_ARGC( timeoutEvents, 2 )

		#ifdef DEBUG
		FUNCTION( _jsapiTests )
		#endif // DEBUG

		#ifdef JSLANG_TEST
		FUNCTION( jslangTest )
		#endif // JSLANG_TEST

	END_STATIC_FUNCTION_SPEC

END_STATIC
