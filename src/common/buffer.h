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


// (TBD) use this Buffer in: jsz, jsio::Descriptor, jsiconv, jslang::Stringify, DecodeOggVorbis, DecodeSound
//       AND perhaps in: jsstd::Buffer::ReadRawAmount, jsstd::Expand

JL_BEGIN_NAMESPACE


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

/*
#define BUFFER_INIT_CHUNK_SIZE 8192
#define BUFFER_INIT_CHUNK_LIST_SIZE 16
#define BUFFER_TYPE_AUTO_THRESHOLD 64
#define BUFFER_PAGE_SIZE 4096

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


typedef void* (*BufferAlloc) (void * opaqueAllocatorContext, size_t size);
typedef void (*BufferFree) (void * opaqueAllocatorContext, void* address);
typedef void* (*BufferRealloc) (void * opaqueAllocatorContext, void* address, size_t size);


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

// allocators
	void* opaqueAllocatorContext;
	BufferAlloc bufferAlloc;
	BufferFree bufferFree;
	BufferRealloc bufferRealloc;

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

	JL_IGNORE(buffer, lastChunk);

	BufferChunk *chunk = &buffer->chunkList[buffer->chunkPos];
	if ( chunk->pos + requiredLength - chunk->size < BUFFER_TYPE_AUTO_THRESHOLD )
		return bufferTypeRealloc;
	else
		return bufferTypeChunk;
}

inline void* DefaultBufferAlloc( void* opaqueAllocatorContext, size_t size ) {

	JL_IGNORE(opaqueAllocatorContext);
	return jl_malloc(size);
}

inline void DefaultBufferFree( void* opaqueAllocatorContext, void* memory ) {

	JL_IGNORE(opaqueAllocatorContext);
	jl_free(memory);
}

inline void* DefaultBufferRealloc( void* opaqueAllocatorContext, void* memory, size_t size ) {

	JL_IGNORE(opaqueAllocatorContext);
	return jl_realloc(memory, size);
}


inline void* _BufferAlloc( Buffer *buf, size_t size ) {

	return buf->bufferAlloc(buf->opaqueAllocatorContext, size);
}

inline void _BufferFree( Buffer *buf, void* memory ) {

	buf->bufferFree(buf->opaqueAllocatorContext, memory);
}

inline void* _BufferRealloc( Buffer *buf, void* memory, size_t size ) {

	return buf->bufferRealloc(buf->opaqueAllocatorContext, memory, size);
}


inline void BufferInitialize( Buffer *buffer, BufferType type, BufferGrowType growType, void* opaqueAllocatorContext, BufferAlloc bufferAlloc, BufferRealloc bufferRealloc, BufferFree bufferFree ) {

	buffer->opaqueAllocatorContext = opaqueAllocatorContext;
	buffer->bufferAlloc = bufferAlloc ? bufferAlloc : DefaultBufferAlloc;
	buffer->bufferFree = bufferFree ? bufferFree : DefaultBufferFree;
	buffer->bufferRealloc = bufferRealloc ? bufferRealloc : DefaultBufferRealloc;

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
			chunk0->begin = (char*)_BufferAlloc(buffer, chunk0->size); // leak from here
			break;
	}
}


inline char *BufferNewChunk( Buffer *buffer, size_t maxLength ) {

	BufferChunk *chunk = &buffer->chunkList[buffer->chunkPos];
	if ( chunk->pos + maxLength <= chunk->size ) // enough room in the current chunk
		return chunk->begin + chunk->pos;

	switch ( buffer->type == bufferTypeAuto ? GuessType(buffer, buffer->chunkPos, maxLength) : buffer->type ) {

		case bufferTypeAuto:
			return NULL; // invalid case.

		case bufferTypeRealloc:
			chunk->size += NextChunkSize(buffer, buffer->chunkPos, maxLength);
			chunk->begin = (char*)_BufferRealloc(buffer, chunk->begin, chunk->size);
			return chunk->begin + chunk->pos;

		case bufferTypeChunk:
			buffer->chunkPos++;
			if ( buffer->chunkPos >= buffer->chunkListSize ) {

				buffer->chunkListSize *= 2;
				if ( buffer->chunkList == buffer->staticChunkList ) {

					buffer->chunkList = (BufferChunk*)_BufferAlloc(buffer, buffer->chunkListSize * sizeof(BufferChunk));
					jl::memcpy(buffer->chunkList, buffer->staticChunkList, sizeof(buffer->staticChunkList));
				} else {

					buffer->chunkList = (BufferChunk*)_BufferRealloc(buffer, buffer->chunkList, buffer->chunkListSize * sizeof(BufferChunk));
				}
			}

			BufferChunk *chunk = &buffer->chunkList[buffer->chunkPos];
			chunk->size = NextChunkSize(buffer, buffer->chunkPos-1, maxLength);
			chunk->begin = (char*)_BufferAlloc(buffer, chunk->size);
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

	JL_IGNORE(buffer);

	return 4096;
}

inline size_t BufferGetOptimalLength( const Buffer *buffer ) {

	return buffer->chunkList[buffer->chunkPos].size - buffer->chunkList[buffer->chunkPos].pos;
}


inline size_t BufferGetLength( const Buffer *buffer ) {

	return buffer->length;
}


inline const char *BufferGetData( Buffer *buffer ) {

	BufferChunk *chunk0 = &buffer->chunkList[0];
	if ( buffer->chunkPos == 0 )
		return chunk0->begin;

	if ( chunk0->begin == buffer->staticBuffer )
		chunk0->begin = (char*)jl::memcpy(_BufferAlloc(buffer, buffer->length), chunk0->begin, chunk0->pos);
	else
		chunk0->begin = (char*)_BufferRealloc(buffer, chunk0->begin, buffer->length);
	chunk0->pos = buffer->length;
	chunk0->size = buffer->length;

	char *dest = chunk0->begin + buffer->length;
	do {

		BufferChunk *chunk = &buffer->chunkList[buffer->chunkPos];
		dest -= chunk->pos;
		jl::memcpy(dest, chunk->begin, chunk->pos);
		_BufferFree(buffer, chunk->begin);
	} while ( --buffer->chunkPos );

	return chunk0->begin;
}


// it is up to the client to free the buffer.
inline char *BufferGetDataOwnership( Buffer *buffer ) {
	
	char *buf = (char*)BufferGetData(buffer);
	if ( buf == buffer->staticBuffer ) // unable to give ownership of the static buffer, (TBD) copy it
		buf = (char*)jl::memcpy(_BufferAlloc(buffer, buffer->length), buf, buffer->length);

	buffer->chunkList[0].begin = NULL;
	return buf;
}

inline char *BufferGetDataZOwnership( Buffer *buffer ) {
	
	*BufferNewChunk(buffer, 1) = '\0';
	return BufferGetDataOwnership(buffer);
}


// length MUST be <= buffer->length !
inline void BufferCopyData( const Buffer *buffer, char *dest, size_t length ) {

	size_t chunkIndex = 0;
	while ( length ) {

		BufferChunk *chunk = &buffer->chunkList[chunkIndex++];
		size_t amount = chunk->pos > length ? length : chunk->pos;
		jl::memcpy(dest, chunk->begin, amount);
		length -= amount;
		dest += amount;
	}
}


inline void BufferFinalize( Buffer *buffer ) {

	for ( size_t i = buffer->chunkList[0].begin == buffer->staticBuffer ? 1 : 0; i <= buffer->chunkPos; i++ )
		_BufferFree(buffer, buffer->chunkList[i].begin);
	if ( buffer->chunkList != buffer->staticChunkList )
		_BufferFree(buffer, buffer->chunkList);
}
*/


template <class T>
class ChunkedBuffer {

	struct Chunk {
		T *begin;
		size_t pos;
		size_t size;
	};

	size_t _length;

	Chunk *_chunkList;
	size_t _chunkPos;
	size_t _chunkListSize;

	Chunk _staticChunkList[8];
	T _staticBuffer[4096];

public:
	~ChunkedBuffer() {

		for ( size_t i = _chunkList[0].begin == _staticBuffer ? 1 : 0; i <= _chunkPos; ++i )
			jl_free(_chunkList[i].begin);
		if ( _chunkList != _staticChunkList )
			jl_free(_chunkList);
	}

	ChunkedBuffer() {

		_chunkList = _staticChunkList;
		_chunkListSize = COUNTOF(_staticChunkList);
		_chunkPos = 0;
		_length = 0;

		Chunk *chunk0 = &_chunkList[0];
		chunk0->pos = 0;
		chunk0->size = sizeof(_staticBuffer);
		chunk0->begin = _staticBuffer;
	}

	void Reserve( size_t maxLength ) {

		Chunk *chunk = &_chunkList[_chunkPos];
		if ( chunk->pos + maxLength > chunk->size ) {

			_chunkPos++;
			if ( _chunkPos >= _chunkListSize ) {

				_chunkListSize *= 2;
				if ( _chunkList == _staticChunkList ) {

					_chunkList = (Chunk*)jl_malloc(_chunkListSize * sizeof(Chunk));
					jl::memcpy(_chunkList, _staticChunkList, sizeof(_staticChunkList));
				} else {

					_chunkList = (Chunk*)jl_realloc(_chunkList, _chunkListSize * sizeof(Chunk));
				}
			}

			chunk = &_chunkList[_chunkPos];
			chunk->size = _chunkList[_chunkPos-1].size * 3/2 + maxLength * 3/2;
			ASSERT( chunk->size >= maxLength );
			chunk->begin = (T*)jl_malloc(chunk->size);
			chunk->pos = 0;
		}
	}

	size_t Length() const {

		return _length;
	}

	size_t PtrLength() const {

		Chunk *chunk = &_chunkList[_chunkPos];
		return chunk->size - chunk->pos;
	}

	T *Ptr() const {

		Chunk *chunk = &_chunkList[_chunkPos];
		return chunk->begin + chunk->pos;
	}

	void Advance( size_t length ) {

		_chunkList[_chunkPos].pos += length;
		_length += length;
	}

	void Write( T *data, size_t length ) {

		Reserve(length);
		jl::memcpy(data, Ptr(), length * sizeof(T));
		Advance(length);
	}

	void CopyTo( T *dest, size_t length ) const {

		size_t chunkIndex = 0;
		while ( length ) {

			Chunk *chunk = &_chunkList[chunkIndex++];
			size_t amount = chunk->pos > length ? length : chunk->pos;
			jl::memcpy(dest, chunk->begin, amount);
			length -= amount;
			dest += amount;
		}
	}

	T *GetData() {

		Chunk *chunk0 = &_chunkList[0];
		if ( _chunkPos == 0 )
			return chunk0->begin;

		if ( chunk0->begin == _staticBuffer )
			chunk0->begin = (T*)jl::memcpy(jl_malloc(_length), chunk0->begin, chunk0->pos);
		else
			chunk0->begin = (T*)jl_realloc(chunk0->begin, _length);
		chunk0->pos = _length;
		chunk0->size = _length;

		T *dest = chunk0->begin + _length;
		do {

			Chunk *chunk = &_chunkList[_chunkPos];
			dest -= chunk->pos;
			jl::memcpy(dest, chunk->begin, chunk->pos);
			jl_free(chunk->begin);
		} while ( --_chunkPos );

		return chunk0->begin;
	}

	T *GetDataOwnership() {

		T *buf = GetData();
		if ( buf == _staticBuffer )
			buf = (T*)jl::memcpy(jl_malloc(_length), buf, _length);
		_chunkList[0].begin = NULL;
		return buf;
	}
};

JL_END_NAMESPACE
