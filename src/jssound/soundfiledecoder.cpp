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

#include <sndfile.h>

#undef malloc
#undef calloc
#undef realloc
#undef free



#define SLOT_INPUT_STREAM 0

/**doc fileIndex:top
$CLASS_HEADER
$SVN_REVISION $Revision$
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


sf_count_t SfGetFilelen(void *user_data) {

	Private *pv = (Private*)user_data;

	jsval tmpVal;
	int position, available;

	ASSERT( pv->cx );
	JL_CHK( JS_GetPropertyById(pv->cx, pv->streamObject, JLID(pv->cx, position), &tmpVal) );
	if ( JSVAL_IS_VOID( tmpVal ) )
		return -1;
	JL_CHK( JL_JsvalToNative(pv->cx, tmpVal, &position) );
	JL_CHK( JS_GetPropertyById(pv->cx, pv->streamObject, JLID(pv->cx, available), &tmpVal) );
	if ( JSVAL_IS_VOID( tmpVal ) )
		return -1;
	JL_CHK( JL_JsvalToNative(pv->cx, tmpVal, &available) );
	return position + available;
bad:
	return -1;
}


sf_count_t SfSeek(sf_count_t offset, int whence, void *user_data) {

	Private *pv = (Private*)user_data;

	jsval tmpVal;
	int position, available;

	ASSERT( pv->cx );
	switch ( whence ) {
		case SEEK_SET:
			if ( offset < 0 )
				return -1;
			JL_CHK( JL_NativeToJsval(pv->cx, offset, &tmpVal) );
			JL_CHK( JS_SetPropertyById(pv->cx, pv->streamObject, JLID(pv->cx, position), &tmpVal) );
			return 0;

		case SEEK_CUR:
			JL_CHK( JS_GetPropertyById(pv->cx, pv->streamObject, JLID(pv->cx, position), &tmpVal) );
			if ( JSVAL_IS_VOID( tmpVal ) )
				return -1;
			JL_CHK( JL_JsvalToNative(pv->cx, tmpVal, &position) );

			position += int(offset);
			JL_CHK( JL_NativeToJsval(pv->cx, position, &tmpVal) );
			JL_CHK( JS_SetPropertyById(pv->cx, pv->streamObject, JLID(pv->cx, position), &tmpVal) );
			return 0;

		case SEEK_END:
			JL_CHK( JS_GetPropertyById(pv->cx, pv->streamObject, JLID(pv->cx, available), &tmpVal) );
			if ( JSVAL_IS_VOID( tmpVal ) )
				return -1;
			JL_CHK( JL_JsvalToNative(pv->cx, tmpVal, &available) );

			JL_CHK( JS_GetPropertyById(pv->cx, pv->streamObject, JLID(pv->cx, position), &tmpVal) );
			if ( JSVAL_IS_VOID( tmpVal ) )
				return -1;
			JL_CHK( JL_JsvalToNative(pv->cx, tmpVal, &position) );

			if ( offset > 0 || -offset > position + available )
				return -1;
			JL_CHK( JL_JsvalToNative(pv->cx, tmpVal, &position) );
			JL_CHK( JL_NativeToJsval(pv->cx, position + available + offset, &tmpVal) ); // the pointer is set to the size of the file plus offset.
			JL_CHK( JS_SetPropertyById(pv->cx, pv->streamObject, JLID(pv->cx, position), &tmpVal) );
			return 0;
	}
bad:
	return -1;
}

sf_count_t SfTell(void *user_data) {

	Private *pv = (Private*)user_data;
	
	jsval tmpVal;
	int position;

	ASSERT( pv->cx );
	JL_CHK( JS_GetPropertyById(pv->cx, pv->streamObject, JLID(pv->cx, position), &tmpVal) );
	if ( JSVAL_IS_VOID( tmpVal ) )
		return -1;
	JL_CHK( JL_JsvalToNative(pv->cx, tmpVal, &position) );
	return position;
bad:
	return -1;
}

sf_count_t SfRead(void *ptr, sf_count_t count, void *user_data) {

	Private *pv = (Private*)user_data;
	size_t amount = (size_t)count;

	ASSERT(pv->cx);

	NIStreamRead read = StreamReadInterface(pv->cx, pv->streamObject);
	JL_CHK( read );
	JL_CHK( read(pv->cx, pv->streamObject, (char*)ptr, &amount) );
	return amount;
bad:
	return -1;
}

static SF_VIRTUAL_IO sfCallbacks = { SfGetFilelen, SfSeek, SfRead, 0, SfTell };


DEFINE_FINALIZE() {

	if ( JL_GetHostPrivate(cx)->canSkipCleanup )
		return;

	Private *pv = (Private*)JL_GetPrivate(obj);
	if ( !pv )
		return;

	sf_close(pv->sfDescriptor);
	JS_free(cx, pv);
}

/**doc
$TOC_MEMBER $INAME
 $INAME( stream )
  Creates a new SoundFileDecoder object. Only seekable streams are supported.
  $H arguments
   $ARG streamObject stream: is the data stream from where encoded data are read from.
  $H example
  {{{
  loadModule('jsstd');
  loadModule('jsio');
  loadModule('jssound');
  var file = new File('41_30secOgg-q0.wav'); // file: http://xiph.org/vorbis/listen.html
  file.open('r');
  var dec = new SoundFileDecoder( file );
  print( dec.bits, '\n' );
  print( dec.channels, '\n' );
  print( dec.rate, '\n' );
  do {
   var block = dec.read(10000);
   print( 'frames: '+block.frames, '\n' );
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

	SF_INFO tmp; // = {0};
	memset(&tmp, 0, sizeof(SF_INFO));

	pv->sfInfo = tmp; // ok, memset does the same thing

	pv->cx = cx;
	pv->sfDescriptor = sf_open_virtual(&sfCallbacks, SFM_READ, &pv->sfInfo, pv);
	pv->bits = 16;

 	JL_CHK( !JL_IsExceptionPending(cx) ); // pv->sfDescriptor may be != NULL and JL_IsExceptionPending(cx) true
	
	//sf_strerror()

	JL_ASSERT( pv->sfDescriptor != NULL, E_ARG, E_NUM(1), E_INVALID ); // "Invalid stream."

	JL_ASSERT( sf_error(pv->sfDescriptor) == SF_ERR_NO_ERROR, E_LIB, E_STR("sndfile"), E_OPERATION, E_ERRNO(sf_error(pv->sfDescriptor)) );

	int subFormat = pv->sfInfo.format & SF_FORMAT_SUBMASK;
	if ( subFormat == SF_FORMAT_FLOAT || subFormat == SF_FORMAT_DOUBLE ) {

		// require the whole file to be read
		int result = sf_command(pv->sfDescriptor, SFC_SET_SCALE_FLOAT_INT_READ, NULL, SF_TRUE); // Doc. Set/clear the scale factor when integer (short/int) data is read from a file containing floating point data. (http://www.mega-nerd.com/libsndfile/api.html#note2)
		JL_IGNORE(result);
	}

	pv->cx = NULL; // see definition
	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Methods ===
**/


// doc:
//   Unless the end of the file was reached during the read, the return value should equal the number of items requested.


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
  var file = new File('41_30secOgg-q0.wav'); // file: http://xiph.org/vorbis/listen.html
  file.open('r');
  var dec = new SoundFileDecoder( file );
  var block = dec.read(10000);
  print( 'rezolution: '+block.bits+' per channel', '\n' );
  print( block.channels == 2 ? 'stereo' : 'mono', '\n' );
  print( block.rate+' frames/seconds', '\n' );
  print( 'time: '+(block.frames/block.rate)+' seconds', '\n' );
  }}}
**/
DEFINE_FUNCTION( read ) {

	uint8_t *buf = NULL;

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	JL_CHKM( pv->sfInfo.channels == 1 || pv->sfInfo.channels == 2, E_NUM(pv->sfInfo.channels), E_STR("channels"), E_FORMAT );
	JL_CHKM( pv->bits == 8 || pv->bits == 16, E_NUM(pv->bits), E_STR("bit"), E_FORMAT );

//	JL_CHKM( pv->sfInfo.format & SF_FORMAT_PCM_16 || pv->sfInfo.bits & SF_FORMAT_PCM_S8, E_NUM(???), E_STR("bit"), E_FORMAT );


	int32_t frames;
	if ( JL_ARG_ISDEF(1) ) {

		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &frames) );
		if ( frames <= 0 ) {

			// like Descriptor::read, returns an empty audio object even if EOF
			JL_CHK( JL_NewByteAudioObjectOwner(cx, NULL, pv->bits, pv->sfInfo.channels, 0, pv->sfInfo.samplerate, JL_RVAL) );
			return JS_TRUE;
		}
	} else {

		sf_count_t total = pv->sfInfo.frames;
		if ( total == SF_COUNT_MAX ) {
		
			frames = -1; // mean unknown
		} else
		if ( total == 0 ) {
		
			*JL_RVAL = JSVAL_VOID; // EOF
			return JS_TRUE;
		} else {

			frames = (int32_t)total;
		}
	}

	pv->cx = cx;
	if ( frames != -1 ) {

		size_t totalSize = 0;

		size_t amount = frames * pv->sfInfo.channels * pv->bits/8; // amount in bytes
		buf = (uint8_t*)jl_malloc(amount);
		JL_ASSERT_ALLOC(buf);
		sf_count_t items = sf_read_short(pv->sfDescriptor, (short*)buf, amount / sizeof(short));
		ASSERT( items >= 0 );

		if ( items == 0 ) {
		
			JL_CHK( !JL_IsExceptionPending(cx) );
			JL_CHKM( sf_error(pv->sfDescriptor) == SF_ERR_NO_ERROR, E_STR("stream"), E_INVALID );
		}

		totalSize = size_t(items) * sizeof(short);

		if ( totalSize == 0 ) {

			jl_free(buf);
			*JL_RVAL = JSVAL_VOID;
		} else {

			if ( JL_MaybeRealloc(amount, totalSize) )
				buf = (uint8_t*)jl_realloc(buf, totalSize);
			JL_CHK( JL_NewByteAudioObjectOwner(cx, buf, pv->bits, pv->sfInfo.channels, totalSize / (pv->sfInfo.channels * pv->bits / 8), pv->sfInfo.samplerate, JL_RVAL) );
		}

	} else {

		sf_count_t items;
		jl::Buf<uint8_t> buffer;
		pv->cx = cx;
		do {
			
			buffer.Reserve(8192);
			items = sf_read_short(pv->sfDescriptor, (short*)buffer.Ptr(), buffer.PtrLength() / sizeof(short)); // bits per sample
			ASSERT( items >= 0 );
			if ( items == 0 ) {
			
				JL_CHK( !JL_IsExceptionPending(cx) );
				JL_CHKM( sf_error(pv->sfDescriptor) == SF_ERR_NO_ERROR, E_STR("stream"), E_INVALID ); // E_LIB, E_STR("sndfile"), E_OPERATION, E_ERRNO(sf_error(pv->sfDescriptor)
			}
			buffer.Advance((size_t)items * sizeof(short));

		} while ( items > 0 );

		if ( buffer.Length() )
			JL_CHK( JL_NewByteAudioObjectOwner(cx, buffer.GetDataOwnership(), pv->bits, pv->sfInfo.channels, buffer.Length() / (pv->sfInfo.channels * pv->bits/8), pv->sfInfo.samplerate, JL_RVAL) );
		else
			*JL_RVAL = JSVAL_VOID;
	}
	pv->cx = NULL;

	return JS_TRUE;
bad:
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

	JL_IGNORE(id);
	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
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

	JL_IGNORE(id);
	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	*vp = INT_TO_JSVAL( pv->sfInfo.samplerate );
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
	*vp = INT_TO_JSVAL( pv->sfInfo.channels );
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
	*vp = INT_TO_JSVAL( size_t(pv->sfInfo.frames) );
	return JS_TRUE;
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
