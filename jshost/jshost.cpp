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

#define NOATEXIT

#include "../common/platform.h"

#include <time.h>

#include <fcntl.h>
#ifdef XP_WIN
	#include <io.h>
#endif

#ifdef XP_UNIX
	#include <unistd.h>
	#include <dlfcn.h>
#endif //XP_UNIX

//#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
//	#include <dlfcn.h>

#include <jsapi.h>
#include "jsstddef.h"
#include "jsprf.h"
#include "jsscript.h"
#include "jscntxt.h"

#include "../common/jsNames.h"
#include "../common/jsHelper.h"
#include "../common/jsConfiguration.h"
#include "../moduleManager/moduleManager.h"

// to be used in the main() function only
#define RT_HOST_MAIN_ASSERT( condition, errorMessage ) \
	if ( !(condition) ) { consoleStdErr( cx, errorMessage, sizeof(errorMessage)-1 ); return -1; }


#ifdef XP_UNIX

void GetAbsoluteModulePath( char* moduleFileName, size_t size, char *modulePath ) {

	if ( modulePath[0] == PATH_SEPARATOR ) { //  /jshost

		strcpy(moduleFileName, modulePath);
		return;
	}

	if ( modulePath[0] == '.' && modulePath[1] == PATH_SEPARATOR ) { //  ./jshost

		getcwd(moduleFileName, size);
		strcat(moduleFileName, modulePath + 1 );
		return;
	}

	if ( modulePath[0] == '.' && modulePath[1] == '.' && modulePath[2] == PATH_SEPARATOR ) { //  ../jshost

		getcwd(moduleFileName, size);
		strcat(moduleFileName, PATH_SEPARATOR_STRING);
		strcat(moduleFileName, modulePath);
		return;
	}

	if ( strchr( modulePath, PATH_SEPARATOR ) != NULL ) { //  xxx/../jshost

		getcwd(moduleFileName, size);
		strcat(moduleFileName, PATH_SEPARATOR_STRING);
		strcat(moduleFileName, modulePath);
		return;
	}

	char *envPath = getenv("PATH");
	char *pos;

	do {

		pos = strchr( envPath, ':' );

		if ( envPath[0] == PATH_SEPARATOR ) {

			if ( pos == NULL ) {

				strcpy(moduleFileName, envPath);
			} else {

				strncpy(moduleFileName, envPath, pos-envPath);
				moduleFileName[pos-envPath] = '\0';
			}

			strcat(moduleFileName, PATH_SEPARATOR_STRING);
			strcat(moduleFileName, modulePath);

			if (access(moduleFileName, R_OK | X_OK ) == 0) // If the requested access is permitted, it returns 0.
				return;
		}

		envPath = pos+1;

	} while (pos != NULL);

	moduleFileName[0] = '\0';
	return;

}

#endif //XP_UNIX


int consoleStdOut( JSContext *, const char *data, int length ) {

	return write( 1, data, length );
}

int consoleStdErr( JSContext *, const char *data, int length ) {

	return write( 2, data, length );
}


bool reportWarnings = true;
bool debugTraces = false;


// function copied from ../js/src/js.c
static void ErrorReporter(JSContext *cx, const char *message, JSErrorReport *report) {

    int i, j, k, n;
    char *prefix, *tmp;
    const char *ctmp;
	char *msg;

    if (!report) {

		consoleStdOut( cx, message, strlen(message) );
		consoleStdOut( cx, "\n", 1 );
		return;
    }

	// Conditionally ignore reported warnings.
	if (JSREPORT_IS_WARNING(report->flags) && !reportWarnings)
		return;

//	if (JSREPORT_IS_EXCEPTION(report->flags) && !reportWarnings)
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
            consoleStdErr( cx, prefix, strlen(prefix) );
        consoleStdErr( cx, message, ctmp - message );
        message = ctmp;
    }

    /* If there were no filename or lineno, the prefix might be empty */
    if (prefix)
	    consoleStdErr( cx, prefix, strlen(prefix) );
    consoleStdErr( cx, message, strlen(message) );

    if (!report->linebuf) {
        consoleStdErr(cx, "\n", 1);
        goto out;
    }

    /* report->linebuf usually ends with a newline. */
    n = strlen(report->linebuf);
    msg = JS_smprintf(":\n%s%s%s%s",
            prefix,
            report->linebuf,
            (n > 0 && report->linebuf[n-1] == '\n') ? "" : "\n",
            prefix);
	 consoleStdErr( cx, msg, strlen(msg) );
    n = PTRDIFF(report->tokenptr, report->linebuf, char);
    for (i = j = 0; i < n; i++) {
        if (report->linebuf[i] == '\t') {
            for (k = (j + 8) & ~7; j < k; j++) {
                consoleStdErr( cx, ".", 1);
            }
            continue;
        }
        consoleStdErr( cx, ".", 1);
        j++;
    }
    consoleStdErr( cx, "^\n", 2);
 out:
//    if (!JSREPORT_IS_WARNING(report->flags))
//        gExitCode = EXITCODE_RUNTIME_ERROR;
    JS_free(cx, prefix);
}

/*
static void LoadErrorReporter(JSContext *cx, const char *message, JSErrorReport *report) {
    if (!report) {
        fprintf(gErrFile, "%s\n", message);
        return;
    }

    // Ignore any exceptions
    if (JSREPORT_IS_EXCEPTION(report->flags))
        return;

    // Otherwise, fall back to the ordinary error reporter.
    ErrorReporter(cx, message, report);
}
*/



/*
// function copied from mozilla/js/src/js.c
static uint32 gBranchLimit = 1000000;
static uint32 gBranchCount;
static JSBool BranchCallback(JSContext *cx, JSScript *script) {

	if (++gBranchCount == gBranchLimit) {

		char *msg;
		if (script) {

			if (script->filename) {

				msg = JS_smprintf("%s:", script->filename);
				consoleStdErr(cx, msg, strlen(msg));
			}
			msg = JS_smprintf("%u: script branch callback (%u callbacks)\n", script->lineno, gBranchLimit);
			consoleStdErr(cx, msg, strlen(msg));
		} else {

			msg = JS_smprintf("native branch callback (%u callbacks)\n", gBranchLimit);
			consoleStdErr(cx, msg, strlen(msg));
		}
		gBranchCount = 0;
		return JS_FALSE;
	}

	if ((gBranchCount & 0x3fff) == 1)
		JS_MaybeGC(cx);
	return JS_TRUE;
}
*/



static JSBool LoadModule(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT_ARGC(1);

	char *fileName;
	RT_JSVAL_TO_STRING( argv[0], fileName );
	char libFileName[MAX_PATH];
	strcpy( libFileName, fileName );
	strcat( libFileName, DLL_EXT );

// MAC OSX: 	'@executable_path' ??
//	if ( !ModuleIsLoaded( libFileName ) ) {

		ModuleId id = ModuleLoad(libFileName, cx, obj);

#ifdef XP_UNIX

	RT_ASSERT_2( id != 0, "Unable to load the module \"%s\": %s", libFileName, dlerror() );
#else // XP_UNIX

		RT_ASSERT_1( id != 0, "Unable to load the module \"%s\".", libFileName );
#endif // XP_UNIX

	//	RT_ASSERT_2( id != 0, "Unable to load the module %s (error:%d).", libFileName, GetLastError() ); // (TBD) rewrite this for Linux
	//	RT_ASSERT_2( id != 0, "Unable to load the module %s (error:%s).", libFileName, dlerror() );
		RT_CHECK_CALL( JS_NewNumberValue(cx, id, rval) ); // (TBD) really needed ? yes, UnloadModule need this ID
//	} else { // module already loaded
//	}
//	*rval = JSVAL_TRUE;
	return JS_TRUE;
}


static JSBool UnloadModule(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT_ARGC(1);
	jsdouble dVal;
	RT_CHECK_CALL( JS_ValueToNumber(cx, argv[0], &dVal) );
	ModuleId id = (ModuleId)dVal;

	if ( ModuleIsUnloadable(id) ) {

		bool st = ModuleUnload(id, cx);
		RT_ASSERT( st == true, "Unable to unload the module" );
		*rval = JSVAL_TRUE;
	} else {
		*rval = JSVAL_FALSE;
	}
	return JS_TRUE;
}


	//var str = "12345";
	//print(str); // -> 12345
	//test( str );
	//print(str); // -> 1X345

//static JSBool global_test(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
//
//	JSString *jssMesage = JS_ValueToString(cx, argv[0]);
//	JS_GetStringBytes(jssMesage)[1] = 'X';
//
//	return JS_TRUE;
//}



bool gEndSignal = false;

JSBool EndSignalGetter(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	*vp = BOOLEAN_TO_JSVAL( gEndSignal );
	return JS_TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////////
JSPropertySpec Global_PropertySpec[] = { // *name, tinyid, flags, getter, setter
	{ "endSignal"    , 0    , JSPROP_SHARED | JSPROP_READONLY | JSPROP_PERMANENT, EndSignalGetter, NULL },
	{ 0 }
};

//////////////////////////////////////////////////////////////////////////////////////////////

#ifdef XP_WIN
BOOL Interrupt(DWORD CtrlType) {

	if (CtrlType == CTRL_LOGOFF_EVENT || CtrlType == CTRL_SHUTDOWN_EVENT)
		return FALSE;
	gEndSignal = true;
  return TRUE;
}
#else
void Interrupt(int CtrlType) {

	gEndSignal = true;
}
#endif // XP_WIN


static JSBool stderrFunction(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	JSString *str;
	str = JS_ValueToString(cx, argv[0]);
	RT_ASSERT( str != NULL, "Unable to convert argument to string.");
	argv[0] = STRING_TO_JSVAL(str); // (TBD) needed ? YES
	consoleStdErr( cx, JS_GetStringBytes(str), JS_GetStringLength(str) );
	return JS_TRUE;
}

static JSBool stdoutFunction(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	JSString *str;
	str = JS_ValueToString(cx, argv[0]);
	RT_ASSERT( str != NULL, "Unable to convert argument to string.");
	argv[0] = STRING_TO_JSVAL(str); // (TBD) needed ? YES
	consoleStdOut( cx, JS_GetStringBytes(str), JS_GetStringLength(str) );
	return JS_TRUE;
}

JSBool GCCallTrace(JSContext *cx, JSGCStatus status) {

	char *statusStr[4] = { "JSGC_BEGIN", "JSGC_END", "JSGC_MARK_END", "JSGC_FINALIZE_END" };
	if ( status == JSGC_BEGIN || status == JSGC_END ) {

		tm time;
		getsystime(&time);

		char timeTmp[256];
		strftime( timeTmp, sizeof(timeTmp), "%m.%d %H:%M:%S", &time);
		
		char tmp[256];
		int len = sprintf(tmp, "## %s %s gcByte:%u\n", timeTmp, statusStr[status], cx->runtime->gcBytes );
		consoleStdErr( cx, tmp, len );
	}
	return JS_TRUE;
}


JSScript *script = NULL;
JSRuntime *rt = NULL;
JSContext *cx = NULL;

//bool _finalized = false;
void Finalize(void);

int main(int argc, char* argv[]) { // check int _tmain(int argc, _TCHAR* argv[]) for UNICODE

	JSBool unsafeMode = JS_FALSE;
	JSObject *globalObject;

//	unsigned long maxbytes = 64L * 1024L * 1024L;
	
	uint32 maxMem = (uint32)-1; // by default, there are no limit
	uint32 maxAlloc = (uint32)-1; // by default, there are no limit

	char** argumentVector = argv;

	for ( argumentVector++; argumentVector[0] && argumentVector[0][0] == '-'; argumentVector++ )
		switch ( argumentVector[0][1] ) {
			case 'm': // maxbytes (GC)
				argumentVector++;
				maxMem = atol( *argumentVector ) * 1024L * 1024L;
				break;
			case 'n': // maxAlloc (GC)
				argumentVector++;
				maxAlloc = atol( *argumentVector ) * 1024L * 1024L;
				break;
			case 'u': // avoid any runtime checks
				argumentVector++;
				unsafeMode = ( atoi( *argumentVector ) != 0 );
				reportWarnings = !unsafeMode;
				// (TBD) set into configuration
				break;
			case 'd': // debugTraces
				debugTraces = true;
				break;
	}

	rt = JS_NewRuntime(0); // maxMem specifies the number of allocated bytes after which garbage collection is run.
	RT_HOST_MAIN_ASSERT( rt != NULL, "unable to create the runtime." ); // (TBD) fix Warning: uninitialized local variable 'cx'


//call of  'js_malloc'  acts on  'runtime->gcMallocBytes'
//do gc IF rt->gcMallocBytes >= rt->gcMaxMallocBytes

	JS_SetGCParameter(rt, JSGC_MAX_BYTES, maxMem); /* maximum nominal heap before last ditch GC */
	JS_SetGCParameter(rt, JSGC_MAX_MALLOC_BYTES, maxAlloc); /* # of JS_malloc bytes before last ditch GC */

	cx = JS_NewContext(rt, 8192L); // http://groups.google.com/group/mozilla.dev.tech.js-engine/browse_thread/thread/be9f404b623acf39/9efdfca81be99ca3
	RT_HOST_MAIN_ASSERT( cx != NULL, "unable to create the context." );

	JS_SetVersion( cx, (JSVersion)JS_VERSION );
	// (TBD) set into configuration file

	// good place to manage stack limit ( that is 32MB by default ):
	//JS_SetScriptStackQuota( cx, JS_DEFAULT_SCRIPT_STACK_QUOTA );
	//	btw, JS_SetScriptStackQuota ( see also JS_SetThreadStackLimit )

// error management
	JS_SetErrorReporter(cx, ErrorReporter);

	if ( debugTraces ) {
		
		JS_SetGCCallbackRT(rt, GCCallTrace);
	}


#ifdef XP_WIN
	BOOL status = SetConsoleCtrlHandler((PHANDLER_ROUTINE)&Interrupt, TRUE);
	RT_HOST_MAIN_ASSERT( status == TRUE, "Unable to set console handler" );
#else
	signal(SIGINT,Interrupt);
	signal(SIGTERM,Interrupt);
#endif // XP_WIN


#ifdef JS_THREADSAFE
    JS_BeginRequest(cx);
#endif

	JSBool jsStatus;
// global object
	JSClass global_class = { NAME_GLOBAL_CLASS, 0, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub };

	globalObject = JS_NewObject(cx, &global_class, NULL, NULL);
	RT_HOST_MAIN_ASSERT( globalObject != NULL, "unable to create the global object." );

// Standard classes
	jsStatus = JS_InitStandardClasses(cx, globalObject); // use NULL instead of globalObject ?
	RT_HOST_MAIN_ASSERT( jsStatus == JS_TRUE, "unable to initialize standard classes." );

// global functions & properties
	JS_DefineProperty(cx, globalObject, NAME_GLOBAL_GLOBAL_OBJECT, OBJECT_TO_JSVAL(JS_GetGlobalObject(cx)), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT );
	JS_DefineProperties( cx, globalObject, Global_PropertySpec );
	JS_DefineFunction( cx, globalObject, NAME_GLOBAL_FUNCTION_LOAD_MODULE, LoadModule, 0, 0 );
	JS_DefineFunction( cx, globalObject, NAME_GLOBAL_FUNCTION_UNLOAD_MODULE, UnloadModule, 0, 0 );
//	JS_DefineFunction( cx, globalObject, "test", global_test, 0, 0 );
// JS_DefineFunction( cx, globalObject, "Module", _Module, 0, 0 );

// Global configuration object
	JSObject *configObject = GetConfigurationObject(cx);
	RT_HOST_MAIN_ASSERT( configObject != NULL, "failed to get/create configuration object." );

	jsval value = OBJECT_TO_JSVAL(JS_GetFunctionObject(JS_NewFunction(cx, stderrFunction, 1, 0, NULL, NULL))); // If you do not assign a name to the function, it is assigned the name "anonymous".
	jsStatus = JS_SetProperty(cx, configObject, NAME_CONFIGURATION_STDERR, &value);
	RT_HOST_MAIN_ASSERT( jsStatus != JS_FALSE, "Unable to set store stderr into configuration." );

	value = OBJECT_TO_JSVAL(JS_GetFunctionObject(JS_NewFunction(cx, stdoutFunction, 1, 0, NULL, NULL))); // If you do not assign a name to the function, it is assigned the name "anonymous".
	jsStatus = JS_SetProperty(cx, configObject, NAME_CONFIGURATION_STDOUT, &value);
	RT_HOST_MAIN_ASSERT( jsStatus != JS_FALSE, "Unable to set store stdout into configuration." );

	value = BOOLEAN_TO_JSVAL(unsafeMode);
	jsStatus = JS_SetProperty(cx, configObject, NAME_CONFIGURATION_UNSAFE_MODE, &value);
	RT_HOST_MAIN_ASSERT( jsStatus != JS_FALSE, "Unable to set store unsafeMode into configuration." );

// script name
  const char *scriptName = *argumentVector;
  RT_HOST_MAIN_ASSERT( scriptName != NULL, "no script specified" );

// arguments
	JSObject *argsObj = JS_NewArrayObject(cx, 0, NULL);
	RT_HOST_MAIN_ASSERT( argsObj != NULL, "unable to create argument array." );

	jsStatus = JS_DefineProperty(cx, globalObject, NAME_GLOBAL_ARGUMENTS, OBJECT_TO_JSVAL(argsObj), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
	RT_HOST_MAIN_ASSERT( jsStatus == JS_TRUE, "unable to store the argument array." );
	int index = 0;
	for ( char** argumentVectorit = argumentVector; *argumentVectorit; argumentVectorit++ ) {

		JSString *str = JS_NewStringCopyZ(cx, *argumentVectorit);
		RT_HOST_MAIN_ASSERT( str != NULL, "unable to store the argument." );
		jsStatus = JS_DefineElement(cx, argsObj, index++, STRING_TO_JSVAL(str), NULL, NULL, JSPROP_ENUMERATE);
		RT_HOST_MAIN_ASSERT( jsStatus == JS_TRUE, "unable to define the argument." );
	}

	char hostFullPath[MAX_PATH +1];

#ifdef XP_WIN
// get hostpath and hostname
	HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(NULL);
	DWORD len = GetModuleFileName(hInstance, hostFullPath, sizeof(hostFullPath));
	RT_HOST_MAIN_ASSERT( len != 0, "unable to GetModuleFileName." );
#else // XP_WIN

	GetAbsoluteModulePath(hostFullPath, sizeof(hostFullPath), argv[0]);
	RT_HOST_MAIN_ASSERT( hostFullPath[0] != '\0', "unable to get module FileName." );

//	int len = readlink("/proc/self/exe", moduleFileName, sizeof(moduleFileName)); // doc: readlink does not append a NUL character to buf.
//	moduleFileName[len] = '\0';

//	strcpy(hostFullPath, argv[0]);

#endif // XP_WIN

	char *hostPath, *hostName;
	hostName = strrchr( hostFullPath, PATH_SEPARATOR );
	if ( hostName != NULL ) {

		*hostName = '\0';
		hostName++;
		hostPath = hostFullPath;
	} else {

		hostPath = ".";
		hostName = hostFullPath;
	}

//	RT_HOST_MAIN_ASSERT( name != NULL, "unable to get module FileName." );
	JS_DefineProperty(cx, globalObject, NAME_GLOBAL_SCRIPT_HOST_PATH, STRING_TO_JSVAL(JS_NewStringCopyZ(cx, hostPath)), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineProperty(cx, globalObject, NAME_GLOBAL_SCRIPT_HOST_NAME, STRING_TO_JSVAL(JS_NewStringCopyZ(cx, hostName)), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);

// language options
// options
	uint32 options = JSOPTION_VAROBJFIX | JSOPTION_XML | JSOPTION_COMPILE_N_GO;
	if ( !unsafeMode )
		options |= JSOPTION_STRICT;
	JS_SetOptions(cx, options );

	JS_SetOptions(cx, JSOPTION_VAROBJFIX | JSOPTION_STRICT | JSOPTION_XML | JSOPTION_COMPILE_N_GO );
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


/*

//	gBranchLimit =
	if ( !unsafeMode ) {

		//JSBranchCallback oldBranchCallback = JS_SetBranchCallback(cx, BranchCallback);
		JS_SetBranchCallback(cx, BranchCallback);
		JS_ToggleOptions(cx, JSOPTION_NATIVE_BRANCH_CALLBACK);
	}
*/

// compile & executes the script


//	script = JS_CompileFile( cx, globalObject, scriptName );

// shebang support
	FILE *file = fopen(scriptName, "r");
	RT_HOST_MAIN_ASSERT( file != NULL, "Script file cannot be opened." );

	char s = getc(file);
	char b = getc(file);
	if ( s == '#' && b == '!' ) {

		ungetc('/', file);
		ungetc('/', file);
	} else {

		ungetc(b, file);
		ungetc(s, file);
	}
	JS_GC(cx); // ...and also just before doing anything that requires compilation (since compilation disables GC until complete).
	script = JS_CompileFileHandle(cx, globalObject, scriptName, file);
	// (TBD) fclose(file); ??

//	JS_AddRoot(cx, &script);

//  JSScript *script = LoadScript( cx, globalObject, scriptName, saveCompiledScripts );
	RT_HOST_MAIN_ASSERT( script != NULL, "unable to compile the script." );

#ifndef NOATEXIT
	int atexitStatus = atexit(Finalize); // returns the value 0 if successful; otherwise the value -1 is returned and the global variable errno is set to indicate the error.
	RT_HOST_MAIN_ASSERT( atexitStatus == 0, "unable to setup exit." );
#endif // NOATEXIT

	// You need to protect a JSScript (via a rooted script object) if and only if a garbage collection can occur between compilation and the start of execution.
	jsval rval;
	jsStatus = JS_ExecuteScript( cx, globalObject, script, &rval ); // MUST be executed only once ( JSOPTION_COMPILE_N_GO )

	// doc: If a script executes successfully, JS_ExecuteScript returns JS_TRUE. Otherwise it returns JS_FALSE. On failure, your application should assume that rval is undefined.
	// if jsStatus != JS_TRUE, an error has been throw while the execution, so there is no need to throw another error

//		printf( "Last executed line %s:%d", script->filename, JS_PCToLineNumber( cx, script, script->code ) ); // if it right ?
// (TBD) enhance this

  int exitValue;
  if ( jsStatus == JS_TRUE )
	  if ( JSVAL_IS_INT(rval) && JSVAL_TO_INT(rval) >= 0 )
		  exitValue = JSVAL_TO_INT(rval);
	  else
		  exitValue = EXIT_SUCCESS;
  else
	  exitValue = EXIT_FAILURE;

//#ifndef NOATEXIT
//	Finalize();
//	return exitValue;
//#else
	exit(exitValue);
//#endif // NOATEXIT

}


void Finalize() { // called by the system on exit(), or at the end of the main.

//	if ( _finalized )
//		return;

	// because atexit(Terminate); is  set just before the JS_ExecuteScript, rt, cx, script ARE defined.

#ifdef JS_THREADSAFE
    JS_EndRequest(cx);
#endif

//	JS_RemoveRoot(cx, &script);

	if ( script != NULL )
		JS_DestroyScript(cx, script);

//  printf("script result: %s\n", JS_GetStringBytes(JS_ValueToString(cx, rval)));

	ModuleReleaseAll(cx);

//	JS_GC(cx); // try to break linked objects
// (TBD) don't

// cleanup
	// For each context you've created
	JS_DestroyContext(cx); // (TBD) is JS_DestroyContextNoGC faster ?

	// For each runtime
	JS_DestroyRuntime(rt);

	// And finally
	JS_ShutDown();

// Beware: because JS engine allocate memory from the DLL, all memory must be disallocated before releasing the DLL
	// free used modules
	ModuleFreeAll();
	// (TBD) make rt, cx, script, ... global and finish them


#ifdef XP_WIN
	BOOL status = SetConsoleCtrlHandler((PHANDLER_ROUTINE)&Interrupt, FALSE);
//	RT_HOST_MAIN_ASSERT( status == TRUE, "Unable to remove console handler" );
#else
	signal(SIGINT, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
#endif // XP_WIN

//	_finalized = true;
}




/*
jshost VERSION:
--------------
	0.1


README:
------
	jshost is strongly based on the Spidermonkey interactive shell : mozilla/js/src/js.c


Spidermonkey configuration:
--------------------------
	I have enabled XDR api by modifying mozilla/js/src/config.h
		#define JS_HAS_XDR_FREEZE_THAW  1

	"If you are on windows make sure you also define JS_USE_ONLY_NSPR_LOCKS in your js builds. That solved a lock hang problem for me."
		cf. http://groups.google.fr/group/mozilla.dev.tech.js-engine/browse_thread/thread/c59a6b91bd072c1e


Spidermonkey compilation:
------------------------
	call "C:\Program Files\Microsoft Platform SDK\SetEnv.Cmd" /XP32 /RETAIL
	call "C:\Program Files\Microsoft Visual Studio 8\VC\vcvarsall.bat" x86
	set path=%path%;C:\tools\cygwin\bin
	cd .\mozilla\js\src
	make -f Makefile.ref clean all BUILD_OPT=1 XCFLAGS=/MT


Directories structure:
---------------------
	.
	..
	jshost <--
	libfile
	libffi
	libjsni
	libsocket
	mozilla
		js
			src


jhost documentation:
-------------------
  Command-line switchs:
		-w <1 or 0> : enable/disable warning reporting
		-m <number> : allocated bytes after which garbage collection is run
		-s <number> : stack size
		-c <1 or 0> : enable/disable compiled scripts to saved on disk
		-u <1 or 0> : enable/disable unsafe mode. Some logical checks are no more done. Use this switch carefully !

	Global object
		.seal(object,deep) : set <object> read-only, <deep> set sub-objects too
		.exec(file) : load and executes <file> ( see -c switch )
		.print(string, ...) : display strings on the standard output (stdout)
		.collectGarbage() : force the garbage collector to run
		.warning(string) : retport the warning <string> to strerr ( see -w switch )


Spidermonkey API DOC reminder:
-----------------------------
  JS_SetPendingException
	  ???
  JS_SetBranchCallback
	  specifies a callback function that is automatically called when a script branches backward during execution, when a function returns,
	  and at the end of the script. One typical use for a callback is in a client application to enable a user to abort an operation.
	JSFunction * JS_DefineFunction(JSContext *cx, JSObject *obj, const char *name, JSNative call, uintN nargs, uintN flags);
		nargs indicates the number of arguments the function expects to receive. JS uses this information to allocate storage space for each argument.

FAQ
---
What's the nargs member of JSFunctionSpec actually used for?
	It's not minimum requirement in the	report-an-error/throw-an-exception-if-fewer-actuals sense.  It's the minimum the callee can count on dereferencing with non-negative indexes via argumentVector.
	If fewer actuals are passed, the engine will push undefined until nargs arguments are available.
	...
	Just because argc reflects the actual parameter count does not mean that you cannot dereference argumentVector[argc] or argumentVector[argc+1] safely -- you can, all the way up to argumentVector[NARGS-1],
	for the value of NARGS you stored in the JSFunctionSpec.nargs initializer, or passed to JS_DefineFunction.
  ...
	Ah, I think I understand now. I can specify 0 for nargs for *all* functions just as long as I check argc's value before dereferencing argumentVector[] elements.
	But if I want guarantee that argumentVector[0], as an example, can be dereferenced (regardless of the number of arguments actually passed), then I need to specify an nargs value of 1 (or higher). Right?


Useful Links:
------------
  jshost web sites:
		http://soubok.googlepages.com/javascript
		http://code.google.com/p/jshost/
  Spidermonkey Web Site
    http://www.mozilla.org/js/spidermonkey/
  Spidermonkey bonsai
    http://bonsai.mozilla.org/rview.cgi?dir=mozilla/js/src&cvsroot=/cvsroot&module=default
  Spidermonkey release-notes
    http://www.mozilla.org/js/spidermonkey/release-notes/
  Spidermonkey API DOC
    http://www.mozilla.org/js/spidermonkey/apidoc/complete-frameset.html
    http://www.sterlingbates.com/jsref/sparse-frameset.html
  socket doc
    http://www.synchro.net/
  tutorial
    http://egachine.berlios.de/embedding-sm-best-practice/embedding-sm-best-practice.html
    http://egachine.berlios.de/embedding-sm-best-practice/
  xpconnect
    http://lxr.mozilla.org/seamonkey/source/js/src/xpconnect/src/xpcwrappednativejsops.cpp
  ???
    http://users.skynet.be/saw/SpiderMonkey.htm
  WXjs
    http://wxjs.sourceforge.net/
  JSFILE
    http://www.mozilla.org/js/js-file-object.html#playwithfire
  NSPR 4.6 - src/bin
    ftp://ftp.mozilla.org/pub/mozilla.org/nspr/releases/v4.6

	Dynamic-Link Library Search Order
		http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dllproc/base/dynamic-link_library_search_order.asp

	Dynamic-Link Library Redirection
		http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dllproc/base/dynamic_link_library_redirection.asp

*/
