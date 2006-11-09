#ifndef _JSHELPER_H_
#define _JSHELPER_H_
#ifdef WIN32
#include <windows.h>
#endif // WIN32


#ifdef JSHELPER_UNSAFE_DEFINED
	extern bool _unsafeMode;
#else
	static bool _unsafeMode = false; // by default, we are in SAFE mode
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
#define RT_ERROR_NOT_INITIALIZED "the object or resource is not proprely initialized."
#define RT_ERROR_INVALID_RESOURCE "the resource is invalid or not proprely initialized."
#define RT_ERROR_CLASS_CREATION_FAILED "unable to create the class."
#define RT_ERROR_UNEXPECTED_TYPE "unexpected data type."

////////////////
// helper macros

#define RT_SAFE(code) \
	if (!_unsafeMode) {code;}

#define RT_UNSAFE(code) \
	if (_unsafeMode) {code;}

#define REPORT_ERROR(errorMessage) \
	JS_ReportError( cx, (errorMessage) ); return JS_FALSE;

/////////
// assert

#define RT_ASSERT_2( condition, errorMessage, arg1, arg2 ) \
	if ( !_unsafeMode && !(condition) ) { JS_ReportError( cx, (errorMessage), (arg1), (arg2) ); return JS_FALSE; }

#define RT_ASSERT_1( condition, errorMessage, arg ) \
	if ( !_unsafeMode && !(condition) ) { JS_ReportError( cx, (errorMessage), (arg) ); return JS_FALSE; }

#define RT_ASSERT( condition, errorMessage ) \
	if ( !_unsafeMode && !(condition) ) { JS_ReportError( cx, (errorMessage) ); return JS_FALSE; }

//////////////////
// advanced assert

#define RT_ASSERT_TYPE(value,jsType) \
	RT_ASSERT( JS_TypeOfValue(cx, (value)) == (jsType), RT_ERROR_UNEXPECTED_TYPE );

#define RT_ASSERT_ALLOC(pointer) \
	RT_ASSERT( (pointer) != NULL, RT_ERROR_OUT_OF_MEMORY );

#define RT_ASSERT_RESOURCE(resourcePointer) \
	RT_ASSERT( (resourcePointer) != NULL, RT_ERROR_INVALID_RESOURCE );

#define RT_ASSERT_CLASS(jsObject, jsClass) \
	RT_ASSERT( JS_GetClass(jsObject) == (jsClass), RT_ERROR_INVALID_CLASS );

#define RT_ASSERT_CLASS_NAME(jsObject, className) \
	RT_ASSERT( strcmp(JS_GetClass(jsObject)->name, (className)) == 0,  RT_ERROR_INVALID_CLASS );

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
/*
	BEGIN_CLASS
	<< define properties and functions here >>
	BEGIN_FUNCTION_MAP
		FUNCTION(name) // need name()
	END_MAP
	BEGIN_PROPERTY_MAP
		READONLY(name) // need nameGetter(), nameSetter()
		READWRITE(name) // need name()
		PROPERTY(name,id,flags,getter,setter)
		CONSTANT(func, name, value)
	END_MAP
	BEGIN_STATIC_FUNCTION_MAP
	END_MAP
	BEGIN_STATIC_PROPERTY_MAP
	END_MAP
	BEGIN_CONSTANT_MAP
		CONSTANT_DOUBLE( toto, 123 )
	END_MAP
	NO_CONSTRUCTOR
	NO_FINALIZE
	NO_CALL
	NO_PROTOTYPE
	END_CLASS(test, NO_PRIVATE, NO_RESERVED_SLOT)
*/
//

/* comment
 *
 *
 */
#define BEGIN_CLASS \
	static JSClass *thisClass; \
	static JSObject *thisClassObject=NULL;

#define NO_PRIVATE 0
#define HAS_PRIVATE JSCLASS_HAS_PRIVATE
#define NO_RESERVED_SLOT 0
#define NO_PROTOTYPE static JSObject *prototype = NULL;
#define NO_CLASS_CONSTRUCT static JSNative ClassConstruct = NULL;
#define NO_OBJECT_CONSTRUCT static JSNative ObjectConstruct = NULL;
#define NO_FINALIZE static JSFinalizeOp Finalize = JS_FinalizeStub;
#define NO_CALL static JSNative Call = NULL;
#define NO_INITCLASSAUX static void InitClassAux(JSContext *cx, JSObject *obj) {};

#define NO_FUNCTION_MAP static JSFunctionSpec *_functionMap = NULL;
#define NO_STATIC_FUNCTION_MAP static JSFunctionSpec *_functionStaticMap = NULL;
#define NO_PROPERTY_MAP static JSPropertySpec *_propertyMap = NULL;
#define NO_STATIC_PROPERTY_MAP static JSPropertySpec *_propertyStaticMap = NULL;
#define NO_CONSTANT_MAP static JSConstDoubleSpec *_constantMap = NULL;

// maps
#define END_MAP {0}};
#define BEGIN_FUNCTION_MAP static JSFunctionSpec _functionMap[] = { // *name, call, nargs, flags, extra
#define BEGIN_STATIC_FUNCTION_MAP static JSFunctionSpec _functionStaticMap[] = {
#define BEGIN_PROPERTY_MAP static JSPropertySpec _propertyMap[] = { // *name, tinyid, flags, getter, setter
#define BEGIN_STATIC_PROPERTY_MAP static JSPropertySpec _propertyStaticMap[] = {
#define BEGIN_CONSTANT_MAP JSConstDoubleSpec _constantMap[] = { // dval; *name; flags; spare[3];

//
#define DEFINE_FINALIZE() static void Finalize(JSContext *cx, JSObject *obj)
#define DEFINE_FUNCTION(name) static JSBool name##(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
#define DEFINE_PROPERTY(name) static JSBool name##(JSContext *cx, JSObject *obj, jsval id, jsval *vp)

#define BIND_STATIC_FUNCTIONS(obj) \
	JS_DefineFunctions(cx, obj, _functionStaticMap);

#define BIND_STATIC_PROPERTIES(obj) \
	JS_DefineProperties(cx, obj, _propertyStaticMap);


// Declares a function for the FUNCTION_MAP
#define FUNCTION(name) { #name, name },
#define FUNCTION_ARGC(name,nargs) { #name, name, nargs },
// Allows to specify the native name too
#define FUNCTION_ALIAS(name,nativeName) { #name, nativeName },
// Declares a read/write property. Yous muse define xxxxGetter and xxxxSetter native functions
#define READWRITE(name) { #name, 0, JSPROP_PERMANENT|JSPROP_SHARED, name##Getter, name##Setter },
// Need setter function to be defined. The resulting value is stored in the object. When the prop is read (get) the stored value is used
#define SET_AND_STORE(name) { #name, 0, JSPROP_PERMANENT, NULL, name },
// Declares a read-only property
#define READONLY(name) { #name, 0, JSPROP_PERMANENT|JSPROP_SHARED|JSPROP_READONLY, name, NULL },
// Allows a full definition of a property
#define PROPERTY(name,id,flags,getter,setter) { #name, id, flags, getter, setter },
// use in PROPERTY_MAP only. 'func' centralize all values. WARNING: 0 <= value <= 255
#define CONSTANT(func, name, value) { #name, value, JSPROP_PERMANENT|JSPROP_READONLY, func, NULL },
// just defines a property
#define CREATE(name) { #name, 0, JSPROP_PERMANENT|JSPROP_READONLY, NULL, NULL },
// use in CONSTANT_MAP only
#define CONSTANT_DOUBLE(name,value) { value, #name },

#define END_CLASS(name,privateSlot,reservedSlotCount) \
	extern JSObject *classObject##name = thisClassObject; \
	extern JSClass class##name = { #name, /*JSCLASS_CONSTRUCT_PROTOTYPE | */privateSlot | JSCLASS_HAS_RESERVED_SLOTS(reservedSlotCount), JS_PropertyStub , JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_EnumerateStub, JS_ResolveStub , JS_ConvertStub , Finalize, 0, 0, Call, ObjectConstruct }; \
	extern JSBool InitClass##name(JSContext *cx, JSObject *obj) { \
		thisClass = &class##name; \
		classObject##name = thisClassObject = JS_InitClass( cx, obj, prototype, thisClass, ClassConstruct, 0, _propertyMap, _functionMap, _propertyStaticMap, _functionStaticMap ); \
		if ( _constantMap != NULL) \
			JS_DefineConstDoubles( cx, thisClassObject, _constantMap ); \
		InitClassAux(cx, obj); \
		return thisClassObject != NULL; \
	}

#define DECLARE_CLASS(name) \
	JSBool InitClass##name( JSContext *cx, JSObject *obj ); \
	extern JSObject *classObject##name; \
	extern JSClass class##name;

#define INIT_CLASS(name) { \
	JSBool status = InitClass##name(cx, obj); \
	RT_ASSERT( status == JS_TRUE, RT_ERROR_CLASS_CREATION_FAILED " (" #name ")" ); \
}

///////////

inline double TimeNow() {
	
#ifdef WIN32
	LARGE_INTEGER frequency, performanceCount;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&performanceCount);
	return 1000 * double(performanceCount.QuadPart) / double(frequency.QuadPart);
#endif // WIN32
}


inline JSBool SetNativeInterface( JSContext *cx, JSObject *obj, const char *name, void *function, void *descriptor ) {

	// the following must work because spidermonkey will never call the getter or setter if it is not explicitly required by the script
	if ( JS_DefineProperty(cx, obj, name, JSVAL_VOID, (JSPropertyOp)function, (JSPropertyOp)descriptor, JSPROP_READONLY | JSPROP_PERMANENT) == JS_FALSE )
		return JS_FALSE;
	return JS_TRUE;
}

inline JSBool GetNativeInterface( JSContext *cx, JSObject *obj, const char *name, void **function, void **descriptor ) {

	uintN attrs;
	JSBool found;
	if ( JS_GetPropertyAttrsGetterAndSetter(cx, obj, name, &attrs, &found, (JSPropertyOp*)function, (JSPropertyOp*)descriptor) == JS_FALSE )
		return JS_FALSE;
	return JS_TRUE;
}

////

inline bool IsInstanceOf( JSContext *cx, JSObject *obj, JSClass *clasp ) {

	while( obj != NULL ) {

		obj = JS_GetPrototype(cx, obj);
		if ( JS_GetClass(obj) == clasp )
			return true;
	}
	return false;
}


inline JSBool GetNamedPrivate( JSContext *cx, JSObject *obj, const char *name, void **pv ) {

	jsval tmp;
	if ( JS_GetProperty(cx, obj, name, &tmp) == JS_FALSE )
		return JS_FALSE;
	*pv = tmp == JSVAL_VOID ? NULL : JSVAL_TO_PRIVATE(tmp);
	return JS_TRUE;
}


inline JSBool SetNamedPrivate( JSContext *cx, JSObject *obj, const char *name, const void *pv ) {

//	RT_SAFE(	if ( (int)pv % 2 ) return JS_FALSE; ); // check if *vp is 2-byte aligned
	if ( JS_DefineProperty(cx, obj, name, PRIVATE_TO_JSVAL(pv), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT ) == JS_FALSE )
		return JS_FALSE;
	return JS_TRUE;
}


inline JSBool GetIntProperty( JSContext *cx, JSObject *obj, const char *propertyName, int *value ) {

	jsval tmp;
	int32 int32Value;
	if ( JS_GetProperty(cx, obj, propertyName, &tmp) == JS_FALSE )
		return JS_FALSE;
	JS_ValueToInt32(cx, tmp, &int32Value);
	*value = int32Value;
	return JS_TRUE;
}




#endif // _JSHELPER_H_
