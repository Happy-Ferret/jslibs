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

#ifndef _JSCLASS_H_
#define _JSCLASS_H_




typedef unsigned short JLClassNameHashId;

ALWAYS_INLINE bool JLClassNameEqual( const JLClassName *a, const JLClassName *b ) {

	if ( ((uint32_t*)a)[0] == ((uint32_t*)b)[0]
	  && ((uint32_t*)a)[1] == ((uint32_t*)b)[1]
	  && ((uint32_t*)a)[2] == ((uint32_t*)b)[2]
	  && ((uint32_t*)a)[3] == ((uint32_t*)b)[3]
	  && ((uint32_t*)a)[4] == ((uint32_t*)b)[4] )
		return true;
	return false;
}


ALWAYS_INLINE void JLClassNameCopy( JLClassName *dst, const JLClassName *src ) {

	((uint32_t*)dst)[0] = ((uint32_t*)src)[0];
	((uint32_t*)dst)[1] = ((uint32_t*)src)[1];
	((uint32_t*)dst)[2] = ((uint32_t*)src)[2];
	((uint32_t*)dst)[3] = ((uint32_t*)src)[3];
	((uint32_t*)dst)[4] = ((uint32_t*)src)[4];
}


ALWAYS_INLINE JLClassNameHashId JLClassNameHashFct( const JLClassName *n ) {

	return 
		(!(*n)[ 0] ?  0 : ((*n)[ 0]) ^ 
		(!(*n)[ 1] ?  1 : ((*n)[ 1]<<1) ^ 
		(!(*n)[ 2] ?  2 : ((*n)[ 2]) ^ 
		(!(*n)[ 3] ?  3 : ((*n)[ 3]<<2) ^ 
		(!(*n)[ 4] ?  4 : ((*n)[ 4]) ^ 
		(!(*n)[ 5] ?  5 : ((*n)[ 5]<<1) ^ 
		(!(*n)[ 6] ?  6 : ((*n)[ 6]) ^ 
		(!(*n)[ 7] ?  7 : ((*n)[ 7]<<2) ^ 
		(!(*n)[ 8] ?  8 : ((*n)[ 8]) ^ 
		(!(*n)[ 9] ?  9 : ((*n)[ 9]<<1) ^ 
		(!(*n)[10] ? 10 : ((*n)[10]) ^ 
		(!(*n)[11] ? 11 : ((*n)[11]<<2) ^ 
		(!(*n)[12] ? 12 : ((*n)[12]) ^ 
		(!(*n)[13] ? 13 : ((*n)[13]<<1) ^ 
		(!(*n)[14] ? 14 : ((*n)[14]) ^ 
		(!(*n)[15] ? 15 : ((*n)[15]<<2) ^
		(!(*n)[16] ? 16 : ((*n)[16]) ^ 
		(!(*n)[17] ? 17 : ((*n)[17]<<1) ^ 
		(!(*n)[18] ? 18 : ((*n)[18]) ^ 
		(!(*n)[19] ? 19 : ((*n)[19]<<2) ^
		0)))))))))))))))))))) & 0x1FF;
}


ALWAYS_INLINE void **JLGetClassNameHash( HostPrivate *hpv, const JLClassName *name ) {

	JLClassNameHashId index = JLClassNameHashFct(name);
	JL_ASSERT( index >= 0 );
	JL_ASSERT( index < COUNTOF(hpv->classNameHash) );

#ifdef DEBUG
	int count;
	count = 0;
#endif // DEBUG

	for (;;) {
			
		JLClassNameHash *item = &hpv->classNameHash[index];
		
		if ( JLClassNameEqual(&item->name, name) )
			return &item->data;

		if ( item->name[0] == 0 ) {

			JLClassNameCopy(&item->name, name);
			return &item->data;
		}

		index = (index + 1) % COUNTOF(hpv->classNameHash);
#ifdef DEBUG
		JL_ASSERT( count++ < COUNTOF(hpv->classNameHash) );
#endif // DEBUG
	}
}


struct JLConstIntegerSpec {
    int ival;
    const char *name;
};

struct JLClassSpec {
	JSExtendedClass xclasp;
	JLClassName className;
	JSNative constructor;
	uintN nargs;
	JLClassName parentProtoName;
	JSPropertySpec *ps;
	JSPropertySpec *static_ps;
	JSFunctionSpec *fs;
	JSFunctionSpec *static_fs;
	JSConstDoubleSpec *cds;
	JLConstIntegerSpec *cis;
	JSBool (*init)(JSContext *cx, JSObject *obj);
	unsigned int revision;
};


// JL_StoreProperty is used to override a property definition (from the prototype to the obj.
// if removeGetterAndSetter is false, it is up to the caller to filter calls using: if ( *vp != JSVAL_VOID ) return JS_TRUE;
// if removeGetterAndSetter is true, the value is stored for r/w getter or setter will never be called again.
inline JSBool JL_StoreProperty( JSContext *cx, JSObject *obj, jsid id, const jsval *vp, bool removeGetterAndSetter ) {

	JSBool found;
	uintN attrs;
	JSPropertyOp getter, setter;
	JL_CHK( JS_GetPropertyAttrsGetterAndSetterById(cx, obj, id, &attrs, &found, &getter, &setter) );
	JL_CHKM( found, "Property not found." );
	if ( (attrs & JSPROP_SHARED) == 0 ) // Has already been stored somewhere. The slot will be updated after JSPropertyOp returns.
		return JS_TRUE;
	attrs &= ~JSPROP_SHARED; // stored mean not shared.
	if ( removeGetterAndSetter ) { // store and never call the getter or setter again.

		getter = NULL;
		setter = NULL;
	}
	return JS_DefinePropertyById(cx, obj, id, *vp, getter, setter, attrs);
	JL_BAD;
}

// because it is difficult to override properties by tinyId (JSPropertyOp) see. bz#526979
ALWAYS_INLINE JSBool JL_DefineClassProperties(JSContext *cx, JSObject *obj, JSPropertySpec *ps) {

	for (; ps->name; ps++) {

		if ( ps->tinyid < 0 )
			JL_CHK( JS_DefineProperty(cx, obj, ps->name, JSVAL_VOID, ps->getter, ps->setter, ps->flags) );
		else
			JL_CHK( JS_DefinePropertyWithTinyId(cx, obj, ps->name, ps->tinyid, JSVAL_VOID, ps->getter, ps->setter, ps->flags) );
	}
	return JS_TRUE;
	JL_BAD;
}

ALWAYS_INLINE char *JLNormalizeFunctionName( const char *name ) {

	char *buf = strdup(name); // (TBD) free ? DO FREE WITH STANDARD free() FUNCTION !!!!
	buf[0] = tolower(buf[0]);
	return buf;
}

ALWAYS_INLINE void JLNormalizeFunctionSpecNames( JSFunctionSpec *functionSpec ) {

	for ( JSFunctionSpec *it = functionSpec; it && it->name; it++ )
		it->name = JLNormalizeFunctionName(it->name);
}

inline JSBool JLInitStatic( JSContext *cx, JSObject *obj, JLClassSpec *cs ) {

	JL_CHK(obj);

	if ( GetHostPrivate(cx)->camelCase == 1 )
		JLNormalizeFunctionSpecNames(cs->static_fs);

	if ( cs->static_fs != NULL )
		JL_CHK( JS_DefineFunctions(cx, obj, cs->static_fs) );

	if ( cs->static_ps != NULL )
		JL_CHK( JL_DefineClassProperties(cx, obj, cs->static_ps) );

	if ( cs->cis != NULL )
		for ( JLConstIntegerSpec *it = cs->cis ; it->name; ++it )
			JL_CHK( JS_DefineProperty(cx, obj, it->name, INT_TO_JSVAL(it->ival), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT) );

	if ( cs->cds != NULL )
		JL_CHK( JS_DefineConstDoubles(cx, obj, cs->cds) );

	if ( cs->init )
		JL_CHK( cs->init(cx, obj) );

	return JS_TRUE;
	JL_BAD;
}

inline JSBool JLInitClass( JSContext *cx, JSObject *obj, JLClassSpec *cs ) {

	JL_CHK(obj);
	JL_S_ASSERT( cs->xclasp.base.name && cs->xclasp.base.name[0], "Invalid class name." );

	if ( GetHostPrivate(cx)->camelCase == 1 ) {

		JLNormalizeFunctionSpecNames(cs->fs);
		JLNormalizeFunctionSpecNames(cs->static_fs);
	}

	HostPrivate *hpv;
	hpv = GetHostPrivate(cx);

	JSObject *parent_proto;
	if ( cs->parentProtoName[0] ) {

		parent_proto = *(JSObject**)JLGetClassNameHash(hpv, &cs->parentProtoName);
		JL_ASSERT( parent_proto != NULL ); // "parent prototype not found"
	} else {

		parent_proto = NULL;
	}

	JSObject *proto;
	proto = JS_InitClass(cx, obj, parent_proto, &cs->xclasp.base, cs->constructor, cs->nargs, NULL, cs->fs, NULL, cs->static_fs);
	
	JSObject **protoHashSlot;
	protoHashSlot = (JSObject**)JLGetClassNameHash(hpv, &cs->className);
	JL_ASSERT( *protoHashSlot == NULL ); // "prototype already defined" 
	*protoHashSlot = proto;

	JSObject *staticDest;
	staticDest = cs->constructor ? JS_GetConstructor(cx, proto) : proto;

	if ( cs->ps != NULL )
		JL_CHK( JL_DefineClassProperties(cx, proto, cs->ps) );

	if ( cs->static_ps != NULL )
		JL_CHK( JL_DefineClassProperties(cx, staticDest, cs->static_ps) );

	if ( cs->cis != NULL )
		for ( JLConstIntegerSpec *it = cs->cis ; it->name; ++it )
			JL_CHK( JS_DefineProperty(cx, staticDest, it->name, INT_TO_JSVAL(it->ival), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT) );

	if ( cs->cds != NULL )
		JL_CHK( JS_DefineConstDoubles(cx, staticDest, cs->cds) );

	JSBool found;
	JL_CHK( JS_SetPropertyAttributes(cx, obj, cs->xclasp.base.name, JSPROP_READONLY | JSPROP_PERMANENT, &found) );
	JL_ASSERT( found ); // "Unable to set class flags."

	JL_CHK( JS_DefinePropertyById(cx, staticDest, JLID(cx, _revision), INT_TO_JSVAL(cs->revision), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT) );

	if ( cs->init )
		JL_CHK( cs->init(cx, staticDest) );

	return JS_TRUE;
	JL_BAD;
}



#define DECLARE_STATIC() \
	extern JLClassSpec *jlStaticSpec; \

#define INIT_STATIC() \
	JL_CHK( JLInitStatic(cx, obj, jlStaticSpec ) ); \

#define BEGIN_STATIC \
	static JLClassSpec *JLClassSpecInit(); \
	JLClassSpec *jlStaticSpec = JLClassSpecInit(); \

#define CONFIGURE_STATIC \
	ALWAYS_INLINE JLClassSpec *JLClassSpecInit() { \
		static JLClassSpec cs; \

#define END_STATIC \
		return &cs; \
	} \



#define DECLARE_CLASS(CLASSNAME) \
	namespace CLASSNAME { \
		extern JLClassSpec *jlClassSpec; \
	} \

#define INIT_CLASS(CLASSNAME) \
	JL_CHK( JLInitClass(cx, obj, CLASSNAME::jlClassSpec ) ); \

#define BEGIN_CLASS(CLASSNAME) \
	namespace CLASSNAME { \
		static const JLClassName className = #CLASSNAME; \
		ALWAYS_INLINE JLClassSpec *JLClassSpecInit(); \
		JLClassSpec *jlClassSpec = JLClassSpecInit(); \

#define CONFIGURE_CLASS \
		ALWAYS_INLINE JLClassSpec *JLClassSpecInit() { \
			static JLClassSpec cs; \
			JLClassNameCopy(&cs.className, &className); \
			cs.xclasp.base.name = (char *)className; \
			cs.xclasp.base.addProperty = JS_PropertyStub; \
			cs.xclasp.base.delProperty = JS_PropertyStub; \
			cs.xclasp.base.getProperty = JS_PropertyStub; \
			cs.xclasp.base.setProperty = JS_PropertyStub; \
			cs.xclasp.base.enumerate = JS_EnumerateStub; \
			cs.xclasp.base.resolve = JS_ResolveStub; \
			cs.xclasp.base.convert = JS_ConvertStub; \
			cs.xclasp.base.finalize = JS_FinalizeStub; \

#define END_CLASS \
			return &cs; \
		} \
	} \

//soubok>	in the JSClass finalize function, what is the best method to distinguish between object and prototype finalization ?
//jorendorff>	that's hard to do... if you're willing to be imprecise about it, you can just check to see if the object's prototype has the same class.
//jorendorff>	The best way is to keep the result of JS_InitClass around somewhere.
//jorendorff>	Then just compare with == to see if you're finalizing that object.

#define JL_CLASS(CLASSNAME) (&(CLASSNAME::jlClassSpec->xclasp.base))
#define JL_THIS_CLASS (&(jlClassSpec->xclasp.base))

#define JL_PROTOTYPE(cx, CLASSNAME) (*(JSObject**)JLGetClassNameHash(GetHostPrivate(cx), &(CLASSNAME::jlClassSpec->className)))
#define JL_THIS_PROTOTYPE (*(JSObject**)JLGetClassNameHash(GetHostPrivate(cx), &(jlClassSpec->className)))

#define _NULL NULL // because in _##getter and _##setter, getter or setter can be NULL.


// const integer
#define BEGIN_CONST_INTEGER_SPEC static JLConstIntegerSpec cis[] = {
#define END_CONST_INTEGER_SPEC {0, NULL}}; cs.cis = cis;
#define CONST_INTEGER(name,value) { value, #name },
#define CONST_INTEGER_SINGLE(name) { name, #name },

// const double
#define BEGIN_CONST_DOUBLE_SPEC static JSConstDoubleSpec cds[] = { // dval; *name; flags; spare[3];
#define END_CONST_DOUBLE_SPEC {0, NULL, 0, {0, 0, 0}}}; cs.cds = cds;
#define CONST_DOUBLE(name,value) { value, #name, 0, {0, 0, 0}},
#define CONST_DOUBLE_SINGLE(name) { name, #name, 0, {0, 0, 0}},

// functions
#define BEGIN_FUNCTION_SPEC static JSFunctionSpec fs[] = { // *name, call, nargs, flags, extra
#define END_FUNCTION_SPEC {NULL, NULL, 0, 0, 0}}; cs.fs = fs;

#define BEGIN_STATIC_FUNCTION_SPEC static JSFunctionSpec static_fs[] = {
#define END_STATIC_FUNCTION_SPEC {NULL, NULL, 0, 0, 0}}; cs.static_fs = static_fs;

// properties
#define BEGIN_PROPERTY_SPEC static JSPropertySpec ps[] = { // *name, tinyid, flags, getter, setter
#define END_PROPERTY_SPEC {NULL, 0, 0, 0, 0}}; cs.ps = ps;

#define BEGIN_STATIC_PROPERTY_SPEC static JSPropertySpec static_ps[] = {
#define END_STATIC_PROPERTY_SPEC {NULL, 0, 0, 0, 0}}; cs.static_ps = static_ps;


#if defined(DEBUG)
inline JSFastNative FastNativeFunction(JSFastNative f) { return f; } // used for type check only.
inline JSNative NativeFunction(JSNative f) { return f; } // used for type check only.
#else
#define FastNativeFunction(f) (f)
#define NativeFunction(f) (f)
#endif // DEBUG

#define FUNCTION(name) JS_FS( #name, NativeFunction(_##name), 0, 0, 0 ),
#define FUNCTION_ARGC(name, nargs) JS_FS( #name, NativeFunction(_##name), nargs, 0, 0 ),
#define FUNCTION_ALIAS(alias, name) JS_FS( #alias, NativeFunction(_##name), 0, 0, 0 ),

	// JS_FN(name,fastcall,minargs,nargs,flags) vs JS_FN(name,fastcall,nargs,flags) (see. http://hg.mozilla.org/tracemonkey/diff/9e185457c656/js/src/jsapi.h)
#define FUNCTION_FAST(name) JS_FN( #name, FastNativeFunction(_##name), 0, 0 ),
#define FUNCTION_FAST_ARGC(name, nargs) JS_FN( #name, FastNativeFunction(_##name), nargs, 0 ),
#define FUNCTION_FAST_ALIAS(alias, name) JS_FN( #alias, FastNativeFunction(_##name), 0, 0 ),


#define PROPERTY(name) { #name, -1, JSPROP_PERMANENT|JSPROP_SHARED, _##name##Getter, _##name##Setter },
#define PROPERTY_READ(name) { #name, -1, JSPROP_PERMANENT|JSPROP_READONLY|JSPROP_SHARED, _##name, NULL }, // (TBD) rename into PROPERTY_GETTER
#define PROPERTY_WRITE(name) { #name, -1, JSPROP_PERMANENT|JSPROP_SHARED, NULL, _##name }, // (TBD) rename into PROPERTY_SETTER
#define PROPERTY_SWITCH(name, function) { #name, name, JSPROP_PERMANENT|JSPROP_SHARED, _##function##Getter, _##function##Setter }, // Used to define multiple properties with only one pari of getter/setter functions ( an enum has to be defiend ... less than 256 items ! )
#define PROPERTY_SWITCH_READ(name, function) { #name, name, JSPROP_PERMANENT|JSPROP_READONLY|JSPROP_SHARED, _##function, NULL },
#define PROPERTY_CREATE(name,id,flags,getter,setter) { #name, id, flags, _##getter, _##setter },
#define PROPERTY_DEFINE(name) { #name, 0, JSPROP_PERMANENT, NULL, NULL },

// configuration 
#define HAS_PRIVATE cs.xclasp.base.flags |= JSCLASS_HAS_PRIVATE;
#define HAS_RESERVED_SLOTS(COUNT) cs.xclasp.base.flags |= JSCLASS_HAS_RESERVED_SLOTS(COUNT);
#define CONSTRUCT_PROTOTYPE cs.xclasp.base.flags |= JSCLASS_CONSTRUCT_PROTOTYPE;
#define IS_GLOBAL cs.xclasp.base.flags |= JSCLASS_GLOBAL_FLAGS;
#define HAS_NEW_RESOLVE_GETS_START cs.xclasp.base.flags |= JSCLASS_NEW_RESOLVE_GETS_START;
#define HAS_ALL_PROPERTIES_SHARED cs.xclasp.base.flags |= JSCLASS_SHARE_ALL_PROPERTIES;
#define REVISION(NUMBER) cs.revision = (NUMBER);

#define HAS_PROTOTYPE(PROTOTYPENAME) \
	static JLClassName parentProtoName = #PROTOTYPENAME; \
	JLClassNameCopy(&cs.parentProtoName, &parentProtoName); \

#define HAS_CONSTRUCTOR_ARGC(ARGC) cs.constructor = Constructor; cs.argc = (ARGC);
#define HAS_CONSTRUCTOR cs.constructor = Constructor;
#define DEFINE_CONSTRUCTOR() static JSBool Constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)

#define HAS_FINALIZE cs.xclasp.base.finalize = Finalize;
#define DEFINE_FINALIZE() static void Finalize(JSContext *cx, JSObject *obj)

#define HAS_OBJECT_CONSTRUCTOR cs.xclasp.base.construct = ObjectConstructor;
#define DEFINE_OBJECT_CONSTRUCTOR() static JSBool ObjectConstructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)

#define HAS_CALL cs.xclasp.base.call = Call;
#define DEFINE_CALL() static JSBool Call(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)

#define HAS_CONVERT cs.xclasp.base.convert = Convert;
#define DEFINE_CONVERT() static JSBool Convert(JSContext *cx, JSObject *obj, JSType type, jsval *vp)

#define HAS_RESOLVE cs.xclasp.base.resolve = Resolve;
#define DEFINE_RESOLVE() static JSBool Resolve(JSContext *cx, JSObject *obj, jsval id)

#define HAS_NEW_RESOLVE cs.xclasp.base.flags |= JSCLASS_NEW_RESOLVE; cs.xclasp.base.resolve = (JSResolveOp)NewResolve;
#define DEFINE_NEW_RESOLVE() static JSBool NewResolve(JSContext *cx, JSObject *obj, jsval id, uintN flags, JSObject **objp)

#define HAS_ENUMERATE cs.xclasp.base.enumerate = Enumerate;
#define DEFINE_ENUMERATE() static JSBool Enumerate(JSContext *cx, JSObject *obj)

#define HAS_TRACER cs.xclasp.base.flags |= JSCLASS_MARK_IS_TRACE; cs.xclasp.base.mark = (JSMarkOp)Tracer;
#define DEFINE_TRACER() static void Tracer(JSTracer *trc, JSObject *obj)

#define HAS_HAS_INSTANCE cs.xclasp.base.hasInstance = HasInstance;
#define DEFINE_HAS_INSTANCE() static JSBool HasInstance(JSContext *cx, JSObject *obj, jsval v, JSBool *bp)

#define HAS_EQUALITY cs.xclasp.base.flags |= JSCLASS_IS_EXTENDED; cs.xclasp.equality = Equality;
#define DEFINE_EQUALITY() static JSBool Equality(JSContext *cx, JSObject *obj, jsval v, JSBool *bp)

#define HAS_WRAPPED_OBJECT cs.xclasp.base.flags |= JSCLASS_IS_EXTENDED; cs.xclasp.wrappedObject = WrappedObject;
#define DEFINE_WRAPPED_OBJECT() static JSObject* WrappedObject(JSContext *cx, JSObject *obj)

#define HAS_INIT cs.init = Init;
#define DEFINE_INIT() static JSBool Init(JSContext *cx, JSObject *obj)

#define HAS_ADD_PROPERTY cs.xclasp.base.addProperty = AddProperty;
#define DEFINE_ADD_PROPERTY() static JSBool AddProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)

#define HAS_DEL_PROPERTY cs.xclasp.base.delProperty = DelProperty;
#define DEFINE_DEL_PROPERTY() static JSBool DelProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)

#define HAS_GET_PROPERTY cs.xclasp.base.getProperty = GetProperty;
#define DEFINE_GET_PROPERTY() static JSBool GetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)

#define HAS_SET_PROPERTY cs.xclasp.base.setProperty = SetProperty;
#define DEFINE_SET_PROPERTY() static JSBool SetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)

#define HAS_GET_OBJECT_OPS cs.xclasp.base.getObjectOps = GetObjectOps;
#define DEFINE_GET_OBJECT_OPS() static JSObjectOps* GetObjectOps(JSContext *cx, JSClass *clasp)

#define HAS_CHECK_ACCESS cs.xclasp.base.checkAccess = CheckAccess;
#define DEFINE_CHECK_ACCESS() static JSBool CheckAccess(JSContext *cx, JSObject *obj, jsval id, JSAccessMode mode, jsval *vp)

#define HAS_XDR cs.xclasp.base.xdrObject = XDRObject;
#define DEFINE_XDR() static JSBool XDRObject(JSXDRState *xdr, JSObject **objp)

#define HAS_ITERATOR_OBJECT cs.xclasp.base.flags |= JSCLASS_IS_EXTENDED; cs.xclasp.iteratorObject = IteratorObject;
#define DEFINE_ITERATOR_OBJECT() static JSObject* IteratorObject(JSContext *cx, JSObject *obj, JSBool keysonly)


// definition
#define DEFINE_FUNCTION_FAST(name) static JSBool _##name(JSContext *cx, uintN argc, jsval *vp)
#define DEFINE_FUNCTION(name) static JSBool _##name(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)

#define DEFINE_PROPERTY(name) static JSBool _##name(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
#define DEFINE_PROPERTY_GETTER(name) static JSBool _##name##Getter(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
#define DEFINE_PROPERTY_SETTER(name) static JSBool _##name##Setter(JSContext *cx, JSObject *obj, jsval id, jsval *vp)


#if 0 // previous version

#include <ctype.h>

#define _NULL NULL // because in _##getter and _##setter, getter or setter can be NULL.

struct JLConstIntegerSpec {
	jsint ival;
	const char *name;
};

// declaration

// const integer
#define BEGIN_CONST_INTEGER_SPEC JLConstIntegerSpec _tmp_cis[] = {
#define END_CONST_INTEGER_SPEC {0, NULL}}; _constIntegerSpec = _tmp_cis;
#define CONST_INTEGER(name,value) { value, #name },
#define CONST_INTEGER_SINGLE(name) { name, #name },

// const double
#define BEGIN_CONST_DOUBLE_SPEC JSConstDoubleSpec _tmp_cds[] = { // dval; *name; flags; spare[3];
#define END_CONST_DOUBLE_SPEC {0, NULL, 0, {0, 0, 0}}}; _constDoubleSpec = _tmp_cds;
#define CONST_DOUBLE(name,value) { value, #name, 0, {0, 0, 0}},
#define CONST_DOUBLE_SINGLE(name) { name, #name, 0, {0, 0, 0}},

// functions
#define BEGIN_FUNCTION_SPEC JSFunctionSpec _tmp_fs[] = { // *name, call, nargs, flags, extra
#define END_FUNCTION_SPEC {NULL, NULL, 0, 0, 0}}; _functionSpec = _tmp_fs;
#define BEGIN_STATIC_FUNCTION_SPEC JSFunctionSpec _tmp_sfs[] = {
#define END_STATIC_FUNCTION_SPEC {NULL, NULL, 0, 0, 0}}; _staticFunctionSpec = _tmp_sfs;

#if defined(DEBUG)
inline JSFastNative FastNativeFunction(JSFastNative f) { return f; } // used fo type check only.
inline JSNative NativeFunction(JSNative f) { return f; } // used fo type check only.
#else
#define FastNativeFunction(f) (f)
#define NativeFunction(f) (f)
#endif // DEBUG

#define FUNCTION(name) JS_FS( #name, NativeFunction(_##name), 0, 0, 0 ),
#define FUNCTION_ARGC(name, nargs) JS_FS( #name, NativeFunction(_##name), nargs, 0, 0 ),
#define FUNCTION_ALIAS(alias, name) JS_FS( #alias, NativeFunction(_##name), 0, 0, 0 ),

	// JS_FN(name,fastcall,minargs,nargs,flags) vs JS_FN(name,fastcall,nargs,flags) (see. http://hg.mozilla.org/tracemonkey/diff/9e185457c656/js/src/jsapi.h)
#define FUNCTION_FAST(name) JS_FN( #name, FastNativeFunction(_##name), 0, 0 ),
#define FUNCTION_FAST_ARGC(name, nargs) JS_FN( #name, FastNativeFunction(_##name), nargs, 0 ),
#define FUNCTION_FAST_ALIAS(alias, name) JS_FN( #alias, FastNativeFunction(_##name), 0, 0 ),

// properties
#define BEGIN_PROPERTY_SPEC JSPropertySpec _tmp_ps[] = { // *name, tinyid, flags, getter, setter
#define END_PROPERTY_SPEC {NULL, 0, 0, 0, 0}}; _propertySpec = _tmp_ps;
#define BEGIN_STATIC_PROPERTY_SPEC JSPropertySpec _tmp_sps[] = {
#define END_STATIC_PROPERTY_SPEC {NULL, 0, 0, 0, 0}}; _staticPropertySpec = _tmp_sps;

#define PROPERTY(name)	{ #name, -1, JSPROP_PERMANENT|JSPROP_SHARED, _##name##Getter, _##name##Setter },
#define PROPERTY_READ(name)	{ #name, -1, JSPROP_PERMANENT|JSPROP_READONLY|JSPROP_SHARED, _##name, NULL }, // (TBD) rename into PROPERTY_GETTER
#define PROPERTY_WRITE(name)	{ #name, -1, JSPROP_PERMANENT|JSPROP_SHARED, NULL, _##name }, // (TBD) rename into PROPERTY_SETTER
#define PROPERTY_SWITCH(name, function)	{ #name, name, JSPROP_PERMANENT|JSPROP_SHARED, _##function##Getter, _##function##Setter }, // Used to define multiple properties with only one pari of getter/setter functions ( an enum has to be defiend ... less than 256 items ! )
#define PROPERTY_SWITCH_READ(name, function) { #name, name, JSPROP_PERMANENT|JSPROP_READONLY|JSPROP_SHARED, _##function, NULL },
//#define PROPERTY_SWITCH_ID(name, id, function)       { #name, id, JSPROP_PERMANENT|JSPROP_SHARED, _##function##Getter, _##function##Setter },
//#define PROPERTY_SWITCH_ID_STORE(name, id, function) { #name, id, JSPROP_PERMANENT, _##function##Getter, _##function##Setter },
//#define PROPERTY_SWITCH_ID_READ_STORE(name, id, function) { #name, id, JSPROP_PERMANENT|JSPROP_READONLY, _##function##Getter, NULL },
#define PROPERTY_CREATE(name,id,flags,getter,setter) { #name, id, flags, _##getter, _##setter },
#define PROPERTY_DEFINE(name) { #name, 0, JSPROP_PERMANENT, NULL, NULL },


// class definition and configuration 
#define DEFINE_FUNCTION_FAST(name) static JSBool _##name(JSContext *cx, uintN argc, jsval *vp)
#define DEFINE_FUNCTION(name) static JSBool _##name(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)

#define DEFINE_PROPERTY(name) static JSBool _##name(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
#define DEFINE_PROPERTY_GETTER(name) static JSBool _##name##Getter(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
#define DEFINE_PROPERTY_SETTER(name) static JSBool _##name##Setter(JSContext *cx, JSObject *obj, jsval id, jsval *vp)


#define HAS_PRIVATE _class->flags |= JSCLASS_HAS_PRIVATE;
#define HAS_RESERVED_SLOTS(COUNT) _class->flags |= JSCLASS_HAS_RESERVED_SLOTS(COUNT);
#define HAS_PROTOTYPE(PROTOTYPE) *_parentPrototype = (PROTOTYPE);
#define CONSTRUCT_PROTOTYPE _class->flags |= JSCLASS_CONSTRUCT_PROTOTYPE;
#define IS_GLOBAL _class->flags |= JSCLASS_GLOBAL_FLAGS;
#define HAS_NEW_RESOLVE_GETS_START _class->flags |= JSCLASS_NEW_RESOLVE_GETS_START;
#define HAS_ALL_PROPERTIES_SHARED _class->flags |= JSCLASS_SHARE_ALL_PROPERTIES;
#define REVISION(REV) (_revision = INT_TO_JSVAL(REV));

#define HAS_CONSTRUCTOR _constructor = Constructor;
#define DEFINE_CONSTRUCTOR() static JSBool Constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)

#define HAS_OBJECT_CONSTRUCTOR _class->construct = ObjectConstructor;
#define DEFINE_OBJECT_CONSTRUCTOR() static JSBool ObjectConstructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)

#define HAS_CALL _class->call = Call;
#define DEFINE_CALL() static JSBool Call(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)

#define HAS_FINALIZE _class->finalize = Finalize;
#define DEFINE_FINALIZE() static void Finalize(JSContext *cx, JSObject *obj)

#define HAS_CONVERT _class->convert = Convert;
#define DEFINE_CONVERT() static JSBool Convert(JSContext *cx, JSObject *obj, JSType type, jsval *vp)

#define HAS_RESOLVE _class->resolve = Resolve;
#define DEFINE_RESOLVE() static JSBool Resolve(JSContext *cx, JSObject *obj, jsval id)

#define HAS_NEW_RESOLVE _class->flags |= JSCLASS_NEW_RESOLVE; _class->resolve = (JSResolveOp)NewResolve;
#define DEFINE_NEW_RESOLVE() static JSBool NewResolve(JSContext *cx, JSObject *obj, jsval id, uintN flags, JSObject **objp)

#define HAS_ENUMERATE _class->enumerate = Enumerate;
#define DEFINE_ENUMERATE() static JSBool Enumerate(JSContext *cx, JSObject *obj)

#define HAS_TRACER _class->flags |= JSCLASS_MARK_IS_TRACE; _class->mark = (JSMarkOp)Tracer;
#define DEFINE_TRACER() static void Tracer(JSTracer *trc, JSObject *obj)

#define HAS_HAS_INSTANCE _class->hasInstance = HasInstance;
#define DEFINE_HAS_INSTANCE() static JSBool HasInstance(JSContext *cx, JSObject *obj, jsval v, JSBool *bp)

#define HAS_EQUALITY _xclass.base.flags |= JSCLASS_IS_EXTENDED; _xclass.equality = Equality;
#define DEFINE_EQUALITY() static JSBool Equality(JSContext *cx, JSObject *obj, jsval v, JSBool *bp)

#define HAS_WRAPPED_OBJECT _xclass.base.flags |= JSCLASS_IS_EXTENDED; _xclass.wrappedObject = WrappedObject;
#define DEFINE_WRAPPED_OBJECT() static JSObject* WrappedObject(JSContext *cx, JSObject *obj)

#define HAS_INIT _init = Init;
#define DEFINE_INIT() static JSBool Init(JSContext *cx, JSObject *obj)

#define HAS_ADD_PROPERTY _class->addProperty = AddProperty;
#define DEFINE_ADD_PROPERTY() static JSBool AddProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)

#define HAS_DEL_PROPERTY _class->delProperty = DelProperty;
#define DEFINE_DEL_PROPERTY() static JSBool DelProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)

#define HAS_GET_PROPERTY _class->getProperty = GetProperty;
#define DEFINE_GET_PROPERTY() static JSBool GetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)

#define HAS_SET_PROPERTY _class->setProperty = SetProperty;
#define DEFINE_SET_PROPERTY() static JSBool SetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)

#define HAS_GET_OBJECT_OPS _class->getObjectOps = GetObjectOps;
#define DEFINE_GET_OBJECT_OPS() static JSObjectOps* GetObjectOps(JSContext *cx, JSClass *clasp)

#define HAS_CHECK_ACCESS _class->checkAccess = CheckAccess;
#define DEFINE_CHECK_ACCESS() static JSBool CheckAccess(JSContext *cx, JSObject *obj, jsval id, JSAccessMode mode, jsval *vp)

#define HAS_XDR _class->xdrObject = XDRObject;
#define DEFINE_XDR() static JSBool XDRObject(JSXDRState *xdr, JSObject **objp)

#define HAS_ITERATOR_OBJECT _xclass.base.flags |= JSCLASS_IS_EXTENDED; _xclass.iteratorObject = IteratorObject;
#define DEFINE_ITERATOR_OBJECT() static JSObject* IteratorObject(JSContext *cx, JSObject *obj, JSBool keysonly)


inline char *_NormalizeFunctionName( const char *name ) {

	char *buf = strdup(name); // (TBD) free ? DO FREE WITH STANDARD free() FUNCTION !!!!
	buf[0] = tolower(buf[0]);
	return buf;
}

inline void _NormalizeFunctionSpecNames( JSFunctionSpec *functionSpec ) {

	for ( JSFunctionSpec *it = functionSpec; it && it->name; it++ )
		it->name = _NormalizeFunctionName(it->name);
}

// because it is difficult to override properties by tinyId (JSPropertyOp) see. bz#526979
inline JSBool JL_DefineClassProperties(JSContext *cx, JSObject *obj, JSPropertySpec *ps) {

	for (; ps->name; ps++) {

		if ( ps->tinyid < 0 )
			JL_CHK( JS_DefineProperty(cx, obj, ps->name, JSVAL_VOID, ps->getter, ps->setter, ps->flags) );
		else
			JL_CHK( JS_DefinePropertyWithTinyId(cx, obj, ps->name, ps->tinyid, JSVAL_VOID, ps->getter, ps->setter, ps->flags) );
	}
	return JS_TRUE;
	JL_BAD;
}

// JL_StoreProperty is used to override a property definition (from the prototype to the obj.
// if removeGetterAndSetter is false, it is up to the caller to filter calls using: if ( *vp != JSVAL_VOID ) return JS_TRUE;
// if removeGetterAndSetter is true, the value is stored for r/w getter or setter will never be called again.
inline JSBool JL_StoreProperty( JSContext *cx, JSObject *obj, jsid id, const jsval *vp, bool removeGetterAndSetter ) {

	JSBool found;
	uintN attrs;
	JSPropertyOp getter, setter;
	JL_CHK( JS_GetPropertyAttrsGetterAndSetterById(cx, obj, id, &attrs, &found, &getter, &setter) );
	JL_CHKM( found, "Property not found." );
	if ( (attrs & JSPROP_SHARED) == 0 ) // Has already been stored somewhere. The slot will be updated after JSPropertyOp returns.
		return JS_TRUE;
	attrs &= ~JSPROP_SHARED; // stored mean not shared.
	if ( removeGetterAndSetter ) { // store and never call the getter or setter again.

		getter = NULL;
		setter = NULL;
	}
	return JS_DefinePropertyById(cx, obj, id, *vp, getter, setter, attrs);
	JL_BAD;
}


// static definition
#define DECLARE_STATIC() \
	JSBool InitializeStatic(JSContext *cx, JSObject *obj);


#define INIT_STATIC() \
	InitializeStatic(cx, obj)

#define BEGIN_STATIC

#define CONFIGURE_STATIC \
	JSBool InitializeStatic(JSContext *cx, JSObject *obj) { \
		JSFunctionSpec *_staticFunctionSpec = NULL; \
		JSPropertySpec *_staticPropertySpec = NULL; \
		JSConstDoubleSpec *_constDoubleSpec = NULL; \
		JLConstIntegerSpec *_constIntegerSpec = NULL; \
		JSBool (*_init)(JSContext *cx, JSObject *obj) = NULL; \
		jsval _revision = JSVAL_VOID;

#define END_STATIC \
		JL_CHK(obj); \
		JSObject *dstObj; \
		dstObj = obj; \
		if ( GetHostPrivate(cx)->camelCase == 1 ) \
			_NormalizeFunctionSpecNames(_staticFunctionSpec); \
		if ( _staticFunctionSpec != NULL ) \
			JS_DefineFunctions(cx, obj, _staticFunctionSpec); \
		if ( _staticPropertySpec != NULL ) \
			JL_CHK( JL_DefineClassProperties(cx, dstObj, _staticPropertySpec) ); \
		if ( _constIntegerSpec != NULL ) { \
			for ( ; _constIntegerSpec->name; _constIntegerSpec++ ) \
				JL_CHK( JS_DefineProperty(cx, dstObj, _constIntegerSpec->name, INT_TO_JSVAL(_constIntegerSpec->ival), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT) ); \
		} \
		if ( _constDoubleSpec != NULL ) \
			JL_CHK( JS_DefineConstDoubles(cx, obj, _constDoubleSpec) ); \
		JL_CHK( JS_DefinePropertyById(cx, obj, JLID(cx, _revision), _revision, NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT) ); \
		if ( _init ) \
			JL_CHK( _init(cx, obj) ); \
		return JS_TRUE; \
		JL_BAD; \
	}

// class definition
#define DECLARE_CLASS( CLASSNAME ) \
	extern JSBool (*InitializeClass##CLASSNAME)(JSContext *cx, JSObject *obj); \
	extern JSClass *class##CLASSNAME; \
	extern JSObject *prototype##CLASSNAME;


#define INIT_CLASS( CLASSNAME ) \
	JL_CHK( InitializeClass##CLASSNAME(cx, obj) )

#define BEGIN_CLASS(CLASSNAME) \
	static JSExtendedClass _xclass = { { #CLASSNAME, 0, JS_PropertyStub , JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_EnumerateStub, JS_ResolveStub , JS_ConvertStub, JS_FinalizeStub, JSCLASS_NO_OPTIONAL_MEMBERS }, 0, 0, 0, 0, 0, 0, 0, 0}; \
	static JSClass *_class = &_xclass.base; \
	JSClass *class##CLASSNAME = &_xclass.base; \
	JSObject *prototype##CLASSNAME = NULL; \
	static JSBool _InitializeClass(JSContext *cx, JSObject *obj); \
	JSBool (*InitializeClass##CLASSNAME)(JSContext *cx, JSObject *obj) = _InitializeClass; \
	static JSObject **_prototype = &prototype##CLASSNAME;

#define CONFIGURE_CLASS \
	static JSBool _InitializeClass(JSContext *cx, JSObject *obj) { \
		JSNative _constructor = NULL; \
		JSFunctionSpec *_functionSpec = NULL; \
		JSFunctionSpec *_staticFunctionSpec = NULL; \
		JSPropertySpec *_propertySpec = NULL; \
		JSPropertySpec *_staticPropertySpec = NULL; \
		JSConstDoubleSpec *_constDoubleSpec = NULL; \
		JLConstIntegerSpec *_constIntegerSpec = NULL; \
		JSObject *_tmp_prototype = NULL; \
		JSObject **_parentPrototype = &_tmp_prototype; \
		JSBool (*_init)(JSContext *cx, JSObject *obj) = NULL; \
		jsval _revision = JSVAL_VOID;

#define END_CLASS \
		JL_CHK(obj); \
		if ( GetHostPrivate(cx)->camelCase == 1 ) { \
			_NormalizeFunctionSpecNames(_functionSpec); \
			_NormalizeFunctionSpecNames(_staticFunctionSpec); \
		} \
		JL_S_ASSERT( _class->name && _class->name[0], "Invalid class name." ); \
		*_prototype = JS_InitClass(cx, obj, *_parentPrototype, _class, _constructor, 0, NULL/*see JL_DefineClassProperties*/, _functionSpec, NULL/*see JL_DefineClassProperties*/, _staticFunctionSpec); \
		JSObject *dstObj; \
		dstObj = _constructor ? JS_GetConstructor(cx, *_prototype) : *_prototype; \
		JL_CHK(dstObj); \
		if ( _propertySpec != NULL ) \
			JL_CHK( JL_DefineClassProperties(cx, *_prototype, _propertySpec) ); \
		if ( _staticPropertySpec != NULL ) \
			JL_CHK( JL_DefineClassProperties(cx, dstObj, _staticPropertySpec) ); \
		if ( _constIntegerSpec != NULL ) \
			for ( ; _constIntegerSpec->name; _constIntegerSpec++ ) \
				JL_CHK( JS_DefineProperty(cx, dstObj, _constIntegerSpec->name, INT_TO_JSVAL(_constIntegerSpec->ival), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT) ); \
		if ( _constDoubleSpec != NULL ) \
			JL_CHK( JS_DefineConstDoubles(cx, dstObj, _constDoubleSpec) ); \
		JSBool found; \
		JL_CHK( JS_SetPropertyAttributes(cx, obj, _class->name, JSPROP_READONLY | JSPROP_PERMANENT, &found) ); \
		JL_CHKM( found, "Unable to set class flags." ); \
		JL_CHK( JS_DefinePropertyById(cx, dstObj, JLID(cx, _revision), _revision, NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT) ); \
		if ( _init ) \
			JL_CHK( _init(cx, dstObj) ); \
		return JS_TRUE; \
		JL_BAD; \
	}
#endif


#endif // _JSCLASS_H_
