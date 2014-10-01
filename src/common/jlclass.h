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

#include "../host/host2.h"


JL_BEGIN_NAMESPACE


typedef int32_t SourceId_t;

struct ConstValueSpec {
	const char *name;
	JS::Value val; // ASSERT( !val.isGCThing() )
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
	JSNative stdIterator;
	bool (*init)(JSContext *cx, ClassSpec *cs, JS::HandleObject proto, JS::HandleObject obj);
	SourceId_t sourceId;
	double buildDate;
};


// StoreProperty is used to override a property definition (from the prototype to the obj). see bug 526979
// if removeGetterAndSetter is false, it is up to the caller to filter calls using: if ( *vp != JSVAL_VOID ) return true;
// if removeGetterAndSetter is true, the value is stored for r/w getter or setter will never be called again.
ALWAYS_INLINE bool FASTCALL
StoreProperty( JSContext *cx, JS::HandleObject obj, IN JS::HandleId id, IN JS::HandleValue vp, bool removeGetterAndSetter ) {

	JS::Rooted<JSPropertyDescriptor> desc(cx);
	JL_CHK( JS_GetPropertyDescriptorById(cx, obj, id, &desc) );
	// bool found = desc.object() != NULL;
	unsigned int attrs = desc.attributes();
	JSPropertyOp getter = desc.getter();
	JSStrictPropertyOp setter = desc.setter();


	//JL_CHK( JS_GetPropertyAttrsGetterAndSetterById(cx, obj, id, &attrs, &found, &getter, &setter) );

	//ASSERT( found );
	// doc: JSPROP_SHARED: https://developer.mozilla.org/en/SpiderMonkey/JSAPI_Reference/JS_GetPropertyAttributes
	if ( (attrs & JSPROP_SHARED) == 0 ) // Has already been stored somewhere. The slot will be updated after JSPropertyOp returns.
		return true;
	attrs &= ~JSPROP_SHARED; // stored mean not shared.
	if ( removeGetterAndSetter ) { // store and never call the getter or setter again.

		getter = NULL;
		setter = NULL;
	}
	return JS_DefinePropertyById(cx, obj, id, vp, attrs, getter, setter);
	JL_BAD;
}


// because it is difficult to override properties by tinyId (JSPropertyOp) see. bz#526979
// note. PROPERTY_SWITCH uses enum values as tinyId
ALWAYS_INLINE bool FASTCALL
DefineClassProperties(JSContext *cx, IN JS::HandleObject obj, IN JSPropertySpec *ps) {

	for ( ; ps->name; ++ps )
		JL_CHK( JS_DefineProperty(cx, obj, ps->name, JL_UNDEFINED, ps->flags, ps->getter.propertyOp.op, ps->setter.propertyOp.op) );
	return true;
	JL_BAD;
}


ALWAYS_INLINE bool FASTCALL
DefineFunctions(JSContext *cx, JS::HandleObject obj, JSFunctionSpec *fs) {

	for ( ; fs->name; fs++ )
		JL_CHK( JS_DefineFunction(cx, obj, fs->name, fs->call.op, fs->nargs, fs->flags) );
	return true;
	JL_BAD;
}


ALWAYS_INLINE bool FASTCALL
DefineConstValues(JSContext *cx, JS::HandleObject obj, ConstValueSpec *cs) {

	JS::RootedValue tmp(cx);
    for ( ; cs->name; cs++ ) {

		ASSERT( !cs->val.isGCThing() );
		tmp.set(cs->val);
		JL_CHK( JS_DefineProperty(cx, obj, cs->name, tmp, JSPROP_READONLY | JSPROP_PERMANENT) );
	}
	return true;
	JL_BAD;
}

INLINE JSObject* FASTCALL
InitStatic( JSContext *cx, JS::HandleObject obj, ClassSpec *cs ) {

	ASSERT( obj );

	// not suitable for static classes
	ASSERT( !cs->parentProtoName );
	ASSERT( !cs->constructor );
	ASSERT( !cs->stdIterator );
	ASSERT( !cs->fs );
	ASSERT( !cs->ps );

	if ( cs->static_fs != NULL )
		JL_CHK( DefineFunctions(cx, obj, cs->static_fs) );

	if ( cs->static_ps != NULL )
		JL_CHK( DefineClassProperties(cx, obj, cs->static_ps) );

	if ( cs->static_const != NULL )
		JL_CHK( DefineConstValues(cx, obj, cs->static_const) );

	bool isExtensible;
	JL_CHK( JS_IsExtensible(cx, obj, &isExtensible) );
	if ( isExtensible ) {
	
		JL_CHK( JS_DefinePropertyById(cx, obj, JLID(cx, _sourceId), cs->sourceId, JSPROP_READONLY | JSPROP_PERMANENT) );
		JL_CHK( JS_DefinePropertyById(cx, obj, JLID(cx, _buildDate), cs->buildDate, JSPROP_READONLY | JSPROP_PERMANENT) );
	}

	if ( cs->init )
		JL_CHK( cs->init(cx, cs, JS::NullPtr(), obj) );

	return obj;
bad:
	return nullptr;
}

INLINE const ClassInfo * FASTCALL
InitClass( JSContext *cx, JS::HandleObject obj, ClassSpec *cs ) {

	ASSERT( cs->clasp.name && cs->clasp.name[0] ); // Invalid class name.
	jl::Global *glob = jl::Global::getGlobal(cx);

	JS::RootedObject parentProto(cx);
	JS::RootedObject ctor(cx);
	JS::RootedObject proto(cx);

	if ( cs->parentProtoName ) {

		ASSERT( cs->parentProtoName[0] );
		parentProto.set( glob->getCachedProto(cs->parentProtoName) );
		JL_CHKM( parentProto != nullptr, E_STR(cs->parentProtoName), E_STR("prototype"), E_NOTFOUND );
	}

	// doc: object that is the prototype for the newly initialized class.
	ASSERT( obj );
	proto.set(JS_InitClass(cx, obj, parentProto, &cs->clasp, cs->constructor, cs->ctorNArgs, NULL, NULL, NULL, NULL));

	JL_ASSERT( proto, E_CLASS, E_NAME(cs->clasp.name), E_CREATE ); //RTE
	ASSERT_IF( cs->clasp.flags & JSCLASS_HAS_PRIVATE, JL_GetPrivate(proto) == NULL );


	const ClassInfo *item = glob->addCachedClassInfo(cx, cs->clasp.name, &cs->clasp, proto);
	JL_CHKM( item, E_CLASS, E_NAME(cs->clasp.name), E_INIT, E_COMMENT("CacheClassProto") );

	ctor.set( cs->constructor ? JL_GetConstructor(cx, proto) : proto );

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

	if ( cs->stdIterator )
		JL_CHK( JS_DefineFunction(cx, ctor, "@@iterator", cs->stdIterator, 0, 0) );
	
	// info
	bool isExtensible;
	JL_CHK( JS_IsExtensible(cx, ctor, &isExtensible) );
	if ( isExtensible ) {
		
		JL_CHK( JS_DefinePropertyById(cx, ctor, JLID(cx, _sourceId), cs->sourceId, JSPROP_READONLY | JSPROP_PERMANENT) );
		JL_CHK( JS_DefinePropertyById(cx, ctor, JLID(cx, _buildDate), cs->buildDate, JSPROP_READONLY | JSPROP_PERMANENT) );
	}

	if ( cs->init )
		JL_CHK( cs->init(cx, cs, proto, ctor) );

	ASSERT( glob->getCachedClasp(cs->clasp.name) == &cs->clasp );
	ASSERT( glob->getCachedProto(cs->clasp.name) == proto );
	
	return item;
bad:
	return nullptr;
}

INLINE bool
InvalidConstructor(JSContext *cx, unsigned, JS::Value *) {

	JL_ERR( E_CLASS, E_NOTCONSTRUCT );
	JL_BAD;
}

INLINE bool
DefaultInstanceof(JSContext *cx, IN JS::HandleObject obj, IN JS::HandleValue v, OUT bool *bp) {

	// *bp = !JSVAL_IS_PRIMITIVE(*v) && js::GetObjectJSClass(JSVAL_TO_OBJECT(*v)) == js::GetObjectJSClass(obj); // incomplete

	if ( !v.isPrimitive() ) {
		
		const JSClass *objClass = js::GetObjectJSClass(obj);
		JS::RootedObject it(cx, &v.toObject());
		do {

			if ( js::GetObjectJSClass(it) == objClass ) {
				
				*bp = true;
				return true;
			}
			it = JL_GetPrototype(cx, it);
		} while ( it != NULL );
	}
	*bp = false;
	return true;
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
	JL_CHK( jl::InitClass(cx, obj, CLASSNAME::classSpec ) ); \


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
			cs.clasp.delProperty = JS_DeletePropertyStub; \
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

#define JL_CLASS(CLASSNAME) (&(CLASSNAME::classSpec->clasp))
#define JL_THIS_CLASS (&(classSpec->clasp))

#define JL_CLASS_NAME(CLASSNAME) (CLASSNAME::className)
#define JL_THIS_CLASS_NAME (className)

#define JL_THIS_CLASS_REVISION (classSpec->sourceId)

#define JL_CLASS_PROTOTYPE(cx, CLASSNAME) (jl::Global::getGlobal(cx)->getCachedProto(CLASSNAME::className) /*JL_GetCachedProto(JL_GetHostPrivate(cx), CLASSNAME::className)*/)
#define JL_THIS_CLASS_PROTOTYPE (jl::Global::getGlobal(cx)->getCachedProto(className) /*JL_GetCachedProto(JL_GetHostPrivate(cx), className)*/)

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
#define BEGIN_PROPERTY_SPEC static JSPropertySpec ps[] = { // *name, flags, getter, setter
#define END_PROPERTY_SPEC {NULL, 0, NULL, NULL}}; cs.ps = ps;

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
#define PROPERTY(name) { #name, JSPROP_PERMANENT|JSPROP_SHARED, JSOP_WRAPPER(_##name##Getter), JSOP_WRAPPER(_##name##Setter) },
#define PROPERTY_GETTER(name) { #name, JSPROP_PERMANENT|JSPROP_READONLY|JSPROP_SHARED, JSOP_WRAPPER(_##name##Getter), JSOP_NULLWRAPPER },
#define PROPERTY_SETTER(name) { #name, JSPROP_PERMANENT|JSPROP_SHARED, JSOP_NULLWRAPPER, JSOP_WRAPPER(_##name##Setter) },
//#define PROPERTY_SWITCH(name, function) { #name, name, JSPROP_PERMANENT|JSPROP_SHARED, JSOP_WRAPPER(_##function##Getter), JSOP_WRAPPER(_##function##Setter) }, // Used to define multiple properties with only one pair of getter/setter functions (an enum has to be defiend with less than 256 items !)
//#define PROPERTY_SWITCH_GETTER(name, function) { #name, name, JSPROP_PERMANENT|JSPROP_READONLY|JSPROP_SHARED, JSOP_WRAPPER(_##function##Getter), JSOP_NULLWRAPPER },
//#define PROPERTY_CREATE(name,id,flags,getter,setter) { #name, id, flags, JSOP_WRAPPER(_##getter), JSOP_WRAPPER(_##setter) },
//#define PROPERTY_DEFINE(name) { #name, 0, JSPROP_PERMANENT, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER },

// configuration
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

#define DEFINE_CONSTRUCTOR() \
	static bool Constructor(JSContext *cx, unsigned argc, JS::Value *vp)

// throw an error if one tries to construct it
#define IS_UNCONSTRUCTIBLE \
	ASSERT(cs.constructor == NULL); \
	cs.constructor = jl::InvalidConstructor;

#define HAS_DEFAULT_INSTANCEOF \
	ASSERT(cs.clasp.hasInstance == NULL); \
	cs.clasp.hasInstance = DefaultInstanceof;

#define HAS_FINALIZE cs.clasp.finalize = Finalize;
// make Finalize able to return a value ( good for bad: ):
//  #define DEFINE_FINALIZE() Finalize_withReturnValue(JSContext *cx, JS::HandleObject obj); static void Finalize(JSContext *cx, JS::HandleObject obj) { Finalize_withReturnValue(cx, obj) } ALWAYS_INLINE bool Finalize_withReturnValue(JSContext *cx, JS::HandleObject obj)
#define DEFINE_FINALIZE() static void Finalize(JSFreeOp *fop, JSObject *obj)

#define HAS_OBJECT_CONSTRUCTOR cs.clasp.construct = ObjectConstructor;
#define DEFINE_OBJECT_CONSTRUCTOR() static bool ObjectConstructor(JSContext *cx, unsigned argc, JS::Value *vp)

// not used any more
//#define HAS_CALL cs.clasp.call = Call;
//#define DEFINE_CALL() static bool Call(JSContext *cx, unsigned argc, JS::Value *vp)
// see also JL_DEFINE_CALL_FUNCTION_OBJ

#define HAS_CONVERT cs.clasp.convert = Convert;
#define DEFINE_CONVERT() static bool Convert(JSContext *cx, JS::HandleObject obj, JSType type, JS::Value *vp)

#define HAS_RESOLVE cs.clasp.resolve = Resolve;
#define DEFINE_RESOLVE() static bool Resolve(JSContext *cx, JS::HandleObject obj, JS::HandleId id)

#define HAS_NEW_RESOLVE cs.clasp.flags |= JSCLASS_NEW_RESOLVE; JSNewResolveOp tmp = NewResolve; cs.clasp.resolve = reinterpret_cast<JSResolveOp>(tmp);
#define DEFINE_NEW_RESOLVE() static bool NewResolve(JSContext *cx, JS::HandleObject obj, JS::HandleId id, JS::MutableHandleObject objp)

#define HAS_ENUMERATE cs.clasp.enumerate = Enumerate;
#define DEFINE_ENUMERATE() static bool Enumerate(JSContext *cx, JS::HandleObject obj, JSIterateOp enum_op, JS::MutableHandleValue statep, JS::MutableHandleId idp)

#define HAS_TRACER cs.clasp.flags |= JSCLASS_IMPLEMENTS_BARRIERS; cs.clasp.trace = Tracer;
#define DEFINE_TRACER() static void Tracer(JSTracer *trc, JSObject *obj)

#define HAS_HAS_INSTANCE cs.clasp.hasInstance = HasInstance;
#define DEFINE_HAS_INSTANCE() static bool HasInstance(JSContext *cx, JS::HandleObject obj, JS::MutableHandleValue vp, bool *bp)

//#define HAS_EQUALITY_OP js::Valueify(&cs.clasp)->ext.equality = EqualityOp;
//#define DEFINE_EQUALITY_OP() static bool EqualityOp(JSContext *cx, JS::HandleObject obj, const JS::Value *v, bool *bp)

//#define HAS_WRAPPED_OBJECT js::Valueify(&cs.clasp)->ext.wrappedObject = WrappedObject;
//#define DEFINE_WRAPPED_OBJECT() static JSObject* WrappedObject(JSContext *cx, JS::HandleObject obj)

#define HAS_INIT cs.init = Init;
#define DEFINE_INIT() static bool Init(JSContext *cx, jl::ClassSpec *cs, JS::HandleObject proto, JS::HandleObject obj)

#define HAS_ADD_PROPERTY cs.clasp.addProperty = AddProperty;
#define DEFINE_ADD_PROPERTY() static bool AddProperty(JSContext *cx, JS::HandleObject obj, JS::HandleId id, JS::MutableHandleValue vp)

#define HAS_DEL_PROPERTY cs.clasp.delProperty = DelProperty;
#define DEFINE_DEL_PROPERTY() static bool DelProperty(JSContext *cx, JS::HandleObject obj, JS::HandleId id, bool *succeeded)

#define HAS_GET_PROPERTY cs.clasp.getProperty = GetProperty;
#define DEFINE_GET_PROPERTY() static bool GetProperty(JSContext *cx, JS::HandleObject obj, JS::HandleId id, JS::MutableHandleValue vp)

#define HAS_SET_PROPERTY cs.clasp.setProperty = SetProperty;
#define DEFINE_SET_PROPERTY() static bool SetProperty(JSContext *cx, JS::HandleObject obj, JS::HandleId id, bool strict, JS::MutableHandleValue vp)

#define HAS_GET_OBJECT_OPS cs.clasp.getObjectOps = GetObjectOps;
#define DEFINE_GET_OBJECT_OPS() static JSObjectOps* GetObjectOps(JSContext *cx, JSClass *clasp)

#define HAS_CHECK_ACCESS cs.clasp.checkAccess = CheckAccess;
#define DEFINE_CHECK_ACCESS() static bool CheckAccess(JSContext *cx, JS::HandleObject obj, JS::HandleId id, JSAccessMode mode, JS::MutableHandleValue vp)

#define HAS_ITERATOR_OBJECT const_cast<js::Class*>(js::Valueify(&cs.clasp))->ext.iteratorObject = IteratorObject;
#define DEFINE_ITERATOR_OBJECT() static JSObject* IteratorObject(JSContext *cx, JS::HandleObject obj, bool keysonly)

#define HAS_STD_ITERATOR cs.stdIterator = StdIterator;
#define DEFINE_STD_ITERATOR() static bool StdIterator(JSContext *cx, unsigned argc, JS::Value *vp)

#define HAS_OUTER_OBJECT const_cast<js::Class*>(js::Valueify(&cs.clasp))->ext.outerObject = OuterObject;
#define OUTER_OBJECT() static JSObject *OuterObject(JSContext *cx, JS::HandleObject obj)

#define HAS_INNER_OBJECT const_cast<js::Class*>(js::Valueify(&cs.clasp))->ext.innerObject = InnerObject;
#define INNER_OBJECT() static JSObject *InnerObject(JSObject *obj)


// ops

#define HAS_OPS_LOOKUP_PROPERTY js::Valueify(&cs.clasp)->ops.lookupProperty = OpsLookupProperty;
#define DEFINE_OPS_LOOKUP_PROPERTY() static bool OpsLookupProperty(JSContext *cx, JS::HandleObject obj, JS::HandlePropertyName name, JS::MutableHandleObject objp, JS::MutableHandleShape propp)

#define HAS_OPS_GET_PROPERTY js::Valueify(&cs.clasp)->ops.getProperty = OpsGetProperty;
#define DEFINE_OPS_GET_PROPERTY() static bool OpsGetProperty((JSContext *cx, JS::HandleObject obj, JS::HandleObject receiver, JS::HandlePropertyName name, JS::MutableHandleValue vp)


// definition

#define DEFINE_FUNCTION(name) static bool _##name(JSContext *cx, unsigned argc, JS::Value *vp)

//#define DEFINE_FUNCTION_NARG(name, nargs) const static uint16_t _##name##_nargs = nargs; static bool _##name(JSContext *cx, unsigned argc, JS::Value *vp)

#define DEFINE_PROPERTY(name) static bool _##name(JSContext *cx, JS::HandleObject obj, JS::HandleId id, JS::MutableHandleValue vp)
#define DEFINE_PROPERTY_GETTER(name) static bool _##name##Getter(JSContext *cx, JS::HandleObject obj, JS::HandleId id, JS::MutableHandleValue vp)
#define DEFINE_PROPERTY_SETTER(name) static bool _##name##Setter(JSContext *cx, JS::HandleObject obj, JS::HandleId id, bool strict, JS::MutableHandleValue vp)

#define DEFINE_PROPERTY_GETTER_SWITCH(NAME, CALL, ARG) \
	static bool _##NAME##Getter(JSContext *cx, JS::HandleObject obj, JS::HandleId id, JS::MutableHandleValue vp) { \
		JS::RootedId tid(cx, INT_TO_JSID(ARG)); \
		return _##CALL##Getter(cx, obj, tid, vp); \
	}

#define DEFINE_PROPERTY_SETTER_SWITCH(NAME, CALL, ARG) \
	static bool _##NAME##Setter(JSContext *cx, JS::HandleObject obj, JS::HandleId id, bool strict, JS::MutableHandleValue vp) { \
		JS::RootedId tid(cx, INT_TO_JSID(ARG)); \
		return _##CALL##Setter(cx, obj, tid, strict, vp); \
	}

#define DEFINE_PROPERTY_SWITCH(NAME, CALL, ARG) DEFINE_PROPERTY_GETTER_SWITCH(NAME, CALL, ARG) DEFINE_PROPERTY_SETTER_SWITCH(NAME, CALL, ARG)

// documentation

#define ADD_DOC(NAME, SYNTAX, DESCRIPTION) \
	static const char *_##NAME##_syntax = SYNTAX; \
	static const char *_##NAME##_description = DESCRIPTION;
