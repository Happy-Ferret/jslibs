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

#include "jslibsModule.cpp"

#include "../jslang/handlePub.h"



volatile bool disabledFree = false;

#define NO_NED_NAMESPACE
#define NO_MALLINFO 1
#include "../../libs/nedmalloc/nedmalloc.h"

void nedfree_handlenull(void *mem) {

	if ( mem != NULL && !disabledFree )
		nedfree(mem);
}



static unsigned char embeddedBootstrapScript[] =
	#include "embeddedBootstrapScript.js.xdr.cres"
;

#define HOST_MAIN_ASSERT( condition, errorMessage ) if ( !(condition) ) { fprintf(stderr, errorMessage ); goto bad; }



volatile bool gEndSignalState = false;
JLSemaphoreHandler gEndSignalEvent;


JSBool EndSignalGetter(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	return BoolToJsval(cx, gEndSignalState, vp);
}

JSBool EndSignalSetter(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	bool tmp;
	JL_CHK( JsvalToBool(cx, *vp, &tmp) );
	gEndSignalState = tmp;
	
	JLReleaseSemaphore(gEndSignalEvent);
	JLAcquireSemaphore(gEndSignalEvent, 0);

	return JS_TRUE;
	JL_BAD;
}


#ifdef XP_WIN
BOOL Interrupt(DWORD CtrlType) {

// see. http://msdn2.microsoft.com/en-us/library/ms683242.aspx
//	if (CtrlType == CTRL_LOGOFF_EVENT || CtrlType == CTRL_SHUTDOWN_EVENT) // CTRL_C_EVENT, CTRL_BREAK_EVENT, CTRL_CLOSE_EVENT, CTRL_LOGOFF_EVENT, CTRL_SHUTDOWN_EVENT
//		return FALSE;
	gEndSignalState = true;
	JLReleaseSemaphore(gEndSignalEvent);
	JLAcquireSemaphore(gEndSignalEvent, 0);
	return TRUE;
}
#else
void Interrupt(int CtrlType) {

	gEndSignalState = true;
	JLReleaseSemaphore(gEndSignalEvent);
	JLAcquireSemaphore(gEndSignalEvent, 0);
}
#endif // XP_WIN


struct MetaPollEndSignalData {
	
	MetaPoll mp;
	volatile bool cancel;
};

static void EndSignalStartPoll( volatile MetaPoll *mp ) {

	MetaPollEndSignalData *mpes = (MetaPollEndSignalData*)mp;
	while ( !gEndSignalState && !mpes->cancel )
		JLAcquireSemaphore(gEndSignalEvent, -1);
}

static void EndSignalCancelPoll( volatile MetaPoll *mp ) {

	MetaPollEndSignalData *mpes = (MetaPollEndSignalData*)mp;
	mpes->cancel = true;

	JLReleaseSemaphore(gEndSignalEvent);
	JLAcquireSemaphore(gEndSignalEvent, 0);
}

static JSBool EndSignalEndPoll( volatile MetaPoll *mp, bool *hasEvent, JSContext *cx, JSObject *obj ) {

	*hasEvent = gEndSignalState;
	return JS_TRUE;
}

static JSBool MetaPollEndSignal(JSContext *cx, uintN argc, jsval *vp) {

	JL_S_ASSERT_ARG(0);

	MetaPollEndSignalData *mpes;
	JL_CHK( CreateHandle(cx, 'poll', sizeof(MetaPollEndSignalData), (void**)&mpes, NULL, JL_FRVAL) );
	mpes->mp.startPoll = EndSignalStartPoll;
	mpes->mp.cancelPoll = EndSignalCancelPoll;
	mpes->mp.endPoll = EndSignalEndPoll;
	mpes->cancel = false;

	return JS_TRUE;
	JL_BAD;
}



int HostStdout( void *privateData, const char *buffer, size_t length ) {

	return write(fileno(stdout), buffer, length);
}

int HostStderr( void *privateData, const char *buffer, size_t length ) {

	return write(fileno(stderr), buffer, length);
}

//void NewScriptHook(JSContext *cx, const char *filename, uintN lineno, JSScript *script, JSFunction *fun, void *callerdata) {
//	printf( "add - %s:%d - %s - %d - %p\n", filename, lineno, fun ? JS_GetFunctionName(fun):"", script->staticDepth, script );
//}
//void DestroyScriptHook(JSContext *cx, JSScript *script, void *callerdata) {
//	printf( "del - %s:%d - ? - %d - %p\n", script->filename, script->lineno, script->staticDepth, script );
//}

//////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) { // check int _tmain(int argc, _TCHAR* argv[]) for UNICODE

#ifdef XP_WIN
	// enable low fragmentation heap
	HANDLE heap = GetProcessHeap();
	ULONG enable = 2;
	HeapSetInformation(heap, HeapCompatibilityInformation, &enable, sizeof(enable));
#endif // XP_WIN

	JSContext *cx = NULL;

	uint32 maxMem = (uint32)-1; // by default, there are no limit
	uint32 maxAlloc = (uint32)-1; // by default, there are no limit
	bool unsafeMode = false;
	bool compileOnly = false;
	float maybeGCInterval = 10; // seconds
	int camelCase = 0; // 0:default, 1:lower, 2:upper
	bool useFileBootstrapScript = false;
	const char *inlineScript = NULL;
	const char *scriptName = NULL;

#ifdef DEBUG
	bool debug;
	debug = false;
#endif

	// (TBD) use getopt instead ?
	char** argumentVector = argv;
	for ( argumentVector++; argumentVector[0] && argumentVector[0][0] == '-'; argumentVector++ )
		switch ( argumentVector[0][1] ) {
			case 'm': // maxbytes (GC)
				argumentVector++;
				HOST_MAIN_ASSERT( *argumentVector, "Missing argument." );
				maxMem = atol( *argumentVector ) * 1024L * 1024L;
				break;
			case 'n': // maxAlloc (GC)
				argumentVector++;
				HOST_MAIN_ASSERT( *argumentVector, "Missing argument." );
				maxAlloc = atol( *argumentVector ) * 1024L * 1024L;
				break;
			case 'u': // avoid any runtime checks
				unsafeMode = true;
				break;
			case 'g': // operationLimitGC
				argumentVector++;
				HOST_MAIN_ASSERT( *argumentVector, "Missing argument." );
				maybeGCInterval = atof(*argumentVector);
				break;
			case 'c': // compileOnly
				compileOnly = true;
				break;
			case 'l': // camelCase
				argumentVector++;
				HOST_MAIN_ASSERT( *argumentVector, "Missing argument." );
				camelCase = atoi( *argumentVector );
				break;
			case 'b': // bootstrap
				useFileBootstrapScript = true;
				break;
			case 'i': // inline script
				// argumentVector++; // keep the script as argument[0]
				HOST_MAIN_ASSERT( *argumentVector, "Missing argument." );
				inlineScript = *(argumentVector+1);
				break;
			case 'v': // version
				fprintf( stderr, "Version r%d - Build %.4d-%.2d-%.2d / %s\n", JL_SvnRevToInt("$Revision$"), __DATE__YEAR, __DATE__MONTH+1, __DATE__DAY, JS_GetImplementationVersion() );
				return EXIT_SUCCESS;
			case '?': // help
			case 'h': //
				fprintf( stderr, "Help: http://code.google.com/p/jslibs/wiki/jshost#Command_line_options\n" );
				return EXIT_SUCCESS;
		#ifdef DEBUG
			case 'd': // debug
				debug = true;
		#endif // DEBUG
	}


	static const bool useJslibsMemoryManager = unsafeMode;


#if defined(XP_WIN) && defined(DEBUG) && defined(REPORT_MEMORY_LEAKS)
	if ( debug ) {
		_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
		_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG );
		_CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDERR );
	}
#endif

#ifdef XP_WIN
	BOOL status;
	status = SetConsoleCtrlHandler((PHANDLER_ROUTINE)&Interrupt, TRUE);
	HOST_MAIN_ASSERT( status == TRUE, "Unable to set the Ctrl-C handler." );
#else
	signal(SIGINT, Interrupt);
	signal(SIGTERM, Interrupt);
#endif // XP_WIN

	if ( useJslibsMemoryManager ) {

		jl_malloc = nedmalloc;
		jl_calloc = nedcalloc;
		jl_memalign = nedmemalign;
		jl_realloc = nedrealloc;
		jl_msize = nedblksize;
		jl_free = nedfree_handlenull;
		
		InitializeMemoryManager(&jl_malloc, &jl_calloc, &jl_memalign, &jl_realloc, &jl_msize, &jl_free);
		
	#ifdef JS_HAS_JSLIBS_RegisterCustomAllocators
		JSLIBS_RegisterCustomAllocators(jl_malloc, jl_calloc, jl_memalign, jl_realloc, jl_msize, jl_free);
	#endif // JS_HAS_JSLIBS_RegisterCustomAllocators

	} else {

		jl_malloc = malloc;
		jl_calloc = calloc;
		jl_memalign = memalign;
		jl_realloc = realloc;
		jl_msize = msize;
		jl_free = free;
	}

	cx = CreateHost(maxMem, maxAlloc, maybeGCInterval * 1000);
	HOST_MAIN_ASSERT( cx != NULL, "Unable to create a javascript execution context." );

	if ( useJslibsMemoryManager )
		MemoryManagerEnableGCEvent(cx);

	HostPrivate *hpv;
	hpv = GetHostPrivate(cx);
	hpv->camelCase = camelCase;

	// custom memory allocators are transfered to modules through the HostPrivate structure:
	hpv->alloc.malloc = jl_malloc;
	hpv->alloc.calloc = jl_calloc;
	hpv->alloc.memalign = jl_memalign;
	hpv->alloc.realloc = jl_realloc;
	hpv->alloc.msize = jl_msize;
	hpv->alloc.free = jl_free;

	JS_SetOptions(cx, JS_GetOptions(cx) | JSOPTION_STRICT | JSOPTION_RELIMIT ); // default, may be disabled in InitHost()

	HOST_MAIN_ASSERT( InitHost(cx, unsafeMode, HostStdout, HostStderr, NULL), "Unable to initialize the host." );

	JSObject *globalObject;
	globalObject = JS_GetGlobalObject(cx);

	gEndSignalEvent = JLCreateSemaphore(0);
	JL_CHK( JS_DefineProperty(cx, globalObject, "endSignal", JSVAL_VOID, EndSignalGetter, EndSignalSetter, JSPROP_SHARED | JSPROP_PERMANENT) );
	JL_CHK( JS_DefineFunction(cx, globalObject, "MetaPollEndSignal", (JSNative)MetaPollEndSignal, 0, JSPROP_SHARED | JSPROP_PERMANENT | JSFUN_FAST_NATIVE) );

// script name
	if ( inlineScript == NULL )
		scriptName = *argumentVector;
	HOST_MAIN_ASSERT( inlineScript != NULL || scriptName != NULL, "No script specified." );

	char hostFullPath[PATH_MAX +1];

#ifdef XP_WIN
// get hostpath and hostname
	HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(NULL);
	DWORD len = GetModuleFileName(hInstance, hostFullPath, sizeof(hostFullPath));
	HOST_MAIN_ASSERT( len != 0, "Unable to GetModuleFileName." );
#else // XP_WIN
	JLGetAbsoluteModulePath(hostFullPath, sizeof(hostFullPath), argv[0]);
	HOST_MAIN_ASSERT( hostFullPath[0] != '\0', "Unable to get module FileName." );
//	int len = readlink("/proc/self/exe", moduleFileName, sizeof(moduleFileName)); // doc: readlink does not append a NUL character to buf.
//	moduleFileName[len] = '\0';
//	strcpy(hostFullPath, argv[0]);
#endif // XP_WIN

	char *hostName;
	const char *hostPath;
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

	JL_CHK( SetPropertyString(cx, globalObject, JLID_NAME(scripthostpath), hostPath) );
	JL_CHK( SetPropertyString(cx, globalObject, JLID_NAME(scripthostname), hostName) );

	if ( sizeof(embeddedBootstrapScript)-1 > 0 )
		JL_CHK( ExecuteBootstrapScript(cx, embeddedBootstrapScript, sizeof(embeddedBootstrapScript)-1) ); // -1 because sizeof("") == 1

	if ( useFileBootstrapScript ) {

		jsval tmp;
		char bootstrapFilename[PATH_MAX +1];
		strcpy(bootstrapFilename, hostPath);
		strcat(bootstrapFilename, PATH_SEPARATOR_STRING);
		strcat(bootstrapFilename, hostName);
		strcat(bootstrapFilename, ".js"); // (TBD) perhaps find another extension for bootstrap scripts (on windows: jshost.exe.js)
		JL_CHK( ExecuteScriptFileName(cx, bootstrapFilename, compileOnly, argc - (argumentVector-argv), argumentVector, &tmp) );
	}

	int exitValue;
	jsval rval;

	JSBool executeStatus;
	if ( inlineScript == NULL )
		executeStatus = ExecuteScriptFileName(cx, scriptName, compileOnly, argc - (argumentVector-argv), argumentVector, &rval);
	else
		executeStatus = ExecuteScript(cx, inlineScript, compileOnly, argc - (argumentVector-argv), argumentVector, &rval);

	if ( executeStatus == JS_TRUE ) {

		if ( JSVAL_IS_INT(rval) && JSVAL_TO_INT(rval) >= 0 ) // (TBD) enhance this, use JsvalToInt() ?
			exitValue = JSVAL_TO_INT(rval);
		else
			exitValue = EXIT_SUCCESS;
	} else {

		if ( JS_IsExceptionPending(cx) ) { // see JSOPTION_DONT_REPORT_UNCAUGHT option.

			jsval ex;
			JS_GetPendingException(cx, &ex);
			JL_ValueOf(cx, &ex, &ex);
			if ( JSVAL_IS_INT(ex) ) {

				exitValue = JSVAL_TO_INT(ex);
			} else {

				JS_ReportPendingException(cx);
				exitValue = EXIT_FAILURE;
			}
		} else {

			exitValue = EXIT_FAILURE;
		}
	}

	if ( useJslibsMemoryManager ) {
	
		disabledFree = true;
		MemoryManagerDisableGCEvent(cx);
		FinalizeMemoryManager(!disabledFree, &jl_malloc, &jl_calloc, &jl_memalign, &jl_realloc, &jl_msize, &jl_free);
	}

	JS_CommenceRuntimeShutDown(JS_GetRuntime(cx));
	JS_SetGCCallback(cx, NULL);
	DestroyHost(cx);
	JS_ShutDown();
	cx = NULL;

#ifdef XP_WIN
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)&Interrupt, FALSE);
//	RT_HOST_MAIN_ASSERT( status == TRUE, "Unable to remove console crtl handler" );
#else
	signal(SIGINT, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
#endif // XP_WIN

#if defined(XP_WIN) && defined(DEBUG) && defined(REPORT_MEMORY_LEAKS)
	if ( debug ) {
//		_CrtMemDumpAllObjectsSince(NULL);
	}
#endif
//	flush(stdout);
//	flush(stderr);
	return exitValue;
bad:

	if ( cx ) {

		if ( useJslibsMemoryManager )
			disabledFree = true;
		JS_CommenceRuntimeShutDown(JS_GetRuntime(cx));
		JS_SetGCCallback(cx, NULL);
		DestroyHost(cx);
	}
	JS_ShutDown();
	return EXIT_FAILURE;
}



/**doc
#summary jshost executable
#labels doc

= jshost executable =
 [http://code.google.com/p/jslibs/ home] *>* [JSLibs] *>* [jshost] - [http://jslibs.googlecode.com/svn/trunk/jshost/jshost.cpp http://jslibs.googlecode.com/svn/wiki/source.png]

=== Description ===

jshost ( javascript host ) is a small executable file that run javascript programs.
The main features are:
 * Lightweight
  The binary executable file is less than 60KB
 * Minimalist internal API
  LoadModule is enough, everything else can be added using dynamic loadable modules.

=== Command line options ===
 * `-c <0 or 1>` (default = 0)
  Compile-only. The script is compiled but not executed. This is useful to detect syntax errors.
 * `-u` (disabled by default)
  Run in unsafe-mode that is a kind of 'release mode'. In unsafe-mode, any runtime checks is avoid and warnings are not reported. This mode allow a better execution speed.
 * `-m <size>` (default: no limit)
  Specifies the maximum memory usage of the script in megabytes.
 * `-n  <size>` (default: no limit)
  Specifies the number of allocated megabytes after which garbage collection is run.
 * `-g <time>` (default = 60)
  This is the frequency (in seconds) at wich the GarbageCollector may be launched (0 for disabled).
 * `-l <case>` (default = 0)
  This is a temporary option that allow to select function name naming. 0:version default, 1:lowerCamelCase, 2:UpperCamelCase
  $H note
   Default is UpperCamelCase for jslibs version < 1.0 and lowerCamelCase for jslibs version >= 1.0
 * `-b`
  Run the bootstrap file (<executable filename>.js, eg. jshost.exe.js on windows and jshost.js on Linux)
 * `-v`
  Displays the current version or revision.
 * `-h` `-h`
  Help.

$H beware
 Options of the host must be *before* the script name.$LF
 Options of the script must be *after* the script name.

=== Exit code ===
 * The exit code of jshost is 1 on error. On success, exit code is the last evaluated expression of the script.
   If this last expression is a positive integer, its value is returned, in any other case, 0 is returned.
 * If there is a pending uncatched exception and if this exception can be converted into a number (see valueOf()), this numeric value is used as exit code.
 $H example
 {{{
 function Exit(code) {
  throw code;
 }

 Exit(2);
 }}}

=== Global functions ===
 * status *LoadModule*( moduleFileName )
  Loads and initialize the specified module.
  Do not provide the file extension in _moduleFileName_.
  $H exemple
  {{{
  LoadModule('jsstd');
  Print( 'Unsafe mode: '+_configuration.unsafeMode, '\n' );
  }}}
  $H note
  You can avoid LoadModule to use the global object and load the module in your own namespace:
  {{{
  var std = {};
  LoadModule.call( std, 'jsstd' );
  std.Print( std.IdOf(1234), '\n' );
  std.Print( std.IdOf(1234), '\n' );
  }}}

=== Global properties ===

 * *arguments*
  The command-line arguments (given after command line options).
  $H example
  {{{
  for ( var i in arguments ) {

   Print( 'argument['+i+'] = '+arguments[i] ,'\n' );
  }
  }}}
  <pre>
  ...
  c:\>jshost -g 600 -u foo.js bar
  argument[0] = foo.js
  argument[1] = bar
  </pre>

 * *endSignal*
  Is $TRUE if a break signal (ctrl-c, ...) has been sent to jshost. This event can be reset.

=== Configuration object ===
 jshost create a global `_configuration` object to provide other modules some useful informations like `stdout` access and `unsafeMode` flag.

== Remarks ==

=== Generated filename extensions are ===
 * ".dll" : for windows
 * ".so" : for linux

=== Modules entry points signature are ===
|| `"ModuleInit"` || `JSBool (*ModuleInitFunction)(JSContext *, JSObject *)` || Called when the module is being load ||
|| `"ModuleRelease"` || `void (*ModuleReleaseFunction)(JSContext *cx)` || Called when the module is not more needed ||
|| `"ModuleFree"` || `void (*ModuleFreeFunction)(void)` || Called to let the module moke some cleanup tasks ||


=== Exemple (win32) ===
{{{
extern "C" __declspec(dllexport) JSBool ModuleInit(JSContext *cx, JSObject *obj) {

 InitFileClass(cx, obj);
 InitDirectoryClass(cx, obj);
 InitSocketClass(cx, obj);
 InitErrorClass(cx, obj);
 InitGlobal(cx, obj);

 return JS_TRUE;
}

== Embedding JS scripts in your jshost binary ==
 This can only be done at jshost compilation time.
 # Checkout [http://code.google.com/p/jslibs/source/checkout jslibs sources]
 # Save your embbeded script in the file _jslibs/src/jshost/embeddedBootstrapScript.js_
 # [jslibsBuild Compile jslibs] (or only jshost if jslibs has already been compiled once)
}}}
**/
