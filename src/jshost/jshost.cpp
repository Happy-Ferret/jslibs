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

#if defined(XP_WIN)
#define USE_NEDMALLOC
#endif


#include <jslibsModule.cpp>

#include "../jslang/handlePub.h"


static volatile bool disabledFree = false;


#ifdef USE_NEDMALLOC

#define NO_NED_NAMESPACE
#define NO_MALLINFO 1
#include "../../libs/nedmalloc/nedmalloc.h"

static NOALIAS void
nedfree_handlenull(void *mem) NOTHROW {

	if ( mem != NULL && !disabledFree )
		nedfree(mem);
}

static NOALIAS size_t
nedblksize_msize(void *mem) NOTHROW {

	return nedblksize(0, mem);
}

#endif // USE_NEDMALLOC


#define HOST_MAIN_ASSERT( CONDITION, ERROR_MESSAGE ) \
	JL_MACRO_BEGIN \
		if ( !(CONDITION) ) { \
			fprintf(stderr, "%s\n", (ERROR_MESSAGE)); \
			goto bad; \
		} \
	JL_MACRO_END


static const unsigned char embeddedBootstrapScript[] =
	#include "embeddedBootstrapScript.js.xdr.cres"
;

static volatile int32_t gEndSignalState = 0;
static JLCondHandler gEndSignalCond;
static JLMutexHandler gEndSignalLock;

JSBool
EndSignalGetter(JSContext *cx, JS::Handle<JSObject*> obj, JS::Handle<jsid> id, JS::MutableHandle<JS::Value> vp) {

	JL_IGNORE(id, obj);

	JL_CHK( JL_NativeToJsval(cx, (int32_t)gEndSignalState, vp) );

	return JS_TRUE;
	JL_BAD;
}

JSBool
EndSignalSetter(JSContext *cx, JS::Handle<JSObject*> obj, JS::Handle<jsid> id, JSBool strict, JS::MutableHandle<JS::Value> vp) {

	JL_IGNORE(strict, id, obj);

	int tmp;
	JL_CHK( JL_JsvalToNative(cx, vp, &tmp) );

	JLMutexAcquire(gEndSignalLock);
	gEndSignalState = tmp;
	JLCondBroadcast(gEndSignalCond);
	JLMutexRelease(gEndSignalLock);

	return JS_TRUE;
	JL_BAD;
}


#if defined(XP_WIN)

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

#elif defined(XP_UNIX)

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


struct EndSignalProcessEvent {
	
	ProcessEvent pe;
	bool cancel;
	jsval callbackFunction;
	JSObject *callbackFunctionThis;
};

S_ASSERT( offsetof(EndSignalProcessEvent, pe) == 0 );

static JSBool
EndSignalPrepareWait( volatile ProcessEvent *pe, JSContext *, JSObject * ) {
	
	EndSignalProcessEvent *upe = (EndSignalProcessEvent*)pe;

	upe->cancel = false;
	return JS_TRUE;
}

static void
EndSignalStartWait( volatile ProcessEvent *pe ) {

	EndSignalProcessEvent *upe = (EndSignalProcessEvent*)pe;

	JLMutexAcquire(gEndSignalLock);
	while ( gEndSignalState == 0 && !upe->cancel )
		JLCondWait(gEndSignalCond, gEndSignalLock);
	JLMutexRelease(gEndSignalLock);
}

static bool
EndSignalCancelWait( volatile ProcessEvent *pe ) {

	EndSignalProcessEvent *upe = (EndSignalProcessEvent*)pe;

	JLMutexAcquire(gEndSignalLock);
	upe->cancel = true;
	JLCondBroadcast(gEndSignalCond);
	JLMutexRelease(gEndSignalLock);

	return true;
}

static JSBool
EndSignalEndWait( volatile ProcessEvent *pe, bool *hasEvent, JSContext *cx, JSObject * ) {

	EndSignalProcessEvent *upe = (EndSignalProcessEvent*)pe;

	*hasEvent = (gEndSignalState != 0);

	if ( !*hasEvent )
		return JS_TRUE;

	if ( JSVAL_IS_VOID( upe->callbackFunction ) )
		return JS_TRUE;

	jsval rval;
	JL_CHK( JS_CallFunctionValue(cx, upe->callbackFunctionThis, upe->callbackFunction, 0, NULL, &rval) );

	return JS_TRUE;
	JL_BAD;
}

JSBool
EndSignalEvents(JSContext *cx, unsigned argc, jsval *vp) {

	JL_DEFINE_ARGS;

	JL_ASSERT_ARGC_RANGE(0, 1);

	EndSignalProcessEvent *upe;
	JL_CHK( HandleCreate(cx, JLHID(pev), &upe, NULL, JL_RVAL) );
	upe->pe.prepareWait = EndSignalPrepareWait;
	upe->pe.startWait = EndSignalStartWait;
	upe->pe.cancelWait = EndSignalCancelWait;
	upe->pe.endWait = EndSignalEndWait;

	if ( JL_ARG_ISDEF(1) ) {

		JL_ASSERT_ARG_IS_CALLABLE(1);
		JL_CHK( SetHandleSlot(cx, *JL_RVAL, 0, JL_OBJVAL) ); // GC protection only
		JL_CHK( SetHandleSlot(cx, *JL_RVAL, 1, JL_ARG(1)) ); // GC protection only

		upe->callbackFunctionThis = JSVAL_TO_OBJECT(JL_OBJVAL); // store "this" object.
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

int
HostStdin( void *, char *buffer, size_t bufferLength ) {

	if (unlikely( stdin_fileno == -1 ))
		stdin_fileno = fileno(stdin);
	return read(stdin_fileno, (void*)buffer, bufferLength);
}

int
HostStdout( void *, const char *buffer, size_t length ) {

	if (unlikely( stdout_fileno == -1 ))
		stdout_fileno = fileno(stdout);
	return write(stdout_fileno, buffer, length);
}

int
HostStderr( void *, const char *buffer, size_t length ) {

	if (unlikely( stderr_fileno == -1 ))
		stderr_fileno = fileno(stderr);
	return write(stderr_fileno, buffer, length);
}


//void NewScriptHook(JSContext *cx, const char *filename, unsigned lineno, JSScript *script, JSFunction *fun, void *callerdata) {
//	printf( "add - %s:%d - %s - %d - %p\n", filename, lineno, fun ? JS_GetFunctionName(fun):"", script->staticDepth, script );
//}
//void DestroyScriptHook(JSContext *cx, JSScript *script, void *callerdata) {
//	printf( "del - %s:%d - ? - %d - %p\n", script->filename, script->lineno, script->staticDepth, script );
//}


// Helps to detect memory leaks (alloc/free balance)
//#define DBG_ALLOC 1


#ifdef DBG_ALLOC

static volatile int32_t allocCount = 0;
static volatile int32_t allocAmount = 0;
static volatile int32_t freeCount = 0;
static volatile int32_t freeAmount = 0;

EXTERN_C void* jl_malloc_count( size_t size ) {
	JLAtomicIncrement(&allocCount);
	void *mem = malloc(size);
	ASSERT( mem );
	JLAtomicAdd(&allocAmount, msize(mem));
	return mem;
}

EXTERN_C void* jl_calloc_count( size_t num, size_t size ) {
	JLAtomicIncrement(&allocCount);
	void *mem = calloc(num, size);
	ASSERT( mem );
	JLAtomicAdd(&allocAmount, msize(mem));
	return mem;
}

EXTERN_C void* jl_memalign_count( size_t alignment, size_t size ) {
	JLAtomicIncrement(&allocCount);
	void *mem = memalign(alignment, size);
	ASSERT( mem );
	JLAtomicAdd(&allocAmount, msize(mem));
	return mem;
}

EXTERN_C void* jl_realloc_count( void *ptr, size_t size ) {

	size_t prev;
	if ( ptr == NULL ) {
		prev = 0;
		JLAtomicIncrement(&allocCount);
	} else {
		prev = msize(ptr);
	}

	void *mem = realloc(ptr, size);
	ASSERT( mem );

	if ( mem != ptr ) {

		JLAtomicAdd(&freeAmount, prev);
		JLAtomicAdd(&allocAmount, msize(mem));
	} else {

		JLAtomicAdd(&allocAmount, msize(mem) - prev);
	}
	return mem;
}

EXTERN_C size_t jl_msize_count( void *ptr ) {
	return msize(ptr);
}

EXTERN_C void jl_free_count( void *ptr ) {
	if ( ptr ) {
		JLAtomicIncrement(&freeCount);
		JLAtomicAdd(&freeAmount, msize(ptr));
	}
	free(ptr);
}

#endif // DBG_ALLOC


//////////////////////////////////////////////////////////////////////////////////////////////


/**qa
	QA.ASSERTOP(host, 'has', 'path');
	QA.ASSERTOP(host, 'has', 'name');
	if ( host.name.indexOf('jshost') == 0 ) {

		QA.ASSERTOP(host, 'has', 'endSignal');
		QA.ASSERTOP(host, 'has', 'endSignalEvents');
	}
**/

int main(int argc, char* argv[]) { // see |int wmain(int argc, wchar_t* argv[])| for wide char

//	BOOL st = SetProcessAffinityMask(GetCurrentProcess(), 1);

#ifdef XP_WIN
	// enable low fragmentation heap
	HANDLE heap = GetProcessHeap();
	ULONG enable = 2;
	HeapSetInformation(heap, HeapCompatibilityInformation, &enable, sizeof(enable));
#endif // XP_WIN

	int exitValue;
	jsval rval;
	jsval arguments;

	JSContext *cx = NULL;

	uint32_t maxMem = (uint32_t)-1; // by default, there are no limit
	uint32_t maxAlloc = (uint32_t)-1; // by default, there are no limit
	bool warningsToErrors = false;
	bool unsafeMode = false;
	bool compileOnly = false;
	float maybeGCInterval = 10; // seconds
	bool useFileBootstrapScript = false;
	const char *inlineScript = NULL;
	const char *scriptName = NULL;
#ifdef DEBUG
	bool debug = false;
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


	static const bool useJslibsMemoryManager =
#ifdef DBG_ALLOC
	false
#else
	unsafeMode
#endif // DBG_ALLOC
;

#if defined(XP_WIN) && defined(DEBUG) && defined(REPORT_MEMORY_LEAKS)
	if ( debug ) {
		_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
		_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG );
		_CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDERR );
	}
#endif


	if ( useJslibsMemoryManager ) {

		#ifdef HAS_JL_ALLOCATORS
		#ifdef USE_NEDMALLOC
		jl_malloc = nedmalloc;
		jl_calloc = nedcalloc;
		jl_memalign = nedmemalign;
		jl_realloc = nedrealloc;
		jl_msize = nedblksize_msize;
		jl_free = nedfree_handlenull;
		#endif // USE_NEDMALLOC
		#endif // HAS_JL_ALLOCATORS


		#ifdef DBG_ALLOC
		jl_malloc = jl_malloc_count;
		jl_calloc = jl_calloc_count;
		jl_memalign = jl_memalign_count;
		jl_realloc = jl_realloc_count;
		jl_msize = jl_msize_count;
		jl_free = jl_free_count;
		#endif // DBG_ALLOC
		

		JL_CHK( InitializeMemoryManager(&jl_malloc, &jl_calloc, &jl_memalign, &jl_realloc, &jl_msize, &jl_free) );
		

	// jslibs and spidermonkey allocator should be the same, else JL_NewString() and JL_NewUCString() should be fixed !
		#ifdef HAS_JL_ALLOCATORS

		js_jl_malloc = jl_malloc;
		js_jl_calloc = jl_calloc;
		js_jl_realloc = jl_realloc;
		js_jl_free = jl_free;
	
		#endif // HAS_JL_ALLOCATORS

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

		js_jl_malloc = jl_malloc;
		js_jl_calloc = jl_calloc;
		js_jl_realloc = jl_realloc;
		js_jl_free = jl_free;

		#endif // DBG_ALLOC
	}


//	setvbuf(stderr, pBuffer, mode, buffer_size);
	
//	JS_Init();
	
	cx = CreateHost(maxMem, maxAlloc, (uint32_t)(maybeGCInterval * 1000));
	HOST_MAIN_ASSERT( cx != NULL, "Unable to initialize the JavaScript engine." );

#ifdef DEBUG	
	if ( debug )
		JS_SetOptions(cx, JS_GetOptions(cx) & ~(JSOPTION_TYPE_INFERENCE));
#endif // DEBUG	

	HostPrivate *hpv;
	hpv = JL_GetHostPrivate(cx);

	// custom memory allocators are transfered to modules through the HostPrivate structure:
	hpv->alloc.malloc = jl_malloc;
	hpv->alloc.calloc = jl_calloc;
	hpv->alloc.memalign = jl_memalign;
	hpv->alloc.realloc = jl_realloc;
	hpv->alloc.msize = jl_msize;
	hpv->alloc.free = jl_free;

	JS_SetOptions(cx, JS_GetOptions(cx) | (warningsToErrors ? JSOPTION_WERROR : 0) ); // default, may be disabled in InitHost()

	JL_CHKM( InitHost(cx, unsafeMode, HostStdin, HostStdout, HostStderr, NULL), E_HOST, E_INIT ); // "Unable to initialize the host."

	gEndSignalCond = JLCondCreate();
	gEndSignalLock = JLMutexCreate();

#if defined(XP_WIN)
	JL_CHKM( SetProcessShutdownParameters(0x100, SHUTDOWN_NORETRY), E_HOST, E_INTERNAL );
	JL_CHKM( SetConsoleCtrlHandler(Interrupt, TRUE) != 0, E_HOST, E_INTERNAL );
#elif defined(XP_UNIX)
	signal(SIGINT, Interrupt);
	signal(SIGTERM, Interrupt);
	signal(SIGKILL, Interrupt);
#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif

	scriptName = *argumentVector;

	//JL_CHKM( inlineScript != NULL || scriptName != NULL || useFileBootstrapScript || sizeof(embeddedBootstrapScript)-1 > 0, E_SCRIPT, E_NOTFOUND ); // "No script specified."

	if ( !(inlineScript != NULL || scriptName != NULL || useFileBootstrapScript || sizeof(embeddedBootstrapScript)-1 > 0) ) {
		
		JL_WARN( E_SCRIPT, E_NOTFOUND );
	}


	char hostFullPath[PATH_MAX];

#if defined(XP_WIN)
// get hostpath and hostname
	HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(NULL);
	JL_CHKM( GetModuleFileName(hInstance, hostFullPath, sizeof(hostFullPath)) != 0, E_HOST, E_INTERNAL ); // "Unable to GetModuleFileName."
#elif defined(XP_UNIX)
	jl::GetAbsoluteModulePath(hostFullPath, sizeof(hostFullPath), argv[0]);
	JL_CHKM( hostFullPath[0] != '\0', E_HOST, E_INTERNAL ); // "Unable to get module FileName."
//	int len = readlink("/proc/self/exe", moduleFileName, sizeof(moduleFileName)); // doc: readlink does not append a NUL character to buf.
//	moduleFileName[len] = '\0';
//	strcpy(hostFullPath, argv[0]);
#else
	#error NOT IMPLEMENTED YET	// (TBD)
#endif

	char *hostName;
	hostName = strrchr(hostFullPath, PATH_SEPARATOR);
	JL_CHK( hostName != NULL );
	hostName += 1;
	int hostPathLength;
	hostPathLength = hostName-hostFullPath;

	char hostPath[PATH_MAX];
	strncpy(hostPath, hostFullPath, hostPathLength);
	hostPath[hostPathLength] = '\0';

	JSObject *hostObj;
	hostObj = JL_GetHostPrivate(cx)->hostObject;

	JL_CHK( JL_NativeToProperty(cx, hostObj, JLID(cx, name), hostName) );
	JL_CHK( JL_NativeToProperty(cx, hostObj, JLID(cx, path), hostPath) );

	JL_CHK( JL_NativeVectorToJsval(cx, argumentVector, argc - (argumentVector-argv), arguments) );
	JL_CHK( JS_SetPropertyById(cx, hostObj, JLID(cx, arguments), &arguments) );

	JL_CHK( JS_DefineProperty(cx, hostObj, "endSignal", JSVAL_VOID, EndSignalGetter, EndSignalSetter, JSPROP_SHARED) ); // https://developer.mozilla.org/en/SpiderMonkey/JSAPI_Reference/JS_GetPropertyAttributes
	JL_CHK( JS_DefineFunction(cx, hostObj, "endSignalEvents", EndSignalEvents, 1, 0) );

#ifdef DBG_ALLOC
	struct Tmp {
		static JSBool dbgAllocGetter(JSContext *cx, JSObject *, jsid, jsval *vp) {
			return JL_NativeToJsval(cx, (int32_t)allocAmount, vp);
		}
	};
	JL_CHK( JS_DefineProperty(cx, hostObj, "dbgAlloc", JSVAL_VOID, Tmp::dbgAllocGetter, NULL, JSPROP_SHARED) );
#endif // DBG_ALLOC

	rval = JSVAL_VOID;


	// embedded bootstrap script

	if ( sizeof(embeddedBootstrapScript)-1 > 0 ) {

		uint32_t prevOpt = JS_SetOptions(cx, JS_GetOptions(cx) & ~JSOPTION_DONT_REPORT_UNCAUGHT); // report uncautch exceptions !
		JSScript *script = JS_DecodeScript(cx, embeddedBootstrapScript, sizeof(embeddedBootstrapScript)-1, NULL, NULL); // -1 because sizeof("") == 1
		JL_CHK( script );
		JL_CHK( JS_ExecuteScript(cx, JL_GetGlobal(cx), script, &rval) );
		JS_SetOptions(cx, prevOpt);
	}


	// file bootstrap script

	if ( useFileBootstrapScript ) {

		char bootstrapFilename[PATH_MAX];
		strcpy(bootstrapFilename, hostFullPath);
		strcat(bootstrapFilename, ".js");
		JL_CHK( ExecuteScriptFileName(cx, bootstrapFilename, compileOnly, &rval) );
	}


	ASSERT( !JL_IsExceptionPending(cx) );


	JSBool executeStatus;
	executeStatus = JS_TRUE;


	// inline (command-line) script

	if ( inlineScript != NULL ) {

		executeStatus = ExecuteScriptText(cx, inlineScript, compileOnly, &rval);
	}


	// file script

	if ( scriptName != NULL && executeStatus == JS_TRUE ) {

		executeStatus = ExecuteScriptFileName(cx, scriptName, compileOnly, &rval);
	}


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
		FinalizeMemoryManager(!disabledFree, &jl_malloc, &jl_calloc, &jl_memalign, &jl_realloc, &jl_msize, &jl_free);
	}

	JS_SetGCCallback(JL_GetRuntime(cx), NULL);
	DestroyHost(cx, disabledFree);
	if ( !disabledFree ) {
	
		JS_ShutDown();
	}
	cx = NULL;


#if defined(XP_WIN)
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)&Interrupt, FALSE);
#elif defined(XP_UNIX)
	signal(SIGINT, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
	signal(SIGKILL, SIG_DFL);
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
		JS_SetGCCallback(JL_GetRuntime(cx), NULL);
		DestroyHost(cx, true);
	}
	JS_ShutDown();
	return EXIT_FAILURE;
}

#ifdef DBG_ALLOC

struct DBG_ALLOC_dummyClass {
	~DBG_ALLOC_dummyClass() { // we must count at exit, see "dynamic atexit destructor"

		fprintf(stderr, "\n{alloc:%d (%dB), leaks:%d (%dB)}\n", allocCount, allocAmount, allocCount - freeCount, allocAmount - freeAmount);
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
