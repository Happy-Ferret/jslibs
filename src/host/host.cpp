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

#include "../common/jslibsModule.h"

#include "host.h"

JSBool jslangInit(JSContext *cx, JSObject *obj);


//bool _unsafeMode = true;

JSErrorFormatString errorFormatString[J_ErrLimit] = {
	#define MSG_DEF(name, number, count, exception, format) { format, count, exception },
	#include "../common/errors.msg"
	#undef MSG_DEF
};


static const JSErrorFormatString *GetErrorMessage(void *userRef, const char *locale, const uintN errorNumber) {

	if ((errorNumber > 0) && (errorNumber < J_ErrLimit))
		return &errorFormatString[errorNumber];
	return NULL;
}


//#define RT_HOST_MAIN_ASSERT( condition, errorMessage )
//	if ( !(condition) ) { consoleStdErr( cx, errorMessage, sizeof(errorMessage)-1 ); return -1; }


static JSBool JSDefaultStdoutFunction(JSContext *cx, uintN argc, jsval *vp) { // fast native

	HostPrivate *pv = GetHostPrivate(cx);
	if (unlikely( pv == NULL || pv->hostStdOut == NULL ))
		return JS_TRUE;

	const char *buffer;
	size_t length;
	for ( uintN i = 0; i < argc; ++i ) {

		JL_CHK( JsvalToStringAndLength(cx, &JL_FARG(i+1), &buffer, &length) );
		pv->hostStdOut(pv->privateData, buffer, length);
	}
	return JS_TRUE;
	JL_BAD;
}


static JSBool JSDefaultStderrFunction(JSContext *cx, uintN argc, jsval *vp) { // fast native

	HostPrivate *pv = GetHostPrivate(cx);
	if (unlikely( pv == NULL || pv->hostStdErr == NULL ))
		return JS_TRUE;

	const char *buffer;
	size_t length;
	for ( uintN i = 0; i < argc; ++i ) {

		JL_CHK( JsvalToStringAndLength(cx, &JL_FARG(i+1), &buffer, &length) );
		pv->hostStdErr(pv->privateData, buffer, length);
	}
	return JS_TRUE;
	JL_BAD;
}


void stdErrRouter(JSContext *cx, const char *message, size_t length) {

	JSObject *globalObject = JS_GetGlobalObject(cx);
	if (likely( globalObject != NULL )) {

		jsval fct;
		if (likely( GetConfigurationValue(cx, NAME_CONFIGURATION_STDERR, &fct) == JS_TRUE && JsvalIsFunction(cx, fct) )) {

			jsval tmp;
			JSTempValueRooter tvr;
			JS_PUSH_SINGLE_TEMP_ROOT(cx, JSVAL_NULL, &tvr); // needed to protect the string.
			JL_CHKB( StringAndLengthToJsval(cx, &tvr.u.value, message, length), bad2 );
			JL_CHKB( JS_CallFunctionValue(cx, globalObject, fct, 1, &tvr.u.value, &tmp), bad2 );

		bad2:
			JS_POP_TEMP_ROOT(cx, &tvr);
			return;
		}
	}

	HostPrivate *pv;
	pv = GetHostPrivate(cx);
	if (unlikely( pv == NULL || pv->hostStdErr == NULL ))
		return;

	pv->hostStdErr(pv->privateData, message, length); // else, use the default.
bad:
	return;
}


// function copied from ../js/src/js.c
static void ErrorReporter(JSContext *cx, const char *message, JSErrorReport *report) {

	HostPrivate *pv = GetHostPrivate(cx);
	if (likely( pv != NULL && JSREPORT_IS_WARNING(report->flags) && pv->unsafeMode )) // no warnings in unsafe mode.
		return;

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
    n = strlen(report->linebuf);
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
	HostPrivate *pv = GetHostPrivate(cx);
//	JSPackedBool *gcRunning = &cx->runtime->gcRunning;
	JLReleaseSemaphore(pv->watchDogSem); // signals that the thread has started
	for (;;) {

		SleepMilliseconds(pv->maybeGCInterval);
//		if ( !*gcRunning )
			JS_TriggerOperationCallback(cx);
	}
}


static JSBool LoadModule(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	JL_S_ASSERT_ARG_MIN(1);
	const char *fileName;
	JL_CHK( JsvalToString(cx, &argv[0], &fileName) );
	char libFileName[PATH_MAX];
	strcpy( libFileName, fileName );
	strcat( libFileName, DLL_EXT );
// MAC OSX: 	'@executable_path' ??

/*
	while (0) { // namespace management. Avoid using val ns = {}, LoadModule.call(ns, '...');

		if ( JL_ARG_ISDEF(2) ) {

			if ( JSVAL_IS_OBJECT(JL_ARG(2)) ) {
				obj = JSVAL_TO_OBJECT(JL_ARG(2));
			} else {
				const char *ns;
				JL_CHK( JsvalToString(cx, &JL_ARG(2), &ns) );

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
	pv = GetHostPrivate(cx);
	JL_S_ASSERT( pv, "Invalid host context private." );

	JL_S_ASSERT( libFileName != NULL && *libFileName != '\0', "Invalid module filename." );
	JLLibraryHandler module;
	module = JLDynamicLibraryOpen(libFileName);
	JL_SAFE(
		if ( !JLDynamicLibraryOk(module) ) {

			char errorBuffer[256];
			JLDynamicLibraryLastErrorMessage( errorBuffer, sizeof(errorBuffer) );
			JL_REPORT_ERROR( "Unable to load the module \"%s\". %s", libFileName, errorBuffer );
		}
	);

	for ( jl::QueueCell *it = jl::QueueBegin(&pv->moduleList); it; it = jl::QueueNext(it) )
		if ( (JLLibraryHandler)jl::QueueGetData(it) == module ) {

			JL_CHK( JLDynamicLibraryClose(&module) );
			*rval = JSVAL_VOID;
			return JS_TRUE;
		}

	ModuleInitFunction moduleInit;
	moduleInit = (ModuleInitFunction)JLDynamicLibrarySymbol(module, NAME_MODULE_INIT);
	JL_CHKBM( moduleInit, bad_dl_close, "Invalid module." );
	JL_CHKBM( moduleInit(cx, obj), bad_dl_close, "Unable to initialize the module." );

	jl::QueueUnshift( &pv->moduleList, module ); // store the module (LIFO)
	JL_CHK( JS_NewNumberValue(cx, (unsigned long)module, rval) ); // really needed ? yes, UnloadModule will need this ID
	return JS_TRUE;

bad_dl_close:
	JL_CHK( JLDynamicLibraryClose(&module) );
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

static JSBool global_resolve(JSContext *cx, JSObject *obj, jsval id, uintN flags, JSObject **objp) { // see LAZY_STANDARD_CLASSES

	if ((flags & JSRESOLVE_ASSIGNING) == 0) {

		JSBool resolved;
		if (!JS_ResolveStandardClass(cx, obj, id, &resolved))
			return JS_FALSE;
		if (resolved) {
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
JSContext* CreateHost(size_t maxMem, size_t maxAlloc, size_t maybeGCInterval ) {

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
	JS_SetOptions(cx, JS_GetOptions(cx) | JSOPTION_VAROBJFIX | JSOPTION_XML /*| JSOPTION_RELIMIT*/ | JSOPTION_JIT);

	JSObject *globalObject;
	globalObject = JS_NewObject(cx, &global_class, NULL, NULL);
	JL_CHK( globalObject ); // "unable to create the global object." );

	//	JS_SetGlobalObject(cx, globalObject); // not needed. Doc: As a side effect, JS_InitStandardClasses establishes obj as the global object for cx, if one is not already established.

// Standard classes
	//	jsStatus = JS_InitStandardClasses(cx, globalObject); // use NULL instead of globalObject ?
	//	RT_HOST_MAIN_ASSERT( jsStatus == JS_TRUE, "unable to initialize standard classes." );

	JS_SetGlobalObject(cx, globalObject); // see LAZY_STANDARD_CLASSES

	HostPrivate *pv;
	pv = (HostPrivate*)jl_malloc(sizeof(HostPrivate)); // beware: don't realloc, because WatchDogThreadProc points on it !!!
	JL_S_ASSERT_ALLOC( pv );
	memset(pv, 0, sizeof(HostPrivate)); // mandatory !
	SetHostPrivate(cx, pv);

	// setup WatchDog
	if ( maybeGCInterval ) {

		pv->maybeGCInterval = maybeGCInterval;
		JS_SetOperationCallback(cx, OperationCallback);
		pv->watchDogSem = JLCreateSemaphore(0);
		pv->watchDogThread = JLThreadStart(WatchDogThreadProc, cx);
		//	JLThreadPriority(pv->watchDogThread, JL_THREAD_PRIORITY_LOW);
		JL_CHKM( JLSemaphoreOk(pv->watchDogSem) && JLThreadOk(pv->watchDogThread), "Unable to create the thread." );
	}
	return cx;

bad:
	return NULL;
}


JSBool InitHost( JSContext *cx, bool unsafeMode, HostOutput stdOut, HostOutput stdErr, void* userPrivateData ) { // init the host for jslibs usage (modules, errors, ...)

	_unsafeMode = unsafeMode;

	HostPrivate *pv = GetHostPrivate(cx);
	if ( pv == NULL ) { // in the case of CreateHost has not been called (because the caller wants to create and manage its own JS runtime)

		pv = (HostPrivate*)jl_malloc(sizeof(HostPrivate)); // beware: don't realloc, because WatchDogThreadProc points on it !!!
		JL_S_ASSERT_ALLOC( pv );
		memset(pv, 0, sizeof(HostPrivate)); // mandatory !
		SetHostPrivate(cx, pv);
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
	JL_CHK( globalObject->defineProperty(cx, ATOM_TO_JSID(JS_GetRuntime(cx)->atomState.typeAtoms[JSTYPE_VOID]), JSVAL_VOID, NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY) ); // by default, undefined is only JSPROP_PERMANENT

// creates a reference to the String object JSClass
	pv->stringObjectClass = JL_GetStringClass(cx);
	JL_CHKM( pv->stringObjectClass, "Unable to find the String class." );

// make GetErrorMessage available from any module

	void **pGetErrorMessage;
	pGetErrorMessage = (void**)jl_malloc(sizeof(void*)); // free is done in DestroyHost()
	*pGetErrorMessage = (void*)&GetErrorMessage; // this indirection is needed for alignement purpose. see PRIVATE_TO_JSVAL and C function alignement.
	JL_CHK( SetConfigurationPrivateValue(cx, NAME_CONFIGURATION_GETERRORMESSAGE, PRIVATE_TO_JSVAL(pGetErrorMessage)) );


	pv->hostStdErr = stdErr;
	pv->hostStdOut = stdOut;

// global functions & properties
	JL_CHKM( JS_DefineProperty( cx, globalObject, NAME_GLOBAL_GLOBAL_OBJECT, OBJECT_TO_JSVAL(JS_GetGlobalObject(cx)), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT ), "unable to define a property." );
	JL_CHKM( JS_DefineFunction( cx, globalObject, GetHostPrivate(cx)->camelCase == 1 ? _NormalizeFunctionName(NAME_GLOBAL_FUNCTION_LOAD_MODULE) : NAME_GLOBAL_FUNCTION_LOAD_MODULE, LoadModule, 0, 0 ), "unable to define a property." );
	// jslibs is not ready to support UnloadModule()
	//	JL_CHKM( JS_DefineFunction( cx, globalObject, GetHostPrivate(cx)->camelCase == 1 ? _NormalizeFunctionName(NAME_GLOBAL_FUNCTION_UNLOAD_MODULE) : NAME_GLOBAL_FUNCTION_UNLOAD_MODULE, UnloadModule, 0, 0 ), "unable to define a property." );

	JL_CHK( SetConfigurationReadonlyValue(cx, NAME_CONFIGURATION_UNSAFE_MODE, unsafeMode ? JSVAL_TRUE : JSVAL_FALSE) );

// support this: var prevStderr = _configuration.stderr; _configuration.stderr = function(txt) { file.Write(txt); prevStderr(txt) };
	jsval value;
	value = OBJECT_TO_JSVAL(JS_GetFunctionObject(JS_NewFunction(cx, (JSNative)JSDefaultStdoutFunction, 1, JSFUN_FAST_NATIVE, NULL, NULL))); // If you do not assign a name to the function, it is assigned the name "anonymous".
	JL_CHK( SetConfigurationValue(cx, NAME_CONFIGURATION_STDOUT, value) );
	value = OBJECT_TO_JSVAL(JS_GetFunctionObject(JS_NewFunction(cx, (JSNative)JSDefaultStderrFunction, 1, JSFUN_FAST_NATIVE, NULL, NULL))); // If you do not assign a name to the function, it is assigned the name "anonymous".
	JL_CHK( SetConfigurationValue(cx, NAME_CONFIGURATION_STDERR, value) );

// init static modules
	JL_CHKM( jslangInit(cx, globalObject), "Unable to initialize jslang." );

	JL_CHK( JS_DefineProperty(cx, globalObject, NAME_MODULE_REVISION_PROPERTY_NAME, INT_TO_JSVAL(JL_SvnRevToInt("$Revision$")), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT) );

	return JS_TRUE;
	JL_BAD;
}


JSBool DestroyHost( JSContext *cx ) {

	JSRuntime *rt = JS_GetRuntime(cx);

//	ModuleReleaseAll(cx);

	HostPrivate *pv = GetHostPrivate(cx);
	JL_S_ASSERT( pv, "Invalid host context private." );

	if ( JLThreadOk(pv->watchDogThread) ) {

		JL_CHK( JLAcquireSemaphore(pv->watchDogSem, -1) ); // prevent thread destruction before it has started
		JL_CHK( JLFreeSemaphore(&pv->watchDogSem) );
		JL_CHK( JLThreadCancel(pv->watchDogThread) );
		JL_CHK( JLWaitThread(pv->watchDogThread) );
		JL_CHK( JLFreeThread(&pv->watchDogThread) ); // beware: it is important to destroy the thread BEFORE destroying the cx !!!
	}

	for ( jl::QueueCell *it = jl::QueueBegin(&pv->moduleList); it; it = jl::QueueNext(it) ) {

		JLLibraryHandler module = (JLLibraryHandler)jl::QueueGetData(it);
		ModuleReleaseFunction moduleRelease = (ModuleReleaseFunction)JLDynamicLibrarySymbol(module, NAME_MODULE_RELEASE);
		if ( moduleRelease != NULL )
			moduleRelease(cx);
	}

	//	don't try to break linked objects with JS_GC(cx) !

	jsval tmp;
	JL_CHK( GetConfigurationValue(cx, NAME_CONFIGURATION_GETERRORMESSAGE, &tmp) );
	if ( tmp != JSVAL_VOID && JSVAL_TO_PRIVATE(tmp) )
		jl_free( JSVAL_TO_PRIVATE(tmp) );

	JL_CHKM( RemoveConfiguration(cx), "Unable to remove the configuration item." );

	JS_SetGlobalObject(cx, JSVAL_TO_OBJECT(JSVAL_NULL)); // remove the global object (TBD) check if it is good or needed to do this.

// cleanup

	// doc:
	//  - Is the only side effect of JS_DestroyContextNoGC that any finalizers I may have specified in custom objects will not get called ?
	//  - Not if you destroy all contexts (whether by NoGC or not), destroy all runtimes, and call JS_ShutDown before exiting or hibernating.
	//    The last JS_DestroyContext* API call will run a GC, no matter which API of that form you call on the last context in the runtime. /be
	JS_CommenceRuntimeShutDown(rt);
	JS_DestroyContext(cx);
	JS_DestroyRuntime(rt);


	// Beware: because JS engine allocate memory from the DLL, all memory must be disallocated before releasing the DLL
	//	ModuleFreeAll();


	while ( !jl::QueueIsEmpty(&pv->moduleList) ) {

		JLLibraryHandler module = (JLLibraryHandler)jl::QueueShift(&pv->moduleList);
		ModuleFreeFunction moduleFree = (ModuleFreeFunction)JLDynamicLibrarySymbol(module, NAME_MODULE_FREE);
		if ( moduleFree != NULL )
			moduleFree();
//#ifndef DEBUG // else the memory block was allocated in a DLL that was unloaded prior to the _CrtMemDumpAllObjectsSince() call.
		JL_CHK( JLDynamicLibraryClose(&module) );
//#endif
	}

	while ( !jl::QueueIsEmpty(&pv->registredNativeClasses) )
		jl::QueueShift(&pv->registredNativeClasses);

	jl_free(pv);
	return JS_TRUE;

bad:
	jl_free(pv);
	return JS_FALSE;
}


/*
void HostPrincipalsDestroy(JSContext *cx, JSPrincipals *principals) {

	jl_free(principals->codebase);
	jl_free(principals);
}
*/

JSBool ExecuteScriptFileName( JSContext *cx, const char *scriptFileName, bool compileOnly, int argc, const char * const * argv, jsval *rval ) { // (TBD) support xdr files 

	uint32 prevOpt = JS_SetOptions(cx, JS_GetOptions(cx) | JSOPTION_COMPILE_N_GO); //  | JSOPTION_DONT_REPORT_UNCAUGHT
	// JSOPTION_COMPILE_N_GO:
	//  caller of JS_Compile*Script promises to execute compiled script once only; enables compile-time scope chain resolution of consts.
	// JSOPTION_DONT_REPORT_UNCAUGHT:
	//  When returning from the outermost API call, prevent uncaught exceptions from being converted to error reports
	//  we can use JS_ReportPendingException to report it manually

	JSObject *globalObject = JS_GetGlobalObject(cx);
	JL_CHKM( globalObject != NULL, "Global object not found." );

// arguments
	JSObject *argsObj;
	argsObj = JS_NewArrayObject(cx, argc, NULL);
	JL_CHKM( argsObj != NULL, "Unable to create argument array on the global object." );

	JL_CHKM( JS_DefineProperty(cx, globalObject, NAME_GLOBAL_ARGUMENTS, OBJECT_TO_JSVAL(argsObj), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT), "unable to store the argument array." );

	for ( int index = 0; index < argc; index++ ) {

		JSString *str = JS_NewStringCopyZ(cx, argv[index]);
		JL_CHKM( str != NULL, "Unable to store the argument." );
		JL_CHKM( JS_DefineElement(cx, argsObj, index, STRING_TO_JSVAL(str), NULL, NULL, JSPROP_ENUMERATE), "unable to define the argument." );
	}

// compile & executes the script

/*
	JSPrincipals *principals = (JSPrincipals*)jl_malloc(sizeof(JSPrincipals));
	JSPrincipals tmp = {0};
	*principals = tmp;
	principals->codebase = (char*)jl_malloc(PATH_MAX);
	strncpy(principals->codebase, scriptFileName, PATH_MAX-1);
	principals->refcount = 1;
	principals->destroy = HostPrincipalsDestroy;
*/

	JSScript *script;
	script = JLLoadScript(cx, globalObject, scriptFileName, true, false); // use xdr if available, but don't save it.
	JL_CHK( script );


	JSTempValueRooter tvr;
	JSObject *scrobj;
	scrobj = JS_NewScriptObject(cx, script);
	JS_PUSH_TEMP_ROOT_OBJECT(cx, scrobj, &tvr);

	// mendatory else the exception is converted into an error before JS_IsExceptionPending can be used. Exceptions can be reported with JS_ReportPendingException().
	JS_SetOptions(cx, JS_GetOptions(cx) | JSOPTION_DONT_REPORT_UNCAUGHT);

	// You need to protect a JSScript (via a rooted script object) if and only if a garbage collection can occur between compilation and the start of execution.
	JSBool status;
	if ( !compileOnly )
		status = JS_ExecuteScript(cx, globalObject, script, rval); // MUST be executed only once ( JSOPTION_COMPILE_N_GO )
	else
		*rval = JSVAL_VOID;

	JS_POP_TEMP_ROOT(cx, &tvr);
	JL_CHK( status );

	//	JS_DestroyScript(cx, script); // Warning: This API is subject to bug 438633, which can cause crashes in almost any program that uses JS_DestroyScript.

	JL_CHKM( JS_DeleteProperty(cx, globalObject, NAME_GLOBAL_ARGUMENTS ), "Unable to remove argument property." );
	JS_SetOptions(cx, prevOpt);
	return JS_TRUE;
bad:
	JS_SetOptions(cx, prevOpt);
	return JS_FALSE;
}


#define MAX_LOAD 7
#define WAIT_HEAD_FILLING 100

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
static MemThreadAction threadAction;
static JLSemaphoreHandler memoryFreeThreadSem;

// alloc functions
void* HostThreadedMalloc( size_t size ) {

	if ( size < sizeof(void*) )
		size = sizeof(void*);
	return malloc(size);
}

void* HostThreadedCalloc( size_t num, size_t size ) {
	
	size = num * size;
	if ( size < sizeof(void*) )
		size = sizeof(void*);
	return calloc(size, 1);
}

void* HostThreadedRealloc( void *ptr, size_t size ) {

	if ( size < sizeof(void*) )
		size = sizeof(void*);
	return realloc(ptr, size);
}

void HostThreadedFree( void *ptr ) {

//	return;
//	free(ptr); return;
	if ( load > MAX_LOAD ) { // too many things to free, the thread can not keep pace.
			
		free(ptr);
		return;
	}

//	*(void**)ptr = (void*)JLAtomicExchange((long*)&head, (long)ptr);
	*(void**)ptr = head;
	head = ptr;
	JLAtomicIncrement(&headLength);
}


ALWAYS_INLINE void FreeHead() {

	//void *next, *tmp = (void*)JLAtomicExchange((long*)&head, 0);
	//while ( tmp ) {
	//	
	//	next = *(void**)tmp;
	//	free(tmp);
	//	tmp = next;
	//}

	if ( !head )
		return;

	headLength = 0;

	void *it, *next = head;
	it = *(void**)next;
	*(void**)next = NULL;

	while ( it ) {
		
		next = *(void**)it;
		free(it);
		it = next;
	}
}

// the thread proc
JLThreadFuncDecl MemoryFreeThreadProc( void *threadArg ) {
	
	for (;;) {
		
		load = 0;
		while ( headLength ) {
			
			load++;
			FreeHead();
		}

		if ( JLAcquireSemaphore(memoryFreeThreadSem, WAIT_HEAD_FILLING) == JLOK )
			switch ( threadAction ) {
				case MemThreadExit:
					JLThreadExit();
				case MemThreadProcess:
					;
			}
	}
}

// GC callback that triggers the thread
JSGCCallback prevThreadMemoryManagementGCCallback;
JSBool ThreadMemoryManagementGCCallback(JSContext *cx, JSGCStatus status) {
	
	if ( status == JSGC_FINALIZE_END ) {

		threadAction = MemThreadProcess;
		JLReleaseSemaphore(memoryFreeThreadSem);
	}
	if ( !prevThreadMemoryManagementGCCallback )
		return JS_TRUE;
	return prevThreadMemoryManagementGCCallback(cx, status);
}

// initialisation and cleanup functions
JSBool BeginThreadMemoryManagement( JSContext *cx ) {

	HostPrivate *pv = GetHostPrivate(cx);
	if ( pv == NULL )
		return JS_FALSE;

	load = 0;
	headLength = 0;
	head = NULL;
	memoryFreeThreadSem = JLCreateSemaphore(0);
	memoryFreeThread = JLThreadStart(MemoryFreeThreadProc, NULL);
//	JLThreadPriority(memoryFreeThread, JL_THREAD_PRIORITY_LOW);

	jl_malloc = HostThreadedMalloc;
	jl_calloc = HostThreadedCalloc;
	jl_realloc = HostThreadedRealloc;
	jl_free = HostThreadedFree;

	pv->malloc = jl_malloc;
	pv->calloc = jl_calloc;
	pv->realloc = jl_realloc;
	pv->free = jl_free;

	prevThreadMemoryManagementGCCallback = JS_SetGCCallback(cx, ThreadMemoryManagementGCCallback);

	JSLIBS_RegisterAllocFunctions(malloc, calloc, realloc, jl_free); // spidermonkey's allocators already check for size < sizeof(void*), then no need to use jl_malloc, ...

	return JS_TRUE;
}

JSBool EndThreadMemoryManagement( JSContext *cx, bool freeQueue ) {

	HostPrivate *pv = GetHostPrivate(cx);
	if ( pv == NULL )
		return JS_FALSE;

	JS_SetGCCallback(cx, prevThreadMemoryManagementGCCallback);

	jl_malloc = malloc;
	jl_calloc = calloc;
	jl_realloc = realloc;
	jl_free = free;

	pv->malloc = jl_malloc;
	pv->calloc = jl_calloc;
	pv->realloc = jl_realloc;
	pv->free = jl_free;

	JSLIBS_RegisterAllocFunctions(malloc, calloc, realloc, jl_free);

	threadAction = MemThreadExit;
	JLReleaseSemaphore(memoryFreeThreadSem);

	//	JLThreadCancel(memoryFreeThread); // cannot kill the thread while it is calling free() !
	JLWaitThread(memoryFreeThread);

	JLFreeSemaphore(&memoryFreeThreadSem);
	JLFreeThread(&memoryFreeThread);

	if ( !freeQueue )
		return JS_TRUE;

	FreeHead();
	if ( head )
		free(head);

	return JS_TRUE;
}

// DisabledFree
static void DisabledFree( void *ptr ) {
}

JSBool DisableMemoryFree( JSContext *cx ) {

	jl_free = DisabledFree;
	GetHostPrivate(cx)->free = jl_free;
	JSLIBS_RegisterAllocFunctions(malloc, calloc, realloc, jl_free);
	return JS_TRUE;
}
