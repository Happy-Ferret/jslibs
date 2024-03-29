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

JL_BEGIN_NAMESPACE

typedef struct Pool {
	
	size_t length;
	void **list;
	size_t listLength;
	size_t maxLength;
} Pool;


inline void PoolInitialize( Pool *pool, size_t maxLength ) {
	
	pool->listLength = 4096 / sizeof(void*); // alloc 1 page
	pool->list = (void**)jl_malloc(pool->listLength * sizeof(void*));
	pool->maxLength = maxLength;
	pool->length = 0;
}

inline void PoolFinalize( Pool *pool ) {
	
	jl_free(pool->list);
}

inline bool PoolIsEmpty( Pool *pool ) {

	return pool->length == 0;
}

//inline bool PoolIsFull( Pool *pool ) {
//
//	return pool->listLength >= pool->maxLength && pool->length >= pool->listLength;
//}

inline bool PoolPush( Pool *pool, void *item ) {

	if ( pool->length >= pool->listLength ) {

		if ( pool->listLength >= pool->maxLength )
			return false;
		pool->listLength *= 2;
		pool->list = (void**)jl_realloc(pool->list, pool->listLength * sizeof(void*));
	}
	pool->list[pool->length] = item;
	++pool->length;
	return true;
}

inline void* PoolPop( Pool *pool ) {
	
	if ( !pool->length ) // empty
		return NULL;
	return pool->list[--pool->length];
}

JL_END_NAMESPACE
