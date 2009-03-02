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
#include "../common/queue.h"

//#include <float.h>
#include <cstring>
#include <stdarg.h>

// JavaScript engine includes
#include <jsnum.h>
#include <jscntxt.h>
#include <jsinterp.h>
#include <jsarena.h>
#include <jsfun.h>
#include <jsscope.h>
#include <jsobj.h>
#include <jsstr.h>
#include <jsscript.h>
#include <jsxdrapi.h>


extern bool _unsafeMode;

#ifdef DEBUG
	#define IFDEBUG(expr) expr
#else
	#define IFDEBUG(expr)
#endif // DEBUG

typedef int (*HostOutput)( void *privateData, const char *buffer, size_t length );

struct HostPrivate {

	void *privateData;
	size_t maybeGCInterval;
	JLThreadHandler watchDogThread;
	bool unsafeMode;
	HostOutput hostStdOut;
	HostOutput hostStdErr;
	jl::Queue moduleList;
	jl::Queue registredNativeClasses;
	jsid Matrix44GetId;
	jsid BufferGetId;
	jsid StreamReadId;
	int camelCase;
};

inline HostPrivate* GetHostPrivate( JSContext *cx ) { // (TDB) use the runtime to store private data !

	// return JS_GetRuntimePrivate(JS_GetRuntime(cx));
	return (HostPrivate*)cx->runtime->data;
}

inline void SetHostPrivate( JSContext *cx, HostPrivate *hostPrivate ) {

	// JS_SetRuntimePrivate(JS_GetRuntime(cx), hostPrivate);
	cx->runtime->data = (void*)hostPrivate;
}

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
#define J_ARG_ISDEF( n ) ( argc >= (n) && !JSVAL_IS_VOID( argv[(n)-1] ) )
// same for fast native
#define J_FARG_ISDEF( n ) ( argc >= (n) && !JSVAL_IS_VOID(JS_ARGV(cx,vp)[(n)-1]) )

// is the current obj (this)
#define J_OBJ (obj)
// same for fast native
#define J_FOBJ (argc, JS_THIS_OBJECT(cx, vp))

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

#define JL_BAD bad:return(JS_FALSE)

// check: used to forward an error.
#define J_CHK( status ) do { \
	if (unlikely(!(status))) { \
		goto bad; \
	} \
} while(0)

// check with message: if status is false, a js exception is rised if it is not already pending.
#define J_CHKM( status, errorMessage ) do { \
	if (unlikely( !(status) )) { \
		if ( !JS_IsExceptionPending(cx) ) \
			JS_ReportError(cx, (errorMessage IFDEBUG(" (@" J__CODE_LOCATION ")"))); \
		goto bad; \
	} \
} while(0)

// check with message and argument (printf like)
#define J_CHKM1( status, errorMessage, arg ) do { \
	if (unlikely( !(status) )) { \
		if ( !JS_IsExceptionPending(cx) ) \
			JS_ReportError(cx, (errorMessage IFDEBUG(" (@" J__CODE_LOCATION ")")), (arg)); \
		goto bad; \
	} \
} while(0)



// check and branch to a errorLabel label on error.
#define J_CHKB( status, errorLabel ) do { \
	if (unlikely(!(status))) { goto errorLabel; } \
} while(0)


// check and branch to a errorLabel label on error AND report an error if no exception is pending.
#define J_CHKBM( status, errorLabel, errorMessage ) do { \
	if (unlikely( !(status) )) { \
		if ( !JS_IsExceptionPending(cx) ) \
			JS_ReportError(cx, (errorMessage IFDEBUG(" (@" J__CODE_LOCATION ")"))); \
		goto errorLabel; \
	} \
} while(0)

// same that J_CHKBM with a additional argument (printf like)
#define J_CHKBM1( status, errorLabel, errorMessage, arg ) do { \
	if (unlikely( !(status) )) { \
		if ( !JS_IsExceptionPending(cx) ) \
			JS_ReportError(cx, (errorMessage IFDEBUG(" (@" J__CODE_LOCATION ")")), (arg)); \
		goto errorLabel; \
	} \
} while(0)


inline bool JsvalIsClass(JSContext *cx, jsval val, JSClass *jsClass) {

	return JSVAL_IS_OBJECT(val) && !JSVAL_IS_NULL(val) && JS_GET_CLASS(cx, JSVAL_TO_OBJECT(val)) == (jsClass);
}

#define J_JSVAL_IS_CLASS(val, jsClass) \
	JsvalIsClass(cx, val, jsClass)


#define J_SAFE_BEGIN if (unlikely( !_unsafeMode )) {
#define J_SAFE_END }

#define J_UNSAFE_BEGIN if (likely( _unsafeMode )) {
#define J_UNSAFE_END }


#define J_SAFE(code) \
	do { if (unlikely( !_unsafeMode )) {code;} } while(0)

#define J_UNSAFE(code) \
	do { if (likely( _unsafeMode )) {code;} } while(0)


// Reports warnings. May be disabled in unsafemode
#define J_REPORT_WARNING(errorMessage) \
	do { if (unlikely(!_unsafeMode)) JS_ReportWarning( cx, (errorMessage IFDEBUG(" (@" J__CODE_LOCATION ")")) ); } while(0)

#define J_REPORT_WARNING_1(errorMessage, arg) \
	do { if (unlikely(!_unsafeMode)) JS_ReportWarning( cx, (errorMessage IFDEBUG(" (@" J__CODE_LOCATION ")")), (arg) ); } while(0)

#define J_REPORT_WARNING_2(errorMessage, arg1, arg2) \
	do { if (unlikely(!_unsafeMode)) JS_ReportWarning( cx, (errorMessage IFDEBUG(" (@" J__CODE_LOCATION ")")), (arg1), (arg2) ); } while(0)


// Reports a fatal errors, script must stop as soon as possible.
#define J_REPORT_ERROR(errorMessage) \
	do { JS_ReportError( cx, (errorMessage IFDEBUG(" (@" J__CODE_LOCATION ")")) ); goto bad; } while(0)

#define J_REPORT_ERROR_1(errorMessage, arg) \
	do { JS_ReportError( cx, (errorMessage IFDEBUG(" (@" J__CODE_LOCATION ")")), (arg) ); goto bad; } while(0)

#define J_REPORT_ERROR_2(errorMessage, arg1, arg2) \
	do { JS_ReportError( cx, (errorMessage IFDEBUG(" (@" J__CODE_LOCATION ")")), (arg1), (arg2) ); goto bad; } while(0)



// J_S_ stands for (J)slibs _ (S)afemode _ and mean that these macros will only be meaningful when unsafemode is false (see jslibs unsafemode).

#define J_S_ASSERT( condition, errorMessage ) \
	do { if (unlikely( !_unsafeMode && !(condition) )) { JS_ReportError( cx, errorMessage IFDEBUG(" (" #condition " @" J__CODE_LOCATION ")") ); goto bad; } } while(0)

#define J_S_ASSERT_1( condition, errorMessage, arg ) \
	do { if (unlikely( !_unsafeMode && !(condition) )) { JS_ReportError( cx, errorMessage IFDEBUG(" (" #condition " @" J__CODE_LOCATION ")"), (arg) ); goto bad; } } while(0)

#define J_S_ASSERT_2( condition, errorMessage, arg1, arg2 ) \
	do { if (unlikely( !_unsafeMode && !(condition) )) { JS_ReportError( cx, errorMessage IFDEBUG(" (" #condition " @" J__CODE_LOCATION ")"), (arg1), (arg2) ); goto bad; } } while(0)


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
	J_S_ASSERT( J_JSVAL_IS_STRING(value), J__ERRMSG_UNEXPECTED_TYPE " String expected." )

#define J_S_ASSERT_OBJECT(value) \
	J_S_ASSERT( JSVAL_IS_OBJECT(value) && !JSVAL_IS_NULL(value), J__ERRMSG_UNEXPECTED_TYPE " Object expected." )

#define J_S_ASSERT_ARRAY(value) \
	J_S_ASSERT( JsvalIsArray(cx, value), J__ERRMSG_UNEXPECTED_TYPE " Array expected." )

#define J_S_ASSERT_FUNCTION(value) \
	J_S_ASSERT( JsvalIsFunction(cx, (value)), " Function is expected." )

#define J_S_ASSERT_CLASS(jsObject, jsClass) \
	J_S_ASSERT_1( (jsObject) != NULL && JS_GET_CLASS(cx, jsObject) == (jsClass), J__ERRMSG_INVALID_CLASS "%s expected.", (jsClass)->name )

#define J_S_ASSERT_CLASS_NAME(jsObject, className) \
	J_S_ASSERT_1( IsClassName(cx, jsObject, className), J__ERRMSG_INVALID_CLASS "%s expected.", className )

#define J_S_ASSERT_THIS_CLASS() \
	J_S_ASSERT_CLASS(obj, _class)

#define J_S_ASSERT_CONSTRUCTING() \
	J_S_ASSERT( JS_IsConstructing(cx) == JS_TRUE, J__ERRMSG_NEED_CONSTRUCTION )

#define J_S_ASSERT_INITIALIZED(pointer) \
	J_S_ASSERT( (pointer) != NULL, J__ERRMSG_UNINITIALIZED )

#define J_S_ASSERT_RESOURCE(resourcePointer) \
	J_S_ASSERT( (resourcePointer) != NULL, J__ERRMSG_INVALID_RESOURCE )

#define J_S_ASSERT_ALLOC(pointer) \
	if (unlikely( (pointer) == NULL )) { J_REPORT_WARNING( J__ERRMSG_OUT_OF_MEMORY ); JS_ReportOutOfMemory(cx); return JS_FALSE; } // This does not cause an exception to be thrown.

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
	JL_BAD;
}
*/

inline unsigned int SvnRevToInt(const char *svnRev) {

	const char *p = strchr(svnRev, ' ');
	return p ? atol(p+1) : 0;
}


inline void *JL_GetPrivate(JSContext *cx, JSObject *obj) {

	jsval v;
	JS_ASSERT(OBJ_GET_CLASS(cx, obj)->flags & JSCLASS_HAS_PRIVATE);
	v = obj->fslots[JSSLOT_PRIVATE];
	if (!JSVAL_IS_INT(v))
		return NULL;
	return JSVAL_TO_PRIVATE(v);
}


inline JSBool JL_SetPrivate(JSContext *cx, JSObject *obj, void *data) {

	JS_ASSERT(OBJ_GET_CLASS(cx, obj)->flags & JSCLASS_HAS_PRIVATE);
	obj->fslots[JSSLOT_PRIVATE] = PRIVATE_TO_JSVAL(data);
	return JS_TRUE;
}


inline jsdouble IsInfinity( JSContext *cx, jsval val ) {

	return JSVAL_IS_DOUBLE( val ) && JSDOUBLE_IS_INFINITE( *JSVAL_TO_DOUBLE( val ) );
}


inline bool IsNaN( JSContext *cx, jsval val ) {

	return JSVAL_IS_DOUBLE( val ) && JSDOUBLE_IS_NaN( *JSVAL_TO_DOUBLE( val ) );
}

inline jsdouble PInfinity( JSContext *cx ) {

	static jsdouble pinf = 0; // it's safe to use static keyword
	if ( pinf == 0 )
		pinf = *JSVAL_TO_DOUBLE(JS_GetPositiveInfinityValue(cx));
	return pinf;
}

inline bool IsPInfinity( JSContext *cx, jsval val ) {

	//	return JS_GetPositiveInfinityValue(cx) == val;
	return JSVAL_IS_DOUBLE(val) && *JSVAL_TO_DOUBLE(val) == PInfinity(cx);
}

inline jsdouble NInfinity( JSContext *cx ) {

	static jsdouble ninf = 0; // it's safe to use static keyword
	if ( ninf == 0 )
		ninf = *JSVAL_TO_DOUBLE(JS_GetNegativeInfinityValue(cx));
	return ninf;
}

inline bool IsNInfinity( JSContext *cx, jsval val ) {

//	return JS_GetNegativeInfinityValue(cx) == val;
	return JSVAL_IS_DOUBLE(val) && *JSVAL_TO_DOUBLE(val) == NInfinity(cx);
}


inline bool JsvalIsFunction( JSContext *cx, jsval val ) {

	return !JSVAL_IS_PRIMITIVE(val) && JS_ObjectIsFunction(cx, JSVAL_TO_OBJECT(val)); // faster than (JS_TypeOfValue(cx, (val)) == JSTYPE_FUNCTION)
}


inline bool JsvalIsArray( JSContext *cx, jsval val ) {

	return !JSVAL_IS_PRIMITIVE(val) && JS_IsArrayObject(cx, JSVAL_TO_OBJECT(val));
}


inline bool InheritFrom( JSContext *cx, JSObject *obj, JSClass *clasp ) {

	while( obj != NULL ) {

		obj = JS_GetPrototype(cx, obj);
		if ( JS_GET_CLASS(cx, obj) == clasp )
			return true;
	}
	return false;
}


inline bool IsClassName( JSContext *cx, JSObject *obj, const char *name ) {

	return strcmp(JS_GET_CLASS(cx, (obj))->name, (name)) == 0;
}

inline bool HasProperty( JSContext *cx, JSObject *obj, const char *propertyName ) {

	uintN attr;
	JSBool found;
	JSBool status = JS_GetPropertyAttributes(cx, obj, propertyName, &attr, &found);
	return ( status == JS_TRUE && found != JS_FALSE );
}


/*
inline JSBool GetNamedPrivate( JSContext *cx, JSObject *obj, const char *name, void **pv ) {

	jsval tmp;
	if ( JS_GetProperty(cx, obj, name, &tmp) == JS_FALSE )
		return JS_FALSE;
	*pv = JSVAL_IS_VOID( tmp ) ? NULL : JSVAL_TO_PRIVATE(tmp);
	return JS_TRUE;
	JL_BAD;
}
*/

/*
inline JSBool SetNamedPrivate( JSContext *cx, JSObject *obj, const char *name, const void *pv ) {

//	J_SAFE(	if ( (int)pv % 2 ) return JS_FALSE; ); // check if *vp is 2-byte aligned
	if ( JS_DefineProperty(cx, obj, name, PRIVATE_TO_JSVAL(pv), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT ) == JS_FALSE )
		return JS_FALSE;
	return JS_TRUE;
	JL_BAD;
}
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
	JL_BAD;
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
	JL_BAD;
}

inline JSBool JL_ValueOf( JSContext *cx, jsval *val, jsval *rval ) {

	if ( JSVAL_IS_OBJECT(*val) ) {

		J_CHK( OBJ_DEFAULT_VALUE(cx, JSVAL_TO_OBJECT(*val), JSTYPE_VOID, rval) );
		//J_CHK( JS_CallFunctionName(cx, JSVAL_TO_OBJECT(*val), "valueOf", 0, NULL, rval) );
	} else {

		*rval = *val;
	}
	return JS_TRUE;
	JL_BAD;
}

inline bool MaybeRealloc( int requested, int received ) {

	return requested != 0 && (128 * received / requested < 115) && (requested - received > 32); // instead using percent, I use per-128
}


///////////////////////////////////////////////////////////////////////////////
//

inline void JL_RegisterNativeClass( JSContext *cx, JSClass *jsClass ) {

	QueuePush(&GetHostPrivate(cx)->registredNativeClasses, (void*)jsClass);
}

inline JSClass *JL_GetRegistredNativeClass( JSContext *cx, const char *className ) {

	JSClass *jsClass;
	for ( jl::QueueCell *it = jl::QueueBegin(&GetHostPrivate(cx)->registredNativeClasses); it; it = jl::QueueNext(it) ) {
		
		jsClass = (JSClass*)QueueGetData(it);
		if ( strcmp(className, jsClass->name) == 0 )
			return jsClass;
	}
	return NULL;
}

inline bool JL_UnregisterNativeClass( JSContext *cx, JSClass *jsClass ) {

	for ( jl::QueueCell *it = jl::QueueBegin(&GetHostPrivate(cx)->registredNativeClasses); it; it = jl::QueueNext(it) ) {

		if ( QueueGetData(it) == (void*)jsClass ) {
			QueueRemoveCell(&GetHostPrivate(cx)->registredNativeClasses, it);
			return true;
		}
	}
	return false;
}

inline JSClass *GetGlobalClassByName( JSContext *cx, const char *className ) {
	
	return JL_GetRegistredNativeClass(cx, className);
}

/*
// The following function wil only works if the class is defined in the global namespace (say global object)
inline JSClass *GetGlobalClassByName( JSContext *cx, const char *className ) {

	// see.  js_FindClassObject(cx, NULL, INT_TO_JSID(JSProto_StopIteration), &v)) / JS_GetClassObject

	JSObject *globalObj = JS_GetGlobalObject(cx);
	if ( globalObj == NULL )
		return NULL;
	jsval classConstructor;
	if ( JS_LookupProperty(cx, globalObj, className, &classConstructor) != JS_TRUE )
		return NULL;
	if ( JsvalIsFunction(cx, classConstructor) ) {

		JSFunction *fun = JS_ValueToFunction(cx, classConstructor);
		if ( fun == NULL )
			return NULL;
		if ( !FUN_SLOW_NATIVE(fun) )
			return NULL;
		return fun->u.n.u.clasp; // return fun->u.n.clasp; // (TBD) replace this by a jsapi.h call and remove dependency to jsarena.h and jsfun.h
	} else
	if ( JSVAL_IS_OBJECT(classConstructor) ) {

		return OBJ_GET_CLASS(cx, JSVAL_TO_OBJECT(classConstructor));
	}
	return NULL;
}
*/

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

	static JSClass *blobClass = NULL; // it's safe to use static keyword because JSClass do not depend on the rt or cx.
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
	JL_BAD;
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
	JL_BAD;
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

	J_S_ASSERT( str != NULL, J__ERRMSG_STRING_CONVERSION_FAILED );
	*length = J_STRING_LENGTH(str);
	return JS_TRUE;
	JL_BAD;
}


inline JSBool JsvalToString( JSContext *cx, jsval *val, const char** buffer ) {

	size_t size; //unused
	return JsvalToStringAndLength( cx, val, buffer, &size );
}


inline JSBool StringToJsval( JSContext *cx, const char* cstr, jsval *val ) {

	if ( cstr == NULL ) {

		*val = JSVAL_VOID;
		return JS_TRUE;
	}
	if (likely( *cstr != '\0' )) {

		JSString *jsstr = JS_NewStringCopyZ(cx, cstr);
		if ( jsstr == NULL )
			J_REPORT_ERROR( "Unable to create the string." );
		*val = STRING_TO_JSVAL(jsstr);
		return JS_TRUE;
	} else {

		*val = JS_GetEmptyStringValue(cx);
		return JS_TRUE;
	}
	JL_BAD;
}


inline JSBool StringAndLengthToJsval( JSContext *cx, jsval *val, const char* cstr, size_t length ) {

	if ( length > 0 ) {

		JSString *jsstr = JS_NewStringCopyN(cx, cstr, length);
		if ( jsstr == NULL )
			J_REPORT_ERROR( "Unable to create the string." );
		*val = STRING_TO_JSVAL(jsstr);
		return JS_TRUE;
	} else {

		*val = JS_GetEmptyStringValue(cx);
		return JS_TRUE;
	}
	JL_BAD;
}


inline JSBool SetPropertyString( JSContext *cx, JSObject *obj, const char *propertyName, const char *str ) {

	jsval val;
	J_CHK( StringToJsval(cx, str, &val) );
	J_CHKM( JS_DefineProperty(cx, obj, propertyName, val, NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT ), "Unable to set the property." ); // Doc. http://developer.mozilla.org/en/docs/JS_DefineUCProperty
	return JS_TRUE;
	JL_BAD;
}

inline JSBool GetPropertyString( JSContext *cx, JSObject *obj, const char *propertyName, const char **str ) {

	jsval val;
	J_CHKM1( JS_GetProperty(cx, obj, propertyName, &val), "Unable to read the property %s.", propertyName );
	J_CHK( JsvalToString(cx, &val, str) );
	return JS_TRUE;
	JL_BAD;
}


inline JSBool JsvalToInt( JSContext *cx, jsval val, int *i ) {

	if ( JSVAL_IS_INT(val) ) {

		*i = JSVAL_TO_INT(val);
		return JS_TRUE;
	}

	if ( JSVAL_IS_NULL(val) ) {

		*i = 0;
		return JS_TRUE;
	}

	jsdouble d;
	J_CHK( JS_ValueToNumber(cx, val, &d) );

	if ( d > -2147483649.0 && 2147483648.0 > d ) {

		*i = (int)d;
		return JS_TRUE;
	}
bad:
	J_REPORT_WARNING( "Unable to convert to an integer." );
	return JS_FALSE;
}


inline JSBool JsvalToUInt( JSContext *cx, jsval val, unsigned int *ui ) {

	if ( JSVAL_IS_INT(val) ) {

		int i = JSVAL_TO_INT(val);

		if ( i >= 0 ) {

			*ui = (unsigned int)i;
			return JS_TRUE;
		}
		goto bad;
	}

	if ( JSVAL_IS_NULL(val) ) {

		*ui = 0;
		return JS_TRUE;
	}

	jsdouble d;
	J_CHK( JS_ValueToNumber(cx, val, &d) );

	if ( d >= 0 && 4294967296.0 > d  ) {

		*ui = (unsigned int)d;
		return JS_TRUE;
	}
bad:
	J_REPORT_WARNING( "Unable to convert to an unsigned integer." );
	return JS_FALSE;
}


inline JSBool IntToJsval( JSContext *cx, int i, jsval *val ) {

	if ( INT_FITS_IN_JSVAL(i) ) {

		*val = INT_TO_JSVAL(i);
		return JS_TRUE;
	} else {

		J_CHK( JS_NewNumberValue(cx, i, val) );
		return JS_TRUE;
	}
bad:
	J_REPORT_WARNING( "Unable to convert to an integer." );
	return JS_FALSE;
}


inline JSBool UIntToJsval( JSContext *cx, unsigned int ui, jsval *val ) {

	if ( ui <= JSVAL_INT_MAX ) {

		*val = INT_TO_JSVAL(ui);
		return JS_TRUE;
	} else {

		J_CHK( JS_NewNumberValue(cx, ui, val) );
		return JS_TRUE;
	}
bad:
	J_REPORT_WARNING( "Unable to convert to an unsigned integer." );
	return JS_FALSE;
}


inline JSBool BoolToJsval( JSContext *cx, bool b, jsval *val ) {

	*val = b ? JSVAL_TRUE : JSVAL_FALSE;
	return JS_TRUE;
}


inline JSBool JsvalToBool( JSContext *cx, const jsval val, bool *b ) {

	if ( JSVAL_IS_BOOLEAN(val) ) {

		*b = (JSVAL_TO_BOOLEAN(val) == JS_TRUE);
		return JS_TRUE;
	} else {

		JSBool tmp;
		J_CHK( JS_ValueToBoolean( cx, val, &tmp ) );
		*b = (tmp == JS_TRUE);
		return JS_TRUE;
	}
bad:
	J_REPORT_WARNING( "Unable to convert to a boolean." );
	return JS_FALSE;
}


inline JSBool SetPropertyBool( JSContext *cx, JSObject *obj, const char *propertyName, bool b ) {

	jsval val;
	J_CHK( BoolToJsval(cx, b, &val) );
	J_CHKM( JS_DefineProperty(cx, obj, propertyName, val, NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT ), "Unable to set the property." ); // Doc. http://developer.mozilla.org/en/docs/JS_DefineUCProperty
	return JS_TRUE;
	JL_BAD;
}

inline JSBool GetPropertyBool( JSContext *cx, JSObject *obj, const char *propertyName, bool *b ) {

	jsval val;
	J_CHKM1( JS_GetProperty(cx, obj, propertyName, &val), "Unable to read the property %s.", propertyName );
	J_CHK( JsvalToBool(cx, val, b) );
	return JS_TRUE;
	JL_BAD;
}



inline JSBool JsvalToFloat( JSContext *cx, jsval val, float *f ) {

	if ( JSVAL_IS_DOUBLE(val) ) {

		*f = *JSVAL_TO_DOUBLE(val);
		return JS_TRUE;
	} else {

		jsdouble tmp;
		J_CHK( JS_ValueToNumber( cx, val, &tmp ) );
		*f = tmp;
		return JS_TRUE;
	}

bad:
	J_REPORT_WARNING( "Unable to convert to a real." );
	return JS_FALSE;
}

inline JSBool JsvalToDouble( JSContext *cx, jsval val, double *d ) {

	if ( JSVAL_IS_DOUBLE(val) ) {

		*d = *JSVAL_TO_DOUBLE(val);
		return JS_TRUE;
	} else {

		jsdouble tmp;
		J_CHK( JS_ValueToNumber( cx, val, &tmp ) );
		*d = tmp;
		return JS_TRUE;
	}

bad:
	J_REPORT_WARNING( "Unable to convert to a real." );
	return JS_FALSE;
}


inline JSBool FloatToJsval( JSContext *cx, float f, jsval *val ) {

	J_CHK( JS_NewNumberValue(cx, f, val) );
	return JS_TRUE;
	JL_BAD;
}

inline JSBool DoubleToJsval( JSContext *cx, double d, jsval *val ) {

	J_CHK( JS_NewNumberValue(cx, d, val) );
	return JS_TRUE;
	JL_BAD;
}



inline JSBool SetPropertyInt( JSContext *cx, JSObject *obj, const char *propertyName, int intVal ) {

	jsval val;
	J_CHK( IntToJsval(cx, intVal, &val) );
	J_CHKM( JS_DefineProperty(cx, obj, propertyName, val, NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT ), "Unable to set the property." ); // Doc. http://developer.mozilla.org/en/docs/JS_DefineUCProperty
	return JS_TRUE;
	JL_BAD;
}

inline JSBool GetPropertyInt( JSContext *cx, JSObject *obj, const char *propertyName, int *intVal ) {

	jsval val;
	J_CHKM1( JS_GetProperty(cx, obj, propertyName, &val), "Unable to read the property %s.", propertyName );
	J_CHK( JsvalToInt(cx, val, intVal) );
	return JS_TRUE;
	JL_BAD;
}


inline JSBool SetPropertyUInt( JSContext *cx, JSObject *obj, const char *propertyName, unsigned int ui ) {

	jsval val;
	J_CHK( UIntToJsval(cx, ui, &val) );
	J_CHKM( JS_DefineProperty(cx, obj, propertyName, val, NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT ), "Unable to set the property." ); // Doc. http://developer.mozilla.org/en/docs/JS_DefineUCProperty
	return JS_TRUE;
	JL_BAD;
}

inline JSBool GetPropertyUInt( JSContext *cx, JSObject *obj, const char *propertyName, unsigned int *ui ) {

	jsval val;
	J_CHKM1( JS_GetProperty(cx, obj, propertyName, &val), "Unable to read the property %s.", propertyName ); // try. OBJ_GET_PROPERTY(...
	J_CHK( JsvalToUInt(cx, val, ui) );
	return JS_TRUE;
	JL_BAD;
}


inline JSBool IntVectorToJsval( JSContext *cx, int *vector, size_t length, jsval *val ) {

	JSObject *arrayObj = JS_NewArrayObject(cx, length, NULL);
	J_CHKM( arrayObj, "Unable to create the Array." );
	*val = OBJECT_TO_JSVAL(arrayObj);
	jsval tmp;
	for ( size_t i = 0; i < length; i++ ) {

		J_CHK( IntToJsval(cx, vector[i], &tmp) );
		J_CHK( JS_SetElement(cx, arrayObj, i, &tmp) );
	}
	return JS_TRUE;
	JL_BAD;
}


inline JSBool JsvalToIntVector( JSContext *cx, jsval val, int *vector, size_t maxLength, size_t *currentLength ) {

	J_S_ASSERT_ARRAY(val);
	JSObject *arrayObj;
	arrayObj = JSVAL_TO_OBJECT(val);
	jsuint tmp;
	J_CHK( JS_GetArrayLength(cx, arrayObj, &tmp) );
	*currentLength = tmp;
	maxLength = J_MIN( *currentLength, maxLength );
	jsval item;
	for ( jsuint i = 0; i < maxLength; i++ ) {

		J_CHK( JS_GetElement(cx, arrayObj, i, &item) );
		J_CHK( JsvalToInt(cx, item, &vector[i]) );
	}
	return JS_TRUE;
	JL_BAD;
}

inline JSBool JsvalToUIntVector( JSContext *cx, jsval val, unsigned int *vector, size_t maxLength, size_t *currentLength ) {

	J_S_ASSERT_ARRAY(val);
	JSObject *arrayObj;
	arrayObj = JSVAL_TO_OBJECT(val);
	jsuint tmp;
	J_CHK( JS_GetArrayLength(cx, arrayObj, &tmp) );
	*currentLength = tmp;
	maxLength = J_MIN( *currentLength, maxLength );
	jsval item;
	for ( jsuint i = 0; i < maxLength; i++ ) {

		J_CHK( JS_GetElement(cx, arrayObj, i, &item) );
		J_CHK( JsvalToUInt(cx, item, &vector[i]) );
	}
	return JS_TRUE;
	JL_BAD;
}


inline JSBool DoubleVectorToJsval( JSContext *cx, const double *vector, size_t length, jsval *val ) {

	JSObject *arrayObj = JS_NewArrayObject(cx, length, NULL);
	J_CHKM( arrayObj, "Unable to create the Array." );
	*val = OBJECT_TO_JSVAL(arrayObj);
	jsval tmp;
	for ( size_t i = 0; i < length; i++ ) {

		J_CHK( DoubleToJsval(cx, vector[i], &tmp) );
		J_CHK( JS_SetElement(cx, arrayObj, i, &tmp) );
	}
	return JS_TRUE;
	JL_BAD;
}

inline JSBool FloatVectorToJsval( JSContext *cx, const float *vector, size_t length, jsval *val ) {

	JSObject *arrayObj = JS_NewArrayObject(cx, length, NULL);
	J_CHKM( arrayObj, "Unable to create the Array." );
	*val = OBJECT_TO_JSVAL(arrayObj);
	jsval tmp;
	for ( size_t i = 0; i < length; i++ ) {

		J_CHK( FloatToJsval(cx, vector[i], &tmp) );
		J_CHK( JS_SetElement(cx, arrayObj, i, &tmp) );
	}
	return JS_TRUE;
	JL_BAD;
}


inline JSBool JsvalToFloatVector( JSContext *cx, jsval val, float *vector, size_t maxLength, size_t *currentLength ) {

	J_S_ASSERT_ARRAY(val);
	JSObject *arrayObj;
	arrayObj = JSVAL_TO_OBJECT(val);
	jsuint tmp;
	J_CHK( JS_GetArrayLength(cx, arrayObj, &tmp) );
	*currentLength = tmp;
	maxLength = J_MIN( *currentLength, maxLength );
	jsval item;
	for ( jsuint i = 0; i < maxLength; i++ ) {

		J_CHK( JS_GetElement(cx, arrayObj, i, &item) );
		J_CHK( JsvalToFloat(cx, item, &vector[i]) );
	}
	return JS_TRUE;
	JL_BAD;
}


inline JSBool JsvalToDoubleVector( JSContext *cx, jsval val, double *vector, size_t maxLength, size_t *currentLength ) {

	J_S_ASSERT_ARRAY(val);
	JSObject *arrayObj;
	arrayObj = JSVAL_TO_OBJECT(val);
	jsuint tmp;
	J_CHK( JS_GetArrayLength(cx, arrayObj, &tmp) );
	*currentLength = tmp;
	maxLength = J_MIN( *currentLength, maxLength );
	jsval item;
	for ( jsuint i = 0; i < maxLength; i++ ) {

		J_CHK( JS_GetElement(cx, arrayObj, i, &item) );
		J_CHK( JsvalToDouble(cx, item, &vector[i]) );
	}
	return JS_TRUE;
	JL_BAD;
}


///////////////////////////////////////////////////////////////////////////////
// Serialization

typedef JSXDRState* Serialized;

inline bool IsSerializable( jsval val ) {
	
	if ( JSVAL_IS_PRIMITIVE(val) )
		return true;
	JSClass *cl = JS_GetClass(JSVAL_TO_OBJECT(val));
	return cl->xdrObject != NULL;
}

inline void SerializerCreate( Serialized *xdr ) {

	*xdr = NULL;
}

inline void SerializerFree( Serialized *xdr ) {

	if ( *xdr != NULL ) {

		JS_XDRMemSetData(*xdr, NULL, 0);
		JS_XDRDestroy(*xdr);
		*xdr = NULL;
	}
}

inline bool SerializerIsEmpty( const Serialized *xdr ) {

	return *xdr == NULL;
}

inline JSBool SerializeJsval( JSContext *cx, Serialized *xdr, jsval *val ) {

	if ( *xdr != NULL )
		SerializerFree(xdr);
	*xdr = JS_XDRNewMem(cx, JSXDR_ENCODE);
	J_S_ASSERT( *xdr != NULL, "Unable to create the serializer." );
	J_CHK( JS_XDRValue(*xdr, val) );
	return JS_TRUE;
	JL_BAD;
}

inline JSBool UnserializeJsval( JSContext *cx, const Serialized *xdr, jsval *rval ) {

	JSXDRState *xdrDecoder = JS_XDRNewMem(cx, JSXDR_DECODE);
	J_S_ASSERT( xdrDecoder != NULL, "Unable to create the unserializer." );
	uint32 length;
	void *data;
	data = JS_XDRMemGetData(*xdr, &length);
	JS_XDRMemSetData(xdrDecoder, data, length);
	J_CHK( JS_XDRValue(xdrDecoder, rval) );
	JS_XDRMemSetData(xdrDecoder, NULL, 0);
	JS_XDRDestroy(xdrDecoder);
	return JS_TRUE;
	JL_BAD;
}


inline jsid StringToJsid( JSContext *cx, const char *cstr ) {

	jsid tmp;
	JSString *jsstr = JS_InternString(cx, cstr);
	J_CHK( jsstr != NULL );
	J_CHK( JS_ValueToId(cx, STRING_TO_JSVAL(jsstr), &tmp) );
	return tmp;
bad:
	return 0;
}


///////////////////////////////////////////////////////////////////////////////
// NativeInterface

inline JSBool ReserveNativeInterface( JSContext *cx, JSObject *obj, const char *name ) {

	J_CHK( JS_DefineProperty(cx, obj, name, JSVAL_FALSE, NULL, (JSPropertyOp)-1, JSPROP_READONLY | JSPROP_PERMANENT ) );
	return JS_TRUE;
	JL_BAD;
}

inline JSBool SetNativeInterface( JSContext *cx, JSObject *obj, const char *name, void *nativeFct ) {

	if ( nativeFct != NULL ) {

		J_CHK( JS_DefineProperty(cx, obj, name, JSVAL_TRUE, NULL, (JSPropertyOp)nativeFct, JSPROP_READONLY | JSPROP_PERMANENT ) );
	} else {

		J_CHK( JS_DeleteProperty(cx, obj, name) );
		J_CHK( ReserveNativeInterface(cx, obj, name) );
	}
	return JS_TRUE;
	JL_BAD;
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
	JL_BAD;
}


///////////////////////////////////////////////////////////////////////////////
// NativeInterface StreamRead

inline JSBool JSStreamRead( JSContext *cx, JSObject *obj, char *buffer, size_t *amount ) {

	jsval tmpVal, rval;
	IntToJsval(cx, *amount, &tmpVal);
	J_CHKM( JS_CallFunctionName(cx, obj, "Read", 1, &tmpVal, &rval), "Read() function not found.");

	if ( JSVAL_IS_VOID( rval ) ) {

		*amount = 0;
		return JS_TRUE;
	}

	const char *tmpBuf;
	size_t size;
	J_CHK( JsvalToStringAndLength(cx, &rval, &tmpBuf, &size) );
	*amount = J_MIN(size, *amount);
	memcpy(buffer, tmpBuf, *amount);
	return JS_TRUE;
	JL_BAD;
}

inline JSBool ReserveStreamReadInterface( JSContext *cx, JSObject *obj ) {

	return ReserveNativeInterface(cx, obj, "_NI_StreamRead" );
}

inline JSBool SetStreamReadInterface( JSContext *cx, JSObject *obj, NIStreamRead pFct ) {

	return SetNativeInterface( cx, obj, "_NI_StreamRead", (void*)pFct );
}

inline NIStreamRead StreamReadNativeInterface( JSContext *cx, JSObject *obj ) {

	jsid propId = GetHostPrivate(cx)->StreamReadId;
	if ( !propId ) {

		propId = StringToJsid(cx, "_NI_StreamRead");
		if ( !propId )
			return NULL;
		GetHostPrivate(cx)->StreamReadId = propId;
	}

	void *streamRead;
	JSObject *obj2;
	if ( propId == 0 || GetNativeInterface( cx, obj, &obj2, propId, &streamRead ) != JS_TRUE )
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
	J_CHK( JsvalToStringAndLength(cx, &rval, buffer, size) ); // (TBD) GC protect rval !!!
	return JS_TRUE;
	JL_BAD;
}

inline JSBool ReserveBufferGetInterface( JSContext *cx, JSObject *obj ) {

	return ReserveNativeInterface(cx, obj, "_NI_BufferGet" );
}

inline JSBool SetBufferGetInterface( JSContext *cx, JSObject *obj, NIBufferGet pFct ) {

	return SetNativeInterface( cx, obj, "_NI_BufferGet", (void*)pFct );
}

inline NIBufferGet BufferGetNativeInterface( JSContext *cx, JSObject *obj ) {

	jsid propId = GetHostPrivate(cx)->BufferGetId;
	if ( !propId ) {

		propId = StringToJsid(cx, "_NI_BufferGet");
		if ( !propId )
			return NULL;
		GetHostPrivate(cx)->BufferGetId = propId;
	}
	void *fct;
	JSObject *obj2;
	if ( propId == 0 || GetNativeInterface( cx, obj, &obj2, propId, &fct ) != JS_TRUE )
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
	JL_BAD;
}
*/

inline JSBool ReserveMatrix44GetInterface( JSContext *cx, JSObject *obj ) {

	return ReserveNativeInterface(cx, obj, "_NIMatrix44Get" );
}

inline JSBool SetMatrix44GetInterface( JSContext *cx, JSObject *obj, NIMatrix44Get pFct ) {

	return SetNativeInterface( cx, obj, "_NI_Matrix44Get", (void*)pFct );
}

inline NIMatrix44Get Matrix44GetNativeInterface( JSContext *cx, JSObject *obj ) {

	jsid propId = GetHostPrivate(cx)->Matrix44GetId;
	if ( !propId ) {

		propId = StringToJsid(cx, "_NI_Matrix44Get");
		if ( !propId )
			return NULL;
		GetHostPrivate(cx)->Matrix44GetId = propId;
	}
	void *fct;
	JSObject *obj2;
	if ( propId == 0 || GetNativeInterface( cx, obj, &obj2, propId, &fct ) != JS_TRUE )
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
