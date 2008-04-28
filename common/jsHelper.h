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

#include "platform.h"

#include <stdarg.h>

#include <jsarena.h>
#include <jsfun.h>

// unsafe mode management

#ifdef USE_UNSAFE_MODE
	extern bool _unsafeMode;
#else
	static bool _unsafeMode = false; // by default, we are in SAFE mode
#endif // USE_UNSAFE_MODE

#define DEFINE_UNSAFE_MODE	bool _unsafeMode = false;

#define SET_UNSAFE_MODE(polarity) _unsafeMode = (polarity);


///////////////////////////////////////////////////////////////////////////////
// common error messages

#define J__STRINGIFY(x) #x
#define J__TOSTRING(x) J__STRINGIFY(x)

#ifdef DEBUG
	#define J__CODE_LOCATION " (in " __FILE__ ":" J__TOSTRING(__LINE__) ")"
#else
	#define J__CODE_LOCATION ""
#endif // DEBUG

#define J__ERRMSG_NO_CONSTRUCT "This object cannot be construct."
#define J__ERRMSG_NEED_CONSTRUCTION "Construction is needed for this object."
#define J__ERRMSG_MISSING_ARGUMENT "This function require more arguments."
#define J__ERRMSG_TOO_MANY_ARGUMENTS "You provide too many argument to the function."
#define J__ERRMSG_MISSING_N_ARGUMENT "This function require %d more argument(s)."
#define J__ERRMSG_INVALID_ARGUMENT "Invalid argument."
#define J__ERRMSG_INVALID_CLASS "Wrong object type."
#define J__ERRMSG_STRING_CONVERSION_FAILED "Unable to convert to string."
#define J__ERRMSG_INT_CONVERSION_FAILED "Unable to convert to integer."
#define J__ERRMSG_OUT_OF_MEMORY "Not enough memory to complete the allocation."
#define J__ERRMSG_NOT_INITIALIZED "The object or resource is not proprely initialized."
#define J__ERRMSG_INVALID_RESOURCE "The resource is invalid or not proprely initialized."
#define J__ERRMSG_CLASS_CREATION_FAILED "Unable to create the class."
#define J__ERRMSG_UNEXPECTED_TYPE "Unexpected data type."
#define J__ERRMSG_INVALID_RANGE "Value is out of range."
#define J__ERRMSG_UNINITIALIZED "Initialization failed."


///////////////////////////////////////////////////////////////////////////////
// helper macros

// new namespace for jslibs: J_

// BEWARE: the following helper macros are only valid inside a JS Native function definition !

#define J_ARGC (argc)

// returns the ARGument Vector
#define J_ARGV (argv)
// same for fast native (cf. http://developer.mozilla.org/en/docs/JS_ARGV)
#define J_FARGV (JS_ARGV(cx,vp))

// returns the ARGument n
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


#define J_JSVAL_IS_ARRAY(value) \
	( JSVAL_IS_OBJECT(value) && JS_IsArrayObject( cx, JSVAL_TO_OBJECT(value) ) == JS_TRUE)

#define J_JSVAL_IS_CLASS(value, jsClass) \
	( JSVAL_IS_OBJECT(value) && !JSVAL_IS_NULL(value) && JS_GET_CLASS(cx, JSVAL_TO_OBJECT(value)) == (jsClass) )

#define J_CHECK_CALL( functionCall ) \
	do { if (unlikely( (functionCall) == JS_FALSE )) { return JS_FALSE; } } while(0)


#define J_IS_SAFE_MODE(code) \

#define J_SAFE(code) \
	do { if (unlikely( !_unsafeMode )) {code;} } while(0)

#define J_UNSAFE(code) \
	do { if (likely( _unsafeMode )) {code;} } while(0)


// Reports warnings. May be disabled in unsafemode
#define J_REPORT_WARNING(errorMessage) \
	do { JS_ReportWarning( cx, (errorMessage J__CODE_LOCATION) ); } while(0)

#define J_REPORT_WARNING_1(errorMessage, arg) \
	do { JS_ReportWarning( cx, (errorMessage J__CODE_LOCATION), (arg) ); } while(0)

#define J_REPORT_WARNING_2(errorMessage, arg1, arg2) \
	do { JS_ReportWarning( cx, (errorMessage J__CODE_LOCATION), (arg1), (arg2) ); } while(0)


// Reports a fatal errors, script must stop as soon as possible.
#define J_REPORT_ERROR(errorMessage) \
	do { JS_ReportError( cx, (errorMessage J__CODE_LOCATION) ); return JS_FALSE; } while(0)

#define J_REPORT_ERROR_1(errorMessage, arg) \
	do { JS_ReportError( cx, (errorMessage J__CODE_LOCATION), (arg) ); return JS_FALSE; } while(0)

#define J_REPORT_ERROR_2(errorMessage, arg1, arg2) \
	do { JS_ReportError( cx, (errorMessage J__CODE_LOCATION), (arg1), (arg2) ); return JS_FALSE; } while(0)


// J_S_ stands for (J)slibs _ (S)afemode _ and mean that these macros will only be meaningful when unsafemode is false (see jslibs unsafemode).

#define J_S_ASSERT( condition, errorMessage ) \
	do { if (unlikely( !_unsafeMode && !(condition) )) { JS_ReportError( cx, (errorMessage J__CODE_LOCATION) ); return JS_FALSE; } } while(0)

#define J_S_ASSERT_1( condition, errorMessage, arg ) \
	do { if (unlikely( !_unsafeMode && !(condition) )) { JS_ReportError( cx, (errorMessage J__CODE_LOCATION), (arg) ); return JS_FALSE; } } while(0)

#define J_S_ASSERT_2( condition, errorMessage, arg1, arg2 ) \
	do { if (unlikely( !_unsafeMode && !(condition) )) { JS_ReportError( cx, (errorMessage J__CODE_LOCATION), (arg1), (arg2) ); return JS_FALSE; } } while(0)


#define J_S_ASSERT_ARG_MIN(minCount) \
	J_S_ASSERT_1( argc >= (minCount), J__ERRMSG_MISSING_N_ARGUMENT, (minCount)-argc )

#define J_S_ASSERT_ARG_MAX(maxCount) \
	J_S_ASSERT( argc <= (maxCount), J__ERRMSG_TOO_MANY_ARGUMENTS )

#define J_S_ASSERT_ARG_RANGE(minCount, maxCount) \
	J_S_ASSERT( argc >= (minCount) && argc <= (maxCount), "Invalid argument count." )


#define J_S_ASSERT_DEFINED(value) \
	J_S_ASSERT( !JSVAL_IS_VOID(value), "Value must be defined." )

#define J_S_ASSERT_TYPE(value, jsType) \
	J_S_ASSERT( JS_TypeOfValue(cx, (value)) == (jsType), J__ERRMSG_UNEXPECTED_TYPE )

#define J_S_ASSERT_BOOLEAN(value) \
	J_S_ASSERT( JSVAL_IS_BOOLEAN(value), J__ERRMSG_UNEXPECTED_TYPE " Boolean expected." )

#define J_S_ASSERT_INT(value) \
	J_S_ASSERT( JSVAL_IS_INT(value), J__ERRMSG_UNEXPECTED_TYPE " Integer expected." )

#define J_S_ASSERT_NUMBER(value) \
	J_S_ASSERT( JSVAL_IS_NUMBER(value), J__ERRMSG_UNEXPECTED_TYPE " Number expected." )

#define J_S_ASSERT_STRING(value) \
	J_S_ASSERT( JSVAL_IS_STRING(value), J__ERRMSG_UNEXPECTED_TYPE " String expected." )

#define J_S_ASSERT_OBJECT(value) \
	J_S_ASSERT( JSVAL_IS_OBJECT(value) && !JSVAL_IS_NULL(value), J__ERRMSG_UNEXPECTED_TYPE " Object expected." )

#define J_S_ASSERT_ARRAY(value) \
	J_S_ASSERT( J_JSVAL_IS_ARRAY(value), J__ERRMSG_UNEXPECTED_TYPE " Array expected." )

#define J_S_ASSERT_FUNCTION(value) \
	J_S_ASSERT( JS_TypeOfValue(cx, (value)) == JSTYPE_FUNCTION, " Function is expected." )

#define J_S_ASSERT_CLASS(jsObject, jsClass) \
	J_S_ASSERT_1( (jsObject) != NULL && JS_GET_CLASS(cx, jsObject) == (jsClass), J__ERRMSG_INVALID_CLASS "%s expected.", (jsClass)->name )

#define J_S_ASSERT_CLASS_NAME(jsObject, className) \
	J_S_ASSERT_1( strcmp(JS_GET_CLASS(cx, (jsObject))->name, (className)) == 0, J__ERRMSG_INVALID_CLASS "%s expected.", className )

#define J_S_ASSERT_THIS_CLASS() \
	J_S_ASSERT_CLASS(obj, _class)

#define J_S_ASSERT_CONSTRUCTING() \
	J_S_ASSERT( JS_IsConstructing(cx) == JS_TRUE, J__ERRMSG_NEED_CONSTRUCTION )

#define J_S_ASSERT_INITIALIZED(pointer) \
	J_S_ASSERT( (pointer) != NULL, J__ERRMSG_UNINITIALIZED )

#define J_S_ASSERT_RESOURCE(resourcePointer) \
	J_S_ASSERT( (resourcePointer) != NULL, J__ERRMSG_INVALID_RESOURCE )

#define J_S_ASSERT_ALLOC(pointer) \
	if (unlikely( (pointer) == NULL )) { J_REPORT_WARNING( J__ERRMSG_OUT_OF_MEMORY ); JS_ReportOutOfMemory(cx); return JS_FALSE; }



///////////////////////////////////////////////////////////////////////////////
// conversion macros

// (TBD) try to use real functions with __forceinline

#define J_JSVAL_TO_BOOL( jsval, boolVariable ) do { \
	if ( JSVAL_IS_BOOLEAN(jsval) ) { \
		boolVariable = (JSVAL_TO_BOOLEAN(jsval) == JS_TRUE); \
	} else { \
		JSBool __b; \
		if (unlikely( JS_ValueToBoolean( cx, jsval, &__b ) != JS_TRUE )) \
			J_REPORT_ERROR( "Unable to convert to integer." ); \
		boolVariable = (__b == JS_TRUE); \
	} \
} while(0)


#define J_JSVAL_TO_REAL( jsval, floatVariable ) do { \
	if ( JSVAL_IS_DOUBLE(jsval) ) { \
		floatVariable = *JSVAL_TO_DOUBLE(jsval); \
	} else { \
		jsdouble __d; \
		if (unlikely( JS_ValueToNumber( cx, jsval, &__d ) != JS_TRUE )) \
			J_REPORT_ERROR( "Unable to convert to real." ); \
		floatVariable = __d; \
	} \
} while(0)


#define J_JSVAL_TO_INT32( jsvalInt, intVariable ) do { \
	int32 __intVal; \
	if (unlikely( JS_ValueToInt32( cx, jsvalInt, &__intVal ) != JS_TRUE )) \
		J_REPORT_ERROR( "Unable to convert to a 32bit integer." ); \
	intVariable = __intVal; \
} while(0)


#define J_JSVAL_TO_UINT32( jsvalUInt, uintVariable ) do { \
	if ( JSVAL_IS_INT(jsvalUInt) && JSVAL_TO_INT(jsvalUInt) >= 0 ) { \
		uintVariable = JSVAL_TO_INT(jsvalUInt); \
	} else { \
		jsdouble __doubleValue; \
		if (unlikely( JS_ValueToNumber(cx, jsvalUInt, &__doubleValue ) != JS_TRUE )) \
			J_REPORT_ERROR( "Unable to convert to a 32bit unsigned integer." ); \
		uintVariable = (unsigned long)__doubleValue; \
		J_S_ASSERT( __doubleValue == (double)((unsigned long)__doubleValue), J__ERRMSG_INT_CONVERSION_FAILED ); \
	} \
} while(0)


#define J_JSVAL_TO_STRING( jsvalString, stringVariable ) do { \
	JSString *__jsString = JS_ValueToString(cx, (jsvalString)); \
	J_S_ASSERT( __jsString != NULL, J__ERRMSG_STRING_CONVERSION_FAILED ); \
	(stringVariable) = JS_GetStringBytes(__jsString); \
	J_S_ASSERT( (stringVariable) != NULL, J__ERRMSG_STRING_CONVERSION_FAILED ); \
} while(0)


#define J_JSVAL_TO_STRING_AND_LENGTH( jsvalString, stringVariable, lengthVariable ) do { \
	JSString *__jsString = JS_ValueToString(cx,(jsvalString)); \
	J_S_ASSERT( __jsString != NULL, J__ERRMSG_STRING_CONVERSION_FAILED ); \
	(stringVariable) = JS_GetStringBytes(__jsString); \
	J_S_ASSERT( (stringVariable) != NULL, J__ERRMSG_STRING_CONVERSION_FAILED ); \
	(lengthVariable) = JS_GetStringLength(__jsString); \
} while(0)


#define J_FILL_JSVAL_ARRAY( vector, length, jsvalVariable ) do { \
} while(0)


#define J_INT_VECTOR_TO_JSVAL( vector, length, jsvalVariable ) do { \
	JSObject *__arrayObj = JS_NewArrayObject(cx, 0, NULL); \
	RT_ASSERT_ALLOC(__arrayObj); \
	(jsvalVariable) = OBJECT_TO_JSVAL(__arrayObj); \
	jsval __tmpValue; \
	for ( jsint __i=0; __i<(length); ++__i ) { \
		__tmpValue = INT_TO_JSVAL((vector)[__i]); \
		J_CHECK_CALL( JS_SetElement(cx, __arrayObj, __i, &__tmpValue) ); \
	} \
} while(0)


#define J_JSVAL_TO_INT_VECTOR( jsvalArray, vectorVariable, lengthVariable ) do { \
	J_S_ASSERT_ARRAY(jsvalArray); \
	JSObject *__arrayObj = JSVAL_TO_OBJECT(jsvalArray); \
	jsuint __length; \
	J_CHECK_CALL( JS_GetArrayLength(cx, __arrayObj, &__length) ); \
	(lengthVariable) = __length; \
	J_S_ASSERT( __length <= sizeof(vectorVariable), "Too many elements in the array." ); \
	jsval __arrayElt; \
	for ( jsuint __i=0; __i<__length; __i++ ) { \
		J_CHECK_CALL( JS_GetElement(cx, __arrayObj, __i, &__arrayElt) ); \
		J_S_ASSERT_INT(__arrayElt); \
		(vectorVariable)[__i] = JSVAL_TO_INT(__arrayElt); \
	} \
} while(0)

// (TBD)
#define J_REAL_VECTOR_TO_JSVAL( vector, length, jsvalVariable ) do { \
} while(0)


#define J_JSVAL_TO_REAL_VECTOR( jsvalArray, vectorVariable, lengthVariable ) do { \
	J_S_ASSERT_ARRAY(jsvalArray); \
	JSObject *__arrayObj = JSVAL_TO_OBJECT(jsvalArray); \
	jsuint __length; \
	J_CHECK_CALL( JS_GetArrayLength(cx, __arrayObj, &__length) ); \
	lengthVariable = __length; \
	J_S_ASSERT( __length <= (lengthVariable), "Too many elements in the array." ); \
	jsval __arrayElt; \
	double __eltValue; \
	for ( jsuint __i=0; __i<__length; __i++ ) { \
		J_CHECK_CALL( JS_GetElement(cx, __arrayObj, __i, &__arrayElt) ); \
		J_CHECK_CALL( JS_ValueToNumber(cx, __arrayElt, &__eltValue) ); \
		(vectorVariable)[__i] = __eltValue; \
	} \
} while(0)


#define J_JSVAL_TO_ARRAY_LENGTH( jsvalArray, lengthVariable ) do { \
	J_S_ASSERT_ARRAY(jsvalArray); \
	JSObject *__arrayObj = JSVAL_TO_OBJECT(jsvalArray); \
	jsuint __length; \
	J_CHECK_CALL( JS_GetArrayLength(cx, __arrayObj, &__length) ); \
	lengthVariable = __length; \
} while(0)


// DEPRECATED macro
#define RT_CHECK_CALL J_CHECK_CALL
#define RT_SAFE J_SAFE
#define RT_UNSAFE J_UNSAFE
#define REPORT_WARNING J_REPORT_WARNING
#define REPORT_WARNING_1 J_REPORT_WARNING_1
#define REPORT_WARNING_2 J_REPORT_WARNING_2
#define REPORT_ERROR J_REPORT_ERROR
#define REPORT_ERROR_1 J_REPORT_ERROR_1
#define REPORT_ERROR_2 J_REPORT_ERROR_2
#define RT_ASSERT J_S_ASSERT
#define RT_ASSERT_1 J_S_ASSERT_1
#define RT_ASSERT_2 J_S_ASSERT_2
#define RT_ASSERT_TYPE J_S_ASSERT_TYPE
#define RT_ASSERT_DEFINED J_S_ASSERT_DEFINED
#define RT_ASSERT_OBJECT J_S_ASSERT_OBJECT
#define RT_ASSERT_ARRAY J_S_ASSERT_ARRAY

#define RT_ASSERT_INT J_S_ASSERT_INT
#define RT_ASSERT_NUMBER J_S_ASSERT_NUMBER
#define RT_ASSERT_STRING J_S_ASSERT_STRING
#define RT_ASSERT_FUNCTION J_S_ASSERT_FUNCTION
#define RT_ASSERT_ALLOC J_S_ASSERT_ALLOC
#define RT_ASSERT_RESOURCE J_S_ASSERT_RESOURCE
#define RT_ASSERT_CLASS J_S_ASSERT_CLASS
#define RT_ASSERT_THIS_CLASS J_S_ASSERT_THIS_CLASS
#define RT_ASSERT_CLASS_NAME J_S_ASSERT_CLASS_NAME
#define RT_ASSERT_ARGC J_S_ASSERT_ARG_MIN
#define RT_ASSERT_ARGC_MAX J_S_ASSERT_ARG_MAX
#define RT_ASSERT_CONSTRUCTING J_S_ASSERT_CONSTRUCTING

#define RT_JSVAL_TO_BOOL J_JSVAL_TO_BOOL
#define RT_JSVAL_TO_REAL J_JSVAL_TO_REAL
#define RT_JSVAL_TO_INT32 J_JSVAL_TO_INT32
#define RT_JSVAL_TO_UINT32 J_JSVAL_TO_UINT32
#define RT_JSVAL_TO_STRING J_JSVAL_TO_STRING
#define RT_JSVAL_TO_STRING_AND_LENGTH J_JSVAL_TO_STRING_AND_LENGTH


///////////////////////////////////////////////////////////////////////////////
// Native Interface mechanism

typedef void (*FunctionPointer)(void);

inline JSBool SetNativeInterface( JSContext *cx, JSObject *obj, const char *name, FunctionPointer function, void *descriptor ) {
	
	// Cannot be called while Finalize
	// the following must works because spidermonkey will never call the getter or setter if it is not explicitly required by the script
	J_CHECK_CALL( JS_DefineProperty(cx, obj, name, JSVAL_VOID, (JSPropertyOp)function, (JSPropertyOp)descriptor, JSPROP_READONLY | JSPROP_PERMANENT) );
	return JS_TRUE;
}

inline JSBool GetNativeInterface( JSContext *cx, JSObject *obj, const char *name, FunctionPointer *function, void **descriptor ) {

	// Cannot be called while Finalize
	uintN attrs;
	JSBool found;
	J_CHECK_CALL( JS_GetPropertyAttrsGetterAndSetter(cx, obj, name, &attrs, &found, (JSPropertyOp*)function, (JSPropertyOp*)descriptor) ); // NULL is supported for function and descriptor
	return JS_TRUE;
}

inline JSBool RemoveNativeInterface( JSContext *cx, JSObject *obj, const char *name ) {
	
	// Cannot be called while Finalize
	J_CHECK_CALL( JS_DeleteProperty(cx, obj, name) );
	return JS_TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// Helper functions

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

//	J_SAFE(	if ( (int)pv % 2 ) return JS_FALSE; ); // check if *vp is 2-byte aligned
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
	J_S_ASSERT( argc <= sizeof(argv)/sizeof(jsval), "Too many arguments." );
	va_start(ap, argc);
	for ( uintN i = 0; i < argc; i++ )
		argv[i] = va_arg(ap, jsval);
	va_end(ap);
	J_S_ASSERT_FUNCTION( functionValue );
	J_CHECK_CALL( JS_CallFunctionValue(cx, obj, functionValue, argc, argv, &rvalTmp) ); // NULL is NOT supported for &rvalTmp ( last arg of JS_CallFunctionValue )
	if ( rval != NULL )
		*rval = rvalTmp;
	return JS_TRUE;
}

// The following function wil only works if the class is defined in the global namespace.
inline JSClass *GetClassByName(JSContext *cx, const char *className) {

	JSObject *globalObj = JS_GetGlobalObject(cx);
	if ( globalObj == NULL )
		return NULL;
	jsval bstringConstructor;
	if ( JS_LookupProperty(cx, globalObj, className, &bstringConstructor) != JS_TRUE )
		return NULL;
	if ( bstringConstructor == JSVAL_VOID )
		return NULL;
	JSFunction *fun = JS_ValueToFunction(cx, bstringConstructor);
	if ( fun == NULL )
		return NULL;
	return fun->u.n.clasp; // (TBD) replace this by a jsapi.h call and remove dependency to jsarena.h and jsfun.h
}

inline bool MaybeRealloc( int requested, int received ) {

	return (100 * received / requested < 90) && (requested - received > 256);
}



#endif // _JSHELPER_H_
