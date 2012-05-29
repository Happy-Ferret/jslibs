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


JL_BEGIN_NAMESPACE

namespace pv {
	static const int NOTINYID = -1; // see DefineClassProperties()
}

typedef int32_t SourceId_t;

struct ConstValueSpec {
	const char *name;
	jsval val;
};


struct ClassSpec {
	JSClass clasp;
	JSNative constructor;
	unsigned ctorNArgs;
	const char *parentProtoName;
	JSPropertySpec *ps;
	JSPropertySpec *static_ps;
	JSFunctionSpec *fs;
	JSFunctionSpec *static_fs;
	ConstValueSpec *static_const;
	JSBool (*init)(JSContext *cx, ClassSpec *sc, JSObject *proto, JSObject *obj);
	SourceId_t sourceId;
	double buildDate;
};


// StoreProperty is used to override a property definition (from the prototype to the obj). see bug 526979
// if removeGetterAndSetter is false, it is up to the caller to filter calls using: if ( *vp != JSVAL_VOID ) return JS_TRUE;
// if removeGetterAndSetter is true, the value is stored for r/w getter or setter will never be called again.
ALWAYS_INLINE JSBool FASTCALL
StoreProperty( JSContext *cx, JSObject *obj, jsid id, const jsval *vp, bool removeGetterAndSetter ) {

	unsigned int attrs;
	JSBool found;
	JSPropertyOp getter;
	JSStrictPropertyOp setter;

	JL_CHK( JS_GetPropertyAttrsGetterAndSetterById(cx, obj, id, &attrs, &found, &getter, &setter) );
	ASSERT( found );
	// doc: JSPROP_SHARED: https://developer.mozilla.org/en/SpiderMonkey/JSAPI_Reference/JS_GetPropertyAttributes
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
// note. PROPERTY_SWITCH uses enum values as tinyId
ALWAYS_INLINE JSBool FASTCALL
DefineClassProperties(JSContext *cx, JSObject *obj, JSPropertySpec *ps) {

	for ( ; ps->name; ++ps ) {

		if ( ps->tinyid == jl::pv::NOTINYID )
			JL_CHK( JS_DefineProperty(cx, obj, ps->name, JSVAL_VOID, ps->getter, ps->setter, ps->flags) );
		else
			JL_CHK( JS_DefinePropertyWithTinyId(cx, obj, ps->name, ps->tinyid, JSVAL_VOID, ps->getter, ps->setter, ps->flags) );
	}
	return JS_TRUE;
	JL_BAD;
}


ALWAYS_INLINE JSBool FASTCALL
DefineFunctions(JSContext *cx, JSObject *obj, JSFunctionSpec *fs) {

	for ( ; fs->name; fs++ )
		JL_CHK( JS_DefineFunction(cx, obj, fs->name, fs->call, fs->nargs, fs->flags) );
	return JS_TRUE;
	JL_BAD;
}


ALWAYS_INLINE JSBool FASTCALL
DefineConstValues(JSContext *cx, JSObject *obj, ConstValueSpec *cs) {

    for ( ; cs->name; cs++ )
		JL_CHK( JS_DefineProperty(cx, obj, cs->name, cs->val, NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT) );
	return JS_TRUE;
	JL_BAD;
}

INLINE JSBool FASTCALL
InitStatic( JSContext *cx, JSObject *obj, ClassSpec *cs ) {

	JL_CHK(obj);

	if ( cs->static_fs != NULL )
		JL_CHK( DefineFunctions(cx, obj, cs->static_fs) );

	if ( cs->static_ps != NULL )
		JL_CHK( DefineClassProperties(cx, obj, cs->static_ps) );

	if ( cs->static_const != NULL )
		JL_CHK( DefineConstValues(cx, obj, cs->static_const) );

	if ( JS_IsExtensible(obj) ) {
	
		JL_CHK( JS_DefinePropertyById(cx, obj, JLID(cx, _sourceId), INT_TO_JSVAL(cs->sourceId), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT) );
		JL_CHK( JS_DefinePropertyById(cx, obj, JLID(cx, _buildDate), DOUBLE_TO_JSVAL(cs->buildDate), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT) );
	}

	if ( cs->init )
		JL_CHK( cs->init(cx, cs, NULL, obj) );

	return JS_TRUE;
	JL_BAD;
}

INLINE JSBool FASTCALL
InitClass( JSContext *cx, JSObject *obj, ClassSpec *cs ) {

	JL_CHK(obj);

	ASSERT( cs->clasp.name && cs->clasp.name[0] ); // Invalid class name.

	HostPrivate *hpv;
	hpv = JL_GetHostPrivate(cx);

	JSObject *parentProto;
	if ( cs->parentProtoName != NULL ) {

		parentProto = JL_GetCachedProto(hpv, cs->parentProtoName);
		JL_CHKM( parentProto != NULL, E_STR(cs->parentProtoName), E_STR("prototype"), E_NOTFOUND );
	} else {

		parentProto = NULL;
	}

	JSObject *proto; // doc: object that is the prototype for the newly initialized class.
	JSObject *ctor;

	proto = JS_InitClass(cx, obj, parentProto, &cs->clasp, cs->constructor, cs->ctorNArgs, NULL, NULL, NULL, NULL);

	JL_ASSERT( proto != NULL, E_CLASS, E_NAME(cs->clasp.name), E_CREATE ); //RTE
	ASSERT_IF( cs->clasp.flags & JSCLASS_HAS_PRIVATE, JL_GetPrivate(proto) == NULL );

	JL_CHKM( JL_CacheClassProto(hpv, cs->clasp.name, &cs->clasp, proto), E_CLASS, E_NAME(cs->clasp.name), E_INIT, E_COMMENT("CacheClassProto") );

	if ( cs->constructor )
		ctor = JL_GetConstructor(cx, proto);
	else
		ctor = proto;

	// functions
	if ( cs->fs )
		JL_CHK( DefineFunctions(cx, proto, cs->fs) );
	
	if ( cs->static_fs )
		JL_CHK( DefineFunctions(cx, ctor, cs->static_fs) );

	// properties
	if ( cs->ps != NULL )
		JL_CHK( DefineClassProperties(cx, proto, cs->ps) );

	if ( cs->static_ps != NULL )
		JL_CHK( DefineClassProperties(cx, ctor, cs->static_ps) );

	if ( cs->static_const != NULL )
		JL_CHK( DefineConstValues(cx, ctor, cs->static_const) );


	// info
	if ( JS_IsExtensible(ctor) ) {
		
		JL_CHK( JS_DefinePropertyById(cx, ctor, JLID(cx, _sourceId), INT_TO_JSVAL(cs->sourceId), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT) );
		JL_CHK( JS_DefinePropertyById(cx, ctor, JLID(cx, _buildDate), DOUBLE_TO_JSVAL(cs->buildDate), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT) );
	}

	if ( cs->init )
		JL_CHK( cs->init(cx, cs, proto, ctor) );

	ASSERT( JL_GetCachedClass(hpv, cs->clasp.name) == &cs->clasp );
	ASSERT( JL_GetCachedProto(hpv, cs->clasp.name) == proto );

	return JS_TRUE;
	JL_BAD;
}

INLINE JSBool
InvalidConstructor(JSContext *cx, unsigned, jsval *) {

	JL_ERR( E_CLASS, E_NOTCONSTRUCT );
	JL_BAD;
}

INLINE JSBool
DefaultInstanceof(JSContext *cx, JSObject *obj, const jsval *v, JSBool *bp) {

	// *bp = !JSVAL_IS_PRIMITIVE(*v) && js::GetObjectJSClass(JSVAL_TO_OBJECT(*v)) == js::GetObjectJSClass(obj); // incomplete

	if ( !JSVAL_IS_PRIMITIVE(*v) ) {
		
		JSClass *objClass = js::GetObjectJSClass(obj);
		JSObject *it = JSVAL_TO_OBJECT(*v);
		do {

			if ( js::GetObjectJSClass(it) == objClass ) {
				
				*bp = JS_TRUE;
				return JS_TRUE;
			}
			it = JL_GetPrototype(cx, it);
		} while ( it != NULL );
	}
	*bp = JS_FALSE;
	return JS_TRUE;
}

JL_END_NAMESPACE


#define DECLARE_STATIC() \
	extern jl::ClassSpec *jlStaticSpec; \

#define INIT_STATIC() \
	JL_CHK( InitStatic(cx, obj, jlStaticSpec ) ); \

#define BEGIN_STATIC \
	static jl::ClassSpec *JLClassSpecInit(); \
	jl::ClassSpec *jlStaticSpec = JLClassSpecInit(); \

#define CONFIGURE_STATIC \
	ALWAYS_INLINE jl::ClassSpec *JLClassSpecInit() { \
		static jl::ClassSpec cs; \

#define END_STATIC \
		return &cs; \
	} \



#define DECLARE_CLASS(CLASSNAME) \
	namespace CLASSNAME { \
		extern const char *className; \
		extern jl::ClassSpec *classSpec; \
	} \

#define INIT_CLASS(CLASSNAME) \
	JL_CHK( jl::InitClass(cx, obj, ::CLASSNAME::classSpec ) ); \


#define BEGIN_CLASS(CLASSNAME) \
	namespace CLASSNAME { \
		const char *className = #CLASSNAME; \
		ALWAYS_INLINE jl::ClassSpec *JLClassSpecInit(); \
		jl::ClassSpec *classSpec = JLClassSpecInit(); \

#define CONFIGURE_CLASS \
		ALWAYS_INLINE jl::ClassSpec *JLClassSpecInit() { \
			static jl::ClassSpec cs; \
			cs.clasp.name = className; \
			cs.clasp.addProperty = JS_PropertyStub; \
			cs.clasp.delProperty = JS_PropertyStub; \
			cs.clasp.getProperty = JS_PropertyStub; \
			cs.clasp.setProperty = JS_StrictPropertyStub; \
			cs.clasp.enumerate = JS_EnumerateStub; \
			cs.clasp.resolve = JS_ResolveStub; \
			cs.clasp.convert = JS_ConvertStub; \
			cs.buildDate = (double)__DATE__EPOCH * 1000; \

#define END_CLASS \
			return &cs; \
		} \
	} /* namespace */ \

//soubok>	in the JSClass finalize function, what is the best method to distinguish between object and prototype finalization ?
//jorendorff>	that's hard to do... if you're willing to be imprecise about it, you can just check to see if the object's prototype has the same class.
//jorendorff>	The best way is to keep the result of JS_InitClass around somewhere.
//jorendorff>	Then just compare with == to see if you're finalizing that object.

#define JL_CLASS(CLASSNAME) (&(::CLASSNAME::classSpec->clasp))
#define JL_THIS_CLASS (&(classSpec->clasp))

#define JL_CLASS_NAME(CLASSNAME) (::CLASSNAME::className)
#define JL_THIS_CLASS_NAME (className)

#define JL_THIS_CLASS_REVISION (classSpec->sourceId)

#define JL_CLASS_PROTOTYPE(cx, CLASSNAME) (JL_GetCachedProto(JL_GetHostPrivate(cx), ::CLASSNAME::className))
#define JL_THIS_CLASS_PROTOTYPE (JL_GetCachedProto(JL_GetHostPrivate(cx), className))

#define _NULL NULL // because in _##getter and _##setter, getter or setter can be NULL.

// const value
#define BEGIN_CONST static jl::ConstValueSpec static_const[] = {
#define END_CONST { NULL, 0 } }; cs.static_const = static_const;

#define CONST_INTEGER(name, ival) { #name, INT_TO_JSVAL(ival) },
#define CONST_INTEGER_SINGLE(name) { #name, INT_TO_JSVAL(name) },

#define CONST_DOUBLE(name, dval) { #name, DOUBLE_TO_JSVAL(dval) },
#define CONST_DOUBLE_SINGLE(name) { #name, DOUBLE_TO_JSVAL(name) },


// functions
#define BEGIN_FUNCTION_SPEC static JSFunctionSpec fs[] = { // *name, call, nargs, flags
#define END_FUNCTION_SPEC JS_FS_END }; cs.fs = fs;

#define BEGIN_STATIC_FUNCTION_SPEC static JSFunctionSpec static_fs[] = {
#define END_STATIC_FUNCTION_SPEC JS_FS_END }; cs.static_fs = static_fs;

// properties
#define BEGIN_PROPERTY_SPEC static JSPropertySpec ps[] = { // *name, tinyid, flags, getter, setter
#define END_PROPERTY_SPEC {NULL, 0, 0, NULL, NULL}}; cs.ps = ps;

#define BEGIN_STATIC_PROPERTY_SPEC static JSPropertySpec static_ps[] = {
#define END_STATIC_PROPERTY_SPEC {NULL, 0, 0, NULL, NULL}}; cs.static_ps = static_ps;

#define FUNCTION_DEFAULT_FLAGS (/*JSPROP_READONLY*/ 0)

#define FUNCTION(name) JS_FN( #name, _##name, 0, FUNCTION_DEFAULT_FLAGS ),
/* Doc. argv[argc] through argv[nargs - 1], if argc < nargs. Here nargs is the number of arguments the function ordinarily expects.
   This is the same as the function's length property. It is specified when the function is created,
	via either JSFunctionSpec.nargs or the nargs parameter to JS_FS, JS_DefineFunction, or JS_NewFunction.
	If present, these locations initially contain JSVAL_VOID. */
#define FUNCTION_ARGC(name, nargs) JS_FN( #name, _##name, nargs, FUNCTION_DEFAULT_FLAGS ),

#define FUNCTION_NARG(name) JS_FN( #name, _##name, _##name##_nargs, FUNCTION_DEFAULT_FLAGS ),

#define FUNCTION_ALIAS(alias, existingName) JS_FN( #alias, _##existingName, 0, FUNCTION_DEFAULT_FLAGS ),

// doc: JSPROP_SHARED - https://developer.mozilla.org/en/SpiderMonkey/JSAPI_Reference/JS_GetPropertyAttributes
#define PROPERTY(name) { #name, jl::pv::NOTINYID, JSPROP_PERMANENT|JSPROP_SHARED, _##name##Getter, _##name##Setter },
#define PROPERTY_GETTER(name) { #name, jl::pv::NOTINYID, JSPROP_PERMANENT|JSPROP_READONLY|JSPROP_SHARED, _##name##Getter, NULL },
#define PROPERTY_SETTER(name) { #name, jl::pv::NOTINYID, JSPROP_PERMANENT|JSPROP_SHARED, NULL, _##name##Setter },
#define PROPERTY_SWITCH(name, function) { #name, name, JSPROP_PERMANENT|JSPROP_SHARED, _##function##Getter, _##function##Setter }, // Used to define multiple properties with only one pair of getter/setter functions (an enum has to be defiend with less than 256 items !)
#define PROPERTY_SWITCH_GETTER(name, function) { #name, name, JSPROP_PERMANENT|JSPROP_READONLY|JSPROP_SHARED, _##function##Getter, NULL },
#define PROPERTY_CREATE(name,id,flags,getter,setter) { #name, id, flags, _##getter, _##setter },
#define PROPERTY_DEFINE(name) { #name, 0, JSPROP_PERMANENT, NULL, NULL },

// configuration
#define FROZEN_PROTOTYPE cs.clasp.flags |= JSCLASS_FREEZE_PROTO;
#define FROZEN_CONSTRUCTOR cs.clasp.flags |= JSCLASS_FREEZE_CTOR;
#define HAS_PRIVATE cs.clasp.flags |= JSCLASS_HAS_PRIVATE;
#define HAS_RESERVED_SLOTS(COUNT) cs.clasp.flags |= JSCLASS_HAS_RESERVED_SLOTS(COUNT);
#define IS_GLOBAL cs.clasp.flags |= JSCLASS_GLOBAL_FLAGS;
#define IS_ANONYMOUS cs.clasp.flags |= JSCLASS_IS_ANONYMOUS;
#define HAS_ALL_PROPERTIES_SHARED cs.clasp.flags |= JSCLASS_SHARE_ALL_PROPERTIES;
#define REVISION(NUMBER) cs.sourceId = (NUMBER);

#define HAS_PROTOTYPE(PROTOTYPENAME) \
	cs.parentProtoName = #PROTOTYPENAME; \

#define HAS_CONSTRUCTOR_ARGC(ARGC) \
	ASSERT(cs.constructor == NULL); \
	cs.constructor = Constructor; cs.ctorNArgs = (ARGC);

#define HAS_CONSTRUCTOR \
	ASSERT(cs.constructor == NULL); \
	cs.constructor = Constructor;

#define DEFINE_CONSTRUCTOR() static JSBool Constructor(JSContext *cx, unsigned argc, jsval *vp)

// throw an error if one tries to construct it
#define IS_UNCONSTRUCTIBLE \
	ASSERT(cs.constructor == NULL); \
	cs.constructor = jl::InvalidConstructor;

#define HAS_DEFAULT_INSTANCEOF \
	ASSERT(cs.clasp.hasInstance == NULL); \
	cs.clasp.hasInstance = DefaultInstanceof;

#define HAS_FINALIZE cs.clasp.finalize = Finalize;
// make Finalize able to return a value ( good for bad: ):
//  #define DEFINE_FINALIZE() Finalize_withReturnValue(JSContext *cx, JSObject *obj); static void Finalize(JSContext *cx, JSObject *obj) { Finalize_withReturnValue(cx, obj) } ALWAYS_INLINE JSBool Finalize_withReturnValue(JSContext *cx, JSObject *obj)
#define DEFINE_FINALIZE() static void Finalize(JSFreeOp *fop, JSObject *obj)

#define HAS_OBJECT_CONSTRUCTOR cs.clasp.construct = ObjectConstructor;
#define DEFINE_OBJECT_CONSTRUCTOR() static JSBool ObjectConstructor(JSContext *cx, JSObject *obj, unsigned argc, jsval *argv, jsval *rval)

#define HAS_CALL cs.clasp.call = Call;
#define DEFINE_CALL() static JSBool Call(JSContext *cx, unsigned argc, jsval *vp)
// see also JL_DEFINE_CALL_FUNCTION_OBJ

#define HAS_CONVERT cs.clasp.convert = Convert;
#define DEFINE_CONVERT() static JSBool Convert(JSContext *cx, JSObject *obj, JSType type, jsval *vp)

#define HAS_RESOLVE cs.clasp.resolve = Resolve;
#define DEFINE_RESOLVE() static JSBool Resolve(JSContext *cx, JSObject *obj, jsid id)

#define HAS_NEW_RESOLVE cs.clasp.flags |= JSCLASS_NEW_RESOLVE; JSNewResolveOp tmp = NewResolve; cs.clasp.resolve = (JSResolveOp)tmp;
//#define HAS_NEW_RESOLVE_GETS_START cs.clasp.flags |= JSCLASS_NEW_RESOLVE_GETS_START; JSNewResolveOp tmp = NewResolve; cs.clasp.resolve = (JSResolveOp)tmp;
#define DEFINE_NEW_RESOLVE() static JSBool NewResolve(JSContext *cx, JSObject *obj, jsid id, unsigned flags, JSObject **objp)

#define HAS_ENUMERATE cs.clasp.enumerate = Enumerate;
#define DEFINE_ENUMERATE() static JSBool Enumerate(JSContext *cx, JSObject *obj)

#define HAS_TRACER cs.clasp.trace = Tracer;
#define DEFINE_TRACER() static void Tracer(JSTracer *trc, JSObject *obj)

#define HAS_HAS_INSTANCE cs.clasp.hasInstance = HasInstance;
#define DEFINE_HAS_INSTANCE() static JSBool HasInstance(JSContext *cx, JSObject *obj, const jsval *v, JSBool *bp)

#define HAS_EQUALITY_OP js::Valueify(&cs.clasp)->ext.equality = EqualityOp;
#define DEFINE_EQUALITY_OP() static JSBool EqualityOp(JSContext *cx, JSObject *obj, const jsval *v, JSBool *bp)

#define HAS_WRAPPED_OBJECT js::Valueify(&cs.clasp)->ext.wrappedObject = WrappedObject;
#define DEFINE_WRAPPED_OBJECT() static JSObject* WrappedObject(JSContext *cx, JSObject *obj)

#define HAS_INIT cs.init = Init;
#define DEFINE_INIT() static JSBool Init(JSContext *cx, jl::ClassSpec *sc, JSObject *proto, JSObject *obj)

#define HAS_ADD_PROPERTY cs.clasp.addProperty = AddProperty;
#define DEFINE_ADD_PROPERTY() static JSBool AddProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp)

#define HAS_DEL_PROPERTY cs.clasp.delProperty = DelProperty;
#define DEFINE_DEL_PROPERTY() static JSBool DelProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp)

#define HAS_GET_PROPERTY cs.clasp.getProperty = GetProperty;
#define DEFINE_GET_PROPERTY() static JSBool GetProperty(JSContext *cx, JSObject *obj, jsid id, jsval *vp)

#define HAS_SET_PROPERTY cs.clasp.setProperty = SetProperty;
#define DEFINE_SET_PROPERTY() static JSBool SetProperty(JSContext *cx, JSObject *obj, jsid id, JSBool strict, jsval *vp)

#define HAS_GET_OBJECT_OPS cs.clasp.getObjectOps = GetObjectOps;
#define DEFINE_GET_OBJECT_OPS() static JSObjectOps* GetObjectOps(JSContext *cx, JSClass *clasp)

#define HAS_CHECK_ACCESS cs.clasp.checkAccess = CheckAccess;
#define DEFINE_CHECK_ACCESS() static JSBool CheckAccess(JSContext *cx, JSObject *obj, jsid id, JSAccessMode mode, jsval *vp)

#define HAS_ITERATOR_OBJECT js::Valueify(&cs.clasp)->ext.iteratorObject = IteratorObject;
#define DEFINE_ITERATOR_OBJECT() static JSObject* IteratorObject(JSContext *cx, JSObject *obj, JSBool keysonly)


// ops

#define HAS_OPS_LOOKUP_PROPERTY js::Valueify(&cs.clasp)->ops.lookupProperty = OpsLookupProperty;
#define DEFINE_OPS_LOOKUP_PROPERTY() static JSBool OpsLookupProperty(JSContext *cx, JSObject *obj, jsid id, JSObject **objp, JSProperty **propp)

#define HAS_OPS_GET_PROPERTY js::Valueify(&cs.clasp)->ops.getProperty = OpsGetProperty;
#define DEFINE_OPS_GET_PROPERTY() static JSBool OpsGetProperty(JSContext *cx, JSObject *obj, JSObject *receiver, jsid id, js::Value *vp)


// definition

#define DEFINE_FUNCTION(name) static JSBool _##name(JSContext *cx, unsigned argc, jsval *vp)

#define DEFINE_FUNCTION_NARG(name, nargs) const static uint16_t _##name##_nargs = nargs; static JSBool _##name(JSContext *cx, unsigned argc, jsval *vp)

#define DEFINE_PROPERTY(name) static JSBool _##name(JSContext *cx, JSObject *obj, jsid id, jsval *vp)
#define DEFINE_PROPERTY_GETTER(name) static JSBool _##name##Getter(JSContext *cx, JSObject *obj, jsid id, jsval *vp)
#define DEFINE_PROPERTY_SETTER(name) static JSBool _##name##Setter(JSContext *cx, JSObject *obj, jsid id, JSBool strict, jsval *vp)


// documentation

#define ADD_DOC(NAME, SYNTAX, DESCRIPTION) \
	static const char *_##NAME##_syntax = SYNTAX; \
	static const char *_##NAME##_description = DESCRIPTION;
