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


#include "jlplatform.h"

#include "jlalloc.h"
#include "queue.h"

#ifdef XP_WIN
#define JS_SYS_TYPES_H_DEFINES_EXACT_SIZE_TYPES
#endif

#ifdef _MSC_VER
#pragma warning( push, 0 )
#endif // _MSC_VER

#include <jsapi.h>
#include <jscntxt.h>
#include <jsscope.h>
#include <jsvalue.h>
#include <jsxdrapi.h>
#include <jstypedarray.h>

#ifdef _MSC_VER
#pragma warning( pop )
#endif // _MSC_VER

#include <sys/stat.h> // used by JL_LoadScript()


extern int _unsafeMode;

class JLStr;


///////////////////////////////////////////////////////////////////////////////
// Native Interface function prototypes

typedef JSBool (*NIStreamRead)( JSContext *cx, JSObject *obj, char *buffer, size_t *amount );
typedef JSBool (*NIBufferGet)( JSContext *cx, JSObject *obj, JLStr *str );
typedef JSBool (*NIMatrix44Get)( JSContext *cx, JSObject *obj, float **pm );

inline NIBufferGet BufferGetNativeInterface( JSContext *cx, JSObject *obj );
inline NIBufferGet BufferGetInterface( JSContext *cx, JSObject *obj );
inline NIMatrix44Get Matrix44GetInterface( JSContext *cx, JSObject *obj );

ALWAYS_INLINE JSBool SetBufferGetInterface( JSContext *cx, JSObject *obj, NIBufferGet pFct );


///////////////////////////////////////////////////////////////////////////////
// helper macros to avoid a function call to the jsapi

ALWAYS_INLINE JSRuntime*
JL_GetRuntime( const JSContext *cx ) {

	return cx->runtime;
}

ALWAYS_INLINE void*
JL_GetRuntimePrivate( const JSRuntime *rt ) {

	return rt->data;
}

ALWAYS_INLINE void
JL_SetRuntimePrivate( JSRuntime *rt, void *data ) {

	rt->data = data;
}

ALWAYS_INLINE void
JL_updateMallocCounter( const JSContext *cx, size_t nbytes ) {

	return JL_GetRuntime(cx)->updateMallocCounter(nbytes); // JS_updateMallocCounter(cx, nbytes);
}

ALWAYS_INLINE JSObject *
JL_GetGlobalObject( const JSContext *cx ) {

    return cx->globalObject;
}

ALWAYS_INLINE JSBool
JL_IsExceptionPending( JSContext *cx ) {

	return cx->isExceptionPending();
}

ALWAYS_INLINE JSBool
JL_NewNumberValue( const JSContext *cx, jsdouble d, jsval *rval ) {

	JL_IGNORE(cx);
	d = JS_CANONICALIZE_NAN(d);
	js::Valueify(rval)->setNumber(d);
	return JS_TRUE;
}

ALWAYS_INLINE jsval
JL_GetNaNValue( const JSContext *cx ) {

	return js::Jsvalify(JL_GetRuntime(cx)->NaNValue);
}

ALWAYS_INLINE JSClass*
JL_GetClass( const JSObject *obj ) {

	return obj->getJSClass();
}

ALWAYS_INLINE const char *
JL_GetClassName( const JSObject *obj ) {

	return obj->getJSClass()->name;
}

ALWAYS_INLINE size_t
JL_GetStringLength( const JSString *jsstr ) {

	return jsstr->length();
}

ALWAYS_INLINE jsval
JL_GetEmptyStringValue( const JSContext *cx ) { // see JS_GetEmptyStringValue()

	return STRING_TO_JSVAL(JL_GetRuntime(cx)->emptyString);
}

ALWAYS_INLINE bool
JL_HasPrivate( const JSContext *cx, const JSObject *obj ) {

	JL_IGNORE(cx);
	return obj->getClass()->flags & JSCLASS_HAS_PRIVATE;
}

ALWAYS_INLINE void*
JL_GetPrivate( const JSContext *cx, const JSObject *obj ) {

	JL_IGNORE(cx);
	return obj->getPrivate();
}


ALWAYS_INLINE void
JL_SetPrivate( const JSContext *cx, JSObject *obj, void *data ) {

	JL_IGNORE(cx);
	obj->setPrivate(data);
}

ALWAYS_INLINE JSObject *
JL_GetPrototype(JSContext *cx, JSObject *obj) {

//	JL_IGNORE(cx);
//	JSObject *proto = obj->getProto();
//	return proto && proto->map ? proto : NULL; // Beware: ref to dead object (we may be called from obj's finalizer).
//	return proto && !proto->isNewborn() ? proto : NULL;
	return JS_GetPrototype(cx, obj);
}


// eg. JS_NewObject(cx, JL_GetStandardClassByKey(cx, JSProto_Date), NULL, NULL);
ALWAYS_INLINE JSClass*
JL_GetStandardClassByKey(JSContext *cx, JSProtoKey protoKey) {

	JSObject *ctor;
	return JS_GetClassObject(cx, JL_GetGlobalObject(cx), protoKey, &ctor) && ctor ? js::Jsvalify(FUN_CLASP(GET_FUNCTION_PRIVATE(cx, ctor))) : NULL;
}

ALWAYS_INLINE JSObject*
JL_GetStandardClassProtoByKey(JSContext *cx, JSProtoKey protoKey) {
	
	JSObject *proto;
	return js_GetClassPrototype(cx, JL_GetGlobalObject(cx), protoKey, &proto, NULL) ? proto : NULL;
}

ALWAYS_INLINE JSBool
JL_GetElement(JSContext *cx, JSObject *obj, jsuint index, jsval *vp) {

    return JS_GetPropertyById(cx, obj, INT_TO_JSID(index), vp);
}

ALWAYS_INLINE JSBool
JL_SetElement(JSContext *cx, JSObject *obj, jsuint index, jsval *vp) {

	return JS_SetPropertyById(cx, obj, INT_TO_JSID(index), vp);
}


ALWAYS_INLINE JSBool
JL_GetReservedSlot( JSContext *cx, JSObject *obj, uint32 slot, jsval *vp ) {

	// return JS_GetReservedSlot(cx, obj, slot, vp);
	JL_IGNORE(cx);

	#ifdef DEBUG
	JS_TypeOfValue(cx, *vp); // used for assertSameCompartment(cx, v)
	#endif // DEBUG

	if ( JS_IsNative(obj) && slot < obj->numSlots() )
		*vp = js::Jsvalify(obj->getSlot(slot));
	else
		js::Valueify(vp)->setUndefined();

	#ifdef DEBUG
	jsval tmp;
	JS_GetReservedSlot(cx, obj, slot, &tmp);
	JSBool same;
	if ( !JS_SameValue(cx, *vp, tmp, &same) )
		return JS_FALSE;
	ASSERT( same );
	#endif // DEBUG

	return JS_TRUE;
}

ALWAYS_INLINE JSBool
JL_SetReservedSlot(JSContext *cx, JSObject *obj, uintN slot, const jsval &v) {

//	return JS_SetReservedSlot(cx, obj, slot, v);

	#ifdef DEBUG
	JS_TypeOfValue(cx, v); // used for assertSameCompartment(cx, v)
	#endif // DEBUG

//	return JS_SetReservedSlot(cx, obj, index, v);
	if ( !JS_IsNative(obj) )
		return JS_TRUE;
	if ( slot >= obj->numSlots() )
		return JS_SetReservedSlot(cx, obj, slot, v);
	obj->setSlot(slot, js::Valueify(v));
	GC_POKE(cx, JS_NULL);
	return JS_TRUE;
}

ALWAYS_INLINE JSString *
JL_NewUCString(JSContext *cx, jschar *chars, size_t length) {

//#ifndef JS_HAS_JSLIBS_RegisterCustomAllocators
//
//	void *tmp = JS_malloc(cx, length);
//	if ( !tmp )
//		return NULL;
//	memcpy(tmp, bytes, length * sizeof(*jschar));
//	jl_free(bytes);
//	bytes = (char*)tmp;
//
//#endif // JS_HAS_JSLIBS_RegisterCustomAllocators

	return JS_NewUCString(cx, chars, length); // doc. https://developer.mozilla.org/en/SpiderMonkey/JSAPI_Reference/JS_NewString
}


///////////////////////////////////////////////////////////////////////////////
// Type check functions

ALWAYS_INLINE bool
JL_IsBooleanObject( JSContext * RESTRICT cx, jsval & RESTRICT value ) {

	return !JSVAL_IS_PRIMITIVE(value) && JL_GetClass(JSVAL_TO_OBJECT(value)) == JL_GetStandardClassByKey(cx, JSProto_Boolean);
}

ALWAYS_INLINE bool
JL_IsBoolean( JSContext * RESTRICT cx, jsval & RESTRICT value ) {

	return JSVAL_IS_BOOLEAN(value) || JL_IsBooleanObject(cx, value);
}

ALWAYS_INLINE bool
JL_IsInteger( JSContext * RESTRICT cx, jsval & RESTRICT value ) {

	JL_IGNORE(cx);
	return JSVAL_IS_INT(value) || (JSVAL_IS_DOUBLE(value) && !JL_DOUBLE_IS_INTEGER(JSVAL_TO_DOUBLE(value)));
}

ALWAYS_INLINE bool
JL_IsNumberObject( JSContext * RESTRICT cx, jsval & RESTRICT value ) {

	return !JSVAL_IS_PRIMITIVE(value) && JL_GetClass(JSVAL_TO_OBJECT(value)) == JL_GetStandardClassByKey(cx, JSProto_Number);
}

ALWAYS_INLINE bool
JL_IsNumber( JSContext * RESTRICT cx, jsval & RESTRICT value ) {

	return JSVAL_IS_NUMBER(value) || JL_IsNumberObject(cx, value);
}

ALWAYS_INLINE bool
JL_IsInteger53( JSContext * RESTRICT cx, jsval & RESTRICT value ) {

	JL_IGNORE(cx);
	return JSVAL_IS_INT(value) || (JSVAL_IS_DOUBLE(value) && JSVAL_TO_DOUBLE(value) < MAX_INT_TO_DOUBLE && JSVAL_TO_DOUBLE(value) > MIN_INT_TO_DOUBLE);
}

ALWAYS_INLINE bool
JL_IsNaN( const JSContext *cx, const jsval &val ) {

	return js::Valueify(val) == JL_GetRuntime(cx)->NaNValue;
}

ALWAYS_INLINE bool
JL_IsPInfinity( const JSContext *cx, const jsval &val ) {

	return js::Valueify(val) == JL_GetRuntime(cx)->positiveInfinityValue;
}

ALWAYS_INLINE bool
JL_IsNInfinity( const JSContext *cx, const jsval &val ) {

	return js::Valueify(val) == JL_GetRuntime(cx)->negativeInfinityValue;
}

ALWAYS_INLINE bool
JL_IsReal( const JSContext *cx, const jsval &val ) {

	JL_IGNORE(cx);
	if ( JSVAL_IS_INT(val) )
		return true;
	if ( JSVAL_IS_DOUBLE(val) ) {

		double tmp = JSVAL_TO_DOUBLE(val);
		return tmp >= MIN_INT_TO_DOUBLE && tmp <= MAX_INT_TO_DOUBLE;
	}
	return false;
}

ALWAYS_INLINE bool
JL_IsNegative( JSContext *cx, const jsval &val ) {

	return ( JSVAL_IS_INT(val) && JSVAL_TO_INT(val) < 0 )
	    || ( JSVAL_IS_DOUBLE(val) && DOUBLE_IS_NEG(JSVAL_TO_DOUBLE(val)) ) // js::Valueify(val).toDouble()
	    || JL_IsNInfinity(cx, val);
}

ALWAYS_INLINE bool
JL_IsClass( const jsval &val, const JSClass *jsClass ) {

	//return !JSVAL_IS_PRIMITIVE(val) && JL_GetClass(JSVAL_TO_OBJECT(val)) == jsClass;
	return !js::Valueify(val).isPrimitive() && jsClass != NULL && js::Valueify(val).toObject().getJSClass() == jsClass;
}

ALWAYS_INLINE bool
JL_IsObjectObject( JSContext *cx, const JSObject *obj ) {

//	JSObject *oproto;
//	return js_GetClassPrototype(cx, NULL, JSProto_Object, &oproto) && JL_GetClass(obj) == JL_GetClass(oproto) && obj->getProto() == oproto;
	ASSERT( obj != NULL );
	return obj->getProto() == JL_GetStandardClassProtoByKey(cx, JSProto_Object);
	//return obj->getProto() == JL_GetHostPrivate(cx)->objectProto;
}

ALWAYS_INLINE bool
JL_IsGeneratorObject( JSContext * RESTRICT cx, JSObject * RESTRICT obj ) {

	JSObject *proto;
	return JS_GetClassObject(cx, JL_GetGlobalObject(cx), JSProto_Generator, &proto) && JL_GetPrototype(cx, obj) == proto;
}

ALWAYS_INLINE bool
JL_IsGeneratorObject( JSContext * RESTRICT cx, jsval &val ) {

	return JSVAL_IS_OBJECT(val) && JL_IsGeneratorObject(cx, JSVAL_TO_OBJECT(val));
}

ALWAYS_INLINE bool
JL_IsGeneratorFunction( JSContext * RESTRICT cx, jsval &val ) {

	JL_IGNORE(cx);
	// copied from firefox-5.0b7.source\mozilla-beta\js\src
    JSObject *funobj;
	if (!js::IsFunctionObject(js::Valueify(val), &funobj)) {
        
        return true;
    }

    JSFunction *fun = GET_FUNCTION_PRIVATE(cx, funobj);

    bool result = false;
    if (fun->isInterpreted()) {
        JSScript *script = fun->u.i.script;
        JS_ASSERT(script->length != 0);
        result = script->code[0] == JSOP_GENERATOR;
    }

    return result;
}

ALWAYS_INLINE bool
JL_ObjectIsArray( JSContext * RESTRICT cx, JSObject * RESTRICT obj ) {

	return JS_IsArrayObject(cx, obj) == JS_TRUE; // Object::isArray() is not public
}

ALWAYS_INLINE bool
JL_ValueIsArray( JSContext *cx, const jsval &val ) {

	return !JSVAL_IS_PRIMITIVE(val) && JL_ObjectIsArray(cx, JSVAL_TO_OBJECT(val)); // Object::isArray() is not public
}


ALWAYS_INLINE bool
JL_IsVector( JSContext *cx, JSObject *obj ) {

	return JL_ObjectIsArray(cx, obj) || js_IsTypedArray(obj); // Object::isArray() is not public
}

ALWAYS_INLINE bool
JL_IsVector( JSContext *cx, const jsval &val ) {

	return !JSVAL_IS_PRIMITIVE(val) && ( JL_ObjectIsArray(cx, JSVAL_TO_OBJECT(val)) || js_IsTypedArray(JSVAL_TO_OBJECT(val)) ); // Object::isArray() is not public
}

ALWAYS_INLINE bool
JL_IsScript( const JSContext *cx, const JSObject *obj ) {

	JL_IGNORE(cx);
	return JL_GetClass(obj) == js::Jsvalify(&js_ScriptClass);
}

ALWAYS_INLINE bool
JL_ObjectIsFunction( const JSContext *cx, const JSObject *obj ) {

	JL_IGNORE(cx);
	return obj->isFunction();
}

ALWAYS_INLINE bool
JL_ValueIsFunction( const JSContext *cx, const jsval &val ) {

	JL_IGNORE(cx);
	return VALUE_IS_FUNCTION(cx, val);
}

ALWAYS_INLINE bool
JL_IsXML( const JSContext *cx, const JSObject *obj ) {

	JL_IGNORE(cx);
	#if JS_HAS_XML_SUPPORT
		extern JS_FRIEND_DATA(js::Class) js_XMLClass;
		return JL_GetClass(obj) == js::Jsvalify(&js_XMLClass);
	#else
		return false;
	#endif // JS_HAS_XML_SUPPORT
}


ALWAYS_INLINE bool
JL_IsStringObject( JSContext *cx, const JSObject *obj ) {

	return JL_GetClass(obj) == JL_GetStandardClassByKey(cx, JSProto_String);
}


ALWAYS_INLINE bool
JL_IsDataObject( JSContext * RESTRICT cx, JSObject * RESTRICT obj ) {

	return BufferGetInterface(cx, obj) != NULL || JL_ObjectIsArray(cx, obj) || (js_IsTypedArray(obj) /*&& js::TypedArray::fromJSObject(obj)->valid()*/) /*|| js_IsArrayBuffer(obj)*/ || JL_IsStringObject(cx, obj);
}


ALWAYS_INLINE bool
JL_IsData( JSContext *cx, const jsval &val ) {

	return JSVAL_IS_STRING(val) || ( !JSVAL_IS_PRIMITIVE(val) && NOIL(JL_IsDataObject)(cx, JSVAL_TO_OBJECT(val)) );
}



///////////////////////////////////////////////////////////////////////////////
// Safe Mode tools

#define JL_IS_SAFE (unlikely(!_unsafeMode))
#define JL_IS_UNSAFE (likely(_unsafeMode))

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

#define JL_BAD bad:return(JS_FALSE)

#define JL_ARGC (argc)

// returns the ARGument Vector
#define JL_ARGV (JS_ARGV(cx,vp))

// returns the ARGument n
#define JL_ARG( n ) (ASSERT(n <= argc), JL_ARGV[(n)-1])

// returns the ARGument n or undefined if it does not exist
#define JL_SARG( n ) (JL_ARGC >= (n) ? JL_ARG(n) : JSVAL_VOID)

// returns true if the ARGument n IS DEFined
#define JL_ARG_ISDEF( n ) (JL_ARGC >= (n) && !JSVAL_IS_VOID(JL_ARG(n)))

// is the current obj (this)
#define JL_OBJ (obj)

// is the current obj (this) as a jsval
#define JL_OBJVAL (argc=argc, JS_THIS(cx, vp))

// the return value
#define JL_RVAL (&JS_RVAL(cx, vp))

// define obj variable for native functions
#define JL_DEFINE_FUNCTION_OBJ \
	JSObject *obj; \
	{ \
		obj = JS_THIS_OBJECT(cx, vp); \
		ASSERT( obj ); \
	}

#define JL_DEFINE_CALL_FUNCTION_OBJ \
	JSObject *obj; \
	{ \
		obj = JSVAL_TO_OBJECT(JS_CALLEE(cx, vp)); \
		ASSERT( obj ); \
	}

// initialise 'this' object (obj) for constructors
// see. JS_NewObjectForConstructor(cx, vp) if JL_THIS_CLASS or JL_THIS_PROTOTYPE cannot be used
#define JL_DEFINE_CONSTRUCTOR_OBJ \
	JSObject *obj; \
	{ \
		if ( !((JL_THIS_CLASS->flags & JSCLASS_CONSTRUCT_PROTOTYPE) && JS_IsConstructing_PossiblyWithGivenThisObject(cx, vp, &obj) && obj) ) { \
			obj = JS_NewObjectWithGivenProto(cx, JL_THIS_CLASS, JL_THIS_PROTOTYPE, NULL); \
			if ( obj == NULL ) { \
				return JS_FALSE; \
			} \
		} \
		JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(obj)); \
		if ( js::Valueify(JL_THIS_CLASS)->ext.equality ) { \
			obj->flags |= JSObject::HAS_EQUALITY; \
		} \
	}



///////////////////////////////////////////////////////////////////////////////
// IDs cache

// defined here because required by jlhostprivate.h
#define JLID_SPEC(name) JLID_##name
enum {
	JLID_SPEC( stdin ),
	JLID_SPEC( stdout ),
	JLID_SPEC( stderr ),
	JLID_SPEC( global ),
	JLID_SPEC( arguments ),
	JLID_SPEC( unsafeMode ),
	JLID_SPEC( _revision ),
	JLID_SPEC( _host ),
	JLID_SPEC( scripthostpath ),
	JLID_SPEC( scripthostname ),
	JLID_SPEC( isfirstinstance ),
	JLID_SPEC( bootstrapScript ),
	JLID_SPEC( _NI_BufferGet ),
	JLID_SPEC( _NI_StreamRead ),
	JLID_SPEC( _NI_Matrix44Get ),
	JLID_SPEC( Get ),
	JLID_SPEC( Read ),
	JLID_SPEC( GetMatrix44 ),
	JLID_SPEC( name ),
	JLID_SPEC( id ),
	JLID_SPEC( push ),
	JLID_SPEC( pop ),
	JLID_SPEC( _serialize ),
	JLID_SPEC( _unserialize ),
	JLID_SPEC( toXMLString ),
	JLID_SPEC( _private1 ),
	JLID_SPEC( _private2 ),
	JLID_SPEC( _private3 ),
	LAST_JSID // see HostPrivate::ids[]
};
#undef JLID_SPEC



///////////////////////////////////////////////////////////////////////////////
// Context private

struct JLContextPrivate {
};

ALWAYS_INLINE JLContextPrivate*
JL_GetContextPrivate( const JSContext *cx ) {

	ASSERT( JS_GetContextPrivate((JSContext*)cx) == cx->data );
	return static_cast<JLContextPrivate*>(cx->data);
}

ALWAYS_INLINE void
JL_SetContextPrivate( const JSContext *cx, JLContextPrivate *ContextPrivate ) {

	ASSERT( JS_GetContextPrivate((JSContext*)cx) == cx->data );
	JL_GetRuntime(cx)->data = static_cast<void*>(ContextPrivate);
}



///////////////////////////////////////////////////////////////////////////////
// Host private

// Using a separate file allow a better versioning of the HostPrivate structure (see JL_HOST_PRIVATE_VERSION).
#include "jlhostprivate.h"

ALWAYS_INLINE HostPrivate*
JL_GetHostPrivate( const JSContext *cx ) {

	return static_cast<HostPrivate*>(JL_GetRuntimePrivate(JL_GetRuntime(cx)));
}

ALWAYS_INLINE void
JL_SetHostPrivate( const JSContext *cx, HostPrivate *hostPrivate ) {

	JL_SetRuntimePrivate(JL_GetRuntime(cx), static_cast<void*>(hostPrivate));
}



///////////////////////////////////////////////////////////////////////////////
// Module private

ALWAYS_INLINE uint8_t
JL_ModulePrivateHash( const uint32_t moduleId ) {

	ASSERT( moduleId != 0 );
//	return ((uint8_t*)&moduleId)[0] ^ ((uint8_t*)&moduleId)[1] ^ ((uint8_t*)&moduleId)[2] ^ ((uint8_t*)&moduleId)[3] << 1;
	uint32_t a = moduleId ^ moduleId >> 16;
	return (a ^ a >> 8) & 0xFF;
}

ALWAYS_INLINE bool
JL_SetModulePrivate( const JSContext *cx, const uint32_t moduleId, void *modulePrivate ) {

	ASSERT( modulePrivate );
	ASSERT( moduleId != 0 );
	uint8_t id = JL_ModulePrivateHash(moduleId);
	HostPrivate::ModulePrivate *mpv = JL_GetHostPrivate(cx)->modulePrivate;
	while ( mpv[id].moduleId != 0 ) { // assumes that modulePrivate struct is init to 0

		if ( mpv[id].moduleId == moduleId )
			return false; // module private already exist or moduleId not unique
		++id; // uses unsigned char overflow
	}
	mpv[id].moduleId = moduleId;
	mpv[id].privateData = modulePrivate;
	return true;
}

ALWAYS_INLINE void*
JL_GetModulePrivateOrNULL( const JSContext *cx, uint32_t moduleId ) {

	ASSERT( moduleId != 0 );
	uint8_t id = JL_ModulePrivateHash(moduleId);
	HostPrivate::ModulePrivate *mpv = JL_GetHostPrivate(cx)->modulePrivate;
	uint8_t id0 = id;

	while ( mpv[id].moduleId != moduleId ) {

		++id; // uses unsigned char overflow
		if ( id == id0 )
			return NULL;
	}
	return mpv[id].privateData;
}


ALWAYS_INLINE void*
JL_GetModulePrivate( const JSContext *cx, uint32_t moduleId ) {

	ASSERT( moduleId != 0 );
	uint8_t id = JL_ModulePrivateHash(moduleId);
	HostPrivate::ModulePrivate *mpv = JL_GetHostPrivate(cx)->modulePrivate;

#ifdef DEBUG
	uint8_t maxid = id;
#endif // DEBUG

	while ( mpv[id].moduleId != moduleId ) {

		++id; // uses unsigned char overflow

#ifdef DEBUG
		ASSERT( id != maxid );
#endif // DEBUG

	}
	return mpv[id].privateData;
}
// example of use: static uint32_t moduleId = 'dbug'; SetModulePrivate(cx, moduleId, mpv);



///////////////////////////////////////////////////////////////////////////////
// cached class and proto

ALWAYS_INLINE NOALIAS uint32_t
JL_ClassNameToClassProtoCacheSlot( const char *n ) {

	ASSERT( n != NULL );
	ASSERT( strlen(n) <= 24 );

	register uint32_t h = 0;
	if ( n[ 0] ) { h ^= n[ 0]<<0;
	if ( n[ 1] ) { h ^= n[ 1]<<4;
	if ( n[ 2] ) { h ^= n[ 2]<<1;
	if ( n[ 3] ) { h ^= n[ 3]<<5;
	if ( n[ 4] ) { h ^= n[ 4]<<2;
	if ( n[ 5] ) { h ^= n[ 5]<<6;
	if ( n[ 6] ) { h ^= n[ 6]<<3;
	if ( n[ 7] ) { h ^= n[ 7]<<7;
	if ( n[ 8] ) { h ^= n[ 8]<<4;
	if ( n[ 9] ) { h ^= n[ 9]<<8;
	if ( n[10] ) { h ^= n[10]<<5;
	if ( n[11] ) { h ^= n[11]<<0;
	if ( n[12] ) { h ^= n[12]<<6;
	if ( n[13] ) { h ^= n[13]<<1;
	if ( n[14] ) { h ^= n[14]<<7;
	if ( n[15] ) { h ^= n[15]<<2;
	if ( n[16] ) { h ^= n[16]<<8;
	if ( n[17] ) { h ^= n[17]<<3;
	if ( n[18] ) { h ^= n[18]<<0;
	if ( n[19] ) { h ^= n[19]<<4;
	if ( n[20] ) { h ^= n[20]<<1;
	if ( n[21] ) { h ^= n[21]<<5;
	if ( n[22] ) { h ^= n[22]<<2;
	if ( n[23] ) { h ^= n[23]<<6;
	}}}}}}}}}}}}}}}}}}}}}}}}
	return ((h >> 7) ^ h) & ((1<<MAX_CLASS_PROTO_CACHE_BIT) - 1);
}


INLINE bool FASTCALL
JL_CacheClassProto( HostPrivate * RESTRICT hpv, const char * RESTRICT className, JSClass * RESTRICT clasp, JSObject * RESTRICT proto ) {

	size_t slotIndex = JL_ClassNameToClassProtoCacheSlot(className);
	size_t first = slotIndex;
	ASSERT( slotIndex < COUNTOF(hpv->classProtoCache) );

	for (;;) {

		ClassProtoCache *slot = &hpv->classProtoCache[slotIndex];

		if ( slot->clasp == NULL ) {

			slot->clasp = clasp;
			slot->proto = proto;
			return true;
		}

		if ( slot->clasp == clasp ) // already cached
			return false;

		slotIndex = (slotIndex + 1) % COUNTOF(hpv->classProtoCache);

		if ( slotIndex == first ) // no more free slot
			return false;
	}
}

ALWAYS_INLINE ClassProtoCache*
JL_GetCachedClassProto( HostPrivate *hpv, const char *className ) {

	size_t slotIndex = JL_ClassNameToClassProtoCacheSlot(className);
	size_t first = slotIndex;
	ASSERT( slotIndex < COUNTOF(hpv->classProtoCache) );

	for (;;) {

		ClassProtoCache *slot = &hpv->classProtoCache[slotIndex];

		if ( slot->clasp == NULL || strcmp(slot->clasp->name, className) == 0 ) // since classes cannot be removed from jslibs, NULL mean "not found"
			return slot;

		slotIndex = (slotIndex + 1) % COUNTOF(hpv->classProtoCache);

		if ( slotIndex == first ) // not found
			return NULL;
	}
}


ALWAYS_INLINE JSObject*
JL_NewJslibsObject( JSContext *cx, const char *className ) {

	ClassProtoCache *cpc = JL_GetCachedClassProto(JL_GetHostPrivate(cx), className);
	ASSERT( cpc );
	return JS_NewObjectWithGivenProto(cx, cpc->clasp, cpc->proto, NULL);
}

ALWAYS_INLINE JSObject*
JL_NewObj( JSContext *cx ) {

	HostPrivate *pv = JL_GetHostPrivate(cx);
	return JS_NewObject(cx, pv->objectClass, pv->objectProto, JL_GetGlobalObject(cx));
}

ALWAYS_INLINE JSObject*
JL_NewProtolessObj( JSContext *cx ) {

	return JS_NewObjectWithGivenProto(cx, NULL, NULL, JL_GetGlobalObject(cx));
}


///////////////////////////////////////////////////////////////////////////////
// IDs cache management


ALWAYS_INLINE jsid
JL_NullJsid() { // is (double)0

	jsid tmp = {0}; // memset(&tmp, 0, sizeof(tmp));
	return tmp;
}

	// slow part of JL_GetPrivateJsid()
	INLINE NEVER_INLINE jsid FASTCALL
	JL__GetPrivateJsid_slow( JSContext * RESTRICT cx, int index, const jschar * RESTRICT name ) {

		jsid id;
		JSString *jsstr = JS_InternUCString(cx, name);
		if (unlikely( jsstr == NULL ))
			return JL_NullJsid();
		if (unlikely( JS_ValueToId(cx, STRING_TO_JSVAL(jsstr), &id) != JS_TRUE ))
			return JL_NullJsid();
		JL_GetHostPrivate(cx)->ids[index] = id;
		return id;
	}

ALWAYS_INLINE jsid
JL_GetPrivateJsid( JSContext * RESTRICT cx, int index, const jschar * RESTRICT name ) {

	ASSERT( JL_GetHostPrivate(cx) != NULL );
	jsid id = JL_GetHostPrivate(cx)->ids[index];
	if (likely( id != JL_NullJsid() ))
		return id;
	return JL__GetPrivateJsid_slow(cx, index, name);
}


#ifdef DEBUG
#define JLID_NAME(cx, name) (JL_IGNORE(cx), JL_IGNORE(JLID_##name), L(#name))
#else
#define JLID_NAME(cx, name) (#name)
#endif // DEBUG


#define JLID(cx, name) JL_GetPrivateJsid(cx, JLID_##name, L(#name))
// example of use: jsid cfg = JLID(cx, _host); char *name = JLID_NAME(_host);



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
#define E_STR( str )                  E_STR_1, str
#define E_NAME( str )                 E_NAME_1, str
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
		JL_GetHostPrivate(cx)->report(cx, false, ##__VA_ARGS__, JL__REPORT_END_ARG); \
		goto bad; \
	JL_MACRO_END


#define JL_WARN( ... ) \
	JL_MACRO_BEGIN \
		if ( JL_IS_SAFE && !JL_GetHostPrivate(cx)->report(cx, true, ##__VA_ARGS__, JL__REPORT_END_ARG) ) \
			goto bad; \
	JL_MACRO_END


#define JL_CHK( CONDITION ) \
	JL_MACRO_BEGIN \
		if (unlikely( !(CONDITION) )) { \
			goto bad; \
		} \
		ASSUME(CONDITION); \
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
		ASSUME(CONDITION); \
	JL_MACRO_END


#define JL_ASSERT( CONDITION, ... ) \
	JL_MACRO_BEGIN \
		if ( JL_IS_SAFE ) { \
			if (unlikely( !(CONDITION) )) { \
				JL_ERR( __VA_ARGS__ ); \
			} \
		} /* else if ( IS_DEBUG ) { ASSERT( (CONDITION) ); } */ \
		ASSUME(CONDITION); \
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
			if (unlikely( (PTR) == NULL )) { \
				ASSERT( !JS_IsExceptionPending(cx) ); \
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
	JL_ASSERT( NOIL(JL_IsBoolean)(cx, val), E_VALUE, E_STR(context), E_TYPE, E_TY_BOOLEAN )

#define JL_ASSERT_IS_INTEGER(val, context) \
	JL_ASSERT( NOIL(JL_IsInteger)(cx, val), E_VALUE, E_STR(context), E_TYPE, E_TY_INTEGER )

#define JL_ASSERT_IS_INTEGER_NUMBER(val, context) \
	JL_ASSERT( NOIL(JL_IsInteger53)(cx, val), E_VALUE, E_STR(context), E_TYPE, E_TY_INTEGERDOUBLE )

#define JL_ASSERT_IS_NUMBER(val, context) \
	JL_ASSERT( NOIL(JL_IsNumber)(cx, val), E_VALUE, E_STR(context), E_TYPE, E_TY_NUMBER )

#define JL_ASSERT_IS_FUNCTION(val, context) \
	JL_ASSERT( NOIL(JL_ValueIsFunction)(cx, val), E_VALUE, E_STR(context), E_TYPE, E_TY_FUNC )

#define JL_ASSERT_IS_ARRAY(val, context) \
	JL_ASSERT( NOIL(JL_ValueIsArray)(cx, val), E_VALUE, E_STR(context), E_TYPE, E_TY_ARRAY )

#define JL_ASSERT_IS_OBJECT(val, context) \
	JL_ASSERT( !JSVAL_IS_PRIMITIVE(val), E_VALUE, E_STR(context), E_TYPE, E_TY_OBJECT )

#define JL_ASSERT_IS_STRING(val, context) \
	JL_ASSERT( NOIL(JL_IsData)(cx, val), E_VALUE, E_STR(context), E_TYPE, E_TY_STRINGDATA )

//

#define JL_ASSERT_RANGE(val, valMin, valMax, context) \
	JL_ASSERT( JL_INRANGE((int)val, (int)valMin, (int)valMax), E_VALUE, E_STR(context), E_RANGE, E_INTERVAL_NUM(valMin, valMax) )


// arg

#define JL_ASSERT_ARGC_MIN(minCount) \
	JL_ASSERT( JL_ARGC >= (minCount), E_ARGC, E_MIN, E_NUM(minCount) )

#define JL_ASSERT_ARGC_MAX(maxCount) \
	JL_ASSERT( JL_ARGC <= (maxCount), E_ARGC, E_MAX, E_NUM(maxCount) )

#define JL_ASSERT_ARGC_RANGE(minCount, maxCount) \
	JL_ASSERT( JL_INRANGE((int)JL_ARGC, (int)minCount, (int)maxCount), E_ARGC, E_RANGE, E_INTERVAL_NUM(uintN(minCount), uintN(maxCount)) )

#define JL_ASSERT_ARGC(count) \
	JL_ASSERT( JL_ARGC == (count), E_ARGC, E_EQUALS, E_NUM(count) )

#define JL_ASSERT_ARG_IS_BOOLEAN(argNum) \
	JL_ASSERT( NOIL(JL_IsBoolean)(cx, JL_ARG(argNum)), E_ARG, E_NUM(argNum), E_TYPE, E_NAME("boolean") )

#define JL_ASSERT_ARG_IS_INTEGER(argNum) \
	JL_ASSERT( NOIL(JL_IsInteger)(cx, JL_ARG(argNum)), E_ARG, E_NUM(argNum), E_TYPE, E_NAME("integer") )

#define JL_ASSERT_ARG_IS_INTEGER_NUMBER(argNum) \
	JL_ASSERT( NOIL(JL_IsInteger53)(cx, JL_ARG(argNum)), E_ARG, E_NUM(argNum), E_TYPE, E_NAME("integer < 2^53") )

#define JL_ASSERT_ARG_IS_NUMBER(argNum) \
	JL_ASSERT( NOIL(JL_IsNumber)(cx, JL_ARG(argNum)), E_ARG, E_NUM(argNum), E_TYPE, E_NAME("number") )

#define JL_ASSERT_ARG_IS_STRING(argNum) \
	JL_ASSERT( NOIL(JL_IsData)(cx, JL_ARG(argNum)), E_ARG, E_NUM(argNum), E_TYPE, E_NAME("string || data") )

#define JL_ASSERT_ARG_IS_OBJECT(argNum) \
	JL_ASSERT( !JSVAL_IS_PRIMITIVE(JL_ARG(argNum)), E_ARG, E_NUM(argNum), E_TYPE, E_NAME("object") )

#define JL_ASSERT_ARG_IS_OBJECT_OR_NULL(argNum) \
	JL_ASSERT( JSVAL_IS_OBJECT(JL_ARG(argNum)), E_ARG, E_NUM(argNum), E_TYPE, E_NAME("object"), E_OR, E_NAME("null") )

#define JL_ASSERT_ARG_IS_ARRAY(argNum) \
	JL_ASSERT( NOIL(JL_ValueIsArray)(cx, JL_ARG(argNum)), E_ARG, E_NUM(argNum), E_TYPE, E_NAME("array") )

#define JL_ASSERT_ARG_IS_VECTOR(argNum) \
	JL_ASSERT( NOIL(JL_IsVector)(cx, JL_ARG(argNum)), E_ARG, E_NUM(argNum), E_TYPE, E_NAME("vector") )

#define JL_ASSERT_ARG_IS_FUNCTION(argNum) \
	JL_ASSERT( NOIL(JL_ValueIsFunction)(cx, JL_ARG(argNum)), E_ARG, E_NUM(argNum), E_TYPE, E_NAME("function") )

// fallback
#define JL_ASSERT_ARG_TYPE(condition, argNum, typeStr) \
	JL_ASSERT( condition, E_ARG, E_NUM(argNum), E_TYPE, E_NAME(typeStr) )

#define JL_ASSERT_ARG_VAL_RANGE(val, valMin, valMax, argNum) \
	JL_ASSERT( JL_INRANGE((int)val, (int)valMin, (int)valMax), E_ARG, E_NUM(argNum), E_RANGE, E_INTERVAL_NUM(valMin, valMax) )



// obj

#define JL_ASSERT_CONSTRUCTING() \
	JL_ASSERT( (JL_ARGC, JS_IsConstructing(cx, vp)), E_THISOBJ, E_CONSTRUCT )


#define JL_ASSERT_INSTANCE( jsObject, jsClass ) \
	JL_ASSERT( JL_GetClass(JL_GetPrototype(cx, jsObject)) == jsClass, E_OBJ, E_INSTANCE, E_NAME((jsClass)->name) ); \

#define JL_ASSERT_THIS_INSTANCE() \
	JL_ASSERT( JL_GetClass(JL_GetPrototype(cx, JL_OBJ)) == JL_THIS_CLASS, E_THISOBJ, E_INSTANCE, E_NAME(JL_THIS_CLASS_NAME) ); \


#define JL_ASSERT_INHERITANCE( jsObject, jsClass ) \
	JL_ASSERT( NOIL(JL_InheritFrom)(cx, JL_GetPrototype(cx, jsObject), (jsClass)), E_OBJ, E_INHERIT, E_NAME((jsClass)->name) )

#define JL_ASSERT_THIS_INHERITANCE() \
	JL_ASSERT( NOIL(JL_InheritFrom)(cx, JL_GetPrototype(cx, JL_OBJ), JL_THIS_CLASS), E_THISOBJ, E_INHERIT, E_NAME(JL_THIS_CLASS_NAME) )


#define JL_ASSERT_OBJECT_STATE( condition, name ) \
	JL_ASSERT( condition, E_OBJ, E_NAME(name), E_STATE )

#define JL_ASSERT_THIS_OBJECT_STATE( condition ) \
	JL_ASSERT( condition, E_THISOBJ, E_NAME(JL_THIS_CLASS_NAME), E_STATE )


///////////////////////////////////////////////////////////////////////////////
// JLStr class

class JLStr {

	enum {
		OWN = 1,
		NT = 2
	};

	struct Inner {
		size_t len;
		jschar *jsstr;
		char *str;
		uint8_t jsstrFlags;
		uint8_t strFlags;
		uint32_t count;
	} *_inner;

	static jl::PreservAllocNone_threadsafe<JLStr::Inner> mem; // require #include <jlhelper.cpp>

	void CreateOwnJsStrZ() {

		ASSERT( IsSet() );
		ASSERT_IF( _inner->jsstr, !JL_HasFlags(_inner->jsstrFlags, OWN | NT) );

		jschar *tmp;
		size_t length = Length();
		if ( _inner->jsstr ) {

			if ( JL_HasFlags(_inner->jsstrFlags, OWN) ) {

				_inner->jsstr = static_cast<jschar*>(jl_realloc(_inner->jsstr, sizeof(jschar) * (length +1)));
				ASSERT( _inner->jsstr );
			} else {

				tmp = static_cast<jschar*>(jl_malloc(sizeof(jschar) * (length +1)));
				ASSERT( tmp );
				memcpy(tmp, _inner->jsstr, length * 2);
				_inner->jsstr = tmp;
			}
			_inner->jsstr[length] = 0;
		} else {

			ASSERT( _inner->str );
			if ( JL_HasFlags(_inner->strFlags, OWN) ) {

				_inner->jsstr = (jschar*)jl_realloc(_inner->str, (length+1) * 2);
				ASSERT( _inner->jsstr );
				_inner->str = NULL;
				_inner->jsstr[length] = 0;

				const char *src = (char*)_inner->jsstr + length;
				tmp = _inner->jsstr + length;
				for ( size_t i = length; i > 0; --i )
					*--tmp = (unsigned char)*--src;
			} else {

				tmp = static_cast<jschar*>(jl_malloc(sizeof(jschar) * (length +1)));
				ASSERT( tmp );
				tmp[length] = 0;
				_inner->jsstr = tmp;
				const char *src = _inner->str;
				for ( size_t i = length; i > 0; --i )
					*(tmp++) = (unsigned char)*(src++);
			}
		}
		_inner->jsstrFlags = OWN | NT;
	}

	void CreateOwnStrZ() {

		ASSERT( IsSet() );
		ASSERT_IF( _inner->str, !JL_HasFlags(_inner->strFlags, OWN | NT) );

		char *tmp;
		size_t length = Length();
		if ( _inner->str ) {

			if ( JL_HasFlags(_inner->strFlags, OWN) ) {

				_inner->str = static_cast<char*>(jl_realloc(_inner->str, sizeof(char) * (length +1)));
				ASSERT( _inner->str );
			} else {

				//JL_Alloc(tmp, length + 1);
				tmp = static_cast<char*>(jl_malloc(sizeof(char) * (length +1)));
				ASSERT( tmp );
				memcpy(tmp, _inner->str, length);
				_inner->str = tmp;
			}
			_inner->str[length] = 0;
		} else {

			ASSERT( _inner->jsstr );
			if ( JL_HasFlags(_inner->jsstrFlags, OWN) ) {

				const jschar *src = _inner->jsstr + length;
				tmp = (char*)_inner->jsstr + length;
				for ( size_t i = length; i > 0; --i )
					*--tmp = (char)(*--src & 0xff);

				_inner->str = (char*)jl_realloc(_inner->jsstr, length + 1);
				ASSERT( _inner->str );
				_inner->jsstr = NULL;
				_inner->str[length] = 0;
			} else {

				tmp = static_cast<char*>(jl_malloc(sizeof(char) * (length +1)));
				ASSERT( tmp );
				tmp[length] = 0;
				_inner->str = tmp;
				const jschar *src = _inner->jsstr;
				for ( size_t i = length; i > 0; --i )
					*(tmp++) = (char)(*(src++) & 0xff);
			}
		}
		_inner->strFlags = OWN | NT;
	}

	ALWAYS_INLINE void NewInner( const jschar * RESTRICT jsstr, const char * RESTRICT str, bool nullTerminated, bool hasOwnership, size_t length = SIZE_MAX ) {

		ASSERT_IF( length == SIZE_MAX, nullTerminated );
		ASSERT( jsstr == NULL || str == NULL );

		_inner = mem.Alloc();
		ASSERT( _inner );

		_inner->count = 1;
		_inner->len = length;
		_inner->jsstr = const_cast<jschar*>(jsstr);
		_inner->str = const_cast<char*>(str);
		_inner->jsstrFlags = jsstr ? (nullTerminated ? NT : 0) | (hasOwnership ? OWN : 0) : 0;
		_inner->strFlags = str ? (nullTerminated ? NT : 0) | (hasOwnership ? OWN : 0) : 0;

		ASSERT( IsSet() );
		ASSERT_IF( length != SIZE_MAX && hasOwnership && jsstr && !nullTerminated, jl_msize((void*)jsstr) >= length );
		ASSERT_IF( length != SIZE_MAX && hasOwnership && str && !nullTerminated, jl_msize((void*)str) >= length );

		ASSERT_IF( length != SIZE_MAX && hasOwnership && jsstr && nullTerminated, jl_msize((void*)jsstr) >= length + 2 );
		ASSERT_IF( length != SIZE_MAX && hasOwnership && str && nullTerminated, jl_msize((void*)str) >= length + 1 );

		ASSERT_IF( length != SIZE_MAX && nullTerminated && jsstr, jsstr[length] == 0 );
		ASSERT_IF( length != SIZE_MAX && nullTerminated && str, str[length] == 0 );
	}

public:

	ALWAYS_INLINE ~JLStr() {

		if ( !_inner || --_inner->count )
			return;
		if ( JL_HasFlags(_inner->jsstrFlags, OWN) )
			jl_free(_inner->jsstr);
		if ( JL_HasFlags(_inner->strFlags, OWN) )
			jl_free(_inner->str);
		mem.Free(_inner);
	}

	ALWAYS_INLINE JLStr() : _inner(NULL) {

		ASSERT( !JS_CStringsAreUTF8() );
	}

	ALWAYS_INLINE JLStr(const JLStr &jlstr) : _inner(jlstr._inner) {

		ASSERT( _inner );
		++_inner->count;
	}

	ALWAYS_INLINE void operator=(const JLStr &jlstr) {

		ASSERT( !_inner );
		_inner = jlstr._inner;
		++_inner->count;
	}

	ALWAYS_INLINE JLStr(JSContext * RESTRICT cx, JSString * RESTRICT jsstr) {

		size_t length;
		const jschar *str = JS_GetStringCharsAndLength(cx, jsstr, &length); // doc. not null-terminated. // see also JS_GetStringCharsZ
		NewInner(str, NULL, false, false, length);
	}


	ALWAYS_INLINE JLStr(const jschar *str, size_t length, bool nullTerminated) {

		NewInner(str, NULL, nullTerminated, false, length);
	}

	ALWAYS_INLINE JLStr(jschar *str, size_t length, bool nullTerminated) { // give ownership of str to JLStr

		NewInner(str, NULL, nullTerminated, true, length);
	}


	ALWAYS_INLINE JLStr(const char *str, bool nullTerminated) {

		NewInner(NULL, str, nullTerminated, false);
	}

	ALWAYS_INLINE JLStr(const char *str, size_t length, bool nullTerminated) {

		NewInner(NULL, str, nullTerminated, false, length);
	}

	ALWAYS_INLINE JLStr(char *str, bool nullTerminated) { // give ownership of str to JLStr

		NewInner(NULL, str, nullTerminated, true);
	}

	ALWAYS_INLINE JLStr(char *str, size_t length, bool nullTerminated) { // give ownership of str to JLStr

		NewInner(NULL, str, nullTerminated, true, length);
	}


	ALWAYS_INLINE JLStr(const uint8_t *str, bool nullTerminated) {

		NewInner(NULL, (char*)str, nullTerminated, false);
	}

	ALWAYS_INLINE JLStr(const uint8_t *str, size_t length, bool nullTerminated) {

		NewInner(NULL, (char*)str, nullTerminated, false, length);
	}

	ALWAYS_INLINE JLStr(uint8_t *str, bool nullTerminated) { // give ownership of str to JLStr

		NewInner(NULL, (char*)str, nullTerminated, true);
	}

	ALWAYS_INLINE JLStr(uint8_t *str, size_t length, bool nullTerminated) { // give ownership of str to JLStr

		NewInner(NULL, (char*)str, nullTerminated, true, length);
	}


	ALWAYS_INLINE JLStr(void *str, size_t length, bool nullTerminated, bool hasOwnership) { // give ownership of str to JLStr

		NewInner(NULL, (char*)str, nullTerminated, hasOwnership, length);
	}


	ALWAYS_INLINE bool IsSet() const {

		return _inner && (_inner->jsstr || _inner->str);
	}

	ALWAYS_INLINE bool HasJsStr() const {

		ASSERT( _inner );
		return _inner->jsstr != NULL;
	}

	ALWAYS_INLINE size_t Length() {

		ASSERT( IsSet() );
		if ( _inner->len != SIZE_MAX ) // known length
			return _inner->len;
		if ( _inner->str )
			return _inner->len = strlen(_inner->str);
		if ( _inner->jsstr )
			return _inner->len = wcslen((wchar_t *)_inner->jsstr);
		ASSERT( _inner->len != SIZE_MAX );
		return 0;
	}

	ALWAYS_INLINE size_t LengthOrZero() {

		return IsSet() ? Length() : 0;
	}

	ALWAYS_INLINE const jschar *GetConstJsStr() {

		ASSERT( IsSet() );
		if ( !_inner->jsstr )
			CreateOwnJsStrZ();
		return _inner->jsstr;
	}

	ALWAYS_INLINE const jschar *GetJsStrConstOrNull() {

		return IsSet() ? GetConstJsStr() : NULL;
	}

	ALWAYS_INLINE jschar *GetJsStrZOwnership() {

		ASSERT( IsSet() );
		if ( !_inner->jsstr || !JL_HasFlags(_inner->jsstrFlags, OWN | NT) )
			CreateOwnJsStrZ();
		jschar *tmp = _inner->jsstr;
		_inner->jsstr = NULL;
		return tmp;
	}

	ALWAYS_INLINE JSString *GetJSString(JSContext *cx) {

		ASSERT( IsSet() );
		if ( Length() == 0 )
			return JSVAL_TO_STRING( JL_GetEmptyStringValue(cx) );
		return JS_NewUCString(cx, GetJsStrZOwnership(), Length()); // (TBD) manage allocator issue in GetJsStrZOwnership() ?
	}

	ALWAYS_INLINE const char *GetConstStr() {

		if ( !_inner->str )
			CreateOwnStrZ();
		return _inner->str;
	}

	ALWAYS_INLINE const char *GetStrConstOrNull() {

		return IsSet() ? GetConstStr() : NULL;
	}

	ALWAYS_INLINE char *GetStrZOwnership() {

		ASSERT( IsSet() );
		if ( !_inner->str || JL_HasFlags(_inner->strFlags, OWN | NT) )
			CreateOwnStrZ();
		char *tmp = _inner->str;
		_inner->str = NULL;
		return tmp;
	}

	ALWAYS_INLINE const char *GetConstStrZ() {

		ASSERT( IsSet() );
		if ( !_inner->str || !JL_HasFlags(_inner->strFlags, NT) )
			CreateOwnStrZ();
		return _inner->str;
	}

	ALWAYS_INLINE const char *GetConstStrZOrNULL() {

		return IsSet() ? GetConstStrZ() : NULL;
	}

	ALWAYS_INLINE operator const char *() {

		return GetConstStrZ();
	}

private:
	void operator *();
	void operator [](size_t);
	void* operator new(size_t);
	void* operator new[](size_t);
};


///////////////////////////////////////////////////////////////////////////////
// convertion functions

/*
template <class T>
ALWAYS_INLINE void
JL_NullTerminate( T* &buf, size_t len ) {

	ASSERT( jl_msize(buf) >= len + 1 );
	buf[len] = 0;
}

template <>
ALWAYS_INLINE void
JL_NullTerminate( void* &buf, size_t len ) {

	ASSERT( jl_msize(buf) >= len + 1 );
	((char*)buf)[len] = '\0';
}
*/


// JLStr


INLINE NEVER_INLINE JSBool FASTCALL // code too big to be forced inline
JL_JsvalToNative( JSContext * RESTRICT cx, jsval &val, JLStr * RESTRICT str ) {

	if (likely( JSVAL_IS_STRING(val) )) { // for string literals

		*str = JLStr(cx, JSVAL_TO_STRING(val));
		return JS_TRUE;
	}

	if (likely( !JSVAL_IS_PRIMITIVE(val) )) { // for NIBufferGet support

		JSObject *obj = JSVAL_TO_OBJECT(val);
		NIBufferGet fct = BufferGetInterface(cx, obj); // BufferGetNativeInterface
		if ( fct )
			return fct(cx, obj, str);

		if ( js_IsTypedArray(obj) ) {

			js::TypedArray *buf = js::TypedArray::fromJSObject(obj);
			JL_CHK( buf );
			if ( buf->length ) {

				ASSERT( buf->data );
				if ( buf->type == js::TypedArray::TYPE_UINT16 )
					*str = JLStr((const jschar*)buf->data, buf->length, false);
				else
					*str = JLStr((const char*)buf->data, buf->length, false);
			} else {

				*str = JLStr(L(""), 0, true);
			}
			return JS_TRUE;
		}

		// the following conversion can be replaced by: (new Uint8Array([1,2,3]))
		//		if ( JL_ObjectIsArray(cx, obj) )
		//			return JL_JSArrayToBuffer(cx, obj, str);
	}
	// fallback
	JSString *jsstr;
	jsstr = JS_ValueToString(cx, val);
//	if ( jsstr == NULL )
//		JL_REPORT_ERROR_NUM( JLSMSG_FAIL_TO_CONVERT_TO, "string" );
	JL_CHKM( jsstr != NULL, E_VALUE, E_CONVERT, E_TY_STRING );
	val = STRING_TO_JSVAL(jsstr); // GC protection
	*str = JLStr(cx, jsstr);
	return JS_TRUE;
	JL_BAD;
}


ALWAYS_INLINE JSBool
JL_NativeToJsval( JSContext *cx, JLStr &cval, jsval *vp ) {

	JSString *str = cval.GetJSString(cx);
	if (likely( str != NULL )) {

		*vp = STRING_TO_JSVAL(str);
		return JS_TRUE;
	} else {

		return JS_FALSE;
	}
}


// jschar

ALWAYS_INLINE JSBool
JL_NativeToJsval( JSContext *cx, const jschar * cval, size_t length, jsval *vp ) {

	if (unlikely( length == 0 )) {

		if (unlikely( cval == NULL ))
			*vp = JSVAL_VOID;
		else
			*vp = JL_GetEmptyStringValue(cx);
		return JS_TRUE;
	}
	ASSERT( cval != NULL );
	JSString *jsstr;
	jsstr = JS_NewUCStringCopyN(cx, cval, length);
	JL_CHK( jsstr );
	*vp = STRING_TO_JSVAL(jsstr);
	return JS_TRUE;
	JL_BAD;
}


typedef Wrap<jschar*> OwnerlessJsstr;

ALWAYS_INLINE JSBool
JL_NativeToJsval( JSContext *cx, OwnerlessJsstr cval, size_t length, jsval *vp ) {

	if (unlikely( length == 0 )) {

		if (unlikely( cval == NULL ))
			*vp = JSVAL_VOID;
		else
			*vp = JL_GetEmptyStringValue(cx);
		return JS_TRUE;
	}
	ASSERT( cval != NULL );
	ASSERT( msize(cval) > length );
	ASSERT( cval[length] == 0 );
	JSString *jsstr;
	jsstr = JS_NewUCString(cx, cval, length);
	JL_CHK( jsstr );
	*vp = STRING_TO_JSVAL(jsstr);
	return JS_TRUE;
	JL_BAD;
}


// c-strings

ALWAYS_INLINE JSBool
JL_NativeToJsval( JSContext *cx, const char* cval, jsval *vp ) {

	if (unlikely( cval == NULL )) {

		*vp = JSVAL_VOID;
		return JS_TRUE;
	}
	if (unlikely( *cval == '\0' )) {

		*vp = JL_GetEmptyStringValue(cx);
		return JS_TRUE;
	}
	JSString *jsstr = JS_NewStringCopyZ(cx, cval); // copy is mandatory
	JL_CHK( jsstr );
	*vp = STRING_TO_JSVAL(jsstr);
	return JS_TRUE;
	JL_BAD;
}


ALWAYS_INLINE JSBool
JL_NativeToJsval( JSContext *cx, const char* cval, size_t length, jsval *vp ) {

	if (unlikely( length == 0 )) {

		if (unlikely( cval == NULL ))
			*vp = JSVAL_VOID;
		else
			*vp = JL_GetEmptyStringValue(cx);
		return JS_TRUE;
	}
	ASSERT( cval != NULL );
	JSString *jsstr = JS_NewStringCopyN(cx, cval, length); // copy is mandatory
	JL_CHK( jsstr );
	*vp = STRING_TO_JSVAL(jsstr);
	return JS_TRUE;
	JL_BAD;
}


// blob

ALWAYS_INLINE JSBool
JL_NativeToJsval( JSContext *cx, const uint8_t *cval, size_t length, jsval *vp ) {

	return JL_NativeToJsval(cx, (const char *)cval, length, vp); // use c-strings one.
}


// int8

//JSBool JL_NativeToJsval( JSContext *cx, const int8_t &num, jsval *vp );
JSBool JL_JsvalToNative( JSContext *cx, const jsval &val, int8_t *num );


// uint8

ALWAYS_INLINE JSBool
JL_NativeToJsval( JSContext *cx, const uint8_t &num, jsval *vp ) {

	JL_IGNORE(cx);
	*vp = INT_TO_JSVAL(num);
	return JS_TRUE;
}


ALWAYS_INLINE JSBool
JL_JsvalToNative( JSContext *cx, const jsval &val, uint8_t *num ) {

	if (likely( JSVAL_IS_INT(val) )) {

		jsint tmp = JSVAL_TO_INT(val);
		if (likely( tmp <= UINT8_MAX && tmp >= 0 )) {

			*num = uint8_t(tmp);
			return JS_TRUE;
		}
	
		JL_ERR( E_VALUE, E_RANGE, E_INTERVAL_NUM(0, UINT8_MAX) );
	}

	UNLIKELY_SPLIT_BEGIN( JSContext *cx, const jsval &val, uint8_t *num )

	jsdouble d;
	if (likely( JSVAL_IS_DOUBLE(val) ))
		d = JSVAL_TO_DOUBLE(val);
	else
		JL_CHK( JS_ValueToNumber(cx, val, &d) );

	if (likely( d >= jsdouble(0) && d <= jsdouble(UINT8_MAX) )) {

		JL_ASSERT_WARN( JL_DOUBLE_IS_INTEGER(d), E_VALUE, E_PRECISION );
		*num = uint8_t(d);
		return JS_TRUE;
	}

	JL_ERR( E_VALUE, E_RANGE, E_INTERVAL_NUM(0, UINT8_MAX) );
	JL_BAD;

	UNLIKELY_SPLIT_END(cx, val, num)

}


// int16

//JSBool JL_NativeToJsval( JSContext *cx, const int16_t &num, jsval *vp );

ALWAYS_INLINE JSBool
JL_JsvalToNative( JSContext *cx, const jsval &val, int16_t *num ) {

	if (likely( JSVAL_IS_INT(val) )) {

		jsint tmp = JSVAL_TO_INT(val);
		if (likely( tmp <= INT16_MAX && tmp >= INT16_MIN )) {

			*num = int16_t(tmp);
			return JS_TRUE;
		}
		JL_ERR( E_VALUE, E_RANGE, E_INTERVAL_NUM(INT16_MIN, INT16_MAX) );
	}

	UNLIKELY_SPLIT_BEGIN( JSContext *cx, const jsval &val, int16_t *num )

	jsdouble d;
	if (likely( JSVAL_IS_DOUBLE(val) ))
		d = JSVAL_TO_DOUBLE(val);
	else
		JL_CHK( JS_ValueToNumber(cx, val, &d) );

	if (likely( d >= jsdouble(INT16_MIN) && d <= jsdouble(INT16_MAX) )) {

		JL_ASSERT_WARN( JL_DOUBLE_IS_INTEGER(d), E_VALUE, E_PRECISION );
		*num = int16_t(d);
		return JS_TRUE;
	}

	JL_ERR( E_VALUE, E_RANGE, E_INTERVAL_NUM(INT16_MIN, INT16_MAX) );
	JL_BAD;

	UNLIKELY_SPLIT_END(cx, val, num)

}


// uint16


ALWAYS_INLINE JSBool
JL_NativeToJsval( JSContext *cx, const uint16_t &num, jsval *vp ) {

	JL_IGNORE(cx);
	*vp = INT_TO_JSVAL(num);
	return JS_TRUE;
}


ALWAYS_INLINE JSBool
JL_JsvalToNative( JSContext *cx, const jsval &val, uint16_t *num ) {

	if (likely( JSVAL_IS_INT(val) )) {

		jsint tmp = JSVAL_TO_INT(val);
		if (likely( tmp <= UINT16_MAX )) {

			*num = uint16_t(tmp);
			return JS_TRUE;
		}
		JL_ERR( E_VALUE, E_RANGE, E_INTERVAL_NUM(0, UINT16_MAX) );
	}

	UNLIKELY_SPLIT_BEGIN( JSContext *cx, const jsval &val, uint16_t *num )

	jsdouble d;
	if (likely( JSVAL_IS_DOUBLE(val) ))
		d = JSVAL_TO_DOUBLE(val);
	else
		JL_CHK( JS_ValueToNumber(cx, val, &d) );

	if (likely( d >= jsdouble(0) && d <= jsdouble(UINT16_MAX) )) {

		JL_ASSERT_WARN( JL_DOUBLE_IS_INTEGER(d), E_VALUE, E_PRECISION );
		*num = uint16_t(d);
		return JS_TRUE;
	}

	JL_ERR( E_VALUE, E_RANGE, E_INTERVAL_NUM(0, UINT16_MAX) );
	JL_BAD;

	UNLIKELY_SPLIT_END(cx, val, num)

}


// int32

S_ASSERT( INT32_MIN == JSVAL_INT_MIN && INT32_MAX == JSVAL_INT_MAX );

ALWAYS_INLINE JSBool
JL_NativeToJsval( JSContext *cx, const int32_t &num, jsval *vp ) {

	JL_IGNORE(cx);
	*vp = INT_TO_JSVAL(num);
	return JS_TRUE;
}


ALWAYS_INLINE JSBool
JL_JsvalToNative( JSContext *cx, const jsval &val, int32_t *num ) {

	if (likely( JSVAL_IS_INT(val) )) {

		*num = JSVAL_TO_INT(val);
		return JS_TRUE;
	}

	UNLIKELY_SPLIT_BEGIN( JSContext *cx, const jsval &val, int32_t *num )

	jsdouble d;
	if (likely( JSVAL_IS_DOUBLE(val) ))
		d = JSVAL_TO_DOUBLE(val);
	else
		JL_CHK( JS_ValueToNumber(cx, val, &d) ); // NULL gives 0

	if (likely( d >= jsdouble(INT32_MIN) && d <= jsdouble(INT32_MAX) )) {

		JL_ASSERT_WARN( JL_DOUBLE_IS_INTEGER(d), E_VALUE, E_PRECISION );
		*num = int32_t(d);
		return JS_TRUE;
	}

	JL_ERR( E_VALUE, E_RANGE, E_INTERVAL_STR("-2^31", "2^31-1") );
	JL_BAD;

	UNLIKELY_SPLIT_END(cx, val, num)

}


// uint32

S_ASSERT( UINT32_MAX >= JSVAL_INT_MAX );

ALWAYS_INLINE JSBool
JL_NativeToJsval( JSContext *cx, const uint32_t &num, jsval *vp ) {

	JL_IGNORE(cx);
	if (likely( num <= uint32_t(JSVAL_INT_MAX) )) {

		*vp = INT_TO_JSVAL(num);
		return JS_TRUE;
	}

	UNLIKELY_SPLIT_BEGIN( const uint32_t &num, jsval *vp )

		ASSERT( jsdouble(num) <= MAX_INT_TO_DOUBLE );
		*vp = DOUBLE_TO_JSVAL(jsdouble(num));
		return JS_TRUE;

	UNLIKELY_SPLIT_END(num, vp)

}

ALWAYS_INLINE JSBool
JL_JsvalToNative( JSContext *cx, const jsval &val, uint32_t *num ) {

	if (likely( JSVAL_IS_INT(val) )) {

		jsint tmp = JSVAL_TO_INT(val);
		if (likely( tmp >= 0 )) {

			*num = uint32_t(tmp);
			return JS_TRUE;
		}
		JL_ERR( E_VALUE, E_RANGE, E_INTERVAL_STR("0", "2^32") );
	}

	UNLIKELY_SPLIT_BEGIN( JSContext *cx, const jsval &val, uint32_t *num )

	jsdouble d;
	if (likely( JSVAL_IS_DOUBLE(val) ))
		d = JSVAL_TO_DOUBLE(val);
	else
		JL_CHK( JS_ValueToNumber(cx, val, &d) );

	if (likely( d >= jsdouble(0) && d <= jsdouble(UINT32_MAX) )) {

		JL_ASSERT_WARN( JL_DOUBLE_IS_INTEGER(d), E_VALUE, E_PRECISION );
		*num = uint32_t(d);
		return JS_TRUE;
	}

	JL_ERR( E_VALUE, E_RANGE, E_INTERVAL_STR("0", "2^32") );
	JL_BAD;

	UNLIKELY_SPLIT_END(cx, val, num)

}


// int64

ALWAYS_INLINE JSBool
JL_NativeToJsval( JSContext *cx, const int64_t &num, jsval *vp ) {

	if (likely( num <= int64_t(JSVAL_INT_MAX) && num >= int64_t(JSVAL_INT_MIN) )) {

		*vp = INT_TO_JSVAL(jsint(num));
	} else {

		JL_CHKM( num >= int64_t(MIN_INT_TO_DOUBLE) && num <= int64_t(MAX_INT_TO_DOUBLE), E_VALUE, E_RANGE, E_INTERVAL_STR("-2^53", "2^53") );
		*vp = DOUBLE_TO_JSVAL(jsdouble(num));
	}
	return JS_TRUE;
	JL_BAD;
}

ALWAYS_INLINE JSBool
JL_JsvalToNative( JSContext *cx, const jsval &val, int64_t *num ) {

	if (likely( JSVAL_IS_INT(val) )) {

		*num = JSVAL_TO_INT(val);
		return JS_TRUE;
	}

	UNLIKELY_SPLIT_BEGIN( JSContext *cx, const jsval &val, int64_t *num )

	jsdouble d;
	if (likely( JSVAL_IS_DOUBLE(val) ))
		d = JSVAL_TO_DOUBLE(val);
	else
		JL_CHK( JS_ValueToNumber(cx, val, &d) ); // NULL gives 0

	if (likely( d >= jsdouble(MIN_INT_TO_DOUBLE) && d <= jsdouble(MAX_INT_TO_DOUBLE) )) {

		JL_ASSERT_WARN( JL_DOUBLE_IS_INTEGER(d), E_VALUE, E_PRECISION );
		*num = int64_t(d);
		return JS_TRUE;
	}

	JL_ERR( E_VALUE, E_RANGE, E_INTERVAL_STR("-2^53", "2^53") );
	JL_BAD;

	UNLIKELY_SPLIT_END(cx, val, num)

}


// uint64

ALWAYS_INLINE JSBool
JL_NativeToJsval( JSContext *cx, const uint64_t &num, jsval *vp ) {

	if ( num <= uint64_t(JSVAL_INT_MAX) ) {

		*vp = INT_TO_JSVAL(int32(num));
		return JS_TRUE;
	}

	if ( num <= uint64_t(MAX_INT_TO_DOUBLE) ) {

		*vp = DOUBLE_TO_JSVAL(jsdouble(num));
		return JS_TRUE;
	}

	JL_ERR( E_VALUE, E_RANGE, E_INTERVAL_STR("0", "2^53") );
	JL_BAD;

/*
	UNLIKELY_SPLIT_BEGIN( JSContext *cx, const uint64_t &num, jsval *vp )

	JL_SAFE_BEGIN
	if (unlikely( num > MAX_INT_TO_DOUBLE ))
		JL_REPORT_WARNING_NUM( JLSMSG_LOSS_OF_PRECISION);
	JL_SAFE_END
	*vp = DOUBLE_TO_JSVAL(jsdouble(num));
	return JS_TRUE;
	JL_BAD;

	UNLIKELY_SPLIT_END(cx, num, vp);
*/
}


ALWAYS_INLINE JSBool
JL_JsvalToNative( JSContext *cx, const jsval &val, uint64_t *num ) {

	if (likely( JSVAL_IS_INT(val) )) {

		*num = JSVAL_TO_INT(val);
		return JS_TRUE;
	}

	UNLIKELY_SPLIT_BEGIN( JSContext *cx, const jsval &val, uint64_t *num )

	jsdouble d;
	if (likely( JSVAL_IS_DOUBLE(val) ))
		d = JSVAL_TO_DOUBLE(val);
	else
		JL_CHK( JS_ValueToNumber(cx, val, &d) ); // NULL gives 0

	if (likely( d >= jsdouble(0) && d <= jsdouble(MAX_INT_TO_DOUBLE) )) { // or d <= jsdouble(UINT64_MAX)

		JL_ASSERT_WARN( JL_DOUBLE_IS_INTEGER(d), E_VALUE, E_PRECISION );
		*num = uint64_t(d);
		return JS_TRUE;
	}

	JL_ERR( E_VALUE, E_RANGE, E_INTERVAL_STR("0", "2^53") ); // 0x20000000000000
	JL_BAD;

	UNLIKELY_SPLIT_END(cx, val, num)

}


// long

ALWAYS_INLINE JSBool
JL_NativeToJsval( JSContext *cx, const long &num, jsval *vp ) {

	if ( sizeof(long) == sizeof(int64_t) ) return JL_NativeToJsval(cx, int64_t(num), vp);
	if ( sizeof(long) == sizeof(int32_t) ) return JL_NativeToJsval(cx, int32_t(num), vp);
	ASSERT(false);
}

ALWAYS_INLINE JSBool
JL_JsvalToNative( JSContext *cx, const jsval &val, long *num ) {

	if ( sizeof(long) == sizeof(int64_t) ) return JL_JsvalToNative(cx, val, (int64_t*)num);
	if ( sizeof(long) == sizeof(int32_t) ) return JL_JsvalToNative(cx, val, (int32_t*)num);
	ASSERT(false);
}


// unsigned long

ALWAYS_INLINE JSBool
JL_NativeToJsval( JSContext *cx, const unsigned long &num, jsval *vp ) {

	if ( sizeof(unsigned long) == sizeof(uint64_t) ) return JL_NativeToJsval(cx, uint64_t(num), vp);
	if ( sizeof(unsigned long) == sizeof(uint32_t) ) return JL_NativeToJsval(cx, uint32_t(num), vp);
	ASSERT(false);
}

ALWAYS_INLINE JSBool
JL_JsvalToNative( JSContext *cx, const jsval &val, unsigned long *num ) {

	if ( sizeof(unsigned long) == sizeof(uint64_t) ) return JL_JsvalToNative(cx, val, (uint64_t*)num);
	if ( sizeof(unsigned long) == sizeof(uint32_t) ) return JL_JsvalToNative(cx, val, (uint32_t*)num);
	ASSERT(false);
}


/*
// size_t

ALWAYS_INLINE JSBool JL_NativeToJsval( JSContext *cx, const size_t &num, jsval *vp ) {

	JL_IGNORE(cx);
	if (likely( num <= JSVAL_INT_MAX ))
		*vp = INT_TO_JSVAL(jsint(num));
	else
		*vp = DOUBLE_TO_JSVAL(jsdouble(num));
	return JS_TRUE;
}

ALWAYS_INLINE JSBool JL_JsvalToNative( JSContext *cx, const jsval &val, size_t *num ) {

	if (likely( JSVAL_IS_INT(val) )) {

		jsint tmp = JSVAL_TO_INT(val);
		if (unlikely( tmp < 0 ))
			JL_REPORT_ERROR_NUM( JLSMSG_VALUE_OUTOFRANGE, ...);
		*num = size_t(uint32_t(tmp));
		return JS_TRUE;
	}

	jsdouble d;
	if (likely( JSVAL_IS_DOUBLE(val) ))
		d = JSVAL_TO_DOUBLE(val);
	else
		JL_CHK( JS_ValueToNumber(cx, val, &d) );

	if (likely( d >= jsdouble(0) && d <= jsdouble(SIZE_T_MAX) )) { // cannot use jl::IsSafeCast(d, *i) because if d is not integer, the test fails.

		JL_SAFE_BEGIN
		if ( !jl::IsSafeCast(d, *num) )
			JL_REPORT_WARNING_NUM( JLSMSG_LOSS_OF_PRECISION);
		JL_SAFE_END

		*num = size_t(d);
		return JS_TRUE;
	}

	JL_REPORT_ERROR_NUM( JLSMSG_VALUE_OUTOFRANGE, ...);
	JL_BAD;
}


// ptrdiff_t

ALWAYS_INLINE JSBool JL_NativeToJsval( JSContext *cx, const ptrdiff_t &num, jsval *vp ) {

	JL_IGNORE(cx);
	if (likely( num >= JSVAL_INT_MIN && size <= JSVAL_INT_MAX ))
		*vp = INT_TO_JSVAL(jsint(num));
	else
		*vp = DOUBLE_TO_JSVAL(jsdouble(num));
	return JS_TRUE;
}

ALWAYS_INLINE JSBool JL_JsvalToNative( JSContext *cx, const jsval &val, ptrdiff_t *num ) {

	if (likely( JSVAL_IS_INT(val) )) {

		*num = ptrdiff_t(JSVAL_TO_INT(val));
		return JS_TRUE;
	}

	jsdouble d;
	if (likely( JSVAL_IS_DOUBLE(val) ))
		d = JSVAL_TO_DOUBLE(val);
	else
		JL_CHK( JS_ValueToNumber(cx, val, &d) );

	if (likely( d >= jsdouble(PTRDIFF_MIN) && d <= jsdouble(PTRDIFF_MAX) )) {

		JL_SAFE_BEGIN
		if ( !jl::IsSafeCast(d, *i) )
			JL_REPORT_WARNING_NUM( JLSMSG_LOSS_OF_PRECISION);
		JL_SAFE_END

		*num = (ptrdiff_t)d;
		return JS_TRUE;
	}

	JL_REPORT_ERROR_NUM( JLSMSG_VALUE_OUTOFRANGE, ...);
	JL_BAD;
}
*/



// double

ALWAYS_INLINE JSBool
JL_NativeToJsval( JSContext *cx, const double &num, jsval *vp ) {

	JL_IGNORE(cx);
	*vp = DOUBLE_TO_JSVAL(num);
	return JS_TRUE;
}

ALWAYS_INLINE JSBool
JL_JsvalToNative( JSContext *cx, const jsval &val, double *num ) {

	if (likely( JSVAL_IS_DOUBLE(val) )) {

		*num = JSVAL_TO_DOUBLE(val);
		return JS_TRUE;
	}
	if (likely( JSVAL_IS_INT(val) )) {

		*num = double(JSVAL_TO_INT(val));
		return JS_TRUE;
	}

	UNLIKELY_SPLIT_BEGIN( JSContext *cx, const jsval &val, double *num )

	if ( !JS_ValueToNumber(cx, val, num) )
		return JS_FALSE;
	ASSERT(isnan(cx->runtime->NaNValue.getDoubleRef()));
	JL_CHKM( !isnan(*num), E_VALUE, E_TYPE, E_TY_NUMBER );
	return JS_TRUE;
	JL_BAD;

	UNLIKELY_SPLIT_END(cx, val, num)

}


// float

ALWAYS_INLINE JSBool
JL_NativeToJsval( JSContext *cx, const float &num, jsval *vp ) {

	JL_IGNORE(cx);
	*vp = DOUBLE_TO_JSVAL(jsdouble(num));
	return JS_TRUE;
}

ALWAYS_INLINE JSBool
JL_JsvalToNative( JSContext *cx, const jsval &val, float *num ) {

	if (likely( JSVAL_IS_DOUBLE(val) )) {

		*num = float(JSVAL_TO_DOUBLE(val));
		return JS_TRUE;
	}
	if (likely( JSVAL_IS_INT(val) )) {

		*num = float(JSVAL_TO_INT(val));
		return JS_TRUE;
	}

	UNLIKELY_SPLIT_BEGIN( JSContext *cx, const jsval &val, float *num )

	jsdouble tmp;
	if ( !JS_ValueToNumber(cx, val, &tmp) )
		return JS_FALSE;
	ASSERT(isnan(cx->runtime->NaNValue.getDoubleRef()));
	JL_CHKM( !isnan(tmp), E_VALUE, E_TYPE, E_TY_NUMBER );
	*num = float(tmp);
	return JS_TRUE;
	JL_BAD;

	UNLIKELY_SPLIT_END(cx, val, num)

}


// boolean

ALWAYS_INLINE JSBool
JL_NativeToJsval( JSContext *cx, const bool &b, jsval *vp ) {

	JL_IGNORE(cx);
	*vp = BOOLEAN_TO_JSVAL(b);
	return JS_TRUE;
}

ALWAYS_INLINE JSBool
JL_JsvalToNative( JSContext *cx, const jsval &val, bool *b ) {

	if (likely( JSVAL_IS_BOOLEAN(val) )) {

		*b = (JSVAL_TO_BOOLEAN(val) == JS_TRUE);
		return JS_TRUE;
	}

	JSBool tmp;
	if (unlikely( !JS_ValueToBoolean(cx, val, &tmp) ))
		return JS_FALSE;
	*b = (tmp == JS_TRUE);
	return JS_TRUE;
}


// void*

ALWAYS_INLINE JSBool
JL_NativeToJsval( JSContext *cx, void *ptr, jsval *vp ) {

	if ( ((uint32)ptr & 1) == 0 ) {
	
		*vp = PRIVATE_TO_JSVAL(ptr);
	} else {

		JSObject *obj = JL_NewProtolessObj(cx);
		JL_CHK( obj );
		*vp = OBJECT_TO_JSVAL(obj);
		jsval tmp;

		if ( 8 * sizeof(ptrdiff_t) == 32 ) {

			tmp = INT_TO_JSVAL( reinterpret_cast<int32>(ptr) );
			JL_CHK( JS_SetPropertyById(cx, obj, INT_TO_JSID(0), &tmp) );
		} else
		if ( 8 * sizeof(ptrdiff_t) == 64 ) {

			#pragma warning(push)
			#pragma warning(disable:4293)
			tmp = INT_TO_JSVAL( reinterpret_cast<ptrdiff_t>(ptr) & 0xFFFFFFFF );
			JL_CHK( JS_SetPropertyById(cx, obj, INT_TO_JSID(0), &tmp) );
			tmp = INT_TO_JSVAL( reinterpret_cast<ptrdiff_t>(ptr) >> 32 );
			JL_CHK( JS_SetPropertyById(cx, obj, INT_TO_JSID(1), &tmp) );
			#pragma warning(pop)
		} else {

			ASSERT(false);
		}
	}
	return JS_TRUE;
	JL_BAD;
}

ALWAYS_INLINE JSBool
JL_JsvalToNative( JSContext *cx, const jsval &val, void **ptr ) {

	if ( JSVAL_IS_OBJECT(val) ) {

		jsval tmp;
		JSObject *obj = JSVAL_TO_OBJECT(val);

		if ( 8 * sizeof(ptrdiff_t) == 32 ) {

			JL_CHK( JS_GetPropertyById(cx, obj, INT_TO_JSID(0), &tmp) );
			*ptr = reinterpret_cast<void*>( JSVAL_TO_INT(tmp) );
		} else
		if ( 8 * sizeof(ptrdiff_t) == 64 ) {

			#pragma warning(push)
			#pragma warning(disable:4293)
			uint32_t h, l;
			JL_CHK( JS_GetPropertyById(cx, obj, INT_TO_JSID(0), &tmp) );
			l = static_cast<uint32_t>(JSVAL_TO_INT(tmp));
			JL_CHK( JS_GetPropertyById(cx, obj, INT_TO_JSID(1), &tmp) );
			h = static_cast<uint32_t>(JSVAL_TO_INT(tmp));
			*ptr = reinterpret_cast<void*>( (h << 32) | l );
			#pragma warning(pop)
		} else {

			ASSERT(false);
		}
	} else {
		
		JL_CHKM( JSVAL_IS_DOUBLE(val), E_JSLIBS, E_INTERNAL );
		*ptr = JSVAL_TO_PRIVATE(val);
	}
	return JS_TRUE;
	JL_BAD;
}


///////////////////////////////////////////////////////////////////////////////
// vector convertion functions

// if useValArray is true, val must be a valid array that is used to store the values.
template <class T>
ALWAYS_INLINE JSBool FASTCALL
JL_NativeVectorToJsval( JSContext * RESTRICT cx, const T * RESTRICT vector, jsuint length, jsval * RESTRICT val, bool useValArray = false ) {

	ASSERT( vector );
	ASSERT( val );

	JSObject *arrayObj;
	if (likely( useValArray )) {

		JL_ASSERT_IS_OBJECT(*val, "vector");
		arrayObj = JSVAL_TO_OBJECT(*val);
		JL_CHK( JS_SetArrayLength(cx, arrayObj, length) );
	} else {

		arrayObj = JS_NewArrayObject(cx, length, NULL);
		JL_CHK( arrayObj );
		*val = OBJECT_TO_JSVAL(arrayObj);
	}

	jsval tmp;
	for ( jsuint i = 0; i < length; ++i ) {

		JL_CHK( JL_NativeToJsval(cx, vector[i], &tmp) );
		JL_CHK( JL_SetElement(cx, arrayObj, i, &tmp) );
	}

	return JS_TRUE;
	JL_BAD;
}


ALWAYS_INLINE uint32_t JLNativeTypeToTypedArrayType( const int8_t & ) { return js::TypedArray::TYPE_INT8; }
ALWAYS_INLINE uint32_t JLNativeTypeToTypedArrayType( const uint8_t & ) { return js::TypedArray::TYPE_UINT8; }
ALWAYS_INLINE uint32_t JLNativeTypeToTypedArrayType( const int16_t & ) { return js::TypedArray::TYPE_INT16; }
ALWAYS_INLINE uint32_t JLNativeTypeToTypedArrayType( const uint16_t & ) { return js::TypedArray::TYPE_UINT16; }
ALWAYS_INLINE uint32_t JLNativeTypeToTypedArrayType( const int32_t & ) { return js::TypedArray::TYPE_INT32; }
ALWAYS_INLINE uint32_t JLNativeTypeToTypedArrayType( const uint32_t & ) { return js::TypedArray::TYPE_UINT32; }
ALWAYS_INLINE uint32_t JLNativeTypeToTypedArrayType( const float32_t & ) { return js::TypedArray::TYPE_FLOAT32; }
ALWAYS_INLINE uint32_t JLNativeTypeToTypedArrayType( const float64_t & ) { return js::TypedArray::TYPE_FLOAT64; }

ALWAYS_INLINE const char * JLNativeTypeToString( const int8_t & ) { return "Int8Array"; }
ALWAYS_INLINE const char * JLNativeTypeToString( const uint8_t & ) { return "Uint8Array"; }
ALWAYS_INLINE const char * JLNativeTypeToString( const int16_t & ) { return "Int16Array"; }
ALWAYS_INLINE const char * JLNativeTypeToString( const uint16_t & ) { return "Uint16Array"; }
ALWAYS_INLINE const char * JLNativeTypeToString( const int32_t & ) { return "Int32Array"; }
ALWAYS_INLINE const char * JLNativeTypeToString( const uint32_t & ) { return "Uint32Array"; }
ALWAYS_INLINE const char * JLNativeTypeToString( const float32_t & ) { return "Float32Array"; }
ALWAYS_INLINE const char * JLNativeTypeToString( const float64_t & ) { return "Float64Array"; }


template <class T>
INLINE JSBool FASTCALL
JL_TypedArrayToNativeVector( JSContext * RESTRICT cx, JSObject * RESTRICT obj, T * RESTRICT vector, jsuint maxLength, jsuint * RESTRICT actualLength ) {

	ASSERT( js_IsTypedArray(obj) );
	js::TypedArray *ta = js::TypedArray::fromJSObject(obj);
	JL_ASSERT_OBJECT_STATE( ta->valid(), "TypedArray" );
	JL_ASSERT( ta->type == JLNativeTypeToTypedArrayType(*vector), E_STR("TypedArray"), E_TYPE, E_NAME(JLNativeTypeToString(*vector)) );
	*actualLength = ta->length;
	maxLength = JL_MIN( *actualLength, maxLength );
	for ( jsuint i = 0; i < maxLength; ++i ) {

		vector[i] = ((T*)ta->data)[i];
	}
	return JS_TRUE;
	JL_BAD;
}


template <class T>
ALWAYS_INLINE JSBool FASTCALL
JL_JsvalToNativeVector( JSContext * RESTRICT cx, jsval & RESTRICT val, T * RESTRICT vector, jsuint maxLength, jsuint *actualLength ) {

	JL_ASSERT_IS_OBJECT(val, "vector");

	JSObject *arrayObj;
	arrayObj = JSVAL_TO_OBJECT(val);

	if (unlikely( js_IsTypedArray(arrayObj) ))
		return JL_TypedArrayToNativeVector(cx, arrayObj, vector, maxLength, actualLength);

	JL_CHK( JS_GetArrayLength(cx, arrayObj, actualLength) );
	maxLength = JL_MIN( *actualLength, maxLength );
	jsval tmp;
	for ( jsuint i = 0; i < maxLength; ++i ) {  // while ( maxLength-- ) { // avoid reverse walk (L1 cache issue)

		JL_CHK( JL_GetElement(cx, arrayObj, i, &tmp) );
		JL_CHK( JL_JsvalToNative(cx, tmp, &vector[i]) );
	}
	return JS_TRUE;
	JL_BAD;
}



///////////////////////////////////////////////////////////////////////////////
// reserved slot convertion functions

template <class T>
ALWAYS_INLINE JSBool FASTCALL
JL_NativeToReservedSlot( JSContext * RESTRICT cx, JSObject * RESTRICT obj, uintN slot, T &value ) {

	jsval tmp;
	JL_CHK( JL_NativeToJsval(cx, value, &tmp) );
	JL_CHK( JL_SetReservedSlot(cx, obj, slot, tmp) );
	return JS_TRUE;
	JL_BAD;
}


template <class T>
ALWAYS_INLINE JSBool FASTCALL
JL_ReservedSlotToNative( JSContext * RESTRICT cx, JSObject * RESTRICT obj, uintN slot, T * RESTRICT value ) {

	jsval tmp;
	JL_CHK( JL_GetReservedSlot(cx, obj, slot, &tmp) );
	JL_CHK( JL_JsvalToNative(cx, tmp, value) );
	return JS_TRUE;
	JL_BAD;
}


///////////////////////////////////////////////////////////////////////////////
// properties conversion helper

// Set

template <class T>
ALWAYS_INLINE JSBool
JL_SetProperty( JSContext *cx, JSObject *obj, const char *name, const T &cval ) {

	jsval tmp;
	return JL_NativeToJsval(cx, cval, &tmp) && JS_SetProperty(cx, obj, name, &tmp);
}

template <class T>
ALWAYS_INLINE JSBool
JL_SetProperty( JSContext *cx, JSObject *obj, jsid id, const T &cval ) {

	jsval tmp;
	return JL_NativeToJsval(cx, cval, &tmp) && JS_SetPropertyById(cx, obj, id, &tmp);
}

// Define

template <class T>
ALWAYS_INLINE JSBool
JL_DefineProperty( JSContext *cx, JSObject *obj, const char *name, const T &cval, bool visible = true, bool modifiable = true ) {

	jsval tmp;
	return JL_NativeToJsval(cx, cval, &tmp) && JS_DefineProperty(cx, obj, name, tmp, NULL, NULL, (modifiable ? 0 : JSPROP_READONLY | JSPROP_PERMANENT) | (visible ? JSPROP_ENUMERATE : 0) );
}

template <class T>
ALWAYS_INLINE JSBool
JL_DefineProperty( JSContext *cx, JSObject *obj, jsid id, const T &cval, bool visible = true, bool modifiable = true ) {

	jsval tmp;
	return JL_NativeToJsval(cx, cval, &tmp) && JS_DefinePropertyById(cx, obj, id, tmp, NULL, NULL, (modifiable ? 0 : JSPROP_READONLY | JSPROP_PERMANENT) | (visible ? JSPROP_ENUMERATE : 0) );
}

// Get

template <class T>
ALWAYS_INLINE JSBool
JL_GetProperty( JSContext *cx, JSObject *obj, const char *propertyName, T *cval ) {

	jsval tmp;
	return JS_GetProperty(cx, obj, propertyName, &tmp) && JL_JsvalToNative(cx, tmp, cval);
}

template <class T>
ALWAYS_INLINE JSBool
JL_GetProperty( JSContext *cx, JSObject *obj, jsid id, T *cval ) {

	jsval tmp;
	return JS_GetPropertyById(cx, obj, id, &tmp) && JL_JsvalToNative(cx, tmp, cval);
}

// Lookup

template <class T>
ALWAYS_INLINE JSBool
JL_LookupProperty( JSContext *cx, JSObject *obj, const char *propertyName, T *cval ) {

	jsval tmp;
	return JS_LookupProperty(cx, obj, propertyName, &tmp) && JL_JsvalToNative(cx, tmp, cval);
}

template <class T>
ALWAYS_INLINE JSBool
JL_LookupProperty( JSContext *cx, JSObject *obj, jsid id, T *cval ) {

	jsval tmp;
	return JS_LookupPropertyById(cx, obj, id, &tmp) && JL_JsvalToNative(cx, tmp, cval);
}


///////////////////////////////////////////////////////////////////////////////
// jsval convertion functions

INLINE JSBool FASTCALL
JL_JSArrayToBuffer( JSContext * RESTRICT cx, JSObject * RESTRICT arrObj, JLStr * RESTRICT str ) {

	ASSERT( JL_ObjectIsArray(cx, arrObj) );
	jsuint length;
	JL_CHK( JS_GetArrayLength(cx, arrObj, &length) );

	jschar *buf;
	buf = static_cast<jschar*>(jl_malloc(sizeof(jschar) * (length +1)));
	buf[length] = 0;

	jsval elt;
	int32 num;
	for ( jsuint i = 0; i < length; ++i ) {

		JL_CHK( JL_GetElement(cx, arrObj, i, &elt) );
		JL_CHK( JL_JsvalToNative(cx, elt, &num) );
		//JL_CHK( JS_ValueToInt32(cx, elt, &num) );
		buf[i] = (jschar)num;
	}
	*str = JLStr(buf, length, true);
	return JS_TRUE;
	JL_BAD;
}


ALWAYS_INLINE jsid
JL_StringToJsid( JSContext *cx, const jschar *cstr ) {

	JSString *jsstr = JS_InternUCString(cx, cstr);
	if ( jsstr == NULL )
		return JL_NullJsid();
//	jsid tmp;
//	if ( !JS_ValueToId(cx, STRING_TO_JSVAL(jsstr), &tmp) )
//		return JL_NullJsid();
//	return tmp;
#ifdef DEBUG
	jsid id, tmp;
	id = ATOM_TO_JSID(&jsstr->asAtom());
	ASSERT( JSID_IS_STRING( id ) );
	JS_ValueToId(cx, STRING_TO_JSVAL(jsstr), &tmp);
	JS_ASSERT( id == tmp );
	return id;
#else
	return ATOM_TO_JSID(&jsstr->asAtom());
#endif // DEBUG
}


ALWAYS_INLINE JSFunction*
JL_ObjectToFunction( JSContext *cx, const JSObject *obj ) {

	JL_IGNORE(cx);
	return GET_FUNCTION_PRIVATE(cx, obj);
}

ALWAYS_INLINE JSFunction*
JL_JsvalToFunction( JSContext *cx, const jsval &val ) {

	JL_IGNORE(cx);
	return GET_FUNCTION_PRIVATE(cx, JSVAL_TO_OBJECT(val));
}


ALWAYS_INLINE JSBool
JL_JsvalToJsid( JSContext * RESTRICT cx, jsval * RESTRICT val, jsid * RESTRICT id ) {

	if ( JSVAL_IS_INT( *val ) ) {

		*id = INT_TO_JSID( JSVAL_TO_INT( *val ) );
	} else
	if ( JSVAL_IS_OBJECT( *val ) ) {

		*id = OBJECT_TO_JSID( JSVAL_TO_OBJECT( *val ) );
	} else
	if ( JSVAL_IS_STRING( *val ) ) {

		*id = ATOM_TO_JSID(&JSVAL_TO_STRING(*val)->asAtom());
		ASSERT( JSID_IS_STRING( *id ) );
	} else
	if ( JSVAL_IS_VOID( *val ) ) {

		*id = JSID_VOID;
	} else
		return JS_ValueToId(cx, *val, id);
	return JS_TRUE;
}


ALWAYS_INLINE JSBool FASTCALL
JL_JsidToJsval( JSContext * RESTRICT cx, jsid id, jsval * RESTRICT val ) {

	JL_IGNORE(cx);
	*val = js::IdToJsval(id); // see jsatom.h
	return JS_TRUE;
}


INLINE NEVER_INLINE JSBool FASTCALL
JL_JsvalToMatrix44( JSContext * RESTRICT cx, jsval &val, float ** RESTRICT m ) {

	static float Matrix44IdentityValue[16] = {
		 1.0f, 0.0f, 0.0f, 0.0f,
		 0.0f, 1.0f, 0.0f, 0.0f,
		 0.0f, 0.0f, 1.0f, 0.0f,
		 0.0f, 0.0f, 0.0f, 1.0f
	};

	if ( JSVAL_IS_NULL(val) ) {

		memcpy(*m, &Matrix44IdentityValue, sizeof(Matrix44IdentityValue));
		return JS_TRUE;
	}

	JL_ASSERT_IS_OBJECT(val, "matrix44");

	JSObject *matrixObj;
	matrixObj = JSVAL_TO_OBJECT(val);

	NIMatrix44Get Matrix44Get;
	Matrix44Get = Matrix44GetInterface(cx, matrixObj);
	if ( Matrix44Get )
		return Matrix44Get(cx, matrixObj, m);

	if ( js_IsTypedArray(matrixObj) ) {

		js::TypedArray *ta = js::TypedArray::fromJSObject(matrixObj);
		if ( ta->valid() && ta->type == js::TypedArray::TYPE_FLOAT32 && ta->length == 16 ) {

			ASSERT( ta->byteLength / ta->length == sizeof(float32_t) );
			memcpy(*m, ta->data, (/*TYPE_FLOAT32:*/32 / 8) * 16);
			return JS_TRUE;
		}
	}

	if ( JL_ObjectIsArray(cx, matrixObj) ) {

		uint32 length;
		jsval element;
		JL_CHK( JL_GetElement(cx, JSVAL_TO_OBJECT(val), 0, &element) );
		if ( JL_ValueIsArray(cx, element) ) { // support for [ [1,1,1,1], [2,2,2,2], [3,3,3,3], [4,4,4,4] ] matrix

			JL_CHK( JL_JsvalToNativeVector(cx, element, (*m)+0, 4, &length ) );
			JL_ASSERT( length == 4, E_VALUE, E_STR("matrix44[0]"), E_TYPE, E_TY_NVECTOR(4) );

			JL_CHK( JL_GetElement(cx, JSVAL_TO_OBJECT(val), 1, &element) );
			JL_CHK( JL_JsvalToNativeVector(cx, element, (*m)+4, 4, &length ) );
			JL_ASSERT_IS_ARRAY( element, "matrix44[1]" );
			JL_ASSERT( length == 4, E_VALUE, E_STR("matrix44[1]"), E_TYPE, E_TY_NVECTOR(4) );

			JL_CHK( JL_GetElement(cx, JSVAL_TO_OBJECT(val), 2, &element) );
			JL_CHK( JL_JsvalToNativeVector(cx, element, (*m)+8, 4, &length ) );
			JL_ASSERT_IS_ARRAY( element, "matrix44[2]" );
			JL_ASSERT( length == 4, E_VALUE, E_STR("matrix44[2]"), E_TYPE, E_TY_NVECTOR(4) );

			JL_CHK( JL_GetElement(cx, JSVAL_TO_OBJECT(val), 3, &element) );
			JL_CHK( JL_JsvalToNativeVector(cx, element, (*m)+12, 4, &length ) );
			JL_ASSERT_IS_ARRAY( element, "matrix44[3]" );
			JL_ASSERT( length == 4, E_VALUE, E_STR("matrix44[3]"), E_TYPE, E_TY_NVECTOR(4) );
			return JS_TRUE;
		}

		JL_CHK( JL_JsvalToNativeVector(cx, val, *m, 16, &length ) );  // support for [ 1,1,1,1, 2,2,2,2, 3,3,3,3, 4,4,4,4 ] matrix
		JL_ASSERT( length == 16, E_VALUE, E_STR("matrix44"), E_TYPE, E_TY_NVECTOR(16) );
		return JS_TRUE;
	}

	JL_ERR( E_VALUE, E_STR("matrix44"), E_INVALID );
	JL_BAD;
}


#define JL_ARG_GEN(N, type) TYPE arg##N; JL_CHK( JL_JsvalToNative(cx, JL_ARG(n), &arg##N) );


///////////////////////////////////////////////////////////////////////////////
// Host info functions (_host global property)


ALWAYS_INLINE JSBool
RemoveHostObject(JSContext *cx) {

	JSObject *globalObject = JL_GetGlobalObject(cx);
	ASSERT( globalObject );
	return JS_DeletePropertyById(cx, globalObject, JLID(cx, _host));
	JL_BAD;
}


INLINE JSObject* FASTCALL
GetHostObject(JSContext *cx) {

	JSObject *cobj, *globalObject = JL_GetGlobalObject(cx);
	JL_CHK( globalObject );
	jsval hostObjectValue;
	jsid hostObjectId;
	hostObjectId = JLID(cx, _host);
	JL_CHK( hostObjectId != JL_NullJsid() );
	JL_CHK( JS_GetPropertyById(cx, globalObject, hostObjectId, &hostObjectValue) );

	if ( JSVAL_IS_VOID( hostObjectValue ) ) { // if configuration object do not exist, we build one

		cobj = JS_NewObject(cx, NULL, NULL, NULL); // possible to use JL_NewObj ?
		JL_CHK( cobj );
		jsval val;
		val = OBJECT_TO_JSVAL( cobj );
		JL_CHK( JS_SetPropertyById(cx, globalObject, hostObjectId, &val) );
		//cobj = JS_DefineObject(cx, globalObject, JLID_NAME(cx, _host), NULL, NULL, 0 );
		//JL_CHK( cobj ); // Doc: If the property already exists, or cannot be created, JS_DefineObject returns NULL.
	} else {
		JL_CHK( JSVAL_IS_OBJECT(hostObjectValue) );
		cobj = JSVAL_TO_OBJECT(hostObjectValue);
	}
	return cobj;
bad:
	return NULL;
}


ALWAYS_INLINE JSBool GetHostObjectValue(JSContext *cx, jsid id, jsval *value) {

	JSObject *cobj = GetHostObject(cx);
	if ( cobj )
		return JS_LookupPropertyById(cx, cobj, id, value);
	*value = JSVAL_VOID;
	return JS_TRUE;
}


ALWAYS_INLINE JSBool GetHostObjectValue(JSContext *cx, const jschar *name, jsval *value) {

	return GetHostObjectValue(cx, JL_StringToJsid(cx, name), value);
}


ALWAYS_INLINE JSBool SetHostObjectValue(JSContext *cx, jsid id, jsval value, bool modifiable = true, bool visible = true) {

	JSObject *cobj = GetHostObject(cx);
	if ( cobj )
		return JS_DefinePropertyById(cx, cobj, id, value, NULL, NULL, (modifiable ? 0 : JSPROP_READONLY | JSPROP_PERMANENT) | (visible ? JSPROP_ENUMERATE : 0) );
	return JS_TRUE;
}


ALWAYS_INLINE JSBool SetHostObjectValue(JSContext *cx, const jschar *name, jsval value, bool modifiable = true, bool visible = true) {

	return SetHostObjectValue(cx, JL_StringToJsid(cx, name), value, modifiable, visible);
}



///////////////////////////////////////////////////////////////////////////////
// Blob functions

// note: a Blob is either a JSString or a Blob object if the jslang module has been loaded.
//       returned value is equivalent to: var ret = Blob(buffer);
INLINE NEVER_INLINE JSBool FASTCALL
JL_NewBlob( JSContext * RESTRICT cx, void* RESTRICT buffer, size_t length, jsval * RESTRICT vp ) {

	if (unlikely( length == 0 || buffer == NULL )) { // Empty Blob must acts like an empty string: !'' === true

		if ( buffer )
			JS_free(cx, buffer);
		*vp = JL_GetEmptyStringValue(cx);
		return JS_TRUE;
	}

	ASSERT( jl_msize(buffer) >= length + 1 );
	ASSERT( ((uint8_t*)buffer)[length] == 0 );

	const ClassProtoCache *classProtoCache = JL_GetCachedClassProto(JL_GetHostPrivate(cx), "Blob");
	if (likely( classProtoCache->clasp != NULL )) { // we have Blob class, jslang is present.

		JSObject *blob;
		blob = JS_ConstructObject(cx, classProtoCache->clasp, classProtoCache->proto, NULL);
		JL_CHK( blob );
		*vp = OBJECT_TO_JSVAL(blob);
		JL_CHK( JL_SetReservedSlot(cx, blob, 0, INT_TO_JSVAL( (int32)length )) ); // slot 0 is SLOT_BLOB_LENGTH.
		JL_SetPrivate(cx, blob, buffer); // blob data
		return JS_TRUE;
	}

	*vp = STRING_TO_JSVAL( JLStr((uint8_t *)buffer, length, true).GetJSString(cx) );
	buffer = NULL; // see bad:

	// now we want a string object, not a string literal.
	JSObject *strObj;
	JL_CHK( JS_ValueToObject(cx, *vp, &strObj) ); // see. OBJ_DEFAULT_VALUE(cx, obj, JSTYPE_OBJECT, &v)
	*vp = OBJECT_TO_JSVAL(strObj);
	return JS_TRUE;

bad:
	JS_free(cx, buffer); // JS_NewString does not free the buffer on error.
	return JS_FALSE;
}


ALWAYS_INLINE JSBool
JL_NewBlobCopyN( JSContext * RESTRICT cx, const void * RESTRICT buffer, size_t length, jsval * RESTRICT vp ) {

	if (unlikely( length == 0 || buffer == NULL )) { // Empty Blob must acts like an empty string: !'' == true

		*vp = JL_GetEmptyStringValue(cx);
		return JS_TRUE;
	}
	// possible optimization: if Blob class is not abailable, copy buffer into JSString's jschar to avoid js_InflateString.
	char *blobBuf = (char *)JS_malloc(cx, length +1);
	JL_CHK( blobBuf );
	memcpy( blobBuf, buffer, length );
	blobBuf[length] = '\0';
	if ( JL_NewBlob(cx, blobBuf, length, vp) )
		return JS_TRUE;
	JS_free(cx, blobBuf);
	JL_BAD;
}



///////////////////////////////////////////////////////////////////////////////
// Helper functions

// see JSAtomState struct in jsatom.h
#define JL_ATOMJSID(CX, NAME) \
	ATOM_TO_JSID(JL_GetRuntime(CX)->atomState.NAME##Atom)


ALWAYS_INLINE JSProtoKey
JL_GetClassProtoKey(const JSClass *clasp) {

    JSProtoKey key = JSCLASS_CACHED_PROTO_KEY(clasp);
    if (key != JSProto_Null)
        return key;
    if (clasp->flags & JSCLASS_IS_ANONYMOUS) // and JSCLASS_IS_GLOBAL ?
        return JSProto_Object;
    return JSProto_Null;
}


ALWAYS_INLINE JSProtoKey
JL_GetObjectProtoKey( JSContext *cx, JSObject *obj ) {

	JSObject *global = JL_GetGlobalObject(cx); //JS_GetGlobalForScopeChain(cx);
	JSObject *proto;
	const JSObject *objProto = JS_GetPrototype(cx, obj);
	JSProtoKey protoKey = JL_GetClassProtoKey(JL_GetClass(obj));
	if ( protoKey == JSProto_Null )
		return JSProto_Null;
	if ( !js_GetClassPrototype(cx, global, protoKey, &proto) )
		return JSProto_Null;
	if ( objProto == proto )
		return protoKey;
	S_ASSERT( sizeof(JSProto_Null) == sizeof(int) );
	for ( int i = int(JSProto_Null)+1; i < int(JSProto_LIMIT); ++i ) {

		if ( !js_GetClassPrototype(cx, global, JSProtoKey(i), &proto) )
			break;
		if ( objProto == proto )
			return JSProtoKey(i);
	}
	return JSProto_Null; // not found;
}


ALWAYS_INLINE JSProtoKey
JL_GetErrorProtoKey( JSContext *cx, JSObject *obj ) {

	JSObject *global = JS_GetGlobalForScopeChain(cx);
	const JSObject *objProto = JS_GetPrototype(cx, obj);
	JSObject *errorProto;
	for ( int i = int(JSProto_Error); i <= int(JSProto_Error + JSEXN_LIMIT); ++i ) {

		if ( !js_GetClassPrototype(cx, global, JSProtoKey(i), &errorProto) )
			break;
		if ( objProto == errorProto )
			return JSProtoKey(i);
	}
	return JSProto_Null; // not found;
}


ALWAYS_INLINE JSBool
JL_CreateErrorException( JSContext *cx, JSExnType exn, JSObject **obj ) {

	JSObject *proto;
	if ( !js_GetClassPrototype(cx, JL_GetGlobalObject(cx), JSProtoKey(JSProto_Error + exn), &proto) || !proto )
		return JS_FALSE;

	*obj = JS_NewObject(cx, JL_GetStandardClassByKey(cx, JSProtoKey(JSProto_Error + exn)), proto, NULL);
	return JS_TRUE;
}

/*
static void
ErrorReporter_ToString(JSContext *cx, const char *message, JSErrorReport *report) {

	JL_IGNORE(cx);
	if ( !report )
		fprintf(stderr, "%s\n", message);
	else
		fprintf(stderr, "%s (%s:%d)\n", message, report->filename, report->lineno);
}

ALWAYS_INLINE JSBool
JL_ReportExceptionToString( JSContext *cx, JSObject *obj, JLStr  ) {
	
	JSErrorReporter prevEr = JS_SetErrorReporter(cx, ErrorReporter_ToString);
	JS_ReportPendingException(cx);
	JS_SetErrorReporter(cx, prevEr);
	return JS_TRUE;
}
*/

ALWAYS_INLINE bool
JL_MaybeRealloc( size_t requested, size_t received ) {

	return requested != 0 && (128 * received / requested < 96) && (requested - received > 128);
}


INLINE NEVER_INLINE JSBool FASTCALL
JL_ThrowOSError(JSContext *cx) {

	char errMsg[1024];
	JLLastSysetmErrorMessage(errMsg, sizeof(errMsg));
	JL_ERR( E_OS, E_DETAILS, E_STR(errMsg) );
	JL_BAD;
}


ALWAYS_INLINE bool
JL_EngineEnding(const JSContext *cx) {

	return JL_GetRuntime(cx)->state == JSRTS_LANDING || JL_GetRuntime(cx)->state == JSRTS_DOWN; // could be replaced by a flag in HostPrivate that keep the state of the engine.
}


ALWAYS_INLINE JSContext*
JL_GetFirstContext(JSRuntime *rt) {

	JSContext *cx = NULL;
	ASSERT( rt != NULL );
	JS_ContextIterator(rt, &cx);
	JS_ASSERT( cx != NULL );
	return cx;
}


ALWAYS_INLINE bool
JL_InheritFrom( JSContext *cx, JSObject *obj, const JSClass *clasp ) {

	JL_IGNORE(cx);
	while ( obj != NULL ) {

		if ( JL_GetClass(obj) == clasp )
			return true;
		obj = JL_GetPrototype(cx, obj);
	}
	return false;
}


ALWAYS_INLINE JSBool
JL_CallFunctionId(JSContext *cx, JSObject *obj, jsid id, uintN argc, jsval *argv, jsval *rval) {

	jsval tmp;
	return JS_GetMethodById(cx, obj, id, NULL, &tmp) && JS_CallFunctionValue(cx, obj, tmp, argc, argv, rval);
// (TBD) choose the best
//	jsval val;
//	return JL_JsidToJsval(cx, id, &val) && JS_CallFunctionValue(cx, obj, val, argc, argv, rval);
}


INLINE JSBool FASTCALL
JL_CallFunctionVA( JSContext * RESTRICT cx, JSObject * RESTRICT obj, const jsval &functionValue, jsval *rval, uintN argc, ... ) {

	va_list ap;
	jsval *argv = (jsval*)alloca((argc+1)*sizeof(jsval));
	va_start(ap, argc);
	for ( uintN i = 1; i <= argc; i++ )
		argv[i] = va_arg(ap, jsval);
	va_end(ap);
	argv[0] = JSVAL_NULL; // the rval
	if ( JL_IS_SAFE )
		JL_CHK( JL_ValueIsFunction(cx, functionValue) );
	JSBool st;
	st = JS_CallFunctionValue(cx, obj, functionValue, argc, argv+1, argv);
	JL_CHK( st );
	if ( rval != NULL )
		*rval = argv[0];
	return JS_TRUE;
	JL_BAD;
}


INLINE JSBool FASTCALL
JL_CallFunctionNameVA( JSContext * RESTRICT cx, JSObject * RESTRICT obj, const char* functionName, jsval *rval, uintN argc, ... ) {

	va_list ap;
	jsval *argv = (jsval*)alloca((argc+1)*sizeof(jsval));
	va_start(ap, argc);
	for ( uintN i = 1; i <= argc; i++ )
		argv[i] = va_arg(ap, jsval);
	va_end(ap);
	argv[0] = JSVAL_NULL; // the rval
	JSBool st = JS_CallFunctionName(cx, obj, functionName, argc, argv+1, argv);
	JL_CHK( st );
	if ( rval != NULL )
		*rval = argv[0];
	return JS_TRUE;
	JL_BAD;
}


INLINE JSBool FASTCALL
JL_Eval( JSContext * RESTRICT cx, JSString * RESTRICT source, jsval *rval ) {

	const char *scriptFilename;
	int scriptLineno;
	JSStackFrame *frame = JS_GetScriptedCaller(cx, NULL);
	if ( frame ) {

		JSScript *script;
		script = JS_GetFrameScript(cx, frame);
		JL_CHK( script );
		scriptFilename = JS_GetScriptFilename(cx, script);
		JL_CHK( scriptFilename );
		scriptLineno = JS_PCToLineNumber(cx, script, JS_GetFramePC(cx, frame));
	} else {
		scriptFilename = "<no_file>";
		scriptLineno = 1;
	}
	size_t length;
	const jschar *chars;
	chars = JS_GetStringCharsAndLength(cx, source, &length);
	JL_CHK( chars );
	return JS_EvaluateUCScript(cx, JS_GetScopeChain(cx), chars, length, scriptFilename, scriptLineno, rval);
	JL_BAD;
}


ALWAYS_INLINE JSBool FASTCALL
JL_Push( JSContext * RESTRICT cx, JSObject * RESTRICT arr, jsval * RESTRICT value ) {

	jsuint length;
	JL_CHK( JS_GetArrayLength(cx, arr, &length) );
	JL_CHK( JL_SetElement(cx, arr, length, value) );
	//JL_CHK( JS_SetArrayLength(cx, arr, length+1) ); // implicitly done.
	return JS_TRUE;
	JL_BAD;
}


ALWAYS_INLINE JSBool FASTCALL
JL_Pop( JSContext * RESTRICT cx, JSObject * RESTRICT arr, jsval * RESTRICT vp ) {

	jsuint length;
	JL_CHK( JS_GetArrayLength(cx, arr, &length) );
	--length;
	JL_CHK( JS_GetPropertyById(cx, arr, INT_TO_JSID(length), vp) ); //JL_CHK( JL_GetElement(cx, arrObj, length, vp) );
	JL_CHK( JS_SetArrayLength(cx, arr, length) ); // pop does reduce the array length
	return JS_TRUE;
	JL_BAD;
}


INLINE JSBool FASTCALL
JL_JsvalToPrimitive( JSContext * RESTRICT cx, const jsval &val, jsval * RESTRICT rval ) { // prev JL_ValueOf

	if ( JSVAL_IS_PRIMITIVE(val) ) {

		*rval = val;
		return JS_TRUE;
	}
	JSObject *obj = JSVAL_TO_OBJECT(val);
	if (unlikely( JL_IsXML(cx, obj) ))
		return JL_CallFunctionId(cx, obj, JLID(cx, toXMLString), 0, NULL, rval);
	//JSClass *clasp = JL_GetClass(obj);
	//if ( clasp->convert ) // note that JS_ConvertStub calls js_TryValueOf
	//	return clasp->convert(cx, obj, JSTYPE_VOID, rval);
	JL_CHK( JL_CallFunctionId(cx, obj, JL_ATOMJSID(cx, valueOf), 0, NULL, rval) );
	if ( !JSVAL_IS_PRIMITIVE(*rval) )
		JL_CHK( JL_CallFunctionId(cx, obj, JL_ATOMJSID(cx, toString), 0, NULL, rval) );
	return JS_TRUE;
	JL_BAD;
}


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
INLINE NEVER_INLINE JSObject* FASTCALL
JL_LoadScript(JSContext * RESTRICT cx, JSObject * RESTRICT obj, const char * RESTRICT fileName, JLEncodingType encoding, bool useCompFile, bool saveCompFile) {

	char *scriptBuffer = NULL;
	size_t scriptFileSize;
	jschar *scriptText = NULL;
	size_t scriptTextLength;

	JSObject *script = NULL;
	void *data = NULL;
	uint32 prevOpts = JS_GetOptions(cx);

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
		size_t compFileSize = compFileStat.st_size; // filelength(file); ?
		data = jl_malloca(compFileSize);
		JL_ASSERT_ALLOC( data );
		int readCount = read( file, data, jl::SafeCast<unsigned int>(compFileSize) ); // here we can use "Memory-Mapped I/O Functions" ( http://developer.mozilla.org/en/docs/NSPR_API_Reference:I/O_Functions#Memory-Mapped_I.2FO_Functions )
		JL_CHKM( readCount >= 0 && (size_t)readCount == compFileSize, E_FILE, E_NAME(compiledFileName), E_READ ); // "Unable to read the file \"%s\" ", compiledFileName
		close( file );

		JSXDRState *xdr = JS_XDRNewMem(cx, JSXDR_DECODE);
		JL_CHK( xdr );
		JS_XDRMemSetData(xdr, data, jl::SafeCast<uint32>(compFileSize));

		// we want silent failures.
//		JSErrorReporter prevErrorReporter = JS_SetErrorReporter(cx, NULL);
//		JSDebugErrorHook debugErrorHook = cx->debugHooks->debugErrorHook;
//		void *debugErrorHookData = cx->debugHooks->debugErrorHookData;
//		JS_SetDebugErrorHook(JL_GetRuntime(cx), NULL, NULL);

		JSBool status = JS_XDRScriptObject(xdr, &script);

//		JS_SetDebugErrorHook(JL_GetRuntime(cx), debugErrorHook, debugErrorHookData);
//		if (cx->lastMessage)
//			JS_free(cx, cx->lastMessage);
//		cx->lastMessage = NULL;
//		JS_SetErrorReporter(cx, prevErrorReporter);

		if ( status == JS_TRUE ) {

//			*script->notes() = 0;
			// (TBD) manage BIG_ENDIAN here ?
			JS_XDRMemSetData(xdr, NULL, 0);
			JS_XDRDestroy(xdr);
			jl_freea(data);
			data = NULL;

			JL_ASSERT_WARN( JS_GetScriptVersion(cx, script->getScript()) >= JS_GetVersion(cx), E_NAME(compiledFileName), E_STR("XDR"), E_VERSION );
			goto good;
		} else {

			JS_ClearPendingException(cx);

//			JL_REPORT_WARNING_NUM( JLSMSG_RUNTIME_ERROR, "bad script XDR magic number");

//			if ( JS_IsExceptionPending(cx) )
//				JS_ReportPendingException(cx);

			jl_freea(data);
			data = NULL;
//			if ( JL_IsExceptionPending(cx) )
//				JS_ClearPendingException(cx);
		}
	}

	if ( !hasSrcFile )
		goto bad; // no source, no compiled version of the source, die.

	if ( saveCompFile )
		JS_SetOptions( cx, JS_GetOptions(cx) & ~JSOPTION_COMPILE_N_GO ); // see https://bugzilla.mozilla.org/show_bug.cgi?id=494363

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

	if ( encoding == ENC_UNKNOWN )
		encoding = JLDetectEncoding((uint8_t**)&scriptBuffer, &scriptFileSize);

	switch ( encoding ) {
		default:
			JL_WARN( E_SCRIPT, E_ENCODING, E_INVALID, E_NAME(fileName) );
			// then use ASCII as default.
		case ENC_ASCII: {

			char *scriptText = scriptBuffer;
			size_t scriptTextLength = scriptFileSize;
			if ( scriptTextLength >= 2 && scriptText[0] == '#' && scriptText[1] == '!' ) { // shebang support

				scriptText[0] = '/';
				scriptText[1] = '/';
			}
			script = JS_CompileScript(cx, obj, scriptText, scriptTextLength, fileName, 1);
			break;
		}
		case ENC_UTF16le: { // (TBD) support big-endian

			jschar *scriptText = (jschar*)scriptBuffer;
			size_t scriptTextLength = scriptFileSize / 2;
			if ( scriptTextLength >= 2 && scriptText[0] == L'#' && scriptText[1] == L'!' ) { // shebang support

				scriptText[0] = L'/';
				scriptText[1] = L'/';
			}
			script = JS_CompileUCScript(cx, obj, scriptText, scriptTextLength, fileName, 1);
			break;
		}
		case ENC_UTF8: { // (TBD) check if JS_DecodeBytes does the right things

			scriptText = (jschar*)jl_malloca(scriptFileSize * 2);
			scriptTextLength = scriptFileSize * 2;
			JL_CHKM( UTF8ToUTF16LE((unsigned char*)scriptText, &scriptTextLength, (unsigned char*)scriptBuffer, &scriptFileSize) >= 0, E_SCRIPT, E_ENCODING, E_INVALID, E_COMMENT("UTF8") ); // "Unable do decode UTF8 data."

			if ( scriptTextLength >= 2 && scriptText[0] == L'#' && scriptText[1] == L'!' ) { // shebang support

				scriptText[0] = L'/';
				scriptText[1] = L'/';
			}
			script = JS_CompileUCScript(cx, obj, scriptText, scriptTextLength, fileName, 1);
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

	JSXDRState *xdr;
	xdr = JS_XDRNewMem(cx, JSXDR_ENCODE);
	JL_CHK( xdr );
	JL_CHK( JS_XDRScriptObject(xdr, &script) );

	uint32 length;
	void *buf;
	buf = JS_XDRMemGetData(xdr, &length);
	JL_CHK( buf );
	// manage BIG_ENDIAN here ?
	JL_CHK( write(file, buf, length) != -1 ); // On error, -1 is returned, and errno is set appropriately.
	JL_CHK( close(file) == 0 );
	JS_XDRDestroy(xdr);
	goto good;

good:

	if ( scriptBuffer )
		jl_freea(scriptBuffer);

	if ( scriptText )
		jl_freea(scriptText);

	JS_SetOptions(cx, prevOpts);
	return script;

bad:

	jl_freea(scriptBuffer); // jl_freea(NULL) is legal
	jl_freea(scriptText);
	JS_SetOptions(cx, prevOpts);
	jl_freea(data); // jl_free(NULL) is legal
	return NULL; // report a warning ?
}



///////////////////////////////////////////////////////////////////////////////
// JS stack related functions

ALWAYS_INLINE JSStackFrame*
JL_CurrentStackFrame(JSContext *cx) {

	#ifdef DEBUG
		JSStackFrame *fp = NULL;
		ASSERT( JS_FrameIterator(cx, &fp) == js::Jsvalify(js_GetTopStackFrame(cx)) ); // Mozilla JS engine private API behavior has changed.
	#endif //DEBUG
	return Jsvalify(js_GetTopStackFrame(cx));
}


ALWAYS_INLINE uint32_t
JL_StackSize(const JSContext * RESTRICT cx, JSStackFrame * RESTRICT fp) {

	JL_IGNORE(cx);
	uint32_t length = 0;
	const js::StackFrame *tmp;
	tmp = js::Valueify(fp);
	for ( ; tmp; tmp = tmp->prev() ) // for ( JSStackFrame *fp = JL_CurrentStackFrame(cx); fp; JS_FrameIterator(cx, &fp) )
		++length;
	return length; // 0 is the first frame
}


INLINE JSStackFrame* FASTCALL
JL_StackFrameByIndex(JSContext *cx, int frameIndex) {

	js::StackFrame *fp = js::Valueify(JL_CurrentStackFrame(cx));
	if ( frameIndex >= 0 ) {

		int currentFrameIndex = JL_StackSize(cx, js::Jsvalify(fp))-1;
		if ( frameIndex > currentFrameIndex )
			return NULL;
		// now, select the right frame
		while ( fp && currentFrameIndex > frameIndex ) {

			fp = fp->prev(); //JS_FrameIterator(cx, &fp);
			--currentFrameIndex;
		}
		return js::Jsvalify(fp);
	}

	while ( fp && frameIndex < 0 ) {

		fp = fp->prev(); //JS_FrameIterator(cx, &fp);
		++frameIndex;
	}
	return js::Jsvalify(fp);
}


INLINE NEVER_INLINE JSBool FASTCALL
JL_DebugPrintScriptLocation( JSContext *cx ) {

	JSStackFrame *fp = NULL;
	do {

		JS_FrameIterator(cx, &fp);
	} while ( fp && !JS_GetFramePC(cx, fp) );

	JL_CHK( fp );
	JSScript *script;
	script = JS_GetFrameScript(cx, fp);
	JL_CHK( script );
	int lineno;
	lineno = JS_PCToLineNumber(cx, script, JS_GetFramePC(cx, fp));
	const char *filename;
	filename = JS_GetScriptFilename(cx, script);
	if ( filename == NULL || *filename == '\0' )
		filename = "<no_filename>";
	printf("%s:%d\n", filename, lineno);\
	return JS_TRUE;
	JL_BAD;
}


INLINE NEVER_INLINE JSBool FASTCALL
JL_ExceptionSetScriptLocation( JSContext * RESTRICT cx, JSObject * RESTRICT obj ) {

	 // see PopulateReportBlame()
    //JSStackFrame *fp;
    //for (fp = js_GetTopStackFrame(cx); fp; fp = fp->down) {
    //    if (fp->regs) {
    //        report->filename = fp->script->filename;
    //        report->lineno = js_FramePCToLineNumber(cx, fp);
    //        break;
    //    }
    //}

	JSStackFrame *fp = NULL;
	do {

		JS_FrameIterator(cx, &fp);
	} while ( fp && !JS_GetFramePC(cx, fp) );

	JL_CHK( fp );
	JSScript *script;
	script = JS_GetFrameScript(cx, fp);
	JL_CHK( script );
	const char *filename;
	int lineno;
	filename = JS_GetScriptFilename(cx, script);
	if ( filename == NULL || *filename == '\0' )
		filename = "<no_filename>";
	lineno = JS_PCToLineNumber(cx, script, JS_GetFramePC(cx, fp));

	jsval tmp;
	JL_CHK( JL_NativeToJsval(cx, filename, &tmp) );
	JL_CHK( JS_SetPropertyById(cx, obj, JL_ATOMJSID(cx, fileName), &tmp) );
	JL_CHK( JL_NativeToJsval(cx, lineno, &tmp) );
	JL_CHK( JS_SetPropertyById(cx, obj, JL_ATOMJSID(cx, lineNumber), &tmp) );

	return JS_TRUE;
	JL_BAD;
}



///////////////////////////////////////////////////////////////////////////////
// NativeInterface API

ALWAYS_INLINE JSBool
ReserveNativeInterface( JSContext *cx, JSObject *obj, const jsid &id ) {

	ASSERT( id != JL_NullJsid() );
	return JS_DefinePropertyById(cx, obj, id, JSVAL_VOID, NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
}


template <class T>
ALWAYS_INLINE JSBool
SetNativeInterface( JSContext *cx, JSObject *obj, const jsid &id, const T nativeFct ) {

	ASSERT( id != JL_NullJsid() );
	if ( nativeFct != NULL ) {

		JL_CHK( JS_DefinePropertyById(cx, obj, id, JSVAL_TRUE, NULL, (JSStrictPropertyOp)nativeFct, JSPROP_READONLY | JSPROP_PERMANENT) ); // hacking the setter of a read-only property seems safe.
	} else {

		JL_CHK( ReserveNativeInterface(cx, obj, id) );
	}
	return JS_TRUE;
	JL_BAD;
}


template <class T>
ALWAYS_INLINE const T
GetNativeInterface( JSContext *cx, JSObject *obj, const jsid &id ) {

	ASSERT( id != JL_NullJsid() );
	JSPropertyDescriptor desc;
	if ( JS_GetPropertyDescriptorById(cx, obj, id, JSRESOLVE_QUALIFIED, &desc) )
		return desc.obj == obj && desc.setter != JS_StrictPropertyStub ? (const T)desc.setter : NULL; // is JS_PropertyStub when eg. Stringify({_NI_BufferGet:function() {} })
	return NULL;
}



///////////////////////////////////////////////////////////////////////////////
// NativeInterface StreamRead

ALWAYS_INLINE JSBool
ReserveStreamReadInterface( JSContext *cx, JSObject *obj ) {

	return ReserveNativeInterface(cx, obj, JLID(cx, _NI_StreamRead) );
}


ALWAYS_INLINE JSBool
SetStreamReadInterface( JSContext *cx, JSObject *obj, NIStreamRead pFct ) {

	return SetNativeInterface( cx, obj, JLID(cx, _NI_StreamRead), pFct );
}


ALWAYS_INLINE NIStreamRead
StreamReadNativeInterface( JSContext *cx, JSObject *obj ) {

	return GetNativeInterface<NIStreamRead>(cx, obj, JLID(cx, _NI_StreamRead));
}


INLINE JSBool
JSStreamRead( JSContext * RESTRICT cx, JSObject * RESTRICT obj, char * RESTRICT buffer, size_t * RESTRICT amount ) {

	jsval tmp;
	JL_CHK( JL_NativeToJsval(cx, *amount, &tmp) );
	JL_CHK( JL_CallFunctionId(cx, obj, JLID(cx, Read), 1, &tmp, &tmp) );
	if ( JSVAL_IS_VOID(tmp) ) { // (TBD)! with sockets, undefined mean 'closed', that is not supported by streams !!

		*amount = 0;
	} else {

		JLStr str;
		JL_CHK( JL_JsvalToNative(cx, tmp, &str) );
		ASSERT( str.Length() <= *amount );
		*amount = str.Length();
		memcpy(buffer, str.GetConstStr(), *amount);
	}
	return JS_TRUE;
	JL_BAD;
}


ALWAYS_INLINE NIStreamRead
StreamReadInterface( JSContext *cx, JSObject *obj ) {

	NIStreamRead fct = StreamReadNativeInterface(cx, obj);
	if (likely( fct != NULL ))
		return fct;
	JSBool found;
	if ( JS_HasPropertyById(cx, obj, JLID(cx, Read), &found) && found ) // JS_GetPropertyById(cx, obj, JLID(cx, Read), &res) != JS_TRUE || !JL_IsFunction(cx, res)
		return JSStreamRead;
	return NULL;
}



///////////////////////////////////////////////////////////////////////////////
// NativeInterface BufferGet

ALWAYS_INLINE JSBool
ReserveBufferGetInterface( JSContext *cx, JSObject *obj ) {

	return ReserveNativeInterface(cx, obj, JLID(cx, _NI_BufferGet) );
}


ALWAYS_INLINE JSBool
SetBufferGetInterface( JSContext *cx, JSObject *obj, NIBufferGet pFct ) {

	return SetNativeInterface( cx, obj, JLID(cx, _NI_BufferGet), pFct );
}


ALWAYS_INLINE NIBufferGet
BufferGetNativeInterface( JSContext *cx, JSObject *obj ) {

	return GetNativeInterface<NIBufferGet>(cx, obj, JLID(cx, _NI_BufferGet));
}


INLINE JSBool
JSBufferGet( JSContext *cx, JSObject *obj, JLStr *str ) {

	jsval tmp;
	return JL_CallFunctionId(cx, obj, JLID(cx, Get), 0, NULL, &tmp) && JL_JsvalToNative(cx, tmp, str);
}


ALWAYS_INLINE NIBufferGet
BufferGetInterface( JSContext *cx, JSObject *obj ) {

	NIBufferGet fct = BufferGetNativeInterface(cx, obj);
	if (likely( fct != NULL ))
		return fct;
	JSBool found;
	if ( JS_HasPropertyById(cx, obj, JLID(cx, Get), &found) && found ) // JS_GetPropertyById(cx, obj, JLID(cx, Get), &res) != JS_TRUE || !JL_IsFunction(cx, res)
		return JSBufferGet;
	return NULL;
}



///////////////////////////////////////////////////////////////////////////////
// NativeInterface Matrix44Get

ALWAYS_INLINE JSBool
ReserveMatrix44GetInterface( JSContext *cx, JSObject *obj ) {

	return ReserveNativeInterface(cx, obj, JLID(cx, _NI_Matrix44Get) );
}


ALWAYS_INLINE JSBool
SetMatrix44GetInterface( JSContext *cx, JSObject *obj, NIMatrix44Get pFct ) {

	return SetNativeInterface( cx, obj, JLID(cx, _NI_Matrix44Get), pFct );
}


ALWAYS_INLINE NIMatrix44Get
Matrix44GetNativeInterface( JSContext *cx, JSObject *obj ) {

	return GetNativeInterface<NIMatrix44Get>(cx, obj, JLID(cx, _NI_Matrix44Get));
}


INLINE JSBool
JSMatrix44Get( JSContext *cx, JSObject *obj, float **m ) {

	JL_IGNORE( cx );
	JL_IGNORE( m );
	JL_IGNORE( obj );
	return JS_FALSE;
}


ALWAYS_INLINE NIMatrix44Get
Matrix44GetInterface( JSContext *cx, JSObject *obj ) {

	NIMatrix44Get fct = Matrix44GetNativeInterface(cx, obj);
	if (likely( fct != NULL ))
		return fct;
	JSBool found;
	if ( JS_HasPropertyById(cx, obj, JLID(cx, GetMatrix44), &found) && found ) // JS_GetPropertyById(cx, obj, JLID(cx, GetMatrix44), &res) != JS_TRUE || !JL_IsFunction(cx, res)
		return JSMatrix44Get;
	return NULL;
}



///////////////////////////////////////////////////////////////////////////////
// ProcessEvent

struct ProcessEvent {
	void (*startWait)( volatile ProcessEvent *self ); // starts the blocking thread and call signalEvent() when an event has arrived.
	bool (*cancelWait)( volatile ProcessEvent *self ); // unlock the blocking thread event if no event has arrived (mean that an event has arrived in another thread).
	JSBool (*endWait)( volatile ProcessEvent *self, bool *hasEvent, JSContext *cx, JSObject *obj ); // process the result
};





///////////////////////////////////////////////////////////////////////////////
// Shared buffer

class SharedBuffer {

	struct Shared {
		size_t count;
		size_t length;
		char buffer[1]; // first char of the buffer
	};
	Shared *_shared;

	void AddRef() {

		++_shared->count;
	}

	void DelRef() {

		if ( !--_shared->count )
			jl_free(_shared);
	}

public:
	~SharedBuffer() {

		DelRef();
	}

	SharedBuffer( size_t length ) {

		_shared = (Shared*)jl_malloc(sizeof(*_shared)-1 + length);
//		ASSERT( _shared );
		_shared->count = 1;
		_shared->length = length;
	}

	SharedBuffer( const SharedBuffer &other ) {

		_shared = other._shared;
		AddRef();
	}

	const SharedBuffer & operator =( const SharedBuffer &other ) {

		DelRef();
		_shared = other._shared;
		AddRef();
		return *this;
	}

	size_t Length() const {

		return _shared->length;
	}

	char *Data() const {

		return _shared->buffer;
	}

private:
	SharedBuffer();
};



///////////////////////////////////////////////////////////////////////////////
// Recycle bin


/*
// Franck, this is hopeless. The JS engine is not going to keep around apparently-dead variables on the off chance that
// someone might call a native function that uses the debugger APIs to read them off the stack. The debugger APIs don't work that way.
// ...
// -j

//ALWAYS_INLINE jsid StringToJsid( JSContext *cx, const char *cstr );
// Get the value of a variable in the current or parent's scopes.
ALWAYS_INLINE JSBool JL_GetVariableValue( JSContext *cx, const char *name, jsval *vp ) {

//	JSStackFrame *fp = JS_GetScriptedCaller(cx, NULL);
//	return JS_EvaluateInStackFrame(cx, fp, name, strlen(name), "", 0, vp);

	JSBool found;
	JSStackFrame *fp = JS_GetScriptedCaller(cx, NULL);

	for ( JSObject *scope = JS_GetFrameScopeChain(cx, fp); scope; scope = scope->getParent() ) {

		JL_CHK( JS_HasProperty(cx, scope, name, &found) );
		if ( found ) {

			JL_CHK( JS_GetProperty(cx, scope, name, vp) );

//			JS_LookupProperty(cx, scope, name, vp);
//			uintN attrs;
//			JS_GetPropertyAttributes(cx, scope, name, &attrs, &found);

//			JSPropertyDescriptor desc;
//			JS_GetPropertyDescriptorById(cx, scope, StringToJsid(cx, name), 0, &desc);
//			*vp = desc.value;

			return JS_TRUE;
		}
	}
	*vp = JSVAL_VOID;

	return JS_TRUE;
	JL_BAD;
}
*/



///////////////////////////////////////////////////////////////////////////////
// Serialization


/*
typedef JSXDRState* Serialized;

ALWAYS_INLINE bool JL_IsSerializable( jsval val ) {

	if ( JSVAL_IS_PRIMITIVE(val) )
		return true;
	JSClass *cl = JL_GetClass(JSVAL_TO_OBJECT(val));
	return cl->xdrObject != NULL;
}

ALWAYS_INLINE void JL_SerializerCreate( Serialized *xdr ) {

	*xdr = NULL;
}

ALWAYS_INLINE void JL_SerializerFree( Serialized *xdr ) {

	if ( *xdr != NULL ) {

		JS_XDRDestroy(*xdr);
//		JS_XDRMemSetData(*xdr, NULL, 0);
		*xdr = NULL;
	}
}

ALWAYS_INLINE bool JL_SerializerIsEmpty( const Serialized *xdr ) {

	return *xdr == NULL;
}

ALWAYS_INLINE JSBool JL_SerializeJsval( JSContext *cx, Serialized *xdr, jsval *val ) {

	if ( *xdr != NULL )
		SerializerFree(xdr);
	*xdr = JS_XDRNewMem(cx, JSXDR_ENCODE);
	JL_ASSERT( *xdr != NULL, "Unable to create the serializer." );
	JL_CHK( JS_XDRValue(*xdr, val) );
	return JS_TRUE;
	JL_BAD;
}

ALWAYS_INLINE JSBool JL_UnserializeJsval( JSContext *cx, const Serialized *xdr, jsval *rval ) {

	JSXDRState *xdrDecoder = JS_XDRNewMem(cx, JSXDR_DECODE);
	JL_ASSERT( xdrDecoder != NULL, "Unable to create the unserializer." );
	uint32 length;
	void *data;
	data = JS_XDRMemGetData(*xdr, &length);
	JS_XDRMemSetData(xdrDecoder, data, length);
	JL_CHK( JS_XDRValue(xdrDecoder, rval) );
	JS_XDRMemSetData(xdrDecoder, NULL, 0);
	JS_XDRDestroy(xdrDecoder);
	return JS_TRUE;
	JL_BAD;
}
*/


// serializer 2

/*
class JLSerializer {
private:

	class ISerializer {
		virtual bool Process( jsval *val );
	};


	class Serializer : public ISerializer {
		bool Process( jsval *val ) {
		}
	};

	class UnSerializer : public ISerializer {
		bool Process( jsval *val ) {
		}
	};

	ISerializer *_serializer;

public:
	JLSerializer( bool serialize ) {

		_serializer = serialize ? new Serializer() : new UnSerializer();
	}

	bool Process( jsval *val ) {

		_serializer->Process( jsval *val );
	}
};
*/

//#include <deque>

/*
class JLAllocator {
  void *operator new(size_t size);
  void operator delete(void *p);
  void *operator new[](size_t size);
  void operator delete[](void *p);
}
*/
/*
class JLSerializationBuffer {

	bool _serialize;

	uint8_t *_start;
	uint8_t *_pos;
	size_t _length;

public:
	JLSerializationBuffer() : _serialize(true) {

		_length = 4096;
		_start = (uint8_t*)jl_malloc(_length);
		ASSERT(_start);
		_pos = _start;
	}

	JLSerializationBuffer( uint8_t *data, size_t length ) : _serialize(false) {

		_length = 4096;
		_start = data;
		_pos = _start;
	}

	bool ReserveBytes( size_t length ) {

		size_t offset = _pos - _start;
		if ( offset + length > _length ) {

			_length *= 2;
			_start = (uint8_t*)jl_realloc(_start, _length);
			ASSERT(_start);
			_pos = _start + offset;
		}
	}

	bool Process( uint32_t &value ) {

		if ( _serialize ) {

			ReserveBytes(sizeof(uint32_t));
			*(uint32_t*)_pos = value;
		} else {

			value = *(uint32_t*)_pos;
		}
		_pos += sizeof(uint32_t);
		return true;
	}
};


JSBool JLSerialize( JSContext *cx, jsval *val ) {

	if ( JSVAL_IS_OBJECT(*val) ) {

		JSObject *obj = JSVAL_TO_OBJECT(*val);

		jsval fctVal;
		JL_CHK( obj->getProperty(cx, JLID(cx, _Serialize), &fctVal) );
		if ( JL_IsFunction(cx, fctVal) ) {

		}
	}
	return JS_TRUE;
	JL_BAD;
}
*/
