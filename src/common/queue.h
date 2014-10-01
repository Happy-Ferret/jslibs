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
	typedef T Type;

	class Item {
	public:
		Item *prev;
		Item *next;
		T data;
		Item(const T &d) : data(d) {}
	};

private:
	Item *_first;
	Item *_last;
	A<Item> allocator;

public:
	typedef Queue1<T,A> ThisType;
	typedef T ValueType;
	enum { itemSize = sizeof(Item) };

	Queue1()
	: _first(NULL), _last(NULL) {
	}

	~Queue1() {

		while ( _first ) {

			Item *tmp = _first;
			_first = _first->next;
			tmp->Item::~Item();
			allocator.Free(tmp);
		}
	}

	ALWAYS_INLINE operator bool() const {
		
		return _first != NULL;
	}

	ALWAYS_INLINE Item* First() const {

		return _first;
	}

	ALWAYS_INLINE Item* Last() const {

		return _last;
	}

	ALWAYS_INLINE Item* AddEnd( const T &data = T() ) {

		Item *newItem = ::new(allocator.Alloc()) Item(data);
		ASSERT(newItem);
		if ( _last != NULL ) {

			newItem->prev = _last;
			newItem->next = NULL;
			_last->next = newItem;
			_last = newItem;
		} else {

			newItem->prev = NULL;
			newItem->next = NULL;
			_first = newItem;
			_last = newItem;
		}
		return newItem;
	}

	ALWAYS_INLINE void RemoveEnd() {

		Item *oldItem = _last;
		if ( _first != _last ) { // do we have only one cell ?

			_last = oldItem->prev;
			_last->next = NULL;
		} else {

			_first = NULL;
			_last = NULL;
		}
		oldItem->Item::~Item();
		allocator.Free(oldItem);
	}

	ALWAYS_INLINE Item* AddBegin( const T &data = T() ) {

		Item *newItem = ::new(allocator.Alloc()) Item;
		if ( _first != NULL ) {

			newItem->prev = NULL;
			newItem->next = _first;
			_first->prev = newItem;
			_first = newItem;
		} else {

			newItem->next = NULL;
			newItem->prev = NULL;
			_first = newItem;
			_last = newItem;
		}
		return newItem;
	}

	ALWAYS_INLINE void RemoveBegin() {

		Item *oldItem = _first;
		if ( _first != _last ) { // do we have only one cell ?

			_first = oldItem->next;
			_first->prev = NULL;
		} else {

			_first = NULL;
			_last = NULL;
		}
		oldItem->Item::~Item();
		allocator.Free(oldItem);
	}

	ALWAYS_INLINE void RemoveItem( Item *item ) {

		if ( item == _first )
			return RemoveBegin();
		if ( item == _last )
			return RemoveEnd();
		item->prev->next = item->next;
		item->next->prev = item->prev;
		item->Item::~Item();
		allocator.Free(item);
	}

	ALWAYS_INLINE Item* Insert( Item *nextItem, const T &data = T() ) {

		if ( nextItem == First() ) {

			AddBegin();
			First()->data = data;
		}
		
		Item *newItem = ::new(allocator.Alloc()) Item;
		newItem->data = data;
		newItem->next = nextItem;
		newItem->prev = nextItem->prev;
		nextItem->prev->next = newItem;
		nextItem->prev = newItem;
		return newItem;
	}


	ALWAYS_INLINE Item* Find( const T &data ) const {

		for ( Item *it = _first; it; it = it->next )
			if ( it->data == data )
				return it;
		return NULL;
	}


	ALWAYS_INLINE void Push( const T &item = T() ) {

		AddEnd();
		Last()->data = item;
	}

	ALWAYS_INLINE void Pop( T &item ) {

		item = Last()->data;
		RemoveEnd();
	}

	ALWAYS_INLINE T Pop() {

		T item = Last()->data;
		RemoveEnd();
		return item;
	}

	ALWAYS_INLINE void Unshift( const T &item = T() ) {
		
		AddBegin();
		First()->data = item;
	}

	ALWAYS_INLINE void Shift( T &item ) {

		item = First()->data;
		RemoveBegin();
	}

};


JL_END_NAMESPACE
