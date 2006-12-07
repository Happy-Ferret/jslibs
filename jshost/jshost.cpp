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

#ifdef WIN32
 #define DLL_EXT ".dll"
#else
 #define DLL_EXT ".so"
#endif

#include "jsstddef.h"

#include <jsapi.h>
#include "jsprf.h"
#include "jsscript.h"

// #include <jsdbgapi.h>
// #include <jscntxt.h>
// #include <jsscript.h>

#include "../common/jshelper.h"
#include "../configuration/configuration.h"

// to be used in the main() function only
#define RT_HOST_MAIN_ASSERT( condition, errorMessage ) \
	if ( !(condition) ) { consoleStdErr( errorMessage, sizeof(errorMessage)-1 ); return -1; }

//#include "../objectEx/objetEx.h"

HMODULE _moduleList[32] = {NULL}; // do not manage the module list dynamicaly, we allow a maximum of 32 modules
//JSBool _Module(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

JSBool unsafeMode = JS_FALSE;

int consoleStdOut( const char *data, int length ) {

	return fwrite( data, 1, length, stdout );
}

int consoleStdErr( const char *data, int length ) {

	return fwrite( data, 1, length, stderr );
}

// function copied from mozilla/js/src/js.c
/*
static uint32 gBranchCount;
static uint32 gBranchLimit;

static JSBool BranchCallback(JSContext *cx, JSScript *script)
{
    if (++gBranchCount == gBranchLimit) {
        if (script) {
            if (script->filename)
                fprintf(gErrFile, "%s:", script->filename);
            fprintf(gErrFile, "%u: script branch callback (%u callbacks)\n",
                    script->lineno, gBranchLimit);
        } else {
            fprintf(gErrFile, "native branch callback (%u callbacks)\n",
                    gBranchLimit);
        }
        gBranchCount = 0;
        return JS_FALSE;
    }
    if ((gBranchCount & 0x3fff) == 1)
        JS_MaybeGC(cx);
    return JS_TRUE;
}
*/


bool reportWarnings = true;

// function copied from mozilla/js/src/js.c
static void ErrorReporter(JSContext *cx, const char *message, JSErrorReport *report) {

    int i, j, k, n;
    char *prefix, *tmp;
    const char *ctmp;

    if (!report) {

		consoleStdOut( message, strlen(message) );
		consoleStdOut( "\n", 1 );
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
            consoleStdErr( prefix, strlen(prefix) );
        consoleStdErr(message, ctmp - message );
        message = ctmp;
    }

    /* If there were no filename or lineno, the prefix might be empty */
    if (prefix)
	    consoleStdErr( prefix, strlen(prefix) );
    consoleStdErr(message, strlen(message) );

    if (!report->linebuf) {
        consoleStdErr("\n", 1);
        goto out;
    }

    /* report->linebuf usually ends with a newline. */
    n = strlen(report->linebuf);
    char *msg = JS_smprintf(":\n%s%s%s%s",
            prefix,
            report->linebuf,
            (n > 0 && report->linebuf[n-1] == '\n') ? "" : "\n",
            prefix);
	 consoleStdErr( msg, strlen(msg) );
    n = PTRDIFF(report->tokenptr, report->linebuf, char);
    for (i = j = 0; i < n; i++) {
        if (report->linebuf[i] == '\t') {
            for (k = (j + 8) & ~7; j < k; j++) {
                consoleStdErr(".", 1);
            }
            continue;
        }
        consoleStdErr(".", 1);
        j++;
    }
    consoleStdErr("^\n", 2);
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



static JSBool global_loadModule(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT_ARGC(1);
	char *fileName;
	RT_JSVAL_TO_STRING( argv[0], fileName );
	char libFileName[MAX_PATH];
	strcpy( libFileName, fileName );
	strcat( libFileName, DLL_EXT );
	HMODULE module = ::LoadLibrary(libFileName);
	RT_ASSERT_2( module != NULL, "Unable to load the library %s (error:%d).", libFileName, GetLastError() );
	int i;
	for ( i = 0; _moduleList[i]; ++i ); // find a free module slot
	RT_ASSERT( i < 32, "unable to load more libraries" );
	_moduleList[i] = module;
	typedef JSBool (*ModuleInitFunction)(JSContext *, JSObject *);
	ModuleInitFunction moduleInit = (ModuleInitFunction)::GetProcAddress( module, (argc>1) ? JS_GetStringBytes(JS_ValueToString(cx, argv[1])) : "ModuleInit" );
	RT_ASSERT_1( moduleInit != NULL, "Module initialization function not found in %s.", libFileName );
	*rval = moduleInit( cx, obj ) == JS_TRUE ? JSVAL_TRUE : JSVAL_FALSE;
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

JSBool global_getter_endSignal(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	*vp = BOOLEAN_TO_JSVAL( gEndSignal );
  return JS_TRUE;
}


//////////////////////////////////////////////////////////////////////////////////////////////
JSPropertySpec Global_PropertySpec[] = { // *name, tinyid, flags, getter, setter
	{ "endSignal"    , 0    , JSPROP_SHARED | JSPROP_READONLY | JSPROP_PERMANENT, global_getter_endSignal, NULL },
	{ 0 }
};

//////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _CONSOLE
#ifdef WIN32

BOOL Interrupt(DWORD CtrlType) {

	if (CtrlType == CTRL_LOGOFF_EVENT || CtrlType == CTRL_SHUTDOWN_EVENT)
		return FALSE;
	gEndSignal = true;
  return TRUE;
}

#endif // WIN32
#endif // _CONSOLE


static JSBool stdoutFunction(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	JSString *str;
	str = JS_ValueToString(cx, argv[0]);
	RT_ASSERT( str != NULL, "Unable to convert argument to string.");
	argv[0] = STRING_TO_JSVAL(str); // [TBD] needed ?
	consoleStdOut( JS_GetStringBytes(str), JS_GetStringLength(str) );
	return JS_TRUE;
}


int main(int argc, char* argv[]) { // check int _tmain(int argc, _TCHAR* argv[]) for UNICODE

	JSRuntime *rt;
	JSContext *cx;
	JSObject *globalObject;

	char* scriptName;

	unsigned long maxbytes = 32L * 1024L * 1024L;
	unsigned long stackSize = 1L * 1024L * 1024L;


	for ( argv++; argv[0] && argv[0][0] == '-'; argv++ )
		switch ( argv[0][1] ) {
			case 'w': // report warnings
				argv++;
				// [TBD] set into configuration
				break;
			case 'm': // maxbytes (GC)
				argv++;
				maxbytes = atol( *argv );
				break;
			case 's': // stackSize
				argv++;
				stackSize = atol( *argv );
				break;
/*
			case 'c': // save compiled scripts
				arg++;
				saveCompiledScripts = ( atoi( *arg ) != 0 );
				break;
*/
			case 'u': // avoid any runtime checks
				argv++;
				unsafeMode = ( atoi( *argv ) != 0 );
				// [TBD] set into configuration
				break;
	}


#ifdef _CONSOLE
#ifdef WIN32
	BOOL status = SetConsoleCtrlHandler((PHANDLER_ROUTINE)&Interrupt, true);
	RT_HOST_MAIN_ASSERT( status == TRUE, "Unable to set console handler" );
#endif // WIN32
#endif // _CONSOLE

	rt = JS_NewRuntime(maxbytes); // maxbytes specifies the number of allocated bytes after which garbage collection is run.
	RT_HOST_MAIN_ASSERT( rt != NULL, "unable to create the runtime." );
	cx = JS_NewContext(rt, stackSize); // A context specifies a stack size for the script, the amount, in bytes, of private memory to allocate to the execution stack for the script.
	RT_HOST_MAIN_ASSERT( cx != NULL, "unable to create the context." );

	JS_SetVersion( cx, JSVERSION_1_7 );
	//	[TBD] set into configuration file

// error management
	JS_SetErrorReporter(cx, ErrorReporter);

#ifdef JS_THREADSAFE
    JS_BeginRequest(cx);
#endif

	JSBool jsStatus;
// global object
	JSClass global_class = { "global", 0, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub };

	globalObject = JS_NewObject(cx, &global_class, NULL, NULL);
	RT_HOST_MAIN_ASSERT( globalObject != NULL, "unable to create the global object." );

// Standard classes
	jsStatus = JS_InitStandardClasses(cx, globalObject);
	RT_HOST_MAIN_ASSERT( jsStatus == JS_TRUE, "unable to initialize standard classes." );

// global functions & properties
	JS_DefineProperties( cx, globalObject, Global_PropertySpec );
	JS_DefineFunction( cx, globalObject, "LoadModule", global_loadModule, 0, 0 );
//	JS_DefineFunction( cx, globalObject, "test", global_test, 0, 0 );
// JS_DefineFunction( cx, globalObject, "Module", _Module, 0, 0 );

// Global configuration object
	JSObject *configObject;
	jsStatus = GetConfigurationObject(cx, &configObject);
	RT_HOST_MAIN_ASSERT( jsStatus != JS_FALSE, "failed to get configuration object." );
	RT_HOST_MAIN_ASSERT( configObject != NULL, "unable to create the configuration data." );
	jsval value = OBJECT_TO_JSVAL(JS_GetFunctionObject(JS_NewFunction(cx, stdoutFunction, 1, 0, NULL, NULL))); // If you do not assign a name to the function, it is assigned the name "anonymous".
	jsStatus = JS_SetProperty(cx, configObject, "stdout", &value);
	RT_HOST_MAIN_ASSERT( jsStatus != JS_FALSE, "Unable to set store stdout into configuration." );
	value = BOOLEAN_TO_JSVAL(unsafeMode);
	jsStatus = JS_SetProperty(cx, configObject, "unsafeMode", &value);
	RT_HOST_MAIN_ASSERT( jsStatus != JS_FALSE, "Unable to set store unsafeMode into configuration." );



// script name
  scriptName = *argv;
  RT_HOST_MAIN_ASSERT( scriptName != NULL, "no script specified" );

// arguments
	JSObject *argsObj = JS_NewArrayObject(cx, 0, NULL);
	RT_HOST_MAIN_ASSERT( argsObj != NULL, "unable to create argument array." );

	jsStatus = JS_DefineProperty(cx, globalObject, "arguments", OBJECT_TO_JSVAL(argsObj), NULL, NULL, 0);
	RT_HOST_MAIN_ASSERT( jsStatus == JS_TRUE, "unable to store the argument array." );
	int index = 0;
	for ( ; *argv; argv++ ) {

		JSString *str = JS_NewStringCopyZ(cx, *argv);
		RT_HOST_MAIN_ASSERT( str != NULL, "unable to store the argument." );
		jsStatus = JS_DefineElement(cx, argsObj, index++, STRING_TO_JSVAL(str), NULL, NULL, JSPROP_ENUMERATE);
		RT_HOST_MAIN_ASSERT( str != NULL, "unable to store the argument." );
	}

// language options
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
	gBranchLimit =
	JS_SetBranchCallback(cx, BranchCallback);
	JS_ToggleOptions(cx, JSOPTION_NATIVE_BRANCH_CALLBACK);
	*/

// compile & executes the script
	JSScript *script = JS_CompileFile( cx, globalObject, scriptName );
//  JSScript *script = LoadScript( cx, globalObject, scriptName, saveCompiledScripts );
	RT_HOST_MAIN_ASSERT( script != NULL, "unable to compile the script." );

	// You need to protect a JSScript (via a rooted script object) if and only if a garbage collection can occur between compilation and the start of execution.
	jsval rval;
	jsStatus = JS_ExecuteScript( cx, globalObject, script, &rval ); // MUST be executed only once ( JSOPTION_COMPILE_N_GO )
	// if jsStatus != JS_TRUE, an error has been throw while the execution, so there is no need to throw another error

//		printf( "Last executed line %s:%d", script->filename, JS_PCToLineNumber( cx, script, script->code ) ); // if it right ? [TBD] enhance this

  JS_DestroyScript( cx, script );

//  printf("script result: %s\n", JS_GetStringBytes(JS_ValueToString(cx, rval)));


#ifdef JS_THREADSAFE
    JS_EndRequest(cx);
#endif

// cleanup
  // For each context you've created
  JS_DestroyContext(cx); // [TBD] is JS_DestroyContextNoGC faster ?

  // For each runtime
  JS_DestroyRuntime(rt);

  // And finally
  JS_ShutDown();

// free used modules

	typedef void (*ModuleFreeFunction)(void);

	for ( int i = sizeof(_moduleList) / sizeof(*_moduleList) - 1; i >= 0; --i ) // beware: 'i' must be signed
		if ( _moduleList[i] != NULL ) {

			ModuleFreeFunction moduleFree = (ModuleFreeFunction)::GetProcAddress( _moduleList[i], "ModuleRelease" );
			if ( moduleFree != NULL )
				moduleFree();
			::FreeLibrary(_moduleList[i]);
		}
  return 0;
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


TODO list: ( - todo, v- done, x- forget, > comment )
---------
  v- rename global.Load into global.exec
  x- add an global.options() function to configure language options
  - add a global.Version() function
	x- add an argument 'once' to .exec : exec('jsni.js', true )
	  > can be done in javascript
	x- add an argument 'save comiled version' to .exec
	v- add an argument 'save comiled version' to command line
	- manage gBranchLimit ( property of global objet ? )
	x- manage exit()
	  > not needed
	- manage object environment ( see mozilla/js/src/js.c )
	- rewrite Load&XDR in javascript ?
	x- support \\ and / path separator
	  > "/" is natively suported by XP
	v- configurable warning report
	  > command-line switch
	- execOnce hash table should be based on the full path name
	- port to linux
	- use a dyn. list for _LoadModule
	- trap Ctrl+C signal


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
	It's not minimum requirement in the	report-an-error/throw-an-exception-if-fewer-actuals sense.  It's the minimum the callee can count on dereferencing with non-negative indexes via argv.
	If fewer actuals are passed, the engine will push undefined until nargs arguments are available.
	...
	Just because argc reflects the actual parameter count does not mean that you cannot dereference argv[argc] or argv[argc+1] safely -- you can, all the way up to argv[NARGS-1],
	for the value of NARGS you stored in the JSFunctionSpec.nargs initializer, or passed to JS_DefineFunction.
  ...
	Ah, I think I understand now. I can specify 0 for nargs for *all* functions just as long as I check argc's value before dereferencing argv[] elements.
	But if I want guarantee that argv[0], as an example, can be dereferenced (regardless of the number of arguments actually passed), then I need to specify an nargs value of 1 (or higher). Right?


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