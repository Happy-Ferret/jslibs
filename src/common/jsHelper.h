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
#include <sys/stat.h>
#ifdef XP_WIN
	#include <io.h>
#endif
#include <fcntl.h>

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
#include <jsdbgapi.h>


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
	JLSemaphoreHandler watchDogSem;
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

ALWAYS_INLINE HostPrivate* GetHostPrivate( JSContext *cx ) { // (TDB) use the runtime to store private data !

	// return JS_GetRuntimePrivate(JS_GetRuntime(cx));
	return (HostPrivate*)cx->runtime->data;
}

ALWAYS_INLINE void SetHostPrivate( JSContext *cx, HostPrivate *hostPrivate ) {

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
	if (unlikely( !(status) )) { \
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

// check with message and argument (printf like)
#define J_CHKM2( status, errorMessage, arg1, arg2 ) do { \
	if (unlikely( !(status) )) { \
		if ( !JS_IsExceptionPending(cx) ) \
			JS_ReportError(cx, (errorMessage IFDEBUG(" (@" J__CODE_LOCATION ")")), (arg1), (arg2)); \
		goto bad; \
	} \
} while(0)


// check and branch to a errorLabel label on error.
#define J_CHKB( status, errorLabel ) do { \
	if (unlikely( !(status) )) { goto errorLabel; } \
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


#define J_SAFE_BEGIN if (unlikely( !_unsafeMode )) {
#define J_SAFE_END }

#define J_UNSAFE_BEGIN if (likely( _unsafeMode )) {
#define J_UNSAFE_END }


#define J_SAFE(code) \
	do { if (unlikely( !_unsafeMode )) {code;} } while(0)

#define J_UNSAFE(code) \
	do { if (likely( _unsafeMode )) {code;} } while(0)


// Reports warnings only in non-unsafeMode.
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
	J_S_ASSERT( !JSVAL_IS_PRIMITIVE(value), J__ERRMSG_UNEXPECTED_TYPE " Object expected." )

#define J_S_ASSERT_ARRAY(value) \
	J_S_ASSERT( JsvalIsArray(cx, value), J__ERRMSG_UNEXPECTED_TYPE " Array expected." )

#define J_S_ASSERT_FUNCTION(value) \
	J_S_ASSERT( JsvalIsFunction(cx, (value)), " Function is expected." )

#define J_S_ASSERT_CLASS(jsObject, jsClass) \
	J_S_ASSERT_1( (jsObject) != NULL && JL_GetClass(jsObject) == (jsClass), J__ERRMSG_INVALID_CLASS " %s expected.", (jsClass)->name )

#define J_S_ASSERT_CLASS_NAME(jsObject, className) \
	J_S_ASSERT_1( IsClassName(jsObject, className), J__ERRMSG_INVALID_CLASS " %s expected.", className )

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
// helper macros to avoid a function call to the jsapi

ALWAYS_INLINE JSClass* JL_GetClass(JSObject *obj) {
	
#ifdef DEBUG
	JS_ASSERT( STOBJ_GET_CLASS(obj) == JS_GetClass(obj) ); // Mozilla JS engine private API behavior has changed.
#endif //DEBUG
	return STOBJ_GET_CLASS(obj); // JS_GET_CLASS(cx, obj);
}

#define J_STRING_LENGTH(jsstr) (JSSTRING_LENGTH(jsstr)) // JS_GetStringLength(jsstr)

// Is string or has jslibs BufferGet interface.
#define J_JSVAL_IS_STRING(val) ( JSVAL_IS_STRING(val) || (!JSVAL_IS_PRIMITIVE(val) && BufferGetInterface(cx, JSVAL_TO_OBJECT(val)) != NULL) )

ALWAYS_INLINE void *JL_GetPrivate(JSContext *cx, JSObject *obj) {

//	return JS_GetPrivate(cx, obj);
	jsval v;
	JS_ASSERT(OBJ_GET_CLASS(cx, obj)->flags & JSCLASS_HAS_PRIVATE);
	v = obj->fslots[JSSLOT_PRIVATE];
#ifdef DEBUG
	JS_ASSERT( (JSVAL_IS_INT(v) ? JSVAL_TO_PRIVATE(v) : NULL) == JS_GetPrivate(cx, obj) ); // Mozilla JS engine private API behavior has changed.
#endif //DEBUG
	if (!JSVAL_IS_INT(v))
		return NULL;
	return JSVAL_TO_PRIVATE(v);
}


ALWAYS_INLINE JSBool JL_SetPrivate(JSContext *cx, JSObject *obj, void *data) {

//	return JS_SetPrivate(cx, obj, data);
	JS_ASSERT(OBJ_GET_CLASS(cx, obj)->flags & JSCLASS_HAS_PRIVATE);
	obj->fslots[JSSLOT_PRIVATE] = PRIVATE_TO_JSVAL(data);
#ifdef DEBUG
	JS_ASSERT( data == JS_GetPrivate(cx, obj) ); // Mozilla JS engine private API behavior has changed.
#endif //DEBUG
	return JS_TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// Helper functions

/* properly
inline bool SwapObjects( JSContext *cx, JSObject *obj1, JSObject *obj2 ) {

	if ( obj1 == NULL || obj2 == NULL )
		return JS_FALSE;

// When JSObject.dslots is not null, JSObject.dslots[-1] records the number of available slots.
//	JSScope *s1 = OBJ_SCOPE(obj1);

//js_ObjectOps.newObjectMap(cx, obj2->map->nrefs, obj2->map->ops, JL_GetClass(cx, obj2), obj1);

	//	JSObjectMap *map1 = obj1->map->ops->newObjectMap(cx, obj2->map->nrefs, obj2->map->ops, JL_GetClass(cx, obj2), obj2);

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


ALWAYS_INLINE JSStackFrame* CurrentStackFrame(JSContext *cx) {

#ifdef DEBUG
	JSStackFrame *fp = NULL;
	JS_ASSERT( JS_FrameIterator(cx, &fp) == js_GetTopStackFrame(cx) ); // Mozilla JS engine private API behavior has changed.
#endif //DEBUG
	return js_GetTopStackFrame(cx);
}


ALWAYS_INLINE unsigned int StackSize(JSContext *cx, JSStackFrame *fp) {

	unsigned int length = 0;
	for ( ; fp; fp = fp->down ) // for ( JSStackFrame *fp = CurrentStackFrame(cx); fp; JS_FrameIterator(cx, &fp) )
		++length;
	return length; // 0 is the first frame
}


ALWAYS_INLINE JSStackFrame *StackFrameByIndex(JSContext *cx, int frameIndex) {

	JSStackFrame *fp = CurrentStackFrame(cx);
	if ( frameIndex >= 0 ) {

		int currentFrameIndex = StackSize(cx, fp)-1;
		if ( frameIndex > currentFrameIndex )
			return NULL;
		// now, select the right frame
		while ( fp && currentFrameIndex > frameIndex ) {

			fp = fp->down; //JS_FrameIterator(cx, &fp);
			--currentFrameIndex;
		}
		return fp;
	}

	while ( fp && frameIndex < 0 ) {

		fp = fp->down; //JS_FrameIterator(cx, &fp);
		++frameIndex;
	}
	return fp;
}

ALWAYS_INLINE bool JsvalIsNaN( JSContext *cx, jsval val ) {

	return JSVAL_IS_DOUBLE( val ) && JSDOUBLE_IS_NaN( *JSVAL_TO_DOUBLE( val ) );
}

ALWAYS_INLINE jsdouble JsvalIsInfinity( JSContext *cx, jsval val ) {

	return JSVAL_IS_DOUBLE( val ) && JSDOUBLE_IS_INFINITE( *JSVAL_TO_DOUBLE( val ) );
}

ALWAYS_INLINE bool JsvalIsPInfinity( JSContext *cx, jsval val ) {

#ifdef DEBUG
	JS_ASSERT( *cx->runtime->jsPositiveInfinity == JS_GetPositiveInfinityValue(cx) ); // Mozilla JS engine private API behavior has changed.
#endif //DEBUG
	return JSVAL_IS_DOUBLE(val) && *JSVAL_TO_DOUBLE(val) == *cx->runtime->jsPositiveInfinity; // JS_GetPositiveInfinityValue
}

ALWAYS_INLINE bool JsvalIsNInfinity( JSContext *cx, jsval val ) {

#ifdef DEBUG
	JS_ASSERT( *cx->runtime->jsNegativeInfinity == JS_GetNegativeInfinityValue(cx) ); // Mozilla JS engine private API behavior has changed.
#endif //DEBUG
	return JSVAL_IS_DOUBLE(val) && *JSVAL_TO_DOUBLE(val) == *cx->runtime->jsNegativeInfinity; // JS_GetNegativeInfinityValue
}


ALWAYS_INLINE bool JsvalIsScript( JSContext *cx, jsval val ) {

	return !JSVAL_IS_PRIMITIVE(val) && JL_GetClass(JSVAL_TO_OBJECT(val)) == &js_ScriptClass;
}

ALWAYS_INLINE bool JsvalIsFunction( JSContext *cx, jsval val ) {

#ifdef DEBUG
	JS_ASSERT( VALUE_IS_FUNCTION(cx, val) == (!JSVAL_IS_PRIMITIVE(val) && JS_ObjectIsFunction(cx, JSVAL_TO_OBJECT(val))) ); // Mozilla JS engine private API behavior has changed.
#endif //DEBUG
	//	return !JSVAL_IS_PRIMITIVE(val) && JS_ObjectIsFunction(cx, JSVAL_TO_OBJECT(val)); // faster than (JS_TypeOfValue(cx, (val)) == JSTYPE_FUNCTION)
	return VALUE_IS_FUNCTION(cx, val);
}

ALWAYS_INLINE bool JsvalIsArray( JSContext *cx, jsval val ) {

	return !JSVAL_IS_PRIMITIVE(val) && JS_IsArrayObject(cx, JSVAL_TO_OBJECT(val));
}


ALWAYS_INLINE bool InheritFrom( JSContext *cx, JSObject *obj, JSClass *clasp ) {

	while ( obj != NULL ) {

		obj = JS_GetPrototype(cx, obj);
		if ( JL_GetClass(obj) == clasp )
			return true;
	}
	return false;
}

ALWAYS_INLINE bool JsvalIsClass( jsval val, JSClass *jsClass ) {

	return !JSVAL_IS_PRIMITIVE(val) && JL_GetClass(JSVAL_TO_OBJECT(val)) == jsClass;
}

ALWAYS_INLINE bool IsClassName( JSObject *obj, const char *name ) {

	return obj != NULL && strcmp(JL_GetClass(obj)->name, name) == 0;
}

/*
ALWAYS_INLINE bool HasProperty( JSContext *cx, JSObject *obj, const char *propertyName ) {

	uintN attr;
	JSBool found;
	JSBool status = JS_GetPropertyAttributes(cx, obj, propertyName, &attr, &found);
	return ( status == JS_TRUE && found != JS_FALSE );
}
*/

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


// If needed, it is up to the caller to protect argv and rval against GC (see JS_PUSH_TEMP_ROOT)
ALWAYS_INLINE JSBool JL_CallFunction( JSContext *cx, JSObject *obj, jsval functionValue, jsval *rval, uintN argc, ... ) {

	va_list ap;
	jsval argv[32]; // argc MUST be <= 32
	jsval rvalTmp;
	J_S_ASSERT( argc <= COUNTOF(argv), "Too many arguments." );
	va_start(ap, argc);
	for ( uintN i = 0; i < argc; i++ )
		argv[i] = va_arg(ap, jsval);
	va_end(ap);
	J_S_ASSERT_FUNCTION( functionValue );
	if ( rval == NULL )
		rval = &rvalTmp;
	J_CHK( JS_CallFunctionValue(cx, obj, functionValue, argc, argv, rval) ); // NULL is NOT supported for &rvalTmp ( last arg of JS_CallFunctionValue )
	return JS_TRUE;
	JL_BAD;
}

// If needed, it is up to the caller to protect argv and rval against GC (see JS_PUSH_TEMP_ROOT)
ALWAYS_INLINE JSBool JL_CallFunctionName( JSContext *cx, JSObject *obj, const char* functionName, jsval *rval, uintN argc, ... ) {

	va_list ap;
	jsval argv[32]; // argc MUST be <= 32
	jsval rvalTmp;
	J_S_ASSERT( argc <= COUNTOF(argv), "Too many arguments." );
	va_start(ap, argc);
	for ( uintN i = 0; i < argc; i++ )
		argv[i] = va_arg(ap, jsval);
	va_end(ap);
	if ( rval == NULL )
		rval = &rvalTmp;
	J_CHK( JS_CallFunctionName(cx, obj, functionName, argc, argv, &rvalTmp) ); // NULL is NOT supported for &rvalTmp ( last arg of JS_CallFunctionValue )
	return JS_TRUE;
	JL_BAD;
}


ALWAYS_INLINE JSBool JL_ValueOf( JSContext *cx, jsval *val, jsval *rval ) {

	if ( !JSVAL_IS_PRIMITIVE(*val) )
		return OBJ_DEFAULT_VALUE(cx, JSVAL_TO_OBJECT(*val), JSTYPE_VOID, rval); // JS_CallFunctionName(cx, JSVAL_TO_OBJECT(*val), "valueOf", 0, NULL, rval)
	*rval = *val;
	return JS_TRUE;
}


// XDR and bytecode compatibility:
//   Backward compatibility is when you run old bytecode on a new engine, and that should work.
//   What you seem to want is forward compatibility, which is new bytecode on an old engine, which is nothing we've ever promised.
// year 2038 bug :
//   Later than midnight, January 1, 1970, and before 19:14:07 January 18, 2038, UTC ( see _stat64 )
// note:
//	You really want to use Script.prototype.thaw and Script.prototype.freeze.  At
//	least imitate their implementations in jsscript.c (script_thaw and
//	script_freeze).  But you might do better to call these via JS_CallFunctionName
//	on your script object.
//
//	/be
ALWAYS_INLINE JSScript* JLLoadScript(JSContext *cx, JSObject *obj, const char *fileName, bool useCompFile, bool saveCompFile) {

	JSScript *script = NULL;

	char compiledFileName[PATH_MAX];
	strcpy( compiledFileName, fileName );
	strcat( compiledFileName, "xdr" );

	struct stat srcFileStat, compFileStat;
	bool hasSrcFile = stat(fileName, &srcFileStat) != -1; // errno == ENOENT
	bool hasCompFile = stat(compiledFileName, &compFileStat) != -1;
	bool compFileUpToDate = ( hasCompFile && !hasSrcFile ) || ( hasCompFile && hasSrcFile && (compFileStat.st_mtime > srcFileStat.st_mtime) ); // true if comp file is up to date or alone

	J_CHKM2( hasSrcFile || hasCompFile, "Unable to load Script, file \"%s\" or \"%s\" not found.", fileName, compiledFileName );

	if ( useCompFile && compFileUpToDate ) {

		int file = open(compiledFileName, O_RDONLY | O_BINARY | O_SEQUENTIAL);
		J_CHKM1( file != -1, "Unable to open file \"%s\" for reading.", compiledFileName );

		int compFileSize = compFileStat.st_size; // filelength(file); ?
		void *data = malloc(compFileSize); // (TBD) free on error
		int readCount = read( file, data, compFileSize ); // here we can use "Memory-Mapped I/O Functions" ( http://developer.mozilla.org/en/docs/NSPR_API_Reference:I/O_Functions#Memory-Mapped_I.2FO_Functions )
		J_CHKM1( readCount != -1 && readCount == compFileSize, "Unable to read the file \"%s\" ", compiledFileName );
		close( file );

		JSXDRState *xdr = JS_XDRNewMem(cx, JSXDR_DECODE);
		J_CHK( xdr );
		JS_XDRMemSetData(xdr, data, compFileSize);
		J_CHK( JS_XDRScript(xdr, &script) );
		// (TBD) manage BIG_ENDIAN here ?
		JS_XDRMemSetData(xdr, NULL, 0);
		JS_XDRDestroy(xdr);
		free(data);
		if ( JS_GetScriptVersion(cx, script) < JS_GetVersion(cx) )
			J_REPORT_WARNING_1("Trying to xdr-decode an old script (%s).", compiledFileName);
		return script; // Done.
	}

// shebang support
	FILE *scriptFile;
	scriptFile = fopen(fileName, "r");
	J_CHKM1( scriptFile != NULL, "Script file \"%s\" cannot be opened.", fileName );

	char s, b;
	s = getc(scriptFile);
	if ( s == '#' ) {

		b = getc(scriptFile);
		if ( b == '!' ) {

			ungetc('/', scriptFile);
			ungetc('/', scriptFile);
		} else {

			ungetc(b, scriptFile);
			ungetc(s, scriptFile);
		}
	} else {

		ungetc(s, scriptFile);
	}

	if ( saveCompFile )
		JS_SetOptions( cx, JS_GetOptions(cx) & ~JSOPTION_COMPILE_N_GO ); // see https://bugzilla.mozilla.org/show_bug.cgi?id=494363

	script = JS_CompileFileHandle(cx, obj, fileName, scriptFile);

	fclose(scriptFile);
	J_CHKM1( script, "Unable to compile the script %s.", fileName );

	if ( !saveCompFile )
		return script; // Done.

	int file;
	file = open(compiledFileName, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY | O_SEQUENTIAL, srcFileStat.st_mode); // (TBD) check the mode
	if ( file == -1 ) // if the file cannot be write, this is not an error ( eg. read-only drive )
		return script;

	JSXDRState *xdr;
	xdr = JS_XDRNewMem(cx, JSXDR_ENCODE);
	J_CHK( xdr );
	J_CHK( JS_XDRScript(xdr, &script) );

	uint32 length;
	void *buf;
	buf = JS_XDRMemGetData(xdr, &length);
	J_CHK( buf );
	// manage BIG_ENDIAN here ?
	J_CHK( write(file, buf, length) != -1 ); // On error, -1 is returned, and errno is set appropriately.
	J_CHK( close(file) == 0 );
	JS_XDRDestroy(xdr);
	return script;

bad:
	// report a warning ?
	return script;
}


///////////////////////////////////////////////////////////////////////////////
// jslibs tools


ALWAYS_INLINE unsigned int SvnRevToInt(const char *svnRev) {

	const char *p = strchr(svnRev, ' ');
	return p ? atol(p+1) : 0;
}

ALWAYS_INLINE bool MaybeRealloc( int requested, int received ) {

	return requested != 0 && (128 * received / requested < 115) && (requested - received > 32); // instead using percent, I use per-128
}


// stores JSClasses that other jslibs modules may rely on.
// note: in jslibs, class->name length is >= 1 (see END_CLASS macro)
ALWAYS_INLINE bool JL_RegisterNativeClass( JSContext *cx, JSClass *jsClass ) {

	QueuePush(&GetHostPrivate(cx)->registredNativeClasses, (void*)jsClass);
	return true;
}

ALWAYS_INLINE JSClass *JL_GetRegistredNativeClass( JSContext *cx, const char *className ) {

	// see js_FindClassObject impl.
	JSClass *jsClass;
	for ( jl::QueueCell *it = jl::QueueBegin(&GetHostPrivate(cx)->registredNativeClasses); it; it = jl::QueueNext(it) ) {

		jsClass = (JSClass*)QueueGetData(it);
		if ( className[0] != jsClass->name[0] ) // optimization (see the note above)
			continue;
		if ( strcmp(className+1, jsClass->name+1) == 0 ) // +1 because [0] has already been tested.
			return jsClass;
	}
	return NULL;
}

ALWAYS_INLINE bool JL_UnregisterNativeClass( JSContext *cx, JSClass *jsClass ) {

	for ( jl::QueueCell *it = jl::QueueBegin(&GetHostPrivate(cx)->registredNativeClasses); it; it = jl::QueueNext(it) ) {

		if ( QueueGetData(it) == (void*)jsClass ) {

			QueueRemoveCell(&GetHostPrivate(cx)->registredNativeClasses, it);
			return true;
		}
	}
	return false;
}

ALWAYS_INLINE void JL_CleanRegisterNativeClasses( JSContext *cx ) {

	jl::QueueDestruct(&GetHostPrivate(cx)->registredNativeClasses);
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


// note: a Blob is either a JSString or a Blob object is the jslang module has been loaded.
ALWAYS_INLINE JSBool J_NewBlob( JSContext *cx, void* buffer, size_t length, jsval *vp ) {

	if (unlikely( length == 0 )) { // Empty Blob must acts like an empty string: !'' == true

		JS_free(cx, buffer); // buffer can be NULL
		*vp = JS_GetEmptyStringValue(cx);
		return JS_TRUE;
	}

	JSClass *blobClass = JL_GetRegistredNativeClass(cx, "Blob"); // don't use static keyword (cf. jstask module)

	if (likely( blobClass != NULL )) { // we have Blob class, jslang is present.

		// A blob/string object can be created without using any jslang/blob.h dependances
		JSObject *blob;
		blob = JS_ConstructObject(cx, blobClass, NULL, NULL); // need to be constructed else Buffer NativeInterface will not be set !
		J_CHK( blob );
		*vp = OBJECT_TO_JSVAL(blob);
		J_CHK( JS_SetReservedSlot(cx, blob, 0, INT_TO_JSVAL( length )) ); // 0 for SLOT_BLOB_LENGTH !!!
		J_CHK( JS_SetPrivate(cx, blob, buffer) ); // blob data
		return JS_TRUE;
	}

	JSString *jsstr;
	jsstr = JS_NewString(cx, (char*)buffer, length); // JS_NewString takes ownership of bytes on success, avoiding a copy; but on error (signified by null return), it leaves bytes owned by the caller. So the caller must free bytes in the error case, if it has no use for them.
	J_CHK( jsstr );
	*vp = STRING_TO_JSVAL(jsstr); // protect from GC.
	
	JSObject *strObj;
	J_CHK( JS_ValueToObject(cx, STRING_TO_JSVAL(jsstr), &strObj) ); // see. OBJ_DEFAULT_VALUE(cx, obj, JSTYPE_OBJECT, &v)
	*vp = OBJECT_TO_JSVAL(strObj);
	return JS_TRUE;

bad:
	JS_free(cx, buffer); // JS_NewString does not free the buffer on error.
	return JS_FALSE;
}


ALWAYS_INLINE JSBool J_NewBlobCopyN( JSContext *cx, const void *data, size_t amount, jsval *vp ) {

	if (unlikely( amount == 0 )) { // Empty Blob must acts like an empty string: !'' == true

		*vp = JS_GetEmptyStringValue(cx);
		return JS_TRUE;
	}
	// possible optimization: if Blob class is not abailable, copy data into JSString's jschar to avoid js_InflateString.
	char *blobBuf = (char*)JS_malloc(cx, amount);
	J_CHK( blobBuf );
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
//	if ( !JSVAL_IS_PRIMITIVE(val) && JL_GetClass(cx, JSVAL_TO_OBJECT(val)) == BlobJSClass(cx) )
//		return true;
}
*/


///////////////////////////////////////////////////////////////////////////////
// jsval convertion functions


// beware: caller should keep a reference to buffer as short time as possible, because it is difficult to protect it from GC.
ALWAYS_INLINE JSBool JsvalToStringAndLength( JSContext *cx, jsval *val, const char** buffer, size_t *size ) {

	if ( JSVAL_IS_STRING(*val) ) { // for string literals

		JSString *str = JSVAL_TO_STRING(*val);
		*buffer = JS_GetStringBytes(str); // JS_GetStringBytes never returns NULL, then J_S_ASSERT( *buffer != NULL, "Invalid string." ); is not needed.
		*size = J_STRING_LENGTH(str);
		return JS_TRUE;
	}
	if ( !JSVAL_IS_PRIMITIVE(*val) ) { // for NIBufferGet support

		NIBufferGet fct = BufferGetNativeInterface(cx, JSVAL_TO_OBJECT(*val));
		if ( fct )
			return fct(cx, JSVAL_TO_OBJECT(*val), buffer, size);
	}
	// and for anything else ...
	JSString *jsstr = JS_ValueToString(cx, *val);
	J_S_ASSERT( jsstr != NULL, J__ERRMSG_STRING_CONVERSION_FAILED );
	*val = STRING_TO_JSVAL(jsstr); // protects *val against GC.
	*size = J_STRING_LENGTH(jsstr);
	*buffer = JS_GetStringBytes(jsstr); // JS_GetStringBytes never returns NULL, then useless to check if (*buffer != NULL).
	return JS_TRUE;
	JL_BAD;
}

ALWAYS_INLINE JSBool JsvalToStringLength( JSContext *cx, jsval val, size_t *length ) {

	if ( JSVAL_IS_STRING(val) ) { // for string literals

		*length = J_STRING_LENGTH( JSVAL_TO_STRING(val) );
		return JS_TRUE;
	}
	if ( !JSVAL_IS_PRIMITIVE(val) ) { // for NIBufferGet support

		NIBufferGet fct = BufferGetNativeInterface(cx, JSVAL_TO_OBJECT(val));
		const char* tmp;
		if ( fct )
			return fct(cx, JSVAL_TO_OBJECT(val), &tmp, length);
	}
	JSString *str = JS_ValueToString(cx, val); // unfortunately, we have to convert to a string to know its length
	J_CHK( str != NULL );
	*length = J_STRING_LENGTH(str);
	return JS_TRUE;
	JL_BAD;
}


ALWAYS_INLINE JSBool JsvalToString( JSContext *cx, jsval *val, const char** buffer ) {

	if ( JSVAL_IS_STRING(*val) ) { // for string literals

		*buffer = JS_GetStringBytes(JSVAL_TO_STRING(*val)); // JS_GetStringBytes never returns NULL, then J_S_ASSERT( *buffer != NULL, "Invalid string." ); is not needed.
		return JS_TRUE;
	}
	if ( !JSVAL_IS_PRIMITIVE(*val) ) { // for NIBufferGet support

		size_t size; //unused
		NIBufferGet fct = BufferGetNativeInterface(cx, JSVAL_TO_OBJECT(*val));
		if ( fct )
			return fct(cx, JSVAL_TO_OBJECT(*val), buffer, &size);
	}
	// and for anything else ...
	JSString *jsstr = JS_ValueToString(cx, *val);
	J_S_ASSERT( jsstr != NULL, J__ERRMSG_STRING_CONVERSION_FAILED );
	*val = STRING_TO_JSVAL(jsstr); // protects *val against GC.
	*buffer = JS_GetStringBytes(jsstr); // JS_GetStringBytes never returns NULL, then useless to check if (*buffer != NULL).
	return JS_TRUE;
	JL_BAD;
}


ALWAYS_INLINE JSBool StringToJsval( JSContext *cx, const char* cstr, jsval *val ) {

	if (unlikely( cstr == NULL )) {

		*val = JSVAL_VOID;
		return JS_TRUE;
	}
	if (unlikely( *cstr == '\0' )) {

		*val = JS_GetEmptyStringValue(cx);
		return JS_TRUE;
	}
	JSString *jsstr = JS_NewStringCopyZ(cx, cstr);
	if (unlikely( jsstr == NULL ))
		J_REPORT_ERROR( "Unable to create the string." );
	*val = STRING_TO_JSVAL(jsstr);
	return JS_TRUE;
	JL_BAD;
}


ALWAYS_INLINE JSBool StringAndLengthToJsval( JSContext *cx, jsval *val, const char* cstr, size_t length ) {

	if (likely( length > 0 )) {

		JSString *jsstr = JS_NewStringCopyN(cx, cstr, length);
		if (unlikely( jsstr == NULL ))
			J_REPORT_ERROR( "Unable to create the string." );
		*val = STRING_TO_JSVAL(jsstr);
		return JS_TRUE;
	}
	if (unlikely( cstr == NULL )) {

		*val = JSVAL_VOID;
		return JS_TRUE;
	}
	*val = JS_GetEmptyStringValue(cx);
	return JS_TRUE;
	JL_BAD;
}


ALWAYS_INLINE JSBool JsvalToInt( JSContext *cx, jsval val, int *i ) {

	if (likely( JSVAL_IS_INT(val) )) {

		*i = JSVAL_TO_INT(val);
		return JS_TRUE;
	}
	if (unlikely( JSVAL_IS_NULL(val) )) {

		*i = 0;
		return JS_TRUE;
	}
	jsdouble d;
	J_CHK( JS_ValueToNumber(cx, val, &d) );
	if (likely( d >= (jsdouble)INT_MIN && d <= (jsdouble)INT_MAX )) {

		*i = (int)d;
		return JS_TRUE;
	}

bad:
	J_REPORT_WARNING( "Unable to convert to an integer." );
	return JS_FALSE;
}


ALWAYS_INLINE JSBool JsvalToUInt( JSContext *cx, jsval val, unsigned int *ui ) {

	if (likely( JSVAL_IS_INT(val) )) {

		int i = JSVAL_TO_INT(val);
		if (likely( i >= 0 )) {

			*ui = (unsigned int)i;
			return JS_TRUE;
		}
		goto bad;
	}
	if (unlikely( JSVAL_IS_NULL(val) )) {

		*ui = 0;
		return JS_TRUE;
	}
	jsdouble d;
	J_CHK( JS_ValueToNumber(cx, val, &d) );
	if (likely( d >= (jsdouble)0 && d <= (jsdouble)UINT_MAX )) {

		*ui = (unsigned int)d;
		return JS_TRUE;
	}

bad:
	J_REPORT_WARNING( "Unable to convert to an unsigned integer." );
	return JS_FALSE;
}


ALWAYS_INLINE JSBool IntToJsval( JSContext *cx, int i, jsval *val ) {

	if (likely( INT_FITS_IN_JSVAL(i) )) {

		*val = INT_TO_JSVAL(i);
		return JS_TRUE;
	}
	J_CHK( JS_NewNumberValue(cx, i, val) );
	return JS_TRUE;

bad:
	J_REPORT_WARNING( "Unable to convert the integer." );
	return JS_FALSE;
}


ALWAYS_INLINE JSBool UIntToJsval( JSContext *cx, unsigned int ui, jsval *val ) {

	if (likely( ui <= JSVAL_INT_MAX )) {

		*val = INT_TO_JSVAL(ui);
		return JS_TRUE;
	}
	J_CHK( JS_NewNumberValue(cx, ui, val) );
	return JS_TRUE;

bad:
	J_REPORT_WARNING( "Unable to convert the unsigned integer." );
	return JS_FALSE;
}


ALWAYS_INLINE JSBool BoolToJsval( JSContext *cx, bool b, jsval *val ) {

	*val = b ? JSVAL_TRUE : JSVAL_FALSE;
	return JS_TRUE;
}


ALWAYS_INLINE JSBool JsvalToBool( JSContext *cx, const jsval val, bool *b ) {

	if (likely( JSVAL_IS_BOOLEAN(val) )) {

		*b = (JSVAL_TO_BOOLEAN(val) == JS_TRUE);
		return JS_TRUE;
	}
	JSBool tmp;
	J_CHK( JS_ValueToBoolean(cx, val, &tmp) );
	*b = (tmp == JS_TRUE);
	return JS_TRUE;

bad:
	J_REPORT_WARNING( "Unable to convert to a boolean." );
	return JS_FALSE;
}


ALWAYS_INLINE JSBool JsvalToFloat( JSContext *cx, jsval val, float *f ) {

	if (likely( JSVAL_IS_DOUBLE(val) )) {

		*f = *JSVAL_TO_DOUBLE(val);
		return JS_TRUE;
	}
	jsdouble tmp;
	J_CHK( JS_ValueToNumber( cx, val, &tmp ) );
	*f = tmp;
	return JS_TRUE;

bad:
	J_REPORT_WARNING( "Unable to convert to a real." );
	return JS_FALSE;
}

ALWAYS_INLINE JSBool FloatToJsval( JSContext *cx, float f, jsval *val ) {

	J_CHK( JS_NewNumberValue(cx, f, val) );
	return JS_TRUE;
	JL_BAD;
}


ALWAYS_INLINE JSBool JsvalToDouble( JSContext *cx, jsval val, double *d ) {

	if (likely( JSVAL_IS_DOUBLE(val) )) {

		*d = *JSVAL_TO_DOUBLE(val);
		return JS_TRUE;
	}
	jsdouble tmp;
	J_CHK( JS_ValueToNumber( cx, val, &tmp ) );
	*d = tmp;
	return JS_TRUE;

bad:
	J_REPORT_WARNING( "Unable to convert to a real." );
	return JS_FALSE;
}

ALWAYS_INLINE JSBool DoubleToJsval( JSContext *cx, double d, jsval *val ) {

	J_CHK( JS_NewNumberValue(cx, d, val) );
	return JS_TRUE;
	JL_BAD;
}



///////////////////////////////////////////////////////////////////////////////
// vector convertion functions


ALWAYS_INLINE JSBool IntVectorToJsval( JSContext *cx, int *vector, size_t length, jsval *val ) {

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


ALWAYS_INLINE JSBool JsvalToIntVector( JSContext *cx, jsval val, int *vector, size_t maxLength, size_t *currentLength ) {

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


ALWAYS_INLINE JSBool JsvalToUIntVector( JSContext *cx, jsval val, unsigned int *vector, size_t maxLength, size_t *currentLength ) {

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


ALWAYS_INLINE JSBool DoubleVectorToJsval( JSContext *cx, const double *vector, size_t length, jsval *val ) {

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

ALWAYS_INLINE JSBool FloatVectorToJsval( JSContext *cx, const float *vector, size_t length, jsval *val ) {

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


ALWAYS_INLINE JSBool JsvalToFloatVector( JSContext *cx, jsval val, float *vector, size_t maxLength, size_t *currentLength ) {

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


ALWAYS_INLINE JSBool JsvalToDoubleVector( JSContext *cx, jsval val, double *vector, size_t maxLength, size_t *currentLength ) {

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



ALWAYS_INLINE JSBool SetPropertyString( JSContext *cx, JSObject *obj, const char *propertyName, const char *str ) {

	jsval val;
	J_CHK( StringToJsval(cx, str, &val) );
	J_CHKM( JS_DefineProperty(cx, obj, propertyName, val, NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT ), "Unable to set the property." ); // Doc. http://developer.mozilla.org/en/docs/JS_DefineUCProperty
	return JS_TRUE;
	JL_BAD;
}

ALWAYS_INLINE JSBool GetPropertyString( JSContext *cx, JSObject *obj, const char *propertyName, const char **str ) {

	jsval val;
	J_CHKM1( JS_GetProperty(cx, obj, propertyName, &val), "Unable to read the property %s.", propertyName );
	J_CHK( JsvalToString(cx, &val, str) );
	return JS_TRUE;
	JL_BAD;
}


ALWAYS_INLINE JSBool SetPropertyBool( JSContext *cx, JSObject *obj, const char *propertyName, bool b ) {

	jsval val;
	J_CHK( BoolToJsval(cx, b, &val) );
	J_CHKM( JS_SetProperty(cx, obj, propertyName, &val), "Unable to set the property." );
	return JS_TRUE;
	JL_BAD;
}

ALWAYS_INLINE JSBool GetPropertyBool( JSContext *cx, JSObject *obj, const char *propertyName, bool *b ) {

	jsval val;
	J_CHKM1( JS_GetProperty(cx, obj, propertyName, &val), "Unable to read the property %s.", propertyName );
	J_CHK( JsvalToBool(cx, val, b) );
	return JS_TRUE;
	JL_BAD;
}


ALWAYS_INLINE JSBool SetPropertyInt( JSContext *cx, JSObject *obj, const char *propertyName, int intVal ) {

	jsval val;
	J_CHK( IntToJsval(cx, intVal, &val) );
	J_CHKM( JS_SetProperty(cx, obj, propertyName, &val), "Unable to set the property." ); // Doc. http://developer.mozilla.org/en/docs/JS_DefineUCProperty
	return JS_TRUE;
	JL_BAD;
}

ALWAYS_INLINE JSBool GetPropertyInt( JSContext *cx, JSObject *obj, const char *propertyName, int *intVal ) {

	jsval val;
	J_CHKM1( JS_GetProperty(cx, obj, propertyName, &val), "Unable to read the property %s.", propertyName );
	J_CHK( JsvalToInt(cx, val, intVal) );
	return JS_TRUE;
	JL_BAD;
}


ALWAYS_INLINE JSBool SetPropertyUInt( JSContext *cx, JSObject *obj, const char *propertyName, unsigned int ui ) {

	jsval val;
	J_CHK( UIntToJsval(cx, ui, &val) );
	J_CHKM( JS_SetProperty(cx, obj, propertyName, &val), "Unable to set the property." ); // Doc. http://developer.mozilla.org/en/docs/JS_DefineUCProperty
	return JS_TRUE;
	JL_BAD;
}

ALWAYS_INLINE JSBool GetPropertyUInt( JSContext *cx, JSObject *obj, const char *propertyName, unsigned int *ui ) {

	jsval val;
	J_CHKM1( JS_GetProperty(cx, obj, propertyName, &val), "Unable to read the property %s.", propertyName ); // try. OBJ_GET_PROPERTY(...
	J_CHK( JsvalToUInt(cx, val, ui) );
	return JS_TRUE;
	JL_BAD;
}



///////////////////////////////////////////////////////////////////////////////
//

ALWAYS_INLINE JSBool ExceptionSetScriptLocation( JSContext *cx, JSObject *obj ) {

	JSStackFrame *fp = NULL;
	do {

		JS_FrameIterator(cx, &fp);
	} while ( fp && !JS_GetFramePC(cx, fp) );

	J_CHK( fp );
	JSScript *script;
	script = JS_GetFrameScript(cx, fp);
	J_CHK( script );
	const char *filename;
	filename = JS_GetScriptFilename(cx, script);
	J_CHK( filename );
	int lineno;
	lineno = JS_PCToLineNumber(cx, script, JS_GetFramePC(cx, fp));

	jsval tmp;
	J_CHK( StringToJsval(cx, filename, &tmp) );
	J_CHK( JS_SetProperty(cx, obj, "fileName", &tmp) );
	J_CHK( IntToJsval(cx, lineno, &tmp) );
	J_CHK( JS_SetProperty(cx, obj, "lineNumber", &tmp) );

	return JS_TRUE;
	JL_BAD;
}



///////////////////////////////////////////////////////////////////////////////
// Serialization

typedef JSXDRState* Serialized;

ALWAYS_INLINE bool IsSerializable( jsval val ) {

	if ( JSVAL_IS_PRIMITIVE(val) )
		return true;
	JSClass *cl = JL_GetClass(JSVAL_TO_OBJECT(val));
	return cl->xdrObject != NULL;
}

ALWAYS_INLINE void SerializerCreate( Serialized *xdr ) {

	*xdr = NULL;
}

ALWAYS_INLINE void SerializerFree( Serialized *xdr ) {

	if ( *xdr != NULL ) {

		JS_XDRDestroy(*xdr);
//		JS_XDRMemSetData(*xdr, NULL, 0);
		*xdr = NULL;
	}
}

ALWAYS_INLINE bool SerializerIsEmpty( const Serialized *xdr ) {

	return *xdr == NULL;
}

ALWAYS_INLINE JSBool SerializeJsval( JSContext *cx, Serialized *xdr, jsval *val ) {

	if ( *xdr != NULL )
		SerializerFree(xdr);
	*xdr = JS_XDRNewMem(cx, JSXDR_ENCODE);
	J_S_ASSERT( *xdr != NULL, "Unable to create the serializer." );
	J_CHK( JS_XDRValue(*xdr, val) );
	return JS_TRUE;
	JL_BAD;
}

ALWAYS_INLINE JSBool UnserializeJsval( JSContext *cx, const Serialized *xdr, jsval *rval ) {

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


ALWAYS_INLINE jsid StringToJsid( JSContext *cx, const char *cstr ) {

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

	return JS_DefineProperty(cx, obj, name, JSVAL_FALSE, NULL, (JSPropertyOp)-1, JSPROP_READONLY | JSPROP_PERMANENT );
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
	J_CHKM( OBJ_LOOKUP_PROPERTY(cx, obj, iid, obj2p, &prop), "Unable to get the native interface."); //(TBD) use JS_LookupPropertyById or JS_GetPropertyById

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

	JSTempValueRooter tvr;
	JS_PUSH_SINGLE_TEMP_ROOT(cx, JSVAL_NULL, &tvr); // needed to protect the returned value.

	J_CHK( IntToJsval(cx, *amount, &tvr.u.value) );
	J_CHKM( JS_CallFunctionName(cx, obj, "Read", 1, &tvr.u.value, &tvr.u.value), "Read() function not found.");

	if ( JSVAL_IS_VOID(tvr.u.value) ) {

		*amount = 0;
		JS_POP_TEMP_ROOT(cx, &tvr);
		return JS_TRUE;
	}

	const char *tmpBuf;
	size_t size;
	J_CHK( JsvalToStringAndLength(cx, &tvr.u.value, &tmpBuf, &size) );
	*amount = J_MIN(size, *amount);
	memcpy(buffer, tmpBuf, *amount);
	JS_POP_TEMP_ROOT(cx, &tvr);
	return JS_TRUE;
bad:
	JS_POP_TEMP_ROOT(cx, &tvr);
	return JS_FALSE;
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

	JSTempValueRooter tvr;
	JS_PUSH_SINGLE_TEMP_ROOT(cx, JSVAL_NULL, &tvr); // needed to protect the returned value.

	J_CHKM( JS_CallFunctionName(cx, obj, "Get", 0, NULL, &tvr.u.value), "Get() function not found."); // do not use toString() !? no !
	J_CHK( JsvalToStringAndLength(cx, &tvr.u.value, buffer, size) ); // (TBD) GC warning, when tvr.u.value will be no more protected, the buffer will be unprotected.

	JS_POP_TEMP_ROOT(cx, &tvr);
	return JS_TRUE;
bad:
	JS_POP_TEMP_ROOT(cx, &tvr);
	return JS_FALSE;
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


	JS_PUSH_SINGLE_TEMP_ROOT(cx, rval, &tvr);
	&tvr.u.value
	...

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
