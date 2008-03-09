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

#ifndef _JSHELPER_H_
#define _JSHELPER_H_

#ifdef WIN32
#include <windows.h>
#endif // WIN32

#ifdef _MSC_VER
#pragma warning(disable:4244 4305)  // for VC++, no precision loss complaints
#pragma warning(disable:4127)  // no "conditional expression is constant" complaints
#endif

//#include <stdbool.h>
#include <stdarg.h>


// buffer realloc policy:

inline bool MaybeRealloc( int requested, int received ) {

	return (100 * received / requested < 90) && (requested - received > 256);
}

// unsafe mode management

#ifdef USE_UNSAFE_MODE
	extern bool _unsafeMode;
#else
	static bool _unsafeMode = false; // by default, we are in SAFE mode
#endif

#define DEFINE_UNSAFE_MODE	bool _unsafeMode = false;
#define SET_UNSAFE_MODE(polarity) _unsafeMode = (polarity);

// RunTime helper macros
//   A set of very simple macro to help embeded spidermonkey do be more simple to read.

////////////////////////
// common error messages

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#ifdef _DEBUG
	#define RT_CODE_LOCATION " (in " __FILE__ ":" TOSTRING(__LINE__) ")"
#else // _DEBUG
	#define RT_CODE_LOCATION ""
#endif // _DEBUG

#define RT_ERROR_NO_CONSTRUCT "this object cannot be construct."
#define RT_ERROR_NEED_CONSTRUCTION "construction is needed for this object."
#define RT_ERROR_MISSING_ARGUMENT "this function require more arguments."
#define RT_ERROR_TOO_MANY_ARGUMENTS "you provide too many argument to the function."
#define RT_ERROR_MISSING_N_ARGUMENT "this function require %d more argument(s)."
#define RT_ERROR_INVALID_ARGUMENT "invalid argument."
#define RT_ERROR_INVALID_CLASS "wrong object type."
#define RT_ERROR_STRING_CONVERSION_FAILED "unable to convert this argument to string."
#define RT_ERROR_INT_CONVERSION_FAILED "unable to convert this argument to integer."
#define RT_ERROR_OUT_OF_MEMORY "not enough memory to complete the allocation."
#define RT_ERROR_NOT_INITIALIZED "the object or resource is not proprely initialized."
#define RT_ERROR_INVALID_RESOURCE "the resource is invalid or not proprely initialized."
#define RT_ERROR_CLASS_CREATION_FAILED "unable to create the class."
#define RT_ERROR_UNEXPECTED_TYPE "unexpected data type."
#define RT_ERROR_FUNCTION_EXPECTED "a function is expected."
#define RT_ERROR_INVALID_RANGE "value is out of range."

////////////////
// helper macros

#define RT_CHECK_CALL( functionCall ) \
	do { if ((functionCall) == JS_FALSE) { return JS_FALSE; } }  while(0)

#define RT_SAFE(code) \
	do { if (!_unsafeMode) {code;} } while(0)

#define RT_UNSAFE(code) \
	do { if (_unsafeMode) {code;} } while(0)

#define REPORT_WARNING(errorMessage) \
	do { JS_ReportWarning( cx, (errorMessage RT_CODE_LOCATION) ); } while(0)

#define REPORT_WARNING_1(errorMessage, arg) \
	do { JS_ReportWarning( cx, (errorMessage RT_CODE_LOCATION), (arg) ); } while(0)

#define REPORT_WARNING_2(errorMessage, arg1, arg2) \
	do { JS_ReportWarning( cx, (errorMessage RT_CODE_LOCATION), (arg1), (arg2) ); } while(0)

// fatal errors: script must stop as soon as possible
#define REPORT_ERROR(errorMessage) \
	do { JS_ReportError( cx, (errorMessage RT_CODE_LOCATION) ); return JS_FALSE; } while(0)

#define REPORT_ERROR_1(errorMessage, arg) \
	do { JS_ReportError( cx, (errorMessage RT_CODE_LOCATION), (arg) ); return JS_FALSE; } while(0)

#define REPORT_ERROR_2(errorMessage, arg1, arg2) \
	do { JS_ReportError( cx, (errorMessage RT_CODE_LOCATION), (arg1), (arg2) ); return JS_FALSE; } while(0)


/////////
// assert

#define RT_ASSERT( condition, errorMessage ) \
	{ if ( !_unsafeMode && !(condition) ) { JS_ReportError( cx, (errorMessage RT_CODE_LOCATION) ); return JS_FALSE; } }

#define RT_ASSERT_1( condition, errorMessage, arg ) \
	{ if ( !_unsafeMode && !(condition) ) { JS_ReportError( cx, (errorMessage RT_CODE_LOCATION), (arg) ); return JS_FALSE; } }

#define RT_ASSERT_2( condition, errorMessage, arg1, arg2 ) \
	{ if ( !_unsafeMode && !(condition) ) { JS_ReportError( cx, (errorMessage RT_CODE_LOCATION), (arg1), (arg2) ); return JS_FALSE; } }


//////////////////
// advanced assert

#define RT_ASSERT_TYPE(value,jsType) \
	RT_ASSERT( JS_TypeOfValue(cx, (value)) == (jsType), RT_ERROR_UNEXPECTED_TYPE );

#define RT_ASSERT_DEFINED(value) \
	RT_ASSERT( value != JSVAL_VOID, "Value is not defined." );

#define RT_ASSERT_OBJECT(value) \
	RT_ASSERT( value != JSVAL_NULL && JSVAL_IS_OBJECT(value), RT_ERROR_UNEXPECTED_TYPE " Object expected." );

#define RT_ASSERT_ARRAY(value) \
	RT_ASSERT( JSVAL_IS_OBJECT(value) && JS_IsArrayObject( cx, JSVAL_TO_OBJECT(value) ) == JS_TRUE, RT_ERROR_UNEXPECTED_TYPE " Array expected." );

#define RT_ASSERT_INT(value) \
	RT_ASSERT( JSVAL_IS_INT(value), RT_ERROR_UNEXPECTED_TYPE " Integer expected." );

#define RT_ASSERT_NUMBER(value) \
	RT_ASSERT( JSVAL_IS_NUMBER(value), RT_ERROR_UNEXPECTED_TYPE " Number expected." );

#define RT_ASSERT_STRING(value) \
	RT_ASSERT( JSVAL_IS_STRING(value), RT_ERROR_UNEXPECTED_TYPE " String expected." );

#define RT_ASSERT_FUNCTION(value) \
	RT_ASSERT( JS_TypeOfValue( cx, value ) == JSTYPE_FUNCTION, RT_ERROR_FUNCTION_EXPECTED );

#define RT_ASSERT_ALLOC(pointer) \
	RT_ASSERT( (pointer) != NULL, RT_ERROR_OUT_OF_MEMORY );

#define RT_ASSERT_RESOURCE(resourcePointer) \
	RT_ASSERT( (resourcePointer) != NULL, RT_ERROR_INVALID_RESOURCE );

#define RT_ASSERT_CLASS(jsObject, jsClass) \
	RT_ASSERT( ((jsObject) != NULL) && (JS_GET_CLASS(cx, jsObject) == (jsClass)), RT_ERROR_INVALID_CLASS );

#define RT_ASSERT_THIS_CLASS() \
	RT_ASSERT( ((obj) != NULL) && (JS_GET_CLASS(cx, obj) == (_class)), RT_ERROR_INVALID_CLASS );

#define RT_ASSERT_CLASS_NAME(jsObject, className) \
	RT_ASSERT( strcmp(JS_GET_CLASS(cx, jsObject)->name, (className)) == 0,  RT_ERROR_INVALID_CLASS " Expecting " className "." );

#define RT_ASSERT_ARGC(minCount) \
	RT_ASSERT_1( argc >= (minCount), RT_ERROR_MISSING_N_ARGUMENT, (minCount)-argc );

#define RT_ASSERT_ARGC_MAX(maxCount) \
	RT_ASSERT( argc <= (maxCount), RT_ERROR_TOO_MANY_ARGUMENTS );

#define RT_ASSERT_CONSTRUCTING(jsClass) { \
	RT_ASSERT( JS_IsConstructing(cx) == JS_TRUE, RT_ERROR_NEED_CONSTRUCTION ); \
	RT_ASSERT_CLASS( obj, (jsClass) ); \
}

// new namespace for jslibs: J_

#define J_ARGC (argc)

// returns the ARGument n or undefined if it does not exist
#define J_ARG( n ) (argv[(n)-1])
// same for fast native (cf. http://developer.mozilla.org/en/docs/JS_ARGV)
#define J_FARG( n ) (JS_ARGV(cx,vp)[(n)-1])


// returns the ARGument n or undefined if it does not exist
#define J_SARG( n ) ( argc >= (n) ? argv[(n)-1] : JSVAL_VOID )
// same for fast native (cf. http://developer.mozilla.org/en/docs/JS_ARGV)
#define J_FSARG( n ) ( argc >= (n) ? JS_ARGV(cx,vp)[(n)-1] : JSVAL_VOID )


// returns true if the ARGument n is DEFined
#define J_ARG_ISDEF( n ) ( argc >= (n) && argv[(n)-1] != JSVAL_VOID )
// same for fast native
#define J_FARG_ISDEF( n ) ( argc >= (n) && JS_ARGV(cx,vp)[(n)-1] != JSVAL_VOID )

// is the current obj (this)
#define J_OBJ (obj)
// same for fast native
#define J_FOBJ JS_THIS_OBJECT(cx, vp)

// the return value
#define J_RVAL (rval)
// same for fast native
#define J_FRVAL (&JS_RVAL(cx, vp))


////////////////////
// conversion macros

#define RT_JSVAL_TO_BOOL( jsval, boolVariable ) do { \
	JSBool b; \
	JSBool st = JS_ValueToBoolean( cx, jsval, &b ); \
	RT_ASSERT( st != JS_FALSE, RT_ERROR_INT_CONVERSION_FAILED ); \
	boolVariable = (b == JS_TRUE); \
} while(0)

#define RT_JSVAL_TO_REAL( jsval, floatVariable ) do { \
	jsdouble d; \
	JSBool st = JS_ValueToNumber( cx, jsval, &d ); \
	RT_ASSERT( st != JS_FALSE, RT_ERROR_INT_CONVERSION_FAILED ); \
	floatVariable = d; \
} while(0)

#define RT_JSVAL_TO_INT32( jsvalInt, intVariable ) do { \
	int32 intVal; \
	JSBool st = JS_ValueToInt32( cx, jsvalInt, &intVal ); \
	RT_ASSERT( st != JS_FALSE, RT_ERROR_INT_CONVERSION_FAILED ); \
	intVariable = intVal; \
} while(0)

#define RT_JSVAL_TO_UINT32( jsvalUInt, uintVariable ) do { \
	jsdouble __tmp; \
	if ( JSVAL_IS_INT(jsvalUInt) && JSVAL_TO_INT(jsvalUInt) >= 0 ) { \
		uintVariable = JSVAL_TO_INT(jsvalUInt); \
	} else { \
		JSBool st = JS_ValueToNumber(cx, jsvalUInt, &__tmp ); \
		RT_ASSERT( st != JS_FALSE, RT_ERROR_INT_CONVERSION_FAILED ); \
		uintVariable = (unsigned long)__tmp; \
		RT_ASSERT( __tmp == (double)((unsigned long)__tmp), RT_ERROR_INT_CONVERSION_FAILED ); \
	} \
} while(0)

#define RT_JSVAL_TO_STRING( jsvalString, stringVariable ) do { \
	JSString *___jssTmp = JS_ValueToString(cx, (jsvalString)); \
	RT_ASSERT( ___jssTmp != NULL, RT_ERROR_STRING_CONVERSION_FAILED ); \
	(stringVariable) = JS_GetStringBytes( ___jssTmp ); \
	RT_ASSERT( (stringVariable) != NULL, RT_ERROR_STRING_CONVERSION_FAILED ); \
} while(0)

#define RT_JSVAL_TO_STRING_AND_LENGTH( jsvalString, stringVariable, lengthVariable ) do { \
	JSString *___jssTmp = JS_ValueToString(cx,(jsvalString)); \
	RT_ASSERT( ___jssTmp != NULL, RT_ERROR_STRING_CONVERSION_FAILED ); \
	(stringVariable) = JS_GetStringBytes( ___jssTmp ); \
	RT_ASSERT( (stringVariable) != NULL, RT_ERROR_STRING_CONVERSION_FAILED ); \
	(lengthVariable) = JS_GetStringLength( ___jssTmp ); \
} while(0)


///////////


inline double TimeNow() {

#ifdef WIN32
	LARGE_INTEGER frequency, performanceCount;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&performanceCount);
	return 1000 * double(performanceCount.QuadPart) / double(frequency.QuadPart);
#endif // WIN32
	return 0; // (TBD) impl.
}


typedef void (*FunctionPointer)(void);


inline JSBool SetNativeInterface( JSContext *cx, JSObject *obj, const char *name, FunctionPointer function, void *descriptor ) {
	
	// Cannot be called while Finalize
	// the following must work because spidermonkey will never call the getter or setter if it is not explicitly required by the script
	RT_CHECK_CALL( JS_DefineProperty(cx, obj, name, JSVAL_VOID, (JSPropertyOp)function, (JSPropertyOp)descriptor, JSPROP_READONLY | JSPROP_PERMANENT) );
	return JS_TRUE;
}

inline JSBool GetNativeInterface( JSContext *cx, JSObject *obj, const char *name, FunctionPointer *function, void **descriptor ) {

	// Cannot be called while Finalize
	uintN attrs;
	JSBool found;
	RT_CHECK_CALL( JS_GetPropertyAttrsGetterAndSetter(cx, obj, name, &attrs, &found, (JSPropertyOp*)function, (JSPropertyOp*)descriptor) ); // NULL is supported for function and descriptor
	return JS_TRUE;
}


inline JSBool RemoveNativeInterface( JSContext *cx, JSObject *obj, const char *name ) {
	
	// Cannot be called while Finalize
	RT_CHECK_CALL( JS_DeleteProperty(cx, obj, name) );
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
		if ( JS_GET_CLASS(cx, obj) == clasp )
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

inline bool HasProperty( JSContext *cx, JSObject *obj, const char *propertyName ) {

	uintN attr;
	JSBool found;
	JSBool status = JS_GetPropertyAttributes(cx, obj, propertyName, &attr, &found);
	return ( status == JS_TRUE && found != JS_FALSE );
}

inline JSBool CallFunction( JSContext *cx, JSObject *obj, jsval functionValue, jsval *rval, uintN argc, ... ) {

	va_list ap;
	jsval argv[16]; // argc MUST be <= 16
	jsval rvalTmp;
	RT_ASSERT( argc <= sizeof(argv)/sizeof(jsval), "Too many arguments." );
	va_start(ap, argc);
	for ( uintN i = 0; i < argc; i++ )
		argv[i] = va_arg(ap, jsval);
	va_end(ap);
	RT_ASSERT_FUNCTION( functionValue );
	RT_CHECK_CALL( JS_CallFunctionValue(cx, obj, functionValue, argc, argv, &rvalTmp) ); // NULL is NOT supported for &rvalTmp ( last arg of JS_CallFunctionValue )
	if ( rval != NULL )
		*rval = rvalTmp;
	return JS_TRUE;
}

#endif // _JSHELPER_H_
