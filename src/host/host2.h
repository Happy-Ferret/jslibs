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

#pragma once

#include <jsprf.h> // JS_smprintf in ErrorReporter

#include <jlclass.h>
#include <jslibsModule.h>
#include "../jslang/jslang.h"

#define NAME_MODULE_INIT "ModuleInit"
#define NAME_MODULE_RELEASE "ModuleRelease"
#define NAME_MODULE_FREE "ModuleFree"

#define NAME_GLOBAL_CLASS "Global"

#define NAME_GLOBAL_FUNCTION_LOAD_MODULE "loadModule"
#define NAME_GLOBAL_FUNCTION_UNLOAD_MODULE "unloadModule"


JL_BEGIN_NAMESPACE

DECLARE_CLASS(host2);

class Std {
public:
	virtual int	stdInput( char *buffer, size_t bufferLength ) { return 0; };
	virtual int	stdOutput( const char *buffer, size_t length ) { return 0; };
	virtual int	stdError( const char *buffer, size_t length ) { return 0; };
};


typedef void* (*malloc_t)( size_t );
typedef void* (*calloc_t)( size_t, size_t );
typedef void* (*memalign_t)( size_t, size_t );
typedef void* (*realloc_t)( void*, size_t );
typedef size_t (*msize_t)( void* );
typedef void (*free_t)( void* );


class Allocators {
public:
	jl_malloc_t _malloc;
	jl_calloc_t _calloc;
	jl_memalign_t _memalign;
	jl_realloc_t _realloc;
	jl_msize_t _msize;
	jl_free_t _free;

	Allocators(jl_malloc_t malloc, jl_calloc_t calloc, jl_memalign_t memalign, jl_realloc_t realloc, jl_msize_t msize, jl_free_t free )
	: _malloc(malloc), _calloc(calloc), _memalign(memalign), _realloc(realloc), _msize(msize), _free(free) {
	}
};


class StdAllocators : public Allocators {
public:
	StdAllocators()
	: Allocators(::malloc, ::calloc, ::memalign, ::realloc, ::msize, ::free) {
	}
};


class AutoExceptionState {
	
	JSContext *_cx;
	JSExceptionState *_exState;

public:
	~AutoExceptionState() {

		if ( _exState )
			JS_RestoreExceptionState(_cx, _exState);
	}

	AutoExceptionState(JSContext *cx) : _cx(cx) {
			
		_exState = JS_SaveExceptionState(_cx);
		JS_ClearPendingException(_cx);
	}

	void drop() {
			
		ASSERT( _exState != NULL );
		JS_DropExceptionState(_cx, _exState);
		_exState = NULL;
	}
};



class AutoErrorReporter {
	
	JSContext *_cx;
	JSErrorReporter _errReporter;

public:
	~AutoErrorReporter() {

		JS_SetErrorReporter(_cx, _errReporter);
	}

	AutoErrorReporter(JSContext *cx, JSErrorReporter errorReporter) : _cx(cx) {
			
		_errReporter = JS_SetErrorReporter(_cx, errorReporter);
	}
};


template <class T, const size_t ITEM_COUNT>
class StaticArray {
	uint8_t data[ITEM_COUNT * sizeof(T)];
	T* items; // for easy debug

public:
	enum {
		length = ITEM_COUNT
	};

	StaticArray()
	: items(reinterpret_cast<T*>(data)) {
	}

	StaticArray(bool construct)
	: items(reinterpret_cast<T*>(data)) {

		constructAll();
	}

	template <typename P1>
	StaticArray(bool construct, P1 p1)
	: items(reinterpret_cast<T*>(data)) {

		constructAll(p1);
	}

	template <typename P1, typename P2>
	StaticArray(bool construct, P1 p1, P2 p2)
	: items(reinterpret_cast<T*>(data)) {

		constructAll(p1, p2);
	}

	template <typename P1, typename P2, typename P3>
	StaticArray(bool construct, P1 p1, P2 p2, P3 p3)
	: items(reinterpret_cast<T*>(data)) {

		constructAll(p1, p2, p3);
	}

	T&
	get(size_t slotIndex) {

		ASSERT( slotIndex < length );
		return items[slotIndex];
	}

	const T&
	getConst(size_t slotIndex) const {

		ASSERT( slotIndex < length );
		return items[slotIndex];
	}


	void
	destruct(size_t item) {

		(&get(item))->T::~T();
	}

	void
	construct(size_t item) {
		
		::new (&get(item)) T();
	}

	template <typename P1>
	void
	construct(size_t item, P1 p1) {
		
		::new (&get(item)) T(p1);
	}

	template <typename P1, typename P2>
	void
	construct(size_t item, P1 p1, P2 p2) {
		
		::new (&get(item)) T(p1, p2);
	}

	template <typename P1, typename P2, typename P3>
	void
	construct(size_t item, P1 p1, P2 p2, P3 p3) {
		
		::new (&get(item)) T(p1, p2, p3);
	}

	void
	destructAll() {

		for ( size_t i = 0; i < length; ++i ) {
			
			(&get(i))->T::~T();
		}
	}

	void
	constructAll() {
		
		for ( size_t i = 0; i < length; ++i ) {
			
			::new (&get(i)) T();
		}
	}

	template <typename P1>
	void
	constructAll(P1 p1) {
		
		for ( size_t i = 0; i < length; ++i ) {
			
			::new (&get(i)) T(p1);
		}
	}

	template <typename P1, typename P2>
	void
	constructAll(P1 p1, P2 p2) {
		
		for ( size_t i = 0; i < length; ++i ) {
			
			::new (&get(i)) T(p1, p2);
		}
	}

	template <typename P1, typename P2, typename P3>
	void
	constructAll(P1 p1, P2 p2, P3 p3) {
		
		for ( size_t i = 0; i < length; ++i ) {
			
			::new (&get(i)) T(p1, p2, p3);
		}
	}

	// ...
};


class RuntimeAccess {
public:
	virtual JSRuntime* runtime() const = 0;
	virtual JSContext* context() const = 0;
};


class WatchDog {

	RuntimeAccess &_hostRuntime;

	JLSemaphoreHandler _watchDogSemEnd;
	JLThreadHandler _watchDogThread;
	uint32_t _maybeGCInterval;

	static bool 
	operationCallback(JSContext *cx) {

		JSOperationCallback tmp = JS_SetOperationCallback(JL_GetRuntime(cx), NULL);
		JS_MaybeGC(cx);
		JS_SetOperationCallback(JL_GetRuntime(cx), tmp);
		return true;
	}

	static JLThreadFuncDecl
	watchDogThreadProc(void *threadArg) {

		WatchDog &watchDog(*(WatchDog*)threadArg);

		for (;;) {

			// used as a breakable Sleep instead of SleepMilliseconds (see SandboxEval).
			if ( JLSemaphoreAcquire(watchDog._watchDogSemEnd, watchDog._maybeGCInterval) != JLTIMEOUT )
				break;
			JS_TriggerOperationCallback(watchDog._hostRuntime.runtime());
		}
		JLThreadExit(0);
		return 0;
	}


public:

	WatchDog(RuntimeAccess &hostRuntime, uint32_t maybeGCInterval)
		: _hostRuntime(hostRuntime), _maybeGCInterval(maybeGCInterval) {
	}

	bool start() {

		JSContext *cx = _hostRuntime.context();
		JSOperationCallback prevOperationCallback = JS_SetOperationCallback(_hostRuntime.runtime(), operationCallback);
		ASSERT( prevOperationCallback == NULL );
		_watchDogSemEnd = JLSemaphoreCreate(0);
		_watchDogThread = JLThreadStart(watchDogThreadProc, this);
		JL_ASSERT( JLSemaphoreOk(_watchDogSemEnd) && JLThreadOk(_watchDogThread), E_HOST, E_CREATE ); // "Unable to create the GC thread."
		return true;
		JL_BAD;
	}

	bool stop() {

		// beware: it is important to destroy the watchDogThread BEFORE destroying the cx !!!

		JSOperationCallback prevOperationCallback = JS_SetOperationCallback(_hostRuntime.runtime(), NULL);
		ASSERT( prevOperationCallback == operationCallback );
		JLSemaphoreRelease(_watchDogSemEnd);
		JLThreadWait(_watchDogThread);
		JLThreadFree(&_watchDogThread);
		JLSemaphoreFree(&_watchDogSemEnd);
		return true;
	}
};


/*
class HostRuntime : public RuntimeAccess, public jl::CppAllocators {

	// global object
	// doc: For full ECMAScript standard compliance, obj should be of a JSClass that has the JSCLASS_GLOBAL_FLAGS flag.
	// note: global_class is a global variable, but this is not an issue even if several runtimes share the same JSClass.
	static const JSClass globalClass;

	uint32_t _maybeGCInterval;

	JSRuntime *rt;
	JSContext *cx;

	bool _isEnding;
	bool _skipCleanup;

	static bool
	OperationCallback(JSContext *cx);

public:
	static void
	ErrorReporterBasic(JSContext *cx, const char *message, JSErrorReport *report);
	HostRuntime();
	JSRuntime *runtime() const;
	JSContext *context() const;
	bool create( uint32_t maxMem = (uint32_t)-1, uint32_t maxAlloc = (uint32_t)-1, uint32_t maybeGCInterval = 0 );
	bool destroy(bool skipCleanup);
};
*/


class HostRuntime : public RuntimeAccess, public jl::CppAllocators {

	// global object
	// doc: For full ECMAScript standard compliance, obj should be of a JSClass that has the JSCLASS_GLOBAL_FLAGS flag.
	// note: global_class is a global variable, but this is not an issue even if several runtimes share the same JSClass.
	static const JSClass _globalClass;

	JSRuntime *rt;
	JSContext *cx;

	Allocators _allocators;

	uint32_t _maybeGCInterval;

	WatchDog _watchDog;

	bool _isEnding;
	bool _skipCleanup;


public:

	static void
	setJSEngineAllocators(Allocators allocators) {

		js_jl_malloc = allocators._malloc;
		js_jl_calloc = allocators._calloc;
		js_jl_realloc = allocators._realloc;
		js_jl_free = allocators._free;
	}

	static void
	errorReporterBasic(JSContext *cx, const char *message, JSErrorReport *report) {

		JL_IGNORE( cx );
		if ( !report )
			fprintf(stderr, "%s\n", message);
		else
			fprintf(stderr, "%s (%s:%d)\n", message, report->filename ? report->filename : "<no filename>", (unsigned int)report->lineno);
	}


	HostRuntime(Allocators allocators = StdAllocators(), uint32_t maybeGCInterval = 10)
		: _allocators(allocators), rt(NULL), cx(NULL), _isEnding(false), _skipCleanup(false), _watchDog(*this, maybeGCInterval) {
	}

	JSRuntime *runtime() const {

		return rt;
	}
	
	JSContext *context() const {

		return cx;
	}

	bool
	create( uint32_t maxMem = (uint32_t)-1, uint32_t maxAlloc = (uint32_t)-1, uint32_t maybeGCInterval = 0 ) {

		rt = JS_NewRuntime(maxAlloc, JS_NO_HELPER_THREADS); // JSGC_MAX_MALLOC_BYTES
		JL_CHK( rt );

		// Number of JS_malloc bytes before last ditch GC.
		ASSERT( JS_GetGCParameter(rt, JSGC_MAX_MALLOC_BYTES) == maxAlloc ); // JS_SetGCParameter(rt, JSGC_MAX_MALLOC_BYTES, maxAlloc);

		// doc: maxMem specifies the number of allocated bytes after which garbage collection is run. Maximum nominal heap before last ditch GC.
		JS_SetGCParameter(rt, JSGC_MAX_BYTES, maxMem); 

		//JS_SetGCParametersBasedOnAvailableMemory

		cx = JS_NewContext(rt, 8192); // set the chunk size of the stack pool to 8192. see http://groups.google.com/group/mozilla.dev.tech.js-engine/browse_thread/thread/be9f404b623acf39/9efdfca81be99ca3
		JL_CHK( cx ); //, "unable to create the context." );

		// Info: Increasing JSContext stack size slows down my scripts:
		//   http://groups.google.com/group/mozilla.dev.tech.js-engine/browse_thread/thread/be9f404b623acf39/9efdfca81be99ca3

		JS_SetErrorReporter(cx, errorReporterBasic);

		JS::ContextOptionsRef(cx)
			.setVarObjFix(true)
			.setTypeInference(true)
			.setIon(true);

		//JS_SetNativeStackQuota(cx, DEFAULT_MAX_STACK_SIZE); // see https://developer.mozilla.org/En/SpiderMonkey/JSAPI_User_Guide
		JS_SetGCParameter(rt, JSGC_MODE, JSGC_MODE_INCREMENTAL); // JSGC_MODE_GLOBAL


		// JSOPTION_ANONFUNFIX: https://bugzilla.mozilla.org/show_bug.cgi?id=376052 
		// JS_SetOptions doc: https://developer.mozilla.org/en/SpiderMonkey/JSAPI_Reference/JS_SetOptions

		// beware: avoid using JSOPTION_COMPILE_N_GO here.

		{

		JS::CompartmentOptions options;
		options
			.setVersion(JSVERSION_LATEST)
			.setInvisibleToDebugger(false)
			.setMergeable(false);

		JS::RootedObject globalObject(cx, JS_NewGlobalObject(cx, &_globalClass, NULL, JS::DontFireOnNewGlobalHook, options));
		JL_CHK( globalObject ); // "unable to create the global object." );

		// set globalObject as current global object.
		JL_CHK( JS_EnterCompartment(cx, globalObject) );

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
	destroy(bool skipCleanup = false) {

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
		JS_DestroyContext(cx);
		cx = NULL;

		#ifdef DEBUG
		JS_DumpHeap(rt, fopen("dump.txt", "w"), NULL, JSTRACE_OBJECT, NULL, 2, NULL);
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
};




class ModuleManager {

	RuntimeAccess &_hostRuntime;

	struct ModulePrivate {
		uint32_t moduleId;
		void *privateData;
	} _modulePrivate[1<<8]; // does not support more than 256 modules.

	jl::Queue _moduleList;

public:

	ModuleManager(RuntimeAccess &hostRuntime) : _hostRuntime(hostRuntime) {

		jl::QueueInitialize(&_moduleList);
	}

	bool
	hasModule(JLLibraryHandler module) {

		for ( jl::QueueCell *it = jl::QueueBegin(&_moduleList); it; it = jl::QueueNext(it) ) {

			if ( (JLLibraryHandler)jl::QueueGetData(it) == module ) {

				return true;
			}
		}
		return false;
	}

	bool
	storeModule(JLLibraryHandler module) {

		// store the module (LIFO)
		jl::QueueUnshift(&_moduleList, module);
	}

	bool loadModule(const char *libFileName, JS::HandleObject obj, JS::MutableHandleValue rval) {

		JSContext *cx = _hostRuntime.context();

		JLLibraryHandler module = JLDynamicLibraryNullHandler;
		JL_ASSERT( libFileName != NULL && *libFileName != '\0', E_ARG, E_NUM(1), E_DEFINED );

		module = JLDynamicLibraryOpen(libFileName);
		if ( !JLDynamicLibraryOk(module) ) {

			JL_SAFE_BEGIN
			char errorBuffer[256];
			JLDynamicLibraryLastErrorMessage( errorBuffer, sizeof(errorBuffer) );
			JL_WARN( E_OS, E_OPERATION, E_DETAILS, E_STR(errorBuffer), E_COMMENT(libFileName) );
			JL_SAFE_END

			rval.setBoolean(false);
			return true;
		}
		
		if ( hasModule(module) ) {

			JLDynamicLibraryClose(&module);
			rval.setNull(); // already loaded
			return true;
		}

		uint32_t uid;
		uid = JLDynamicLibraryId(module); // module unique ID
		ModuleInitFunction moduleInit;
		moduleInit = (ModuleInitFunction)JLDynamicLibrarySymbol(module, NAME_MODULE_INIT);
		JL_ASSERT( moduleInit, E_MODULE, E_NAME(libFileName), E_INIT ); // "Invalid module."
	
		// CHKHEAP();

		if ( !moduleInit(cx, obj, uid) ) {

			JL_CHK( !JL_IsExceptionPending(cx) );
			char filename[PATH_MAX];
			JLDynamicLibraryName((void*)moduleInit, filename, sizeof(filename));
			JL_ERR( E_MODULE, E_NAME(filename), E_INIT );
		}

		// CHKHEAP();

		storeModule(module);
	
		//JL_CHK( JL_NewNumberValue(cx, uid, JL_RVAL) ); // really needed ? yes, UnloadModule will need this ID, ... but UnloadModule is too complicated to implement and will never exist.
		rval.setObject(*obj);

		return true;

	bad:
		if ( JLDynamicLibraryOk(module) )
			JLDynamicLibraryClose(&module);
		return false;

	}

	bool
	releaseModules() {
		
		JSContext *cx = _hostRuntime.context();

		for ( jl::QueueCell *it = jl::QueueBegin(&_moduleList); it; it = jl::QueueNext(it) ) {

			JLLibraryHandler module = (JLLibraryHandler)jl::QueueGetData(it);
			ModuleReleaseFunction moduleRelease = (ModuleReleaseFunction)JLDynamicLibrarySymbol(module, NAME_MODULE_RELEASE);
			if ( moduleRelease != NULL ) {
		
				if ( !moduleRelease(cx) ) {

					char filename[PATH_MAX];
					JLDynamicLibraryName((void*)moduleRelease, filename, sizeof(filename));
					JL_WARN( E_MODULE, E_NAME(filename), E_FIN ); // "Fail to release module \"%s\".", filename
				}
			}
		}

		if ( !jslangModuleRelease(cx) ) {
		
			JL_WARN( E_MODULE, E_NAME("jslang"), E_FIN ); // "Fail to release static module jslang."
		}

		return true;
		JL_BAD;
	}

	bool
	freeModules() {

		// Beware: because JS engine allocate memory from the DLL, all memory must be disallocated before releasing the DLL

		while ( !jl::QueueIsEmpty(&_moduleList) ) {

			JLLibraryHandler module = (JLLibraryHandler)jl::QueueShift(&_moduleList);
			ModuleFreeFunction moduleFree = (ModuleFreeFunction)JLDynamicLibrarySymbol(module, NAME_MODULE_FREE);
			if ( moduleFree != NULL ) {

				moduleFree();
			}

			//#ifndef DEBUG // else the memory block was allocated in a DLL that was unloaded prior to the _CrtMemDumpAllObjectsSince() call.
			JLDynamicLibraryClose(&module);
			//#endif
		}

		jslangModuleFree();

		return true;
	}
};



class ClassProtoCache {
public:
	const JSClass *clasp;
	JS::PersistentRootedObject proto;

	ClassProtoCache(JSRuntime *rt)
	: clasp(NULL), proto(rt) {
	}

	ClassProtoCache(JSRuntime *rt, const JSClass *c, JS::HandleObject p)
	: clasp(c), proto(rt, p) {
	}
};


// does not support more than (1<<MAX_CLASS_PROTO_CACHE_BIT)-1 proto.
template <const size_t CACHE_LENGTH>
class ProtoCache {

	const JSClass*
	removedSlotClasp() const {

		return reinterpret_cast<JSClass*>(-1);
	}

public:
	StaticArray<ClassProtoCache, CACHE_LENGTH> items;
	
	~ProtoCache() {

		// destruct only items that has been constructed.
		for ( int i = 0; i < items.length; ++i ) {
			
			if ( items.getConst(i).clasp != NULL && items.getConst(i).clasp != removedSlotClasp() ) {

				items.destruct(i);
			}
		}
	}

	ProtoCache() {

		// set all slots as 'unused' (aka. NULL) and let them unconstructed.
		for ( int i = 0; i < items.length; ++i ) {
			
			items.get(i).clasp = NULL;
		}
	}

	bool
	add( JSRuntime *rt, const char * const className, JSClass * const clasp, IN JS::HandleObject proto ) {

		ASSERT( removedSlotClasp() != NULL );
		ASSERT( className != NULL );
		ASSERT( className[0] != '\0' );
		ASSERT( clasp != NULL );
		ASSERT( clasp != removedSlotClasp() );
		ASSERT( proto != NULL );
		ASSERT( JL_GetClass(proto) == clasp );

		size_t slotIndex = JL_ClassNameToClassProtoCacheSlot(className);
		size_t first = slotIndex;

	//	ASSERT( slotIndex < COUNTOF(hpv->classProtoCache) );

		for (;;) {

			const ClassProtoCache &slot = items.getConst(slotIndex);

			if ( slot.clasp == NULL ) {

				items.construct(slotIndex, rt, clasp, proto);
				return true;
			}

			if ( slot.clasp == clasp ) // already cached
				return false;

			slotIndex = (slotIndex + 1) % CACHE_LENGTH;

			if ( slotIndex == first ) // no more free slot
				return false;
		}
	}


	const ClassProtoCache*
	get( const char * const className ) const {

		size_t slotIndex = JL_ClassNameToClassProtoCacheSlot(className);
		const size_t first = slotIndex;

		ASSERT( slotIndex < CACHE_LENGTH );

		for (;;) {

			const ClassProtoCache &slot = items.getConst(slotIndex);
		
			// slot->clasp == NULL -> empty
			// slot->clasp == removedSlotClasp() -> slot removed, but maybe next slot will match !

			if ( slot.clasp == NULL ) // not found
				return NULL;

			if ( slot.clasp != removedSlotClasp() && ( slot.clasp->name == className || !strcmp(slot.clasp->name, className) ) ) // see "Enable String Pooling"
				return &slot;

			slotIndex = (slotIndex + 1) % CACHE_LENGTH;

			if ( slotIndex == first ) // not found
				return NULL;
		}
	}

	
	void
	remove( const char *const className ) {

		ASSERT( removedSlotClasp() != NULL );

		size_t slotIndex = JL_ClassNameToClassProtoCacheSlot(className);
		size_t first = slotIndex;
		
		ASSERT( slotIndex < CACHE_LENGTH );

		for (;;) {

			ClassProtoCache &slot = items.get(slotIndex);

			if ( slot.clasp == NULL || ( slot.clasp != removedSlotClasp() && ( slot.clasp->name == className || strcmp(slot.clasp->name, className) == 0 ) ) ) {
			
				items.destruct(slotIndex);
				slot.clasp = removedSlotClasp();
				return;
			}

			slotIndex = (slotIndex + 1) % CACHE_LENGTH;

			if ( slotIndex == first ) // not found
				return;
		}
	}
};


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

static const JSErrorFormatString *
ErrorCallback(void *userRef, const char *, const unsigned) {

	return (JSErrorFormatString*)userRef;
}


class Host : public jl::CppAllocators {

	bool _unsafeMode;
	static const uint32_t versionId;
	RuntimeAccess &_hostRuntime;
	Std &_hostStd;
	uint32_t _versionId; // used to ensure compatibility between host and modules. see JL_HOSTPRIVATE_KEY macro.
	ModuleManager _modules;
	JS::PersistentRootedObject _hostObject;

	ProtoCache< 1<<JL_HOSTPRIVATE_MAX_CLASS_PROTO_CACHE_BIT > _classProtoCache;
	StaticArray< JS::PersistentRootedId, LAST_JSID > _ids;


	static void
	errorReporterBasic(JSContext *cx, const char *message, JSErrorReport *report) {

		JL_IGNORE( cx );
		if ( !report )
			fprintf(stderr, "%s\n", message);
		else
			fprintf(stderr, "%s (%s:%d)\n", message, report->filename ? report->filename : "<no filename>", (unsigned int)report->lineno);
	}


	static void
	errorReporter(JSContext *cx, const char *message, JSErrorReport *report) {

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
			size_t len = JL_MIN(strlen(STR), remaining); \
			jl::memcpy(buf, STR, len); \
			buf += len; \
		JL_MACRO_END

		#define fwrite(STR, SIZE, COUNT, FILE) \
		JL_MACRO_BEGIN \
			size_t remaining = sizeof(buffer)-(buf-buffer); \
			if ( remaining == 0 ) break; \
			size_t len = JL_MIN(size_t((SIZE)*(COUNT)), remaining); \
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
	
		Host::getHostPrivate(cx).hostStderrWrite(buffer, buf-buffer);
	}


	bool
	hostStderrWrite(const char *message, size_t length) {

		JSContext *cx = _hostRuntime.context();

		JS::RootedObject globalObject(cx, JL_GetGlobal(cx));
		ASSERT( globalObject );
		AutoExceptionState autoEx(cx);

		// avoid reentrancy if stderr function rise an error.
		AutoErrorReporter autoErrorReporter(cx, JL_IS_SAFE ? errorReporterBasic : NULL);
		
		JS::RootedValue fct(cx);
		JL_CHK( JS_GetPropertyById(cx, _hostObject, JLID(cx, stderr), &fct) );
		JL_CHK( JL_ValueIsCallable(cx, fct) );

		{
		JS::RootedValue rval(cx);
		JS::RootedValue text(cx);
		JL_CHK( JL_NativeToJsval(cx, message, length, &text) ); // beware out of memory case !
		JL_CHK( JL_CallFunctionVA(cx, globalObject, fct, &rval, text) );
		}

		return true;
	bad:
		autoEx.drop();
		return false;
	}

public:

	static Host&
	getHostPrivate( JSRuntime *rt ) {

		return *static_cast<Host*>(JL_GetRuntimePrivate(rt));
	}

	static Host&
	getHostPrivate( JSContext *cx ) {

		return getHostPrivate(JL_GetRuntime(cx));
	}


	Host( RuntimeAccess &hr, Std hostStd = Std(), bool unsafeMode = false )
	: _hostRuntime(hr), _modules(hr), _versionId((jl::SvnRevToInt("$Revision: 3524 $") << 16) | (sizeof(Host) & 0xFFFF)), _unsafeMode(unsafeMode), _hostStd(hostStd), _hostObject(hr.runtime()), _ids(true, hr.runtime()) {
	}

	~Host() {

		_modules.freeModules();
	}

	// init the host for jslibs usage (modules, errors, ...)
	bool
	create() {

		JSContext *cx = _hostRuntime.context();

		JL_SetRuntimePrivate(_hostRuntime.runtime(), static_cast<void*>(this));

		JS::ContextOptionsRef(cx)
			.setStrictMode(!_unsafeMode);
	
		JS_SetErrorReporter(cx, errorReporter);

		JS::RootedObject obj(cx, JL_GetGlobal(cx));
		ASSERT( obj != NULL ); // "Global object not found."

		//JSObject *newObject = JS_NewObject(cx, NULL, NULL, NULL);
		//hpv->objectClass = JL_GetClass(newObject);
		//hpv->objectProto = JL_GetPrototype(cx, newObject);
		//JL_CHK( JL_GetClassPrototype(cx, JSProto_Object, &hpv->objectProto) ); // JS_GetObjectPrototype
		//hpv->objectClass = JL_GetClass(hpv->objectProto);
		//ASSERT( hpv->objectClass );
		//ASSERT( hpv->objectProto );
	
		// global functions & properties
		JL_CHKM( JS_DefinePropertyById(cx, obj, JLID(cx, global), OBJECT_TO_JSVAL(obj), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT), E_PROP, E_CREATE );

		//JL_CHK( jl::InitClass(cx, globalObject, host2::classSpec) ); // 
		INIT_CLASS( host2 );

		// init static modules
		if ( !jslangModuleInit(cx, obj) )
			JL_ERR( E_MODULE, E_NAME("jslang"), E_INIT );

		return true;
		JL_BAD;
	}

	bool
	destroy() {
		
		return _modules.releaseModules();
	}

	bool unsafeMode() const {

		return _unsafeMode;
	}

	int	stdInput( char *buffer, size_t bufferLength ) const {
		
		return _hostStd.stdInput(buffer, bufferLength);
	}

	int	stdOutput( const char *buffer, size_t length ) const {

		return _hostStd.stdOutput(buffer, length);
	}

	int	stdError( const char *buffer, size_t length ) const {

		return _hostStd.stdError(buffer, length);
	}

	bool
	report( bool isWarning, ... ) {

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
		return JS_ReportErrorFlagsAndNumber( _hostRuntime.context(), isWarning ? JSREPORT_WARNING : JSREPORT_ERROR, ErrorCallback, (void*)&format, 0);

	//bad:
	//	va_end(vl);
	//	return false;
	}


	bool setHostArguments(char **hostArgv, size_t hostArgc) {

		JSContext *cx = _hostRuntime.context();
		JS::RootedValue argumentsVal(_hostRuntime.runtime());

		JL_CHK( JL_NativeVectorToJsval(cx, hostArgv, hostArgc, &argumentsVal) );
		JL_CHK( JS_SetPropertyById(cx, _hostObject, JLID(cx, arguments), argumentsVal) );
		return true;
		JL_BAD;
	}

	bool setHostName(const char *hostPath, const char *hostName) {

		JSContext *cx = _hostRuntime.context();
		JL_CHK( JL_NativeToProperty(cx, _hostObject, JLID(cx, name), hostName) );
		JL_CHK( JL_NativeToProperty(cx, _hostObject, JLID(cx, path), hostPath) );
		return true;
		JL_BAD;
	}


	ALWAYS_INLINE const ClassProtoCache*
	getCachedClassProto( const char * const className ) {

		return _classProtoCache.get(className);
	}




	void
	__test__() {

		JSRuntime *rt = _hostRuntime.runtime();

		_classProtoCache.add(rt, "Socket", NULL, JS::NullPtr());
		_classProtoCache.get("Socket");
		_classProtoCache.remove("Socket");

	}
};


namespace ThreadedAllocator {




};

JL_END_NAMESPACE
