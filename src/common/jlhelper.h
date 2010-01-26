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

#include "jlalloc.h"
#include "jlplatform.h"
#include <jsapi.h>

typedef JSBool (*NIStreamRead)( JSContext *cx, JSObject *obj, char *buffer, unsigned int *amount );
typedef JSBool (*NIBufferGet)( JSContext *cx, JSObject *obj, const char **buffer, unsigned int *size );
typedef JSBool (*NIMatrix44Get)( JSContext *cx, JSObject *obj, float **pm );

inline NIBufferGet BufferGetNativeInterface( JSContext *cx, JSObject *obj );
inline NIBufferGet BufferGetInterface( JSContext *cx, JSObject *obj );

#include "queue.h"

#include <cstring>
#include <stdarg.h>
#include <sys/stat.h>
#ifdef XP_WIN
	#include <io.h>
#endif
#include <fcntl.h>

// JavaScript engine

#ifdef _MSC_VER
#pragma warning( push, 0 )
#endif // _MSC_VER

#include <jscntxt.h>
#include <jsscope.h>
#include <jsxdrapi.h>

#ifdef _MSC_VER
#pragma warning( pop )
#endif // _MSC_VER


#ifdef DEBUG
	#define IFDEBUG(expr) expr
#else
	#define IFDEBUG(expr)
#endif // DEBUG


extern bool _unsafeMode;

#define JL_SAFE_BEGIN if (unlikely( !_unsafeMode )) {
#define JL_SAFE_END }

#define JL_UNSAFE_BEGIN if (likely( _unsafeMode )) {
#define JL_UNSAFE_END }


#define JL_SAFE(code) \
JL_MACRO_BEGIN \
	if (unlikely( !_unsafeMode )) {code;} \
JL_MACRO_END

#define JL_UNSAFE(code) \
JL_MACRO_BEGIN \
	if (likely( _unsafeMode )) {code;} \
JL_MACRO_END

// see JSAtomState struct in jsatom.h
#define JL_ATOMJSID(CX, NAME) ATOM_TO_JSID(CX->runtime->atomState.NAME##Atom)

#define JLID_SPEC(name) JLID_##name
enum {
	JLID_SPEC( stdout ),
	JLID_SPEC( stderr ),
	JLID_SPEC( global ),
	JLID_SPEC( arguments ),
	JLID_SPEC( unsafeMode ),
	JLID_SPEC( _revision ),
	JLID_SPEC( _configuration ),
	JLID_SPEC( scripthostpath ),
	JLID_SPEC( scripthostname ),
	JLID_SPEC( isfirstinstance ),
	JLID_SPEC( bootstrapScript ),
	JLID_SPEC( _NI_BufferGet ),
	JLID_SPEC( _NI_StreamRead ),
	JLID_SPEC( _NI_Matrix44Get ),
	JLID_SPEC( Get ),
	JLID_SPEC( Read ),
	JLID_SPEC( name ),
	JLID_SPEC( id ),
	LAST_JSID // see HostPrivate::ids[]
};
#undef JLID_SPEC


#include <jlhostprivate.h>


ALWAYS_INLINE HostPrivate* GetHostPrivate( JSContext *cx ) {

//	return (HostPrivate*)JS_GetRuntimePrivate(JS_GetRuntime(cx));
//	return reinterpret_cast<HostPrivate*>(cx->runtime->data);
	return (HostPrivate*)cx->runtime->data;
}

ALWAYS_INLINE void SetHostPrivate( JSContext *cx, HostPrivate *hostPrivate ) {

//	JS_SetRuntimePrivate(JS_GetRuntime(cx), hostPrivate);
	cx->runtime->data = (void*)hostPrivate;
}

ALWAYS_INLINE unsigned char ModulePrivateHash( const uint32_t moduleId ) {

	return ((uint8_t*)&moduleId)[0] ^ ((uint8_t*)&moduleId)[1] ^ ((uint8_t*)&moduleId)[2] ^ ((uint8_t*)&moduleId)[3] << 1;
}

ALWAYS_INLINE bool SetModulePrivate( JSContext *cx, const uint32_t moduleId, void *modulePrivate ) {

	JL_ASSERT( moduleId != 0 );
	unsigned char id = ModulePrivateHash(moduleId);
	HostPrivate::ModulePrivate *mpv = GetHostPrivate(cx)->modulePrivate;
	while ( mpv[id].moduleId != 0 ) { // assumes that modulePrivate struct is init to 0

		if ( mpv[id].moduleId == moduleId )
			return false; // module private already exist or moduleId not unique
		++id; // uses unsigned char overflow
	}
	mpv[id].moduleId = moduleId;
	mpv[id].privateData = modulePrivate;
	return true;
}

ALWAYS_INLINE void* GetModulePrivate( JSContext *cx, uint32_t moduleId ) {

	JL_ASSERT( moduleId != 0 );
	unsigned char id = ModulePrivateHash(moduleId);
	HostPrivate::ModulePrivate *mpv = GetHostPrivate(cx)->modulePrivate;
	while ( mpv[id].moduleId != moduleId ) {

		++id; // uses unsigned char overflow
	}
	return mpv[id].privateData;
}
// example of use: static uint32_t moduleId = 'dbug'; SetModulePrivate(cx, moduleId, mpv);


ALWAYS_INLINE jsid GetPrivateJsid( JSContext *cx, int index, const char *name ) {

	jsid id = GetHostPrivate(cx)->ids[index];
	if (likely( id != 0 ))
		return id;
	JSString *jsstr = JS_InternString(cx, name);
	if ( jsstr == NULL )
		return 0;
	if ( JS_ValueToId(cx, STRING_TO_JSVAL(jsstr), &id) != JS_TRUE )
		return 0;
	GetHostPrivate(cx)->ids[index] = id;
	return id;
}

#ifdef DEBUG
#define JLID_NAME(name) (jl_unused(JLID_##name), #name)
#else
#define JLID_NAME(name) (#name)
#endif // DEBUG

#define JLID(cx, name) GetPrivateJsid(cx, JLID_##name, #name)
// example of use: jsid cfg = JLID(cx, _configuration); char *name = JLID_NAME(_configuration);


///////////////////////////////////////////////////////////////////////////////
// helper macros

// BEWARE: the following helper macros are only valid inside a JS Native function definition !

#define JL_ARGC (argc)

// returns the ARGument Vector
#define JL_ARGV (argv)
// same for fast native (cf. http://developer.mozilla.org/en/docs/JS_ARGV)
#define JL_FARGV (JS_ARGV(cx,vp))

// returns the ARGument n
#define JL_ARG( n ) (argv[(n)-1])
// same for fast native (cf. http://developer.mozilla.org/en/docs/JS_ARGV)
#define JL_FARG( n ) (JS_ARGV(cx,vp)[(n)-1])

// returns the ARGument n or undefined if it does not exist
#define JL_SARG( n ) ( argc >= (n) ? argv[(n)-1] : JSVAL_VOID )
// same for fast native (cf. http://developer.mozilla.org/en/docs/JS_ARGV)
#define JL_FSARG( n ) ( argc >= (n) ? JS_ARGV(cx,vp)[(n)-1] : JSVAL_VOID )

// returns true if the ARGument n is DEFined
#define JL_ARG_ISDEF( n ) ( argc >= (n) && !JSVAL_IS_VOID( argv[(n)-1] ) )
// same for fast native
#define JL_FARG_ISDEF( n ) ( argc >= (n) && !JSVAL_IS_VOID(JS_ARGV(cx,vp)[(n)-1]) )

// is the current obj (this)
#define JL_OBJ (obj)
// same for fast native
#define JL_FOBJ (argc=argc, JS_THIS_OBJECT(cx, vp))

// the return value
#define JL_RVAL (rval)
// same for fast native
#define JL_FRVAL (&JS_RVAL(cx, vp))

#define JL_BAD bad:return(JS_FALSE)


///////////////////////////////////////////////////////////////////////////////
// Error management

enum JLErrNum {
#define MSG_DEF(name, number, count, exception, format) \
	name = number,
#include "jlerrors.msg"
#undef MSG_DEF
	JLErrLimit
};

// Reports a fatal errors, script must stop as soon as possible.
#define JL_REPORT_ERROR( errorMessage, ... ) \
JL_MACRO_BEGIN \
	JS_ReportError( cx, (errorMessage IFDEBUG(" (@" JL_CODE_LOCATION ")")), ##__VA_ARGS__ ); \
	goto bad; \
JL_MACRO_END

// Report a jslibs error. see jlerrors.msg
#define JL_REPORT_ERROR_NUM( cx, num, ... ) \
JL_MACRO_BEGIN \
	HostPrivate *hpv = GetHostPrivate(cx); \
	if ( hpv != NULL && hpv->errorCallback != NULL ) \
		JS_ReportErrorNumber(cx, hpv->errorCallback, NULL, (num), ##__VA_ARGS__); \
	else \
		JS_ReportError(cx, "undefined nessage %d", (num)); \
	goto bad; \
JL_MACRO_END


// Reports warnings (optimisation: check non-unsafeMode. see ErrorReporter() in host.cpp).
#define JL_REPORT_WARNING( errorMessage, ... ) \
JL_MACRO_BEGIN \
	if (unlikely( !_unsafeMode )) JS_ReportWarning( cx, (errorMessage IFDEBUG(" (@" JL_CODE_LOCATION ")")), ##__VA_ARGS__ ); \
JL_MACRO_END

#define JL_REPORT_WARNING_NUM( cx, num, ... ) \
JL_MACRO_BEGIN \
	if (unlikely( !_unsafeMode )) { \
		HostPrivate *hpv = GetHostPrivate(cx); \
		if ( hpv != NULL && hpv->errorCallback != NULL ) \
			JS_ReportErrorFlagsAndNumber(cx, JSREPORT_WARNING, hpv->errorCallback, NULL, (num), ##__VA_ARGS__); \
		else \
			JS_ReportWarning(cx, "undefined nessage %d", (num)); \
	} \
JL_MACRO_END


// check: used to forward an error. // (TBD) try ultra-safe mode at compile-time: #define JL_CHK( status ) (status)
#define JL_CHK( status ) \
JL_MACRO_BEGIN \
	if (unlikely( !(status) )) { \
/*		IFDEBUG( fprintf(stderr, "DEBUG: JL_CHK failed" IFDEBUG(" (@" JL_CODE_LOCATION ")") "\n") ); */ \
		goto bad; \
	} \
JL_MACRO_END


// check with message: if status is false, a js exception is rised if it is not already pending.
// (Support for variadic macros was introduced in Visual C++ 2005)
#define JL_CHKM( status, errorMessage, ... ) \
JL_MACRO_BEGIN \
	if (unlikely( !(status) )) { \
		if ( !JS_IsExceptionPending(cx) ) \
			JS_ReportError(cx, (errorMessage IFDEBUG(" (@" JL_CODE_LOCATION ")")), ##__VA_ARGS__); \
		goto bad; \
	} \
JL_MACRO_END


// check and branch to a errorLabel label on error.
#define JL_CHKB( status, errorLabel ) \
JL_MACRO_BEGIN \
	if (unlikely( !(status) )) { \
/*	IFDEBUG( fprintf(stderr, "DEBUG: JL_CHKB(,%s) failed" IFDEBUG(" (@" JL_CODE_LOCATION ")") "\n", #errorLabel ) ); */ \
		goto errorLabel; \
	} \
JL_MACRO_END


// check and branch to a errorLabel label on error AND report an error if no exception is pending.
#define JL_CHKBM( status, errorLabel, errorMessage, ... ) \
JL_MACRO_BEGIN \
	if (unlikely( !(status) )) { \
		if ( !JS_IsExceptionPending(cx) ) \
			JS_ReportError(cx, (errorMessage IFDEBUG(" (@" JL_CODE_LOCATION ")")), ##__VA_ARGS__); \
		goto errorLabel; \
	} \
JL_MACRO_END


// JL_S_ stands for (J)s(L)ibs _ (S)afemode _ and mean that these macros will only be meaningful when _unsafeMode is false. (see jslibs unsafemode).

#define JL_S_ASSERT( condition, errorMessage, ... ) \
JL_MACRO_BEGIN \
	JL_SAFE_BEGIN \
		if (unlikely( !(condition) )) { \
			JS_ReportError( cx, errorMessage IFDEBUG(" (" #condition " @" JL_CODE_LOCATION ")"), ##__VA_ARGS__ ); \
			goto bad; \
		} \
	JL_SAFE_END \
JL_MACRO_END

#define JL_S_ASSERT_ERROR_NUM( condition, errorNum, ... ) \
JL_MACRO_BEGIN \
	JL_SAFE_BEGIN \
		if (unlikely( !(condition) )) \
			JL_REPORT_ERROR_NUM( cx, (errorNum),  ##__VA_ARGS__ ); \
	JL_SAFE_END \
JL_MACRO_END

#define JL_S_ASSERT_ALLOC(pointer) \
JL_MACRO_BEGIN \
	JL_SAFE_BEGIN \
		if ( (pointer) == NULL ) { \
			JS_ReportOutOfMemory(cx); \
			goto bad; \
		} \
	JL_SAFE_END \
JL_MACRO_END

#define JL_S_ASSERT_ARG_MIN(minCount) \
	JL_S_ASSERT_ERROR_NUM( (argc) >= (minCount), JLSMSG_TOO_FEW_ARGUMENTS, #minCount );

#define JL_S_ASSERT_ARG_MAX(maxCount) \
	JL_S_ASSERT_ERROR_NUM( (argc) <= (maxCount), JLSMSG_TOO_MANY_ARGUMENTS, #maxCount );

#define JL_S_ASSERT_ARG_RANGE(minCount, maxCount) \
JL_MACRO_BEGIN \
	JL_S_ASSERT_ARG_MIN(minCount); \
	JL_S_ASSERT_ARG_MAX(maxCount); \
JL_MACRO_END

#define JL_S_ASSERT_ARG(count) \
	JL_S_ASSERT_ARG_RANGE(count, count);

#define JL_S_ASSERT_DEFINED(value) \
	JL_S_ASSERT_ERROR_NUM( !JSVAL_IS_VOID(value), JLSMSG_NEED_DEFINED );

// jsType: JSTYPE_VOID, JSTYPE_OBJECT, JSTYPE_FUNCTION, JSTYPE_STRING, JSTYPE_NUMBER, JSTYPE_BOOLEAN, JSTYPE_NULL, JSTYPE_XML, JSTYPE_LIMIT
#define JL_S_ASSERT_TYPE(value, jsType) \
	JL_S_ASSERT_ERROR_NUM( JS_TypeOfValue(cx, (value)) == (jsType), JLSMSG_EXPECT_TYPE, (#jsType)+7 );

#define JS_S_ASSERT_CONVERT(condition, typeName) \
	JL_S_ASSERT_ERROR_NUM( (condition), JLSMSG_FAIL_TO_CONVERT_TO, typeName );

#define JL_S_ASSERT_BOOLEAN(value) \
	JL_S_ASSERT_ERROR_NUM( JSVAL_IS_BOOLEAN(value) || (!JSVAL_IS_PRIMITIVE(value) && JL_GetClass(JSVAL_TO_OBJECT(value)) == JL_GetStandardClass(cx, JSProto_Boolean)), JLSMSG_EXPECT_TYPE, "boolean" );

#define JL_S_ASSERT_LOSSLESS_INT(value) \
	JL_S_ASSERT_ERROR_NUM( JSVAL_IS_INT(value) || (JSVAL_IS_DOUBLE(value) && *JSVAL_TO_DOUBLE(value) < MAX_INTDOUBLE && *JSVAL_TO_DOUBLE(value) > -MAX_INTDOUBLE), JLSMSG_EXPECT_TYPE, "smaller integer" );

#define JL_S_ASSERT_NUMBER(value) \
	JL_S_ASSERT_ERROR_NUM( JSVAL_IS_NUMBER(value) || (!JSVAL_IS_PRIMITIVE(value) && JL_GetClass(JSVAL_TO_OBJECT(value)) == JL_GetStandardClass(cx, JSProto_Number)), JLSMSG_EXPECT_TYPE, "number" );

#define JL_S_ASSERT_INT(value) \
	JL_S_ASSERT_ERROR_NUM( JSVAL_IS_INT(value), JLSMSG_EXPECT_TYPE, "integer" );

#define JL_S_ASSERT_STRING(value) \
	JL_S_ASSERT_ERROR_NUM( JsvalIsData(cx, (value)), JLSMSG_EXPECT_TYPE, "string or blob" );

#define JL_S_ASSERT_OBJECT(value) \
	JL_S_ASSERT_ERROR_NUM( !JSVAL_IS_PRIMITIVE(value), JLSMSG_EXPECT_TYPE, "object" );

#define JL_S_ASSERT_ARRAY(value) \
	JL_S_ASSERT_ERROR_NUM( JsvalIsArray(cx, (value)), JLSMSG_EXPECT_TYPE, "array" );

#define JL_S_ASSERT_FUNCTION(value) \
	JL_S_ASSERT_ERROR_NUM( JsvalIsFunction(cx, (value)), JLSMSG_EXPECT_TYPE, "function" );

#define JL_S_ASSERT_CLASS(jsObject, jsClass) \
	JL_S_ASSERT_ERROR_NUM( (jsObject) != NULL && JL_GetClass(jsObject) == (jsClass), JLSMSG_EXPECT_TYPE, (jsClass)->name );

#define JL_S_ASSERT_THIS_CLASS() \
	JL_S_ASSERT_CLASS(obj, JL_THIS_CLASS)

#define JL_S_ASSERT_INHERITANCE(jsObject, jsClass) \
	JL_S_ASSERT_ERROR_NUM( JL_InheritFrom(cx, (jsObject), (jsClass)), JLSMSG_INVALID_INHERITANCE, (jsClass)->name );

#define JL_S_ASSERT_THIS_INSTANCE() \
	JL_S_ASSERT_ERROR_NUM( JL_InheritFrom(cx, (obj), JL_THIS_CLASS) && (obj) != JL_THIS_PROTOTYPE, JLSMSG_INVALID_INHERITANCE, JL_THIS_CLASS->name );

#define JL_S_ASSERT_CONSTRUCTING() \
	JL_S_ASSERT_ERROR_NUM( JS_IsConstructing(cx), JLSMSG_NEED_CONSTRUCT );

#define JL_S_ASSERT_RESOURCE(resourcePointer) \
	JL_S_ASSERT_ERROR_NUM( (resourcePointer) != NULL, JLSMSG_INVALID_RESOURCE );

#define JL_S_ASSERT_VALID(condition, name) \
	JL_S_ASSERT_ERROR_NUM( condition, JLSMSG_INVALIDATED_OBJECT, name );


///////////////////////////////////////////////////////////////////////////////
// helper macros to avoid a function call to the jsapi

ALWAYS_INLINE JSClass* JL_GetClass(JSObject *obj) {

	return obj->getClass();
}

ALWAYS_INLINE unsigned int JL_GetStringLength(JSString *jsstr) {

	return jsstr->length();
}

ALWAYS_INLINE void *JL_GetPrivate(JSContext *cx, JSObject *obj) {

	return obj->getPrivate();
}

ALWAYS_INLINE void JL_SetPrivate(JSContext *cx, JSObject *obj, void *data) {

	obj->setPrivate(data);
}

ALWAYS_INLINE JSBool JL_GetReservedSlot(JSContext *cx, JSObject *obj, uint32 index, jsval *vp) {

	JL_ASSERT( OBJ_IS_NATIVE(obj) );
	JSClass *clasp = obj->getClass();
	JS_LOCK_OBJ(cx, obj);
	JL_ASSERT( index < JSCLASS_RESERVED_SLOTS(clasp) || index < JSCLASS_RESERVED_SLOTS(clasp) + (clasp->reserveSlots ? clasp->reserveSlots(cx, obj) : 0) );
	uint32 slot = JSSLOT_START(clasp) + index;
	*vp = (slot < STOBJ_NSLOTS(obj)) ? STOBJ_GET_SLOT(obj, slot) : JSVAL_VOID;
#ifdef DEBUG
	jsval tmp;
	JL_ASSERT( JS_GetReservedSlot(cx, obj, index, &tmp) == JS_TRUE );
	JL_ASSERT( *vp == tmp ); // ensure that JL_GetReservedSlot gives the same result as JS_GetReservedSlot.
#endif // DEBUG
	JS_UNLOCK_OBJ(cx, obj);
	return true;
}

ALWAYS_INLINE JSBool JL_SetReservedSlot(JSContext *cx, JSObject *obj, uint32 index, jsval v) {

	return JS_SetReservedSlot(cx, obj, index, v);
}


///////////////////////////////////////////////////////////////////////////////
// Helper functions

ALWAYS_INLINE JSBool JL_ThrowOSError(JSContext *cx) {

	char errMsg[1024];
	JLLastSysetmErrorMessage(errMsg, sizeof(errMsg));
	JL_REPORT_ERROR_NUM(cx, JLSMSG_OS_ERROR, errMsg);
bad:
	return JS_FALSE;
}

ALWAYS_INLINE bool JL_Ending(JSContext *cx) {

	return cx->runtime->state == JSRTS_LANDING || cx->runtime->state == JSRTS_DOWN; // could be replaced by a flag in HostPrivate that keep the state of the engine.
}

// eg. JS_NewObject(cx, JL_GetStandardClass(cx, JSProto_TypeError), NULL, NULL);
ALWAYS_INLINE JSClass* JL_GetStandardClass(JSContext *cx, JSProtoKey key) {

	JSObject *constructor;
	JL_CHK( JS_GetClassObject(cx, JS_GetGlobalObject(cx), key, &constructor) );
	JL_CHK( constructor );
//	FUN_CLASP( JS_ValueToFunction(cx, OBJECT_TO_JSVAL(constructor)) );
	return FUN_CLASP(GET_FUNCTION_PRIVATE(cx, constructor));
bad:
	return NULL;
}

ALWAYS_INLINE JSContext *JL_GetContext(JSRuntime *rt) {

	JSContext *cx = NULL;
	JS_ContextIterator(rt, &cx);
	JS_ASSERT( cx != NULL );
	return cx;
}

ALWAYS_INLINE JSStackFrame* JL_CurrentStackFrame(JSContext *cx) {

	#ifdef DEBUG
		JSStackFrame *fp = NULL;
		JL_ASSERT( JS_FrameIterator(cx, &fp) == js_GetTopStackFrame(cx) ); // Mozilla JS engine private API behavior has changed.
	#endif //DEBUG
	return js_GetTopStackFrame(cx);
}

ALWAYS_INLINE unsigned int JL_StackSize(JSContext *cx, JSStackFrame *fp) {

	unsigned int length = 0;
	for ( ; fp; fp = fp->down ) // for ( JSStackFrame *fp = JL_CurrentStackFrame(cx); fp; JS_FrameIterator(cx, &fp) )
		++length;
	return length; // 0 is the first frame
}

ALWAYS_INLINE JSStackFrame *JL_StackFrameByIndex(JSContext *cx, int frameIndex) {

	JSStackFrame *fp = JL_CurrentStackFrame(cx);
	if ( frameIndex >= 0 ) {

		int currentFrameIndex = JL_StackSize(cx, fp)-1;
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

	JL_ASSERT( sizeof(uint64_t) == sizeof(double) );
//	return JSVAL_IS_DOUBLE(val) && *(uint64_t*)JSVAL_TO_DOUBLE(val) == *(uint64_t*)cx->runtime->jsNaN; // see also JS_SameValue
	return JSVAL_IS_DOUBLE(val) && *(uint64_t*)JSVAL_TO_DOUBLE(val) == *(uint64_t*)JSVAL_TO_DOUBLE(cx->runtime->NaNValue); // see also JS_SameValue
}

ALWAYS_INLINE bool JsvalIsPInfinity( JSContext *cx, jsval val ) {

	JL_ASSERT( sizeof(uint64_t) == sizeof(double) );
//	return JSVAL_IS_DOUBLE(val) && *(uint64_t*)JSVAL_TO_DOUBLE(val) == *(uint64_t*)cx->runtime->jsPositiveInfinity;
	return JSVAL_IS_DOUBLE(val) && *(uint64_t*)JSVAL_TO_DOUBLE(val) == *(uint64_t*)JSVAL_TO_DOUBLE(cx->runtime->positiveInfinityValue);
}

ALWAYS_INLINE bool JsvalIsNInfinity( JSContext *cx, jsval val ) {

	JL_ASSERT( sizeof(uint64_t) == sizeof(double) );
//	return JSVAL_IS_DOUBLE(val) && *(uint64_t*)JSVAL_TO_DOUBLE(val) == *(uint64_t*)cx->runtime->jsNegativeInfinity;
	return JSVAL_IS_DOUBLE(val) && *(uint64_t*)JSVAL_TO_DOUBLE(val) == *(uint64_t*)JSVAL_TO_DOUBLE(cx->runtime->negativeInfinityValue);
}


ALWAYS_INLINE bool JsvalIsScript( JSContext *cx, jsval val ) {

	return !JSVAL_IS_PRIMITIVE(val) && JL_GetClass(JSVAL_TO_OBJECT(val)) == &js_ScriptClass;
}

ALWAYS_INLINE bool JsvalIsFunction( JSContext *cx, jsval val ) {

	#ifdef DEBUG
		JL_ASSERT( VALUE_IS_FUNCTION(cx, val) == (!JSVAL_IS_PRIMITIVE(val) && JS_ObjectIsFunction(cx, JSVAL_TO_OBJECT(val))) ); // Mozilla JS engine private API behavior has changed.
	#endif //DEBUG
	//	return !JSVAL_IS_PRIMITIVE(val) && JS_ObjectIsFunction(cx, JSVAL_TO_OBJECT(val)); // faster than (JS_TypeOfValue(cx, (val)) == JSTYPE_FUNCTION)
	return VALUE_IS_FUNCTION(cx, val);
}

ALWAYS_INLINE bool JsvalIsArray( JSContext *cx, jsval val ) {

	return !JSVAL_IS_PRIMITIVE(val) && JS_IsArrayObject(cx, JSVAL_TO_OBJECT(val));
}

// Is string or has jslibs BufferGet interface (including Blob).
//#define JL_JSVAL_IS_STRING(val) ( JSVAL_IS_STRING(val) || (!JSVAL_IS_PRIMITIVE(val) && BufferGetInterface(cx, JSVAL_TO_OBJECT(val)) != NULL) ) // || JL_GetClass(JSVAL_TO_OBJECT(val)) == &js_StringClass

/*
ALWAYS_INLINE JSClass* JL_GetErrorClass( JSContext *cx ) {

//	JS_GetClassObject(cx, ...	JSProto_Error
	JSObject *globalObject = JS_GetGlobalObject(cx);
	jsval errorVal;
	JL_CHK( JS_CallFunctionName(cx, globalObject, "Error", 0, NULL, &errorVal) );
	return JL_GetClass(JSVAL_TO_OBJECT(errorVal));

bad:
	return NULL;
}


ALWAYS_INLINE JSClass* JL_GetStringClass( JSContext *cx ) {

	JSObject *emptyStringObject;
	if ( JS_ValueToObject(cx, JS_GetEmptyStringValue(cx), &emptyStringObject) )
		return JL_GetClass(emptyStringObject);
	return NULL;
}
*/

#define JL_VALUE_IS_STRING_OBJECT(cx, val) \
	(!JSVAL_IS_PRIMITIVE(val) && JL_GetClass(JSVAL_TO_OBJECT(val)) == GetHostPrivate(cx)->stringObjectClass)

ALWAYS_INLINE bool JsvalIsData( JSContext *cx, jsval val ) {

	return ( JSVAL_IS_STRING(val) || JL_VALUE_IS_STRING_OBJECT(cx, val) || (!JSVAL_IS_PRIMITIVE(val) && BufferGetInterface(cx, JSVAL_TO_OBJECT(val)) != NULL) );
}

/*
ALWAYS_INLINE bool JL_IsRValOptional( JSContext *cx, void *nativeFct ) {

	struct CodeSpec {
		 int8 length; // length including opcode byte
		 uint32 format; // immediate operand format
	};
	static const CodeSpec codeSpec[] = {
		#define OPDEF(op,val,name,token,length,nuses,ndefs,prec,format) \
			{length, format},
		#include "jsopcode.tbl"
		#undef OPDEF
	};

	//JSStackFrame *fp = JL_CurrentStackFrame(cx);
	//while (fp) { // see js_GetScriptedCaller()
	//
	//	if (fp->script)
	//		break;
	//	fp = fp->down;
	//}
	//jsval *vp1 = fp->regs->sp - (argc + 2);
	//JSObject *obj2 = JSVAL_TO_OBJECT(*vp1);
	//JSFunction *fun = GET_FUNCTION_PRIVATE(cx, obj2); // last function called from the script



	JSStackFrame *fp = JL_CurrentStackFrame(cx);
	if ( fp->script ) // fast native
		return false;

	if ( !fp->fun || !FUN_SLOW_NATIVE(fp->fun) || fp->fun->u.n.native != nativeFct )
		return false;

	while (fp) { // see js_GetScriptedCaller()

		if (fp->script)
			break;
		fp = fp->down;
	}

	if ( !fp || !fp->regs )
		return false;

//	if ( !fp->fun || fp->fun->flags & JSFUN_FAST_NATIVE )
//		return false;

	jsbytecode *pcEnd = fp->script->code + fp->script->length;
	jsbytecode *pc = fp->regs->pc;

	if ( *pc != JSOP_CALL )
		return false;

	while ( pc < pcEnd ) {

		pc += codeSpec[*pc].length;
		switch ( *pc ) {
			case JSOP_TRACE:
				break; // skip
			case JSOP_POPN: // regs.sp -= GET_UINT16(regs.pc);
				if ( GET_UINT16(pc) >= 1 )
					return true;
				break;
			case JSOP_POP: // regs.sp--;
			case JSOP_VOID: // STORE_OPND(-1, JSVAL_VOID);
				return true;

			//case JSOP_SETRVAL:
			//case JSOP_POPV: // fp->rval = POP_OPND();
			//	switch( *(pc + codeSpec[*pc].length) ) {
			//		case JSOP_STOP:
			//			return false; // var a = eval("TestDebug()");
			//	}

			default:
				return false;
		}
	}
	return false;
}
*/


ALWAYS_INLINE bool JL_InheritFrom( JSContext *cx, JSObject *obj, const JSClass *clasp ) {

	while ( obj != NULL ) {

		if ( JL_GetClass(obj) == clasp )
			return true;
		obj = JS_GetPrototype(cx, obj);
	}
	return false;
}

ALWAYS_INLINE bool JsvalIsClass( jsval val, const JSClass *jsClass ) {

	return !JSVAL_IS_PRIMITIVE(val) && JL_GetClass(JSVAL_TO_OBJECT(val)) == jsClass;
}

//ALWAYS_INLINE bool IsClassName( JSObject *obj, const char *name ) {
//
//	return obj != NULL && strcmp(JL_GetClass(obj)->name, name) == 0;
//}

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

//	JL_SAFE(	if ( (int)pv % 2 ) return JS_FALSE; ); // check if *vp is 2-byte aligned
	if ( JS_DefineProperty(cx, obj, name, PRIVATE_TO_JSVAL(pv), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT ) == JS_FALSE )
		return JS_FALSE;
	return JS_TRUE;
	JL_BAD;
}
*/

ALWAYS_INLINE JSBool JS_CallFunctionId(JSContext *cx, JSObject *obj, jsid id, uintN argc, jsval *argv, jsval *rval) {

	JSAutoTempValueRooter tvr(cx); // tvr.addr(); tvr.value()
	return JS_GetMethodById(cx, obj, id, NULL, tvr.addr()) && JS_CallFunctionValue(cx, obj, tvr.value(), argc, argv, rval);
}


// If needed, it is up to the caller to protect argv and rval against GC (see JS_PUSH_TEMP_ROOT)
ALWAYS_INLINE JSBool JL_CallFunction( JSContext *cx, JSObject *obj, jsval functionValue, jsval *rval, uintN argc, ... ) {

	va_list ap;
//	jsval argv[32]; // argc MUST be <= 32
//	JL_S_ASSERT( argc <= COUNTOF(argv), "Too many arguments." );
	jsval *argv = (jsval*)alloca(argc*sizeof(jsval));
	jsval rvalTmp;
	va_start(ap, argc);
	for ( uintN i = 0; i < argc; i++ )
		argv[i] = va_arg(ap, jsval);
	va_end(ap);
	JL_S_ASSERT_FUNCTION( functionValue );
	if ( rval == NULL )
		rval = &rvalTmp;
	JL_CHK( JS_CallFunctionValue(cx, obj, functionValue, argc, argv, rval) ); // NULL is NOT supported for &rvalTmp ( last arg of JS_CallFunctionValue )
	return JS_TRUE;
	JL_BAD;
}

// If needed, it is up to the caller to protect argv and rval against GC (see JS_PUSH_TEMP_ROOT)
ALWAYS_INLINE JSBool JL_CallFunctionName( JSContext *cx, JSObject *obj, const char* functionName, jsval *rval, uintN argc, ... ) {

	va_list ap;
//	jsval argv[32]; // argc MUST be <= 32
//	JL_S_ASSERT( argc <= COUNTOF(argv), "Too many arguments." );
	jsval *argv = (jsval*)alloca(argc*sizeof(jsval));
	jsval rvalTmp;
	va_start(ap, argc);
	for ( uintN i = 0; i < argc; i++ )
		argv[i] = va_arg(ap, jsval);
	va_end(ap);
	if ( rval == NULL )
		rval = &rvalTmp;
	JL_CHK( JS_CallFunctionName(cx, obj, functionName, argc, argv, &rvalTmp) ); // NULL is NOT supported for &rvalTmp ( last arg of JS_CallFunctionValue )
	return JS_TRUE;
	JL_BAD;
}


ALWAYS_INLINE JSBool JL_ValueOf( JSContext *cx, jsval *val, jsval *rval ) {

	if ( JSVAL_IS_PRIMITIVE(*val) ) {

		*rval = *val;
		return JS_TRUE;
	}
	return JSVAL_TO_OBJECT(*val)->defaultValue(cx, JSTYPE_VOID, rval);
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
	void *data = NULL;
	uint32 prevOpts = JS_GetOptions(cx);

	char compiledFileName[PATH_MAX];
	strcpy( compiledFileName, fileName );
	strcat( compiledFileName, "xdr" );

	struct stat srcFileStat, compFileStat;
	bool hasSrcFile = stat(fileName, &srcFileStat) != -1; // errno == ENOENT
	bool hasCompFile = stat(compiledFileName, &compFileStat) != -1;
	bool compFileUpToDate = ( hasCompFile && !hasSrcFile ) || ( hasCompFile && hasSrcFile && (compFileStat.st_mtime > srcFileStat.st_mtime) ); // true if comp file is up to date or alone

	JL_CHKM( hasSrcFile || hasCompFile, "Unable to load Script, file \"%s\" or \"%s\" not found.", fileName, compiledFileName );

	if ( useCompFile && compFileUpToDate ) {

		int file = open(compiledFileName, O_RDONLY | O_BINARY | O_SEQUENTIAL);
		JL_CHKM( file != -1, "Unable to open file \"%s\" for reading.", compiledFileName );

		int compFileSize = compFileStat.st_size; // filelength(file); ?
		data = jl_malloc(compFileSize); // (TBD) free on error
		int readCount = read( file, data, compFileSize ); // here we can use "Memory-Mapped I/O Functions" ( http://developer.mozilla.org/en/docs/NSPR_API_Reference:I/O_Functions#Memory-Mapped_I.2FO_Functions )
		JL_CHKM( readCount != -1 && readCount == compFileSize, "Unable to read the file \"%s\" ", compiledFileName );
		close( file );

		JSXDRState *xdr = JS_XDRNewMem(cx, JSXDR_DECODE);
		JL_CHK( xdr );
		JS_XDRMemSetData(xdr, data, compFileSize);


		// we want silent failures.
		JSErrorReporter prevErrorReporter = JS_SetErrorReporter(cx, NULL);
		JSDebugErrorHook debugErrorHook = cx->debugHooks->debugErrorHook;
		void *debugErrorHookData = cx->debugHooks->debugErrorHookData;
		JS_SetDebugErrorHook(JS_GetRuntime(cx), NULL, NULL);
		JSBool status = JS_XDRScript(xdr, &script);
		JS_SetDebugErrorHook(JS_GetRuntime(cx), debugErrorHook, debugErrorHookData);
		if (cx->lastMessage)
			JS_free(cx, cx->lastMessage);
		cx->lastMessage = NULL;
		JS_SetErrorReporter(cx, prevErrorReporter);

		if ( status == JS_TRUE ) {

//			*script->notes() = 0;
			// (TBD) manage BIG_ENDIAN here ?
			JS_XDRMemSetData(xdr, NULL, 0);
			JS_XDRDestroy(xdr);
			jl_free(data);
			data = NULL;
			if ( JS_GetScriptVersion(cx, script) < JS_GetVersion(cx) )
				JL_REPORT_WARNING("Trying to xdr-decode an old script (%s).", compiledFileName);
			goto good;
		} else {

			jl_free(data);
			data = NULL;
//			if ( JS_IsExceptionPending(cx) )
//				JS_ClearPendingException(cx);
		}
	}

	if ( !hasSrcFile )
		goto bad; // no source, no compiled version of the source, die.

	if ( saveCompFile )
		JS_SetOptions( cx, JS_GetOptions(cx) & ~JSOPTION_COMPILE_N_GO ); // see https://bugzilla.mozilla.org/show_bug.cgi?id=494363

#define JL_UC
#ifndef JL_UC

	FILE *scriptFile;
	scriptFile = fopen(fileName, "r");
	JL_CHKM( scriptFile != NULL, "Script file \"%s\" cannot be opened.", fileName );

	// shebang support
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

	script = JS_CompileFileHandle(cx, obj, fileName, scriptFile);
	fclose(scriptFile);

#else //JL_UC

	int scriptFile;
	scriptFile = open(fileName, O_RDONLY | O_BINARY | O_SEQUENTIAL);
	JL_CHKM( scriptFile >= 0, "Unable to open file \"%s\" for reading.", fileName );

	int scriptFileSize;
	scriptFileSize = lseek(scriptFile, 0, SEEK_END);
//	int scriptFileSize;
//	scriptFileSize = (unsigned)tell(scriptFile);
	lseek(scriptFile, 0, SEEK_SET);
	char *scriptBuffer;
	scriptBuffer = (char*)alloca(scriptFileSize);
	int res;
	res = read(scriptFile, (void*)scriptBuffer, scriptFileSize);
	close(scriptFile);
	JL_CHKM( res >= 0, "Unable to read file \"%s\".", fileName );
	scriptFileSize = (unsigned)res;

	JLEncodingType enc;
	enc = JLDetectEncoding(&scriptBuffer, &scriptFileSize);
	if ( enc == ASCII ) {

		char *scriptText = scriptBuffer;
		size_t scriptTextLength = scriptFileSize;
		if ( scriptText[0] == '#' && scriptText[1] == '!' ) { // shebang support

			scriptText[0] = '/';
			scriptText[1] = '/';
		}
		script = JS_CompileScript(cx, obj, scriptText, scriptTextLength, fileName, 1);
	} else
	if ( enc == UTF16le ) { // (TBD) support big-endian

		jschar *scriptText = (jschar*)scriptBuffer;
		size_t scriptTextLength = scriptFileSize / 2;
		if ( scriptText[0] == '#' && scriptText[1] == '!' ) { // shebang support

			scriptText[0] = '/';
			scriptText[1] = '/';
		}
		script = JS_CompileUCScript(cx, obj, scriptText, scriptTextLength, fileName, 1);
	} else
	if ( enc == UTF8 ) { // (TBD) check if JS_DecodeBytes does the right things

		jschar *scriptText = (jschar *)alloca(scriptFileSize * 2);
		int scriptTextLength = scriptFileSize * 2;

		JL_CHKM( UTF8ToUTF16LE((unsigned char*)scriptText, &scriptTextLength, (unsigned char*)scriptBuffer, &scriptFileSize) >= 0, "Unable do decode UTF8 data." );

		if ( scriptText[0] == '#' && scriptText[1] == '!' ) { // shebang support

			scriptText[0] = '/';
			scriptText[1] = '/';
		}
		script = JS_CompileUCScript(cx, obj, scriptText, scriptTextLength, fileName, 1);
	}

	JL_CHKM( script, "Unable to compile the script \"%s\".", fileName );

#endif //JL_UC

	if ( !saveCompFile )
		goto good;

	int file;
	file = open(compiledFileName, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY | O_SEQUENTIAL, srcFileStat.st_mode); // (TBD) check the mode
	if ( file == -1 ) // if the file cannot be write, this is not an error ( eg. read-only drive )
		goto good;

	JSXDRState *xdr;
	xdr = JS_XDRNewMem(cx, JSXDR_ENCODE);
	JL_CHK( xdr );
	JL_CHK( JS_XDRScript(xdr, &script) );

	uint32 length;
	void *buf;
	buf = JS_XDRMemGetData(xdr, &length);
	JL_CHK( buf );
	// manage BIG_ENDIAN here ?
	JL_CHK( write(file, buf, length) != -1 ); // On error, -1 is returned, and errno is set appropriately.
	JL_CHK( close(file) == 0 );
	JS_XDRDestroy(xdr);
	goto good;

good:
	JS_SetOptions(cx, prevOpts);
	return script;

bad:
	JS_SetOptions(cx, prevOpts);
	if ( data )
		jl_free(data);
	return NULL; // report a warning ?
}


/*
// Franck, this is hopeless. The JS engine is not going to keep around apparently-dead variables on the off chance that
// someone might call a native function that uses the debugger APIs to read them off the stack. The debugger APIs don't work that way.
// ...
// -j

//ALWAYS_INLINE jsid StringToJsid( JSContext *cx, const char *cstr );
// Get the value of a variable in the current or parent's scopes.
ALWAYS_INLINE JSBool JL_GetVariableValue( JSContext *cx, const char *name, jsval *vp ) {

//	JSStackFrame *fp = JS_GetScriptedCaller(cx, NULL);
//	return JS_EvaluateInStackFrame(cx, fp, name, strlen(name), "", 0, vp);

	JSBool found;
	JSStackFrame *fp = JS_GetScriptedCaller(cx, NULL);

	for ( JSObject *scope = JS_GetFrameScopeChain(cx, fp); scope; scope = scope->getParent() ) {

		JL_CHK( JS_HasProperty(cx, scope, name, &found) );
		if ( found ) {

			JL_CHK( JS_GetProperty(cx, scope, name, vp) );

//			JS_LookupProperty(cx, scope, name, vp);
//			uintN attrs;
//			JS_GetPropertyAttributes(cx, scope, name, &attrs, &found);

//			JSPropertyDescriptor desc;
//			JS_GetPropertyDescriptorById(cx, scope, StringToJsid(cx, name), 0, &desc);
//			*vp = desc.value;

			return JS_TRUE;
		}
	}
	*vp = JSVAL_VOID;

	return JS_TRUE;
	JL_BAD;
}
*/


///////////////////////////////////////////////////////////////////////////////
// jslibs tools

ALWAYS_INLINE bool JL_MaybeRealloc( unsigned int requested, unsigned int received ) {

	return requested != 0 && (128 * received / requested < 96) && (requested - received > 64); // "128 *": instead using percent, we use per-128
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

//ALWAYS_INLINE void JL_CleanRegisterNativeClasses( JSContext *cx ) {
//
//	jl::QueueDestruct(&GetHostPrivate(cx)->registredNativeClasses); // QueueDestruct make free( GetHostPrivate(cx)->registredNativeClasses ) !!!
//}


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

ALWAYS_INLINE bool JL_ObjectIsBlob( JSContext *cx, JSObject *obj ) {

	return JL_GetClass(obj) == JL_GetRegistredNativeClass(cx, "Blob");
}


ALWAYS_INLINE bool JL_ValueIsBlob( JSContext *cx, jsval v ) {

	return !JSVAL_IS_PRIMITIVE(v) && JL_ObjectIsBlob(cx, JSVAL_TO_OBJECT(v));
}


// note: a Blob is either a JSString or a Blob object if the jslang module has been loaded.
ALWAYS_INLINE JSBool JL_NewBlob( JSContext *cx, void* buffer, unsigned int length, jsval *vp ) {

	if (unlikely( length == 0 )) { // Empty Blob must acts like an empty string: !'' === true

		if ( buffer )
			JS_free(cx, buffer);
		*vp = JS_GetEmptyStringValue(cx);
		return JS_TRUE;
	}

	JSClass *blobClass = JL_GetRegistredNativeClass(cx, "Blob"); // don't use static keyword (cf. jstask module)

	if (likely( blobClass != NULL )) { // we have Blob class, jslang is present.

		// A blob/string object can be created without using any jslang/blob.h dependances
		JSObject *blob;
		blob = JS_ConstructObject(cx, blobClass, NULL, NULL); // need to be constructed else Buffer NativeInterface will not be set !
		JL_CHK( blob );
		*vp = OBJECT_TO_JSVAL(blob);
		JL_CHK( JS_SetReservedSlot(cx, blob, 0, INT_TO_JSVAL( length )) ); // 0 for SLOT_BLOB_LENGTH !!!
		JL_SetPrivate(cx, blob, buffer); // blob data
		return JS_TRUE;
	}

	JSString *jsstr;
	jsstr = JS_NewString(cx, (char*)buffer, length); // JS_NewString takes ownership of bytes on success, avoiding a copy; but on error (signified by null return), it leaves bytes owned by the caller. So the caller must free bytes in the error case, if it has no use for them.
	JL_CHK( jsstr );
	*vp = STRING_TO_JSVAL(jsstr); // protect from GC.

	JSObject *strObj;
	JL_CHK( JS_ValueToObject(cx, STRING_TO_JSVAL(jsstr), &strObj) ); // see. OBJ_DEFAULT_VALUE(cx, obj, JSTYPE_OBJECT, &v)
	*vp = OBJECT_TO_JSVAL(strObj);
	return JS_TRUE;

bad:
	JS_free(cx, buffer); // JS_NewString does not free the buffer on error.
	return JS_FALSE;
}


ALWAYS_INLINE JSBool JL_NewBlobCopyN( JSContext *cx, const void *data, unsigned int amount, jsval *vp ) {

	if (unlikely( amount == 0 )) { // Empty Blob must acts like an empty string: !'' == true

		*vp = JS_GetEmptyStringValue(cx);
		return JS_TRUE;
	}
	// possible optimization: if Blob class is not abailable, copy data into JSString's jschar to avoid js_InflateString.
	char *blobBuf = (char*)JS_malloc(cx, amount);
	JL_CHK( blobBuf );
	memcpy( blobBuf, data, amount );
	return JL_NewBlob(cx, blobBuf, amount, vp);
	JL_BAD;
}


///////////////////////////////////////////////////////////////////////////////
// jsval convertion functions


ALWAYS_INLINE jsid StringToJsid( JSContext *cx, const char *cstr ) {

	jsid tmp;
	JSString *jsstr = JS_InternString(cx, cstr);
	if ( jsstr == NULL )
		return 0;
	if ( !JS_ValueToId(cx, STRING_TO_JSVAL(jsstr), &tmp) )
		return 0;
	return tmp;
}


// beware: caller should keep a reference to buffer as short time as possible, because it is difficult to protect it from GC.
ALWAYS_INLINE JSBool JsvalToStringAndLength( JSContext *cx, jsval *val, const char** buffer, unsigned int *size ) {

	if ( JSVAL_IS_STRING(*val) ) { // for string literals

		JSString *str = JSVAL_TO_STRING(*val);
		*buffer = JS_GetStringBytes(str); // JS_GetStringBytes never returns NULL, then JL_S_ASSERT( *buffer != NULL, "Invalid string." ); is not needed.
		*size = JL_GetStringLength(str);
		return JS_TRUE;
	}
	if ( !JSVAL_IS_PRIMITIVE(*val) ) { // for NIBufferGet support

		NIBufferGet fct = BufferGetNativeInterface(cx, JSVAL_TO_OBJECT(*val));
		if ( fct )
			return fct(cx, JSVAL_TO_OBJECT(*val), buffer, size);
	}
	// and for anything else ...
	JSString *jsstr = JS_ValueToString(cx, *val);

	JS_S_ASSERT_CONVERT( jsstr != NULL, "string" );

	*val = STRING_TO_JSVAL(jsstr); // protects *val against GC.
	*size = JL_GetStringLength(jsstr);
	*buffer = JS_GetStringBytes(jsstr); // JS_GetStringBytes never returns NULL, then useless to check if (*buffer != NULL).
	return JS_TRUE;
	JL_BAD;
}


ALWAYS_INLINE JSBool JsvalToStringLength( JSContext *cx, jsval val, unsigned int *length ) {

	if ( JSVAL_IS_STRING(val) ) { // for string literals

		*length = JL_GetStringLength( JSVAL_TO_STRING(val) );
		return JS_TRUE;
	}
	if ( !JSVAL_IS_PRIMITIVE(val) ) { // for NIBufferGet support

		NIBufferGet fct = BufferGetNativeInterface(cx, JSVAL_TO_OBJECT(val));
		const char* tmp;
		if ( fct )
			return fct(cx, JSVAL_TO_OBJECT(val), &tmp, length);
	}
	JSString *str = JS_ValueToString(cx, val); // unfortunately, we have to convert to a string to know its length
	JL_CHK( str != NULL );
	*length = JL_GetStringLength(str);
	return JS_TRUE;
	JL_BAD;
}


ALWAYS_INLINE JSBool JsvalToString( JSContext *cx, jsval *val, const char** buffer ) {

	if (likely( JSVAL_IS_STRING(*val) )) { // for string literals

		*buffer = JS_GetStringBytes(JSVAL_TO_STRING(*val)); // JS_GetStringBytes never returns NULL, then JL_S_ASSERT( *buffer != NULL, "Invalid string." ); is not needed.
		return JS_TRUE;
	}
	if (likely( !JSVAL_IS_PRIMITIVE(*val) )) { // for NIBufferGet support

		unsigned int size; //unused
		NIBufferGet fct = BufferGetNativeInterface(cx, JSVAL_TO_OBJECT(*val));
		if ( fct )
			return fct(cx, JSVAL_TO_OBJECT(*val), buffer, &size);
	}
	// and for anything else ...
	JSString *jsstr = JS_ValueToString(cx, *val);
	JS_S_ASSERT_CONVERT( jsstr != NULL, "string" );
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
		JL_REPORT_ERROR( "Unable to create the string." );
	*val = STRING_TO_JSVAL(jsstr);
	return JS_TRUE;
	JL_BAD;
}


ALWAYS_INLINE JSBool StringAndLengthToJsval( JSContext *cx, jsval *val, const char* cstr, unsigned int length ) {

	if (likely( length > 0 )) {

		JSString *jsstr = JS_NewStringCopyN(cx, cstr, length);
		if (unlikely( jsstr == NULL ))
			JL_REPORT_ERROR( "Unable to create the string." );
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
	JL_CHK( JS_ValueToNumber(cx, val, &d) );
	if (likely( d >= (jsdouble)INT_MIN && d <= (jsdouble)INT_MAX )) {

		*i = (int)d;
		return JS_TRUE;
	}

bad:
	JL_REPORT_WARNING_NUM(cx, JLSMSG_FAIL_TO_CONVERT_TO, "integer" );
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
	JL_CHK( JS_ValueToNumber(cx, val, &d) );
	if (likely( d >= (jsdouble)0 && d <= (jsdouble)UINT_MAX )) {

		*ui = (unsigned int)d;
		return JS_TRUE;
	}

bad:
	JL_REPORT_WARNING_NUM(cx, JLSMSG_FAIL_TO_CONVERT_TO, "unsigned integer" );
	return JS_FALSE;
}


ALWAYS_INLINE JSBool IntToJsval( JSContext *cx, int i, jsval *val ) {

	if (likely( INT_FITS_IN_JSVAL(i) )) {

		*val = INT_TO_JSVAL(i);
		return JS_TRUE;
	}
	JL_CHK( JS_NewNumberValue(cx, i, val) );
	return JS_TRUE;

bad:
	JL_REPORT_WARNING_NUM(cx, JLSMSG_FAIL_TO_CONVERT_TO, "integer" );
	return JS_FALSE;
}


ALWAYS_INLINE JSBool UIntToJsval( JSContext *cx, unsigned int ui, jsval *val ) {

	if (likely( ui <= JSVAL_INT_MAX )) {

		*val = INT_TO_JSVAL(ui);
		return JS_TRUE;
	}
	JL_CHK( JS_NewNumberValue(cx, ui, val) );
	return JS_TRUE;

bad:
	JL_REPORT_WARNING_NUM(cx, JLSMSG_FAIL_TO_CONVERT_TO, "unsigned integer" );
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
	JL_CHK( JS_ValueToBoolean(cx, val, &tmp) );
	*b = (tmp == JS_TRUE);
	return JS_TRUE;

bad:
	JL_REPORT_WARNING( "Unable to convert to a boolean." );
	return JS_FALSE;
}


ALWAYS_INLINE JSBool JsvalToDouble( JSContext *cx, jsval val, double *d ) {

	if (likely( JSVAL_IS_DOUBLE(val) )) {

		*d = *JSVAL_TO_DOUBLE(val);
		return JS_TRUE;
	}
	if ( JSVAL_IS_INT(val) ) {

		*d = JSVAL_TO_INT(val);
		return JS_TRUE;
	}
	JL_CHK( JS_ValueToNumber( cx, val, d ) ); // jsdouble is a double
	return JS_TRUE;

bad:
	JL_REPORT_WARNING( "Unable to convert to a real." );
	return JS_FALSE;
}

ALWAYS_INLINE JSBool DoubleToJsval( JSContext *cx, double d, jsval *rval ) {

	return JS_NewDoubleValue(cx, d, rval); // return JS_NewNumberValue(cx, f, val); // slower ?
}


ALWAYS_INLINE JSBool JsvalToFloat( JSContext *cx, jsval val, float *f ) {

	if (likely( JSVAL_IS_DOUBLE(val) )) {

		*f = *JSVAL_TO_DOUBLE(val);
		return JS_TRUE;
	}
	if ( JSVAL_IS_INT(val) ) {

		*f = JSVAL_TO_INT(val);
		return JS_TRUE;
	}
	jsdouble tmp;
	JL_CHK( JS_ValueToNumber( cx, val, &tmp ) );
	*f = tmp;
	return JS_TRUE;

bad:
	JL_REPORT_WARNING( "Unable to convert to a real." );
	return JS_FALSE;
}

ALWAYS_INLINE JSBool FloatToJsval( JSContext *cx, float f, jsval *rval ) {

	return DoubleToJsval(cx, f, rval); // return JS_NewNumberValue(cx, f, val); // slower ?
}

//#define JL_OBJECT_TO_SCRIPT(obj) ((JSScript*)JL_GetPrivate(cx, (obj)))
ALWAYS_INLINE JSBool ObjectToScript( JSContext *cx, JSObject *obj, JSScript **script ) {

	#ifdef DEBUG
		JL_ASSERT( JS_GetClass(obj) == &js_ScriptClass ); // Mozilla JS engine private API behavior has changed.
	#endif //DEBUG

	*script = (JSScript*)JL_GetPrivate(cx, obj);
	return JS_TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// vector convertion functions

// if useValArray is true, val must be a valid array that is used to store the values.
ALWAYS_INLINE JSBool IntVectorToJsval( JSContext *cx, int *vector, uint32 length, jsval *val, bool useValArray = false ) {

	JSObject *arrayObj;
	if ( useValArray ) {

		JL_S_ASSERT_OBJECT(*val);
		arrayObj = JSVAL_TO_OBJECT(*val);
		JL_CHK( JS_SetArrayLength(cx, arrayObj, length) );
	} else {

		arrayObj = JS_NewArrayObject(cx, length, NULL);
		JL_CHK( arrayObj );
		*val = OBJECT_TO_JSVAL(arrayObj);
	}
	jsval tmp;
	for ( unsigned int i = 0; i < length; i++ ) {

		JL_CHK( IntToJsval(cx, vector[i], &tmp) );
		JL_CHK( JS_SetElement(cx, arrayObj, i, &tmp) );
	}
//	JL_CHK( JS_SetArrayLength(cx, arrayObj, length) );
	return JS_TRUE;
	JL_BAD;
}


ALWAYS_INLINE JSBool JsvalToIntVector( JSContext *cx, jsval val, int *vector, uint32 maxLength, uint32 *currentLength ) {

	JL_S_ASSERT_OBJECT(val);
	JSObject *arrayObj;
	arrayObj = JSVAL_TO_OBJECT(val);
	JL_CHK( JS_GetArrayLength(cx, arrayObj, currentLength) );
	maxLength = JL_MIN( *currentLength, maxLength );
	for ( jsuint i = 0; i < maxLength; i++ ) {

		JL_CHK( JS_GetElement(cx, arrayObj, i, &val) );
		JL_CHK( JsvalToInt(cx, val, &vector[i]) );
	}
	return JS_TRUE;
	JL_BAD;
}



ALWAYS_INLINE JSBool UIntVectorToJsval( JSContext *cx, unsigned int *vector, uint32 length, jsval *val, bool useValArray = false ) {

	JSObject *arrayObj;
	if ( useValArray ) {

		JL_S_ASSERT_OBJECT(*val);
		arrayObj = JSVAL_TO_OBJECT(*val);
		JL_CHK( JS_SetArrayLength(cx, arrayObj, length) );
	} else {

		arrayObj = JS_NewArrayObject(cx, length, NULL);
		JL_CHK( arrayObj );
		*val = OBJECT_TO_JSVAL(arrayObj);
	}
	jsval tmp;
	for ( unsigned int i = 0; i < length; i++ ) {

		JL_CHK( UIntToJsval(cx, vector[i], &tmp) );
		JL_CHK( JS_SetElement(cx, arrayObj, i, &tmp) );
	}
//	JL_CHK( JS_SetArrayLength(cx, arrayObj, length) );
	return JS_TRUE;
	JL_BAD;
}



ALWAYS_INLINE JSBool JsvalToUIntVector( JSContext *cx, jsval val, unsigned int *vector, uint32 maxLength, uint32 *currentLength ) {

	JL_S_ASSERT_OBJECT(val);
	JSObject *arrayObj;
	arrayObj = JSVAL_TO_OBJECT(val);
	JL_CHK( JS_GetArrayLength(cx, arrayObj, currentLength) );
	maxLength = JL_MIN( *currentLength, maxLength );
	for ( jsuint i = 0; i < maxLength; i++ ) {

		JL_CHK( JS_GetElement(cx, arrayObj, i, &val) );
		JL_CHK( JsvalToUInt(cx, val, &vector[i]) );
	}
	return JS_TRUE;
	JL_BAD;
}


ALWAYS_INLINE JSBool FloatVectorToJsval( JSContext *cx, const float *vector, uint32 length, jsval *val, bool reuseValArray = false ) {

	JSObject *arrayObj;
	if ( reuseValArray ) {

		JL_S_ASSERT_OBJECT(*val);
		arrayObj = JSVAL_TO_OBJECT(*val);
		JL_CHK( JS_SetArrayLength(cx, arrayObj, length) );
	} else {

		arrayObj = JS_NewArrayObject(cx, length, NULL);
		JL_CHK( arrayObj );
		*val = OBJECT_TO_JSVAL(arrayObj);
	}
	jsval tmp;
	for ( unsigned int i = 0; i < length; i++ ) {

		JL_CHK( FloatToJsval(cx, vector[i], &tmp) );
		JL_CHK( JS_SetElement(cx, arrayObj, i, &tmp) );
	}
//	JL_CHK( JS_SetArrayLength(cx, arrayObj, length) );
	return JS_TRUE;
	JL_BAD;
}


ALWAYS_INLINE JSBool JsvalToFloatVector( JSContext *cx, jsval val, float *vector, uint32 maxLength, uint32 *currentLength ) {

	JL_S_ASSERT_OBJECT(val);
	JSObject *arrayObj;
	arrayObj = JSVAL_TO_OBJECT(val);
	JL_CHK( JS_GetArrayLength(cx, arrayObj, currentLength) );
	maxLength = JL_MIN( *currentLength, maxLength );
	for ( jsuint i = 0; i < maxLength; i++ ) {

		JL_CHK( JS_GetElement(cx, arrayObj, i, &val) );
		JL_CHK( JsvalToFloat(cx, val, &vector[i]) );
	}
	return JS_TRUE;
	JL_BAD;
}


ALWAYS_INLINE JSBool DoubleVectorToJsval( JSContext *cx, const double *vector, uint32 length, jsval *val, bool reuseValArray = false ) {

	JSObject *arrayObj;
	if ( reuseValArray ) {

		JL_S_ASSERT_OBJECT(*val);
		arrayObj = JSVAL_TO_OBJECT(*val);
		JL_CHK( JS_SetArrayLength(cx, arrayObj, length) );
	} else {

		arrayObj = JS_NewArrayObject(cx, length, NULL);
		JL_CHK( arrayObj );
		*val = OBJECT_TO_JSVAL(arrayObj);
	}
	jsval tmp;
	for ( unsigned int i = 0; i < length; i++ ) {

		JL_CHK( DoubleToJsval(cx, vector[i], &tmp) );
		JL_CHK( JS_SetElement(cx, arrayObj, i, &tmp) );
	}
//	JL_CHK( JS_SetArrayLength(cx, arrayObj, length) );
	return JS_TRUE;
	JL_BAD;
}


ALWAYS_INLINE JSBool JsvalToDoubleVector( JSContext *cx, jsval val, double *vector, uint32 maxLength, uint32 *currentLength ) {

	JL_S_ASSERT_OBJECT(val);
	JSObject *arrayObj;
	arrayObj = JSVAL_TO_OBJECT(val);
	JL_CHK( JS_GetArrayLength(cx, arrayObj, currentLength) );
	maxLength = JL_MIN( *currentLength, maxLength );
	for ( jsuint i = 0; i < maxLength; i++ ) {

		JL_CHK( JS_GetElement(cx, arrayObj, i, &val) );
		JL_CHK( JsvalToDouble(cx, val, &vector[i]) );
	}
	return JS_TRUE;
	JL_BAD;
}


///////////////////////////////////////////////////////////////////////////////
// properties helper


ALWAYS_INLINE JSBool SetPropertyString( JSContext *cx, JSObject *obj, const char *propertyName, const char *str ) {

	jsval val; // (TBD) need GC protection
	JL_CHK( StringToJsval(cx, str, &val) );
	JL_CHKM( JS_DefineProperty(cx, obj, propertyName, val, NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT ), "Unable to set the property." ); // Doc. http://developer.mozilla.org/en/docs/JS_DefineUCProperty
	return JS_TRUE;
	JL_BAD;
}

ALWAYS_INLINE JSBool GetPropertyString( JSContext *cx, JSObject *obj, const char *propertyName, const char **str ) {

	jsval val;
	JL_CHKM( JS_GetProperty(cx, obj, propertyName, &val), "Unable to read the property %s.", propertyName );
	JL_CHK( JsvalToString(cx, &val, str) );
	return JS_TRUE;
	JL_BAD;
}


ALWAYS_INLINE JSBool SetPropertyBool( JSContext *cx, JSObject *obj, const char *propertyName, bool b ) {

	jsval val;
	JL_CHK( BoolToJsval(cx, b, &val) );
	JL_CHKM( JS_SetProperty(cx, obj, propertyName, &val), "Unable to set the property." );
	return JS_TRUE;
	JL_BAD;
}

ALWAYS_INLINE JSBool GetPropertyBool( JSContext *cx, JSObject *obj, const char *propertyName, bool *b ) {

	jsval val;
	JL_CHKM( JS_GetProperty(cx, obj, propertyName, &val), "Unable to read the property %s.", propertyName );
	JL_CHK( JsvalToBool(cx, val, b) );
	return JS_TRUE;
	JL_BAD;
}


ALWAYS_INLINE JSBool SetPropertyInt( JSContext *cx, JSObject *obj, const char *propertyName, int intVal ) {

	jsval val;
	JL_CHK( IntToJsval(cx, intVal, &val) );
	JL_CHKM( JS_SetProperty(cx, obj, propertyName, &val), "Unable to set the property." ); // Doc. http://developer.mozilla.org/en/docs/JS_DefineUCProperty
	return JS_TRUE;
	JL_BAD;
}

ALWAYS_INLINE JSBool GetPropertyInt( JSContext *cx, JSObject *obj, const char *propertyName, int *intVal ) {

	jsval val;
	JL_CHKM( JS_GetProperty(cx, obj, propertyName, &val), "Unable to read the property %s.", propertyName );
	JL_CHK( JsvalToInt(cx, val, intVal) );
	return JS_TRUE;
	JL_BAD;
}


ALWAYS_INLINE JSBool SetPropertyUInt( JSContext *cx, JSObject *obj, const char *propertyName, unsigned int ui ) {

	jsval val;
	JL_CHK( UIntToJsval(cx, ui, &val) );
	JL_CHKM( JS_SetProperty(cx, obj, propertyName, &val), "Unable to set the property." ); // Doc. http://developer.mozilla.org/en/docs/JS_DefineUCProperty
	return JS_TRUE;
	JL_BAD;
}

ALWAYS_INLINE JSBool GetPropertyUInt( JSContext *cx, JSObject *obj, const char *propertyName, unsigned int *ui ) {

	jsval val;
	JL_CHKM( JS_GetProperty(cx, obj, propertyName, &val), "Unable to read the property %s.", propertyName ); // try. OBJ_GET_PROPERTY(...
	JL_CHK( JsvalToUInt(cx, val, ui) );
	return JS_TRUE;
	JL_BAD;
}


///////////////////////////////////////////////////////////////////////////////
//

ALWAYS_INLINE JSBool ExceptionSetScriptLocation( JSContext *cx, JSObject *obj ) {

	 // see PopulateReportBlame()
    //JSStackFrame *fp;
    //for (fp = js_GetTopStackFrame(cx); fp; fp = fp->down) {
    //    if (fp->regs) {
    //        report->filename = fp->script->filename;
    //        report->lineno = js_FramePCToLineNumber(cx, fp);
    //        break;
    //    }
    //}

	JSStackFrame *fp = NULL;
	do {

		JS_FrameIterator(cx, &fp);
	} while ( fp && !JS_GetFramePC(cx, fp) );

	JL_CHK( fp );
	JSScript *script;
	script = JS_GetFrameScript(cx, fp);
	JL_CHK( script );
	const char *filename;
	int lineno;
	filename = JS_GetScriptFilename(cx, script);
	if ( filename == NULL || *filename == '\0' )
		filename = "<no_filename>";
	lineno = JS_PCToLineNumber(cx, script, JS_GetFramePC(cx, fp));

	jsval tmp;
	JL_CHK( StringToJsval(cx, filename, &tmp) );
	JL_CHK( JS_SetPropertyById(cx, obj, JL_ATOMJSID(cx, fileName), &tmp) );
	JL_CHK( IntToJsval(cx, lineno, &tmp) );
	JL_CHK( JS_SetPropertyById(cx, obj, JL_ATOMJSID(cx, lineNumber), &tmp) );

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
	JL_S_ASSERT( *xdr != NULL, "Unable to create the serializer." );
	JL_CHK( JS_XDRValue(*xdr, val) );
	return JS_TRUE;
	JL_BAD;
}

ALWAYS_INLINE JSBool UnserializeJsval( JSContext *cx, const Serialized *xdr, jsval *rval ) {

	JSXDRState *xdrDecoder = JS_XDRNewMem(cx, JSXDR_DECODE);
	JL_S_ASSERT( xdrDecoder != NULL, "Unable to create the unserializer." );
	uint32 length;
	void *data;
	data = JS_XDRMemGetData(*xdr, &length);
	JS_XDRMemSetData(xdrDecoder, data, length);
	JL_CHK( JS_XDRValue(xdrDecoder, rval) );
	JS_XDRMemSetData(xdrDecoder, NULL, 0);
	JS_XDRDestroy(xdrDecoder);
	return JS_TRUE;
	JL_BAD;
}

//

ALWAYS_INLINE JSBool SetPrivateNativeFunction( JSContext *cx, JSObject *obj, const char *name, void *nativeFct ) {

	return JS_DefineProperty(cx, obj, name, JSVAL_TRUE, NULL, (JSPropertyOp)nativeFct, JSPROP_READONLY | JSPROP_PERMANENT );
}

ALWAYS_INLINE JSBool GetPrivateNativeFunction( JSContext *cx, JSObject *obj, const char *name, void **nativeFct ) {

	uintN attrs;
	JSBool found;
	JL_CHK( JS_GetPropertyAttrsGetterAndSetter(cx, obj, name, &attrs, &found, NULL, (JSPropertyOp*)nativeFct) );
	if ( !found )
		*nativeFct = NULL;
	return JS_TRUE;
	JL_BAD;
}

ALWAYS_INLINE JSBool RemovePrivateNativeFunction( JSContext *cx, JSObject *obj, const char *name ) {

	return JS_DeleteProperty(cx, obj, name);
}


///////////////////////////////////////////////////////////////////////////////
// NativeInterface

ALWAYS_INLINE JSBool ReserveNativeInterface( JSContext *cx, JSObject *obj, const char *name ) {

//	return obj->defineProperty(cx, id, JSVAL_FALSE, NULL, (JSPropertyOp)-1, JSPROP_READONLY | JSPROP_PERMANENT );
	return JS_DefineProperty(cx, obj, name, JSVAL_FALSE, NULL, (JSPropertyOp)-1, JSPROP_READONLY | JSPROP_PERMANENT );
}

ALWAYS_INLINE JSBool SetNativeInterface( JSContext *cx, JSObject *obj, const char *name, void *nativeFct ) {

	if ( nativeFct != NULL ) {

		JL_CHK( JS_DefineProperty(cx, obj, name, JSVAL_TRUE, NULL, (JSPropertyOp)nativeFct, JSPROP_READONLY | JSPROP_PERMANENT ) );
	} else {

		JL_CHK( JS_DeleteProperty(cx, obj, name) );
		JL_CHK( ReserveNativeInterface(cx, obj, name) );
	}
	return JS_TRUE;
	JL_BAD;
}

ALWAYS_INLINE JSBool GetNativeInterface( JSContext *cx, JSObject *obj, JSObject **obj2p, jsid iid, void **nativeFct ) {

	JSProperty *prop;
	JL_CHKM( obj->lookupProperty(cx, iid, obj2p, &prop), "Unable to get the native interface." ); //(TBD) use JS_LookupPropertyById or JS_GetPropertyById

	if ( prop && obj == *obj2p && ((JSScopeProperty*)prop)->setter != (JSPropertyOp)-1 )
		*nativeFct = (void*)((JSScopeProperty*)prop)->setter; // is NULL if obj is non-native
	else
		*nativeFct = NULL;

	if ( prop )
		(*obj2p)->dropProperty(cx, prop);

	return JS_TRUE;
	JL_BAD;
}


///////////////////////////////////////////////////////////////////////////////
// NativeInterface StreamRead

inline JSBool JSStreamRead( JSContext *cx, JSObject *obj, char *buffer, unsigned int *amount ) {

	JSAutoTempValueRooter tvr(cx); // tvr.addr(); tvr.value()
	JL_CHK( IntToJsval(cx, *amount, tvr.addr()) );
	JL_CHKM( JS_CallFunctionId(cx, obj, JLID(cx, Read), 1, tvr.addr(), tvr.addr()), "Read() function not found.");

	if ( JSVAL_IS_VOID(tvr.value()) ) { // (TBD)! with sockets, undefined mean 'closed', that is not supported.

		*amount = 0;
		return JS_TRUE;
	}

	const char *tmpBuf;
	unsigned int size;
	JL_CHK( JsvalToStringAndLength(cx, tvr.addr(), &tmpBuf, &size) );
	*amount = JL_MIN(size, *amount);
	memcpy(buffer, tmpBuf, *amount);
	return JS_TRUE;

bad:
	return JS_FALSE;
}


inline JSBool ReserveStreamReadInterface( JSContext *cx, JSObject *obj ) {

	return ReserveNativeInterface(cx, obj, JLID_NAME(_NI_StreamRead) );
}

inline JSBool SetStreamReadInterface( JSContext *cx, JSObject *obj, NIStreamRead pFct ) {

	return SetNativeInterface( cx, obj, JLID_NAME(_NI_StreamRead), (void*)pFct );
}

inline NIStreamRead StreamReadNativeInterface( JSContext *cx, JSObject *obj ) {

	void *streamRead;
	JSObject *obj2;
	jsid propId = JLID(cx, _NI_StreamRead);
	if ( propId == 0 || GetNativeInterface( cx, obj, &obj2, propId, &streamRead ) != JS_TRUE )
		return NULL;
	return (NIStreamRead)streamRead;
}

inline NIStreamRead StreamReadInterface( JSContext *cx, JSObject *obj ) {

	void *fct = (void*)StreamReadNativeInterface(cx, obj);
	if ( fct )
		return (NIStreamRead)fct;
	jsval res;
	if ( obj->getProperty(cx, JLID(cx, Read), &res) != JS_TRUE || !JsvalIsFunction(cx, res) )
		return NULL;
	return JSStreamRead;
}


///////////////////////////////////////////////////////////////////////////////
// NativeInterface BufferGet

inline JSBool JSBufferGet( JSContext *cx, JSObject *obj, const char **buffer, unsigned int *size ) {

	JSTempValueRooter tvr;
	JS_PUSH_SINGLE_TEMP_ROOT(cx, JSVAL_NULL, &tvr); // needed to protect the returned value.

//	JS_GetMethodById(cx, obj, JLID(cx, Get), NULL, &tvr.u.value);
//	JS_CallFunctionValue(cx, obj, tvr.u.value, 0, NULL, &tvr.u.value);

//	JL_CHKM( JS_CallFunctionName(cx, obj, "Get", 0, NULL, &tvr.u.value), "Get() function not found."); // do not use toString() !? no !
	JL_CHKM( JS_CallFunctionId(cx, obj, JLID(cx, Get), 0, NULL, &tvr.u.value), "Get() function not found.");
	JL_CHK( JsvalToStringAndLength(cx, &tvr.u.value, buffer, size) ); // (TBD) GC warning, when tvr.u.value will be no more protected, the buffer will be unprotected.

	JS_POP_TEMP_ROOT(cx, &tvr);
	return JS_TRUE;
bad:
	JS_POP_TEMP_ROOT(cx, &tvr);
	return JS_FALSE;
}

inline JSBool ReserveBufferGetInterface( JSContext *cx, JSObject *obj ) {

	return ReserveNativeInterface(cx, obj, JLID_NAME(_NI_BufferGet) );
}

inline JSBool SetBufferGetInterface( JSContext *cx, JSObject *obj, NIBufferGet pFct ) {

	return SetNativeInterface( cx, obj, JLID_NAME(_NI_BufferGet), (void*)pFct );
}

inline NIBufferGet BufferGetNativeInterface( JSContext *cx, JSObject *obj ) {

	void *fct;
	JSObject *obj2;
	jsid propId = JLID(cx, _NI_BufferGet);
	if ( propId == 0 || GetNativeInterface( cx, obj, &obj2, propId, &fct ) != JS_TRUE )
		return NULL;
	return (NIBufferGet)fct;
}

inline NIBufferGet BufferGetInterface( JSContext *cx, JSObject *obj ) {

	void *fct = (void*)BufferGetNativeInterface(cx, obj);
	if ( fct )
		return (NIBufferGet)fct;

	jsval res;
	if ( obj->getProperty(cx, JLID(cx, Get), &res) != JS_TRUE || !JsvalIsFunction(cx, res) ) // do not use toString() directly, but Get can call toString().
		return NULL;

	return JSBufferGet;
}


///////////////////////////////////////////////////////////////////////////////
// NativeInterface Matrix44Get

/*
inline JSBool JSMatrix44Get( JSContext *cx, JSObject *obj, const char **buffer, unsigned int *size ) {


	JS_PUSH_SINGLE_TEMP_ROOT(cx, rval, &tvr);
	&tvr.u.value
	...

	JL_CHKM( JS_CallFunctionName(cx, obj, "Get", 0, NULL, &rval), "Get() function not found."); // do not use toString() !?
	JL_CHK( JsvalToStringAndLength(cx, rval, buffer, size) );
	return JS_TRUE;
	JL_BAD;
}
*/

inline JSBool ReserveMatrix44GetInterface( JSContext *cx, JSObject *obj ) {

	return ReserveNativeInterface(cx, obj, JLID_NAME(_NI_Matrix44Get) );
}

inline JSBool SetMatrix44GetInterface( JSContext *cx, JSObject *obj, NIMatrix44Get pFct ) {

	return SetNativeInterface( cx, obj, JLID_NAME(_NI_Matrix44Get), (void*)pFct );
}

inline NIMatrix44Get Matrix44GetNativeInterface( JSContext *cx, JSObject *obj ) {

	void *fct;
	JSObject *obj2;
	jsid propId = JLID(cx, _NI_Matrix44Get);
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
	jsid propId = GetPrivateJsid(cx, GetHostPrivate(cx), "GetMatrix", PRIVATE_JSID_GetMatrix);
	if ( obj->getProperty(cx, propId, &res) != JS_TRUE != JS_TRUE || !JsvalIsFunction(cx, res) )
		return NULL;
	return JSMatrix44Get;
*/
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
//

inline JSBool JsvalToMatrix44( JSContext *cx, jsval val, float **m ) {

	static float Matrix44IdentityValue[16] = {
		 1.0f, 0.0f, 0.0f, 0.0f,
		 0.0f, 1.0f, 0.0f, 0.0f,
		 0.0f, 0.0f, 1.0f, 0.0f,
		 0.0f, 0.0f, 0.0f, 1.0f
	};

	JL_S_ASSERT_OBJECT(val);

	JSObject *matrixObj;
	matrixObj = JSVAL_TO_OBJECT(val);

	if ( JSVAL_IS_NULL(val) ) {

		memcpy(*m, &Matrix44IdentityValue, sizeof(Matrix44IdentityValue));
		return JS_TRUE;
	}

	NIMatrix44Get Matrix44Get;
	Matrix44Get = Matrix44GetInterface(cx, matrixObj);
	if ( Matrix44Get )
		return Matrix44Get(cx, matrixObj, m);

	if ( JS_IsArrayObject(cx, matrixObj) ) {

		uint32 length;
		jsval element;
		JL_CHK( JS_GetElement(cx, JSVAL_TO_OBJECT(val), 0, &element) );
		if ( JsvalIsArray(cx, element) ) { // support for [ [1,1,1,1], [2,2,2,2], [3,3,3,3], [4,4,4,4] ] matrix

			JL_CHK( JsvalToFloatVector(cx, element, (*m)+0, 4, &length ) );
			JL_S_ASSERT( length == 4, "Too few (%d) elements in the array.", length );

			JL_CHK( JS_GetElement(cx, JSVAL_TO_OBJECT(val), 1, &element) );
			JL_S_ASSERT_ARRAY( element );
			JL_CHK( JsvalToFloatVector(cx, element, (*m)+4, 4, &length ) );
			JL_S_ASSERT( length == 4, "Too few (%d) elements in the array.", length );

			JL_CHK( JS_GetElement(cx, JSVAL_TO_OBJECT(val), 2, &element) );
			JL_S_ASSERT_ARRAY( element );
			JL_CHK( JsvalToFloatVector(cx, element, (*m)+8, 4, &length ) );
			JL_S_ASSERT( length == 4, "Too few (%d) elements in the array.", length );

			JL_CHK( JS_GetElement(cx, JSVAL_TO_OBJECT(val), 3, &element) );
			JL_S_ASSERT_ARRAY( element );
			JL_CHK( JsvalToFloatVector(cx, element, (*m)+12, 4, &length ) );
			JL_S_ASSERT( length == 4, "Too few (%d) elements in the array.", length );
			return JS_TRUE;
		}

		JL_CHK( JsvalToFloatVector(cx, val, *m, 16, &length ) );  // support for [ 1,1,1,1, 2,2,2,2, 3,3,3,3, 4,4,4,4 ] matrix
		JL_S_ASSERT( length == 16, "Too few (%d) elements in the array.", length );
		return JS_TRUE;
	}

	JL_REPORT_ERROR("Unable to read a 4x4 matrix.");
	JL_BAD;
}


///////////////////////////////////////////////////////////////////////////////
// MetaPoll

struct MetaPoll {
	void (*startPoll)( volatile MetaPoll *mp ); // starts the blocking thread and call signalEvent() when an event has arrived.
	bool (*cancelPoll)( volatile MetaPoll *mp ); // unlock the blocking thread event if no event has arrived (mean that an event has arrived in another thread).
	JSBool (*endPoll)( volatile MetaPoll *mp, bool *hasEvent, JSContext *cx, JSObject *obj ); // process the result
};

#endif // _JSHELPER_H_
