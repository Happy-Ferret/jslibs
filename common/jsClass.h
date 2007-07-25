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

// functions
#define BEGIN_FUNCTION_SPEC JSFunctionSpec _tmp_fs[] = { // *name, call, nargs, flags, extra
#define END_FUNCTION_SPEC {0}}; _functionSpec = _tmp_fs;
#define BEGIN_STATIC_FUNCTION_SPEC JSFunctionSpec _tmp_sfs[] = {
#define END_STATIC_FUNCTION_SPEC {0}}; _staticFunctionSpec = _tmp_sfs;

#define DEFINE_FUNCTION(name) static JSBool name(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
#define DEFINE_CONSTRUCTOR() static JSBool Constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
#define DEFINE_OBJECT_CONSTRUCTOR() static JSBool ObjectConstructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
#define DEFINE_FINALIZE() static void Finalize(JSContext *cx, JSObject *obj)
#define DEFINE_CONVERT() static JSBool Convert(JSContext *cx, JSObject *obj, JSType type, jsval *vp)
#define DEFINE_RESOLVE() static JSBool Resolve(JSContext *cx, JSObject *obj, jsval id, uintN flags, JSObject **objp)

#define FUNCTION(name) { #name, name },
#define FUNCTION2(name,nativeName) { #name, nativeName },
#define FUNCTION_ARGC(name,nargs) { #name, name, nargs },

// properties
#define BEGIN_PROPERTY_SPEC JSPropertySpec _tmp_ps[] = { // *name, tinyid, flags, getter, setter
#define END_PROPERTY_SPEC {0}}; _propertySpec = _tmp_ps;
#define BEGIN_STATIC_PROPERTY_SPEC JSPropertySpec _tmp_sps[] = {
#define END_STATIC_PROPERTY_SPEC {0}}; _staticPropertySpec = _tmp_sps;

#define DEFINE_PROPERTY(name) static JSBool name(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
#define DEFINE_PROPERTY_NULL(name) static JSPropertyOp name = NULL;

#define PROPERTY(name)       { #name, 0, JSPROP_PERMANENT|JSPROP_SHARED, name##Getter, name##Setter },
#define PROPERTY_STORE(name) { #name, 0, JSPROP_PERMANENT              , name##Getter, name##Setter },
#define PROPERTY_READ(name)       { #name, 0, JSPROP_PERMANENT|JSPROP_READONLY|JSPROP_SHARED, name, NULL },
#define PROPERTY_READ_STORE(name) { #name, 0, JSPROP_PERMANENT|JSPROP_READONLY              , name, NULL },
#define PROPERTY_WRITE(name)       { #name, 0, JSPROP_PERMANENT|JSPROP_SHARED, NULL, name },
#define PROPERTY_WRITE_STORE(name) { #name, 0, JSPROP_PERMANENT              , NULL, name },
#define PROPERTY_SWITCH(name, function)       { #name, name, JSPROP_PERMANENT|JSPROP_SHARED, function##Getter, function##Setter }, // Used to define multiple properties with only one pari of getter/setter functions ( an enum has to be defiend ... less than 256 items ! )
#define PROPERTY_SWITCH_STORE(name, function) { #name, name, JSPROP_PERMANENT, function##Getter, function##Setter },
#define PROPERTY_SWITCH_READ_STORE(name, function) { #name, name, JSPROP_PERMANENT|JSPROP_READONLY, function##Getter, NULL },

#define PROPERTY_CREATE(name,id,flags,getter,setter) { #name, id, flags, getter, setter },
#define PROPERTY_DEFINE(name) { #name, 0, JSPROP_PERMANENT, NULL, NULL },

// add del get set
#define DEFINE_ADD_PROPERTY() static JSBool AddProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
#define DEFINE_DEL_PROPERTY() static JSBool DelProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
#define DEFINE_GET_PROPERTY() static JSBool GetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
#define DEFINE_SET_PROPERTY() static JSBool SetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)

// const double
#define BEGIN_CONST_DOUBLE_SPEC JSConstDoubleSpec _tmp_cds[] = { // dval; *name; flags; spare[3];
#define END_CONST_DOUBLE_SPEC {0}}; _constSpec = _tmp_cds;

#define CONST_DOUBLE(name,value) { value, #name },



// static definition
#define BEGIN_STATIC

#define CONFIGURE_STATIC \
	JSBool InitializeStatic(JSContext *cx, JSObject *obj) { \
	JSFunctionSpec *_staticFunctionSpec = NULL; \
	JSPropertySpec *_staticPropertySpec = NULL; \

#define END_STATIC \
	if ( _staticFunctionSpec != NULL ) JS_DefineFunctions(cx, obj, _staticFunctionSpec); \
	if ( _staticPropertySpec != NULL ) JS_DefineProperties(cx, obj, _staticPropertySpec); \
	return JS_TRUE; } \

#define DECLARE_STATIC() \
	JSBool InitializeStatic(JSContext *cx, JSObject *obj);

#define INIT_STATIC() \
	InitializeStatic(cx, obj); \



// class definition
#define DECLARE_CLASS( CLASSNAME ) \
	extern JSBool (*InitializeClass##CLASSNAME)(JSContext *cx, JSObject *obj); \
	extern JSClass class##CLASSNAME; \
	extern JSObject *prototype##CLASSNAME; \

#define INIT_CLASS( CLASSNAME ) \
	InitializeClass##CLASSNAME(cx, obj); \

#define BEGIN_CLASS(CLASSNAME) \
	static const char *_name = #CLASSNAME; \
	JSClass class##CLASSNAME = {0}; \
	static JSClass *_class = &class##CLASSNAME; \
	JSObject *prototype##CLASSNAME = NULL; \
	static JSObject **_prototype = &prototype##CLASSNAME; \
	static JSBool _InitializeClass(JSContext *cx, JSObject *obj); \
	JSBool (*InitializeClass##CLASSNAME)(JSContext *cx, JSObject *obj) = _InitializeClass; \

#define CONFIGURE_CLASS \
	static JSBool _InitializeClass(JSContext *cx, JSObject *obj) { \
		JSClass _tmp_class = { _name, 0, JS_PropertyStub , JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_EnumerateStub, JS_ResolveStub , JS_ConvertStub, JS_FinalizeStub, JSCLASS_NO_OPTIONAL_MEMBERS }; \
		*_class = _tmp_class; \
		JSNative _constructor = NULL; \
		JSFunctionSpec *_functionSpec = NULL, *_staticFunctionSpec = NULL; \
		JSPropertySpec *_propertySpec = NULL, *_staticPropertySpec = NULL; \
		JSConstDoubleSpec *_constSpec = NULL; \
		JSObject *_tmp_prototype = NULL; \
		JSObject **_parentPrototype = &_tmp_prototype; \

#define END_CLASS \
		*_prototype = JS_InitClass(cx, obj, *_parentPrototype, _class, _constructor, 0, _propertySpec, _functionSpec, _staticPropertySpec, _staticFunctionSpec); \
		if ( _constSpec != NULL ) if ( JS_DefineConstDoubles(cx, JS_GetConstructor(cx, *_prototype), _constSpec) == JS_FALSE ) return JS_FALSE; \
		return JS_TRUE; \
	} \

// class configuration
#define HAS_CONSTRUCTOR   _constructor = Constructor;
#define HAS_OBJECT_CONSTRUCTOR   _class->construct = ObjectConstructor;
#define CONSTRUCT_PROTOTYPE   _class->flags |= JSCLASS_CONSTRUCT_PROTOTYPE;
#define HAS_PRIVATE   _class->flags |= JSCLASS_HAS_PRIVATE;
#define SHARE_ALL_PROPERTIES   _class->flags |= JSCLASS_SHARE_ALL_PROPERTIES;
#define HAS_RESERVED_SLOTS(COUNT)   _class->flags |= JSCLASS_HAS_RESERVED_SLOTS(COUNT);
#define HAS_CALL   _class->call = Call;
#define HAS_FINALIZE   _class->finalize = Finalize;
#define HAS_CONVERT   _class->convert = Convert;
#define HAS_RESOLVE   _class->resolve = (JSResolveOp)Resolve;
#define HAS_PROTOTYPE(PROTOTYPE)   *_parentPrototype = (PROTOTYPE);
#define HAS_ADD_PROPERTY   _class->addProperty = AddProperty;
#define HAS_DEL_PROPERTY   _class->delProperty = DelProperty;
#define HAS_GET_PROPERTY   _class->getProperty = GetProperty;
#define HAS_SET_PROPERTY   _class->setProperty = SetProperty;

#endif _JSCLASS_H_