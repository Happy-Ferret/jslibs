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
#include <buffer.h>

#define malloc jl_malloc_fct
#define calloc jl_calloc_fct
#define realloc jl_realloc_fct
#define free jl_free_fct

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4244 ) // 'argument' : conversion from 'xxx' to 'yyy', possible loss of data
#endif // _MSC_VER

#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#ifdef _MSC_VER
#pragma warning( pop )
#endif // _MSC_VER


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


int seek_func(void *datasource, ogg_int64_t offset, int whence) {

	Private *pv = (Private*)datasource;
	ASSERT(pv->cx);

	JS::RootedValue tmpVal(pv->cx);
	int64_t position;
	int available;

	switch (whence) {
		case SEEK_SET:
			if ( offset < 0 )
				return -1;
			JL_CHK( JL_NativeToProperty(pv->cx, pv->streamObject, JLID(pv->cx, position), offset) );

//			JL_CHK( JL_NativeToJsval(pv->cx, offset, &tmpVal) ); // (TBD) manage error
//			JL_CHK( JS_SetProperty(pv->cx, pv->streamObject, JLID(pv->cx, position), &tmpVal) ); // (TBD) manage error
			return 0;

		case SEEK_CUR:
			JL_CHK( JS_GetPropertyById(pv->cx, pv->streamObject, JLID(pv->cx, position), tmpVal.address()) ); // (TBD) manage error
			if ( JSVAL_IS_VOID( tmpVal ) ) //
				return -1;
			if ( offset == 0 ) // no move, just tested, but let -1 to be return if no position property available.
				return 0;
			JL_CHK( JL_JsvalToNative(pv->cx, tmpVal, &position) ); // (TBD) manage error
			position += offset;
			JL_CHK( JL_NativeToJsval(pv->cx, position, tmpVal) ); // (TBD) manage error
			JL_CHK( JS_SetPropertyById(pv->cx, pv->streamObject, JLID(pv->cx, position), tmpVal.address()) ); // (TBD) manage error
			return 0;

		case SEEK_END:
			JL_CHK( JS_GetPropertyById(pv->cx, pv->streamObject, JLID(pv->cx, available), tmpVal.address()) );
			if ( JSVAL_IS_VOID( tmpVal ) )
				return -1;
			JL_CHK( JL_JsvalToNative(pv->cx, tmpVal, &available) );

			JL_CHK( JS_GetPropertyById(pv->cx, pv->streamObject, JLID(pv->cx, position), tmpVal.address()) );
			if ( JSVAL_IS_VOID( tmpVal ) )
				return -1;
			JL_CHK( JL_JsvalToNative(pv->cx, tmpVal, &position) );

			if ( offset > 0 || -offset > position + available )
				return -1;
			JL_CHK( JL_JsvalToNative(pv->cx, tmpVal, &position) );
			JL_CHK( JL_NativeToJsval(pv->cx, position + available + offset, tmpVal) ); // the pointer is set to the size of the file plus offset.
			JL_CHK( JS_SetPropertyById(pv->cx, pv->streamObject, JLID(pv->cx, position), tmpVal.address()) );
			return 0;
	}

bad:
	return -1; // doc: you *MUST* return -1 if the stream is unseekable
}


long tell_func(void *datasource) {

	Private *pv = (Private*)datasource;
	jsval tmpVal;
	int position;
	ASSERT(pv->cx);
	JL_CHK( JS_GetPropertyById(pv->cx, pv->streamObject, JLID(pv->cx, position), &tmpVal) );
	if ( JSVAL_IS_VOID( tmpVal ) )
		return -1;
	JL_CHK( JL_JsvalToNative(pv->cx, tmpVal, &position) );
	return position;
bad:
	return -1;
}


size_t read_func( void *ptr, size_t size, size_t nmemb, void *privateData ) {

	Private *pv = (Private*)privateData;
	ASSERT(pv->cx);
	size_t amount = size * nmemb;

	NIStreamRead read = StreamReadInterface(pv->cx, pv->streamObject);
	JL_CHK( read );
	JL_CHK( read(pv->cx, pv->streamObject, (char*)ptr, &amount) );
	return amount;
bad:
	errno = 1;
	return 0;
}

// doc: http://xiph.org/vorbis/doc/vorbisfile/ov_callbacks.html
static const ov_callbacks ovCallbacks = { read_func, seek_func, 0, tell_func };


DEFINE_FINALIZE() {

	if ( JL_GetHostPrivate(fop->runtime())->canSkipCleanup )
		return;

	Private *pv = (Private*)JL_GetPrivate(obj);
	if ( pv != NULL ) {

		ov_clear(&pv->ofDescriptor); // beware: info must be valid
		JS_freeop(fop, pv);
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
  loadModule('jsstd');
  loadModule('jsio');
  loadModule('jssound');
  var file = new File('41_30secOgg-q0.ogg'); // file: http://xiph.org/vorbis/listen.html
  file.open('r');
  var dec = new OggVorbisDecoder( file );
  print( dec.bits, '\n' );
  print( dec.channels, '\n' );
  print( dec.rate, '\n' );
  do {
   var block = dec.read(10000);
   print( 'frames: ' + block.frames, '\n' );
  } while(block);
  }}}
**/
DEFINE_CONSTRUCTOR() {

	Private *pv = NULL;

	JL_DEFINE_ARGS;
	JL_DEFINE_CONSTRUCTOR_OBJ;
	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT_ARG_IS_OBJECT(1);
	JL_ASSERT_CONSTRUCTING();

	pv = (Private*)JS_malloc(cx, sizeof(Private));
	JL_CHK( pv );
	pv->ofInfo = NULL;

	JL_CHK( JL_SetReservedSlot( obj, SLOT_INPUT_STREAM, JL_ARG(1)) );
	pv->streamObject = JSVAL_TO_OBJECT(JL_ARG(1));

	pv->cx = cx;
	int result = ov_open_callbacks(pv, &pv->ofDescriptor, NULL, 0, ovCallbacks); // doc: 0 for success
	if ( result != 0 ) {

		JL_CHK( !JL_IsExceptionPending(cx) ); // error is already set.
		JL_ERR( E_ARG, E_NUM(1), E_INVALID, E_COMMENT("ogg vorbis descriptor") );
	}

	pv->ofInfo = ov_info(&pv->ofDescriptor, -1); // doc: returns NULL if the specified bitstream does not exist or the file has been initialized improperly.
	if ( pv->ofInfo == NULL ) {

		JL_CHK( !JL_IsExceptionPending(cx) ); // error is already set.
		JL_ERR( E_ARG, E_NUM(1), E_INVALID, E_COMMENT("ogg vorbis info") );
	}

	pv->bits = 16;

	pv->cx = NULL; // see definition

	JL_SetPrivate(obj, pv);
	return JS_TRUE;

bad:
	if ( pv ) {

		if ( pv->ofInfo )
			ov_clear(&pv->ofDescriptor); // beware: info must be valid
		jl_free(pv);
	}
	return JS_FALSE;
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
  loadModule('jsstd');
  loadModule('jsio');
  loadModule('jssound');
  var file = new File('41_30secOgg-q0.ogg'); // file: http://xiph.org/vorbis/listen.html
  file.open('r');
  var dec = new OggVorbisDecoder( file );
  var block = dec.read(10000);
  print( 'resolution: '+block.bits+' per channel', '\n' );
  print( block.channels == 2 ? 'stereo' : 'mono', '\n' );
  print( block.rate+' frames/seconds', '\n' );
  print( 'time: '+(block.frames/block.rate)+' seconds', '\n' );
  }}}
**/
DEFINE_FUNCTION( read ) {

	uint8_t *buf = NULL;

	JL_DEFINE_ARGS;
	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	JL_CHKM( pv->ofInfo->channels == 1 || pv->ofInfo->channels == 2, E_NUM(pv->ofInfo->channels), E_STR("channels"), E_FORMAT );
	JL_CHKM( pv->bits == 8 || pv->bits == 16, E_NUM(pv->bits), E_STR("bit"), E_FORMAT );

	int32_t frames;
	if ( JL_ARG_ISDEF(1) ) {

		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &frames) );
		if ( frames <= 0 ) {

			// like Descriptor::read, returns an empty audio object even if EOF
			JL_CHK( JL_NewByteAudioObjectOwner(cx, NULL, pv->bits, pv->ofInfo->channels, 0, pv->ofInfo->rate, *JL_RVAL) );
			return JS_TRUE;
		}
	} else {

		ogg_int64_t total = ov_pcm_total(&pv->ofDescriptor, 0); // logical bitstream 0
		if ( total == OV_EINVAL ) {
		
			frames = -1; // mean unknown
		} else
		if ( total == 0 ) {
		
			*JL_RVAL = JSVAL_VOID; // EOF
			return JS_TRUE;
		} else {
		
			frames = (int32_t)total;
		}
	}


	if ( frames != -1 ) {

		size_t totalSize = 0;

		size_t amount = frames * pv->ofInfo->channels * (pv->bits/8); // amount in bytes
		buf = (uint8_t*)jl_malloc(amount);
		JL_ASSERT_ALLOC(buf);

		long bytes;
		int bitStream = 0;
		pv->cx = cx;
		do {
			int prevBitstream = bitStream;
			bytes = ov_read(&pv->ofDescriptor, (char*)buf + totalSize, amount - totalSize, 0, pv->bits/8, 1, &bitStream);
			JL_CHKM( bitStream == prevBitstream, E_STR("bitstream"), E_INVALID ); // bitstream has unexpectedly changed
			// (TBD) update the channels, rate, ... according to: ov_info(&pv->ofDescriptor, bitStream); ?
			if ( bytes < 0 ) {
				
				JL_CHK( !JL_IsExceptionPending(cx) );
				if ( bytes == OV_HOLE )
					continue; // ignore corrupted/dropped/lost parts
				JL_ERR( E_STR("stream"), E_INVALID );
				// doc: OV_EINVAL indicates the initial file headers couldn't be read or are corrupt, or that the initial open call for vf failed.
				// doc: OV_EBADLINK indicates that an invalid stream section was supplied to libvorbisfile, or the requested link is corrupt.
			}
			totalSize += bytes;

		} while ( bytes > 0 && totalSize < amount );
		
		pv->cx = NULL;

		if ( totalSize == 0 ) {

			jl_free(buf);
			*JL_RVAL = JSVAL_VOID;
		} else {

			if ( JL_MaybeRealloc(amount, totalSize) )
				buf = (uint8_t*)jl_realloc(buf, totalSize);
			JL_CHK( JL_NewByteAudioObjectOwner(cx, buf, pv->bits, pv->ofInfo->channels, totalSize / (pv->bits/8 * pv->ofInfo->channels), pv->ofInfo->rate, *JL_RVAL) );
		}

	} else {

		long bytes;
		jl::Buf<char> buffer;
		int bitStream = 0;
		pv->cx = cx;
		do {
			int prevBitstream = bitStream;
			buffer.Reserve(8192);
			bytes = ov_read(&pv->ofDescriptor, buffer.Ptr(), buffer.PtrLength(), 0, pv->bits/8, 1, &bitStream);
			JL_CHKM( bitStream == prevBitstream,  E_STR("bitstream"), E_INVALID ); // bitstream has unexpectedly changed
			JL_CHK( bytes > 0 || !JL_IsExceptionPending(cx) );

			if ( bytes < 0 ) {
				
				if ( bytes == OV_HOLE )
					continue; // ignore corrupted/dropped/lost parts
				JL_ERR( E_STR("stream"), E_INVALID );
				// doc: OV_EINVAL indicates the initial file headers couldn't be read or are corrupt, or that the initial open call for vf failed.
				// doc: OV_EBADLINK indicates that an invalid stream section was supplied to libvorbisfile, or the requested link is corrupt.
			}

			buffer.Advance(bytes);

		} while ( bytes > 0 ); // 0 indicates EOF
		pv->cx = NULL;

		if ( buffer.Length() )
			JL_CHK( JL_NewByteAudioObjectOwner(cx, (uint8_t*)buffer.GetDataOwnership(), pv->bits, pv->ofInfo->channels, buffer.Length() / (pv->ofInfo->channels * pv->bits/8), pv->ofInfo->rate, *JL_RVAL) );
		else
			*JL_RVAL = JSVAL_VOID;
	}

	return JS_TRUE;
bad:
	if ( buf )
		jl_free(buf);
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

	JL_IGNORE(id);
	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JL_CHK( JL_GetReservedSlot( obj, SLOT_INPUT_STREAM, vp) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  Is the number of bits per frame and per channel.
**/
DEFINE_PROPERTY_GETTER( bits ) {

	JL_IGNORE(id);
	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	vp.setInt32( pv->bits );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  Is the number of frames per seconds of the sound.
**/
DEFINE_PROPERTY_GETTER( rate ) {

	JL_IGNORE(id);
	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	vp.setInt32( pv->ofInfo->rate );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  Is the number of channels of the sound. 1 is mono, 2 is stereo.
**/
DEFINE_PROPERTY_GETTER( channels ) {

	JL_IGNORE(id);
	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	vp.setInt32( pv->ofInfo->channels );
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

	JL_IGNORE(id);
	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ogg_int64_t pcmTotal = ov_pcm_total(&pv->ofDescriptor, -1);
	if ( pcmTotal == OV_EINVAL ) { // if the stream is not seekable (we can't know the length) or only partially open.

		vp.setUndefined();
		return JS_TRUE;
	}
//	size_t frames = pcmTotal / pv->ofInfo->channels; // (TBD) ???
	return JL_NativeToJsval(cx, pcmTotal, vp); // *vp = INT_TO_JSVAL( pcmTotal );
	JL_BAD;
}



CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision$"))
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1)

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION( read )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_GETTER( inputStream )
		PROPERTY_GETTER( bits )
		PROPERTY_GETTER( rate )
		PROPERTY_GETTER( channels )
		PROPERTY_GETTER( frames )
	END_PROPERTY_SPEC

END_CLASS
