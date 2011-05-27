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

#include <jlapiexport.h>
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
	char *msg;
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

			memcpy(buf, " ", 1);
			buf += 1;
		}

		strEnd = str + strlen(str);
		pos = str;

		for (;;) {
			
			const char *newPos = strchr(pos, '%');
			if ( !newPos ) {

				memcpy(buf, pos, strEnd-pos);
				buf += strEnd-pos;
				break;
			} else {

				memcpy(buf, pos, newPos-pos);
				buf += newPos-pos;
			}
			pos = newPos;

			switch ( *++pos ) {
				case 'd':
					++pos;
					ltoa(va_arg(vl, long), buf, 10);
					buf += strlen(buf);
					break;
				case 'x':
					++pos;
					memcpy(buf, "0x", 2);
					buf += 2;
					ltoa(va_arg(vl, long), buf, 16);
					buf += strlen(buf);
					break;
				case 's': {
					++pos;
					const char * tmp = va_arg(vl, char *);
					int len = strlen(tmp);
					if ( len > 128 ) {
						
						memcpy(buf, tmp, 128);
						buf += 128;
						memcpy(buf, "...", 3);
						buf += 3;
					} else {

						memcpy(buf, tmp, len);
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
	memcpy(buf, ".", 1);
	buf += 1;
	*buf = '\0';

	va_end(vl);

	JSErrorFormatString format = { message, 0, (int16)exn };
	return JS_ReportErrorFlagsAndNumber(cx, isWarning ? JSREPORT_WARNING : JSREPORT_ERROR, ErrorCallback, (void*)&format, 0);

bad:
	va_end(vl);
	return JS_FALSE;
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


// route: Print() => _host->stdout() => JSDefaultStdoutFunction() => pv->hostStdOut()
JSBool JSDefaultStdoutFunction(JSContext *cx, uintN argc, jsval *vp) {

	*JL_RVAL = JSVAL_VOID;
	HostPrivate *pv = JL_GetHostPrivate(cx);
	if (unlikely( pv == NULL || pv->hostStdOut == NULL ))
		return JS_TRUE;

	for ( uintN i = 0; i < argc; ++i ) {

		JLStr str;
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

		JLStr str;
		JL_CHK( JL_JsvalToNative(cx, JL_ARGV[i], &str) );
		int status = pv->hostStdErr(pv->privateData, str.GetConstStr(), str.Length());
		JL_ASSERT_WARN( status != -1, E_HOST, E_INTERNAL, E_SEP, E_COMMENT("stderr"), E_WRITE );
	}
	return JS_TRUE;
	JL_BAD;
}


void ErrorReporterBasic(JSContext *cx, const char *message, JSErrorReport *report) {

	JL_USE(cx);
	if ( !report )
		fprintf(stderr, "%s\n", message);
	else
		fprintf(stderr, "%s (%s:%d)\n", message, report->filename, report->lineno);
}


void StderrWrite(JSContext *cx, const char *message, size_t length) {

	JSObject *globalObject = JL_GetGlobalObject(cx);
	ASSERT( globalObject );

	jsval fct;
	if (unlikely( GetHostObjectValue(cx, JLID(cx, stderr), &fct) != JS_TRUE || !JL_ValueIsFunction(cx, fct) ))
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


void ErrorReporter(JSContext *cx, const char *message, JSErrorReport *report) {

	// trap JSMSG_OUT_OF_MEMORY error to avoid calling ErrorReporter_stdErrRouter() that may allocate memory that will lead to nested call.
	if (unlikely( report && report->errorNumber == JSMSG_OUT_OF_MEMORY )) { // (TBD) do something better
		
		fprintf(stderr, "%s (%s:%d)\n", message, report->filename, report->lineno);
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
		memcpy(buf, STR, len); \
		buf += len; \
	JL_MACRO_END

	#define fwrite(STR, SIZE, COUNT, FILE) \
	JL_MACRO_BEGIN \
		size_t remaining = sizeof(buffer)-(buf-buffer); \
		if ( remaining == 0 ) break; \
		size_t len = JL_MIN(size_t((SIZE)*(COUNT)), remaining); \
		memcpy(buf, (STR), len); \
		buf += len; \
	JL_MACRO_END

	#define fputc(CHR, FILE) \
	JL_MACRO_BEGIN \
		size_t remaining = sizeof(buffer)-(buf-buffer); \
		if ( remaining == 0 ) break; \
		buf[0] = (CHR); \
		buf += 1; \
	JL_MACRO_END
	

// copy-paste from /js/src/js.cpp (my_ErrorReporter)
//	 ---8<---

    int i, j, k, n;
    char *prefix, *tmp;
    const char *ctmp;

    if (!report) {
        fprintf(gErrFile, "%s\n", message);
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
	//JSPackedBool *gcRunning = &cx->runtime->gcRunning;
	for (;;) {

		if ( JLSemaphoreAcquire(pv->watchDogSemEnd, pv->maybeGCInterval) != JLTIMEOUT ) // used as a breakable Sleep instead of SleepMilliseconds (see SandboxEval).
			break;
		JS_TriggerOperationCallback(cx);
	}
	JLThreadExit(0);
	return 0;
}


JSBool LoadModule(JSContext *cx, uintN argc, jsval *vp) {

	JLStr str;
	JLLibraryHandler module = JLDynamicLibraryNullHandler;

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARGC_MIN(1);


	if (unlikely( JL_ARGC < 1 )) {

		*JL_RVAL = JSVAL_VOID;
		return JS_TRUE;
	}

	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &str) );

	char libFileName[PATH_MAX];
	strncpy( libFileName, str.GetConstStr(), str.Length() ); // (TBD) check
	libFileName[str.Length()] = '\0';
	strcat( libFileName, DLL_EXT );
// MAC OSX: 	'@executable_path' ??

/*
	while (0) { // namespace management. Avoid using val ns = {}, LoadModule.call(ns, '...');

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
	JL_CHK( JL_NewNumberValue(cx, uid, JL_RVAL) ); // really needed ? yes, UnloadModule will need this ID
	return JS_TRUE;

bad:
	if ( JLDynamicLibraryOk(module) )
		JLDynamicLibraryClose(&module);
	return JS_FALSE;
}


JSBool global_enumerate(JSContext *cx, JSObject *obj) { // see LAZY_STANDARD_CLASSES

	return JS_EnumerateStandardClasses(cx, obj);
}

JSBool global_resolve(JSContext *cx, JSObject *obj, jsid id, uintN flags, JSObject **objp) { // see LAZY_STANDARD_CLASSES

	if ( (flags & JSRESOLVE_ASSIGNING) == 0 ) {

		JSBool resolved;
		if ( !JS_ResolveStandardClass(cx, obj, id, &resolved) )
			return JS_FALSE;

		//if ( !resolved && JSID_IS_ATOM(id, CLASS_ATOM(cx, Reflect)) ) {

		//	if ( !js_InitReflectClass(cx, obj) )
		//		return JS_FALSE;
		//	resolved = JS_TRUE;
		//}

		if ( resolved ) {
			
			*objp = obj;
			return JS_TRUE;
		}
	}
	return JS_TRUE;
}

// global object
// doc: For full ECMAScript standard compliance, obj should be of a JSClass that has the JSCLASS_GLOBAL_FLAGS flag.
// note: global_class is a global variable, but this is not an issue even is several runtimes share the same JSClass.
static JSClass global_class = {
	NAME_GLOBAL_CLASS, JSCLASS_GLOBAL_FLAGS | JSCLASS_NEW_RESOLVE,
	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
	global_enumerate, (JSResolveOp)global_resolve, JS_ConvertStub, JS_FinalizeStub, // global_resolve: see LAZY_STANDARD_CLASSES
	JSCLASS_NO_OPTIONAL_MEMBERS
};


// default: CreateHost(-1, -1, 0);
JSContext* CreateHost(uint32 maxMem, uint32 maxAlloc, uint32 maybeGCInterval ) {

	JSRuntime *rt = JS_NewRuntime(0);
	JL_CHK( rt );

	// doc: maxMem specifies the number of allocated bytes after which garbage collection is run.
	JS_SetGCParameter(rt, JSGC_MAX_BYTES, maxMem); // maximum nominal heap before last ditch GC
	JS_SetGCParameter(rt, JSGC_MAX_MALLOC_BYTES, maxAlloc); // # of JS_malloc bytes before last ditch GC
//	JS_SetGCParameter(rt, JSGC_TRIGGER_FACTOR, 3);

	uint32 stackpoolLifespan = JS_GetGCParameter(rt, JSGC_STACKPOOL_LIFESPAN);
	ASSERT( stackpoolLifespan == 30000 ); // check if default has chaged.
	JS_SetGCParameter(rt, JSGC_STACKPOOL_LIFESPAN, 15000);

	JS_SetGCParameter(rt, JSGC_MODE, JSGC_MODE_GLOBAL);

	JSContext *cx;
	cx = JS_NewContext(rt, 8192); // set the chunk size of the stack pool to 8192. see http://groups.google.com/group/mozilla.dev.tech.js-engine/browse_thread/thread/be9f404b623acf39/9efdfca81be99ca3
	JL_CHK( cx ); //, "unable to create the context." );

	// Info: Increasing JSContext stack size slows down my scripts:
	//   http://groups.google.com/group/mozilla.dev.tech.js-engine/browse_thread/thread/be9f404b623acf39/9efdfca81be99ca3

	JS_SetScriptStackQuota(cx, JS_DEFAULT_SCRIPT_STACK_QUOTA); // good place to manage stack limit ( that is 32MB by default ). Btw, JS_SetScriptStackQuota ( see also JS_SetThreadStackLimit )

	uint32 maxCodeCacheBytes = JS_GetGCParameterForThread(cx, JSGC_MAX_CODE_CACHE_BYTES);
	ASSERT( maxCodeCacheBytes == 16 * 1024 * 1024 ); // check if default has chaged.
	JS_SetGCParameterForThread(cx, JSGC_MAX_CODE_CACHE_BYTES, 16 * 1024 * 1024 * 2);

	//JS_SetNativeStackQuota(cx, DEFAULT_MAX_STACK_SIZE); // see https://developer.mozilla.org/En/SpiderMonkey/JSAPI_User_Guide

	JS_SetVersion(cx, (JSVersion)JSVERSION_LATEST);

	JS_SetErrorReporter(cx, ErrorReporter);

	// JSOPTION_ANONFUNFIX: https://bugzilla.mozilla.org/show_bug.cgi?id=376052 
	// JS_SetOptions doc: https://developer.mozilla.org/en/SpiderMonkey/JSAPI_Reference/JS_SetOptions
	ASSERT( JS_GetOptions(cx) == 0 );
	JS_SetOptions(cx, JSOPTION_VAROBJFIX | JSOPTION_ANONFUNFIX | JSOPTION_XML | JSOPTION_RELIMIT | JSOPTION_JIT | JSOPTION_METHODJIT | /*JSOPTION_METHODJIT_ALWAYS |*/ JSOPTION_PROFILING);

	JSObject *globalObject;
	globalObject = JS_NewCompartmentAndGlobalObject(cx, &global_class, NULL);
	JL_CHK( globalObject ); // "unable to create the global object." );

	//	Doc: As a side effect, JS_InitStandardClasses establishes obj as the global object for cx, if one is not already established.
	JS_SetGlobalObject(cx, globalObject); // no call to JS_InitStandardClasses(), then JS_SetGlobalObject() is required (see also LAZY_STANDARD_CLASSES).

	HostPrivate *pv;
	pv = (HostPrivate*)jl_calloc(sizeof(HostPrivate), 1); // beware: don't realloc, because WatchDogThreadProc points on it !!!
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


JSBool InitHost( JSContext *cx, bool unsafeMode, HostInput stdIn, HostOutput stdOut, HostOutput stdErr, void* userPrivateData ) { // init the host for jslibs usage (modules, errors, ...)

	ASSERT( !JS_CStringsAreUTF8() );

	_unsafeMode = unsafeMode;
	HostPrivate *pv = JL_GetHostPrivate(cx);
	if ( pv == NULL ) { // in the case of CreateHost has not been called (because the caller wants to create and manage its own JS runtime)

		pv = (HostPrivate*)jl_calloc(sizeof(HostPrivate), 1); // beware: don't realloc, because WatchDogThreadProc points on it !!!
		JL_ASSERT_ALLOC( pv );
		pv->hostPrivateVersion = JL_HOST_PRIVATE_VERSION;
		JL_SetHostPrivate(cx, pv);
	}

	pv->privateData = userPrivateData;

	pv->jlapi = &jlapi;

	jl::QueueInitialize(&pv->moduleList);

	pv->unsafeMode = unsafeMode;

	if ( unsafeMode )
		JS_SetOptions(cx, JS_GetOptions(cx) & ~(JSOPTION_STRICT | JSOPTION_RELIMIT));

	JSObject *globalObject;
	globalObject = JL_GetGlobalObject(cx);
	ASSERT( globalObject != NULL ); // "Global object not found."
	
	pv->report = Report;

	pv->hostStdErr = stdErr;
	pv->hostStdOut = stdOut;
	pv->hostStdIn = stdIn;

	// global functions & properties
	JL_CHKM( JS_DefinePropertyById( cx, globalObject, JLID(cx, global), OBJECT_TO_JSVAL(JL_GetGlobalObject(cx)), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT ), E_PROP, E_CREATE ); // "unable to define a property."
	JL_CHKM( JS_DefineFunction( cx, globalObject, JL_GetHostPrivate(cx)->camelCase == 1 ? JLNormalizeFunctionName(NAME_GLOBAL_FUNCTION_LOAD_MODULE) : NAME_GLOBAL_FUNCTION_LOAD_MODULE, LoadModule, 0, 0 ), E_PROP, E_CREATE ); // "unable to define a property."

	JL_CHK( SetHostObjectValue(cx, JLID(cx, unsafeMode), unsafeMode ? JSVAL_TRUE : JSVAL_FALSE, false) );

	// note: we have to support: var prevStderr = _host.stderr; _host.stderr = function(txt) { file.Write(txt); prevStderr(txt) };
	jsval value;
	value = OBJECT_TO_JSVAL(JS_GetFunctionObject(JS_NewFunctionById(cx, JSDefaultStdinFunction, 1, 0, NULL, JLID(cx, stdin)))); // doc: If you do not assign a name to the function, it is assigned the name "anonymous".
	JL_CHK( SetHostObjectValue(cx, JLID(cx, stdin), value) );
	value = OBJECT_TO_JSVAL(JS_GetFunctionObject(JS_NewFunctionById(cx, JSDefaultStdoutFunction, 1, 0, NULL, JLID(cx, stdout))));
	JL_CHK( SetHostObjectValue(cx, JLID(cx, stdout), value) );
	value = OBJECT_TO_JSVAL(JS_GetFunctionObject(JS_NewFunctionById(cx, JSDefaultStderrFunction, 1, 0, NULL, JLID(cx, stderr))));
	JL_CHK( SetHostObjectValue(cx, JLID(cx, stderr), value) );

	JL_CHK( JL_NativeToJsval(cx, JL_SvnRevToInt(SVN_REVISION_STR), &value) );
	JL_CHK( SetHostObjectValue(cx, L("revision"), value) );
	JL_CHK( JL_NativeToJsval(cx, JL_BUILD, &value) ); // __DATE__YEAR, __DATE__MONTH+1, __DATE__DAY
	JL_CHK( SetHostObjectValue(cx, L("build"), value) );
	JL_CHK( JL_NativeToJsval(cx, (int)JS_GetVersion(cx), &value) ); // JS_GetImplementationVersion()
	JL_CHK( SetHostObjectValue(cx, L("jsVersion"), value) );
	

	// init static modules
	if ( !jslangModuleInit(cx, globalObject) )
		JL_ERR( E_MODULE, E_NAME("jslang"), E_INIT );

//	JL_CHK( JS_DefinePropertyById(cx, globalObject, JLID(cx, _revision), INT_TO_JSVAL(JL_SvnRevToInt(SVN_REVISION_STR)), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT) ); // see _host.revision

	return JS_TRUE;
	JL_BAD;
}


JSBool DestroyHost( JSContext *cx, bool skipCleanup ) {

	JSRuntime *rt = JL_GetRuntime(cx);

	HostPrivate *pv = JL_GetHostPrivate(cx);
	JL_ASSERT( pv, E_HOST, E_STATE, E_COMMENT("context private") );

	pv->canSkipCleanup = skipCleanup;

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

//	JS_SetGlobalObject(cx, JSVAL_TO_OBJECT(JSVAL_NULL)); // remove the global object (TBD) check if it is good or needed to do this.

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

	jl_free(pv);
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

	JSObject *globalObject = JL_GetGlobalObject(cx);
	JL_ASSERT( globalObject != NULL, E_HOST, E_INTERNAL ); // "Global object not found."
	JL_CHKM( JS_DeletePropertyById(cx, globalObject, JLID(cx, arguments)), E_HOST, E_INTERNAL ); // "Unable to cleanup script arguments."
	return JS_TRUE;
	JL_BAD;
}

JSBool CreateScriptArguments( JSContext *cx, int argc, const char * const * argv ) {

	JSObject *globalObject = JL_GetGlobalObject(cx);
	JL_ASSERT( globalObject != NULL, E_HOST, E_INTERNAL ); // "Global object not found."

	JSObject *argsObj;
	argsObj = JS_NewArrayObject(cx, argc, NULL); // JL_IsArray(cx, OBJECT_TO_JSVAL(argsObj));
	JL_ASSERT_ALLOC( argsObj ); // JL_CHKM( argsObj != NULL, E_HOST, E_INTERNAL ); // "Unable to create script arguments."
	JL_CHKM( JS_DefinePropertyById(cx, globalObject, JLID(cx, arguments), OBJECT_TO_JSVAL(argsObj), NULL, NULL, /*JSPROP_READONLY | JSPROP_PERMANENT*/ 0), E_HOST, E_INTERNAL ); // "Unable to store script arguments."

	for ( int index = 0; index < argc; index++ ) {

		JSString *str = JS_NewStringCopyZ(cx, argv[index]);
		JL_ASSERT( str != NULL, E_HOST, E_INTERNAL ); // "Unable to store the argument."
		JL_CHKM( JS_DefineElement(cx, argsObj, index, STRING_TO_JSVAL(str), NULL, NULL, JSPROP_ENUMERATE), E_HOST, E_INTERNAL ); // "Unable to define the argument."
	}
	return JS_TRUE;
	JL_BAD;
}


JSBool ExecuteScriptText( JSContext *cx, const char *scriptText, bool compileOnly, int argc, const char * const * argv, jsval *rval ) {

	uint32 prevOpt = JS_SetOptions(cx, JS_GetOptions(cx) | JSOPTION_COMPILE_N_GO); //  | JSOPTION_DONT_REPORT_UNCAUGHT
	// JSOPTION_COMPILE_N_GO:
	//  caller of JS_Compile*Script promises to execute compiled script once only; enables compile-time scope chain resolution of consts.
	// JSOPTION_DONT_REPORT_UNCAUGHT:
	//  When returning from the outermost API call, prevent uncaught exceptions from being converted to error reports
	//  we can use JS_ReportPendingException to report it manually

	JSObject *globalObject = JL_GetGlobalObject(cx);
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

	JSObject *script;
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



JSBool ExecuteScriptFileName( JSContext *cx, const char *scriptFileName, bool compileOnly, int argc, const char * const * argv, jsval *rval ) { // (TBD) support xdr files

	uint32 prevOpt = JS_SetOptions(cx, JS_GetOptions(cx) | JSOPTION_COMPILE_N_GO);
	JSObject *globalObject = JL_GetGlobalObject(cx);
	JL_ASSERT( globalObject != NULL, E_HOST, E_INTERNAL ); // "Global object not found."
	JL_CHK( CreateScriptArguments(cx, argc, argv) );

	JSObject *script;
	script = JL_LoadScript(cx, globalObject, scriptFileName, true, false); // use xdr if available, but don't save it.
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


JSBool ExecuteBootstrapScript( JSContext *cx, void *xdrScript, uint32 xdrScriptLength ) {

	uint32 prevOpt = JS_SetOptions(cx, JS_GetOptions(cx) & ~JSOPTION_DONT_REPORT_UNCAUGHT); // report uncautch exceptions !
//	JL_CHKM( JS_EvaluateScript(cx, JL_GetGlobalObject(cx), embeddedBootstrapScript, sizeof(embeddedBootstrapScript)-1, "bootstrap", 1, &tmp), "Invalid bootstrap." ); // for plain text scripts.
//	JSObject *scriptObjRoot;
	JSXDRState *xdr = JS_XDRNewMem(cx, JSXDR_DECODE);
	JL_CHK( xdr );
	JS_XDRMemSetData(xdr, xdrScript, xdrScriptLength);
	JSObject *script;
	script = NULL;
	JL_CHK( JS_XDRScriptObject(xdr, &script) );
	JS_XDRMemSetData(xdr, NULL, 0); // embeddedBootstrapScript is a static buffer, this avoid JS_free to be called on it.
	JS_XDRDestroy(xdr);
//	scriptObjRoot = JS_NewScriptObject(cx, script);
//	JL_CHK( SetConfigurationReadonlyValue(cx, JLID_NAME(cx, bootstrapScript), OBJECT_TO_JSVAL(bootstrapScriptObject)) ); // bootstrap script cannot be hidden
	jsval tmp;
	JL_CHK( JS_ExecuteScript(cx, JL_GetGlobalObject(cx), script, &tmp) );
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
#define BIG_ALLOC 4096


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

	if (unlikely( base_msize(ptr) >= BIG_ALLOC || load >= MAX_LOAD )) { // if blocks is big OR too many things to free, the thread can not keep pace.

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

	JL_USE(threadArg);

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
     Print( i[1] + ' x ' + i[0] )
*/


/* memory pool, to be fixed: memory grows endless

void *memoryPool[14] = {NULL};
JLMutexHandler poolMutex[14];

ALWAYS_INLINE int PoolSelect( size_t size ) {

	if ( size == 44 ) return 0;
	if ( size == 16 ) return 1;
	if ( size == 48 ) return 2;
	if ( size == 12 ) return 3;

	//if ( size <=   18 ) return 5;
	//if ( size <=   38 ) return 6;
	//if ( size <=   68 ) return 7;
	//if ( size <=  128 ) return 8;
	//if ( size <=  512 ) return 9;
	//if ( size <= 1078 ) return 10;
	//if ( size <= 2096 ) return 11;
	//if ( size <= 4100 ) return 12;
	//if ( size <= 8200 ) return 13;

	return -1;
}


ALWAYS_INLINE void MemoryPoolFree( void *ptr ) {

	size_t size = malloc_usable_size(ptr);
	int pool = PoolSelect(size);
	if ( pool == -1 ) {

		free(ptr);
		return;
	}

	JLAcquireMutex(poolMutex[pool]);
	*(void**)ptr = memoryPool[pool];
	memoryPool[pool] = ptr;
	JLReleaseMutex(poolMutex[pool]);
}

ALWAYS_INLINE void* MemoryPoolMalloc( size_t size ) {

	int pool = PoolSelect(size);
	if ( pool == -1 || memoryPool[pool] == NULL )
		return malloc(size);

	JLAcquireMutex(poolMutex[pool]);
	void *ptr = memoryPool[pool];
	memoryPool[pool] = *(void**)memoryPool[pool];
	JLReleaseMutex(poolMutex[pool]);
	return ptr;
}

void MemoryPoolInit() {

	for ( int i = 0; i < COUNTOF(poolMutex); i++ )
		poolMutex[i] = JLCreateMutex();
}

void MemoryPoolFinalize() {

	for ( int i = 0; i < COUNTOF(poolMutex); i++ ) {

		JLFreeMutex(&poolMutex[i]);
		while ( memoryPool[i] ) {

			void *next = *(void**)memoryPool[i];
			free(memoryPool[i]);
			memoryPool[i] = next;
		}
	}
}
*/
