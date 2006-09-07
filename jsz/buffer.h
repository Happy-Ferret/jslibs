#include <stdlib.h>

#define STATIC_BUFFER_LENGTH 8192
#define INITIAL_QUEUE_LENGTH 8

struct BufferChunk {
friend class Buffer;

	size_t total;
	int useCount;
public:
	char *data;
	size_t avail;
};


class Buffer {

	char _staticMem[STATIC_BUFFER_LENGTH];
	bool _hasStaticMem;

	size_t _defaultLength;

	BufferChunk *_queue;
	int _endIndex;
	int _queueLength;
	int _useCount;

	size_t _prevAvail;
	size_t _prevRequest;

public:

	~Buffer() {

		for ( int i=0; i<_endIndex; i++ )
			if ( _queue[i].data != _staticMem )
				free(_queue[i].data);
	}

	Buffer( size_t defaultLength ) {

		_defaultLength = defaultLength;

		_hasStaticMem = true;

		_queueLength = INITIAL_QUEUE_LENGTH;
		_queue = (BufferChunk*)malloc( _queueLength * sizeof BufferChunk );
		_endIndex = 0;

		_queue[0].data = NULL;
		_queue[0].total = 0;
		_queue[0].avail = 0;
		_queue[0].useCount = 0;

		_prevAvail = 0;
		_prevRequest = 0;
		_useCount = 0;
	}

	size_t OptimalLength() {
		
		size_t length = _queue[_endIndex].avail;

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
	
			float prevUsedRatio = (float)(_prevAvail - _queue[_endIndex].avail) / (float)(_prevRequest +1);

			if ( prevUsedRatio > 0.99f ) {
				length = _prevRequest * 2;
			} else if ( prevUsedRatio < 0.5f ) {
				length = _prevRequest / 2 + 1;
			} else {
				length = _prevRequest;
			}
		}
		return length;
	}

	BufferChunk *Next( size_t length ) {

		_useCount++;

		if ( _queue[_endIndex].avail <= 0 ) { // if not enough space in the current ckunk
			
			_endIndex++;
			if ( _endIndex >= _queueLength ) {

				_queueLength *= 2;
				_queue = (BufferChunk*)realloc( _queue, _queueLength * sizeof BufferChunk );
			}
			
			_queue[_endIndex].useCount = 0;
			if ( _hasStaticMem && length <= sizeof _staticMem ) {

				_queue[_endIndex].total = _queue[_endIndex].avail = sizeof _staticMem;
				_queue[_endIndex].data = _staticMem;
				_hasStaticMem = false;
			} else {

				_queue[_endIndex].total = _queue[_endIndex].avail = length;
				_queue[_endIndex].data = (char*)malloc(length);
			}
		} else {
			_queue[_endIndex].useCount++;
		}
		
		_prevRequest = length;
		_prevAvail = _queue[_endIndex].avail;

		return &_queue[_endIndex];
	}

	size_t MergeLength() {

		size_t totalLength=0;
		for ( int i=0; i<_endIndex; ++i )
			totalLength += _queue[i].total - _queue[i].avail;
		return totalLength;
	}

	void Merge( char *data ) { // alloc is made by the caller

		for ( int i=0; i<_endIndex; ++i ) {
			
			size_t length = _queue[i].total - _queue[i].avail;
			memcpy( data, _queue[i].data, length );
			data += length;
		}
	}

	size_t _WasteSpace() {

		size_t waste=0;
		for ( int i=0; i<_endIndex; ++i )
			waste += _queue[i].avail;
		return waste;
	}

};