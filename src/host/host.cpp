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
#include <string.h>

#ifdef XP_UNIX
#include <dlfcn.h> // cf. dlerror() in LoadModule()
#endif

#include <jsprf.h>
#include <jsstddef.h>

#include "moduleManager.h"

#include "../common/jsHelper.h"
#include "../common/jsNames.h"
#include "../common/jsConfiguration.h"
#include "../common/errors.h"

#include "../jslang/jslang.h"

#include "host.h"

JSErrorFormatString errorFormatString[J_ErrLimit] = {
#define MSG_DEF(name, number, count, exception, format) \
    { format, count, exception } ,
#include "../common/errors.msg"
#undef MSG_DEF
};

static const JSErrorFormatString *GetErrorMessage(void *userRef, const char *locale, const uintN errorNumber) {

	if ((errorNumber > 0) && (errorNumber < J_ErrLimit))
		return &errorFormatString[errorNumber];
	return NULL;
}


static const void *_pGetErrorMessage = (const void*)&GetErrorMessage; //global variable !

static bool _unsafeMode = false; // global variable !
extern bool *_pUnsafeMode = &_unsafeMode; // global variable !

static u_int32_t gBranchCount = 1; // global variable !

static HostOutput hostStdOut = NULL; // global variable !
static HostOutput hostStdErr = NULL; // global variable !


//#define RT_HOST_MAIN_ASSERT( condition, errorMessage )
//	if ( !(condition) ) { consoleStdErr( cx, errorMessage, sizeof(errorMessage)-1 ); return -1; }


static JSBool JSDefaultStdoutFunction(JSContext *cx, uintN argc, jsval *vp) {

	if ( hostStdOut == NULL )
		return JS_TRUE;
	const char *buffer;
	size_t length;
	for ( uintN i = 0; i < argc; i++ ) {

		J_CHK( JsvalToStringAndLength(cx, &J_FARG(i+1), &buffer, &length) );
		hostStdOut(buffer, length);
	}
	return JS_TRUE;
}


static JSBool JSDefaultStderrFunction(JSContext *cx, uintN argc, jsval *vp) {

	if ( hostStdErr == NULL )
		return JS_TRUE;
	const char *buffer;
	size_t length;
	for ( uintN i = 0; i < argc; i++ ) {

		J_CHK( JsvalToStringAndLength(cx, &J_FARG(i+1), &buffer, &length) );
		hostStdErr(buffer, length);
	}
	return JS_TRUE;
}


void stdErrRouter( JSContext *cx, const char *message, size_t length ) {

	JSObject *globalObject = JS_GetGlobalObject(cx);
	if ( globalObject != NULL ) {

		jsval fct;
		if ( GetConfigurationValue(cx, NAME_CONFIGURATION_STDERR, &fct) == JS_TRUE && JsvalIsFunction(cx, fct) ) {

			jsval rval, strVal;
			StringAndLengthToJsval(cx, &strVal, message, length);
			JS_CallFunctionValue(cx, globalObject, fct, 1, &strVal, &rval);
			return;
		}
	}

	if ( hostStdErr != NULL )
		hostStdErr(message, length); // else, use the default.
}


// function copied from ../js/src/js.c
static void ErrorReporter(JSContext *cx, const char *message, JSErrorReport *report) {

	int i, j, k, n;
	char *prefix, *tmp;
	const char *ctmp;
	char *msg;

    if (!report) {

		stdErrRouter( cx, message, strlen(message) );
		stdErrRouter( cx, "\n", 1 );
		return;
    }

	// check if we should report warnings
	if ( JSREPORT_IS_WARNING(report->flags) )
//		if ( GetConfigurationValue(cx, NAME_CONFIGURATION_UNSAFE_MODE ) == JSVAL_TRUE )
		if ( _unsafeMode )
			return;

//	if (JSREPORT_IS_EXCEPTION(report->flags))
//		return;

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
            stdErrRouter( cx, prefix, strlen(prefix) );
        stdErrRouter( cx, message, ctmp - message );
        message = ctmp;
    }

    /* If there were no filename or lineno, the prefix might be empty */
    if (prefix)
	    stdErrRouter( cx, prefix, strlen(prefix) );
    stdErrRouter( cx, message, strlen(message) );

    if (!report->linebuf) {
        stdErrRouter(cx, "\n", 1);
        goto out;
    }

    /* report->linebuf usually ends with a newline. */
    n = strlen(report->linebuf);
    msg = JS_smprintf(":\n%s%s%s%s",
            prefix,
            report->linebuf,
            (n > 0 && report->linebuf[n-1] == '\n') ? "" : "\n",
            prefix);
	 stdErrRouter( cx, msg, strlen(msg) );
    n = PTRDIFF(report->tokenptr, report->linebuf, char);
    for (i = j = 0; i < n; i++) {
        if (report->linebuf[i] == '\t') {
            for (k = (j + 8) & ~7; j < k; j++) {
                stdErrRouter( cx, ".", 1);
            }
            continue;
        }
        stdErrRouter( cx, ".", 1);
        j++;
    }
    stdErrRouter( cx, "^\n", 2);
 out:
//    if (!JSREPORT_IS_WARNING(report->flags))
//        gExitCode = EXITCODE_RUNTIME_ERROR;
    JS_free(cx, prefix);
}


static JSBool BranchCallback(JSContext *cx, JSScript *script) {

	if ((++gBranchCount & (0x1000-1)) != 1) // every 4096
		return JS_TRUE;
	JS_MaybeGC(cx);
	return JS_TRUE;
}


static JSBool LoadModule(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	J_S_ASSERT_ARG_MIN(1);
	const char *fileName;
	J_CHK( JsvalToString(cx, &argv[0], &fileName) );
	char libFileName[PATH_MAX];
	strcpy( libFileName, fileName );
	strcat( libFileName, DLL_EXT );
// MAC OSX: 	'@executable_path' ??
	ModuleId id = ModuleLoad(libFileName, cx, obj);

#ifdef XP_UNIX
	J_S_ASSERT_2( id != 0, "Unable to load the module \"%s\": %s", libFileName, dlerror() );
#else // XP_UNIX
	J_S_ASSERT_2( id != 0, "Unable to load the module \"%s\": %d", libFileName, GetLastError() );
#endif // XP_UNIX

	J_CHK( JS_NewNumberValue(cx, id, rval) ); // really needed ? yes, UnloadModule need this ID
	return JS_TRUE;
}


static JSBool UnloadModule(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	J_S_ASSERT_ARG_MIN(1);
	jsdouble dVal;
	J_CHK( JS_ValueToNumber(cx, argv[0], &dVal) );
	ModuleId id = (ModuleId)dVal;

	if ( ModuleIsUnloadable(id) ) {

		bool st = ModuleUnload(id, cx);
		J_S_ASSERT( st == true, "Unable to unload the module" );
		*rval = JSVAL_TRUE;
	} else {

		*rval = JSVAL_FALSE;
	}
	return JS_TRUE;
}


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
static JSClass global_class = { // global variable !

	NAME_GLOBAL_CLASS, JSCLASS_GLOBAL_FLAGS | JSCLASS_NEW_RESOLVE,
	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
	global_enumerate, (JSResolveOp)global_resolve, JS_ConvertStub, JS_FinalizeStub, // see LAZY_STANDARD_CLASSES
	JSCLASS_NO_OPTIONAL_MEMBERS
};


JSContext* CreateHost(size_t maxMem, size_t maxAlloc) {

//	JS_SetCStringsAreUTF8(); // don't use !
	JSRuntime *rt = JS_NewRuntime(0); // maxMem specifies the number of allocated bytes after which garbage collection is run.
//	J_CHKM( rt != NULL, "unable to create the runtime." );
	if ( rt == NULL )
		return NULL;

//call of  'js_malloc'  acts on  'runtime->gcMallocBytes'
//do gc IF rt->gcMallocBytes >= rt->gcMaxMallocBytes

	JS_SetGCParameter(rt, JSGC_MAX_BYTES, maxMem); /* maximum nominal heap before last ditch GC */
	JS_SetGCParameter(rt, JSGC_MAX_MALLOC_BYTES, maxAlloc); /* # of JS_malloc bytes before last ditch GC */

	JSContext *cx = JS_NewContext(rt, 8192L); // set the chunk size of the stack pool to 8192. see http://groups.google.com/group/mozilla.dev.tech.js-engine/browse_thread/thread/be9f404b623acf39/9efdfca81be99ca3
	if ( cx == NULL )
		return NULL; //, "unable to create the context." );

	// Info: Increasing JSContext stack size slows down my scripts:
	//   http://groups.google.com/group/mozilla.dev.tech.js-engine/browse_thread/thread/be9f404b623acf39/9efdfca81be99ca3

	JS_SetScriptStackQuota(cx, JS_DEFAULT_SCRIPT_STACK_QUOTA); // good place to manage stack limit ( that is 32MB by default )
	//	btw, JS_SetScriptStackQuota ( see also JS_SetThreadStackLimit )

	JS_SetVersion(cx, (JSVersion)JS_VERSION);

// error management
	JS_SetErrorReporter(cx, ErrorReporter);

	// language options
	// options
	//	uint32 options = JSOPTION_VAROBJFIX | JSOPTION_XML | JSOPTION_COMPILE_N_GO;
	//	if ( !unsafeMode )
	//		options |= JSOPTION_STRICT;
	//	JS_SetOptions(cx, options );

	JS_SetOptions(cx, JSOPTION_VAROBJFIX | JSOPTION_XML | JSOPTION_COMPILE_N_GO | JSOPTION_RELIMIT | JSOPTION_NATIVE_BRANCH_CALLBACK);
  // JSOPTION_COMPILE_N_GO:
	//  caller of JS_Compile*Script promises to execute compiled script once only; enables compile-time scope chain resolution of consts.
  // JSOPTION_DONT_REPORT_UNCAUGHT:
	//  When returning from the outermost API call, prevent uncaught exceptions from being converted to error reports
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

	// JSBranchCallback oldBranchCallback =
	JS_SetBranchCallback(cx, BranchCallback);

	JSObject *globalObject = JS_NewObject(cx, &global_class, NULL, NULL);
	if ( globalObject == NULL )
		return NULL; //, "unable to create the global object." );

	//	JS_SetGlobalObject(cx, globalObject); // not needed. Doc: As a side effect, JS_InitStandardClasses establishes obj as the global object for cx, if one is not already established.

// Standard classes
//	jsStatus = JS_InitStandardClasses(cx, globalObject); // use NULL instead of globalObject ?
//	RT_HOST_MAIN_ASSERT( jsStatus == JS_TRUE, "unable to initialize standard classes." );

	JS_SetGlobalObject(cx, globalObject); // see LAZY_STANDARD_CLASSES

	return cx;
}





JSBool InitHost( JSContext *cx, bool unsafeMode, HostOutput stdOut, HostOutput stdErr ) {

	_unsafeMode = unsafeMode;

	if ( _unsafeMode )
		JS_SetOptions(cx, JS_GetOptions(cx) | JSOPTION_STRICT);

	JSObject *globalObject = JS_GetGlobalObject(cx);
	J_CHKM( globalObject != NULL, "Global object not found." );

// make GetErrorMessage available from any module
	J_CHK( SetConfigurationPrivateValue(cx, NAME_CONFIGURATION_GETERRORMESSAGE, PRIVATE_TO_JSVAL(&_pGetErrorMessage)) );

// global functions & properties
	J_CHKM( JS_DefineProperty( cx, globalObject, NAME_GLOBAL_GLOBAL_OBJECT, OBJECT_TO_JSVAL(JS_GetGlobalObject(cx)), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT ), "unable to define a property." );
	J_CHKM( JS_DefineFunction( cx, globalObject, NAME_GLOBAL_FUNCTION_LOAD_MODULE, LoadModule, 0, 0 ), "unable to define a property." );
	J_CHKM( JS_DefineFunction( cx, globalObject, NAME_GLOBAL_FUNCTION_UNLOAD_MODULE, UnloadModule, 0, 0 ), "unable to define a property." );

//	J_CHK( SetConfigurationValue(cx, NAME_CONFIGURATION_UNSAFE_MODE, BOOLEAN_TO_JSVAL(_unsafeMode)) );
	J_CHK( SetConfigurationValue(cx, NAME_CONFIGURATION_UNSAFE_MODE_PTR, PRIVATE_TO_JSVAL(&_unsafeMode)) );

	hostStdOut = stdOut;
	hostStdErr = stdErr;

	jsval value;
	value = OBJECT_TO_JSVAL(JS_GetFunctionObject(JS_NewFunction(cx, (JSNative)JSDefaultStdoutFunction, 1, JSFUN_FAST_NATIVE, NULL, NULL))); // If you do not assign a name to the function, it is assigned the name "anonymous".
	J_CHK( SetConfigurationValue(cx, NAME_CONFIGURATION_STDOUT, value) );

	value = OBJECT_TO_JSVAL(JS_GetFunctionObject(JS_NewFunction(cx, (JSNative)JSDefaultStderrFunction, 1, JSFUN_FAST_NATIVE, NULL, NULL))); // If you do not assign a name to the function, it is assigned the name "anonymous".
	J_CHK( SetConfigurationValue(cx, NAME_CONFIGURATION_STDERR, value) );


// init static modules
	J_CHKM( jslangInit(cx, globalObject), "Unable to initialize jslang." );
	return JS_TRUE;
}


void DestroyHost( JSContext *cx ) {

	JSRuntime *rt = JS_GetRuntime(cx);

	ModuleReleaseAll(cx);

	//	don't try to break linked objects with JS_GC(cx)

	RemoveConfiguration(cx);

	JS_SetGlobalObject(cx, JSVAL_TO_OBJECT(JSVAL_NULL)); // remove the global object

// cleanup

// doc:
//  - Is the only side effect of JS_DestroyContextNoGC that any finalizers I may have specified in custom objects will not get called ? 
//  - Not if you destroy all contexts (whether by NoGC or not), destroy all runtimes, and call JS_ShutDown before exiting or hibernating.
//    The last JS_DestroyContext* API call will run a GC, no matter which API of that form you call on the last context in the runtime. /be
	JS_DestroyContext(cx);
	JS_DestroyRuntime(rt);
	JS_ShutDown();

// Beware: because JS engine allocate memory from the DLL, all memory must be disallocated before releasing the DLL
	ModuleFreeAll();
}


JSBool ExecuteScript( JSContext *cx, const char *scriptFileName, bool compileOnly, int argc, const char * const * argv, jsval *rval ) {

	JSObject *globalObject = JS_GetGlobalObject(cx);
	J_CHKM( globalObject != NULL, "Global object not found." );

// arguments
	JSObject *argsObj = JS_NewArrayObject(cx, 0, NULL);
	J_CHKM( argsObj != NULL, "unable to create argument array on the global object." );

	J_CHKM( JS_DefineProperty(cx, globalObject, NAME_GLOBAL_ARGUMENTS, OBJECT_TO_JSVAL(argsObj), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT), "unable to store the argument array." );

	for ( int index = 0; index < argc; index++ ) {

		JSString *str = JS_NewStringCopyZ(cx, argv[index]);
		J_CHKM( str != NULL, "unable to store the argument." );
		J_CHKM( JS_DefineElement(cx, argsObj, index, STRING_TO_JSVAL(str), NULL, NULL, JSPROP_ENUMERATE), "unable to define the argument." );
	}

// compile & executes the script
//	script = JS_CompileFile( cx, globalObject, scriptName );

// shebang support
	FILE *file = fopen(scriptFileName, "r");
	J_CHKM1( file != NULL, "Script %s file cannot be opened.", scriptFileName );

	char s = getc(file);
	char b = getc(file);
	if ( s == '#' && b == '!' ) {

		ungetc('/', file);
		ungetc('/', file);
	} else {

		ungetc(b, file);
		ungetc(s, file);
	}

//	JS_GC(cx); // ...and also just before doing anything that requires compilation (since compilation disables GC until complete).

	JSScript *script = JS_CompileFileHandle(cx, globalObject, scriptFileName, file);
	J_CHKM1( script != NULL, "Unable to compile the script %s.", scriptFileName );
	fclose(file);

	// You need to protect a JSScript (via a rooted script object) if and only if a garbage collection can occur between compilation and the start of execution.

	if ( !compileOnly ) {

		J_CHK( JS_ExecuteScript( cx, globalObject, script, rval ) ); // MUST be executed only once ( JSOPTION_COMPILE_N_GO )
	} else {

		*rval = JSVAL_VOID;
	}

	J_CHKM( JS_DeleteProperty(cx, globalObject, NAME_GLOBAL_ARGUMENTS ), "Unable to remove argument property" );
	return JS_TRUE;
}
