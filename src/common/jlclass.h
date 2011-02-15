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

typedef uint32_t JLRevisionType;

struct JLConstIntegerSpec {
    int ival;
    const char *name;
};

struct JLClassSpec {
	JSClass clasp;
	JSNative constructor;
	uintN nargs;
	const char *parentProtoName;
	JSPropertySpec *ps;
	JSPropertySpec *static_ps;
	JSFunctionSpec *fs;
	JSFunctionSpec *static_fs;
	JSConstDoubleSpec *cds;
	JLConstIntegerSpec *cis;
	JSBool (*init)(JSContext *cx, JLClassSpec *sc, JSObject *proto, JSObject *obj);
	JLRevisionType revision;
};


// JL_StoreProperty is used to override a property definition (from the prototype to the obj.
// if removeGetterAndSetter is false, it is up to the caller to filter calls using: if ( *vp != JSVAL_VOID ) return JS_TRUE;
// if removeGetterAndSetter is true, the value is stored for r/w getter or setter will never be called again.
inline JSBool JL_StoreProperty( JSContext *cx, JSObject *obj, jsid id, const jsval *vp, bool removeGetterAndSetter ) {

	JSBool found;
	uintN attrs;
	JSPropertyOp getter;
	JSStrictPropertyOp setter;
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

	char *buf = JL_strdup(name); // (TBD) if needed, do free with js_free() function.
	buf[0] = (char)tolower(buf[0]);
	return buf;
}

ALWAYS_INLINE void JLNormalizeFunctionSpecNames( JSFunctionSpec *functionSpec ) {

	for ( JSFunctionSpec *it = functionSpec; it && it->name; it++ )
		it->name = JLNormalizeFunctionName(it->name);
}

inline JSBool JLInitStatic( JSContext *cx, JSObject *obj, JLClassSpec *cs ) {

	JL_CHK(obj);

	if ( JL_GetHostPrivate(cx)->camelCase == 1 )
		JLNormalizeFunctionSpecNames(cs->static_fs);

	if ( cs->static_fs != NULL )
		JL_CHK( JS_DefineFunctions(cx, obj, cs->static_fs) );

	if ( cs->static_ps != NULL )
		JL_CHK( JL_DefineClassProperties(cx, obj, cs->static_ps) );

	if ( cs->cis != NULL )
		for ( JLConstIntegerSpec *it = cs->cis ; it->name; ++it )
			JL_CHK( JS_DefineProperty(cx, obj, it->name, INT_TO_JSVAL(jl::SafeCast<int>(it->ival)), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT) );

	if ( cs->cds != NULL )
		JL_CHK( JS_DefineConstDoubles(cx, obj, cs->cds) );

	if ( cs->init )
		JL_CHK( cs->init(cx, cs, NULL, obj) );

	return JS_TRUE;
	JL_BAD;
}

inline JSBool JLInitClass( JSContext *cx, JSObject *obj, JLClassSpec *cs ) {

	JL_CHK(obj);
	JL_S_ASSERT( cs->clasp.name && cs->clasp.name[0], "Invalid class name." );

	if ( JL_GetHostPrivate(cx)->camelCase == 1 ) {

		JLNormalizeFunctionSpecNames(cs->fs);
		JLNormalizeFunctionSpecNames(cs->static_fs);
	}

	HostPrivate *hpv;
	hpv = JL_GetHostPrivate(cx);

	JSObject *parent_proto;
	if ( cs->parentProtoName != NULL ) {

		parent_proto = JL_GetCachedClassProto(hpv, cs->parentProtoName)->proto;
		JL_S_ASSERT( parent_proto != NULL, "%s prototype not found", cs->parentProtoName );
//		JL_ASSERT( parent_proto != NULL ); // "parent class has no prototype"
	} else {

		parent_proto = NULL;
	}

	JSObject *proto;
	proto = JS_InitClass(cx, obj, parent_proto, &cs->clasp, cs->constructor, cs->nargs, NULL, cs->fs, NULL, cs->static_fs);
	
	JL_S_ASSERT( proto != NULL, "Unable to create the %s class", cs->clasp.name );

	JL_ASSERT_IF( cs->clasp.flags & JSCLASS_HAS_PRIVATE, JL_GetPrivate(cx, proto) == NULL );

	JL_CHKM( JL_CacheClassProto(hpv, cs->clasp.name, &cs->clasp, proto), "Unable to cache %s class prototype", cs->clasp.name );

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
	JL_CHK( JS_SetPropertyAttributes(cx, obj, cs->clasp.name, JSPROP_READONLY | JSPROP_PERMANENT, &found) );
	JL_ASSERT( found ); // "Unable to set class flags."

	JL_CHK( JS_DefinePropertyById(cx, staticDest, JLID(cx, _revision), INT_TO_JSVAL(cs->revision), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT) );

	if ( cs->init )
		JL_CHK( cs->init(cx, cs, proto, staticDest) );

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
		static const char *className = #CLASSNAME; \
		ALWAYS_INLINE JLClassSpec *JLClassSpecInit(); \
		JLClassSpec *jlClassSpec = JLClassSpecInit(); \

#define CONFIGURE_CLASS \
		ALWAYS_INLINE JLClassSpec *JLClassSpecInit() { \
			static JLClassSpec cs; \
			cs.clasp.name = (char *)className; \
			cs.clasp.addProperty = JS_PropertyStub; \
			cs.clasp.delProperty = JS_PropertyStub; \
			cs.clasp.getProperty = JS_PropertyStub; \
			cs.clasp.setProperty = JS_StrictPropertyStub; \
			cs.clasp.enumerate = JS_EnumerateStub; \
			cs.clasp.resolve = JS_ResolveStub; \
			cs.clasp.convert = JS_ConvertStub; \
			cs.clasp.finalize = JS_FinalizeStub; \

#define END_CLASS \
			return &cs; \
		} \
	} \

//soubok>	in the JSClass finalize function, what is the best method to distinguish between object and prototype finalization ?
//jorendorff>	that's hard to do... if you're willing to be imprecise about it, you can just check to see if the object's prototype has the same class.
//jorendorff>	The best way is to keep the result of JS_InitClass around somewhere.
//jorendorff>	Then just compare with == to see if you're finalizing that object.

#define JL_CLASS(CLASSNAME) (&(CLASSNAME::jlClassSpec->clasp))
#define JL_THIS_CLASS (&(jlClassSpec->clasp))
#define JL_THIS_CLASS_NAME (jlClassSpec->clasp.name)
#define JL_THIS_REVISION (jlClassSpec->revision)

#define JL_PROTOTYPE(cx, CLASSNAME) (JL_GetCachedClassProto(JL_GetHostPrivate(cx), CLASSNAME::jlClassSpec->clasp.name)->proto)
#define JL_THIS_PROTOTYPE (JL_GetCachedClassProto(JL_GetHostPrivate(cx), jlClassSpec->clasp.name)->proto)

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
#define BEGIN_FUNCTION_SPEC static JSFunctionSpec fs[] = { // *name, call, nargs, flags
#define END_FUNCTION_SPEC {NULL, NULL, 0, 0}}; cs.fs = fs;

#define BEGIN_STATIC_FUNCTION_SPEC static JSFunctionSpec static_fs[] = {
#define END_STATIC_FUNCTION_SPEC {NULL, NULL, 0, 0}}; cs.static_fs = static_fs;

// properties
#define BEGIN_PROPERTY_SPEC static JSPropertySpec ps[] = { // *name, tinyid, flags, getter, setter
#define END_PROPERTY_SPEC {NULL, 0, 0, NULL, NULL}}; cs.ps = ps;

#define BEGIN_STATIC_PROPERTY_SPEC static JSPropertySpec static_ps[] = {
#define END_STATIC_PROPERTY_SPEC {NULL, 0, 0, NULL, NULL}}; cs.static_ps = static_ps;

#define FUNCTION(name) JS_FN( #name, _##name, 0, 0 ),
/* Doc. argv[argc] through argv[nargs - 1], if argc < nargs. Here nargs is the number of arguments the function ordinarily expects.
   This is the same as the function's length property. It is specified when the function is created,
	via either JSFunctionSpec.nargs or the nargs parameter to JS_FS, JS_DefineFunction, or JS_NewFunction.
	If present, these locations initially contain JSVAL_VOID. */
#define FUNCTION_ARGC(name, nargs) JS_FN( #name, _##name, nargs, 0 ),
#define FUNCTION_ALIAS(alias, name) JS_FN( #alias, _##name, 0, 0 ),

#define PROPERTY(name) { #name, -1, JSPROP_PERMANENT|JSPROP_SHARED, _##name##Getter, _##name##Setter },
#define PROPERTY_READ(name) { #name, -1, JSPROP_PERMANENT|JSPROP_READONLY|JSPROP_SHARED, _##name##Getter, NULL }, // (TBD) rename into PROPERTY_GETTER
#define PROPERTY_WRITE(name) { #name, -1, JSPROP_PERMANENT|JSPROP_SHARED, NULL, _##name##Setter }, // (TBD) rename into PROPERTY_SETTER
#define PROPERTY_SWITCH(name, function) { #name, name, JSPROP_PERMANENT|JSPROP_SHARED, _##function##Getter, _##function##Setter }, // Used to define multiple properties with only one pari of getter/setter functions ( an enum has to be defiend ... less than 256 items ! )
#define PROPERTY_SWITCH_READ(name, function) { #name, name, JSPROP_PERMANENT|JSPROP_READONLY|JSPROP_SHARED, _##function##Getter, NULL },
#define PROPERTY_CREATE(name,id,flags,getter,setter) { #name, id, flags, _##getter, _##setter },
#define PROPERTY_DEFINE(name) { #name, 0, JSPROP_PERMANENT, NULL, NULL },

// configuration
#define HAS_PRIVATE cs.clasp.flags |= JSCLASS_HAS_PRIVATE;
#define HAS_RESERVED_SLOTS(COUNT) cs.clasp.flags |= JSCLASS_HAS_RESERVED_SLOTS(COUNT);
#define CONSTRUCT_PROTOTYPE cs.clasp.flags |= JSCLASS_CONSTRUCT_PROTOTYPE;
#define IS_GLOBAL cs.clasp.flags |= JSCLASS_GLOBAL_FLAGS;
#define HAS_ALL_PROPERTIES_SHARED cs.clasp.flags |= JSCLASS_SHARE_ALL_PROPERTIES;
#define REVISION(NUMBER) cs.revision = (NUMBER);

#define HAS_PROTOTYPE(PROTOTYPENAME) \
	cs.parentProtoName = #PROTOTYPENAME; \

#define HAS_CONSTRUCTOR_ARGC(ARGC) cs.constructor = Constructor; cs.argc = (ARGC);
#define HAS_CONSTRUCTOR cs.constructor = Constructor;
#define DEFINE_CONSTRUCTOR() static JSBool Constructor(JSContext *cx, uintN argc, jsval *vp)

#define HAS_FINALIZE cs.clasp.finalize = Finalize;
#define DEFINE_FINALIZE() static void Finalize(JSContext *cx, JSObject *obj)

#define HAS_OBJECT_CONSTRUCTOR cs.clasp.construct = ObjectConstructor;
#define DEFINE_OBJECT_CONSTRUCTOR() static JSBool ObjectConstructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)

#define HAS_CALL cs.clasp.call = Call;
#define DEFINE_CALL() static JSBool Call(JSContext *cx, uintN argc, jsval *vp)

#define HAS_CONVERT cs.clasp.convert = Convert;
#define DEFINE_CONVERT() static JSBool Convert(JSContext *cx, JSObject *obj, JSType type, jsval *vp)

#define HAS_RESOLVE cs.clasp.resolve = Resolve;
#define DEFINE_RESOLVE() static JSBool Resolve(JSContext *cx, JSObject *obj, jsid id)

#define HAS_NEW_RESOLVE cs.clasp.flags |= JSCLASS_NEW_RESOLVE; JSNewResolveOp tmp = NewResolve; cs.clasp.resolve = (JSResolveOp)tmp;
#define HAS_NEW_RESOLVE_GETS_START cs.clasp.flags |= JSCLASS_NEW_RESOLVE_GETS_START; JSNewResolveOp tmp = NewResolve; cs.clasp.resolve = (JSResolveOp)tmp;
#define DEFINE_NEW_RESOLVE() static JSBool NewResolve(JSContext *cx, JSObject *obj, jsid id, uintN flags, JSObject **objp)

#define HAS_ENUMERATE cs.clasp.enumerate = Enumerate;
#define DEFINE_ENUMERATE() static JSBool Enumerate(JSContext *cx, JSObject *obj)

#define HAS_TRACER cs.clasp.flags |= JSCLASS_MARK_IS_TRACE; JSTraceOp tmp = Tracer; cs.clasp.mark = (JSMarkOp)tmp;
#define DEFINE_TRACER() static void Tracer(JSTracer *trc, JSObject *obj)

#define HAS_HAS_INSTANCE cs.clasp.hasInstance = HasInstance;
#define DEFINE_HAS_INSTANCE() static JSBool HasInstance(JSContext *cx, JSObject *obj, const jsval *v, JSBool *bp)

#define HAS_EQUALITY_OP js::Valueify(&cs.clasp)->ext.equality = js::Valueify(EqualityOp);/* cs.clasp.flags |= JSObject::HAS_EQUALITY;*/
#define DEFINE_EQUALITY_OP() static JSBool EqualityOp(JSContext *cx, JSObject *obj, const jsval *v, JSBool *bp)

#define HAS_WRAPPED_OBJECT js::Valueify(&cs.clasp)->ext.wrappedObject = WrappedObject;
#define DEFINE_WRAPPED_OBJECT() static JSObject* WrappedObject(JSContext *cx, JSObject *obj)

#define HAS_INIT cs.init = Init;
#define DEFINE_INIT() static JSBool Init(JSContext *cx, JLClassSpec *sc, JSObject *proto, JSObject *obj)

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

#define DEFINE_FUNCTION(name) static JSBool _##name(JSContext *cx, uintN argc, jsval *vp)

#define DEFINE_PROPERTY(name) static JSBool _##name(JSContext *cx, JSObject *obj, jsid id, jsval *vp)
#define DEFINE_PROPERTY_GETTER(name) static JSBool _##name##Getter(JSContext *cx, JSObject *obj, jsid id, jsval *vp)
#define DEFINE_PROPERTY_SETTER(name) static JSBool _##name##Setter(JSContext *cx, JSObject *obj, jsid id, JSBool strict, jsval *vp)

#endif // _JSCLASS_H_
