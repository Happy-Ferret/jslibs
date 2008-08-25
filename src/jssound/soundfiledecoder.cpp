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

#include "stdafx.h"
#include <cstring>

#include "../common/stack.h"

#include	<sndfile.h>

#define SLOT_INPUT_STREAM 0

/**doc fileIndex:top
$CLASS_HEADER
 The SoundFileDecoder support various data format decoding.
 Main supported formats are: wav, aiff, au, voc, sd2, flac, ...
 For more information about supported formats, see http://www.mega-nerd.com/libsndfile/
**/
BEGIN_CLASS( SoundFileDecoder )

typedef struct {
	JSContext *cx; // temporary set to the current JSContext while sndfile API is called !
	JSObject *streamObject;
	SNDFILE *sfDescriptor;
	SF_INFO sfInfo;
	int bits;
} Private;


static sf_count_t SfGetFilelen(void *user_data) {

	Private *pv = (Private*)user_data;

	jsval tmpVal;
	int position;
	JS_GetProperty(pv->cx, pv->streamObject, "position", &tmpVal); // (TBD) manage error
	if ( tmpVal == JSVAL_VOID )
		return -1;
	JsvalToInt(pv->cx, tmpVal, &position); // (TBD) manage error

	int available;
	JS_GetProperty(pv->cx, pv->streamObject, "available", &tmpVal); // (TBD) manage error
	if ( tmpVal == JSVAL_VOID )
		return -1;
	JsvalToInt(pv->cx, tmpVal, &available); // (TBD) manage error

	return position + available;
}


static sf_count_t SfSeek(sf_count_t offset, int whence, void *user_data) {

	Private *pv = (Private*)user_data;

	jsval tmpVal;
	int position, available;

	switch (whence) {
		case SEEK_SET:
			if ( offset < 0 )
				return -1;
			IntToJsval(pv->cx, offset, &tmpVal); // (TBD) manage error
			JS_SetProperty(pv->cx, pv->streamObject, "position", &tmpVal); // (TBD) manage error
			return 0;

		case SEEK_CUR:
			JS_GetProperty(pv->cx, pv->streamObject, "position", &tmpVal); // (TBD) manage error
			if ( tmpVal == JSVAL_VOID )
				return -1;
			JsvalToInt(pv->cx, tmpVal, &position); // (TBD) manage error

			position += offset;
			IntToJsval(pv->cx, position, &tmpVal); // (TBD) manage error
			JS_SetProperty(pv->cx, pv->streamObject, "position", &tmpVal); // (TBD) manage error
			return 0;

		case SEEK_END:
			JS_GetProperty(pv->cx, pv->streamObject, "available", &tmpVal);
			if ( tmpVal == JSVAL_VOID )
				return -1;
			JsvalToInt(pv->cx, tmpVal, &available);

			JS_GetProperty(pv->cx, pv->streamObject, "position", &tmpVal);
			if ( tmpVal == JSVAL_VOID )
				return -1;
			JsvalToInt(pv->cx, tmpVal, &position);

			if ( offset > 0 || -offset > position + available )
				return -1;
			JsvalToInt(pv->cx, tmpVal, &position);
			IntToJsval(pv->cx, position + available + offset, &tmpVal); // the pointer is set to the size of the file plus offset.
			JS_SetProperty(pv->cx, pv->streamObject, "position", &tmpVal);
			return 0;
	}
	return -1;
}

static sf_count_t SfTell(void *user_data) {

	Private *pv = (Private*)user_data;
	jsval tmpVal;

	int position;
	JS_GetProperty(pv->cx, pv->streamObject, "position", &tmpVal);
	if ( tmpVal == JSVAL_VOID )
		return -1;
	JsvalToInt(pv->cx, tmpVal, &position);

	return position;
}

static sf_count_t SfRead(void *ptr, sf_count_t count, void *user_data) {

	Private *pv = (Private*)user_data;

	size_t amount = count;
//	if ( pv->streamRead( pv->cx, pv->obj, (char*)ptr, &amount ) != JS_TRUE )
//		return -1; // (TBD) find a better error
	if ( StreamReadInterface( pv->cx, pv->streamObject)( pv->cx, pv->streamObject, (char*)ptr, &amount ) != JS_TRUE )
		return -1; // (TBD) find a better error
	return amount;
}

static SF_VIRTUAL_IO sfCallbacks = { SfGetFilelen, SfSeek, SfRead, 0, SfTell };


DEFINE_FINALIZE() {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	if ( pv != NULL ) {

		sf_close(pv->sfDescriptor);
		free(pv);
	}
}

/**doc
 * $INAME( stream )
  Creates a new SoundFileDecoder object. Only seekable streams are supported.
  $H arguments
   $ARG streamObject stream: is the data stream from where encoded data are read from.
  $H example
   {{{
   LoadModule('jsstd');
   LoadModule('jsio');
   LoadModule('jssound');
   var file = new File('41_30secOgg-q0.wav'); // file: http://xiph.org/vorbis/listen.html
   file.Open('r');
   var dec = new SoundFileDecoder( file );
   Print( dec.bits, '\n' );
   Print( dec.channels, '\n' );
   Print( dec.rate, '\n' );
	do {
		var block = dec.Read(10000);
		Print( 'frames: '+block.frames, '\n' );
	} while(block);
   }}}
**/
DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();
	J_S_ASSERT_ARG_MIN(1);
	J_S_ASSERT_OBJECT( J_ARG(1) );

	Private *pv = (Private*)malloc(sizeof(Private));
	J_S_ASSERT_ALLOC(pv);
	J_CHK( JS_SetPrivate(cx, obj, pv) );

	J_CHK( JS_SetReservedSlot(cx, obj, SLOT_INPUT_STREAM, J_ARG(1) ) );
	pv->streamObject = JSVAL_TO_OBJECT(J_ARG(1));

	SF_INFO tmp = {0};
	pv->sfInfo = tmp; // ok, memset does the same thing
	
	pv->cx = cx;
	pv->sfDescriptor = sf_open_virtual(&sfCallbacks, SFM_READ, &pv->sfInfo, pv);
	pv->bits = 16;

	if ( JS_IsExceptionPending(cx) )
		return JS_FALSE;

	// (TBD) manage sf_error_str()
	J_S_ASSERT_1( sf_error(pv->sfDescriptor) == SF_ERR_NO_ERROR, "sndfile error: %d", sf_error(pv->sfDescriptor) );
	J_S_ASSERT( pv->sfDescriptor != NULL, "Invalid stream." ); // (TBD) needed ?

	int subFormat = pv->sfInfo.format & SF_FORMAT_SUBMASK;
	if ( subFormat == SF_FORMAT_FLOAT || subFormat == SF_FORMAT_DOUBLE ) {

		// require the whole file to be read
		int result = sf_command(pv->sfDescriptor, SFC_SET_SCALE_FLOAT_INT_READ, NULL, SF_TRUE); // Doc. Set/clear the scale factor when integer (short/int) data is read from a file containing floating point data. (http://www.mega-nerd.com/libsndfile/api.html#note2)
	}

	pv->cx = NULL; // see definition
	return JS_TRUE;
}

/**doc
=== Methods ===
**/


// doc:
//   Unless the end of the file was reached during the read, the return value should equal the number of items requested. 


/**doc
 * $TYPE soundObject $INAME( [frames] )
  Decodes a piece of audio data. If _frames_ argument is omited, the whole stream is decoded.
  $H arguments
   $ARG integer frames: the number of frames to decode. A frame is a sample of sound.
  $H return value
   returns a Blob object that has the following properties set: bits, rate, channels, frames
  $H beware
   If all data has been decoded and the Read function is called again, the return expression is evaluated to false.
	This is because an empty Blob must be evaluated as false, like string literals ( !empty_blob == !"" )
  $H example
  {{{
  LoadModule('jsstd');
  LoadModule('jsio');
  LoadModule('jssound');
  var file = new File('41_30secOgg-q0.wav'); // file: http://xiph.org/vorbis/listen.html
  file.Open('r');
  var dec = new SoundFileDecoder( file );
  var block = dec.Read(10000);
  Print( 'rezolution: '+block.bits+' per channel', '\n' );
  Print( block.channels == 2 ? 'stereo' : 'mono', '\n' );
  Print( block.rate+' frames/seconds', '\n' );
  Print( 'time: '+(block.frames/block.rate)+' seconds', '\n' );
  }}}
**/DEFINE_FUNCTION_FAST( Read ) {

	Private *pv = (Private*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(pv);

	J_S_ASSERT( pv->sfInfo.channels == 1 || pv->sfInfo.channels == 2, "Unsupported channel count." );

	char *buf;
	long totalSize = 0;

	pv->cx = cx;
	if ( J_FARG_ISDEF(1) ) {

		size_t frames;
		J_JSVAL_TO_INT32(J_FARG(1), frames);

		size_t amount = frames * pv->sfInfo.channels * pv->bits/8; // amount in bytes
		buf = (char*)malloc(amount);
		J_S_ASSERT_ALLOC(buf);

		sf_count_t items = sf_read_short(pv->sfDescriptor, (short*)buf, amount/sizeof(short));

		if ( JS_IsExceptionPending(cx) )
			return JS_FALSE; // (TBD) free memory

		totalSize = items * sizeof(short);

		if ( MaybeRealloc(amount, totalSize) )
			buf = (char*)realloc(buf, totalSize);

	} else {

		void *stack;
		StackInit(&stack);

		int bufferSize = 16384 - 16; // try to alloc less than one page

		sf_count_t items;
		do {

			char *buffer = (char*)JS_malloc(cx, bufferSize);
			J_S_ASSERT_ALLOC(buffer);
			StackPush(&stack, buffer);

			char *data = buffer+sizeof(int);
			int *len = (int*)buffer;
			int maxlen = bufferSize - sizeof(int);

			// pv->bits: ???=8, sf_read_short=16, sf_read_int=32
			items = sf_read_short(pv->sfDescriptor, (short*)data, maxlen/sizeof(short)); // bits per sample

			if ( JS_IsExceptionPending(cx) )
				return JS_FALSE; // (TBD) free memory

			J_S_ASSERT_1( sf_error(pv->sfDescriptor) == SF_ERR_NO_ERROR, "sndfile error: %d", sf_error(pv->sfDescriptor) );

			if ( items <= 0 ) { // 0 indicates EOF

				*len = 0;
			} else {

				*len = items * sizeof(short);
				totalSize += items * sizeof(short);
			}

		} while (items > 0);

		// convert data chunks into a single memory buffer.
		buf = (char*)JS_malloc(cx, totalSize);
		J_S_ASSERT_ALLOC(buf);

		// because the stack is LIFO, we have to start from the end.
		buf += totalSize;
		while( !StackIsEnd(&stack) ) {

			char *buffer = (char *)StackPop(&stack);
			int size = *(int*)buffer;
			buf = buf - size;
			memcpy( buf, buffer+sizeof(int), size );
			free(buffer);
		}
	}
	pv->cx = NULL; // see definition

	jsval blobVal;
	J_CHK( J_NewBlob(cx, buf, totalSize, &blobVal) );
	JSObject *blobObj;
	J_CHK( JS_ValueToObject(cx, blobVal, &blobObj) );
	J_S_ASSERT( blobObj != NULL, "Unable to create the Blob object.");
	*J_FRVAL = OBJECT_TO_JSVAL(blobVal);

	J_CHK( SetPropertyInt(cx, blobObj, "bits", pv->bits) ); // bits per sample
	J_CHK( SetPropertyInt(cx, blobObj, "rate", pv->sfInfo.samplerate) ); // samples per second
	J_CHK( SetPropertyInt(cx, blobObj, "channels", pv->sfInfo.channels) ); // 1:mono, 2:stereo
	J_CHK( SetPropertyInt(cx, blobObj, "frames", totalSize / (pv->sfInfo.channels * pv->bits / 8) ) );

	return JS_TRUE;
}



/**doc
=== Properties ===
**/

/**doc
 * object $INAME $READONLY
  Is the stream object where encoded audio data are read from.
**/
DEFINE_PROPERTY( inputStream ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(pv);
	J_CHK( JS_GetReservedSlot(cx, obj, SLOT_INPUT_STREAM, vp) );
	return JS_TRUE;
}


/**doc
 * $INT $INAME $READONLY
  Is the number of bits per frame and per channel.
**/
DEFINE_PROPERTY( bits ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(pv);
	*vp = INT_TO_JSVAL( pv->bits );
	return JS_TRUE;
}

/**doc
 * $INT $INAME $READONLY
  Is the number of frames per seconds of the sound.
**/
DEFINE_PROPERTY( rate ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(pv);
	*vp = INT_TO_JSVAL( pv->sfInfo.samplerate );
	return JS_TRUE;
}

/**doc
 * $INT $INAME $READONLY
  Is the number of channels of the sound. 1 is mono, 2 is stereo.
**/
DEFINE_PROPERTY( channels ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(pv);
	*vp = INT_TO_JSVAL( pv->sfInfo.channels );
	return JS_TRUE;
}

/**doc
 * $INT $INAME $READONLY
  Is the length (in frames) of the sound.
  To compute the duration of the sound, use (frames/rate)
**/
DEFINE_PROPERTY( frames ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(pv);
	*vp = INT_TO_JSVAL( pv->sfInfo.frames );
	return JS_TRUE;
}


CONFIGURE_CLASS

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST(Read)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ(bits)
		PROPERTY_READ(rate)
		PROPERTY_READ(channels)
		PROPERTY_READ(frames)
	END_PROPERTY_SPEC

	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1)

END_CLASS