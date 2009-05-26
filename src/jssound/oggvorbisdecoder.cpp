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

#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#define SLOT_INPUT_STREAM 0


/**doc fileIndex:top
$CLASS_HEADER
$SVN_REVISION $Revision$
 The OggVorbisDecoder support ogg vorbis data format decoding.
**/
BEGIN_CLASS( OggVorbisDecoder )


typedef struct {
	JSContext *cx; // temporary set to the current JSContext while sndfile API is called !
	JSObject *streamObject; // object available via SLOT_INPUT_STREAM too.
	OggVorbis_File ofDescriptor;
	vorbis_info *ofInfo;
	int bits;
} Private;



static size_t read_func( void *ptr, size_t size, size_t nmemb, void *privateData ) {

	Private *pv = (Private*)privateData;

	size_t amount = size * nmemb;
//	if ( info->streamRead( info->cx, info->obj, (char*)ptr, &amount ) != JS_TRUE )
//		return -1; // (TBD) check for a better error
	if ( StreamReadInterface(pv->cx, pv->streamObject)(pv->cx, pv->streamObject, (char*)ptr, &amount) != JS_TRUE )
		return -1; // (TBD) check for a better error
	return amount;
}

static int seek_func(void *datasource, ogg_int64_t offset, int whence) {

//	return -1;

	Private *pv = (Private*)datasource;

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
			if ( JSVAL_IS_VOID( tmpVal ) ) //
				return -1;
			if ( offset == 0 ) // no move, just tested, but let -1 to be return if no position property available.
				return 0;
			JsvalToInt(pv->cx, tmpVal, &position); // (TBD) manage error
			position += offset;
			IntToJsval(pv->cx, position, &tmpVal); // (TBD) manage error
			JS_SetProperty(pv->cx, pv->streamObject, "position", &tmpVal); // (TBD) manage error
			return 0;

		case SEEK_END:
			JS_GetProperty(pv->cx, pv->streamObject, "available", &tmpVal);
			if ( JSVAL_IS_VOID( tmpVal ) )
				return -1;
			JsvalToInt(pv->cx, tmpVal, &available);

			JS_GetProperty(pv->cx, pv->streamObject, "position", &tmpVal);
			if ( JSVAL_IS_VOID( tmpVal ) )
				return -1;
			JsvalToInt(pv->cx, tmpVal, &position);

			if ( offset > 0 || -offset > position + available )
				return -1;
			JsvalToInt(pv->cx, tmpVal, &position);
			IntToJsval(pv->cx, position + available + offset, &tmpVal); // the pointer is set to the size of the file plus offset.
			JS_SetProperty(pv->cx, pv->streamObject, "position", &tmpVal);
			return 0;
	}
	return -1; // doc: you *MUST* return -1 if the stream is unseekable
}




static long tell_func(void *datasource) {

	Private *pv = (Private*)datasource;
	jsval tmpVal;

	int position;
	JS_GetProperty(pv->cx, pv->streamObject, "position", &tmpVal);
	if ( JSVAL_IS_VOID( tmpVal ) )
		return -1;
	JsvalToInt(pv->cx, tmpVal, &position);
	return position;
}


static const ov_callbacks ovCallbacks = { read_func, seek_func, 0, tell_func };


DEFINE_FINALIZE() {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	if ( pv != NULL ) {

		ov_clear(&pv->ofDescriptor); // beware: info must be valid
		JS_free(cx, pv);
	}
}


/**doc
$TOC_MEMBER $INAME
 $INAME( stream )
  Creates a new OggVorbisDecoder object. Seekable and non-seekable streams are supported.
  $H arguments
   $ARG streamObject stream: is the data stream from where encoded data are read from.
  $H example
  {{{
  LoadModule('jsstd');
  LoadModule('jsio');
  LoadModule('jssound');
  var file = new File('41_30secOgg-q0.ogg'); // file: http://xiph.org/vorbis/listen.html
  file.Open('r');
  var dec = new OggVorbisDecoder( file );
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

	JL_S_ASSERT_CONSTRUCTING();
	JL_S_ASSERT_THIS_CLASS();
	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT_OBJECT( JL_ARG(1) );

	Private *pv = (Private*)JS_malloc(cx, sizeof(Private));
	JL_CHK( pv );
	JL_CHK( JL_SetPrivate(cx, obj, pv) );

	JL_CHK( JS_SetReservedSlot(cx, obj, SLOT_INPUT_STREAM, JL_ARG(1) ) );
	pv->streamObject = JSVAL_TO_OBJECT(JL_ARG(1));

	pv->cx = cx;
	int result = ov_open_callbacks(pv, &pv->ofDescriptor, NULL, 0, ovCallbacks);
	JL_S_ASSERT(result == 0, "Invalid ogg vorbis descriptor.");

	pv->ofInfo = ov_info(&pv->ofDescriptor, -1);
	JL_S_ASSERT(pv->ofInfo != NULL, "Invalid ogg vorbis info.");

	pv->bits = 16;

	pv->cx = NULL; // see definition
	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Methods ===
**/


// doc:
//   OV_HOLE is only alerting the application that a page was
//   corrupted/dropped/lost or there was garbage in the stream.  The app
//   can safely ignore the error and retry if it doesn't care.

/**doc
$TOC_MEMBER $INAME
 $TYPE soundObject $INAME( [frames] )
  Decodes a piece of audio data. If _frames_ argument is omited, the whole stream is decoded.
  $H arguments
   $ARG $INT frames: the number of frames to decode. A frame is a sample of sound.
  $H return value
   A Blob object that has the following properties set: bits, rate, channels, frames
  $H beware
   If all data has been decoded and the Read function is called again, the return expression is evaluated to false.
  $H example
  {{{
  LoadModule('jsstd');
  LoadModule('jsio');
  LoadModule('jssound');
  var file = new File('41_30secOgg-q0.ogg'); // file: http://xiph.org/vorbis/listen.html
  file.Open('r');
  var dec = new OggVorbisDecoder( file );
  var block = dec.Read(10000);
  Print( 'rezolution: '+block.bits+' per channel', '\n' );
  Print( block.channels == 2 ? 'stereo' : 'mono', '\n' );
  Print( block.rate+' frames/seconds', '\n' );
  Print( 'time: '+(block.frames/block.rate)+' seconds', '\n' );
  }}}
**/
DEFINE_FUNCTION_FAST( Read ) {

	Private *pv = (Private*)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE(pv);

	JL_S_ASSERT( pv->bits != 8 || pv->bits == 16, "Unsupported bits count." );
	JL_S_ASSERT( pv->ofInfo->channels == 1 || pv->ofInfo->channels == 2, "Unsupported channel count." );

	char *buf;
	size_t totalSize = 0;

	pv->cx = cx;
	if ( JL_FARG_ISDEF(1) ) {

		size_t frames;
		JL_CHK( JsvalToUInt(cx, JL_FARG(1), &frames) );

		if ( frames > 0 ) {

			size_t amount = frames * pv->ofInfo->channels * pv->bits/8; // amount in bytes
			buf = (char*)malloc(amount);
			JL_S_ASSERT_ALLOC(buf);

	//		sf_count_t items = sf_read_short(pv->sfDescriptor, (short*)buf, amount/sizeof(short));

			int bitStream = 0;
			long bytes;
			do {

				int prevBitstream = bitStream;
				bytes = ov_read(&pv->ofDescriptor, buf + totalSize, amount - totalSize, 0, pv->bits / 8, 1, &bitStream);
				JL_S_ASSERT( bitStream == prevBitstream, "Invalid ogg bitstream."); // bitstream has changed

				// (TBD) update the channels, rate, ... according to: ov_info(&pv->ofDescriptor, bitStream);
				if ( JS_IsExceptionPending(cx) )
					return JS_FALSE; // (TBD) free memory

				if ( bytes == OV_HOLE )
					continue; // ignore corrupted/dropped/lost parts

				totalSize += bytes;

			} while ( bytes > 0 && totalSize < amount );
			// manage ov_read errors here


	/*
			if ( bytes < 0 ) { // 0 indicates EOF

				// (TBD) manage errors
				if ( bytes == OV_HOLE ) { // indicates there was an interruption in the data. (one of: garbage between pages, loss of sync followed by recapture, or a corrupt page)

				} else if ( bytes == OV_EINVAL ) { // doc. indicates that an invalid stream section was supplied to libvorbisfile, or the requested link is corrupt.
					break;
				}
			}
	*/

			if ( MaybeRealloc(amount, totalSize) )
				buf = (char*)realloc(buf, totalSize);
		}

	} else {

		void *stack;
		jl::StackInit(&stack);

		int bufferSize = 16384 - 16; // try to alloc less than one page

		int bitStream = 0;
		long bytes;
		do {

			char *buffer = (char*)JS_malloc(cx, bufferSize);
			JL_CHK( buffer );
			jl::StackPush(&stack, buffer);

			char *data = buffer+sizeof(int);
			int *len = (int*)buffer;
			int maxlen = bufferSize - sizeof(int);

			// pv->bits: ???=8, sf_read_short=16, sf_read_int=32
//			items = sf_read_short(pv->sfDescriptor, (short*)data, maxlen/sizeof(short)); // bits per sample

			int prevBitstream = bitStream;
			bytes = ov_read(&pv->ofDescriptor, data, maxlen, 0, pv->bits / 8, 1, &bitStream);
			JL_S_ASSERT( bitStream == prevBitstream, "Invalid ogg bitstream."); // bitstream has changed

			if ( JS_IsExceptionPending(cx) )
				return JS_FALSE; // (TBD) free memory

			if ( bytes == OV_HOLE)
				continue; // ignore corrupted/dropped/lost parts

//(TBD)			JL_S_ASSERT_1( sf_error(pv->ofDescriptor) == SF_ERR_NO_ERROR, "sndfile error: %d", sf_error(pv->sfDescriptor) );

			if ( bytes <= 0 ) { // < 0 is an error

				*len = 0;
			} else {

				*len = bytes;
				totalSize += bytes;
			}

		} while (bytes > 0); // 0 indicates EOF

		// convert data chunks into a single memory buffer.
		buf = (char*)JS_malloc(cx, totalSize);
		JL_CHK( buf );

		// because the stack is LIFO, we have to start from the end.
		buf += totalSize;
		while( !jl::StackIsEnd(&stack) ) {

			char *buffer = (char *)jl::StackPop(&stack);
			int size = *(int*)buffer;
			buf = buf - size;
			memcpy( buf, buffer+sizeof(int), size );
			free(buffer);
		}
	}
	pv->cx = NULL; // see definition

	if ( totalSize == 0 ) {
	
		*JL_FRVAL = JSVAL_VOID;
		return JS_TRUE;
	}

	jsval blobVal;
	JL_CHK( JL_NewBlob(cx, buf, totalSize, &blobVal) );
	JSObject *blobObj;
	JL_CHK( JS_ValueToObject(cx, blobVal, &blobObj) );
	JL_S_ASSERT( blobObj != NULL, "Unable to create the Blob object.");
	*JL_FRVAL = OBJECT_TO_JSVAL(blobObj);

	JL_CHK( SetPropertyInt(cx, blobObj, "bits", pv->bits) ); // bits per sample
	JL_CHK( SetPropertyInt(cx, blobObj, "rate", pv->ofInfo->rate) ); // samples per second
	JL_CHK( SetPropertyInt(cx, blobObj, "channels", pv->ofInfo->channels) ); // 1:mono, 2:stereo
	JL_CHK( SetPropertyInt(cx, blobObj, "frames", totalSize / (pv->ofInfo->channels * pv->bits / 8) ) );

	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Properties ===
**/

/**doc
$TOC_MEMBER $INAME
 object $INAME $READONLY
  Is the stream object where encoded audio data are read from.
**/
DEFINE_PROPERTY( inputStream ) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	JL_CHK( JS_GetReservedSlot(cx, obj, SLOT_INPUT_STREAM, vp) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  Is the number of bits per frame and per channel.
**/
DEFINE_PROPERTY( bits ) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	*vp = INT_TO_JSVAL( pv->bits );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  Is the number of frames per seconds of the sound.
**/
DEFINE_PROPERTY( rate ) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	*vp = INT_TO_JSVAL( pv->ofInfo->rate );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  Is the number of channels of the sound. 1 is mono, 2 is stereo.
**/
DEFINE_PROPERTY( channels ) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	*vp = INT_TO_JSVAL( pv->ofInfo->channels );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  Is the length (in frames) of the sound.
  To compute the duration of the sound, use (frames/rate)
**/
DEFINE_PROPERTY( frames ) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	ogg_int64_t pcmTotal = ov_pcm_total(&pv->ofDescriptor, -1);
	if ( pcmTotal == OV_EINVAL ) { // if the stream is not seekable (we can't know the length) or only partially open.

		*vp = JSVAL_VOID;
		return JS_TRUE;
	}
//	size_t frames = pcmTotal / pv->ofInfo->channels; // (TBD) ???
	*vp = INT_TO_JSVAL( pcmTotal );
	return JS_TRUE;
	JL_BAD;
}



CONFIGURE_CLASS

	REVISION(SvnRevToInt("$Revision$"))
	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST(Read)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ(inputStream)
		PROPERTY_READ(bits)
		PROPERTY_READ(rate)
		PROPERTY_READ(channels)
		PROPERTY_READ(frames)
	END_PROPERTY_SPEC

	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1)

END_CLASS
