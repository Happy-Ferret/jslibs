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


#define NAME_MODULE_INIT "ModuleInit"
#define NAME_MODULE_RELEASE "ModuleRelease"
#define NAME_MODULE_FREE "ModuleFree"

#define NAME_GLOBAL_CLASS "Global"

#define NAME_GLOBAL_FUNCTION_LOAD_MODULE "loadModule"
#define NAME_GLOBAL_FUNCTION_UNLOAD_MODULE "unloadModule"


#define JL_HOSTPRIVATE_MAX_CLASS_PROTO_CACHE_BIT (9)

	
///////////////////////////////////////////////////////////////////////////////
// IDs cache

// defined here because required by jlhostprivate.h
#define JLID_SPEC(name) JLID_##name
enum {
	JLID_SPEC( global ),
	JLID_SPEC( get ),
	JLID_SPEC( read ),
	JLID_SPEC( getMatrix44 ),
	JLID_SPEC( _NI_BufferGet ),
	JLID_SPEC( _NI_StreamRead ),
	JLID_SPEC( _NI_Matrix44Get ),
	JLID_SPEC( name ),
	JLID_SPEC( length ),
	JLID_SPEC( id ),
	JLID_SPEC( valueOf ),
	JLID_SPEC( toString ),
	JLID_SPEC( next ),
	JLID_SPEC( iterator ),
	JLID_SPEC( arguments ),
	JLID_SPEC( unsafeMode ),
	JLID_SPEC( stdin ),
	JLID_SPEC( stdout ),
	JLID_SPEC( stderr ),
	JLID_SPEC( width ),
	JLID_SPEC( height ),
	JLID_SPEC( channels ),
	JLID_SPEC( bits ),
	JLID_SPEC( rate ),
	JLID_SPEC( frames ),
	JLID_SPEC( sourceId ),
	JLID_SPEC( _sourceId ),
	JLID_SPEC( buildDate ),
	JLID_SPEC( _buildDate ),
	JLID_SPEC( path ),
	JLID_SPEC( isFirstInstance ),
	JLID_SPEC( bootstrapScript ),
	JLID_SPEC( _serialize ),
	JLID_SPEC( _unserialize ),
	JLID_SPEC( eval ),
	JLID_SPEC( push ),
	JLID_SPEC( pop ),
	JLID_SPEC( toXMLString ),
	JLID_SPEC( fileName ),
	JLID_SPEC( lineNumber ),
	JLID_SPEC( stack ),
	JLID_SPEC( message ),
	JLID_SPEC( Reflect ),
	JLID_SPEC( Debugger ),
	JLID_SPEC( isGenerator ),
	JLID_SPEC( writable ),
	JLID_SPEC( readable ),
	JLID_SPEC( hangup ),
	JLID_SPEC( exception ),
	JLID_SPEC( error ),
	JLID_SPEC( position ),
	JLID_SPEC( available ),
	JLID_SPEC( data ),
	JLID_SPEC( type ),
	JLID_SPEC( doc ),

	LAST_JSID // see HostPrivate::ids[]
};
#undef JLID_SPEC
// examples:
//   JLID(cx, _unserialize) -> jsid
//   JLID_NAME(cx, _unserialize) -> w_char



JL_BEGIN_NAMESPACE

bool enableLowFragmentationHeap();


class Std {
public:
	virtual int	stdInput( char *buffer, size_t bufferLength ) { JL_IGNORE(buffer, bufferLength); return 0; };
	virtual int	stdOutput( const char *buffer, size_t length ) { JL_IGNORE(buffer, length); return 0; };
	virtual int	stdError( const char *buffer, size_t length ) { JL_IGNORE(buffer, length); return 0; };
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
};


//////////////////////////////////////////////////////////////////////////////
// Alloc
	
typedef void* (*malloc_t)( size_t ) NOTHROW;
typedef void* (*calloc_t)( size_t, size_t ) NOTHROW;
typedef void* (*memalign_t)( size_t, size_t ) NOTHROW;
typedef void* (*realloc_t)( void*, size_t ) NOTHROW;
typedef size_t (*msize_t)( void* ) NOTHROW;
typedef void (*free_t)( void* ) NOTHROW;

class Allocators {
public:
	jl_malloc_t malloc;
	jl_calloc_t calloc;
	jl_memalign_t memalign;
	jl_realloc_t realloc;
	jl_msize_t msize;
	jl_free_t free;

	Allocators() {
	}

	Allocators( jl_malloc_t malloc, jl_calloc_t calloc, jl_memalign_t memalign, jl_realloc_t realloc, jl_msize_t msize, jl_free_t free )
	: malloc(malloc), calloc(calloc), memalign(memalign), realloc(realloc), msize(msize), free(free) {
	}
};



class StdAllocators : public Allocators {
public:
	StdAllocators()
	: Allocators(::malloc, ::calloc, ::memalign, ::realloc, ::msize, ::free) {
	}
};


//////////////////////////////////////////////////////////////////////////////
// Threaded memory deallocator

class ThreadedAllocator {

	enum Constants {
		MAX_LOAD = 7, // the "load" increase by one each time the thread loop without freeing the whole memory chunk list. When MAX_LOAD is reached, memory is freed synchronously.
		BIG_ALLOC = 8192 // memory chunks bigger than BIG_ALLOC are freed synchronously.
	};

	Allocators &_current;
	static Allocators _base;

	static bool _skipCleanup;

	// block-to-free chain
	static void *_head;

	// thread stats
	static volatile int32_t _headLength;
	static volatile int _load;

	// thread actions
	enum MemThreadAction {
		memThreadExit,
		memThreadProcess
	};

	// thread handler
	static JLThreadHandler _memoryFreeThread;

	static volatile MemThreadAction _threadAction;
	static JLSemaphoreHandler _memoryFreeThreadSem;

	static bool _canTriggerFreeThread;

	static ThreadedAllocator *_owner;

	static void FASTCALL
	freeHead();

	// the thread proc
	static JLThreadFuncDecl
	memoryFreeThreadProc( void* );

	// alloc functions
	static RESTRICT_DECL void* 
	_malloc( size_t size );

	static RESTRICT_DECL void*
	_calloc( size_t num, size_t size );

	static RESTRICT_DECL void*
	_memalign( size_t alignment, size_t size );

	static void*
	_realloc( void *ptr, size_t size );

	static size_t
	_msize( void *ptr );

	static void
	_free( void *ptr );

public:
	~ThreadedAllocator();

	ThreadedAllocator(Allocators &allocators);

	void
	setSkipCleanup(bool skipCleanup) {

		_skipCleanup = skipCleanup;
	}
};


//////////////////////////////////////////////////////////////////////////////
// CountedAlloc
//
// - Helps to detect memory leaks (alloc/free balance)

class CountedAlloc {

	Allocators &_current;
	static Allocators _base;

	static CountedAlloc *_owner;

	static volatile int32_t _allocCount;
	static volatile int32_t _allocAmount;
	static volatile int32_t _freeCount;
	static volatile int32_t _freeAmount;

	static RESTRICT_DECL void*
	_malloc( size_t size );

	static RESTRICT_DECL void*
	_calloc( size_t num, size_t size );

	static RESTRICT_DECL void*
	_memalign( size_t alignment, size_t size );

	static void*
	_realloc( void *ptr, size_t size );

	static size_t
	_msize( void *ptr );

	static void
	_free( void *ptr );

public:
	~CountedAlloc();

	CountedAlloc(Allocators &current);
};


//////////////////////////////////////////////////////////////////////////////
// 

class RuntimeAccess {
public:
	virtual JSRuntime* runtime() const = 0;
	virtual JSContext* context() const = 0;
};


//////////////////////////////////////////////////////////////////////////////
// WatchDog

class WatchDog {

	RuntimeAccess &_hostRuntime;

	JLSemaphoreHandler _watchDogSemEnd;
	JLThreadHandler _watchDogThread;
	uint32_t _maybeGCInterval;

	static bool 
	operationCallback(JSContext *cx);

	static JLThreadFuncDecl
	watchDogThreadProc(void *threadArg);

public:

	WatchDog(RuntimeAccess &hostRuntime, uint32_t maybeGCInterval);

	bool
	start();

	bool
	stop();
};


//////////////////////////////////////////////////////////////////////////////
// HostRuntime

class HostRuntime : public RuntimeAccess, public jl::CppAllocators {

	// global object
	// doc: For full ECMAScript standard compliance, obj should be of a JSClass that has the JSCLASS_GLOBAL_FLAGS flag.
	// note: global_class is a global variable, but this is not an issue even if several runtimes share the same JSClass.
	static const JSClass _globalClass;
	static const JSClass _globalClass_lazy;

	JSRuntime *rt;
	JSContext *cx;

	Allocators _allocators;

	uint32_t _maybeGCInterval;

	WatchDog _watchDog;

	bool _isEnding;
	bool _skipCleanup;

public:

	static void
	setJSEngineAllocators(Allocators allocators);

	static void
	errorReporterBasic(JSContext *cx, const char *message, JSErrorReport *report);

	HostRuntime(Allocators allocators = StdAllocators(), uint32_t maybeGCInterval = 0);

	JSRuntime *
	runtime() const;
	
	JSContext *
	context() const;

	bool
	create(uint32_t maxMem = (uint32_t)-1, uint32_t maxAlloc = (uint32_t)-1, bool lazyStandardClasses = true);

	bool
	destroy(bool skipCleanup = false);
};


//////////////////////////////////////////////////////////////////////////////
// ModuleManager

class ModuleManager {

	RuntimeAccess &_hostRuntime;

	struct ModulePrivate {
		uint32_t moduleId;
		void *privateData;
	} _modulePrivate[1<<8]; // does not support more than 256 modules.

	jl::Queue _moduleList;

public:

	ModuleManager(RuntimeAccess &hostRuntime);

	bool
	hasModule(JLLibraryHandler module);

	bool
	storeModule(JLLibraryHandler module);

	bool
	loadModule(const char *libFileName, JS::HandleObject obj, JS::MutableHandleValue rval);

	bool
	releaseModules();

	bool
	freeModules();





	static ALWAYS_INLINE uint8_t
	moduleHash( const uint32_t moduleId ) {

		ASSERT( moduleId != 0 );
		//	return ((uint8_t*)&moduleId)[0] ^ ((uint8_t*)&moduleId)[1] ^ ((uint8_t*)&moduleId)[2] ^ ((uint8_t*)&moduleId)[3] << 1;
		uint32_t a = moduleId ^ (moduleId >> 16);
		return (a ^ a >> 8) & 0xFF;
	}


	ALWAYS_INLINE bool
	setModulePrivate( const uint32_t moduleId, void *modulePrivate ) {

		ASSERT( moduleId != 0 );
		ASSERT( modulePrivate );

		uint8_t id = moduleHash(moduleId);

		while ( _modulePrivate[id].moduleId != 0 ) { // assumes that modulePrivate struct is init to 0

			if ( _modulePrivate[id].moduleId == moduleId )
				return false; // module private already exist or moduleId not unique
			++id; // uses unsigned char overflow
		}
		_modulePrivate[id].moduleId = moduleId;
		_modulePrivate[id].privateData = modulePrivate;
		return true;
	}


	ALWAYS_INLINE void* FASTCALL
	getModulePrivateOrNULL( uint32_t moduleId ) {

		ASSERT( moduleId != 0 );

		uint8_t id = moduleHash(moduleId);
		uint8_t id0 = id;

		while ( _modulePrivate[id].moduleId != moduleId ) {

			++id; // uses unsigned char overflow
			if ( id == id0 )
				return NULL;
		}
		return _modulePrivate[id].privateData;
	}


	ALWAYS_INLINE void* FASTCALL
	getModulePrivate( uint32_t moduleId ) {

		ASSERT( moduleId != 0 );
		uint8_t id = moduleHash(moduleId);

	#ifdef DEBUG
		uint8_t maxid = id;
	#endif // DEBUG

		while ( _modulePrivate[id].moduleId != moduleId ) {

			++id; // uses unsigned char overflow

	#ifdef DEBUG
			ASSERT( id != maxid );
	#endif // DEBUG

		}
		return _modulePrivate[id].privateData;
	}
	

};


//////////////////////////////////////////////////////////////////////////////
// 


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

	// see:
	//   SuperFastHash (http://www.azillionmonkeys.com/qed/hash.html)
	//   FNV variants (http://isthe.com/chongo/tech/comp/fnv/)
	//   MurmurHash 1.0 (https://sites.google.com/site/murmurhash/)
	static ALWAYS_INLINE uint32_t FASTCALL
	slotHash( const char * const n ) {

		ASSERT( n != NULL );
		ASSERT( strlen(n) <= 24 );

		register uint32_t h = 0;
		if ( n[ 0] ) { h ^= n[ 0]<<0;
		if ( n[ 1] ) { h ^= n[ 1]<<4;
		if ( n[ 2] ) { h ^= n[ 2]<<1;
		if ( n[ 3] ) { h ^= n[ 3]<<5;
		if ( n[ 4] ) { h ^= n[ 4]<<2;
		if ( n[ 5] ) { h ^= n[ 5]<<6;
		if ( n[ 6] ) { h ^= n[ 6]<<3;
		if ( n[ 7] ) { h ^= n[ 7]<<7;
		if ( n[ 8] ) { h ^= n[ 8]<<4;
		if ( n[ 9] ) { h ^= n[ 9]<<8;
		if ( n[10] ) { h ^= n[10]<<5;
		if ( n[11] ) { h ^= n[11]<<0;
		if ( n[12] ) { h ^= n[12]<<6;
		if ( n[13] ) { h ^= n[13]<<1;
		if ( n[14] ) { h ^= n[14]<<7;
		if ( n[15] ) { h ^= n[15]<<2;
		if ( n[16] ) { h ^= n[16]<<8;
		if ( n[17] ) { h ^= n[17]<<3;
		if ( n[18] ) { h ^= n[18]<<0;
		if ( n[19] ) { h ^= n[19]<<4;
		if ( n[20] ) { h ^= n[20]<<1;
		if ( n[21] ) { h ^= n[21]<<5;
		if ( n[22] ) { h ^= n[22]<<2;
		if ( n[23] ) { h ^= n[23]<<6;
		}}}}}}}}}}}}}}}}}}}}}}}}
		return ((h >> 7) ^ h) & ((1<<JL_HOSTPRIVATE_MAX_CLASS_PROTO_CACHE_BIT) - 1);
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

		size_t slotIndex = slotHash(className);
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


	ALWAYS_INLINE const ClassProtoCache*
	get( const char * const className ) const {

		size_t slotIndex = slotHash(className);
		const size_t first = slotIndex;

		ASSERT( slotIndex < CACHE_LENGTH );

		for (;;) {

			// note:
			//   slot->clasp == NULL -> empty
			//   slot->clasp == removedSlotClasp() -> slot removed, but maybe next slot will match !

			const ClassProtoCache &slot = items.getConst(slotIndex);

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

		size_t slotIndex = slotHash(className);
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


class Host : public jl::CppAllocators {

//	JSRuntime *rt;

	bool _unsafeMode;
	static const uint32_t versionId;
	RuntimeAccess &_hostRuntime;
	Std &_hostStd;
	uint32_t _versionId; // used to ensure compatibility between host and modules. see JL_HOSTPRIVATE_KEY macro.
	ModuleManager _moduleManager;
	JS::PersistentRootedObject _hostObject;
	
	JS::PersistentRootedObject _objectProto;
	const JSClass *_objectClasp;

	ProtoCache< 1<<JL_HOSTPRIVATE_MAX_CLASS_PROTO_CACHE_BIT > _classProtoCache;
	StaticArray< JS::PersistentRootedId, LAST_JSID > _ids;

	static const JSErrorFormatString *
	errorCallback(void *userRef, const char *, const unsigned);

	static void
	errorReporterBasic(JSContext *cx, const char *message, JSErrorReport *report);

	static void
	errorReporter(JSContext *cx, const char *message, JSErrorReport *report);

	bool
	hostStderrWrite(const char *message, size_t length);


public:

	static ALWAYS_INLINE Host&
	getHost( JSRuntime *rt ) {

		return *static_cast<Host*>(JL_GetRuntimePrivate(rt));
	}

	static ALWAYS_INLINE Host&
	getHost( JSContext *cx ) {

		return getHost(JL_GetRuntime(cx));
	}


	Host( RuntimeAccess &hr, Std hostStd = Std(), bool unsafeMode = false );

	~Host();

	// init the host for jslibs usage (modules, errors, ...)
	bool
	create();

	bool
	destroy();

	bool
	unsafeMode() const {

		return _unsafeMode;
	}

	int
	stdInput( char *buffer, size_t bufferLength ) const {
		
		return _hostStd.stdInput(buffer, bufferLength);
	}

	int
	stdOutput( const char *buffer, size_t length ) const {

		return _hostStd.stdOutput(buffer, length);
	}

	int
	stdError( const char *buffer, size_t length ) const {

		return _hostStd.stdError(buffer, length);
	}

	bool
	report( bool isWarning, ... );

	bool
	setHostArguments(char **hostArgv, size_t hostArgc);

	bool
	setHostName( const char *hostPath, const char *hostName );


	JSObject *
	newObject();


	// CachedClassProto

	bool
	addCachedClassProto( const char * const className, JSClass * const clasp, IN JS::HandleObject proto ) {

		return _classProtoCache.add(_hostRuntime.runtime(), className, clasp, proto);
	}

	ALWAYS_INLINE const ClassProtoCache*
	getCachedClassProto( IN const char * const className ) {

		return _classProtoCache.get(className);
	}

	ALWAYS_INLINE const JS::HandleObject
	getCachedProto( IN const char * const className ) {

		const ClassProtoCache * cpc = getCachedClassProto(className);
		if ( cpc )
			return JS::HandleObject::fromMarkedLocation(cpc->proto.address());
		else
			return JS::NullPtr();
	}
	
	ALWAYS_INLINE const JSClass*
	getCachedClasp( IN const char * const className ) {

		return getCachedClassProto(className)->clasp;
	}


	// ids

	static INLINE NEVER_INLINE void FASTCALL
	getPrivateJsidSlow( JSContext *cx, JS::PersistentRootedId &id, int index, const jschar *name ) {

		JS::RootedString jsstr(cx, JS_InternUCString(cx, name));
		ASSERT( jsstr != NULL );
		id.set(JL_StringToJsid(cx, jsstr));
	}

		
	ALWAYS_INLINE JS::HandleId
	getId( int index, const jschar *name ) {

		JS::PersistentRootedId &id = _ids.get(index);
		if ( JSID_IS_VOID(id) ) {

			getPrivateJsidSlow(_hostRuntime.context(), id, index, name);
		}
		return JS::HandleId::fromMarkedLocation(id.address());
	}
};


JL_END_NAMESPACE


