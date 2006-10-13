#ifndef _JSHELPER_H_
#define _JSHELPER_H_

#ifdef JSHELPER_UNSAFE_DEFINED
	extern bool _unsafeMode;
#else
	static bool _unsafeMode = false;
#endif

#define DEFINE_UNSAFE_MODE	extern bool _unsafeMode = false;

#define SET_UNSAFE_MODE(polarity) _unsafeMode = (polarity);

// RunTime helper macros
//   A set of very simple macro to help embeded spidermonkey do be more simple to read.

////////////////////////
// common error messages
#define RT_ERROR_NEED_CONSTRUCTION "construction is needed for this object."
#define RT_ERROR_MISSING_ARGUMENT "this function require more arguments."
#define RT_ERROR_MISSING_N_ARGUMENT "this function require %d more argument(s)."
#define RT_ERROR_INVALID_CLASS "wrong object type."
#define RT_ERROR_STRING_CONVERSION_FAILED "unable to convert this argument to string."
#define RT_ERROR_INT_CONVERSION_FAILED "unable to convert this argument to integer."
#define RT_ERROR_OUT_OF_MEMORY "not enough memory to complete the allocation."
#define RT_ERROR_NOT_INITIALIZED "the object is not proprely initialized."

////////////////
// helper macros

#define RT_SAFE(code) \
	if (!_unsafeMode) {code;}

#define RT_UNSAFE(code) \
	if (_unsafeMode) {code;}

/////////
// assert

#define RT_ASSERT_1( condition, errorMessage, arg ) \
	if ( !_unsafeMode && !(condition) ) { JS_ReportError( cx, (errorMessage), (arg) ); return JS_FALSE; }

#define RT_ASSERT( condition, errorMessage ) \
	if ( !_unsafeMode && !(condition) ) { JS_ReportError( cx, (errorMessage) ); return JS_FALSE; }

//////////////////
// advanced assert

#define RT_ASSERT_ALLOC(pointer) \
	RT_ASSERT( (pointer) != NULL, RT_ERROR_OUT_OF_MEMORY );

#define RT_ASSERT_CLASS(jsObject, jsClass) \
	RT_ASSERT( JS_GetClass(jsObject) == (jsClass), RT_ERROR_INVALID_CLASS );

#define RT_ASSERT_ARGC(minCount) \
	RT_ASSERT_1( argc >= (minCount), RT_ERROR_MISSING_N_ARGUMENT, (minCount)-argc );

#define RT_ASSERT_CONSTRUCTING(jsClass) \
	RT_ASSERT( JS_IsConstructing(cx) == JS_TRUE, RT_ERROR_NEED_CONSTRUCTION ); \
	RT_ASSERT_CLASS( obj, (jsClass) );

////////////////////
// conversion macros

#define RT_JSVAL_TO_INT32( jsvalInt, intVariable ) \
	if ( _unsafeMode ) { \
		intVariable = JSVAL_TO_INT(jsvalInt); \
	} else { \
		RT_ASSERT( JS_ValueToInt32( cx, jsvalInt, &intVariable ) != JS_FALSE, RT_ERROR_INT_CONVERSION_FAILED ); \
	}

#define RT_JSVAL_TO_STRING( jsvalString, stringVariable ) \
	stringVariable = JS_GetStringBytes( JS_ValueToString(cx,jsvalString) ); \
	RT_ASSERT( stringVariable != NULL, RT_ERROR_STRING_CONVERSION_FAILED );

#define RT_JSVAL_TO_STRING_AND_LENGTH( jsvalString, stringVariable, lengthVariable ) \
	{ \
		JSString *___jssTmp = JS_ValueToString(cx,jsvalString); \
		RT_ASSERT( ___jssTmp != NULL, RT_ERROR_STRING_CONVERSION_FAILED ); \
		stringVariable = JS_GetStringBytes( ___jssTmp ); \
		RT_ASSERT( stringVariable != NULL, RT_ERROR_STRING_CONVERSION_FAILED ); \
		lengthVariable = JS_GetStringLength( ___jssTmp ); \
	}



///////////////////////////////////////////////////////////
// This set of macros help js class creation
////////////////////////////////////////////
// symbols:
//   JSClass *thisClass
//   JSObject *thisClassObject
//   JSObject *prototype
//   JSNative Constructor
//   JSFinalizeOp Finalize
//   JSNative Call
////////////////////////////////////////////
//example:
//  BEGIN_CLASS
//    << define properties and functions here >>
//	 BEGIN_FUNCTION_MAP
//    FUNCTION(name) // need name()
//  END_MAP
//  BEGIN_PROPERTY_MAP
//    READONLY(name) // need nameGetter(), nameSetter()
//    READWRITE(name) // need name()
//    PROPERTY(name,id,flags,getter,setter)
//  END_MAP
//  BEGIN_STATIC_FUNCTION_MAP
//  END_MAP
//  BEGIN_STATIC_PROPERTY_MAP
//  END_MAP
//  NO_CONSTRUCTOR
//  NO_FINALIZE
//  NO_CALL
//  NO_PROTOTYPE
//  END_CLASS(test, NO_PRIVATE, NO_RESERVED_SLOT)

#define BEGIN_CLASS \
	static JSClass *thisClass; \
	static JSObject *thisClassObject=NULL;

#define NO_PRIVATE 0
#define HAS_PRIVATE JSCLASS_HAS_PRIVATE
#define NO_RESERVED_SLOT 0
#define NO_PROTOTYPE JSObject *prototype = NULL;
#define NO_CLASS_CONSTRUCT JSNative ClassConstruct = NULL;
#define NO_OBJECT_CONSTRUCT JSNative ObjectConstruct = NULL;
#define NO_FINALIZE JSFinalizeOp Finalize = JS_FinalizeStub;
#define NO_CALL JSNative Call = NULL;

#define END_MAP {0}};

#define BEGIN_FUNCTION_MAP JSFunctionSpec _functionMap[] = { // *name, call, nargs, flags, extra
#define BEGIN_STATIC_FUNCTION_MAP JSFunctionSpec _functionStaticMap[] = {
#define FUNCTION(name) { #name, name },

#define BEGIN_PROPERTY_MAP JSPropertySpec _propertyMap[] = { // *name, tinyid, flags, getter, setter
#define BEGIN_STATIC_PROPERTY_MAP JSPropertySpec _propertyStaticMap[] = {
#define READWRITE(name) { #name, 0, JSPROP_PERMANENT|JSPROP_SHARED, name##Getter, name##Setter },
#define READONLY(name) { #name, 0, JSPROP_PERMANENT|JSPROP_SHARED|JSPROP_READONLY, name, NULL },
#define PROPERTY(name,id,flags,getter,setter) { #name, id, flags, getter, setter },
#define END_CLASS(name,privateSlot,reservedSlotCount) \
	extern JSObject *classObject##name = thisClassObject; \
	extern JSClass class##name = { #name, JSCLASS_CONSTRUCT_PROTOTYPE | privateSlot | JSCLASS_HAS_RESERVED_SLOTS(reservedSlotCount), JS_PropertyStub , JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_EnumerateStub, JS_ResolveStub , JS_ConvertStub , Finalize, 0, 0, Call, ObjectConstruct }; \
	extern void InitClass##name(JSContext *cx, JSObject *obj) { \
		thisClass = &class##name; \
		thisClassObject = JS_InitClass( cx, obj, prototype, thisClass, ClassConstruct, 0, _propertyMap, _functionMap, _propertyStaticMap, _functionStaticMap ); \
	}

#define DECLARE_CLASS(name) \
	void InitClass##name( JSContext *cx, JSObject *obj ); \
	extern JSObject *classObject##name; \
	extern JSClass class##name;

#endif // _JSHELPER_H_
