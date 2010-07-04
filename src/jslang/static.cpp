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

#include <cstring>

#include "jslang.h"

#include "static.h"

#include "stack.h"
#include "buffer.h"

#include "jsbool.h"

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
  This function converts any value of stream into a string.
**/
DEFINE_FUNCTION( Stringify ) {

	JL_S_ASSERT_ARG(1);

	if ( !JSVAL_IS_PRIMITIVE(JL_ARG(1)) ) {

		JSObject *sobj;
		sobj = JSVAL_TO_OBJECT( JL_ARG(1) );
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
			
			JSString *jsstr;
			jsstr = JS_NewString(cx, newBuffer, total);
			JL_CHK( jsstr );
			*JL_RVAL = STRING_TO_JSVAL( jsstr );

			BufferFinalize(&buf);
			return JS_TRUE;
		bad_freeBuffer:
			BufferFinalize(&buf);
			return JS_FALSE;
		}
	}

	const char *buffer;
	size_t length;
	 // this include NIBufferGet compatible objects
	JL_CHK( JsvalToStringAndLength(cx, &JL_ARG(1), &buffer, &length) ); // warning: GC on the returned buffer !

	char *newBuffer;
	newBuffer = (char*)JS_malloc(cx, length +1);
	JL_CHK( newBuffer );
	newBuffer[length] = '\0';
	memcpy(newBuffer, buffer, length);

	JSString *jsstr;
	jsstr = JS_NewString(cx, newBuffer, length);
	*JL_RVAL = STRING_TO_JSVAL( jsstr );
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

		st = JLSemaphoreAcquire(ti->startSem, -1);
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
	ModulePrivate *mpv = (ModulePrivate*)GetModulePrivate(cx, jslangModuleId);

	JL_S_ASSERT_ARG_RANGE( 1, COUNTOF(mpv->processEventThreadInfo) );
	ProcessEvent *peList[COUNTOF(mpv->processEventThreadInfo)]; // cache to avoid calling GetHandlePrivate() too often.

	uintN i;
	for ( i = 0; i < argc; ++i ) {

		JL_S_ASSERT( IsHandleType(cx, JL_ARGV[i], 'pev'), "Invalid event handle." );
		ProcessEvent *pe = (ProcessEvent*)GetHandlePrivate(cx, JL_ARGV[i]);
		JL_S_ASSERT_RESOURCE( pe );
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

		st = JLSemaphoreAcquire(mpv->processEventSignalEventSem, -1);
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
		if ( JS_IsExceptionPending(cx) ) {

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

static void TimeoutStartWait( volatile ProcessEvent *pe ) {

	UserProcessEvent *upe = (UserProcessEvent*)pe;

	if ( upe->timeout > 0 ) {
		
		int st = JLEventWait(upe->cancel, upe->timeout);
		upe->canceled = (st == JLOK);
	} else {
		
		upe->canceled = false;
	}
}

static bool TimeoutCancelWait( volatile ProcessEvent *pe ) {

	UserProcessEvent *upe = (UserProcessEvent*)pe;

	JLEventTrigger(upe->cancel);
	return true;
}

static JSBool TimeoutEndWait( volatile ProcessEvent *pe, bool *hasEvent, JSContext *cx, JSObject *obj ) {

	UserProcessEvent *upe = (UserProcessEvent*)pe;

	JLEventFree(&upe->cancel);
	*hasEvent = !upe->canceled;
	if ( !*hasEvent )
		return JS_TRUE;
	if ( JSVAL_IS_VOID( upe->callbackFunction ) )
		return JS_TRUE;
	jsval rval;
	JL_CHK( JS_CallFunctionValue(cx, JS_GetGlobalObject(cx), upe->callbackFunction, 0, NULL, &rval) );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_FUNCTION_FAST( TimeoutEvents ) {

	JL_S_ASSERT_ARG_RANGE( 1, 2 );

	unsigned int timeout;
	JL_CHK( JsvalToUInt(cx, JL_FARG(1), &timeout) );
	if ( JL_FARG_ISDEF(2) )
		JL_S_ASSERT_FUNCTION( JL_FARG(2) );

	UserProcessEvent *upe;
	JL_CHK( CreateHandle(cx, 'pev', sizeof(UserProcessEvent), (void**)&upe, NULL, JL_FRVAL) );
	upe->pe.startWait = TimeoutStartWait;
	upe->pe.cancelWait = TimeoutCancelWait;
	upe->pe.endWait = TimeoutEndWait;
	upe->timeout = timeout;
	upe->cancel = JLEventCreate(false);
	JL_ASSERT( JLEventOk(upe->cancel) );
	SetHandleSlot(cx, *JL_FRVAL, 0, JL_FARG(2));

	if ( JL_FARG_ISDEF(2) ) {

		JL_S_ASSERT_FUNCTION( JL_FARG(2) );
		JL_CHK( SetHandleSlot(cx, *JL_FRVAL, 0, JL_FARG(2)) ); // GC protection only
		upe->callbackFunction = JL_FARG(2);
	} else {
	
		upe->callbackFunction = JSVAL_VOID;
	}

	return JS_TRUE;
	JL_BAD;
}





class JLBufferInfo {
private:
	const uint8_t *_data;
	size_t _length;
public:
	JLBufferInfo() {};
	JLBufferInfo( const void *data, size_t count )
		:_data((const uint8_t *)data), _length(count) {
	}
	template <class T>
	JLBufferInfo( const T *data, size_t count )
		:_data((const uint8_t *)data), _length(sizeof(T)*count) {
	}

	const uint8_t *Data() const {
		
		return _data;
	}

	size_t Length() const {
		
		return _length;
	}
};


class JLSerializationBuffer {

	uint8_t *_start;
	uint8_t *_pos;
	size_t _length;

	void ReserveBytes( size_t length ) {

		size_t offset = _pos - _start;
		if ( offset + length > _length ) {
			
			_length = _length * 2 + length;
			_start = (uint8_t*)jl_realloc(_start, _length);
			JL_ASSERT( _start != NULL );
			_pos = _start + offset;
		}
	}

public:
	~JLSerializationBuffer() {
		
		jl_free(_start);
	}

	JLSerializationBuffer() {

		_length = 4096;
		_start = (uint8_t*)jl_malloc(_length);
		JL_ASSERT( _start != NULL );
		_pos = _start;
	}

	const uint8_t *Data() const {
	
		return _start;
	}

	size_t Length() const {
	
		return _pos - _start;
	}

	JLSerializationBuffer& operator <<( const char *buf ) {

		size_t length = strlen(buf)+1;
		*this << length;
		ReserveBytes(length);
		memcpy(_pos, buf, length);
		_pos += length;
		return *this; 
	}

	JLSerializationBuffer& operator <<( const JLBufferInfo &buf ) {

		*this << buf.Length();
		ReserveBytes(buf.Length());
		memcpy(_pos, buf.Data(), buf.Length());
		_pos += buf.Length();
		return *this; 
	}

	template <class T>
	JLSerializationBuffer& operator <<( const T value ) {

		ReserveBytes(sizeof(T));
		*(T*)_pos = value;
		_pos += sizeof(T);
		return *this;
	}
};


class JLUnSerializationBuffer {

	const uint8_t *_start;
	const uint8_t *_pos;
	size_t _length;

public:

	JLUnSerializationBuffer( const uint8_t *data, size_t length )
		:_length(length), _start(data), _pos(_start) {
	}

	bool AssertData( size_t length ) const {
		
		return (_pos - _start) + length <= _length;
	}

	JLUnSerializationBuffer& operator >>( const char *&buf ) {

		size_t length;
		*this >> length;
		JL_ASSERT( AssertData(length) );
		buf = (const char*)_pos;
		_pos += length;
		return *this; 
	}

	JLUnSerializationBuffer& operator >>( JLBufferInfo &buf ) {

		size_t length;
		*this >> length;
		JL_ASSERT( AssertData(length) );
		buf = JLBufferInfo(_pos, length);
		_pos += length;
		return *this; 
	}

	template <class T>
	JLUnSerializationBuffer& operator >>( T &value ) {

		JL_ASSERT( AssertData(sizeof(T)) );
		value = *(T*)_pos;
		_pos += sizeof(T);
		return *this;
	}
};


enum JLSerializeType {

	JLTHole,
	JLTVoid,
	JLTNull,
	JLTBool,
	JLTInt,
	JLTDouble,
	JLTString,
	JLTFunction,
	JLTArray,
	JLTObject,
	JLTObjectValue,
	JLTCustomObject,
	JLTNativeObject
};


class JLUnserializer {
private:
	JLUnSerializationBuffer buffer;

public:
	JLUnserializer( const uint8_t *data, size_t length )
		: buffer(data, length) {
	}

	JSBool Unserialize( JSContext *cx, jsval &val ) {

		unsigned char type;
		buffer >> type;

		switch (type) {
			case JLTHole:
				val = JSVAL_HOLE;
				break;
			case JLTVoid:
				val = JSVAL_VOID;
				break;
			case JLTNull:
				val = JSVAL_NULL;
				break;
			case JLTBool: {
				char b;
				buffer >> b;
				val = BOOLEAN_TO_JSVAL(b);
				break;
			}
			case JLTInt: {
				jsint i;
				buffer >> i;
				val = INT_TO_JSVAL(i);
				break;
			}
			case JLTDouble: {
				jsdouble d;
				buffer >> d;
				JL_CHK( JS_NewDoubleValue(cx, d, &val) );
				break;
			}
			case JLTString: {
				JLBufferInfo buf;
				buffer >> buf;
				JSString *jsstr;
				jsstr = JS_NewUCStringCopyN(cx, (const jschar *)buf.Data(), buf.Length()/2);
				val = STRING_TO_JSVAL(jsstr);
				break;
			}
			case JLTFunction: {
				JLBufferInfo buf;
				buffer >> buf;
				JSXDRState *xdr;
				xdr = JS_XDRNewMem(cx, JSXDR_DECODE);
				JS_XDRMemSetData(xdr, (void*)buf.Data(), buf.Length());
				JL_CHK( JS_XDRValue(xdr, &val) );
				JS_XDRMemSetData(xdr, NULL, 0);
				JS_XDRDestroy(xdr);
				break;
			}
			case JLTArray: {

				jsuint length;
				buffer >> length;
				JSObject *arr;
				arr = JS_NewArrayObject(cx, length, NULL);
				val = OBJECT_TO_JSVAL(arr);

				jsval tmp;
				for ( jsuint i = 0; i < length; ++i ) {
					
					JL_CHK( Unserialize(cx, tmp) );
					if ( tmp != JSVAL_HOLE ) // optional check
						JL_CHK( JS_SetElement(cx, arr, i, &tmp) );
				}
				break;
			}
			case JLTObject: {

				jsint length;
				buffer >> length;

				JSObject *obj = JS_NewObject(cx, NULL, NULL, NULL);
				val = OBJECT_TO_JSVAL(obj);

				const char *name;

				for ( int i = 0; i < length; ++i ) {
					
					buffer >> name;
					jsval value;
					JL_CHK( Unserialize(cx, value) );
					JL_CHK( JS_SetProperty(cx, obj, name, &value) );
				}
				break;
			}
			case JLTObjectValue: {

				const char *className;
				buffer >> className;
				jsval value;
				JL_CHK( Unserialize(cx, value) );
				JSObject *scope = JS_GetScopeChain(cx); // JS_GetGlobalForScopeChain
				jsval prop;
				JL_CHK( JS_GetProperty(cx, scope, className, &prop) );
				JSObject *newObj = JS_New(cx, JSVAL_TO_OBJECT(prop), 1, &value);
				val = OBJECT_TO_JSVAL(newObj);
				break;
			}
			case JLTNativeObject: {

				const char *className;
				buffer >> className;
				jsval value;
				JL_CHK( Unserialize(cx, value) );

				JSObject *scope = JS_GetScopeChain(cx); // JS_GetGlobalForScopeChain
				jsval constructor;
				JL_CHK( JS_GetProperty(cx, scope, className, &constructor) );

//				JSObject *proto = JS_GetPrototype(cx, JSVAL_TO_OBJECT(constructor));

				jsval argv[] = { JSVAL_NULL, value };
				js::AutoArrayRooter avr(cx, COUNTOF(argv), argv);

				JS_NewObject(


				JL_CHK( JS_CallFunctionName(cx, JSVAL_TO_OBJECT(constructor), "_unserialize", COUNTOF(argv-1), argv+1, argv) );
				val = *argv;
				break;
			}

			case JLTCustomObject: {

				const char *className;
				buffer >> className;
				jsval value;
				JL_CHK( Unserialize(cx, value) );
				JSObject *scope = JS_GetScopeChain(cx); // JS_GetGlobalForScopeChain
				jsval prop;
				JL_CHK( JS_GetProperty(cx, scope, className, &prop) );
				JSObject *obj = JSVAL_TO_OBJECT(prop);
				jsval argv[] = { JSVAL_NULL, value };
				js::AutoArrayRooter avr(cx, COUNTOF(argv), argv);

//				JS_GetMethod(cx, obj->getProto(), "_unserialize", NULL, &fctVal);

				//JL_CHK( JS_CallFunctionName(cx, obj->getProto(), "_unserialize", COUNTOF(argv-1), argv+1, argv) );
				val = *argv;
				break;
			}

		}

		return JS_TRUE;
		JL_BAD;
	}

};




class JLSerializer {
private:
	JLSerializationBuffer buffer;

public:

	JLSerializer()
		: buffer() {
	}

	const uint8_t *Data() const {
		
		return buffer.Data();
	}

	size_t Length() const {
		
		return buffer.Length();
	}

	JSBool Serialize( JSContext *cx, jsval val ) {

		unsigned char type;
		if ( JSVAL_IS_PRIMITIVE(val) ) {

			if ( val == JSVAL_HOLE ) {

				type = JLTHole;
				buffer << type;
			} else
			if ( JSVAL_IS_VOID(val) ) {
				
				type = JLTVoid;
				buffer << type;
			} else
			if ( JSVAL_IS_NULL(val) ) {
				
				type = JLTNull;
				buffer << type;
			} else
			if ( JSVAL_IS_BOOLEAN(val) ) {

				type = JLTBool;
				buffer << type << char(JSVAL_TO_BOOLEAN(val));
			} else
			if ( JSVAL_IS_INT(val) ) {
				
				type = JLTInt;
				buffer << type << JSVAL_TO_INT(val);
			} else
			if ( JSVAL_IS_DOUBLE(val) ) {
				
				type = JLTDouble;
				buffer << type << *JSVAL_TO_DOUBLE(val);
			} else
			if ( JSVAL_IS_STRING(val) ) {

				type = JLTString;
				JSString *jsstr = JSVAL_TO_STRING(val);
				buffer << type << JLBufferInfo(JS_GetStringChars(jsstr), JS_GetStringLength(jsstr));
			}
		} else { // !JSVAL_IS_PRIMITIVE

			JSObject *obj = JSVAL_TO_OBJECT(val);

			if ( JS_IsArrayObject(cx, obj) ) {
				
				type = JLTArray;
				jsuint length;
				JL_CHK( JS_GetArrayLength(cx, obj, &length) );
				buffer << type << length;

				jsval elt;
				for ( jsint i = 0; i < jl::SafeCast<jsint>(length); ++i ) {

					JL_CHK( JS_GetElement(cx, obj, i, &elt) );
					if ( JSVAL_IS_VOID(elt) ) {

						JSBool found;
						JL_CHK( JS_HasElement(cx, obj, i, &found) );
						if ( !found )
							elt = JSVAL_HOLE;
					}
					JL_CHK( Serialize(cx, elt) );
				}
			} else
			if ( JS_ObjectIsFunction(cx, obj) ) {
				
				type = JLTFunction;
				JSXDRState *xdr = JS_XDRNewMem(cx, JSXDR_ENCODE);
				JL_CHK( JS_XDRValue(xdr, &val) );
				uint32 length;
				void *buf = JS_XDRMemGetData(xdr, &length);
				JL_ASSERT( buf );
				buffer << type << JLBufferInfo(buf, length);
				JS_XDRDestroy(xdr);
			} else {

				jsval fctVal;
//				JL_CHK( obj->getProperty(cx, JLID(cx, _Serialize), &fctVal) );
//				JL_CHK( JS_GetProperty(cx, obj, "_serialize", &fctVal) ); // JS_GetMethod(cx, obj, "_serialize", 

//				JL_CHK( JS_GetMethod(cx, JS_GetConstructor(cx, obj), "_serialize", NULL, &fctVal) ); // for native class
//				JL_CHK( JS_GetMethod(cx, obj, "_serialize", NULL, &fctVal) ); // for non-native class

				JL_CHK( JS_GetMethod(cx, obj, "_serialize", NULL, &fctVal) );

				
//				JL_CHK( JS_GetProperty(cx, obj, "_serialize", &fctVal) );

				if ( fctVal != JSVAL_VOID ) {
					
					JL_ASSERT( JsvalIsFunction(cx, fctVal) );

					JSFunction *fct = JS_ValueToFunction(cx, fctVal);
					if ( fct->isInterpreted() ) { // poor unreliable

						type = JLTCustomObject;
						jsval unserializeFctVal;
						JL_CHK( JS_GetMethod(cx, obj, "_unserialize", NULL, &unserializeFctVal) );
						JL_CHK( Serialize(cx, unserializeFctVal) );
						buffer << type << unserializeFctVal;
					} else {

						type = JLTNativeObject;
						buffer << type << obj->getClass()->name;
					}

					jsval argv[] = { JSVAL_NULL };
					js::AutoArrayRooter avr(cx, COUNTOF(argv), argv);
					JL_CHK( JS_CallFunctionValue(cx, obj, fctVal, COUNTOF(argv-1), argv+1, argv) );
					JL_CHK( Serialize(cx, *argv) );

				} else
				if ( JL_ObjectIsObject(cx, obj) ) {

					type = JLTObject;
					buffer << type;

					JSIdArray *idArray = JS_Enumerate(cx, obj);
					buffer << idArray->length;
					jsval name, value;
					for ( int i = 0; i < idArray->length; ++i ) {
						
						name = ID_TO_VALUE( idArray->vector[i] ); // JL_CHK( JS_IdToValue(cx, idArray->vector[i], &name) );
						JSString *jsstr = JSVAL_IS_STRING(name) ? JSVAL_TO_STRING(name) : JS_ValueToString(cx, name);
						buffer << JS_GetStringBytesZ(cx, jsstr);
						JL_CHK( obj->getProperty(cx, idArray->vector[i], &value) );
						JL_CHK( Serialize(cx, value) );
					}
					JS_DestroyIdArray(cx, idArray);
				} else { // fallback

					type = JLTObjectValue;
					buffer << type << obj->getClass()->name;
					jsval value;
					JL_CHK( obj->defaultValue(cx, JSTYPE_VOID, &value) );
					JL_CHK( Serialize(cx, value) );
				}
			}
		}
		
		return JS_TRUE;
		JL_BAD;
	}
};






#ifdef DEBUG
DEFINE_FUNCTION( jslang_test ) {


	JLSerializer ser;
	JL_CHK( ser.Serialize( cx, JL_ARG(1) ) );
	
	const char *data = (const char *)ser.Data();
	size_t length = ser.Length();
	{
	JLUnserializer unser((uint8_t*)data, length);
	JL_CHK( unser.Unserialize(cx, *rval) );
	}
	return JS_TRUE;
	JL_BAD;
}
#endif // DEBUG


CONFIGURE_STATIC

//	REVISION(JL_SvnRevToInt("$Revision$")) // avoid to set a revision to the global context
	BEGIN_STATIC_FUNCTION_SPEC

		#ifdef DEBUG
		FUNCTION( jslang_test )
		#endif // DEBUG

		FUNCTION_ARGC( Stringify, 1 )
		FUNCTION( ProcessEvents )
		FUNCTION_FAST( TimeoutEvents )
	END_STATIC_FUNCTION_SPEC

END_STATIC
