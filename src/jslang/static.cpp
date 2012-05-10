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

ADD_DOC(isBoolean, "boolean isBoolean(value)", "returns true if value is a boolean or a boolean object");

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
/**qa
	QA.ASSERTOP( isPrimitive(true), '==', true );
	QA.ASSERTOP( isPrimitive(false), '==', true );
	QA.ASSERTOP( isPrimitive(1), '==', true );
	QA.ASSERTOP( isPrimitive('a'), '==', true );
	QA.ASSERTOP( isPrimitive(1.23), '==', true );
	QA.ASSERTOP( isPrimitive(null), '==', true );
	QA.ASSERTOP( isPrimitive(undefined), '==', true );
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


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( value )
  This function converts any value into a floating point value.
**/
DEFINE_FUNCTION( real ) {

	JL_ASSERT_ARGC(1);

	double val;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &val) );
	*JL_RVAL = DOUBLE_TO_JSVAL(val);

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( value [, toArrayBuffer] )
  This function converts any value or stream into a string.
  If _toArrayBuffer_ is given and is true, the result is stored into an ArrayBuffer.
  The _value_ argument may be: StreamRead compatible object, any kind of data value (string, TypedArray, ...)
**/
/**qa
	QA.ASSERTOP( stringify(), '==', undefined );
	QA.ASSERTOP( stringify(undefined), '==', undefined );
	QA.ASSERTOP( stringify('str'), '==', 'str' );
	QA.ASSERTOP( stringify('str', false), 'typeof', 'string' );
	QA.ASSERTOP( stringify('str', true), 'instanceof', ArrayBuffer );
	QA.ASSERTOP( stringify(stringify(stringify(stringify(stringify('abcd'), true)), true)), '===', 'abcd' );
	QA.ASSERTOP( stringify(Uint16Array(stringify('\x00\x01\x02\x03', true))), '===', '\u0100\u0302' ); // endian issue ?
**/

ADD_DOC(stringify, "string|ArrayBuffer stringify(value [, toArrayBuffer])", "convert a value to a string or an ArrayBuffer");

DEFINE_FUNCTION( stringify ) {

	JLData str;

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

			jl::Buf<char> buf;

			size_t length;
			do {

				length = 4096;
				buf.Reserve(4096);
				JL_CHK( read(cx, sobj, buf.Ptr(), &length) );
				buf.Advance(length);
			} while ( length != 0 );

			if ( toArrayBuffer ) {

				JL_CHK( JL_NewBufferGetOwnership(cx, buf.GetDataOwnership(), buf.Length(), JL_RVAL) );
			} else {

				JSString *jsstr = JS_NewStringCopyN(cx, buf.GetData(), buf.Length());
				JL_CHK( jsstr );
				*JL_RVAL = STRING_TO_JSVAL( jsstr );
			}

			return JS_TRUE;
		}

	}

	// fallback:
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &str) );
	JL_CHK( toArrayBuffer ? str.GetArrayBuffer(cx, JL_RVAL) : str.GetJSString(cx, JL_RVAL) );

	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $STR $INAME( iterableObject [, toArrayBuffer] )
  Joins data generated by iterating the _iterableObject_ value.
 The _iterableObject_ may be arrays, array-like objects, iterators and generators.
 If _toArrayBuffer_ is given and is true, the result is stored into an ArrayBuffer instead of a string.
**/
/**qa
	QA.ASSERT(join([1,2,3,4,5]), '12345');
	QA.ASSERT(join([stringify('abc', true), stringify('def', true)]), 'abcdef');
	QA.ASSERT(join([]), '');
	QA.ASSERT(join(Iterator([0,0,0], true)), '012');
	QA.ASSERT(join((function() {yield ''; yield 'a'; yield 'bc'; yield ''; yield 'def'; yield 'g' })()) , 'abcdefg');
	
	QA.ASSERTOP( join((function() {yield})()), '===', 'undefined');
	
	var a = 'abcd';
	var it = {
		next: function() {
			delete this.next; // has no effect
			a = a.substr(1);
			if ( !a )
				throw StopIteration;
			return a[0];
		}
	};
	QA.ASSERTOP( join(it), '==', 'bcd');
	QA.ASSERTOP( stringify(stringify('123456789', true).slice(3)), '==', '456789');
**/
ADD_DOC( join, "string|ArrayBuffer join(iterableObject [,toArrayBuffer])", "joins data" )
DEFINE_FUNCTION( join ) {

	js::AutoValueVector avr(cx);
	avr.reserve(16);

	jl::Stack<JLData, jl::StaticAllocMedium> strList;
	size_t length = 0;

	jsval val;

	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT_ARG_IS_OBJECT(1);

	JSObject *argObj;
	argObj = JSVAL_TO_OBJECT(JL_ARG(1));

	if ( JL_ObjectIsArrayLike(cx, argObj) ) {

		uint32_t arrayLen;
		JL_CHK( JS_GetArrayLength(cx, argObj, &arrayLen) );
		for ( unsigned i = 0; i < arrayLen; ++i ) {

			JL_CHK( JL_GetElement(cx, argObj, i, &val) );
			JL_CHK( JL_JsvalToNative(cx, val, &*++strList) );
			length += strList->Length();
			avr.append(val);
		}
	} else {

		jsval nextFct;
		JL_CHK( JS_GetPropertyById(cx, argObj, JLID(cx, next), &nextFct) );
		JL_ASSERT_IS_CALLABLE(nextFct, "iterator");
		while ( JS_CallFunctionValue(cx, argObj, nextFct, 0, NULL, &val) != JS_FALSE ) { // loop until StopIteration or error

			JL_CHK( JL_JsvalToNative(cx, val, &*++strList) );
			length += strList->Length();
			avr.append(val);
		}
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

ADD_DOC( indexOf, "string|ArrayBuffer indexOf(data, pattern [,startIndex = 0])", "" );
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
		*JL_RVAL = INT_TO_JSVAL( jl::Match(srcStr.GetConstWStr()+start, srcStr.Length()-start, patStr.GetConstWStr(), patStr.Length()) );
	else
		*JL_RVAL = INT_TO_JSVAL( jl::Match(srcStr.GetConstStr()+start, srcStr.Length()-start, patStr.GetConstStr(), patStr.Length()) );

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

  processEvents( timeoutEvents(500, onTimeout), host.endSignalEvents(onEndSignal) );
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
		
		*JL_RVAL = JSVAL_ZERO;
		return JS_TRUE;
	}

	// fill peList slots
	unsigned int i;
	for ( i = 0; i < argc; ++i ) {

		JL_ASSERT_ARG_TYPE( IsHandle(cx, JL_ARGV[i]), i+1, "(pev) Handle" );
		JL_ASSERT_ARG_TYPE( IsHandleType(cx, JSVAL_TO_OBJECT(JL_ARGV[i]), jl::CastCStrToUint32("pev")), i+1, "(pev) Handle" );
		ProcessEvent *pe = (ProcessEvent*)GetHandlePrivate(cx, JL_ARGV[i]);
		JL_ASSERT( pe != NULL, E_ARG, E_NUM(i+1), E_STATE ); //JL_ASSERT( pe != NULL, E_ARG, E_NUM(i+1), E_ANINVALID, E_NAME("pev Handle") );

		ASSERT( pe->prepareWait );
		ASSERT( pe->startWait );
		ASSERT( pe->cancelWait );
		ASSERT( pe->endWait );

		JL_CHK( pe->prepareWait(pe, cx, JSVAL_TO_OBJECT(JL_ARGV[i])) );

		peList[i] = pe;
	}

	// prepare threads
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

	// wait !
	JLSemaphoreAcquire(mpv->processEventSignalEventSem, JLINFINITE); // wait for an event (timeout can also be managed here)
	JLSemaphoreRelease(mpv->processEventSignalEventSem);

	// cancel other waits
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

	int32_t eventsMask;
	eventsMask = 0;
	bool hasEvent;
	JSBool ok;
	ok = JS_TRUE;

	// notify waiters
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

//		JL_CHK( HandleClose(cx, JL_ARGV[i]) ); // (TBD) recycle items instead of closing them
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
/**qa
	QA.ASSERTOP( timeoutEvents(0), 'instanceof', Handle );
	QA.ASSERTOP( timeoutEvents(0), '==', '[Handle  pev]' );
	QA.ASSERTOP( processEvents(timeoutEvents(0)), '===', 1 );
	QA.ASSERTOP( function() timeoutEvents(undefined), 'ex', RangeError );
	QA.ASSERTOP( function() timeoutEvents(-1), 'ex', RangeError );
	var d = Date.now();
	processEvents(timeoutEvents(123));
	d = Date.now() - d;
	QA.ASSERTOP( d, '>=', 123 -5); // error margin
	QA.ASSERTOP( d, '<=', 123 +10); // error margin
**/
struct TimeoutProcessEvent {
	ProcessEvent pe;
	unsigned int timeout;
	JLEventHandler cancel;
	bool canceled;
	jsval callbackFunction;
	JSObject *callbackFunctionThis;
};

S_ASSERT( offsetof(TimeoutProcessEvent, pe) == 0 );

static JSBool TimeoutPrepareWait( volatile ProcessEvent *, JSContext *, JSObject * ) {
	
	return JS_TRUE;
}

static void TimeoutStartWait( volatile ProcessEvent *pe ) {

	TimeoutProcessEvent *upe = (TimeoutProcessEvent*)pe;

	if ( upe->timeout > 0 ) {

		int st = JLEventWait(upe->cancel, upe->timeout);
		upe->canceled = (st == JLOK);
	} else {

		upe->canceled = false;
	}
}

static bool TimeoutCancelWait( volatile ProcessEvent *pe ) {

	TimeoutProcessEvent *upe = (TimeoutProcessEvent*)pe;

	JLEventTrigger(upe->cancel);
	return true;
}

static JSBool TimeoutEndWait( volatile ProcessEvent *pe, bool *hasEvent, JSContext *cx, JSObject *obj ) {

	JL_IGNORE(obj);

	TimeoutProcessEvent *upe = (TimeoutProcessEvent*)pe;

	*hasEvent = !upe->canceled;

	// not triggered, then nothing to reset
	if ( !*hasEvent )
		return JS_TRUE;

	// reset for further use.
	JLEventReset(upe->cancel);
	upe->canceled = false;

	if ( JSVAL_IS_VOID( upe->callbackFunction ) )
		return JS_TRUE;

	jsval rval;
	JL_CHK( JL_CallFunctionVA(cx, upe->callbackFunctionThis, upe->callbackFunction, &rval) );

	return JS_TRUE;
	JL_BAD;
}

static void TimeoutFinalize( void* data ) {
	
	TimeoutProcessEvent *upe = (TimeoutProcessEvent*)data;
	JLEventFree(&upe->cancel);
}


DEFINE_FUNCTION( timeoutEvents ) {

	JL_ASSERT_ARGC_RANGE(1, 2);

	uint32_t timeout;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &timeout) );

	TimeoutProcessEvent *upe;

	JL_CHK( HandleCreate(cx, JLHID(pev), &upe, TimeoutFinalize, JL_RVAL) );

	upe->pe.prepareWait = TimeoutPrepareWait;
	upe->pe.startWait = TimeoutStartWait;
	upe->pe.cancelWait = TimeoutCancelWait;
	upe->pe.endWait = TimeoutEndWait;

	upe->timeout = timeout;
	upe->cancel = JLEventCreate(false);
	ASSERT( JLEventOk(upe->cancel) );

	if ( JL_ARG_ISDEF(2) ) {

		JL_ASSERT_ARG_IS_CALLABLE(2);

		JL_CHK( SetHandleSlot(cx, *JL_RVAL, 0, JL_OBJVAL) ); // GC protection only
		JL_CHK( SetHandleSlot(cx, *JL_RVAL, 1, JL_ARG(2)) ); // GC protection only

		upe->callbackFunctionThis = JSVAL_TO_OBJECT(JL_OBJVAL); // store "this" object.
		upe->callbackFunction = JL_ARG(2); // access to ->callbackFunction is faster than Handle slots.
	} else {

		upe->callbackFunction = JSVAL_VOID;
	}

	return JS_TRUE;
	JL_BAD;
}


#if defined(DEBUG) // || 1
#define HAS_JL_API_TESTS
#endif


#ifdef HAS_JL_API_TESTS

#if !DEBUG
#pragma message ( "WARNING: test API available in non-debug mode !" )
#endif

#define TEST(expr) \
	( (expr) ? (void)0 : JL_AssertFailure(#expr, JL_CODE_LOCATION) )


/**qa
	if ( typeof _jsapiTests != 'undefined' )
		_jsapiTests();
**/

DEFINE_FUNCTION( _jsapiTests ) {

	JL_IGNORE(cx, argc, vp);

	// allocators ////////////////////////////////////////////////////

	void *tmp1 = jl_malloc(100000);
	js_free(tmp1);

	void *tmp2 = js_malloc(100000);
	jl_free(tmp2);


	/////////////////////////////////////////////////////////////////

	// Data issues //////////////////////////////////////////////////

	JLData dat1("0123456789", false, 10);
	
	const char *sd1 = dat1.GetConstStrZ();
	const char *sd2 = dat1.GetConstStrZ();

	TEST( sd1 == sd2 );

	/////////////////////////////////////////////////////////////////

	// Data issues 2 ////////////////////////////////////////////////
	{
	JLData path;
	jsval tmp;
	tmp = JSVAL_ONE;
	JL_CHK( JL_JsvalToNative(cx, tmp, &path) );
	const char *d1 = path.GetConstStrZ();
	const char *d2 = path.GetConstStrZ();
	TEST( d1 == d2 );
	}

	/////////////////////////////////////////////////////////////////

	TEST( JSVAL_IS_PRIMITIVE(JSVAL_NULL) );

	///////////////////////////////////////////////////////////////
	// check JL_JsvalToJsid -> JL_JsidToJsval
	//
	JSObject *o = JL_NewObj(cx);
	jsid id;
	jsval s;
	s = OBJECT_TO_JSVAL(o);
	TEST( JL_JsvalToJsid(cx, s, &id) );
	TEST( JSID_IS_OBJECT(id) );
	jsval r;
	TEST( JL_JsidToJsval(cx, id, &r) );
	TEST( JSVAL_TO_OBJECT(r) == o );

	TEST( JS_ValueToId(cx, OBJECT_TO_JSVAL(o), &id) );
	TEST( !JSID_IS_OBJECT(id) );

	JSBool found;
	TEST( JS_DefineProperty(cx, o, "test", JSVAL_ONE, NULL, NULL, JSPROP_PERMANENT) );
	TEST( JS_HasProperty(cx, o, "test", &found) );
	TEST( found );

	JSString *jsstr = JS_NewUCStringCopyZ(cx, L("testtesttesttesttesttesttesttesttesttesttesttest"));
	jsid pid;
	pid = JL_StringToJsid(cx, jsstr);

	TEST( JL_JsvalToJsid(cx, OBJECT_TO_JSVAL(JS_NewObject(cx, NULL, NULL, NULL)), &id) );
	TEST( JSID_IS_OBJECT(id) );


//	// see Bug 688510
	TEST( JS_GetParent(JS_NewObject(cx, NULL, NULL, NULL)) != NULL );
//	TEST( JS_GetParent(JL_NewObjectWithGivenProto(cx, NULL, NULL, NULL)) != NULL );


	JS_ThrowStopIteration(cx);
	TEST( JL_IsStopIterationExceptionPending(cx) );
	JS_ClearPendingException(cx);

/*
	JL_CHK( JS_SetPropertyAttributes(cx, o, "test", 0, &found) );

//	jsval ok;
//	jsid pid;
//	pid = JL_StringToJsid(cx, L("test"));
//	JS_DeletePropertyById2(cx, o, pid, &ok);

//	JL_RemovePropertyById(cx, o, JL_StringToJsid(cx, L("test")));

	JL_CHK( JS_HasProperty(cx, o, "test", &found) );
	TEST( !found );
*/

	// jl::Stack ////////////////////////////////////////////////////
	{
	static int STest_count = 0;
	struct STest {
		~STest() { STest_count++; }
	};
	jl::Stack<STest> stack;
	++stack;
	--stack;
	TEST( STest_count == 1 ); // test if the destructor of STest is called properly
	STest_count = 0;
	}
	/////////////////////////////////////////////////////////////////


	// Buffer ////////////////////////////////////////////////////

	{
	char *ref = (char*)jl_malloc(20000);
	for ( int i = 0; i < 20000; i++ )
		ref[i] = rand() & 0xff; // 0->255
	int refPos = 0;

	jl::Buf<char> b;

	for ( int i = 0; i < 100; i++ ) {

		int rnd = rand() & 0xff; // 0->127
		b.Reserve(rnd);
		char *tmp = b.Ptr();
		jl::memcpy(tmp, ref + refPos, rnd);
		b.Advance(rnd);
		refPos += rnd;
	}

	int l = b.Length();

	char *tmp = (char*)jl_malloc(l);
	b.CopyTo(tmp, l);

	const char *d = b.GetData();
	bool success = memcmp(ref, d, l) == 0 && memcmp(ref, tmp, l) == 0;
	TEST( success );

	jl_free(tmp);
	jl_free(ref);
	}


	/////////////////////////////////////////////////////////////////

	// Buffer (2) ////////////////////////////////////////////////////

	{
	jl::Buf<char> resultBuffer;
	resultBuffer.Reserve(10000);
	resultBuffer.Advance(10000);
	jl_free( resultBuffer.GetDataOwnership() );
	}

	/////////////////////////////////////////////////////////////////

	// JLData ///////////////////////////////////////////////////////
	{
	JLData str1("aé\x10\x1B\xFF", 1); // 97 130
	TEST( str1.GetCharAt(0) == 'a' );
	TEST( str1.GetCharAt(1) == 'é' );

	JLData str2(L("aé\x10\x1B\xFF"), 1);
	TEST( str1.GetWCharAt(0) == L('a') );
	TEST( str1.GetWCharAt(1) == L('é') );

	jschar s1[100];
	str1.CopyTo(s1);
	s1[str1.Length()] = 0;
	TEST( !wcscmp(s1, L("aé\x10\x1B\xFF") ) );

	char s2[100];
	str2.CopyTo(s2);
	s2[str2.Length()] = 0;
	TEST( !strcmp(s2, "aé\x10\x1B\xFF" ) );

	TEST( !wcscmp(str1.GetConstWStrZ(), L("aé\x10\x1B\xFF") ) );
	TEST( !strcmp(str2.GetConstStrZ(), "aé\x10\x1B\xFF" ) );
	}
	/////////////////////////////////////////////////////////////////

	// misc inlining ////////////////////////////////////////////////

	static JSObject *obj;
	size_t a = jl::GetEIP();
	{
	obj = JL_CLASS_PROTOTYPE(cx, Handle);
	JL_IGNORE(obj);
	}
	a = jl::GetEIP() - a;
	TEST( a < 200 );
	//printf("%d\n", a); exit(0);

	/////////////////////////////////////////////////////////////////

	// misc ////////////////////////////////////////////////

//	js::ArrayBuffer::create(cx, 0, NULL);
//	OBJECT_TO_JSVAL(NULL);

	/////////////////////////////////////////////////////////////////

	// ptr -> native -> ptr ///////////////////////////////////////////////////////////////

	{
	void *ptr = (void*)0; // 00...00
	
	jsval v;
	void *tmp = ptr;
	JL_CHK( JL_NativeToJsval(cx, tmp, &v) );
	JL_CHK( JL_JsvalToNative(cx, v, &tmp) );
	TEST( tmp == ptr );
	}

	{
	void *ptr = (void*)1; // 00...01
	
	jsval v;
	void *tmp = ptr;
	JL_CHK( JL_NativeToJsval(cx, tmp, &v) );
	JL_CHK( JL_JsvalToNative(cx, v, &tmp) );
	TEST( tmp == ptr );
	}

	{
	void *ptr = (void*)-1; // ff...ff
	
	jsval v;
	void *tmp = ptr;
	JL_CHK( JL_NativeToJsval(cx, tmp, &v) );
	JL_CHK( JL_JsvalToNative(cx, v, &tmp) );
	TEST( tmp == ptr );
	}

	{
	void *ptr = (void*)(size_t((void*)-1)-1); // ff...fe
	
	jsval v;
	void *tmp = ptr;
	JL_CHK( JL_NativeToJsval(cx, tmp, &v) );
	JL_CHK( JL_JsvalToNative(cx, v, &tmp) );
	TEST( tmp == ptr );
	}

	{
	void *ptr = (void*)(size_t((void*)-1) >> 1); // 7f...ff
	
	jsval v;
	void *tmp = ptr;
	JL_CHK( JL_NativeToJsval(cx, tmp, &v) );
	JL_CHK( JL_JsvalToNative(cx, v, &tmp) );
	TEST( tmp == ptr );
	}

	{
	void *ptr = (void*)((size_t((void*)-1)>>1) - 1); // 7f...fe
	
	jsval v;
	void *tmp = ptr;
	JL_CHK( JL_NativeToJsval(cx, tmp, &v) );
	JL_CHK( JL_JsvalToNative(cx, v, &tmp) );
	TEST( tmp == ptr );
	}

	/////////////////////////////////////////////////////////////////

	// JLID ///////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////

	return JS_TRUE;
	JL_BAD;
}

#undef TEST

#endif // HAS_JL_API_TESTS




#if defined(DEBUG) // || 1
#define JSLANG_TEST 
#endif

#ifdef JSLANG_TEST

INLINE NEVER_INLINE void
testFct( JSContext *cx ) {
	
	JL_IGNORE(cx);

	static jsval tmp = JSVAL_ONE;
/* 2000
	static JSObject *o = JS_NewArrayObject(cx, 0, NULL);
	JS_SetElement(cx, o, 1, &tmp); JS_SetElement(cx, o, 2, &tmp); JS_SetElement(cx, o, 3, &tmp); JS_SetElement(cx, o, 4, &tmp); JS_SetElement(cx, o, 5, &tmp); JS_SetElement(cx, o, 6, &tmp); JS_SetElement(cx, o, 7, &tmp); JS_SetElement(cx, o, 8, &tmp); JS_SetElement(cx, o, 9, &tmp); JS_SetElement(cx, o, 10, &tmp); JS_SetElement(cx, o, 11, &tmp); JS_SetElement(cx, o, 12, &tmp); JS_SetElement(cx, o, 13, &tmp); JS_SetElement(cx, o, 14, &tmp); JS_SetElement(cx, o, 15, &tmp); JS_SetElement(cx, o, 16, &tmp);
*/

/* 750
	static AutoValueVector avr(cx);
	avr.append(tmp); avr.append(tmp); avr.append(tmp); avr.append(tmp); avr.append(tmp); avr.append(tmp); avr.append(tmp); avr.append(tmp); avr.append(tmp); avr.append(tmp); avr.append(tmp); avr.append(tmp); avr.append(tmp); avr.append(tmp); avr.append(tmp); avr.append(tmp);
*/

//	malloc(1);

}


struct TestIf {
	
	virtual int fct(int i) = 0;
};


struct Test1 : public TestIf {

	int fct(int i) {
		return i * 123;
	}
};


DEFINE_FUNCTION( jslangTest ) {
	
	JL_IGNORE(cx, argc, vp);

/*
	static uint64_t xx[2];

	__asm { int 3 }
	__asm { int 3 }
	__asm { int 3 }

	xx[1] = DOUBLE_TO_JSVAL(123).asRawBits();
	xx[2] = DOUBLE_TO_JSVAL(456).asRawBits();

	__asm { int 3 }
	__asm { int 3 }
	__asm { int 3 }

	JL_IGNORE(xx);
*/
	

/*
	JL_CHK( ::SetThreadAffinityMask(GetCurrentThread(), 1) );
	JL_CHK( ::SetProcessPriorityBoost(GetCurrentProcess(), TRUE) ); // disable dynamic boosting
	JL_CHK( ::SetPriorityClass(::GetCurrentProcess(), REALTIME_PRIORITY_CLASS) );
	JL_CHK( ::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL) );

	// (TBD) try to get the 'agruments' variable instead of using rootedValues ?
	// (TBD) use AutoValueVector avr(cx); avr.reserve(16); avr.append(val);

	unsigned __int64 count, min = UINT64_MAX, sum = 0;

	int i = 0;
	for (; i < 10000; ++i ) {

		count = rdtsc();
		testFct(cx);
		testFct(cx);
		testFct(cx);
		testFct(cx);
		testFct(cx);
		testFct(cx);
		testFct(cx);
		testFct(cx);
		count = (rdtsc() - count) / 8;
		sum += count;
		min = JL_MIN(min, count);
	}

	printf("min cycles: %I64d\n", min);
	printf("avg cycles: %I64d\n", sum/i);

	exit(0);
*/


/*
	size_t a = JLGetEIP();
	//
	a = JLGetEIP() - a;
	printf("res: %d\n", a); exit(0);



	JSObject *o = JSVAL_TO_OBJECT(*vp);

	static const void *tmp;

	size_t a = JLGetEIP();

	tmp = JL_GetCachedClassProto(JL_GetHostPrivate(cx), "Handle");
	
	a = JLGetEIP() - a;
	
	printf("%d\n", a); exit(0);
*/


/*
	//jsval constructor;
	//JS_GetProperty(cx, JS_GetGlobalObject(cx), "SyntaxError", &constructor);
	
	JSObject *proto;
	js_GetClassPrototype(cx, JS_GetGlobalObject(cx), JSProto_Error, &proto, NULL);

	JSClass *cl = JS_GetClass(proto);
*/

/*
	jsval constructor, val;

	JL_CHK( JS_GetProperty(cx, JL_GetGlobal(cx), "SyntaxError", &constructor) );
	JSObject *errorObj = JS_NewObjectForConstructor(cx, &constructor);
	//JSObject *errorObj = JS_New(cx, JSVAL_TO_OBJECT(constructor), 0, NULL);
	val = OBJECT_TO_JSVAL(errorObj);
*/



/*
	const jschar *name = L("SyntaxError");

	jsval constructor;
	JL_CHK( JS_GetUCProperty(cx, JL_GetGlobal(cx), name, wcslen(name), &constructor) );

	//JSObject *test = JS_GetConstructor(cx, JSVAL_TO_OBJECT(constructor));

	JSObject *proto = JS_GetPrototype(JSVAL_TO_OBJECT(constructor));

	JSObject *ob = JS_NewObject(cx, NULL, proto, NULL);

	JSClass *cl = JS_GetClass(ob);
	//JSObject *errorObj = JS_NewObjectForConstructor(cx, &constructor);
*/

	JL_BAD;
}

#endif // JSLANG_TEST



/**qa
	if ( '_jsapiTests' in global )
		_jsapiTests();

	QA.ASSERTOP( NaN, '==', NaN ); // test qa.js
	QA.ASSERTOP( NaN, '===', NaN );  // test qa.js
	QA.ASSERTOP( 1, '!=', NaN ); // test qa.js
	QA.ASSERTOP( 1, '!==', NaN );  // test qa.js

	QA.ASSERTOP( Handle._buildDate, '>', 0 );

	QA.ASSERTOP( Handle, 'has', 'prototype' );
	QA.ASSERTOP( Handle, 'has', 'constructor' );

	QA.ASSERTOP( timeoutEvents(1), 'instanceof', Handle );
	QA.ASSERTOP( timeoutEvents(1), 'has', '__proto__' );
	QA.ASSERTOP( timeoutEvents(1), 'has', 'constructor' );
**/

CONFIGURE_STATIC

//	REVISION(jl::SvnRevToInt("$Revision$")) // avoid to set a sourceId property to the global context.
	BEGIN_STATIC_FUNCTION_SPEC

		FUNCTION_ARGC( isBoolean, 1 )

		FUNCTION_ARGC( isNumber, 1 )
		FUNCTION_ARGC( isPrimitive, 1 )
		FUNCTION_ARGC( isCallable, 1 )
		
		FUNCTION_ARGC( real, 1 )

		FUNCTION_ARGC( stringify, 2 )
		FUNCTION_ARGC( join, 2 )
		FUNCTION_ARGC( indexOf, 3 )

		FUNCTION_ARGC( processEvents, 4 ) // (4 is just a guess)
		FUNCTION_ARGC( timeoutEvents, 2 )

		#ifdef HAS_JL_API_TESTS
		FUNCTION( _jsapiTests )
		#endif

		#ifdef JSLANG_TEST
		FUNCTION( jslangTest )
		#endif

	END_STATIC_FUNCTION_SPEC

END_STATIC
