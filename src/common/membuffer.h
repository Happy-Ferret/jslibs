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

#ifndef _MEMBUFFER_H_
#define _MEMBUFFER_H_

#include "stdafx.h"
#include "../jslang/idpub.h"


struct MemBuffer;

typedef bool (*MemBufferFree_t)(MemBuffer *mb); // it is up to the callee to update mem and size and pv.
typedef bool (*MemBufferResize_t)(MemBuffer *mb, unsigned int newSize, bool preserveExistingData); // it is up to the callee to update mem and size and pv.

typedef bool (*MemBufferAcquire_t)(MemBuffer *mb, int mode);
typedef bool (*MemBufferRelease_t)(MemBuffer *mb);

struct MemBuffer {
//private:
	void *mem;
public:
	unsigned int size; // in bytes
	MemBufferFree_t MemBufferFree;
	MemBufferResize_t MemBufferResize;
	void *pv; // private data for the user of this structure
};

void MemBufferFinalizeCallback( void* data ) {
	
	MemBuffer *membuf = (MemBuffer*)data;
	if ( membuf->MemBufferFree ) // if the mem have to be freed.
		membuf->MemBufferFree(membuf);
}

JSBool MemoryBufferObjectCreate( JSContext *cx, jsval *memBufferVal, void *pv, void* mem, unsigned int size, MemBufferFree_t free, MemBufferResize_t resize ) {

	MemBuffer *membuf;
	JL_CHK( CreateId(cx, 'MEMB', sizeof(MemBuffer), (void**)&membuf, MemBufferFinalizeCallback, memBufferVal) );
	membuf->mem = mem;
	membuf->size = size;
	membuf->MemBufferFree = free;
	membuf->MemBufferResize = resize;
	membuf->pv = pv;
	return JS_TRUE;
	JL_BAD;
}

JSBool MemoryBufferObjectGet( JSContext *cx, jsval memBufferVal, MemBuffer **membuffer ) {

	JL_S_ASSERT( IsIdType(cx, memBufferVal, 'MEMB'), "Invalid memory object." );
	*membuffer = (MemBuffer*)GetIdPrivate(cx, memBufferVal);
	return JS_TRUE;
	JL_BAD;
}



/*  everything can be done with the previous function (MemoryBufferObjectGet).
JSBool MemoryBufferObjectResize( JSContext *cx, jsval memBufferVal, unsigned int newSize ) {

	JL_S_ASSERT( IsIdType(cx, memBufferVal, 'MEMB'), "Invalid memory object." );
	MemBuffer *membuf = (MemBuffer*)GetIdPrivate(cx, memBufferVal);
	bool st = membuf->MemBufferRealloc(membuf, newSize);
	JL_S_ASSERT( st, "Unable to reallocate the buffer." );
	return JS_TRUE;
	JL_BAD;
}

JSBool MemoryBufferObjectFree( JSContext *cx, jsval memBufferVal ) {

	JL_S_ASSERT( IsIdType(cx, memBufferVal, 'MEMB'), "Invalid memory object." );
	MemBuffer *membuf = (MemBuffer*)GetIdPrivate(cx, memBufferVal);
	bool st = membuf->MemBufferFree(membuf); 
	JL_S_ASSERT( st, "Unable to free the buffer." );
	return JS_TRUE;
	JL_BAD;
}
*/

#endif // _MEMBUFFER_H_