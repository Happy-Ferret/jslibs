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


// note: JL_HOSTPRIVATE_KEY is supposed to change each time the structure is modified.
#define JL_HOSTPRIVATE_KEY ( (jl::SvnRevToInt("$Revision: 3524 $") << 16) | ((sizeof(HostPrivate) ^ offsetof(HostPrivate, ids) ^ offsetof(HostPrivate, modulePrivate)) & 0xFFFF) )
#define JL_HOSTPRIVATE_MAX_CLASS_PROTO_CACHE_BIT (9)


template <class T, const size_t ITEM_COUNT>
class StaticArray {

	uint8_t data[ITEM_COUNT * sizeof(T)];
public:
	enum {
		length = ITEM_COUNT
	};

	T& Get(size_t slotIndex) {

		ASSERT( slotIndex < length );
//		return *reinterpret_cast<T*>(data)[slotIndex];
		T* tmp = (T*)data;
		return ((T*)data)[slotIndex];
	}

	const T& GetConst(size_t slotIndex) const {

		ASSERT( slotIndex < length );
		return ((T*)data)[slotIndex];
	}


	void Destruct(size_t item) {

		(&Get(item))->T::~T();
	}

	void Construct(size_t item) {
		
		::new (&Get(item)) T();
	}

	template <typename P1>
	void Construct(size_t item, P1 p1) {
		
		::new (&Get(item)) T(p1);
	}

	template <typename P1, typename P2>
	void Construct(size_t item, P1 p1, P2 p2) {
		
		::new (&Get(item)) T(p1, p2);
	}

	template <typename P1, typename P2, typename P3>
	void Construct(size_t item, P1 p1, P2 p2, P3 p3) {
		
		::new (&Get(item)) T(p1, p2, p3);
	}

	void DestructAll() {

		for ( size_t i = 0; i < length; ++i ) {
			
			(&Get(i))->T::~T();
		}
	}

	void ConstructAll() {
		
		for ( size_t i = 0; i < length; ++i ) {
			
			::new (&Get(i)) T();
		}
	}

	template <typename P1>
	void ConstructAll(P1 p1) {
		
		for ( size_t i = 0; i < length; ++i ) {
			
			::new (&Get(i)) T(p1);
		}
	}

	template <typename P1, typename P2>
	void ConstructAll(P1 p1, P2 p2) {
		
		for ( size_t i = 0; i < length; ++i ) {
			
			::new (&Get(i)) T(p1, p2);
		}
	}

	template <typename P1, typename P2, typename P3>
	void ConstructAll(P1 p1, P2 p2, P3 p3) {
		
		for ( size_t i = 0; i < length; ++i ) {
			
			::new (&Get(i)) T(p1, p2, p3);
		}
	}

	// ...

};


class ClassProtoCache {
public:
	JSClass *clasp;
	JS::PersistentRootedObject proto;

	ClassProtoCache(JSContext *cx) : clasp(NULL), proto(cx) {
	}

	ClassProtoCache(JSContext *cx, JSClass *c, JS::HandleObject p) : clasp(c), proto(cx, p) {
	}
};

// does not support more than (1<<MAX_CLASS_PROTO_CACHE_BIT)-1 proto.
template <const size_t CACHE_LENGTH>
struct ProtoCache {
	
	StaticArray<ClassProtoCache, CACHE_LENGTH> items;
	
	~ProtoCache() {

//		items.Destruct();

		for ( int i = 0; i < items.length; ++i ) {
			
			if ( items.Get(i).clasp != NULL && items.Get(i).clasp != (JSClass*)jlpv::RemovedSlot() ) {

				items.Destruct(i);
			}
		}
	}

	ProtoCache() {

		// set all slots as 'unused'
		for ( int i = 0; i < items.length; ++i ) {
			
			items.Get(i).clasp = NULL;
		}
	}

	bool Add(JSContext *cx, const char * const className, JSClass * const clasp, IN JS::HandleObject proto) {

		ASSERT( jlpv::RemovedSlot() != NULL );
		ASSERT( className != NULL );
		ASSERT( className[0] != '\0' );
		ASSERT( clasp != NULL );
		ASSERT( clasp != jlpv::RemovedSlot() );
		ASSERT( proto != NULL );
		ASSERT( JL_GetClass(proto) == clasp );

		size_t slotIndex = JL_ClassNameToClassProtoCacheSlot(className);
		size_t first = slotIndex;

	//	ASSERT( slotIndex < COUNTOF(hpv->classProtoCache) );

		for (;;) {

			ClassProtoCache &slot = items.Get(slotIndex);

			if ( slot.clasp == NULL ) {

				items.Construct(slotIndex, cx, clasp, proto);
				//::new (&slot) ClassProtoCache(cx, clasp, proto);
				return true;
			}

			if ( slot.clasp == clasp ) // already cached
				return false;

			slotIndex = (slotIndex + 1) % CACHE_LENGTH;

			if ( slotIndex == first ) // no more free slot
				return false;
		}
	}

	const ClassProtoCache* Get( const char * const className ) const {

		size_t slotIndex = JL_ClassNameToClassProtoCacheSlot(className);
		const size_t first = slotIndex;

		ASSERT( slotIndex < CACHE_LENGTH );

		for (;;) {

			const ClassProtoCache &slot = items.GetConst(slotIndex);
		
			// slot->clasp == NULL -> empty
			// slot->clasp == jlpv::RemovedSlot() -> slot removed, but maybe next slot will match !

			if ( slot.clasp == NULL ) // not found
				return NULL;

			if ( slot.clasp != (JSClass*)jlpv::RemovedSlot() && ( slot.clasp->name == className || !strcmp(slot.clasp->name, className) ) ) // see "Enable String Pooling"
				return &slot;

			slotIndex = (slotIndex + 1) % CACHE_LENGTH;

			if ( slotIndex == first ) // not found
				return NULL;
		}
	}

	
	void Remove( const char *const className ) {

		ASSERT( jlpv::RemovedSlot() != NULL );

		size_t slotIndex = JL_ClassNameToClassProtoCacheSlot(className);
		size_t first = slotIndex;
		
		ASSERT( slotIndex < CACHE_LENGTH );

		for (;;) {

			const ClassProtoCache &slot = items[slotIndex];

			if ( slot.clasp == NULL || ( slot.clasp != (JSClass*)jlpv::RemovedSlot() && ( slot.clasp->name == className || strcmp(slot.clasp->name, className) == 0 ) ) ) {
			
				items.Destruct(slotIndex);
				slot.clasp = (JSClass*)jlpv::RemovedSlot();
				//slot.~ClassProtoCache();
				return;
			}

			slotIndex = (slotIndex + 1) % COUNTOF(hpv->classProtoCache);

			if ( slotIndex == first ) // not found
				return;
		}
	}
};


class HostPrivate {
public:
	~HostPrivate() {
		
		ids.DestructAll();
	}

	HostPrivate(JSContext *cx) : hostObject(cx), objectProto(cx) {
		
		ids.ConstructAll(cx);
	}

	uint32_t versionKey; // used to ensure compatibility between host and modules. see JL_HOSTPRIVATE_KEY macro.
	bool unsafeMode; // used to spread the unsafe status across modules.
	JS::PersistentRootedObject hostObject;
	bool isEnding;
	bool canSkipCleanup; // allows modules to skip the memory cleanup phase.
	void *privateData;
	uint32_t maybeGCInterval;
	JLSemaphoreHandler watchDogSemEnd;
	JLThreadHandler watchDogThread;
	int (*hostStdIn)( void *privateData, char *buffer, size_t bufferSize );
	int (*hostStdOut)( void *privateData, const char *buffer, size_t length );
	int (*hostStdErr)( void *privateData, const char *buffer, size_t length );
	bool (*report)( JSContext *cx, bool isWarning, ... );
	struct ModulePrivate {
		uint32_t moduleId;
		void *privateData;
	} modulePrivate[1<<8]; // does not support more than 256 modules.
	jl::Queue moduleList;
	const JSClass *objectClass;
	JS::PersistentRootedObject objectProto;
	jl_allocators_t alloc;
	ProtoCache<1 << JL_HOSTPRIVATE_MAX_CLASS_PROTO_CACHE_BIT> classProtoCache;
	//JS::PersistentRootedId ids[LAST_JSID];
	StaticArray<JS::PersistentRootedId, LAST_JSID> ids;
	
// experimental:
	//	jl::StaticAlloc<char[16], 128> p16;
#ifdef DEBUG
	uint32_t tmp_count;
#endif
};

S_ASSERT( offsetof(HostPrivate, versionKey) == 0 ); // versionKey must be reachable before any other member.
S_ASSERT( offsetof(HostPrivate, unsafeMode) == 4 ); // then unsafeMode.


ALWAYS_INLINE HostPrivate *
ConstructHostPrivate(JSContext *cx) {

	return ::new(jl_calloc(1, sizeof(HostPrivate))) HostPrivate(cx); // beware: don't realloc later because WatchDogThreadProc points on it.
}

ALWAYS_INLINE void
DestructHostPrivate(HostPrivate *hpv) {

	hpv->HostPrivate::~HostPrivate();
	jl_free(hpv);
}
