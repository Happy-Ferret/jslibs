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

typedef JSBool (*NIStreamRead)( JSContext *cx, JSObject *obj, char *buffer, size_t *amount );
typedef JSBool (*NIBufferGet)( JSContext *cx, JSObject *obj, const char **buffer, size_t *size );
typedef JSBool (*NIMatrix44Get)( JSContext *cx, JSObject *obj, float **pm );


inline NIBufferGet BufferGetNativeInterface( JSContext *cx, JSObject *obj );
inline NIBufferGet BufferGetInterface( JSContext *cx, JSObject *obj );

#include "platform.h"

#include <cstring>

#include <stdarg.h>

#include <jsarena.h>
#include <jsfun.h>
#include <jsscope.h>
#include <jsobj.h>
#include <jsstr.h>
#include <jsscript.h>
#include <jscntxt.h>
#include <jsinterp.h>


#ifdef DEBUG
	#define IFDEBUG(expr) expr
#else
	#define IFDEBUG(expr)
#endif // DEBUG

// unsafe mode management

/*
#ifdef USE_UNSAFE_MODE
	extern bool _unsafeMode;
#else
	static bool _unsafeMode = false; // by default, we are in SAFE mode
#endif // USE_UNSAFE_MODE

#define DEFINE_UNSAFE_MODE	bool _unsafeMode = false;

#define SET_UNSAFE_MODE(polarity) _unsafeMode = (polarity);
*/

extern bool *_pUnsafeMode;

///////////////////////////////////////////////////////////////////////////////
// common error messages

#define J__STRINGIFY(x) #x
#define J__TOSTRING(x) J__STRINGIFY(x)

#ifdef DEBUG
	#define J__CODE_LOCATION __FILE__ ":" J__TOSTRING(__LINE__)
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

#define J_MIN(a,b) ( (a) < (b) ? (a) : (b) )
#define J_MAX(a,b) ( (a) > (b) ? (a) : (b) )

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


#ifdef DEBUG
	#define J_ADD_ROOT(cx, rp) (JS_AddNamedRoot((cx), (void*)(rp), J__CODE_LOCATION))
#else
	#define J_ADD_ROOT(cx, rp) (JS_AddRoot((cx),(void*)(rp)))
#endif // DEBUG

#define J_REMOVE_ROOT(cx, rp) (JS_RemoveRoot((cx),(void*)(rp)))


// check: used to forward an error.
#define J_CHK( status ) \
	do { \
		if (unlikely(!(status))) { return JS_FALSE; } \
	} while(0) \

// check with message: if status is false, a js exception is rised if it is not already pending.
#define J_CHKM( status, errorMessage ) \
	do { \
		if (unlikely( !(status) )) { \
			if ( !JS_IsExceptionPending(cx) ) \
				JS_ReportError(cx, (errorMessage IFDEBUG(" (@" J__CODE_LOCATION ")"))); \
			return JS_FALSE; \
		} \
	} while(0) \

// check with message and argument (printf like)
#define J_CHKM1( status, errorMessage, arg ) \
	do { \
		if (unlikely( !(status) )) { \
			if ( !JS_IsExceptionPending(cx) ) \
				JS_ReportError(cx, (errorMessage IFDEBUG(" (@" J__CODE_LOCATION ")")), (arg)); \
			return JS_FALSE; \
		} \
	} while(0) \



#define J_JSVAL_IS_CLASS(value, jsClass) \
	( JSVAL_IS_OBJECT(value) && !JSVAL_IS_NULL(value) && JS_GET_CLASS(cx, JSVAL_TO_OBJECT(value)) == (jsClass) )




#define J_SAFE_BEGIN if (unlikely( !*_pUnsafeMode )) {
#define J_SAFE_END }

#define J_UNSAFE_BEGIN if (likely( *_pUnsafeMode ))
#define J_UNSAFE_END }


#define J_SAFE(code) \
	do { if (unlikely( !*_pUnsafeMode )) {code;} } while(0)

#define J_UNSAFE(code) \
	do { if (likely( *_pUnsafeMode )) {code;} } while(0)


// Reports warnings. May be disabled in unsafemode
#define J_REPORT_WARNING(errorMessage) \
	do { JS_ReportWarning( cx, (errorMessage IFDEBUG(" (@" J__CODE_LOCATION ")")) ); } while(0)

#define J_REPORT_WARNING_1(errorMessage, arg) \
	do { JS_ReportWarning( cx, (errorMessage IFDEBUG(" (@" J__CODE_LOCATION ")")), (arg) ); } while(0)

#define J_REPORT_WARNING_2(errorMessage, arg1, arg2) \
	do { JS_ReportWarning( cx, (errorMessage IFDEBUG(" (@" J__CODE_LOCATION ")")), (arg1), (arg2) ); } while(0)


// Reports a fatal errors, script must stop as soon as possible.
#define J_REPORT_ERROR(errorMessage) \
	do { JS_ReportError( cx, (errorMessage IFDEBUG(" (@" J__CODE_LOCATION ")")) ); return JS_FALSE; } while(0)

#define J_REPORT_ERROR_1(errorMessage, arg) \
	do { JS_ReportError( cx, (errorMessage IFDEBUG(" (@" J__CODE_LOCATION ")")), (arg) ); return JS_FALSE; } while(0)

#define J_REPORT_ERROR_2(errorMessage, arg1, arg2) \
	do { JS_ReportError( cx, (errorMessage IFDEBUG(" (@" J__CODE_LOCATION ")")), (arg1), (arg2) ); return JS_FALSE; } while(0)



// J_S_ stands for (J)slibs _ (S)afemode _ and mean that these macros will only be meaningful when unsafemode is false (see jslibs unsafemode).

#define J_S_ASSERT( condition, errorMessage ) \
	do { if (unlikely( !*_pUnsafeMode && !(condition) )) { JS_ReportError( cx, errorMessage IFDEBUG(" (" #condition " @" J__CODE_LOCATION ")") ); return JS_FALSE; } } while(0)

#define J_S_ASSERT_1( condition, errorMessage, arg ) \
	do { if (unlikely( !*_pUnsafeMode && !(condition) )) { JS_ReportError( cx, errorMessage IFDEBUG(" (" #condition " @" J__CODE_LOCATION ")"), (arg) ); return JS_FALSE; } } while(0)

#define J_S_ASSERT_2( condition, errorMessage, arg1, arg2 ) \
	do { if (unlikely( !*_pUnsafeMode && !(condition) )) { JS_ReportError( cx, errorMessage IFDEBUG(" (" #condition " @" J__CODE_LOCATION ")"), (arg1), (arg2) ); return JS_FALSE; } } while(0)


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
//

// The following function wil only works if the class is defined in the global namespace (say global object)
inline JSClass *GetGlobalClassByName(JSContext *cx, const char *className) {

	// see.  js_FindClassObject(cx, NULL, INT_TO_JSID(JSProto_StopIteration), &v)) / JS_GetClassObject

	JSObject *globalObj = JS_GetGlobalObject(cx);
	if ( globalObj == NULL )
		return NULL;
	jsval classConstructor;
	if ( JS_LookupProperty(cx, globalObj, className, &classConstructor) != JS_TRUE )
		return NULL;
	if ( JS_TypeOfValue(cx, classConstructor) != JSTYPE_FUNCTION )
		return NULL;
	JSFunction *fun = JS_ValueToFunction(cx, classConstructor);
	if ( fun == NULL )
		return NULL;
	if ( !FUN_SLOW_NATIVE(fun) )
		return NULL;
	return fun->u.n.clasp; // (TBD) replace this by a jsapi.h call and remove dependency to jsarena.h and jsfun.h
}

///////////////////////////////////////////////////////////////////////////////
// test and conversion functions

//#define J_STRING_LENGTH(jsstr) (JS_GetStringLength(jsstr))
#define J_STRING_LENGTH(jsstr) (JSSTRING_LENGTH(jsstr))


#define J_JSVAL_IS_STRING(val) ( JSVAL_IS_STRING(val) || (JSVAL_IS_OBJECT(val) && !JSVAL_IS_NULL(val) && BufferGetInterface(cx, JSVAL_TO_OBJECT(val)) != NULL) )


inline JSBool J_NewBlob( JSContext *cx, void* buffer, size_t length, jsval *vp ) {

	if ( length == 0 ) { // Empty Blob must acts like an empty string: !'' == true

		*vp = JS_GetEmptyStringValue(cx);
		return JS_TRUE;
	}

	static JSClass *blobClass = NULL;
	if ( blobClass == NULL )
		blobClass = GetGlobalClassByName(cx, "Blob");

	JSObject *blob;
	if ( blobClass != NULL ) { // we have Blob class, jslang is present.

//		blob = JS_NewObject(cx, blobClass, NULL, NULL);
		blob = JS_ConstructObject(cx, blobClass, NULL, NULL); // need to be constructed else Buffer NativeInterface will not be set !

		*vp = OBJECT_TO_JSVAL(blob);
		if ( blob == NULL )
			goto err;
		if ( JS_SetReservedSlot(cx, blob, 0, INT_TO_JSVAL( length )) != JS_TRUE ) // 0 for SLOT_BLOB_LENGTH !!!
			goto err;
		if ( JS_SetPrivate(cx, blob, buffer) != JS_TRUE ) // blob data
			goto err;
	} else {

		JSString *jsstr = JS_NewString(cx, (char*)buffer, length); // JS_NewString takes ownership of bytes on success, avoiding a copy; but on error (signified by null return), it leaves bytes owned by the caller. So the caller must free bytes in the error case, if it has no use for them.
		if ( jsstr == NULL )
			goto err;
		*vp = STRING_TO_JSVAL(jsstr);
	}
	return JS_TRUE;
err:
	*vp = JSVAL_VOID; // needed ?
	JS_free(cx, buffer); // JS_NewString does not free the buffer on error.
	return JS_FALSE;
}


inline JSBool J_NewBlobCopyN( JSContext *cx, const void *data, size_t amount, jsval *vp ) {

	if ( amount == 0 ) { // Empty Blob must acts like an empty string: !'' == true

		*vp = JS_GetEmptyStringValue(cx);
		return JS_TRUE;
	}

	// possible optimization: if Blob class is not abailable, copy data into JSString's jschar to avoid js_InflateString.
	char *blobBuf = (char*)JS_malloc(cx, amount);
	J_S_ASSERT_ALLOC( blobBuf );
	memcpy( blobBuf, data, amount );
	J_CHK( J_NewBlob(cx, blobBuf, amount, vp) );
	return JS_TRUE;
}

/*
inline bool JsvalIsDataBuffer( JSContext *cx, jsval val ) {

	if ( JSVAL_IS_STRING(val) )
		return true;

	if ( !JSVAL_IS_OBJECT(val) )
		return false;
//	NIBufferGet fct = BufferGetNativeInterface(cx, JSVAL_TO_OBJECT(val)); // why not BufferGetInterface() ?
	NIBufferGet fct = BufferGetInterface(cx, JSVAL_TO_OBJECT(val));
	if ( fct )
		return true;
	return false;
//	if ( JSVAL_IS_OBJECT(val) && !JSVAL_IS_NULL(val) && JS_GET_CLASS(cx, JSVAL_TO_OBJECT(val)) == BlobJSClass(cx) )
//		return true;
}
*/

// beware: caller should keep a reference to buffer as short time as possible, because it is difficult to protect it from GC.
inline JSBool JsvalToStringAndLength( JSContext *cx, jsval *val, const char** buffer, size_t *size ) {

	if ( JSVAL_IS_STRING(*val) ) { // for string literals

		JSString *str = JSVAL_TO_STRING(*val);
		*buffer = JS_GetStringBytes(str); // JS_GetStringBytes never returns NULL
//		J_S_ASSERT( *buffer != NULL, "Invalid string." );
		*size = J_STRING_LENGTH(str);
		return JS_TRUE;
	}

	if ( JSVAL_IS_OBJECT(*val) && !JSVAL_IS_NULL(*val) ) { // for NIBufferGet support

		NIBufferGet fct = BufferGetNativeInterface(cx, JSVAL_TO_OBJECT(*val));
		if ( fct )
			return fct(cx, JSVAL_TO_OBJECT(*val), buffer, size);
	}

	// and for anything else ...
	JSString *jsstr = JS_ValueToString(cx, *val);
	*val = STRING_TO_JSVAL(jsstr); // *val must be GC protected
	J_S_ASSERT( jsstr != NULL, J__ERRMSG_STRING_CONVERSION_FAILED );
	*size = J_STRING_LENGTH(jsstr);
	*buffer = JS_GetStringBytes(jsstr); // JS_GetStringBytes never returns NULL
//	J_S_ASSERT( *buffer != NULL, J__ERRMSG_STRING_CONVERSION_FAILED );
	return JS_TRUE;
}


inline JSBool JsvalToStringLength( JSContext *cx, jsval val, size_t *length ) {

	if ( JSVAL_IS_STRING(val) ) {

		*length = J_STRING_LENGTH( JSVAL_TO_STRING(val) );
		return JS_TRUE;
	}

	if ( JSVAL_IS_OBJECT(val) && !JSVAL_IS_NULL(val) ) {

		NIBufferGet fct = BufferGetNativeInterface(cx, JSVAL_TO_OBJECT(val));
		const char* tmp;
		if ( fct )
			return fct(cx, JSVAL_TO_OBJECT(val), &tmp, length);
	}

	// unfortunately, we have to convert to a string to know its length
	JSString *str = JS_ValueToString(cx, val); // no GC protection needed.

	// *val = STRING_TO_JSVAL(str); <-- (TBD) to solve the GC issue.

	J_S_ASSERT( str != NULL, J__ERRMSG_STRING_CONVERSION_FAILED );
	*length = J_STRING_LENGTH(str);
	return JS_TRUE;
}


inline JSBool JsvalToString( JSContext *cx, jsval *val, const char** buffer ) {

	size_t size; //unused
	return JsvalToStringAndLength( cx, val, buffer, &size );
}


inline JSBool StringToJsval( JSContext *cx, jsval *val, const char* cstr ) {

	JSString *jsstr = JS_NewStringCopyZ(cx, cstr);
	if ( jsstr == NULL )
		J_REPORT_ERROR( "Unable to create the string." );
	*val = STRING_TO_JSVAL(jsstr);
	return JS_TRUE;
}


inline JSBool StringAndLengthToJsval( JSContext *cx, jsval *val, const char* cstr, size_t length ) {

	JSString *jsstr = JS_NewStringCopyN(cx, cstr, length);
	if ( jsstr == NULL )
		J_REPORT_ERROR( "Unable to create the string." );
	*val = STRING_TO_JSVAL(jsstr);
	return JS_TRUE;
}


inline JSBool JsvalToInt( JSContext *cx, jsval val, int *intVal ) {

	if ( JSVAL_IS_INT(val) )
		*intVal = JSVAL_TO_INT(val);
	else {
		int32 tmp;
		if (unlikely( JS_ValueToInt32(cx, val, &tmp) != JS_TRUE ))
			J_REPORT_ERROR( "Unable to convert to a 32bit integer." );
		*intVal = tmp;
	}
	return JS_TRUE;
}

inline JSBool JsvalToUInt( JSContext *cx, jsval val, unsigned int *uintVal ) {

	if ( JSVAL_IS_INT(val) )
		*uintVal = JSVAL_TO_INT(val);
	else {

		jsdouble d;
		J_CHKM( JS_ValueToNumber(cx, val, &d), "Unable to convert to a 32bit integer." );
		*uintVal = (unsigned int)d;
	}
	return JS_TRUE;
}


inline JSBool IntToJsval( JSContext *cx, int intVal, jsval *val ) {

	if ( INT_FITS_IN_JSVAL(intVal) )
		*val = INT_TO_JSVAL(intVal);
	else
		if (unlikely( JS_NewNumberValue(cx, intVal, val) != JS_TRUE ))
			J_REPORT_ERROR( "Unable to convert to a 32bit integer." );
	return JS_TRUE;
}


inline JSBool UIntToJsval( JSContext *cx, unsigned int uintVal, jsval *val ) {

	if ( uintVal <= JSVAL_INT_MAX )
		*val = INT_TO_JSVAL(uintVal);
	else
		if (unlikely( JS_NewNumberValue(cx, uintVal, val) != JS_TRUE ))
			J_REPORT_ERROR( "Unable to convert to a 32bit integer." );
	return JS_TRUE;
}



inline JSBool JsvalToBool( JSContext *cx, const jsval val, bool *b ) {

	if ( JSVAL_IS_BOOLEAN(val) ) {
		
		*b = (JSVAL_TO_BOOLEAN(val) == JS_TRUE);
		return JS_TRUE;
	} else {
		JSBool tmp;
		if (unlikely( JS_ValueToBoolean( cx, val, &tmp ) != JS_TRUE ))
			J_REPORT_ERROR( "Unable to convert to boolean." );
		*b = (tmp == JS_TRUE);
		return JS_TRUE;
	}
}


inline JSBool JsvalToDouble( JSContext *cx, jsval val, double *d ) {

	if ( JSVAL_IS_DOUBLE(val) ) {
		
		*d = *JSVAL_TO_DOUBLE(val);
		return JS_TRUE;
	} else {

		jsdouble tmp;
		if (unlikely( JS_ValueToNumber( cx, val, &tmp ) != JS_TRUE ))
			J_REPORT_ERROR( "Unable to convert to real." );
		*d = tmp;
		return JS_TRUE;
	}
}




inline JSBool SetPropertyInt( JSContext *cx, JSObject *obj, const char *propertyName, int intVal ) {

	jsval val;
	if ( INT_FITS_IN_JSVAL(intVal) )
		val = INT_TO_JSVAL(intVal);
	else
		if (unlikely( JS_NewNumberValue(cx, intVal, &val) != JS_TRUE ))
			J_REPORT_ERROR( "Unable to convert to an integer." );
	// Doc. http://developer.mozilla.org/en/docs/JS_DefineUCProperty
	if (unlikely( JS_DefineProperty(cx, obj, propertyName, val, NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT ) != JS_TRUE ))
//	if (unlikely( JS_SetProperty(cx, obj, propertyName, &val ) != JS_TRUE ))
		J_REPORT_ERROR( "Unable to set the property." );
	return JS_TRUE;
}

inline JSBool GetPropertyInt( JSContext *cx, JSObject *obj, const char *propertyName, int *intVal ) {

	jsval val;
	if (unlikely( JS_GetProperty(cx, obj, propertyName, &val) != JS_TRUE )) // cf. OBJ_GET_PROPERTY(...
		J_REPORT_ERROR_1( "Unable to read the property %s.", propertyName );
	if ( JSVAL_IS_INT(val) )
		*intVal = JSVAL_TO_INT(val);
	else {

		int32 tmp;
		if (unlikely( JS_ValueToInt32(cx, val, &tmp) != JS_TRUE ))
			J_REPORT_ERROR( "Unable to convert to an integer." );
		*intVal = tmp;
	}
	return JS_TRUE;
}

#define J_JSVAL_TO_INT32( jsvalInt, intVariable ) do { \
	if ( JSVAL_IS_INT(jsvalInt) ) { \
		intVariable = JSVAL_TO_INT(jsvalInt); \
	} else { \
		int32 __intVal; \
		if (unlikely( JS_ValueToInt32( cx, jsvalInt, &__intVal ) != JS_TRUE )) \
			J_REPORT_ERROR( "Unable to convert to a 32bit integer." ); \
		intVariable = __intVal; \
	} \
} while(0)


#define J_PROPERTY_TO_INT32( jsobject, propertyName, intVariable ) do { \
	jsval __tmpVal; \
	J_CHK( JS_GetProperty(cx, jsobject, propertyName, &__tmpVal) ); \
	J_JSVAL_TO_INT32( __tmpVal, intVariable ); \
} while(0)



#define J_JSVAL_TO_UINT32( jsvalUInt, uintVariable ) do { \
	if ( JSVAL_IS_INT(jsvalUInt) ) { \
		uintVariable = JSVAL_TO_INT(jsvalUInt); \
	} else { \
		jsdouble __doubleValue; \
		if (unlikely( JS_ValueToNumber(cx, jsvalUInt, &__doubleValue ) != JS_TRUE )) \
			J_REPORT_ERROR( "Unable to convert to a 32bit unsigned integer." ); \
		uintVariable = (unsigned long)__doubleValue; \
		J_S_ASSERT( __doubleValue == (double)((unsigned long)__doubleValue), J__ERRMSG_INT_CONVERSION_FAILED ); \
	} \
	J_S_ASSERT( uintVariable >= 0, "Unable to convert to a 32bit unsigned integer." ); \
} while(0)




///////////////////////////////////////////////////////////////////////////////
// conversion macros

// (TBD) try to use real functions with __forceinline ?

/*
inline JSBool JsvalToBool( JSContext *cx, jsval val, bool *bval ) {

	if ( JSVAL_IS_BOOLEAN(val) ) {

		*bval = JSVAL_TO_BOOLEAN(val) == JS_TRUE;
		return JS_TRUE;
	}
	JSBool b;
	J_CHKM( JS_ValueToBoolean(cx, val, &b), "Unable to convert to boolean." );
	*bval = b == JS_TRUE;
	return JS_TRUE;
}
*/


#define J_JSVAL_TO_BOOL( jsval, boolVariable ) do { \
	if ( JSVAL_IS_BOOLEAN(jsval) ) { \
		boolVariable = (JSVAL_TO_BOOLEAN(jsval) == JS_TRUE); \
	} else { \
		JSBool __b; \
		if (unlikely( JS_ValueToBoolean( cx, jsval, &__b ) != JS_TRUE )) \
			J_REPORT_ERROR( "Unable to convert to boolean." ); \
		boolVariable = (__b == JS_TRUE); \
	} \
} while(0)



#define J_JSVAL_TO_REAL( jsval, realVal ) do { \
	if ( JSVAL_IS_DOUBLE(jsval) ) { \
		realVal = *JSVAL_TO_DOUBLE(jsval); \
	} else { \
		jsdouble __d; \
		if (unlikely( JS_ValueToNumber( cx, jsval, &__d ) != JS_TRUE )) \
			J_REPORT_ERROR( "Unable to convert to real." ); \
		realVal = __d; \
	} \
} while(0)


// (TBD) ?
//#define J_FILL_JSVAL_ARRAY( vector, length, jsvalVariable ) do { \
//} while(0)


#define J_INT_VECTOR_TO_JSVAL( vector, length, jsvalVariable ) do { \
	JSObject *__arrayObj = JS_NewArrayObject(cx, 0, NULL); \
	J_S_ASSERT_ALLOC(__arrayObj); \
	(jsvalVariable) = OBJECT_TO_JSVAL(__arrayObj); \
	jsval __tmpValue; \
	for ( jsint __i=0; __i<(length); ++__i ) { \
		__tmpValue = INT_TO_JSVAL((vector)[__i]); \
		J_CHK( JS_SetElement(cx, __arrayObj, __i, &__tmpValue) ); \
	} \
} while(0)


#define J_JSVAL_TO_INT_VECTOR( jsvalArray, vectorVariable, lengthVariable ) do { \
	J_S_ASSERT_ARRAY(jsvalArray); \
	JSObject *__arrayObj = JSVAL_TO_OBJECT(jsvalArray); \
	jsuint __length; \
	J_CHK( JS_GetArrayLength(cx, __arrayObj, &__length) ); \
	(lengthVariable) = __length; \
	J_S_ASSERT( __length <= sizeof(vectorVariable), "Too many elements in the array." ); \
	jsval __arrayElt; \
	for ( jsuint __i=0; __i<__length; __i++ ) { \
		J_CHK( JS_GetElement(cx, __arrayObj, __i, &__arrayElt) ); \
		J_S_ASSERT_INT(__arrayElt); \
		(vectorVariable)[__i] = JSVAL_TO_INT(__arrayElt); \
	} \
} while(0)


#define J_REAL_VECTOR_TO_JSVAL( vector, length, jsvalVariable ) do { \
	JSObject *__arrayObj = JS_NewArrayObject(cx, 0, NULL); \
	J_S_ASSERT_ALLOC(__arrayObj); \
	(jsvalVariable) = OBJECT_TO_JSVAL(__arrayObj); \
	jsval __tmpValue; \
	for ( jsint __i=0; __i<(length); ++__i ) { \
		J_CHK( JS_NewNumberValue(cx, (vector)[__i], &__tmpValue) ); \
		J_CHK( JS_SetElement(cx, __arrayObj, __i, &__tmpValue) ); \
	} \
} while(0)


#define J_JSVAL_TO_REAL_VECTOR( jsvalArray, vectorVariable, lengthVariable ) do { \
	J_S_ASSERT_ARRAY(jsvalArray); \
	JSObject *__arrayObj = JSVAL_TO_OBJECT(jsvalArray); \
	jsuint __length; \
	J_CHK( JS_GetArrayLength(cx, __arrayObj, &__length) ); \
	lengthVariable = __length; \
	J_S_ASSERT( __length <= (lengthVariable), "Too many elements in the array." ); \
	jsval __arrayElt; \
	double __eltValue; \
	for ( jsuint __i=0; __i<__length; __i++ ) { \
		J_CHK( JS_GetElement(cx, __arrayObj, __i, &__arrayElt) ); \
		J_CHK( JS_ValueToNumber(cx, __arrayElt, &__eltValue) ); \
		(vectorVariable)[__i] = __eltValue; \
	} \
} while(0)


#define J_JSVAL_TO_ARRAY_LENGTH( jsvalArray, lengthVariable ) do { \
	J_S_ASSERT_ARRAY(jsvalArray); \
	JSObject *__arrayObj = JSVAL_TO_OBJECT(jsvalArray); \
	jsuint __length; \
	J_CHK( JS_GetArrayLength(cx, __arrayObj, &__length) ); \
	lengthVariable = __length; \
} while(0)


///////////////////////////////////////////////////////////////////////////////
// Helper functions

/* properly
inline bool SwapObjects( JSContext *cx, JSObject *obj1, JSObject *obj2 ) {

	if ( obj1 == NULL || obj2 == NULL )
		return JS_FALSE;

// When JSObject.dslots is not null, JSObject.dslots[-1] records the number of available slots.
//	JSScope *s1 = OBJ_SCOPE(obj1);

//js_ObjectOps.newObjectMap(cx, obj2->map->nrefs, obj2->map->ops, JS_GET_CLASS(cx, obj2), obj1);

	//	JSObjectMap *map1 = obj1->map->ops->newObjectMap(cx, obj2->map->nrefs, obj2->map->ops, JS_GET_CLASS(cx, obj2), obj2);

	// exchange object contents
	JSObject tmp = *obj1;
	*obj1 = *obj2;
	*obj2 = tmp;

	// fix scope owners
	OBJ_SCOPE(obj1)->object = obj1;
	OBJ_SCOPE(obj2)->object = obj2;

	// fix referencing objects count  ?????
	jsrefcount nrefs = obj1->map->nrefs;
	obj1->map->nrefs = obj2->map->nrefs;
	obj2->map->nrefs = nrefs;

	return JS_TRUE;
}
*/


inline bool IsPInfinity( JSContext *cx, jsval val ) {

	return JS_GetPositiveInfinityValue(cx) == val;
}

inline bool IsNInfinity( JSContext *cx, jsval val ) {

	return JS_GetNegativeInfinityValue(cx) == val;
}

inline bool JsvalIsFunction( JSContext *cx, jsval val ) {
	return ( JS_TypeOfValue(cx, (val)) == JSTYPE_FUNCTION );
}

inline bool JsvalIsArray( JSContext *cx, jsval val ) {
	return ( JSVAL_IS_OBJECT(val) && JS_IsArrayObject( cx, JSVAL_TO_OBJECT(val) ) == JS_TRUE );
}

#define J_JSVAL_IS_ARRAY(value) \
	JsvalIsArray(cx, (value))


inline bool InheritFrom( JSContext *cx, JSObject *obj, JSClass *clasp ) {

	while( obj != NULL ) {

		obj = JS_GetPrototype(cx, obj);
		if ( JS_GET_CLASS(cx, obj) == clasp )
			return true;
	}
	return false;
}

/*
inline JSBool GetNamedPrivate( JSContext *cx, JSObject *obj, const char *name, void **pv ) {

	jsval tmp;
	if ( JS_GetProperty(cx, obj, name, &tmp) == JS_FALSE )
		return JS_FALSE;
	*pv = tmp == JSVAL_VOID ? NULL : JSVAL_TO_PRIVATE(tmp);
	return JS_TRUE;
}
*/

/*
inline JSBool SetNamedPrivate( JSContext *cx, JSObject *obj, const char *name, const void *pv ) {

//	J_SAFE(	if ( (int)pv % 2 ) return JS_FALSE; ); // check if *vp is 2-byte aligned
	if ( JS_DefineProperty(cx, obj, name, PRIVATE_TO_JSVAL(pv), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT ) == JS_FALSE )
		return JS_FALSE;
	return JS_TRUE;
}
*/


inline bool HasProperty( JSContext *cx, JSObject *obj, const char *propertyName ) {

	uintN attr;
	JSBool found;
	JSBool status = JS_GetPropertyAttributes(cx, obj, propertyName, &attr, &found);
	return ( status == JS_TRUE && found != JS_FALSE );
}

/*
#define J_HAS_PROPERTY( object, propertyName, has ) do { \
	uintN __attr;
	JSBool __found;
	JSBool __status = JS_GetPropertyAttributes(cx, obj, propertyName, &__attr, &__found);
	(has) = ( __status == JS_TRUE && __found != JS_FALSE );
} while(0)
*/


inline JSBool JL_CallFunction( JSContext *cx, JSObject *obj, jsval functionValue, jsval *rval, uintN argc, ... ) {

	va_list ap;
	jsval argv[16]; // argc MUST be <= 16
	jsval rvalTmp;
	J_S_ASSERT( argc <= sizeof(argv)/sizeof(*argv), "Too many arguments." );
	va_start(ap, argc);
	for ( uintN i = 0; i < argc; i++ )
		argv[i] = va_arg(ap, jsval);
	va_end(ap);
	J_S_ASSERT_FUNCTION( functionValue );
	J_CHK( JS_CallFunctionValue(cx, obj, functionValue, argc, argv, &rvalTmp) ); // NULL is NOT supported for &rvalTmp ( last arg of JS_CallFunctionValue )
	if ( rval != NULL )
		*rval = rvalTmp;
	return JS_TRUE;
}

inline JSBool JL_CallFunctionName( JSContext *cx, JSObject *obj, const char* functionName, jsval *rval, uintN argc, ... ) {

	va_list ap;
	jsval argv[16]; // argc MUST be <= 16
	jsval rvalTmp;
	J_S_ASSERT( argc <= sizeof(argv)/sizeof(*argv), "Too many arguments." );
	va_start(ap, argc);
	for ( uintN i = 0; i < argc; i++ )
		argv[i] = va_arg(ap, jsval);
	va_end(ap);
	J_CHK( JS_CallFunctionName(cx, obj, functionName, argc, argv, &rvalTmp) ); // NULL is NOT supported for &rvalTmp ( last arg of JS_CallFunctionValue )
	if ( rval != NULL )
		*rval = rvalTmp;
	return JS_TRUE;
}


inline bool MaybeRealloc( int requested, int received ) {

	return requested != 0 && (128 * received / requested < 115) && (requested - received > 32); // instead using percent, I use per-128
}


///////////////////////////////////////////////////////////////////////////////
// NativeInterface

inline JSBool ReserveNativeInterface( JSContext *cx, JSObject *obj, const char *name ) {

	J_CHK( JS_DefineProperty(cx, obj, name, JSVAL_FALSE, NULL, (JSPropertyOp)-1, JSPROP_READONLY | JSPROP_PERMANENT ) );
	return JS_TRUE;
}

inline JSBool SetNativeInterface( JSContext *cx, JSObject *obj, const char *name, void *nativeFct ) {

	if ( nativeFct != NULL ) {

		J_CHK( JS_DefineProperty(cx, obj, name, JSVAL_TRUE, NULL, (JSPropertyOp)nativeFct, JSPROP_READONLY | JSPROP_PERMANENT ) );
	} else {

		J_CHK( JS_DeleteProperty(cx, obj, name) );
		J_CHK( ReserveNativeInterface(cx, obj, name) );
	}
	return JS_TRUE;
}

inline JSBool GetNativeInterface( JSContext *cx, JSObject *obj, JSObject **obj2p, jsid iid, void **nativeFct ) {

//	JSObject *obj2;
	JSProperty *prop;
	J_CHKM( OBJ_LOOKUP_PROPERTY(cx, obj, iid, obj2p, &prop), "Unable to get the native interface.");

//	const char *name = JS_GetStringBytes(JS_ValueToString(cx, iid));
	if ( prop && obj == *obj2p && ((JSScopeProperty*)prop)->setter != (JSPropertyOp)-1 )
		*nativeFct = (void*)((JSScopeProperty*)prop)->setter; // is NULL if obj is non-native
	else
		*nativeFct = NULL;

	if ( prop )
		OBJ_DROP_PROPERTY(cx, *obj2p, prop);

	return JS_TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// NativeInterface StreamRead

inline JSBool JSStreamRead( JSContext *cx, JSObject *obj, char *buffer, size_t *amount ) {

	jsval tmpVal, rval;
	IntToJsval(cx, *amount, &tmpVal);
	J_CHKM( JS_CallFunctionName(cx, obj, "Read", 1, &tmpVal, &rval), "Read() function not found.");

	if ( rval == JSVAL_VOID ) {

		*amount = 0;
		return JS_TRUE;
	}

	const char *tmpBuf;
	size_t size;
	J_CHK( JsvalToStringAndLength(cx, &rval, &tmpBuf, &size) );
	*amount = J_MIN(size, *amount);
	memcpy(buffer, tmpBuf, *amount);
	return JS_TRUE;
}

inline JSBool ReserveStreamReadInterface( JSContext *cx, JSObject *obj ) {

	return ReserveNativeInterface(cx, obj, "_NI_StreamRead" );
}

inline JSBool SetStreamReadInterface( JSContext *cx, JSObject *obj, NIStreamRead pFct ) {

	return SetNativeInterface( cx, obj, "_NI_StreamRead", (void*)pFct );
}

inline NIStreamRead StreamReadNativeInterface( JSContext *cx, JSObject *obj ) {

	static jsid propId = 0; // (TBD) try to make this higher than module-static
	if ( !propId )
		if ( JS_ValueToId(cx, STRING_TO_JSVAL(JS_InternString(cx, "_NI_StreamRead")), &propId) != JS_TRUE )
			return NULL;
	void *streamRead;
	JSObject *obj2;
	if ( GetNativeInterface( cx, obj, &obj2, propId, &streamRead ) != JS_TRUE )
		return NULL;
	return (NIStreamRead)streamRead;
}

inline NIStreamRead StreamReadInterface( JSContext *cx, JSObject *obj ) {

	void *fct = (void*)StreamReadNativeInterface(cx, obj);
	if ( fct )
		return (NIStreamRead)fct;

	jsval res;
	if ( JS_GetProperty(cx, obj, "Read", &res) != JS_TRUE || !JsvalIsFunction(cx, res) )
		return NULL;

	return JSStreamRead;
}


///////////////////////////////////////////////////////////////////////////////
// NativeInterface BufferGet

inline JSBool JSBufferGet( JSContext *cx, JSObject *obj, const char **buffer, size_t *size ) {

	jsval rval;
	J_CHKM( JS_CallFunctionName(cx, obj, "Get", 0, NULL, &rval), "Get() function not found."); // do not use toString() !? no !
	J_CHK( JsvalToStringAndLength(cx, &rval, buffer, size) ); // (TBD) GC protect rval
	return JS_TRUE;
}

inline JSBool ReserveBufferGetInterface( JSContext *cx, JSObject *obj ) {

	return ReserveNativeInterface(cx, obj, "_NI_BufferGet" );
}

inline JSBool SetBufferGetInterface( JSContext *cx, JSObject *obj, NIBufferGet pFct ) {

	return SetNativeInterface( cx, obj, "_NI_BufferGet", (void*)pFct );
}

inline NIBufferGet BufferGetNativeInterface( JSContext *cx, JSObject *obj ) {

	static jsid propId = 0; // (TBD) try to make this higher than module-static
	if ( !propId )
		if ( JS_ValueToId(cx, STRING_TO_JSVAL(JS_InternString(cx, "_NI_BufferGet")), &propId) != JS_TRUE )
			return NULL;
	void *fct;
	JSObject *obj2;
	if ( GetNativeInterface( cx, obj, &obj2, propId, &fct ) != JS_TRUE )
		return NULL;
	return (NIBufferGet)fct;
}

inline NIBufferGet BufferGetInterface( JSContext *cx, JSObject *obj ) {

	void *fct = (void*)BufferGetNativeInterface(cx, obj);
	if ( fct )
		return (NIBufferGet)fct;

	jsval res;
	if ( JS_GetProperty(cx, obj, "Get", &res) != JS_TRUE || !JsvalIsFunction(cx, res) ) // do not use toString() directly, but Get can call toString().
		return NULL;

	return JSBufferGet;
}


///////////////////////////////////////////////////////////////////////////////
// NativeInterface Matrix44Get

/*
inline JSBool JSMatrix44Get( JSContext *cx, JSObject *obj, const char **buffer, size_t *size ) {

	jsval rval;
	J_CHKM( JS_CallFunctionName(cx, obj, "Get", 0, NULL, &rval), "Get() function not found."); // do not use toString() !?
	J_CHK( JsvalToStringAndLength(cx, rval, buffer, size) );
	return JS_TRUE;
}
*/

inline JSBool ReserveMatrix44GetInterface( JSContext *cx, JSObject *obj ) {

	return ReserveNativeInterface(cx, obj, "_NIMatrix44Get" );
}

inline JSBool SetMatrix44GetInterface( JSContext *cx, JSObject *obj, NIMatrix44Get pFct ) {

	return SetNativeInterface( cx, obj, "_NI_Matrix44Get", (void*)pFct );
}

inline NIMatrix44Get Matrix44GetNativeInterface( JSContext *cx, JSObject *obj ) {

	static jsid propId = 0; // (TBD) try to make this higher than module-static
	if ( !propId )
		if ( JS_ValueToId(cx, STRING_TO_JSVAL(JS_InternString(cx, "_NI_Matrix44Get")), &propId) != JS_TRUE )
			return NULL;
	void *fct;
	JSObject *obj2;
	if ( GetNativeInterface( cx, obj, &obj2, propId, &fct ) != JS_TRUE )
		return NULL;
	return (NIMatrix44Get)fct;
}

inline NIMatrix44Get Matrix44GetInterface( JSContext *cx, JSObject *obj ) {

	void *fct = (void*)Matrix44GetNativeInterface(cx, obj);
	if ( fct )
		return (NIMatrix44Get)fct;
/*
	jsval res;
	if ( JS_GetProperty(cx, obj, "Get", &res) != JS_TRUE || !JsvalIsFunction(cx, res) )
		return NULL;
	return JSMatrix44Get;
*/
	return NULL;
}

#endif // _JSHELPER_H_
