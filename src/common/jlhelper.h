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


#ifdef _MSC_VER
#pragma warning( push, 0 )
#pragma warning(disable : 4800) // 'XXX' : forcing value to bool 'true' or 'false' (performance warning)

#endif // _MSC_VER

#include <jsapi.h>
#include <jsfriendapi.h>
#include <jsdbgapi.h>
#include "jsatom.h"

#include <jstypedarray.h>

#include "polyfill.h"

#ifdef _MSC_VER
#pragma warning( pop )
#endif // _MSC_VER


#include <sys/stat.h> // stat() used by JL_LoadScript()


extern int _unsafeMode;

class JLData;

ALWAYS_INLINE bool FASTCALL
JL_NewEmptyBuffer( JSContext *cx, jsval *rval );

ALWAYS_INLINE bool FASTCALL
JL_NewBufferGetOwnership( JSContext *cx, void *src, size_t nbytes, jsval *rval );

ALWAYS_INLINE bool FASTCALL
JL_MaybeRealloc( size_t requested, size_t received );

typedef JSBool (*NIStreamRead)( JSContext *cx, JSObject *obj, char *buffer, size_t *amount );
typedef JSBool (*NIBufferGet)( JSContext *cx, JSObject *obj, JLData *str );
typedef JSBool (*NIMatrix44Get)( JSContext *cx, JSObject *obj, float **pm );

ALWAYS_INLINE NIBufferGet
BufferGetNativeInterface( JSContext *cx, JSObject *obj );

ALWAYS_INLINE NIBufferGet
BufferGetInterface( JSContext *cx, JSObject *obj );

ALWAYS_INLINE NIMatrix44Get
Matrix44GetInterface( JSContext *cx, JSObject *obj );

ALWAYS_INLINE JSBool
SetBufferGetInterface( JSContext *cx, JSObject *obj, NIBufferGet pFct );


///////////////////////////////////////////////////////////////////////////////
// js string handling

JS_ALWAYS_INLINE size_t FASTCALL
js_strlen(const jschar *s)
{
    const jschar *t;

    for (t = s; *t != 0; t++)
        continue;
    return (size_t)(t - s);
}

JS_ALWAYS_INLINE void FASTCALL
js_strncpy(jschar *dst, const jschar *src, size_t nelem)
{
    memcpy(dst, src, nelem * sizeof(jschar));
}

ALWAYS_INLINE jschar * FASTCALL
js_strchr(const jschar *s, jschar c)
{
    while (*s != 0) {
        if (*s == c)
            return (jschar *)s;
        s++;
    }
    return NULL;
}

ALWAYS_INLINE jschar * FASTCALL
js_strchr_limit(const jschar *s, jschar c, const jschar *limit)
{
    while (s < limit) {
        if (*s == c)
            return (jschar *)s;
        s++;
    }
    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// helper macros to avoid a function call to the jsapi

ALWAYS_INLINE JSRuntime* FASTCALL
JL_GetRuntime( JSContext *cx ) {

	return js::GetRuntime(cx); // jsfriendapi
}

ALWAYS_INLINE void* FASTCALL
JL_GetRuntimePrivate( JSRuntime *rt ) {

#ifdef JSRUNTIME_HAS_JLDATA
	return js::RuntimeFriendFields::get(rt)->jldata;
#else
	return JS_GetRuntimePrivate(rt);
#endif
}

ALWAYS_INLINE void FASTCALL
JL_SetRuntimePrivate( JSRuntime *rt, void *data ) {

#ifdef JSRUNTIME_HAS_JLDATA
	const_cast<js::RuntimeFriendFields*>(js::RuntimeFriendFields::get(rt))->jldata = data;
#else
	JS_SetRuntimePrivate(rt, data);
#endif
}

ALWAYS_INLINE void FASTCALL
JL_updateMallocCounter( JSContext *cx, size_t nbytes ) {

	JS_updateMallocCounter(cx, nbytes);
}

ALWAYS_INLINE JSObject * FASTCALL
JL_GetGlobal( JSContext *cx ) {
	
	return JS_GetGlobalForScopeChain(cx); // JS_GetGlobalForObject(cx, obj); ?  // JS_GetGlobalObject(cx):Obsolete;
}

ALWAYS_INLINE JSBool FASTCALL
JL_IsExceptionPending( JSContext *cx ) {
	
	return JS_IsExceptionPending(cx);
}

ALWAYS_INLINE JSBool FASTCALL
JL_NewNumberValue( JSContext *cx, double d, jsval *rval ) {

	JL_IGNORE(cx);
	ASSERT( JS_NewNumberValue(cx, d, rval) ); // AssertNoGC(cx);
    d = JS_CANONICALIZE_NAN(d);
    rval->setNumber(d);
    return JS_TRUE;
}

ALWAYS_INLINE jsval FASTCALL
JL_GetNaNValue( JSContext *cx ) {

	return JS_GetNaNValue(cx);
}

ALWAYS_INLINE JSClass* FASTCALL
JL_GetClass( JSObject *obj ) {

	return js::GetObjectJSClass(obj); // jsfriendapi
}

ALWAYS_INLINE const char * FASTCALL
JL_GetClassName( JSObject *obj ) {

	return JL_GetClass(obj)->name;
}

ALWAYS_INLINE size_t FASTCALL
JL_GetStringLength( JSString *jsstr ) {

	return JS_GetStringLength(jsstr);
}

ALWAYS_INLINE jsval FASTCALL
JL_GetEmptyStringValue( JSContext *cx ) {

	return JS_GetEmptyStringValue(cx);
}

ALWAYS_INLINE bool FASTCALL
JL_HasPrivate( JSObject *obj ) {

	return !!(JL_GetClass(obj)->flags & JSCLASS_HAS_PRIVATE);
}

ALWAYS_INLINE void* FASTCALL
JL_GetPrivate( JSObject *obj ) {

	ASSERT( JS_IsNative(obj) );
	ASSERT( JL_HasPrivate(obj) );
	return js::GetObjectPrivate(obj); // jsfriendapi
}

ALWAYS_INLINE void FASTCALL
JL_SetPrivate( JSContext *cx, JSObject *obj, void *data ) {

	JL_IGNORE(cx);
	ASSERT( JS_IsNative(obj) );
	ASSERT( JL_HasPrivate(obj) );
	JS_SetPrivate(obj, data);
}

ALWAYS_INLINE JSObject* FASTCALL
JL_GetPrototype(JSContext *cx, JSObject *obj) {

	JL_IGNORE(cx);
	return js::GetObjectProto(obj); // jsfriendapi
}

ALWAYS_INLINE JSObject* FASTCALL
JL_GetConstructor(JSContext *cx, JSObject *obj) {

	return JS_GetConstructor(cx, obj);
}

ALWAYS_INLINE JSObject* FASTCALL
JL_GetParent(JSContext *cx, JSObject *obj) {
	
	JL_IGNORE(cx);
	return js::GetObjectParent(obj);
}

ALWAYS_INLINE JSBool FASTCALL
JL_GetClassPrototype(JSContext *cx, JSObject *scopeobj, JSProtoKey protoKey, JSObject **protop) {

	return js_GetClassPrototype(cx, scopeobj, protoKey, protop);
}

ALWAYS_INLINE JSClass* FASTCALL
JL_GetErrorJSClassJSClassByProtoKey( JSContext *cx, JSProtoKey protoKey, JSObject *parent ) {

	JSObject *proto;
	if ( !JL_GetClassPrototype(cx, parent, protoKey, &proto) )
		return NULL;
	return JL_GetClass(proto);
}

ALWAYS_INLINE JSBool FASTCALL
JL_GetElement(JSContext *cx, JSObject *obj, unsigned index, jsval *vp) {

	return JS_ForwardGetElementTo(cx, obj, index, obj, vp);
}

ALWAYS_INLINE JSBool FASTCALL
JL_SetElement(JSContext *cx, JSObject *obj, unsigned index, jsval *vp) {

	return JS_SetElement(cx, obj, index, vp);
}


ALWAYS_INLINE JSBool FASTCALL
JL_GetReservedSlot( JSContext *, JSObject *obj, uint32_t slot, jsval *vp ) {

	ASSERT( slot < JSCLASS_RESERVED_SLOTS(JL_GetClass(obj)) );
	ASSERT( JS_IsNative(obj) );
	*vp = js::GetReservedSlot(obj, slot); // jsfriendapi
	return JS_TRUE;
}

ALWAYS_INLINE JSBool FASTCALL
JL_SetReservedSlot(JSContext *, JSObject *obj, unsigned slot, const jsval &v) {

	ASSERT( JS_IsNative(obj) );
	js::SetReservedSlot(obj, slot, v); // jsfriendapi
	return JS_TRUE;
}


ALWAYS_INLINE JSString* FASTCALL
JL_NewUCString(JSContext *cx, jschar *chars, size_t length) {

//if spidermonkey et jslibs allocators are not the same:
//
//	void *tmp = JS_malloc(cx, length);
//	if ( !tmp )
//		return NULL;
//	jl_memcpy(tmp, bytes, length * sizeof(*jschar));
//	jl_free(bytes);
//	bytes = (char*)tmp;

	return JS_NewUCString(cx, chars, length); // doc. https://developer.mozilla.org/en/SpiderMonkey/JSAPI_Reference/JS_NewString
}


namespace jspv {

	// useful for structure with jsid initialized to 0, (see HostPrivate ids)
	ALWAYS_INLINE const jsid
	NullJsid() {

		jsid tmp;
		JSID_BITS(tmp) = 0;
		ASSERT(JSID_IS_ZERO(tmp));
		return tmp;
	}
}


ALWAYS_INLINE jsid FASTCALL
JL_StringToJsid( JSContext *cx, JSString *jsstr ) {

	ASSERT( jsstr != NULL );
	jsstr = JS_InternJSString(cx, jsstr); // if ( !JS_StringHasBeenInterned(cx, jsstr) )
	if ( jsstr == NULL )
		return jspv::NullJsid();
	return INTERNED_STRING_TO_JSID(cx, jsstr);
}


ALWAYS_INLINE jsid FASTCALL
JL_StringToJsid( JSContext *cx, const jschar *wstr ) {

	ASSERT( wstr != NULL );
	JSString *jsstr = JS_InternUCString(cx, wstr);
	if ( jsstr == NULL )
		return jspv::NullJsid();
	jsid id = JL_StringToJsid(cx, jsstr);
	ASSERT( JSID_IS_STRING(id) );
	return id;
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


///////////////////////////////////////////////////////////////////////////////
// IDs cache

// defined here because required by jlhostprivate.h
#define JLID_SPEC(name) JLID_##name
enum {
	JLID_SPEC( global ),
	JLID_SPEC( get ),
	JLID_SPEC( read ),
	JLID_SPEC( getMatrix44 ),
	JLID_SPEC( _NI_BufferGet ),
	JLID_SPEC( _NI_StreamRead ),
	JLID_SPEC( _NI_Matrix44Get ),
	JLID_SPEC( name ),
	JLID_SPEC( length ),
	JLID_SPEC( id ),
	JLID_SPEC( valueOf ),
	JLID_SPEC( toString ),
	JLID_SPEC( next ),
	JLID_SPEC( iterator ),
	JLID_SPEC( arguments ),
	JLID_SPEC( unsafeMode ),
	JLID_SPEC( stdin ),
	JLID_SPEC( stdout ),
	JLID_SPEC( stderr ),
	JLID_SPEC( width ),
	JLID_SPEC( height ),
	JLID_SPEC( channels ),
	JLID_SPEC( bits ),
	JLID_SPEC( rate ),
	JLID_SPEC( frames ),
	JLID_SPEC( revision ),
	JLID_SPEC( _revision ),
	JLID_SPEC( buildDate ),
	JLID_SPEC( _buildDate ),
	JLID_SPEC( path ),
	JLID_SPEC( isFirstInstance ),
	JLID_SPEC( bootstrapScript ),
	JLID_SPEC( _serialize ),
	JLID_SPEC( _unserialize ),
	JLID_SPEC( _private1 ),
	JLID_SPEC( _private2 ),
	JLID_SPEC( _private3 ),
	JLID_SPEC( eval ),
	JLID_SPEC( push ),
	JLID_SPEC( pop ),
	JLID_SPEC( toXMLString ),
	JLID_SPEC( fileName ),
	JLID_SPEC( lineNumber ),
	JLID_SPEC( stack ),
	JLID_SPEC( message ),
	JLID_SPEC( Reflect ),
	JLID_SPEC( Debugger ),
	JLID_SPEC( Function ),
	JLID_SPEC( isGenerator ),
	JLID_SPEC( writable ),
	JLID_SPEC( readable ),
	JLID_SPEC( hangup ),
	JLID_SPEC( exception ),
	JLID_SPEC( error ),
	JLID_SPEC( position ),
	JLID_SPEC( available ),
	JLID_SPEC( data ),

	LAST_JSID // see HostPrivate::ids[]
};
#undef JLID_SPEC
// examples:
//   JLID(cx, _unserialize) -> jsid
//   JLID_NAME(cx, _unserialize) -> w_char



///////////////////////////////////////////////////////////////////////////////
// Context private

/* not used right now
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
*/


///////////////////////////////////////////////////////////////////////////////
// Host private

// Using a separate file allow a better versioning of the HostPrivate structure (see JL_HOSTPRIVATE_KEY).
#include "jlhostprivate.h"

ALWAYS_INLINE HostPrivate*
JL_GetHostPrivate( JSContext *cx ) {

	HostPrivate *hpv = static_cast<HostPrivate*>(JL_GetRuntimePrivate(JL_GetRuntime(cx)));
#ifdef DEBUG
	hpv->tmp_count++;
#endif
	return hpv;
}

ALWAYS_INLINE void
JL_SetHostPrivate( JSContext *cx, HostPrivate *hostPrivate ) {

	JL_SetRuntimePrivate(JL_GetRuntime(cx), static_cast<void*>(hostPrivate));
}


///////////////////////////////////////////////////////////////////////////////
// Module private

ALWAYS_INLINE uint8_t
JL_ModulePrivateHash( const uint32_t moduleId ) {

	ASSERT( moduleId != 0 );
//	return ((uint8_t*)&moduleId)[0] ^ ((uint8_t*)&moduleId)[1] ^ ((uint8_t*)&moduleId)[2] ^ ((uint8_t*)&moduleId)[3] << 1;
	uint32_t a = moduleId ^ (moduleId >> 16);
	return (a ^ a >> 8) & 0xFF;
}

ALWAYS_INLINE bool
JL_SetModulePrivate( JSContext *cx, const uint32_t moduleId, void *modulePrivate ) {

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

ALWAYS_INLINE void* FASTCALL
JL_GetModulePrivateOrNULL( JSContext *cx, uint32_t moduleId ) {

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


ALWAYS_INLINE void* FASTCALL
JL_GetModulePrivate( JSContext *cx, uint32_t moduleId ) {

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

// see:
//   SuperFastHash (http://www.azillionmonkeys.com/qed/hash.html)
//   FNV variants (http://isthe.com/chongo/tech/comp/fnv/)
//   MurmurHash 1.0 (https://sites.google.com/site/murmurhash/)
ALWAYS_INLINE NOALIAS uint32_t FASTCALL
JL_ClassNameToClassProtoCacheSlot( const char * const n ) {

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
	return ((h >> 7) ^ h) & ((1<<JL_HOSTPRIVATE_MAX_CLASS_PROTO_CACHE_BIT) - 1);
}

namespace jlpv {

	ALWAYS_INLINE void *RemovedSlot() {

		return (void*)-1;
	}
}


INLINE bool FASTCALL
JL_CacheClassProto( HostPrivate * RESTRICT hpv, const char * const RESTRICT className, JSClass * const RESTRICT clasp, JSObject * const RESTRICT proto ) {

	ASSERT( jlpv::RemovedSlot() != NULL );
	ASSERT( className != NULL );
	ASSERT( className[0] != '\0' );
	ASSERT( clasp != NULL );
	ASSERT( clasp != jlpv::RemovedSlot() );
	ASSERT( proto != NULL );
	ASSERT( JL_GetClass(proto) == clasp );

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


ALWAYS_INLINE const ClassProtoCache* FASTCALL
JL_GetCachedClassProto( const HostPrivate * const hpv, const char * const className ) {

	ASSERT( jlpv::RemovedSlot() != NULL );

	size_t slotIndex = JL_ClassNameToClassProtoCacheSlot(className);
	const size_t first = slotIndex;
	ASSERT( slotIndex < COUNTOF(hpv->classProtoCache) );

	for (;;) {

		const ClassProtoCache *slot = &hpv->classProtoCache[slotIndex];
		
		// slot->clasp == NULL -> empty
		// slot->clasp == jlpv::RemovedSlot() -> slot removed, but maybe next slot will match !

		if ( slot->clasp == NULL ) // not found
			return NULL;

		if ( slot->clasp != (JSClass*)jlpv::RemovedSlot() && ( slot->clasp->name == className || !strcmp(slot->clasp->name, className) ) ) // see "Enable String Pooling"
			return slot;

		slotIndex = (slotIndex + 1) % COUNTOF(hpv->classProtoCache);

		if ( slotIndex == first ) // not found
			return NULL;
	}
}


ALWAYS_INLINE JSClass * FASTCALL
JL_GetCachedClass( const HostPrivate * const hpv, const char * const className ) {
	
	const ClassProtoCache *cpc = JL_GetCachedClassProto(hpv, className);
	return cpc ? cpc->clasp : NULL;
}

ALWAYS_INLINE JSObject * FASTCALL
JL_GetCachedProto( const HostPrivate * const hpv, const char * const className ) {
	
	const ClassProtoCache *cpc = JL_GetCachedClassProto(hpv, className);
	return cpc ? cpc->proto : NULL;
}


ALWAYS_INLINE void FASTCALL
JL_RemoveCachedClassProto( HostPrivate * const hpv, const char *const className ) {

	ASSERT( jlpv::RemovedSlot() != NULL );

	size_t slotIndex = JL_ClassNameToClassProtoCacheSlot(className);
	size_t first = slotIndex;
	ASSERT( slotIndex < COUNTOF(hpv->classProtoCache) );

	for (;;) {

		ClassProtoCache *slot = &hpv->classProtoCache[slotIndex];

		if ( slot->clasp == NULL || ( slot->clasp != (JSClass*)jlpv::RemovedSlot() && ( slot->clasp->name == className || strcmp(slot->clasp->name, className) == 0 ) ) ) {
			
			slot->clasp = (JSClass*)jlpv::RemovedSlot();
			slot->proto = NULL;
			return;
		}

		slotIndex = (slotIndex + 1) % COUNTOF(hpv->classProtoCache);

		if ( slotIndex == first ) // not found
			return;
	}
}



///////////////////////////////////////////////////////////////////////////////
// new object management

/*
ALWAYS_INLINE JSObject* FASTCALL
JL_NewObjectWithGivenProtoKey( JSContext *cx, JSProtoKey protoKey, JSObject *parent ) {

	ASSERT( parent );
	JSObject *proto;
	if ( !JL_GetClassPrototype(cx, parent, protoKey, &proto) )
		return NULL;
	JSClass *clasp = JL_GetClass(proto);
	JSObject *obj = JS_NewObjectWithGivenProto(cx, clasp, proto, parent);
	ASSERT( JL_GetParent(cx, obj) != NULL );
	return obj;
}
*/

ALWAYS_INLINE JSObject* FASTCALL
JL_NewObjectWithGivenProto( JSContext *cx, JSClass *clasp, JSObject *proto, JSObject *parent ) {

	ASSERT_IF( proto != NULL, JL_GetParent(cx, proto) != NULL );
	// Doc. JS_NewObject, JL_NewObjectWithGivenProto behaves exactly the same, except that if proto is NULL, it creates an object with no prototype.
	JSObject *obj = JS_NewObjectWithGivenProto(cx, clasp, proto, parent);  // (TBD) test if parent is ok (see bug 688510)
	ASSERT( JL_GetParent(cx, obj) != NULL );
	return obj;
}


ALWAYS_INLINE JSObject* FASTCALL
JL_NewProtolessObj( JSContext *cx ) {

	JSObject *obj = JL_NewObjectWithGivenProto(cx, NULL, NULL, JL_GetGlobal(cx));
	ASSERT( JL_GetParent(cx, obj) != NULL );
	ASSERT( JL_GetPrototype(cx, obj) == NULL );
	return obj;
}


ALWAYS_INLINE JSObject* FASTCALL
JL_NewObj( JSContext *cx ) {

	HostPrivate *pv = JL_GetHostPrivate(cx);
	ASSERT( pv );
	ASSERT( pv->objectClass );
	ASSERT( pv->objectProto );
	return JS_NewObject(cx, pv->objectClass, pv->objectProto, JL_GetGlobal(cx));
}


ALWAYS_INLINE JSObject* FASTCALL
JL_NewJslibsObject( JSContext *cx, const char *className ) {

	const ClassProtoCache *cpc = JL_GetCachedClassProto(JL_GetHostPrivate(cx), className);
	if ( cpc != NULL )
		return JL_NewObjectWithGivenProto(cx, cpc->clasp, cpc->proto, NULL);
	return NULL;
}


#define JL_DEFINE_FUNCTION_OBJ \
	JSObject *obj = JS_THIS_OBJECT(cx, vp); \
	ASSERT( obj ); \


#define JL_DEFINE_CALL_FUNCTION_OBJ \
	JSObject *obj = JSVAL_TO_OBJECT(JS_CALLEE(cx, vp)); \
	ASSERT( obj ); \


namespace jlpv {
	
	ALWAYS_INLINE JSObject *
	CreateConstructorObject(JSContext *cx, JSClass *clasp, JSObject *proto, jsval *vp) {

		JSObject *obj = JL_NewObjectWithGivenProto(cx, clasp, proto, NULL);
		if ( obj != NULL )
			JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(obj));
		return obj;
	}
}

// Initialise 'this' object (obj variable) for constructors native functions ( support constructing and non-constructing form, eg. |Stream()| and  |new Stream()| ).
// If JL_THIS_CLASS or JL_THIS_CLASS_PROTOTYPE are not available, use JS_NewObjectForConstructor(cx, vp) instead.
#define JL_DEFINE_CONSTRUCTOR_OBJ \
	JSObject *obj; obj = jlpv::CreateConstructorObject(cx, JL_THIS_CLASS, JL_THIS_CLASS_PROTOTYPE, vp)



///////////////////////////////////////////////////////////////////////////////
// IDs cache management

namespace jlpv {

	INLINE NEVER_INLINE jsid FASTCALL
	GetPrivateJsidSlow( JSContext *cx, jsid * RESTRICT ids, int index, const jschar * RESTRICT name ) {

		ASSERT( ids != NULL );
		jsid id;
		JSString *jsstr = JS_InternUCString(cx, name);
		if (unlikely( jsstr == NULL ))
			return jspv::NullJsid();
		id = JL_StringToJsid(cx, jsstr);
		ids[index] = id;
		return id;
	}
}

ALWAYS_INLINE jsid FASTCALL
JL_GetPrivateJsid( JSContext *cx, int index, const jschar *name ) {

	HostPrivate *hpv = JL_GetHostPrivate(cx);
	ASSERT( hpv != NULL );
	jsid *ids = hpv->ids;
	jsid id = ids[index];
	return id != jspv::NullJsid() ? id : jlpv::GetPrivateJsidSlow(cx, ids, index, name);
}


#ifdef DEBUG
#define JLID_NAME(cx, name) (JL_IGNORE(cx), JL_IGNORE(JLID_##name), L(#name))
#else
#define JLID_NAME(cx, name) (#name)
#endif // DEBUG

#define JLID(cx, name) JL_GetPrivateJsid(cx, JLID_##name, L(#name))
// eg: jsid cfg = JLID(cx, fileName); const char *name = JLID_NAME(fileName);



///////////////////////////////////////////////////////////////////////////////
// Type check functions

ALWAYS_INLINE bool FASTCALL
JL_ObjectIsBoolean( JSContext *cx, JSObject *obj ) {

	ASSERT( obj != NULL );
	JSObject *proto;
	return JL_GetClassPrototype(cx, JL_GetGlobal(cx), JSProto_Boolean, &proto) && JL_GetClass(obj) == JL_GetClass(proto);
}

ALWAYS_INLINE bool FASTCALL
JL_ValueIsBooleanObject( JSContext * RESTRICT cx, jsval & RESTRICT value ) {

	JL_IGNORE(cx);
	return !JSVAL_IS_PRIMITIVE(value) && JL_ObjectIsBoolean(cx, JSVAL_TO_OBJECT(value));
}

ALWAYS_INLINE bool FASTCALL
JL_ValueIsBoolean( JSContext * RESTRICT cx, jsval & RESTRICT value ) {

	return JSVAL_IS_BOOLEAN(value) || JL_ValueIsBooleanObject(cx, value);
}

ALWAYS_INLINE bool FASTCALL
JL_ValueIsInteger( JSContext * RESTRICT cx, jsval & RESTRICT value ) {

	JL_IGNORE(cx);
	return JSVAL_IS_INT(value) || (JSVAL_IS_DOUBLE(value) && JL_DOUBLE_IS_INTEGER(JSVAL_TO_DOUBLE(value)));
}

ALWAYS_INLINE bool FASTCALL
JL_ObjectIsNumber( JSContext *cx, JSObject *obj ) {

	ASSERT( obj != NULL );
	JSObject *proto;
	return JL_GetClassPrototype(cx, JL_GetGlobal(cx), JSProto_Number, &proto) && JL_GetClass(obj) == JL_GetClass(proto);
}

ALWAYS_INLINE bool FASTCALL
JL_ValueIsNumberObject( JSContext * RESTRICT cx, jsval & RESTRICT value ) {

	return !JSVAL_IS_PRIMITIVE(value) && JL_ObjectIsNumber(cx, JSVAL_TO_OBJECT(value));
}

ALWAYS_INLINE bool FASTCALL
JL_ValueIsNumber( JSContext * RESTRICT cx, jsval & RESTRICT value ) {

	return JSVAL_IS_NUMBER(value) || JL_ValueIsNumberObject(cx, value);
}

/*
ALWAYS_INLINE bool FASTCALL
JL_ValueIsInteger53( JSContext * RESTRICT cx, jsval & RESTRICT value ) {

	JL_IGNORE(cx);
	return JSVAL_IS_INT(value) || (JSVAL_IS_DOUBLE(value) && JSVAL_TO_DOUBLE(value) < MAX_INT_TO_DOUBLE && JSVAL_TO_DOUBLE(value) > MIN_INT_TO_DOUBLE); INCOMPLETE CONDITION !
}
*/

ALWAYS_INLINE bool FASTCALL
JL_ValueIsNaN( JSContext *cx, const jsval &val ) {

	return val == JL_GetNaNValue(cx);
}

ALWAYS_INLINE bool FASTCALL
JL_ValueIsPInfinity( JSContext *cx, const jsval &val ) {

	return val == JS_GetPositiveInfinityValue(cx);
}

ALWAYS_INLINE bool FASTCALL
JL_ValueIsNInfinity( JSContext *cx, const jsval &val ) {

	return val == JS_GetNegativeInfinityValue(cx);
}

/* useless
ALWAYS_INLINE bool FASTCALL
JL_ValueIsReal( const JSContext *cx, const jsval &val ) {

	JL_IGNORE(cx);
	if ( JSVAL_IS_INT(val) )
		return true;
	if ( JSVAL_IS_DOUBLE(val) ) {

		double tmp = JSVAL_TO_DOUBLE(val);
		return tmp >= MIN_INT_TO_DOUBLE && tmp <= MAX_INT_TO_DOUBLE;
	}
	return false;
}
*/

ALWAYS_INLINE bool FASTCALL
JL_ValueIsPositive( JSContext *cx, const jsval &val ) {

	// handle string conversion and valueOf ?

	return ( JSVAL_IS_INT(val) && JSVAL_TO_INT(val) > 0 )
	    || ( JSVAL_IS_DOUBLE(val) && JSVAL_TO_DOUBLE(val) > 0 )
	    || JL_ValueIsPInfinity(cx, val);
}

ALWAYS_INLINE bool FASTCALL
JL_ValueIsNegative( JSContext *cx, const jsval &val ) {

	// handle string conversion and valueOf ?

	return ( JSVAL_IS_INT(val) && JSVAL_TO_INT(val) < 0 )
	    || ( JSVAL_IS_DOUBLE(val) && DOUBLE_IS_NEG(JSVAL_TO_DOUBLE(val)) ) // handle NEGZERO ?
	    || JL_ValueIsNInfinity(cx, val);
}

ALWAYS_INLINE bool FASTCALL
JL_ObjectIsObject( JSContext *cx, JSObject *obj ) {

	ASSERT( obj != NULL );
	JSObject *proto;
	return JL_GetClassPrototype(cx, JL_GetGlobal(cx), JSProto_Object, &proto) && JL_GetClass(obj) == JL_GetClass(proto);
}

ALWAYS_INLINE bool FASTCALL
JL_ObjectIsString( JSContext *cx, JSObject *obj ) {

	ASSERT( obj );
	JSObject *proto;
	return JL_GetClassPrototype(cx, JL_GetGlobal(cx), JSProto_String, &proto) && JL_GetClass(obj) == JL_GetClass(proto);
}


ALWAYS_INLINE bool FASTCALL
JL_ObjectIsArray( JSContext * RESTRICT cx, JSObject * RESTRICT obj ) {

	return JS_IsArrayObject(cx, obj) == JS_TRUE;
}

ALWAYS_INLINE bool FASTCALL
JL_ValueIsArray( JSContext *cx, const jsval &val ) {

	return !JSVAL_IS_PRIMITIVE(val) && JL_ObjectIsArray(cx, JSVAL_TO_OBJECT(val));
}


// note that TypedArray, String and Array objects have a length property (ArrayBuffer does not),
// and unfortunately Function also have a length property.
ALWAYS_INLINE bool FASTCALL
JL_ObjectIsArrayLike( JSContext *cx, JSObject *obj ) {

	JSBool found;
	return JS_IsArrayObject(cx, obj)
		|| (
		JS_HasPropertyById(cx, obj, JLID(cx, length), &found)
	    && found == JS_TRUE
	    && !JS_ObjectIsFunction(cx, obj)
		);
}

ALWAYS_INLINE bool FASTCALL
JL_ValueIsArrayLike( JSContext *cx, const jsval &val ) {

	return !JSVAL_IS_PRIMITIVE(val) && JL_ObjectIsArrayLike(cx, JSVAL_TO_OBJECT(val));
}

ALWAYS_INLINE bool FASTCALL
JL_ObjectIsData( JSContext * RESTRICT cx, JSObject * RESTRICT obj ) {

	return JS_IsArrayBufferObject(obj) || BufferGetInterface(cx, obj) != NULL || JL_ObjectIsArrayLike(cx, obj);
}

ALWAYS_INLINE bool FASTCALL
JL_ValueIsData( JSContext *cx, const jsval &val ) {

	return JSVAL_IS_STRING(val) || ( !JSVAL_IS_PRIMITIVE(val) && NOIL(JL_ObjectIsData)(cx, JSVAL_TO_OBJECT(val)) );
}


ALWAYS_INLINE bool FASTCALL
JL_ObjectIsIterable( JSContext * RESTRICT cx, JSObject * RESTRICT obj ) {

	JSBool found;
	return JS_HasPropertyById(cx, obj, JLID(cx, next), &found) && found == JS_TRUE;
}

ALWAYS_INLINE bool FASTCALL
JL_ValueIsIterable( JSContext * RESTRICT cx, jsval &val ) {

	return !JSVAL_IS_PRIMITIVE(val) && JL_ObjectIsIterable(cx, JSVAL_TO_OBJECT(val));
}


ALWAYS_INLINE bool FASTCALL
JL_IsStopIterationExceptionPending( JSContext *cx ) {

	JSObject *proto;
	jsval ex;
	return JS_GetPendingException(cx, &ex) // note: JS_GetPendingException returns false if no exception is pending.
	    && JSVAL_IS_OBJECT(ex)
	    && JL_GetClassPrototype(cx, JL_GetGlobal(cx), JSProto_StopIteration, &proto)
	    && JL_GetClass(JSVAL_TO_OBJECT(ex)) == JL_GetClass(proto);
}

ALWAYS_INLINE bool FASTCALL
JL_ValueIsGenerator( JSContext *cx, jsval &val ) {

	// Function.prototype.isGenerator.call(Gen)
	JSObject *proto;
	jsval fct, rval;
	return JSVAL_IS_OBJECT(val)
		&& JL_GetClassPrototype(cx, JL_GetGlobal(cx), JSProto_Function, &proto) 
	    && JS_GetPropertyById(cx, proto, JLID(cx, isGenerator), &fct)
		&& JS_CallFunctionValue(cx, JSVAL_TO_OBJECT(val), fct, 0, NULL, &rval)
		&& rval == JSVAL_TRUE;
}

ALWAYS_INLINE bool FASTCALL
JL_ObjectIsXML( JSContext *cx, JSObject *obj ) {

	ASSERT( obj );
	return JS_TypeOfValue(cx, OBJECT_TO_JSVAL(obj)) == JSTYPE_XML;
}

ALWAYS_INLINE bool FASTCALL
JL_ObjectIsError( JSContext *cx, JSObject *obj ) {

	ASSERT( obj );
	JSObject *proto;
	return JL_GetClassPrototype(cx, JL_GetGlobal(cx), JSProto_Error, &proto) && JL_GetClass(obj) == JL_GetClass(proto); // note: JS_GetClass( (new SyntaxError()) ) => JSProto_Error
}


ALWAYS_INLINE bool FASTCALL
JL_ObjectIsCallable( JSContext *cx, JSObject *obj ) {

	return JS_ObjectIsCallable(cx, obj) == JS_TRUE;
}

ALWAYS_INLINE bool FASTCALL
JL_ValueIsCallable( JSContext *cx, jsval &val ) {

	return !JSVAL_IS_PRIMITIVE(val) && JL_ObjectIsCallable(cx, JSVAL_TO_OBJECT(val));
}

ALWAYS_INLINE bool FASTCALL
JL_ValueIsClass( const jsval &val, const JSClass *jsClass ) {
	
	ASSERT( jsClass != NULL );
	return !JSVAL_IS_PRIMITIVE(val) && JL_GetClass(JSVAL_TO_OBJECT(val)) == jsClass;
}

ALWAYS_INLINE bool FASTCALL
JL_ObjectIsInstanceOf( JSContext *cx, JSObject * RESTRICT obj, JSClass * RESTRICT clasp ) {

	JL_IGNORE(cx);
	return obj && JL_GetClass(obj) == clasp;
}



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
	JL_ASSERT( NOIL(JL_ValueIsInteger)(cx, val), E_VALUE, E_STR(context), E_TYPE, E_TY_INTEGER )

#define JL_ASSERT_IS_INTEGER_NUMBER(val, context) \
	JL_ASSERT( NOIL(JL_ValueIsInteger)(cx, val), E_VALUE, E_STR(context), E_TYPE, E_TY_INTEGERDOUBLE )

#define JL_ASSERT_IS_NUMBER(val, context) \
	JL_ASSERT( NOIL(JL_ValueIsNumber)(cx, val), E_VALUE, E_STR(context), E_TYPE, E_TY_NUMBER )

#define JL_ASSERT_IS_CALLABLE(val, context) \
	JL_ASSERT( NOIL(JL_ValueIsCallable)(cx, val), E_VALUE, E_STR(context), E_TYPE, E_TY_FUNC )

#define JL_ASSERT_IS_ARRAY(val, context) \
	JL_ASSERT( NOIL(JL_ValueIsArray)(cx, val), E_VALUE, E_STR(context), E_TYPE, E_TY_ARRAY )

#define JL_ASSERT_IS_OBJECT(val, context) \
	JL_ASSERT( !JSVAL_IS_PRIMITIVE(val), E_VALUE, E_STR(context), E_TYPE, E_TY_OBJECT )

#define JL_ASSERT_IS_STRING(val, context) \
	JL_ASSERT( NOIL(JL_ValueIsData)(cx, val), E_VALUE, E_STR(context), E_TYPE, E_TY_STRINGDATA )

//

#define JL_ASSERT_RANGE(val, valMin, valMax, context) \
	JL_ASSERT( JL_INRANGE((int)val, (int)valMin, (int)valMax), E_VALUE, E_STR(context), E_RANGE, E_INTERVAL_NUM(valMin, valMax) )


// arg

#define JL_ASSERT_ARGC_MIN(minCount) \
	JL_ASSERT( JL_ARGC >= (minCount), E_ARGC, E_MIN, E_NUM(minCount) )

#define JL_ASSERT_ARGC_MAX(maxCount) \
	JL_ASSERT( JL_ARGC <= (maxCount), E_ARGC, E_MAX, E_NUM(maxCount) )

#define JL_ASSERT_ARGC_RANGE(minCount, maxCount) \
	JL_ASSERT( JL_INRANGE((int)JL_ARGC, (int)minCount, (int)maxCount), E_ARGC, E_RANGE, E_INTERVAL_NUM(unsigned(minCount), unsigned(maxCount)) )

#define JL_ASSERT_ARGC(count) \
	JL_ASSERT( JL_ARGC == (count), E_ARGC, E_EQUALS, E_NUM(count) )

#define JL_ASSERT_ARG_IS_BOOLEAN(argNum) \
	JL_ASSERT( NOIL(JL_ValueIsBoolean)(cx, JL_ARG(argNum)), E_ARG, E_NUM(argNum), E_TYPE, E_NAME("boolean") )

#define JL_ASSERT_ARG_IS_INTEGER(argNum) \
	JL_ASSERT( NOIL(JL_ValueIsInteger)(cx, JL_ARG(argNum)), E_ARG, E_NUM(argNum), E_TYPE, E_NAME("integer") )

#define JL_ASSERT_ARG_IS_INTEGER_NUMBER(argNum) \
	JL_ASSERT( NOIL(JL_ValueIsInteger)(cx, JL_ARG(argNum)), E_ARG, E_NUM(argNum), E_TYPE, E_NAME("integer < 2^53") )

#define JL_ASSERT_ARG_IS_NUMBER(argNum) \
	JL_ASSERT( NOIL(JL_ValueIsNumber)(cx, JL_ARG(argNum)), E_ARG, E_NUM(argNum), E_TYPE, E_NAME("number") )

#define JL_ASSERT_ARG_IS_OBJECT(argNum) \
	JL_ASSERT( !JSVAL_IS_PRIMITIVE(JL_ARG(argNum)), E_ARG, E_NUM(argNum), E_TYPE, E_NAME("object") )

#define JL_ASSERT_ARG_IS_OBJECT_OR_NULL(argNum) \
	JL_ASSERT( JSVAL_IS_OBJECT(JL_ARG(argNum)), E_ARG, E_NUM(argNum), E_TYPE, E_NAME("object"), E_OR, E_NAME("null") )

#define JL_ASSERT_ARG_IS_STRING(argNum) \
	JL_ASSERT( NOIL(JL_ValueIsData)(cx, JL_ARG(argNum)), E_ARG, E_NUM(argNum), E_TYPE, E_NAME("string || data") )

#define JL_ASSERT_ARG_IS_ARRAY(argNum) \
	JL_ASSERT( NOIL(JL_ValueIsArray)(cx, JL_ARG(argNum)), E_ARG, E_NUM(argNum), E_TYPE, E_NAME("array") )

#define JL_ASSERT_ARG_IS_ARRAYLIKE(argNum) \
	JL_ASSERT( NOIL(JL_ValueIsArrayLike)(cx, JL_ARG(argNum)), E_ARG, E_NUM(argNum), E_TYPE, E_NAME("array") )

#define JL_ASSERT_ARG_IS_CALLABLE(argNum) \
	JL_ASSERT( NOIL(JL_ValueIsCallable)(cx, JL_ARG(argNum)), E_ARG, E_NUM(argNum), E_TYPE, E_NAME("function") )

// fallback
#define JL_ASSERT_ARG_TYPE(condition, argNum, typeStr) \
	JL_ASSERT( condition, E_ARG, E_NUM(argNum), E_TYPE, E_NAME(typeStr) )

#define JL_ASSERT_ARG_VAL_RANGE(val, valMin, valMax, argNum) \
	JL_ASSERT( JL_INRANGE((int)val, (int)valMin, (int)valMax), E_ARG, E_NUM(argNum), E_RANGE, E_INTERVAL_NUM(valMin, valMax) )



// obj

#define JL_ASSERT_CONSTRUCTING() \
	JL_ASSERT( (JL_ARGC, JS_IsConstructing(cx, vp)), E_THISOBJ, E_CONSTRUCT )

// note: JL_GetClass(JL_GetPrototype(... because |JL_ASSERT_THIS_INSTANCE( new Stream() )| must pass whereas |JL_ASSERT_THIS_INSTANCE( Stream.prototype )| must fail.
#define JL_ASSERT_INSTANCE( jsObject, jsClass ) \
	JL_ASSERT( JL_GetClass(JL_GetPrototype(cx, jsObject)) == jsClass, E_OBJ, E_INSTANCE, E_NAME((jsClass)->name) ) // ReportIncompatibleMethod(cx, CallReceiverFromArgv(argv), Valueify(clasp));

#define JL_ASSERT_THIS_INSTANCE() \
	JL_ASSERT( JL_GetClass(JL_GetPrototype(cx, JL_OBJ)) == JL_THIS_CLASS, E_THISOBJ, E_INSTANCE, E_NAME(JL_THIS_CLASS_NAME) ) // ReportIncompatibleMethod(cx, CallReceiverFromArgv(argv), Valueify(clasp));

#define JL_ASSERT_INHERITANCE( jsObject, jsClass ) \
	JL_ASSERT( NOIL(JL_InheritFrom)(cx, JL_GetPrototype(cx, jsObject), (jsClass)), E_OBJ, E_INHERIT, E_NAME((jsClass)->name) )

#define JL_ASSERT_THIS_INHERITANCE() \
	JL_ASSERT( NOIL(JL_InheritFrom)(cx, JL_GetPrototype(cx, JL_OBJ), JL_THIS_CLASS), E_THISOBJ, E_INHERIT, E_NAME(JL_THIS_CLASS_NAME) )


#define JL_ASSERT_OBJECT_STATE( condition, name ) \
	JL_ASSERT( condition, E_OBJ, E_NAME(name), E_STATE )

#define JL_ASSERT_THIS_OBJECT_STATE( condition ) \
	JL_ASSERT( condition, E_THISOBJ, E_NAME(JL_THIS_CLASS_NAME), E_STATE )


///////////////////////////////////////////////////////////////////////////////
// JLData class

class JLData {

	mutable void *_buf;
	size_t _len;
	bool _own; // has ownership
	bool _nt; // null-terminated
	bool _w; // is wide-char

	void Check() {

		ASSERT( IsSet() );
		ASSERT_IF( _len == SIZE_MAX, _nt );
		ASSERT_IF( _len != SIZE_MAX && _own && _w && !_nt, jl_msize(_buf) >= _len*2 );
		ASSERT_IF( _len != SIZE_MAX && _own && !_w && !_nt, jl_msize(_buf) >= _len );
		ASSERT_IF( _len != SIZE_MAX && _own && _w && _nt, jl_msize(_buf) >= _len*2+2 );
		ASSERT_IF( _len != SIZE_MAX && _own && !_w && _nt, jl_msize(_buf) >= _len+1 );
		ASSERT_IF( _len != SIZE_MAX && _nt && _w, ((jschar*)_buf)[_len] == 0 );
		ASSERT_IF( _len != SIZE_MAX && _nt && !_w, ((char*)_buf)[_len] == 0 );
		//ASSERT_IF( _own && _nt && !_w, jl_msize(_buf) >= (Length()+1) );
		//ASSERT_IF( _own && _nt && _w, jl_msize(_buf) >= (Length()+1)*2 );
		//ASSERT_IF( _nt && !_w, ((char*)_buf)[Length()] == 0 );
		//ASSERT_IF( _nt && _w, ((jschar*)_buf)[Length()] == 0 );
	}

	bool FASTCALL ForceMutation( bool own, bool nullTerminated, bool wide ) {

		if ( _nt && !nullTerminated /*&& _own == own && _w == wide*/ ) // pointless to remove nullTerminated /*only*/.
			nullTerminated = true;

		void *dst;
		const size_t length = Length();
		const size_t dstSize = (length + (nullTerminated ? 1 : 0)) * (wide ? 2 : 1) + IFDEBUG( 2 ); // IFDEBUG: force non-Z strings if !nullTerminated;

		//printf( "*** _own:%d, own:%d, _w:%d, wide:%d, _nt:%d, nullTerminated:%d\n", _own, own, _w, wide, _nt, nullTerminated );

		if ( !_own && ( own || ( _w != wide ) || ( !_nt && nullTerminated ) ) ) {

			dst = jl_malloc(dstSize);
			if ( !dst )
				return false;
			_own = true; // dst, not _buf
		} else {
			
			ASSERT( _own );
			dst = _buf;
		}

		if ( _w && !wide ) {

			// shrink before realloc
			jschar *s = (jschar*)_buf;
			char *d = (char*)dst;
			size_t tmpLen = length;
			while ( tmpLen-- > 0 )
				*(d++) = *(s++) & 0xFF;
			_w = false; // dst, not _buf

			if ( dst == _buf ) {
				
				ASSERT( _own );
				//if ( dstSize > 64 ) {
				if ( JL_MaybeRealloc(length, dstSize) ) {
					
					dst = jl_realloc(_buf, dstSize);
					if ( !dst )
						return false;
				}
			}
		} else
		if ( _w == wide ) {

			if ( dst == _buf ) {
				
				ASSERT( _own );
				dst = jl_realloc(_buf, dstSize);
				if ( !dst )
					return false;
			} else {
			
				//ASSERT( !_own );
				jl_memcpy(dst, _buf, length * (wide ? 2 : 1));
			}
		} else
		if ( !_w && wide ) {

			if ( dst == _buf ) {
				
				ASSERT( _own );
				_buf = dst = jl_realloc(_buf, dstSize);
				if ( !dst )
					return false;
			}

			// grow after realloc
			char *s = (char*)_buf + length;
			jschar *d = (jschar*)dst + length;
			size_t tmpLen = length;
			while ( tmpLen-- > 0 )
				*(--d) = *(--s) & 0xFF;
			_w = true; // dst, not _buf
		} else {
			
			ASSERT(false); // invalid case.
		}
		
		_buf = dst;

		ASSERT( _own );
		if ( nullTerminated ) {

			if ( wide )
				((jschar*)_buf)[length] = 0;
			else
				((char*)_buf)[length] = 0;
		}

		IFDEBUG( 
		if ( !nullTerminated ) {

			if ( wide )
				((jschar*)_buf)[length] = L('X');
			else
				((char*)_buf)[length] = 'X';
		}
		);

		_nt = nullTerminated;
		return true;
	}

	ALWAYS_INLINE bool Mutate( bool own, bool nullTerminated, bool wide ) {

		ASSERT( IsSet() );
		if ( (!_own && own) || (_w != wide) || (!_nt && nullTerminated) )
			return ForceMutation(own, nullTerminated, wide);
		else
			return true;
	}

	NEVER_INLINE size_t FASTCALL Length_slow() {

		ASSERT( _nt );
		return _len = _w ? wcslen((const wchar_t*)_buf) : strlen((const char*)_buf);
	}

private:
	void operator [](size_t);
	void* operator new(size_t);
	void* operator new[](size_t);

public:
	ALWAYS_INLINE ~JLData() {
		
		if ( _buf && _own )
			jl_free(_buf);
	}

	ALWAYS_INLINE JLData() : _buf(NULL) {
	}

	ALWAYS_INLINE JLData( const JLData &data ) {

		_buf = data._buf;
		_len = data._len;
		_own = data._own;
		_nt = data._nt;
		_w = data._w;
		data._buf = NULL; // mutable
	}

	ALWAYS_INLINE void operator=( const JLData &data ) {

		void *tmp = data._buf; // allow to write: data = data;
		data._buf = NULL; // mutable
		if ( _buf && _own )
			jl_free(_buf);

		_buf = tmp;
		_len = data._len;
		_own = data._own;
		_nt = data._nt;
		_w = data._w;
	}

	//ALWAYS_INLINE JLData& operator *() {
	//	// workarount to:
	//	// C4239: nonstandard extension used : 'argument' : conversion from 'FOO' to 'FOO &'
	//	// A non-const reference may only be bound to an lvalue; copy constructor takes a reference to non-const
	//	//
	//	// The problem is that a fundamental requirement of standard containers is that objects are copy-constructible.
	//	// That not only means that they have a copy constructor, but that also means that if you copy the object, the copy and the original are the same.
	//	return *this;
	//}

	ALWAYS_INLINE bool IsSet() const {
		
		return _buf != NULL;
	}

	ALWAYS_INLINE bool IsWide() const {
		
		ASSERT( IsSet() );
		return _w;
	}

	ALWAYS_INLINE bool IsNullTerminated() const {
		
		ASSERT( IsSet() );
		return _nt;
	}

	ALWAYS_INLINE size_t Length() {
		
		ASSERT( IsSet() );
		if ( _len != SIZE_MAX ) // known length
			return _len;
		return Length_slow();
	}

	ALWAYS_INLINE size_t LengthOrZero() {

		return IsSet() ? Length() : 0;
	}

	// raw data
	ALWAYS_INLINE JLData( const void *buf, size_t size ) : _buf((void*)buf), _len(size), _own(false), _nt(false), _w(false) { IFDEBUG(Check()); }
	ALWAYS_INLINE JLData(       void *buf, size_t size ) : _buf(buf), _len(size), _own(true), _nt(false), _w(false) { IFDEBUG(Check()); }
	ALWAYS_INLINE JLData(       void *buf, size_t size, bool own, bool nullTerminated, bool wide ) : _buf(buf), _len(size), _own(own), _nt(nullTerminated), _w(wide) { IFDEBUG(Check()); }

	// jschar
	ALWAYS_INLINE JLData( const jschar *str, bool nullTerminated, size_t length = SIZE_MAX ) : _buf((void*)str), _len(length), _own(false), _nt(nullTerminated), _w(true) { IFDEBUG(Check()); }
	ALWAYS_INLINE JLData(       jschar *str, bool nullTerminated, size_t length = SIZE_MAX ) : _buf((void*)str), _len(length), _own(true), _nt(nullTerminated), _w(true) { IFDEBUG(Check()); }

	// char
	ALWAYS_INLINE JLData( const char *str, bool nullTerminated, size_t length = SIZE_MAX ) : _buf((void*)str), _len(length), _own(false), _nt(nullTerminated), _w(false) { IFDEBUG(Check()); }
	ALWAYS_INLINE JLData(       char *str, bool nullTerminated, size_t length = SIZE_MAX ) : _buf((void*)str), _len(length), _own(true), _nt(nullTerminated), _w(false) { IFDEBUG(Check()); }

	static ALWAYS_INLINE JLData Empty() {

		return JLData("", true, 0);
	}

	// other
	ALWAYS_INLINE JLData( JSContext *cx, JSString *jsstr ) : _own(false), _nt(false), _w(true) {
	
		_buf = (void*)JS_GetStringCharsAndLength(cx, jsstr, &_len); // see also JS_GetStringCharsZAndLength
		IFDEBUG(Check());
	}

	ALWAYS_INLINE void CopyTo( jschar *dst ) {

		ASSERT( IsSet() );
		size_t length = Length();
		if ( _w ) {

			jl_memcpy(dst, _buf, length*2);
		} else {

			const char *src = (const char*)_buf;
			while ( length-- > 0 )
				*(dst++) = *(src++) & 0xFF;
		}
	}

	ALWAYS_INLINE void CopyTo( char *dst ) {

		ASSERT( IsSet() );
		size_t length = Length();
		if ( !_w ) {

			jl_memcpy(dst, _buf, length);
		} else {

			const jschar *src = (const jschar*)_buf;
			while ( length-- > 0 )
				*(dst++) = *(src++) & 0xFF;
		}
	}

	ALWAYS_INLINE void CopyTo( uint8_t *dst ) {
	
		CopyTo((char*)dst);
	}


	// jschar

	ALWAYS_INLINE const jschar* GetConstWStr() {

		if ( !Mutate(false, false, true) )
			return NULL;
		ASSERT( _w );
		return (const jschar*)_buf;
	}

	ALWAYS_INLINE const jschar* GetConstWStrZ() {

		if ( !Mutate(false, true, true) )
			return NULL;
		ASSERT( _w && _nt );
		return (const jschar*)_buf;
	}

	ALWAYS_INLINE jschar* GetWStrOwnership() {

		if ( !Mutate(true, false, true) )
			return NULL;
		ASSERT( _w && _own );
		_own = false;
		jschar *_tmp = (jschar*)_buf;
		return _tmp;
	}

	ALWAYS_INLINE jschar* GetWStrZOwnership() {

		if ( !Mutate(true, true, true) )
			return NULL;
		ASSERT( _w && _nt && _own );
		_own = false;
		jschar *_tmp = (jschar*)_buf;
		return _tmp;
	}

	ALWAYS_INLINE const jschar *GetConstWStrOrNull() {

		return IsSet() ? GetConstWStr() : NULL;
	}

	ALWAYS_INLINE jschar GetWCharAt( size_t index ) {
		
		ASSERT( IsSet() );
		return _w ? ((jschar*)_buf)[index] : ((char*)_buf)[index] & 0xFF;
	}

	
	// char

	ALWAYS_INLINE const char* GetConstStr() {

		if ( !Mutate(false, false, false) )
			return NULL;
		ASSERT( !_w );
		return (const char*)_buf;
	}

	ALWAYS_INLINE const char* GetConstStrZ() {

		if ( !Mutate(false, true, false) )
			return NULL;
		ASSERT( !_w && _nt );
		return (const char*)_buf;
	}

	ALWAYS_INLINE const char* GetStrConstOrNull() {

		return IsSet() ? GetConstStr() : NULL;
	}

	ALWAYS_INLINE char* GetStrOwnership() {

		if ( !Mutate(true, false, false) )
			return NULL;
		ASSERT( !_w && _own );
		_own = false;
		char *tmp = (char*)_buf;
		return tmp;
	}

	ALWAYS_INLINE char* GetStrZOwnership() {

		if ( !Mutate(true, true, false) )
			return NULL;
		ASSERT( !_w && _nt && _own );
		_own = false;
		char *tmp = (char*)_buf;
		return tmp;
	}

	ALWAYS_INLINE const char* GetConstStrZOrNULL() {

		return IsSet() ? GetConstStrZ() : NULL;
	}

	ALWAYS_INLINE char GetCharAt( size_t index ) {
		
		ASSERT( IsSet() );
		return _w ? ((jschar*)_buf)[index] & 0xFF : ((char*)_buf)[index];
	}


	// other

	ALWAYS_INLINE JSBool GetJSString( JSContext *cx, jsval *rval ) {

		ASSERT( IsSet() );
		size_t length = Length();
		if ( length != 0 ) {

			JSString *jsstr = JL_NewUCString(cx, GetWStrZOwnership(), length);
			if ( !jsstr )
				return JS_FALSE;
			rval->setString(jsstr);
		} else {

			*rval = JL_GetEmptyStringValue(cx);
		}
		return JS_TRUE;
	}

	ALWAYS_INLINE JSBool GetArrayBuffer( JSContext *cx, jsval *rval ) {

		ASSERT( IsSet() );
		size_t length = Length();
		if ( length != 0 )
			return JL_NewBufferGetOwnership(cx, GetStrOwnership(), length, rval);
		return JL_NewEmptyBuffer(cx, rval);
	}

	ALWAYS_INLINE operator const char *() {

		return GetConstStrZ();
	}

	ALWAYS_INLINE operator const jschar *() {

		return GetConstWStrZ();
	}
};



///////////////////////////////////////////////////////////////////////////////
// convertion functions


// JLData

ALWAYS_INLINE JSBool FASTCALL
JL_JsvalToNative( JSContext *cx, jsval &val, JLData *str ) {

	if (likely( JSVAL_IS_STRING(val) )) { // for string literals

		*str = JLData(cx, JSVAL_TO_STRING(val));
		return JS_TRUE;
	}

	UNLIKELY_SPLIT_BEGIN( JSContext *cx, jsval &val, JLData *str )

	if (likely( !JSVAL_IS_PRIMITIVE(val) )) { // for NIBufferGet support

		JSObject *obj = JSVAL_TO_OBJECT(val);
		NIBufferGet fct = BufferGetInterface(cx, obj); // BufferGetNativeInterface
		if ( fct )
			return fct(cx, obj, str);

		if ( JS_IsArrayBufferObject(obj) ) {
			
			uint32_t length = JS_GetArrayBufferByteLength(obj);
			if ( length )
				*str = JLData((const char*)JS_GetArrayBufferData(obj), false, length);
			else
				*str = JLData::Empty();
			return JS_TRUE;
		}
		
		if ( js_IsTypedArray(obj) ) {

			uint32_t length = JS_GetTypedArrayLength(obj);
			if ( length ) {

				if ( JS_GetTypedArrayType(obj) == js::TypedArray::TYPE_UINT16 )
					*str = JLData((const jschar*)JS_GetTypedArrayData(obj), false, length);
				else
					*str = JLData((const char*)JS_GetTypedArrayData(obj), false, length);
			} else {

				*str = JLData::Empty();
			}
			return JS_TRUE;
		}
	}

	// fallback
	JSString *jsstr;
	jsstr = JS_ValueToString(cx, val);
	JL_CHKM( jsstr != NULL, E_VALUE, E_CONVERT, E_TY_STRING );
	val = STRING_TO_JSVAL(jsstr); // GC protection
	*str = JLData(cx, jsstr);
	return JS_TRUE;

	JL_BAD;

	UNLIKELY_SPLIT_END(cx, val, str)

}


ALWAYS_INLINE JSBool FASTCALL
JL_NativeToJsval( JSContext *cx, JLData &cval, jsval *vp, bool toArrayBuffer ) {

	return toArrayBuffer ? cval.GetJSString(cx, vp) : cval.GetArrayBuffer(cx, vp);
}


// jschar*

ALWAYS_INLINE JSBool FASTCALL
JL_NativeToJsval( JSContext *cx, const jschar *cval, size_t length, jsval *vp ) {

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

ALWAYS_INLINE JSBool FASTCALL
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
	jsstr = JL_NewUCString(cx, cval, length);
	JL_CHK( jsstr );
	*vp = STRING_TO_JSVAL(jsstr);
	return JS_TRUE;
	JL_BAD;
}


// const char * (c-strings)

ALWAYS_INLINE JSBool FASTCALL
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


ALWAYS_INLINE JSBool FASTCALL
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

ALWAYS_INLINE JSBool FASTCALL
JL_NativeToJsval( JSContext *cx, const uint8_t *cval, size_t length, jsval *vp ) {

	return JL_NativeToJsval(cx, (const char *)cval, length, vp); // use c-strings one.
}


// int8

//JSBool JL_NativeToJsval( JSContext *cx, const int8_t &num, jsval *vp );
JSBool JL_JsvalToNative( JSContext *cx, const jsval &val, int8_t *num );


// uint8

ALWAYS_INLINE JSBool FASTCALL
JL_NativeToJsval( JSContext *cx, const uint8_t &num, jsval *vp ) {

	JL_IGNORE(cx);
	*vp = INT_TO_JSVAL(num);
	return JS_TRUE;
}


ALWAYS_INLINE JSBool FASTCALL
JL_JsvalToNative( JSContext *cx, const jsval &val, uint8_t *num ) {

	if (likely( JSVAL_IS_INT(val) )) {

		int tmp = JSVAL_TO_INT(val);
		if (likely( tmp <= UINT8_MAX && tmp >= 0 )) {

			*num = uint8_t(tmp);
			return JS_TRUE;
		}

		JL_ERR( E_VALUE, E_RANGE, E_INTERVAL_NUM(0, UINT8_MAX) );
	}

	UNLIKELY_SPLIT_BEGIN( JSContext *cx, const jsval &val, uint8_t *num )

	double d;
	if (likely( JSVAL_IS_DOUBLE(val) ))
		d = JSVAL_TO_DOUBLE(val);
	else
		JL_CHK( JS_ValueToNumber(cx, val, &d) );

	if (likely( d >= double(0) && d <= double(UINT8_MAX) )) {

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

ALWAYS_INLINE JSBool FASTCALL
JL_JsvalToNative( JSContext *cx, const jsval &val, int16_t *num ) {

	if (likely( JSVAL_IS_INT(val) )) {

		int tmp = JSVAL_TO_INT(val);
		if (likely( tmp <= INT16_MAX && tmp >= INT16_MIN )) {

			*num = int16_t(tmp);
			return JS_TRUE;
		}
		JL_ERR( E_VALUE, E_RANGE, E_INTERVAL_NUM(INT16_MIN, INT16_MAX) );
	}

	UNLIKELY_SPLIT_BEGIN( JSContext *cx, const jsval &val, int16_t *num )

	double d;
	if (likely( JSVAL_IS_DOUBLE(val) ))
		d = JSVAL_TO_DOUBLE(val);
	else
		JL_CHK( JS_ValueToNumber(cx, val, &d) );

	if (likely( d >= double(INT16_MIN) && d <= double(INT16_MAX) )) {

		JL_ASSERT_WARN( JL_DOUBLE_IS_INTEGER(d), E_VALUE, E_PRECISION );
		*num = int16_t(d);
		return JS_TRUE;
	}

	JL_ERR( E_VALUE, E_RANGE, E_INTERVAL_NUM(INT16_MIN, INT16_MAX) );
	JL_BAD;

	UNLIKELY_SPLIT_END(cx, val, num)

}


// uint16


ALWAYS_INLINE JSBool FASTCALL
JL_NativeToJsval( JSContext *cx, const uint16_t &num, jsval *vp ) {

	JL_IGNORE(cx);
	*vp = INT_TO_JSVAL(num);
	return JS_TRUE;
}


ALWAYS_INLINE JSBool FASTCALL
JL_JsvalToNative( JSContext *cx, const jsval &val, uint16_t *num ) {

	if (likely( JSVAL_IS_INT(val) )) {

		int tmp = JSVAL_TO_INT(val);
		if (likely( tmp <= UINT16_MAX )) {

			*num = uint16_t(tmp);
			return JS_TRUE;
		}
		JL_ERR( E_VALUE, E_RANGE, E_INTERVAL_NUM(0, UINT16_MAX) );
	}

	UNLIKELY_SPLIT_BEGIN( JSContext *cx, const jsval &val, uint16_t *num )

	double d;
	if (likely( JSVAL_IS_DOUBLE(val) ))
		d = JSVAL_TO_DOUBLE(val);
	else
		JL_CHK( JS_ValueToNumber(cx, val, &d) );

	if (likely( d >= double(0) && d <= double(UINT16_MAX) )) {

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

ALWAYS_INLINE JSBool FASTCALL
JL_NativeToJsval( JSContext *cx, const int32_t &num, jsval *vp ) {

	JL_IGNORE(cx);
	*vp = INT_TO_JSVAL(num);
	return JS_TRUE;
}


ALWAYS_INLINE JSBool FASTCALL
JL_JsvalToNative( JSContext *cx, const jsval &val, int32_t *num ) {

	if (likely( JSVAL_IS_INT(val) )) {

		*num = JSVAL_TO_INT(val);
		return JS_TRUE;
	}

	UNLIKELY_SPLIT_BEGIN( JSContext *cx, const jsval &val, int32_t *num )

	double d;
	if (likely( JSVAL_IS_DOUBLE(val) ))
		d = JSVAL_TO_DOUBLE(val);
	else
		JL_CHK( JS_ValueToNumber(cx, val, &d) ); // NULL gives 0

	if (likely( d >= double(INT32_MIN) && d <= double(INT32_MAX) )) {

		JL_ASSERT_WARN( JL_DOUBLE_IS_INTEGER(d), E_VALUE, E_PRECISION );
		*num = int32_t(d);
		return JS_TRUE;
	}

	JL_ERR( E_VALUE, E_RANGE, E_INTERVAL_STR("-2^31", "2^31-1") );
	JL_BAD;

	UNLIKELY_SPLIT_END(cx, val, num)

}


// uint32_t

S_ASSERT( UINT32_MAX >= JSVAL_INT_MAX );

/*
namespace jlpv {
	INLINE NEVER_INLINE JSBool
	NativeToJsval_slow( const uint32_t &num, jsval *vp ) {

		ASSERT( double(num) <= MAX_INT_TO_DOUBLE );
		*vp = DOUBLE_TO_JSVAL(double(num));
		return JS_TRUE;
	}
}
*/

ALWAYS_INLINE JSBool FASTCALL
JL_NativeToJsval( JSContext *cx, const uint32_t &num, jsval *vp ) {

	JL_IGNORE(cx);
	if (likely( num <= uint32_t(JSVAL_INT_MAX) )) {

		*vp = INT_TO_JSVAL(num);
		return JS_TRUE;
	}
//	return jlpv::NativeToJsval_slow(num, vp);

	UNLIKELY_SPLIT_BEGIN( const uint32_t &num, jsval *vp )

		ASSERT( double(num) <= MAX_INT_TO_DOUBLE );
		*vp = DOUBLE_TO_JSVAL(double(num));
		return JS_TRUE;

	UNLIKELY_SPLIT_END(num, vp)

}

ALWAYS_INLINE JSBool FASTCALL
JL_JsvalToNative( JSContext *cx, const jsval &val, uint32_t *num ) {

	if (likely( JSVAL_IS_INT(val) )) {

		int tmp = JSVAL_TO_INT(val);
		if (likely( tmp >= 0 )) {

			*num = uint32_t(tmp);
			return JS_TRUE;
		}
		JL_ERR( E_VALUE, E_RANGE, E_INTERVAL_STR("0", "2^32") );
	}

	UNLIKELY_SPLIT_BEGIN( JSContext *cx, const jsval &val, uint32_t *num )

	double d;
	if (likely( JSVAL_IS_DOUBLE(val) ))
		d = JSVAL_TO_DOUBLE(val);
	else
		JL_CHK( JS_ValueToNumber(cx, val, &d) );

	if (likely( d >= double(0) && d <= double(UINT32_MAX) )) {

		JL_ASSERT_WARN( JL_DOUBLE_IS_INTEGER(d), E_VALUE, E_PRECISION );
		*num = uint32_t(d);
		return JS_TRUE;
	}

	JL_ERR( E_VALUE, E_RANGE, E_INTERVAL_STR("0", "2^32") );
	JL_BAD;

	UNLIKELY_SPLIT_END(cx, val, num)

}


// int64

ALWAYS_INLINE JSBool FASTCALL
JL_NativeToJsval( JSContext *cx, const int64_t &num, jsval *vp ) {

	if (likely( num <= int64_t(JSVAL_INT_MAX) && num >= int64_t(JSVAL_INT_MIN) )) {

		*vp = INT_TO_JSVAL(int(num));
	} else {

		JL_CHKM( num >= int64_t(MIN_INT_TO_DOUBLE) && num <= int64_t(MAX_INT_TO_DOUBLE), E_VALUE, E_RANGE, E_INTERVAL_STR("-2^53", "2^53") );
		*vp = DOUBLE_TO_JSVAL(double(num));
	}
	return JS_TRUE;
	JL_BAD;
}

ALWAYS_INLINE JSBool FASTCALL
JL_JsvalToNative( JSContext *cx, const jsval &val, int64_t *num ) {

	if (likely( JSVAL_IS_INT(val) )) {

		*num = JSVAL_TO_INT(val);
		return JS_TRUE;
	}

	UNLIKELY_SPLIT_BEGIN( JSContext *cx, const jsval &val, int64_t *num )

	double d;
	if (likely( JSVAL_IS_DOUBLE(val) ))
		d = JSVAL_TO_DOUBLE(val);
	else
		JL_CHK( JS_ValueToNumber(cx, val, &d) ); // NULL gives 0

	if (likely( d >= double(MIN_INT_TO_DOUBLE) && d <= double(MAX_INT_TO_DOUBLE) )) {

		JL_ASSERT_WARN( JL_DOUBLE_IS_INTEGER(d), E_VALUE, E_PRECISION );
		*num = int64_t(d);
		return JS_TRUE;
	}

	JL_ERR( E_VALUE, E_RANGE, E_INTERVAL_STR("-2^53", "2^53") );
	JL_BAD;

	UNLIKELY_SPLIT_END(cx, val, num)

}


// uint64

ALWAYS_INLINE JSBool FASTCALL
JL_NativeToJsval( JSContext *cx, const uint64_t &num, jsval *vp ) {

	if ( num <= uint64_t(JSVAL_INT_MAX) ) {

		*vp = INT_TO_JSVAL(int32_t(num));
		return JS_TRUE;
	}

	if ( num <= uint64_t(MAX_INT_TO_DOUBLE) ) {

		*vp = DOUBLE_TO_JSVAL(double(num));
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
	*vp = DOUBLE_TO_JSVAL(double(num));
	return JS_TRUE;
	JL_BAD;

	UNLIKELY_SPLIT_END(cx, num, vp);
*/
}


ALWAYS_INLINE JSBool FASTCALL
JL_JsvalToNative( JSContext *cx, const jsval &val, uint64_t *num ) {

	if (likely( JSVAL_IS_INT(val) )) {

		*num = JSVAL_TO_INT(val);
		return JS_TRUE;
	}

	UNLIKELY_SPLIT_BEGIN( JSContext *cx, const jsval &val, uint64_t *num )

	double d;
	if (likely( JSVAL_IS_DOUBLE(val) ))
		d = JSVAL_TO_DOUBLE(val);
	else
		JL_CHK( JS_ValueToNumber(cx, val, &d) ); // NULL gives 0

	if (likely( d >= double(0) && d <= double(MAX_INT_TO_DOUBLE) )) { // or d <= double(UINT64_MAX)

		JL_ASSERT_WARN( JL_DOUBLE_IS_INTEGER(d), E_VALUE, E_PRECISION );
		*num = uint64_t(d);
		return JS_TRUE;
	}

	JL_ERR( E_VALUE, E_RANGE, E_INTERVAL_STR("0", "2^53") ); // 0x20000000000000
	JL_BAD;

	UNLIKELY_SPLIT_END(cx, val, num)

}


// long

ALWAYS_INLINE JSBool FASTCALL
JL_NativeToJsval( JSContext *cx, const long &num, jsval *vp ) {

	if ( sizeof(long) == sizeof(int64_t) ) return JL_NativeToJsval(cx, int64_t(num), vp);
	if ( sizeof(long) == sizeof(int32_t) ) return JL_NativeToJsval(cx, int32_t(num), vp);
	ASSERT(false);
}

ALWAYS_INLINE JSBool FASTCALL
JL_JsvalToNative( JSContext *cx, const jsval &val, long *num ) {

	if ( sizeof(long) == sizeof(int64_t) ) return JL_JsvalToNative(cx, val, (int64_t*)num);
	if ( sizeof(long) == sizeof(int32_t) ) return JL_JsvalToNative(cx, val, (int32_t*)num);
	ASSERT(false);
}


// unsigned long

ALWAYS_INLINE JSBool FASTCALL
JL_NativeToJsval( JSContext *cx, const unsigned long &num, jsval *vp ) {

	if ( sizeof(unsigned long) == sizeof(uint64_t) ) return JL_NativeToJsval(cx, uint64_t(num), vp);
	if ( sizeof(unsigned long) == sizeof(uint32_t) ) return JL_NativeToJsval(cx, uint32_t(num), vp);
	ASSERT(false);
}

ALWAYS_INLINE JSBool FASTCALL
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
		*vp = INT_TO_JSVAL(int(num));
	else
		*vp = DOUBLE_TO_JSVAL(double(num));
	return JS_TRUE;
}

ALWAYS_INLINE JSBool JL_JsvalToNative( JSContext *cx, const jsval &val, size_t *num ) {

	if (likely( JSVAL_IS_INT(val) )) {

		int tmp = JSVAL_TO_INT(val);
		if (unlikely( tmp < 0 ))
			JL_REPORT_ERROR_NUM( JLSMSG_VALUE_OUTOFRANGE, ...);
		*num = size_t(uint32_t(tmp));
		return JS_TRUE;
	}

	double d;
	if (likely( JSVAL_IS_DOUBLE(val) ))
		d = JSVAL_TO_DOUBLE(val);
	else
		JL_CHK( JS_ValueToNumber(cx, val, &d) );

	if (likely( d >= double(0) && d <= double(SIZE_T_MAX) )) { // cannot use jl::IsSafeCast(d, *i) because if d is not integer, the test fails.

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
		*vp = INT_TO_JSVAL(int(num));
	else
		*vp = DOUBLE_TO_JSVAL(double(num));
	return JS_TRUE;
}

ALWAYS_INLINE JSBool JL_JsvalToNative( JSContext *cx, const jsval &val, ptrdiff_t *num ) {

	if (likely( JSVAL_IS_INT(val) )) {

		*num = ptrdiff_t(JSVAL_TO_INT(val));
		return JS_TRUE;
	}

	double d;
	if (likely( JSVAL_IS_DOUBLE(val) ))
		d = JSVAL_TO_DOUBLE(val);
	else
		JL_CHK( JS_ValueToNumber(cx, val, &d) );

	if (likely( d >= double(PTRDIFF_MIN) && d <= double(PTRDIFF_MAX) )) {

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

ALWAYS_INLINE JSBool FASTCALL
JL_NativeToJsval( JSContext *cx, const double &num, jsval *vp ) {

	JL_IGNORE(cx);
	*vp = DOUBLE_TO_JSVAL(num);
	return JS_TRUE;
}

ALWAYS_INLINE JSBool FASTCALL
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

	JL_CHK( JS_ValueToNumber(cx, val, num) );
	ASSERT( JSDOUBLE_IS_NaN(JSVAL_TO_DOUBLE(JL_GetNaNValue(cx))) );
	JL_CHKM( !JSDOUBLE_IS_NaN(*num), E_VALUE, E_TYPE, E_TY_NUMBER );
	return JS_TRUE;
	JL_BAD;

	UNLIKELY_SPLIT_END(cx, val, num)

}


// float

ALWAYS_INLINE JSBool FASTCALL
JL_NativeToJsval( JSContext *cx, const float &num, jsval *vp ) {

	JL_IGNORE(cx);
	*vp = DOUBLE_TO_JSVAL(double(num));
	return JS_TRUE;
}

ALWAYS_INLINE JSBool FASTCALL
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

	double tmp;
	if ( !JS_ValueToNumber(cx, val, &tmp) )
		return JS_FALSE;
	ASSERT( JSDOUBLE_IS_NaN(JSVAL_TO_DOUBLE(JL_GetNaNValue(cx))) );
	JL_CHKM( !JSDOUBLE_IS_NaN(tmp), E_VALUE, E_TYPE, E_TY_NUMBER );
	*num = float(tmp);
	return JS_TRUE;
	JL_BAD;

	UNLIKELY_SPLIT_END(cx, val, num)

}


// boolean

ALWAYS_INLINE JSBool FASTCALL
JL_NativeToJsval( JSContext *cx, const bool &b, jsval *vp ) {

	JL_IGNORE(cx);
	*vp = BOOLEAN_TO_JSVAL(b);
	return JS_TRUE;
}

ALWAYS_INLINE JSBool FASTCALL
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

ALWAYS_INLINE JSBool FASTCALL
JL_NativeToJsval( JSContext *cx, void *ptr, jsval *vp ) {

	if ( ((uint32_t)ptr & 1) == 0 ) {

		*vp = PRIVATE_TO_JSVAL(ptr);
	} else {

		JSObject *obj = JL_NewProtolessObj(cx);
		JL_CHK( obj );
		*vp = OBJECT_TO_JSVAL(obj);
		jsval tmp;

		if ( 8 * sizeof(ptrdiff_t) == 32 ) {

			tmp = INT_TO_JSVAL( reinterpret_cast<int32_t>(ptr) );
			JL_CHK( JS_SetPropertyById(cx, obj, INT_TO_JSID(0), &tmp) );
		} else
		if ( 8 * sizeof(ptrdiff_t) == 64 ) {

			#ifdef XP_WIN
			#pragma warning(push)
			#pragma warning(disable:4293)
			#endif // XP_WIN
			tmp = INT_TO_JSVAL( reinterpret_cast<ptrdiff_t>(ptr) & 0xFFFFFFFF );
			JL_CHK( JS_SetPropertyById(cx, obj, INT_TO_JSID(0), &tmp) );
			tmp = INT_TO_JSVAL( reinterpret_cast<ptrdiff_t>(ptr) >> 32 );
			JL_CHK( JS_SetPropertyById(cx, obj, INT_TO_JSID(1), &tmp) );
			#ifdef XP_WIN
			#pragma warning(pop)
			#endif // XP_WIN
		} else {

			ASSERT(false);
		}
	}
	return JS_TRUE;
	JL_BAD;
}

ALWAYS_INLINE JSBool FASTCALL
JL_JsvalToNative( JSContext *cx, const jsval &val, void **ptr ) {

	if ( JSVAL_IS_OBJECT(val) ) {

		jsval tmp;
		JSObject *obj = JSVAL_TO_OBJECT(val);

		if ( 8 * sizeof(ptrdiff_t) == 32 ) {

			JL_CHK( JS_GetPropertyById(cx, obj, INT_TO_JSID(0), &tmp) );
			*ptr = reinterpret_cast<void*>( JSVAL_TO_INT(tmp) );
		} else
		if ( 8 * sizeof(ptrdiff_t) == 64 ) {

	#ifdef XP_WIN
	#endif // XP_WIN

			#ifdef XP_WIN
			#pragma warning(push)
			#pragma warning(disable:4293)
			#endif // XP_WIN
			uint32_t h, l;
			JL_CHK( JS_GetPropertyById(cx, obj, INT_TO_JSID(0), &tmp) );
			l = static_cast<uint32_t>(JSVAL_TO_INT(tmp));
			JL_CHK( JS_GetPropertyById(cx, obj, INT_TO_JSID(1), &tmp) );
			h = static_cast<uint32_t>(JSVAL_TO_INT(tmp));
			*ptr = reinterpret_cast<void*>( (h << 32) | l );
			#ifdef XP_WIN
			#pragma warning(pop)
			#endif // XP_WIN
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
JL_NativeVectorToJsval( JSContext * RESTRICT cx, const T * RESTRICT vector, unsigned length, jsval * RESTRICT val, bool useValArray = false ) {

	ASSERT( vector );
	ASSERT( val );

	jsval tmp;
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

	for ( unsigned i = 0; i < length; ++i ) {

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
JL_TypedArrayToNativeVector( JSContext * RESTRICT cx, JSObject * RESTRICT obj, T * RESTRICT vector, unsigned maxLength, unsigned * RESTRICT actualLength ) {

	ASSERT( js_IsTypedArray(obj) );
	JL_ASSERT( JS_GetTypedArrayType(obj) == JLNativeTypeToTypedArrayType(*vector), E_TY_TYPEDARRAY, E_TYPE, E_NAME(JLNativeTypeToString(*vector)) );
	void *data;
	data = JS_GetTypedArrayData(obj);
	*actualLength = JS_GetTypedArrayLength(obj);
	maxLength = JL_MIN( *actualLength, maxLength );
	for ( unsigned i = 0; i < maxLength; ++i ) {

		vector[i] = ((T*)data)[i];
	}
	return JS_TRUE;
	JL_BAD;
}

template <class T>
INLINE JSBool FASTCALL
JL_ArrayBufferToNativeVector( JSContext * RESTRICT cx, JSObject * RESTRICT obj, T * RESTRICT vector, unsigned maxLength, unsigned * RESTRICT actualLength ) {
	
	JL_IGNORE(cx);
	ASSERT( js_IsArrayBuffer(obj) );
	uint8_t *buffer = JS_GetArrayBufferData(obj);
	ASSERT( buffer != NULL );
	*actualLength = JS_GetArrayBufferByteLength(obj);
	maxLength = JL_MIN( *actualLength, maxLength );
	jl_memcpy((uint8_t*)vector, buffer, maxLength);
	return JS_TRUE;
	JL_BAD;
}


// supports Array-like objects and typedArray
template <class T>
ALWAYS_INLINE JSBool FASTCALL
JL_JsvalToNativeVector( JSContext * RESTRICT cx, jsval & RESTRICT val, T * RESTRICT vector, unsigned maxLength, unsigned *actualLength ) {

	jsval tmp;

	JL_ASSERT_IS_OBJECT(val, "vector");

	JSObject *arrayObj;
	arrayObj = JSVAL_TO_OBJECT(val);

	if (unlikely( js_IsTypedArray(arrayObj) ))
		return JL_TypedArrayToNativeVector(cx, arrayObj, vector, maxLength, actualLength);

	if (unlikely( js_IsArrayBuffer(arrayObj) )) {
		
		if ( sizeof(*vector) == 1 )
			return JL_ArrayBufferToNativeVector(cx, arrayObj, (uint8_t *)vector, maxLength, actualLength);
		else
			JL_ERR( E_TY_ARRAYBUFFER, E_UNEXP );
	}

	JL_CHK( JS_GetArrayLength(cx, arrayObj, actualLength) );
	maxLength = JL_MIN( *actualLength, maxLength );
	for ( unsigned i = 0; i < maxLength; ++i ) {  // while ( maxLength-- ) { // avoid reverse walk (L1 cache issue)

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
JL_NativeToReservedSlot( JSContext * RESTRICT cx, JSObject * RESTRICT obj, unsigned slot, T &value ) {

	jsval tmp;
	JL_CHK( JL_NativeToJsval(cx, value, &tmp) );
	JL_CHK( JL_SetReservedSlot(cx, obj, slot, tmp) );
	return JS_TRUE;
	JL_BAD;
}


template <class T>
ALWAYS_INLINE JSBool FASTCALL
JL_ReservedSlotToNative( JSContext * RESTRICT cx, JSObject * RESTRICT obj, unsigned slot, T * RESTRICT value ) {

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
ALWAYS_INLINE JSBool FASTCALL
JL_NativeToProperty( JSContext *cx, JSObject *obj, const char *name, const T &cval ) {

	jsval tmp;
	return JL_NativeToJsval(cx, cval, &tmp) && JS_SetProperty(cx, obj, name, &tmp);
}

template <class T>
ALWAYS_INLINE JSBool FASTCALL
JL_NativeToProperty( JSContext *cx, JSObject *obj, jsid id, const T &cval ) {

	jsval tmp;
	return JL_NativeToJsval(cx, cval, &tmp) && JS_SetPropertyById(cx, obj, id, &tmp);
}


// Define

template <class T>
ALWAYS_INLINE JSBool FASTCALL
JL_DefineProperty( JSContext *cx, JSObject *obj, const char *name, const T &cval, bool visible = true, bool modifiable = true ) {

	jsval tmp;
	return JL_NativeToJsval(cx, cval, &tmp) && JS_DefineProperty(cx, obj, name, tmp, NULL, NULL, (modifiable ? 0 : JSPROP_READONLY | JSPROP_PERMANENT) | (visible ? JSPROP_ENUMERATE : 0) );
}

ALWAYS_INLINE JSBool FASTCALL
JL_DefineProperty( JSContext *cx, JSObject *obj, const char *name, const jsval &val, bool visible = true, bool modifiable = true ) {

	return JS_DefineProperty(cx, obj, name, val, NULL, NULL, (modifiable ? 0 : JSPROP_READONLY | JSPROP_PERMANENT) | (visible ? JSPROP_ENUMERATE : 0) );
}


template <class T>
ALWAYS_INLINE JSBool FASTCALL
JL_DefineProperty( JSContext *cx, JSObject *obj, jsid id, const T &cval, bool visible = true, bool modifiable = true ) {

	jsval tmp;
	return JL_NativeToJsval(cx, cval, &tmp) && JS_DefinePropertyById(cx, obj, id, tmp, NULL, NULL, (modifiable ? 0 : JSPROP_READONLY | JSPROP_PERMANENT) | (visible ? JSPROP_ENUMERATE : 0) );
}

ALWAYS_INLINE JSBool FASTCALL
JL_DefineProperty( JSContext *cx, JSObject *obj, jsid id, const jsval &val, bool visible = true, bool modifiable = true ) {

	return JS_DefinePropertyById(cx, obj, id, val, NULL, NULL, (modifiable ? 0 : JSPROP_READONLY | JSPROP_PERMANENT) | (visible ? JSPROP_ENUMERATE : 0) );
}


// Get

template <class T>
ALWAYS_INLINE JSBool FASTCALL
JL_PropertyToNative( JSContext *cx, JSObject *obj, const char *propertyName, T *cval ) {

	jsval tmp;
	return JS_GetProperty(cx, obj, propertyName, &tmp) && JL_JsvalToNative(cx, tmp, cval);
}

template <class T>
ALWAYS_INLINE JSBool FASTCALL
JL_PropertyToNative( JSContext *cx, JSObject *obj, jsid id, T *cval ) {

	jsval tmp;
	return JS_GetPropertyById(cx, obj, id, &tmp) && JL_JsvalToNative(cx, tmp, cval);
}

// Lookup

template <class T>
ALWAYS_INLINE JSBool FASTCALL
JL_LookupProperty( JSContext *cx, JSObject *obj, const char *propertyName, T *cval ) {

	jsval tmp;
	return JS_LookupProperty(cx, obj, propertyName, &tmp) && JL_JsvalToNative(cx, tmp, cval);
}

template <class T>
ALWAYS_INLINE JSBool FASTCALL
JL_LookupProperty( JSContext *cx, JSObject *obj, jsid id, T *cval ) {

	jsval tmp;
	return JS_LookupPropertyById(cx, obj, id, &tmp) && JL_JsvalToNative(cx, tmp, cval);
}


///////////////////////////////////////////////////////////////////////////////
// jsval convertion functions

/* need to create template-based variants of this function to handle all types supported by type arrays.
INLINE JSBool FASTCALL
JL_JSArrayToBuffer( JSContext * RESTRICT cx, JSObject * RESTRICT arrObj, JLData * RESTRICT str ) {

	ASSERT( JL_ObjectIsArray(cx, arrObj) );
	unsigned length;
	JL_CHK( JS_GetArrayLength(cx, arrObj, &length) );

	jschar *buf;
	buf = static_cast<jschar*>(jl_malloc(sizeof(jschar) * (length +1)));
	buf[length] = 0;

	jsval elt;
	int32_t num;
	for ( unsigned i = 0; i < length; ++i ) {

		JL_CHK( JL_GetElement(cx, arrObj, i, &elt) );
		JL_CHK( JL_JsvalToNative(cx, elt, &num) ); //JL_CHK( JS_ValueToInt32(cx, elt, &num) );
		buf[i] = (jschar)num;
	}
	*str = JLData(buf, length, true);
	return JS_TRUE;
	JL_BAD;
}
*/

ALWAYS_INLINE JSFunction* FASTCALL
JL_ObjectToFunction( JSContext *cx, JSObject *obj ) {

	JL_IGNORE(cx);
	//return GET_FUNCTION_PRIVATE(cx, obj);
	return JS_ValueToFunction(cx, OBJECT_TO_JSVAL(obj));
}

ALWAYS_INLINE JSFunction* FASTCALL
JL_JsvalToFunction( JSContext *cx, jsval &val ) {

	JL_IGNORE(cx);
	//return GET_FUNCTION_PRIVATE(cx, JSVAL_TO_OBJECT(val));
	return JS_ValueToFunction(cx, val);
}

ALWAYS_INLINE jsval FASTCALL
JL_FunctionToJsval(JSContext *cx, JSNative call, unsigned nargs, unsigned flags, JSObject *parent, jsid id) {

	return OBJECT_TO_JSVAL(JS_GetFunctionObject(JS_NewFunctionById(cx, call , nargs, flags, parent, id)));
}


ALWAYS_INLINE JSBool FASTCALL
JL_JsvalToJsid( JSContext * RESTRICT cx, jsval val, jsid * RESTRICT id ) {

	if ( JSVAL_IS_STRING( val ) ) {

		*id = JL_StringToJsid(cx, JSVAL_TO_STRING( val ));
		ASSERT( JSID_IS_STRING( *id ) );
	} else
	if ( JSVAL_IS_INT( val ) && INT_FITS_IN_JSID( JSVAL_TO_INT( val ) ) ) {

		*id = INT_TO_JSID( JSVAL_TO_INT( val ) );
	} else
	if ( JSVAL_IS_OBJECT( val ) ) {

		*id = OBJECT_TO_JSID( JSVAL_TO_OBJECT( val ) );
	} else
	if ( JSVAL_IS_VOID( val ) ) {

		*id = JSID_VOID;
	} else
		return JS_ValueToId(cx, val, id);
	return JS_TRUE;
}


ALWAYS_INLINE JSBool FASTCALL
JL_JsidToJsval( JSContext * RESTRICT cx, jsid id, jsval * RESTRICT val ) {

	JL_IGNORE(cx);
	if (JSID_IS_STRING(id)) {
        
		*val = js::StringValue(JSID_TO_STRING(id));
	} else
	if (JS_LIKELY(JSID_IS_INT(id))) {

		*val = js::Int32Value(JSID_TO_INT(id));
	} else
	if (JS_LIKELY(JSID_IS_OBJECT(id))) {

		*val = js::ObjectValue(*JSID_TO_OBJECT(id));
	} else
	if (JS_LIKELY(JSID_IS_VOID(id))) {

		*val = js::UndefinedValue();
	} else
		return JS_IdToValue(cx, id, val);
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

		jl_memcpy(*m, &Matrix44IdentityValue, sizeof(Matrix44IdentityValue));
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

		if ( JS_GetTypedArrayType(matrixObj) == js::TypedArray::TYPE_FLOAT32 ) {
			
			if ( JS_GetTypedArrayLength(matrixObj) == 16 ) {

				jl_memcpy(*m, JS_GetTypedArrayData(matrixObj), (32 / 8) * 16);
				return JS_TRUE;
			}
		}
	}

	if ( JL_ObjectIsArrayLike(cx, matrixObj) ) {

		uint32_t length;
		jsval element;
		JL_CHK( JL_GetElement(cx, JSVAL_TO_OBJECT(val), 0, &element) );
		if ( JL_ValueIsArrayLike(cx, element) ) { // support for [ [1,1,1,1], [2,2,2,2], [3,3,3,3], [4,4,4,4] ] matrix

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
// Buffer

ALWAYS_INLINE uint8_t* FASTCALL
JL_DataBufferAlloc( JSContext *cx, size_t nbytes ) {

	JL_IGNORE(cx);
	return (uint8_t*)jl_malloc(nbytes);
}

ALWAYS_INLINE uint8_t* FASTCALL
JL_DataBufferRealloc( JSContext *cx, uint8_t *data, size_t nbytes ) {

	JL_IGNORE(cx);
	return (uint8_t*)jl_realloc(data, nbytes);
}

ALWAYS_INLINE void FASTCALL
JL_DataBufferFree( JSContext *cx, uint8_t *data ) {
	
	JL_IGNORE(cx);
	return jl_free(data);
}


//

ALWAYS_INLINE uint8_t* FASTCALL
JL_NewBuffer( JSContext *cx, size_t nbytes, jsval *rval ) {

	JSObject *bufferObj = js::ArrayBuffer::create(cx, nbytes); // JS_NewArrayBuffer(cx, nbytes);
	if ( bufferObj ) {

		*rval = OBJECT_TO_JSVAL(bufferObj);
		return JS_GetArrayBufferData(bufferObj);
	} else {

		return NULL;
	}
}


ALWAYS_INLINE bool FASTCALL
JL_NewBufferCopyN( JSContext *cx, const void *src, size_t nbytes, jsval *rval ) {

	JSObject *bufferObj = js::ArrayBuffer::create(cx, nbytes, (uint8_t*)src); // JS_NewArrayBuffer(cx, nbytes);
	if ( bufferObj ) {

		*rval = OBJECT_TO_JSVAL(bufferObj);
		return true;
	} else {

		return false;
	}
}


ALWAYS_INLINE bool FASTCALL
JL_NewBufferGetOwnership( JSContext *cx, void *src, size_t nbytes, jsval *rval ) {

	// (TBD) need to handle ownership properly
	bool ok = JL_NewBufferCopyN(cx, src, nbytes, rval);
	jl_free(src);
	return ok;
}


ALWAYS_INLINE bool FASTCALL
JL_NewEmptyBuffer( JSContext *cx, jsval *rval ) {
	
	JSObject *obj = js::ArrayBuffer::create(cx, 0);
	if ( obj ) {

		*rval = OBJECT_TO_JSVAL(obj);
		return true;
	} else {
		
		return false;
	}
}


ALWAYS_INLINE JSBool FASTCALL
JL_FreeBuffer( JSContext *cx, jsval *rval ) {

	JL_IGNORE( rval, cx );
	// do nothing at the moment. The CG will free the buffer.
	return JS_TRUE;
}


ALWAYS_INLINE uint8_t* FASTCALL
JL_ChangeBufferLength( JSContext *cx, jsval *rval, size_t nbytes ) {

	// need to create a new buffer because ArrayBuffer does not support realloc nor length changing, then we copy it in a new one.

	ASSERT( JSVAL_IS_OBJECT(*rval) );
	JSObject *arrayBufferObj = JSVAL_TO_OBJECT(*rval);
	ASSERT( JS_IsArrayBufferObject(arrayBufferObj) );
	uint32_t bufferLen = JS_GetArrayBufferByteLength(arrayBufferObj);
	void *bufferData = JS_GetArrayBufferData(arrayBufferObj);

	if ( nbytes == bufferLen )
		return (uint8_t*)bufferData;

	JSObject *newBufferObj;
	void *newBufferData;
	if ( nbytes < bufferLen ) {

		newBufferObj = js::ArrayBuffer::create(cx, nbytes, (uint8_t*)bufferData); // JS_NewArrayBuffer(cx, nbytes);
		if ( !newBufferObj )
			return false;
		newBufferData = JS_GetArrayBufferData(arrayBufferObj);
	} else {

		newBufferObj = js::ArrayBuffer::create(cx, nbytes);
		if ( !newBufferObj )
			return false;
		newBufferData = JS_GetArrayBufferData(arrayBufferObj);
		jl_memcpy(newBufferData, bufferData, bufferLen);
	}
	*rval = OBJECT_TO_JSVAL(newBufferObj);
	return (uint8_t*)newBufferData;
}



///////////////////////////////////////////////////////////////////////////////
// Generic Image object

template <class T, class U>
ALWAYS_INLINE uint8_t* FASTCALL
JL_NewByteImageObject( JSContext *cx, T width, T height, U channels, jsval *rval ) {

	ASSERT( width >= 0 && height >= 0 && channels > 0 );

	jsval dataVal;
	JSObject *imageObj = JL_NewObj(cx);
	JL_CHK( imageObj );
	*rval = OBJECT_TO_JSVAL(imageObj);
	JSObject *dataObj = js::ArrayBuffer::create(cx, width * height* channels); // JS_NewArrayBuffer(cx, nbytes);
	JL_CHK( dataObj );
	dataVal = OBJECT_TO_JSVAL(dataObj);
	JL_CHK( JS_SetPropertyById(cx, imageObj, JLID(cx, data), &dataVal) );
	JL_CHK( JL_NativeToProperty(cx, imageObj, JLID(cx, width), width) );
	JL_CHK( JL_NativeToProperty(cx, imageObj, JLID(cx, height), height) );
	JL_CHK( JL_NativeToProperty(cx, imageObj, JLID(cx, channels), channels) );
	return JS_GetArrayBufferData(dataObj);
bad:
	return NULL;
}

template <class T, class U>
ALWAYS_INLINE JSBool FASTCALL
JL_NewByteImageObjectOwner( JSContext *cx, uint8_t* buffer, T width, T height, U channels, jsval *rval ) {

	ASSERT_IF( buffer == NULL, width * height * channels == 0 );
	ASSERT_IF( buffer != NULL, width > 0 && height > 0 && channels > 0 );
	ASSERT_IF( buffer != NULL, jl_msize(buffer) >= (size_t)(width * height * channels) );

	jsval dataVal;
	JSObject *imageObj = JL_NewObj(cx);
	JL_CHK( imageObj );
	*rval = OBJECT_TO_JSVAL(imageObj);
	JSObject *dataObj = js::ArrayBuffer::create(cx, width * height * channels, buffer); // (TBD) handle ArrayBuffer ownership
	jl_free(buffer);
	JL_CHK( dataObj );
	dataVal = OBJECT_TO_JSVAL(dataObj);
	JL_CHK( JS_SetPropertyById(cx, imageObj, JLID(cx, data), &dataVal) );
	JL_CHK( JL_NativeToProperty(cx, imageObj, JLID(cx, width), width) );
	JL_CHK( JL_NativeToProperty(cx, imageObj, JLID(cx, height), height) );
	JL_CHK( JL_NativeToProperty(cx, imageObj, JLID(cx, channels), channels) );
	return JS_TRUE;
	JL_BAD;
}


template <class T, class U>
ALWAYS_INLINE JLData FASTCALL
JL_GetByteImageObject( JSContext *cx, jsval &val, T *width, T *height, U *channels ) {

	JLData data;
	JL_ASSERT_IS_OBJECT(val, "image");

	JSObject *imageObj = JSVAL_TO_OBJECT(val);
	JL_CHK( JL_PropertyToNative(cx, imageObj, JLID(cx, data), &data) );
	JL_CHK( JL_PropertyToNative(cx, imageObj, JLID(cx, width), width) );
	JL_CHK( JL_PropertyToNative(cx, imageObj, JLID(cx, height), height) );
	JL_CHK( JL_PropertyToNative(cx, imageObj, JLID(cx, channels), channels) );

	JL_ASSERT( width >= 0 && height >= 0 && channels > 0, E_STR("image"), E_FORMAT );
	JL_ASSERT( data.IsSet() && jl::SafeCast<int>(data.Length()) == *width * *height * *channels * 1, E_DATASIZE, E_INVALID );
	return data;
bad:
	return JLData();
}


///////////////////////////////////////////////////////////////////////////////
// Generic Audio object

template <class T, class U, class V,  class W>
ALWAYS_INLINE uint8_t* FASTCALL
JL_NewByteAudioObject( JSContext *cx, T bits, U channels, V frames, W rate, jsval *rval ) {

	ASSERT( bits > 0 && (bits % 8) == 0 && channels > 0 && frames >= 0 && rate > 0 );

	jsval dataVal;
	JSObject *audioObj = JL_NewObj(cx);
	JL_CHK( audioObj );
	*rval = OBJECT_TO_JSVAL(audioObj);
	JSObject *dataObj = js::ArrayBuffer::create(cx, (bits/8) * channels * frames); // JS_NewArrayBuffer(cx, nbytes);
	JL_CHK( dataObj );
	dataVal = OBJECT_TO_JSVAL(dataObj);
	JL_CHK( JS_SetPropertyById(cx, audioObj, JLID(cx, data), &dataVal) );
	JL_CHK( JL_NativeToProperty(cx, audioObj, JLID(cx, bits), bits) );
	JL_CHK( JL_NativeToProperty(cx, audioObj, JLID(cx, channels), channels) );
	JL_CHK( JL_NativeToProperty(cx, audioObj, JLID(cx, frames), frames) );
	JL_CHK( JL_NativeToProperty(cx, audioObj, JLID(cx, rate), rate) );
	return JS_GetArrayBufferData(dataObj);
bad:
	return NULL;
}

template <class T, class U, class V,  class W>
ALWAYS_INLINE JSBool FASTCALL
JL_NewByteAudioObjectOwner( JSContext *cx, uint8_t* buffer, T bits, U channels, V frames, W rate, jsval *rval ) {
	
	ASSERT_IF( buffer == NULL, frames == 0 );
	ASSERT( bits > 0 && (bits % 8) == 0 && channels > 0 && frames >= 0 && rate > 0 );
	ASSERT_IF( buffer != NULL, jl_msize(buffer) >= (size_t)( (bits/8) * channels * frames ) );

	jsval dataVal;
	JSObject *audioObj = JL_NewObj(cx);
	JL_CHK( audioObj );
	*rval = OBJECT_TO_JSVAL(audioObj);
	JSObject *dataObj = js::ArrayBuffer::create(cx, (bits/8) * channels * frames, buffer); // (TBD) handle ArrayBuffer ownership
	jl_free(buffer);
	JL_CHK( dataObj );
	dataVal = OBJECT_TO_JSVAL(dataObj);
	JL_CHK( JS_SetPropertyById(cx, audioObj, JLID(cx, data), &dataVal) );
	JL_CHK( JL_NativeToProperty(cx, audioObj, JLID(cx, bits), bits) );
	JL_CHK( JL_NativeToProperty(cx, audioObj, JLID(cx, channels), channels) );
	JL_CHK( JL_NativeToProperty(cx, audioObj, JLID(cx, frames), frames) );
	JL_CHK( JL_NativeToProperty(cx, audioObj, JLID(cx, rate), rate) );
	return JS_TRUE;
	JL_BAD;
}

template <class T, class U, class V,  class W>
ALWAYS_INLINE JLData FASTCALL
JL_GetByteAudioObject( JSContext *cx, jsval &val, T *bits, U *channels, V *frames, W *rate ) {

	JLData data;
	JL_ASSERT_IS_OBJECT(val, "audio");

	JSObject *audioObj = JSVAL_TO_OBJECT(val);
	JL_CHK( JL_PropertyToNative(cx, audioObj, JLID(cx, data), &data) );
	JL_CHK( JL_PropertyToNative(cx, audioObj, JLID(cx, bits), bits) );
	JL_CHK( JL_PropertyToNative(cx, audioObj, JLID(cx, channels), channels) );
	JL_CHK( JL_PropertyToNative(cx, audioObj, JLID(cx, frames), frames) );
	JL_CHK( JL_PropertyToNative(cx, audioObj, JLID(cx, rate), rate) );
 
	JL_ASSERT( *bits > 0 && (*bits % 8) == 0 && *rate > 0 && *channels > 0 && *frames >= 0, E_STR("audio"), E_FORMAT );
	JL_ASSERT( data.IsSet() && data.Length() == (size_t)( (*bits/8) * *channels * *frames ), E_DATASIZE, E_INVALID );
	return data;
bad:
	return JLData();
}




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
ALWAYS_INLINE JSBool
JL_CreateErrorException( JSContext *cx, JSExnType exn, JSObject **obj ) {

	JSObject *proto;
	if ( !JL_GetClassPrototype(cx, JL_GetGlobal(cx), JSProtoKey(JSProto_Error + exn), &proto) || !proto )
		return JS_FALSE;

	*obj = JS_NewObject(cx, JL_GetStandardClassByKey(cx, JSProtoKey(JSProto_Error + exn)), proto, NULL);
	return JS_TRUE;
}
*/

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
JL_ReportExceptionToString( JSContext *cx, JSObject *obj, JLData  ) {

	JSErrorReporter prevEr = JS_SetErrorReporter(cx, ErrorReporter_ToString);
	JS_ReportPendingException(cx);
	JS_SetErrorReporter(cx, prevEr);
	return JS_TRUE;
}
*/

ALWAYS_INLINE bool FASTCALL
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


ALWAYS_INLINE JSContext* FASTCALL
JL_GetFirstContext(JSRuntime *rt) {

	JSContext *cx = NULL;
	ASSERT( rt != NULL );
	JS_ContextIterator(rt, &cx);
	JS_ASSERT( cx != NULL );
	return cx;
}


ALWAYS_INLINE bool FASTCALL
JL_InheritFrom( JSContext *cx, JSObject *obj, const JSClass *clasp ) {

	JL_IGNORE(cx);
	while ( obj != NULL ) {

		if ( JL_GetClass(obj) == clasp )
			return true;
		obj = JL_GetPrototype(cx, obj);
	}
	return false;
}


ALWAYS_INLINE JSBool FASTCALL
JL_CallFunctionId(JSContext *cx, JSObject *obj, jsid id, unsigned argc, jsval *argv, jsval *rval) {

	jsval tmp;
	return JS_GetMethodById(cx, obj, id, NULL, &tmp) && JS_CallFunctionValue(cx, obj, tmp, argc, argv, rval);
}


// JL_CallFunctionVA (cx, obj, function, rval, ... )

// (TBD)


// JL_CallFunctionVA (cx, obj, functionId, rval, ... )

// (TBD)


// JL_CallFunctionVA (cx, obj, functionValue, rval, ... )

ALWAYS_INLINE JSBool FASTCALL
JL_CallFunctionVA( JSContext * RESTRICT cx, JSObject * RESTRICT obj, jsval &functionValue, jsval *rval ) {

	return JS_CallFunctionValue(cx, obj, functionValue, 0, NULL, rval);
}

ALWAYS_INLINE JSBool FASTCALL
JL_CallFunctionVA( JSContext * RESTRICT cx, JSObject * RESTRICT obj, jsval &functionValue, jsval *rval, const jsval &arg1 ) {

	jsval args[] = { arg1 };
	return JS_CallFunctionValue(cx, obj, functionValue, COUNTOF(args), args, rval);
}

ALWAYS_INLINE JSBool FASTCALL
JL_CallFunctionVA( JSContext * RESTRICT cx, JSObject * RESTRICT obj, jsval &functionValue, jsval *rval, const jsval &arg1, const jsval &arg2 ) {

	jsval args[] = { arg1, arg2 };
	return JS_CallFunctionValue(cx, obj, functionValue, COUNTOF(args), args, rval);
}

ALWAYS_INLINE JSBool FASTCALL
JL_CallFunctionVA( JSContext * RESTRICT cx, JSObject * RESTRICT obj, jsval &functionValue, jsval *rval, const jsval &arg1, const jsval &arg2, const jsval &arg3 ) {

	jsval args[] = { arg1, arg2, arg3 };
	return JS_CallFunctionValue(cx, obj, functionValue, COUNTOF(args), args, rval);
}



INLINE JSBool FASTCALL
JL_Eval( JSContext *cx, JSString *source, jsval *rval ) { // used in jsvalserializer.h

 	jsval argv = STRING_TO_JSVAL(source);
	return JL_CallFunctionId(cx, JL_GetGlobal(cx), JLID(cx, eval), 1, &argv, rval); // see JS_EvaluateUCScript
}



ALWAYS_INLINE JSBool FASTCALL
JL_Push( JSContext * RESTRICT cx, JSObject * RESTRICT arr, jsval * RESTRICT value ) {

	unsigned length;
	return JS_GetArrayLength(cx, arr, &length) && JL_SetElement(cx, arr, length, value);
}


ALWAYS_INLINE JSBool FASTCALL
JL_Pop( JSContext * RESTRICT cx, JSObject * RESTRICT arr, jsval * RESTRICT vp ) {

	unsigned length;
	return JS_GetArrayLength(cx, arr, &length) && JL_GetElement(cx, arr, --length, vp) && JS_SetArrayLength(cx, arr, length); 
}


INLINE JSBool FASTCALL
JL_JsvalToPrimitive( JSContext * RESTRICT cx, const jsval &val, jsval * RESTRICT rval ) { // prev JL_ValueOf

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
	JL_CHK( JL_CallFunctionId(cx, obj, JLID(cx, valueOf), 0, NULL, rval) );
	if ( !JSVAL_IS_PRIMITIVE(*rval) )
		JL_CHK( JL_CallFunctionId(cx, obj, JLID(cx, toString), 0, NULL, rval) );

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
INLINE NEVER_INLINE JSScript* FASTCALL
JL_LoadScript(JSContext * RESTRICT cx, JSObject * RESTRICT obj, const char * RESTRICT fileName, JLEncodingType encoding, bool useCompFile, bool saveCompFile) {

	char *scriptBuffer = NULL;
	size_t scriptFileSize;
	jschar *scriptText = NULL;
	size_t scriptTextLength;

	JSScript *script = NULL;
	void *data = NULL;
	uint32_t prevOpts = JS_GetOptions(cx);

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
		int readCount = read(file, data, jl::SafeCast<unsigned int>(compFileSize)); // here we can use "Memory-Mapped I/O Functions" ( http://developer.mozilla.org/en/docs/NSPR_API_Reference:I/O_Functions#Memory-Mapped_I.2FO_Functions )
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

			JL_ASSERT_WARN( JS_GetScriptVersion(cx, script) >= JS_GetVersion(cx), E_NAME(compiledFileName), E_STR("XDR"), E_VERSION );
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
	if ( saveCompFile ) // saving the compiled file mean that we cannot promise to execute compiled script once only.
		JS_SetOptions(cx, prevOpts & ~JSOPTION_COMPILE_N_GO); // previous options a restored below.

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
		case ENC_UTF8: {

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

	JS_SetOptions(cx, prevOpts);
	return script;

bad:

	jl_freea(scriptBuffer); // jl_freea(NULL) is legal
	jl_freea(scriptText);
	JS_SetOptions(cx, prevOpts);
	jl_freea(data); // jl_free(NULL) is legal
	return NULL; // report a warning ?
}


ALWAYS_INLINE JSBool FASTCALL
ExecuteScriptText( JSContext *cx, const char *scriptText, bool compileOnly, jsval *rval ) {

	uint32_t prevOpt = JS_SetOptions(cx, JS_GetOptions(cx) /*| JSOPTION_COMPILE_N_GO*/); //  | JSOPTION_DONT_REPORT_UNCAUGHT
	// JSOPTION_COMPILE_N_GO:
	//  caller of JS_Compile*Script promises to execute compiled script once only; enables compile-time scope chain resolution of consts.
	// JSOPTION_DONT_REPORT_UNCAUGHT:
	//  When returning from the outermost API call, prevent uncaught exceptions from being converted to error reports
	//  we can use JS_ReportPendingException to report it manually

	JSObject *globalObject = JL_GetGlobal(cx);
	JL_ASSERT( globalObject != NULL, E_HOST, E_INTERNAL ); // "Global object not found."

// compile & executes the script

	//JSPrincipals *principals = (JSPrincipals*)jl_malloc(sizeof(JSPrincipals));
	//JSPrincipals tmp = {0};
	//*principals = tmp;
	//principals->codebase = (char*)jl_malloc(PATH_MAX);
	//strncpy(principals->codebase, scriptFileName, PATH_MAX-1);
	//principals->refcount = 1;
	//principals->destroy = HostPrincipalsDestroy;

	JSScript *script;
	script = JS_CompileScript(cx, globalObject, scriptText, strlen(scriptText), "inline", 1);
	JL_CHK( script );
	
	// mendatory else the exception is converted into an error before JL_IsExceptionPending can be used. Exceptions can be reported with JS_ReportPendingException().
	JS_SetOptions(cx, JS_GetOptions(cx) | JSOPTION_DONT_REPORT_UNCAUGHT);

	// You need to protect a JSScript (via a rooted script object) if and only if a garbage collection can occur between compilation and the start of execution.
	if ( !compileOnly )
		JL_CHK( JS_ExecuteScript(cx, globalObject, script, rval) ); // MUST be executed only once ( JSOPTION_COMPILE_N_GO )
	else
		*rval = JSVAL_VOID;

	JS_SetOptions(cx, prevOpt);
	return JS_TRUE;

bad:
	JS_SetOptions(cx, prevOpt);
	return JS_FALSE;
}


ALWAYS_INLINE JSBool FASTCALL
ExecuteScriptFileName( JSContext *cx, const char *scriptFileName, bool compileOnly, jsval *rval ) {

	uint32_t prevOpt = JS_SetOptions(cx, JS_GetOptions(cx) /*| JSOPTION_COMPILE_N_GO*/);
	JSObject *globalObject = JL_GetGlobal(cx);
	JL_ASSERT( globalObject != NULL, E_HOST, E_INTERNAL ); // "Global object not found."

	JSScript *script;
	script = JL_LoadScript(cx, globalObject, scriptFileName, ENC_UNKNOWN, true, false); // use xdr if available, but don't save it.
	JL_CHK( script );

	// mendatory else the exception is converted into an error before JL_IsExceptionPending can be used. Exceptions can be reported with JS_ReportPendingException().
	JS_SetOptions(cx, JS_GetOptions(cx) | JSOPTION_DONT_REPORT_UNCAUGHT);

	// You need to protect a JSScript (via a rooted script object) if and only if a garbage collection can occur between compilation and the start of execution.
	if ( !compileOnly )
		JL_CHK( JS_ExecuteScript(cx, globalObject, script, rval) ); // MUST be executed only once ( JSOPTION_COMPILE_N_GO )
	else
		*rval = JSVAL_VOID;

	JS_SetOptions(cx, prevOpt);
	return JS_TRUE;

bad:
	JS_SetOptions(cx, prevOpt);
	return JS_FALSE;
}


///////////////////////////////////////////////////////////////////////////////
// JS stack related functions

ALWAYS_INLINE JSStackFrame* FASTCALL
JL_CurrentStackFrame(JSContext *cx) {

	JSStackFrame *fp = NULL;
	return JS_FrameIterator(cx, &fp);
}


ALWAYS_INLINE uint32_t FASTCALL
JL_StackSize(JSContext *cx, JSStackFrame *fp) {

	JL_IGNORE(cx);
	uint32_t length = 0;
	for ( ; fp; JS_FrameIterator(cx, &fp) )
		++length;
	return length; // 0 is the first frame
}


INLINE JSStackFrame* FASTCALL
JL_StackFrameByIndex(JSContext *cx, int frameIndex) {

	//js::StackFrame *fp = js::Valueify(JL_CurrentStackFrame(cx));
	JSStackFrame *fp = JL_CurrentStackFrame(cx);
	if ( frameIndex >= 0 ) {

		int currentFrameIndex = JL_StackSize(cx, fp)-1;
		if ( frameIndex > currentFrameIndex )
			return NULL;
		// now, select the right frame
		while ( fp && currentFrameIndex > frameIndex ) {

			JS_FrameIterator(cx, &fp);
			--currentFrameIndex;
		}
		return fp;
	}

	while ( fp && frameIndex < 0 ) {

		JS_FrameIterator(cx, &fp);
		++frameIndex;
	}
	return fp;
}


INLINE NEVER_INLINE JSBool FASTCALL
JL_DebugPrintScriptLocation( JSContext *cx ) {

	const char *filename;
	JSScript *script;
	unsigned lineno;
	JL_CHK( JS_DescribeScriptedCaller(cx, &script, &lineno) );
	filename = JS_GetScriptFilename(cx, script);
	if ( filename == NULL || *filename == '\0' )
		filename = "<no_filename>";
	printf("%s:%d\n", filename, lineno);
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

	const char *filename;
	JSScript *script;
	unsigned lineno;
	jsval tmp;

	JL_CHK( JS_DescribeScriptedCaller(cx, &script, &lineno) );
	filename = JS_GetScriptFilename(cx, script);

	if ( filename == NULL || *filename == '\0' )
		filename = "<no_filename>";

	JL_CHK( JL_NativeToJsval(cx, filename, &tmp) );
	JL_CHK( JS_SetPropertyById(cx, obj, JLID(cx, fileName), &tmp) );
	JL_CHK( JL_NativeToJsval(cx, lineno, &tmp) );
	JL_CHK( JS_SetPropertyById(cx, obj, JLID(cx, lineNumber), &tmp) );

	return JS_TRUE;
	JL_BAD;
}



///////////////////////////////////////////////////////////////////////////////
// NativeInterface API

ALWAYS_INLINE JSBool
ReserveNativeInterface( JSContext *cx, JSObject *obj, const jsid &id ) {

	ASSERT( id != jspv::NullJsid() );
	return JS_DefinePropertyById(cx, obj, id, JSVAL_VOID, NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
}


template <class T>
ALWAYS_INLINE JSBool
SetNativeInterface( JSContext *cx, JSObject *obj, const jsid &id, const T nativeFct ) {

	ASSERT( id != jspv::NullJsid() );
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

	ASSERT( id != jspv::NullJsid() );
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
	JL_CHK( JL_CallFunctionId(cx, obj, JLID(cx, read), 1, &tmp, &tmp) );
	if ( JSVAL_IS_VOID(tmp) ) { // (TBD) with sockets, undefined mean 'closed', that is not supported by NIStreamRead.

		*amount = 0;
	} else {

		JLData str;
		JL_CHK( JL_JsvalToNative(cx, tmp, &str) );
		ASSERT( str.Length() <= *amount );
		*amount = str.Length();
		str.CopyTo(buffer);
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
	if ( JS_HasPropertyById(cx, obj, JLID(cx, read), &found) && found ) // JS_GetPropertyById(cx, obj, JLID(cx, Read), &res) != JS_TRUE || !JL_IsCallable(cx, res)
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
JSBufferGet( JSContext *cx, JSObject *obj, JLData *str ) {

	jsval tmp;
	return JL_CallFunctionId(cx, obj, JLID(cx, get), 0, NULL, &tmp) && JL_JsvalToNative(cx, tmp, str);
}


ALWAYS_INLINE NIBufferGet
BufferGetInterface( JSContext *cx, JSObject *obj ) {

	NIBufferGet fct = BufferGetNativeInterface(cx, obj);
	if (likely( fct != NULL ))
		return fct;
	JSBool found;
	if ( JS_HasPropertyById(cx, obj, JLID(cx, get), &found) && found ) // JS_GetPropertyById(cx, obj, JLID(cx, Get), &res) != JS_TRUE || !JL_IsCallable(cx, res)
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

	JL_IGNORE( cx, m, obj );
	return JS_FALSE;
}


ALWAYS_INLINE NIMatrix44Get
Matrix44GetInterface( JSContext *cx, JSObject *obj ) {

	NIMatrix44Get fct = Matrix44GetNativeInterface(cx, obj);
	if (likely( fct != NULL ))
		return fct;
	JSBool found;
	if ( JS_HasPropertyById(cx, obj, JLID(cx, getMatrix44), &found) && found ) // JS_GetPropertyById(cx, obj, JLID(cx, GetMatrix44), &res) != JS_TRUE || !JL_IsCallable(cx, res)
		return JSMatrix44Get;
	return NULL;
}



///////////////////////////////////////////////////////////////////////////////
// ProcessEvent

struct ProcessEvent {
	JSBool (*prepareWait)( volatile ProcessEvent *self, JSContext *cx, JSObject *obj );
	void (*startWait)( volatile ProcessEvent *self ); // starts the blocking thread and call signalEvent() when an event has arrived.
	bool (*cancelWait)( volatile ProcessEvent *self ); // unlock the blocking thread event if no event has arrived (mean that an event has arrived in another thread).
	JSBool (*endWait)( volatile ProcessEvent *self, bool *hasEvent, JSContext *cx, JSObject *obj ); // process the result
};

