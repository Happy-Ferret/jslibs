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

#include	<sndfile.h>

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
	int position;
	JS_GetProperty(pv->cx, pv->streamObject, "position", &tmpVal); // (TBD) manage error
	if ( JSVAL_IS_VOID( tmpVal ) )
		return -1;
	JL_JsvalToNative(pv->cx, tmpVal, &position); // (TBD) manage error

	int available;
	JS_GetProperty(pv->cx, pv->streamObject, "available", &tmpVal); // (TBD) manage error
	if ( JSVAL_IS_VOID( tmpVal ) )
		return -1;
	JL_JsvalToNative(pv->cx, tmpVal, &available); // (TBD) manage error

	return position + available;
}


sf_count_t SfSeek(sf_count_t offset, int whence, void *user_data) {

	Private *pv = (Private*)user_data;

	jsval tmpVal;
	int position, available;

	switch (whence) {
		case SEEK_SET:
			if ( offset < 0 )
				return -1;
			JL_NativeToJsval(pv->cx, offset, &tmpVal); // (TBD) manage error
			JS_SetProperty(pv->cx, pv->streamObject, "position", &tmpVal); // (TBD) manage error
			return 0;

		case SEEK_CUR:
			JS_GetProperty(pv->cx, pv->streamObject, "position", &tmpVal); // (TBD) manage error
			if ( JSVAL_IS_VOID( tmpVal ) )
				return -1;
			JL_JsvalToNative(pv->cx, tmpVal, &position); // (TBD) manage error

			position += int(offset);
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
	return -1;
}

sf_count_t SfTell(void *user_data) {

	Private *pv = (Private*)user_data;
	jsval tmpVal;

	int position;
	JS_GetProperty(pv->cx, pv->streamObject, "position", &tmpVal);
	if ( JSVAL_IS_VOID( tmpVal ) )
		return -1;
	JL_JsvalToNative(pv->cx, tmpVal, &position);

	return position;
}

sf_count_t SfRead(void *ptr, sf_count_t count, void *user_data) {

	Private *pv = (Private*)user_data;

	size_t amount = size_t(count);
//	if ( pv->streamRead( pv->cx, pv->obj, (char*)ptr, &amount ) != JS_TRUE )
//		return -1; // (TBD) find a better error
	if ( StreamReadInterface( pv->cx, pv->streamObject)( pv->cx, pv->streamObject, (char*)ptr, &amount ) != JS_TRUE )
		return -1; // (TBD) find a better error
	return amount;
}

static SF_VIRTUAL_IO sfCallbacks = { SfGetFilelen, SfSeek, SfRead, 0, SfTell };


DEFINE_FINALIZE() {

	if ( JL_GetHostPrivate(cx)->canSkipCleanup )
		return;

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
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

	if ( JL_IsExceptionPending(cx) )
		return JS_FALSE;

	// (TBD) manage sf_error_str()

	JL_ASSERT( sf_error(pv->sfDescriptor) == SF_ERR_NO_ERROR, E_LIB, E_STR("sndfile"), E_OPERATION, E_ERRNO(sf_error(pv->sfDescriptor)) );
	JL_ASSERT( pv->sfDescriptor != NULL, E_ARG, E_NUM(1), E_INVALID ); // "Invalid stream."


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
**/
DEFINE_FUNCTION( Read ) {

	char *buf = NULL;
	char *buffer = NULL;

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	JL_CHKM( pv->sfInfo.channels == 1 || pv->sfInfo.channels == 2, E_NUM(pv->sfInfo.channels), E_STR("channels"), E_FORMAT );
//	JL_CHKM( pv->sfInfo.format & SF_FORMAT_PCM_16 || pv->sfInfo.bits & SF_FORMAT_PCM_S8, E_NUM(???), E_STR("bit"), E_FORMAT );


	long totalSize = 0;

	pv->cx = cx;
	if ( JL_ARG_ISDEF(1) ) {

		size_t frames;
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &frames) );

		if ( frames > 0 ) {

			size_t amount = frames * pv->sfInfo.channels * pv->bits/8; // amount in bytes
			buf = (char*)jl_malloc(amount +1);
			JL_ASSERT_ALLOC(buf);

			sf_count_t items = sf_read_short(pv->sfDescriptor, (short*)buf, amount/sizeof(short));

			if ( JL_IsExceptionPending(cx) )
				return JS_FALSE;

			totalSize = size_t(items) * sizeof(short);

			if ( JL_MaybeRealloc(amount, totalSize) )
				buf = (char*)jl_realloc(buf, totalSize +1);
		} else {

			JL_ERR( E_ARG, E_NUM(1), E_MIN, E_NUM(1) );
		}

	} else {

		void *stack;
		jl::StackInit(&stack);

		int bufferSize = 16384 - 16; // try to alloc less than one page

		sf_count_t items;
		do {

			buffer = (char*)jl_malloc(bufferSize);
			JL_CHK( buffer );
			jl::StackPush(&stack, buffer);

			char *data = buffer+sizeof(int);
			int *len = (int*)buffer;
			int maxlen = bufferSize - sizeof(int);

			// pv->bits: ???=8, sf_read_short=16, sf_read_int=32
			items = sf_read_short(pv->sfDescriptor, (short*)data, maxlen/sizeof(short)); // bits per sample

			if ( JL_IsExceptionPending(cx) )
				return JS_FALSE;

			JL_ASSERT( sf_error(pv->sfDescriptor) == SF_ERR_NO_ERROR, E_LIB, E_STR("sndfile"), E_OPERATION, E_ERRNO(sf_error(pv->sfDescriptor)) );

			if ( items <= 0 ) { // 0 indicates EOF

				*len = 0;
			} else {

				*len = int(items) * sizeof(short);
				totalSize += int(items) * sizeof(short);
			}

		} while (items > 0);

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
	JL_CHK(JL_SetProperty(cx, blobObj, "rate", pv->sfInfo.samplerate) ); // samples per second
	JL_CHK(JL_SetProperty(cx, blobObj, "channels", pv->sfInfo.channels) ); // 1:mono, 2:stereo
	JL_CHK(JL_SetProperty(cx, blobObj, "frames", totalSize / (pv->sfInfo.channels * pv->bits / 8) ) );

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

	JL_IGNORE(id);
	JL_ASSERT_THIS_INSTANCE();

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

	JL_IGNORE(id);
	JL_ASSERT_THIS_INSTANCE();

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

	JL_IGNORE(id);
	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
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

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
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

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	*vp = INT_TO_JSVAL( size_t(pv->sfInfo.frames) );
	return JS_TRUE;
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
		PROPERTY_GETTER( inputStream )
		PROPERTY_GETTER( bits )
		PROPERTY_GETTER( rate )
		PROPERTY_GETTER( channels )
		PROPERTY_GETTER( frames )
	END_PROPERTY_SPEC

	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1)

END_CLASS
