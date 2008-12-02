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

#define BUFFER_INIT_CHUNK_SIZE 1024
#define BUFFER_INIT_CHUNK_LIST_SIZE 16

// (TBD) use this Buffer in: jsz, jsio::Descriptor, jsiconv, jslang::Stringify, DecodeOggVorbis, DecodeSound
//       AND perhaps in: jsstd::Buffer::ReadRawAmount, jsstd::Expand

namespace jl {


enum BufferType {
	bufferTypeChunk,
	bufferTypeRealloc,
};


enum BufferGrowType {
	bufferGrowTypeNoGuess, // minimalist grow
	bufferGrowTypeGuess, // best grow
	bufferGrowTypeDouble, // exponential grow
	bufferGrowTypePage // constant grow
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
	BufferChunk staticChunkList[BUFFER_INIT_CHUNK_LIST_SIZE];
	char staticBuffer[BUFFER_INIT_CHUNK_SIZE];
};


inline size_t GuessNextChunkSize( const Buffer *buffer, size_t lastChunk, size_t requiredLength ) {

	switch ( buffer->growType ) {
		case bufferGrowTypeGuess:
			return buffer->chunkList[lastChunk].size * 3/2 + requiredLength * 3/2;
		case bufferGrowTypeNoGuess:
			return requiredLength;
		case bufferGrowTypeDouble:
			return buffer->length;
		case bufferGrowTypePage:
		default:
			return 4096;
	}
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
	if ( type == bufferTypeChunk ) {

		chunk0->size = sizeof(buffer->staticBuffer);
		chunk0->begin = buffer->staticBuffer;
	} else
	if ( type == bufferTypeRealloc ) {

		chunk0->size = BUFFER_INIT_CHUNK_SIZE;
		chunk0->begin = (char*)malloc(chunk0->size);
	}
}


inline char *BufferNewChunk( Buffer *buffer, size_t maxLength ) {

//	buffer->length += maxLength; // see Confirm()
	BufferChunk *chunk = &buffer->chunkList[buffer->chunkPos];
	if ( chunk->pos + maxLength <= chunk->size ) { //enough room in the current buffer
		
//		char *tmp = chunk->begin + chunk->pos;
//		chunk->pos += maxLength;
//		return tmp;
		return chunk->begin + chunk->pos;
	}

	if ( buffer->type == bufferTypeRealloc ) {

		chunk->size += GuessNextChunkSize(buffer, buffer->chunkPos, maxLength);
		chunk->begin = (char*)realloc(chunk->begin, chunk->size);
//		char *tmp = chunk->begin + chunk->pos;
//		chunk->pos += maxLength;
//		return tmp;
		return chunk->begin + chunk->pos;
	} else
	if ( buffer->type == bufferTypeChunk ) {

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
		chunk->size = GuessNextChunkSize(buffer, buffer->chunkPos-1, maxLength);
		chunk->begin = (char*)malloc(chunk->size);
//		chunk->pos = maxLength;
		chunk->pos = 0;
		return chunk->begin;
	}
	return NULL;
}

/*
// adjust the lastest chunk length
inline void BufferUnused( Buffer *buffer, size_t unused ) {

	buffer->chunkList[buffer->chunkPos].pos -= unused;
	buffer->length -= unused;
}
*/

inline void BufferConfirm( Buffer *buffer, size_t amount ) {

	buffer->chunkList[buffer->chunkPos].pos += amount;
	buffer->length += amount;
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

	for ( size_t i = buffer->chunkList[0].begin == buffer->staticBuffer ? 1 : 0 ; i <= buffer->chunkPos; i++ )
		free(buffer->chunkList[i].begin);
	if ( buffer->chunkList != buffer->staticChunkList )
		free(buffer->chunkList);
}


}
#endif // _BUFFER_H_



/* QA test:


	char *ref = (char*)malloc(2000000);
	for ( int i = 0; i < 2000000; i++ )
		ref[i] = rand() & 0xff; // 0->255
	int refPos = 0;

	using namespace jl;
	Buffer b;
	BufferInitialize(&b, BUFFER_TYPE_CHUNK);

	for ( int i = 0; i < 100; i++ ) {

		int rnd = rand() & 0xff; // 0->127
		char *tmp = BufferNewChunk(&b, rnd);
		memcpy(tmp, ref + refPos, rnd);
		refPos += rnd;
	}

	int l = BufferGetLength(&b);

	char *tmp = (char*)malloc(l);
	BufferCopyData(&b, tmp, l);

	const char *d = BufferGetData(&b);
	bool success = memcmp(ref, d, l) == 0 && memcmp(ref, tmp, l) == 0;

*/

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

#define BUFFER_INIT_CHUNK_SIZE 1024
#define BUFFER_INIT_CHUNK_LIST_SIZE 16

// (TBD) use this Buffer in: jsz, jsio::Descriptor, jsiconv, jslang::Stringify, DecodeOggVorbis, DecodeSound
//       AND perhaps in: jsstd::Buffer::ReadRawAmount, jsstd::Expand

namespace jl {


enum BufferType {
	bufferTypeChunk,
	bufferTypeRealloc,
};


enum BufferGrowType {
	bufferGrowTypeNoGuess, // minimalist grow
	bufferGrowTypeGuess, // best grow
	bufferGrowTypeDouble, // exponential grow
	bufferGrowTypePage // constant grow
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
	BufferChunk staticChunkList[BUFFER_INIT_CHUNK_LIST_SIZE];
	char staticBuffer[BUFFER_INIT_CHUNK_SIZE];
};


inline size_t GuessNextChunkSize( const Buffer *buffer, size_t lastChunk, size_t requiredLength ) {

	switch ( buffer->growType ) {
		case bufferGrowTypeGuess:
			return buffer->chunkList[lastChunk].size * 3/2 + requiredLength * 3/2;
		case bufferGrowTypeNoGuess:
			return requiredLength;
		case bufferGrowTypeDouble:
			return buffer->length;
		case bufferGrowTypePage:
		default:
			return 4096;
	}
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
	if ( type == bufferTypeChunk ) {

		chunk0->size = sizeof(buffer->staticBuffer);
		chunk0->begin = buffer->staticBuffer;
	} else
	if ( type == bufferTypeRealloc ) {

		chunk0->size = BUFFER_INIT_CHUNK_SIZE;
		chunk0->begin = (char*)malloc(chunk0->size);
	}
}


inline char *BufferNewChunk( Buffer *buffer, size_t maxLength ) {

	buffer->length += maxLength;
	BufferChunk *chunk = &buffer->chunkList[buffer->chunkPos];
	if ( chunk->pos + maxLength <= chunk->size ) { //enough room in the current buffer
		
		char *tmp = chunk->begin + chunk->pos;
		chunk->pos += maxLength;
		return tmp;
	}

	if ( buffer->type == bufferTypeRealloc ) {

		chunk->size += GuessNextChunkSize(buffer, buffer->chunkPos, maxLength);
		chunk->begin = (char*)realloc(chunk->begin, chunk->size);
		char *tmp = chunk->begin + chunk->pos;
		chunk->pos += maxLength;
		return tmp;
	} else
	if ( buffer->type == bufferTypeChunk ) {

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
		chunk->size = GuessNextChunkSize(buffer, buffer->chunkPos-1, maxLength);
		chunk->begin = (char*)malloc(chunk->size);
		chunk->pos = maxLength;
		return chunk->begin;
	}
	return NULL;
}


// adjust the lastest chunk length
inline void BufferUnused( Buffer *buffer, size_t unused ) {

	buffer->chunkList[buffer->chunkPos].pos -= unused;
	buffer->length -= unused;
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

	for ( size_t i = buffer->chunkList[0].begin == buffer->staticBuffer ? 1 : 0 ; i <= buffer->chunkPos; i++ )
		free(buffer->chunkList[i].begin);
	if ( buffer->chunkList != buffer->staticChunkList )
		free(buffer->chunkList);
}


}
#endif // _BUFFER_H_



/* QA test:


	char *ref = (char*)malloc(2000000);
	for ( int i = 0; i < 2000000; i++ )
		ref[i] = rand() & 0xff; // 0->255
	int refPos = 0;

	using namespace jl;
	Buffer b;
	BufferInitialize(&b, BUFFER_TYPE_CHUNK);

	for ( int i = 0; i < 100; i++ ) {

		int rnd = rand() & 0xff; // 0->127
		char *tmp = BufferNewChunk(&b, rnd);
		memcpy(tmp, ref + refPos, rnd);
		refPos += rnd;
	}

	int l = BufferGetLength(&b);

	char *tmp = (char*)malloc(l);
	BufferCopyData(&b, tmp, l);

	const char *d = BufferGetData(&b);
	bool success = memcmp(ref, d, l) == 0 && memcmp(ref, tmp, l) == 0;

*/

