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

#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <stdlib.h>

typedef struct QueueCell {
	struct QueueCell *prev, *next;
	void *data;
} QueueCell;

typedef struct Queue {
	QueueCell *begin, *end;
} Queue;

inline Queue *QueueConstruct() {

	Queue *queue = (Queue*)malloc(sizeof(Queue));
	queue->begin = NULL;
	queue->end = NULL;
	return queue;
}

inline void QueueDestruct( Queue *queue ) {

	QueueCell *it = queue->begin;
	while ( it != NULL ) {

		QueueCell *next = it->next;
		free(it);
		it = next;
	}
	free(queue);
}

inline bool QueueIsEmpty( Queue *queue ) {

	return queue->begin == NULL;
}

inline void QueueSetData( QueueCell *cell, void *data ) {

	cell->data = data;
}


inline void QueuePush( Queue *queue, void *data ) {

	QueueCell *cell = (QueueCell*)malloc(sizeof(QueueCell));
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
	free(cell);
	return data;
}

inline void QueueUnshift( Queue *queue, void *data ) {

	QueueCell *cell = (QueueCell*)malloc(sizeof(QueueCell));
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
	free(cell);
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

inline void *QueueRemoveCell( Queue *queue, QueueCell *cell ) {

	if ( cell == queue->begin )
		return QueueShift(queue);
	if ( cell == queue->end )
		return QueuePop(queue);
	cell->prev->next = cell->next;
	cell->next->prev = cell->prev;
	void *data = cell->data;
	free(cell);
	return data;
}

inline void QueueInsertCell( Queue *queue, QueueCell *nextCell, void *data ) {

	if ( nextCell == queue->begin )
		return QueueUnshift(queue, data);
	QueueCell *newCell = (QueueCell*)malloc(sizeof(QueueCell));
	newCell->data = data;
	newCell->next = nextCell;
	newCell->prev = nextCell->prev;
	nextCell->prev->next = newCell;
	nextCell->prev = newCell;
}


inline void QueueInsertCellAfter( Queue *queue, QueueCell *prevCell, void *data ) {

	if ( prevCell == queue->end )
		return QueuePush(queue, data);
	QueueCell *newCell = (QueueCell*)malloc(sizeof(QueueCell));
	newCell->data = data;
	newCell->prev = prevCell;
	newCell->next = prevCell->next;
	prevCell->next->prev = newCell;
	prevCell->next = newCell;
}


#endif // _QUEUE_H_
