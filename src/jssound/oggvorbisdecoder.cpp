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


#define malloc jl_malloc_fct
#define calloc jl_calloc_fct
#define realloc jl_realloc_fct
#define free jl_free_fct

#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#undef malloc
#undef calloc
#undef realloc
#undef free



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



size_t read_func( void *ptr, size_t size, size_t nmemb, void *privateData ) {

	Private *pv = (Private*)privateData;

	size_t amount = size * nmemb;
//	if ( info->streamRead( info->cx, info->obj, (char*)ptr, &amount ) != JS_TRUE )
//		return -1; // (TBD) check for a better error
	if ( StreamReadInterface(pv->cx, pv->streamObject)(pv->cx, pv->streamObject, (char*)ptr, &amount) != JS_TRUE )
		return (size_t)-1; // (TBD) check for a better error
	return amount;
}

int seek_func(void *datasource, ogg_int64_t offset, int whence) {

//	return -1;

	Private *pv = (Private*)datasource;

	jsval tmpVal;
	int64_t position;
	int available;

	switch (whence) {
		case SEEK_SET:
			if ( offset < 0 )
				return -1;
			JL_NativeToJsval(pv->cx, offset, &tmpVal); // (TBD) manage error
			JS_SetProperty(pv->cx, pv->streamObject, "position", &tmpVal); // (TBD) manage error
			return 0;

		case SEEK_CUR:
			JS_GetProperty(pv->cx, pv->streamObject, "position", &tmpVal); // (TBD) manage error
			if ( JSVAL_IS_VOID( tmpVal ) ) //
				return -1;
			if ( offset == 0 ) // no move, just tested, but let -1 to be return if no position property available.
				return 0;
			JL_JsvalToNative(pv->cx, tmpVal, &position); // (TBD) manage error
			position += offset;
			JL_NativeToJsval(pv->cx, position, &tmpVal); // (TBD) manage error
			JS_SetProperty(pv->cx, pv->streamObject, "position", &tmpVal); // (TBD) manage error
			return 0;

		case SEEK_END:
			JS_GetProperty(pv->cx, pv->streamObject, "available", &tmpVal);
			if ( JSVAL_IS_VOID( tmpVal ) )
				return -1;
			JL_JsvalToNative(pv->cx, tmpVal, &available);

			JS_GetProperty(pv->cx, pv->streamObject, "position", &tmpVal);
			if ( JSVAL_IS_VOID( tmpVal ) )
				return -1;
			JL_JsvalToNative(pv->cx, tmpVal, &position);

			if ( offset > 0 || -offset > position + available )
				return -1;
			JL_JsvalToNative(pv->cx, tmpVal, &position);
			JL_NativeToJsval(pv->cx, position + available + offset, &tmpVal); // the pointer is set to the size of the file plus offset.
			JS_SetProperty(pv->cx, pv->streamObject, "position", &tmpVal);
			return 0;
	}
	return -1; // doc: you *MUST* return -1 if the stream is unseekable
}




long tell_func(void *datasource) {

	Private *pv = (Private*)datasource;
	jsval tmpVal;

	int position;
	JS_GetProperty(pv->cx, pv->streamObject, "position", &tmpVal);
	if ( JSVAL_IS_VOID( tmpVal ) )
		return -1;
	JL_JsvalToNative(pv->cx, tmpVal, &position);
	return position;
}


static const ov_callbacks ovCallbacks = { read_func, seek_func, 0, tell_func };


DEFINE_FINALIZE() {

	if ( JL_GetHostPrivate(cx)->canSkipCleanup )
		return;

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

	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT_ARG_IS_OBJECT(1);

	Private *pv = (Private*)JS_malloc(cx, sizeof(Private));
	JL_CHK( pv );
	JL_SetPrivate(cx, obj, pv);

	JL_CHK( JL_SetReservedSlot(cx, obj, SLOT_INPUT_STREAM, JL_ARG(1) ) );
	pv->streamObject = JSVAL_TO_OBJECT(JL_ARG(1));

	pv->cx = cx;
	int result = ov_open_callbacks(pv, &pv->ofDescriptor, NULL, 0, ovCallbacks);
	JL_CHKM( result == 0, E_ARG, E_NUM(1), E_INVALID, E_COMMENT("ogg vorbis descriptor") );

	pv->ofInfo = ov_info(&pv->ofDescriptor, -1);
	JL_CHKM( pv->ofInfo != NULL, E_ARG, E_NUM(1), E_INVALID, E_COMMENT("ogg vorbis info") );

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
   If all data has been decoded and the Read function is called again, the return expression is evaluated to $FALSE.
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
DEFINE_FUNCTION( Read ) {

	char *buf = NULL;
	char *buffer = NULL;

	JL_DEFINE_FUNCTION_OBJ;

	Private *pv = (Private*)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	JL_CHKM( pv->ofInfo->channels == 1 || pv->ofInfo->channels == 2, E_NUM(pv->ofInfo->channels), E_STR("channels"), E_FORMAT );
	JL_CHKM( pv->bits == 8 || pv->bits == 16, E_NUM(pv->bits), E_STR("bit"), E_FORMAT );

	size_t totalSize = 0;

	pv->cx = cx;
	if ( JL_ARG_ISDEF(1) ) {

		size_t frames;
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &frames) );

		if ( frames > 0 ) {

			size_t amount = frames * pv->ofInfo->channels * pv->bits/8; // amount in bytes
			buf = (char*)jl_malloc(amount +1);
			JL_ASSERT_ALLOC(buf);

	//		sf_count_t items = sf_read_short(pv->sfDescriptor, (short*)buf, amount/sizeof(short));

			int bitStream = 0;
			long bytes;
			do {

				int prevBitstream = bitStream;
				bytes = ov_read(&pv->ofDescriptor, buf + totalSize, amount - totalSize, 0, pv->bits / 8, 1, &bitStream);
				JL_CHKM( bitStream == prevBitstream, E_ARG, E_NUM(1), E_FORMAT, E_COMMENT("ogg vorbis bitstream") ); // bitstream has changed

				// (TBD) update the channels, rate, ... according to: ov_info(&pv->ofDescriptor, bitStream);
				if ( JL_IsExceptionPending(cx) )
					return JS_FALSE;

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

			if ( JL_MaybeRealloc(amount, totalSize) )
				buf = (char*)jl_realloc(buf, totalSize +1);
		} else {

			JL_ERR( E_ARG, E_NUM(1), E_MIN, E_NUM(1) );
		}

	} else {

		void *stack;
		jl::StackInit(&stack);

		int bufferSize = 16384 - 16; // try to alloc less than one page

		int bitStream = 0;
		long bytes;
		do {

			buffer = (char*)jl_malloc(bufferSize);
			JL_CHK( buffer );
			jl::StackPush(&stack, buffer);

			char *data = buffer+sizeof(int);
			int *len = (int*)buffer;
			int maxlen = bufferSize - sizeof(int);

			// pv->bits: ???=8, sf_read_short=16, sf_read_int=32
//			items = sf_read_short(pv->sfDescriptor, (short*)data, maxlen/sizeof(short)); // bits per sample

			int prevBitstream = bitStream;
			bytes = ov_read(&pv->ofDescriptor, data, maxlen, 0, pv->bits / 8, 1, &bitStream);
			JL_CHKM( bitStream == prevBitstream, E_ARG, E_NUM(1), E_FORMAT, E_COMMENT("ogg vorbis bitstream") ); // bitstream has changed

			if ( JL_IsExceptionPending(cx) )
				return JS_FALSE;

			if ( bytes == OV_HOLE)
				continue; // ignore corrupted/dropped/lost parts

//(TBD)			JL_ASSERT( sf_error(pv->ofDescriptor) == SF_ERR_NO_ERROR, "sndfile error: %d", sf_error(pv->sfDescriptor) );

			if ( bytes <= 0 ) { // < 0 is an error

				*len = 0;
			} else {

				*len = bytes;
				totalSize += bytes;
			}

		} while (bytes > 0); // 0 indicates EOF

		// convert data chunks into a single memory buffer.
		buf = (char*)jl_malloc(totalSize +1);
		JL_CHK( buf );

		// because the stack is LIFO, we have to start from the end.
		buf += totalSize;
		while( !jl::StackIsEnd(&stack) ) {

			char *buffer = (char *)jl::StackPop(&stack);
			int size = *(int*)buffer;
			buf = buf - size;
			memcpy( buf, buffer+sizeof(int), size );
			jl_free(buffer);
		}
	}
	pv->cx = NULL; // see definition

	if ( totalSize == 0 ) {

		*JL_RVAL = JSVAL_VOID;
		return JS_TRUE;
	}

	buf[totalSize] = 0;
	JL_CHK( JL_NewBlob(cx, buf, totalSize, JL_RVAL) );
	JL_updateMallocCounter(cx, totalSize);
	JSObject *blobObj;
	JL_CHK( JS_ValueToObject(cx, *JL_RVAL, &blobObj) );
	JL_CHKM( blobObj != NULL, E_STR("Blob"), E_CREATE );
	*JL_RVAL = OBJECT_TO_JSVAL(blobObj);

	JL_CHK(JL_SetProperty(cx, blobObj, "bits", pv->bits) ); // bits per sample
	JL_CHK(JL_SetProperty(cx, blobObj, "rate", pv->ofInfo->rate) ); // samples per second
	JL_CHK(JL_SetProperty(cx, blobObj, "channels", pv->ofInfo->channels) ); // 1:mono, 2:stereo
	JL_CHK(JL_SetProperty(cx, blobObj, "frames", totalSize / (pv->ofInfo->channels * pv->bits / 8) ) );

	return JS_TRUE;
bad:
	jl_free(buf);
	jl_free(buffer);
	return JS_FALSE;
}

/**doc
=== Properties ===
**/

/**doc
$TOC_MEMBER $INAME
 object $INAME $READONLY
  Is the stream object where encoded audio data are read from.
**/
DEFINE_PROPERTY_GETTER( inputStream ) {

	JL_USE(id);

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JL_CHK( JL_GetReservedSlot(cx, obj, SLOT_INPUT_STREAM, vp) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  Is the number of bits per frame and per channel.
**/
DEFINE_PROPERTY_GETTER( bits ) {

	JL_USE(id);

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	*vp = INT_TO_JSVAL( pv->bits );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  Is the number of frames per seconds of the sound.
**/
DEFINE_PROPERTY_GETTER( rate ) {

	JL_USE(id);

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	*vp = INT_TO_JSVAL( pv->ofInfo->rate );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  Is the number of channels of the sound. 1 is mono, 2 is stereo.
**/
DEFINE_PROPERTY_GETTER( channels ) {

	JL_USE(id);

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
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
DEFINE_PROPERTY_GETTER( frames ) {

	JL_USE(id);

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ogg_int64_t pcmTotal = ov_pcm_total(&pv->ofDescriptor, -1);
	if ( pcmTotal == OV_EINVAL ) { // if the stream is not seekable (we can't know the length) or only partially open.

		*vp = JSVAL_VOID;
		return JS_TRUE;
	}
//	size_t frames = pcmTotal / pv->ofInfo->channels; // (TBD) ???
	return JL_NativeToJsval(cx, pcmTotal, vp); // *vp = INT_TO_JSVAL( pcmTotal );
	JL_BAD;
}



CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION( Read )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ( inputStream )
		PROPERTY_READ( bits )
		PROPERTY_READ( rate )
		PROPERTY_READ( channels )
		PROPERTY_READ( frames )
	END_PROPERTY_SPEC

	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1)

END_CLASS
