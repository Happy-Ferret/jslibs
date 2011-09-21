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

// set stack to 2MB:
#if defined(XP_WIN)
	#pragma comment (linker, "/STACK:0x400000")
#elif defined(XP_UNIX)
	#pragma stacksize 4194304
	//char stack[0x200000] __attribute__ ((section ("STACK"))) = { 0 };
	//init_sp(stack + sizeof (stack));
#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif


#include <jslibsModule.cpp>

#include "../jslang/handlePub.h"


volatile bool disabledFree = false;

#define NO_NED_NAMESPACE
#define NO_MALLINFO 1
#include "../../libs/nedmalloc/nedmalloc.h"


NOALIAS void nedfree_handlenull(void *mem) NOTHROW {

	if ( mem != NULL && !disabledFree )
		nedfree(mem);
}

NOALIAS size_t nedblksize_msize(void *mem) NOTHROW {

	return nedblksize(0, mem);
}


static unsigned char embeddedBootstrapScript[] =
	#include "embeddedBootstrapScript.js.xdr.cres"
;

#define HOST_MAIN_ASSERT( CONDITION, ERROR_MESSAGE ) \
	JL_MACRO_BEGIN \
		if ( !(CONDITION) ) { \
			fprintf(stderr, ERROR_MESSAGE "\n"); \
			goto bad; \
		} \
	JL_MACRO_END

volatile int gEndSignalState = 0;
JLCondHandler gEndSignalCond;
JLMutexHandler gEndSignalLock;


JSBool EndSignalGetter(JSContext *cx, JSObject *obj, jsid id, jsval *vp) {

	JL_IGNORE(cx);
	JL_IGNORE(obj);
	JL_IGNORE(id);
	//return JL_NativeToJsval(cx, (int)gEndSignalState, vp);
	*vp = INT_TO_JSVAL(gEndSignalState);
	return JS_TRUE;
}

JSBool EndSignalSetter(JSContext *cx, JSObject *obj, jsid id, JSBool strict, jsval *vp) {

	JL_IGNORE(obj);
	JL_IGNORE(id);
	JL_IGNORE(strict);

	int tmp;
	JL_CHK( JL_JsvalToNative(cx, *vp, &tmp) );

	JLMutexAcquire(gEndSignalLock);
	gEndSignalState = tmp;
	JLCondBroadcast(gEndSignalCond);
	JLMutexRelease(gEndSignalLock);

	return JS_TRUE;
	JL_BAD;
}


#if defined(XP_WIN)
BOOL WINAPI Interrupt(DWORD CtrlType) {

// see. http://msdn2.microsoft.com/en-us/library/ms683242.aspx
//	if (CtrlType == CTRL_LOGOFF_EVENT || CtrlType == CTRL_SHUTDOWN_EVENT) // CTRL_C_EVENT, CTRL_BREAK_EVENT, CTRL_CLOSE_EVENT, CTRL_LOGOFF_EVENT, CTRL_SHUTDOWN_EVENT
//		return FALSE;

	//JL_IGNORE(CtrlType);
	JLMutexAcquire(gEndSignalLock);
	gEndSignalState = (CtrlType == CTRL_C_EVENT ? 1 : 2);
	JLCondBroadcast(gEndSignalCond);
	JLMutexRelease(gEndSignalLock);

	return TRUE;
}
#elif defined(XP_UNIX)
void Interrupt(int CtrlType) {

	JLMutexAcquire(gEndSignalLock);
	gEndSignalState = 1;
	JLCondBroadcast(gEndSignalCond);
	JLMutexRelease(gEndSignalLock);

}
#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif


struct UserProcessEvent {
	
	ProcessEvent pe;

	bool cancel;
	jsval callbackFunction;
};

S_ASSERT( offsetof(UserProcessEvent, pe) == 0 );

void EndSignalStartWait( volatile ProcessEvent *pe ) {

	UserProcessEvent *upe = (UserProcessEvent*)pe;

	JLMutexAcquire(gEndSignalLock);
	while ( gEndSignalState == 0 && !upe->cancel )
		JLCondWait(gEndSignalCond, gEndSignalLock);
	JLMutexRelease(gEndSignalLock);
}

bool EndSignalCancelWait( volatile ProcessEvent *pe ) {

	UserProcessEvent *upe = (UserProcessEvent*)pe;

	JLMutexAcquire(gEndSignalLock);
	upe->cancel = true;
	JLCondBroadcast(gEndSignalCond);
	JLMutexRelease(gEndSignalLock);

	return true;
}

JSBool EndSignalEndWait( volatile ProcessEvent *pe, bool *hasEvent, JSContext *cx, JSObject *obj ) {

	JL_IGNORE(obj);
	UserProcessEvent *upe = (UserProcessEvent*)pe;

	*hasEvent = gEndSignalState != 0;
	if ( !*hasEvent )
		return JS_TRUE;
	jsval rval;
	if ( JSVAL_IS_VOID( upe->callbackFunction ) )
		return JS_TRUE;
	JL_CHK( JS_CallFunctionValue(cx, JL_GetGlobalObject(cx), upe->callbackFunction, 0, NULL, &rval) );
	return JS_TRUE;
	JL_BAD;
}

JSBool EndSignalEvents(JSContext *cx, uintN argc, jsval *vp) {

	JL_ASSERT_ARGC_RANGE( 0, 1 );

	UserProcessEvent *upe;
	JL_CHK( HandleCreate(cx, JLHID(pev), sizeof(UserProcessEvent), (void**)&upe, NULL, JL_RVAL) );
	upe->pe.startWait = EndSignalStartWait;
	upe->pe.cancelWait = EndSignalCancelWait;
	upe->pe.endWait = EndSignalEndWait;
	upe->cancel = false;

	if ( JL_ARG_ISDEF(1) ) {

		JL_ASSERT_ARG_IS_FUNCTION(1);
		JL_CHK( SetHandleSlot(cx, *JL_RVAL, 0, JL_ARG(1)) ); // GC protection only
		upe->callbackFunction = JL_ARG(1);
	} else {
	
		upe->callbackFunction = JSVAL_VOID;
	}

	return JS_TRUE;
	JL_BAD;
}


static int stdin_fileno = -1;
static int stdout_fileno = -1;
static int stderr_fileno = -1;

int HostStdin( void *privateData, char *buffer, size_t bufferLength ) {

	JL_IGNORE(privateData);
	if (unlikely( stdin_fileno == -1 ))
		stdin_fileno = fileno(stdin);
	return read(stdin_fileno, (void*)buffer, bufferLength);
}

int HostStdout( void *privateData, const char *buffer, size_t length ) {

	JL_IGNORE(privateData);
	if (unlikely( stdout_fileno == -1 ))
		stdout_fileno = fileno(stdout);
	return write(stdout_fileno, buffer, length);
}

int HostStderr( void *privateData, const char *buffer, size_t length ) {

	JL_IGNORE(privateData);
	if (unlikely( stderr_fileno == -1 ))
		stderr_fileno = fileno(stderr);
	return write(stderr_fileno, buffer, length);
}


//void NewScriptHook(JSContext *cx, const char *filename, uintN lineno, JSScript *script, JSFunction *fun, void *callerdata) {
//	printf( "add - %s:%d - %s - %d - %p\n", filename, lineno, fun ? JS_GetFunctionName(fun):"", script->staticDepth, script );
//}
//void DestroyScriptHook(JSContext *cx, JSScript *script, void *callerdata) {
//	printf( "del - %s:%d - ? - %d - %p\n", script->filename, script->lineno, script->staticDepth, script );
//}


// Helps to detect memory leaks (alloc/free balance)
//#define DBG_ALLOC 1

#ifdef DBG_ALLOC

static volatile int allocCount = 0;
static volatile int freeCount = 0;

EXTERN_C void* jl_malloc_count( size_t size ) {
	allocCount++;
	return malloc(size);
}
EXTERN_C void* jl_calloc_count( size_t num, size_t size ) {
	allocCount++;
	return calloc(num, size);
}
EXTERN_C void* jl_memalign_count( size_t alignment, size_t size ) {
	allocCount++;
	return memalign(alignment, size);
}
EXTERN_C void* jl_realloc_count( void *ptr, size_t size ) {
	if ( !ptr )
		allocCount++;
	return realloc(ptr, size);
}
EXTERN_C size_t jl_msize_count( void *ptr ) {
	return msize(ptr);
}
EXTERN_C void jl_free_count( void *ptr ) {
	if ( ptr )
		freeCount++;
	free(ptr);
}

#endif // DBG_ALLOC

//////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) { // check int _tmain(int argc, _TCHAR* argv[]) for UNICODE

//	BOOL st = SetProcessAffinityMask(GetCurrentProcess(), 1);

#ifdef XP_WIN
	// enable low fragmentation heap
	HANDLE heap = GetProcessHeap();
	ULONG enable = 2;
	HeapSetInformation(heap, HeapCompatibilityInformation, &enable, sizeof(enable));
#endif // XP_WIN

	JSContext *cx = NULL;

	uint32 maxMem = (uint32)-1; // by default, there are no limit
	uint32 maxAlloc = (uint32)-1; // by default, there are no limit
	bool warningsToErrors = false;
	bool unsafeMode = false;
	bool compileOnly = false;
	float maybeGCInterval = 10; // seconds
	char camelCase = 0; // 0:default, 1:lower, 2:upper
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
			case 'w': // convert warnings to errors
				warningsToErrors = true;
				break;
			case 'g': // operationLimitGC
				argumentVector++;
				HOST_MAIN_ASSERT( *argumentVector, "Missing argument." );
				maybeGCInterval = (float)atof(*argumentVector);
				break;
			case 'c': // compileOnly
				compileOnly = true;
				break;
			case 'l': // camelCase
				argumentVector++;
				HOST_MAIN_ASSERT( *argumentVector, "Missing argument." );
				camelCase = char(atoi( *argumentVector ));
				break;
			case 'b': // bootstrap
				useFileBootstrapScript = true;
				break;
			case 'i': // inline script
				argumentVector++; // keep the script as argument[0]
				HOST_MAIN_ASSERT( *argumentVector, "Missing argument." );
				inlineScript = *(argumentVector);
				break;
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

	if ( useJslibsMemoryManager ) {

		jl_malloc = nedmalloc;
		jl_calloc = nedcalloc;
		jl_memalign = nedmemalign;
		jl_realloc = nedrealloc;
		jl_msize = nedblksize_msize;
		jl_free = nedfree_handlenull;

	#ifdef DBG_ALLOC
		jl_malloc = jl_malloc_count;
		jl_calloc = jl_calloc_count;
		jl_memalign = jl_memalign_count;
		jl_realloc = jl_realloc_count;
		jl_msize = jl_msize_count;
		jl_free = jl_free_count;
	#endif // DBG_ALLOC
		
		InitializeMemoryManager(&jl_malloc, &jl_calloc, &jl_memalign, &jl_realloc, &jl_msize, &jl_free);
		
	// jslibs and spidermonkey allocator should be the same, else JL_NewString() and JL_NewUCString() should be fixed !
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

	#ifdef DBG_ALLOC
		jl_malloc = jl_malloc_count;
		jl_calloc = jl_calloc_count;
		jl_memalign = jl_memalign_count;
		jl_realloc = jl_realloc_count;
		jl_msize = jl_msize_count;
		jl_free = jl_free_count;
		JSLIBS_RegisterCustomAllocators(jl_malloc, jl_calloc, jl_memalign, jl_realloc, jl_msize, jl_free);
	#endif // DBG_ALLOC

	}

//	setvbuf(stderr, pBuffer, mode, buffer_size);

	cx = CreateHost(maxMem, maxAlloc, (uint32)(maybeGCInterval * 1000));

#ifdef DEBUG	
	if ( debug )
		JS_SetOptions(cx, JS_GetOptions(cx) & ~(JSOPTION_JIT | JSOPTION_METHODJIT | JSOPTION_PROFILING));
#endif // DEBUG	


	HOST_MAIN_ASSERT( cx != NULL, "Unable to initialize JavaScript engine." );

	if ( useJslibsMemoryManager )
		MemoryManagerEnableGCEvent(cx);

	HostPrivate *hpv;
	hpv = JL_GetHostPrivate(cx);
	hpv->camelCase = camelCase;

	// custom memory allocators are transfered to modules through the HostPrivate structure:
	hpv->alloc.malloc = jl_malloc;
	hpv->alloc.calloc = jl_calloc;
	hpv->alloc.memalign = jl_memalign;
	hpv->alloc.realloc = jl_realloc;
	hpv->alloc.msize = jl_msize;
	hpv->alloc.free = jl_free;

	JS_SetOptions(cx, JS_GetOptions(cx) | JSOPTION_STRICT | JSOPTION_RELIMIT | (warningsToErrors ? JSOPTION_WERROR : 0) ); // default, may be disabled in InitHost()

	JL_CHKM( InitHost(cx, unsafeMode, HostStdin, HostStdout, HostStderr, NULL), E_HOST, E_INIT ); // "Unable to initialize the host."

	JSObject *globalObject;
	globalObject = JL_GetGlobalObject(cx);

	gEndSignalCond = JLCondCreate();
	gEndSignalLock = JLMutexCreate();

#if defined(XP_WIN)
	JL_CHKM( SetConsoleCtrlHandler(Interrupt, TRUE) != 0, E_HOST, E_INTERNAL ); // "Unable to set the Ctrl-C handler."
#elif defined(XP_UNIX)
	signal(SIGINT, Interrupt);
	signal(SIGTERM, Interrupt);
#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif

	JL_CHK( JS_DefineProperty(cx, globalObject, "endSignal", JSVAL_VOID, EndSignalGetter, EndSignalSetter, JSPROP_SHARED | JSPROP_PERMANENT) );
	JL_CHK( JS_DefineFunction(cx, globalObject, "EndSignalEvents", (JSNative)EndSignalEvents, 0, JSPROP_SHARED | JSPROP_PERMANENT) );

// script name
	//	if ( inlineScript == NULL )
	scriptName = *argumentVector;

	JL_CHKM( inlineScript != NULL || scriptName != NULL || sizeof(embeddedBootstrapScript)-1 > 0, E_SCRIPT, E_NOTFOUND ); // "No script specified."

	char hostFullPath[PATH_MAX +1];

#if defined(XP_WIN)
// get hostpath and hostname
	HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(NULL);
	JL_CHKM( GetModuleFileName(hInstance, hostFullPath, sizeof(hostFullPath)) != 0, E_HOST, E_INTERNAL ); // "Unable to GetModuleFileName."
#elif defined(XP_UNIX)
	JLGetAbsoluteModulePath(hostFullPath, sizeof(hostFullPath), argv[0]);
	JL_CHKM( hostFullPath[0] != '\0', E_HOST, E_INTERNAL ); // "Unable to get module FileName."
//	int len = readlink("/proc/self/exe", moduleFileName, sizeof(moduleFileName)); // doc: readlink does not append a NUL character to buf.
//	moduleFileName[len] = '\0';
//	strcpy(hostFullPath, argv[0]);
#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif

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

	JL_CHK( JL_SetProperty(cx, globalObject, JLID(cx, scripthostpath), hostPath) );
	JL_CHK( JL_SetProperty(cx, globalObject, JLID(cx, scripthostname), hostName) );

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

	ASSERT( !JL_IsExceptionPending(cx) );

	JSBool executeStatus;
	if ( inlineScript != NULL )
		executeStatus = ExecuteScriptText(cx, inlineScript, compileOnly, argc - (argumentVector-argv), argumentVector, &rval);

	if ( (!inlineScript || inlineScript && executeStatus == JS_TRUE) && scriptName != NULL )
		executeStatus = ExecuteScriptFileName(cx, scriptName, compileOnly, argc - (argumentVector-argv), argumentVector, &rval);

	if ( executeStatus == JS_TRUE ) {

		if ( JSVAL_IS_INT(rval) && JSVAL_TO_INT(rval) >= 0 ) // (TBD) enhance this, use JL_JsvalToNative() ?
			exitValue = JSVAL_TO_INT(rval);
		else
			exitValue = EXIT_SUCCESS;
	} else {

		if ( JL_IsExceptionPending(cx) ) { // see JSOPTION_DONT_REPORT_UNCAUGHT option.

			jsval ex;
			JS_GetPendingException(cx, &ex);
			JL_JsvalToPrimitive(cx, ex, &ex);
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

	JS_SetGCCallback(cx, NULL);
	DestroyHost(cx, disabledFree);
	JS_ShutDown();
	cx = NULL;


#if defined(XP_WIN)
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)&Interrupt, FALSE);
//	RT_HOST_MAIN_ASSERT( status == TRUE, "Unable to remove console crtl handler" );
#elif defined(XP_UNIX)
	signal(SIGINT, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif


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
		JS_SetGCCallback(cx, NULL);
		DestroyHost(cx, true);
	}
	JS_ShutDown();
	return EXIT_FAILURE;
}

#ifdef DBG_ALLOC
struct DBG_ALLOC_dummyClass {
	~DBG_ALLOC_dummyClass() { // we must count at exit, see "dynamic atexit destructor"
		fprintf(stderr, "alloc:%d  free:%d (diff:%d)\n", allocCount, freeCount, allocCount - freeCount);
	}
} DBG_ALLOC_dummy;
#endif // DBG_ALLOC


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
 * `-w` (disabled by default)
  Convert warnings to error.
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
  Print( 'Unsafe mode: '+_host.unsafeMode, '\n' );
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

=== Host object ===
 jshost create a global `_host` object to provide other modules some useful informations like `stdin/stdout/stderr` access and `unsafeMode` flag.
 The `_host` also contains the `revision`, `build` and `jsVersion` properties.

==== Example ====
 host version information can be obtained using: `jshost -i "_host.stdout(_host.build+' r'+_host.revision)"`

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
