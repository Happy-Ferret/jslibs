#ifndef _JSHELPER_H_
#define _JSHELPER_H_

#ifdef WIN32
#include <windows.h>
#endif // WIN32

#ifdef _MSC_VER
#pragma warning(disable:4244 4305)  // for VC++, no precision loss complaints
#endif

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

#define RT_ASSERT_RETURN( functionCall ) \
	if ((functionCall) == JS_FALSE) { return JS_FALSE; }

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

#define RT_ASSERT_DEFINED(value) \
	RT_ASSERT( value != JSVAL_VOID, "Value is not defined." );

#define RT_ASSERT_OBJECT(value) \
	RT_ASSERT( JSVAL_IS_OBJECT(value), RT_ERROR_UNEXPECTED_TYPE " Object expected." );

#define RT_ASSERT_INT(value) \
	RT_ASSERT( JSVAL_IS_INT(value), RT_ERROR_UNEXPECTED_TYPE " Integer expected." );

#define RT_ASSERT_NUMBER(value) \
	RT_ASSERT( JSVAL_IS_NUMBER(value), RT_ERROR_UNEXPECTED_TYPE " Number expected." );

#define RT_ASSERT_ALLOC(pointer) \
	RT_ASSERT( (pointer) != NULL, RT_ERROR_OUT_OF_MEMORY );

#define RT_ASSERT_RESOURCE(resourcePointer) \
	RT_ASSERT( (resourcePointer) != NULL, RT_ERROR_INVALID_RESOURCE );

#define RT_ASSERT_CLASS(jsObject, jsClass) \
	RT_ASSERT( JS_GetClass(jsObject) == (jsClass), RT_ERROR_INVALID_CLASS );

#define RT_ASSERT_CLASS_NAME(jsObject, className) \
	RT_ASSERT( strcmp(JS_GetClass(jsObject)->name, (className)) == 0,  RT_ERROR_INVALID_CLASS " Expecting " className "." );

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

//inline const char *JsvalToString(cx, jsval) {
//}

#define RT_JSVAL_TO_STRING_AND_LENGTH( jsvalString, stringVariable, lengthVariable ) \
	{ \
		JSString *___jssTmp = JS_ValueToString(cx,jsvalString); \
		RT_ASSERT( ___jssTmp != NULL, RT_ERROR_STRING_CONVERSION_FAILED ); \
		stringVariable = JS_GetStringBytes( ___jssTmp ); \
		RT_ASSERT( stringVariable != NULL, RT_ERROR_STRING_CONVERSION_FAILED ); \
		lengthVariable = JS_GetStringLength( ___jssTmp ); \
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


typedef void (*FunctionPointer)(void);

inline JSBool SetNativeInterface( JSContext *cx, JSObject *obj, const char *name, FunctionPointer function, void *descriptor ) {

	// the following must work because spidermonkey will never call the getter or setter if it is not explicitly required by the script
	if ( JS_DefineProperty(cx, obj, name, JSVAL_VOID, (JSPropertyOp)function, (JSPropertyOp)descriptor, JSPROP_READONLY | JSPROP_PERMANENT) == JS_FALSE )
		return JS_FALSE;
	return JS_TRUE;
}

inline JSBool GetNativeInterface( JSContext *cx, JSObject *obj, const char *name, FunctionPointer *function, void **descriptor ) {

	uintN attrs;
	JSBool found;
	if ( JS_GetPropertyAttrsGetterAndSetter(cx, obj, name, &attrs, &found, (JSPropertyOp*)function, (JSPropertyOp*)descriptor) == JS_FALSE )
		return JS_FALSE;
	return JS_TRUE;
}

////

 
inline bool IsPInfinity( JSContext *cx, jsval val ) {
	
	return JS_GetPositiveInfinityValue(cx) == val;
}

inline bool IsNInfinity( JSContext *cx, jsval val ) {
	
	return JS_GetNegativeInfinityValue(cx) == val;
}


inline bool InheritFrom( JSContext *cx, JSObject *obj, JSClass *clasp ) {

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
