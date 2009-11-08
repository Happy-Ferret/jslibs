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

#include <ctype.h>

#define _NULL NULL // because in _##getter and _##setter, getter or setter can be NULL.

struct JLConstIntegerSpec {
	jsint ival;
	const char *name;
};

// declaration

// const integer
#define BEGIN_CONST_INTEGER_SPEC JLConstIntegerSpec _tmp_cis[] = {
#define END_CONST_INTEGER_SPEC {0}}; _constIntegerSpec = _tmp_cis;
#define CONST_INTEGER(name,value) { value, #name },
#define CONST_INTEGER_SINGLE(name) { name, #name },

// const double
#define BEGIN_CONST_DOUBLE_SPEC JSConstDoubleSpec _tmp_cds[] = { // dval; *name; flags; spare[3];
#define END_CONST_DOUBLE_SPEC {0}}; _constDoubleSpec = _tmp_cds;
#define CONST_DOUBLE(name,value) { value, #name },
#define CONST_DOUBLE_SINGLE(name) { name, #name },

// functions

#define BEGIN_FUNCTION_SPEC JSFunctionSpec _tmp_fs[] = { // *name, call, nargs, flags, extra
#define END_FUNCTION_SPEC {0}}; _functionSpec = _tmp_fs;
#define BEGIN_STATIC_FUNCTION_SPEC JSFunctionSpec _tmp_sfs[] = {
#define END_STATIC_FUNCTION_SPEC {0}}; _staticFunctionSpec = _tmp_sfs;

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
#define END_PROPERTY_SPEC {0}}; _propertySpec = _tmp_ps;
#define BEGIN_STATIC_PROPERTY_SPEC JSPropertySpec _tmp_sps[] = {
#define END_STATIC_PROPERTY_SPEC {0}}; _staticPropertySpec = _tmp_sps;

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


// definition

#define DEFINE_FUNCTION_FAST(name) static JSBool _##name(JSContext *cx, uintN argc, jsval *vp)
#define DEFINE_FUNCTION(name) static JSBool _##name(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)

#define DEFINE_PROPERTY(name) static JSBool _##name(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
#define DEFINE_PROPERTY_GETTER(name) static JSBool _##name##Getter(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
#define DEFINE_PROPERTY_SETTER(name) static JSBool _##name##Setter(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
//#define DEFINE_PROPERTY_NULL(name) static JSPropertyOp _##name = NULL;


#define DEFINE_CONSTRUCTOR() static JSBool Constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
#define DEFINE_OBJECT_CONSTRUCTOR() static JSBool ObjectConstructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
#define DEFINE_FINALIZE() static void Finalize(JSContext *cx, JSObject *obj)
#define DEFINE_CONVERT() static JSBool Convert(JSContext *cx, JSObject *obj, JSType type, jsval *vp)
#define DEFINE_RESOLVE() static JSBool Resolve(JSContext *cx, JSObject *obj, jsval id)
#define DEFINE_NEW_RESOLVE() static JSBool NewResolve(JSContext *cx, JSObject *obj, jsval id, uintN flags, JSObject **objp)
#define DEFINE_ENUMERATE() static JSBool Enumerate(JSContext *cx, JSObject *obj)
#define DEFINE_TRACER() static void Tracer(JSTracer *trc, JSObject *obj)
#define DEFINE_INIT() static JSBool Init(JSContext *cx, JSObject *obj)
#define DEFINE_FREE() static JSBool Free(JSContext *cx, JSObject *obj)
#define DEFINE_ADD_PROPERTY() static JSBool AddProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
#define DEFINE_DEL_PROPERTY() static JSBool DelProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
#define DEFINE_GET_PROPERTY() static JSBool GetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
#define DEFINE_SET_PROPERTY() static JSBool SetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
#define DEFINE_HAS_INSTANCE() static JSBool HasInstance(JSContext *cx, JSObject *obj, jsval v, JSBool *bp)
#define DEFINE_CALL() static JSBool Call(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
#define DEFINE_EQUALITY() static JSBool Equality(JSContext *cx, JSObject *obj, jsval v, JSBool *bp)
#define DEFINE_WRAPPED_OBJECT() static JSObject* WrappedObject(JSContext *cx, JSObject *obj)
#define DEFINE_GET_OBJECT_OPS() static JSObjectOps* GetObjectOps(JSContext *cx, JSClass *clasp)
#define DEFINE_CHECK_ACCESS() static JSBool CheckAccess(JSContext *cx, JSObject *obj, jsval id, JSAccessMode mode, jsval *vp)
#define DEFINE_XDR() static JSBool XDRObject(JSXDRState *xdr, JSObject **objp)
#define DEFINE_ITERATOR_OBJECT() static JSObject* IteratorObject(JSContext *cx, JSObject *obj, JSBool keysonly)

// class configuration
#define HAS_PRIVATE   _class->flags |= JSCLASS_HAS_PRIVATE;
#define HAS_RESERVED_SLOTS(COUNT)   _class->flags |= JSCLASS_HAS_RESERVED_SLOTS(COUNT);
#define HAS_PROTOTYPE(PROTOTYPE)   *_parentPrototype = (PROTOTYPE);
#define CONSTRUCT_PROTOTYPE   _class->flags |= JSCLASS_CONSTRUCT_PROTOTYPE;
#define SHARE_ALL_PROPERTIES   _class->flags |= JSCLASS_SHARE_ALL_PROPERTIES;

#define HAS_CONSTRUCTOR   _constructor = Constructor;
#define HAS_OBJECT_CONSTRUCTOR   _class->construct = ObjectConstructor;
#define HAS_CALL   _class->call = Call;
#define HAS_FINALIZE   _class->finalize = Finalize;
#define HAS_CONVERT   _class->convert = Convert;
#define HAS_RESOLVE   _class->resolve = Resolve;
#define HAS_NEW_RESOLVE   _class->flags |= JSCLASS_NEW_RESOLVE; _class->resolve = (JSResolveOp)NewResolve;
#define HAS_NEW_RESOLVE_GETS_START   _class->flags |= JSCLASS_NEW_RESOLVE_GETS_START;
#define HAS_ENUMERATE  _class->enumerate = Enumerate;
#define HAS_TRACER   _class->flags |= JSCLASS_MARK_IS_TRACE; _class->mark = (JSMarkOp)Tracer;
#define HAS_HAS_INSTANCE _class->hasInstance = HasInstance;
#define HAS_EQUALITY _xclass.base.flags |= JSCLASS_IS_EXTENDED; _xclass.equality = Equality;
#define HAS_WRAPPED_OBJECT _xclass.base.flags |= JSCLASS_IS_EXTENDED; _xclass.wrappedObject = WrappedObject;
#define HAS_INIT  _init = Init;
#define HAS_FREE  _free = Free;
#define HAS_ADD_PROPERTY   _class->addProperty = AddProperty;
#define HAS_DEL_PROPERTY   _class->delProperty = DelProperty;
#define HAS_GET_PROPERTY   _class->getProperty = GetProperty;
#define HAS_SET_PROPERTY   _class->setProperty = SetProperty;
#define HAS_GET_OBJECT_OPS _class->getObjectOps = GetObjectOps;
#define HAS_CHECK_ACCESS   _class->checkAccess = CheckAccess;
#define IS_GLOBAL  _class->flags |= JSCLASS_IS_GLOBAL;
#define HAS_XDR _class->xdrObject = XDRObject;
#define HAS_ITERATOR_OBJECT _xclass.base.flags |= JSCLASS_IS_EXTENDED; _xclass.iteratorObject = IteratorObject;

#define REVISION(REV) (_revision = INT_TO_JSVAL(REV));


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
	JSBool (*_free)(JSContext *cx, JSObject *obj) = NULL; \
	jsval _revision = JSVAL_VOID;

#define END_STATIC \
	JL_CHK(obj); \
	if ( GetHostPrivate(cx)->camelCase == 1 ) \
		_NormalizeFunctionSpecNames(_staticFunctionSpec); \
	if ( _staticFunctionSpec != NULL ) \
		JS_DefineFunctions(cx, obj, _staticFunctionSpec); \
	if ( _staticPropertySpec != NULL ) \
		JS_DefineProperties(cx, obj, _staticPropertySpec); \
	JSObject *dstObj; \
	dstObj = obj; \
	if ( _constIntegerSpec != NULL ) { \
		for ( ; _constIntegerSpec->name; _constIntegerSpec++ ) \
			JL_CHK( JS_DefineProperty(cx, dstObj, _constIntegerSpec->name, INT_TO_JSVAL(_constIntegerSpec->ival), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT) ); \
	} \
	if ( _staticPropertySpec != NULL ) \
		JL_CHK( JL_DefineClassProperties(cx, dstObj, _staticPropertySpec) ); \
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
	static JSExtendedClass _xclass = { { #CLASSNAME, 0, JS_PropertyStub , JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_EnumerateStub, JS_ResolveStub , JS_ConvertStub, JS_FinalizeStub, JSCLASS_NO_OPTIONAL_MEMBERS }, 0}; \
	JSClass *class##CLASSNAME = &_xclass.base; \
	static JSClass *_class = &_xclass.base; \
	JSObject *prototype##CLASSNAME = NULL; \
	static JSObject **_prototype = &prototype##CLASSNAME; \
	static JSBool _InitializeClass(JSContext *cx, JSObject *obj); \
	JSBool (*InitializeClass##CLASSNAME)(JSContext *cx, JSObject *obj) = _InitializeClass;

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
		JSBool (*_free)(JSContext *cx, JSObject *obj) = NULL; \
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
		JSBool found; \
		JL_CHK( JS_SetPropertyAttributes(cx, obj, _class->name, JSPROP_READONLY | JSPROP_PERMANENT, &found) ); \
		JL_CHKM( found, "Unable to set class flags." ); \
		if ( _constDoubleSpec != NULL ) \
			JL_CHK( JS_DefineConstDoubles(cx, dstObj, _constDoubleSpec) ); \
		JL_CHK( JS_DefinePropertyById(cx, dstObj, JLID(cx, _revision), _revision, NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT) ); \
		if ( _init ) \
			JL_CHK( _init(cx, dstObj) ); \
		return JS_TRUE; \
		JL_BAD; \
	}

#endif // _JSCLASS_H_
