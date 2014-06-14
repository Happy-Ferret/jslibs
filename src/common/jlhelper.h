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

#pragma once


#include <js/RequiredDefines.h>

#include "jlplatform.h"

#include "jlalloc.h"

#ifdef MSC
	#pragma warning(push)
	#pragma warning(disable: 4800 4100 4512 4099 4251 4480 4481 4510 4610 )
#endif

#include <jsapi.h>
#include <jsfriendapi.h>

#ifdef MSC
	#pragma warning(pop)
#endif

// only defined as JS_FRIEND_API in js/src/js/src/jsdate.h

extern JS_FRIEND_API(int)
js_DateGetYear(JSContext *cx, JSObject* obj);

extern JS_FRIEND_API(int)
js_DateGetMonth(JSContext *cx, JSObject* obj);

extern JS_FRIEND_API(int)
js_DateGetDate(JSContext *cx, JSObject* obj);

extern JS_FRIEND_API(int)
js_DateGetHours(JSContext *cx, JSObject* obj);

extern JS_FRIEND_API(int)
js_DateGetMinutes(JSContext *cx, JSObject* obj);

extern JS_FRIEND_API(int)
js_DateGetSeconds(JSObject* obj);


///////////////////////////////////////////////////////////////////////////////
// helper macros and inline functions to avoid a function call to the jsapi

ALWAYS_INLINE JSRuntime* FASTCALL
JL_GetRuntime( JSContext *cx ) {

	return js::GetRuntime(cx); // jsfriendapi
}

ALWAYS_INLINE void* FASTCALL
JL_GetRuntimePrivate( const JSRuntime *rt ) {

#ifdef JSRUNTIME_HAS_JLDATA
	return js::PerThreadDataFriendFields::getMainThread(rt)->jldata;
#else
	return JS_GetRuntimePrivate(rt);
#endif
}

ALWAYS_INLINE void FASTCALL
JL_SetRuntimePrivate( JSRuntime *rt, void *data ) {

#ifdef JSRUNTIME_HAS_JLDATA
	js::PerThreadDataFriendFields::getMainThread(rt)->jldata = data;
#else
	JS_SetRuntimePrivate(rt, data);
#endif
}

ALWAYS_INLINE void FASTCALL
JL_updateMallocCounter( JSContext *cx, size_t nbytes ) {

	JS_updateMallocCounter(cx, nbytes);
}

ALWAYS_INLINE JSObject* FASTCALL
JL_GetGlobal( JSContext *cx ) {

	return JS::CurrentGlobalOrNull(cx);
}

ALWAYS_INLINE bool FASTCALL
JL_IsExceptionPending( JSContext *cx ) {

	return JS_IsExceptionPending(cx);
}

ALWAYS_INLINE const JSClass* FASTCALL
JL_GetClass( IN JS::HandleObject obj ) {

	return js::GetObjectJSClass(obj); // jsfriendapi
}

ALWAYS_INLINE const char * FASTCALL
JL_GetClassName( IN JS::HandleObject obj ) {

	return JL_GetClass(obj)->name;
}

ALWAYS_INLINE size_t FASTCALL
JL_GetStringLength( JSString *jsstr ) {

	return JS_GetStringLength(jsstr);
}

ALWAYS_INLINE JS::Value FASTCALL
JL_GetEmptyStringValue( JSContext *cx ) {

	return JS_GetEmptyStringValue(cx);
}

ALWAYS_INLINE bool FASTCALL
JL_HasPrivate( IN JS::HandleObject obj ) {

	//return !!(JL_GetClass(obj)->flags & JSCLASS_HAS_PRIVATE);
	return js::Valueify(JL_GetClass(obj))->hasPrivate();
}

ALWAYS_INLINE void* FASTCALL
JL_GetPrivate( IN JS::HandleObject obj ) {

	ASSERT( JS_IsNative(obj) );
	ASSERT( JL_HasPrivate(obj) );
	return js::GetObjectPrivate(obj); // jsfriendapi
}

ALWAYS_INLINE void* FASTCALL
JL_GetPrivate( IN JS::HandleValue val ) {

	ASSERT( val.isObject() );
	ASSERT( JS_IsNative(&val.toObject()) );
//	ASSERT( JL_HasPrivate(JS::HandleObject::fromMarkedLocation(val.toObjectOrNull())) );
	// JS::RootedObject obj(cx, &val.toObject());
	// return JL_GetPrivate(obj);
	return js::GetObjectPrivate(&val.toObject()); // jsfriendapi // rooting issue?
}

ALWAYS_INLINE void FASTCALL
JL_SetPrivate( IN JS::HandleObject obj, void *data ) {

	ASSERT( JS_IsNative(obj) );
	ASSERT( JL_HasPrivate(obj) );
	JS_SetPrivate(obj, data);
}

ALWAYS_INLINE JSObject* FASTCALL
JL_GetPrototype(JSContext *cx, IN JS::HandleObject obj) {

	JS::RootedObject rproto(cx);
	if ( js::GetObjectProto(cx, obj, &rproto) )
		return rproto;
	else
		return NULL;
}

ALWAYS_INLINE const JSClass* FASTCALL
JL_GetClassOfPrototype(JSContext *cx, IN JS::HandleObject proto) {

	JS::RootedObject obj(cx, JL_GetPrototype(cx, proto));
	return JL_GetClass(obj);
}

ALWAYS_INLINE const JSClass* FASTCALL
JL_GetClassOfPrototype(JSContext *cx, IN JS::HandleValue protoVal) {

	JS::RootedObject proto(cx, &protoVal.toObject());
	return JL_GetClassOfPrototype(cx, proto);
}

ALWAYS_INLINE JSObject* FASTCALL
JL_GetConstructor(JSContext *cx, IN JS::HandleObject obj) {

	return JS_GetConstructor(cx, obj);
}

ALWAYS_INLINE JSObject* FASTCALL
JL_GetParent(JSContext *, IN JS::HandleObject obj) {

	return js::GetObjectParent(obj);
}

ALWAYS_INLINE bool FASTCALL
JL_GetClassPrototype(JSContext *cx, JSProtoKey protoKey, OUT JS::MutableHandleObject proto) {

	return JS_GetClassPrototype(cx, protoKey, proto);
}

ALWAYS_INLINE const JSClass* FASTCALL
JL_GetErrorClaspByProtoKey( JSContext *cx, JSProtoKey protoKey ) {

	JS::RootedObject proto(cx);
	if ( !JL_GetClassPrototype(cx, protoKey, &proto) )
		return NULL;
	return JL_GetClass(proto);
}

ALWAYS_INLINE bool FASTCALL
JL_GetElement(JSContext *cx, IN JS::HandleObject obj, unsigned index, OUT JS::MutableHandleValue vp) {

	return JS_ForwardGetElementTo(cx, obj, index, obj, vp);
}

ALWAYS_INLINE bool FASTCALL
JL_SetElement(JSContext *cx, IN JS::HandleObject obj, unsigned index, IN JS::HandleValue value) {

	return JS_SetElement(cx, obj, index, value);
}


ALWAYS_INLINE JSIdArray *
JL_Enumerate(JSContext *cx, JS::HandleObject obj) {

	JS::RootedObject robj(cx, obj);
	return JS_Enumerate(cx, robj);
}

ALWAYS_INLINE bool FASTCALL
JL_GetReservedSlot(IN JS::HandleObject obj, uint32_t slot, OUT JS::MutableHandleValue vp) {

	ASSERT( JS_IsNative(obj) );
	vp.set(js::GetReservedSlot(obj, slot)); // jsfriendapi
	return true;
}

ALWAYS_INLINE bool FASTCALL
JL_SetReservedSlot(JS::HandleObject obj, unsigned slot, IN JS::HandleValue v) {

	ASSERT( JS_IsNative(obj) );
	js::SetReservedSlot(obj, slot, v); // jsfriendapi
	return true;
}

ALWAYS_INLINE JSString* FASTCALL
JL_NewUCString(JSContext *cx, jschar *chars, size_t length) {

//if spidermonkey et jslibs allocators are not the same:
//
//	void *tmp = JS_malloc(cx, length);
//	if ( !tmp )
//		return NULL;
//	jl::memcpy(tmp, bytes, length * sizeof(*jschar));
//	jl_free(bytes);
//	bytes = (char*)tmp;

	return JS_NewUCString(cx, chars, length); // doc. https://developer.mozilla.org/en/SpiderMonkey/JSAPI_Reference/JS_NewString
}


// Handle "constants"

ALWAYS_INLINE JS::Value FASTCALL
JL_ZInitValue() {

	ASSERT(JS::Value().asRawBits() == 0);
	ASSERT(!JS::Value().isMarkable());
	ASSERT(JS::Value().isDouble());

	return JS::Value();
}

ALWAYS_INLINE JS::HandleValue FASTCALL
JL_TRUE() {

    static JS::Value v = JS::TrueValue();
	return JS::HandleValue::fromMarkedLocation(&v);
}

ALWAYS_INLINE JS::HandleValue FASTCALL
JL_FALSE() {

    static JS::Value v = JS::FalseValue();
	return JS::HandleValue::fromMarkedLocation(&v);
}

ALWAYS_INLINE JS::HandleValue FASTCALL
JL_UNDEFINED() {

    static JS::Value v = JS::UndefinedValue();
	return JS::HandleValue::fromMarkedLocation(&v);
}

ALWAYS_INLINE JS::HandleObject FASTCALL
JL_NULL() {

	return JS::HandleObject::fromMarkedLocation(NULL);
}

// useful for structure with jsid initialized to 0.
ALWAYS_INLINE JS::HandleId FASTCALL
JL_IDZ() {

	jsid tmp;
	JSID_BITS(tmp) = 0;
	ASSERT( JSID_IS_ZERO(INT_TO_JSID(0)) );
	return JS::HandleId::fromMarkedLocation(&tmp);
	// return JS::HandleId::fromMarkedLocation(&INT_TO_JSID(0));
}

ALWAYS_INLINE JS::HandleId FASTCALL
JL_JSID_INT32(int32_t i) {

	jsid id = INT_TO_JSID(i);
	return JS::HandleId::fromMarkedLocation(&id);
}


////


JL_BEGIN_NAMESPACE


class BufString;
	
typedef bool (*NIStreamRead)( JSContext *cx, JS::HandleObject obj, char *buffer, size_t *amount );
typedef bool (*NIBufferGet)( JSContext *cx, JS::HandleObject obj, jl::BufString *str );
typedef bool (*NIMatrix44Get)( JSContext *cx, JS::HandleObject obj, float **pm );

ALWAYS_INLINE NIBufferGet
bufferGetNativeInterface( JSContext *cx, JS::HandleObject obj );

ALWAYS_INLINE NIBufferGet
bufferGetInterface( JSContext *cx, JS::HandleObject obj );

ALWAYS_INLINE NIMatrix44Get
matrix44GetInterface( JSContext *cx, JS::HandleObject obj );



////////////////////
// helper classes

class AutoInterruptCallback {
	JSRuntime *_rt;
	JSInterruptCallback _prevCallback;
	AutoInterruptCallback();
	AutoInterruptCallback( const AutoInterruptCallback & );
public:
	AutoInterruptCallback(JSRuntime *rt, JSInterruptCallback newCallback)
	: _rt(rt), _prevCallback(JS_SetInterruptCallback(rt, newCallback)) {
	}

	~AutoInterruptCallback() {

		JS_SetInterruptCallback(_rt, _prevCallback);
	}
};


class AutoJSEngineInit {
public:
	AutoJSEngineInit() {

		bool st = JS_Init();
		ASSERT(st);
	}

	~AutoJSEngineInit() {

		JS_ShutDown();
	}
};


class AutoExceptionState {
	JSContext *_cx;
	JSExceptionState *_exState;
public:
	~AutoExceptionState() {

		if ( _exState )
			JS_RestoreExceptionState(_cx, _exState);
	}

	AutoExceptionState(JSContext *cx) : _cx(cx) {

		_exState = JS_SaveExceptionState(_cx);
		JS_ClearPendingException(_cx);
	}

	void drop() {

		ASSERT( _exState != NULL );
		JS_DropExceptionState(_cx, _exState);
		_exState = NULL;
	}
};


class AutoErrorReporter {
	JSContext *_cx;
	JSErrorReporter _errReporter;
public:
	~AutoErrorReporter() {

		JS_SetErrorReporter(_cx, _errReporter);
	}

	AutoErrorReporter(JSContext *cx, JSErrorReporter errorReporter) : _cx(cx) {

		_errReporter = JS_SetErrorReporter(_cx, errorReporter);
	}
};


////


ALWAYS_INLINE bool FASTCALL
maybeRealloc( size_t requested, size_t received ) {

	return requested != 0 && (128 * received / requested < 96) && (requested - received > 256); // less than 75% AND mode than 256 bytes
}


ALWAYS_INLINE JSContext* FASTCALL
getFirstContext( JSRuntime *rt ) {

	// see DefaultJSContext() / SetDefaultJSContextCallback()
	JSContext *cx = NULL;
	ASSERT( rt != NULL );
	JS_ContextIterator(rt, &cx);
	ASSERT( cx != NULL );
	return cx;
}


ALWAYS_INLINE bool FASTCALL
inheritFrom( JSContext *cx, JS::HandleObject obj, const JSClass *clasp ) {

	JS::RootedObject proto(cx, obj);
	while ( proto != NULL ) {

		if ( JL_GetClass(proto) == clasp )
			return true;
		proto.set(JL_GetPrototype(cx, proto));
	}
	return false;
}


ALWAYS_INLINE bool FASTCALL
protoOfInheritFrom( JSContext *cx, JS::HandleObject obj, const JSClass *clasp ) {

    JS::RootedObject proto(cx, JL_GetPrototype(cx, obj));
	while ( proto != NULL ) {

		if ( JL_GetClass(proto) == clasp )
			return true;
		proto.set(JL_GetPrototype(cx, proto));
	}
	return false;
}


ALWAYS_INLINE JSObject* FASTCALL
newObjectWithGivenProto( JSContext *cx, const JSClass *clasp, IN JS::HandleObject proto, IN JS::HandleObject parent = JS::NullPtr() ) {

	ASSERT_IF( proto != NULL, JL_GetParent(cx, proto) != NULL );
	// Doc. JS_NewObject, jl::newObjectWithGivenProto behaves exactly the same, except that if proto is NULL, it creates an object with no prototype.
	JS::RootedObject obj(cx, JS_NewObjectWithGivenProto(cx, clasp, proto, parent));  // (TBD) test if parent is ok (see bug 688510)
	ASSERT( JL_GetParent(cx, obj) != NULL );
	return obj;
}


ALWAYS_INLINE JSObject* FASTCALL
newObjectWithoutProto( JSContext *cx ) {

	JS::RootedObject obj(cx, newObjectWithGivenProto(cx, NULL, JS::NullPtr())); // JL_GetGlobal(cx) ??
	ASSERT( JL_GetParent(cx, obj) != NULL );
	ASSERT( JL_GetPrototype(cx, obj) == NULL );
	return obj;
}


ALWAYS_INLINE jsid FASTCALL
stringToJsid( JSContext *cx, JS::HandleString jsstr ) {

	ASSERT( jsstr != NULL );
	JS::RootedString tmp(cx, JS_InternJSString(cx, jsstr)); // if ( !JS_StringHasBeenInterned(cx, jsstr) )
	ASSERT( tmp );
	JS::RootedId id(cx, INTERNED_STRING_TO_JSID(cx, jsstr));
	return id;
}


ALWAYS_INLINE jsid FASTCALL
stringToJsid( JSContext *cx, const jschar *wstr ) {

	ASSERT( wstr != NULL );
	JS::RootedString jsstr(cx, JS_InternUCString(cx, wstr));
	ASSERT( jsstr );
	JS::RootedId id(cx, stringToJsid(cx, jsstr));
	ASSERT( JSID_IS_STRING(id) );
	return id;
}


JL_END_NAMESPACE


#include "jlArgs.h"

#include "jlAssert.h"

#include "../host/host2.h"

#include "jlBuffer.h"

#include "jlTypeCheck.h"

#include "jlTypeConv.h"

#include "jlExec.h"

#include "jlNativeInterface.h"


JL_BEGIN_NAMESPACE


INLINE NEVER_INLINE bool FASTCALL
getScriptLocation( JSContext *cx, OUT const char **filename, OUT unsigned *lineno ) {

	JS::AutoFilename autoFilename;
	if ( JS::DescribeScriptedCaller(cx, filename ? &autoFilename : NULL, lineno ? lineno : NULL) ) {

		if ( filename )
			*filename = autoFilename.get();
		return true;
	}
	return false;
}


INLINE NEVER_INLINE bool FASTCALL
addScriptLocation( JSContext * RESTRICT cx, IN OUT JS::MutableHandleObject obj ) {

	JS::RootedValue tmp(cx);
	JS::AutoFilename autoFilename;
	const char *filename;
	unsigned lineno;
	JL_CHK( JS::DescribeScriptedCaller(cx, &autoFilename, &lineno) );
	if ( autoFilename.get() == NULL || *autoFilename.get() == '\0' )
		filename = "<no_filename>";
	else
		filename = autoFilename.get();
	JL_CHK( jl::setProperty(cx, obj, JLID(cx, fileName), filename) );
	JL_CHK( jl::setProperty(cx, obj, JLID(cx, lineNumber), lineno) );
	return true;
	JL_BAD;
}


INLINE NEVER_INLINE bool FASTCALL
debugPrintScriptLocation( JSContext *cx ) {

	const char *filename;
	unsigned int lineno;
	JL_CHK( getScriptLocation(cx, &filename, &lineno) );
	fprintf(stderr, "%s:%d\n", filename, lineno);
	return true;
	JL_BAD;
}


INLINE NEVER_INLINE bool FASTCALL
throwOSErrorCode( JSContext *cx, JLSystemErrorCode errorCode, const char *moduleName ) {

	char errMsg[1024];
	JLSysetmErrorMessage(errMsg, sizeof(errMsg), errorCode, moduleName);
	JL_ERR( E_OS, E_DETAILS, E_STR(errMsg) );
	JL_BAD;
}


INLINE NEVER_INLINE bool FASTCALL
throwOSError( JSContext *cx ) {

	char errMsg[1024];
	JLLastSysetmErrorMessage(errMsg, sizeof(errMsg));
	JL_ERR( E_OS, E_DETAILS, E_STR(errMsg) );
	JL_BAD;
}


JL_END_NAMESPACE


///////////////////////////////////////////////////////////////////////////////
// ProcessEvent

#include <../jslang/handlePub.h>

class ProcessEvent2 : public HandlePrivate {
public:
	// called before startWait() to allow one to prepare the blocking step
	virtual bool prepareWait(JSContext *cx, JS::HandleObject obj) = 0;
	// starts the blocking thread and call signalEvent() when an event has arrived.
	virtual void startWait() = 0;
	// unlock the blocking thread event if no event has arrived (mean that an event has arrived in another thread).
	virtual bool cancelWait() = 0;
	// process the result
	virtual bool endWait(bool *hasEvent, JSContext *cx, JS::HandleObject obj) = 0;

	JL_HANDLE_TYPE typeId() const {

		return JLHID(pev);
	}
};




///////////////////////////////////////////////////////////////////////////////
// Helper functions

/*
ALWAYS_INLINE JSProtoKey FASTCALL
JL_GetClassProtoKey( const JSClass *clasp ) {

    JSProtoKey key = JSCLASS_CACHED_PROTO_KEY(clasp);
    if (key != JSProto_Null)
        return key;
    if (clasp->flags & JSCLASS_IS_ANONYMOUS) // and JSCLASS_IS_GLOBAL ?
        return JSProto_Object;
    return JSProto_Null;
}


ALWAYS_INLINE JSProtoKey FASTCALL
JL_GetObjectProtoKey( JSContext *cx, JSObject *obj ) {

	JSObject *global = JL_GetGlobal(cx); //JS_GetGlobalForScopeChain(cx);
	JSObject *proto;
	const JSObject *objProto = JL_GetPrototype(obj);
	JSProtoKey protoKey = JL_GetClassProtoKey(JL_GetClass(obj));
	if ( protoKey == JSProto_Null )
		return JSProto_Null;
	if ( !JL_GetClassPrototype(cx, global, protoKey, &proto) )
		return JSProto_Null;
	if ( objProto == proto )
		return protoKey;
	S_ASSERT( sizeof(JSProto_Null) == sizeof(int) );
	for ( int i = int(JSProto_Null)+1; i < int(JSProto_LIMIT); ++i ) {

		if ( !JL_GetClassPrototype(cx, global, JSProtoKey(i), &proto) )
			break;
		if ( objProto == proto )
			return JSProtoKey(i);
	}
	return JSProto_Null; // not found;
}
*/

/*
ALWAYS_INLINE JSProtoKey
JL_GetErrorProtoKey( JSContext *cx, JSObject *obj ) {

	JSObject *global = JS_GetGlobalForScopeChain(cx);
	const JSObject *objProto = JL_GetPrototype(cx, obj);
	JSObject *errorProto;
	for ( int i = int(JSProto_Error); i <= int(JSProto_Error + JSEXN_LIMIT); ++i ) {

		if ( !JL_GetClassPrototype(cx, global, JSProtoKey(i), &errorProto) )
			break;
		if ( objProto == errorProto )
			return JSProtoKey(i);
	}
	return JSProto_Null; // not found;
}
*/

/*
ALWAYS_INLINE bool
JL_CreateErrorException( JSContext *cx, JSExnType exn, JSObject **obj ) {

	JSObject *proto;
	if ( !JL_GetClassPrototype(cx, JL_GetGlobal(cx), JSProtoKey(JSProto_Error + exn), &proto) || !proto )
		return false;

	*obj = JS_NewObject(cx, JL_GetStandardClassByKey(cx, JSProtoKey(JSProto_Error + exn)), proto, NULL);
	return true;
}
*/

/*
static void
ErrorReporter_ToString(JSContext *, const char *message, JSErrorReport *report) {

	if ( !report )
		fprintf(stderr, "%s\n", message);
	else
		fprintf(stderr, "%s (%s:%d)\n", message, report->filename, report->lineno);
}

ALWAYS_INLINE bool
JL_ReportExceptionToString( JSContext *cx, JSObject *obj, JLData  ) {

	JSErrorReporter prevEr = JS_SetErrorReporter(cx, ErrorReporter_ToString);
	JS_ReportPendingException(cx);
	JS_SetErrorReporter(cx, prevEr);
	return true;
}
*/
