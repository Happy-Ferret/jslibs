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

#ifndef _JSHELPER_H_
#define _JSHELPER_H_

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

#ifdef _MSC_VER
#pragma warning( pop )
#endif // _MSC_VER

#include <sys/stat.h> // see JL_LoadScript()

extern bool _unsafeMode;


///////////////////////////////////////////////////////////////////////////////
// helper macros to avoid a function call to the jsapi


static ALWAYS_INLINE JSRuntime*
JL_GetRuntime(JSContext *cx) {

	return cx->runtime;
}

static ALWAYS_INLINE void*
JL_GetRuntimePrivate( JSRuntime *rt ) {
    
	return rt->data;
}

static ALWAYS_INLINE JSObject *
JL_GetGlobalObject( JSContext *cx ) {

    return cx->globalObject;
}

static ALWAYS_INLINE JSBool
JL_IsExceptionPending( JSContext *cx ) {
	
	return (JSBool) cx->throwing;
}

static ALWAYS_INLINE JSBool
JL_NewNumberValue( JSContext *cx, jsdouble d, jsval *rval ) {
    
	JL_UNUSED(cx);
	d = JS_CANONICALIZE_NAN(d);
	js::Valueify(rval)->setNumber(d);
	return JS_TRUE;
}

static ALWAYS_INLINE jsval
JL_GetNaNValue( JSContext *cx ) {
    
	return Jsvalify(cx->runtime->NaNValue);
}

static ALWAYS_INLINE JSClass*
JL_GetClass( const JSObject *obj ) {

	return obj->getJSClass();
}

static ALWAYS_INLINE size_t
JL_GetStringLength( const JSString *jsstr ) {

	return jsstr->length();
}

static ALWAYS_INLINE jsval
JL_GetEmptyStringValue( const JSContext *cx ) { // see JS_GetEmptyStringValue()

	return STRING_TO_JSVAL(cx->runtime->emptyString);
}

static ALWAYS_INLINE void*
JL_GetPrivate( const JSContext *cx, const JSObject *obj ) {

	JL_UNUSED(cx);
	return obj->getPrivate();
}

static ALWAYS_INLINE void
JL_SetPrivate( const JSContext *cx, JSObject *obj, void *data ) {

	JL_UNUSED(cx);
	obj->setPrivate(data);
}

static ALWAYS_INLINE JSBool
JL_GetReservedSlot( JSContext *cx, JSObject *obj, uint32 slot, jsval *vp ) {

	// return JS_GetReservedSlot(cx, obj, slot, vp);
	
	#ifndef DEBUG
	JL_UNUSED(cx);
	#endif // DEBUG

	#ifdef DEBUG
	JS_TypeOfValue(cx, *vp); // used for assertSameCompartment(cx, v)
	#endif // DEBUG

	if (obj->isNative() && slot < obj->numSlots())
		*vp = js::Jsvalify(obj->getSlot(slot));
	else
		js::Valueify(vp)->setUndefined();

	#ifdef DEBUG
	jsval tmp;
	JS_GetReservedSlot(cx, obj, slot, &tmp);
	JS_ASSERT( JS_SameValue(cx, *vp, tmp) );
	#endif // DEBUG

	return JS_TRUE;
}

static ALWAYS_INLINE JSBool
JL_SetReservedSlot(JSContext *cx, JSObject *obj, uintN slot, const jsval &v) {

//	return JS_SetReservedSlot(cx, obj, slot, v);

	#ifdef DEBUG
	JS_TypeOfValue(cx, v); // used for assertSameCompartment(cx, v)
	#endif // DEBUG

//	return JS_SetReservedSlot(cx, obj, index, v);
    if (!obj->isNative())
        return JS_TRUE;
	if ( slot >= obj->numSlots() )
		return JS_SetReservedSlot(cx, obj, slot, v);
	obj->setSlot(slot, js::Valueify(v));
    return JS_TRUE;
}

////
// redefining these two function allow us to get ride of which allocator (jl/js) should be used.
static ALWAYS_INLINE JSString *
JL_NewString(JSContext *cx, char *bytes, size_t length) {

// At the moment, the following code is not necessary because jshost use the same allocators as spidermonkey (see JS_HAS_JSLIBS_RegisterCustomAllocators in jshost.cpp and jsutil.h)
//
//#ifndef JS_HAS_JSLIBS_RegisterCustomAllocators
//
//	void *tmp = JS_malloc(cx, length);
//	if ( !tmp )
//		return NULL;
//	memcpy(tmp, bytes, length * sizeof(*bytes));
//	jl_free(bytes);
//	bytes = (char*)tmp;
//
//#endif //

	return JS_NewString(cx, bytes, length);
}


static ALWAYS_INLINE JSString *
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
//#endif //

	return JS_NewUCString(cx, chars, length);
}


///////////////////////////////////////////////////////////////////////////////
// Safe Mode tools

#define JL_IS_SAFE (!_unsafeMode)

#define JL_SAFE_BEGIN if (unlikely( JL_IS_SAFE )) {
#define JL_SAFE_END }

#define JL_UNSAFE_BEGIN if (likely( !JL_IS_SAFE )) {
#define JL_UNSAFE_END }

#define JL_SAFE(code) \
	JL_MACRO_BEGIN \
		if (unlikely( JL_IS_SAFE )) {code;} \
	JL_MACRO_END

#define JL_UNSAFE(code) \
	JL_MACRO_BEGIN \
		if (likely( !JL_IS_SAFE )) {code;} \
	JL_MACRO_END

// see JSAtomState struct in jsatom.h
#define JL_ATOMJSID(CX, NAME) \
	ATOM_TO_JSID((CX)->runtime->atomState.NAME##Atom)


///////////////////////////////////////////////////////////////////////////////
// helper macros

// BEWARE: the following helper macros are only valid inside a JS Native/FastNative function definition !

#define JL_BAD bad:return(JS_FALSE)

#define JL_ARGC (argc)

// returns the ARGument Vector
#define JL_ARGV (JS_ARGV(cx,vp))

// returns the ARGument n
#define JL_ARG( n ) (JS_ARGV(cx,vp)[(n)-1])

// returns the ARGument n or undefined if it does not exist
#define JL_SARG( n ) ( argc >= (n) ? JS_ARGV(cx,vp)[(n)-1] : JSVAL_VOID )

// returns true if the ARGument n is DEFined
#define JL_ARG_ISDEF( n ) ( argc >= (n) && !JSVAL_IS_VOID(JS_ARGV(cx,vp)[(n)-1]) )

// is the current obj (this)
//#define JL_OBJ (argc=argc, JS_THIS_OBJECT(cx, vp))
#define JL_OBJ (obj)

// is the current obj (this) as a jsval
#define JL_OBJVAL (argc=argc, JS_THIS(cx, vp))

// the return value
#define JL_RVAL (&JS_RVAL(cx, vp))

// define obj variable for native functions
#define JL_DEFINE_FUNCTION_OBJ \
	JSObject *obj = JS_THIS_OBJECT(cx, vp); \
	JL_ASSERT( obj );

#define JL_DEFINE_CALL_FUNCTION_OBJ \
	JSObject *obj = JSVAL_TO_OBJECT(JS_CALLEE(cx, vp))

// initialise 'this' object (obj) for constructors
// see. JS_NewObjectForConstructor(cx, vp) if JL_THIS_CLASS or JL_THIS_PROTOTYPE cannot be used
#define JL_DEFINE_CONSTRUCTOR_OBJ \
	JSObject *obj; \
	{ \
	if ( !((JL_THIS_CLASS->flags & JSCLASS_CONSTRUCT_PROTOTYPE) && JS_IsConstructing_PossiblyWithGivenThisObject(cx, vp, &obj) && obj) ) { \
		obj = JS_NewObjectWithGivenProto(cx, JL_THIS_CLASS, JL_THIS_PROTOTYPE, NULL); \
		if ( obj == NULL ) \
			return JS_FALSE; \
	} \
	JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(obj)); \
	if ( js::Valueify(JL_THIS_CLASS)->ext.equality ) \
		obj->flags |= JSObject::HAS_EQUALITY; \
	}


///////////////////////////////////////////////////////////////////////////////
// IDs cache

// required by jlhostprivate.h
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
	JLID_SPEC( name ),
	JLID_SPEC( id ),
	JLID_SPEC( push ),
	JLID_SPEC( pop ),
	JLID_SPEC( _serialize ),
	JLID_SPEC( _unserialize ),
	JLID_SPEC( toXMLString ),
	LAST_JSID // see HostPrivate::ids[]
};
#undef JLID_SPEC


///////////////////////////////////////////////////////////////////////////////
// Context private

struct JLContextPrivate {
};

static ALWAYS_INLINE JLContextPrivate*
JL_GetContextPrivate( const JSContext *cx ) {

	JL_ASSERT( JS_GetContextPrivate((JSContext*)cx) == cx->data );
	return reinterpret_cast<JLContextPrivate*>(cx->data);
}

static ALWAYS_INLINE void
JL_SetContextPrivate( const JSContext *cx, JLContextPrivate *ContextPrivate ) {

	JL_ASSERT( JS_GetContextPrivate((JSContext*)cx) == cx->data );
	cx->runtime->data = reinterpret_cast<void*>(ContextPrivate);
}


///////////////////////////////////////////////////////////////////////////////
// Host private

// Using a separate file allow a better versioning of the HostPrivate structure (see JL_HOST_PRIVATE_VERSION).
#include <jlhostprivate.h>

static ALWAYS_INLINE HostPrivate*
JL_GetHostPrivate( const JSContext *cx ) {

//	return (HostPrivate*)JL_GetRuntimePrivate(JL_GetRuntime(cx));
//	return reinterpret_cast<HostPrivate*>(cx->runtime->data);
	JL_ASSERT( JL_GetRuntimePrivate(JL_GetRuntime(const_cast<JSContext*>(cx))) == cx->runtime->data );
	return (HostPrivate*)cx->runtime->data;
}

static ALWAYS_INLINE void
JL_SetHostPrivate( const JSContext *cx, HostPrivate *hostPrivate ) {

//	JS_SetRuntimePrivate(JL_GetRuntime(cx), hostPrivate);
	cx->runtime->data = (void*)hostPrivate;
}


///////////////////////////////////////////////////////////////////////////////
// Module private

static ALWAYS_INLINE uint8_t
JL_ModulePrivateHash( const uint32_t moduleId ) {

//	return ((uint8_t*)&moduleId)[0] ^ ((uint8_t*)&moduleId)[1] ^ ((uint8_t*)&moduleId)[2] ^ ((uint8_t*)&moduleId)[3] << 1;
	uint32_t a = moduleId ^ moduleId >> 16;
	return (a ^ a >> 8) & 0xFF;
}

static ALWAYS_INLINE bool
JL_SetModulePrivate( const JSContext *cx, const uint32_t moduleId, void *modulePrivate ) {

	JL_ASSERT( moduleId != 0 );
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

static ALWAYS_INLINE void*
JL_GetModulePrivate( const JSContext *cx, uint32_t moduleId ) {

	JL_ASSERT( moduleId != 0 );
	uint8_t id = JL_ModulePrivateHash(moduleId);
	HostPrivate::ModulePrivate *mpv = JL_GetHostPrivate(cx)->modulePrivate;
	while ( mpv[id].moduleId != moduleId ) {

		++id; // uses unsigned char overflow
	}
	return mpv[id].privateData;
}
// example of use: static uint32_t moduleId = 'dbug'; SetModulePrivate(cx, moduleId, mpv);



///////////////////////////////////////////////////////////////////////////////
// cached class and proto

static ALWAYS_INLINE uint32_t
JL_ClassNameToClassProtoCacheSlot( const char *n ) {

	JL_ASSERT( n != NULL && strlen(n) <= 24 );
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


static INLINE bool
JL_CacheClassProto( HostPrivate *hpv, const char *className, JSClass *clasp, JSObject *proto ) {

	size_t slotIndex = JL_ClassNameToClassProtoCacheSlot(className);
	size_t first = slotIndex;
	JL_ASSERT( slotIndex < COUNTOF(hpv->classProtoCache) );

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

static ALWAYS_INLINE ClassProtoCache*
JL_GetCachedClassProto( HostPrivate *hpv, const char *className ) {

	size_t slotIndex = JL_ClassNameToClassProtoCacheSlot(className);
	size_t first = slotIndex;
	JL_ASSERT( slotIndex < COUNTOF(hpv->classProtoCache) );

	for (;;) {

		ClassProtoCache *slot = &hpv->classProtoCache[slotIndex];

		if ( slot->clasp == NULL || strcmp(slot->clasp->name, className) == 0 ) // since classes cannot be removed from jslibs, NULL mean "not found"
			return slot;

		slotIndex = (slotIndex + 1) % COUNTOF(hpv->classProtoCache);

		if ( slotIndex == first ) // not found
			return NULL;
	}
}


static ALWAYS_INLINE JSObject*
JL_NewJslibsObject( JSContext *cx, const char *className ) {

	ClassProtoCache *cpc = JL_GetCachedClassProto(JL_GetHostPrivate(cx), className);
	return JS_NewObjectWithGivenProto(cx, cpc->clasp, cpc->proto, NULL);
}



///////////////////////////////////////////////////////////////////////////////
// IDs cache management


static ALWAYS_INLINE jsid
JL_NullJsid() { // is (double)0

	jsid tmp = { 0 };
	return tmp;
}


static ALWAYS_INLINE jsid
JL_GetPrivateJsid( JSContext *cx, int index, const char *name ) {

	jsid id = JL_GetHostPrivate(cx)->ids[index];
	if (likely( id != JL_NullJsid() ))
		return id;
	JSString *jsstr = JS_InternString(cx, name);
	if (unlikely( jsstr == NULL ))
		return JL_NullJsid();
	if (unlikely( JS_ValueToId(cx, STRING_TO_JSVAL(jsstr), &id) != JS_TRUE ))
		return JL_NullJsid();
	JL_GetHostPrivate(cx)->ids[index] = id;
	return id;
}


#ifdef DEBUG
#define JLID_NAME(cx, name) (JL_UNUSED(cx), JL_UNUSED(JLID_##name), #name)
#else
#define JLID_NAME(cx, name) (#name)
#endif // DEBUG


#define JLID(cx, name) JL_GetPrivateJsid(cx, JLID_##name, #name)
// example of use: jsid cfg = JLID(cx, _host); char *name = JLID_NAME(_host);



///////////////////////////////////////////////////////////////////////////////
// Error management

enum JLErrNum {
#define MSG_DEF(name, number, count, exception, format) \
	name = number,
#include "jlerrors.msg"
#undef MSG_DEF
	JLErrLimit
};

// Reports a fatal errors, script must stop as soon as possible.
#define JL_REPORT_ERROR( errorMessage, ... ) \
	JL_MACRO_BEGIN \
		JS_ReportError( cx, (errorMessage IFDEBUG(" (@" JL_CODE_LOCATION ")")), ##__VA_ARGS__ ); \
		goto bad; \
	JL_MACRO_END


// Report a jslibs error. see jlerrors.msg
#define JL_REPORT_ERROR_NUM( cx, num, ... ) \
	JL_MACRO_BEGIN \
		HostPrivate *hpv = JL_GetHostPrivate(cx); \
		if ( hpv != NULL && hpv->errorCallback != NULL ) \
			JS_ReportErrorNumber(cx, hpv->errorCallback, NULL, (num), ##__VA_ARGS__); \
		else \
			JS_ReportError(cx, "undefined message %d", (num)); \
		goto bad; \
	JL_MACRO_END


// Reports warnings (optimisation: check non-unsafeMode. see ErrorReporter() in host.cpp).
#define JL_REPORT_WARNING( errorMessage, ... ) \
	JL_MACRO_BEGIN \
		if (unlikely( !_unsafeMode )) \
			if ( !JS_ReportWarning(cx, (errorMessage IFDEBUG(" (@" JL_CODE_LOCATION ")")), ##__VA_ARGS__) ) { \
				goto bad; \
			} \
	JL_MACRO_END


#define JL_REPORT_WARNING_NUM( cx, num, ... ) \
	JL_MACRO_BEGIN \
		if (unlikely( !_unsafeMode )) { \
			HostPrivate *hpv = JL_GetHostPrivate(cx); \
			if ( hpv != NULL && hpv->errorCallback != NULL ) { \
				if ( !JS_ReportErrorFlagsAndNumber(cx, JSREPORT_WARNING, hpv->errorCallback, NULL, (num), ##__VA_ARGS__) ) \
					goto bad; \
			} else { \
				if ( !JS_ReportWarning(cx, "undefined message %d", (num)) ) \
					goto bad; \
			} \
		} \
	JL_MACRO_END


// check: used to forward an error. // (TBD) try ultra-safe mode at compile-time: #define JL_CHK( status ) (status)
#define JL_CHK( status ) \
	JL_MACRO_BEGIN \
		if (unlikely( !(status) )) { \
			goto bad; \
		} \
	JL_MACRO_END


// check with message: if status is false, a js exception is rised if it is not already pending.
// (Support for variadic macros was introduced in Visual C++ 2005)
#define JL_CHKM( status, errorMessage, ... ) \
	JL_MACRO_BEGIN \
		if (unlikely( !(status) )) { \
			if ( !JL_IsExceptionPending(cx) ) \
				JS_ReportError(cx, (errorMessage IFDEBUG(" (@" JL_CODE_LOCATION ")")), ##__VA_ARGS__); \
			goto bad; \
		} \
	JL_MACRO_END


// check and branch to a errorLabel label on error.
#define JL_CHKB( status, errorLabel ) \
	JL_MACRO_BEGIN \
		if (unlikely( !(status) )) { \
	/*	IFDEBUG( fprintf(stderr, "DEBUG: JL_CHKB(,%s) failed" IFDEBUG(" (@" JL_CODE_LOCATION ")") "\n", #errorLabel ) ); */ \
			goto errorLabel; \
		} \
	JL_MACRO_END


// check and branch to a errorLabel label on error AND report an error if no exception is pending.
#define JL_CHKBM( status, errorLabel, errorMessage, ... ) \
	JL_MACRO_BEGIN \
		if (unlikely( !(status) )) { \
			if ( !JL_IsExceptionPending(cx) ) \
				JS_ReportError(cx, (errorMessage IFDEBUG(" (@" JL_CODE_LOCATION ")")), ##__VA_ARGS__); \
			goto errorLabel; \
		} \
	JL_MACRO_END


// JL_S_ stands for (J)s(L)ibs _ (S)afemode _ and mean that these macros will only be meaningful when _unsafeMode is false. (see jslibs unsafemode).

#define JL_S_ASSERT( condition, errorMessage, ... ) \
	JL_MACRO_BEGIN \
		JL_SAFE_BEGIN \
			if (unlikely( !(condition) )) { \
				JS_ReportError( cx, errorMessage IFDEBUG(" (" #condition " @" JL_CODE_LOCATION ")"), ##__VA_ARGS__ ); \
				goto bad; \
			} \
		JL_SAFE_END \
	JL_MACRO_END


#define JL_S_ASSERT_ERROR_NUM( condition, errorNum, ... ) \
	JL_MACRO_BEGIN \
		JL_SAFE_BEGIN \
			if (unlikely( !(condition) )) \
				JL_REPORT_ERROR_NUM( cx, (errorNum),  ##__VA_ARGS__ ); \
		JL_SAFE_END \
	JL_MACRO_END


#define JL_S_ASSERT_ALLOC(pointer) \
	JL_MACRO_BEGIN \
		JL_SAFE_BEGIN \
			if ( (pointer) == NULL ) { \
				JS_ReportOutOfMemory(cx); \
				goto bad; \
			} \
		JL_SAFE_END \
	JL_MACRO_END


#define JL_S_ASSERT_ARG_MIN(minCount) \
	JL_S_ASSERT_ERROR_NUM( (argc) >= (minCount), JLSMSG_TOO_FEW_ARGUMENTS, #minCount );


#define JL_S_ASSERT_ARG_MAX(maxCount) \
	JL_S_ASSERT_ERROR_NUM( (argc) <= (maxCount), JLSMSG_TOO_MANY_ARGUMENTS, #maxCount );


#define JL_S_ASSERT_ARG_RANGE(minCount, maxCount) \
	JL_MACRO_BEGIN \
		JL_S_ASSERT_ARG_MIN(minCount); \
		JL_S_ASSERT_ARG_MAX(maxCount); \
	JL_MACRO_END


#define JL_S_ASSERT_ARG(count) \
	JL_S_ASSERT_ARG_RANGE(count, count);


#define JL_S_ASSERT_DEFINED(value) \
	JL_S_ASSERT_ERROR_NUM( !JSVAL_IS_VOID(value), JLSMSG_NEED_DEFINED );


// jsType: JSTYPE_VOID, JSTYPE_OBJECT, JSTYPE_FUNCTION, JSTYPE_STRING, JSTYPE_NUMBER, JSTYPE_BOOLEAN, JSTYPE_NULL, JSTYPE_XML, JSTYPE_LIMIT
#define JL_S_ASSERT_TYPE(value, jsType) \
	JL_S_ASSERT_ERROR_NUM( JS_TypeOfValue(cx, (value)) == (jsType), JLSMSG_EXPECT_TYPE, (#jsType)+7 );


#define JS_S_ASSERT_CONVERT(condition, typeName) \
	JL_S_ASSERT_ERROR_NUM( (condition), JLSMSG_FAIL_TO_CONVERT_TO, typeName );


#define JL_S_ASSERT_BOOLEAN(value) \
	JL_S_ASSERT_ERROR_NUM( JSVAL_IS_BOOLEAN(value) || (!JSVAL_IS_PRIMITIVE(value) && JL_GetClass(JSVAL_TO_OBJECT(value)) == JL_GetStandardClassByKey(cx, JSProto_Boolean)), JLSMSG_EXPECT_TYPE, "boolean" );


#define JL_S_ASSERT_LOSSLESS_INT(value) \
	JL_S_ASSERT_ERROR_NUM( JSVAL_IS_INT(value) || (JSVAL_IS_DOUBLE(value) && JSVAL_TO_DOUBLE(value) < MAX_INT_TO_DOUBLE && JSVAL_TO_DOUBLE(value) > -MAX_INT_TO_DOUBLE), JLSMSG_EXPECT_TYPE, "smaller integer" );


#define JL_S_ASSERT_NUMBER(value) \
	JL_S_ASSERT_ERROR_NUM( JSVAL_IS_NUMBER(value) || (!JSVAL_IS_PRIMITIVE(value) && JL_GetClass(JSVAL_TO_OBJECT(value)) == JL_GetStandardClassByKey(cx, JSProto_Number)), JLSMSG_EXPECT_TYPE, "number" );


#define JL_S_ASSERT_INT(value) \
	JL_S_ASSERT_ERROR_NUM( JSVAL_IS_INT(value), JLSMSG_EXPECT_TYPE, "integer" );


#define JL_S_ASSERT_STRING(value) \
	JL_S_ASSERT_ERROR_NUM( JL_JsvalIsData(cx, (value)), JLSMSG_EXPECT_TYPE, "string or blob" );


#define JL_S_ASSERT_OBJECT(value) \
	JL_S_ASSERT_ERROR_NUM( !JSVAL_IS_PRIMITIVE(value), JLSMSG_EXPECT_TYPE, "object" );


#define JL_S_ASSERT_OBJECT_OR_NULL(value) \
	JL_S_ASSERT_ERROR_NUM( !JSVAL_IS_OBJECT(value), JLSMSG_EXPECT_TYPE, "object" );


#define JL_S_ASSERT_ARRAY(value) \
	JL_S_ASSERT_ERROR_NUM( JL_JsvalIsArray(cx, (value)), JLSMSG_EXPECT_TYPE, "array" );


#define JL_S_ASSERT_FUNCTION(value) \
	JL_S_ASSERT_ERROR_NUM( JL_JsvalIsFunction(cx, (value)), JLSMSG_EXPECT_TYPE, "function" );


#define JL_S_ASSERT_CLASS(jsObject, jsClass) \
	JL_S_ASSERT_ERROR_NUM( (jsObject) != NULL && JL_GetClass(jsObject) == (jsClass), JLSMSG_EXPECT_TYPE, (jsClass)->name );


#define JL_S_ASSERT_THIS_CLASS() \
	JL_S_ASSERT_CLASS(obj, JL_THIS_CLASS)


#define JL_S_ASSERT_INHERITANCE(jsObject, jsClass) \
	JL_S_ASSERT_ERROR_NUM( JL_InheritFrom(cx, (jsObject), (jsClass)), JLSMSG_INVALID_INHERITANCE, (jsClass)->name );


#define JL_S_ASSERT_THIS_INSTANCE() \
	JL_S_ASSERT_ERROR_NUM( JL_InheritFrom(cx, (obj), JL_THIS_CLASS) && (obj) != JL_THIS_PROTOTYPE, JLSMSG_INVALID_INHERITANCE, JL_THIS_CLASS->name );


#define JL_S_ASSERT_CONSTRUCTING() \
	JL_S_ASSERT_ERROR_NUM( JS_IsConstructing(cx, vp), JLSMSG_NEED_CONSTRUCT );


#define JL_S_ASSERT_RESOURCE(resourcePointer) \
	JL_S_ASSERT_ERROR_NUM( (resourcePointer) != NULL, JLSMSG_INVALID_RESOURCE );


#define JL_S_ASSERT_VALID(condition, name) \
	JL_S_ASSERT_ERROR_NUM( condition, JLSMSG_INVALIDATED_OBJECT, name );



///////////////////////////////////////////////////////////////////////////////
// Native Interface

class JLStr;

typedef JSBool (*NIStreamRead)( JSContext *cx, JSObject *obj, char *buffer, size_t *amount );
typedef JSBool (*NIBufferGet)( JSContext *cx, JSObject *obj, JLStr *str );
typedef JSBool (*NIMatrix44Get)( JSContext *cx, JSObject *obj, float **pm );

inline NIBufferGet BufferGetNativeInterface( JSContext *cx, JSObject *obj );
inline NIBufferGet BufferGetInterface( JSContext *cx, JSObject *obj );



///////////////////////////////////////////////////////////////////////////////
// Type check functions

static ALWAYS_INLINE JSClass*
JL_ObjectJSClass( JSContext *cx ) {

	JSObject *oproto;
	if ( js_GetClassPrototype(cx, NULL, JSProto_Object, &oproto) )
		return JL_GetClass(oproto);
	return NULL;
}

static ALWAYS_INLINE bool
JL_ObjectIsObject( JSContext *cx, JSObject *obj ) {

	JSObject *oproto;
	return js_GetClassPrototype(cx, NULL, JSProto_Object, &oproto) && obj->getProto() == oproto;
}

#if JS_HAS_XML_SUPPORT
extern JS_FRIEND_DATA(js::Class) js_XMLClass;
#endif // JS_HAS_XML_SUPPORT

static ALWAYS_INLINE bool
JL_ObjectIsXML( JSContext *cx, const JSObject *obj ) {

	JL_UNUSED(cx);
#if JS_HAS_XML_SUPPORT
	return JL_GetClass(obj) == js::Jsvalify(&js_XMLClass);
#else
	return false
#endif // JS_HAS_XML_SUPPORT
}


static ALWAYS_INLINE bool
JL_JsvalIsNaN( const JSContext *cx, const jsval &val ) {

	return js::Valueify(val) == cx->runtime->NaNValue;
}


static ALWAYS_INLINE bool
JL_JsvalIsPInfinity( const JSContext *cx, const jsval &val ) {

	return js::Valueify(val) == cx->runtime->positiveInfinityValue;
}


static ALWAYS_INLINE bool
JL_JsvalIsNInfinity( const JSContext *cx, const jsval &val ) {

	return js::Valueify(val) == cx->runtime->negativeInfinityValue;
}


static ALWAYS_INLINE bool
JL_JsvalIsReal( JSContext *cx, const jsval &val ) {

	JL_UNUSED(cx);
	return JSVAL_IS_INT(val)
	    || ( JSVAL_IS_DOUBLE(val) && JSVAL_TO_DOUBLE(val) > -MAX_INT_TO_DOUBLE && JSVAL_TO_DOUBLE(val) < MAX_INT_TO_DOUBLE );
}


static ALWAYS_INLINE bool
JL_JsvalIsNegative( JSContext *cx, const jsval &val ) {

	return ( JSVAL_IS_INT(val) && JSVAL_TO_INT(val) < 0 )
	    || ( JSVAL_IS_DOUBLE(val) && DOUBLE_IS_NEG(JSVAL_TO_DOUBLE(val)) ) // js::Valueify(val).toDouble()
	    || JL_JsvalIsNInfinity(cx, val);
}


static ALWAYS_INLINE bool
JL_JsvalIsScript( const JSContext *cx, const jsval &val ) {

	JL_UNUSED(cx);
	return JSVAL_IS_PRIMITIVE(val) && JL_GetClass(JSVAL_TO_OBJECT(val)) == js::Jsvalify(&js_ScriptClass);
}

static ALWAYS_INLINE bool
JL_ObjectIsFunction( const JSContext *cx, const JSObject *obj ) {

	JL_UNUSED(cx);
	return obj->isFunction();
}

static ALWAYS_INLINE bool
JL_JsvalIsFunction( const JSContext *cx, const jsval &val ) {

	JL_UNUSED(cx);
	return VALUE_IS_FUNCTION(cx, val);
}


static ALWAYS_INLINE bool
JL_JsvalIsArray( JSContext *cx, const jsval &val ) {

	JL_UNUSED(cx);
	return !JSVAL_IS_PRIMITIVE(val) && JS_IsArrayObject(cx, JSVAL_TO_OBJECT(val)); // Object::isArray() is not public
}


static ALWAYS_INLINE bool
JL_JsvalIsStringObject( const JSContext *cx, const jsval &val ) {

//	return (!JSVAL_IS_PRIMITIVE(val) && js::Valueify(val).toObject().getJSClass() == JL_GetHostPrivate(cx)->stringObjectClass);
	return !JSVAL_IS_PRIMITIVE(val) && JL_GetClass(JSVAL_TO_OBJECT(val)) == JL_GetHostPrivate(cx)->stringObjectClass;
}


static ALWAYS_INLINE bool
JL_JsvalIsData( JSContext *cx, const jsval &val ) {

	return ( JSVAL_IS_STRING(val) || JL_JsvalIsStringObject(cx, val) || (!JSVAL_IS_PRIMITIVE(val) && BufferGetInterface(cx, JSVAL_TO_OBJECT(val)) != NULL) );
}


static ALWAYS_INLINE bool
JL_JsvalIsClass( const jsval &val, const JSClass *jsClass ) {

	//return !JSVAL_IS_PRIMITIVE(val) && JL_GetClass(JSVAL_TO_OBJECT(val)) == jsClass;
	return jsClass != NULL && !js::Valueify(val).isPrimitive() && js::Valueify(val).toObject().getJSClass() == jsClass;
}



///////////////////////////////////////////////////////////////////////////////
// jsval convertion functions


static ALWAYS_INLINE jsid 
JL_StringToJsid( JSContext *cx, const char *cstr ) {

	JSString *jsstr = JS_InternString(cx, cstr);
	if ( jsstr == NULL )
		return JL_NullJsid();
//	jsid tmp;
//	if ( !JS_ValueToId(cx, STRING_TO_JSVAL(jsstr), &tmp) )
//		return JL_NullJsid();
//	return tmp;
#ifdef DEBUG
	jsid id, tmp;
	id = ATOM_TO_JSID(STRING_TO_ATOM(jsstr));
	JL_ASSERT( JSID_IS_STRING( id ) );
	JS_ValueToId(cx, STRING_TO_JSVAL(jsstr), &tmp);
	JS_ASSERT( id == tmp );
	return id;
#else
	return ATOM_TO_JSID(STRING_TO_ATOM(jsstr));
#endif // DEBUG
}


static ALWAYS_INLINE JSFunction*
JL_ObjectToFunction( JSContext *cx, const JSObject *obj ) {

	JL_UNUSED(cx);
	return GET_FUNCTION_PRIVATE(cx, obj);
}

static ALWAYS_INLINE JSFunction*
JL_JsvalToFunction( JSContext *cx, const jsval &val ) {

	JL_UNUSED(cx);
	return GET_FUNCTION_PRIVATE(cx, JSVAL_TO_OBJECT(val));
}


static ALWAYS_INLINE JSBool
JL_JsvalToJsid( JSContext *cx, jsval *val, jsid *id ) {

	if ( JSVAL_IS_INT( *val ) ) {
		
		*id = INT_TO_JSID( JSVAL_TO_INT( *val ) );
		return JS_TRUE;
	}

	if ( JSVAL_IS_OBJECT( *val ) ) {
		
		*id = OBJECT_TO_JSID( JSVAL_TO_OBJECT( *val ) );
		return JS_TRUE;
	}

	if ( JSVAL_IS_STRING( *val ) ) {

		*id = ATOM_TO_JSID(STRING_TO_ATOM(JSVAL_TO_STRING( *val )));
		JL_ASSERT( JSID_IS_STRING( *id ) );
		return JS_TRUE;
	}

	if ( JSVAL_IS_VOID( *val ) ) {
		
		*id = JSID_VOID;
		return JS_TRUE;
	}

	return JS_ValueToId(cx, *val, id);
}


static ALWAYS_INLINE JSBool
JL_JsidToJsval( JSContext *cx, jsid id, jsval *val ) {

	if ( JSID_IS_INT( id ) ) {
		
		*val = INT_TO_JSVAL( JSID_TO_INT( id ) );
		return JS_TRUE;
	}

	if ( JSID_IS_OBJECT( id ) ) {
		
		*val = OBJECT_TO_JSVAL( JSID_TO_OBJECT( id ) );
		return JS_TRUE;
	}

	if ( JSID_IS_STRING( id ) ) {

		*val = STRING_TO_JSVAL(ATOM_TO_STRING(JSID_TO_ATOM(id)));
		JL_ASSERT( JSVAL_IS_STRING(*val) );
		return JS_TRUE;
	}

	if ( JSID_IS_VOID( id ) ) {
		
		*val = JSVAL_VOID;
		return JS_TRUE;
	}

	return JS_IdToValue(cx, id, val);
}



///////////////////////////////////////////////////////////////////////////////
// convertion functions


template <typename T>
static ALWAYS_INLINE bool
JL_Alloc( T*&ptr, size_t count = 1 ) {

	ptr = (T*)jl_malloc(sizeof(T)*count);
	return ptr != NULL;
}

template <typename T>
static ALWAYS_INLINE bool
JL_Realloc( T*&ptr, size_t count = 1 ) {

	ptr = (T*)jl_realloc(ptr, sizeof(T)*count);
	return ptr != NULL;
}


class JLStr {

	struct Own {

		size_t len;
		uint32_t count;
		const jschar *cjsstr;
		jschar *jsstr;
		const char *cstr;
		char *str;
		const char *cstrz;
		char *strz;
	};
	Own *_own;

	ALWAYS_INLINE void NewOwn(const jschar *cjsstr, jschar *jsstr, const char *cstr, char *str, const char *cstrz, char *strz, size_t len) {

		JL_Alloc(_own);
		_own->count = 1;
		_own->len = len;
		_own->cjsstr = cjsstr;
		_own->jsstr = jsstr;
		_own->cstr = cstr;
		_own->str = str;
		_own->cstrz = cstrz;
		_own->strz = strz;
		JL_ASSERT( IsSet() );
	}

	ALWAYS_INLINE jschar * CreateJsStr() {

		JL_ASSERT( _own );
		JL_ASSERT( _own->cstrz || _own->strz || _own->cstr || _own->str );
		size_t length = Length();
		const char *src;
		if ( _own->cstrz )
			src = _own->cstrz;
		else if ( _own->strz )
			src = _own->strz;
		else if ( _own->cstr )
			src = _own->cstr;
		else if ( _own->str )
			src = _own->str;
		jschar *dst;
		if ( !JL_Alloc(dst, length) )
			return NULL;
		jschar *tmp = dst;
		for ( size_t i = length; i > 0; --i )
			*(tmp++) = (unsigned char)*(src++);
		return dst;
	}

	ALWAYS_INLINE char * CreateStrZ() {

		JL_ASSERT( _own );
		JL_ASSERT( _own->cjsstr || _own->jsstr );
		size_t length = Length();
		const jschar *src = _own->cjsstr ? _own->cjsstr : _own->jsstr;
		char *dst;
		if ( !JL_Alloc(dst, length +1) )
			return NULL;
		char *tmp = dst;
		for ( size_t i = length; i > 0; --i )
			*(tmp++) = (char)*(src++);
		*tmp = '\0';
		return dst;
	}

public:
	~JLStr() {
		
		if ( !_own || --_own->count )
			return;
		if ( _own->jsstr )
			jl_free(_own->jsstr);
		if ( _own->strz )
			jl_free(_own->strz);
		if ( _own->str )
			jl_free(_own->str);
		jl_free(_own);
	}
	
	ALWAYS_INLINE JLStr() : _own(NULL) {

		JL_ASSERT( !JS_CStringsAreUTF8() );
	}

	ALWAYS_INLINE JLStr(const JLStr &jlstr) : _own(jlstr._own) {
		
		JL_ASSERT( _own );
		++_own->count;
	}

	ALWAYS_INLINE void operator=(const JLStr &jlstr) {

		JL_ASSERT( !_own );
		_own = jlstr._own;
		++_own->count;
	}

	ALWAYS_INLINE JLStr(JSString *jsstr) {

		size_t length;
		const jschar *str = JS_GetStringCharsAndLength(jsstr, &length); // doc. not null-terminated. // see also JS_GetStringCharsZ
		NewOwn(str, NULL, NULL, NULL, NULL, NULL, length);
	}

	ALWAYS_INLINE JLStr(const jschar *str, size_t length) {
		
		NewOwn(str, NULL, NULL, NULL, NULL, NULL, length);
	}

	ALWAYS_INLINE JLStr(jschar *str, size_t length) { // give ownership of str to JLStr

		NewOwn(NULL, str, NULL, NULL, NULL, NULL, length);
	}

	ALWAYS_INLINE JLStr(const char *str, bool hasZ) {

		JL_ASSERT_IF( hasZ, ((uint8_t*)str)[strlen(str)] == 0 ); 

		if ( hasZ )
			NewOwn(NULL, NULL, NULL, NULL, str, NULL, SIZE_MAX);
		else
			NewOwn(NULL, NULL, str, NULL, NULL, NULL, SIZE_MAX);
	}

	ALWAYS_INLINE JLStr(const char *str, size_t length, bool hasZ) {

		JL_ASSERT_IF( hasZ, ((uint8_t*)str)[length] == 0 ); 

		if ( hasZ )
			NewOwn(NULL, NULL, NULL, NULL, str, NULL, length);
		else
			NewOwn(NULL, NULL, str, NULL, NULL, NULL, length);
	}

	ALWAYS_INLINE JLStr(char *str, bool hasZ) { // give ownership of str to JLStr

		JL_ASSERT_IF( hasZ, jl_msize(str) >= strlen(str) + 1 );

		if ( hasZ )
			NewOwn(NULL, NULL, NULL, NULL, NULL, str, SIZE_MAX);
		else
			NewOwn(NULL, NULL, NULL, str, NULL, NULL, SIZE_MAX);
	}

	ALWAYS_INLINE JLStr(char *str, size_t length, bool hasZ) { // give ownership of str to JLStr

		JL_ASSERT_IF( hasZ, jl_msize(str) >= length + 1 );
		JL_ASSERT_IF( hasZ, ((uint8_t*)str)[length] == 0 ); 

		if ( hasZ )
			NewOwn(NULL, NULL, NULL, NULL, NULL, str, length);
		else
			NewOwn(NULL, NULL, NULL, str, NULL, NULL, length);
	}

	ALWAYS_INLINE bool IsSet() const {
		
		return _own && (_own->cjsstr || _own->cstrz || _own->cstr || _own->jsstr || _own->str || _own->strz);
	}

	ALWAYS_INLINE bool HasJsStr() const {

		JL_ASSERT( _own );
		return _own->cjsstr || _own->jsstr;
	}

	ALWAYS_INLINE size_t Length() {
		
		JL_ASSERT( IsSet() );
		if ( _own->len != SIZE_MAX ) // known length
			return _own->len;
		JL_ASSERT( _own->cstrz || _own->strz );
		_own->len = strlen(_own->cstrz ? _own->cstrz : _own->strz);
		return _own->len;
	}

	ALWAYS_INLINE size_t LengthOrZero() {
		
		return IsSet() ? Length() : 0;
	}

	INLINE const jschar *GetConstJsStr() {

		JL_ASSERT( IsSet() );
		if ( _own->cjsstr )
			return _own->cjsstr;
		if ( _own->jsstr )
			return _own->jsstr;
		JL_ASSERT( !_own->jsstr );
		_own->jsstr = CreateJsStr();
		return _own->jsstr;
	}

	ALWAYS_INLINE const jschar *GetJsStrConstOrNull() {

		return IsSet() ? GetConstJsStr() : NULL;
	}

	INLINE jschar *GetJsStrOwnership() {

		JL_ASSERT( IsSet() );
		jschar *tmp;
		if ( _own->jsstr ) {

			tmp = _own->jsstr;
			_own->jsstr = NULL;
			return tmp;
		}
		if ( _own->cjsstr ) {
	
			size_t length = Length();
			JL_Alloc(tmp, length); // js_malloc
			JL_ASSERT(tmp != NULL);
			memcpy(tmp, _own->cjsstr, length * sizeof(*tmp));
			return tmp;
		}
		return CreateJsStr();
	}

	INLINE const char *GetConstStr() {

		JL_ASSERT( IsSet() );
		if ( _own->strz )
			return _own->strz;
		if ( _own->cstrz )
			return _own->cstrz;
		if ( _own->cstr )
			return _own->cstr;
		if ( _own->str )
			return _own->str;
		_own->strz = CreateStrZ();
		return _own->strz;
	}

	ALWAYS_INLINE const char *GetStrConstOrNull() {

		return IsSet() ? GetConstStr() : NULL;
	}

	INLINE char *GetStrZOwnership() {

		JL_ASSERT( IsSet() );

		char *tmp;

		if ( _own->strz ) {

			tmp = _own->strz;
			_own->strz = NULL;
			return tmp;
		}

		size_t length = Length();

		if ( _own->cstrz ) {
			
			JL_Alloc(tmp, length +1);
			JL_ASSERT( tmp != NULL );
			tmp[length] = '\0';
			memcpy(tmp, _own->cstrz, length * sizeof(*tmp));
			return tmp;
		}

		if ( _own->cstr ) {
			
			JL_Alloc(tmp, length +1);
			JL_ASSERT( tmp != NULL );
			tmp[length] = '\0';
			memcpy(tmp, _own->cstr, length * sizeof(*tmp));
			return tmp;
		}

		if ( _own->str ) {

			tmp = _own->str;
			_own->str = NULL;
			JL_Realloc(tmp, length +1);
			JL_ASSERT( tmp != NULL );
			tmp[length] = '\0';
			return tmp;
		}

		return CreateStrZ();
	}

	INLINE const char *GetConstStrZ() {
	
		if ( !IsSet() )
			return NULL;

		if ( _own->strz )
			return _own->strz;

		if ( _own->cstrz )
			return _own->cstrz;

		size_t length = Length();

		if ( _own->str ) {

			JL_ASSERT( !_own->str );
			JL_Realloc(_own->str, length + 1);
			JL_ASSERT( _own->str );
			_own->str[length] = '\0';
			_own->strz = _own->str;
			_own->str = NULL;
			return _own->strz;
		}

		if ( _own->cstr ) {
			
			JL_ASSERT( !_own->strz );
			JL_Alloc(_own->strz, length + 1);
			JL_ASSERT( _own->strz );
			_own->strz[length] = '\0';
			memcpy(_own->strz, _own->cstr, length);
			return _own->strz;
		}

		_own->strz = CreateStrZ();
		return _own->strz;
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


// JLStr

static ALWAYS_INLINE JSBool
JL_JsvalToNative( JSContext *cx, jsval &val, JLStr *str ) {

	if (likely( JSVAL_IS_STRING(val) )) { // for string literals

		*str = JLStr(JSVAL_TO_STRING(val));
		return JS_TRUE;
	}
	if (likely( !JSVAL_IS_PRIMITIVE(val) )) { // for NIBufferGet support

		NIBufferGet fct = BufferGetNativeInterface(cx, JSVAL_TO_OBJECT(val));
		if ( fct )
			return fct(cx, JSVAL_TO_OBJECT(val), str);
	}
	JSString *jsstr = JS_ValueToString(cx, val);
	if ( jsstr == NULL )
		JL_REPORT_ERROR_NUM(cx, JLSMSG_FAIL_TO_CONVERT_TO, "string" );
	val = STRING_TO_JSVAL(jsstr); // GC protection
	*str = JLStr(jsstr);
	return JS_TRUE;
	JL_BAD;
}


// jschar

static ALWAYS_INLINE JSBool
JL_NativeToJsval( JSContext *cx, const jschar* cval, size_t length, jsval *vp ) {

	if (unlikely( cval == NULL )) {

		*vp = JSVAL_VOID;
		return JS_TRUE;
	}
	if (unlikely( length == 0 )) {

		*vp = JL_GetEmptyStringValue(cx);
		return JS_TRUE;
	}
	JSString *jsstr = JS_NewUCStringCopyN(cx, cval, length);
	if (unlikely( jsstr == NULL ))
		JL_REPORT_ERROR( "Unable to create the string." );
	*vp = STRING_TO_JSVAL(jsstr);
	return JS_TRUE;
	JL_BAD;
}


// c-strings

static ALWAYS_INLINE JSBool
JL_NativeToJsval( JSContext *cx, const char* cval, jsval *vp ) {

	if (unlikely( cval == NULL )) {

		*vp = JSVAL_VOID;
		return JS_TRUE;
	}
	if (unlikely( *cval == '\0' )) {

		*vp = JL_GetEmptyStringValue(cx);
		return JS_TRUE;
	}
	JSString *jsstr = JS_NewStringCopyZ(cx, cval);
	if (unlikely( jsstr == NULL ))
		JL_REPORT_ERROR( "Unable to create the string." );
	*vp = STRING_TO_JSVAL(jsstr);
	return JS_TRUE;
	JL_BAD;
}

static ALWAYS_INLINE JSBool
JL_NativeToJsval( JSContext *cx, const char* cval, size_t length, jsval *vp ) {

	if (unlikely( cval == NULL )) {

		*vp = JSVAL_VOID;
		return JS_TRUE;
	}
	if (unlikely( length == 0 )) {

		*vp = JL_GetEmptyStringValue(cx);
		return JS_TRUE;
	}
	JSString *jsstr = JS_NewStringCopyN(cx, cval, length);
	if (unlikely( jsstr == NULL ))
		JL_REPORT_ERROR( "Unable to create the string." );
	*vp = STRING_TO_JSVAL(jsstr);
	return JS_TRUE;
	JL_BAD;
}


// int8 / uint8

//static JSBool JL_NativeToJsval( JSContext *cx, const int8_t &num, jsval *vp );
static JSBool JL_JsvalToNative( JSContext *cx, const jsval &val, int8_t *num );
//static JSBool JL_NativeToJsval( JSContext *cx, const uint8_t &num, jsval *vp );
static JSBool JL_JsvalToNative( JSContext *cx, const jsval &val, uint8_t *num );


// int16 / uint16

//static JSBool JL_NativeToJsval( JSContext *cx, const int16_t &num, jsval *vp );
static JSBool JL_JsvalToNative( JSContext *cx, const jsval &val, int16_t *num );
//static JSBool JL_NativeToJsval( JSContext *cx, const uint16_t &num, jsval *vp );
static JSBool JL_JsvalToNative( JSContext *cx, const jsval &val, uint16_t *num );


// int32

JL_STATIC_ASSERT( _I32_MIN == JSVAL_INT_MIN && _I32_MAX == JSVAL_INT_MAX );

static ALWAYS_INLINE JSBool
JL_NativeToJsval( JSContext *cx, const int32_t &num, jsval *vp ) {

	JL_UNUSED(cx);
	*vp = INT_TO_JSVAL(num);
	return JS_TRUE;
}

static INLINE JSBool
JL_JsvalToNative( JSContext *cx, const jsval &val, int32_t *num ) {

	if (likely( JSVAL_IS_INT(val) )) {

		*num = JSVAL_TO_INT(val);
		return JS_TRUE;
	}

	jsdouble d;
	if (likely( JSVAL_IS_DOUBLE(val) ))
		d = JSVAL_TO_DOUBLE(val);
	else
		JL_CHK( JS_ValueToNumber(cx, val, &d) ); // NULL gives 0

	if (likely( d >= jsdouble(_I32_MIN) && d <= jsdouble(_I32_MAX) )) {

		JL_SAFE_BEGIN
		if ( !JL_DOUBLE_IS_INTEGER(d) )
			JL_REPORT_WARNING_NUM(cx, JLSMSG_VALUE_LOSSOFDATA);
		JL_SAFE_END

		*num = int32_t(d);
		return JS_TRUE;
	}

	JL_REPORT_ERROR_NUM(cx, JLSMSG_VALUE_OUTOFRANGE);
	JL_BAD;
}


// uint32

JL_STATIC_ASSERT( _UI32_MAX >= JSVAL_INT_MAX );

static ALWAYS_INLINE JSBool
JL_NativeToJsval( JSContext *cx, const uint32_t &num, jsval *vp ) {

	JL_UNUSED(cx);

	if (likely( num <= uint32_t(JSVAL_INT_MAX) )) {
		
		*vp = INT_TO_JSVAL(num);
	} else {

		JL_ASSERT( jsdouble(num) <= MAX_INT_TO_DOUBLE );
		*vp = DOUBLE_TO_JSVAL(jsdouble(num));
	}
	return JS_TRUE;
}

static INLINE JSBool
JL_JsvalToNative( JSContext *cx, const jsval &val, uint32_t *num ) {

	if (likely( JSVAL_IS_INT(val) )) {

		jsint tmp = JSVAL_TO_INT(val);
		if (unlikely( tmp < 0 ))
			JL_REPORT_ERROR_NUM(cx, JLSMSG_VALUE_OUTOFRANGE);
		*num = uint32_t(tmp);
		return JS_TRUE;
	}

	jsdouble d;
	if (likely( JSVAL_IS_DOUBLE(val) ))
		d = JSVAL_TO_DOUBLE(val);
	else
		JL_CHK( JS_ValueToNumber(cx, val, &d) );

	if (likely( d >= jsdouble(0) && d <= jsdouble(_UI32_MAX) )) {

		JL_SAFE_BEGIN
		if ( !JL_DOUBLE_IS_INTEGER(d) )
			JL_REPORT_WARNING_NUM(cx, JLSMSG_VALUE_LOSSOFDATA);
		JL_SAFE_END

		*num = uint32_t(d);
		return JS_TRUE;
	}

	JL_REPORT_ERROR_NUM(cx, JLSMSG_VALUE_OUTOFRANGE);
	JL_BAD;
}


// int64

static ALWAYS_INLINE JSBool
JL_NativeToJsval( JSContext *cx, const int64_t &num, jsval *vp ) {

	if ( num >= int64_t(JSVAL_INT_MIN) && num <= int64_t(JSVAL_INT_MAX) ) {

		*vp = INT_TO_JSVAL(jsint(num));
	} else {

		if ( num < int64_t(-MAX_INT_TO_DOUBLE) || num > int64_t(MAX_INT_TO_DOUBLE) )
			JL_REPORT_ERROR_NUM(cx, JLSMSG_VALUE_OUTOFRANGE);
		*vp = DOUBLE_TO_JSVAL(jsdouble(num));
	}	
	return JS_TRUE;
	JL_BAD;
}

static INLINE JSBool
JL_JsvalToNative( JSContext *cx, const jsval &val, int64_t *num ) {

	if (likely( JSVAL_IS_INT(val) )) {

		*num = JSVAL_TO_INT(val);
		return JS_TRUE;
	}

	jsdouble d;
	if (likely( JSVAL_IS_DOUBLE(val) ))
		d = JSVAL_TO_DOUBLE(val);
	else
		JL_CHK( JS_ValueToNumber(cx, val, &d) ); // NULL gives 0

	if (likely( d >= jsdouble(_I64_MIN) && d <= jsdouble(_I64_MAX) )) {

		JL_SAFE_BEGIN
		if ( !JL_DOUBLE_IS_INTEGER(d) )
			JL_REPORT_WARNING_NUM(cx, JLSMSG_VALUE_LOSSOFDATA);
		JL_SAFE_END

		*num = int64_t(d);
		return JS_TRUE;
	}

	JL_REPORT_ERROR_NUM(cx, JLSMSG_VALUE_OUTOFRANGE);
	JL_BAD;
}


// uint64

static ALWAYS_INLINE JSBool
JL_NativeToJsval( JSContext *cx, const uint64_t &num, jsval *vp ) {

	if (likely( num <= uint64_t(JSVAL_INT_MAX) )) {

		*vp = INT_TO_JSVAL(int32(num));
	} else {

		JL_SAFE_BEGIN
		if ( num > MAX_INT_TO_DOUBLE )
			JL_REPORT_WARNING_NUM(cx, JLSMSG_VALUE_LOSSOFDATA);
		JL_SAFE_END
		*vp = DOUBLE_TO_JSVAL(jsdouble(num));
	}
	return JS_TRUE;
	JL_BAD;
}

static INLINE JSBool
JL_JsvalToNative( JSContext *cx, const jsval &val, uint64_t *num ) {

	if (likely( JSVAL_IS_INT(val) )) {

		*num = JSVAL_TO_INT(val);
		return JS_TRUE;
	}

	jsdouble d;
	if (likely( JSVAL_IS_DOUBLE(val) ))
		d = JSVAL_TO_DOUBLE(val);
	else
		JL_CHK( JS_ValueToNumber(cx, val, &d) ); // NULL gives 0

	if (likely( d < MAX_INT_TO_DOUBLE )) { // or d <= jsdouble(_UI64_MAX)

		JL_SAFE_BEGIN
		if ( !JL_DOUBLE_IS_INTEGER(d) )
			JL_REPORT_WARNING_NUM(cx, JLSMSG_VALUE_LOSSOFDATA);
		JL_SAFE_END

		*num = uint64_t(d);
		return JS_TRUE;
	}

	JL_REPORT_ERROR_NUM(cx, JLSMSG_VALUE_OUTOFRANGE);
	JL_BAD;
}


// long

static ALWAYS_INLINE JSBool
JL_NativeToJsval( JSContext *cx, const long &num, jsval *vp ) {

	if ( sizeof(long) == sizeof(int64_t) ) return JL_NativeToJsval(cx, int64_t(num), vp);
	if ( sizeof(long) == sizeof(int32_t) ) return JL_NativeToJsval(cx, int32_t(num), vp);
	JL_ASSERT(false);
}

static ALWAYS_INLINE JSBool
JL_JsvalToNative( JSContext *cx, const jsval &val, long *num ) {

	if ( sizeof(long) == sizeof(int64_t) ) return JL_JsvalToNative(cx, val, (int64_t*)num);
	if ( sizeof(long) == sizeof(int32_t) ) return JL_JsvalToNative(cx, val, (int32_t*)num);
	JL_ASSERT(false);
}


// unsigned long

static ALWAYS_INLINE JSBool
JL_NativeToJsval( JSContext *cx, const unsigned long &num, jsval *vp ) {

	if ( sizeof(unsigned long) == sizeof(uint64_t) ) return JL_NativeToJsval(cx, uint64_t(num), vp);
	if ( sizeof(unsigned long) == sizeof(uint32_t) ) return JL_NativeToJsval(cx, uint32_t(num), vp);
	JL_ASSERT(false);
}

static ALWAYS_INLINE JSBool
JL_JsvalToNative( JSContext *cx, const jsval &val, unsigned long *num ) {

	if ( sizeof(unsigned long) == sizeof(uint64_t) ) return JL_JsvalToNative(cx, val, (uint64_t*)num);
	if ( sizeof(unsigned long) == sizeof(uint32_t) ) return JL_JsvalToNative(cx, val, (uint32_t*)num);
	JL_ASSERT(false);
}


/*
// size_t

static ALWAYS_INLINE JSBool JL_NativeToJsval( JSContext *cx, const size_t &num, jsval *vp ) {

	JL_UNUSED(cx);
	if (likely( num <= JSVAL_INT_MAX ))
		*vp = INT_TO_JSVAL(jsint(num));
	else
		*vp = DOUBLE_TO_JSVAL(jsdouble(num));
	return JS_TRUE;
}

static ALWAYS_INLINE JSBool JL_JsvalToNative( JSContext *cx, const jsval &val, size_t *num ) {

	if (likely( JSVAL_IS_INT(val) )) {

		jsint tmp = JSVAL_TO_INT(val);
		if (unlikely( tmp < 0 ))
			JL_REPORT_ERROR_NUM(cx, JLSMSG_VALUE_OUTOFRANGE);
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
			JL_REPORT_WARNING_NUM(cx, JLSMSG_VALUE_LOSSOFDATA);
		JL_SAFE_END

		*num = size_t(d);
		return JS_TRUE;
	}

	JL_REPORT_ERROR_NUM(cx, JLSMSG_VALUE_OUTOFRANGE);
	JL_BAD;
}


// ssize_t

static ALWAYS_INLINE JSBool JL_NativeToJsval( JSContext *cx, const ssize_t &num, jsval *vp ) {

	JL_UNUSED(cx);
	if (likely( num >= JSVAL_INT_MIN && size <= JSVAL_INT_MAX ))
		*vp = INT_TO_JSVAL(jsint(num));
	else
		*vp = DOUBLE_TO_JSVAL(jsdouble(num));
	return JS_TRUE;
}

static ALWAYS_INLINE JSBool JL_JsvalToNative( JSContext *cx, const jsval &val, ssize_t *num ) {

	if (likely( JSVAL_IS_INT(val) )) {

		*num = ssize_t(JSVAL_TO_INT(val));
		return JS_TRUE;
	}

	jsdouble d;
	if (likely( JSVAL_IS_DOUBLE(val) ))
		d = JSVAL_TO_DOUBLE(val);
	else
		JL_CHK( JS_ValueToNumber(cx, val, &d) );

	if (likely( d >= jsdouble(SSIZE_T_MIN) && d <= jsdouble(SSIZE_T_MAX) )) {

		JL_SAFE_BEGIN
		if ( !jl::IsSafeCast(d, *i) )
			JL_REPORT_WARNING_NUM(cx, JLSMSG_VALUE_LOSSOFDATA);
		JL_SAFE_END

		*num = (ssize_t)d;
		return JS_TRUE;
	}

	JL_REPORT_ERROR_NUM(cx, JLSMSG_VALUE_OUTOFRANGE);
	JL_BAD;
}
*/


// double

static ALWAYS_INLINE JSBool
JL_NativeToJsval( JSContext *cx, const double &num, jsval *vp ) {

	JL_UNUSED(cx);
	*vp = DOUBLE_TO_JSVAL(num);
	return JS_TRUE;
}

static ALWAYS_INLINE JSBool
JL_JsvalToNative( JSContext *cx, const jsval &val, double *num ) {

	if (likely( JSVAL_IS_DOUBLE(val) )) {

		*num = JSVAL_TO_DOUBLE(val);
		return JS_TRUE;
	}
	if ( JSVAL_IS_INT(val) ) {

		*num = double(JSVAL_TO_INT(val));
		return JS_TRUE;
	}
	return JS_ValueToNumber(cx, val, num);
}


// float

static ALWAYS_INLINE JSBool
JL_NativeToJsval( JSContext *cx, const float &num, jsval *vp ) {

	JL_UNUSED(cx);
	*vp = DOUBLE_TO_JSVAL(num);
	return JS_TRUE;
}

static ALWAYS_INLINE JSBool
JL_JsvalToNative( JSContext *cx, const jsval &val, float *num ) {

	if (likely( JSVAL_IS_DOUBLE(val) )) {

		*num = float(JSVAL_TO_DOUBLE(val));
		return JS_TRUE;
	}
	if ( JSVAL_IS_INT(val) ) {

		*num = float(JSVAL_TO_INT(val));
		return JS_TRUE;
	}
	jsdouble tmp;
	if ( !JS_ValueToNumber(cx, val, &tmp) )
		return JS_FALSE;
	*num = float(tmp);
	return JS_TRUE;
}


// boolean

static ALWAYS_INLINE JSBool
JL_NativeToJsval( JSContext *cx, const bool &b, jsval *vp ) {

	JL_UNUSED(cx);
	*vp = b ? JSVAL_TRUE : JSVAL_FALSE;
	return JS_TRUE;
}

static ALWAYS_INLINE JSBool
JL_JsvalToNative( JSContext *cx, const jsval &val, bool *b ) {

	//if (likely( JSVAL_IS_BOOLEAN(val) )) {
	//	*b = (JSVAL_TO_BOOLEAN(val) == JS_TRUE);
	//	return JS_TRUE;
	if ( js::Valueify(val).isTrue() ) {

		*b = true;
		return JS_TRUE;
	}
	if ( js::Valueify(val).isFalse() ) {

		*b = false;
		return JS_TRUE;
	}
	JSBool tmp;
	if ( !JS_ValueToBoolean(cx, val, &tmp) )
		return JS_FALSE;
	*b = (tmp == JS_TRUE);
	return JS_TRUE;
}


// void*

static ALWAYS_INLINE JSBool
JL_NativeToJsval( JSContext *cx, void *ptr, jsval *vp ) {

	if ( ((uint32)ptr & 1) == 1 ) { // cannot be stored with PRIVATE_TO_JSVAL()

//		void **data;
//		JL_CHK( HandleCreate(cx, JL_CAST_CSTR_TO_UINT32("PRIV"), sizeof(void*), (void*)&data, NULL, vp) );
//		*data = ptr;
		JL_REPORT_ERROR("Unable to store non-aligned pointers.");
	} else {
	
		JL_UNUSED(cx);
		*vp = PRIVATE_TO_JSVAL(ptr);
	}
	return JS_TRUE;
	JL_BAD;
}

static ALWAYS_INLINE JSBool
JL_JsvalToNative( JSContext *cx, const jsval &val, void **ptr ) {

	JL_UNUSED(cx);
//	if ( IsHandleType(cx, val, JL_CAST_CSTR_TO_UINT32("PRIV") ) {
//		
//		*ptr = *(void**)GetHandlePrivate(cx, val);
//		return JS_TRUE;
//	} else {

		*ptr = JSVAL_TO_PRIVATE(val);
//	}
	return JS_TRUE;
}



///////////////////////////////////////////////////////////////////////////////
// vector convertion functions

// if useValArray is true, val must be a valid array that is used to store the values.
template <class T>
static INLINE JSBool
JL_CValVectorToJsval( JSContext *cx, const T *vector, jsuint length, jsval *val, bool useValArray = false ) {

	js::AutoValueRooter avr(cx);
	JSObject *arrayObj;
	if ( useValArray ) {

		JL_S_ASSERT_OBJECT(*val);
		arrayObj = JSVAL_TO_OBJECT(*val);
		JL_CHK( JS_SetArrayLength(cx, arrayObj, length) );
	} else {

		// js_NewArrayObjectWithCapacity
		arrayObj = JS_NewArrayObject(cx, length, NULL);
		JL_CHK( arrayObj );
		*val = OBJECT_TO_JSVAL(arrayObj);
	}

	for ( jsuint i = 0; i < length; ++i ) {

		JL_CHK( JL_NativeToJsval(cx, vector[i], avr.jsval_addr()) );
		//JL_CHK( JS_SetPropertyById(cx, arrayObj, INT_TO_JSID(i), avr.jsval_addr()) );
		JL_CHK( JS_SetElement(cx, arrayObj, i, avr.jsval_addr()) );
	}
//	JL_CHK( JS_SetArrayLength(cx, arrayObj, length) );
	return JS_TRUE;
	JL_BAD;
}


template <class T>
static INLINE JSBool
JL_JsvalToCValVector( JSContext *cx, jsval &val, T *vector, jsuint maxLength, jsuint *actualLength ) {

	JL_S_ASSERT_OBJECT(val);
	JSObject *arrayObj;
	arrayObj = JSVAL_TO_OBJECT(val);
	JL_CHK( JS_GetArrayLength(cx, arrayObj, actualLength) );
	maxLength = JL_MIN( *actualLength, maxLength );
	for ( jsuint i = 0; i < maxLength; ++i ) {

		JL_CHK( JS_GetElement(cx, arrayObj, i, &val) );
		//JL_CHK( JS_GetPropertyById(cx, arrayObj, INT_TO_JSID(i), &val) );
		JL_CHK( JL_JsvalToNative(cx, val, &vector[i]) );
	}
	return JS_TRUE;
	JL_BAD;
}



///////////////////////////////////////////////////////////////////////////////
// properties conversion helper


template <class T>
static ALWAYS_INLINE JSBool
JL_SetProperty( JSContext *cx, JSObject *obj, const char *propertyName, const T &cval, bool publicData = true ) {

	js::AutoValueRooter avr(cx);
	JL_CHK( JL_NativeToJsval(cx, cval, avr.jsval_addr()) );
	if ( publicData )
		JL_CHKM( JS_SetProperty(cx, obj, propertyName, avr.jsval_addr()), "Unable to set the property %s.", propertyName );
	else
		JL_CHKM( JS_DefineProperty(cx, obj, propertyName, avr.jsval_value(), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT ), "Unable to set the property %s." ); // Doc. http://developer.mozilla.org/en/docs/JS_DefineUCProperty
	return JS_TRUE;
	JL_BAD;
}

template <class T>
static ALWAYS_INLINE JSBool
JL_SetProperty( JSContext *cx, JSObject *obj, jsid id, const T &cval, bool publicData = true ) {

	js::AutoValueRooter avr(cx);
	JL_CHK( JL_NativeToJsval(cx, cval, avr.jsval_addr()) );
	if ( publicData )
		JL_CHKM( JS_SetPropertyById(cx, obj, id, avr.jsval_addr()), "Unable to set the property." );
	else
		JL_CHKM( JS_DefinePropertyById(cx, obj, id, avr.jsval_value(), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT ), "Unable to set the property." ); // Doc. http://developer.mozilla.org/en/docs/JS_DefineUCProperty
	return JS_TRUE;
	JL_BAD;
}


template <class T>
static ALWAYS_INLINE JSBool
JL_GetProperty( JSContext *cx, JSObject *obj, const char *propertyName, T *cval ) {

	jsval v;
	JL_CHKM( JS_GetProperty(cx, obj, propertyName, &v), "Unable to read the property %s.", propertyName );
	JL_CHK( JL_JsvalToNative(cx, v, cval) );
	return JS_TRUE;
	JL_BAD;
}

template <class T>
static ALWAYS_INLINE JSBool
JL_GetProperty( JSContext *cx, JSObject *obj, jsid id, T *cval ) {

	jsval v;
	JL_CHKM( JS_GetPropertyById(cx, obj, id, &v), "Unable to read the property." );
	JL_CHK( JL_JsvalToNative(cx, v, cval) );
	return JS_TRUE;
	JL_BAD;
}



///////////////////////////////////////////////////////////////////////////////
// Host info functions


ALWAYS_INLINE JSBool RemoveHostObject(JSContext *cx) {

	JSObject *globalObject = JL_GetGlobalObject(cx);
	JL_S_ASSERT( globalObject != NULL, "Unable to find the global object." );
	return JS_DeletePropertyById(cx, globalObject, JLID(cx, _host));
	JL_BAD;
}


inline JSObject *GetHostObject(JSContext *cx) {

	JSObject *cobj, *globalObject = JL_GetGlobalObject(cx);
	JL_CHK( globalObject );
	jsval hostObjectValue;
	jsid hostObjectId;
	hostObjectId = JLID(cx, _host);
	JL_CHK( hostObjectId != JL_NullJsid() );
	JL_CHK( JS_GetPropertyById(cx, globalObject, hostObjectId, &hostObjectValue) );

	if ( JSVAL_IS_VOID( hostObjectValue ) ) { // if configuration object do not exist, we build one

		cobj = JS_DefineObject(cx, globalObject, JLID_NAME(cx, _host), NULL, NULL, 0 );
		JL_CHK( cobj ); // Doc: If the property already exists, or cannot be created, JS_DefineObject returns NULL.
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


ALWAYS_INLINE JSBool GetHostObjectValue(JSContext *cx, const char *name, jsval *value) {

	JSObject *cobj = GetHostObject(cx);
	if ( cobj )
		return JS_LookupProperty(cx, cobj, name, value);
	*value = JSVAL_VOID;
	return JS_TRUE;
}


ALWAYS_INLINE JSBool SetHostObjectValue(JSContext *cx, const char *name, jsval value, bool modifiable = true, bool visible = true) {

	JSObject *cobj = GetHostObject(cx);
	if ( cobj )
		return JS_DefineProperty(cx, cobj, name, value, NULL, NULL, JSPROP_ENUMERATE | (modifiable ? 0 : JSPROP_READONLY | JSPROP_PERMANENT) | (visible ? JSPROP_ENUMERATE : 0) );
	return JS_TRUE;
}



///////////////////////////////////////////////////////////////////////////////
// Blob functions


// note: a Blob is either a JSString or a Blob object if the jslang module has been loaded.
//       returned value is equivalent to: var ret = Blob(buffer);
static INLINE JSBool
JL_NewBlob( JSContext *cx, void* buffer, size_t length, jsval *vp ) {

	JL_ASSERT( jl_msize(buffer) >= length + 1 );
	JL_ASSERT( ((uint8_t*)buffer)[length] == 0 );

	if (unlikely( length == 0 || buffer == NULL )) { // Empty Blob must acts like an empty string: !'' === true

		if ( buffer )
			JS_free(cx, buffer);
		*vp = JL_GetEmptyStringValue(cx);
		return JS_TRUE;
	}

	const ClassProtoCache *classProtoCache = JL_GetCachedClassProto(JL_GetHostPrivate(cx), "Blob");

	if (likely( classProtoCache->clasp != NULL )) { // we have Blob class, jslang is present.

		// A blob/string object can be created without using any jslang/blob.h dependances
		JSObject *blob;
//		JSObject *blobProto = JL_PROTOTYPE(cx, Blob);
		blob = JS_ConstructObject(cx, classProtoCache->clasp, classProtoCache->proto, NULL); // need to be constructed else Buffer NativeInterface will not be set !
		JL_CHK( blob );
		*vp = OBJECT_TO_JSVAL(blob);
		JL_S_ASSERT( length <= JSVAL_INT_MAX, "Blob too long." );
		JL_CHK( JL_SetReservedSlot(cx, blob, 0, INT_TO_JSVAL( (jsint)length )) ); // 0 for SLOT_BLOB_LENGTH !!!
		JL_SetPrivate(cx, blob, buffer); // blob data
		return JS_TRUE;
	}

	JSString *jsstr;
	// JS_NewString takes ownership of bytes on success, avoiding a copy; but on error (signified by null return), it leaves bytes owned by the caller.
	// So the caller must free bytes in the error case, if it has no use for them.
	jsstr = JL_NewString(cx, (char*)buffer, length);
	JL_CHK( jsstr );
	buffer = NULL; // see bad:
	*vp = STRING_TO_JSVAL(jsstr); // protect from GC.
	// now we want a string object, not a string literal.
	JSObject *strObj;
	JL_CHK( JS_ValueToObject(cx, STRING_TO_JSVAL(jsstr), &strObj) ); // see. OBJ_DEFAULT_VALUE(cx, obj, JSTYPE_OBJECT, &v)
	*vp = OBJECT_TO_JSVAL(strObj);
	return JS_TRUE;

bad:
	if ( buffer )
		JS_free(cx, buffer); // JS_NewString does not free the buffer on error.
	return JS_FALSE;
}


static ALWAYS_INLINE JSBool
JL_NewBlobCopyN( JSContext *cx, const void *data, size_t amount, jsval *vp ) {

	if (unlikely( amount == 0 || data == NULL )) { // Empty Blob must acts like an empty string: !'' == true

		*vp = JL_GetEmptyStringValue(cx);
		return JS_TRUE;
	}
	// possible optimization: if Blob class is not abailable, copy data into JSString's jschar to avoid js_InflateString.
	uint8_t *blobBuf = (uint8_t*)JS_malloc(cx, amount +1);
	JL_CHK( blobBuf );
	blobBuf[amount] = 0;
	memcpy( blobBuf, data, amount );
	if ( !JL_NewBlob(cx, blobBuf, amount, vp) ) {

		JS_free(cx, blobBuf);
		return JS_FALSE;
	}
	return JS_TRUE;
	JL_BAD;
}



///////////////////////////////////////////////////////////////////////////////
// JS stack management functions


static ALWAYS_INLINE JSStackFrame*
JL_CurrentStackFrame(JSContext *cx) {

	#ifdef DEBUG
		JSStackFrame *fp = NULL;
		JL_ASSERT( JS_FrameIterator(cx, &fp) == js_GetTopStackFrame(cx) ); // Mozilla JS engine private API behavior has changed.
	#endif //DEBUG
	return js_GetTopStackFrame(cx);
}


static ALWAYS_INLINE uint32_t
JL_StackSize(const JSContext *cx, const JSStackFrame *fp) {

	JL_UNUSED(cx);
	uint32_t length = 0;
	for ( ; fp; fp = fp->prev() ) // for ( JSStackFrame *fp = JL_CurrentStackFrame(cx); fp; JS_FrameIterator(cx, &fp) )
		++length;
	return length; // 0 is the first frame
}


static INLINE JSStackFrame*
JL_StackFrameByIndex(JSContext *cx, int frameIndex) {

	JSStackFrame *fp = JL_CurrentStackFrame(cx);
	if ( frameIndex >= 0 ) {

		int currentFrameIndex = JL_StackSize(cx, fp)-1;
		if ( frameIndex > currentFrameIndex )
			return NULL;
		// now, select the right frame
		while ( fp && currentFrameIndex > frameIndex ) {

			fp = fp->prev(); //JS_FrameIterator(cx, &fp);
			--currentFrameIndex;
		}
		return fp;
	}

	while ( fp && frameIndex < 0 ) {

		fp = fp->prev(); //JS_FrameIterator(cx, &fp);
		++frameIndex;
	}
	return fp;
}



///////////////////////////////////////////////////////////////////////////////
// Helper functions


static INLINE JSBool
JL_GetScriptLocation( JSContext *cx, char *location, size_t locationLen ) {

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
	strcpy(location, filename);
	strcat(location, ":");
	ltoa(lineno, location + strlen(location), 10);

	return JS_TRUE;
	JL_BAD;
}


static INLINE JSBool
JL_ExceptionSetScriptLocation( JSContext *cx, JSObject *obj ) {

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


static ALWAYS_INLINE bool
JL_MaybeRealloc( size_t requested, size_t received ) {

	return requested != 0 && (128 * received / requested < 96) && (requested - received > 64); // "128 *": instead using percent, we use per-128
}


static ALWAYS_INLINE JSBool
JL_ThrowOSError(JSContext *cx) {

	char errMsg[1024];
	JLLastSysetmErrorMessage(errMsg, sizeof(errMsg));
	JL_REPORT_ERROR_NUM(cx, JLSMSG_OS_ERROR, errMsg);
bad:
	return JS_FALSE;
}


static ALWAYS_INLINE bool
JL_EngineEnding(const JSContext *cx) {

	return cx->runtime->state == JSRTS_LANDING || cx->runtime->state == JSRTS_DOWN; // could be replaced by a flag in HostPrivate that keep the state of the engine.
}


// eg. JS_NewObject(cx, JL_GetStandardClassByKey(cx, JSProto_TypeError), NULL, NULL);
static ALWAYS_INLINE JSClass*
JL_GetStandardClassByKey(JSContext *cx, JSProtoKey key) {

	JSObject *ctor;
	JL_CHK( JS_GetClassObject(cx, JL_GetGlobalObject(cx), key, &ctor) );
	JL_CHK( ctor );
	return js::Jsvalify(FUN_CLASP(GET_FUNCTION_PRIVATE(cx, ctor))); // FUN_CLASP( JS_ValueToFunction(cx, OBJECT_TO_JSVAL(constructor)) );
bad:
	return NULL;
}


static ALWAYS_INLINE JSContext*
JL_GetFirstContext(JSRuntime *rt) {

	JSContext *cx = NULL;
	JL_ASSERT( rt != NULL );
	JS_ContextIterator(rt, &cx);
	JS_ASSERT( cx != NULL );
	return cx;
}


static ALWAYS_INLINE bool
JL_InheritFrom( JSContext *cx, JSObject *obj, const JSClass *clasp ) {

	JL_UNUSED(cx);

	JSObject *proto;
	while ( obj != NULL ) {

		if ( JL_GetClass(obj) == clasp )
			return true;
//		obj = JS_GetPrototype(cx, obj);
		proto = obj->getProto();
		obj = proto && proto->map ? proto : NULL;
	}
	return false;
}


static ALWAYS_INLINE JSBool
JL_CallFunctionId(JSContext *cx, JSObject *obj, jsid id, uintN argc, jsval *argv, jsval *rval) {

	js::AutoValueRooter tvr(cx);
	return JS_GetMethodById(cx, obj, id, NULL, tvr.jsval_addr()) && JS_CallFunctionValue(cx, obj, tvr.jsval_value(), argc, argv, rval);
// (TBD) choose the best
//	jsval val;
//	return JL_JsidToJsval(cx, id, &val) && JS_CallFunctionValue(cx, obj, val, argc, argv, rval);
}


static INLINE JSBool
JL_CallFunctionVA( JSContext *cx, JSObject *obj, const jsval &functionValue, jsval *rval, uintN argc, ... ) {

	va_list ap;
	jsval *argv = (jsval*)alloca((argc+1)*sizeof(jsval));
	va_start(ap, argc);
	for ( uintN i = 1; i <= argc; i++ )
		argv[i] = va_arg(ap, jsval);
	va_end(ap);
	js::AutoArrayRooter tvr(cx, argc+1, argv);
	argv[0] = JSVAL_NULL; // the rval
	JL_S_ASSERT_FUNCTION( functionValue );
	JSBool st;
	st = JS_CallFunctionValue(cx, obj, functionValue, argc, argv+1, argv); // NULL is NOT supported for &rvalTmp ( last arg of JS_CallFunctionValue )
	JL_CHK( st );
	if ( rval != NULL )
		*rval = argv[0];
	return JS_TRUE;
	JL_BAD;
}


static INLINE JSBool
JL_CallFunctionNameVA( JSContext *cx, JSObject *obj, const char* functionName, jsval *rval, uintN argc, ... ) {

	va_list ap;
	jsval *argv = (jsval*)alloca((argc+1)*sizeof(jsval));
	va_start(ap, argc);
	for ( uintN i = 1; i <= argc; i++ )
		argv[i] = va_arg(ap, jsval);
	va_end(ap);
	js::AutoArrayRooter tvr(cx, argc+1, argv);
	argv[0] = JSVAL_NULL; // the rval
	JSBool st = JS_CallFunctionName(cx, obj, functionName, argc, argv+1, argv); // NULL is NOT supported for &rvalTmp ( last arg of JS_CallFunctionValue )
	JL_CHK( st );
	if ( rval != NULL )
		*rval = argv[0];
	return JS_TRUE;
	JL_BAD;
}

static INLINE JSBool
JL_Eval( JSContext *cx, JSString *source, jsval *rval ) {

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

	size_t length;
	const jschar *chars;
	chars = JS_GetStringCharsAndLength(source, &length);
	return JS_EvaluateUCScript(cx, JS_GetScopeChain(cx), chars, length, filename, lineno, rval);
	JL_BAD;
}

static INLINE JSBool
JL_Push( JSContext *cx, JSObject *arr, jsval *value ) {

	jsuint length;
	JL_CHK( JS_GetArrayLength(cx, arr, &length) );
	JL_CHK( JS_SetPropertyById(cx, arr, INT_TO_JSID(length), value) ); //JL_CHK( JS_SetElement(cx, arrObj, length, value) );
	JL_CHK( JS_SetArrayLength(cx, arr, length+1) );
	return JS_TRUE;
	JL_BAD;
}


static INLINE JSBool
JL_Pop( JSContext *cx, JSObject *arr, jsval *vp ) {

	jsuint length;
	JL_CHK( JS_GetArrayLength(cx, arr, &length) );
	--length;
	JL_CHK( JS_GetPropertyById(cx, arr, INT_TO_JSID(length), vp) ); //JL_CHK( JS_GetElement(cx, arrObj, length, vp) );
	JL_CHK( JS_SetArrayLength(cx, arr, length) );
	return JS_TRUE;
	JL_BAD;
}


static INLINE JSBool
JL_JsvalToPrimitive( JSContext *cx, const jsval &val, jsval *rval ) { // prev JL_ValueOf

	if ( JSVAL_IS_PRIMITIVE(val) ) {

		*rval = val;
		return JS_TRUE;
	}
	JSObject *obj = JSVAL_TO_OBJECT(val);
	if (unlikely( JL_ObjectIsXML(cx, obj) ))
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
static INLINE JSScript*
JL_LoadScript(JSContext *cx, JSObject *obj, const char *fileName, bool useCompFile, bool saveCompFile) {

	JSScript *script = NULL;
	void *data = NULL;
	uint32 prevOpts = JS_GetOptions(cx);

	char compiledFileName[PATH_MAX];
	strcpy( compiledFileName, fileName );
	strcat( compiledFileName, "xdr" );

	struct stat srcFileStat, compFileStat;
	bool hasSrcFile = stat(fileName, &srcFileStat) != -1; // errno == ENOENT
	bool hasCompFile = stat(compiledFileName, &compFileStat) != -1;
	bool compFileUpToDate = ( hasCompFile && !hasSrcFile ) || ( hasCompFile && hasSrcFile && (compFileStat.st_mtime > srcFileStat.st_mtime) ); // true if comp file is up to date or alone

	JL_CHKM( hasSrcFile || hasCompFile, "Unable to load Script, file \"%s\" or \"%s\" not found.", fileName, compiledFileName );

	if ( useCompFile && compFileUpToDate ) {

		int file = open(compiledFileName, O_RDONLY | O_BINARY | O_SEQUENTIAL);
		JL_CHKM( file != -1, "Unable to open file \"%s\" for reading.", compiledFileName );

		size_t compFileSize = compFileStat.st_size; // filelength(file); ?
		data = jl_malloc(compFileSize); // (TBD) free on error
		int readCount = read( file, data, jl::SafeCast<unsigned int>(compFileSize) ); // here we can use "Memory-Mapped I/O Functions" ( http://developer.mozilla.org/en/docs/NSPR_API_Reference:I/O_Functions#Memory-Mapped_I.2FO_Functions )
		JL_CHKM( readCount >= 0 && (size_t)readCount == compFileSize, "Unable to read the file \"%s\" ", compiledFileName );
		close( file );

		JSXDRState *xdr = JS_XDRNewMem(cx, JSXDR_DECODE);
		JL_CHK( xdr );
		JS_XDRMemSetData(xdr, data, jl::SafeCast<uint32>(compFileSize));

		// we want silent failures.
		JSErrorReporter prevErrorReporter = JS_SetErrorReporter(cx, NULL);
		JSDebugErrorHook debugErrorHook = cx->debugHooks->debugErrorHook;
		void *debugErrorHookData = cx->debugHooks->debugErrorHookData;
		JS_SetDebugErrorHook(JL_GetRuntime(cx), NULL, NULL);
		JSBool status = JS_XDRScript(xdr, &script);
		JS_SetDebugErrorHook(JL_GetRuntime(cx), debugErrorHook, debugErrorHookData);
		if (cx->lastMessage)
			JS_free(cx, cx->lastMessage);
		cx->lastMessage = NULL;
		JS_SetErrorReporter(cx, prevErrorReporter);

		if ( status == JS_TRUE ) {

//			*script->notes() = 0;
			// (TBD) manage BIG_ENDIAN here ?
			JS_XDRMemSetData(xdr, NULL, 0);
			JS_XDRDestroy(xdr);
			jl_free(data);
			data = NULL;
			if ( JS_GetScriptVersion(cx, script) < JS_GetVersion(cx) )
				JL_REPORT_WARNING("Trying to xdr-decode an old script (%s).", compiledFileName);
			goto good;
		} else {

			jl_free(data);
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
	JL_CHKM( scriptFile != NULL, "Script file \"%s\" cannot be opened.", fileName );

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
	JL_CHKM( scriptFile >= 0, "Unable to open file \"%s\" for reading.", fileName );

	size_t scriptFileSize;
	scriptFileSize = lseek(scriptFile, 0, SEEK_END);
	JL_S_ASSERT( scriptFileSize <= UINT_MAX, "Compiled file too big." ); // see read()
	lseek(scriptFile, 0, SEEK_SET); // see tell(scriptFile);
	char *scriptBuffer;
	scriptBuffer = (char*)alloca(scriptFileSize);
	int res;
	res = read(scriptFile, (void*)scriptBuffer, (unsigned int)scriptFileSize);
	close(scriptFile);
	JL_CHKM( res >= 0, "Unable to read file \"%s\".", fileName );
	JL_ASSERT( (size_t)res == scriptFileSize );
	scriptFileSize = (size_t)res;

	JLEncodingType enc;
	enc = JLDetectEncoding(&scriptBuffer, &scriptFileSize);
	if ( enc == ASCII ) {

		char *scriptText = scriptBuffer;
		size_t scriptTextLength = scriptFileSize;
		if ( scriptText[0] == '#' && scriptText[1] == '!' ) { // shebang support

			scriptText[0] = '/';
			scriptText[1] = '/';
		}
		script = JS_CompileScript(cx, obj, scriptText, scriptTextLength, fileName, 1);
	} else
	if ( enc == UTF16le ) { // (TBD) support big-endian

		jschar *scriptText = (jschar*)scriptBuffer;
		size_t scriptTextLength = scriptFileSize / 2;
		if ( scriptText[0] == '#' && scriptText[1] == '!' ) { // shebang support

			scriptText[0] = '/';
			scriptText[1] = '/';
		}
		script = JS_CompileUCScript(cx, obj, scriptText, scriptTextLength, fileName, 1);
	} else
	if ( enc == UTF8 ) { // (TBD) check if JS_DecodeBytes does the right things

		jschar *scriptText = (jschar*)alloca(scriptFileSize * 2);
		size_t scriptTextLength = scriptFileSize * 2;
		JL_CHKM( UTF8ToUTF16LE((unsigned char*)scriptText, &scriptTextLength, (unsigned char*)scriptBuffer, &scriptFileSize) >= 0, "Unable do decode UTF8 data." );
		if ( scriptText[0] == '#' && scriptText[1] == '!' ) { // shebang support

			scriptText[0] = '/';
			scriptText[1] = '/';
		}
		script = JS_CompileUCScript(cx, obj, scriptText, scriptTextLength, fileName, 1);
	}

	JL_CHKM( script, "Unable to compile the script \"%s\".", fileName );

#endif //JL_UC

	if ( !saveCompFile )
		goto good;

	int file;
	file = open(compiledFileName, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY | O_SEQUENTIAL, srcFileStat.st_mode); // (TBD) check the mode
	if ( file == -1 ) // if the file cannot be write, this is not an error ( eg. read-only drive )
		goto good;

	JSXDRState *xdr;
	xdr = JS_XDRNewMem(cx, JSXDR_ENCODE);
	JL_CHK( xdr );
	JL_CHK( JS_XDRScript(xdr, &script) );

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
	JS_SetOptions(cx, prevOpts);
	return script;

bad:
	JS_SetOptions(cx, prevOpts);
	if ( data )
		jl_free(data);
	return NULL; // report a warning ?
}



///////////////////////////////////////////////////////////////////////////////
// NativeInterface

static ALWAYS_INLINE JSBool
ReserveNativeInterface( JSContext *cx, JSObject *obj, const jsid id ) {

	return JS_DefinePropertyById(cx, obj, id, JSVAL_VOID, NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
}

static ALWAYS_INLINE JSBool
SetNativeInterface( JSContext *cx, JSObject *obj, const jsid &id, const void *nativeFct ) {

	if ( nativeFct != NULL ) {

		JL_CHK( JS_DefinePropertyById(cx, obj, id, JSVAL_TRUE, NULL, (JSPropertyOp)nativeFct, JSPROP_READONLY | JSPROP_PERMANENT) );
	} else {

		JL_CHK( JS_DeletePropertyById(cx, obj, id) );
		JL_CHK( ReserveNativeInterface(cx, obj, id) );
	}
	return JS_TRUE;
	JL_BAD;
}

static ALWAYS_INLINE JSBool
GetNativeInterface( JSContext *cx, JSObject *obj, jsid iid, void **nativeFct ) {

	uintN attrs;
	JSBool found;
	JSPropertyOp getter, setter;
	JL_CHK( JS_GetPropertyAttrsGetterAndSetterById(cx, obj, iid, &attrs, &found, &getter, &setter) );
	*nativeFct = setter;
	return JS_TRUE;
	JL_BAD;
}



///////////////////////////////////////////////////////////////////////////////
// NativeInterface StreamRead


inline JSBool
JSStreamRead( JSContext *cx, JSObject *obj, char *buffer, size_t *amount ) {

	js::AutoValueRooter tvr(cx);

	JL_S_ASSERT( *amount < INT_MAX, "Too many data." );
	JL_CHK( JL_NativeToJsval(cx, (int)*amount, tvr.jsval_addr()) );
	JL_CHKM( JL_CallFunctionId(cx, obj, JLID(cx, Read), 1, tvr.jsval_addr(), tvr.jsval_addr()), "Read() function not found.");

	if ( tvr.value().isUndefined() ) { // (TBD)! with sockets, undefined mean 'closed', that is not supported.

		*amount = 0;
		return JS_TRUE;
	}

	{
	JLStr str;
	JL_CHK( JL_JsvalToNative(cx, *tvr.jsval_addr(), &str) );
	*amount = str.Length();
	memcpy(buffer, str.GetConstStr(), *amount);
	}

	return JS_TRUE;

bad:
	return JS_FALSE;
}


inline JSBool
ReserveStreamReadInterface( JSContext *cx, JSObject *obj ) {

	return ReserveNativeInterface(cx, obj, JLID(cx, _NI_StreamRead) );
}

inline JSBool
SetStreamReadInterface( JSContext *cx, JSObject *obj, NIStreamRead pFct ) {

	return SetNativeInterface( cx, obj, JLID(cx, _NI_StreamRead), (void*)pFct );
}

inline NIStreamRead
StreamReadNativeInterface( JSContext *cx, JSObject *obj ) {

	void *streamRead;
	jsid propId = JLID(cx, _NI_StreamRead);
	if ( propId == JL_NullJsid() || GetNativeInterface( cx, obj, propId, &streamRead ) != JS_TRUE )
		return NULL;
	return (NIStreamRead)streamRead;
}

inline NIStreamRead
StreamReadInterface( JSContext *cx, JSObject *obj ) {

	void *fct = (void*)StreamReadNativeInterface(cx, obj);
	if ( fct )
		return (NIStreamRead)fct;
	jsval res;
//	if ( obj->getProperty(cx, JLID(cx, Read), &res) != JS_TRUE || !JsvalIsFunction(cx, res) )
//		return NULL;
	if ( JS_GetPropertyById(cx, obj, JLID(cx, Read), &res) != JS_TRUE || !JL_JsvalIsFunction(cx, res) )
		return NULL;
	return JSStreamRead;
}



///////////////////////////////////////////////////////////////////////////////
// NativeInterface BufferGet

inline JSBool
JSBufferGet( JSContext *cx, JSObject *obj, JLStr *str ) {

	js::AutoValueRooter tvr(cx); // use AutoArrayRooter instead ?

//	JS_GetMethodById(cx, obj, JLID(cx, Get), NULL, &tvr.u.value);
//	JS_CallFunctionValue(cx, obj, tvr.u.value, 0, NULL, &tvr.u.value);

//	JL_CHKM( JS_CallFunctionName(cx, obj, "Get", 0, NULL, &tvr.u.value), "Get() function not found."); // do not use toString() !? no !
	JL_CHKM( JL_CallFunctionId(cx, obj, JLID(cx, Get), 0, NULL, tvr.jsval_addr()), "Get() function not found.");
//	JL_CHK( JL_JsvalToStringAndLength(cx, tvr.jsval_addr(), buffer, size) ); // (TBD) GC warning, when tvr.u.value will be no more protected, the buffer will be unprotected.
	JL_CHK( JL_JsvalToNative(cx, *tvr.jsval_addr(), str) );
	return JS_TRUE;
	JL_BAD;
}

inline JSBool
ReserveBufferGetInterface( JSContext *cx, JSObject *obj ) {

	return ReserveNativeInterface(cx, obj, JLID(cx, _NI_BufferGet) );
}

inline JSBool
SetBufferGetInterface( JSContext *cx, JSObject *obj, NIBufferGet pFct ) {

	return SetNativeInterface( cx, obj, JLID(cx, _NI_BufferGet), (void*)pFct );
}

inline NIBufferGet
BufferGetNativeInterface( JSContext *cx, JSObject *obj ) {

	void *fct;
	jsid propId = JLID(cx, _NI_BufferGet);
	JL_ASSERT( propId != JL_NullJsid() );
	if ( GetNativeInterface( cx, obj, propId, &fct ) != JS_TRUE )
		return NULL;
	return (NIBufferGet)fct;
}

inline NIBufferGet
BufferGetInterface( JSContext *cx, JSObject *obj ) {

	void *fct = (void*)BufferGetNativeInterface(cx, obj);
	if ( fct )
		return (NIBufferGet)fct;

	jsval res;
//	if ( obj->getProperty(cx, JLID(cx, Get), &res) != JS_TRUE || !JsvalIsFunction(cx, res) ) // do not use toString() directly, but Get can call toString().
//		return NULL;
	if ( JS_GetPropertyById(cx, obj, JLID(cx, Get), &res) != JS_TRUE || !JL_JsvalIsFunction(cx, res) ) // do not use toString() directly, but Get can call toString().
		return NULL;

	return JSBufferGet;
}


///////////////////////////////////////////////////////////////////////////////
// NativeInterface Matrix44Get

/*
inline JSBool JSMatrix44Get( JSContext *cx, JSObject *obj, const char **buffer, unsigned int *size ) {


	JS_PUSH_SINGLE_TEMP_ROOT(cx, rval, &tvr);
	&tvr.u.value
	...

	JL_CHKM( JS_CallFunctionName(cx, obj, "Get", 0, NULL, &rval), "Get() function not found."); // do not use toString() !?
	JL_CHK( JL_JsvalToStringAndLength(cx, rval, buffer, size) );
	return JS_TRUE;
	JL_BAD;
}
*/

inline JSBool
ReserveMatrix44GetInterface( JSContext *cx, JSObject *obj ) {

	return ReserveNativeInterface(cx, obj, JLID(cx, _NI_Matrix44Get) );
}

inline JSBool
SetMatrix44GetInterface( JSContext *cx, JSObject *obj, NIMatrix44Get pFct ) {

	return SetNativeInterface( cx, obj, JLID(cx, _NI_Matrix44Get), (void*)pFct );
}

inline NIMatrix44Get
Matrix44GetNativeInterface( JSContext *cx, JSObject *obj ) {

	void *fct;
	jsid propId = JLID(cx, _NI_Matrix44Get);
	if ( propId == JL_NullJsid() || GetNativeInterface( cx, obj, propId, &fct ) != JS_TRUE )
		return NULL;
	return (NIMatrix44Get)fct;
}

inline NIMatrix44Get
Matrix44GetInterface( JSContext *cx, JSObject *obj ) {

	void *fct = (void*)Matrix44GetNativeInterface(cx, obj);
	if ( fct )
		return (NIMatrix44Get)fct;

/*
	jsval res;
	jsid propId = JL_GetPrivateJsid(cx, JL_GetHostPrivate(cx), "GetMatrix", PRIVATE_JSID_GetMatrix);
	if ( obj->getProperty(cx, propId, &res) != JS_TRUE != JS_TRUE || !JsvalIsFunction(cx, res) )
		return NULL;
	return JSMatrix44Get;
*/
	return NULL;
}



///////////////////////////////////////////////////////////////////////////////
// jsval -> matrix 4x4

static INLINE JSBool
JL_JsvalToMatrix44( JSContext *cx, jsval &val, float **m ) {

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

	JL_S_ASSERT_OBJECT(val);

	JSObject *matrixObj;
	matrixObj = JSVAL_TO_OBJECT(val);

	NIMatrix44Get Matrix44Get;
	Matrix44Get = Matrix44GetInterface(cx, matrixObj);
	if ( Matrix44Get )
		return Matrix44Get(cx, matrixObj, m);

	if ( JS_IsArrayObject(cx, matrixObj) ) {

		uint32 length;
		jsval element;
		JL_CHK( JS_GetElement(cx, JSVAL_TO_OBJECT(val), 0, &element) );
		if ( JL_JsvalIsArray(cx, element) ) { // support for [ [1,1,1,1], [2,2,2,2], [3,3,3,3], [4,4,4,4] ] matrix

			JL_CHK( JL_JsvalToCValVector(cx, element, (*m)+0, 4, &length ) );
			JL_S_ASSERT( length == 4, "Too few (%d) elements in the array.", length );

			JL_CHK( JS_GetElement(cx, JSVAL_TO_OBJECT(val), 1, &element) );
			JL_S_ASSERT_ARRAY( element );
			JL_CHK( JL_JsvalToCValVector(cx, element, (*m)+4, 4, &length ) );
			JL_S_ASSERT( length == 4, "Too few (%d) elements in the array.", length );

			JL_CHK( JS_GetElement(cx, JSVAL_TO_OBJECT(val), 2, &element) );
			JL_S_ASSERT_ARRAY( element );
			JL_CHK( JL_JsvalToCValVector(cx, element, (*m)+8, 4, &length ) );
			JL_S_ASSERT( length == 4, "Too few (%d) elements in the array.", length );

			JL_CHK( JS_GetElement(cx, JSVAL_TO_OBJECT(val), 3, &element) );
			JL_S_ASSERT_ARRAY( element );
			JL_CHK( JL_JsvalToCValVector(cx, element, (*m)+12, 4, &length ) );
			JL_S_ASSERT( length == 4, "Too few (%d) elements in the array.", length );
			return JS_TRUE;
		}

		JL_CHK( JL_JsvalToCValVector(cx, val, *m, 16, &length ) );  // support for [ 1,1,1,1, 2,2,2,2, 3,3,3,3, 4,4,4,4 ] matrix
		JL_S_ASSERT( length == 16, "Too few (%d) elements in the array.", length );
		return JS_TRUE;
	}

	JL_REPORT_ERROR("Unable to read matrix44.");
	JL_BAD;
}



///////////////////////////////////////////////////////////////////////////////
// ProcessEvent

struct ProcessEvent {
	void (*startWait)( volatile ProcessEvent *self ); // starts the blocking thread and call signalEvent() when an event has arrived.
	bool (*cancelWait)( volatile ProcessEvent *self ); // unlock the blocking thread event if no event has arrived (mean that an event has arrived in another thread).
	JSBool (*endWait)( volatile ProcessEvent *self, bool *hasEvent, JSContext *cx, JSObject *obj ); // process the result
};



///////////////////////////////////////////////////////////////////////////////
// memory management

namespace jl {

	class _NOVTABLE CppAllocators {
	public:
		ALWAYS_INLINE void* operator new(size_t size) _NOTHROW {
			return jl_malloc(size);
		}
		ALWAYS_INLINE void* operator new[](size_t size) _NOTHROW {
			return jl_malloc(size);
		}
		ALWAYS_INLINE void operator delete(void *ptr, size_t size) {
			jl_free(ptr);
			JL_UNUSED(size);
		}
		ALWAYS_INLINE void operator delete[](void *ptr, size_t size) {
			jl_free(ptr);
			JL_UNUSED(size);
		}
	};


	class _NOVTABLE DefaultAlloc {
	public:
		ALWAYS_INLINE void Free(void *ptr) {
			jl_free(ptr);
		}
		ALWAYS_INLINE void* Alloc(size_t size) {
			return jl_malloc(size);
		}
		ALWAYS_INLINE void* Realloc(void *ptr, size_t size) {
			return jl_realloc(ptr, size);
		}
	};


	template <const size_t PREALLOC = 0>
	class _NOVTABLE PreservAlloc : private DefaultAlloc {

		void *_last;
		uint8_t *_prealloc;
		uint8_t *_preallocEnd;

	public:
		ALWAYS_INLINE PreservAlloc() : _last(NULL), _prealloc(NULL) {
		}

		ALWAYS_INLINE ~PreservAlloc() {

			while ( _last != NULL ) {

				void *tmp = _last;
				_last = *(void**)_last;
				if ( PREALLOC == 0 || tmp > _preallocEnd || tmp < _prealloc ) // do not free preallocated memory
					DefaultAlloc::Free(tmp);
			}
			if ( PREALLOC > 0 )
				DefaultAlloc::Free(_prealloc);
		}

		ALWAYS_INLINE void Free(void *ptr) {

			*(void**)ptr = _last;
			_last = ptr;
		}

		ALWAYS_INLINE void* Alloc(size_t size) {

			if ( size < sizeof(void*) )
				size = sizeof(void*);

			if ( PREALLOC > 0 && _prealloc == NULL ) {

				_prealloc = (uint8_t*)DefaultAlloc::Alloc(PREALLOC * size);
				_preallocEnd = _prealloc + PREALLOC * size;
				for ( uint8_t *it = _prealloc; it != _preallocEnd; it += size ) {

					*(void**)it = _last;
					_last = it;
				}
			}

			if ( _last != NULL ) {

				void *tmp = _last;
				_last = *(void**)_last;
				return tmp;
			}
			return DefaultAlloc::Alloc(size);
		}

		ALWAYS_INLINE void* Realloc(void *ptr, size_t size) {

			if ( size < sizeof(void*) )
				size = sizeof(void*);
			return DefaultAlloc::Realloc(ptr, size);
		}
	};


	template <const size_t PREALLOC_SIZE = 1024>
	class _NOVTABLE StaticAlloc : private DefaultAlloc {

		void *_last;
		uint8_t *_preallocEnd;

		uint8_t _prealloc[PREALLOC_SIZE];
#ifdef DEBUG
		size_t _dbg_size;
#endif

	public:
		ALWAYS_INLINE StaticAlloc() : _last(NULL), _preallocEnd(NULL) {

#ifdef DEBUG
			_dbg_size = 0;
#endif
		}

		ALWAYS_INLINE ~StaticAlloc() {

			while ( _last != NULL ) {

				void *tmp = _last;
				_last = *(void**)_last;
				if ( _preallocEnd == NULL || tmp > _preallocEnd || tmp < _prealloc ) // do not free preallocated memory
					DefaultAlloc::Free(tmp);
			}
		}

		ALWAYS_INLINE void Free(void *ptr) {

			*(void**)ptr = _last;
			_last = ptr;
		}

		ALWAYS_INLINE void* Alloc(size_t size) {

#ifdef DEBUG
			JL_ASSERT( size != 0 );
			if ( _dbg_size == 0 )
				_dbg_size = size;
			JL_ASSERT( size == _dbg_size );
#endif

			if ( size < sizeof(void*) )
				size = sizeof(void*);

			if ( _preallocEnd == NULL ) {

				_preallocEnd = _prealloc + (sizeof(_prealloc)/size)*size;
				for ( uint8_t *it = _prealloc; it < _preallocEnd; it += size ) {

					*(void**)it = _last;
					_last = it;
				}
			}
			if ( _last != NULL ) {

				void *tmp = _last;
				_last = *(void**)_last;
				return tmp;
			}

			return DefaultAlloc::Alloc(size);
		}

		ALWAYS_INLINE void* Realloc(void *ptr, size_t size) {

			if ( size < sizeof(void*) )
				size = sizeof(void*);

			return DefaultAlloc::Realloc(ptr, size);
		}
	};
}

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
//		JL_ASSERT( _shared );
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
static ALWAYS_INLINE JSBool JL_GetVariableValue( JSContext *cx, const char *name, jsval *vp ) {

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

static ALWAYS_INLINE bool JL_IsSerializable( jsval val ) {

	if ( JSVAL_IS_PRIMITIVE(val) )
		return true;
	JSClass *cl = JL_GetClass(JSVAL_TO_OBJECT(val));
	return cl->xdrObject != NULL;
}

static ALWAYS_INLINE void JL_SerializerCreate( Serialized *xdr ) {

	*xdr = NULL;
}

static ALWAYS_INLINE void JL_SerializerFree( Serialized *xdr ) {

	if ( *xdr != NULL ) {

		JS_XDRDestroy(*xdr);
//		JS_XDRMemSetData(*xdr, NULL, 0);
		*xdr = NULL;
	}
}

static ALWAYS_INLINE bool JL_SerializerIsEmpty( const Serialized *xdr ) {

	return *xdr == NULL;
}

static ALWAYS_INLINE JSBool JL_SerializeJsval( JSContext *cx, Serialized *xdr, jsval *val ) {

	if ( *xdr != NULL )
		SerializerFree(xdr);
	*xdr = JS_XDRNewMem(cx, JSXDR_ENCODE);
	JL_S_ASSERT( *xdr != NULL, "Unable to create the serializer." );
	JL_CHK( JS_XDRValue(*xdr, val) );
	return JS_TRUE;
	JL_BAD;
}

static ALWAYS_INLINE JSBool JL_UnserializeJsval( JSContext *cx, const Serialized *xdr, jsval *rval ) {

	JSXDRState *xdrDecoder = JS_XDRNewMem(cx, JSXDR_DECODE);
	JL_S_ASSERT( xdrDecoder != NULL, "Unable to create the unserializer." );
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
		JL_ASSERT(_start);
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
			JL_ASSERT(_start);
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
		if ( JL_JsvalIsFunction(cx, fctVal) ) {

		}
	}
	return JS_TRUE;
	JL_BAD;
}
*/



#endif // _JSHELPER_H_
