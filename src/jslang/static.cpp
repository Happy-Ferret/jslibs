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

ADD_DOC(isCallable, "boolean isCallable(value)", "returns true if the value can be called like a function");

DEFINE_FUNCTION( isCallable ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC(1);

	JL_RVAL.setBoolean( JL_ValueIsCallable(cx, JL_ARG(1)) );
	return true;
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

	JL_DEFINE_ARGS;
	JLData str;

	if ( JL_ARGC == 1 && JSVAL_IS_STRING(JL_ARG(1)) ) { // identity
		
		JL_RVAL.set(JL_ARG(1));
		return true;
	} else 
	if ( JL_ARGC == 0 || (JL_ARGC == 1 && JL_ARG(1).isUndefined()) ) { // undefined

		JL_RVAL.setUndefined();
		return true;
	}

	bool toArrayBuffer;
	if ( JL_ARG_ISDEF(2) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &toArrayBuffer) );
	else
		toArrayBuffer = false;

	if ( !JSVAL_IS_PRIMITIVE(JL_ARG(1)) ) {

		JS::RootedObject sobj(cx, &JL_ARG(1).toObject() );

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
				JL_RVAL.setString(jsstr);
			}

			return true;
		}

	}

	// fallback:
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &str) );
	JL_CHK( toArrayBuffer ? str.GetArrayBuffer(cx, JL_RVAL) : str.GetJSString(cx, JL_RVAL) );

	return true;
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

	JL_DEFINE_ARGS;

	JS::AutoValueVector avr(cx);
	avr.reserve(16);

	jl::Stack<JLData, jl::StaticAllocMedium> strList;
	size_t length = 0;

	JS::RootedValue val(cx);

	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT_ARG_IS_OBJECT(1);

	{

	JS::RootedObject argObj(cx, &JL_ARG(1).toObject());

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

		JS::RootedValue nextFct(cx);
		JL_CHK( JS_GetPropertyById(cx, argObj, JLID(cx, next), &nextFct) );
		JL_ASSERT_IS_CALLABLE(nextFct, "iterator");
		while ( JS_CallFunctionValue(cx, argObj, nextFct, 0, NULL, val.address()) != false ) { // loop until StopIteration or error

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
		JL_RVAL.setString(jsstr);
	}

	}

	return true;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $STR $INAME( data, pattern [, startIndex = 0] )
**/

ADD_DOC( indexOf, "string|ArrayBuffer indexOf(data, pattern [,startIndex = 0])", "" );
DEFINE_FUNCTION( indexOf ) {

	JL_DEFINE_ARGS;

	JLData srcStr, patStr;
	uint32_t start;
	
	JL_ASSERT_ARGC_MIN(2);

	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &srcStr) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &patStr) );

	if ( JL_ARG_ISDEF(3) ) {

		JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &start) );
		if ( start > srcStr.Length() - patStr.Length() ) {
			
			JL_RVAL.setInt32(-1);
			return true;
		}
	} else {
	
		start = 0;
	}

	if ( srcStr.IsWide() )
		JL_RVAL.setInt32( jl::Match(srcStr.GetConstWStr()+start, srcStr.Length()-start, patStr.GetConstWStr(), patStr.Length()) );
	else
		JL_RVAL.setInt32( jl::Match(srcStr.GetConstStr()+start, srcStr.Length()-start, patStr.GetConstStr(), patStr.Length()) );

	return true;
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

		st = JLSemaphoreAcquire(ti->startSem);
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
	return 0;
}

DEFINE_FUNCTION( processEvents ) {

	JL_DEFINE_ARGS;
	int st;
	ModulePrivate *mpv = (ModulePrivate*)JL_GetModulePrivate(cx, jslangModuleId);

	JS::RootedObject handleObj(cx);

	JL_ASSERT_ARGC_MAX( COUNTOF(mpv->processEventThreadInfo) );
	
	ProcessEvent *peList[COUNTOF(mpv->processEventThreadInfo)]; // cache to avoid calling GetHandlePrivate() too often.

	if ( JL_ARGC == 0 ) {
		
		JL_RVAL.setInt32(0);
		return true;
	}

	// fill peList slots
	unsigned int i;
	for ( i = 0; i < argc; ++i ) {

		handleObj.set(&JL_ARG(i+1).toObject());
		JL_ASSERT_ARG_TYPE( IsHandle(cx, handleObj), i+1, "(pev) Handle" );
		JL_ASSERT_ARG_TYPE( IsHandleType(cx, handleObj, jl::CastCStrToUint32("pev")), i+1, "(pev) Handle" );
		ProcessEvent *pe = (ProcessEvent*)GetHandlePrivate(cx, handleObj);
		JL_ASSERT( pe != NULL, E_ARG, E_NUM(i+1), E_STATE ); //JL_ASSERT( pe != NULL, E_ARG, E_NUM(i+1), E_ANINVALID, E_NAME("pev Handle") );

		ASSERT( pe->prepareWait );
		ASSERT( pe->startWait );
		ASSERT( pe->cancelWait );
		ASSERT( pe->endWait );

		JL_CHK( pe->prepareWait(pe, cx, handleObj) );

		peList[i] = pe;
	}

	// prepare threads
	for ( i = 0; i < argc; ++i ) {

		ProcessEventThreadInfo *ti = &mpv->processEventThreadInfo[i];

		if ( ti->startSem == 0 ) {

			ti->startSem = JLSemaphoreCreate(0);
			ASSERT( JLSemaphoreOk(ti->startSem) );
		}
		
		if ( ti->thread == 0 ) { // create the thread stuff, see jl_cmalloc in jslangModuleInit()

			ti->thread = JLThreadStart(ProcessEventThread, ti);

			if ( !JLThreadOk(ti->thread) )
				return JL_ThrowOSError(cx);

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
	JLSemaphoreAcquire(mpv->processEventSignalEventSem); // wait for an event (timeout can also be managed here)
	JLSemaphoreRelease(mpv->processEventSignalEventSem);

	// cancel other waits
	for ( i = 0; i < argc; ++i ) {

		volatile ProcessEvent *peSlot = mpv->processEventThreadInfo[i].peSlot; // avoids to mutex ti->peSlot access.
		if ( peSlot != NULL ) { // see ProcessEventThread(). if peSlot is null this mean that peSlot->startWait() has returned.

			if ( !peSlot->cancelWait(peSlot) ) { // if the thread cannot be gracefully canceled then kill it. However, keep the signalEventSem semaphore (no JLSemaphoreFree(&ti->startSem)).

				ProcessEventThreadInfo *ti = &mpv->processEventThreadInfo[i];
				ti->peSlot = NULL;
				JLSemaphoreRelease(ti->signalEventSem); // see ProcessEventThread()
				JLThreadCancel(ti->thread);
				JLThreadWait(ti->thread); // (TBD) needed ?
				JLThreadFree(&ti->thread);
				ti->thread = 0; // mean that "the thread is free/unused" (see thread creation place)
			}
		}
	}

	for ( i = 0; i < argc; ++i ) {

		st = JLSemaphoreAcquire(mpv->processEventSignalEventSem);
		ASSERT( st );
	}

	ASSERT( argc <= JSVAL_INT_BITS ); // bits

	int32_t eventsMask;
	eventsMask = 0;
	bool hasEvent;
	bool ok;
	ok = true;

	// notify waiters
	for ( i = 0; i < argc; ++i ) {

		ProcessEvent *pe = peList[i];

		JSExceptionState *exState = NULL;
		if ( JL_IsExceptionPending(cx) ) {

			exState = JS_SaveExceptionState(cx);
			JS_ClearPendingException(cx);
		}

		handleObj.set(&JL_ARG(i+1).toObject());
		if ( pe->endWait(pe, &hasEvent, cx, handleObj) != true )
			ok = false; // report errors later

		if ( exState )
			JS_RestoreExceptionState(cx, exState);

		if ( hasEvent )
			eventsMask |= 1 << i;

//		JL_CHK( HandleClose(cx, JL_ARGV[i]) ); // (TBD) recycle items instead of closing them ?
	}

#ifdef DEBUG
	for ( i = 0; i < argc; ++i )
		ASSERT( mpv->processEventThreadInfo[i].peSlot == NULL );
	ASSERT( JLSemaphoreAcquire(mpv->processEventSignalEventSem, 0) == JLTIMEOUT ); // else invalid state
#endif // DEBUG

	JL_RVAL.setInt32(eventsMask);
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
	//jsval callbackFunction;
	JS::PersistentRootedValue callbackFunction;
	//JSObject *callbackFunctionThis;
	JS::PersistentRootedObject callbackFunctionThis;
};

S_ASSERT( offsetof(TimeoutProcessEvent, pe) == 0 );

static bool TimeoutPrepareWait( volatile ProcessEvent *, JSContext *, JS::HandleObject ) {
	
	return true;
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

static bool TimeoutEndWait( volatile ProcessEvent *pe, bool *hasEvent, JSContext *cx, JS::HandleObject obj ) {

	JL_IGNORE(obj);

	TimeoutProcessEvent *upe = (TimeoutProcessEvent*)pe;

	*hasEvent = !upe->canceled;

	// not triggered, then nothing to reset
	if ( !*hasEvent )
		return true;

	// reset for further use.
	JLEventReset(upe->cancel);
	upe->canceled = false;

	if ( upe->callbackFunction.get().isUndefined() )
		return true;

	JS::RootedValue rval(cx);
	JL_CHK( JL_CallFunctionVA(cx, upe->callbackFunctionThis, upe->callbackFunction, &rval) );

	return true;
	JL_BAD;
}

static void TimeoutFinalize( void* data ) {
	
	TimeoutProcessEvent *upe = (TimeoutProcessEvent*)data;
	JLEventFree(&upe->cancel);
}


DEFINE_FUNCTION( timeoutEvents ) {

	JL_DEFINE_ARGS;
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

		JL_CHK( SetHandleSlot(cx, JL_RVAL, 0, JL_OBJVAL) ); // GC protection only
		JL_CHK( SetHandleSlot(cx, JL_RVAL, 1, JL_ARG(2)) ); // GC protection only

		upe->callbackFunctionThis = JSVAL_TO_OBJECT(JL_OBJVAL); // store "this" object.
		upe->callbackFunction = JL_ARG(2); // access to ->callbackFunction is faster than Handle slots.
	} else {

		upe->callbackFunction = JSVAL_VOID;
	}

	return true;
	JL_BAD;
}


#if defined(DEBUG) // || 1
//#define HAS_JL_API_TESTS
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

S_ASSERT(sizeof(wchar_t) == sizeof(jschar));

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
	JS::RootedValue tmp(cx, JSVAL_ONE);
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
	JS::RootedObject o(cx, JL_NewObj(cx));
	JS::RootedId id(cx);
	JS::RootedValue s(cx);
	s.setObject(*o);

	TEST( JL_JsvalToJsid(cx, s, &id) );
	TEST( JSID_IS_OBJECT(id) );
	JS::RootedValue r(cx);
	TEST( JL_JsidToJsval(cx, id, &r) );
	TEST( JSVAL_TO_OBJECT(r) == o );

	TEST( JS_ValueToId(cx, OBJECT_TO_JSVAL(o), id.address()) );
	TEST( !JSID_IS_OBJECT(id) );

	bool found;
	TEST( JS_DefineProperty(cx, o, "test", JSVAL_ONE, NULL, NULL, JSPROP_PERMANENT) );
	TEST( JS_HasProperty(cx, o, "test", &found) );
	TEST( found );

	JSString *jsstr = JS_NewUCStringCopyZ(cx, L("testtesttesttesttesttesttesttesttesttesttesttest"));
	JS::RootedId pid(cx);
	pid.set(JL_StringToJsid(cx, jsstr));

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
	JLData str1("a�\x10\x1B\xFF", 1); // 97 130
	TEST( str1.GetCharAt(0) == 'a' );
	TEST( str1.GetCharAt(1) == '�' );

	JLData str2(L("a�\x10\x1B\xFF"), 1);
	TEST( str1.GetWCharAt(0) == L('a') );
	TEST( str1.GetWCharAt(1) == L('�') );

	jschar s1[100];
	str1.CopyTo(s1);
	s1[str1.Length()] = 0;
	TEST( !wcscmp(s1, L("a�\x10\x1B\xFF") ) );

	char s2[100];
	str2.CopyTo(s2);
	s2[str2.Length()] = 0;
	TEST( !strcmp(s2, "a�\x10\x1B\xFF" ) );

	TEST( !wcscmp(str1.GetConstWStrZ(), L("a�\x10\x1B\xFF") ) );
	TEST( !strcmp(str2.GetConstStrZ(), "a�\x10\x1B\xFF" ) );
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
	
	JS::RootedValue v;
	void *tmp = ptr;
	JL_CHK( JL_NativeToJsval(cx, tmp, v) );
	JL_CHK( JL_JsvalToNative(cx, v, &tmp) );
	TEST( tmp == ptr );
	}

	{
	void *ptr = (void*)1; // 00...01
	
	JS::RootedValue v;
	void *tmp = ptr;
	JL_CHK( JL_NativeToJsval(cx, tmp, v) );
	JL_CHK( JL_JsvalToNative(cx, v, &tmp) );
	TEST( tmp == ptr );
	}

	{
	void *ptr = (void*)-1; // ff...ff
	
	JS::RootedValue v;
	void *tmp = ptr;
	JL_CHK( JL_NativeToJsval(cx, tmp, v) );
	JL_CHK( JL_JsvalToNative(cx, v, &tmp) );
	TEST( tmp == ptr );
	}

	{
	void *ptr = (void*)(size_t((void*)-1)-1); // ff...fe
	
	JS::RootedValue v;
	void *tmp = ptr;
	JL_CHK( JL_NativeToJsval(cx, tmp, v) );
	JL_CHK( JL_JsvalToNative(cx, v, &tmp) );
	TEST( tmp == ptr );
	}

	{
	void *ptr = (void*)(size_t((void*)-1) >> 1); // 7f...ff
	
	JS::RootedValue v;
	void *tmp = ptr;
	JL_CHK( JL_NativeToJsval(cx, tmp, v) );
	JL_CHK( JL_JsvalToNative(cx, v, &tmp) );
	TEST( tmp == ptr );
	}

	{
	void *ptr = (void*)((size_t((void*)-1)>>1) - 1); // 7f...fe
	
	JS::RootedValue v;
	void *tmp = ptr;
	JL_CHK( JL_NativeToJsval(cx, tmp, v) );
	JL_CHK( JL_JsvalToNative(cx, v, &tmp) );
	TEST( tmp == ptr );
	}

	/////////////////////////////////////////////////////////////////

	// JLID ///////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////

	return true;
	JL_BAD;
}

#undef TEST

#endif // HAS_JL_API_TESTS




#if defined(DEBUG)  //   || 1
//#define JSLANG_TEST
#endif

#ifdef JSLANG_TEST

INLINE NEVER_INLINE void
testFct( JSContext *cx ) {
	
	JL_IGNORE(cx);

//	static JS::RootedValue tmp = JSVAL_ONE;
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

	JL_DEFINE_ARGS;
	JL_DEFINE_FUNCTION_OBJ;

	__asm { int 3 }

	JS::RootedValue test1(cx);
	__asm { nop }
	JS::RootedValue v1 = test1;
	__asm { nop }
	test1 = v1;

	__asm { nop }

	JS::MutableHandleValue test2(&test1);
	__asm { nop }
	JS::RootedValue v2 = test2;
	__asm { nop }
	test2.set(v2);
	
	JS::HandleValue test3(test2);
	JS::RootedValue v3 = test3;
	JL_IGNORE(v3);

	struct { void operator()(JS::MutableHandleValue m1) {
		
		JL_IGNORE(m1);
	} } cl1;

	cl1(test2);

	args.rval().set(test3);




//	bool st = JS_IsInt8Array(obj, cx);

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




#include "jlclass3.h"


/*
STATIC_CLASS()

	JL_FUNCTION( xxx, 2 ) {


		return false;
	}


CLASS_END



CLASS( test )

//	JL_PROTOTYPE( pTest )


	JL_HAS_PRIVATE

	JL_SLOT(foo)
	JL_SLOT(bar)

	JL_CONST( fooInt, 1234 )
	JL_CONST( fooDbl, 1234.5 )

	JL_CONSTRUCTOR() {
	
		return true;
	}

	JL_FUNCTION( fct1, 2 ) {

		//JL_GetReservedSlot(
		_slot_foo.index;
		printf("const:%d\n", _const_fooInt.value);
		return false;
	}



	JL_PROPERTY( status )

		JL_GETTER() {

			return true;
		}

		JL_SETTER() {

			return true;
		}
	JL_PROPERTY_END



	JL_PROPERTY( status1 )

		JL_GETTER() {

			return true;
		}

	JL_PROPERTY_END

	JL_INIT() {

		return true;
	}

CLASS_END

*/


////


DEFINE_INIT() {


	JS::RootedObject robj(cx, JL_GetGlobal(cx));

	//REGISTER_STATIC();
	REGISTER_CLASS(FooBar);
	
//	JL_CHK( _static::_classSpec.Register(cx, &robj) );

	return true;
	JL_BAD;
}



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

	HAS_INIT

//	REVISION(jl::SvnRevToInt("$Revision$")) // avoid to set a sourceId property to the global context.
	BEGIN_STATIC_FUNCTION_SPEC

		FUNCTION_ARGC( isCallable, 1 )
		
		FUNCTION_ARGC( stringify, 2 )
		FUNCTION_ARGC( join, 2 )
		FUNCTION_ARGC( indexOf, 3 )

		FUNCTION_ARGC( processEvents, 8 ) // (8 is just a guess)
		FUNCTION_ARGC( timeoutEvents, 2 )

		#ifdef HAS_JL_API_TESTS
		FUNCTION( _jsapiTests )
		#endif

		#ifdef JSLANG_TEST
		FUNCTION( jslangTest )
		#endif

	END_STATIC_FUNCTION_SPEC

END_STATIC



/* test for jlclass3.h

enum { status2, status3, status4 };

CLASS(EmptyClass)
CLASS_END

DOC("static class has a lot of static mumbers", "some more details...")
CLASS(FooBar)

	REV(jl::SvnRevToInt("$Revision$"))

	SLOT(bar)
	SLOT(foo)

	PROTO( Handle )

	PRIVATE {
		int i;
		int j;
	}

	// CONSTRUCTOR UNCONSTRUCTABLE
	CONSTRUCTOR NATIVE() {
		
		JL_DEFINE_ARGS;
		JL_DEFINE_CONSTRUCTOR_OBJ;
		JL_SetPrivate(obj, new config::Private);
		return true;
		JL_BAD;
	}


	FINALIZE_RET() {
			
		delete JL_GetPrivate(obj);
		return true;
	}


	DOC("state property is cool", "details about that...")
	STATIC_PROP 
		NAME( state )
		GET() {

//			printf("%d\n", SLOT_INDEX(foo));
			return true;
		}


	DOC(".status is a status prop");
	PROP
		NAME( status )
		SET() {
		
			return true;
		}


	DOC("status0 doc ...")
	PROP NAME( status0 )
		GET() {

			SLOT_INDEX(bar);
			return true;
		}
		SET() {
			return true;
		}


	CALL ARGC(0,1) NATIVE() {
		
		
		return true;
	}



	PROP
		DOC("status2 is a property")
		NAME_ID( status2 )
		
		DOC("status3 is a property")
		NAME_ID( status3 )
		
		DOC("status4 is a property")
		NAME_ID( status4 )

		GET() {
			return true;
		}

		SET() {
			return true;
		}

	

	JL_FUNCTION(yyy, 0, 1) {

		return true;
	}


	FUNC NAME(zzz) ARGC(0, 1) NATIVE() {

		_item.argcMax;

		return true;
	}


	DOC("this is a function that is named 'fct2', call it like this: fct2()");
	STATIC_FUNC
		NAME(fct2)
		ARGMIN(2)
		NATIVE() {
			
			JL_DEFINE_ARGS;

			return true;
			JL_BAD;
		}

	DOC("const1 is a constant value\n"
		"this is a 2nd line"
	)


	CONSTANT_NAME( const1, 1234.5 )

	DOC("MY_CONST is another constant")

	#define MY_CONST 789
	CONSTANT( MY_CONST )
	CONSTANT( 999 )

	CONSTANT( PATH_MAX )
	
	ITERATOR() {
	
		JS::RootedObject o(cx);
		return o;
	}


	INIT() {

		return true;
	}
	
CLASS_END

*/