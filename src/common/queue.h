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

#include "jlalloc.h"

JL_BEGIN_NAMESPACE

typedef struct QueueCell {
	struct QueueCell *prev, *next;
	void *data;
} QueueCell;

typedef struct Queue {
	QueueCell *begin, *end;
} Queue;

inline void QueueInitialize( Queue *queue ) {

	queue->begin = NULL;
	queue->end = NULL;
}

inline void QueueEmpty( Queue *queue ) {

	QueueCell *it = queue->begin;
	while ( it != NULL ) {

		QueueCell *next = it->next;
		jl_free(it);
		it = next;
	}
}


inline Queue *QueueConstruct() {

	Queue *queue = (Queue*)jl_malloc(sizeof(Queue));
	QueueInitialize(queue);
	return queue;
}

inline void QueueDestruct( Queue *queue ) {

	QueueEmpty(queue);
	jl_free(queue);
}

inline bool QueueIsEmpty( Queue *queue ) {

	return queue->begin == NULL;
}

inline void QueueSetData( QueueCell *cell, void *data ) {

	cell->data = data;
}


inline void QueuePush( Queue *queue, void *data ) {

	QueueCell *cell = (QueueCell*)jl_malloc(sizeof(QueueCell));
	cell->data = data;
	if ( QueueIsEmpty(queue) ) {

		cell->prev = NULL;
		cell->next = NULL;
		queue->begin = cell;
		queue->end = cell;
	} else {

		cell->prev = queue->end;
		cell->next = NULL;
		queue->end->next = cell;
		queue->end = cell;
	}
}

inline void *QueuePop( Queue *queue ) {

	QueueCell *cell = queue->end;
	if ( queue->begin == queue->end ) { // do we have only one cell ?

		queue->begin = NULL;
		queue->end = NULL;
	} else {

		queue->end = cell->prev;
		queue->end->next = NULL;
	}
	void *data = cell->data;
	jl_free(cell);
	return data;
}

inline void QueueUnshift( Queue *queue, void *data ) {

	QueueCell *cell = (QueueCell*)jl_malloc(sizeof(QueueCell));
	cell->data = data;
	if ( QueueIsEmpty(queue) ) {

		cell->prev = NULL;
		cell->next = NULL;
		queue->begin = cell;
		queue->end = cell;
	} else {

		cell->next = queue->begin;
		cell->prev = NULL;
		queue->begin->prev = cell;
		queue->begin = cell;
	}
}

inline void *QueueShift( Queue *queue ) {

	QueueCell *cell = queue->begin;
	if ( queue->begin == queue->end ) { // do we have only one cell ?

		queue->begin = NULL;
		queue->end = NULL;
	} else {

		queue->begin = cell->next;
		queue->begin->prev = NULL;
	}
	void *data = cell->data;
	jl_free(cell);
	return data;
}


inline QueueCell *QueueBegin( Queue *queue ) {

	return queue->begin;
}


inline QueueCell *QueueEnd( Queue *queue ) {

	return queue->end;
}

inline QueueCell *QueueNext( QueueCell *cell ) {

	return cell->next;
}

inline void *QueueGetData( QueueCell *cell ) {

	return cell->data;
}

inline void **QueueGetDataPtr( QueueCell *cell ) {

	return &cell->data;
}

inline void *QueueRemoveCell( Queue *queue, QueueCell *cell ) {

	if ( cell == queue->begin )
		return QueueShift(queue);
	if ( cell == queue->end )
		return QueuePop(queue);
	cell->prev->next = cell->next;
	cell->next->prev = cell->prev;
	void *data = cell->data;
	jl_free(cell);
	return data;
}

inline void QueueInsertCell( Queue *queue, QueueCell *nextCell, void *data ) {

	if ( nextCell == queue->begin )
		return QueueUnshift(queue, data);
	QueueCell *newCell = (QueueCell*)jl_malloc(sizeof(QueueCell));
	newCell->data = data;
	newCell->next = nextCell;
	newCell->prev = nextCell->prev;
	nextCell->prev->next = newCell;
	nextCell->prev = newCell;
}


inline void QueueInsertCellAfter( Queue *queue, QueueCell *prevCell, void *data ) {

	if ( prevCell == queue->end )
		return QueuePush(queue, data);
	QueueCell *newCell = (QueueCell*)jl_malloc(sizeof(QueueCell));
	newCell->data = data;
	newCell->prev = prevCell;
	newCell->next = prevCell->next;
	prevCell->next->prev = newCell;
	prevCell->next = newCell;
}

#ifdef DEBUG
inline bool QueueIntegrityCheck( Queue *queue ) {

	if ( queue->begin == NULL && queue->end != NULL )
		return false;
	if ( queue->end != NULL && queue->begin == NULL )
		return false;
	if ( queue->begin && queue->begin->prev )
		return false;
	if ( queue->end && queue->end->next )
		return false;
	QueueCell *it;
	it = queue->begin;
	int c1 = 0, c2 = 0;
	while ( it ) {
		it = it->next;
		c1++;
	}
	it = queue->end;
	while ( it ) {
		it = it->prev;
		c2++;
	}
	if ( c1 != c2 )
		return false;
	return true;
}
#endif

/*
inline QueueCell *SearchFirstData( Queue *queue, void *data ) {

	for ( QueueCell *it = QueueBegin(queue); it; it = QueueNext(it) )
		if ( QueueGetData(it) == data )
			return it;
	return NULL;
}
*/


template <class T, template<class> class A = DefaultAlloc>
class NOVTABLE Queue1 {

public:
	struct Item {

		Item *prev;
		Item *next;
		T data;
	};

private:
	Item *_begin;
	Item *_end;

	A<Item> allocator;

public:

	typedef Queue1<T,A> ThisType;
	typedef T ValueType;
	enum { itemSize = sizeof(Item) };


	Queue1() : _begin(NULL), _end(NULL) {
	}

	~Queue1() {

		while ( _begin ) {
		
			Item *tmp = _begin;
			_begin = _begin->next;
			tmp->Item::~Item();
			allocator.Free(tmp);
		}
	}

	ALWAYS_INLINE operator bool() const {
		
		return _begin != NULL;
	}

	ALWAYS_INLINE Item* Begin() const {

		return _begin;
	}

	ALWAYS_INLINE Item* End() const {

		return _end;
	}

	ALWAYS_INLINE void AddEnd() {

		Item *newItem = ::new(allocator.Alloc()) Item;
		if ( _end != NULL ) {

			newItem->prev = _end;
			newItem->next = NULL;
			_end->next = newItem;
			_end = newItem;
		} else {

			newItem->prev = NULL;
			newItem->next = NULL;
			_begin = newItem;
			_end = newItem;
		}
	}

	ALWAYS_INLINE void RemoveEnd() {

		Item *oldItem = _end;
		if ( _begin != _end ) { // do we have only one cell ?

			_end = oldItem->prev;
			_end->next = NULL;
		} else {

			_begin = NULL;
			_end = NULL;
		}
		oldItem->Item::~Item();
		allocator.Free(oldItem);
	}

	ALWAYS_INLINE void AddBegin() {

		Item *newItem = ::new(allocator.Alloc()) Item;
		if ( _begin != NULL ) {

			newItem->prev = NULL;
			newItem->next = _begin;
			_begin->prev = newItem;
			_begin = newItem;
		} else {

			newItem->next = NULL;
			newItem->prev = NULL;
			_begin = newItem;
			_end = newItem;
		}
	}

	ALWAYS_INLINE void RemoveBegin() {

		Item *oldItem = _begin;
		if ( _begin != _end ) { // do we have only one cell ?

			_begin = oldItem->next;
			_begin->prev = NULL;
		} else {

			_begin = NULL;
			_end = NULL;
		}
		oldItem->Item::~Item();
		allocator.Free(oldItem);
	}

	ALWAYS_INLINE void Remove( Item *item ) {

		if ( item == _begin )
			return RemoveBegin();
		if ( item == _end )
			return RemoveEnd();
		item->prev->next = item->next;
		item->next->prev = item->prev;
		item->Item::~Item();
		allocator.Free(item);
	}

	ALWAYS_INLINE void Insert( Item *nextItem, const T &data ) {

		if ( nextItem == Begin() ) {

			AddBegin();
			Begin()->data = data;
		}
		
		Item *newItem = ::new(allocator.Alloc()) Item;
		newItem->data = data;
		newItem->next = nextItem;
		newItem->prev = nextItem->prev;
		nextItem->prev->next = newItem;
		nextItem->prev = newItem;
	}


	ALWAYS_INLINE Item* Find( const T &data ) const {

		for ( Item *it = _begin; it; it = it->next )
			if ( it->data == data )
				return it;
		return NULL;
	}


	ALWAYS_INLINE void Push( const T &item ) {

		AddEnd();
		End()->data = item;
	}

	ALWAYS_INLINE void Pop( T &item ) {

		item = End()->data;
		RemoveEnd();
	}

	ALWAYS_INLINE void Pop() {

		RemoveEnd();
	}

	ALWAYS_INLINE void Unshift( const T &item ) {
		
		AddBegin();
		Begin()->data = item;
	}

	ALWAYS_INLINE void Shift( T &item ) {

		item = Begin()->data;
		RemoveBegin();
	}

};


JL_END_NAMESPACE
