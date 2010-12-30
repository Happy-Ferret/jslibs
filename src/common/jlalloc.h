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

#ifndef _JLALLOC_H_
#define _JLALLOC_H_

#include <jlplatform.h>

#include <sys/types.h>
#include <malloc.h>

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

///////////////////////////////////////////////////////////////////////////////
// alloc wrappers

/*
template <typename T>
static ALWAYS_INLINE bool
JL_Alloc( T*&ptr, size_t count = 1 ) {

	ptr = (T*)jl_malloc(sizeof(T)*count);
	return ptr != NULL;
}

template <typename T>
static ALWAYS_INLINE bool
JL_Realloc( T*&ptr, size_t count = 1 ) {

	ptr = (T*)jl_realloc(ptr, sizeof(T)*count);
	return ptr != NULL;
}
*/


///////////////////////////////////////////////////////////////////////////////
// malloca

#define JL_MALLOCA_THRESHOLD 8192

ALWAYS_INLINE void *
jl_malloca_private(void *mem, size_t heapMem) {
	
	if (likely( mem != NULL )) {

		*(size_t*)mem = heapMem;
		return (size_t*)mem+1;
	} else {

		return NULL;
	}
}

#define jl_malloca(size) \
	( ( (size)+sizeof(size_t) > JL_MALLOCA_THRESHOLD ) ? jl_malloca_private(jl_malloc((size)+sizeof(size_t)), 1) : jl_malloca_private(alloca((size)+sizeof(size_t)), 0) )

ALWAYS_INLINE void
jl_freea(void *mem) {
	
	if (unlikely( mem && *((size_t*)mem-1) ))
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

	class _NOVTABLE CppNoAlloc {
		void* operator new(size_t);
		void* operator new[](size_t);
		void operator delete(void *, size_t);
		void operator delete[](void *, size_t);
	};


	class _NOVTABLE CppAllocators {
	public:
		ALWAYS_INLINE void* operator new(size_t size) _NOTHROW {
			return jl_malloc(size);
		}
		ALWAYS_INLINE void* operator new[](size_t size) _NOTHROW {
			return jl_malloc(size);
		}
		ALWAYS_INLINE void operator delete(void *ptr, size_t size) {
			jl_free(ptr);
			JL_UNUSED(size);
		}
		ALWAYS_INLINE void operator delete[](void *ptr, size_t size) {
			jl_free(ptr);
			JL_UNUSED(size);
		}
	};


	template <typename T>
	class _NOVTABLE DefaultAlloc {
	public:
		ALWAYS_INLINE void Free(void *ptr) {
			jl_free(ptr);
		}
		ALWAYS_INLINE void* Alloc() {
			return jl_malloc(sizeof(T));
		}
	};


	template <typename T, const size_t PREALLOC = 0>
	class _NOVTABLE PreservAlloc {

		void *_last;
		uint8_t *_prealloc;
		uint8_t *_preallocEnd;

	public:
		ALWAYS_INLINE PreservAlloc() : _last(NULL), _prealloc(NULL) {
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
		}

		ALWAYS_INLINE void Free(T *ptr) {

			*(void**)ptr = _last;
			_last = ptr;
		}

		ALWAYS_INLINE T* Alloc() {
			
			size_t size = sizeof(T);
			if ( size < sizeof(void*) )
				size = sizeof(void*);

			if ( PREALLOC > 0 && _prealloc == NULL ) {

				_prealloc = (uint8_t*)jl_malloc(PREALLOC * size);
				_preallocEnd = _prealloc + PREALLOC * size;
				for ( uint8_t *it = _prealloc; it != _preallocEnd; it += size ) {

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

		//ALWAYS_INLINE void* Realloc(void *ptr, size_t size) {

		//	if ( size < sizeof(void*) )
		//		size = sizeof(void*);
		//	return DefaultAlloc::Realloc(ptr, size);
		//}
	};


	template <typename T, const size_t PREALLOC_SIZE = 1024>
	class _NOVTABLE StaticAlloc {

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

		//ALWAYS_INLINE void* Realloc(void *ptr, size_t size) {

		//	if ( size < sizeof(void*) )
		//		size = sizeof(void*);

		//	return DefaultAlloc::Realloc(ptr, size);
		//}
	};
}


#endif // _JLALLOC_H_
