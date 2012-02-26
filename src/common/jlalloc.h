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

typedef void* (*jl_malloc_t)( size_t );
typedef void* (*jl_calloc_t)( size_t, size_t );
typedef void* (*jl_memalign_t)( size_t, size_t );
typedef void* (*jl_realloc_t)( void*, size_t );
typedef size_t (*jl_msize_t)( void* );
typedef void (*jl_free_t)( void* );

typedef struct {
	jl_malloc_t malloc;
	jl_calloc_t calloc;
	jl_memalign_t memalign;
	jl_realloc_t realloc;
	jl_msize_t msize;
	jl_free_t free;
} jl_allocators_t;

extern jl_malloc_t jl_malloc;
extern jl_calloc_t jl_calloc;
extern jl_memalign_t jl_memalign;
extern jl_realloc_t jl_realloc;
extern jl_msize_t jl_msize;
extern jl_free_t jl_free;

ALWAYS_INLINE char *
JL_strdup(const char * src) {

	size_t size;
	char *dst;
	size = strlen(src) + 1;
	dst = (char*)jl_malloc(size);
	if ( dst == NULL )
		return NULL;
	memcpy(dst, src, size);
	return dst;
}


///////////////////////////////////////////////////////////////////////////////
// alloc wrappers

/*
template <class T>
ALWAYS_INLINE bool
JL_Alloc( T*&ptr, size_t count = 1 ) {

	ptr = (T*)jl_malloc(sizeof(T)*count);
	return ptr != NULL;
}

template <class T>
ALWAYS_INLINE bool
JL_Realloc( T*&ptr, size_t count = 1 ) {

	ptr = (T*)jl_realloc(ptr, sizeof(T)*count);
	return ptr != NULL;
}
*/


///////////////////////////////////////////////////////////////////////////////
// malloca

// note: MSVC _ALLOCA_S_THRESHOLD is 1024
#define JL_MALLOCA_THRESHOLD 2048

ALWAYS_INLINE void * FASTCALL
jl__malloca_internal(void *mem, size_t heapMem) {
	
	if (likely( mem != NULL )) {

		*(size_t*)mem = heapMem;
		return (size_t*)mem+1;
	} else {

		return NULL;
	}
}

#define jl_malloca(size) \
	( ( (size)+sizeof(size_t) > JL_MALLOCA_THRESHOLD ) ? jl__malloca_internal(jl_malloc((size)+sizeof(size_t)), 1) : jl__malloca_internal(alloca((size)+sizeof(size_t)), 0) )

ALWAYS_INLINE void
jl_freea(void *mem) {
	
	if ( mem && *((size_t*)mem-1) )
		jl_free((size_t*)mem-1);
}

/*
#define JL_AutoFreea(ptr, size)

class JL_AutoMallocaClass {
	void *_ptr;
	size_t _size;
public:
	JL_AutoMallocaClass(void *ptr, size_t size) : _ptr(ptr), _size(size) {}
	~JL_AutoMallocaClass() { jl_freea(_ptr, _size); }
	void *Ptr() const { return _ptr; }
};

#define JL_AutoMalloca(size) JL_AutoMallocaClass(jl_malloca(size), size);
*/


///////////////////////////////////////////////////////////////////////////////
// memory management

namespace jl {

	class NOVTABLE CppNoAlloc {
		void* operator new(size_t);
		void* operator new[](size_t);
		void operator delete(void *, size_t);
		void operator delete[](void *, size_t);
	};


	class NOVTABLE CppAllocators {
	public:
		ALWAYS_INLINE void* operator new(size_t size) NOTHROW {
			return jl_malloc(size);
		}
		ALWAYS_INLINE void* operator new[](size_t size) NOTHROW {
			return jl_malloc(size);
		}
		ALWAYS_INLINE void operator delete(void *ptr, size_t size) {
			jl_free(ptr);
			JL_IGNORE(size);
		}
		ALWAYS_INLINE void operator delete[](void *ptr, size_t size) {
			jl_free(ptr);
			JL_IGNORE(size);
		}
	};


	template <class T>
	class NOVTABLE DefaultAlloc {
	public:
		ALWAYS_INLINE void Free(void *ptr) {
			jl_free(ptr);
		}
		ALWAYS_INLINE void* Alloc() {
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

				_prealloc = (uint8_t*)jl_malloc(PREALLOC * size);
				_preallocEnd = _prealloc + PREALLOC * size;
				for ( uint8_t *it = _prealloc; it != _preallocEnd; it += size ) {

					*(void**)it = _last;
					_last = it;
				}
				_count += PREALLOC;
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

}
