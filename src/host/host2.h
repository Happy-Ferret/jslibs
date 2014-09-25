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
  
/*
      runtime/context -- runtimePrivate <- HostRuntime
             |                               ^    ^
             |                               |    |
  compartment/global <-------------------- Global |
             |                                 ^  |
             |                                 |  |
             +--------- compartmentPrivate <- JLHost


  JS_SetRuntimePrivate
  JS_SetContextPrivate
  JS_GetSecondContextPrivate
  JS_SetCompartmentPrivate
  JS_SetPrivate
 */

static const moduleId_t FREE_MODULE_SLOT = 0;

#define NAME_GLOBAL_CLASS "Global"

#define NAME_GLOBAL_FUNCTION_LOAD_MODULE "loadModule"
#define NAME_GLOBAL_FUNCTION_UNLOAD_MODULE "unloadModule"


#define JL_HOST_VERSIONID (uint32_t((jl::SvnRevToInt("$Revision: 3524 $") << 16) | (sizeof(jl::Host) & 0xFFFF)))

#define JL_MAX_CLASS_PROTO_CACHE_BIT (9)

extern DLLAPI bool _unsafeMode;

#pragma warning(disable : 4530)
#include <vector>
#include <list>

#include <jlalloc.h>
#include <queue.h>

#define JLID_SPEC(name) JLID_##name
enum {
	JLID_SPEC( global ),
	JLID_SPEC( host ),
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
	JLID_SPEC( source ),
	JLID_SPEC( done ),
	JLID_SPEC( value ),
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
	JLID_SPEC( string ),
	LAST_JSID // see HostPrivate::ids[]
};
#undef JLID_SPEC


JL_BEGIN_NAMESPACE


class StdIO {
public:
	virtual int	input( jl::BufString & ) {

		return 0;
	}
	virtual int	output( jl::StrDataSrc & ) {

		return 0;
	}
	virtual int	error( jl::StrDataSrc & ) {
		
		return 0;
	}
};


//////////////////////////////////////////////////////////////////////////////
// Valid

class DLLAPI Valid {
	bool _valid;
public:
	Valid() :
	_valid(true) {
	}

	operator bool() const {
		
		return _valid;
	}
	
	void
	invalidate() {

		_valid = false;
	}
};


//////////////////////////////////////////////////////////////////////////////
// Event system

template <class EVENT>
class Observer {
public:
	virtual ~Observer() {}
	virtual bool operator ()(EVENT &ev) = 0;
};

template <class EVENT>
class Observable {
	std::list<Observer<EVENT>*> _observers;
public:
	Observable() {}
	virtual ~Observable() {}
	void addObserver(Observer<EVENT> *observer) {

		_observers.push_back(observer);
	}
	void removeObserver(Observer<EVENT> *observer) {

		_observers.remove(observer);
	}
	
	bool notify(EVENT &ev) {

		std::list<Observer<EVENT>*>::iterator it;
		for (it = _observers.begin(); it != _observers.end(); it++)
			if ( (**it)(ev) )
				return false;
		return true;
	}
};


/*

template <class T>
class Callback {
public:
	typedef T EventType;
	virtual bool operator()( const T& ev ) = 0;
};

template <class T>
class Events {

public:

	typedef jl::Queue1<Callback<T>*> List;
	typedef typename List::Item ListItem;


private:
	List _list;

public:
	typename ListItem *
	addListener( Callback<T>* cb ) {

		_list.AddEnd();
		_list.Last() = cb;
		return _list.Last();
	}

	void
	removeListener( ListItem *item ) {

		_list.Remove(item);
	}

	// returns false on the first listener that returns false (error)
	bool 
	fireEvent( const T& ev ) const {

		for ( ListItem *it = _list.First(); it; it = it->next )
			if ( !(*it->data.cb)(ev) )
				return false;
		return true;
	}
};

*/


//////////////////////////////////////////////////////////////////////////////
// Alloc

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

class DLLAPI ThreadedAllocator {

	enum Constants {
		MAX_LOAD = 7, // the "load" increase by one each time the thread loop without freeing the whole memory chunk list. When MAX_LOAD is reached, memory is freed synchronously.
		BIG_ALLOC = 8192 // memory chunks bigger than BIG_ALLOC are freed synchronously.
	};

	Allocators &_current;
	static Allocators _base;

	static volatile bool _skipCleanup;

	// block-to-free chain
	static volatile void *_head;

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

	static volatile bool _canTriggerFreeThread;

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

class DLLAPI CountedAlloc {

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



// end of tools



//////////////////////////////////////////////////////////////////////////////
// WatchDog

class HostRuntime;

class WatchDog {

	HostRuntime &_hostRuntime;

	JLSemaphoreHandler _watchDogSemEnd;
	JLThreadHandler _watchDogThread;
	volatile uint32_t _interruptInterval;

	static bool 
	interruptCallback(JSContext *cx);

	static JLThreadFuncDecl
	watchDogThreadProc(void *threadArg);

	void
	start();

	void
	stop();

public:

	~WatchDog() {

		if ( _interruptInterval != 0 )
			stop();
	}

	WatchDog(HostRuntime &hostRuntime);

	uint32_t
	interruptInterval() const {

		return _interruptInterval;
	}
	
	void
	setInterruptInterval( uint32_t interruptIntervalMs ) {
		
		if ( _interruptInterval == 0 && interruptIntervalMs != 0 ) {

			_interruptInterval = interruptIntervalMs;
			start();
		} else			 
		if ( _interruptInterval != 0 && interruptIntervalMs == 0 ) {
			
			_interruptInterval = interruptIntervalMs;
			stop();
		} else {

			_interruptInterval = interruptIntervalMs;
		}
	}

};


//////////////////////////////////////////////////////////////////////////////
// HostRuntime

// see: http://lxr.mozilla.org/mozilla-central/source/js/xpconnect/src/XPCJSRuntime.cpp#3217
// see: http://lxr.mozilla.org/mozilla-central/source/dom/workers/RuntimeService.cpp#95
#if defined(MOZ_ASAN) || (defined(DEBUG) && !defined(XP_WIN))
static const size_t defaultNativeStackQuota = 2 * 128 * sizeof(size_t) * 1024;
#else
static const size_t defaultNativeStackQuota = 128 * sizeof(size_t) * 1024; // 512KB
#endif

/*
enum HostRuntimeEvents {
	BEFORE_DESTROY_RUNTIME,
	AFTER_DESTROY_RUNTIME,
	DESTRUCT_HOSTRUNTIME,
	DESTROY_COMPARTMENT,
	INTERRUPT,
};
*/

struct EventBeforeDestroyRuntime {
};

struct EventAfterDestroyRuntime {
};

struct EventDestroyHostRuntime {
};

struct EventDestroyCompartment {
};

struct EventInterrupt {
	JSContext *cx;
	EventInterrupt(JSContext *cx) : cx(cx) {}
};

class DLLAPI HostRuntime :
	public Valid,
	public Observable<EventBeforeDestroyRuntime>,
	public Observable<EventAfterDestroyRuntime>,
	public Observable<EventDestroyHostRuntime>,
	public Observable<EventDestroyCompartment>,
	public Observable<EventInterrupt>,
	public jl::CppAllocators
{
public:
	using Observable<EventBeforeDestroyRuntime>::addObserver;
	using Observable<EventBeforeDestroyRuntime>::removeObserver;
	using Observable<EventBeforeDestroyRuntime>::notify;

	using Observable<EventAfterDestroyRuntime>::addObserver;
	using Observable<EventAfterDestroyRuntime>::removeObserver;
	using Observable<EventAfterDestroyRuntime>::notify;

	using Observable<EventDestroyHostRuntime>::addObserver;
	using Observable<EventDestroyHostRuntime>::removeObserver;
	using Observable<EventDestroyHostRuntime>::notify;

	using Observable<EventDestroyCompartment>::addObserver;
	using Observable<EventDestroyCompartment>::removeObserver;
	using Observable<EventDestroyCompartment>::notify;

	using Observable<EventInterrupt>::addObserver;
	using Observable<EventInterrupt>::removeObserver;
	using Observable<EventInterrupt>::notify;
private:

	JSRuntime *rt;
//	JSContext *cx;

	Allocators _allocators;

	WatchDog _watchDog;

	bool _isEnding;
	bool _skipCleanup;
	bool _unsafeMode;

public: // static

	static void
	setJSEngineAllocators(const Allocators &allocators) {

		::js_jl_malloc = allocators.malloc;
		::js_jl_calloc = allocators.calloc;
		::js_jl_realloc = allocators.realloc;
		::js_jl_free = allocators.free;
	}

	static void
	setHostAllocators(const Allocators &allocators) {

		::jl_malloc = allocators.malloc;
		::jl_calloc = allocators.calloc;
		::jl_memalign = allocators.memalign;
		::jl_realloc = allocators.realloc;
		::jl_msize = allocators.msize;
		::jl_free = allocators.free;
	}

	static void
	errorReporterBasic(JSContext *cx, const char *message, JSErrorReport *report);

	static void
	HostRuntime::destroyCompartmentCallback(JSFreeOp *fop, JSCompartment *compartment);

	static void
	HostRuntime::destroyZoneCallback(JS::Zone *zone);

	static HostRuntime&
	getJLRuntime( JSRuntime *rt ) {

		//return static_cast<Host*>(JL_GetRuntimePrivate(rt))->hostRuntime();
		HostRuntime* hostRuntime = static_cast<HostRuntime*>(JL_GetRuntimePrivate(rt));
		ASSERT( hostRuntime );
		ASSERT( *hostRuntime );
		return *hostRuntime;
	}

	static HostRuntime&
	getJLRuntime( JSContext *cx ) {

		return getJLRuntime(JL_GetRuntime(cx));
	}

public:

	~HostRuntime();

	HostRuntime(Allocators allocators/* = StdAllocators()*/, bool unsafeMode = false, uint32_t maxbytes = JS::DefaultHeapMaxBytes, size_t nativeStackQuota = defaultNativeStackQuota);

	JSRuntime *&
	runtime() {
		
		ASSERT( *this );
		return rt;
	}
	
	JSContext *
	createContext();


/*
	JSContext *&
	context() {

		ASSERT( *this );
		return cx;
	}
*/

	ALWAYS_INLINE const Allocators &
	allocators() const {
	
		return _allocators;
	}

	ALWAYS_INLINE bool
	skipCleanup() const {

		return _skipCleanup;
	}

	ALWAYS_INLINE void
	setSkipCleanup() {

		_skipCleanup = true;
	}

	ALWAYS_INLINE bool
	isEnding() const {

		return _isEnding;
	}

	ALWAYS_INLINE bool
	unsafeMode() const {

		return _unsafeMode;
	}



	uint32_t
	interruptInterval() const {

		return _watchDog.interruptInterval();
	}
	
	void
	setInterruptInterval( uint32_t interruptIntervalMs ) {
		
		_watchDog.setInterruptInterval(interruptIntervalMs);
	}
};



//////////////////////////////////////////////////////////////////////////////
// ModuleManager

#define MAX_MODULES 256


class DLLAPI ModuleManager {

	struct Module {
		moduleId_t moduleId;
		JLLibraryHandler moduleHandle; // JLDynamicLibraryNullHandler if uninitialized
		void *privateData; // each module has its user data

		Module() :
			moduleId(FREE_MODULE_SLOT),
			moduleHandle(JLDynamicLibraryNullHandler),
			privateData(nullptr) {
		}

/* unused
		Module(moduleId_t moduleId, JLLibraryHandler moduleHandle, void *privateData) :
			moduleId(moduleId),
			moduleHandle(moduleHandle),
			privateData(privateData) {
		}
*/
	};

//	friend class Host;

	Module _moduleList[MAX_MODULES];
	uint16_t _moduleCount;


	ALWAYS_INLINE uint16_t
	moduleHash( const moduleId_t moduleId ) {

		ASSERT( moduleId != FREE_MODULE_SLOT );
		return moduleId % MAX_MODULES;
		//	return ((uint8_t*)&moduleId)[0] ^ ((uint8_t*)&moduleId)[1] ^ ((uint8_t*)&moduleId)[2] ^ ((uint8_t*)&moduleId)[3] << 1;
		// uint32_t a = moduleId ^ (moduleId >> 16);
		// return (a ^ a >> 8) & 0xFF;
	}

public:

	~ModuleManager();

	ModuleManager();

	bool
	loadModule(JSContext *cx, const char *libFileName, JS::HandleObject obj, JS::MutableHandleValue rval);

	bool
	addLocalModule(JSContext *cx, const char *moduleName, ModuleInitFunction initFunction, JS::HandleObject obj);

/* unused
	ALWAYS_INLINE Module &
	getFreeModuleSlot( const moduleId_t moduleId ) {

		ASSERT( moduleId != FREE_MODULE_SLOT );
		uint16_t hashIndex = moduleHash(moduleId);
		while ( _moduleList[hashIndex].moduleId != FREE_MODULE_SLOT ) {

			if ( ++hashIndex >= MAX_MODULES )
				hashIndex = 0;
		}
		return _moduleList[hashIndex];
	}
*/

	ALWAYS_INLINE bool
	isSlotFree( const Module &module ) const {

		return module.moduleId == FREE_MODULE_SLOT;
	}

	ALWAYS_INLINE Module &
	moduleSlot( const moduleId_t moduleId ) {

		ASSERT( moduleId != FREE_MODULE_SLOT );
		uint16_t hashIndex = moduleHash(moduleId);
		
		while ( !isIn(_moduleList[hashIndex].moduleId, FREE_MODULE_SLOT, moduleId) ) {

			if ( ++hashIndex >= MAX_MODULES )
				hashIndex = 0;
		}
		return _moduleList[hashIndex];
	}

	ALWAYS_INLINE bool FASTCALL
	hasModulePrivate( const moduleId_t moduleId ) {

		ASSERT( moduleId != FREE_MODULE_SLOT );
		return moduleSlot(moduleId).privateData != nullptr;
	}

	ALWAYS_INLINE void* & FASTCALL
	modulePrivate( const moduleId_t moduleId ) {

		ASSERT( moduleId != FREE_MODULE_SLOT );
		return moduleSlot(moduleId).privateData;
	}

	template <class T>
	ALWAYS_INLINE T & FASTCALL
	modulePrivateT( const moduleId_t moduleId ) {

		ASSERT( moduleId != FREE_MODULE_SLOT );
		return (T&)moduleSlot( moduleId ).privateData;
	}

};


//////////////////////////////////////////////////////////////////////////////
// 


// does not support more than (1<<MAX_CLASS_PROTO_CACHE_BIT)-1 proto.

class DLLAPI ProtoCache {

	const JSClass*
	removedSlotClasp() const {

		return reinterpret_cast<JSClass*>(-1);
	}

public:

	class Item {
	public:
		const JSClass *clasp;
		JS::PersistentRootedObject proto;

		Item(JSContext *cx)
		: clasp(NULL), proto(cx) {
		}

		Item(JSContext *cx, const JSClass *c, JS::HandleObject p)
		: clasp(c), proto(cx, p) {
		}
	};

	StaticArray< Item, 1<<JL_MAX_CLASS_PROTO_CACHE_BIT > items;

	void removeAll() {

		for ( int i = 0; i < items.length; ++i ) {
			
			if ( items.getConst(i).clasp != NULL && items.getConst(i).clasp != removedSlotClasp() ) {

				items.destruct(i);
				items.get(i).clasp = NULL;
			}
		}
	}

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
	slotHash( const char *n ) {

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
		return ((h >> 7) ^ h) & (( 1<<JL_MAX_CLASS_PROTO_CACHE_BIT ) - 1);
	}


	bool
	add( JSContext *cx, const char *className, const JSClass * const clasp, IN JS::HandleObject proto ) {

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

			const ProtoCache::Item &slot = items.getConst(slotIndex);

			if ( slot.clasp == NULL ) {

				items.construct(slotIndex, cx, clasp, proto);
				return true;
			}

			if ( slot.clasp == clasp ) // already cached
				return false;

			slotIndex = (slotIndex + 1) % (1<<JL_MAX_CLASS_PROTO_CACHE_BIT);

			if ( slotIndex == first ) // no more free slot
				return false;
		}
	}


	ALWAYS_INLINE const Item*
	get( const char *className ) const {

		size_t slotIndex = slotHash(className);
		const size_t first = slotIndex;

		ASSERT( slotIndex < (1<<JL_MAX_CLASS_PROTO_CACHE_BIT) );

		for (;;) {

			// note:
			//   slot->clasp == NULL -> empty
			//   slot->clasp == removedSlotClasp() -> slot removed, but maybe next slot will match !

			const ProtoCache::Item &slot = items.getConst(slotIndex);

			if ( slot.clasp == NULL ) // not found
				return NULL;

			if ( slot.clasp != removedSlotClasp() && ( slot.clasp->name == className || !strcmp(slot.clasp->name, className) ) ) // see "Enable String Pooling"
				return &slot;

			slotIndex = (slotIndex + 1) % (1<<JL_MAX_CLASS_PROTO_CACHE_BIT);

			if ( slotIndex == first ) // not found
				return NULL;
		}
	}

	
	void
	remove( const char *const className ) {

		ASSERT( removedSlotClasp() != NULL );

		size_t slotIndex = slotHash(className);
		size_t first = slotIndex;
		
		ASSERT( slotIndex < (1<<JL_MAX_CLASS_PROTO_CACHE_BIT) );

		for (;;) {

			ProtoCache::Item &slot = items.get(slotIndex);

			if ( slot.clasp == NULL || ( slot.clasp != removedSlotClasp() && ( slot.clasp->name == className || strcmp(slot.clasp->name, className) == 0 ) ) ) {
			
				items.destruct(slotIndex);
				slot.clasp = removedSlotClasp();
				return;
			}

			slotIndex = (slotIndex + 1) % (1<<JL_MAX_CLASS_PROTO_CACHE_BIT);

			if ( slotIndex == first ) // not found
				return;
		}
	}
};


class DLLAPI ErrorManager {
public:
	class DLLAPI ErrArg {
	public:	
		enum ErrArgType {
			UNDEFINED,
			STRING,
			WSTRING,
			INTEGER,
			FLOAT
		};

	private:
		ErrArgType _type;

		union {
			const char * _string;
			const wchar_t * _wstring;
			long int _integer;
			float _float;
		};

	public:
		ErrArg()
		: _type(ErrArg::UNDEFINED) {
		}

		ErrArg( int val )
		: _type(ErrArg::INTEGER), _integer(val) {
		}

		ErrArg( size_t val )
		: _type(ErrArg::INTEGER), _integer(val) {
		}

		ErrArg( long val )
		: _type(ErrArg::INTEGER), _integer(val) {
		}

		ErrArg( unsigned long val )
		: _type(ErrArg::INTEGER), _integer(val) {
		}

		ErrArg( float val )
		: _type(ErrArg::FLOAT), _float(val) {
		}

		ErrArg( double val )
		: _type(ErrArg::FLOAT), _float(float(val)) {
		}

		ErrArg( const char * val )
		: _type(ErrArg::STRING), _string(val) {
		}

		ErrArg( const wchar_t * val )
		: _type(ErrArg::WSTRING), _wstring(val) {
		}

		ErrArg( jl::StrDataSrc & val );

		ErrArgType
		type() const {

			return _type;
		}

		template <char TYPE>
		bool is() const {

			return type() == TYPE;
		}

		const char * asString() const {

			ASSERT(is<STRING>());
			return _string;
		};

		const wchar_t * asWstring() const {

			ASSERT(is<WSTRING>());
			return _wstring;
		};

		long int asInteger() const {

			ASSERT(is<INTEGER>());
			return _integer;
		};

		float asFloat() const {

			ASSERT(is<FLOAT>());
			return _float;
		};
	};

private:

	struct ErrorList {
		const wchar_t *name;
		const JSExnType exn;
	};

	static const ErrorList _errorList[];

	static const wchar_t * _defaultMessages[];
	const wchar_t ** _currentMessages;

public:

	ErrorManager()
	: _currentMessages(_defaultMessages) {
	}

	~ErrorManager() {

		restoreDefaultMessages();
	}

	void restoreDefaultMessages() {

		if ( _currentMessages != _defaultMessages && _currentMessages != nullptr ) {

			for ( size_t i = 0; i < E__END; ++i )
				jl_free( const_cast<wchar_t*>(_currentMessages[i]) );
			jl_free( _currentMessages );
		}
		_currentMessages = _defaultMessages;
	}


	bool importMessages( JSContext *cx, JS::HandleObject messagesObj );

	bool exportMessages( JSContext *cx, JS::MutableHandleObject messagesObj );


	static const JSErrorFormatString *
	errorCallback( void *userRef, const unsigned errorNumber );


	INLINE NEVER_INLINE
	bool
	report( JSContext *cx, bool isWarning, size_t argc, const ErrArg *args ) const;

/*
	bool report( bool isWarning, ErrArg a0 ) const {
		const ErrArg args[] = { a0 }; return report( isWarning, COUNTOF(args), args );
	}

	bool report( bool isWarning, ErrArg a0, ErrArg a1 ) const {
		const ErrArg args[] = { a0, a1 }; return report( isWarning, COUNTOF(args), args );
	}

	bool report( bool isWarning, ErrArg a0, ErrArg a1, ErrArg a2 ) const {
		const ErrArg args[] = { a0, a1, a2 }; return report( isWarning, COUNTOF(args), args );
	}

	bool report( bool isWarning, ErrArg a0, ErrArg a1, ErrArg a2, ErrArg a3 ) const {
		const ErrArg args[] = { a0, a1, a2, a3 }; return report( isWarning, COUNTOF(args), args );
	}

	bool report( bool isWarning, ErrArg a0, ErrArg a1, ErrArg a2, ErrArg a3, ErrArg a4 ) const {
		const ErrArg args[] = { a0, a1, a2, a3, a4 }; return report( isWarning, COUNTOF(args), args );
	}

	bool report( bool isWarning, ErrArg a0, ErrArg a1, ErrArg a2, ErrArg a3, ErrArg a4, ErrArg a5 ) const {
		const ErrArg args[] = { a0, a1, a2, a3, a4, a5 }; return report( isWarning, COUNTOF(args), args );
	}

	bool report( bool isWarning, ErrArg a0, ErrArg a1, ErrArg a2, ErrArg a3, ErrArg a4, ErrArg a5, ErrArg a6 ) const {
		const ErrArg args[] = { a0, a1, a2, a3, a4, a5, a6 }; return report( isWarning, COUNTOF(args), args );
	}

	bool report( bool isWarning, ErrArg a0, ErrArg a1, ErrArg a2, ErrArg a3, ErrArg a4, ErrArg a5, ErrArg a6, ErrArg a7 ) const {
		const ErrArg args[] = { a0, a1, a2, a3, a4, a5, a6, a7 }; return report( isWarning, COUNTOF(args), args );
	}

	bool report( bool isWarning, ErrArg a0, ErrArg a1, ErrArg a2, ErrArg a3, ErrArg a4, ErrArg a5, ErrArg a6, ErrArg a7, ErrArg a8 ) const {
		const ErrArg args[] = { a0, a1, a2, a3, a4, a5, a6, a7, a8 }; return report( isWarning, COUNTOF(args), args );
	}

	bool report( bool isWarning, ErrArg a0, ErrArg a1, ErrArg a2, ErrArg a3, ErrArg a4, ErrArg a5, ErrArg a6, ErrArg a7, ErrArg a8, ErrArg a9 ) const {
		const ErrArg args[] = { a0, a1, a2, a3, a4, a5, a6, a7, a8, a9 }; return report( isWarning, COUNTOF(args), args );
	}

	bool report( bool isWarning, ErrArg a0, ErrArg a1, ErrArg a2, ErrArg a3, ErrArg a4, ErrArg a5, ErrArg a6, ErrArg a7, ErrArg a8, ErrArg a9, ErrArg a10 ) const {
		const ErrArg args[] = { a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10 }; return report( isWarning, COUNTOF(args), args );
	}

	bool report( bool isWarning, ErrArg a0, ErrArg a1, ErrArg a2, ErrArg a3, ErrArg a4, ErrArg a5, ErrArg a6, ErrArg a7, ErrArg a8, ErrArg a9, ErrArg a10, ErrArg a11 ) const {
		const ErrArg args[] = { a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11 }; return report( isWarning, COUNTOF(args), args );
	}

	bool report( bool isWarning, ErrArg a0, ErrArg a1, ErrArg a2, ErrArg a3, ErrArg a4, ErrArg a5, ErrArg a6, ErrArg a7, ErrArg a8, ErrArg a9, ErrArg a10, ErrArg a11, ErrArg a12 ) const {
		const ErrArg args[] = { a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12 }; return report( isWarning, COUNTOF(args), args );
	}

	bool report( bool isWarning, ErrArg a0, ErrArg a1, ErrArg a2, ErrArg a3, ErrArg a4, ErrArg a5, ErrArg a6, ErrArg a7, ErrArg a8, ErrArg a9, ErrArg a10, ErrArg a11, ErrArg a12, ErrArg a13 ) const {
		const ErrArg args[] = { a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13 }; return report( isWarning, COUNTOF(args), args );
	}

	bool report( bool isWarning, ErrArg a0, ErrArg a1, ErrArg a2, ErrArg a3, ErrArg a4, ErrArg a5, ErrArg a6, ErrArg a7, ErrArg a8, ErrArg a9, ErrArg a10, ErrArg a11, ErrArg a12, ErrArg a13, ErrArg a14 ) const {
		const ErrArg args[] = { a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14 }; return report( isWarning, COUNTOF(args), args );
	}
*/
};


class DLLAPI Global : public Valid, public jl::CppAllocators {

	// global object
	// doc: For full ECMAScript standard compliance, obj should be of a JSClass that has the JSCLASS_GLOBAL_FLAGS flag.
	// note: global_class is a global variable, but this is not an issue even if several runtimes share the same JSClass.

	static bool
	_lazy_enumerate(JSContext *cx, JS::HandleObject obj);

	static bool
	Global::_lazy_resolve(JSContext *cx, JS::HandleObject obj, JS::HandleId id, JS::MutableHandleObject objp);

	static const JSClass _globalClass;
	static const JSClass _globalClass_lazy;

	JS::PersistentRootedObject _globalObject;

private:
	Global( const Global & );
	Global & operator ==( const Global & );
public:

	Global( JSContext *cx, bool lazyStandardClasses = true );

	~Global() {
		
		IFDEBUG( invalidate() );
		IFDEBUG( _globalObject.set(nullptr); );
	}

	JSObject *
	globalObject() const {

		ASSERT( *this );
		return _globalObject;		
	}

	JSCompartment *
	compartment() const {

		 return js::GetObjectCompartment(globalObject());
	}

};


class EventHostDestroy {
};


class DLLAPI Host :
	public Valid,
	public Observable<EventHostDestroy>,
	public jl::CppAllocators
{

	HostRuntime &_hostRuntime;
	Global &_global;
	ModuleManager _moduleManager;

	JS::PersistentRootedObject _hostObject;
	StdIO &_hostStdIO;
	const uint32_t _compatId; // used to ensure compatibility between host and modules. see JL_HOST_VERSIONID macro.
	JS::PersistentRootedObject _objectProto;
	const JSClass *_objectClasp;
	ProtoCache _classProtoCache;
	StaticArray< JS::PersistentRootedId, LAST_JSID > _ids;
	ErrorManager _errorManager;
//	Events<EventHostDestroy>::List::Item *_interruptHandler;

//	static void
//	errorReporterBasic( JSContext *cx, const char *message, JSErrorReport *report );

	static void
	errorReporter( JSContext *cx, const char *message, JSErrorReport *report );

	bool
	hostStderrWrite( JSContext *cx, const TCHAR *message, size_t length );

	INLINE NEVER_INLINE void FASTCALL
	fillPrivateJsidSlow( JSContext *cx, JS::PersistentRootedId &id, const jschar *name ) {

		id.set(stringToJsid(cx, name));
	}

private:
	Host( const Host & );
	Host & operator ==( const Host & );
public:
	~Host();

	Host( JSContext *cx, Global &glob, StdIO &hostStdIO );

	// init the host for jslibs usage (modules, errors, ...)
	
	ALWAYS_INLINE bool
	checkCompatId(uint32_t compatId) const {

		return compatId != 0 && _compatId == compatId;
	}

	ALWAYS_INLINE Global &
	global() const {
			
		return _global;
	}

	ALWAYS_INLINE StdIO &
	stdIO() const {

		return _hostStdIO;
	}

	bool
	setHostArguments( JSContext *cx, TCHAR **hostArgv, size_t hostArgc );

	bool
	setHostPath( JSContext *cx, const TCHAR *hostPath );

	bool
	setHostName( JSContext *cx, const TCHAR *hostName );


	JS::HandleObject
	hostObject() const {

		return JS::HandleObject::fromMarkedLocation(_hostObject.address());
	}


	ALWAYS_INLINE JSObject *
	newObject(JSContext *cx) const {

		return jl::newObjectWithGivenProto(cx, _objectClasp, _objectProto); // JL_GetGlobal(cx)
	}


	ALWAYS_INLINE JSObject *
	newJLObject( JSContext *cx, const char *className ) const {

		const ProtoCache::Item *cpc = _classProtoCache.get(className);
		if ( cpc != NULL )
			return jl::newObjectWithGivenProto(cx, cpc->clasp, cpc->proto);
		else
			return NULL;
	}


	// modules
	
	ALWAYS_INLINE ModuleManager&
	moduleManager() {

		return _moduleManager;
	}

	ALWAYS_INLINE ErrorManager&
	errorManager() {

		return _errorManager;
	}

/*
	// alloc

	ALWAYS_INLINE void
	getAllocators( jl_malloc_t &mallocRef, jl_calloc_t &callocRef, jl_memalign_t &memalignRef, jl_realloc_t &reallocRef, jl_msize_t &msizeRef, jl_free_t &freeRef ) {
	
		const Allocators &alloc = _hostRuntime.allocators();
		mallocRef = alloc.malloc;
		callocRef = alloc.calloc;
		memalignRef = alloc.memalign;
		reallocRef = alloc.realloc;
		msizeRef = alloc.msize;
		freeRef = alloc.free;
	}
*/

	// CachedClassProto

	bool
	addCachedClassProto( JSContext *cx, const char *className, const JSClass * const clasp, IN JS::HandleObject proto ) {

		return _classProtoCache.add(cx, className, clasp, proto);
	}

	ALWAYS_INLINE const ProtoCache::Item*
	getCachedClassProto( IN const char *className ) const {

		return _classProtoCache.get(className);
	}

	ALWAYS_INLINE const JS::HandleObject
	getCachedProto( IN const char *className ) const {

		const ProtoCache::Item * cpc = getCachedClassProto(className);
		if ( cpc )
			return JS::HandleObject::fromMarkedLocation(cpc->proto.address());
		else
			return JS::NullPtr();
	}
	
	ALWAYS_INLINE const JSClass*
	getCachedClasp( IN const char *className ) const {

		return getCachedClassProto(className)->clasp;
	}

	ALWAYS_INLINE bool
	hasCachedClassProto(IN const char *className) const {

		return _classProtoCache.get(className) != NULL;
	}

	ALWAYS_INLINE void
	removeCachedClassProto(IN const char *className) {
		
		_classProtoCache.remove(className);
	}

	// ids
	ALWAYS_INLINE JS::HandleId
	getId( JSContext *cx, int index, const jschar *name ) {

		JS::PersistentRootedId &id = _ids.get(index);
		ASSERT( !JSID_IS_ZERO(id) );
		if ( JSID_IS_VOID(id) ) {

			fillPrivateJsidSlow(cx, id, name);
		}
		return JS::HandleId::fromMarkedLocation(&id.get());
	}


// static

public:

/*
	// (TBD) remove:
	static ALWAYS_INLINE Host&
	getHost( JSRuntime *rt ) {

		return *static_cast<Host*>(JL_GetRuntimePrivate(rt));
	}

	// (TBD) remove:
	static ALWAYS_INLINE Host&
	getHost( JSContext *cx ) {

		return getHost(JL_GetRuntime(cx));
	}
*/

	static ALWAYS_INLINE bool
	hasJLHost( JSContext *cx ) {
	
		ASSERT( cx );
		return JS_GetCompartmentPrivate(js::GetContextCompartment(cx)) != nullptr;
	}

	static ALWAYS_INLINE Host&
	getJLHost( JS::HandleObject obj ) {

		ASSERT( obj );
		JSCompartment *objectCompartment = js::GetObjectCompartment(obj);
		ASSERT( objectCompartment );
		Host* pHost = static_cast<Host*>(JS_GetCompartmentPrivate(objectCompartment));
		ASSERT( pHost );
		ASSERT( *pHost );
		return *pHost;
	}

	static ALWAYS_INLINE Host&
	getJLHost( JSContext *cx ) {

		ASSERT( cx );
		JSCompartment *currentCompartment = js::GetContextCompartment(cx);
		ASSERT( currentCompartment );
		Host* pHost = static_cast<Host*>(JS_GetCompartmentPrivate(currentCompartment));
		ASSERT( pHost );
		ASSERT( *pHost );
		return *pHost;
	}
};


JL_END_NAMESPACE

	
	
	
///////////////////////////////////////////////////////////////////////////////
// IDs cache (see JLID_SPEC)

// examples:
//   JLID(cx, _unserialize) -> jsid
//   JLID_NAME(cx, _unserialize) -> w_char


#ifdef DEBUG
#define JLID_NAME(cx, name) (JL_IGNORE(cx), JL_IGNORE(JLID_##name), L(#name))
#else
#define JLID_NAME(cx, name) (#name)
#endif // DEBUG

#define JLID(cx, name) (jl::Host::getJLHost(cx).getId(cx, JLID_##name, L(#name)))

// eg:
//   jsid cfg = JLID(cx, fileName); const char *name = JLID_NAME(fileName);


//////////////////////////////////////////////////////////////////////////////
// the following helper functions depends on the host object


ALWAYS_INLINE JSObject* FASTCALL
JL_NewObj( JSContext *cx ) {

	return jl::Host::getJLHost(cx).newObject(cx);
}


ALWAYS_INLINE JSObject* FASTCALL
JL_NewJslibsObject( JSContext *cx, const char *className ) {

	return jl::Host::getJLHost(cx).newJLObject(cx, className);
}

