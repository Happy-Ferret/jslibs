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
#include "queue.h"


#ifdef MSC
	#pragma warning(push)
	#pragma warning(disable: 4800 4100 4512 4099 4251 4480 4481 4510 4610 )
#endif

#include <jsapi.h>
#include <jsfriendapi.h>
#include <js/OldDebugAPI.h> // JS_GetScriptVersion, JS_GetScriptFilename, JS_GetScriptFilename
#include <js/Id.h>

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


#include <sys/stat.h> // stat() used by JL_LoadScript()


JL_BEGIN_NAMESPACE

class BufString;

JL_END_NAMESPACE

//ALWAYS_INLINE bool FASTCALL
//JL_NewEmptyBuffer( IN JSContext *cx, OUT JS::MutableHandleValue vp );

//ALWAYS_INLINE bool FASTCALL
//JL_NewBufferGetOwnership( JSContext *cx, void *src, size_t nbytes, OUT JS::MutableHandleValue rval );

ALWAYS_INLINE bool FASTCALL
JL_MaybeRealloc( size_t requested, size_t received );

typedef bool (*NIStreamRead)( JSContext *cx, JS::HandleObject obj, char *buffer, size_t *amount );
typedef bool (*NIBufferGet)( JSContext *cx, JS::HandleObject obj, jl::BufString *str );
typedef bool (*NIMatrix44Get)( JSContext *cx, JS::HandleObject obj, float **pm );

ALWAYS_INLINE NIBufferGet
BufferGetNativeInterface( JSContext *cx, JS::HandleObject obj );

ALWAYS_INLINE NIBufferGet
BufferGetInterface( JSContext *cx, JS::HandleObject obj );

ALWAYS_INLINE NIMatrix44Get
Matrix44GetInterface( JSContext *cx, JS::HandleObject obj );

ALWAYS_INLINE bool
SetBufferGetInterface( JSContext *cx, JS::HandleObject obj, NIBufferGet pFct );


///////////////////////////////////////////////////////////////////////////////
// helper macros to avoid a function call to the jsapi

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

ALWAYS_INLINE JSString* FASTCALL
JL_GetEmptyString( JSContext *cx ) {

	return JS_GetEmptyString(JL_GetRuntime(cx));
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

    JS::RootedObject robj(cx, obj);
	JS::RootedObject rproto(cx);
    return js::GetObjectProto(cx, robj, &rproto) ? rproto.get() : NULL;
}

ALWAYS_INLINE const JSClass* FASTCALL
JL_GetClassOfPrototype(JSContext *cx, IN JS::HandleObject proto) {

	JS::RootedObject obj(cx, JL_GetPrototype(cx, proto));
	return JL_GetClass(obj);
}

ALWAYS_INLINE const JSClass* FASTCALL
JL_GetClassOfPrototype(JSContext *cx, IN JS::HandleValue protoVal) {

	JS::RootedObject proto(cx, &protoVal.toObject());
	JS::RootedObject obj(cx, JL_GetPrototype(cx, proto));
	return JL_GetClass(obj);
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



ALWAYS_INLINE jsid FASTCALL
JL_StringToJsid( JSContext *cx, JS::HandleString jsstr ) {

	ASSERT( jsstr != NULL );
	JS::RootedString tmp(cx, JS_InternJSString(cx, jsstr)); // if ( !JS_StringHasBeenInterned(cx, jsstr) )
	ASSERT( tmp );
	JS::RootedId id(cx, INTERNED_STRING_TO_JSID(cx, jsstr));
	return id;
}


ALWAYS_INLINE jsid FASTCALL
JL_StringToJsid( JSContext *cx, const jschar *wstr ) {

	ASSERT( wstr != NULL );
	JS::RootedString jsstr(cx, JS_InternUCString(cx, wstr));
	ASSERT( jsstr );
	JS::RootedId id(cx, JL_StringToJsid(cx, jsstr));
	ASSERT( JSID_IS_STRING(id) );
	return id;
}


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



///////////////////////////////////////////////////////////////////////////////
// Safe Mode tools

#define JL_IS_SAFE (unlikely(!::_unsafeMode))
#define JL_IS_UNSAFE (likely(::_unsafeMode))

#define JL_SAFE_BEGIN if (JL_IS_SAFE) {
#define JL_SAFE_END }

#define JL_UNSAFE_BEGIN if (JL_IS_UNSAFE) {
#define JL_UNSAFE_END }

#define JL_SAFE(code) \
	JL_MACRO_BEGIN \
		if (JL_IS_SAFE) {code;} \
	JL_MACRO_END

#define JL_UNSAFE(code) \
	JL_MACRO_BEGIN \
		if (JL_IS_UNSAFE) {code;} \
	JL_MACRO_END



///////////////////////////////////////////////////////////////////////////////
// helper macros

// BEWARE: the following helper macros are only valid inside a JS Native/FastNative function definition !

#define JL_BAD bad:return(false)

#define JL_BADVAL(RETURN_VALUE) bad:return(RETURN_VALUE)

///////////////////////////////////////////////////////////////////////////////
// Error management

enum E_TXTID {

		E__INVALID,
	#define JL_NEW_ERR
	#define DEF( NAME, TEXT, EXN ) \
		E_##NAME,
	#include "jlerrors.msg"
	#undef DEF
	#undef JL_NEW_ERR
		E__LIMIT,
};

// simple helpers
#define E_ERRNO( num )                E_ERRNO_1, num
#define E_STR( str )                  E_STR_1, (const char *)str
#define E_NAME( str )                 E_NAME_1, (const char *)str
#define E_NUM( num )                  E_NUM_1, num
#define E_COMMENT( str )              E_COMMENT_1, str
#define E_COMMENT2( str1, str2 )      E_COMMENT_2, str1, str2
#define E_INTERVAL_NUM( vMin, vMax )  E_INTERVAL_NUM_2, vMin, vMax
#define E_INTERVAL_STR( sMin, sMax )  E_INTERVAL_STR_2, sMin, sMax
#define E_TY_NARRAY( num )            E_TY_NARRAY_1, num
#define E_TY_NVECTOR( num )           E_TY_NVECTOR_1, num


#ifdef DEBUG
#define JL__REPORT_END_ARG E_COMMENT(JL_CODE_LOCATION), E__INVALID
#else
#define JL__REPORT_END_ARG E__INVALID
#endif


// note: Support for variadic macros was introduced in Visual C++ 2005
#define JL_ERR( ... ) \
	JL_MACRO_BEGIN \
		jl::Host::getHost(cx).report(false, ##__VA_ARGS__, JL__REPORT_END_ARG); \
		goto bad; \
	JL_MACRO_END


#define JL_WARN( ... ) \
	JL_MACRO_BEGIN \
		if ( JL_IS_SAFE && !jl::Host::getHost(cx).report(true, ##__VA_ARGS__, JL__REPORT_END_ARG) ) \
			goto bad; \
	JL_MACRO_END


#define JL_CHK( CONDITION ) \
	JL_MACRO_BEGIN \
		if (unlikely( !(CONDITION) )) { \
			goto bad; \
		} \
		ASSUME(!!(CONDITION)); \
	JL_MACRO_END


#define JL_CHKB( CONDITION, LABEL ) \
	JL_MACRO_BEGIN \
		if (unlikely( !(CONDITION) )) { \
			goto LABEL; \
		} \
	JL_MACRO_END


#define JL_CHKM( CONDITION, ... ) \
	JL_MACRO_BEGIN \
		if (unlikely( !(CONDITION) )) { \
			JL_ERR( __VA_ARGS__ ); \
		} \
		ASSUME(!!(CONDITION)); \
	JL_MACRO_END


#define JL_ASSERT( CONDITION, ... ) \
	JL_MACRO_BEGIN \
		if ( JL_IS_SAFE ) { \
			if (unlikely( !(CONDITION) )) { \
				JL_ERR( __VA_ARGS__ ); \
			} \
		} /* else if ( IS_DEBUG ) { ASSERT( (CONDITION) ); } */ \
		ASSUME(!!(CONDITION)); \
	JL_MACRO_END


#define JL_ASSERT_WARN( CONDITION, ... ) \
	JL_MACRO_BEGIN \
		if ( JL_IS_SAFE ) { \
			if (unlikely( !(CONDITION) )) { \
				JL_WARN( __VA_ARGS__ ); \
			} \
		} \
	JL_MACRO_END


// misc

#define JL_ASSERT_ALLOC( PTR ) \
	JL_MACRO_BEGIN \
		if ( JL_IS_SAFE ) { \
			if (unlikely( !(PTR) )) { \
				ASSERT( !JL_IsExceptionPending(cx) ); \
				JS_ReportOutOfMemory(cx); \
				goto bad; \
			} \
		} \
		else if ( IS_DEBUG ) { \
			ASSERT( (PTR) ); \
		} \
		ASSUME(PTR); \
	JL_MACRO_END


// val

#define JL_ASSERT_IS_BOOLEAN(val, context) \
	JL_ASSERT( NOIL(JL_ValueIsBoolean)(cx, val), E_VALUE, E_STR(context), E_TYPE, E_TY_BOOLEAN )

#define JL_ASSERT_IS_INTEGER(val, context) \
	JL_ASSERT( NOIL(isInt)(cx, val), E_VALUE, E_STR(context), E_TYPE, E_TY_INTEGER )

#define JL_ASSERT_IS_INTEGER_NUMBER(val, context) \
	JL_ASSERT( NOIL(jl::isInt)(cx, val), E_VALUE, E_STR(context), E_TYPE, E_TY_INTEGERDOUBLE )

#define JL_ASSERT_IS_NUMBER(val, context) \
	JL_ASSERT( NOIL(jl::isNumber)(cx, val), E_VALUE, E_STR(context), E_TYPE, E_TY_NUMBER )

#define JL_ASSERT_IS_CALLABLE(val, context) \
	JL_ASSERT( NOIL(jl::isCallable)(cx, val), E_VALUE, E_STR(context), E_TYPE, E_TY_FUNC )

#define JL_ASSERT_IS_ARRAY(val, context) \
	JL_ASSERT( NOIL(jl::isArray)(cx, val), E_VALUE, E_STR(context), E_TYPE, E_TY_ARRAY )

#define JL_ASSERT_IS_OBJECT(val, context) \
	JL_ASSERT( !JSVAL_IS_PRIMITIVE(val), E_VALUE, E_STR(context), E_TYPE, E_TY_OBJECT )

#define JL_ASSERT_IS_STRING(val, context) \
	JL_ASSERT( NOIL(JL_ValueIsData)(cx, val), E_VALUE, E_STR(context), E_TYPE, E_TY_STRINGDATA )

//

#define JL_ASSERT_RANGE(val, valMin, valMax, context) \
	JL_ASSERT( jl::isInRange((int)val, (int)valMin, (int)valMax), E_VALUE, E_STR(context), E_RANGE, E_INTERVAL_NUM(valMin, valMax) )


// arg

#define JL_ASSERT_ARGC_MIN(minCount) \
	JL_ASSERT( JL_ARGC >= (minCount), E_ARGC, E_MIN, E_NUM(minCount) )

#define JL_ASSERT_ARGC_MAX(maxCount) \
	JL_ASSERT( JL_ARGC <= (maxCount), E_ARGC, E_MAX, E_NUM(maxCount) )

#define JL_ASSERT_ARGC_RANGE(minCount, maxCount) \
	JL_ASSERT( jl::isInRange((int)JL_ARGC, (int)minCount, (int)maxCount), E_ARGC, E_RANGE, E_INTERVAL_NUM(unsigned(minCount), unsigned(maxCount)) )

#define JL_ASSERT_ARGC(count) \
	JL_ASSERT( JL_ARGC == (count), E_ARGC, E_EQUALS, E_NUM(count) )

#define JL_ASSERT_ARG_IS_BOOLEAN(argNum) \
	JL_ASSERT( NOIL(jl::isBool)(cx, JL_ARG(argNum)), E_ARG, E_NUM(argNum), E_TYPE, E_NAME("boolean") )

#define JL_ASSERT_ARG_IS_INTEGER(argNum) \
	JL_ASSERT( NOIL(jl::isInt)(cx, JL_ARG(argNum)), E_ARG, E_NUM(argNum), E_TYPE, E_NAME("integer") )

#define JL_ASSERT_ARG_IS_INTEGER_NUMBER(argNum) \
	JL_ASSERT( NOIL(jl::isInt)(cx, JL_ARG(argNum)), E_ARG, E_NUM(argNum), E_TYPE, E_NAME("integer < 2^53") )

#define JL_ASSERT_ARG_IS_NUMBER(argNum) \
	JL_ASSERT( NOIL(jl::isNumber)(cx, JL_ARG(argNum)), E_ARG, E_NUM(argNum), E_TYPE, E_NAME("number") )

#define JL_ASSERT_ARG_IS_OBJECT(argNum) \
	JL_ASSERT( !JSVAL_IS_PRIMITIVE(JL_ARG(argNum)), E_ARG, E_NUM(argNum), E_TYPE, E_NAME("object") )

#define JL_ASSERT_ARG_IS_OBJECT_OR_NULL(argNum) \
	JL_ASSERT( JSVAL_IS_OBJECT(JL_ARG(argNum)), E_ARG, E_NUM(argNum), E_TYPE, E_NAME("object"), E_OR, E_NAME("null") )

#define JL_ASSERT_ARG_IS_STRING(argNum) \
	JL_ASSERT( NOIL(jl::isData)(cx, JL_ARG(argNum)), E_ARG, E_NUM(argNum), E_TYPE, E_NAME("string || data") )

#define JL_ASSERT_ARG_IS_ARRAY(argNum) \
	JL_ASSERT( NOIL(jl::isArray)(cx, JL_ARG(argNum)), E_ARG, E_NUM(argNum), E_TYPE, E_NAME("array") )

#define JL_ASSERT_ARG_IS_ARRAYLIKE(argNum) \
	JL_ASSERT( NOIL(jl::isArrayLike)(cx, JL_ARG(argNum)), E_ARG, E_NUM(argNum), E_TYPE, E_NAME("array") )

#define JL_ASSERT_ARG_IS_CALLABLE(argNum) \
	JL_ASSERT( NOIL(jl::isCallable)(cx, JL_ARG(argNum)), E_ARG, E_NUM(argNum), E_TYPE, E_NAME("function") )

// fallback
#define JL_ASSERT_ARG_TYPE(condition, argNum, typeStr) \
	JL_ASSERT( condition, E_ARG, E_NUM(argNum), E_TYPE, E_NAME(typeStr) )

#define JL_ASSERT_ARG_VAL_RANGE(val, valMin, valMax, argNum) \
	JL_ASSERT( jl::isInRange((int)val, (int)valMin, (int)valMax), E_ARG, E_NUM(argNum), E_RANGE, E_INTERVAL_NUM(valMin, valMax) )



// obj

// note:
//   If JS_IsConstructing is true, JS_THIS must not be used, the constructor should construct and return a new object.
//   JS_IsConstructing must be called from within a native given the native's original cx and vp arguments !
#define JL_ASSERT_CONSTRUCTING() \
	JL_ASSERT( args.isConstructing() /*(JL_ARGC, JS_IsConstructing(cx, vp))*/, E_THISOBJ, E_CONSTRUCT )

// note: JL_GetClass(JL_GetPrototype(... because |JL_ASSERT_THIS_INSTANCE( new Stream() )| must pass whereas |JL_ASSERT_THIS_INSTANCE( Stream.prototype )| must fail.
#define JL_ASSERT_INSTANCE( jsObject, jsClass ) \
	JL_ASSERT( JL_GetClassOfPrototype(cx, jsObject) == jsClass, E_OBJ, E_INSTANCE, E_NAME((jsClass)->name) ) // ReportIncompatibleMethod(cx, CallReceiverFromArgv(argv), Valueify(clasp));

#define JL_ASSERT_THIS_INSTANCE() \
	JL_ASSERT( JL_GetClassOfPrototype(cx, JL_OBJ) == JL_THIS_CLASS, E_THISOBJ, E_INSTANCE, E_NAME(JL_THIS_CLASS_NAME) ) // ReportIncompatibleMethod(cx, CallReceiverFromArgv(argv), Valueify(clasp));

#define JL_ASSERT_INHERITANCE( jsObject, jsClass ) \
	JL_ASSERT( NOIL(JL_ProtoOfInheritFrom)(cx, jsObject, (jsClass)), E_OBJ, E_INHERIT, E_NAME((jsClass)->name) )

#define JL_ASSERT_THIS_INHERITANCE() \
	JL_ASSERT( NOIL(JL_ProtoOfInheritFrom)(cx, JL_OBJ, JL_THIS_CLASS), E_THISOBJ, E_INHERIT, E_NAME(JL_THIS_CLASS_NAME) )


#define JL_ASSERT_OBJECT_STATE( condition, name ) \
	JL_ASSERT( condition, E_OBJ, E_NAME(name), E_STATE )

#define JL_ASSERT_THIS_OBJECT_STATE( condition ) \
	JL_ASSERT( condition, E_THISOBJ, E_NAME(JL_THIS_CLASS_NAME), E_STATE )


JL_BEGIN_NAMESPACE


ALWAYS_INLINE JSObject* FASTCALL
newObjectWithGivenProto( JSContext *cx, const JSClass *clasp, IN JS::HandleObject proto, IN JS::HandleObject parent = JS::NullPtr() );



#define ARGSARGS cx, argc, vp

struct Args {

	JS::PersistentRootedObject _thisObj; // use HandleObject instead ?
	const JS::CallArgs _jsargs;
	JSContext *&_cx;

	Args(JSContext *&cx, unsigned argc, JS::Value *&vp)
	: _cx(cx), _thisObj(cx), _jsargs( JS::CallArgsFromVp(argc, vp) ) {
	}

	JSObject *callee() {

		return &_jsargs.callee();
	}

	unsigned length() const {

		return _jsargs.length();
	}

	bool hasDefined(unsigned i) const {

		return _jsargs.hasDefined(i);
	}

	JS::MutableHandleValue handleAt(unsigned i) const {

		return _jsargs[i];
	}

	JS::HandleValue handleOrUndefinedAt(unsigned i) const {

		return _jsargs.get(i);
	}

	JS::MutableHandleValue rval() const {

		return _jsargs.rval();
	}

	bool isConstructing() const {

		return _jsargs.isConstructing();
	}

	void constructThis(JSClass *clasp, JS::HandleObject proto) {

		ASSERT( isConstructing() );
		_thisObj.set( jl::newObjectWithGivenProto(_cx, clasp, proto) ); // JS_NewObjectForConstructor() use the callee to determine parentage and [[Prototype]].
		rval().setObject(*_thisObj);
		_jsargs.setThis(rval());
	}

	void computeThis() {

		JS::Value tmp = _jsargs.thisv();
		if ( tmp.isObject() ) {

			_thisObj.set( &tmp.toObject() );
		} else
		if ( tmp.isNullOrUndefined() ) {

			_thisObj.set( JL_GetGlobal(_cx) ); //_thisObj.set( JS_GetGlobalForObject(_cx, &_jsargs.callee() ) );
		} else {

			if ( !JS_ValueToObject(_cx, _jsargs.thisv(), &_thisObj) ) {

				_thisObj.set(NULL);
			}
		}
		_jsargs.setThis(JS::ObjectValue(*_thisObj));
	}

	JS::HandleObject thisObj() {

		if ( !_thisObj )
			computeThis();
		return JS::HandleObject::fromMarkedLocation(_thisObj.address());
	}

	JS::HandleValue thisObjVal() {

		if ( _jsargs.thisv().isObject() )
			return _jsargs.thisv();
		computeThis();
		return _jsargs.thisv();
	}

private:
	void operator=( const Args & );
};


#define PROPARGSARGS cx, obj, id, vp

struct PropArgs {

	JS::HandleObject &_thisObj;
	JS::MutableHandleValue &_vp;

	PropArgs(JSContext *cx, JS::HandleObject &obj, JS::HandleId id, JS::MutableHandleValue &vp)
	: _thisObj(obj), _vp(vp) {
	}

	unsigned length() const {

		return 1;
	}

	bool hasDefined(unsigned) const {

		return true;
	}

	JS::MutableHandleValue &handleAt(unsigned) {

		return _vp;
	}

	JS::HandleValue handleOrUndefinedAt(unsigned) const {

		return _vp;
	}

	JS::MutableHandleValue &rval() {

		return _vp;
	}

	JS::HandleObject thisObj() const {

		return _thisObj;
	}

private:
	void operator=( const PropArgs & );
};

	
JL_END_NAMESPACE



#define JL_ARGC (args.length())

// returns the ARGument n
#define JL_ARG( n ) (ASSERT((n) > 0 && (unsigned)(n) <= JL_ARGC), args.handleAt((n)-1))

// returns the ARGument n or undefined if it does not exist
#define JL_SARG( n ) (args.handleOrUndefinedAt((n)-1))

// returns true if the ARGument n IS DEFined
#define JL_ARG_ISDEF( n ) (args.hasDefined((n)-1))

// the return value
#define JL_RVAL (args.rval())

// is the current obj (this)
#define JL_OBJ (args.thisObj())

// is the current obj (this) as a JS::Value. if this method returns null, an error has occurred and must be propagated or caught.
#define JL_OBJVAL (args.thisObjVal())


#define JL_DEFINE_ARGS \
	jl::Args args(ARGSARGS);

#define JL_DEFINE_PROP_ARGS \
	jl::PropArgs args(PROPARGSARGS);

#define JL_DEFINE_CALL_FUNCTION_OBJ \
	JS::RootedObject obj(cx, JS_CALLEE(cx, vp).toObjectOrNull();

#define JL_DEFINE_CONSTRUCTOR_OBJ \
	JL_MACRO_BEGIN \
	args.constructThis(JL_THIS_CLASS, jl::Host::getHost(cx).getCachedProto(JL_THIS_CLASS_NAME)); \
	JL_MACRO_END



// test:
#define JL_ARG_GEN(N, type) \
	TYPE arg##N; \
	if ( JL_ARG_ISDEF(N) ) \
		JL_CHK( jl::getValue(cx, JL_ARG(n), &arg##N) ); \



#include "../host/host2.h"



///////////////////////////////////////////////////////////////////////////////


JL_BEGIN_NAMESPACE


class BufBase : public jl::CppAllocators {
public:
	enum InitializationType { Uninitialized = 1 };
	typedef uint8_t* Type;
	static const size_t UnknownSize = size_t(~0);

private:
	Type _data;
	mutable size_t _size;
	mutable bool _owner; // true if this object is responsible of freeing the memory pointed by _data

	void operator =( const BufBase & );
	//void operator [](size_t);
public:

	~BufBase() {

		if ( hasData() && owner() )
			free();
	}

	BufBase(InitializationType) {
	}

	BufBase()
	: _data(nullptr) {
	}

	template <class T>
	explicit BufBase( T *data, size_t size = UnknownSize )
	: _data(reinterpret_cast<Type>(const_cast<RemoveConst(T)*>(data))), _size(size), _owner(!IsConst(T)) {
	}

	explicit BufBase( const BufBase& buf, bool withOwnership = true )
	: _data(buf._data), _size(buf._size) {

		if ( withOwnership ) {

			bool tmp = buf.owner(); // handle when this is buf
			buf.dropOwnership(); // only one can free the buffer
			_owner = tmp;
		} else {

			_owner = false;
		}
	}

	template <class T>
	void
	get( T *data, size_t size = UnknownSize ) {

		if ( hasData() && owner() )
			free();
		setData(data);
		setSize(size);
		_owner = !IsConst(T);
	}

	void
	get( const BufBase &buf, bool withOwnership = true ) {

		if ( hasData() && owner() && !is(buf) )
			free();

		setData(buf.data());
		setSize(buf.size());
		
		if ( withOwnership ) {

			bool tmp = buf.owner(); // handle when this is buf
			buf.dropOwnership(); // only one can free the buffer
			_owner = tmp;
		} else {

			_owner = false;
		}
	}

	void
	setEmpty() {

		if ( hasData() && owner() )
			free();
		setSize(0);
		dropOwnership();
	}

	bool
	empty() {

		return hasData() || size() == 0;
	}

	void
	free() {

		ASSERT( _owner );
		ASSERT( _data );

		//jl::zeromem(_data, _size);
		jl_free(_data);
		_data = nullptr;
	}

	template <class T>
	void
	setData( const T *data ) {

		_data = reinterpret_cast<Type>(const_cast<T*>(data));
	}
	
	void
	setData( nullptr_t data ) {

		_data = data;
	}

	bool
	hasData() const {

		return _data != nullptr;
	}

	operator bool() const {

		return _data != nullptr;
	}

	template <class T>
	const T
	dataAs() const {

		return reinterpret_cast<const T>(_data);
	}

	template <class T>
	T
	dataAs() {

		return reinterpret_cast<T>(_data);
	}

	const Type
	data() const {

		return _data;
	}

	Type
	data() {

		return _data;
	}

	size_t
	size() const {

		ASSERT_IF( _data == nullptr, _size == 0 );

		return _size;
	}

	void
	setSize( size_t size ) const {

		_size = size;
	}

	bool
	owner() const {

		return _owner;
	}

	void
	dropOwnership() const {

		_owner = false;
	}

	size_t
	used() const {

		ASSERT_IF( _data == nullptr, _size == 0 );

		return _size;
	}


	ALWAYS_INLINE bool
	is( const BufBase &buf ) const {

		return buf.data() == data();
	}

	bool
	own() {
		
		ASSERT( !owner() );

		Type tmp = static_cast<Type>(jl_malloc(size()));
		jl::memcpy(tmp, data(), used());
		setData(tmp);
		_owner = true;
		return tmp != nullptr;
	}

	void
	copyTo( BufBase &buf ) {
		
		ASSERT( size() == buf.size() );

		jl::memcpy(buf.data(), data(), size());
	}

	void
	copyFrom( BufBase &buf ) {
		
		ASSERT( size() == buf.size() );

		jl::memcpy(data(), buf.data(), size());
	}

	void
	alloc( size_t size ) {
		
		ASSERT_IF( _data != nullptr, !_owner ); // else memory leak

		get(jl_malloc(size), size);
	}

	void
	realloc( size_t newSize ) {

		if ( _owner ) {

			setData(jl_realloc(data(), newSize));
		} else {

			void *newData = jl_malloc(newSize);
			if ( newData )
				jl::memcpy(newData, data(), jl::min(size(), newSize));
			setData(newData);
			_owner = true;
		}
		setSize(newSize);
	}
};


////


class BufPartial : public BufBase {
	mutable size_t _used;
	
public:

	size_t
	used() const {

		return _used;
	}

	void
	setUsed( size_t used ) const {
		
		ASSERT( used <= size() );
		_used = used;
	}

	void
	alloc( size_t size ) {
		
		BufBase::alloc(size);
		setUsed(0);
	}

	void
	realloc( size_t newSize ) {
		
		BufBase::realloc(newSize);
		//setUsed(jl::min(used(), newSize));
		ASSERT( used() <= size() );
	}

	void
	maybeRealloc() {
		
		ASSERT( owner() );

		if ( JL_MaybeRealloc(size(), used()) ) {
		
			realloc(used());
			ASSERT( hasData() ); // assume it always possible to reduce the size of an allocated block
		}
	}

	void
	setEmpty() {

		BufBase::empty();
		setUsed(0);
	}

	bool
	empty() {

		return !hasData() || used() == 0;
	}


	template <class T>
	void
	get( T *data, size_t size ) {

		BufBase::get(data, size);
		setUsed(size);
	}

	void
	get( const BufBase &buf, bool withOwnership = true ) {
	
		BufBase::get(buf, withOwnership);
		setUsed(buf.size());
	}

	void
	get( const BufPartial &buf, bool withOwnership = true ) {
	
		BufBase::get(buf, withOwnership);
		setUsed(buf.used());
	}

	void operator =( const BufPartial &buf ) {
		
		get(buf);
	}

	BufPartial() {
	}

	template <class T>
	explicit BufPartial( T *data, size_t size )
	: BufBase(data, size), _used(size) {
	}

	explicit BufPartial( const BufBase& buf, bool withOwnership = true )
	: BufBase(buf, withOwnership), _used(buf.size()) {
	}

	BufPartial( const BufPartial& buf, bool withOwnership = true )
	: BufBase(buf, withOwnership), _used(buf.used()) {
	}

//

	bool
	stealArrayBuffer( JSContext *cx, JS::HandleObject obj ) {
		
		ASSERT( !owner() );

		JL_ASSERT( JS_IsArrayBufferObject(obj), E_TY_ARRAYBUFFER, E_REQUIRED );

		size_t size = JS_GetArrayBufferByteLength(obj);
		void *data = JS_StealArrayBufferContents(cx, obj);
		get(data, size);
		JL_CHK( data != nullptr );
		return true;
		JL_BAD;
	}


	bool
	toArrayBuffer( JSContext *cx, JS::MutableHandleValue rval ) {
		
		ASSERT( used() != UnknownSize );

		if ( used() == 0 ) {
			
			rval.setObject(*JS_NewArrayBuffer(cx, 0));
		} else {

			if ( !owner() )
				owner();
			rval.setObject(*JS_NewArrayBufferWithContents(cx, used(), data()));
			dropOwnership();
		}
		JL_CHK( !rval.isNull() );
		return true;
		JL_BAD;
	}

};


////////


class BufString : public BufPartial {
	typedef jschar WideChar;

	uint8_t _charSize;
	uint8_t _terminatorLength;

public:

	uint8_t
	charSize() const {

		ASSERT( _charSize == sizeof(char) || _charSize == sizeof(WideChar) );
		return _charSize;
	}

	BufString&
	setCharSize( uint8_t charSize ) {

		_charSize = charSize;
		return *this;
	}

	bool
	wide() const {

		return _charSize == sizeof(WideChar);
	}

	bool
	nt() const {

		return _terminatorLength != 0;
	}

	BufString&
	setNt( bool nullTerminated ) {

		_terminatorLength = nullTerminated ? 1 : 0;
		return *this;
	}

	void
	alloc( size_t size ) {
		
		BufPartial::alloc(size);
	}

	BufString&
	setEmpty() {

		BufPartial::get("", 1);
		_charSize = sizeof(char);
		_terminatorLength = 1;
		return *this;
	}

	bool
	empty() {

		return !hasData() || used() == 0 || used() / charSize() == _terminatorLength;
	}


	template <class T>
	void
	get( T *str, size_t len = UnknownSize, bool nullTerminated = true ) {
	
		ASSERT_IF( len == UnknownSize, nullTerminated == true );

		_charSize = sizeof(*str);
		_terminatorLength = nullTerminated ? 1 : 0;
		BufPartial::get(str, len != UnknownSize ? sizeof(*str) * (len + _terminatorLength) : UnknownSize);
	}

	void
	get( const BufBase & buf, bool withOwnership = true ) {

		BufPartial::get(buf, withOwnership);
	}

	void
	get( const BufString & buf, bool withOwnership = true ) {

		BufPartial::get(buf, withOwnership);
		setCharSize( buf.charSize() );
		setNt( buf.nt() );
	}

	void
	get( JSContext *cx, JS::HandleString str ) {

		size_t len;
		const jschar *chars = JS_GetStringCharsAndLength(cx, str, &len);
		get(chars, len, false);
		_charSize = sizeof(*chars);
		_terminatorLength = 0;
	}

//

	void operator =( const BufString &buf ) {
		
		get(buf);
	}

	ALWAYS_INLINE
	BufString() {
	}

	template <class T>
	explicit BufString( T *str, size_t len = UnknownSize, bool nullTerminated = true )
	: BufPartial(str, len), _charSize(sizeof(T)), _terminatorLength(nullTerminated ? 1 : 0) {
	}

	explicit BufString( const BufBase& buf, bool withOwnership = true )
	: BufPartial(buf, withOwnership), _charSize(1), _terminatorLength(0) {
	}

	explicit BufString( const BufPartial& buf, bool withOwnership = true )
	: BufPartial(buf, withOwnership), _charSize(1), _terminatorLength(0) {
	}

	BufString( const BufString& buf, bool withOwnership = true )
	: BufPartial(buf, withOwnership), _charSize(buf._charSize), _terminatorLength(buf._terminatorLength) {
	}

	explicit BufString( JSContext *cx, JS::HandleString str )
	: _charSize(sizeof(jschar)), _terminatorLength(0) {

		size_t len;
		const jschar *chars = JS_GetStringCharsAndLength(cx, str, &len);
		get(chars, len, false);
	}

//

	INLINE size_t
	length() const {

		size_t len;
		if ( used() == UnknownSize ) {

			ASSERT( size() == UnknownSize );
			ASSERT( nt() );

			len = wide() ? jl::strlen(dataAs<WideChar*>()) : jl::strlen(dataAs<char*>());
			setSize((len + 1) * charSize());
			setUsed(size());
		} else {
				
			len = used() / charSize() - _terminatorLength;
		}

		ASSERT_IF( wide(), used() % 2 == 0 );
		return len;
	}


	size_t
	lengthOrZero() const {

		return hasData() ? length() : 0;
	}

//

	bool
	operator ==( const BufString &str ) const {

		size_t len = length();
		if ( len == str.length() ) {

			if ( wide() ) {
			
				return str.wide() ? jl::tstrncmp(dataAs<WideChar*>(), str.dataAs<WideChar*>(), len) == 0 : jl::tstrncmp(dataAs<WideChar*>(), str.dataAs<char*>(), len) == 0;
			} else {

				return str.wide() ? jl::tstrncmp(dataAs<char*>(), str.dataAs<WideChar*>(), len) == 0 : jl::tstrncmp(dataAs<char*>(), str.dataAs<char*>(), len) == 0;
			}
		} else {

			return false;
		}
	}

	bool
	operator !=( const BufString &str ) const {
	
		return !(operator ==(str));
	}


	template <class T>
	bool
	operator ==( const T *str ) const {
		
		if ( nt() ) {
			
			return dataAs<T*>() == str || ( wide() ? jl::tstrcmp(dataAs<WideChar*>(), str) == 0 : jl::tstrcmp(dataAs<char*>(), str) == 0);
		} else {

			size_t len = length();
			return jl::strlen(str) == len && ( wide() ? jl::tstrncmp(dataAs<WideChar*>(), str, len) == 0 : jl::tstrncmp(dataAs<char*>(), str, len) == 0);
		}
	}

	template <class T>
	bool
	operator !=( const T *str ) const {

		return !(operator ==(str));
	}

//

	template <class T>
	T
	charAt( size_t index ) const {

		T retChar;
		
		DISABLE_SMALLER_TYPE_CHECK;

		if ( sizeof(T) == charSize() )
			retChar = dataAs<T*>()[index];
		else
		if ( sizeof(T) == sizeof(char) )
			retChar = wide() ? dataAs<WideChar*>()[index] : dataAs<char*>()[index];
		else
		if ( sizeof(T) == sizeof(WideChar) )
			retChar = wide() ? dataAs<WideChar*>()[index] : dataAs<char*>()[index];
		else
			ASSERT(false);
		
		RESTORE_SMALLER_TYPE_CHECK;
		
		return retChar;
	}

//

	template <class T>
	size_t
	copyTo( T *dst, size_t maxLength = UnknownSize ) const {

		size_t len = length();
		maxLength = maxLength == UnknownSize ? len : jl::min(len, maxLength);
		if ( charSize() == sizeof(T) )
			jl::memcpy(dst, data(), len * sizeof(T));
		else
		if ( charSize() == sizeof(char) )
			reinterpretBuffer<T, char>(dst, data(), len);
		else
		if ( charSize() == sizeof(WideChar) )
			reinterpretBuffer<T, WideChar>(dst, data(), len);
		else
			ASSERT(false);
		return maxLength;
	}

//

	template<class T, bool nullTerminated>
	T
	to() {

		typedef RemovePointer(T) Base;
		typedef RemoveConst(Base) MutableBase;
		const bool asConst = IsConst(Base);

		ASSERT( this->operator bool() );
		ASSERT( charSize() > 0 && charSize() <= 2 );

		const size_t len = length();

		ASSERT_IF( len == UnknownSize, nt() );
		ASSERT_IF( len != UnknownSize && owner() && wide() && !nt(), jl_msize(data()) >= len*2 );
		ASSERT_IF( len != UnknownSize && owner() && !wide() && !nt(), jl_msize(data()) >= len );
		ASSERT_IF( len != UnknownSize && owner() && wide() && nt(), jl_msize(data()) >= len*2+2 );
		ASSERT_IF( len != UnknownSize && owner() && !wide() && nt(), jl_msize(data()) >= len+1 );
		ASSERT_IF( len != UnknownSize && nt() && wide(), ((jschar*)data())[len] == 0 );
		ASSERT_IF( len != UnknownSize && nt() && !wide(), ((char*)data())[len] == 0 );
		ASSERT_IF( wide(), used() % 2 == 0 );

		if ( len == 0 )
			return nullptr;

		// something to do ?
		if ( sizeof(Base) != charSize() || (!asConst && !owner()) || (nullTerminated && !nt()) ) {

			const size_t requiredSize = (len + (nullTerminated ? 1 : 0)) * sizeof(Base);

			BufBase dst(*this, false);

			// two reasons to allocate
			if ( size() < requiredSize || !owner() ) {

				if ( sizeof(Base) != charSize() ) {

					dst.alloc(requiredSize);
				} else {

					realloc(requiredSize);
					dst.get(*this, false); // sync
				}
			}

			if ( charSize() == sizeof(Base) )
				;// nothing
			else
			if ( charSize() == sizeof(char) )
				reinterpretBuffer<MutableBase, char>(dst.data(), data(), len);
			else
			if ( charSize() == sizeof(WideChar) )
				reinterpretBuffer<MutableBase, WideChar>(dst.data(), data(), len);

			if ( nullTerminated )
				dst.dataAs<MutableBase*>()[len] = Base(0);

			if ( !is(dst) )
				get(dst, true);

			setCharSize(sizeof(Base));
			setNt(nullTerminated);
		}

		if ( !asConst )
			dropOwnership();

		return dataAs<T>();
	}

//

	template<class T>
	T
	toData() {

		return to<T, false>();
	}

	template<class T>
	T
	toStringZ() {

		return to<T, true>();
	}

//

	template<class T>
	T
	toDataOrNull() {

		return hasData() ? toData<T>() : nullptr;
	}

	template<class T>
	T
	toStringZOrNull() {

		return hasData() ? toStringZ<T>() : nullptr;
	}

	ALWAYS_INLINE
	operator const char *() {

		return toStringZOrNull<const char*>();
	}

	ALWAYS_INLINE
	operator const WideChar *() {

		return toStringZOrNull<const WideChar*>();
	}

	bool
	toString( JSContext *cx, JS::MutableHandleValue rval ) {

		size_t len = length();
		if ( len == 0 )
			rval.set(JL_GetEmptyStringValue(cx));
		else
			rval.setString(JS_NewUCString(cx, toStringZ<jschar*>(), len));
		return true;
		JL_BAD;
	}
};



namespace pv {

template<typename Source>
struct SignificandStringValue {
	
	static const char * min() {
	
		static char buffer[(::std::numeric_limits<Source>::is_signed ? ::std::numeric_limits<Source>::digits10 + 1 : 0) + 2];
		static char* str = jl::itoa10( ::std::numeric_limits<Source>::min(), buffer);
		return str;
	}

	static const char * max() {
	
		static char buffer[::std::numeric_limits<Source>::digits10 + 2];
		static char* str = jl::itoa10( ::std::numeric_limits<Source>::max(), buffer);
		return str;
	}
};

template<>
struct SignificandStringValue<int32_t> {
	static const char * min() { return "-2^31"; }
	static const char * max() { return "2^31-1"; }
};

template<>
struct SignificandStringValue<uint32_t> {
	static const char * min() { return "0"; }
	static const char * max() { return "2^32"; }
};

template<>
struct SignificandStringValue<int64_t> {
	static const char * min() { return "2^63"; }
	static const char * max() { return "2^63-1"; }
};

template<>
struct SignificandStringValue<uint64_t> {
	static const char * min() { return "0"; }
	static const char * max() { return "2^64"; }
};

template<>
struct SignificandStringValue<float> {
	static const char * min() { return "-2^24"; }
	static const char * max() { return "2^24"; }
};

template<>
struct SignificandStringValue<double> {
	static const char * min() { return "-2^53"; }
	static const char * max() { return "2^53"; }
};


template <class T>
INLINE NEVER_INLINE bool FASTCALL
getNumberValue_slow( JSContext *cx, const JS::HandleValue &val, bool valIsDouble, T *num ) {

	double d;

	if ( valIsDouble ) {

		d = val.toDouble();
	} else {

		// try getPrimitive instead ?
		JL_CHK( JS::ToNumber(cx, val, &d) );
		ASSERT( mozilla::IsNaN(JS_GetNaNValue(cx).toDouble()) );
		JL_CHKM( !mozilla::IsNaN(d), E_VALUE, E_TYPE, E_TY_NUMBER );
	}

	// optimization when T is double
	if ( jl::isTypeFloat64(*num) ) {

		*num = (T)d;
		return true;
	}

	if ( isInBounds<T>(d) ) {

		*num = static_cast<T>(d);
		// only warn for precision lost if num is integral
		JL_ASSERT_WARN( !::std::numeric_limits<T>::is_exact || jl::IsIntegerValue(d), E_VALUE, E_PRECISION );
		return true;
	}

bad_range:
	JL_ERR( E_VALUE, E_RANGE, E_INTERVAL_STR(SignificandStringValue<T>::min(), SignificandStringValue<T>::max()) );
	JL_BAD;
}


// error if out of range, warning if loss of precision
template <class T>
ALWAYS_INLINE bool FASTCALL
getNumberValue( JSContext *cx, const JS::HandleValue &val, T *num ) {

	if ( val.isInt32() ) {

		int32_t intVal = val.toInt32();
		if ( jl::isInBounds<T>(intVal) ) {

			*num = static_cast<T>(intVal);
			return true;
		}
		goto bad_range;
	}

	bool valIsDouble = val.isDouble();
	
	// optimization when T and val are double
	if ( jl::isTypeFloat64(*num) && valIsDouble ) {

		*num = (T)val.toDouble();
		return true;
	}

	// optimization when T is float
	if ( jl::isTypeFloat32(*num) && valIsDouble ) {

		double d = val.toDouble();
		if ( abs(d) < ::std::numeric_limits<T>::max() ) { //if ( isInBounds<T>(d) ) {

			*num = static_cast<T>(d);
			return true;
		}
		goto bad_range;
	}


	return getNumberValue_slow(cx, val, valIsDouble, num);
bad_range:
	JL_ERR( E_VALUE, E_RANGE, E_INTERVAL_STR(SignificandStringValue<T>::min(), SignificandStringValue<T>::max()) );
	JL_BAD;
}

} // namespace pv


namespace pv {

INLINE NEVER_INLINE bool FASTCALL
getValue_slow( JSContext *cx, JS::HandleValue val, jl::BufString* data ) {

	if ( val.isObject() ) {

		JS::RootedObject obj(cx, &val.toObject());
		NIBufferGet fct = BufferGetInterface(cx, obj); // BufferGetNativeInterface
		if ( fct )
			return fct(cx, obj, data);

		if ( JS_IsArrayBufferObject(obj) ) {

			uint32_t length = JS_GetArrayBufferByteLength(obj);
			if ( length ) {
				
				data->get(static_cast<const uint8_t*>(JS_GetArrayBufferData(obj)), length, false);
			} else {
			
				data->setEmpty();
			}
			ASSERT( !data->owner() );
			return true;
		}

		if ( JS_IsTypedArrayObject(obj) ) {

			uint32_t length = JS_GetTypedArrayLength(obj);
			if ( length ) {

				if ( JS_GetArrayBufferViewType(obj) == js::ArrayBufferView::TYPE_UINT16 )
					data->get(reinterpret_cast<const jschar*>(JS_GetUint16ArrayData(obj)), length, false);
				else
					data->get(static_cast<const uint8_t*>(JS_GetArrayBufferViewData(obj)), length, false);
			} else {

				data->setEmpty();
			}
			ASSERT( !data->owner() );
			return true;
		}
	}

	// fallback
	JS::RootedString jsstr(cx, JS::ToString(cx, val));
	JL_CHKM( jsstr != NULL, E_VALUE, E_CONVERT, E_TY_STRING );
	data->get(cx, jsstr);
	return true;
	JL_BAD;
}


template <class T>
class StrSpec {
	static const size_t undefined = size_t(-1);
	T _str;
	mutable size_t _len;
public:
	StrSpec(T str)
	: _str(str), _len(StrSpec::undefined) {
	}

	StrSpec(T str, size_t len)
	: _str(str), _len(len) {

		ASSERT( len != StrSpec::undefined );
	}

	const T	str() const {

		return _str;
	}

	T str() {

		return _str;
	}

	size_t len() const {

		ASSERT( _str );
		if (unlikely( _len == StrSpec::undefined )) {

			_len = jl::strlen(_str);
			ASSERT( _len != StrSpec::undefined );
		}
		return _len;
	}
};

} // namespace pv



typedef pv::StrSpec<const char *> CStrSpec;

typedef pv::StrSpec<const jschar *> WCStrSpec;

typedef pv::StrSpec<jschar *> OwnerlessWCStrSpec;


ALWAYS_INLINE CStrSpec FASTCALL
strSpec( const char *str, size_t len ) {

	return CStrSpec(str, len);
}

ALWAYS_INLINE WCStrSpec FASTCALL
strSpec( const jschar *str, size_t len ) {

	return WCStrSpec(str, len);
}


////////

bool setValue( JSContext *cx, JS::MutableHandleValue rval, const void * ); // forbidden case


ALWAYS_INLINE bool FASTCALL
setValue( JSContext *cx, JS::MutableHandleValue rval, IN jl::BufString &data, bool toArrayBuffer = true ) {

	return toArrayBuffer ? data.toString(cx, rval) : data.toArrayBuffer(cx, rval);
}


ALWAYS_INLINE bool FASTCALL
setValue( JSContext *cx, JS::MutableHandleValue rval, const bool b ) {

	rval.setBoolean(b);
	return true;
}

ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const int8_t num) {

	rval.set(JS::NumberValue(num));
	return true;
}

ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const uint8_t num) {

	rval.set(JS::NumberValue(num));
	return true;
}

ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const int16_t num) {

	rval.set(JS::NumberValue(num));
	return true;
}

ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const uint16_t num) {

	rval.set(JS::NumberValue(num));
	return true;
}

ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const int32_t num) {

	rval.set(JS::NumberValue(num));
	return true;
}

ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const uint32_t num) {

	rval.set(JS::NumberValue(num));
	return true;
}

ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const int64_t num) {

	rval.set(JS::NumberValue(num));
	return true;
}

ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const uint64_t num) {

	rval.set(JS::NumberValue(num));
	return true;
}

ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const long num) {

	rval.set(JS::NumberValue(num));
	return true;
}

ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const unsigned long num) {

	rval.set(JS::NumberValue(num));
	return true;
}


ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const float num) {

	rval.set(JS::NumberValue(num));
	return true;
}

ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const double num) {

	rval.set(JS::NumberValue(num));
	return true;
}

/*
// unfortunately calling setValue("foo") use the following function instead of setValue(pv::StrSpec).
ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const void *ptr) {

	if ( ptr == NULL ) {

		rval.setNull();
	} else {

		if ( ((uint32_t)ptr & 1) == 0 ) { // see PRIVATE_PTR_TO_JSVAL_IMPL()

			vp.address()->setPrivate(ptr);
		} else { // rare since pointers are alligned (except function ptr in DBG mode ?)

			JS::RootedObject obj(cx, jl::newObjectWithoutProto(cx));



//		jl::HandlePrivate pv = new HandlePrivate();
//		JL_CHK( HandleCreate(cx, pv, rval) );

		rval.address()->setPrivate(const_cast<void*>(ptr));
	}
	return true;
}
*/


ALWAYS_INLINE bool FASTCALL
setValue( JSContext *cx, JS::MutableHandleValue rval, IN OwnerlessWCStrSpec &str ) {

	if ( str.str() != NULL ) {
		
		if ( str.len() > 0 ) {

			ASSERT( msize(str.str()) >= str.len()+1 );
			ASSERT( str.str()[str.len()] == 0 );
			JS::RootedString jsstr(cx, JL_NewUCString(cx, str.str(), str.len()));
			JL_CHK( jsstr );
			rval.setString(jsstr);
		} else {

			rval.setString(JS_GetEmptyString(JL_GetRuntime(cx))); // = JL_GetEmptyStringValue(cx);
		}
	} else {

		rval.setUndefined();
	}
	
	return true;
	JL_BAD;
}

ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const CStrSpec &s) {

	if ( s.str() != NULL ) {

		JS::RootedString str(cx, JS_NewStringCopyN(cx, s.str(), s.len())); // !length is checked
		rval.setString(str);
	} else {

		rval.setUndefined();
	}
	return true;
}

ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const WCStrSpec &s) {

	if ( s.str() != NULL ) {

		JS::RootedString str(cx, JS_NewUCStringCopyN(cx, s.str(), s.len()));
		rval.setString(str);
	} else {

		rval.setUndefined();
	}
	return true;
}


// since implicit constructors are not applied during template deduction, we have to force the char* to pv::StrSpec conversion here.
ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const char* s) {

	return setValue(cx, rval, CStrSpec(s));
}

// since implicit constructors are not applied during template deduction, we have to force the jschar* to pv::StrSpec conversion here.
ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const jschar* s) {

	return setValue(cx, rval, WCStrSpec(s));
}


ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const JS::HandleValue val) {

	rval.set(val);
	return true;
}

ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const JS::HandleObject obj) {

	rval.setObject(*obj);
	return true;
}

ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const JS::HandleId id) {

	rval.set(js::IdToValue(id)); // -or- return JS_IdToValue(cx, id, val);
	return true;
}

ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const JS::HandleString str) {

	rval.setString(str);
	return true;
}

// rooted IN : JS::Rooted& / JS::Rooted* / JS::MutableHandle / JS::Handle


// since neither implicit constructors nor conversion operators are applied during template deduction, we have to force the JS::Rooted to JS::Handle conversion here.
template <typename T>
ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const JS::Rooted<T>& rv) {

	return setValue(cx, rval, JS::Handle<T>(rv));
}

// since neither implicit constructors nor conversion operators are applied during template deduction, we have to force the MutableHandle to JS::Handle conversion here.
template <typename T>
ALWAYS_INLINE bool FASTCALL
setValue(JSContext *cx, JS::MutableHandleValue rval, const JS::MutableHandle<T> rv) {

	return setValue(cx, rval, JS::Handle<T>(rv));
}


////////


ALWAYS_INLINE bool FASTCALL
getValue( JSContext *cx, JS::HandleValue val, OUT jl::BufPartial* str );


ALWAYS_INLINE bool FASTCALL
getValue( JSContext *cx, JS::HandleValue val, OUT jl::BufString* str ) {

	if ( val.isString() ) { // for string literals

		JS::RootedString tmp(cx, val.toString());
		str->get(cx, tmp);
		return true;
	}
	return pv::getValue_slow(cx, val, str);
}


ALWAYS_INLINE bool FASTCALL
getValue( JSContext *cx, JS::HandleValue val, OUT bool *b ) {

	*b = val.toBoolean();
	return true;
}

ALWAYS_INLINE bool FASTCALL
getValue( JSContext *cx, JS::HandleValue val, OUT int8_t *num ) {

	return pv::getNumberValue(cx, val, num);
}


ALWAYS_INLINE bool FASTCALL
getValue( JSContext *cx, JS::HandleValue val, OUT uint8_t *num ) {

	return pv::getNumberValue(cx, val, num);
}

ALWAYS_INLINE bool FASTCALL
getValue( JSContext *cx, JS::HandleValue val, OUT int16_t *num ) {

	return pv::getNumberValue(cx, val, num);
}

ALWAYS_INLINE bool FASTCALL
getValue( JSContext *cx, JS::HandleValue val, OUT uint16_t *num ) {

	return pv::getNumberValue(cx, val, num);
}

ALWAYS_INLINE bool FASTCALL
getValue( JSContext *cx, JS::HandleValue val, OUT int32_t *num ) {

	return pv::getNumberValue(cx, val, num);
}

ALWAYS_INLINE bool FASTCALL
getValue( JSContext *cx, JS::HandleValue val, OUT uint32_t *num ) {

	return pv::getNumberValue(cx, val, num);
}

ALWAYS_INLINE bool FASTCALL
getValue( JSContext *cx, JS::HandleValue val, OUT int64_t *num ) {

	return pv::getNumberValue(cx, val, num);
}

ALWAYS_INLINE bool FASTCALL
getValue( JSContext *cx, JS::HandleValue val, OUT uint64_t *num ) {

	return pv::getNumberValue(cx, val, num);
}

ALWAYS_INLINE bool FASTCALL
getValue( JSContext *cx, JS::HandleValue val, OUT long *num ) {

	return pv::getNumberValue(cx, val, num);
}

ALWAYS_INLINE bool FASTCALL
getValue( JSContext *cx, JS::HandleValue val, OUT unsigned long *num ) {

	return pv::getNumberValue(cx, val, num);
}

ALWAYS_INLINE bool FASTCALL
getValue( JSContext *cx, JS::HandleValue val, OUT float *num ) {

	return pv::getNumberValue(cx, val, num);
}

ALWAYS_INLINE bool FASTCALL
getValue( JSContext *cx, JS::HandleValue val, OUT double *num ) {

	return pv::getNumberValue(cx, val, num);
}

/*
ALWAYS_INLINE bool FASTCALL
getValue(JSContext *cx, JS::HandleValue val, OUT JS::RootedValue* rval) {

	(*rval).set(val);
	return true;
}
*/

ALWAYS_INLINE bool FASTCALL
getValue(JSContext *cx, JS::HandleValue val, OUT JS::MutableHandleValue* rval) {

	(*rval).set(val);
	return true;
}


ALWAYS_INLINE bool FASTCALL
getValue(JSContext *cx, JS::HandleValue val, OUT JS::MutableHandleObject* obj) {

	JL_ASSERT( val.isObject(), E_VALUE, E_TYPE, E_TY_OBJECT );
	(*obj).set(&val.toObject());
	return true;
	JL_BAD;
}


// rooted OUT: JS::Rooted& / JS::Rooted* / JS::MutableHandle


// since neither implicit constructors nor conversion operators are applied during template deduction, we have to force the JS::Rooted* to JS::MutableHandle conversion here.
template <typename T>
ALWAYS_INLINE bool FASTCALL
getValue(JSContext *cx, JS::HandleValue val, OUT JS::Rooted<T>* rval) {

	JS::MutableHandle<T> tmp(rval);
	return getValue(cx, val, &tmp);
}


// since neither implicit constructors nor conversion operators are applied during template deduction, we have to force the MutableHandle& to JS::MutableHandle conversion here.
template <typename T>
ALWAYS_INLINE bool FASTCALL
getValue(JSContext *cx, JS::HandleValue val, OUT JS::MutableHandle<T> rval) {

	return getValue(cx, val, &rval);
}



////////


template <class T>
ALWAYS_INLINE bool FASTCALL
setElement( JSContext *cx, JS::HandleObject objArg, uint32_t index, const T &v /*, JS::MutableHandleValue tmpRoot*/ ) {

	JS::RootedValue value(cx);
	JL_CHK( setValue(cx, &value, v) );
	return JS_SetElement(cx, objArg, index, value);
	JL_BAD;
}

template <class T>
ALWAYS_INLINE bool FASTCALL
setElement( JSContext *cx, JS::HandleValue objArg, uint32_t index, const T &v ) {

	ASSERT( objArg.isObject() );
	JS::RootedObject obj(cx, &objArg.toObject());
	return setElement(cx, obj, index, v);
}

//

template <class T>
ALWAYS_INLINE bool FASTCALL
pushElement( JSContext *cx, JS::HandleObject objArg, const T &v ) {

	JS::RootedValue value(cx);
	uint32_t length;
	JL_CHK( JS_GetArrayLength(cx, objArg, &length) );
	JL_CHK( setValue(cx, &value, v) );
	return JS_SetElement(cx, objArg, length, value);
	JL_BAD;
}


////


template <class T>
ALWAYS_INLINE bool FASTCALL
getElement( JSContext *cx, const JS::HandleObject &objArg, uint32_t index, T* v ) {

	JS::RootedValue value(cx);
	JL_CHK( JS_ForwardGetElementTo(cx, objArg, index, objArg, &value) ); //JL_CHK( JS_GetElement(cx, objArg, index, &value) );
	return getValue(cx, value, v);
	JL_BAD;
}

template <class T>
ALWAYS_INLINE bool FASTCALL
getElement( JSContext *cx, const JS::HandleObject &objArg, uint32_t index, JS::MutableHandle<T> v ) {

	return getElement(cx, objArg, index, &v);
}


//

template <class T>
ALWAYS_INLINE bool FASTCALL
getElement( JSContext *cx, const JS::HandleValue &objArg, uint32_t index, T v ) {

	ASSERT( objArg.isObject() );
	JS::RootedObject obj(cx, &objArg.toObject());
	return getElement(cx, obj, index, v);
}



////


template <class T>
ALWAYS_INLINE bool FASTCALL
setSlot( JSContext *cx, JS::HandleObject obj, size_t slotIndex, const T &val ) {
	
	ASSERT( JS_IsNative(obj) );
	JS::RootedValue tmp(cx);
	JL_CHK( setValue(cx, &tmp, val) );
	//return JL_SetReservedSlot(obj, slotIndex, v);
	js::SetReservedSlot(obj, slotIndex, tmp); // jsfriendapi
	return true;
	JL_BAD;
}

ALWAYS_INLINE bool FASTCALL
setSlot( JSContext *cx, JS::HandleObject obj, size_t slotIndex, JS::HandleValue val ) {
	
	ASSERT( JS_IsNative(obj) );
	js::SetReservedSlot(obj, slotIndex, val); // jsfriendapi
	return true;
}

//

template <class T>
ALWAYS_INLINE bool FASTCALL
getSlot( JSContext *cx, JS::HandleObject obj, size_t slotIndex, T* rval ) {

	ASSERT( JS_IsNative(obj) );
	//JS::RootedValue v(cx);
	//JL_CHK( JL_GetReservedSlot(obj, slotIndex, v) );
	JS::RootedValue tmp(cx, js::GetReservedSlot(obj, slotIndex)); // jsfriendapi
	return getValue(cx, tmp, rval);
	JL_BAD;
}

ALWAYS_INLINE bool FASTCALL
getSlot( JSContext *cx, JS::HandleObject obj, size_t slotIndex, JS::MutableHandleValue v ) {

	v.set(js::GetReservedSlot(obj, slotIndex)); // jsfriendapi
	return true;
}


////


template <typename T>
ALWAYS_INLINE bool FASTCALL
setException( JSContext *cx, T &val ) {

	JS::RootedValue tmp(cx);
	JL_CHK( jl::setValue(cx, &tmp, val) );
	JS_SetPendingException(cx, tmp);
	return true;
	JL_BAD;
}


////

// obj

ALWAYS_INLINE bool FASTCALL
hasProperty( JSContext *cx, JS::HandleObject obj, JS::HandleId nameId ) {

	ASSERT( obj );
	bool found;
	JL_CHK( JS_HasPropertyById(cx, obj, nameId, &found) );
	return found;
	JL_BAD;
}

ALWAYS_INLINE bool FASTCALL
hasProperty( JSContext *cx, JS::HandleObject obj, const CStrSpec name ) {

	ASSERT( obj );
	bool found;
	JL_CHK( JS_HasProperty(cx, obj, name.str(), &found) );
	return found;
	JL_BAD;
}

ALWAYS_INLINE bool FASTCALL
hasProperty( JSContext *cx, JS::HandleObject obj, const WCStrSpec name ) {

	ASSERT( obj );
	bool found;
	JL_CHK( JS_HasUCProperty(cx, obj, name.str(), name.len(), &found) );
	return found;
	JL_BAD;
}


// hasProperty(cx, jsval, *)
template <class NAME>
ALWAYS_INLINE bool FASTCALL
hasProperty( JSContext *cx, JS::HandleValue objVal, NAME name ) {

	ASSERT( objVal.isObject() );
	JS::RootedObject obj(cx, &objVal.toObject());
	return hasProperty(cx, obj, name);
}


////////


template <typename T>
ALWAYS_INLINE bool FASTCALL
setProperty( JSContext *cx, JS::HandleObject objArg, JS::HandleId nameId, const T &v ) {

	JS::RootedValue value(cx);
	JL_CHK( setValue(cx, &value, v) );
	return JS_SetPropertyById(cx, objArg, nameId, value);
	JL_BAD;
}

template <typename T>
ALWAYS_INLINE bool FASTCALL
setProperty( JSContext *cx, JS::HandleObject objArg, const CStrSpec name, const T &v ) {

	JS::RootedValue value(cx);
	JL_CHK( setValue(cx, &value, v) );
	return JS_SetProperty(cx, objArg, name.str(), value);
	JL_BAD;
}

template <typename T>
ALWAYS_INLINE bool FASTCALL
setProperty( JSContext *cx, JS::HandleObject objArg, const WCStrSpec name, const T &v ) {

	JS::RootedValue value(cx);
	JL_CHK( setValue(cx, &value, v) );
	return JS_SetUCProperty(cx, objArg, name.str(), name.len(), value);
	JL_BAD;
}


ALWAYS_INLINE bool FASTCALL
setProperty( JSContext *cx, JS::HandleObject objArg, JS::HandleId nameId, JS::HandleValue value ) {

	return JS_SetPropertyById(cx, objArg, nameId, value);
}

ALWAYS_INLINE bool FASTCALL
setProperty( JSContext *cx, JS::HandleObject objArg, CStrSpec name, JS::HandleValue value ) {

	return JS_SetProperty(cx, objArg, name.str(), value);
}

ALWAYS_INLINE bool FASTCALL
setProperty( JSContext *cx, JS::HandleObject objArg, WCStrSpec name, JS::HandleValue value ) {

	return JS_SetUCProperty(cx, objArg, name.str(), name.len(), value);
}



// setProperty(cx, jsval, *, *)
template <typename N, typename T>
ALWAYS_INLINE bool FASTCALL
setProperty( JSContext *cx, JS::HandleValue objArg, N name, const T& v ) {

	ASSERT( objArg.isObject() );
	JS::RootedObject obj(cx, &objArg.toObject());
	return setProperty(cx, obj, name, v);
}


////


template <typename T>
ALWAYS_INLINE bool FASTCALL
getProperty( JSContext *cx, JS::HandleObject objArg, CStrSpec name, T* v ) {

	JS::RootedValue value(cx);
	JL_CHK( JS_GetProperty(cx, objArg, name.str(), &value) );
	JL_CHK( getValue(cx, value, v) );
	return true;
	JL_BAD;
}

template <typename T>
ALWAYS_INLINE bool FASTCALL
getProperty( JSContext *cx, JS::HandleObject objArg, WCStrSpec name, T* v ) {

	JS::RootedValue value(cx);
	JL_CHK( JS_GetUCProperty(cx, objArg, name.str(), name.len(), &value) );
	JL_CHK( getValue(cx, value, v) );
	return true;
	JL_BAD;
}

template <typename T>
ALWAYS_INLINE bool FASTCALL
getProperty( JSContext *cx, JS::HandleObject objArg, JS::HandleId nameId, T* v ) {

	JS::RootedValue value(cx);
	JL_CHK( JS_GetPropertyById(cx, objArg, nameId, &value) );
	JL_CHK( getValue(cx, value, v) );
	return true;
	JL_BAD;
}

template <typename T>
ALWAYS_INLINE bool FASTCALL
getProperty( JSContext *cx, JS::HandleObject objArg, JS::RootedId & nameId, T* v ) {

	JS::HandleId name(nameId);
	return getProperty(cx, objArg, name, v);
}

template <typename N, typename T>
ALWAYS_INLINE bool FASTCALL
getProperty( JSContext *cx, JS::HandleObject objArg, const N &name, JS::MutableHandle<T> v ) {

	return getProperty(cx, objArg, name, &v);
}



//


// setProperty(cx, jsval, *, *)
template <typename NAME, typename T>
ALWAYS_INLINE bool FASTCALL
getProperty( JSContext *cx, JS::HandleValue objArg, NAME name, T v ) {

	ASSERT( objArg.isObject() );
	JS::RootedObject obj(cx, &objArg.toObject());
	return getProperty(cx, obj, name, v);
}


////////

// vector

template <class T>
ALWAYS_INLINE bool FASTCALL
setVector( JSContext *cx, JS::MutableHandleValue rval, const T *vector, uint32_t length, bool useValArray = false ) {

	ASSERT( vector );

	JS::RootedValue value(cx);
	JS::RootedObject arrayObj(cx);

	if (likely( useValArray )) {

		JL_ASSERT_IS_OBJECT(rval, "vector");
		arrayObj = &rval.toObject();
		JL_CHK( JS_SetArrayLength(cx, arrayObj, length) );
	} else {

		arrayObj = JS_NewArrayObject(cx, length);
		JL_ASSERT_ALLOC( arrayObj );
		rval.setObject(*arrayObj);
	}

	for ( uint32_t i = 0; i < length; ++i ) {

		JL_CHK( setValue(cx, &value, vector[i]) );
		JL_CHK( JS_SetElement(cx, arrayObj, i, value) );
	}

	return true;
	JL_BAD;
}



ALWAYS_INLINE JSArrayBufferViewType JLNativeTypeToTypedArrayType( const int8_t & ) { return js::ArrayBufferView::TYPE_INT8; }
ALWAYS_INLINE JSArrayBufferViewType JLNativeTypeToTypedArrayType( const uint8_t & ) { return js::ArrayBufferView::TYPE_UINT8; }
ALWAYS_INLINE JSArrayBufferViewType JLNativeTypeToTypedArrayType( const int16_t & ) { return js::ArrayBufferView::TYPE_INT16; }
ALWAYS_INLINE JSArrayBufferViewType JLNativeTypeToTypedArrayType( const uint16_t & ) { return js::ArrayBufferView::TYPE_UINT16; }
ALWAYS_INLINE JSArrayBufferViewType JLNativeTypeToTypedArrayType( const int32_t & ) { return js::ArrayBufferView::TYPE_INT32; }
ALWAYS_INLINE JSArrayBufferViewType JLNativeTypeToTypedArrayType( const uint32_t & ) { return js::ArrayBufferView::TYPE_UINT32; }
ALWAYS_INLINE JSArrayBufferViewType JLNativeTypeToTypedArrayType( const float32_t & ) { return js::ArrayBufferView::TYPE_FLOAT32; }
ALWAYS_INLINE JSArrayBufferViewType JLNativeTypeToTypedArrayType( const float64_t & ) { return js::ArrayBufferView::TYPE_FLOAT64; }

ALWAYS_INLINE const char * JLNativeTypeToString( const int8_t & ) { return "Int8Array"; }
ALWAYS_INLINE const char * JLNativeTypeToString( const uint8_t & ) { return "Uint8Array"; }
ALWAYS_INLINE const char * JLNativeTypeToString( const int16_t & ) { return "Int16Array"; }
ALWAYS_INLINE const char * JLNativeTypeToString( const uint16_t & ) { return "Uint16Array"; }
ALWAYS_INLINE const char * JLNativeTypeToString( const int32_t & ) { return "Int32Array"; }
ALWAYS_INLINE const char * JLNativeTypeToString( const uint32_t & ) { return "Uint32Array"; }
ALWAYS_INLINE const char * JLNativeTypeToString( const float32_t & ) { return "Float32Array"; }
ALWAYS_INLINE const char * JLNativeTypeToString( const float64_t & ) { return "Float64Array"; }


template <class T>
INLINE bool FASTCALL
getTypedArray( JSContext *cx, IN JS::HandleObject obj, OUT T * vector, IN uint32_t maxLength, OUT uint32_t &actualLength ) {

	ASSERT( JS_IsTypedArrayObject(obj) );
	JL_ASSERT( JS_GetArrayBufferViewType(obj) == JLNativeTypeToTypedArrayType(*vector), E_TY_TYPEDARRAY, E_TYPE, E_NAME(JLNativeTypeToString(*vector)) );
	void *data;
	data = JS_GetArrayBufferViewData(obj);
	actualLength = JS_GetTypedArrayLength(obj);
	maxLength = jl::min( actualLength, maxLength );
	for ( uint32_t i = 0; i < maxLength; ++i )
		vector[i] = static_cast<T*>(data)[i];
	return true;
	JL_BAD;
}


template <class T>
INLINE bool FASTCALL
getArrayBuffer( JSContext *cx, JS::HandleObject obj, OUT T * vector, IN uint32_t maxLength, OUT uint32_t &actualLength ) {

	JL_IGNORE(cx);
	ASSERT( JS_IsArrayBufferObject(obj) );
	uint8_t *buffer = JS_GetArrayBufferData(obj);
	ASSERT( buffer != NULL );
	actualLength = JS_GetArrayBufferByteLength(obj);
	maxLength = jl::min( actualLength, maxLength );
	jl::memcpy(vector, buffer, maxLength);
	return true;
	JL_BAD;
}


// supports Array-like objects and typedArray
template <class T>
ALWAYS_INLINE bool FASTCALL
getVector( JSContext *cx, IN JS::HandleValue val, OUT T * vector, IN uint32_t maxLength, OUT uint32_t *actualLength ) {

	JL_ASSERT_IS_OBJECT(val, "vector");

	{

	JS::RootedValue value(cx);
	JS::RootedObject arrayObj(cx, &val.toObject());

	if (unlikely( JS_IsTypedArrayObject(arrayObj) ))
		return getTypedArray(cx, arrayObj, vector, maxLength, *actualLength);

	if (unlikely( JS_IsArrayBufferObject(arrayObj) )) {

		if ( sizeof(*vector) == 1 )
			return getArrayBuffer(cx, arrayObj, (uint8_t*)vector, maxLength, *actualLength);
		else
			JL_ERR( E_TY_ARRAYBUFFER, E_UNEXP );
	}

	JL_CHK( JS_GetArrayLength(cx, arrayObj, actualLength) );
	maxLength = jl::min( *actualLength, maxLength );
	for ( unsigned i = 0; i < maxLength; ++i ) { // while ( maxLength-- ) { // avoid reverse walk (L1 cache issue)
		
		JL_CHK( JS_ForwardGetElementTo(cx, arrayObj, i, arrayObj, &value) ); //JL_CHK( JS_GetElement(cx, objArg, index, &value) );
		JL_CHK( getValue(cx, value, &vector[i]) );
	}

	}

	return true;
	JL_BAD;
}


////


// fun as function
ALWAYS_INLINE bool FASTCALL
call(JSContext *cx, JS::HandleObject thisObj, JS::HandleValue fun, const JS::HandleValueArray& args, JS::MutableHandleValue rval) {

	return JS_CallFunctionValue(cx, thisObj, fun, args, rval);
}

ALWAYS_INLINE bool FASTCALL
call(JSContext *cx, JS::HandleObject thisObj, JS::HandleFunction fun, const JS::HandleValueArray &args, JS::MutableHandleValue rval) {

    return JS_CallFunction(cx, thisObj, fun, args, rval);
}

// fun as name (obj.fun)
ALWAYS_INLINE bool FASTCALL
call(JSContext *cx, JS::HandleObject thisObj, JS::HandleId funId, const JS::HandleValueArray& args, JS::MutableHandleValue rval) {

    JS::RootedValue funVal(cx);
	JL_CHK( JS_GetPropertyById(cx, thisObj, funId, &funVal) );
	return call(cx, thisObj, funVal, args, rval);
	JL_BAD;
}


ALWAYS_INLINE bool FASTCALL
call(JSContext *cx, JS::HandleObject thisObj, const CStrSpec name, const JS::HandleValueArray& args, JS::MutableHandleValue rval) {

    return JS_CallFunctionName(cx, thisObj, name.str(), args, rval);
}


ALWAYS_INLINE bool FASTCALL
call(JSContext *cx, JS::HandleObject thisObj, const WCStrSpec name, const JS::HandleValueArray& args, JS::MutableHandleValue rval) {

	JS::RootedValue fval(cx);
	JL_CHK( JS_GetUCProperty(cx, thisObj, name.str(), name.len(), &fval) );
	return call(cx, thisObj, fval, args, rval);
	JL_BAD;
}


// handle case when thisArg is a value and not an object
template <class FCT>
ALWAYS_INLINE bool FASTCALL
call(JSContext *cx, JS::HandleValue thisArg, const FCT &fct, const JS::HandleValueArray& args, JS::MutableHandleValue rval) {

	JS::RootedObject thisObj(cx, &thisArg.toObject());
	return call(cx, thisObj, fct, args, rval);
}

ALWAYS_INLINE bool FASTCALL
call(JSContext *cx, JS::HandleValue thisArg, const JS::RootedValue &fval, const JS::HandleValueArray& args, JS::MutableHandleValue rval) {

	return JS::Call(cx, thisArg, fval, args, rval);
}

////

template <class THIS, class FCT>
ALWAYS_INLINE bool FASTCALL
call( JSContext *cx, const THIS &thisArg, const FCT &fct, JS::MutableHandleValue rval ) {

	return call(cx, thisArg, fct, JS::HandleValueArray::empty(), rval);
}

template <class THIS, class FCT, class T1>
ALWAYS_INLINE bool FASTCALL
call( JSContext *cx, const THIS &thisArg, const FCT &fct, JS::MutableHandleValue rval, const T1 &v1 ) {

	JS::AutoValueArray<1> ava(cx);
	JL_CHK( setValue(cx, ava[0], v1) );
	return call(cx, thisArg, fct, ava, rval);
	JL_BAD;
}

template <class THIS, class FCT, class T1, class T2>
ALWAYS_INLINE bool FASTCALL
call( JSContext *cx, const THIS &thisArg, const FCT &fct, JS::MutableHandleValue rval, const T1 &v1, const T2 &v2 ) {

	JS::AutoValueArray<2> ava(cx);
	JL_CHK( setValue(cx, ava[0], v1) );
	JL_CHK( setValue(cx, ava[1], v2) );
	return call(cx, thisArg, fct, ava, rval);
	JL_BAD;
}

template <class THIS, class FCT, class T1, class T2, class T3>
ALWAYS_INLINE bool FASTCALL
call( JSContext *cx, const THIS &thisArg, const FCT &fct, JS::MutableHandleValue rval, const T1 &v1, const T2 &v2, const T3 &v3 ) {

	JS::AutoValueArray<3> ava(cx);
	JL_CHK( setValue(cx, ava[0], v1) );
	JL_CHK( setValue(cx, ava[1], v2) );
	JL_CHK( setValue(cx, ava[2], v3) );
	return call(cx, thisArg, fct, ava, rval);
	JL_BAD;
}

template <class THIS, class FCT, class T1, class T2, class T3, class T4>
ALWAYS_INLINE bool FASTCALL
call( JSContext *cx, const THIS &thisArg, const FCT &fct, JS::MutableHandleValue rval, const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4 ) {

	JS::AutoValueArray<4> ava(cx);
	JL_CHK( setValue(cx, ava[0], v1) );
	JL_CHK( setValue(cx, ava[1], v2) );
	JL_CHK( setValue(cx, ava[2], v3) );
	JL_CHK( setValue(cx, ava[3], v4) );
	return call(cx, thisArg, fct, ava, rval);
	JL_BAD;
}

template <class THIS, class FCT, class T1, class T2, class T3, class T4, class T5>
ALWAYS_INLINE bool FASTCALL
call( JSContext *cx, const THIS &thisArg, const FCT &fct, JS::MutableHandleValue rval, const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4, const T5 &v5 ) {

	JS::AutoValueArray<5> ava(cx);
	JL_CHK( setValue(cx, ava[0], v1) );
	JL_CHK( setValue(cx, ava[1], v2) );
	JL_CHK( setValue(cx, ava[2], v3) );
	JL_CHK( setValue(cx, ava[3], v4) );
	JL_CHK( setValue(cx, ava[4], v5) );
	return call(cx, thisArg, fct, ava, rval);
	JL_BAD;
}

template <class THIS, class FCT, class T1, class T2, class T3, class T4, class T5, class T6>
ALWAYS_INLINE bool FASTCALL
call( JSContext *cx, const THIS &thisArg, const FCT &fct, JS::MutableHandleValue rval, const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4, const T5 &v5, const T6 &v6 ) {

	JS::AutoValueArray<6> ava(cx);
	JL_CHK( setValue(cx, ava[0], v1) );
	JL_CHK( setValue(cx, ava[1], v2) );
	JL_CHK( setValue(cx, ava[2], v3) );
	JL_CHK( setValue(cx, ava[3], v4) );
	JL_CHK( setValue(cx, ava[4], v5) );
	JL_CHK( setValue(cx, ava[5], v6) );
	return call(cx, thisArg, fct, ava, rval);
	JL_BAD;
}

template <class THIS, class FCT, class T1, class T2, class T3, class T4, class T5, class T6, class T7>
ALWAYS_INLINE bool FASTCALL
call( JSContext *cx, const THIS &thisArg, const FCT &fct, JS::MutableHandleValue rval, const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4, const T5 &v5, const T6 &v6, const T7 &v7 ) {

	JS::AutoValueArray<7> ava(cx);
	JL_CHK( setValue(cx, ava[0], v1) );
	JL_CHK( setValue(cx, ava[1], v2) );
	JL_CHK( setValue(cx, ava[2], v3) );
	JL_CHK( setValue(cx, ava[3], v4) );
	JL_CHK( setValue(cx, ava[4], v5) );
	JL_CHK( setValue(cx, ava[5], v6) );
	JL_CHK( setValue(cx, ava[6], v7) );
	return call(cx, thisArg, fct, ava, rval);
	JL_BAD;
}

//...

////

ALWAYS_INLINE JSObject* FASTCALL
construct( JSContext *cx, JS::HandleObject proto ) {

	JS::RootedObject ctor(cx, JL_GetConstructor(cx, proto));
	JL_CHK( ctor );
	return JS_New(cx, ctor, JS::HandleValueArray::empty());
	JL_BADVAL(nullptr);
}

template <class T1>
ALWAYS_INLINE JSObject* FASTCALL
construct( JSContext *cx, JS::HandleObject proto, const T1 &v1 ) {

	JS::AutoValueArray<1> ava(cx);
	JS::RootedObject ctor(cx, JL_GetConstructor(cx, proto));
	JL_CHK( ctor );
	JL_CHK( setValue(cx, ava[0], v1) );
	return JS_New(cx, ctor, ava);
	JL_BADVAL(nullptr);
}

template <class T1, class T2>
ALWAYS_INLINE JSObject* FASTCALL
construct( JSContext *cx, JS::HandleObject proto, const T1 &v1, const T2 &v2 ) {

	JS::AutoValueArray<1> ava(cx);
	JS::RootedObject ctor(cx, JL_GetConstructor(cx, proto));
	JL_CHK( ctor );
	JL_CHK( setValue(cx, ava[0], v1) );
	JL_CHK( setValue(cx, ava[1], v2) );
	return JS_New(cx, ctor, ava.length(), ava.begin());
	JL_BADVAL(nullptr);
}

//...


////


ALWAYS_INLINE JSObject* FASTCALL
newArray( JSContext *cx ) {

	return JS_NewArrayObject(cx, 0);
}

template <typename T1>
ALWAYS_INLINE JSObject* FASTCALL
newArray( JSContext *cx, const T1 v1 ) {

	JS::AutoValueArray<1> ava(cx);
	JL_CHK( setValue(cx, ava[0], v1) );
	return JS_NewArrayObject(cx, ava);
	JL_BADVAL(nullptr);
}

template <typename T1, typename T2>
ALWAYS_INLINE JSObject* FASTCALL
newArray( JSContext *cx, const T1 v1, const T2 v2 ) {

	JS::AutoValueArray<2> ava(cx);
	JL_CHK( setValue(cx, ava[0], v1) );
	JL_CHK( setValue(cx, ava[1], v2) );
	return JS_NewArrayObject(cx, ava);
	JL_BADVAL(nullptr);
}

template <typename T1, typename T2, typename T3>
ALWAYS_INLINE JSObject* FASTCALL
newArray( JSContext *cx, const T1 v1, const T2 v2, const T3 v3 ) {

	JS::AutoValueArray<3> ava(cx);
	JL_CHK( setValue(cx, ava[0], v1) );
	JL_CHK( setValue(cx, ava[1], v2) );
	JL_CHK( setValue(cx, ava[2], v3) );
	return JS_NewArrayObject(cx, ava);
	JL_BADVAL(nullptr);
}

template <typename T1, typename T2, typename T3, typename T4>
ALWAYS_INLINE JSObject* FASTCALL
newArray( JSContext *cx, const T1 v1, const T2 v2, const T3 v3, const T4 v4 ) {

	JS::AutoValueArray<4> ava(cx);
	JL_CHK( setValue(cx, ava[0], v1) );
	JL_CHK( setValue(cx, ava[1], v2) );
	JL_CHK( setValue(cx, ava[2], v3) );
	JL_CHK( setValue(cx, ava[3], v4) );
	return JS_NewArrayObject(cx, ava);
	JL_BADVAL(nullptr);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5>
ALWAYS_INLINE JSObject* FASTCALL
newArray( JSContext *cx, const T1 v1, const T2 v2, const T3 v3, const T4 v4, const T5 v5 ) {

	JS::AutoValueArray<4> ava(cx);
	JL_CHK( setValue(cx, ava[0], v1) );
	JL_CHK( setValue(cx, ava[1], v2) );
	JL_CHK( setValue(cx, ava[2], v3) );
	JL_CHK( setValue(cx, ava[3], v4) );
	JL_CHK( setValue(cx, ava[4], v5) );
	return JS_NewArrayObject(cx, ava);
	JL_BADVAL(nullptr);
}


////////


ALWAYS_INLINE bool FASTCALL
isInstanceOf( JSContext *, JS::HandleObject obj, JSClass *clasp ) {

	return obj && JL_GetClass(obj) == clasp;
}


ALWAYS_INLINE bool FASTCALL
isObjectObject( JSContext *cx, JS::HandleObject obj ) {

//	jl::Host::getHost(cx)._objectProto

	ASSERT( obj != NULL );
	JS::RootedObject proto(cx);
	return JL_GetClassPrototype(cx, JSProto_Object, &proto) && JL_GetClass(obj) == JL_GetClass(proto);
}


ALWAYS_INLINE bool FASTCALL
isClass( JSContext *cx, JS::HandleObject obj, const JSClass *clasp ) {

	ASSERT( clasp != NULL );

	return JL_GetClass(obj) == clasp;
}

ALWAYS_INLINE bool FASTCALL
isClass( JSContext *cx, JS::HandleValue value, const JSClass *clasp ) {

	ASSERT( clasp != NULL );

	if ( value.isObject() ) {

		JS::RootedObject obj(cx, &value.toObject());
		return isClass(cx, obj, clasp);
	}
	return false;
}

ALWAYS_INLINE bool FASTCALL
isError( JSContext *cx, JS::HandleObject obj ) {

	ASSERT( obj );
	JS::RootedObject proto(cx);
	return JL_GetClassPrototype(cx, JSProto_Error, &proto) && JL_GetClass(obj) == JL_GetClass(proto); // note: JS_GetClass( (new SyntaxError()) ) => JSProto_Error
}



ALWAYS_INLINE bool FASTCALL
isCallable( JSContext *cx, JS::HandleObject obj ) {

	return JS_ObjectIsCallable(cx, obj); // FunctionClassPtr || call
}

ALWAYS_INLINE bool FASTCALL
isCallable( JSContext *cx, JS::HandleValue value ) {

	if ( value.isObject() ) {

		JS::RootedObject obj(cx, &value.toObject());
		return isCallable(cx, obj);
	}
	return false;
}


ALWAYS_INLINE bool FASTCALL
isBool( JSContext *cx, JS::HandleObject obj ) {

	if ( !obj.get() )
		return false;
	JS::RootedObject proto(cx);
	return JL_GetClassPrototype(cx, JSProto_Boolean, &proto) && isClass(cx, obj, JL_GetClass(proto));
}

ALWAYS_INLINE bool FASTCALL
isBool( JSContext *cx, JS::HandleValue value ) {

	if ( value.isBoolean() )
		return true;
	if ( value.isObject() ) {

		JS::RootedObject obj(cx, &value.toObject());
		return isBool(cx, obj);
	}
	return false;
}


ALWAYS_INLINE bool FASTCALL
isInt( JSContext *, IN JS::HandleValue value ) {

	return value.isInt32() || value.isDouble() && jl::IsIntegerValue(value.toDouble()) && jl::isInBounds<int32_t>(value.toDouble());
}


ALWAYS_INLINE bool FASTCALL
isNumber( JSContext *cx, JS::HandleObject obj ) {

	ASSERT( obj.get() );
	JS::RootedObject proto(cx);
	return JL_GetClassPrototype(cx, JSProto_Number, &proto) && isClass(cx, obj, JL_GetClass(proto));
}

ALWAYS_INLINE bool FASTCALL
isNumber( JSContext *cx, JS::HandleValue value ) {

	if ( value.isNumber() )
		return true;
	if ( value.isObject() ) {

		JS::RootedObject obj(cx, &value.toObject());
		return isNumber(cx, obj);
	}
	return false;
}


ALWAYS_INLINE bool FASTCALL
isDate( JSContext *cx, JS::HandleObject obj ) {

	ASSERT( obj.get() );
	return JS_ObjectIsDate(cx, obj);
}

ALWAYS_INLINE bool FASTCALL
isDate( JSContext *cx, JS::HandleValue value ) {

	if ( value.isObject() ) {

		JS::RootedObject obj(cx, &value.toObject());
		return isDate(cx, obj);
	}
	return false;
}


ALWAYS_INLINE bool FASTCALL
isNaN( JSContext *cx, IN JS::HandleValue val ) {

	ASSERT( (val.isDouble() && mozilla::IsNaN(val.toDouble())) == (val == JS_GetNaNValue(cx)) );
	return val == JS_GetNaNValue(cx);
}

ALWAYS_INLINE bool FASTCALL
isPInfinity( JSContext *cx, IN JS::HandleValue val ) {

	return val == JS_GetPositiveInfinityValue(cx);
}

ALWAYS_INLINE bool FASTCALL
isNInfinity( JSContext *cx, IN JS::HandleValue val ) {

	return val == JS_GetNegativeInfinityValue(cx);
}


ALWAYS_INLINE bool FASTCALL
isPositive( JSContext *cx, IN JS::HandleValue val ) {

	// handle string conversion and valueOf ?
	return ( val.toInt32() && val.toInt32() > 0 ) || ( val.isDouble() && val.toDouble() > 0 );

	//return ( val.toInt32() && val.toInt32() > 0 )
	//	|| ( val.isDouble() && val.toDouble() > 0 )
	//    || jl::isPInfinity(cx, val);
}

ALWAYS_INLINE bool FASTCALL
isNegative( JSContext *cx, IN JS::HandleValue val ) {

	// handle string conversion and valueOf ?
	return ( val.toInt32() && val.toInt32() < 0 ) || ( val.isDouble() && jl::DoubleIsNeg(val.toDouble()) );

	//return ( val.toInt32() && val.toInt32() < 0 )
	//    || ( val.isDouble() && jl::DoubleIsNeg(val.toDouble()) ) // handle NEGZERO ?
	//    || jl::isNInfinity(cx, val);
}


ALWAYS_INLINE bool FASTCALL
isString( JSContext *cx, JS::HandleObject obj ) {

	if ( !obj.get() )
		return false;
	JS::RootedObject proto(cx);
	return JL_GetClassPrototype(cx, JSProto_String, &proto) && isClass(cx, obj, JL_GetClass(proto));
}

ALWAYS_INLINE bool FASTCALL
isString( JSContext *cx, JS::HandleValue val ) {

	if ( val.isString() )
		return true;
	if ( val.isObject() ) {

		JS::RootedObject obj(cx, &val.toObject());
		return isString(cx, obj);
	}
	return false;
}



ALWAYS_INLINE bool FASTCALL
isArray( JSContext *cx, JS::HandleObject obj ) {

	return JS_IsArrayObject(cx, obj);
}

ALWAYS_INLINE bool FASTCALL
isArray( JSContext *cx, IN JS::HandleValue val ) {

	if ( val.isObject() ) {

		JS::RootedObject obj(cx, &val.toObject());
		return isArray(cx, obj);
	}
	return false;
}


// note that TypedArray, String and Array objects have a length property (ArrayBuffer does not), and unfortunately Function also have a length property.
ALWAYS_INLINE bool FASTCALL
isArrayLike( JSContext *cx, JS::HandleObject obj ) {

	if ( JS_IsArrayObject(cx, obj) )
		return true;
	return hasProperty(cx, obj, JLID(cx, length)) && !JS_ObjectIsFunction(cx, obj); // exclude (function(){}).length
}

ALWAYS_INLINE bool FASTCALL
isArrayLike( JSContext *cx, IN JS::HandleValue val ) {

	if ( val.isObject() ) {

		JS::RootedObject obj(cx, &val.toObject());
		return isArrayLike(cx, obj);
	}
	if ( isString(cx, val) )
		return true;
	return false;
}


ALWAYS_INLINE bool FASTCALL
isData( JSContext * cx, JS::HandleObject obj ) {

	return JS_IsArrayBufferObject(obj) || BufferGetInterface(cx, obj) != NULL || isArrayLike(cx, obj);
}

ALWAYS_INLINE bool FASTCALL
isData( JSContext *cx, JS::HandleValue val ) {

	if ( val.isString() )
		return true;
	if ( val.isObject() ) {

		JS::RootedObject obj(cx, &val.toObject());
		return NOIL(isData)(cx, obj);
	}
	return false;
}


ALWAYS_INLINE bool FASTCALL
isIterable( JSContext * cx, JS::HandleObject obj ) {

	return hasProperty(cx, obj, JLID(cx, next));
}

ALWAYS_INLINE bool FASTCALL
isIterable( JSContext * cx, JS::HandleValue val ) {

	return hasProperty(cx, val, JLID(cx, next));
}


ALWAYS_INLINE bool FASTCALL
isGenerator( JSContext *cx, JS::HandleValue val ) {

	// Function.prototype.isGenerator.call(Gen)
	JS::RootedObject valueObj(cx, &val.toObject());
	JS::RootedObject proto(cx);
	JS::RootedValue fct(cx), rval(cx);
	return val.isObject()
		&& JL_GetClassPrototype(cx, JSProto_Function, &proto)
	    && JS_GetPropertyById(cx, proto, JLID(cx, isGenerator), &fct)
		&& JS_CallFunctionValue(cx, valueObj, fct, JS::HandleValueArray::empty(), &rval)
		&& rval == JSVAL_TRUE;
}


ALWAYS_INLINE bool FASTCALL
isStopIteration( JSContext *cx, JS::HandleObject obj ) {

	// JS_IsStopIteration
	JS::RootedObject proto(cx);
	return JL_GetClassPrototype(cx, JSProto_StopIteration, &proto)
		&& JL_GetClass(obj) == JL_GetClass(proto);
}

ALWAYS_INLINE bool FASTCALL
isStopIterationExceptionPending( JSContext *cx ) {

	JS::RootedValue ex(cx);
	if ( !JS_GetPendingException(cx, &ex) || !ex.isObject() ) // note: JS_GetPendingException returns false if no exception is pending.
		return false;
	JS::RootedObject exObj(cx, &ex.toObject());
	return isStopIteration(cx, exObj);
}


INLINE bool FASTCALL
getPrimitive( JSContext * RESTRICT cx, IN JS::HandleValue val, OUT JS::MutableHandleValue rval ) {

	if ( val.isPrimitive() ) {

		rval.set(val);
		return true;
	}
	JS::RootedObject obj(cx, &val.toObject());
	JL_CHK( jl::call(cx, obj, JLID(cx, valueOf), rval) );
	if ( !rval.isPrimitive() )
		JL_CHK( jl::call(cx, obj, JLID(cx, toString), rval) );

	return true;
	JL_BAD;
}


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
setScriptLocation( JSContext * RESTRICT cx, IN OUT JS::MutableHandleObject obj ) {

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


ALWAYS_INLINE JSObject* FASTCALL
newObjectWithGivenProto( JSContext *cx, const JSClass *clasp, IN JS::HandleObject proto, IN JS::HandleObject parent/* = JS::NullPtr()*/ ) {

	ASSERT_IF( proto != NULL, JL_GetParent(cx, proto) != NULL );
	// Doc. JS_NewObject, jl::newObjectWithGivenProto behaves exactly the same, except that if proto is NULL, it creates an object with no prototype.
	JS::RootedObject obj(cx, JS_NewObjectWithGivenProto(cx, clasp, proto, parent));  // (TBD) test if parent is ok (see bug 688510)
	ASSERT( JL_GetParent(cx, obj) != NULL );
	return obj;
}

ALWAYS_INLINE JSObject* FASTCALL
newObjectWithoutProto( JSContext *cx ) {

	JS::RootedObject obj(cx, jl::newObjectWithGivenProto(cx, NULL, JS::NullPtr())); // JL_GetGlobal(cx) ??
	ASSERT( JL_GetParent(cx, obj) != NULL );
	ASSERT( JL_GetPrototype(cx, obj) == NULL );
	return obj;
}


INLINE NEVER_INLINE bool FASTCALL
getMatrix44( JSContext * cx, IN JS::HandleValue val, OUT float32_t ** m ) {

	static float32_t Matrix44IdentityValue[16] = {
		 1.0f, 0.0f, 0.0f, 0.0f,
		 0.0f, 1.0f, 0.0f, 0.0f,
		 0.0f, 0.0f, 1.0f, 0.0f,
		 0.0f, 0.0f, 0.0f, 1.0f
	};

	if ( val.isNull() ) {

		jl::memcpy(*m, &Matrix44IdentityValue, sizeof(Matrix44IdentityValue));
		return true;
	}

	JL_ASSERT_IS_OBJECT(val, "matrix44");

	{

	JS::RootedObject matrixObj(cx, &val.toObject());

	NIMatrix44Get Matrix44Get;
	Matrix44Get = Matrix44GetInterface(cx, matrixObj);
	if ( Matrix44Get )
		return Matrix44Get(cx, matrixObj, m);

	if ( JS_IsFloat32Array(matrixObj) ) {

		if ( JS_GetTypedArrayLength(matrixObj) == 16 ) {

			jl::memcpy(*m, JS_GetFloat32ArrayData(matrixObj), sizeof(float32_t) * 16);
			return true;
		}
	}

	if ( jl::isArrayLike(cx, matrixObj) ) {

		uint32_t length;
		JS::RootedValue element(cx);

		JL_CHK( JL_GetElement(cx, matrixObj, 0, &element) );
		if ( jl::isArrayLike(cx, element) ) { // support for [ [1,1,1,1], [2,2,2,2], [3,3,3,3], [4,4,4,4] ] matrix

			JL_CHK( jl::getVector(cx, element, (*m)+0, 4, &length ) );
			JL_ASSERT( length == 4, E_VALUE, E_STR("matrix44[0]"), E_TYPE, E_TY_NVECTOR(4) );

			JL_CHK( JL_GetElement(cx, matrixObj, 1, &element) );
			JL_CHK( jl::getVector(cx, element, (*m)+4, 4, &length ) );
			JL_ASSERT_IS_ARRAY( element, "matrix44[1]" );
			JL_ASSERT( length == 4, E_VALUE, E_STR("matrix44[1]"), E_TYPE, E_TY_NVECTOR(4) );

			JL_CHK( JL_GetElement(cx, matrixObj, 2, &element) );
			JL_CHK( jl::getVector(cx, element, (*m)+8, 4, &length ) );
			JL_ASSERT_IS_ARRAY( element, "matrix44[2]" );
			JL_ASSERT( length == 4, E_VALUE, E_STR("matrix44[2]"), E_TYPE, E_TY_NVECTOR(4) );

			JL_CHK( JL_GetElement(cx, matrixObj, 3, &element) );
			JL_CHK( jl::getVector(cx, element, (*m)+12, 4, &length ) );
			JL_ASSERT_IS_ARRAY( element, "matrix44[3]" );
			JL_ASSERT( length == 4, E_VALUE, E_STR("matrix44[3]"), E_TYPE, E_TY_NVECTOR(4) );
			return true;
		}

		JL_CHK( jl::getVector(cx, val, *m, 16, &length ) );  // support for [ 1,1,1,1, 2,2,2,2, 3,3,3,3, 4,4,4,4 ] matrix
		JL_ASSERT( length == 16, E_VALUE, E_STR("matrix44"), E_TYPE, E_TY_NVECTOR(16) );
		return true;
	}

	}

	JL_ERR( E_VALUE, E_STR("matrix44"), E_INVALID );
	JL_BAD;
}


JL_END_NAMESPACE


// Define

template <class T>
ALWAYS_INLINE bool FASTCALL
JL_DefineProperty( JSContext *cx, IN JS::HandleObject obj, const char *name, const T &cval, bool visible = true, bool modifiable = true ) {

	JS::RootedValue tmp(cx);
	return jl::setValue(cx, tmp, cval) && JS_DefineProperty(cx, obj, name, tmp, (modifiable ? 0 : JSPROP_READONLY | JSPROP_PERMANENT) | (visible ? JSPROP_ENUMERATE : 0) );
}

ALWAYS_INLINE bool FASTCALL
JL_DefineProperty( JSContext *cx, IN JS::HandleObject obj, const char *name, IN JS::HandleValue val, bool visible = true, bool modifiable = true ) {

	return JS_DefineProperty(cx, obj, name, val, (modifiable ? 0 : JSPROP_READONLY | JSPROP_PERMANENT) | (visible ? JSPROP_ENUMERATE : 0) );
}


template <class T>
ALWAYS_INLINE bool FASTCALL
JL_DefineProperty( JSContext *cx, IN JS::HandleObject obj, jsid id, const T &cval, bool visible = true, bool modifiable = true ) {

	JS::RootedValue tmp(cx);
	return jl::setValue(cx, tmp, cval) && JS_DefinePropertyById(cx, obj, id, tmp, (modifiable ? 0 : JSPROP_READONLY | JSPROP_PERMANENT) | (visible ? JSPROP_ENUMERATE : 0) );
}

ALWAYS_INLINE bool FASTCALL
JL_DefineProperty( JSContext *cx, IN JS::HandleObject obj, jsid id, IN JS::HandleValue val, bool visible = true, bool modifiable = true ) {

	return JS_DefinePropertyById(cx, obj, id, val, NULL, NULL, (modifiable ? 0 : JSPROP_READONLY | JSPROP_PERMANENT) | (visible ? JSPROP_ENUMERATE : 0) );
}


/*
ALWAYS_INLINE bool FASTCALL
JL_JsvalToJsid( JSContext * RESTRICT cx, IN JS::HandleValue val, JS::MutableHandleId id ) {

	if ( val.isString() ) {

		JS::RootedString str(cx, val.toString());
		id.set(JL_StringToJsid(cx, str));
		ASSERT( JSID_IS_STRING( id ) || JSID_IS_INT( id ) ); // see AtomToId()
	} else
	if ( JSVAL_IS_INT( val ) && INT_FITS_IN_JSID( val.toInt32() ) ) {

		id.set(INT_TO_JSID( val.toInt32() ));
	} else
	if ( val.isObject() ) {

		id.set(OBJECT_TO_JSID( &val.toObject() ));
	} else
	if ( val.isUndefined() ) {

		id.set(JSID_VOID);
	} else
	if ( val.isNull() ) {

		id.set(OBJECT_TO_JSID( val.toObjectOrNull() ));
	} else
		return JS_ValueToId(cx, val, id);
	return true;
}
*/

/*
ALWAYS_INLINE bool FASTCALL
JL_JsidToJsval( JSContext * RESTRICT cx, IN jsid id, OUT JS::MutableHandleValue val ) {

	if (JSID_IS_STRING(id)) {

		val.set(JS::StringValue(JSID_TO_STRING(id)));
	} else
	if (likely( JSID_IS_INT(id) )) {

		val.set(JS::Int32Value(JSID_TO_INT(id)));
	} else
	if (likely( JSID_IS_OBJECT(id) )) {

		val.set(JS::ObjectValue(*JSID_TO_OBJECT(id)));
	} else
	if (likely( JSID_IS_VOID(id) )) {

		val.setUndefined();
	} else
		return JS_IdToValue(cx, id, val);
	return true;
}
*/



///////////////////////////////////////////////////////////////////////////////
// Buffer

// JS_AllocateArrayBufferContents

/*
ALWAYS_INLINE uint8_t* FASTCALL
JL_DataBufferAlloc( JSContext *, size_t nbytes ) {

	return (uint8_t*)jl_malloc(nbytes);
}

ALWAYS_INLINE uint8_t* FASTCALL
JL_DataBufferRealloc( JSContext *, uint8_t *data, size_t nbytes ) {

	return (uint8_t*)jl_realloc(data, nbytes);
}

ALWAYS_INLINE void FASTCALL
JL_DataBufferFree( JSContext *, uint8_t *data ) {

	return jl_free(data);
}
*/

//
/*
ALWAYS_INLINE uint8_t* FASTCALL
JL_NewBuffer( JSContext *cx, size_t nbytes, OUT JS::MutableHandleValue vp ) {

	JS::RootedObject bufferObj(cx, JS_NewArrayBuffer(cx, nbytes));
	if ( bufferObj ) {

		vp.setObject(*bufferObj);
		return JS_GetArrayBufferData(bufferObj);
	} else {

		return NULL;
	}
}
*/

/*
ALWAYS_INLINE bool FASTCALL
JL_NewBufferCopyN( JSContext *cx, IN const void *src, IN size_t nbytes, OUT JS::MutableHandleValue vp ) {

	JS::RootedObject bufferObj(cx, JS_NewArrayBuffer(cx, nbytes));
	if ( !bufferObj )
		return false;
	void *data = JS_GetArrayBufferData(bufferObj);
	if ( !data )
		return false;
	jl::memcpy(data, src, nbytes);
	vp.setObject(*bufferObj);
	return true;
}
*/

/*
ALWAYS_INLINE bool FASTCALL
JL_NewBufferGetOwnership( JSContext *cx, IN void *src, IN size_t nbytes, OUT JS::MutableHandleValue rval ) {

	// (TBD) need to handle ownership properly
	bool ok = JL_NewBufferCopyN(cx, src, nbytes, rval);
	jl_free(src);
	return ok;
}
*/

/*
ALWAYS_INLINE bool FASTCALL
JL_NewEmptyBuffer( JSContext *cx, OUT JS::MutableHandleValue vp ) {

	JS::RootedObject obj(cx, JS_NewArrayBuffer(cx, 0));
	if ( obj ) {

		vp.setObject(*obj);
		return true;
	}
	return false;
}
*/

/*
template <class T>
ALWAYS_INLINE bool FASTCALL
JL_FreeBuffer( JSContext *, T ) {

	// do nothing at the moment. The CG will free the buffer.
	return true;
}
*/

/*
ALWAYS_INLINE uint8_t* FASTCALL
JL_ChangeBufferLength( JSContext *cx, IN OUT JS::MutableHandleValue vp, size_t nbytes ) {

	// need to create a new buffer because ArrayBuffer does not support realloc nor length changing, then we copy it in a new one.
	// see JS_ReallocateArrayBufferContents

	ASSERT( vp.isObject() );
	JS::RootedObject arrayBufferObj(cx, &vp.toObject());
	ASSERT( JS_IsArrayBufferObject(arrayBufferObj) );
	uint32_t bufferLen = JS_GetArrayBufferByteLength(arrayBufferObj);
	void *bufferData = JS_GetArrayBufferData(arrayBufferObj);

	if ( nbytes == bufferLen )
		return (uint8_t*)bufferData;

	JS::RootedObject newBufferObj(cx);
	void *newBufferData;
	if ( nbytes < bufferLen ) {

		newBufferObj = JS_NewArrayBuffer(cx, nbytes);
		void *data = JS_GetArrayBufferData(newBufferObj);
		jl::memcpy(data, bufferData, nbytes);

		if ( !newBufferObj )
			return false;
		newBufferData = JS_GetArrayBufferData(arrayBufferObj);
	} else {

		newBufferObj = JS_NewArrayBuffer(cx, nbytes);
		if ( !newBufferObj )
			return false;
		newBufferData = JS_GetArrayBufferData(arrayBufferObj);
		jl::memcpy(newBufferData, bufferData, bufferLen);
	}
	vp.setObject(*newBufferObj);
	return (uint8_t*)newBufferData;
}
*/


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

ALWAYS_INLINE bool FASTCALL
JL_MaybeRealloc( size_t requested, size_t received ) {

	return requested != 0 && (128 * received / requested < 96) && (requested - received > 128); // less than 75% AND at least 128 bytes more 
}


INLINE NEVER_INLINE bool FASTCALL
JL_ThrowOSErrorCode( JSContext *cx, JLSystemErrorCode errorCode, const char *moduleName ) {

	char errMsg[1024];
	JLSysetmErrorMessage(errMsg, sizeof(errMsg), errorCode, moduleName);
	JL_ERR( E_OS, E_DETAILS, E_STR(errMsg) );
	JL_BAD;
}


INLINE NEVER_INLINE bool FASTCALL
JL_ThrowOSError( JSContext *cx ) {

	char errMsg[1024];
	JLLastSysetmErrorMessage(errMsg, sizeof(errMsg));
	JL_ERR( E_OS, E_DETAILS, E_STR(errMsg) );
	JL_BAD;
}


ALWAYS_INLINE JSContext* FASTCALL
JL_GetFirstContext( JSRuntime *rt ) {

	JSContext *cx = NULL;
	ASSERT( rt != NULL );
	JS_ContextIterator(rt, &cx);
	JS_ASSERT( cx != NULL );
	return cx;
}


ALWAYS_INLINE bool FASTCALL
JL_InheritFrom( JSContext *cx, JS::HandleObject obj, const JSClass *clasp ) {

	JS::RootedObject proto(cx, obj);
	while ( proto != NULL ) {

		if ( JL_GetClass(proto) == clasp )
			return true;

		if ( !JS_GetPrototype(cx, proto, &proto) )
			return false;
	}
	return false;
}


ALWAYS_INLINE bool FASTCALL
JL_ProtoOfInheritFrom( JSContext *cx, JS::HandleObject obj, const JSClass *clasp ) {

    JS::RootedObject proto(cx);
	if ( !JS_GetPrototype(cx, obj, &proto) )
		return false;
	while ( proto != NULL ) {

		if ( JL_GetClass(proto) == clasp )
			return true;
		if ( !JS_GetPrototype(cx, proto, &proto) )
			return false;
	}
	return false;
}



/* no more used ?
INLINE bool FASTCALL
JL_Eval( JSContext *cx, JSString *source, JS::Value *rval ) { // used in JS::Valueserializer.h

	// GetOriginalEval
 	JS::Value argv = STRING_TO_JSVAL(source);
	return JL_CallFunctionId(cx, JL_GetGlobal(cx), JLID(cx, eval), 1, &argv, rval); // see JS_EvaluateUCScript
}
*/



/*
INLINE bool FASTCALL
JL_JsvalToPrimitive( JSContext * RESTRICT cx, IN JS::HandleValue val, OUT JS::MutableHandleValue rval ) { // prev JL_ValueOf

	if ( val.isPrimitive() ) {

		rval.set(val);
		return true;
	}
	JS::RootedObject obj(cx, &val.toObject());
	JL_CHK( jl::call(cx, obj, JLID(cx, valueOf), rval) );
	if ( !rval.isPrimitive() )
		JL_CHK( jl::call(cx, obj, JLID(cx, toString), rval) );

	return true;
	JL_BAD;
}
*/



// XDR and bytecode compatibility:
//   Backward compatibility is when you run old bytecode on a new engine, and that should work.
//   What you seem to want is forward compatibility, which is new bytecode on an old engine, which is nothing we've ever promised.
// year 2038 bug :
//   Later than midnight, January 1, 1970, and before 19:14:07 January 18, 2038, UTC ( see _stat64 )
// note:
//	You really want to use Script.prototype.thaw and Script.prototype.freeze.  At
//	least imitate their implementations in jsscript.c (script_thaw and
//	script_freeze).  But you might do better to call these via JS_CallFunctionName
//	on your script object.
//
//	/be
INLINE NEVER_INLINE JSScript* FASTCALL
JL_LoadScript(JSContext * RESTRICT cx, IN JS::HandleObject obj, const char * RESTRICT fileName, jl::EncodingType encoding, bool useCompFile, bool saveCompFile) {

	char *scriptBuffer = NULL;
	size_t scriptFileSize;
	jschar *scriptText = NULL;
	size_t scriptTextLength;

	JS::RootedScript script(cx);
	JS::CompileOptions compileOptions(cx);

	//JS::CompartmentOptionsRef(cx).cloneSingletonsOverride().set(true);

	void *data = NULL;

	char compiledFileName[PATH_MAX];
	strcpy( compiledFileName, fileName );
	strcat( compiledFileName, "xdr" );

	struct stat srcFileStat, compFileStat;
	bool hasSrcFile = stat(fileName, &srcFileStat) != -1; // errno == ENOENT
	bool hasCompFile = stat(compiledFileName, &compFileStat) != -1;
	bool compFileUpToDate = ( hasCompFile && !hasSrcFile ) || ( hasCompFile && hasSrcFile && (compFileStat.st_mtime > srcFileStat.st_mtime) ); // true if comp file is up to date or alone

	JL_CHKM( hasSrcFile || hasCompFile, E_SCRIPT, E_NAME(fileName), E_OR, E_STR(compiledFileName), E_NOTFOUND );

	if ( useCompFile && compFileUpToDate ) {

		int file = open(compiledFileName, O_RDONLY | O_BINARY | O_SEQUENTIAL);
		JL_CHKM( file != -1, E_FILE, E_NAME(compiledFileName), E_ACCESS ); // "Unable to open file \"%s\" for reading.", compiledFileName
		size_t compFileSize;
		compFileSize = compFileStat.st_size; // filelength(file); ?
		data = jl_malloca(compFileSize);
		JL_ASSERT_ALLOC( data );
		// jl::isInBounds<unsigned int>(
		int readCount = read(file, data, compFileSize); // here we can use "Memory-Mapped I/O Functions" ( http://developer.mozilla.org/en/docs/NSPR_API_Reference:I/O_Functions#Memory-Mapped_I.2FO_Functions )
		JL_CHKM( readCount >= 0 && (size_t)readCount == compFileSize, E_FILE, E_NAME(compiledFileName), E_READ ); // "Unable to read the file \"%s\" ", compiledFileName
		close( file );

		// we want silent failures.
//		JSErrorReporter prevErrorReporter = JS_SetErrorReporter(cx, NULL);
//		JSDebugErrorHook debugErrorHook = cx->debugHooks->debugErrorHook;
//		void *debugErrorHookData = cx->debugHooks->debugErrorHookData;
//		JS_SetDebugErrorHook(JL_GetRuntime(cx), NULL, NULL);

		script = JS_DecodeScript(cx, data, readCount, NULL, NULL);

//		JS_SetDebugErrorHook(JL_GetRuntime(cx), debugErrorHook, debugErrorHookData);
//		if (cx->lastMessage)
//			JS_free(cx, cx->lastMessage);
//		cx->lastMessage = NULL;
//		JS_SetErrorReporter(cx, prevErrorReporter);

		if ( script ) {

			// (TBD) manage BIG_ENDIAN here ?
			jl_freea(data);
			data = NULL;

//			JL_ASSERT_WARN( JS_GetScriptVersion(cx, script) >= JS_GetVersion(cx), E_NAME(compiledFileName), E_STR("XDR"), E_VERSION );
			goto good;
		} else {

			JS_ClearPendingException(cx);

//			JL_REPORT_WARNING_NUM( JLSMSG_RUNTIME_ERROR, "bad script XDR magic number");

//			if ( JL_IsExceptionPending(cx) )
//				JS_ReportPendingException(cx);

			jl_freea(data);
			data = NULL;
//			if ( JL_IsExceptionPending(cx) )
//				JS_ClearPendingException(cx);
		}
	}

	if ( !hasSrcFile )
		goto bad; // no source, no compiled version of the source, die.

	// doc.
	//   JSOPTION_COMPILE_N_GO: caller of JS_Compile*Script promises to execute compiled script once only; enables compile-time scope chain resolution of consts.
	// see https://bugzilla.mozilla.org/show_bug.cgi?id=494363
	// see also bug 920322 : Add support for compileAndGo optimizations to XDRScript
	//if ( saveCompFile ) // saving the compiled file mean that we cannot promise to execute compiled script once only.
	//	compileOptions.setCompileAndGo(false);  // JS_SetOptions(cx, prevOpts & ~JSOPTION_COMPILE_N_GO); // previous options a restored below.

#define JL_UC
#ifndef JL_UC

	FILE *scriptFile;
	scriptFile = fopen(fileName, "r");
	JL_ASSERT( scriptFile != NULL, E_FILE, E_NAME(scriptFile), E_ACCESS ); // "Script file \"%s\" cannot be opened."

	// shebang support
	char s, b;
	s = getc(scriptFile);
	if ( s == '#' ) {

		b = getc(scriptFile);
		if ( b == '!' ) {

			ungetc('/', scriptFile);
			ungetc('/', scriptFile);
		} else {

			ungetc(b, scriptFile);
			ungetc(s, scriptFile);
		}
	} else {

		ungetc(s, scriptFile);
	}

	script = JS_CompileFileHandle(cx, obj, fileName, scriptFile);
	fclose(scriptFile);

#else //JL_UC


	int scriptFile;
	scriptFile = open(fileName, O_RDONLY | O_BINARY | O_SEQUENTIAL);
	JL_CHKM( scriptFile >= 0, E_FILE, E_NAME(fileName), E_ACCESS ); // "Unable to open file \"%s\" for reading.", fileName

	scriptFileSize = lseek(scriptFile, 0, SEEK_END);
	ASSERT( scriptFileSize <= UINT_MAX ); // Compiled file too big.

	lseek(scriptFile, 0, SEEK_SET); // see tell(scriptFile);
	scriptBuffer = (char*)jl_malloca(scriptFileSize);
	int res;
	res = read(scriptFile, (void*)scriptBuffer, (unsigned int)scriptFileSize);
	close(scriptFile);

	//JL_CHKM( res >= 0, "Unable to read file \"%s\".", fileName );
	JL_CHKM( res >= 0, E_FILE, E_NAME(fileName), E_READ );


	ASSERT( (size_t)res == scriptFileSize );
	scriptFileSize = (size_t)res;

	if ( encoding == jl::ENC_UNKNOWN )
		encoding = jl::DetectEncoding((uint8_t**)&scriptBuffer, &scriptFileSize);

	switch ( encoding ) {
		default:
			JL_WARN( E_SCRIPT, E_ENCODING, E_INVALID, E_NAME(fileName) );
			// then use ASCII as default.
		case jl::ENC_ASCII: {

			char *scriptText = scriptBuffer;
			size_t scriptTextLength = scriptFileSize;
			if ( scriptTextLength >= 2 && scriptText[0] == '#' && scriptText[1] == '!' ) { // shebang support

				scriptText[0] = '/';
				scriptText[1] = '/';
			}
			compileOptions.setFileAndLine(fileName, 1);
			compileOptions.setCompileAndGo(true);

			//printf("DEBUG %p", obj.address());

			script = JS_CompileScript(cx, obj, scriptText, scriptTextLength, compileOptions);
			break;
		}
		case jl::ENC_UTF16le: { // (TBD) support big-endian

			jschar *scriptText = (jschar*)scriptBuffer;
			size_t scriptTextLength = scriptFileSize / 2;
			if ( scriptTextLength >= 2 && scriptText[0] == L('#') && scriptText[1] == L('!') ) { // shebang support

				scriptText[0] = L('/');
				scriptText[1] = L('/');
			}
			compileOptions.setFileAndLine(fileName, 1);
			compileOptions.setCompileAndGo(true);

			//script = JS_CompileUCScript(cx, obj, scriptText, scriptTextLength, compileOptions);
			script = JS::Compile(cx, obj, compileOptions, scriptText, scriptTextLength);
			break;
		}
		case jl::ENC_UTF8: {

			scriptText = (jschar*)jl_malloca(scriptFileSize * 2);
			scriptTextLength = scriptFileSize * 2;
			JL_CHKM( jl::UTF8ToUTF16LE((unsigned char*)scriptText, &scriptTextLength, (unsigned char*)scriptBuffer, &scriptFileSize) >= 0, E_SCRIPT, E_ENCODING, E_INVALID, E_COMMENT("UTF8") ); // "Unable do decode UTF8 data."

			if ( scriptTextLength >= 2 && scriptText[0] == L('#') && scriptText[1] == L('!') ) { // shebang support

				scriptText[0] = L('/');
				scriptText[1] = L('/');
			}
			compileOptions.setFileAndLine(fileName, 1);
			compileOptions.setCompileAndGo(true);

			script = JS_CompileUCScript(cx, obj, scriptText, scriptTextLength, compileOptions);
			break;
		}
	}

	//JL_CHKM( script, E_SCRIPT, E_NAME(fileName), E_COMPILE ); // do not overwrite the default exception.
	JL_CHK( script );

#endif //JL_UC

	if ( !saveCompFile )
		goto good;

	int file;
	file = open(compiledFileName, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY | O_SEQUENTIAL, srcFileStat.st_mode);
	if ( file == -1 ) // if the file cannot be write, this is not an error ( eg. read-only drive )
		goto good;

	uint32_t length;
	void *buf;
	buf = JS_EncodeScript(cx, script, &length);
	JL_CHK( buf );

	// manage BIG_ENDIAN here ?
	JL_CHK( write(file, buf, length) != -1 ); // On error, -1 is returned, and errno is set appropriately.
	JL_CHK( close(file) == 0 );
	js_free(buf);
	goto good;

good:

	if ( scriptBuffer )
		jl_freea(scriptBuffer);

	if ( scriptText )
		jl_freea(scriptText);

//	JS_SetOptions(cx, prevOpts);
	return script;

bad:

	jl_freea(scriptBuffer); // jl_freea(NULL) is legal
	jl_freea(scriptText);
//	JS_SetOptions(cx, prevOpts);
	jl_freea(data); // jl_free(NULL) is legal
	return NULL; // report a warning ?
}


ALWAYS_INLINE bool FASTCALL
ExecuteScriptText( JSContext *cx, IN JS::HandleObject obj, const char *scriptText, bool compileOnly, OUT JS::MutableHandleValue rval ) {

//	uint32_t prevOpt = JS_SetOptions(cx, JS_GetOptions(cx) /*| JSOPTION_COMPILE_N_GO*/); //  | JSOPTION_DONT_REPORT_UNCAUGHT
	// JSOPTION_COMPILE_N_GO:
	//  caller of JS_Compile*Script promises to execute compiled script once only; enables compile-time scope chain resolution of consts.
	// JSOPTION_DONT_REPORT_UNCAUGHT:
	//  When returning from the outermost API call, prevent uncaught exceptions from being converted to error reports
	//  we can use JS_ReportPendingException to report it manually

	JS::AutoSaveContextOptions autoCxOpts(cx);

// compile & executes the script

	//JSPrincipals *principals = (JSPrincipals*)jl_malloc(sizeof(JSPrincipals));
	//JSPrincipals tmp = {0};
	//*principals = tmp;
	//principals->codebase = (char*)jl_malloc(PATH_MAX);
	//strncpy(principals->codebase, scriptFileName, PATH_MAX-1);
	//principals->refcount = 1;
	//principals->destroy = HostPrincipalsDestroy;

	JS::RootedScript script(cx);
	JS::CompileOptions compileOptions(cx);
	compileOptions.setFileAndLine("inline", 1);
	compileOptions.setCompileAndGo(true);

	script = JS_CompileScript(cx, obj, scriptText, strlen(scriptText), compileOptions);
	JL_CHK( script );

	// mendatory else the exception is converted into an error before JL_IsExceptionPending can be used. Exceptions can be reported with JS_ReportPendingException().
	JS::ContextOptionsRef(cx).setDontReportUncaught(true);

	// You need to protect a JSScript (via a rooted script object) if and only if a garbage collection can occur between compilation and the start of execution.
	if ( !compileOnly )
		JL_CHK( JS_ExecuteScript(cx, obj, script, rval) ); // MUST be executed only once ( JSOPTION_COMPILE_N_GO )
	else
		rval.setUndefined();

	return true;
	JL_BAD;
}


ALWAYS_INLINE bool FASTCALL
ExecuteScriptFileName( JSContext *cx, IN JS::HandleObject obj, const char *scriptFileName, bool compileOnly, OUT JS::MutableHandleValue rval ) {

	JS::AutoSaveContextOptions autoCxOpts(cx);

	JS::RootedScript script(cx, JL_LoadScript(cx, obj, scriptFileName, jl::ENC_UNKNOWN, true, false)); // use xdr if available, but don't save it.
	JL_CHK( script );


	// mendatory else the exception is converted into an error before JL_IsExceptionPending can be used. Exceptions can be reported with JS_ReportPendingException().
	JS::ContextOptionsRef(cx).setDontReportUncaught(true);

	// You need to protect a JSScript (via a rooted script object) if and only if a garbage collection can occur between compilation and the start of execution.
	if ( !compileOnly )
		JL_CHK( JS_ExecuteScript(cx, obj, script, rval) ); // MUST be executed only once ( JSOPTION_COMPILE_N_GO )
	else
		rval.setUndefined();

	return true;
	JL_BAD;
}




INLINE NEVER_INLINE bool FASTCALL
JL_DebugPrintScriptLocation( JSContext *cx ) {

	JS::AutoFilename autoFilename;
	const char *filename;
	unsigned lineno;
	JL_CHK( JS::DescribeScriptedCaller(cx, &autoFilename, &lineno) );

	if ( autoFilename.get() == NULL || *autoFilename.get() == '\0' )
		filename = "<no_filename>";
	else
		filename = autoFilename.get();

	fprintf(stderr, "%s:%d\n", filename, lineno);
	return true;
	JL_BAD;
}




///////////////////////////////////////////////////////////////////////////////
// NativeInterface API

ALWAYS_INLINE bool
ReserveNativeInterface( JSContext *cx, JS::HandleObject obj, JS::HandleId id ) {

	JSID_IS_STRING(id) && !JSID_IS_ZERO(id);

	return JS_DefinePropertyById(cx, obj, id, JSVAL_VOID, NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
}


template <class T>
ALWAYS_INLINE bool
SetNativeInterface( JSContext *cx, JS::HandleObject obj, JS::HandleId id, const T nativeFct ) {

	JSID_IS_STRING(id) && !JSID_IS_ZERO(id);

	if ( nativeFct != NULL ) {

		JL_CHK( JS_DefinePropertyById(cx, obj, id, JSVAL_TRUE, NULL, (JSStrictPropertyOp)nativeFct, JSPROP_READONLY | JSPROP_PERMANENT) ); // hacking the setter of a read-only property seems safe.
	} else {

		JL_CHK( ReserveNativeInterface(cx, obj, id) );
	}
	return true;
	JL_BAD;
}


template <class T>
ALWAYS_INLINE const T
GetNativeInterface( JSContext *cx, JS::HandleObject obj, JS::HandleId id ) {

	ASSERT( JSID_IS_STRING(id) && !JSID_IS_ZERO(id) );
	
	JS::Rooted<JSPropertyDescriptor> desc(cx);
	if ( JS_GetPropertyDescriptorById(cx, obj, id, 0, &desc) ) {

		return desc.object() == obj && desc.setter() != JS_StrictPropertyStub ? (const T)desc.setter() : NULL; // is JS_PropertyStub when eg. Stringify({_NI_BufferGet:function() {} })
	} else {

		return NULL;
	}
}



///////////////////////////////////////////////////////////////////////////////
// NativeInterface StreamRead

ALWAYS_INLINE bool
ReserveStreamReadInterface( JSContext *cx, JS::HandleObject obj ) {

	return ReserveNativeInterface(cx, obj, JLID(cx, _NI_StreamRead) );
}


ALWAYS_INLINE bool
SetStreamReadInterface( JSContext *cx, JS::HandleObject obj, NIStreamRead pFct ) {

	return SetNativeInterface( cx, obj, JLID(cx, _NI_StreamRead), pFct );
}


ALWAYS_INLINE NIStreamRead
StreamReadNativeInterface( JSContext *cx, JS::HandleObject obj ) {

	return GetNativeInterface<NIStreamRead>(cx, obj, JLID(cx, _NI_StreamRead));
}


INLINE bool
JSStreamRead( JSContext * RESTRICT cx, JS::HandleObject obj, char * RESTRICT buffer, size_t * RESTRICT amount ) {

	JS::RootedValue rval(cx);
	JL_CHK( jl::call(cx, obj, JLID(cx, read), &rval, *amount) );
	if ( rval.isUndefined() ) { // (TBD) with sockets, undefined mean 'closed', that is not supported by NIStreamRead.

		*amount = 0;
	} else {

		jl::BufString str;
		JL_CHK( jl::getValue(cx, rval, &str) );
		size_t len = str.length();
		JL_ASSERT( len <= *amount, E_DATASIZE, E_MAX, E_NUM(*amount) );
		ASSERT( len <= *amount );
		*amount = len;
		str.copyTo(buffer);
	}
	return true;
	JL_BAD;
}


ALWAYS_INLINE NIStreamRead
StreamReadInterface( JSContext *cx, JS::HandleObject obj ) {

	NIStreamRead fct = StreamReadNativeInterface(cx, obj);
	if (likely( fct != NULL ))
		return fct;
	bool found;
	if ( JS_HasPropertyById(cx, obj, JLID(cx, read), &found) && found ) // JS_GetPropertyById(cx, obj, JLID(cx, Read), &res) != true || !JL_IsCallable(cx, res)
		return JSStreamRead;
	return NULL;
}



///////////////////////////////////////////////////////////////////////////////
// NativeInterface BufferGet

ALWAYS_INLINE bool
ReserveBufferGetInterface( JSContext *cx, JS::HandleObject obj ) {

	return ReserveNativeInterface(cx, obj, JLID(cx, _NI_BufferGet) );
}


ALWAYS_INLINE bool
SetBufferGetInterface( JSContext *cx, JS::HandleObject obj, NIBufferGet pFct ) {

	return SetNativeInterface( cx, obj, JLID(cx, _NI_BufferGet), pFct );
}


ALWAYS_INLINE NIBufferGet
BufferGetNativeInterface( JSContext *cx, JS::HandleObject obj ) {

	return GetNativeInterface<NIBufferGet>(cx, obj, JLID(cx, _NI_BufferGet));
}


INLINE bool
JSBufferGet( JSContext *cx, JS::HandleObject obj, jl::BufString *str ) {

	JS::RootedValue tmp(cx);
	return jl::call(cx, obj, JLID(cx, get), &tmp) && jl::getValue(cx, tmp, str);
}


ALWAYS_INLINE NIBufferGet
BufferGetInterface( JSContext *cx, JS::HandleObject obj ) {

	NIBufferGet fct = BufferGetNativeInterface(cx, obj);
	if (likely( fct != NULL ))
		return fct;
	bool found;
	if ( JS_HasPropertyById(cx, obj, JLID(cx, get), &found) && found ) // JS_GetPropertyById(cx, obj, JLID(cx, Get), &res) != true || !JL_IsCallable(cx, res)
		return JSBufferGet;
	return NULL;
}



///////////////////////////////////////////////////////////////////////////////
// NativeInterface Matrix44Get

ALWAYS_INLINE bool
ReserveMatrix44GetInterface( JSContext *cx, JS::HandleObject obj ) {

	return ReserveNativeInterface(cx, obj, JLID(cx, _NI_Matrix44Get) );
}


ALWAYS_INLINE bool
SetMatrix44GetInterface( JSContext *cx, JS::HandleObject obj, NIMatrix44Get pFct ) {

	return SetNativeInterface( cx, obj, JLID(cx, _NI_Matrix44Get), pFct );
}


ALWAYS_INLINE NIMatrix44Get
Matrix44GetNativeInterface( JSContext *cx, JS::HandleObject obj ) {

	return GetNativeInterface<NIMatrix44Get>(cx, obj, JLID(cx, _NI_Matrix44Get));
}


INLINE bool
JSMatrix44Get( JSContext *, JS::HandleObject , float ** ) {

	ASSERT( false ); // ???

	return false;
}


ALWAYS_INLINE NIMatrix44Get
Matrix44GetInterface( JSContext *cx, JS::HandleObject obj ) {

	NIMatrix44Get fct = Matrix44GetNativeInterface(cx, obj);
	if (likely( fct != NULL ))
		return fct;
	bool found;
	if ( JS_HasPropertyById(cx, obj, JLID(cx, getMatrix44), &found) && found ) // JS_GetPropertyById(cx, obj, JLID(cx, GetMatrix44), &res) != true || !JL_IsCallable(cx, res)
		return JSMatrix44Get;
	return NULL;
}



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
