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


#include "jlplatform.h"

typedef void* (__cdecl *jl_malloc_t)( size_t );
typedef void* (__cdecl *jl_calloc_t)( size_t, size_t );
typedef void* (__cdecl *jl_memalign_t)( size_t, size_t );
typedef void* (__cdecl *jl_realloc_t)( void*, size_t );
typedef size_t (__cdecl *jl_msize_t)( void* );
typedef void (__cdecl *jl_free_t)( void* );

extern DLLAPI jl_malloc_t jl_malloc;
extern DLLAPI jl_calloc_t jl_calloc;
extern DLLAPI jl_memalign_t jl_memalign;
extern DLLAPI jl_realloc_t jl_realloc;
extern DLLAPI jl_msize_t jl_msize;
extern DLLAPI jl_free_t jl_free;

// provide functions to access jl allocators (external libraries are using these symbols).
EXTERN_C void* jl_malloc_fct( size_t size );
EXTERN_C void* jl_calloc_fct( size_t num, size_t size );
EXTERN_C void* jl_memalign_fct( size_t alignment, size_t size );
EXTERN_C void* jl_realloc_fct( void *ptr, size_t size );
EXTERN_C size_t jl_msize_fct( void *ptr );
EXTERN_C void jl_free_fct( void *ptr );


///////////////////////////////////////////////////////////////////////////////
// alloc wrappers

template<class T>
class JLAutoPtr {
   T *_ptr;
public:
	JLAutoPtr(T *p)
	: _ptr(p) {
	}

	~JLAutoPtr() {

		Free();
	}
	operator const void *() {

		return _ptr;
	}
	operator T *() {
		
		return _ptr;
	}
	operator const T *() const {
		
		return _ptr;
	}

	T *&
	operator->() {

		return _ptr;
	}

	T *&
	operator=(T *p) {

		_ptr = p;
		return _ptr;
	}

	void
	Free() {

		jl_free(_ptr);
	}
};


///////////////////////////////////////////////////////////////////////////////
// Auto buffer

template <class T>
class JLAutoBuffer {
	void *_ptr;
	JLAutoBuffer(const JLAutoBuffer &);
	JLAutoBuffer & operator =(const JLAutoBuffer &);
public:

	JLAutoBuffer(size_t length) {

		_ptr = jl_malloc(length * sizeof(T));
	}

	~JLAutoBuffer() {

		Free();
	}

	operator T *() {
	
		return _ptr;
	}

	operator const T *() const {
		
		return _ptr;
	}

	T *GetOwnership() {
		
		T *tmp = _ptr;
		_ptr = NULL;
		return tmp;
	}

	void Free() {
		
		if ( _ptr )
			jl_free(_ptr);
		IFDEBUG( _ptr = NULL );
	}
};


///////////////////////////////////////////////////////////////////////////////
// malloca

// note: MSVC _ALLOCA_S_THRESHOLD is 1024
#define JL_MALLOCA_THRESHOLD 1024


JL_BEGIN_NAMESPACE

namespace pv {

	ALWAYS_INLINE void * FASTCALL
	MallocaInternal(void *mem, size_t heapMem) {
		
		if (likely( mem != NULL )) {

			*(size_t*)mem = heapMem;
			return (size_t*)mem+1;
		} else {

			return NULL;
		}
	}
}

JL_END_NAMESPACE


#define jl_malloca(size) \
	( ( (size)+sizeof(size_t) > JL_MALLOCA_THRESHOLD ) ? jl::pv::MallocaInternal(jl_malloc((size)+sizeof(size_t)), 1) : jl::pv::MallocaInternal(alloca((size)+sizeof(size_t)), 0) )


ALWAYS_INLINE void
jl_freea(void *mem) {
	
	if ( mem && *((size_t*)mem-1) )
		jl_free((size_t*)mem-1);
}


JL_BEGIN_NAMESPACE


ALWAYS_INLINE char *
strdup(const char * src) {

	size_t size;
	char *dst;
	size = strlen(src) + 1;
	dst = (char*)jl_malloc(size);
	if ( dst == NULL )
		return NULL;
	jl::memcpy(dst, src, size);
	return dst;
}



///////////////////////////////////////////////////////////////////////////////
// memory management

class NOVTABLE CppNoAlloc {
	void* operator new(size_t);
	void* operator new[](size_t);
	void operator delete(void *, size_t);
	void operator delete[](void *, size_t);
};


class NOVTABLE CppAllocators {
public:
	ALWAYS_INLINE void* 
	operator new(size_t size) NOTHROW {

		return jl_malloc(size);
	}

	ALWAYS_INLINE void*
	operator new[](size_t size) NOTHROW {

		return jl_malloc(size);
	}

	ALWAYS_INLINE void
	operator delete(void *ptr, size_t) {

		jl_free(ptr);
	}

	ALWAYS_INLINE void
	operator delete[](void *ptr, size_t) {

		jl_free(ptr);
	}
};


template <class T>
class NOVTABLE DefaultAlloc {
public:
	ALWAYS_INLINE void
	Free(void *ptr) {

		jl_free(ptr);
	}

	ALWAYS_INLINE void*
	Alloc() {

		return jl_malloc(sizeof(T));
	}
};


template <class T, const size_t PREALLOC = 0, const bool SYNC = false>
class NOVTABLE PreservAlloc {

	void *_last;
	uint32_t _count;
	uint8_t *_prealloc;
	uint8_t *_preallocEnd;
	JLMutexHandler _mx;

public:
	ALWAYS_INLINE PreservAlloc() : _last(NULL), _prealloc(NULL), _count(0) {
		
		if ( SYNC )
			_mx = JLMutexCreate();
	}

	ALWAYS_INLINE ~PreservAlloc() {

		while ( _last != NULL ) {

			void *tmp = _last;
			_last = *(void**)_last;
			if ( PREALLOC == 0 || tmp > _preallocEnd || tmp < _prealloc ) // do not free preallocated memory
				jl_free(tmp);
		}
		if ( PREALLOC > 0 )
			jl_free(_prealloc);
		if ( SYNC )
			JLMutexFree(&_mx);
	}

	ALWAYS_INLINE void Cleanup(size_t keepCount) {

		if ( SYNC )
			JLMutexAcquire(_mx);

		while ( _last != NULL && _count > keepCount ) {

			void *tmp = _last;
			_last = *(void**)_last;
			if ( PREALLOC == 0 || tmp > _preallocEnd || tmp < _prealloc ) // do not free preallocated memory
				jl_free(tmp);
			--_count;
		}

		if ( SYNC )
			JLMutexRelease(_mx);
	}


	ALWAYS_INLINE void Free(T *ptr) {

		if ( SYNC )
			JLMutexAcquire(_mx);
		*(void**)ptr = _last;
		_last = ptr;
		_count++;
		if ( SYNC )
			JLMutexRelease(_mx);
	}

	ALWAYS_INLINE T* Alloc() {
		
		size_t size = sizeof(T);
		if ( size < sizeof(void*) )
			size = sizeof(void*);

		if ( SYNC )
			JLMutexAcquire(_mx);

		if ( PREALLOC > 0 && _prealloc == NULL ) {

			_count = PREALLOC / size;
			_prealloc = (uint8_t*)jl_malloc(_count * size);
			_preallocEnd = _prealloc + _count * size;
			
			for ( uint8_t *it = _prealloc; it != _preallocEnd; it += size ) {

				*(void**)it = _last;
				_last = it;
			}
		}

		if ( _last != NULL ) {

			_count--;
			void *tmp = _last;
			_last = *(void**)_last;
			if ( SYNC )
				JLMutexRelease(_mx);
			return (T*)tmp;
		} else {
		
			if ( SYNC )
				JLMutexRelease(_mx);
			return (T*)jl_malloc(size);
		}
	}
};

template <class T>
class NOVTABLE PreservAllocNone : public PreservAlloc<T, 0> {};

template <class T>
class NOVTABLE PreservAllocNone_threadsafe : public PreservAlloc<T, 0, true> {};


template <class T>
class NOVTABLE PreservAllocSmall : public PreservAlloc<T, 256> {};

template <class T>
class NOVTABLE PreservAllocMedium : public PreservAlloc<T, 4096> {};

template <class T>
class NOVTABLE PreservAllocBig : public PreservAlloc<T, 65536> {};




template <class T, const size_t PREALLOC_SIZE = 1024>
class NOVTABLE StaticAlloc {

	void *_last;
	uint8_t *_preallocEnd;
	uint8_t _prealloc[PREALLOC_SIZE];

public:
	ALWAYS_INLINE StaticAlloc() : _last(NULL), _preallocEnd(NULL) {
	}

	ALWAYS_INLINE ~StaticAlloc() {

		while ( _last != NULL ) {

			void *tmp = _last;
			_last = *(void**)_last;
			if ( _preallocEnd == NULL || tmp > _preallocEnd || tmp < _prealloc ) // do not free preallocated memory
				jl_free(tmp);
		}
	}

	ALWAYS_INLINE void Free(T *ptr) {

		*(void**)ptr = _last;
		_last = ptr;
	}

	ALWAYS_INLINE T* Alloc() {

		size_t size = sizeof(T);
		if ( size < sizeof(void*) )
			size = sizeof(void*);

		if ( _preallocEnd == NULL ) {

			_preallocEnd = _prealloc + (sizeof(_prealloc)/size)*size;
			for ( uint8_t *it = _prealloc; it < _preallocEnd; it += size ) {

				*(void**)it = _last;
				_last = it;
			}
		}
		if ( _last != NULL ) {

			void *tmp = _last;
			_last = *(void**)_last;
			return (T*)tmp;
		}

		return (T*)jl_malloc(size);
	}
};




template <class T>
class NOVTABLE StaticAllocSmall : public StaticAlloc<T, 256> {};

template <class T>
class NOVTABLE StaticAllocMedium : public StaticAlloc<T, 4096> {};

template <class T>
class NOVTABLE StaticAllocBig : public StaticAlloc<T, 65536> {};


JL_END_NAMESPACE


/* memory pool, to be fixed: memory grows endless

void *memoryPool[14] = {NULL};
JLMutexHandler poolMutex[14];

ALWAYS_INLINE int PoolSelect( size_t size ) {

	if ( size == 44 ) return 0;
	if ( size == 16 ) return 1;
	if ( size == 48 ) return 2;
	if ( size == 12 ) return 3;

	//if ( size <=   18 ) return 5;
	//if ( size <=   38 ) return 6;
	//if ( size <=   68 ) return 7;
	//if ( size <=  128 ) return 8;
	//if ( size <=  512 ) return 9;
	//if ( size <= 1078 ) return 10;
	//if ( size <= 2096 ) return 11;
	//if ( size <= 4100 ) return 12;
	//if ( size <= 8200 ) return 13;

	return -1;
}


ALWAYS_INLINE void MemoryPoolFree( void *ptr ) {

	size_t size = malloc_usable_size(ptr);
	int pool = PoolSelect(size);
	if ( pool == -1 ) {

		free(ptr);
		return;
	}

	JLAcquireMutex(poolMutex[pool]);
	*(void**)ptr = memoryPool[pool];
	memoryPool[pool] = ptr;
	JLReleaseMutex(poolMutex[pool]);
}

ALWAYS_INLINE void* MemoryPoolMalloc( size_t size ) {

	int pool = PoolSelect(size);
	if ( pool == -1 || memoryPool[pool] == NULL )
		return malloc(size);

	JLAcquireMutex(poolMutex[pool]);
	void *ptr = memoryPool[pool];
	memoryPool[pool] = *(void**)memoryPool[pool];
	JLReleaseMutex(poolMutex[pool]);
	return ptr;
}

void MemoryPoolInit() {

	for ( int i = 0; i < COUNTOF(poolMutex); i++ )
		poolMutex[i] = JLCreateMutex();
}

void MemoryPoolFinalize() {

	for ( int i = 0; i < COUNTOF(poolMutex); i++ ) {

		JLFreeMutex(&poolMutex[i]);
		while ( memoryPool[i] ) {

			void *next = *(void**)memoryPool[i];
			free(memoryPool[i]);
			memoryPool[i] = next;
		}
	}
}
*/



template <class T, const size_t ITEM_COUNT>
class StaticArray {
	uint8_t data[ITEM_COUNT * sizeof(T)];
public:
	enum {
		length = ITEM_COUNT
	};

	StaticArray() {
	}

	StaticArray(bool construct) {

		constructAll();
	}

	template <typename P1>
	StaticArray(bool construct, P1 p1) {

		constructAll(p1);
	}

	template <typename P1, typename P2>
	StaticArray(bool construct, P1 p1, P2 p2) {

		constructAll(p1, p2);
	}

	template <typename P1, typename P2, typename P3>
	StaticArray(bool construct, P1 p1, P2 p2, P3 p3) {

		constructAll(p1, p2, p3);
	}

	T&
	get(size_t slotIndex) {

		ASSERT( slotIndex < length );
		return reinterpret_cast<T*>(data)[slotIndex];
	}

	const T&
	getConst(size_t slotIndex) const {

		ASSERT( slotIndex < length );
		return reinterpret_cast<const T*>(data)[slotIndex];
	}

	T&
	operator[](size_t index) {

		return get(index);
	}


	void
	destruct(size_t item) {

		get(item).T::~T();
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
			
			destruct(i);
		}
	}

	void
	constructAll() {
		
		for ( size_t i = 0; i < length; ++i ) {
			
			construct(i);
		}
	}

	template <typename P1>
	void
	constructAll(P1 p1) {
		
		for ( size_t i = 0; i < length; ++i ) {
			
			construct(i, p1);
		}
	}

	template <typename P1, typename P2>
	void
	constructAll(P1 p1, P2 p2) {
		
		for ( size_t i = 0; i < length; ++i ) {
			
			construct(i, p1, p2);
		}
	}

	template <typename P1, typename P2, typename P3>
	void
	constructAll(P1 p1, P2 p2, P3 p3) {
		
		for ( size_t i = 0; i < length; ++i ) {
			
			construct(i, p1, p2, p3);
		}
	}
};
