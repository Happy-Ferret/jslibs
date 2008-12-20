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


#ifndef _BUFFER_H_
#define _BUFFER_H_


#define BUFFER_INIT_CHUNK_SIZE 8192
#define BUFFER_INIT_CHUNK_LIST_SIZE 16
#define BUFFER_TYPE_AUTO_THRESHOLD 64
#define BUFFER_PAGE_SIZE 4096


// (TBD) use this Buffer in: jsz, jsio::Descriptor, jsiconv, jslang::Stringify, DecodeOggVorbis, DecodeSound
//       AND perhaps in: jsstd::Buffer::ReadRawAmount, jsstd::Expand

namespace jl {


/* (TBD) replace BufferType and BufferGrowType by some flags:

final size:

	tinny (<4 KB)
	small (~64 KB)
	big (~1 MB)
	huge (>10 MB)

chunk size:
	fixed
	variable
*/

enum BufferType {
	bufferTypeAuto, // try to guess what is the best between chunk or realloc
	bufferTypeChunk, // allocates a new memory chunk each time the buffer needs to be extend. Flatten the buffer may be required at the end.
	bufferTypeRealloc // only use realloc to increase the buffer size. No buffer flatten is needed at the end.
};


enum BufferGrowType {
	bufferGrowTypeAuto, // guess the best grow
	bufferGrowTypeNoGuess, // minimalist grow, only the required size.
	bufferGrowTypeDouble, // double the current buffer length.
	bufferGrowTypePage // try to alloc full system pages
};


struct BufferChunk {
	char *begin;
	size_t pos;
	size_t size;
};

struct Buffer {
	BufferType type;
	BufferGrowType growType;
	size_t length;
	BufferChunk *chunkList;
	size_t chunkPos;
	size_t chunkListSize;
// static memory
	BufferChunk staticChunkList[BUFFER_INIT_CHUNK_LIST_SIZE];
	char staticBuffer[BUFFER_INIT_CHUNK_SIZE];
// buffer stats

};


inline size_t NextChunkSize( const Buffer *buffer, size_t lastChunk, size_t requiredLength ) {

	switch ( buffer->growType ) {
		case bufferGrowTypeAuto:
			return buffer->chunkList[lastChunk].size * 3/2 + requiredLength * 3/2;
		case bufferGrowTypeNoGuess:
			return requiredLength;
		case bufferGrowTypeDouble:
			return buffer->length;
		case bufferGrowTypePage:
			if ( buffer->type == bufferTypeChunk )
				return BUFFER_PAGE_SIZE;
			if ( buffer->type == bufferTypeRealloc ) {
				size_t size = buffer->chunkList[lastChunk].size;
				return (((size / BUFFER_PAGE_SIZE)+1) * BUFFER_PAGE_SIZE) - size; // amount to the next 4096 length multiple
			}
		default:
			return 4096;
	}
}


inline BufferType GuessType( const Buffer *buffer, size_t lastChunk, size_t requiredLength ) {

	BufferChunk *chunk = &buffer->chunkList[buffer->chunkPos];
	if ( chunk->pos + requiredLength - chunk->size < BUFFER_TYPE_AUTO_THRESHOLD )
		return bufferTypeRealloc;
	else
		return bufferTypeChunk;
}


inline void BufferInitialize( Buffer *buffer, BufferType type, BufferGrowType growType ) {

	buffer->type = type;
	buffer->growType = growType;

	buffer->chunkList = buffer->staticChunkList;
	buffer->chunkListSize = COUNTOF(buffer->staticChunkList);
	buffer->chunkPos = 0;
	buffer->length = 0;

	BufferChunk *chunk0 = &buffer->chunkList[0];
	chunk0->pos = 0;
	switch ( type ) {
		case bufferTypeChunk:
			chunk0->size = sizeof(buffer->staticBuffer);
			chunk0->begin = buffer->staticBuffer;
			break;
		case bufferTypeAuto:
		case bufferTypeRealloc:
			chunk0->size = BUFFER_INIT_CHUNK_SIZE;
			chunk0->begin = (char*)malloc(chunk0->size);
			break;
	}
}


inline char *BufferNewChunk( Buffer *buffer, size_t maxLength ) {

	BufferChunk *chunk = &buffer->chunkList[buffer->chunkPos];
	if ( chunk->pos + maxLength <= chunk->size ) // enough room in the current chunk
		return chunk->begin + chunk->pos;

	switch ( buffer->type == bufferTypeAuto ? GuessType(buffer, buffer->chunkPos, maxLength) : buffer->type ) {
		case bufferTypeRealloc:
			chunk->size += NextChunkSize(buffer, buffer->chunkPos, maxLength);
			chunk->begin = (char*)realloc(chunk->begin, chunk->size);
			return chunk->begin + chunk->pos;

		case bufferTypeChunk:
			buffer->chunkPos++;
			if ( buffer->chunkPos >= buffer->chunkListSize ) {

				buffer->chunkListSize *= 2;
				if ( buffer->chunkList == buffer->staticChunkList ) {

					buffer->chunkList = (BufferChunk*)malloc(buffer->chunkListSize * sizeof(BufferChunk));
					memcpy(buffer->chunkList, buffer->staticChunkList, sizeof(buffer->staticChunkList));
				} else {

					buffer->chunkList = (BufferChunk*)realloc(buffer->chunkList, buffer->chunkListSize * sizeof(BufferChunk));
				}
			}

			BufferChunk *chunk = &buffer->chunkList[buffer->chunkPos];
			chunk->size = NextChunkSize(buffer, buffer->chunkPos-1, maxLength);
			chunk->begin = (char*)malloc(chunk->size);
			chunk->pos = 0;
			return chunk->begin;
	}
	return NULL;
}


inline void BufferConfirm( Buffer *buffer, size_t amount ) {

	buffer->chunkList[buffer->chunkPos].pos += amount;
	buffer->length += amount;
}

inline size_t BufferGetRecommendedLength( const Buffer *buffer ) {

	return 4096;
}

inline size_t BufferGetOptimalLength( const Buffer *buffer ) {

	return buffer->chunkList[buffer->chunkPos].size - buffer->chunkList[buffer->chunkPos].pos;
}


inline const size_t BufferGetLength( const Buffer *buffer ) {

	return buffer->length;
}

inline const char *BufferGetData( Buffer *buffer ) {

	BufferChunk *chunk0 = &buffer->chunkList[0];
	if ( buffer->chunkPos == 0 )
		return chunk0->begin;

	char *buf = (char*)malloc(buffer->length);
	char *dest = buf + buffer->length;
	size_t chunkIndex = buffer->chunkPos;

	while ( dest != buf ) {

		BufferChunk *chunk = &buffer->chunkList[chunkIndex--];
		dest -= chunk->pos;
		memcpy(dest, chunk->begin, chunk->pos);
		if ( chunk->begin != buffer->staticBuffer )
			free(chunk->begin);
	}

	chunk0->begin = buf;
	chunk0->pos = buffer->length;
	chunk0->size = buffer->length;
	buffer->chunkPos = 0;
	return buf;
}

// it is up to the client to free the buffer.
inline const char *BufferGetDataOwnership( Buffer *buffer ) {
	
	char *buf = (char*)BufferGetData(buffer);
	BufferChunk *chunk0 = &buffer->chunkList[0];
	if ( chunk0->begin == buffer->staticBuffer ) // unable to give ownership of the static buffer
		return NULL;
	chunk0->begin = NULL;
	chunk0->pos = 0;
	chunk0->size = 0;
	return buf;
}

// length MUST be <= buffer->length !
inline void BufferCopyData( const Buffer *buffer, char *dest, size_t length ) {

	size_t chunkIndex = 0;
	while ( length ) {

		BufferChunk *chunk = &buffer->chunkList[chunkIndex++];
		size_t amount = chunk->pos > length ? length : chunk->pos;
		memcpy(dest, chunk->begin, amount);
		length -= amount;
		dest += amount;
	}
}


inline void BufferFinalize( Buffer *buffer ) {

	for ( size_t i = buffer->chunkList[0].begin == buffer->staticBuffer ? 1 : 0; i <= buffer->chunkPos; i++ )
		free(buffer->chunkList[i].begin);
	if ( buffer->chunkList != buffer->staticChunkList )
		free(buffer->chunkList);
}


}
#endif // _BUFFER_H_


/* QA test:

DEFINE_FUNCTION_FAST( Test ) {

	char *ref = (char*)malloc(2000000);
	for ( int i = 0; i < 2000000; i++ )
		ref[i] = rand() & 0xff; // 0->255
	int refPos = 0;

	using namespace jl;
	Buffer b;
	BufferInitialize(&b, bufferTypeChunk, bufferGrowTypeGuess);

	for ( int i = 0; i < 100; i++ ) {

		int rnd = rand() & 0xff; // 0->127
		char *tmp = BufferNewChunk(&b, rnd);
		memcpy(tmp, ref + refPos, rnd);
		BufferConfirm(&b, rnd);
		refPos += rnd;
	}

	int l = BufferGetLength(&b);

	char *tmp = (char*)malloc(l);
	BufferCopyData(&b, tmp, l);

	const char *d = BufferGetData(&b);
	bool success = memcmp(ref, d, l) == 0 && memcmp(ref, tmp, l) == 0;

	return JS_TRUE;
}
*/
