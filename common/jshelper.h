#pragma once

#ifdef JSHELPER_UNSAFE_DEFINED
extern bool _unsafeMode;
#else //JSHELPER_UNSAFE_DEFINED
static bool _unsafeMode = false;
#endif //JSHELPER_UNSAFE_DEFINED


#define DEFINE_UNSAFE_MODE	extern bool _unsafeMode = false;

#define SET_UNSAFE_MODE(polarity) _unsafeMode = polarity;


// RunTime helper macros
//   A set of very simple macro to help embeded spidermonkey do be more simple to read.

// common error messages
#define RT_ERROR_NEED_CONSTRUCTION "construction is needed for this object."
#define RT_ERROR_MISSING_ARGUMENT "this function require more arguments."
#define RT_ERROR_MISSING_N_ARGUMENT "this function require %d more argument(s)."
#define RT_ERROR_INVALID_CLASS "wrong object type."
#define RT_ERROR_STRING_CONVERSION_FAILED "unable to convert this argument to string."
#define RT_ERROR_INT_CONVERSION_FAILED "unable to convert this argument to integer."
#define RT_ERROR_OUT_OF_MEMORY "not enough memory to complete the allocation."
#define RT_ERROR_NOT_INITIALIZED "the object is not proprely initialized."

// helper macros

#define RT_SAFE(code) \
	if (!_unsafeMode) {code;}

#define RT_UNSAFE(code) \
	if (_unsafeMode) {code;}

// assert

#define RT_ASSERT_1( condition, errorMessage, arg ) \
	if ( !_unsafeMode && !(condition) ) { JS_ReportError( cx, (errorMessage), (arg) ); return JS_FALSE; }

#define RT_ASSERT( condition, errorMessage ) \
	if ( !_unsafeMode && !(condition) ) { JS_ReportError( cx, (errorMessage) ); return JS_FALSE; }

// advanced assert

#define RT_ASSERT_ALLOC( pointer ) \
	RT_ASSERT( (pointer) != NULL, RT_ERROR_OUT_OF_MEMORY );

#define RT_ASSERT_CLASS( jsObject, jsClass ) \
	RT_ASSERT( JS_GetClass(jsObject) == (jsClass), RT_ERROR_INVALID_CLASS );

#define RT_ASSERT_ARGC( minCount ) \
	RT_ASSERT_1( argc >= (minCount), RT_ERROR_MISSING_N_ARGUMENT, (minCount)-argc );

#define RT_ASSERT_CONSTRUCTING(jsClass) \
	RT_ASSERT( JS_IsConstructing(cx) == JS_TRUE, RT_ERROR_NEED_CONSTRUCTION ); \
	RT_ASSERT_CLASS( obj, (jsClass) );

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
		JSString *__jssTmp = JS_ValueToString(cx,jsvalString); \
		RT_ASSERT( __jssTmp != NULL, RT_ERROR_STRING_CONVERSION_FAILED ); \
		stringVariable = JS_GetStringBytes( __jssTmp ); \
		RT_ASSERT( stringVariable != NULL, RT_ERROR_STRING_CONVERSION_FAILED ); \
		lengthVariable = JS_GetStringLength( __jssTmp ); \
	}

