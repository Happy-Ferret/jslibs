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

#include "stdafx.h"

#include <jslibsModule.h>

#define SVN_REVISION_STR "$Revision$"
#include "host.h"
#include "../jslang/jslang.h"

/*
JSErrorFormatString jlErrorFormatString[] = {
#define JLMSG_DEF(name, exception, format, count) { format, count, exception },
#include "jlerrors.msg"
#undef JLMSG_DEF
};


const JSErrorFormatString *GetErrorMessage(void *, const char *, const uintN errorNumber) {

	if ( errorNumber > JLErrOffset && errorNumber < JLErrLimit )
		return &jlErrorFormatString[errorNumber-JLErrOffset-1];
	return NULL;
}
*/



struct {
	JSExnType exn;
	const char *msg;
} E_msg[] = {
		{ JSEXN_NONE, 0 },
	#define DEF( NAME, TEXT, EXN ) \
		{ EXN, TEXT },
	#include "jlerrors.msg"
	#undef DEF
};


static const JSErrorFormatString *
ErrorCallback(void *userRef, const char *, const uintN) {

	return (JSErrorFormatString*)userRef;
}


static JSBool
Report( JSContext *cx, bool isWarning, ... ) {

   va_list vl;
	va_start(vl, isWarning);

	int id;
	JSExnType exn = JSEXN_NONE;

	char message[1024];
	char *buf = message;
	const char *str, *strEnd, *pos;

	while ( (id = va_arg(vl, int)) != E__INVALID ) {

		ASSERT( id > E__INVALID );
		ASSERT( id < E__LIMIT );

		if ( exn == JSEXN_NONE && E_msg[id].exn != JSEXN_NONE )
			exn = E_msg[id].exn;

		str = E_msg[id].msg;

		if ( buf != message ) {

			jl_memcpy(buf, " ", 1);
			buf += 1;
		}

		strEnd = str + strlen(str);
		pos = str;

		for (;;) {
			
			const char *newPos = strchr(pos, '%');
			if ( !newPos ) {

				jl_memcpy(buf, pos, strEnd-pos);
				buf += strEnd-pos;
				break;
			} else {

				jl_memcpy(buf, pos, newPos-pos);
				buf += newPos-pos;
			}
			pos = newPos;

			switch ( *++pos ) {
				case 'd':
					++pos;
					JL_itoa(va_arg(vl, long), buf, 10);
					buf += strlen(buf);
					break;
				case 'x':
					++pos;
					jl_memcpy(buf, "0x", 2);
					buf += 2;
					JL_itoa(va_arg(vl, long), buf, 16);
					buf += strlen(buf);
					break;
				case 's': {
					++pos;
					const char * tmp = va_arg(vl, char *);
					int len = strlen(tmp);
					if ( len > 128 ) {
						
						jl_memcpy(buf, tmp, 128);
						buf += 128;
						jl_memcpy(buf, "...", 3);
						buf += 3;
					} else {

						jl_memcpy(buf, tmp, len);
						buf += len;
					}
					break;
				}
				default:
					*(buf++) = '%';
					break;
			}
		}
	}
	jl_memcpy(buf, ".", 1);
	buf += 1;
	*buf = '\0';

	va_end(vl);

	JSErrorFormatString format = { message, 0, (int16_t)exn };
	return JS_ReportErrorFlagsAndNumber(cx, isWarning ? JSREPORT_WARNING : JSREPORT_ERROR, ErrorCallback, (void*)&format, 0);

//bad:
//	va_end(vl);
//	return JS_FALSE;
}


JSBool JSDefaultStdinFunction(JSContext *cx, uintN, jsval *vp) {

	HostPrivate *pv = JL_GetHostPrivate(cx);
	if (unlikely( pv == NULL || pv->hostStdIn == NULL )) {

		*JL_RVAL = JSVAL_VOID;
		return JS_TRUE;
	}
	char buffer[8192];
	int status = pv->hostStdIn(pv->privateData, buffer, COUNTOF(buffer));
	if ( status > 0 ) {
	
		JSString *jsstr = JS_NewStringCopyN(cx, buffer, status);
		JL_CHK( jsstr );
		*JL_RVAL = STRING_TO_JSVAL( jsstr );
	} else {

		JL_WARN( E_HOST, E_INTERNAL, E_SEP, E_COMMENT("stdin"), E_READ );
		*JL_RVAL = JL_GetEmptyStringValue(cx);
	}
	return JS_TRUE;
	JL_BAD;
}


// route: print() => _host->stdout() => JSDefaultStdoutFunction() => pv->hostStdOut()
JSBool JSDefaultStdoutFunction(JSContext *cx, uintN argc, jsval *vp) {

	*JL_RVAL = JSVAL_VOID;
	HostPrivate *pv = JL_GetHostPrivate(cx);
	if (unlikely( pv == NULL || pv->hostStdOut == NULL ))
		return JS_TRUE;

	for ( uintN i = 0; i < argc; ++i ) {

		JLData str;
		JL_CHK( JL_JsvalToNative(cx, JL_ARGV[i], &str) );
		int status = pv->hostStdOut(pv->privateData, str.GetConstStr(), str.Length());
		JL_ASSERT_WARN( status != -1, E_HOST, E_INTERNAL, E_SEP, E_COMMENT("stdout"), E_WRITE );
	}
	return JS_TRUE;
	JL_BAD;
}


JSBool JSDefaultStderrFunction(JSContext *cx, uintN argc, jsval *vp) {

	*JL_RVAL = JSVAL_VOID;
	HostPrivate *pv = JL_GetHostPrivate(cx);
	if (unlikely( pv == NULL || pv->hostStdErr == NULL ))
		return JS_TRUE;

	for ( uintN i = 0; i < argc; ++i ) {

		JLData str;
		JL_CHK( JL_JsvalToNative(cx, JL_ARGV[i], &str) );
		int status = pv->hostStdErr(pv->privateData, str.GetConstStr(), str.Length());
		JL_ASSERT_WARN( status != -1, E_HOST, E_INTERNAL, E_SEP, E_COMMENT("stderr"), E_WRITE );
	}
	return JS_TRUE;
	JL_BAD;
}


void ErrorReporterBasic(JSContext *cx, const char *message, JSErrorReport *report) {

	JL_IGNORE(cx);
	if ( !report )
		fprintf(stderr, "%s\n", message);
	else
		fprintf(stderr, "%s (%s:%d)\n", message, report->filename, report->lineno);
}


void StderrWrite(JSContext *cx, const char *message, size_t length) {

	JSObject *globalObject = JL_GetGlobal(cx);
	ASSERT( globalObject );

	jsval fct;
	if (unlikely( GetHostObjectValue(cx, JLID(cx, stderr), &fct) != JS_TRUE || !JL_ValueIsCallable(cx, fct) ))
		return;
		
	JSExceptionState *exs = JS_SaveExceptionState(cx);
	JS_ClearPendingException(cx);
	JSErrorReporter prevErrorReporter = JS_SetErrorReporter(cx, JL_IS_SAFE ? ErrorReporterBasic : NULL);

	jsval rval, text;
	JL_CHKB( JL_NativeToJsval(cx, message, length, &text), bad1 ); // beware out of memory case !
	JL_CHKB( JS_CallFunctionValue(cx, globalObject, fct, 1, &text, &rval), bad1 );

	JS_SetErrorReporter(cx, prevErrorReporter);
	JS_RestoreExceptionState(cx, exs);
	return;

bad1:
	JS_SetErrorReporter(cx, prevErrorReporter);
	JS_DropExceptionState(cx, exs);
	return;
}

/* defined in jsfriendapi.h
typedef enum JSErrNum {
#define MSG_DEF(name, number, count, exception, format) \
    name = number,
#include "js.msg"
#undef MSG_DEF
    JSErr_Limit
} JSErrNum;
*/


void ErrorReporter(JSContext *cx, const char *message, JSErrorReport *report) {

	// trap JSMSG_OUT_OF_MEMORY error to avoid calling ErrorReporter_stdErrRouter() that may allocate memory that will lead to nested call.
	if (unlikely( report && report->errorNumber == JSMSG_OUT_OF_MEMORY )) {
		
		ErrorReporterBasic(cx, message, report);
		return;
	}

	bool reportWarnings = JL_IS_SAFE; // no warnings in unsafe mode.

	char buffer[1024];
	char *buf = buffer;

	#if defined(fprintf) || defined(fputs) || defined(fwrite) || defined(fputc)
		#error CANNOT DEFINE MACROS fprintf, fputs, fwrite, fputc
	#endif

	#define fprintf(FILE, FORMAT, ...) \
	JL_MACRO_BEGIN \
		size_t remaining = sizeof(buffer)-(buf-buffer); \
		if ( remaining == 0 ) break; \
		int count = snprintf(buf, remaining, FORMAT, ##__VA_ARGS__); \
		buf += count < 0 ? remaining : count; \
	JL_MACRO_END

	#define fputs(STR, FILE) \
	JL_MACRO_BEGIN \
		size_t remaining = sizeof(buffer)-(buf-buffer); \
		if ( remaining == 0 ) break; \
		size_t len = JL_MIN(strlen(STR), remaining); \
		jl_memcpy(buf, STR, len); \
		buf += len; \
	JL_MACRO_END

	#define fwrite(STR, SIZE, COUNT, FILE) \
	JL_MACRO_BEGIN \
		size_t remaining = sizeof(buffer)-(buf-buffer); \
		if ( remaining == 0 ) break; \
		size_t len = JL_MIN(size_t((SIZE)*(COUNT)), remaining); \
		jl_memcpy(buf, (STR), len); \
		buf += len; \
	JL_MACRO_END

	#define fputc(CHR, FILE) \
	JL_MACRO_BEGIN \
		size_t remaining = sizeof(buffer)-(buf-buffer); \
		if ( remaining == 0 ) break; \
		buf[0] = (CHR); \
		buf += 1; \
	JL_MACRO_END

	#define fflush(FILE) \
	JL_MACRO_BEGIN \
	JL_MACRO_END


// copy-paste from /js/src/js.cpp (my_ErrorReporter)
//	 ---8<---

    int i, j, k, n;
    char *prefix, *tmp;
    const char *ctmp;

    if (!report) {
        fprintf(gErrFile, "%s\n", message);
        fflush(gErrFile);
        return;
    }

    /* Conditionally ignore reported warnings. */
    if (JSREPORT_IS_WARNING(report->flags) && !reportWarnings)
        return;

    prefix = NULL;
    if (report->filename)
        prefix = JS_smprintf("%s:", report->filename);
    if (report->lineno) {
        tmp = prefix;
        prefix = JS_smprintf("%s%u: ", tmp ? tmp : "", report->lineno);
        JS_free(cx, tmp);
    }
    if (JSREPORT_IS_WARNING(report->flags)) {
        tmp = prefix;
        prefix = JS_smprintf("%s%swarning: ",
                             tmp ? tmp : "",
                             JSREPORT_IS_STRICT(report->flags) ? "strict " : "");
        JS_free(cx, tmp);
    }

    /* embedded newlines -- argh! */
    while ((ctmp = strchr(message, '\n')) != 0) {
        ctmp++;
        if (prefix)
            fputs(prefix, gErrFile);
        fwrite(message, 1, ctmp - message, gErrFile);
        message = ctmp;
    }

    /* If there were no filename or lineno, the prefix might be empty */
    if (prefix)
        fputs(prefix, gErrFile);
    fputs(message, gErrFile);

    if (!report->linebuf) {
        fputc('\n', gErrFile);
        goto out;
    }

    /* report->linebuf usually ends with a newline. */
    n = strlen(report->linebuf);
    fprintf(gErrFile, ":\n%s%s%s%s",
            prefix,
            report->linebuf,
            (n > 0 && report->linebuf[n-1] == '\n') ? "" : "\n",
            prefix);
    n = report->tokenptr - report->linebuf;
    for (i = j = 0; i < n; i++) {
        if (report->linebuf[i] == '\t') {
            for (k = (j + 8) & ~7; j < k; j++) {
                fputc('.', gErrFile);
            }
            continue;
        }
        fputc('.', gErrFile);
        j++;
    }
    fputs("^\n", gErrFile);
out:
    //if (!JSREPORT_IS_WARNING(report->flags)) {
    //    if (report->errorNumber == JSMSG_OUT_OF_MEMORY) {
    //        gExitCode = EXITCODE_OUT_OF_MEMORY;
    //    } else {
    //        gExitCode = EXITCODE_RUNTIME_ERROR;
    //    }
    //}
    JS_free(cx, prefix);

//	 ---8<---

	#undef fprintf
	#undef fputs
	#undef fwrite
	#undef fputc
	#undef fflush
	 
	StderrWrite(cx, buffer, buf-buffer);
}



JSBool OperationCallback(JSContext *cx) {

	JSOperationCallback tmp = JS_SetOperationCallback(cx, NULL);
	JS_MaybeGC(cx);
	JS_SetOperationCallback(cx, tmp);
	return JS_TRUE;
}


JLThreadFuncDecl WatchDogThreadProc(void *threadArg) {

	JSContext *cx = (JSContext*)threadArg;
	HostPrivate *pv = JL_GetHostPrivate(cx);
	for (;;) {

		if ( JLSemaphoreAcquire(pv->watchDogSemEnd, pv->maybeGCInterval) != JLTIMEOUT ) // used as a breakable Sleep instead of SleepMilliseconds (see SandboxEval).
			break;
		JS_TriggerOperationCallback(cx);
	}
	JLThreadExit(0);
	return 0;
}


JSBool LoadModule(JSContext *cx, uintN argc, jsval *vp) {

	JLData str;
	JLLibraryHandler module = JLDynamicLibraryNullHandler;

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARGC_MIN(1);


	if (unlikely( JL_ARGC < 1 )) {

		*JL_RVAL = JSVAL_VOID;
		return JS_TRUE;
	}

	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &str) );

	char libFileName[PATH_MAX];
	strncpy( libFileName, str.GetConstStr(), str.Length() );
	libFileName[str.Length()] = '\0';
	strcat( libFileName, DLL_EXT );
// MAC OSX: 	'@executable_path' ??

/*
	while (0) { // namespace management. Avoid using val ns = {}, loadModule.call(ns, '...');

		if ( JL_ARG_ISDEF(2) ) {

			if ( JSVAL_IS_OBJECT(JL_ARG(2)) ) {
				obj = JSVAL_TO_OBJECT(JL_ARG(2));
			} else {
				const char *ns;
				JL_CHK( JL_JsvalToNative(cx, &JL_ARG(2), &ns) );

				jsval existingNsVal;
				JL_CHK( JS_GetProperty(cx, obj, ns, &existingNsVal) );
				JSObject *nsObj;
				if ( existingNsVal == JSVAL_VOID ) {

					nsObj = JS_NewObject(cx, NULL, NULL, NULL);
					JL_CHK( JS_DefineProperty(cx, obj, ns, OBJECT_TO_JSVAL(nsObj), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT) ); // doc. On success, JS_DefineProperty returns JS_TRUE. If the property already exists or cannot be created, JS_DefineProperty returns JS_FALSE.
				} else {

					JL_ASSERT_OBJECT( existingNsVal );
					nsObj = JSVAL_TO_OBJECT( existingNsVal );
				}
				obj = nsObj;
			}
		}
	}
*/

	HostPrivate *pv;
	pv = JL_GetHostPrivate(cx);
	JL_ASSERT( pv, E_HOST, E_STATE, E_COMMENT("context private") );
	JL_ASSERT( libFileName != NULL && *libFileName != '\0', E_ARG, E_NUM(1), E_DEFINED );

	module = JLDynamicLibraryOpen(libFileName);
	if ( !JLDynamicLibraryOk(module) ) {

		JL_SAFE_BEGIN
		char errorBuffer[256];
		JLDynamicLibraryLastErrorMessage( errorBuffer, sizeof(errorBuffer) );
		JL_WARN( E_OS, E_OPERATION, E_DETAILS, E_STR(errorBuffer) );
		JL_SAFE_END

		*JL_RVAL = JSVAL_FALSE;
		return JS_TRUE;
	}
	for ( jl::QueueCell *it = jl::QueueBegin(&pv->moduleList); it; it = jl::QueueNext(it) )
		if ( (JLLibraryHandler)jl::QueueGetData(it) == module ) {

			JLDynamicLibraryClose(&module);
			*JL_RVAL = JSVAL_VOID; // already loaded
			return JS_TRUE;
		}

	uint32_t uid;
	uid = JLDynamicLibraryId(module); // module unique ID
	ModuleInitFunction moduleInit;
	moduleInit = (ModuleInitFunction)JLDynamicLibrarySymbol(module, NAME_MODULE_INIT);
	JL_ASSERT( moduleInit, E_MODULE, E_NAME(libFileName), E_INIT ); // "Invalid module."
	
//	CHKHEAP();

	if ( !moduleInit(cx, JL_OBJ, uid) ) {

		if ( JL_IsExceptionPending(cx) )
			goto bad;
		char filename[PATH_MAX];
		JLDynamicLibraryName((void*)moduleInit, filename, sizeof(filename));
		JL_ERR( E_MODULE, E_NAME(filename), E_INIT );
		goto bad;
	}

//	CHKHEAP();

	jl::QueueUnshift( &pv->moduleList, module ); // store the module (LIFO)
	
	//JL_CHK( JL_NewNumberValue(cx, uid, JL_RVAL) ); // really needed ? yes, UnloadModule will need this ID, ... but UnloadModule is too complicated to implement and will never exist.
	*JL_RVAL = OBJECT_TO_JSVAL(JL_OBJ);

	return JS_TRUE;

bad:
	if ( JLDynamicLibraryOk(module) )
		JLDynamicLibraryClose(&module);
	return JS_FALSE;
}


// global object
// doc: For full ECMAScript standard compliance, obj should be of a JSClass that has the JSCLASS_GLOBAL_FLAGS flag.
// note: global_class is a global variable, but this is not an issue even if several runtimes share the same JSClass.
static JSClass global_class = {
	NAME_GLOBAL_CLASS, JSCLASS_GLOBAL_FLAGS,
	JS_PropertyStub, JS_PropertyStub,
	JS_PropertyStub, JS_StrictPropertyStub,
	JS_EnumerateStub, JS_ResolveStub,
	JS_ConvertStub, JS_FinalizeStub,
	JSCLASS_NO_OPTIONAL_MEMBERS
};


// default: CreateHost(-1, -1, 0);
JSContext* CreateHost(uint32_t maxMem, uint32_t maxAlloc, uint32_t maybeGCInterval ) {

	JSRuntime *rt = JS_NewRuntime(maxAlloc); // JSGC_MAX_MALLOC_BYTES
	JL_CHK( rt );

	//JS_SetGCParameter(rt, JSGC_MAX_MALLOC_BYTES, maxAlloc); // Number of JS_malloc bytes before last ditch GC.

	// doc: maxMem specifies the number of allocated bytes after which garbage collection is run.
	JS_SetGCParameter(rt, JSGC_MAX_BYTES, maxMem); // Maximum nominal heap before last ditch GC.

	JSContext *cx;
	cx = JS_NewContext(rt, 8192); // set the chunk size of the stack pool to 8192. see http://groups.google.com/group/mozilla.dev.tech.js-engine/browse_thread/thread/be9f404b623acf39/9efdfca81be99ca3
	JL_CHK( cx ); //, "unable to create the context." );

	// Info: Increasing JSContext stack size slows down my scripts:
	//   http://groups.google.com/group/mozilla.dev.tech.js-engine/browse_thread/thread/be9f404b623acf39/9efdfca81be99ca3


/* 2012.02.10 removed ?
	uint32_t maxCodeCacheBytes;
	maxCodeCacheBytes = JS_GetGCParameterForThread(cx, JSGC_MAX_CODE_CACHE_BYTES);
	ASSERT( maxCodeCacheBytes == 16 * 1024 * 1024 ); // check if default has chaged.
	JS_SetGCParameterForThread(cx, JSGC_MAX_CODE_CACHE_BYTES, 16 * 1024 * 1024 * 3/2);
*/
	//JS_SetNativeStackQuota(cx, DEFAULT_MAX_STACK_SIZE); // see https://developer.mozilla.org/En/SpiderMonkey/JSAPI_User_Guide

	JS_SetVersion(cx, JSVERSION_LATEST);

	JS_SetErrorReporter(cx, ErrorReporter);

	// JSOPTION_ANONFUNFIX: https://bugzilla.mozilla.org/show_bug.cgi?id=376052 
	// JS_SetOptions doc: https://developer.mozilla.org/en/SpiderMonkey/JSAPI_Reference/JS_SetOptions
	ASSERT( JS_GetOptions(cx) == 0 );
	JS_SetOptions(cx, JSOPTION_VAROBJFIX | JSOPTION_XML | JSOPTION_RELIMIT | JSOPTION_METHODJIT | JSOPTION_TYPE_INFERENCE); // beware: avoid using JSOPTION_COMPILE_N_GO here.

	JSObject *globalObject;
	globalObject = JS_NewCompartmentAndGlobalObject(cx, &global_class, NULL);
	JL_CHK( globalObject ); // "unable to create the global object." );

	//	doc: As a side effect, JS_InitStandardClasses establishes obj as the global object for cx, if one is not already established.
	JS_SetGlobalObject(cx, globalObject); // no call to JS_InitStandardClasses(), then JS_SetGlobalObject() is required (see also LAZY_STANDARD_CLASSES).

	JL_CHK( JS_InitStandardClasses(cx, globalObject) );
	JL_CHK( JS_DefineDebuggerObject(cx, globalObject) ); // doc: https://developer.mozilla.org/en/SpiderMonkey/JS_Debugger_API_Guide
	JL_CHK( JS_InitReflect(cx, globalObject) );
#ifdef JS_HAS_CTYPES
	JL_CHK( JS_InitCTypesClass(cx, globalObject) );
#endif

	HostPrivate *pv = ConstructHostPrivate();

	JL_ASSERT_ALLOC( pv );
	pv->hostPrivateVersion = JL_HOST_PRIVATE_VERSION;
	JL_SetHostPrivate(cx, pv);

	// setup WatchDog
	if ( maybeGCInterval ) {

		pv->maybeGCInterval = maybeGCInterval;
		JSOperationCallback prevOperationCallback;
		prevOperationCallback = JS_SetOperationCallback(cx, OperationCallback);
		ASSERT( prevOperationCallback == NULL );
		pv->watchDogSemEnd = JLSemaphoreCreate(0);
		pv->watchDogThread = JLThreadStart(WatchDogThreadProc, cx);
		//	JLThreadPriority(pv->watchDogThread, JL_THREAD_PRIORITY_LOW);
		JL_ASSERT( JLSemaphoreOk(pv->watchDogSemEnd) && JLThreadOk(pv->watchDogThread), E_HOST, E_CREATE ); // "Unable to create the GC thread."
	}
	return cx;

bad:
	return NULL;
}

/**qa
	QA.ASSERT_EQ( '==', processEvents, global.processEvents );
	QA.ASSERT_EQ( '>', _host.buildDate, 0 );
	QA.ASSERT_EQ( '>', _host.buildDate, +new Date(2012, 3 -1, 1) );
	QA.ASSERT_EQ( '<=', _host.buildDate, Date.now() - new Date().getTimezoneOffset() * 60 * 1000 );
**/
JSBool InitHost( JSContext *cx, bool unsafeMode, HostInput stdIn, HostOutput stdOut, HostOutput stdErr, void* userPrivateData ) { // init the host for jslibs usage (modules, errors, ...)

	ASSERT( !JS_CStringsAreUTF8() );

	_unsafeMode = unsafeMode;
	HostPrivate *pv = JL_GetHostPrivate(cx);
	if ( pv == NULL ) { // in the case of CreateHost has not been called (because the caller wants to create and manage its own JS runtime)

		pv = ConstructHostPrivate();
		JL_ASSERT_ALLOC( pv );
		pv->hostPrivateVersion = JL_HOST_PRIVATE_VERSION;
		JL_SetHostPrivate(cx, pv);
	}

	pv->privateData = userPrivateData;

	jl::QueueInitialize(&pv->moduleList);

	pv->unsafeMode = unsafeMode;

	if ( unsafeMode )
		JS_SetOptions(cx, JS_GetOptions(cx) & ~(JSOPTION_STRICT | JSOPTION_RELIMIT));

	JSObject *globalObject;
	globalObject = JL_GetGlobal(cx);
	ASSERT( globalObject != NULL ); // "Global object not found."
	
	pv->report = Report;

	pv->hostStdErr = stdErr;
	pv->hostStdOut = stdOut;
	pv->hostStdIn = stdIn;

	// Object class & proto cache ( see JL_NewObj() )
	
	//pv->objectClass = JL_GetStandardClassByKey(cx, JSProto_Object);
	//pv->objectProto = JL_GetStandardClassProtoByKey(cx, JSProto_Object);


	//JSObject *newObject = JS_NewObject(cx, NULL, NULL, NULL);
	//pv->objectClass = JL_GetClass(newObject);
	//pv->objectProto = JL_GetPrototype(cx, newObject);
	js_GetClassPrototype(cx, globalObject, JSProto_Object, &pv->objectProto, NULL);
	pv->objectClass = JL_GetClass(pv->objectProto);
	ASSERT( pv->objectClass && pv->objectProto );
	
	// global functions & properties
	JL_CHKM( JS_DefinePropertyById( cx, globalObject, JLID(cx, global), OBJECT_TO_JSVAL(globalObject), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT ), E_PROP, E_CREATE );
	JL_CHKM( JS_DefineFunction( cx, globalObject, JL_GetHostPrivate(cx)->camelCase == JL_CAMELCASE_UPPER ? JLNormalizeFunctionName(NAME_GLOBAL_FUNCTION_LOAD_MODULE) : NAME_GLOBAL_FUNCTION_LOAD_MODULE, LoadModule, 0, 0 ), E_PROP, E_CREATE );

	JL_CHK( SetHostObjectValue(cx, JLID(cx, unsafeMode), BOOLEAN_TO_JSVAL(unsafeMode), false) );

	// note: we have to support: var prevStderr = _host.stderr; _host.stderr = function(txt) { file.Write(txt); prevStderr(txt) };

	 // doc: If you do not assign a name to the function, it is assigned the name "anonymous".
	JL_CHK( SetHostObjectValue(cx, JLID(cx, stdin), OBJECT_TO_JSVAL(JS_GetFunctionObject(JS_NewFunctionById(cx, JSDefaultStdinFunction, 1, 0, NULL, JLID(cx, stdin))))) );
	JL_CHK( SetHostObjectValue(cx, JLID(cx, stdout), OBJECT_TO_JSVAL(JS_GetFunctionObject(JS_NewFunctionById(cx, JSDefaultStdoutFunction, 1, 0, NULL, JLID(cx, stdout))))) );
	JL_CHK( SetHostObjectValue(cx, JLID(cx, stderr), OBJECT_TO_JSVAL(JS_GetFunctionObject(JS_NewFunctionById(cx, JSDefaultStderrFunction, 1, 0, NULL, JLID(cx, stderr))))) );

	JL_CHK( SetHostObjectValue(cx, L("revision"), INT_TO_JSVAL(JL_SvnRevToInt(SVN_REVISION_STR))) ); // or JLID(cx, _revision) ?
	JL_CHK( SetHostObjectValue(cx, L("buildDate"), DOUBLE_TO_JSVAL((double)__DATE__EPOCH * 1000)) );
	JL_CHK( SetHostObjectValue(cx, L("jsVersion"), INT_TO_JSVAL(JS_GetVersion(cx))) ); // see also JS_GetImplementationVersion()

	// init static modules
	if ( !jslangModuleInit(cx, globalObject) )
		JL_ERR( E_MODULE, E_NAME("jslang"), E_INIT );

//	JL_CHK( JS_DefinePropertyById(cx, globalObject, JLID(cx, _revision), INT_TO_JSVAL(JL_SvnRevToInt(SVN_REVISION_STR)), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT) ); // see _host.revision

	return JS_TRUE;

bad:
	return JS_FALSE;
}


JSBool DestroyHost( JSContext *cx, bool skipCleanup ) {

	JSRuntime *rt = JL_GetRuntime(cx);

	HostPrivate *pv = JL_GetHostPrivate(cx);
	JL_ASSERT( pv, E_HOST, E_STATE, E_COMMENT("context private") );

	pv->canSkipCleanup = skipCleanup;
	
	JL_ASSERT( pv->isEnding == false );
	pv->isEnding = true;

	if ( JLThreadOk(pv->watchDogThread) ) {

		// beware: it is important to destroy the watchDogThread BEFORE destroying the cx or pv !!!
		JLSemaphoreRelease(pv->watchDogSemEnd);
		JLThreadWait(pv->watchDogThread, NULL);
		JLThreadFree(&pv->watchDogThread);
		JLSemaphoreFree(&pv->watchDogSemEnd);
	}

	for ( jl::QueueCell *it = jl::QueueBegin(&pv->moduleList); it; it = jl::QueueNext(it) ) {

		JLLibraryHandler module = (JLLibraryHandler)jl::QueueGetData(it);
		ModuleReleaseFunction moduleRelease = (ModuleReleaseFunction)JLDynamicLibrarySymbol(module, NAME_MODULE_RELEASE);
		if ( moduleRelease != NULL ) {
		
			if ( !moduleRelease(cx) ) {

				//if ( !JL_IsExceptionPending(cx) ) { // (TBD)
				char filename[PATH_MAX];
				JLDynamicLibraryName((void*)moduleRelease, filename, sizeof(filename));
				JL_WARN( E_MODULE, E_NAME(filename), E_FIN ); // "Fail to release module \"%s\".", filename
			}
		}
	}

	if ( !jslangModuleRelease(cx) ) {
		
		JL_WARN( E_MODULE, E_NAME("jslang"), E_FIN ); // "Fail to release static module jslang."
	}

	//	don't try to break linked objects with JS_GC(cx) !

//	jsval tmp;
//	JL_CHK( GetConfigurationValue(cx, JLID_NAME(_getErrorMessage), &tmp) );
//	if ( tmp != JSVAL_VOID && JSVAL_TO_PRIVATE(tmp) )
//		jl_free( JSVAL_TO_PRIVATE(tmp) );

	JL_CHKM( RemoveHostObject(cx), E_HOST, E_INTERNAL ); // "Unable to remove the host object."

// cleanup

	// doc:
	//  - Is the only side effect of JS_DestroyContextNoGC that any finalizers I may have specified in custom objects will not get called ?
	//  - Not if you destroy all contexts (whether by NoGC or not), destroy all runtimes, and call JS_ShutDown before exiting or hibernating.
	//    The last JS_DestroyContext* API call will run a GC, no matter which API of that form you call on the last context in the runtime. /be
	JS_DestroyContext(cx);
	JS_DestroyRuntime(rt);
	cx = NULL;

	// Beware: because JS engine allocate memory from the DLL, all memory must be disallocated before releasing the DLL

	while ( !jl::QueueIsEmpty(&pv->moduleList) ) {

		JLLibraryHandler module = (JLLibraryHandler)jl::QueueShift(&pv->moduleList);
		ModuleFreeFunction moduleFree = (ModuleFreeFunction)JLDynamicLibrarySymbol(module, NAME_MODULE_FREE);
		if ( moduleFree != NULL )
			moduleFree();
//#ifndef DEBUG // else the memory block was allocated in a DLL that was unloaded prior to the _CrtMemDumpAllObjectsSince() call.
		JLDynamicLibraryClose(&module);
//#endif
	}

	jslangModuleFree();

	DestructHostPrivate(pv);

	return JS_TRUE;

bad:
	// on error, do the minimum.
	if ( cx ) {
		
		JS_DestroyContext(cx);
		JS_DestroyRuntime(rt);
	}
	return JS_FALSE;
}


/*
void HostPrincipalsDestroy(JSContext *cx, JSPrincipals *principals) {

	jl_free(principals->codebase);
	jl_free(principals);
}
*/

JSBool RemoveScriptArguments( JSContext *cx ) {

	JSObject *globalObject = JL_GetGlobal(cx);
	JL_ASSERT( globalObject != NULL, E_HOST, E_INTERNAL ); // "Global object not found."
	JL_CHKM( JS_DeletePropertyById(cx, globalObject, JLID(cx, arguments)), E_HOST, E_INTERNAL ); // "Unable to cleanup script arguments." // beware: permanant properties cannot be removed.
	return JS_TRUE;
	JL_BAD;
}

JSBool CreateScriptArguments( JSContext *cx, int argc, const char * const * argv ) {

	JSObject *globalObject = JL_GetGlobal(cx);
	JL_ASSERT( globalObject != NULL, E_HOST, E_INTERNAL ); // "Global object not found."

	JSObject *argsObj;
	argsObj = JS_NewArrayObject(cx, argc, NULL); // JL_IsArray(cx, OBJECT_TO_JSVAL(argsObj));
	JL_ASSERT_ALLOC( argsObj ); // JL_CHKM( argsObj != NULL, E_HOST, E_INTERNAL ); // "Unable to create script arguments."
	JL_CHKM( JS_DefinePropertyById(cx, globalObject, JLID(cx, arguments), OBJECT_TO_JSVAL(argsObj), NULL, NULL, /*JSPROP_READONLY | JSPROP_PERMANENT*/ 0), E_HOST, E_INTERNAL ); // "Unable to store script arguments."

	for ( int index = 0; index < argc; index++ ) {

		JSString *str = JS_NewStringCopyZ(cx, argv[index]);
		JL_ASSERT( str != NULL, E_HOST, E_INTERNAL ); // "Unable to store the argument."
		//JL_CHKM( JS_DefineElement(cx, argsObj, index, STRING_TO_JSVAL(str), NULL, NULL, JSPROP_ENUMERATE), E_HOST, E_INTERNAL ); // "Unable to define the argument."
		jsval tmp;
		tmp = STRING_TO_JSVAL(str);
		JL_CHKM( JL_SetElement(cx, argsObj, index, &tmp), E_HOST, E_INTERNAL ); // "Unable to define the argument."
	}
	return JS_TRUE;
	JL_BAD;
}


JSBool ExecuteScriptText( JSContext *cx, const char *scriptText, bool compileOnly, int argc, const char * const * argv, jsval *rval ) {

	uint32_t prevOpt = JS_SetOptions(cx, JS_GetOptions(cx) | JSOPTION_COMPILE_N_GO); //  | JSOPTION_DONT_REPORT_UNCAUGHT
	// JSOPTION_COMPILE_N_GO:
	//  caller of JS_Compile*Script promises to execute compiled script once only; enables compile-time scope chain resolution of consts.
	// JSOPTION_DONT_REPORT_UNCAUGHT:
	//  When returning from the outermost API call, prevent uncaught exceptions from being converted to error reports
	//  we can use JS_ReportPendingException to report it manually

	JSObject *globalObject = JL_GetGlobal(cx);
	JL_ASSERT( globalObject != NULL, E_HOST, E_INTERNAL ); // "Global object not found."

	JL_CHK( CreateScriptArguments(cx, argc, argv) );


// compile & executes the script

	//JSPrincipals *principals = (JSPrincipals*)jl_malloc(sizeof(JSPrincipals));
	//JSPrincipals tmp = {0};
	//*principals = tmp;
	//principals->codebase = (char*)jl_malloc(PATH_MAX);
	//strncpy(principals->codebase, scriptFileName, PATH_MAX-1);
	//principals->refcount = 1;
	//principals->destroy = HostPrincipalsDestroy;

	JSScript *script;
	script = JS_CompileScript(cx, globalObject, scriptText, strlen(scriptText), "inline", 1);
	JL_CHK( script );
	
	{
//	JS::Anchor<JSObject*> scriptObjRoot(JS_NewScriptObject(cx, script));

	// mendatory else the exception is converted into an error before JL_IsExceptionPending can be used. Exceptions can be reported with JS_ReportPendingException().
	JS_SetOptions(cx, JS_GetOptions(cx) | JSOPTION_DONT_REPORT_UNCAUGHT);

	// You need to protect a JSScript (via a rooted script object) if and only if a garbage collection can occur between compilation and the start of execution.
	if ( !compileOnly )
		JL_CHK( JS_ExecuteScript(cx, globalObject, script, rval) ); // MUST be executed only once ( JSOPTION_COMPILE_N_GO )
	else
		*rval = JSVAL_VOID;

//	JS_DestroyScript(cx, script); // Warning: This API is subject to bug 438633, which can cause crashes in almost any program that uses JS_DestroyScript.
	}

	JL_CHK( RemoveScriptArguments( cx ) );
	JS_SetOptions(cx, prevOpt);
	return JS_TRUE;

bad:
	JS_SetOptions(cx, prevOpt);
	return JS_FALSE;
}



JSBool ExecuteScriptFileName( JSContext *cx, const char *scriptFileName, bool compileOnly, int argc, const char * const * argv, jsval *rval ) {

	uint32_t prevOpt = JS_SetOptions(cx, JS_GetOptions(cx) | JSOPTION_COMPILE_N_GO);
	JSObject *globalObject = JL_GetGlobal(cx);
	JL_ASSERT( globalObject != NULL, E_HOST, E_INTERNAL ); // "Global object not found."
	JL_CHK( CreateScriptArguments(cx, argc, argv) );

	JSScript *script;
	script = JL_LoadScript(cx, globalObject, scriptFileName, ENC_UNKNOWN, true, false); // use xdr if available, but don't save it.
	JL_CHK( script );

	{
//	JS::Anchor<JSObject*> scriptObjRoot(JS_NewScriptObject(cx, script));

	// mendatory else the exception is converted into an error before JL_IsExceptionPending can be used. Exceptions can be reported with JS_ReportPendingException().
	JS_SetOptions(cx, JS_GetOptions(cx) | JSOPTION_DONT_REPORT_UNCAUGHT);

	// You need to protect a JSScript (via a rooted script object) if and only if a garbage collection can occur between compilation and the start of execution.
	if ( !compileOnly )
		JL_CHK( JS_ExecuteScript(cx, globalObject, script, rval) ); // MUST be executed only once ( JSOPTION_COMPILE_N_GO )
	else
		*rval = JSVAL_VOID;

//	JS_DestroyScript(cx, script); // Warning: This API is subject to bug 438633 (FIXED), which can cause crashes in almost any program that uses JS_DestroyScript.
	}

	JL_CHK( RemoveScriptArguments(cx) );
	JS_SetOptions(cx, prevOpt);
	return JS_TRUE;

bad:
	JS_SetOptions(cx, prevOpt);
	return JS_FALSE;
}


JSBool ExecuteBootstrapScript( JSContext *cx, void *xdrScript, uint32_t xdrScriptLength ) {

	uint32_t prevOpt = JS_SetOptions(cx, JS_GetOptions(cx) & ~JSOPTION_DONT_REPORT_UNCAUGHT); // report uncautch exceptions !
//	JL_CHKM( JS_EvaluateScript(cx, JL_GetGlobal(cx), embeddedBootstrapScript, sizeof(embeddedBootstrapScript)-1, "bootstrap", 1, &tmp), "Invalid bootstrap." ); // for plain text scripts.
//	JSObject *scriptObjRoot;
	JSXDRState *xdr = JS_XDRNewMem(cx, JSXDR_DECODE);
	JL_CHK( xdr );
	JS_XDRMemSetData(xdr, xdrScript, xdrScriptLength);
	JSScript *script;
	script = NULL;
	JL_CHK( JS_XDRScript(xdr, &script) );
	JS_XDRMemSetData(xdr, NULL, 0); // embeddedBootstrapScript is a static buffer, this avoid JS_free to be called on it.
	JS_XDRDestroy(xdr);
//	scriptObjRoot = JS_NewScriptObject(cx, script);
//	JL_CHK( SetConfigurationReadonlyValue(cx, JLID_NAME(cx, bootstrapScript), OBJECT_TO_JSVAL(bootstrapScriptObject)) ); // bootstrap script cannot be hidden
	jsval tmp;
	JL_CHK( JS_ExecuteScript(cx, JL_GetGlobal(cx), script, &tmp) );
//	JS_DestroyScript(cx, script);
	JS_SetOptions(cx, prevOpt);
	return JS_TRUE;
bad:
	JS_SetOptions(cx, prevOpt);
	return JS_FALSE;
}



//////////////////////////////////////////////////////////////////////////////
// Threaded memory deallocator

// the "load" increase by one each time the thread loop without freeing the whole memory chunk list. When MAX_LOAD is reached, memory is freed synchronously.
#define MAX_LOAD 7

#define WAIT_HEAD_FILLING 50

// memory chunks bigger than BIG_ALLOC are freed synchronously.
#define BIG_ALLOC 8192


static jl_malloc_t base_malloc;
static jl_calloc_t base_calloc;
static jl_memalign_t base_memalign;
static jl_realloc_t base_realloc;
static jl_msize_t base_msize;
static jl_free_t base_free;


// block-to-free chain
static void *head;

// thread stats
static volatile int32_t headLength;
static volatile int load;

// thread handler
static JLThreadHandler memoryFreeThread;

// thread actions
enum MemThreadAction {
	MemThreadExit,
	MemThreadProcess
};

static volatile MemThreadAction threadAction;
static JLSemaphoreHandler memoryFreeThreadSem;


// alloc functions
RESTRICT_DECL void*
JslibsMalloc( size_t size ) {

	if (likely( size >= sizeof(void*) ))
		return base_malloc(size);
	return base_malloc(sizeof(void*));
}

RESTRICT_DECL void* 
JslibsCalloc( size_t num, size_t size ) {

	size *= num;
	if (likely( size >= sizeof(void*) ))
		return base_calloc(size, 1);
	return base_calloc(sizeof(void*), 1);
}

RESTRICT_DECL void* 
JslibsMemalign( size_t alignment, size_t size ) {

	if (likely( size >= sizeof(void*) ))
		return base_memalign(alignment, size);
	return base_memalign(alignment, sizeof(void*));
}

void*
JslibsRealloc( void *ptr, size_t size ) {

	if (likely( size >= sizeof(void*) ))
		return base_realloc(ptr, size);
	return base_realloc(ptr, sizeof(void*));
}

size_t 
JslibsMsize( void *ptr ) {

	return base_msize(ptr);
}

void 
JslibsFree( void *ptr ) {
	
	if (unlikely( ptr == NULL )) // issue wuth nedmalloc free(NULL)
		return;

	ASSERT( ptr > (void*)0x1000 );

	if (unlikely( load >= MAX_LOAD || base_msize(ptr) >= BIG_ALLOC )) { // if blocks is big OR too many things to free, the thread can not keep pace.

		base_free(ptr);
		return;
	}

	*(void**)ptr = head;
	head = ptr;
	JLAtomicIncrement(&headLength);
}


ALWAYS_INLINE void FASTCALL
FreeHead() {

	headLength = 0;

	void *it, *next = head;
	it = *(void**)next;
	*(void**)next = NULL;

	while ( it ) {

		ASSERT( it != *(void**)it );
		next = *(void**)it;
		base_free(it);
		it = next;
	}
}

// the thread proc
JLThreadFuncDecl
MemoryFreeThreadProc( void *threadArg ) {

	JL_IGNORE(threadArg);

	for (;;) {

		if ( JLSemaphoreAcquire(memoryFreeThreadSem, WAIT_HEAD_FILLING) == JLOK ) {
			switch ( threadAction ) {
				case MemThreadExit:
					JLThreadExit(0);
				case MemThreadProcess:
					;
			}
		}

		for ( load = 1; headLength; load++ )
			FreeHead();
	}
	JLThreadExit(0);
	return 0;
}

// GC callback that triggers the thread
static JSGCCallback prevGCCallback;

JSBool NewGCCallback(JSContext *cx, JSGCStatus status) {

	if ( status == JSGC_FINALIZE_END ) {

		threadAction = MemThreadProcess;
		JLSemaphoreRelease(memoryFreeThreadSem);
	}
	return prevGCCallback ? prevGCCallback(cx, status) : JS_TRUE;
}

// (TBD) manage nested GCCallbacks
JSBool MemoryManagerEnableGCEvent( JSContext *cx ) {

	prevGCCallback = JS_SetGCCallback(cx, NewGCCallback);
	return JS_TRUE;
}

JSBool MemoryManagerDisableGCEvent( JSContext *cx ) {

	return JS_SetGCCallback(cx, prevGCCallback) == NewGCCallback;
}


// initialisation and cleanup functions
bool InitializeMemoryManager( jl_malloc_t *malloc, jl_calloc_t *calloc, jl_memalign_t *memalign, jl_realloc_t *realloc, jl_msize_t *msize, jl_free_t *free ) {
	
	base_malloc = *malloc;
	base_calloc = *calloc;
	base_memalign = *memalign;
	base_realloc = *realloc;
	base_msize = *msize;
	base_free = *free;

	*malloc = JslibsMalloc;
	*calloc = JslibsCalloc;
	*memalign = JslibsMemalign;
	*realloc = JslibsRealloc;
	*msize = JslibsMsize;
	*free = JslibsFree;

	load = 0;
	headLength = 0;
	head = NULL;
	JslibsFree(JslibsMalloc(0)); // make head non-NULL
	memoryFreeThreadSem = JLSemaphoreCreate(0);
	memoryFreeThread = JLThreadStart(MemoryFreeThreadProc, NULL);
//	JLThreadPriority(memoryFreeThread, JL_THREAD_PRIORITY_LOW);
	return true;
}


bool FinalizeMemoryManager( bool freeQueue, jl_malloc_t *malloc, jl_calloc_t *calloc, jl_memalign_t *memalign, jl_realloc_t *realloc, jl_msize_t *msize, jl_free_t *free ) {

	*malloc = base_malloc;
	*calloc = base_calloc;
	*memalign = base_memalign;
	*realloc = base_realloc;
	*msize = base_msize;
	*free = base_free;

	threadAction = MemThreadExit;
	JLSemaphoreRelease(memoryFreeThreadSem);
	// beware: Never use JLThreadCancel on a thread that call free().
	JLThreadWait(memoryFreeThread, NULL);
	JLThreadFree(&memoryFreeThread);
	JLSemaphoreFree(&memoryFreeThreadSem);

	if ( freeQueue ) {

		FreeHead();
		base_free(head);
	}
	return true;
}

/* memory stat report maker:

//	fprintf(stderr, "%x ", malloc_usable_size(ptr)); // for stat

var stat = [];
for each ( var sizehex in data.split(' ') ) {

    var size = parseInt(sizehex, 16);
    stat[size] = (stat[size]||0) + 1;
}
var stat2 = [];
for ( var size in stat )
    stat2.push( [Number(size), stat[size]] );
stat2.sort(function(a,b) b[1]-a[1]);
for each ( var i in stat2 )
     print( i[1] + ' x ' + i[0] )
*/
