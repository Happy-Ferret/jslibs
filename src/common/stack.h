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

#include <jlalloc.h>

// very simple stack functions

/* internal
     NULL
     ^
    [0][a]
    ^
   [0][b]
   ^
  [0][c]
  ^
 [0][d]
 ^
[0][e] <- stack
*/

/* test
	void *stack;
	StackInit(&stack);
	StackPush(&stack,(void*)5);
	StackPush(&stack,(void*)6);

	void *p;
	p = StackFind( &stack, (void*)7 );
	p = StackFind( &stack, (void*)6 );
	p = StackFind( &stack, (void*)5 );
	p = StackFind( &stack, (void*)4 );
*/

namespace jl {

inline void StackInit( void **stack ) {

	*stack = NULL;
}


inline void* StackData( const void * const * stack ) {

  return ((void**)*stack)[1];
}

inline void StackSetData( const void * const * stack, void *data ) {

  ((void**)*stack)[1] = data;
}


inline void* StackPrev( const void * const * stack ) {

  return ((void**)*stack)[0];
}


inline void StackGoPrev( void ** stack ) {

	*stack = ((void**)*stack)[0];
}


inline bool StackIsEnd( const void * const * stack ) {

	return *stack == NULL;
}


inline int StackLength( const void * const * stack ) {

	register int length;
	for ( length = 0; *stack; stack = (const void * const *)*stack, length++ ) ;
	return length;
}


inline void* StackFind( void * const * stack, const void *data ) {

	void *it;
	for ( it = *stack; it && StackData(&it) != data; StackGoPrev(&it) ) ;
	return it;
}


inline bool StackHas( void * const * stack, const void *data ) {

	return StackFind(stack, data) != NULL;
}


inline void StackPush( void **stack, void *data ) {

  void **newItem = (void**)jl_malloc( sizeof( void* ) * 2 ); // create a new item for the list ( pointer, pointer )
  newItem[0] = *stack; // chain the list
  newItem[1] = data; // store the address of the new allocated memory
  *stack = newItem; // store the new start of the list in *list
}


inline void* StackPop( void **stack ) {

  void *data, **item = (void**)*stack;
  *stack = item[0]; // keep the chain
  data = item[1];
  jl_free( item );
  return data;
}


inline bool StackReplaceData( void * const *stack, const void *data, void *newData ) {

	void *it;
	if ( (it = StackFind( stack, data )) )
		StackSetData( stack, newData );
	return it != NULL;
}


// usage: for ( void *it = NULL; StackIterate( &stack, &it ); )
// alternate method: for ( void *it = stack; !StackIsEnd(&it); it = StackPrev(&it) )
inline bool StackIterate( void * const * stack, void **iterator ) {

	return (*iterator = (*iterator == NULL) ? *stack : StackPrev(iterator)) != NULL;
}


inline void StackReverse( void **stack ) {

	for ( void *tmp, *ptr = *stack; *(void**)ptr != NULL; ) {

		tmp = *stack;
		*stack = *(void**)ptr;
		*(void**)ptr = *(void**)*stack;
		*(void**)*stack = tmp;
	}
}


inline bool StackRemove( void **stack, void *data ) {

	for ( ; *stack; stack = (void**)*stack )
		if ( StackData(stack) == data ) {

			void *tmp = *stack;
			*stack = StackPrev(stack);
			jl_free(tmp);
			return true;
		}
	return false;
}


inline void StackFreeContent( void **stack ) {

	while ( *stack )
      jl_free( StackPop( stack ) );
}


inline void StackFree( void **stack ) {

	while ( *stack )
      StackPop( stack );
}



template <class T, template<class> class A = DefaultAlloc>
class NOVTABLE Stack {

	class Item {
	public:
		T data;
		Item *prev;
	};

	A<Item> allocator;

	Item* _top;

private: // forbidden access
	ALWAYS_INLINE Stack& operator++( int );
	ALWAYS_INLINE Stack& operator--( int );
	Stack( const Stack & );
	Stack & operator=( const Stack & );

public:
	typedef Stack<T> ThisType;
	typedef T ValueType;
	enum { itemSize = sizeof(Item) };

	Stack() : _top(NULL) {
	}

	~Stack() {

		Clear();
	}

	ALWAYS_INLINE operator bool() const {

		return _top != NULL;
	}

	ALWAYS_INLINE operator T*() const {

		ASSERT( _top != NULL );
		return &_top->data;
	}

	ALWAYS_INLINE T& operator*() const {

		ASSERT( _top != NULL );
		return _top->data;
	}

	ALWAYS_INLINE T* operator->() const {

		ASSERT( _top != NULL );
		return &_top->data;
	}

	ALWAYS_INLINE Stack& operator++() { // ++s

		Item *newItem = ::new(allocator.Alloc()) Item;
		newItem->prev = _top;
		_top = newItem;
		return *this;
	}

	ALWAYS_INLINE Stack& operator--() { // --s

		ASSERT( _top != NULL );
		Item* oldItem = _top;
		_top = _top->prev;
		oldItem->~Item();
		allocator.Free(oldItem);
		return *this;
	}

	template <class U>
	ALWAYS_INLINE Stack& operator+=( const U &src ) {

		class Copy {

			ThisType &_dst;
			Item **_itemPtr;
			Item *_prevTop;

		public:
			ALWAYS_INLINE Copy( ThisType &dst ) : _dst(dst), _prevTop(dst._top), _itemPtr(&dst._top) {
			}

			ALWAYS_INLINE ~Copy() {

				*_itemPtr = _prevTop;
			}

			ALWAYS_INLINE bool operator()( const T &value ) {

				Item *newItem = ::new(allocator.Alloc(itemSize)) Item;
				newItem->data = value;
				*_itemPtr = newItem;
				_itemPtr = &newItem->prev;
				return false;
			}
		};

		src.BackForEach( Copy(*this) );
		return *this;
	}

	ALWAYS_INLINE T* operator[]( int i ) const {

		ASSERT( _top != NULL );
		for ( Item* item = _top; i; item = item->prev, --i );
		return &_top->data;
	}

	ALWAYS_INLINE size_t Length() const {

		ASSERT( _top != NULL );
		size_t length = 0;
		for ( Item* item = _top; item; item = item->prev )
			++length;
		return length;
	}

	ALWAYS_INLINE bool Has( const T &ref ) const {

		for ( Item* item = _top; item; item = item->prev )
			if ( item->data == ref )
				return true;
		return false;
	}

	ALWAYS_INLINE void Clear() {

		while ( this->operator bool() )
			this->operator--();
	}

	ALWAYS_INLINE bool Remove( const T &ref ) {

		if ( _top->data == ref ) {

			Item* tmp = _top;
			_top = _top->prev;
			tmp->~Item();
			allocator.Free(tmp);
			return true;
		}
		for ( Item *prev, *item = _top; prev = item->prev; item = prev ) {

			if ( prev->data == ref ) {

				item->prev = prev->prev;
				prev->~Item();
				allocator.Free(prev);
				return true;
			}
		}
		return false;
	}

	ALWAYS_INLINE void Revert() {

		Item *tmp, *next = NULL;

		while ( _top ) {

			tmp = _top->prev;
			_top->prev = next;
			next = _top;
			_top = tmp;
		}
		_top = next;
	}

	/* ForEach() example:

  		bool iter1( int &value ) {

			printf("%d\n", value);
			return false; // do not cancel iteration
		}

		struct {
			bool operator()( int &value ) {

				printf("%d\n", value);
				return false;
			}
		} iter2;

		jl::Stack<int> s;
		s.ForEach( iter1 );
		s.ForEach( iter2 );
	*/
	template <class P>
	ALWAYS_INLINE bool BackForEach( P &pre ) const {

		for ( Item* item = _top; item; item = item->prev )
			if ( pre( item->data ) )
				return true;
		return false;
	}

};



}

