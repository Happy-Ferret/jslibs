
// maps
//#define END_MAP {0}};
//#define BEGIN_FUNCTION_SPEC static JSFunctionSpec _functionSpec[] = { // *name, call, nargs, flags, extra
//#define BEGIN_STATIC_FUNCTION_SPEC static JSFunctionSpec _functionStaticSpec[] = {
//#define BEGIN_PROPERTY_SPEC static JSPropertySpec _propertySpec[] = { // *name, tinyid, flags, getter, setter
//#define BEGIN_STATIC_PROPERTY_SPEC static JSPropertySpec _propertyStaticSpec[] = {
//#define BEGIN_CONSTANT_SPEC JSConstDoubleSpec _constantSpec[] = { // dval; *name; flags; spare[3];


#define BEGIN_FUNCTION_SPEC { JSFunctionSpec _tmp[] = {
#define END_FUNCTION_SPEC {0}}; _functionSpec = _tmp; }
#define BEGIN_STATIC_FUNCTION_SPEC { JSFunctionSpec _tmp[] = {
#define END_STATIC_FUNCTION_SPEC {0}}; _staticFunctionSpec = _tmp; }

#define FUNCTION(name) { #name, name },
#define FUNCTION2(name,nativeName) { #name, nativeName },
#define FUNCTION_ARGC(name,nargs) { #name, name, nargs },


#define BEGIN_PROPERTY_SPEC { JSPropertySpec _tmp[] = {
#define END_PROPERTY_SPEC {0}}; _propertySpec = _tmp; }
#define BEGIN_STATIC_PROPERTY_SPEC { JSPropertySpec _tmp[] = {
#define END_STATIC_PROPERTY_SPEC {0}}; _staticPropertySpec = _tmp; }

// _S sufix means 'store' ( no JSPROP_SHARED attribute used )
#define PROPERTY(name)   { #name, 0, JSPROP_PERMANENT|JSPROP_SHARED, name##Getter, name##Setter },
#define PROPERTY_S(name) { #name, 0, JSPROP_PERMANENT              , name##Getter, name##Setter },

#define PROPERTY_READ(name)   { #name, 0, JSPROP_PERMANENT|JSPROP_READONLY|JSPROP_SHARED, name, NULL },
#define PROPERTY_READ_S(name) { #name, 0, JSPROP_PERMANENT|JSPROP_READONLY              , name, NULL },

#define PROPERTY_WRITE(name)   { #name, 0, JSPROP_PERMANENT|JSPROP_SHARED, NULL, name },
#define PROPERTY_WRITE_S(name) { #name, 0, JSPROP_PERMANENT              , NULL, name },

#define PROPERTY_SWITCH(name, function) { #name, name, JSPROP_PERMANENT|JSPROP_SHARED, function##Getter, function##Setter },

// Used to define multiple properties with only one pari of getter/setter functions ( an enum has to be defiend ... less than 256 items ! )
#define PROPERTY_CREATE(name,id,flags,getter,setter) { #name, id, flags, getter, setter },
#define PROPERTY_DEFINE(name) { #name, 0, JSPROP_PERMANENT, NULL, NULL },


#define BEGIN_CONST_DOUBLE_SPEC { JSConstDoubleSpec _tmp[] = {
#define END_CONST_DOUBLE_SPEC {0}}; _constSpec = _tmp; }

#define CONST_DOUBLE(name,value) { value, #name },


#define DECLARE_CLASS( CLASSNAME ) \
	extern JSBool (*InitializeClass##CLASSNAME)(JSContext *cx, JSObject *obj); \
	
#define INIT_CLASS( CLASSNAME ) \
	InitializeClass##CLASSNAME(cx, obj); \

#define BEGIN_CLASS(CLASSNAME) \
	static const char *_name = #CLASSNAME; \
	extern JSClass class##CLASSNAME = {0}; \
	static JSClass *_class = &class##CLASSNAME; \
	extern JSObject *proto##CLASSNAME = NULL; \
	static JSObject **_proto = &proto##CLASSNAME; \
	static JSBool _InitializeClass(JSContext *cx, JSObject *obj); \
	extern JSBool (*InitializeClass##CLASSNAME)(JSContext *cx, JSObject *obj) = _InitializeClass; \


#define INIT \
	static JSBool _InitializeClass(JSContext *cx, JSObject *obj) { \
		{ JSClass _tmp = { _name, 0, JS_PropertyStub , JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_EnumerateStub, JS_ResolveStub , JS_ConvertStub, JS_FinalizeStub }; \
		*_class = _tmp; }\
		JSNative _constructor = NULL; \
		JSFunctionSpec *_functionSpec = NULL, *_staticFunctionSpec = NULL; \
		JSPropertySpec *_propertySpec = NULL, *_staticPropertySpec = NULL; \
		JSConstDoubleSpec *_constSpec = NULL; \
		JSObject *_protoTmp = NULL; \
		JSObject **_parentProto = &_protoTmp; \

#define END_CLASS \
		*_proto = JS_InitClass(cx, obj, *_parentProto, _class, _constructor, 0, _propertySpec, _functionSpec, _staticPropertySpec, _staticFunctionSpec); \
		if ( _constSpec != NULL ) if ( JS_DefineConstDoubles( cx, *_proto, _constSpec ) == JS_FALSE ) return JS_FALSE; \
		return JS_TRUE; } \

#define HAS_PRIVATE _class->flags |= JSCLASS_HAS_PRIVATE;
#define HAS_RESERVED_SLOTS( COUNT ) _class->flags |= JSCLASS_HAS_RESERVED_SLOTS(COUNT);
#define HAS_CALL _class->call = Call;
#define HAS_CONSTRUCTOR _constructor = Constructor;
#define HAS_OBJECT_CONSTRUCT _class->construct = ObjectConstruct;
#define HAS_FINALIZE _class->finalize = Finalize;
#define PROTOTYPE( PROTO ) *_parentProto = (PROTO);
#define CONSTRUCT_PROTO _class->flags |= JSCLASS_CONSTRUCT_PROTOTYPE;






