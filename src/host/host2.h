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

typedef ptrdiff_t moduleId_t;

#define NAME_GLOBAL_CLASS "Global"

#define NAME_GLOBAL_FUNCTION_LOAD_MODULE "loadModule"
#define NAME_GLOBAL_FUNCTION_UNLOAD_MODULE "unloadModule"


#define JL_HOST_VERSIONID (uint32_t((jl::SvnRevToInt("$Revision: 3524 $") << 16) | (sizeof(jl::Host) & 0xFFFF)))

#define JL_MAX_CLASS_PROTO_CACHE_BIT (9)

extern DLLAPI bool _unsafeMode;

#include <jlalloc.h>


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
	virtual int	output( jl::BufString & ) {

		return 0;
	}
	virtual int	error( jl::BufString & ) {
		
		return 0;
	}
};



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
	uint32_t _maybeGCInterval;

	static bool 
	interruptCallback(JSContext *cx);

	static JLThreadFuncDecl
	watchDogThreadProc(void *threadArg);

public:

	WatchDog(HostRuntime &hostRuntime, uint32_t maybeGCInterval);

	bool
	start();

	bool
	stop();
};


//////////////////////////////////////////////////////////////////////////////
// HostRuntime

class DLLAPI HostRuntime : public jl::CppAllocators {

	// global object
	// doc: For full ECMAScript standard compliance, obj should be of a JSClass that has the JSCLASS_GLOBAL_FLAGS flag.
	// note: global_class is a global variable, but this is not an issue even if several runtimes share the same JSClass.
	static const JSClass _globalClass;
	static const JSClass _globalClass_lazy;

	JSContext *cx;
	JSRuntime *rt;

	Allocators _allocators;

	uint32_t _maybeGCInterval;

	WatchDog _watchDog;

	bool _isEnding;
	bool _skipCleanup;

public: // static

	static void
	setJSEngineAllocators(Allocators allocators);

	static void
	errorReporterBasic(JSContext *cx, const char *message, JSErrorReport *report);

public:

	HostRuntime(Allocators allocators = StdAllocators(), uint32_t maybeGCInterval = 0);

	JSRuntime *
	runtime() const {

		return rt;
	}
	
	JSContext *
	context() const {

		return cx;
	}

	ALWAYS_INLINE const Allocators &
	allocators() const {
	
		return _allocators;
	}

	ALWAYS_INLINE bool
	skipCleanup() const {

		return _skipCleanup;
	}

	ALWAYS_INLINE bool
	isEnding() const {

		return _isEnding;
	}

	bool
	create(uint32_t maxMem = (uint32_t)-1, uint32_t maxAlloc = (uint32_t)-1, size_t nativeStackQuota = 0, bool lazyStandardClasses = true);

	bool
	destroy(bool skipCleanup = false);
};



//////////////////////////////////////////////////////////////////////////////
// ModuleManager

#define MAX_MODULES 256


class DLLAPI ModuleManager {

	struct Module {
		moduleId_t moduleId; // 0 = free slot
		JLLibraryHandler moduleHandle; // JLDynamicLibraryNullHandler if uninitialized
		void *privateData; // user data

		Module() : moduleId(0), moduleHandle(JLDynamicLibraryNullHandler), privateData(NULL) {
		}
	};

	friend class Host;

	HostRuntime &_hostRuntime;
	Module _moduleList[MAX_MODULES];
	uint16_t _moduleCount;


	ALWAYS_INLINE uint16_t
	moduleHash( const moduleId_t moduleId ) {

		ASSERT( moduleId != 0 );
		return moduleId % MAX_MODULES;
		//	return ((uint8_t*)&moduleId)[0] ^ ((uint8_t*)&moduleId)[1] ^ ((uint8_t*)&moduleId)[2] ^ ((uint8_t*)&moduleId)[3] << 1;
		// uint32_t a = moduleId ^ (moduleId >> 16);
		// return (a ^ a >> 8) & 0xFF;
	}



public:

	ModuleManager(HostRuntime &hostRuntime);

	bool
	loadModule(const char *libFileName, JS::HandleObject obj, JS::MutableHandleValue rval);

	bool
	releaseModules();

	void
	freeModules(bool skipCleanup);

/*
	ALWAYS_INLINE Module &
	getFreeModuleSlot( const moduleId_t moduleId ) {

		ASSERT( moduleId );
		uint16_t hashIndex = moduleHash(moduleId);
		while ( _moduleList[hashIndex].moduleId != 0 ) {

			if ( ++hashIndex >= MAX_MODULES )
				hashIndex = 0;
		}
		return _moduleList[hashIndex];
	}
*/

	ALWAYS_INLINE bool
	isSlotFree( const Module &module ) const {

		return module.moduleId == 0;
	}

	ALWAYS_INLINE Module &
	moduleSlot( const moduleId_t moduleId ) {

		ASSERT( moduleId );
		uint16_t hashIndex = moduleHash(moduleId);
		
		while ( !isIn(_moduleList[hashIndex].moduleId, 0, moduleId) ) {

			if ( ++hashIndex >= MAX_MODULES )
				hashIndex = 0;
		}
		return _moduleList[hashIndex];
	}

	ALWAYS_INLINE void* & FASTCALL
	modulePrivate( const moduleId_t moduleId ) {

		ASSERT( moduleId );
		return moduleSlot(moduleId).privateData;
	}

	template <class T>
	ALWAYS_INLINE T & FASTCALL
	modulePrivateT( const moduleId_t moduleId ) {

		ASSERT( moduleId );
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

		Item(JSRuntime *rt)
		: clasp(NULL), proto(rt) {
		}

		Item(JSRuntime *rt, const JSClass *c, JS::HandleObject p)
		: clasp(c), proto(rt, p) {
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
	add( JSRuntime *rt, const char *className, const JSClass * const clasp, IN JS::HandleObject proto ) {

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

				items.construct(slotIndex, rt, clasp, proto);
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


class DLLAPI Host : public jl::CppAllocators {

	HostRuntime &_hostRuntime;
	ModuleManager _moduleManager;
	JS::PersistentRootedObject _hostObject;
	StdIO &_hostStdIO;
	const uint32_t _compatId; // used to ensure compatibility between host and modules. see JL_HOST_VERSIONID macro.
	bool _unsafeMode;
	JS::PersistentRootedObject _objectProto;
	const JSClass *_objectClasp;
	ProtoCache _classProtoCache;
	StaticArray< JS::PersistentRootedId, LAST_JSID > _ids;

//

	static const JSErrorFormatString *
	errorCallback( void *userRef, const char *, const unsigned );

//	static void
//	errorReporterBasic( JSContext *cx, const char *message, JSErrorReport *report );

	static void
	errorReporter( JSContext *cx, const char *message, JSErrorReport *report );

	bool
	hostStderrWrite( const TCHAR *message, size_t length );

	static INLINE NEVER_INLINE void FASTCALL
	getPrivateJsidSlow( JSContext *cx, JS::PersistentRootedId &id, const jschar *name ) {

		JS::RootedString jsstr(cx, JS_InternUCStringN(cx, name, jl::strlen(name)));
		ASSERT( jsstr );
		id.set(stringToJsid(cx, jsstr));
	}


public:
	Host( HostRuntime &hr, StdIO &hostStdIO, bool unsafeMode = false );

	// init the host for jslibs usage (modules, errors, ...)
	
	bool
	create();

	bool
	destroy(bool skipCleanup = false);

	void
	free(bool skipCleanup = false);

	ALWAYS_INLINE bool
	unsafeMode() const {

		return _unsafeMode;
	}

	ALWAYS_INLINE bool
	checkCompatId(uint32_t compatId) const {

		return compatId != 0 && _compatId == compatId;
	}

	ALWAYS_INLINE HostRuntime &
	hostRuntime() const {
			
		return _hostRuntime;
	}

	ALWAYS_INLINE StdIO &
	stdIO() const {

		return _hostStdIO;
	}

	bool
	vsreport( bool isWarning, va_list vl ) const;

	bool
	report( bool isWarning, ... ) const;

	bool
	setHostArguments( TCHAR **hostArgv, size_t hostArgc );

	bool
	setHostPath( const TCHAR *hostPath );

	bool
	setHostName( const TCHAR *hostName );

	void
	setHostObject(JS::HandleObject hostObj);

	JS::HandleObject
	hostObject();


	ALWAYS_INLINE JSObject *
	newObject() {

		return jl::newObjectWithGivenProto(_hostRuntime.context(), _objectClasp, _objectProto); // JL_GetGlobal(cx)
	}


	ALWAYS_INLINE JSObject *
	newJLObject( const char *className ) const {

		const ProtoCache::Item *cpc = _classProtoCache.get(className);
		if ( cpc != NULL )
			return jl::newObjectWithGivenProto(_hostRuntime.context(), cpc->clasp, cpc->proto);
		else
			return NULL;
	}


	// modules
	
	ALWAYS_INLINE ModuleManager&
	moduleManager() {

		return _moduleManager;
	}


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


	// CachedClassProto

	bool
	addCachedClassProto( const char *className, const JSClass * const clasp, IN JS::HandleObject proto ) {

		return _classProtoCache.add(_hostRuntime.runtime(), className, clasp, proto);
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
	hasCachedClassProto(IN const char *className) {

		return _classProtoCache.get(className) != NULL;
	}

	ALWAYS_INLINE void
	removeCachedClassProto(IN const char *className) {
		
		_classProtoCache.remove(className);
	}

	// ids
	ALWAYS_INLINE JS::HandleId
	getId( int index, const jschar *name ) {

		JS::PersistentRootedId &id = _ids.get(index);
		ASSERT( !JSID_IS_ZERO(id) );
		if ( JSID_IS_VOID(id) ) {

			getPrivateJsidSlow(_hostRuntime.context(), id, name);
		}
		return JS::HandleId::fromMarkedLocation(id.address());
	}


// static

private:

	static void
	setHostAllocators(Allocators allocators) {

		::jl_malloc = allocators.malloc;
		::jl_calloc = allocators.calloc;
		::jl_memalign = allocators.memalign;
		::jl_realloc = allocators.realloc;
		::jl_msize = allocators.msize;
		::jl_free = allocators.free;
	}

public:

	static ALWAYS_INLINE Host&
	getHost( JSRuntime *rt ) {

		return *static_cast<Host*>(JL_GetRuntimePrivate(rt));
	}

	static ALWAYS_INLINE Host&
	getHost( JSContext *cx ) {

		return getHost(JL_GetRuntime(cx));
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

//#define JLID(cx, name) JL_GetPrivateJsid(cx, JLID_##name, (jschar*)L(#name))
#define JLID(cx, name) (jl::Host::getHost(cx).getId(JLID_##name, L(#name)))

// eg:
//   jsid cfg = JLID(cx, fileName); const char *name = JLID_NAME(fileName);


//////////////////////////////////////////////////////////////////////////////
// the following helper functions depends on the host object


ALWAYS_INLINE JSObject* FASTCALL
JL_NewObj( JSContext *cx ) {

	return jl::Host::getHost(cx).newObject();
}


ALWAYS_INLINE JSObject* FASTCALL
JL_NewJslibsObject( JSContext *cx, const char *className ) {

	return jl::Host::getHost(cx).newJLObject(className);
}

