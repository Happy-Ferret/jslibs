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

#include "js/GCAPI.h"

#include <jsprf.h> // JS_smprintf in ErrorReporter

#include <jslibsModule.h>

#include "../jslang/jslang.h"


// by default, we run in unsafe mode.
// mixing safe and unsafe is not allowed.
DLLAPI bool _unsafeMode = true;

DLLAPI jl_malloc_t jl_malloc = NULL;
DLLAPI jl_calloc_t jl_calloc = NULL;
DLLAPI jl_memalign_t jl_memalign = NULL;
DLLAPI jl_realloc_t jl_realloc = NULL;
DLLAPI jl_msize_t jl_msize = NULL;
DLLAPI jl_free_t jl_free = NULL;



JL_BEGIN_NAMESPACE

//////////////////////////////////////////////////////////////////////////////
// Threaded memory deallocator

ThreadedAllocator *ThreadedAllocator::_owner = NULL;

Allocators ThreadedAllocator::_base;

volatile bool ThreadedAllocator::_skipCleanup;

// block-to-free chain
void *ThreadedAllocator::_head;

// thread stats
volatile int32_t ThreadedAllocator::_headLength;
volatile int ThreadedAllocator::_load;

// thread handler
JLThreadHandler ThreadedAllocator::_memoryFreeThread;

volatile ThreadedAllocator::MemThreadAction ThreadedAllocator::_threadAction;
JLSemaphoreHandler ThreadedAllocator::_memoryFreeThreadSem;

bool ThreadedAllocator::_canTriggerFreeThread;


ALWAYS_INLINE void FASTCALL
ThreadedAllocator::freeHead() {

	_headLength = 0;

	void *next = _head;
	void *it = *(void**)next;
	*(void**)next = NULL;

	while ( it ) {

		ASSERT( it != *(void**)it );
		next = *(void**)it;
		_base.free(it);
		it = next;
	}
}

// the thread proc
JLThreadFuncDecl
ThreadedAllocator::memoryFreeThreadProc( void* ) {

	for (;;) {

		_canTriggerFreeThread = true;
		if ( JLSemaphoreAcquire(_memoryFreeThreadSem) == JLOK ) {
			switch ( _threadAction ) {
				case memThreadExit:
					goto end;
				case memThreadProcess:
					break;
			}
		}

		Sleep(5);

		if ( !_skipCleanup )
			for ( _load = 1; _headLength; ++_load )
				freeHead();
	}
	
end:
	_canTriggerFreeThread = false;
	JLThreadExit(0);
	return 0;
}



RESTRICT_DECL void*
ThreadedAllocator::_malloc( size_t size ) {

	if (likely( size >= sizeof(void*) ))
		return _base.malloc(size);
	return _base.malloc(sizeof(void*));
}

RESTRICT_DECL void* 
ThreadedAllocator::_calloc( size_t num, size_t size ) {

	size *= num;
	if (likely( size >= sizeof(void*) ))
		return _base.calloc(size, 1);
	return _base.calloc(sizeof(void*), 1);
}

RESTRICT_DECL void* 
ThreadedAllocator::_memalign( size_t alignment, size_t size ) {

	if (likely( size >= sizeof(void*) ))
		return _base.memalign(alignment, size);
	return _base.memalign(alignment, sizeof(void*));
}

void*
ThreadedAllocator::_realloc( void *ptr, size_t size ) {

	if (likely( size >= sizeof(void*) ))
		return _base.realloc(ptr, size);
	return _base.realloc(ptr, sizeof(void*));
}

size_t 
ThreadedAllocator::_msize( void *ptr ) {

	return _base.msize(ptr);
}

void
ThreadedAllocator::_free( void *ptr ) {
	
	if (unlikely( ptr == NULL ))
		return;

	ASSERT( ptr > (void*)0x1000 );

	if (unlikely( _load >= MAX_LOAD || _base.msize(ptr) >= BIG_ALLOC )) { // if blocks is big OR too many things to free, the thread can not keep pace.

		_base.free(ptr);
		return;
	}

	*(void**)ptr = _head;
	_head = ptr;
	JLAtomicIncrement(&_headLength);

	if ( _canTriggerFreeThread ) {
		
		_canTriggerFreeThread = false;
		_threadAction = memThreadProcess;
		ASSERT( JLSemaphoreOk(_memoryFreeThreadSem) );
		JLSemaphoreRelease(_memoryFreeThreadSem);
	}
}


ThreadedAllocator::~ThreadedAllocator() {
		
	_threadAction = memThreadExit;
	JLSemaphoreRelease(_memoryFreeThreadSem);
	// beware: Never use JLThreadCancel on a thread that call free().
	JLThreadWait(_memoryFreeThread);
	JLThreadFree(&_memoryFreeThread);
	JLSemaphoreFree(&_memoryFreeThreadSem);

	if ( !_skipCleanup ) {

		freeHead();
		_base.free(_head);
	}

	_current = _base;
	_owner = NULL;
}

ThreadedAllocator::ThreadedAllocator(Allocators &allocators)
: _current(allocators) {
		
	ASSERT( _owner == NULL );
	_owner = this;

	_base = allocators;
	allocators = Allocators(_malloc, _calloc, _memalign, _realloc, _msize, _free);
		
	_skipCleanup = false;
	_load = 0;
	_headLength = 0;
	_head = NULL;
	_memoryFreeThreadSem = JLSemaphoreCreate(0);
	_memoryFreeThread = JLThreadStart(memoryFreeThreadProc);
	_canTriggerFreeThread = false;

	_free(_malloc(0)); // make head non-NULL (TBD) why?
}


//////////////////////////////////////////////////////////////////////////////
// CountedAlloc

CountedAlloc *CountedAlloc::_owner = NULL;

volatile int32_t CountedAlloc::_allocCount;
volatile int32_t CountedAlloc::_allocAmount;
volatile int32_t CountedAlloc::_freeCount;
volatile int32_t CountedAlloc::_freeAmount;

Allocators CountedAlloc::_base;

RESTRICT_DECL void*
CountedAlloc::_malloc( size_t size ) {
	
	JLAtomicIncrement(&CountedAlloc::_allocCount);
	void *mem = _base.malloc(size);
	ASSERT( mem );
	JLAtomicAdd(&CountedAlloc::_allocAmount, _base.msize(mem));
	return mem;
}

RESTRICT_DECL void*
CountedAlloc::_calloc( size_t num, size_t size ) {
	
	JLAtomicIncrement(&CountedAlloc::_allocCount);
	void *mem = _base.calloc(num, size);
	ASSERT( mem );
	JLAtomicAdd(&CountedAlloc::_allocAmount, _base.msize(mem));
	return mem;
}

RESTRICT_DECL void*
CountedAlloc::_memalign( size_t alignment, size_t size ) {
	
	JLAtomicIncrement(&CountedAlloc::_allocCount);
	void *mem = _base.memalign(alignment, size);
	ASSERT( mem );
	JLAtomicAdd(&CountedAlloc::_allocAmount, _base.msize(mem));
	return mem;
}

void*
CountedAlloc::_realloc( void *ptr, size_t size ) {

	size_t prev;
	if ( ptr == NULL ) {

		prev = 0;
		JLAtomicIncrement(&CountedAlloc::_allocCount);
	} else {

		prev = _base.msize(ptr);
	}

	void *mem = _base.realloc(ptr, size);
	ASSERT( mem );

	if ( mem != ptr ) {

		JLAtomicAdd(&CountedAlloc::_freeAmount, prev);
		JLAtomicAdd(&CountedAlloc::_allocAmount, _base.msize(mem));
	} else {

		JLAtomicAdd(&CountedAlloc::_allocAmount, _base.msize(mem) - prev);
	}
	return mem;
}

size_t
CountedAlloc::_msize( void *ptr ) {

	return _base.msize(ptr);
}

void
CountedAlloc::_free( void *ptr ) {

	if ( ptr ) {

		JLAtomicIncrement(&CountedAlloc::_freeCount);
		JLAtomicAdd(&CountedAlloc::_freeAmount, _base.msize(ptr));
	}
	_base.free(ptr);
}

CountedAlloc::CountedAlloc(Allocators &current)
: _current(current) {
	
	ASSERT( _owner == NULL );
	_owner = this;

	_allocCount = 0;
	_allocAmount = 0;
	_freeCount = 0;
	_freeAmount = 0;
		
	_base = _current;
	_current = Allocators(_malloc, _calloc, _memalign, _realloc, _msize, _free);
}

CountedAlloc::~CountedAlloc() {

	fprintf(stderr, "\n{alloc:%d (%dB), leaks:%d (%dB)}\n", _allocCount, _allocAmount, _allocCount - _freeCount, _allocAmount - _freeAmount);
		
	_current = _base;
	_owner = NULL;
}


//////////////////////////////////////////////////////////////////////////////
// WatchDog

bool 
WatchDog::interruptCallback(JSContext *cx) {

	JSRuntime *rt = JL_GetRuntime(cx);

//	JSInterruptCallback tmp = JS_SetInterruptCallback(rt, NULL);

	//For a collection to be carried out incrementally the following conditions must be met:
	// - The collection must be run by calling JS::IncrementalGC() rather than JS_GC().
	// - The GC mode must have been set to JSGC_MODE_INCREMENTAL with JS_SetGCParameter().
	// - All native objects that have their own trace hook must indicate that they implement read and write barriers with the JSCLASS_IMPLEMENTS_BARRIERS flag.
 
	JS::IncrementalGC(rt, JS::gcreason::NO_REASON);
	bool fin = !JS::IsIncrementalGCInProgress(rt);

	ASSERT( fin );

	//JS_MaybeGC(cx);
	//JS_GC(rt);
	//JS::IncrementalGC(rt, JS::gcreason::MAYBEGC);

//	JS_SetInterruptCallback(rt, tmp);
	return true;
}


JLThreadFuncDecl
WatchDog::watchDogThreadProc(void *threadArg) {

	WatchDog &watchDog(*static_cast<WatchDog*>(threadArg));

	for (;;) {

		// used as a breakable Sleep instead of SleepMilliseconds (see SandboxEval).
		if ( JLSemaphoreAcquire(watchDog._watchDogSemEnd, watchDog._maybeGCInterval) != JLTIMEOUT )
			break;
		JSRuntime *rt = watchDog._hostRuntime.runtime();

		ASSERT( JS_GetInterruptCallback(rt) );
		JS_RequestInterruptCallback(rt);
	}
	JLThreadExit(0);
	return 0;
}


WatchDog::WatchDog(HostRuntime &hostRuntime, uint32_t maybeGCInterval)
: _hostRuntime(hostRuntime), _maybeGCInterval(maybeGCInterval) {
}

bool
WatchDog::start() {

	if ( _maybeGCInterval ) {

		JSContext *cx = _hostRuntime.context();
		JSInterruptCallback prevOperationCallback = JS_SetInterruptCallback(_hostRuntime.runtime(), interruptCallback);
		ASSERT( prevOperationCallback == NULL );
		_watchDogSemEnd = JLSemaphoreCreate(0);
		_watchDogThread = JLThreadStart(watchDogThreadProc, this);
		JL_ASSERT( JLSemaphoreOk(_watchDogSemEnd) && JLThreadOk(_watchDogThread), E_HOST, E_CREATE ); // "Unable to create the GC thread."
	}	
	return true;
	JL_BAD;
}

bool
WatchDog::stop() {

	// beware: it is important to destroy the watchDogThread BEFORE destroying the cx !!!

	if ( _maybeGCInterval ) {

		JSInterruptCallback prev_interruptCallback = JS_SetInterruptCallback(_hostRuntime.runtime(), NULL);
		ASSERT( prev_interruptCallback == interruptCallback );
		JLSemaphoreRelease(_watchDogSemEnd);
		JLThreadWait(_watchDogThread);
		JLThreadFree(&_watchDogThread);
		JLSemaphoreFree(&_watchDogSemEnd);
	}
	return true;
}



//////////////////////////////////////////////////////////////////////////////
// HostRuntime

static bool
_globalClass_lazy_enumerate(JSContext *cx, JS::HandleObject obj) {

	return JS_EnumerateStandardClasses(cx, obj);
}

static bool
_globalClass_lazy_resolve(JSContext *cx, JS::HandleObject obj, JS::HandleId id, unsigned flags, JS::MutableHandleObject objp) {

	bool resolved;

	if (!JS_ResolveStandardClass(cx, obj, id, &resolved))
		return false;

	if (resolved) {

		objp.set(obj);
		return true;
	}
	return true;
}

const JSClass HostRuntime::_globalClass_lazy = {
	NAME_GLOBAL_CLASS, JSCLASS_GLOBAL_FLAGS,
	JS_PropertyStub, JS_DeletePropertyStub,
	JS_PropertyStub, JS_StrictPropertyStub,
	_globalClass_lazy_enumerate, (JSResolveOp)_globalClass_lazy_resolve,
	JS_ConvertStub, nullptr,
    nullptr, nullptr, nullptr,
	JS_GlobalObjectTraceHook
};

const JSClass HostRuntime::_globalClass = {
	NAME_GLOBAL_CLASS, JSCLASS_GLOBAL_FLAGS,
	JS_PropertyStub, JS_DeletePropertyStub,
	JS_PropertyStub, JS_StrictPropertyStub,
	JS_EnumerateStub, JS_ResolveStub,
	JS_ConvertStub, nullptr,
    nullptr, nullptr, nullptr,
	JS_GlobalObjectTraceHook
};


void
HostRuntime::setJSEngineAllocators(Allocators allocators) {

	js_jl_malloc = allocators.malloc;
	js_jl_calloc = allocators.calloc;
	js_jl_realloc = allocators.realloc;
	js_jl_free = allocators.free;
}


void
HostRuntime::errorReporterBasic(JSContext *cx, const char *message, JSErrorReport *report) {

	JL_IGNORE( cx );
	if ( !report )
		fprintf(stderr, "%s\n", message);
	else
		fprintf(stderr, "%s (%s:%d)\n", message, report->filename ? report->filename : "<no filename>", (unsigned int)report->lineno);
}


HostRuntime::HostRuntime(Allocators allocators, uint32_t maybeGCIntervalMs)
: _allocators(allocators), rt(NULL), cx(NULL), _isEnding(false), _skipCleanup(false), _watchDog(*MOZ_THIS_IN_INITIALIZER_LIST(), maybeGCIntervalMs) {
}

 
bool
HostRuntime::create( uint32_t maxMem, uint32_t maxAlloc, size_t nativeStackQuota, bool lazyStandardClasses ) {

	rt = JS_NewRuntime(maxAlloc, JS_USE_HELPER_THREADS); // JSGC_MAX_MALLOC_BYTES
	JL_CHK( rt );

	// Number of JS_malloc bytes before last ditch GC.
	// ASSERT( JS_GetGCParameter(rt, JSGC_MAX_MALLOC_BYTES) == maxAlloc ); // JS_SetGCParameter(rt, JSGC_MAX_MALLOC_BYTES, maxAlloc);

	// doc: maxMem specifies the number of allocated bytes after which garbage collection is run. Maximum nominal heap before last ditch GC.
	JS_SetGCParameter(rt, JSGC_MAX_BYTES, maxMem); 
	//JS_SetNativeStackQuota(rt, 128 * sizeof(size_t) * 1024); // doc. To disable stack size checking pass 0.
	JS_SetNativeStackQuota(rt, 0);

	//JS::DisableGenerationalGC(rt);

	//JS_SetGCParametersBasedOnAvailableMemory

	//JS_SetNativeStackQuota(rt, nativeStackQuota); // doc: 0:disabled

	cx = JS_NewContext(rt, 8192); // set the chunk size of the stack pool to 8192. see http://groups.google.com/group/mozilla.dev.tech.js-engine/browse_thread/thread/be9f404b623acf39/9efdfca81be99ca3
	JL_CHK( cx ); //, "unable to create the context." );

	JS_BeginRequest(cx);

	// Info: Increasing JSContext stack size slows down my scripts:
	//   http://groups.google.com/group/mozilla.dev.tech.js-engine/browse_thread/thread/be9f404b623acf39/9efdfca81be99ca3

	JS_SetErrorReporter(cx, errorReporterBasic);
	JS::RuntimeOptionsRef(cx)
		.setIon(true)
		.setAsmJS(true)
	;

	JS::ContextOptionsRef(cx)
		// doc. VarObjFix is recommended.  Without it, the two scripts "x = 1" and "var x = 1", where no variable x is in scope, do two different things.
		//      The former creates a property on the global object.  The latter creates a property on obj.  With this flag, both create a global property.
		.setVarObjFix(true)
//		.setCloneSingletons(true)
	;

	//JS_SetNativeStackQuota(cx, DEFAULT_MAX_STACK_SIZE); // see https://developer.mozilla.org/En/SpiderMonkey/JSAPI_User_Guide
	JS_SetGCParameter(rt, JSGC_MODE, JSGC_MODE_INCREMENTAL); // != JSGC_MODE_GLOBAL


	// JSOPTION_ANONFUNFIX: https://bugzilla.mozilla.org/show_bug.cgi?id=376052 
	// JS_SetOptions doc: https://developer.mozilla.org/en/SpiderMonkey/JSAPI_Reference/JS_SetOptions

	// beware: avoid using JSOPTION_COMPILE_N_GO here.

	{

	JS::CompartmentOptions options;
	options
		.setVersion(JSVERSION_LATEST)
		.setInvisibleToDebugger(false)
		.setMergeable(false)
	;

	JS::RootedObject globalObject(cx, JS_NewGlobalObject(cx, lazyStandardClasses ? &_globalClass_lazy : &_globalClass, NULL, JS::DontFireOnNewGlobalHook, options));
	JL_CHK( globalObject ); // "unable to create the global object." );

	// set globalObject as current global object.
	JL_CHK( JS_EnterCompartment(cx, globalObject) == nullptr );

	JL_CHK( JS_InitStandardClasses(cx, globalObject) );
	JL_CHK( JS_DefineDebuggerObject(cx, globalObject) ); // doc: https://developer.mozilla.org/en/SpiderMonkey/JS_Debugger_API_Guide
	JL_CHK( JS_InitReflect(cx, globalObject) );
	#ifdef JS_HAS_CTYPES
	JL_CHK( JS_InitCTypesClass(cx, globalObject) );
	#endif

	JS_FireOnNewGlobalObject(cx, globalObject);

	}

	JL_CHK( _watchDog.start() );

	return true;
	JL_BAD;
}


bool
HostRuntime::destroy(bool skipCleanup) {

	ASSERT( _isEnding == false );
	_isEnding = true;
	_skipCleanup = skipCleanup;

	JL_CHK( _watchDog.stop() );

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
		
	// see create()
	JS_LeaveCompartment(cx, NULL);
	JS_EndRequest(cx);
	JS_DestroyContext(cx);
	cx = NULL;

	#ifdef DEBUG
	JS_DumpHeap(rt, fopen("dump.txt", "w"), NULL, JSTRACE_OBJECT, NULL, 1, NULL);
	#endif

	JS_DestroyRuntime(rt);
	rt = NULL;

	return true;

bad:
	// on error, do the minimum.
	if ( cx ) {

		JS_LeaveCompartment(cx, NULL);
		JS_DestroyContext(cx);
		JS_DestroyRuntime(rt);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////////
// ModuleManager


ModuleManager::ModuleManager(HostRuntime &hostRuntime)
: _hostRuntime(hostRuntime), _moduleCount(0) {
}


bool
ModuleManager::loadModule(const char *libFileName, JS::HandleObject obj, JS::MutableHandleValue rval) {

	JSContext *cx = _hostRuntime.context();

	JLLibraryHandler moduleHandle = JLDynamicLibraryNullHandler;
	JL_ASSERT( libFileName != NULL && *libFileName != '\0', E_ARG, E_NUM(1), E_DEFINED );
	JL_ASSERT( _moduleCount < MAX_MODULES, E_MODULE, E_COUNT, E_MAX, E_NUM(MAX_MODULES) );

	moduleHandle = JLDynamicLibraryOpen(libFileName);
	if ( !JLDynamicLibraryOk(moduleHandle) ) {

		JL_SAFE_BEGIN
		char errorBuffer[256];
		JLDynamicLibraryLastErrorMessage( errorBuffer, sizeof(errorBuffer) );
		JL_WARN( E_OS, E_OPERATION, E_DETAILS, E_STR(errorBuffer), E_COMMENT(libFileName) );
		JL_SAFE_END

		rval.setBoolean(false);
		return true;
	}

	ModuleInitFunction moduleInit;
	moduleInit = (ModuleInitFunction)JLDynamicLibrarySymbol(moduleHandle, NAME_MODULE_INIT);
	JL_ASSERT( moduleInit, E_MODULE, E_NAME(libFileName), E_INIT ); // "Invalid module."

	//if ( !moduleInit ) {
	//	JL_SAFE_BEGIN
	//	char errorBuffer[256];
	//	JLDynamicLibraryLastErrorMessage( errorBuffer, sizeof(errorBuffer) );
	//	JL_WARN( E_OS, E_OPERATION, E_DETAILS, E_STR(errorBuffer), E_COMMENT(libFileName) );
	//	JL_SAFE_END
	//	rval.setBoolean(false);
	//	return true;
	//}



	moduleId_t moduleId;
	moduleId = reinterpret_cast<moduleId_t>(moduleInit); // JLDynamicLibraryId(module); // module unique ID

	Module &module = moduleSlot(moduleId);
	
	if ( !isSlotFree(module) ) {  // already loaded ?
		
		ASSERT( module.moduleHandle == moduleHandle );
		JLDynamicLibraryClose(&moduleHandle);
		rval.setNull();
		return true;
	}

	ASSERT( module.moduleHandle == JLDynamicLibraryNullHandler );

	module.moduleHandle = moduleHandle;
	module.moduleId = moduleId;

	if ( !moduleInit(cx, obj) ) {

		module = Module();

		JL_CHK( !JL_IsExceptionPending(cx) );
		char filename[PATH_MAX];
		JLDynamicLibraryName((void*)moduleInit, filename, sizeof(filename));
		JL_ERR( E_MODULE, E_NAME(filename), E_INIT );
	}
	
	//JL_CHK( JL_NewNumberValue(cx, uid, JL_RVAL) ); // really needed ? yes, UnloadModule will need this ID, ... but UnloadModule is too complicated to implement and will never exist.
	rval.setObject(*obj);
	
	++_moduleCount;
	return true;

bad:
	if ( JLDynamicLibraryOk(moduleHandle) )
		JLDynamicLibraryClose(&moduleHandle);
	return false;

}

bool
ModuleManager::releaseModules() {

	JSContext *cx = _hostRuntime.context();

	for ( uint16_t i = 0; i < MAX_MODULES; ++i ) {
		
		Module &module = _moduleList[i];
		if ( module.moduleId ) {

			ModuleReleaseFunction moduleRelease = (ModuleReleaseFunction)JLDynamicLibrarySymbol(module.moduleHandle, NAME_MODULE_RELEASE);
			if ( moduleRelease != NULL ) {

				if ( !moduleRelease(cx) ) {

					char filename[PATH_MAX];
					JLDynamicLibraryName((void*)moduleRelease, filename, sizeof(filename));
					JL_WARN( E_MODULE, E_NAME(filename), E_FIN ); // "Fail to release module \"%s\".", filename
				}
			}
		}
	}

	return true;
	JL_BAD;
}

void
ModuleManager::freeModules(bool skipCleanup) {

	// Beware: because JS engine allocate memory from the DLL, all memory must be disallocated before releasing the DLL

	for ( uint16_t i = 0; i < MAX_MODULES; ++i ) {
		
		ModuleManager::Module &module = _moduleList[i];
		if ( module.moduleId ) {

			ModuleFreeFunction moduleFree = (ModuleFreeFunction)JLDynamicLibrarySymbol(module.moduleHandle, NAME_MODULE_FREE);
			if ( moduleFree != NULL ) {
		
				moduleFree(skipCleanup, module.privateData);
			}

			//#ifndef DEBUG // else the memory block was allocated in a DLL that was unloaded prior to the _CrtMemDumpAllObjectsSince() call.
			if ( JLDynamicLibraryOk(module.moduleHandle) )
				JLDynamicLibraryClose(&module.moduleHandle);
			//#endif

			#ifdef DEBUG
			module = Module();
			#endif
		}
	}
}




//////////////////////////////////////////////////////////////////////////////
// host


BEGIN_CLASS( host )

/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER( unsafeMode ) {

	JL_IGNORE( id, obj );

	JL_CHK( jl::setValue(cx, vp, Host::getHost(cx).unsafeMode()) );
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER( jsVersion ) {

	JL_IGNORE( id, obj );

	JL_CHK( jl::setValue(cx, vp, JS_GetVersion(cx)) ); // btw, see JS_GetImplementationVersion()
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER( incrementalGarbageCollector ) {

	JL_IGNORE( id, obj );

	uint32_t gcMode = JS_GetGCParameter(JL_GetRuntime(cx), JSGC_MODE);
	JL_CHK( jl::setValue(cx, vp, gcMode == JSGC_MODE_INCREMENTAL) ); // JSGC_MODE_GLOBAL
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( incrementalGarbageCollector ) {

	JL_IGNORE( strict, id, obj );

	bool incGc;
	JL_CHK( jl::getValue(cx, vp, &incGc) );
	JS_SetGCParameter(JL_GetRuntime(cx), JSGC_MODE, incGc ? JSGC_MODE_INCREMENTAL : JSGC_MODE_GLOBAL);
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME()
**/
DEFINE_FUNCTION( stdout ) {

	JL_DEFINE_ARGS;
	JL_RVAL.setUndefined();
	jl::BufString str;
	Host &host(Host::getHost(cx));
	for ( unsigned i = 0; i < argc; ++i ) {

		JL_CHK( jl::getValue(cx, JL_ARG(i+1), &str) );
		int status = host.stdIO().output(str.toData<const char *>(), str.length());
		JL_ASSERT_WARN( status != -1, E_HOST, E_INTERNAL, E_SEP, E_COMMENT("stdout"), E_WRITE );
	}
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME()
**/
DEFINE_FUNCTION( stderr ) {

	JL_DEFINE_ARGS;
	JL_RVAL.setUndefined();
	jl::BufString str;
	Host &host(Host::getHost(cx));
	for ( unsigned i = 0; i < argc; ++i ) {

		JL_CHK( jl::getValue(cx, JL_ARG(i+1), &str) );
		int status = host.stdIO().error(str.toData<const char *>(), str.length());
		JL_ASSERT_WARN( status != -1, E_HOST, E_INTERNAL, E_SEP, E_COMMENT("stderr"), E_WRITE );
	}
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $STR $INAME()
**/
DEFINE_FUNCTION( stdin ) {

	JL_DEFINE_ARGS;

	Host &host(Host::getHost(cx));

	char buffer[8192];
	int status = host.stdIO().input(buffer, COUNTOF(buffer));
	if ( status > 0 ) {
		
		JS::RootedString jsstr(cx, JS_NewStringCopyN(cx, buffer, status));
		JL_CHK( jsstr );
		JL_RVAL.setString(jsstr);
	} else {

		JL_WARN( E_HOST, E_INTERNAL, E_SEP, E_COMMENT("stdin"), E_READ );
		JL_RVAL.setString(JL_GetEmptyString(cx));
	}
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $OBJ | null $INAME()
**/
DEFINE_FUNCTION( loadModule ) {

	// JLLibraryHandler module = JLDynamicLibraryNullHandler;
	jl::BufString str;

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC(1);

	JL_CHK( jl::getValue(cx, JL_ARG(1), &str) );

	char libFileName[PATH_MAX];
	//jl::strncpy( libFileName, str.toData<const char *>(), str.length() ); // (TBD) use copyTo()
	str.copyTo(libFileName);
	libFileName[str.length()] = '\0';
	strcat( libFileName, DLL_EXT );
// MAC OSX: 	'@executable_path' ??

/*
	while (0) { // namespace management. Avoid using val ns = {}, loadModule.call(ns, '...');

		if ( JL_ARG_ISDEF(2) ) {

			if ( JSVAL_IS_OBJECT(JL_ARG(2)) ) {
				obj = JSVAL_TO_OBJECT(JL_ARG(2));
			} else {
				const char *ns;
				JL_CHK( jl::getValue(cx, &JL_ARG(2), &ns) );

				jsval existingNsVal;
				JL_CHK( JS_GetProperty(cx, obj, ns, &existingNsVal) );
				JSObject *nsObj;
				if ( existingNsVal == JSVAL_VOID ) {

					nsObj = JS_NewObject(cx, NULL, NULL, NULL);
					JL_CHK( JS_DefineProperty(cx, obj, ns, OBJECT_TO_JSVAL(nsObj), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT) ); // doc. On success, JS_DefineProperty returns true. If the property already exists or cannot be created, JS_DefineProperty returns false.
				} else {

					JL_ASSERT_OBJECT( existingNsVal );
					nsObj = JSVAL_TO_OBJECT( existingNsVal );
				}
				obj = nsObj;
			}
		}
	}
*/

	JL_CHK( Host::getHost(cx).moduleManager().loadModule(libFileName, JL_OBJ, JL_RVAL) );


//	JL_ASSERT( hpv, E_HOST, E_STATE, E_COMMENT("context private") );


/*
	JL_ASSERT( libFileName != NULL && *libFileName != '\0', E_ARG, E_NUM(1), E_DEFINED );

	module = JLDynamicLibraryOpen(libFileName);
	if ( !JLDynamicLibraryOk(module) ) {

		JL_SAFE_BEGIN
		char errorBuffer[256];
		JLDynamicLibraryLastErrorMessage( errorBuffer, sizeof(errorBuffer) );
		JL_WARN( E_OS, E_OPERATION, E_DETAILS, E_STR(errorBuffer), E_COMMENT(libFileName) );
		JL_SAFE_END

		JL_RVAL.setBoolean(false);
		return true;
	}

	Host &host = Host::getHost(cx);

	if ( host.moduleManager().hasModule(module) ) {

		JLDynamicLibraryClose(&module);
		JL_RVAL.setNull(); // already loaded
		return true;
	}

	uint32_t uid;
	uid = JLDynamicLibraryId(module); // module unique ID
	ModuleInitFunction moduleInit;
	moduleInit = (ModuleInitFunction)JLDynamicLibrarySymbol(module, NAME_MODULE_INIT);
	JL_ASSERT( moduleInit, E_MODULE, E_NAME(libFileName), E_INIT ); // "Invalid module."
	
	if ( !moduleInit(cx, JL_OBJ) ) {

		JL_CHK( !JL_IsExceptionPending(cx) );
		char filename[PATH_MAX];
		JLDynamicLibraryName((void*)moduleInit, filename, sizeof(filename));
		JL_ERR( E_MODULE, E_NAME(filename), E_INIT );
	}

	host.moduleManager().storeModule(module);
	
	//JL_CHK( JL_NewNumberValue(cx, uid, JL_RVAL) ); // really needed ? yes, UnloadModule will need this ID, ... but UnloadModule is too complicated to implement and will never exist.
	JL_RVAL.setObject(*JL_OBJ);

	return true;

bad:
	if ( JLDynamicLibraryOk(module) )
		JLDynamicLibraryClose(&module);
	return false;
*/

	return true;
	JL_BAD;
}


DEFINE_INIT() {

	JL_IGNORE( proto, cs );

	Host::getHost(cx).setHostObject(obj);

	return true;
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



//////////////////////////////////////////////////////////////////////////////
// Host

struct {
	JSExnType exn;
	const char *msg;
} static E_msg[] = {
		{ JSEXN_NONE, 0 },
	#define DEF( NAME, TEXT, EXN ) \
		{ EXN, TEXT },
	#include "jlerrors.msg"
	#undef DEF
};



const JSErrorFormatString *
Host::errorCallback(void *userRef, const char *, const unsigned) {

	return (JSErrorFormatString*)userRef;
}

void
Host::errorReporterBasic(JSContext *cx, const char *message, JSErrorReport *report) {

	JL_IGNORE( cx );
	if ( !report )
		fprintf(stderr, "%s\n", message);
	else
		fprintf(stderr, "%s (%s:%d)\n", message, report->filename ? report->filename : "<no filename>", (unsigned int)report->lineno);
}


void
Host::errorReporter(JSContext *cx, const char *message, JSErrorReport *report) {

	// trap JSMSG_OUT_OF_MEMORY error to avoid calling ErrorReporter_stdErrRouter() that may allocate memory that will lead to nested call.
	if (unlikely( report && report->errorNumber == JSMSG_OUT_OF_MEMORY )) {
		
		HostRuntime::errorReporterBasic(cx, message, report);
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
		size_t len = jl::min(strlen(STR), remaining); \
		jl::memcpy(buf, STR, len); \
		buf += len; \
	JL_MACRO_END

	#define fwrite(STR, SIZE, COUNT, FILE) \
	JL_MACRO_BEGIN \
		size_t remaining = sizeof(buffer)-(buf-buffer); \
		if ( remaining == 0 ) break; \
		size_t len = jl::min(size_t((SIZE)*(COUNT)), remaining); \
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

// copy-paste from /js/src/jscntxt.cpp (js::PrintError)
//	 ---8<---

		if (!report) {
		fprintf(file, "%s\n", message);
		fflush(file);
		return;
	}

	/* Conditionally ignore reported warnings. */
	if (JSREPORT_IS_WARNING(report->flags) && !reportWarnings)
		return;

	char *prefix = nullptr;
	if (report->filename)
		prefix = JS_smprintf("%s:", report->filename);
	if (report->lineno) {
		char *tmp = prefix;
		prefix = JS_smprintf("%s%u:%u ", tmp ? tmp : "", report->lineno, report->column);
		JS_free(cx, tmp);
	}
	if (JSREPORT_IS_WARNING(report->flags)) {
		char *tmp = prefix;
		prefix = JS_smprintf("%s%swarning: ",
								tmp ? tmp : "",
								JSREPORT_IS_STRICT(report->flags) ? "strict " : "");
		JS_free(cx, tmp);
	}

	/* embedded newlines -- argh! */
	const char *ctmp;
	while ((ctmp = strchr(message, '\n')) != 0) {
		ctmp++;
		if (prefix)
			fputs(prefix, file);
		fwrite(message, 1, ctmp - message, file);
		message = ctmp;
	}

	/* If there were no filename or lineno, the prefix might be empty */
	if (prefix)
		fputs(prefix, file);
	fputs(message, file);

	if (report->linebuf) {
		/* report->linebuf usually ends with a newline. */
		int n = strlen(report->linebuf);
		fprintf(file, ":\n%s%s%s%s",
				prefix,
				report->linebuf,
				(n > 0 && report->linebuf[n-1] == '\n') ? "" : "\n",
				prefix);
		n = report->tokenptr - report->linebuf;
		for (int i = 0, j = 0; i < n; i++) {
			if (report->linebuf[i] == '\t') {
				for (int k = (j + 8) & ~7; j < k; j++) {
					fputc('.', file);
				}
				continue;
			}
			fputc('.', file);
			j++;
		}
		fputc('^', file);
	}
	fputc('\n', file);
	fflush(file);
	JS_free(cx, prefix);

//	 ---8<---

	#undef fprintf
	#undef fputs
	#undef fwrite
	#undef fputc
	#undef fflush
	
	Host::getHost(cx).hostStderrWrite(buffer, buf-buffer);
}


bool
Host::hostStderrWrite(const char *message, size_t length) {

	JSContext *cx = _hostRuntime.context();

	JS::RootedObject globalObject(cx, JL_GetGlobal(cx));
	ASSERT( globalObject );
	AutoExceptionState autoEx(cx);

	// avoid reentrancy if stderr function rise an error.
	AutoErrorReporter autoErrorReporter(cx, JL_IS_SAFE ? errorReporterBasic : NULL);
		
	JS::RootedValue rval(cx);
	JS::RootedValue fct(cx);
	JS::RootedObject hostObj(cx, _hostObject);

	JL_CHK( JS_WrapObject(cx, &hostObj) );

	JL_CHK( jl::getProperty(cx, hostObj, JLID(cx, stderr), &fct) );
	JL_CHK( jl::isCallable(cx, fct) );
	JL_CHK( jl::call(cx, globalObject, fct, &rval, jl::strSpec(message, length)) ); // beware out of memory case !

	return true;
bad:
	autoEx.drop();
	return false;
}


Host::Host( HostRuntime &hr, StdIO &hostStdIO, bool unsafeMode )
: _hostRuntime(hr), _moduleManager(hr), _compatId(JL_HOST_VERSIONID), _unsafeMode(unsafeMode), _hostStdIO(hostStdIO), _objectProto(hr.runtime()), _hostObject(hr.runtime()), _ids() {

	::_unsafeMode = unsafeMode;
	Host::setHostAllocators(_hostRuntime.allocators());
	IFDEBUG( jl_free(js_malloc(256)) );
	IFDEBUG( js_free(jl_malloc(256)) );

	_ids.constructAll(hr.runtime());
	ASSERT( !JSID_IS_ZERO(_ids.get(0)) );
	ASSERT( !JSID_IS_ZERO(_ids.get(LAST_JSID-1)) );
}

// init the host for jslibs usage (modules, errors, ...)
bool
Host::create() {

	JL_SetRuntimePrivate(_hostRuntime.runtime(), this);

	JSContext *cx = _hostRuntime.context();

	JS::ContextOptionsRef(cx)
		.setStrictMode(_unsafeMode)
		.setExtraWarnings(!_unsafeMode)
	;
	
	JS_SetErrorReporter(cx, errorReporter);

	JS::RootedObject obj(cx, JL_GetGlobal(cx));
	ASSERT( obj != NULL ); // "Global object not found."

	_objectProto.set(JS_GetObjectPrototype(cx, obj));
	_objectClasp = JL_GetClass(_objectProto);
	
	// global functions & properties
	JL_CHKM( JS_DefinePropertyById(cx, obj, JLID(cx, global), OBJECT_TO_JSVAL(obj), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT), E_PROP, E_CREATE );

	ASSERT(cx && obj);
	INIT_CLASS( host );
	ASSERT( _objectProto );

	// init static modules (jslang)
	ModuleManager::Module &module = moduleManager().moduleSlot(jslangModuleId);
	ASSERT( moduleManager().isSlotFree(module) ); // free slot
	module.moduleHandle = JLDynamicLibraryNullHandler;
	module.moduleId = jslangModuleId;
	ASSERT( jslangModuleInit != (ModuleInitFunction)NULL);
	if ( !jslangModuleInit(cx, obj) )
		JL_ERR( E_MODULE, E_NAME("jslang"), E_INIT );

	return true;
	JL_BAD;
}


bool
Host::destroy(bool skipCleanup) {
	
	JSContext *cx = _hostRuntime.context();
		
	JL_CHK( _moduleManager.releaseModules() );

	ASSERT( jslangModuleRelease != (ModuleReleaseFunction)NULL );
	if ( !jslangModuleRelease(cx) ) {
		
		JL_WARN( E_MODULE, E_NAME("jslang"), E_FIN ); // "Fail to release static module jslang."
	}

	_classProtoCache.removeAll();
	_hostObject.set(NULL);
	_objectProto.set(NULL);

	if ( !skipCleanup ) {
	
		_ids.destructAll();
	}

	return true;
	JL_BAD;
}

void
Host::free(bool skipCleanup) {

	if ( !skipCleanup ) {

		_moduleManager.freeModules(skipCleanup);
		ASSERT( jslangModuleFree != (ModuleFreeFunction)NULL);
		jslangModuleFree(skipCleanup, NULL);
	}
}


bool
Host::report( bool isWarning, ... ) const {

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
	return JS_ReportErrorFlagsAndNumber( _hostRuntime.context(), isWarning ? JSREPORT_WARNING : JSREPORT_ERROR, errorCallback, (void*)&format, 0);

//bad:
//	va_end(vl);
//	return false;
}


bool
Host::setHostArguments( char **hostArgv, size_t hostArgc ) {

	JSContext *cx = _hostRuntime.context();
	JS::RootedValue argumentsVal(_hostRuntime.runtime());

	JL_CHK( jl::setVector(cx, &argumentsVal, hostArgv, hostArgc) );
	JL_CHK( JS_SetPropertyById(cx, _hostObject, JLID(cx, arguments), argumentsVal) );
	return true;
	JL_BAD;
}

bool
Host::setHostPath( const char *hostPath) {

	JSContext *cx = _hostRuntime.context();
	JL_CHK( jl::setProperty(cx, _hostObject, JLID(cx, path), hostPath) );
	return true;
	JL_BAD;
}

bool
Host::setHostName( const char *hostName ) {

	JSContext *cx = _hostRuntime.context();
	JL_CHK( jl::setProperty(cx, _hostObject, JLID(cx, name), hostName) );
	return true;
	JL_BAD;
}

void
Host::setHostObject( JS::HandleObject hostObj ) {
	
	_hostObject.set(hostObj);
}


JS::HandleObject
Host::hostObject() {

	return JS::HandleObject::fromMarkedLocation(_hostObject.address());
}






JL_END_NAMESPACE
