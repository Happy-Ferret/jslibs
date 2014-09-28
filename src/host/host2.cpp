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
#include <jswrapper.h> // unwrap

#include <jslibsModule.h>

#include "../jslang/jslang.h"


// by default, we run in unsafe mode.
// mixing safe and unsafe is not allowed.
DLLAPI bool _unsafeMode = true;

DLLAPI jl_malloc_t jl_malloc = nullptr;
DLLAPI jl_calloc_t jl_calloc = nullptr;
DLLAPI jl_memalign_t jl_memalign = nullptr;
DLLAPI jl_realloc_t jl_realloc = nullptr;
DLLAPI jl_msize_t jl_msize = nullptr;
DLLAPI jl_free_t jl_free = nullptr;



JL_BEGIN_NAMESPACE

//////////////////////////////////////////////////////////////////////////////
// Threaded memory deallocator

ThreadedAllocator *ThreadedAllocator::_owner = nullptr;

Allocators ThreadedAllocator::_base;

volatile bool ThreadedAllocator::_skipCleanup;

// block-to-free chain
volatile void *ThreadedAllocator::_head;

// thread stats
volatile int32_t ThreadedAllocator::_headLength;
volatile int ThreadedAllocator::_load;

// thread handler
JLThreadHandler ThreadedAllocator::_memoryFreeThread;

volatile ThreadedAllocator::MemThreadAction ThreadedAllocator::_threadAction;
JLSemaphoreHandler ThreadedAllocator::_memoryFreeThreadSem;

volatile bool ThreadedAllocator::_canTriggerFreeThread;


ALWAYS_INLINE void FASTCALL
ThreadedAllocator::freeHead() {

	_headLength = 0;

	volatile void *next = _head;
	volatile void *it = *(void**)next;
	*(void**)next = nullptr;

	while ( it ) {

		ASSERT( it != *(void**)it );
		next = *(void**)it;
		_base.free((void*)it);
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

	if (unlikely( ptr == nullptr ))
		return;

	ASSERT( ptr > (void*)0x1000 );

	if (unlikely( _load >= MAX_LOAD || _base.msize(ptr) >= BIG_ALLOC )) { // if blocks is big OR too many things to free, the thread can not keep pace.

		_base.free(ptr);
		return;
	}

	*(volatile void**)ptr = _head;
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
		_base.free((void*)_head);
	}

	_current = _base;
	_owner = nullptr;
}

ThreadedAllocator::ThreadedAllocator(Allocators &allocators)
: _current(allocators) {

	ASSERT( _owner == nullptr );
	_owner = this;

	_base = allocators;
	allocators = Allocators(_malloc, _calloc, _memalign, _realloc, _msize, _free);

	_skipCleanup = false;
	_load = 0;
	_headLength = 0;
	_head = nullptr;
	_memoryFreeThreadSem = JLSemaphoreCreate(0);
	_memoryFreeThread = JLThreadStart(memoryFreeThreadProc);
	_canTriggerFreeThread = false;

	_free(_malloc(0)); // make head non-nullptr (TBD) why?
}


//////////////////////////////////////////////////////////////////////////////
// CountedAlloc

CountedAlloc *CountedAlloc::_owner = nullptr;

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
	if ( ptr == nullptr ) {

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

	ASSERT( _owner == nullptr );
	_owner = this;

	_allocCount = 0;
	_allocAmount = 0;
	_freeCount = 0;
	_freeAmount = 0;

	_base = _current;
	_current = Allocators(_malloc, _calloc, _memalign, _realloc, _msize, _free);
}

CountedAlloc::~CountedAlloc() {

	// see HostStdIO on failure (wideshare issue)
	fprintf(stderr, "\n{alloc:%d (%dB), leaks:%d (%dB)}\n", _allocCount, _allocAmount, _allocCount - _freeCount, _allocAmount - _freeAmount);

	_current = _base;
	_owner = nullptr;
}


//////////////////////////////////////////////////////////////////////////////
// WatchDog

bool
WatchDog::interruptCallback(JSContext *cx) {

	JSRuntime *rt = JL_GetRuntime(cx);

	// disable the interrupt callback
	JSInterruptCallback interruptCallback = JS_GetInterruptCallback(rt);
	JS_SetInterruptCallback(rt, nullptr);

	jl::HostRuntime &runtime = jl::HostRuntime::getJLRuntime(rt);
	jl::EventInterrupt ev(cx);
	JL_CHK( runtime.notify(ev) );
	
	// enable the interrupt callback
	JSInterruptCallback tmp = JS_SetInterruptCallback(rt, interruptCallback);
	ASSERT( tmp == nullptr );
	
	return true;
	JL_BAD;
}


JLThreadFuncDecl
WatchDog::watchDogThreadProc(void *threadArg) {

	WatchDog &watchDog(*static_cast<WatchDog*>(threadArg));

	for (;;) {

		// used as a breakable Sleep instead of SleepMilliseconds (see SandboxEval).
		if ( JLSemaphoreAcquire(watchDog._watchDogSemEnd, watchDog._interruptInterval) != JLTIMEOUT )
			break;
		JSRuntime *rt = watchDog._hostRuntime.runtime();

		ASSERT( rt );
		//ASSERT( JS_GetInterruptCallback(rt) );
		if ( JS_GetInterruptCallback(rt) )
			JS_RequestInterruptCallback(rt);
	}
	JLThreadExit(0);
	return 0;
}


WatchDog::WatchDog(HostRuntime &hostRuntime) :
	_hostRuntime(hostRuntime),
	_interruptInterval(0),
	_watchDogThread(JLThreadInvalidHandler) {
}


void
WatchDog::start() {

	ASSERT( _interruptInterval != 0 );
	ASSERT( _watchDogThread == JLThreadInvalidHandler );

	Dbg<JSInterruptCallback> prevInterruptCallback = JS_SetInterruptCallback(_hostRuntime.runtime(), interruptCallback);
	ASSERT( prevInterruptCallback == nullptr );

	_watchDogSemEnd = JLSemaphoreCreate(0);
	ASSERT( JLSemaphoreOk(_watchDogSemEnd) );

	_watchDogThread = JLThreadStart(watchDogThreadProc, this);
	ASSERT( JLThreadOk(_watchDogThread) );
}

void
WatchDog::stop() {

	// beware: it is important to destroy the watchDogThread BEFORE destroying the cx !!!
	ASSERT( _hostRuntime.runtime() );

	ASSERT( _interruptInterval == 0 );
	ASSERT( JLThreadOk(_watchDogThread) );

	JLSemaphoreRelease(_watchDogSemEnd);
	JLThreadWait(_watchDogThread);
	JLThreadFree(&_watchDogThread);
	JLSemaphoreFree(&_watchDogSemEnd);

	Dbg<JSInterruptCallback> prev_interruptCallback = JS_SetInterruptCallback(_hostRuntime.runtime(), nullptr);
	//ASSERT( prev_interruptCallback == interruptCallback );
}



//////////////////////////////////////////////////////////////////////////////
// HostRuntime

void
HostRuntime::errorReporterBasic( JSContext *cx, const char *message, JSErrorReport *report ) {

	jl::SimpleBufferBuffer<wchar_t, 256> buf;

	buf.cat( message );
	if ( report ) {

		buf.cat( L(" (") );
		buf.cat( report->filename ? report->filename : "<no filename>" );
		buf.cat( L(":") );
		buf.cat( report->lineno, 10 );
		buf.cat( L(")") );
	}
	buf.cat( L("\n") );

	jl::BufString tmpErrTxt( buf.toString(), buf.length() );
	Host::getJLHost( cx ).stdIO().error( tmpErrTxt );
}

void
HostRuntime::destroyCompartmentCallback(JSFreeOp *fop, JSCompartment *compartment) {

//	Dbg<bool> st = getJLRuntime(fop->runtime()).fireEvent(DESTROY_COMPARTMENT);
//	ASSERT( st );
}

void
HostRuntime::destroyZoneCallback(JS::Zone *zone) {
}


HostRuntime::~HostRuntime() {

	ASSERT( rt );
	ASSERT( _isEnding == false );
	_isEnding = true;

	_watchDog.setInterruptInterval(0);

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

	bool st;

	EventBeforeDestroyRuntime ev;
	st = notify(ev), E_HOST, E_INTERNAL, E_NAME("BEFORE_DESTROY_RUNTIME");
	ASSERT( st );

	destroyAllContext(rt);

	JS_DestroyRuntime(rt);
	rt = nullptr;

	{
	EventAfterDestroyRuntime ev;
	st = notify(ev), E_HOST, E_INTERNAL, E_NAME("AFTER_DESTROY_RUNTIME");
	ASSERT( st );
	}

	{
	EventDestroyHostRuntime ev;
	st = notify(ev), E_HOST, E_INTERNAL, E_NAME("DESTRUCT_HOSTRUNTIME");
	ASSERT( st );
	}

bad:
	// on error, do the minimum.
	if ( rt ) {

		destroyAllContext(rt);
		JS_DestroyRuntime(rt);
	}

	IFDEBUG( invalidate() );
}

HostRuntime::HostRuntime( Allocators allocators, bool unsafeMode, uint32_t maxbytes, size_t nativeStackQuota )
: _allocators(allocators), rt(nullptr), _unsafeMode(unsafeMode), _isEnding(false), _skipCleanup(false), _watchDog(*MOZ_THIS_IN_INITIALIZER_LIST()) {

	::_unsafeMode = unsafeMode;

	rt = JS_NewRuntime(maxbytes);
	JL_CHK( rt );

	JS_SetNativeStackQuota(rt, nativeStackQuota); // doc: 0:disabled ?

	//	JS_SetGCParametersBasedOnAvailableMemory(rt, 512); // MB ?

	JS_SetGCParameter(rt, JSGC_MODE, JSGC_MODE_INCREMENTAL);
	JS_SetGCParameter(rt, JSGC_SLICE_TIME_BUDGET, 40);
	JS_SetGCParameter(rt, JSGC_MAX_MALLOC_BYTES, 2 * 1024 * 1024); // Number of JS_malloc bytes before last ditch GC.
	JS_SetGCParameter(rt, JSGC_MAX_BYTES, 2 * 1024 * 1024); // Maximum nominal heap before last ditch GC. (impacted by JL_updateMallocCounter)

	JS::RuntimeOptionsRef(rt)
		.setBaseline(true)
		.setIon(true)
		.setAsmJS(true)
			// doc. VarObjFix is recommended.  Without it, the two scripts "x = 1" and "var x = 1", where no variable x is in scope, do two different things.
			//      The former creates a property on the global object.  The latter creates a property on obj.  With this flag, both create a global property.
		.setVarObjFix(true)
		.setNativeRegExp(true)
	;

	ASSERT( JS::IsIncrementalGCEnabled( rt ) );
	ASSERT( JS::IsGenerationalGCEnabled( rt ) );

	ASSERT( JL_GetRuntimePrivate(rt) == nullptr );
	JL_SetRuntimePrivate(rt, this);

	JS_SetDestroyCompartmentCallback(rt, destroyCompartmentCallback);
	JS_SetDestroyZoneCallback(rt, destroyZoneCallback);
	
	return;
bad:
	invalidate();
}


JSContext *
HostRuntime::createContext() {

	JSContext *cx = JS_NewContext(rt, 8192); // set the chunk size of the stack pool to 8192. see http://groups.google.com/group/mozilla.dev.tech.js-engine/browse_thread/thread/be9f404b623acf39/9efdfca81be99ca3
	JL_CHK( cx ); //, "unable to create the context." );

	JS_BeginRequest(cx);

	// Info: Increasing JSContext stack size slows down my scripts:
	//   http://groups.google.com/group/mozilla.dev.tech.js-engine/browse_thread/thread/be9f404b623acf39/9efdfca81be99ca3

	JS_SetErrorReporter( cx, HostRuntime::errorReporterBasic );

    JS_SetGCParameterForThread(cx, JSGC_MAX_CODE_CACHE_BYTES, 16 * 1024 * 1024);

	//JS::ContextOptionsRef(cx);
	//JS_SetNativeStackQuota(cx, DEFAULT_MAX_STACK_SIZE); // see https://developer.mozilla.org/En/SpiderMonkey/JSAPI_User_Guide
	// JSOPTION_ANONFUNFIX: https://bugzilla.mozilla.org/show_bug.cgi?id=376052
	// JS_SetOptions doc: https://developer.mozilla.org/en/SpiderMonkey/JSAPI_Reference/JS_SetOptions
	// beware: avoid using JSOPTION_COMPILE_N_GO here.

	return cx;
	JL_BADVAL(nullptr);
}



//////////////////////////////////////////////////////////////////////////////
// ModuleManager

ModuleManager::~ModuleManager() {

/*
	struct FreeDynamicLibraryHandle : Callback {

		JLLibraryHandler _libraryHandler;

		FreeDynamicLibraryHandle(JLLibraryHandler libraryHandler) : _libraryHandler(libraryHandler) {
		}

		bool operator()() {

			//#ifndef DEBUG // else the memory block was allocated in a DLL that was unloaded prior to the _CrtMemDumpAllObjectsSince() call.
			if ( JLDynamicLibraryOk(_libraryHandler) )
				JLDynamicLibraryClose(&_libraryHandler);
			//#endif
			return true;
		}
	};


	for ( uint16_t i = 0; i < MAX_MODULES; ++i ) {

		ModuleManager::Module &module = _moduleList[i];
		if ( module.moduleId != FREE_MODULE_SLOT ) {

			_hostRuntime.addListener(jl::HostRuntimeEvents::DESTRUCT_HOSTRUNTIME, new FreeDynamicLibraryHandle(module.moduleHandle));
		}
	}
*/
}


ModuleManager::ModuleManager()
:  _moduleCount(0) {
}


bool
ModuleManager::loadModule(JSContext *cx, const char *libFileName, JS::HandleObject obj, JS::MutableHandleValue rval) {

	ASSERT( obj );

	JLLibraryHandler moduleHandle = JLDynamicLibraryNullHandler;
	JL_ASSERT( libFileName != nullptr && *libFileName != '\0', E_ARG, E_NUM(1), E_DEFINED );
	JL_ASSERT( _moduleCount < MAX_MODULES, E_MODULE, E_COUNT, E_MAX, E_NUM(MAX_MODULES) );

	moduleHandle = JLDynamicLibraryOpen(libFileName);
	if ( !JLDynamicLibraryOk(moduleHandle) ) {

		JL_SAFE_BEGIN
		char errorBuffer[256];
		JLDynamicLibraryLastErrorMessage(errorBuffer, COUNTOF(errorBuffer));
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
		TCHAR filename[PATH_MAX];
		JLDynamicLibraryName((void*)moduleInit, filename, COUNTOF(filename));
		JL_ERR( E_MODULE, E_NAME(filename), E_INIT );
	}


	struct FreeDynamicLibraryHandle : Observer<EventDestroyHostRuntime>  {

		JLLibraryHandler _libraryHandler;

		FreeDynamicLibraryHandle(JLLibraryHandler libraryHandler) : _libraryHandler(libraryHandler) {
		}

		bool operator()( EventType &ev ) {

			//#ifndef DEBUG // else the memory block was allocated in a DLL that was unloaded prior to the _CrtMemDumpAllObjectsSince() call.
			if ( JLDynamicLibraryOk(_libraryHandler) )
				JLDynamicLibraryClose(&_libraryHandler);
			//#endif
			return true;
		}
	};


	jl::HostRuntime::getJLRuntime(cx).addObserver(new FreeDynamicLibraryHandle(module.moduleHandle));


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
ModuleManager::addLocalModule(JSContext *cx, const char *moduleName, ModuleInitFunction initFunction, JS::HandleObject obj) {

	ASSERT( obj );
	ASSERT( initFunction != nullptr );
	moduleId_t moduleId = localModuleId(initFunction);
	ModuleManager::Module &module = moduleSlot(moduleId);
	ASSERT( isSlotFree(module) ); // free slot
	module.moduleHandle = JLDynamicLibraryNullHandler;
	module.moduleId = moduleId;
	JL_CHKM( initFunction(cx, obj), E_MODULE, E_NAME(moduleName), E_INIT );
	
	return true;
	JL_BAD;
}



//////////////////////////////////////////////////////////////////////////////
// Global

bool
Global::_lazy_enumerate(JSContext *cx, JS::HandleObject obj) {

	return JS_EnumerateStandardClasses(cx, obj);
}

bool
Global::_lazy_resolve(JSContext *cx, JS::HandleObject obj, JS::HandleId id, JS::MutableHandleObject objp) {

	bool resolved;
	if (!JS_ResolveStandardClass(cx, obj, id, &resolved))
		return false;
	if (resolved)
		objp.set(obj);
	else
		objp.set(nullptr);
    return true;
}

const JSClass Global::_globalClass_lazy = {
	NAME_GLOBAL_CLASS,
	/* JSCLASS_HAS_PRIVATE | */ JSCLASS_GLOBAL_FLAGS_WITH_SLOTS(0) | JSCLASS_NEW_RESOLVE,
	JS_PropertyStub, JS_DeletePropertyStub,
	JS_PropertyStub, JS_StrictPropertyStub,
	Global::_lazy_enumerate, (JSResolveOp)Global::_lazy_resolve,
	JS_ConvertStub, nullptr,
    nullptr, nullptr, nullptr,
	JS_GlobalObjectTraceHook
};

const JSClass Global::_globalClass = {
	NAME_GLOBAL_CLASS,
	/* JSCLASS_HAS_PRIVATE | */ JSCLASS_GLOBAL_FLAGS_WITH_SLOTS(0),
	JS_PropertyStub, JS_DeletePropertyStub,
	JS_PropertyStub, JS_StrictPropertyStub,
	JS_EnumerateStub, JS_ResolveStub,
	JS_ConvertStub, nullptr,
    nullptr, nullptr, nullptr,
	JS_GlobalObjectTraceHook
};


Global::Global( JSContext *cx, bool lazyStandardClasses ) :
_globalObject( cx ) {

	JS::CompartmentOptions compartmentOptions;
	compartmentOptions
		.setVersion(JSVERSION_LATEST)
	;

	_globalObject.set( JS_NewGlobalObject(cx, lazyStandardClasses ? &_globalClass_lazy : &_globalClass, nullptr, JS::DontFireOnNewGlobalHook, compartmentOptions) );
	JL_CHK( _globalObject ); // "unable to create the global object." );
	ASSERT( JS_IsGlobalObject(_globalObject) );

	{
		// set globalObject as current global object.
		JSAutoCompartment ac(cx, _globalObject);

		JL_CHK( JS_InitStandardClasses(cx, _globalObject) );
		JL_CHK( JS_InitReflect(cx, _globalObject) );
		#ifdef JS_HAS_CTYPES
		JL_CHK( JS_InitCTypesClass(cx, _globalObject) );
		#endif

		JS_FireOnNewGlobalObject(cx, _globalObject);

	}

	return;

bad:
	invalidate();
}


//////////////////////////////////////////////////////////////////////////////
// ErrorManager

/*
wchar_t *ErrorManager::_errorMessageNameList[] = {
#define DEF( NAME, TXT, EXN ) \
	L(#NAME),
#include "jlerrors.msg"
#undef DEF
};
*/

const ErrorManager::ErrorList ErrorManager::_errorList[] = {
#define DEF( NAME, TXT, EXN ) \
	{ L(#NAME), EXN },
#include "jlerrors.msg"
#undef DEF
};


const wchar_t* ErrorManager::_defaultMessages[] = {
#define DEF( NAME, TXT, EXN ) \
	L(TXT),
#include "jlerrors.msg"
#undef DEF
};


bool
ErrorManager::exportMessages( JSContext *cx, JS::MutableHandleObject messagesObj ) {

	messagesObj.set( jl::newObject( cx ) );
	JL_ASSERT_ALLOC( messagesObj );
	for ( size_t i = 0; i < E__END; ++i )
		JL_CHK(jl::setProperty(cx, messagesObj, _errorList[i].name, _currentMessages[i]));
	return true;
	JL_BAD;
}

bool
ErrorManager::importMessages( JSContext *cx, JS::HandleObject messagesObj ) {

	restoreDefaultMessages();
	if ( messagesObj ) {

		jl::BufString buf;
		_currentMessages = (const wchar_t**)jl_calloc( sizeof(wchar_t*) * E__END, 1 );
		JL_ASSERT_ALLOC(_currentMessages);
		for ( size_t i = 0; i < E__END; ++i ) {

			JL_CHK(jl::getProperty(cx, messagesObj, _errorList[i].name, &buf));
			_currentMessages[i] = buf.toStringZ<wchar_t*>();
			JL_ASSERT_ALLOC(_currentMessages[i]);
		}
	}
	return true;
bad:
	restoreDefaultMessages();
	return false;
}


// JSErrorCallback
const JSErrorFormatString *
ErrorManager::errorCallback(void *userRef, const unsigned errorNumber) {

	return (JSErrorFormatString*)userRef;
}



// ErrArg

ErrorManager::ErrArg::ErrArg( jl::StrDataSrc & val ) {

	JS::AutoCheckCannotGC nogc;
	if ( val.isWide() ) {

		_type = ErrArg::WSTRING;
		_wstring = val.toWStrZ(nogc);
	} else {

		_type = ErrArg::STRING;
		_string = val.toStrZ(nogc);
	}
}


bool
ErrorManager::report( JSContext *cx, bool isWarning, size_t argc, const ErrArg *args ) const {

	const ErrArg *argsEnd = args + argc;

	jl::SimpleBufferBuffer<wchar_t, 2048> buf;

	JSExnType exn = JSEXN_NONE;

	for ( ; args != argsEnd && args->is<ErrArg::INTEGER>() && args->asInteger() != E__END; ++args ) {

		const wchar_t *str;
		const wchar_t *strEnd;
		const wchar_t *pos;
		int len;

		ASSERT( args->asInteger() >= 0 );
		ASSERT( args->asInteger() < E__END );

		const wchar_t* errChunk = _currentMessages[args->asInteger()];

		if (exn == JSEXN_NONE && _errorList[args->asInteger()].exn != JSEXN_NONE)
			exn = _errorList[args->asInteger()].exn;
		str = errChunk;

		if ( !buf.isEmpty() )
			buf.cat( L( " " ) );

		pos = str;
		strEnd = pos + jl::strlen(pos);

		for ( ;; ) {

			const wchar_t *ppos = jl::strchr( pos, '%' );
			if ( !ppos ) {

				buf.cat( pos, strEnd );
				break;
			}

			buf.cat( pos, ppos );
			pos = ppos + 1;

			++args;
			JL_CHK( args != argsEnd );

			switch ( *pos ) {
			case 'd':
				++pos;
				JL_CHK( args->type() == ErrArg::INTEGER );
				buf.cat( args->asInteger(), 10 );
				break;
			case 'x':
				++pos;
				JL_CHK(args->type() == ErrArg::INTEGER);
				buf.cat( L( "0x" ) );
				buf.cat( args->asInteger(), 16 );
				break;
			case 's': {
				++pos;
				if (args->type() == ErrArg::STRING) {

					const char * tmp = args->asString();
					len = jl::strlen(tmp);
					if (len > 128) {

						buf.cat(tmp, tmp + 128);
						buf.cat(L("..."));
					}
					else {

						buf.cat(tmp, tmp + len);
					}
					break;
				}
				else if (args->type() == ErrArg::WSTRING) {

					const wchar_t * tmp = args->asWstring();
					len = jl::strlen(tmp);
					if (len > 128) {

						buf.cat(tmp, tmp + 128);
						buf.cat(L("..."));
					}
					else {

						buf.cat(tmp, tmp + len);
					}
					break;
				}
				JL_CHK(false);
			}
			default:
				buf.cat( L( "%" ) );
				break;
			}
		}
	}

	buf.cat( L( "." ) );

	JL_CHK( args == argsEnd || ( args->is<ErrArg::INTEGER>() && args->asInteger() == E__END) );

	{
		JSErrorFormatString format = { "{0}", 1, (int16_t)exn };
		return JS_ReportErrorFlagsAndNumberUC( cx, isWarning ? JSREPORT_WARNING : JSREPORT_ERROR, errorCallback, &format, 0, static_cast<const wchar_t *>(buf) );
	}

bad:
	{
		JSErrorFormatString format = { "Invalid error message.", 0, JSEXN_INTERNALERR };
		return JS_ReportErrorFlagsAndNumberUC( cx, JSREPORT_ERROR, errorCallback, &format, 0 );
	}
}


//////////////////////////////////////////////////////////////////////////////
// Host

namespace pv {

BEGIN_CLASS( Host )

DEFINE_FINALIZE() {

	//void *pv = JL_GetPrivateFromFinalize(obj);

	obj = js::UncheckedUnwrap(obj);

	void *pv = JS_GetPrivate(obj);
	if ( pv ) {

		jl::Host &host = *static_cast<jl::Host*>(pv);
		ASSERT( host );

		jl::Global &global = host.global();
		ASSERT( global );

		delete &host;



		delete &global;
	}
}


DEFINE_CONSTRUCTOR() {

	JL_DEFINE_ARGS;
	
	if ( JL_ARGC == 0 ) {

		JL_DEFINE_CONSTRUCTOR_OBJ;
		return true;
	}

	JL_ASSERT_ARGC_RANGE(1, 2);
	JL_ASSERT_ARG_IS_CALLABLE(1);

	jl::Global &global = *new jl::Global(cx);
	JL_CHK( global );

	jl::Host &host = *new jl::Host(cx, global, jl::Host::getJLHost(cx).stdIO());
	JL_CHK( host );

	JL_SetPrivate(host.hostObject(), &host); // Host object is hold by the JS Host instance

	{

		JS::RootedObject globalObject(cx, global.globalObject());
		JSAutoCompartment ac(cx, globalObject);

		JS::RootedObject hostMainFctObj(cx, JL_ARG(1).toObjectOrNull());

		uint32_t length;
		void *scriptData = JS_EncodeInterpretedFunction(cx, hostMainFctObj, &length);
		JL_CHK( scriptData );
		JL_CHK( length );
		hostMainFctObj.set( JS_DecodeInterpretedFunction(cx, scriptData, length, nullptr) );
		js_free(scriptData);
		JL_CHK( hostMainFctObj );

		hostMainFctObj.set( JS_CloneFunctionObject(cx, hostMainFctObj, globalObject) );
		JL_CHK( hostMainFctObj );

		JS::RootedValue arg(cx, JL_SARG(2));
		JL_CHK( JS_WrapValue(cx, &arg) );
		JL_CHK( jl::callNoRval(cx, globalObject, hostMainFctObj, arg) );

	}


	
	{

		JS::RootedObject newHostObject(cx, host.hostObject());
		JS_WrapObject(cx, &newHostObject);
		args.setThis(newHostObject);

		
		ASSERT( JS_GetPrivate(js::UncheckedUnwrap(newHostObject)) == &host );

		//jl::setProperty(cx, global.globalObject(), "xxx", newHostObject);

		//JS_ObjectToInnerObject

	}

	return true;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER( unsafeMode ) {

	JL_IGNORE( id, obj );

	JL_CHK( jl::setValue(cx, vp, jl::HostRuntime::getJLRuntime(cx).unsafeMode()) );
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
$BOOL $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER(jsVersion) {

	JL_IGNORE(id, obj);

	JL_CHK(jl::setValue(cx, vp, JS_GetVersion(cx))); // btw, see JS_GetImplementationVersion()
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
$BOOL $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER(lang) {

	JL_IGNORE(id, obj);

	wchar_t lang[LOCALE_NAME_MAX_LENGTH];
	JLGetUserLocaleName(lang, LOCALE_NAME_MAX_LENGTH);

	JL_CHK(jl::setValue(cx, vp, lang));
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
$BOOL $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER( errorMessages ) {

	JL_DEFINE_PROP_ARGS;
	JS::RootedObject messagesObj(cx);
	JL_CHK( jl::Host::getJLHost( cx ).errorManager().exportMessages( cx, &messagesObj ) );
	JL_RVAL.setObject(*messagesObj);
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
$BOOL $INAME $READONLY
**/
DEFINE_PROPERTY_SETTER( errorMessages ) {

	JL_DEFINE_PROP_ARGS;
	JL_ASSERT_IS_OBJECT_OR_NULL(JL_RVAL, "error messages");
	{
		JS::RootedObject messagesObj(cx, JL_RVAL.toObjectOrNull());
		JL_CHK( jl::Host::getJLHost( cx ).errorManager().importMessages( cx, messagesObj ) );
	}
	JL_CHK( jl::StoreProperty(cx, obj, id, vp, false) );
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
$BOOL $INAME $READONLY
**/
DEFINE_PROPERTY_SETTER( interruptInterval ) {

	JL_DEFINE_PROP_ARGS;

	uint32_t interval;
	JL_CHK( jl::getValue(cx, JL_RVAL, &interval) );
	jl::HostRuntime &runtime = jl::HostRuntime::getJLRuntime( cx );
	runtime.setInterruptInterval(interval);
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
$BOOL $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER( interruptInterval ) {

	JL_DEFINE_PROP_ARGS;

	JL_CHK( jl::setValue(cx, JL_RVAL, jl::HostRuntime::getJLRuntime( cx ).interruptInterval()) );
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
	jl::StrData str(cx);
	jl::Host &host = jl::Host::getJLHost( cx );
	for ( unsigned i = 0; i < argc; ++i ) {

		JL_CHK( jl::getValue( cx, JL_ARGV[i], &str ) );
		int status = host.stdIO().output(str);
		JL_ASSERT_WARN( status >= 0, E_HOST, E_INTERNAL, E_SEP, E_COMMENT("stdout"), E_WRITE );
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
	jl::StrData str(cx);
	jl::Host &host = jl::Host::getJLHost( cx );
	for ( unsigned i = 0; i < argc; ++i ) {

		JL_CHK( jl::getValue( cx, JL_ARGV[i], &str ) );
		int status = host.stdIO().error(str);
		JL_ASSERT_WARN( status >= 0, E_HOST, E_INTERNAL, E_SEP, E_COMMENT("stderr"), E_WRITE );
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
	jl::Host &host = jl::Host::getJLHost( cx );

/*
	char buffer[8192];
	int status = host.stdIO().input( buffer, COUNTOF( buffer ) );
	if ( status > 0 ) {

		JL_CHK( jl::setValue( cx, JL_RVAL, jl::CStrSpec( buffer, status ) ) );
	} else {

		JL_WARN( E_HOST, E_INTERNAL, E_SEP, E_COMMENT( "stdin" ), E_READ );
		JL_RVAL.set( JL_GetEmptyStringValue( cx ) );
	}
*/

/*
	jl::ChunkedBuffer<char> buf;
	int status;
	do {

		buf.Reserve( 4096 );
		status = host.stdIO().input( buf.Ptr(), 4096 );
		if ( status < 0 ) // error
			break;
		buf.Advance( status );
	} while ( status > 0 );

	if ( status < 0 )
		JL_WARN( E_HOST, E_INTERNAL, E_SEP, E_COMMENT( "stdin" ), E_READ );
	JL_CHK( BlobCreate( cx, buf.GetDataOwnership(), buf.Length(), JL_RVAL ) );
*/

	jl::BufString buf;
	int status = host.stdIO().input( buf );
	JL_ASSERT_WARN( status >= 0, E_HOST, E_INTERNAL, E_SEP, E_COMMENT( "stdin" ), E_READ );
	JL_CHK( BlobCreate( cx, buf, JL_RVAL ) );

	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $OBJ | null $INAME()
**/
DEFINE_FUNCTION( loadModule ) {

	jl::StrData str(cx);

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC(1);

	char libFileName[PATH_MAX];

	{
		JL_CHK( jl::getValue(cx, JL_ARG(1), &str) );

		//jl::strncpy( libFileName, str.toData<const char *>(), str.length() ); // (TBD) use copyTo()
		size_t count = str.copyTo( libFileName, COUNTOF(libFileName)-1 );
		libFileName[count] = '\0';
		jl::strcat( libFileName, DLL_EXT );
		// MAC OSX: 	'@executable_path' ??
	}

	JL_CHK( jl::Host::getJLHost(cx).moduleManager().loadModule(cx, libFileName, JL_OBJ, JL_RVAL) );
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME()
**/
DEFINE_FUNCTION( collectGarbage ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_RANGE(0, 3);

	bool requestIncrementalGC = jl::getValueDefault(cx, JL_SARG(1), false);
	int64_t sliceMillis = jl::getValueDefault<int64_t>(cx, JL_SARG(2), 0);
	bool requestAllZonesGC = jl::getValueDefault(cx, JL_SARG(3), false);

	JSRuntime *rt = JL_GetRuntime(cx);

	// http://dxr.mozilla.org/mozilla-central/source/js/public/GCAPI.h#195

	bool incrementalGCInProgress = JS::IsIncrementalGCInProgress( rt );

	if ( !incrementalGCInProgress && !requestIncrementalGC ) {

		JS_GC(rt);
		JL_RVAL.setBoolean(true);
		return true;
	}

	if ( incrementalGCInProgress ) {

		JS::PrepareForIncrementalGC(rt); // select previously selected zones

		if ( !requestIncrementalGC ) {

			JS::FinishIncrementalGC( rt, JS::gcreason::API );
			JL_RVAL.setBoolean(true);
			return true;
		}

	} else {

		if ( requestAllZonesGC )
			JS::PrepareForFullGC(rt); // select all zones
		else
			PrepareZoneForGC(js::GetCompartmentZone(js::GetObjectCompartment(JL_OBJ)));
	}

	JS::IncrementalGC( rt, JS::gcreason::API, sliceMillis );
	JL_RVAL.setBoolean( !JS::IsIncrementalGCInProgress( rt ) );
	return true;
	JL_BAD;
}


/**qa
	QA.ASSERTOP(global, 'has', 'host');
	QA.ASSERTOP(host, 'has', 'stdout');
	QA.ASSERTOP(host, 'has', 'unsafeMode');
**/
CONFIGURE_CLASS

	HAS_PRIVATE
	HAS_CONSTRUCTOR
	HAS_FINALIZE

	REVISION(jl::SvnRevToInt("$Revision$"))

	BEGIN_FUNCTION_SPEC
		FUNCTION_ARGC(loadModule, 1)

		// note: we have to support: var prevStderr = host.stderr; host.stderr = function(txt) { file.Write(txt); prevStderr(txt) };
		// route: print() => host->stdout() => JSDefaultStdoutFunction() => hpv->hostStdOut()
		FUNCTION_ARGC(stdin, 0)
		FUNCTION_ARGC(stdout, 1)
		FUNCTION_ARGC(stderr, 1)
		FUNCTION_ARGC(collectGarbage, 3)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_GETTER(unsafeMode)
		PROPERTY_GETTER(jsVersion)
		PROPERTY_GETTER(lang)
		PROPERTY(errorMessages)
		PROPERTY(interruptInterval)
	END_PROPERTY_SPEC

END_CLASS

} // namespace pv

//////////////////////////////////////////////////////////////////////////////
// Host


void
Host::errorReporter(JSContext *cx, const char *message, JSErrorReport *report) {

	// trap JSMSG_OUT_OF_MEMORY error to avoid calling ErrorReporter_stdErrRouter() that may allocate memory that will lead to nested call.
	if (unlikely( report && report->errorNumber == JSMSG_OUT_OF_MEMORY )) {

		HostRuntime::errorReporterBasic( cx, message, report );
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
		size_t remaining = COUNTOF(buffer)-(buf-buffer); \
		if ( remaining == 0 ) break; \
		int count = snprintf(buf, remaining, FORMAT, ##__VA_ARGS__); \
		buf += count < 0 ? remaining : count; \
	JL_MACRO_END

	#define fputs(STR, FILE) \
	JL_MACRO_BEGIN \
		size_t remaining = COUNTOF(buffer)-(buf-buffer); \
		if ( remaining == 0 ) break; \
		size_t len = jl::min(strlen(STR), remaining); \
		jl::memcpy(buf, STR, len); \
		buf += len; \
	JL_MACRO_END

	#define fwrite(STR, SIZE, COUNT, FILE) \
	JL_MACRO_BEGIN \
		size_t remaining = COUNTOF(buffer)-(buf-buffer); \
		if ( remaining == 0 ) break; \
		size_t len = jl::min(size_t((SIZE)*(COUNT)), remaining); \
		jl::memcpy(buf, (STR), len); \
		buf += len; \
	JL_MACRO_END

	#define fputc(CHR, FILE) \
	JL_MACRO_BEGIN \
		size_t remaining = COUNTOF(buffer)-(buf-buffer); \
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

	fputc('\0', file);

	#undef fprintf
	#undef fputs
	#undef fwrite
	#undef fputc
	#undef fflush

	jl::BufString tmpErrTxt(jl::constPtr(buffer), buf - buffer -1, true);

	if ( Host::hasJLHost(cx) ) {

		Host::getJLHost( cx ).hostStderrWrite( cx, tmpErrTxt, tmpErrTxt.length() );
	} else {

		jl::fputs(tmpErrTxt.toWStrZ(JS::AutoCheckCannotGC()), stderr);
	}

}


bool
Host::hostStderrWrite(JSContext *cx, const TCHAR *message, size_t length) {

	AutoRestoreExceptionState ares( cx );
	// avoid reentrancy if stderr function rise an error.
	AutoRestoreErrorReporter arer( cx, JL_IS_SAFE ? HostRuntime::errorReporterBasic : nullptr );

	JS::RootedObject globalObject(cx, JL_GetGlobal(cx));
	ASSERT( globalObject );

	JS::RootedValue fct(cx);
	JS::RootedObject hostObj(cx, _hostObject);

	JL_CHK( JS_WrapObject(cx, &hostObj) );

	JL_CHK( jl::getProperty(cx, hostObj, JLID(cx, stderr), &fct) );
	if ( jl::isCallable(cx, fct) ) {

		JL_CHK( jl::callNoRval(cx, globalObject, fct, jl::strSpec(message, length)) ); // beware out of memory case !
	}
	return true;

bad:
	ares.drop();
	return false;
}




Host::~Host() {

	EventHostDestroy ev(_hostRuntime, *this);
	notify(ev);

	ASSERT( JS_GetCompartmentPrivate( _global.compartment() ) == this );
	JS_SetCompartmentPrivate( _global.compartment(), nullptr );

	if ( _interruptObserverItem )
		_hostRuntime.removeObserver(_interruptObserverItem);

	_ids.destructAll();

//	JL_SetPrivate(hostObject(), nullptr);

	IFDEBUG( invalidate() );
}


Host::Host( JSContext *cx, Global &glob, StdIO &hostStdIO ) :
_hostRuntime(HostRuntime::getJLRuntime(cx)),
_global(glob),
_moduleManager(),
_errorManager(),
_compatId(JL_HOST_VERSIONID),
_hostStdIO(hostStdIO),
_objectProto(cx),
_hostObject(cx),
_ids(),
_interruptObserverItem(nullptr) {

//	JL_CHKM(_global, E_GLOBAL, E_INVALID);

	ASSERT(cx);

	_ids.constructAll(cx);

	ASSERT( !JSID_IS_ZERO(_ids.get(0)) );
	ASSERT( !JSID_IS_ZERO(_ids.get(LAST_JSID-1)) );

	ASSERT( JS_GetCompartmentPrivate( glob.compartment() ) == nullptr );
	JS_SetCompartmentPrivate( glob.compartment(), this );


	JS::RuntimeOptionsRef(cx)
		.setExtraWarnings(!HostRuntime::getJLRuntime(cx).unsafeMode())
	;

	JS_SetErrorReporter(cx, errorReporter);

	{

		JS::RootedObject globalObj(cx, _global.globalObject());
		JSAutoCompartment ac(cx, globalObj);

		JL_ASSERT( globalObj, E_GLOBAL, E_NOTFOUND ); // "Global object not found."

		_objectProto.set( JS_GetObjectPrototype(cx, globalObj) );
		ASSERT( _objectProto );

		_objectClasp = JL_GetClass(_objectProto);
		ASSERT( _objectClasp );

		IFDEBUG( bool found; ASSERT( JS_HasPropertyById(cx, globalObj, JLID(cx, global), &found) && !found ) );

		// global functions & properties
		JL_CHKM( JS_DefinePropertyById(cx, globalObj, JLID(cx, global), globalObj, JSPROP_READONLY | JSPROP_PERMANENT), E_PROP, E_CREATE );


		// var host = new Host();
		JS::RootedObject hostConstructor(cx, jl::InitClass(cx, globalObj, pv::Host::classSpec ));
		_hostObject.set( JS_New(cx, hostConstructor, JS::HandleValueArray::empty()) );

		JL_CHK( _hostObject );

		// JL_SetPrivate( _hostObject, this ); <- Host object is hold by the caller, do not store it

		//JL_CHK( jl::setProperty(cx, globalObj, JLID(cx, host), _hostObject) );
		JL_CHK( JS_DefinePropertyById(cx, globalObj, JLID(cx, host), _hostObject, JSPROP_READONLY | JSPROP_PERMANENT) );

		JL_CHK( moduleManager().addLocalModule(cx, "jslang", jslangModuleInit, globalObj) );

	}
	

	struct OnInterupt : Observer<EventInterrupt> {
		Host &_host;
		OnInterupt(Host &host) : _host(host) {}
		bool operator()( EventType &ev ) {

			JSContext *cx = ev.cx;
			JS::AutoSaveExceptionState ase(cx);
			JSAutoCompartment ac(cx, _host.hostObject());
			JS::RootedValue fctVal(cx);
			JL_CHK( JS_GetProperty(cx, _host.hostObject(), "onInterrupt", &fctVal) );
			if ( jl::isCallable(cx, fctVal) ) {

				JSAutoCompartment ac(cx, &fctVal.toObject());
				JS::RootedValue rval(cx);
				JL_CHK( JS_CallFunctionValue(cx, JS::NullPtr(), fctVal, JS::HandleValueArray::empty(), &rval) );
			}
			return true;
		bad:
			JS_ReportPendingException(cx);
			return true; // error is reported, but do not stop events fireing
		}
	};

	_interruptObserverItem = _hostRuntime.addObserver(new OnInterupt(*this));

	return;
bad:
	invalidate();
}


bool
Host::setHostArguments( JSContext *cx, TCHAR **hostArgv, size_t hostArgc ) {

	JS::RootedValue argumentsVal(_hostRuntime.runtime());

	JL_CHK( jl::setVector(cx, &argumentsVal, hostArgv, hostArgc) );
	JL_CHK( JS_SetPropertyById(cx, _hostObject, JLID(cx, arguments), argumentsVal) );
	return true;
	JL_BAD;
}

bool
Host::setHostPath( JSContext *cx, const TCHAR *hostPath) {

	JL_CHK( jl::setProperty(cx, _hostObject, JLID(cx, path), hostPath) );
	return true;
	JL_BAD;
}

bool
Host::setHostName( JSContext *cx, const TCHAR *hostName ) {

	JL_CHK( jl::setProperty(cx, _hostObject, JLID(cx, name), hostName) );
	return true;
	JL_BAD;
}


JL_END_NAMESPACE
