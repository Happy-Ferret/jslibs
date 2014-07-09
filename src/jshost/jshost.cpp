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

#define HOST_STACK_SIZE 4194304 // = 4 * 1024 * 1024

// set stack to 4MB:
#if defined(MSC)
	#pragma comment (linker, JL_TOSTRING(/STACK:HOST_STACK_SIZE))
#elif defined(GCC)
	#pragma stacksize HOST_STACK_SIZE
	//char stack[HOST_STACK_SIZE] __attribute__ ((section ("STACK"))) = { 0 };
	//init_sp(stack + sizeof (stack));
#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif

#if defined(WIN)
#define USE_NEDMALLOC
#endif


// #include <jslibsModule.cpp>

#include "../jslang/handlePub.h"


#define HOST_MAIN_ASSERT( CONDITION, ERROR_MESSAGE ) \
	JL_MACRO_BEGIN \
		if ( !(CONDITION) ) { \
			fprintf(stderr, "%s\n", (ERROR_MESSAGE)); \
			goto bad; \
		} \
	JL_MACRO_END


static const uint8_t embeddedBootstrapScript[] =
	#include "embeddedBootstrapScript.js.xdr.cres"
;



struct CmdLineArguments {
	int help;
	uint32_t maxMem;
	uint32_t maxAlloc;
	bool warningsToErrors;
	bool unsafeMode;
	bool compileOnly;
	float maybeGCInterval;
	bool useFileBootstrapScript;
	const TCHAR *inlineScript;
	#ifdef DEBUG
	bool debug;
	#endif DEBUG

	int jsArgc;
	TCHAR** jsArgv;

	bool
	parse(int argc, TCHAR* argv[]) {

		maxMem = (uint32_t)-1; // by default, there are no limit
		maxAlloc = (uint32_t)-1; // by default, there are no limit
		warningsToErrors = false;
		unsafeMode = false;
		compileOnly = false;
		maybeGCInterval = 10; // seconds
		useFileBootstrapScript = false;
		inlineScript = NULL;
		help = false;

		#ifdef DEBUG
		debug = false;
		#endif DEBUG

		TCHAR** argumentVector = argv;
		for ( argumentVector++; argumentVector[0] && argumentVector[0][0] == '-'; argumentVector++ )
			switch ( argumentVector[0][1] ) {
				case 'm': // maxbytes (GC)
					argumentVector++;
					HOST_MAIN_ASSERT( *argumentVector, "Missing argument." );
					maxMem = jl::atoi( *argumentVector, 10 ) * 1024L * 1024L;
					break;
				case 'n': // maxAlloc (GC)
					argumentVector++;
					HOST_MAIN_ASSERT( *argumentVector, "Missing argument." );
					maxAlloc = jl::atoi( *argumentVector, 10 ) * 1024L * 1024L;
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
					maybeGCInterval = float(jl::atof(*argumentVector));
					break;
				case 'c': // compileOnly
					compileOnly = true;
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
					help = true;
					break;
	
			#ifdef DEBUG
				case 'd': // debug
					debug = true;
					break;
			#endif // DEBUG
		}

		jsArgc = argc - (argumentVector-argv);
		jsArgv = argumentVector;

		return true;
		JL_BAD;
	}
};


class HostStdIO : public jl::StdIO {
	int stdin_fileno;
	int stdout_fileno;
	int stderr_fileno;
public:

	HostStdIO()
	: stdin_fileno(-1), stdout_fileno(-1), stderr_fileno(-1) {
	}

	int
	input( char *buffer, size_t bufferLength ) {

		if (unlikely( stdin_fileno == -1 ))
			stdin_fileno = fileno(stdin);
		return read( stdin_fileno, (void*)buffer, bufferLength );
	}

	int
	output( const char *buffer, size_t length ) {
		
		if (unlikely(stdout_fileno == -1)) {
			stdout_fileno = fileno(stdout);
//			_setmode(stdout_fileno, _O_U16TEXT);
		}
		return write(stdout_fileno, buffer, length);
	}

	int
	error( const char *buffer, size_t length ) {
		
		if (unlikely(stderr_fileno == -1)) {

			stderr_fileno = fileno(stderr);
//			_setmode(stderr_fileno, _O_U16TEXT);
		}
		return write(stderr_fileno, buffer, length);
	}
};




static volatile int32_t gEndSignalState = 0;
static JLCondHandler gEndSignalCond;
static JLMutexHandler gEndSignalLock;

bool
EndSignalGetter(JSContext *cx, JS::Handle<JSObject*> obj, JS::Handle<jsid> id, JS::MutableHandle<JS::Value> vp) {

	JL_IGNORE(id, obj);

	JL_CHK( jl::setValue(cx, vp, gEndSignalState) );
	return true;
	JL_BAD;
}

bool
EndSignalSetter(JSContext *cx, JS::Handle<JSObject*> obj, JS::Handle<jsid> id, bool strict, JS::MutableHandle<JS::Value> vp) {

	JL_IGNORE(strict, id, obj);

	int tmp;
	JL_CHK( jl::getValue(cx, vp, &tmp) );

	JLMutexAcquire(gEndSignalLock);
	gEndSignalState = tmp;
	JLCondBroadcast(gEndSignalCond);
	JLMutexRelease(gEndSignalLock);
	return true;
	JL_BAD;
}

#if defined(WIN)

BOOL WINAPI
Interrupt( DWORD CtrlType ) {

// see. http://msdn2.microsoft.com/en-us/library/ms683242.aspx
//	if (CtrlType == CTRL_LOGOFF_EVENT || CtrlType == CTRL_SHUTDOWN_EVENT) // CTRL_C_EVENT, CTRL_BREAK_EVENT, CTRL_CLOSE_EVENT, CTRL_LOGOFF_EVENT, CTRL_SHUTDOWN_EVENT
//		return FALSE;

	//JL_IGNORE(CtrlType);
	JLMutexAcquire(gEndSignalLock);
	switch ( CtrlType ) {
		case CTRL_C_EVENT:
		case CTRL_BREAK_EVENT:
		case CTRL_CLOSE_EVENT:
			gEndSignalState = 1;
			break;
		case CTRL_LOGOFF_EVENT:
		case CTRL_SHUTDOWN_EVENT:
			gEndSignalState = 2;
			break;
		default:
			ASSERT(false);
	}
	JLCondBroadcast(gEndSignalCond);
	JLMutexRelease(gEndSignalLock);
	return TRUE;
}

#elif defined(UNIX)

void
Interrupt( int CtrlType ) {

	JLMutexAcquire(gEndSignalLock);
	switch ( CtrlType ) {
		case SIGINT:
		case SIGTERM:
			gEndSignalState = 1;
			break;
		case SIGKILL:
			gEndSignalState = 2;
			break;
		default:
			ASSERT(false);
	}
	JLCondBroadcast(gEndSignalCond);
	JLMutexRelease(gEndSignalLock);
}

#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif

struct EndSignalProcessEvent : public ProcessEvent2 {
	
	bool cancel;

	bool
	prepareWait( JSContext *, JS::HandleObject ) {
	
		cancel = false;
		return true;
	}

	void
	startWait() {

		JLMutexAcquire(gEndSignalLock);
		while ( gEndSignalState == 0 && !cancel )
			JLCondWait(gEndSignalCond, gEndSignalLock);
		JLMutexRelease(gEndSignalLock);
	}

	bool
	cancelWait() {

		JLMutexAcquire(gEndSignalLock);
		cancel = true;
		JLCondBroadcast(gEndSignalCond);
		JLMutexRelease(gEndSignalLock);

		return true;
	}

	bool
	endWait( bool *hasEvent, JSContext *cx, JS::HandleObject obj ) {

		*hasEvent = (gEndSignalState != 0);

		if ( !*hasEvent )
			return true;

		if ( slot( 0 ) != JL_VALUEZ ) {
		
			JS::RootedObject callThisObj(cx);
			callThisObj.set(&slot(1).toObject());
			JS::Value rval; // rval is unused then there is no need to root it
			JL_CHK( JS_CallFunctionValue(cx, callThisObj, hslot(0), JS::HandleValueArray::empty(), JS::MutableHandleValue::fromMarkedLocation(&rval)) );
		}
		return true;
		JL_BAD;
	}
};


bool
EndSignalEvents( JSContext *cx, unsigned argc, jsval *vp ) {

	JL_DEFINE_ARGS;

	JL_ASSERT_ARGC_RANGE(0, 1);

	EndSignalProcessEvent *upe = new EndSignalProcessEvent();
	JL_CHK( HandleCreate(cx, upe, JL_RVAL) );

	if ( JL_ARG_ISDEF(1) ) {

		JL_ASSERT_ARG_IS_CALLABLE(1);
		upe->slot(0) = JL_ARG(1);
		upe->slot(1) = JL_OBJVAL;
	}

	return true;
	JL_BAD;
}


bool
initInterrupt() {

	gEndSignalLock = JLMutexCreate();
	gEndSignalCond = JLCondCreate();

#if defined(WIN)
	JL_CHK( SetProcessShutdownParameters(0x180, SHUTDOWN_NORETRY) ); // last shutdown range: 100-1FF

//	DWORD shutdownlevel, shutdownflags;
//	GetProcessShutdownParameters(&shutdownlevel, &shutdownflags);
//	SetProcessShutdownParameters(shutdownlevel+1, SHUTDOWN_NORETRY);

	JL_CHK( SetConsoleCtrlHandler(Interrupt, TRUE) );
#elif defined(UNIX)
	signal(SIGINT, Interrupt);
	signal(SIGTERM, Interrupt);
	signal(SIGKILL, Interrupt);
#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif
	return true;
	JL_BAD;
}

bool
freeInterrupt() {

#if defined(WIN)
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)&Interrupt, FALSE);
#elif defined(UNIX)
	signal(SIGINT, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
	signal(SIGKILL, SIG_DFL);
#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif

	JLCondFree(&gEndSignalCond);
	JLMutexFree(&gEndSignalLock);
	return true;
	JL_BAD;
}

//////////////////////////////////////////////////////////////////////////////



#ifdef USE_NEDMALLOC

#define NO_NED_NAMESPACE
#define NO_MALLINFO 1
#include "../../libs/nedmalloc/nedmalloc.h"


class NedAllocators : public jl::Allocators {
	static volatile bool _skipCleanup;

	static NOALIAS void
	nedfree_handlenull(void *mem) NOTHROW {

		if ( !_skipCleanup && mem != NULL )
			nedfree(mem);
	}

	static NOALIAS size_t
	nedblksize_msize(void *mem) NOTHROW {

		return nedblksize(0, mem);
	}

public:
	NedAllocators()
	: Allocators(nedmalloc, nedcalloc, nedmemalign, nedrealloc, nedblksize_msize, nedfree_handlenull) {
	}

	void
	setSkipCleanup(bool skipCleanup) {

		_skipCleanup = skipCleanup;
	}
};

volatile bool NedAllocators::_skipCleanup = false;

#endif // USE_NEDMALLOC


using namespace jl;

// see |int wmain(int argc, wchar_t* argv[])| for wide char
//int main(int argc, char* argv[]) {
int _tmain( int argc, TCHAR* argv[] ) {

	int exitValue;
	CmdLineArguments args;
	JL_CHK( args.parse(argc, argv) );

	if ( args.help ) {

		fprintf( stderr, "Help: http://code.google.com/p/jslibs/wiki/jshost#Command_line_options\n" );
		exitValue = EXIT_SUCCESS;
	} else {

		//JL_setMonoCPU();
		JL_enableLowFragmentationHeap();

		// js engine and jslibs low-level allocators must the same
//		#if defined(USE_NEDMALLOC) && defined(HAS_JL_ALLOCATORS)
//		NedAllocators allocators;
//		#else
		StdAllocators allocators;
//		#endif // USE_NEDMALLOC


		//ThreadedAllocator alloc(allocators);
		
		IFDEBUG( CountedAlloc countAlloc(allocators) );

		// js engine and jslibs allocators must the same
		HostRuntime::setJSEngineAllocators(allocators); // need to be done before AutoJSEngineInit ?

		AutoJSEngineInit ase;

		//alloc.setSkipCleanup(true);
		//nedAlloc.setSkipCleanup(true);

		HostRuntime hostRuntime(allocators, uint32_t(args.maybeGCInterval * 1000)); // 0 mean no periodical GC

		// HOST_MAIN_ASSERT
		JL_CHK( hostRuntime.create((uint32_t)-1, (uint32_t)-1, HOST_STACK_SIZE) );

		JSContext *cx = hostRuntime.context();

		JS::ContextOptionsRef(cx).setWerror(args.warningsToErrors);

		//JS::RootedValue tmpVal(cx);
		//JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));
		//JL_CHK( ExecuteScriptText(cx, global, "(function() { for (var i = 0; i < 10000; ++i); })()", false, &tmpVal) );

		HostStdIO hostIO;
		jl::Host host(hostRuntime, hostIO);
		JL_CHK( host.create() );

		JL_CHKM( initInterrupt(), E_HOST, E_INTERNAL );
		// https://developer.mozilla.org/en/SpiderMonkey/JSAPI_Reference/JS_GetPropertyAttributes
		JL_CHK( JS_DefineProperty(cx, host.hostObject(), "endSignal", JL_UNDEFINED, JSPROP_SHARED, EndSignalGetter, EndSignalSetter) );
		JL_CHK( JS_DefineFunction(cx, host.hostObject(), "endSignalEvents", EndSignalEvents, 1, 0) );


		TCHAR hostFullPath[PATH_MAX];
		JL_CHK( jl::GetModuleFileName(hostFullPath) );

		TCHAR *hostName;
		hostName = jl::strrchr(hostFullPath, TEXT(PATH_SEPARATOR));
		JL_CHK( hostName );
		hostName += 1;
		int hostPathLength;
		hostPathLength = hostName - hostFullPath;

		TCHAR hostPath[PATH_MAX];
		jl::strncpy(hostPath, hostFullPath, hostPathLength);
		hostPath[hostPathLength] = TEXT('\0');

		JL_CHK( host.setHostPath(hostPath) );
		JL_CHK( host.setHostName(hostName) );
		
		ASSERT( !JS_IsExceptionPending(cx) );

		JL_CHK( host.setHostArguments(args.jsArgv, args.jsArgc) );

		JL_ASSERT_WARN(!(!args.inlineScript && args.jsArgc == 0 && !args.useFileBootstrapScript && COUNTOF(embeddedBootstrapScript) - 1 == 0), E_SCRIPT, E_NOTSPECIFIED);

		{

			JS::RootedObject globalObject(cx, JL_GetGlobal(cx));
			JS::RootedValue rval(cx);

			// embedded bootstrap script

			if (COUNTOF(embeddedBootstrapScript) - 1 > 0) {

				JS::AutoSaveContextOptions asco(cx);
				JS::ContextOptionsRef(cx).setDontReportUncaught(false);

				JS::RootedScript script(cx, JS_DecodeScript(cx, embeddedBootstrapScript, COUNTOF(embeddedBootstrapScript) - 1, NULL)); // -1 because sizeof("") == 1
				JL_CHK( script );
				JL_CHK( JS_ExecuteScript(cx, globalObject, script, &rval) );
			}

			// file bootstrap script

			if ( args.useFileBootstrapScript ) {

				TCHAR bootstrapFilename[PATH_MAX];
				jl::strcpy( bootstrapFilename, hostFullPath );
				jl::strcat( bootstrapFilename, TEXT(".js") );
				JL_CHK( jl::executeScriptFileName(cx, globalObject, bootstrapFilename, args.compileOnly, &rval) );
			}

			ASSERT( !JL_IsExceptionPending(cx) );

			bool executeStatus;
			executeStatus = true;

			// inline (command-line) script

			if ( args.inlineScript != NULL ) {

				executeStatus = jl::executeScriptText(cx, globalObject, args.inlineScript, args.compileOnly, &rval);
			}

			// file script

			if ( args.jsArgc == 1 && executeStatus == true ) {

				executeStatus = jl::executeScriptFileName(cx, globalObject, args.jsArgv[0], args.compileOnly, &rval);
			}

			if ( executeStatus == true ) {

				if ( rval.isInt32() && rval.toInt32() >= 0 ) // (TBD) enhance this, use jl::getValue() ?
					exitValue = rval.toInt32();
				else
					exitValue = EXIT_SUCCESS;
			} else {

				if ( JL_IsExceptionPending(cx) ) { // see JSOPTION_DONT_REPORT_UNCAUGHT option.

					JS::RootedValue ex(cx);
					JS_GetPendingException(cx, &ex);
					JL_CHK( jl::getPrimitive(cx, ex, &ex) );
					if ( ex.isInt32() ) {

						exitValue = ex.toInt32();
					} else {

						JS_ReportPendingException(cx);
						exitValue = EXIT_FAILURE;
					}
				} else {

					exitValue = EXIT_FAILURE;
				}
			}

		}

		freeInterrupt();

		// JS_SetGCCallback(JL_GetRuntime(cx), NULL, NULL);

		host.destroy();
		hostRuntime.destroy();
		host.free(); // must be executed after runtime destroy
	}

	return exitValue;
	JL_BAD;

/*
#ifdef DBG_ALLOC
	struct Tmp {
		static bool dbgAllocGetter(JSContext *cx, JSObject *, jsid, jsval *vp) {

			return jl::setValue(cx, vp, allocAmount);
		}
	};
	JL_CHK( JS_DefineProperty(cx, hostObj, "dbgAlloc", JSVAL_VOID, Tmp::dbgAllocGetter, NULL, JSPROP_SHARED) );
#endif // DBG_ALLOC
*/
/*
#if defined(WIN) && defined(DEBUG) && defined(REPORT_MEMORY_LEAKS)
	if ( debug ) {
		_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
		_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG );
		_CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDERR );
	}
#endif
*/
/*
#if defined(WIN) && defined(DEBUG) && defined(REPORT_MEMORY_LEAKS)
	if ( debug ) {
//		_CrtMemDumpAllObjectsSince(NULL);
	}
#endif
*/

}


int basic_test_main(int argc, char* argv[]) {


	const JSClass global_class = {
		"global", JSCLASS_GLOBAL_FLAGS, JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub, JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, nullptr, nullptr, nullptr, nullptr, JS_GlobalObjectTraceHook
	};

	JS_Init();

	JSRuntime *rt = JS_NewRuntime(32L * 1024L * 1024L);
	JSContext *cx = JS_NewContext(rt, 8192);
	JS_BeginRequest(cx);

	{

		JS::RootedObject globalObject(cx, JS_NewGlobalObject(cx, &global_class, nullptr, JS::FireOnNewGlobalHook));
		JSAutoCompartment ac(cx, globalObject);
		JS_InitStandardClasses(cx, globalObject);
	/*
		JS::PersistentRootedObject pr(cx);
		JS_GC(rt);
	*/

		JS::CompileOptions compileOptions(cx);
		compileOptions
			.setFileAndLine(__FILE__, __LINE__)
			.setNoScriptRval(true)
		;

		char scriptText[] = "";
		JS::RootedScript script(cx, JS_CompileScript(cx, globalObject, scriptText, jl::strlen(scriptText), compileOptions));


	}

	JS_EndRequest(cx);
	JS_DestroyContext(cx);
	JS_DestroyRuntime(rt);
	JS_ShutDown();
	return 0;
}

/*
int main_test2(int argc, char* argv[]) {

	const JSClass global_class = {
		"global", JSCLASS_GLOBAL_FLAGS, JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub, JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, nullptr, nullptr, nullptr, nullptr, JS_GlobalObjectTraceHook
	};

	JS_Init();

	JSRuntime *rt = JS_NewRuntime(32L * 1024L * 1024L, JS_NO_HELPER_THREADS);
	JSContext *cx = JS_NewContext(rt, 8192);
	JS_BeginRequest(cx);
	{
	JS::CompileOptions compileOptions(cx);

	JS::RootedObject globalObject(cx, JS_NewGlobalObject(cx, &global_class, nullptr, JS::FireOnNewGlobalHook));
	JSAutoCompartment ac(cx, globalObject);
	JS_InitStandardClasses(cx, globalObject);

////
	JS::RootedScript script(cx);
	char scriptText[] = "";
	script = JS_CompileScript(cx, globalObject, scriptText, strlen(scriptText), compileOptions);
////

	}

	JS_EndRequest(cx);
	JS_DestroyContext(cx);
	JS_DestroyRuntime(rt);
	JS_ShutDown();
	return 0;
}
*/


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
  loadModule is enough, everything else can be added using dynamic loadable modules.

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
 function exit(code) {

  throw code;
 }

 exit(2);
 }}}

=== Global functions ===
 * $THIS *loadModule*( moduleFileName )
  Loads and initialize the specified module.
  Do not provide the file extension in _moduleFileName_.
  $H exemple
  {{{
  loadModule('jsstd');
  print( 'Unsafe mode: ' + host.unsafeMode, '\n' );
  }}}
  $H note
  You can avoid loadModule to use the global object and load the module in your own namespace:
  $H example 1
  {{{
  var std = {};
  loadModule.call(std, 'jsstd');
  std.print( std.idOf(1234), '\n' );
  std.print( std.idOf(1234), '\n' );
  }}}
  $H example 2
  {{{
  var std = loadModule.call({}, 'jsstd');
  std.print('hello ');
  std.print('world');
  }}}
  $H example 3
  {{{
  var moduleMap = new Map();
  function myLoadModule(name) {
    
    var ns = {};
    var id = loadModule.call(ns, name);
    return id ? (moduleMap.set(name, ns), ns) : moduleMap.get(name);
  }

  // ...

  var s1 = myLoadModule('jsstd');
  var s2 = myLoadModule('jsstd');
  var s3 = myLoadModule('jsstd');

  s1.print('hello\n');
  s2.print('hello\n');
  s3.print('hello\n');

  throw 0;
  }}}

=== Global properties ===

 * *arguments*
  The command-line arguments (given after command line options).
  $H example
  {{{
  for ( var i in host.arguments ) {

   print( 'argument['+i+'] = '+host.arguments[i] ,'\n' );
  }
  }}}
  <pre>
  ...
  c:\>jshost -g 600 -u foo.js bar
  argument[0] = foo.js
  argument[1] = bar
  </pre>

 * *host.endSignal*
  Is $TRUE if a break signal (ctrl-c, ...) has been sent to jshost. This event can be reset.

=== Host object ===
 jshost create a global `host` object to provide other modules some useful informations like `stdin/stdout/stderr` access and `unsafeMode` flag.
 The `host` also contains the `sourceId`, `buildDate` and `jsVersion` properties.

==== Example ====
 host version information can be obtained using: `jshost -i "host.stdout(_host.build+' r'+_host.sourceId)"`

==== Example ====
{{{
var r = host.sourceId + (((2006*12 + 6)*31 + 22)*24 + 0);

var d = 12 * 31 * 24;
var year = Math.floor(r / d);
r = r % d;

var d = 31 * 24;
var month = Math.floor(r / d);
r = r % d;

var d = 24;
var day = Math.floor(r / d);
}}}

== Remarks ==

=== Generated filename extensions are ===
 * ".dll" : for windows
 * ".so" : for linux

=== Modules entry points signature are ===
|| `"ModuleInit"` || `bool (*ModuleInitFunction)(JSContext *, JSObject *)` || Called when the module is being load ||
|| `"ModuleRelease"` || `void (*ModuleReleaseFunction)(JSContext *cx)` || Called when the module is not more needed ||
|| `"ModuleFree"` || `void (*ModuleFreeFunction)(void)` || Called to let the module moke some cleanup tasks ||


=== Exemple (win32) ===
{{{
extern "C" __declspec(dllexport) bool ModuleInit(JSContext *cx, JSObject *obj) {

 InitFileClass(cx, obj);
 InitDirectoryClass(cx, obj);
 InitSocketClass(cx, obj);
 InitErrorClass(cx, obj);
 InitGlobal(cx, obj);

 return true;
}

== Embedding JS scripts in your jshost binary ==
 This can only be done at jshost compilation time.
 # Checkout [http://code.google.com/p/jslibs/source/checkout jslibs sources]
 # Save your embbeded script in the file _jslibs/src/jshost/embeddedBootstrapScript.js_
 # [jslibsBuild Compile jslibs] (or only jshost if jslibs has already been compiled once)
}}}
**/


//////////////////////////////////////////////////////////////////////////////////////////////


/**qa
	QA.ASSERTOP(host, 'has', 'path');
	QA.ASSERTOP(host, 'has', 'name');
	if ( host.name.indexOf('jshost') == 0 ) {

		QA.ASSERTOP(host, 'has', 'endSignal');
		QA.ASSERTOP(host, 'has', 'endSignalEvents');
	}
**/


