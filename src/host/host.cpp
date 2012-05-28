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

#include "host.h"
#include "../jslang/jslang.h"

/*
JSErrorFormatString jlErrorFormatString[] = {
#define JLMSG_DEF(name, exception, format, count) { format, count, exception },
#include "jlerrors.msg"
#undef JLMSG_DEF
};


const JSErrorFormatString *GetErrorMessage(void *, const char *, const unsigned errorNumber) {

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
ErrorCallback(void *userRef, const char *, const unsigned) {

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

			jl::memcpy(buf, " ", 1);
			buf += 1;
		}

		strEnd = str + strlen(str);
		pos = str;

		for (;;) {
			
			const char *newPos = strchr(pos, '%');
			if ( !newPos ) {

				jl::memcpy(buf, pos, strEnd-pos);
				buf += strEnd-pos;
				break;
			} else {

				jl::memcpy(buf, pos, newPos-pos);
				buf += newPos-pos;
			}
			pos = newPos;

			switch ( *++pos ) {
				case 'd':
					++pos;
					jl::itoa(va_arg(vl, long), buf, 10);
					buf += strlen(buf);
					break;
				case 'x':
					++pos;
					jl::memcpy(buf, "0x", 2);
					buf += 2;
					jl::itoa(va_arg(vl, long), buf, 16);
					buf += strlen(buf);
					break;
				case 's': {
					++pos;
					const char * tmp = va_arg(vl, char *);
					int len = strlen(tmp);
					if ( len > 128 ) {
						
						jl::memcpy(buf, tmp, 128);
						buf += 128;
						jl::memcpy(buf, "...", 3);
						buf += 3;
					} else {

						jl::memcpy(buf, tmp, len);
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
	jl::memcpy(buf, ".", 1);
	buf += 1;
	*buf = '\0';

	va_end(vl);

	JSErrorFormatString format = { message, 0, (int16_t)exn };
	return JS_ReportErrorFlagsAndNumber(cx, isWarning ? JSREPORT_WARNING : JSREPORT_ERROR, ErrorCallback, (void*)&format, 0);

//bad:
//	va_end(vl);
//	return JS_FALSE;
}


void ErrorReporterBasic(JSContext *cx, const char *message, JSErrorReport *report) {

	JL_IGNORE( cx );
	if ( !report )
		fprintf(stderr, "%s\n", message);
	else
		fprintf(stderr, "%s (%s:%d)\n", message, report->filename, report->lineno);
}


void StderrWrite(JSContext *cx, const char *message, size_t length) {

	JSObject *globalObject = JL_GetGlobal(cx);
	ASSERT( globalObject );

	jsval fct;
	if (unlikely( JS_GetPropertyById(cx, JL_GetHostPrivate(cx)->hostObject, JLID(cx, stderr), &fct) != JS_TRUE || !JL_ValueIsCallable(cx, fct) ))
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
		jl::memcpy(buf, STR, len); \
		buf += len; \
	JL_MACRO_END

	#define fwrite(STR, SIZE, COUNT, FILE) \
	JL_MACRO_BEGIN \
		size_t remaining = sizeof(buffer)-(buf-buffer); \
		if ( remaining == 0 ) break; \
		size_t len = JL_MIN(size_t((SIZE)*(COUNT)), remaining); \
		jl::memcpy(buf, (STR), len); \
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
	HostPrivate *hpv = JL_GetHostPrivate(cx);
	for (;;) {

		if ( JLSemaphoreAcquire(hpv->watchDogSemEnd, hpv->maybeGCInterval) != JLTIMEOUT ) // used as a breakable Sleep instead of SleepMilliseconds (see SandboxEval).
			break;
		JS_TriggerOperationCallback(JL_GetRuntime(cx));
	}
	JLThreadExit(0);
	return 0;
}


// global object
// doc: For full ECMAScript standard compliance, obj should be of a JSClass that has the JSCLASS_GLOBAL_FLAGS flag.
// note: global_class is a global variable, but this is not an issue even if several runtimes share the same JSClass.
static JSClass global_class = {
	NAME_GLOBAL_CLASS, JSCLASS_GLOBAL_FLAGS,
	JS_PropertyStub, JS_PropertyStub,
	JS_PropertyStub, JS_StrictPropertyStub,
	JS_EnumerateStub, JS_ResolveStub,
	JS_ConvertStub
};



BEGIN_CLASS( host )

/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER( unsafeMode ) {

	JL_IGNORE( id, obj );

	JL_CHK( JL_NativeToJsval(cx, JL_GetHostPrivate(cx)->unsafeMode, vp) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER( jsVersion ) {

	JL_IGNORE( id, obj );

	JL_CHK( JL_NativeToJsval(cx, JS_GetVersion(cx), vp) ); // btw, see JS_GetImplementationVersion()
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER( incrementalGarbageCollector ) {

	JL_IGNORE( id, obj );

	uint32_t gcMode = JS_GetGCParameter(JL_GetRuntime(cx), JSGC_MODE);
	JL_CHK( JL_NativeToJsval(cx, gcMode == JSGC_MODE_INCREMENTAL, vp) ); // JSGC_MODE_GLOBAL
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( incrementalGarbageCollector ) {

	JL_IGNORE( strict, id, obj );

	bool incGc;
	JL_CHK( JL_JsvalToNative(cx, *vp, &incGc) );
	JS_SetGCParameter(JL_GetRuntime(cx), JSGC_MODE, incGc ? JSGC_MODE_INCREMENTAL : JSGC_MODE_GLOBAL);
	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME()
**/
DEFINE_FUNCTION( stdout ) {

	*JL_RVAL = JSVAL_VOID;
	JLData str;
	HostPrivate *hpv = JL_GetHostPrivate(cx);
	if (likely( hpv != NULL && hpv->hostStdOut != NULL )) {

		for ( unsigned i = 0; i < argc; ++i ) {

			JL_CHK( JL_JsvalToNative(cx, JL_ARGV[i], &str) );
			int status = hpv->hostStdOut(hpv->privateData, str.GetConstStr(), str.Length());
			JL_ASSERT_WARN( status != -1, E_HOST, E_INTERNAL, E_SEP, E_COMMENT("stdout"), E_WRITE );
		}
	}
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME()
**/
DEFINE_FUNCTION( stderr ) {

	*JL_RVAL = JSVAL_VOID;
	JLData str;
	HostPrivate *hpv = JL_GetHostPrivate(cx);
	if (likely( hpv != NULL && hpv->hostStdErr != NULL )) {

		for ( unsigned i = 0; i < argc; ++i ) {

			JL_CHK( JL_JsvalToNative(cx, JL_ARGV[i], &str) );
			int status = hpv->hostStdErr(hpv->privateData, str.GetConstStr(), str.Length());
			JL_ASSERT_WARN( status != -1, E_HOST, E_INTERNAL, E_SEP, E_COMMENT("stderr"), E_WRITE );
		}
	}
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $STR $INAME()
**/
DEFINE_FUNCTION( stdin ) {

	JL_IGNORE( argc );

	HostPrivate *hpv = JL_GetHostPrivate(cx);
	if (likely( hpv != NULL && hpv->hostStdIn != NULL )) {

		char buffer[8192];
		int status = hpv->hostStdIn(hpv->privateData, buffer, COUNTOF(buffer));
		if ( status > 0 ) {
		
			JSString *jsstr = JS_NewStringCopyN(cx, buffer, status);
			JL_CHK( jsstr );
			*JL_RVAL = STRING_TO_JSVAL( jsstr );
		} else {

			JL_WARN( E_HOST, E_INTERNAL, E_SEP, E_COMMENT("stdin"), E_READ );
			*JL_RVAL = JL_GetEmptyStringValue(cx);
		}
	} else {

		*JL_RVAL = JSVAL_VOID;
	}
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $OBJ | null $INAME()
**/
DEFINE_FUNCTION( loadModule ) {

	JLLibraryHandler module = JLDynamicLibraryNullHandler;
	JLData str;

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARGC(1);

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

	HostPrivate *hpv;
	hpv = JL_GetHostPrivate(cx);
	JL_ASSERT( hpv, E_HOST, E_STATE, E_COMMENT("context private") );
	JL_ASSERT( libFileName != NULL && *libFileName != '\0', E_ARG, E_NUM(1), E_DEFINED );

	module = JLDynamicLibraryOpen(libFileName);
	if ( !JLDynamicLibraryOk(module) ) {

		JL_SAFE_BEGIN
		char errorBuffer[256];
		JLDynamicLibraryLastErrorMessage( errorBuffer, sizeof(errorBuffer) );
		JL_WARN( E_OS, E_OPERATION, E_DETAILS, E_STR(errorBuffer), E_COMMENT(libFileName) );
		JL_SAFE_END

		*JL_RVAL = JSVAL_FALSE;
		return JS_TRUE;
	}

	for ( jl::QueueCell *it = jl::QueueBegin(&hpv->moduleList); it; it = jl::QueueNext(it) ) {

		if ( (JLLibraryHandler)jl::QueueGetData(it) == module ) {

			JLDynamicLibraryClose(&module);
			*JL_RVAL = JSVAL_NULL; // already loaded
			return JS_TRUE;
		}
	}

	uint32_t uid;
	uid = JLDynamicLibraryId(module); // module unique ID
	ModuleInitFunction moduleInit;
	moduleInit = (ModuleInitFunction)JLDynamicLibrarySymbol(module, NAME_MODULE_INIT);
	JL_ASSERT( moduleInit, E_MODULE, E_NAME(libFileName), E_INIT ); // "Invalid module."
	
//	CHKHEAP();

	if ( !moduleInit(cx, JL_OBJ, uid) ) {

		JL_CHK( !JL_IsExceptionPending(cx) );
		char filename[PATH_MAX];
		JLDynamicLibraryName((void*)moduleInit, filename, sizeof(filename));
		JL_ERR( E_MODULE, E_NAME(filename), E_INIT );
	}

//	CHKHEAP();

	jl::QueueUnshift( &hpv->moduleList, module ); // store the module (LIFO)
	
	//JL_CHK( JL_NewNumberValue(cx, uid, JL_RVAL) ); // really needed ? yes, UnloadModule will need this ID, ... but UnloadModule is too complicated to implement and will never exist.
	*JL_RVAL = OBJECT_TO_JSVAL(JL_OBJ);

	return JS_TRUE;

bad:
	if ( JLDynamicLibraryOk(module) )
		JLDynamicLibraryClose(&module);
	return JS_FALSE;
}


DEFINE_INIT() {

	JL_IGNORE( proto, sc );

	JL_GetHostPrivate(cx)->hostObject = obj;
	return JS_TRUE;
}


/**qa
	QA.ASSERTOP(global, 'has', 'host');
	QA.ASSERTOP(host, 'has', 'stdout');
	QA.ASSERTOP(host, 'has', 'unsafeMode');
**/
CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision$"))

	HAS_INIT

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION_ARGC(loadModule, 1)

		// note: we have to support: var prevStderr = host.stderr; host.stderr = function(txt) { file.Write(txt); prevStderr(txt) };
		// route: print() => host->stdout() => JSDefaultStdoutFunction() => hpv->hostStdOut()
		FUNCTION_ARGC(stdin, 0)
		FUNCTION_ARGC(stdout, 1)
		FUNCTION_ARGC(stderr, 1)
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_GETTER( unsafeMode )
		PROPERTY_GETTER( jsVersion )
		PROPERTY( incrementalGarbageCollector )
	END_STATIC_PROPERTY_SPEC

END_CLASS



// default: CreateHost(-1, -1, 0);
JSContext* CreateHost( uint32_t maxMem, uint32_t maxAlloc, uint32_t maybeGCInterval ) {

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

	JS_SetGCParameter(rt, JSGC_MODE, JSGC_MODE_INCREMENTAL); // JSGC_MODE_GLOBAL

	JS_SetErrorReporter(cx, ErrorReporter);

	// JSOPTION_ANONFUNFIX: https://bugzilla.mozilla.org/show_bug.cgi?id=376052 
	// JS_SetOptions doc: https://developer.mozilla.org/en/SpiderMonkey/JSAPI_Reference/JS_SetOptions
	ASSERT( JS_GetOptions(cx) == 0 );
	JS_SetOptions(cx, JSOPTION_ATLINE | JSOPTION_VAROBJFIX | JSOPTION_XML | JSOPTION_RELIMIT | JSOPTION_METHODJIT | JSOPTION_TYPE_INFERENCE); // beware: avoid using JSOPTION_COMPILE_N_GO here.

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

	HostPrivate *hpv;
	
	hpv = ConstructHostPrivate();
	JL_ASSERT_ALLOC( hpv );
	hpv->versionKey = JL_HOSTPRIVATE_KEY;
	JL_SetHostPrivate(cx, hpv);

	// setup WatchDog
	if ( maybeGCInterval ) {

		hpv->maybeGCInterval = maybeGCInterval;
		JSOperationCallback prevOperationCallback;
		prevOperationCallback = JS_SetOperationCallback(cx, OperationCallback);
		ASSERT( prevOperationCallback == NULL );
		hpv->watchDogSemEnd = JLSemaphoreCreate(0);
		hpv->watchDogThread = JLThreadStart(WatchDogThreadProc, cx);
		//	JLThreadPriority(hpv->watchDogThread, JL_THREAD_PRIORITY_LOW);
		JL_ASSERT( JLSemaphoreOk(hpv->watchDogSemEnd) && JLThreadOk(hpv->watchDogThread), E_HOST, E_CREATE ); // "Unable to create the GC thread."
	}
	return cx;

bad:
	return NULL;
}

/**qa
	QA.ASSERTOP( global, 'has', 'processEvents' );
//	QA.ASSERTOP( host, '!has', 'processEvents' );
	QA.ASSERTOP( host._buildDate, '>', 0 );
	QA.ASSERTOP( host._buildDate, '>', +new Date(2012, 3 -1, 1) );
	QA.ASSERTOP( host._buildDate, '<=', Date.now() - new Date().getTimezoneOffset() * 60 * 1000 );
**/
JSBool InitHost( JSContext *cx, bool unsafeMode, HostInput stdIn, HostOutput stdOut, HostOutput stdErr, void* userPrivateData ) { // init the host for jslibs usage (modules, errors, ...)

	ASSERT( !JS_CStringsAreUTF8() );

	_unsafeMode = unsafeMode;
	HostPrivate *hpv = JL_GetHostPrivate(cx);
	if ( hpv == NULL ) { // in the case of CreateHost has not been called (because the caller wants to create and manage its own JS runtime)

		hpv = ConstructHostPrivate();
		JL_ASSERT_ALLOC( hpv );
		hpv->versionKey = JL_HOSTPRIVATE_KEY;
		JL_SetHostPrivate(cx, hpv);
	}

	hpv->privateData = userPrivateData;

	jl::QueueInitialize(&hpv->moduleList);

	hpv->unsafeMode = unsafeMode;

	if ( unsafeMode )
		JS_SetOptions(cx, JS_GetOptions(cx) & ~(JSOPTION_STRICT | JSOPTION_RELIMIT));

	hpv->report = Report;

	hpv->hostStdErr = stdErr;
	hpv->hostStdOut = stdOut;
	hpv->hostStdIn = stdIn;


	JSObject *globalObject;
	globalObject = JL_GetGlobal(cx);
	ASSERT( globalObject != NULL ); // "Global object not found."

	//JSObject *newObject = JS_NewObject(cx, NULL, NULL, NULL);
	//hpv->objectClass = JL_GetClass(newObject);
	//hpv->objectProto = JL_GetPrototype(cx, newObject);
	JL_CHK( js_GetClassPrototype(cx, globalObject, JSProto_Object, &hpv->objectProto, NULL) );
	hpv->objectClass = JL_GetClass(hpv->objectProto);
	ASSERT( hpv->objectClass );
	ASSERT( hpv->objectProto );
	
	// global functions & properties
	JL_CHKM( JS_DefinePropertyById(cx, globalObject, JLID(cx, global), OBJECT_TO_JSVAL(globalObject), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT), E_PROP, E_CREATE );

	JL_CHK( jl::InitClass(cx, globalObject, host::classSpec) ); // INIT_CLASS( host );

	// init static modules
	if ( !jslangModuleInit(cx, globalObject) )
		JL_ERR( E_MODULE, E_NAME("jslang"), E_INIT );

	return JS_TRUE;
bad:
	return JS_FALSE;
}


JSBool DestroyHost( JSContext *cx, bool skipCleanup ) {

	JSRuntime *rt = JL_GetRuntime(cx);

	HostPrivate *hpv = JL_GetHostPrivate(cx);
	JL_ASSERT( hpv, E_HOST, E_STATE, E_COMMENT("context private") );

	ASSERT( hpv->isEnding == false );

	hpv->isEnding = true;
	hpv->canSkipCleanup = skipCleanup;

	if ( JLThreadOk(hpv->watchDogThread) ) {

		// beware: it is important to destroy the watchDogThread BEFORE destroying the cx or hpv !!!
		JLSemaphoreRelease(hpv->watchDogSemEnd);
		JLThreadWait(hpv->watchDogThread, NULL);
		JLThreadFree(&hpv->watchDogThread);
		JLSemaphoreFree(&hpv->watchDogSemEnd);
	}

	for ( jl::QueueCell *it = jl::QueueBegin(&hpv->moduleList); it; it = jl::QueueNext(it) ) {

		JLLibraryHandler module = (JLLibraryHandler)jl::QueueGetData(it);
		ModuleReleaseFunction moduleRelease = (ModuleReleaseFunction)JLDynamicLibrarySymbol(module, NAME_MODULE_RELEASE);
		if ( moduleRelease != NULL ) {
		
			if ( !moduleRelease(cx) ) {

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


// cleanup

	// doc:
	//  - Is the only side effect of JS_DestroyContextNoGC that any finalizers I may have specified in custom objects will not get called ?
	//  - Not if you destroy all contexts (whether by NoGC or not), destroy all runtimes, and call JS_ShutDown before exiting or hibernating.
	//    The last JS_DestroyContext* API call will run a GC, no matter which API of that form you call on the last context in the runtime. /be
	JS_DestroyContext(cx);
	JS_DestroyRuntime(rt);
	cx = NULL;

	// Beware: because JS engine allocate memory from the DLL, all memory must be disallocated before releasing the DLL

	while ( !jl::QueueIsEmpty(&hpv->moduleList) ) {

		JLLibraryHandler module = (JLLibraryHandler)jl::QueueShift(&hpv->moduleList);
		ModuleFreeFunction moduleFree = (ModuleFreeFunction)JLDynamicLibrarySymbol(module, NAME_MODULE_FREE);
		if ( moduleFree != NULL )
			moduleFree();
//#ifndef DEBUG // else the memory block was allocated in a DLL that was unloaded prior to the _CrtMemDumpAllObjectsSince() call.
		JLDynamicLibraryClose(&module);
//#endif
	}

	jslangModuleFree();

	DestructHostPrivate(hpv);

	return JS_TRUE;

bad:
	// on error, do the minimum.
	if ( cx ) {
		
		JS_DestroyContext(cx);
		JS_DestroyRuntime(rt);
	}
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

	JL_IGNORE( threadArg );

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

void NewGCCallback(JSRuntime *, JSGCStatus status) {

	if ( status == JSGC_END ) {

		threadAction = MemThreadProcess;
		JLSemaphoreRelease(memoryFreeThreadSem);
	}
}

// (TBD) manage nested GCCallbacks
void MemoryManagerEnableGCEvent( JSContext *cx ) {

	JS_SetGCCallback(JL_GetRuntime(cx), NewGCCallback);
}

void MemoryManagerDisableGCEvent( JSContext *cx ) {

	JS_SetGCCallback(JL_GetRuntime(cx), NULL);
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
