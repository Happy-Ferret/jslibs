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

#define SVN_REVISION_STR "$Revision$"

#include "jslibsModule.h"

#include "host.h"

/*
static JLMutexHandler globalModuleSlotLock;
struct {
	JLLibraryHandler module;
	int instances;
} globalModuleSlot[1<<8]; // does not support more than 256 modules.
*/

JSBool jslangModuleInit(JSContext *cx, JSObject *obj);
JSBool jslangModuleRelease(JSContext *cx);
void jslangModuleFree();

//bool _unsafeMode = true;

JSErrorFormatString JLerrorFormatString[JLErrLimit] = {
#define MSG_DEF(name, number, count, exception, format) { format, count, exception },
	#include "jlerrors.msg"
#undef MSG_DEF
};


static const JSErrorFormatString *GetErrorMessage(void *userRef, const char *locale, const uintN errorNumber) {

	uintN err = errorNumber - 1000;
	if ( err > 0 && err < JLErrLimit )
		return &JLerrorFormatString[err];
	return NULL;
}


//#define RT_HOST_MAIN_ASSERT( condition, errorMessage )
//	if ( !(condition) ) { consoleStdErr( cx, errorMessage, sizeof(errorMessage)-1 ); return -1; }


static JSBool JSDefaultStdoutFunction(JSContext *cx, uintN argc, jsval *vp) { // fast native

	*JL_RVAL = JSVAL_VOID;
	HostPrivate *pv = JL_GetHostPrivate(cx);
	if (unlikely( pv == NULL || pv->hostStdOut == NULL ))
		return JS_TRUE;

//	const char *buffer;
//	size_t length;

	for ( uintN i = 0; i < argc; ++i ) {

		//JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(i+1), &buffer, &length) );
		//JL_CHKM( pv->hostStdOut(pv->privateData, buffer, length) != -1, "Unable to use write on host's StdOut." );
		JLStr str;
		JL_CHK( JL_JsvalToNative(cx, JL_ARGV[i], &str) );
		JL_CHKM( pv->hostStdOut(pv->privateData, str.GetConstStr(), str.Length()) != -1, "Unable to use write on host's StdOut." );
	}
	return JS_TRUE;
	JL_BAD;
}


static JSBool JSDefaultStderrFunction(JSContext *cx, uintN argc, jsval *vp) { // fast native

	*JL_RVAL = JSVAL_VOID;
	HostPrivate *pv = JL_GetHostPrivate(cx);
	if (unlikely( pv == NULL || pv->hostStdErr == NULL ))
		return JS_TRUE;

	for ( uintN i = 0; i < argc; ++i ) {

//		JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(i+1), &buffer, &length) );
//		JL_CHKM( pv->hostStdErr(pv->privateData, buffer, length) != -1, "Unable to use write on host's StdErr." );
		JLStr str;
		JL_CHK( JL_JsvalToNative(cx, JL_ARGV[i], &str) );
		JL_CHKM( pv->hostStdErr(pv->privateData, str.GetConstStr(), str.Length()) != -1, "Unable to use write on host's StdErr." );
	}
	return JS_TRUE;
	JL_BAD;
}


void stdErrRouter(JSContext *cx, const char *message, size_t length) {

	JSObject *globalObject = JS_GetGlobalObject(cx);
	if (likely( globalObject != NULL )) {

		jsval fct;
		if (likely( GetConfigurationValue(cx, JLID_NAME(cx, stderr), &fct) == JS_TRUE && JL_JsvalIsFunction(cx, fct) )) {
			
			// possible optimization, but not very useful since errors occurs rarely.
			//JSFunction *fun = GET_FUNCTION_PRIVATE(cx, JSVAL_TO_OBJECT(fct));
			//if ( FUN_FAST_NATIVE(fun) == (JSFastNative)JSDefaultStderrFunction )
			//	goto standard_way;

			jsval unused;
			js::AutoValueRooter tvr(cx); // needed to protect the string.

//			JL_CHKB( JL_StringAndLengthToJsval(cx, tvr.jsval_addr(), message, length), bad2 ); // beware out of memory case !
			JL_CHKB( JL_NativeToJsval(cx, message, length, tvr.jsval_addr()), bad2 ); // beware out of memory case !
			JL_CHKB( JS_CallFunctionValue(cx, globalObject, fct, 1, tvr.jsval_addr(), &unused), bad2 );

		bad2:
			return;
		}
	}

	HostPrivate *pv;
	pv = JL_GetHostPrivate(cx);
	if (unlikely( pv == NULL || pv->hostStdErr == NULL ))
		return;
	pv->hostStdErr(pv->privateData, message, length); // else, use the default.
	return;
}


// function copied from ../js/src/js.c
static void ErrorReporter(JSContext *cx, const char *message, JSErrorReport *report) {

	HostPrivate *pv = JL_GetHostPrivate(cx);
	if (likely( pv != NULL && JSREPORT_IS_WARNING(report->flags) && pv->unsafeMode )) // no warnings in unsafe mode.
		return;

	// trap JSMSG_OUT_OF_MEMORY error to avoid calling stdErrRouter() that may allocate memory that will lead to nested call.
	if ( report->errorNumber == JSMSG_OUT_OF_MEMORY ) { // (TBD) do something better
		
		fprintf(stderr, "%s (%s:%d)\n", message, report->filename, report->lineno );
		return;
	}

	char *msg;


    int i, j, k, n;
    char *prefix, *tmp;
    const char *ctmp;

    if (!report) {
        //fprintf(gErrFile, "%s\n", message);
		 stdErrRouter( cx, message, strlen(message) );
		 stdErrRouter( cx, "\n", 1 );
        return;
    }

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
            //fputs(prefix, gErrFile);
				stdErrRouter( cx, prefix, strlen(prefix) );
        //fwrite(message, 1, ctmp - message, gErrFile);
		  stdErrRouter( cx, message, ctmp - message );

        message = ctmp;
    }

    /* If there were no filename or lineno, the prefix might be empty */
    if (prefix)
        //fputs(prefix, gErrFile);
		  stdErrRouter( cx, prefix, strlen(prefix) );
    //fputs(message, gErrFile);
	 stdErrRouter( cx, message, strlen(message) );

    if (!report->linebuf) {
        //fputc('\n', gErrFile);
		 stdErrRouter( cx, "\n", 1 );
        goto out;
    }

    /* report->linebuf usually ends with a newline. */
    n = (int)strlen(report->linebuf);
    //fprintf(gErrFile, ":\n%s%s%s%s",
	 msg = JS_smprintf(":\n%s%s%s%s",
            prefix,
            report->linebuf,
            (n > 0 && report->linebuf[n-1] == '\n') ? "" : "\n",
            prefix);
	 stdErrRouter( cx, msg, strlen(msg) );
	JS_smprintf_free(msg);
    n = report->tokenptr - report->linebuf;
    for (i = j = 0; i < n; i++) {
        if (report->linebuf[i] == '\t') {
            for (k = (j + 8) & ~7; j < k; j++) {
                //fputc('.', gErrFile);
					stdErrRouter( cx, ".", 1 );
            }
            continue;
        }
        //fputc('.', gErrFile);
		  stdErrRouter( cx, ".", 1 );
        j++;
    }
    //fputs("^\n", gErrFile);
	 stdErrRouter( cx, "^\n", 2 );
 out:
/*
	 if (!JSREPORT_IS_WARNING(report->flags)) {
        if (report->errorNumber == JSMSG_OUT_OF_MEMORY) {
            gExitCode = EXITCODE_OUT_OF_MEMORY;
        } else {
            gExitCode = EXITCODE_RUNTIME_ERROR;
        }
    }
*/
    JS_free(cx, prefix);
}


static JSBool OperationCallback(JSContext *cx) {

	JS_MaybeGC(cx);
	return JS_TRUE;
}


JLThreadFuncDecl WatchDogThreadProc(void *threadArg) {

	JSContext *cx = (JSContext*)threadArg;
	HostPrivate *pv = JL_GetHostPrivate(cx);
//	JSPackedBool *gcRunning = &cx->runtime->gcRunning;
//	JLReleaseSemaphore(pv->watchDogSem); // signals that the thread has started
	for (;;) {

		//SleepMilliseconds(pv->maybeGCInterval); // use a timed semaphore instead (see SandboxEval)
		if ( JLSemaphoreAcquire(pv->watchDogSemEnd, pv->maybeGCInterval) != JLTIMEOUT ) // used as a breakable Sleep.
			JLThreadExit(0);
		JS_TriggerOperationCallback(cx);
	}
	JLThreadExit(0);
	return 0;
}


static JSBool LoadModule(JSContext *cx, uintN argc, jsval *vp) {

	JLStr str;

	//	CHKHEAP();

	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_ARG_MIN(1);
//	const char *fileName;
//	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &fileName) );
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

					JL_S_ASSERT_OBJECT( existingNsVal );
					nsObj = JSVAL_TO_OBJECT( existingNsVal );
				}
				obj = nsObj;
			}
		}
	}
*/
	HostPrivate *pv;
	pv = JL_GetHostPrivate(cx);
	JL_S_ASSERT( pv, "Invalid host context private." );

	JL_S_ASSERT( libFileName != NULL && *libFileName != '\0', "Invalid module filename." );
	JLLibraryHandler module;
	module = JLDynamicLibraryOpen(libFileName);
	if ( !JLDynamicLibraryOk(module) ) {
		JL_SAFE_BEGIN

			char errorBuffer[256];
			JLDynamicLibraryLastErrorMessage( errorBuffer, sizeof(errorBuffer) );
			JL_REPORT_WARNING( "Unable to load the module \"%s\". %s", libFileName, errorBuffer );
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
	JL_CHKBM( moduleInit, bad_dl_close, "Invalid module." );

//	CHKHEAP();

	JL_CHKBM( moduleInit(cx, JL_OBJ, uid), bad_dl_close, "Unable to initialize the module." );

//	CHKHEAP();

	jl::QueueUnshift( &pv->moduleList, module ); // store the module (LIFO)
	JL_CHK( JL_NewNumberValue(cx, uid, JL_RVAL) ); // really needed ? yes, UnloadModule will need this ID
	return JS_TRUE;

bad_dl_close:
	JLDynamicLibraryClose(&module);
bad:
	return JS_FALSE;
}

/* (TBD)
static JSBool UnloadModule(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	JL_S_ASSERT_ARG_MIN(1);
	jsdouble dVal;
	JL_CHK( JS_ValueToNumber(cx, argv[0], &dVal) );
	ModuleId id = (ModuleId)dVal;

	if ( ModuleIsUnloadable(id) ) {

		bool st = ModuleUnload(id, cx);
		JL_S_ASSERT( st == true, "Unable to unload the module" );
		*rval = JSVAL_TRUE;
	} else {

		*rval = JSVAL_FALSE;
	}
	return JS_TRUE;
	JL_BAD;
}
*/

static JSBool global_enumerate(JSContext *cx, JSObject *obj) { // see LAZY_STANDARD_CLASSES

	return JS_EnumerateStandardClasses(cx, obj);
}

static JSBool global_resolve(JSContext *cx, JSObject *obj, jsid id, uintN flags, JSObject **objp) { // see LAZY_STANDARD_CLASSES

	if ( (flags & JSRESOLVE_ASSIGNING) == 0 ) {

		JSBool resolved;
		if ( !JS_ResolveStandardClass(cx, obj, id, &resolved) )
			return JS_FALSE;

		if ( resolved ) {
			
			*objp = obj;
			return JS_TRUE;
		}
	}
	return JS_TRUE;
}

// global object
// doc: For full ECMAScript standard compliance, obj should be of a JSClass that has the JSCLASS_GLOBAL_FLAGS flag.
static JSClass global_class = { // global variable, but this is not an issue even is several runtimes share the same JSClass.
	NAME_GLOBAL_CLASS, JSCLASS_GLOBAL_FLAGS | JSCLASS_NEW_RESOLVE,  // | JSCLASS_HAS_PRIVATE
	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
	global_enumerate, (JSResolveOp)global_resolve, JS_ConvertStub, JS_FinalizeStub, // see LAZY_STANDARD_CLASSES
	JSCLASS_NO_OPTIONAL_MEMBERS
};


// default: CreateHost(-1, -1, 0);
JSContext* CreateHost(uint32 maxMem, uint32 maxAlloc, uint32 maybeGCInterval ) {

//	JS_SetCStringsAreUTF8(); // don't use !
	JSRuntime *rt = JS_NewRuntime(0); // maxMem specifies the number of allocated bytes after which garbage collection is run.
//	JL_CHKM( rt != NULL, "unable to create the runtime." );
	JL_CHK( rt );

//call of  'js_malloc'  acts on  'runtime->gcMallocBytes'
//do gc IF rt->gcMallocBytes >= rt->gcMaxMallocBytes

	JS_SetGCParameter(rt, JSGC_MAX_BYTES, maxMem); // maximum nominal heap before last ditch GC
	JS_SetGCParameter(rt, JSGC_MAX_MALLOC_BYTES, maxAlloc); // # of JS_malloc bytes before last ditch GC
//	JS_SetGCParameter(rt, JSGC_TRIGGER_FACTOR, 3);

	JSContext *cx;
	cx = JS_NewContext(rt, 8192L); // set the chunk size of the stack pool to 8192. see http://groups.google.com/group/mozilla.dev.tech.js-engine/browse_thread/thread/be9f404b623acf39/9efdfca81be99ca3
	JL_CHK( cx ); //, "unable to create the context." );

	// Info: Increasing JSContext stack size slows down my scripts:
	//   http://groups.google.com/group/mozilla.dev.tech.js-engine/browse_thread/thread/be9f404b623acf39/9efdfca81be99ca3

//	JS_SetNativeStackQuota(cx, 0); // see https://developer.mozilla.org/En/SpiderMonkey/JSAPI_User_Guide

	JS_SetScriptStackQuota(cx, JS_DEFAULT_SCRIPT_STACK_QUOTA); // good place to manage stack limit ( that is 32MB by default ). Btw, JS_SetScriptStackQuota ( see also JS_SetThreadStackLimit )
	JS_SetVersion(cx, (JSVersion)JSVERSION_LATEST);
	JS_SetErrorReporter(cx, ErrorReporter);

	// JSOPTION_VAROBJFIX:
	//  Not quite: with JSOPTION_VAROBJFIX, both explicitly declared global
	//  variables (var x) and implicit ones (x = 42 where no x exists yet in the
	//  scope chain) both go in the last object on the parent-linked scope
	//  chain.  Without that option, explicit globals go in the first object on
	//  the scope chain, while implicit globals go on the last.
	//  ---
	//  One way to use JSOPTION_VAROBJFIX would be to temporarily
	//  JS_SetParent(cx, libobj, NULL) and JS_SetParent(cx, libobj, global)
	//  around all JS_Evaluate*Script* and JS_Compile* API calls.)
	// JSOPTION_RELIMIT:
	//  Throw exception on any regular expression which backtracks more than n^3 times, where n is length of the input string
	// JSOPTION_JIT: "I think it's possible we'll remove even this little bit of API, and just have the JIT always-on. -j"
	// JSOPTION_ANONFUNFIX: https://bugzilla.mozilla.org/show_bug.cgi?id=376052 
	JL_ASSERT( JS_GetOptions(cx) == 0 );
	JS_SetOptions(cx, JSOPTION_VAROBJFIX | JSOPTION_XML | JSOPTION_JIT | JSOPTION_METHODJIT | JSOPTION_PROFILING | JSOPTION_ANONFUNFIX);

	JSObject *globalObject;
	globalObject = JS_NewGlobalObject(cx, &global_class);
	JL_CHK( globalObject ); // "unable to create the global object." );

	//	JS_SetGlobalObject(cx, globalObject); // not needed. Doc: As a side effect, JS_InitStandardClasses establishes obj as the global object for cx, if one is not already established.

// Standard classes
	//	jsStatus = JS_InitStandardClasses(cx, globalObject); // use NULL instead of globalObject ?
	//	RT_HOST_MAIN_ASSERT( jsStatus == JS_TRUE, "unable to initialize standard classes." );

	JS_SetGlobalObject(cx, globalObject); // see LAZY_STANDARD_CLASSES

	HostPrivate *pv;
	pv = (HostPrivate*)jl_calloc(sizeof(HostPrivate), 1); // beware: don't realloc, because WatchDogThreadProc points on it !!!
	JL_S_ASSERT_ALLOC( pv );
//	memset(pv, 0, sizeof(HostPrivate)); // mandatory ! or use calloc
	pv->hostPrivateVersion = JL_HOST_PRIVATE_VERSION;
	JL_SetHostPrivate(cx, pv);

	// setup WatchDog
	if ( maybeGCInterval ) {

		pv->maybeGCInterval = maybeGCInterval;
		JS_SetOperationCallback(cx, OperationCallback);
		pv->watchDogSemEnd = JLSemaphoreCreate(0);
		pv->watchDogThread = JLThreadStart(WatchDogThreadProc, cx);
		//	JLThreadPriority(pv->watchDogThread, JL_THREAD_PRIORITY_LOW);
		JL_CHKM( JLSemaphoreOk(pv->watchDogSemEnd) && JLThreadOk(pv->watchDogThread), "Unable to create the GC thread." );
	}
	return cx;

bad:
	return NULL;
}


JSBool InitHost( JSContext *cx, bool unsafeMode, HostOutput stdOut, HostOutput stdErr, void* userPrivateData ) { // init the host for jslibs usage (modules, errors, ...)

	_unsafeMode = unsafeMode;

	HostPrivate *pv = JL_GetHostPrivate(cx);
	if ( pv == NULL ) { // in the case of CreateHost has not been called (because the caller wants to create and manage its own JS runtime)

		pv = (HostPrivate*)jl_calloc(sizeof(HostPrivate), 1); // beware: don't realloc, because WatchDogThreadProc points on it !!!
		JL_S_ASSERT_ALLOC( pv );
//		memset(pv, 0, sizeof(HostPrivate)); // mandatory ! or use calloc
		pv->hostPrivateVersion = JL_HOST_PRIVATE_VERSION;
		JL_SetHostPrivate(cx, pv);
	}

	pv->privateData = userPrivateData;

	jl::QueueInitialize(&pv->moduleList);

	pv->unsafeMode = unsafeMode;

	if ( unsafeMode )
		JS_SetOptions(cx, JS_GetOptions(cx) & ~(JSOPTION_STRICT | JSOPTION_RELIMIT));

	JSObject *globalObject;
	globalObject = JS_GetGlobalObject(cx);
	JL_CHKM( globalObject != NULL, "Global object not found." );

	//	JSBool found;
	//	uintN attrs;
	//	JL_CHK( JS_GetPropertyAttributes(cx, globalObject, "undefined", &attrs, &found) );
	//	JL_CHK( JS_SetPropertyAttributes(cx, globalObject, "undefined", attrs | JSPROP_READONLY, &found) );
	
	//JL_CHK( globalObject->defineProperty(cx, ATOM_TO_JSID(JL_GetRuntime(cx)->atomState.typeAtoms[JSTYPE_VOID]), JSVAL_VOID, NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY) ); // by default, undefined is only JSPROP_PERMANENT
	

	JL_CHK( JS_DefinePropertyById( cx, globalObject, ATOM_TO_JSID(JL_GetRuntime(cx)->atomState.typeAtoms[JSTYPE_VOID]), JSVAL_VOID, NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY) ); // by default, undefined is only JSPROP_PERMANENT

	// creates a reference to the String object JSClass
	pv->stringObjectClass = JL_GetStandardClass(cx, JSProto_String);
	JL_CHKM( pv->stringObjectClass, "Unable to find the String class." );

// make GetErrorMessage available from any module
//	void **pGetErrorMessage;
//	pGetErrorMessage = (void**)jl_malloc(sizeof(void*)); // free is done in DestroyHost()
//	*pGetErrorMessage = (void*)&GetErrorMessage; // this indirection is needed for alignement purpose. see PRIVATE_TO_JSVAL and C function alignement.
//	JL_CHK( SetConfigurationPrivateValue(cx, JLID_NAME(_getErrorMessage), PRIVATE_TO_JSVAL(pGetErrorMessage)) );
	JS_SetLocaleCallbacks(cx, &pv->localeCallbacks);

	pv->errorCallback = GetErrorMessage;
	pv->localeCallbacks.localeGetErrorMessage = pv->errorCallback;

	pv->hostStdErr = stdErr;
	pv->hostStdOut = stdOut;

// global functions & properties
	JL_CHKM( JS_DefinePropertyById( cx, globalObject, JLID(cx, global), OBJECT_TO_JSVAL(JS_GetGlobalObject(cx)), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT ), "unable to define a property." );
	JL_CHKM( JS_DefineFunction( cx, globalObject, JL_GetHostPrivate(cx)->camelCase == 1 ? JLNormalizeFunctionName(NAME_GLOBAL_FUNCTION_LOAD_MODULE) : NAME_GLOBAL_FUNCTION_LOAD_MODULE, LoadModule, 0, 0 ), "unable to define a property." );
	// jslibs is not ready to support UnloadModule()
	//	JL_CHKM( JS_DefineFunction( cx, globalObject, JL_GetHostPrivate(cx)->camelCase == 1 ? _NormalizeFunctionName(NAME_GLOBAL_FUNCTION_UNLOAD_MODULE) : NAME_GLOBAL_FUNCTION_UNLOAD_MODULE, UnloadModule, 0, 0 ), "unable to define a property." );

	JL_CHK( SetConfigurationReadonlyValue(cx, JLID_NAME(cx, unsafeMode), unsafeMode ? JSVAL_TRUE : JSVAL_FALSE) );

// support this: var prevStderr = _configuration.stderr; _configuration.stderr = function(txt) { file.Write(txt); prevStderr(txt) };
	jsval value;
	value = OBJECT_TO_JSVAL(JS_GetFunctionObject(JS_NewFunction(cx, JSDefaultStdoutFunction, 1, 0, NULL, NULL))); // If you do not assign a name to the function, it is assigned the name "anonymous".
	JL_CHK( SetConfigurationValue(cx, JLID_NAME(cx, stdout), value) );
	value = OBJECT_TO_JSVAL(JS_GetFunctionObject(JS_NewFunction(cx, JSDefaultStderrFunction, 1, 0, NULL, NULL))); // If you do not assign a name to the function, it is assigned the name "anonymous".
	JL_CHK( SetConfigurationValue(cx, JLID_NAME(cx, stderr), value) );

// init static modules
	JL_CHKM( jslangModuleInit(cx, globalObject), "Unable to initialize jslang." );

	JL_CHK( JS_DefinePropertyById(cx, globalObject, JLID(cx, _revision), INT_TO_JSVAL(JL_SvnRevToInt(SVN_REVISION_STR)), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT) );

	return JS_TRUE;
	JL_BAD;
}


JSBool DestroyHost( JSContext *cx ) {

	JSRuntime *rt = JL_GetRuntime(cx);

	HostPrivate *pv = JL_GetHostPrivate(cx);
	JL_S_ASSERT( pv, "Invalid host context private." );

	if ( JLThreadOk(pv->watchDogThread) ) {

		// beware: it is important to destroy the watchDogThread BEFORE destroying the cx or pv !!!
		JLSemaphoreRelease(pv->watchDogSemEnd);
		JLThreadWait(pv->watchDogThread, NULL);
		JLThreadFree(&pv->watchDogThread);
		JLSemaphoreFree(&pv->watchDogSemEnd);
	}

	jslangModuleRelease(cx);

	for ( jl::QueueCell *it = jl::QueueBegin(&pv->moduleList); it; it = jl::QueueNext(it) ) {

		JLLibraryHandler module = (JLLibraryHandler)jl::QueueGetData(it);
		ModuleReleaseFunction moduleRelease = (ModuleReleaseFunction)JLDynamicLibrarySymbol(module, NAME_MODULE_RELEASE);
		if ( moduleRelease != NULL )
			moduleRelease(cx);
	}


	//	don't try to break linked objects with JS_GC(cx) !

//	jsval tmp;
//	JL_CHK( GetConfigurationValue(cx, JLID_NAME(_getErrorMessage), &tmp) );
//	if ( tmp != JSVAL_VOID && JSVAL_TO_PRIVATE(tmp) )
//		jl_free( JSVAL_TO_PRIVATE(tmp) );

	JL_CHKM( RemoveConfiguration(cx), "Unable to remove the configuration item." );

	JS_SetGlobalObject(cx, JSVAL_TO_OBJECT(JSVAL_NULL)); // remove the global object (TBD) check if it is good or needed to do this.

// cleanup

	// doc:
	//  - Is the only side effect of JS_DestroyContextNoGC that any finalizers I may have specified in custom objects will not get called ?
	//  - Not if you destroy all contexts (whether by NoGC or not), destroy all runtimes, and call JS_ShutDown before exiting or hibernating.
	//    The last JS_DestroyContext* API call will run a GC, no matter which API of that form you call on the last context in the runtime. /be
	JS_DestroyContext(cx);
	JS_DestroyRuntime(rt);
	cx = NULL;


	// Beware: because JS engine allocate memory from the DLL, all memory must be disallocated before releasing the DLL
	//	ModuleFreeAll();

	jslangModuleFree();

	while ( !jl::QueueIsEmpty(&pv->moduleList) ) {

		JLLibraryHandler module = (JLLibraryHandler)jl::QueueShift(&pv->moduleList);
		ModuleFreeFunction moduleFree = (ModuleFreeFunction)JLDynamicLibrarySymbol(module, NAME_MODULE_FREE);
		if ( moduleFree != NULL )
			moduleFree();
//#ifndef DEBUG // else the memory block was allocated in a DLL that was unloaded prior to the _CrtMemDumpAllObjectsSince() call.
		JLDynamicLibraryClose(&module);
//#endif
	}



	while ( !jl::QueueIsEmpty(&pv->registredNativeClasses) )
		jl::QueueShift(&pv->registredNativeClasses);

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

	JSObject *globalObject = JS_GetGlobalObject(cx);
	JL_CHKM( globalObject != NULL, "Global object not found." );
	JL_CHKM( JS_DeletePropertyById(cx, globalObject, JLID(cx, arguments)), "Unable to cleanup script arguments." );
	return JS_TRUE;
	JL_BAD;
}

JSBool CreateScriptArguments( JSContext *cx, int argc, const char * const * argv ) {

	JSObject *globalObject = JS_GetGlobalObject(cx);
	JL_CHKM( globalObject != NULL, "Global object not found." );

	JSObject *argsObj;
	argsObj = JS_NewArrayObject(cx, argc, NULL); // JL_JsvalIsArray(cx, OBJECT_TO_JSVAL(argsObj));
	
	JL_CHKM( argsObj != NULL, "Unable to create script arguments." );

	JL_CHKM( JS_DefinePropertyById(cx, globalObject, JLID(cx, arguments), OBJECT_TO_JSVAL(argsObj), NULL, NULL, /*JSPROP_READONLY | JSPROP_PERMANENT*/ 0), "Unable to store script arguments." );

	for ( int index = 0; index < argc; index++ ) {

		JSString *str = JS_NewStringCopyZ(cx, argv[index]);
		JL_CHKM( str != NULL, "Unable to store the argument." );
		JL_CHKM( JS_DefineElement(cx, argsObj, index, STRING_TO_JSVAL(str), NULL, NULL, JSPROP_ENUMERATE), "Unable to define the argument." );
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

	js::AutoObjectRooter scriptObjRoot(cx);

	JSObject *globalObject = JS_GetGlobalObject(cx);
	JL_CHKM( globalObject != NULL, "Global object not found." );

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
	scriptObjRoot.setObject(JS_NewScriptObject(cx, script));

	// mendatory else the exception is converted into an error before JL_IsExceptionPending can be used. Exceptions can be reported with JS_ReportPendingException().
	JS_SetOptions(cx, JS_GetOptions(cx) | JSOPTION_DONT_REPORT_UNCAUGHT);

	// You need to protect a JSScript (via a rooted script object) if and only if a garbage collection can occur between compilation and the start of execution.
	if ( !compileOnly )

		JL_CHK( JS_ExecuteScript(cx, globalObject, script, rval) ); // MUST be executed only once ( JSOPTION_COMPILE_N_GO )
	else
		*rval = JSVAL_VOID;

	JS_DestroyScript(cx, script); // Warning: This API is subject to bug 438633, which can cause crashes in almost any program that uses JS_DestroyScript.

	JL_CHK( RemoveScriptArguments( cx ) );
	JS_SetOptions(cx, prevOpt);
	return JS_TRUE;

bad:
	JS_SetOptions(cx, prevOpt);
	return JS_FALSE;
}



JSBool ExecuteScriptFileName( JSContext *cx, const char *scriptFileName, bool compileOnly, int argc, const char * const * argv, jsval *rval ) { // (TBD) support xdr files

	uint32 prevOpt = JS_SetOptions(cx, JS_GetOptions(cx) | JSOPTION_COMPILE_N_GO);
	js::AutoObjectRooter scriptObjRoot(cx);
	JSObject *globalObject = JS_GetGlobalObject(cx);
	JL_CHKM( globalObject != NULL, "Global object not found." );
	JL_CHK( CreateScriptArguments(cx, argc, argv) );

	JSScript *script;
	script = JL_LoadScript(cx, globalObject, scriptFileName, true, false); // use xdr if available, but don't save it.
	JL_CHK( script );

	scriptObjRoot.setObject(JS_NewScriptObject(cx, script));

	// mendatory else the exception is converted into an error before JL_IsExceptionPending can be used. Exceptions can be reported with JS_ReportPendingException().
	JS_SetOptions(cx, JS_GetOptions(cx) | JSOPTION_DONT_REPORT_UNCAUGHT);

	// You need to protect a JSScript (via a rooted script object) if and only if a garbage collection can occur between compilation and the start of execution.

	if ( !compileOnly )
		JL_CHK( JS_ExecuteScript(cx, globalObject, script, rval) ); // MUST be executed only once ( JSOPTION_COMPILE_N_GO )
	else
		*rval = JSVAL_VOID;

	JS_DestroyScript(cx, script); // Warning: This API is subject to bug 438633 (FIXED), which can cause crashes in almost any program that uses JS_DestroyScript.

	//JL_CHKM( JS_DeletePropertyById(cx, globalObject, JLID(cx, arguments)), "Unable to remove argument property." );
	JL_CHK( RemoveScriptArguments(cx) );
	JS_SetOptions(cx, prevOpt);
	return JS_TRUE;

bad:
	JS_SetOptions(cx, prevOpt);
	return JS_FALSE;
}


JSBool ExecuteBootstrapScript( JSContext *cx, void *xdrScript, uint32 xdrScriptLength ) {

	uint32 prevOpt = JS_SetOptions(cx, JS_GetOptions(cx) & ~JSOPTION_DONT_REPORT_UNCAUGHT); // report uncautch exceptions !
//	JL_CHKM( JS_EvaluateScript(cx, JS_GetGlobalObject(cx), embeddedBootstrapScript, sizeof(embeddedBootstrapScript)-1, "bootstrap", 1, &tmp), "Invalid bootstrap." ); // for plain text scripts.
	js::AutoObjectRooter scriptObjRoot(cx);
	JSXDRState *xdr = JS_XDRNewMem(cx, JSXDR_DECODE);
	JL_CHK( xdr );
	JS_XDRMemSetData(xdr, xdrScript, xdrScriptLength);
	JSScript *script = NULL;
	JL_CHK( JS_XDRScript(xdr, &script) );
	JS_XDRMemSetData(xdr, NULL, 0); // embeddedBootstrapScript is a static buffer, this avoid JS_free to be called on it.
	JS_XDRDestroy(xdr);
	scriptObjRoot.setObject(JS_NewScriptObject(cx, script));
//	JL_CHK( SetConfigurationReadonlyValue(cx, JLID_NAME(cx, bootstrapScript), OBJECT_TO_JSVAL(bootstrapScriptObject)) ); // bootstrap script cannot be hidden
	jsval tmp;
	JL_CHK( JS_ExecuteScript(cx, JS_GetGlobalObject(cx), script, &tmp) );
	JS_DestroyScript(cx, script);
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
static volatile long headLength;
static volatile long load;

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
static void* JslibsMalloc( size_t size ) {

	if (likely( size >= sizeof(void*) ))
		return base_malloc(size);
	return base_malloc(sizeof(void*));
}

static void* JslibsCalloc( size_t num, size_t size ) {

	size *= num;
	if (likely( size >= sizeof(void*) ))
		return base_calloc(size, 1);
	return base_calloc(sizeof(void*), 1);
}

static void* JslibsMemalign( size_t alignment, size_t size ) {

	if (likely( size >= sizeof(void*) ))
		return base_memalign(alignment, size);
	return base_memalign(alignment, sizeof(void*));
}

static void* JslibsRealloc( void *ptr, size_t size ) {

	if (likely( size >= sizeof(void*) ))
		return base_realloc(ptr, size);
	return base_realloc(ptr, sizeof(void*));
}

static size_t JslibsMsize( void *ptr ) {

	return base_msize(ptr);
}

static void JslibsFree( void *ptr ) {
	
	if (unlikely( ptr == NULL )) // issue wuth nedmalloc free(NULL)
		return;

	if (unlikely( base_msize(ptr) >= BIG_ALLOC || load >= MAX_LOAD )) { // if blocks is big OR too many things to free, the thread can not keep pace.

		base_free(ptr);
		return;
	}

	*(void**)ptr = head;
	head = ptr;
	JLAtomicIncrement(&headLength);
}


ALWAYS_INLINE void FreeHead() {

	headLength = 0;

	void *it, *next = head;
	it = *(void**)next;
	*(void**)next = NULL;

	while ( it ) {

		JL_ASSERT( it != *(void**)it );
		next = *(void**)it;
		base_free(it);
		it = next;
	}
}

// the thread proc
static JLThreadFuncDecl MemoryFreeThreadProc( void *threadArg ) {

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
