#include <stdlib.h>

#define STATIC_BUFFER_LENGTH (16384 -1)
#define INITIAL_QUEUE_LENGTH 8


struct BufferChunk {
friend class Buffer;

	unsigned char *mem;
public:
	unsigned char *data;
	size_t avail;
};


class Buffer {

	unsigned char _staticMem[STATIC_BUFFER_LENGTH];
	bool _hasStaticMem;

	size_t _defaultLength;

	BufferChunk *_queue;
	int _currentIndex;
	int _queueLength;
	int _useCount;
	size_t _length;

	size_t _prevAvail;
	size_t _prevRequest;

public:

	static const size_t staticBufferLength = STATIC_BUFFER_LENGTH;

	~Buffer() {

		for ( int i=0; i<=_currentIndex; i++ )
			if ( _queue[i].mem != _staticMem )
				free(_queue[i].mem);
	}

	Buffer( size_t defaultLength ) {

		_defaultLength = defaultLength;

		_hasStaticMem = true;

		_queueLength = INITIAL_QUEUE_LENGTH;
		_queue = (BufferChunk*)malloc( _queueLength * sizeof BufferChunk );
		_currentIndex = 0;

		_queue[0].mem = NULL;
		_queue[0].data = NULL;
		_queue[0].avail = 0;

		_prevAvail = 0;
		_prevRequest = 0;
		_useCount = 0;
		_length = 0;
	}

	size_t OptimalLength() {
		
		size_t length = _queue[_currentIndex].avail;

		if ( length > 0 )
			return length;

		if ( _hasStaticMem )
			return sizeof _staticMem;

		return _defaultLength;
	}

	size_t SmartLength() {
		
		size_t length;

		if ( _useCount == 0 ) {

			 length = sizeof _staticMem;
		} else {
	
			float prevUsedRatio = (float)(_prevAvail - _queue[_currentIndex].avail) / (float)(_prevRequest +1);

			if ( prevUsedRatio >= 1.f ) {
				length = (float)_prevRequest * 1.5f;
			} else if ( prevUsedRatio < 0.5f ) {
				length = 1+ (float)_prevRequest / 1.5f;
			} else {
				length = _prevRequest;
			}
		}
		return length;
	}

	BufferChunk *Next( size_t length ) {

		_useCount++;
		if ( _queue[_currentIndex].avail <= 0 ) { // if not enough space in the current ckunk
			
			_length += _queue[_currentIndex].data - _queue[_currentIndex].mem;

			_currentIndex++;
			if ( _currentIndex >= _queueLength ) {

				_queueLength *= 2;
				_queue = (BufferChunk*)realloc( _queue, _queueLength * sizeof BufferChunk );
			}
			
			if ( _hasStaticMem && length <= sizeof _staticMem ) {

				_queue[_currentIndex].avail = sizeof _staticMem;
				_queue[_currentIndex].mem = _queue[_currentIndex].data = _staticMem;
				_hasStaticMem = false; // it is no more available
			} else {

				_queue[_currentIndex].avail = length;
				_queue[_currentIndex].mem = _queue[_currentIndex].data = (unsigned char*)malloc(length);
			}
		}

		_prevRequest = length;
		_prevAvail = _queue[_currentIndex].avail;

		return &_queue[_currentIndex];
	}

	size_t Length() {

//		size_t totalLength=0;
//		for ( int i=0; i<=_currentIndex; ++i )
//			totalLength += _queue[i].data - _queue[i].mem;
//		return totalLength;
		return _length + _queue[_currentIndex].data - _queue[_currentIndex].mem;
	}

	void Read( unsigned char *data ) { // alloc is made by the caller

		for ( int i=0; i<=_currentIndex; ++i ) {
			
			size_t length = _queue[i].data - _queue[i].mem;
			memcpy( data, _queue[i].mem, length );
			data += length;
		}
	}

	size_t _WasteSpace() {

		size_t waste=0;
		for ( int i=0; i<_currentIndex; ++i )
			waste += _queue[i].avail;
		return waste;
	}

};