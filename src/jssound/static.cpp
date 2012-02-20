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

#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include	<sndfile.h>

DECLARE_STATIC()

// doc. For seek_func(), you *MUST* return -1 if the stream is unseekable

typedef struct {
	JSContext *cx;
	JSObject *obj;
//	NIStreamRead streamRead;
} StreamReadInfo;


/**doc fileIndex:topmost **/

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_STATIC

size_t readStream( void *ptr, size_t size, size_t nmemb, void *pv ) {

	StreamReadInfo *info = (StreamReadInfo*)pv;
	size_t amount = size * nmemb;
//	if ( info->streamRead( info->cx, info->obj, (char*)ptr, &amount ) != JS_TRUE )
//		return -1; // (TBD) check for a better error
	if ( StreamReadInterface(info->cx, info->obj)(info->cx, info->obj, (char*)ptr, &amount) != JS_TRUE )
		return (size_t)-1; // (TBD) check for a better error

	return amount;
}

static const ov_callbacks ovCallbacks = { readStream,0,0,0 };


/**   doc
$TOC_MEMBER $INAME
 $TYPE soundObject $INAME( stream ) $DEPRECATED
  Decodes a ogg vorbis sample to a sound object.
  $H arguments
   $ARG streamObject stream: any object that has a Read function or that supports the NIStreamRead Native Interface ( file, socket, new Stream(buffer), ... ). For further details about stream objects, see jslang::Stream object and NativeInterface mechanism.
  $H return value
   A sound object in a 16-bit per sample format.
  $H example
  {{{
  loadModule('jsstd');
  loadModule('jsio');
  loadModule('jssound');

  var file = new File('41_30secOgg-q0.ogg');
  var buffer = file.content;
  var stream = new Stream(buffer);
  var mySoundObject = decodeOggVorbis(stream);
  print('sample length: ', mySoundObject.length, '\n');
  }}}
**/

DEFINE_FUNCTION( decodeOggVorbis ) {

	JL_ASSERT_ARGC_MIN( 1 );
	JL_ASSERT_ARG_IS_OBJECT(1);
	JSObject *streamObj = JSVAL_TO_OBJECT( JL_ARG(1) );

//	NIStreamRead streamReader;
//	JL_CHK( GetStreamReadInterface(cx, StreamObj, &streamReader) );
//	JL_ASSERT( streamReader != NULL, "Invalid stream." );

	StreamReadInfo pv; // = { cx, StreamObj };
	pv.cx = cx;
	pv.obj = streamObj;

	OggVorbis_File descriptor;
	ov_open_callbacks(&pv, &descriptor, NULL, 0, ovCallbacks);

	// (TBD) manage errors

	vorbis_info *info = ov_info(&descriptor, -1);
	int bits = 16;

	JL_CHKM( bits == 8 || bits == 16, E_NUM(bits), E_STR("bit"), E_FORMAT );
	JL_CHKM( info->channels == 1 || info->channels == 2, E_NUM(info->channels), E_STR("channels"), E_FORMAT );

	int bitStream;
	void *stack;
	jl::StackInit(&stack);

	int bufferSize = 4096 - 16; // try to alloc less than one page

	long totalSize = 0;
	long bytes;
	do {

		char *buffer = (char*)jl_malloc(bufferSize);
		JL_ASSERT_ALLOC(buffer);

		jl::StackPush(&stack, buffer);

		bytes = ov_read(&descriptor, buffer+sizeof(int), bufferSize-sizeof(int), 0, bits / 8, 1, &bitStream);

		if ( bytes < 0 ) { // 0 indicates EOF

			*(int*)buffer = 0;

			// (TBD) manage errors

			if ( bytes == OV_HOLE ) { // indicates there was an interruption in the data. (one of: garbage between pages, loss of sync followed by recapture, or a corrupt page)

			} else if ( bytes == OV_EINVAL ) { // doc. indicates that an invalid stream section was supplied to libvorbisfile, or the requested link is corrupt.

				break;
			}
		} else {

			*(int*)buffer = bytes;
			totalSize += bytes;
		}

	} while (bytes > 0);

	// convert data chunks into a single memory buffer.
	char *buf = (char*)jl_malloc(totalSize +1);
	JL_CHK( buf );

	buf[totalSize] = '\0';
	JL_CHK( JL_NewBlob(cx, buf, totalSize, JL_RVAL) );
	JL_updateMallocCounter(cx, totalSize);
	JSObject *bstrObj;
	JL_CHK( JS_ValueToObject(cx, *JL_RVAL, &bstrObj) );
	JL_CHKM( bstrObj != NULL, E_STR("Blob"), E_CREATE );
	*JL_RVAL = OBJECT_TO_JSVAL(bstrObj);

	JL_CHK(JL_SetProperty(cx, bstrObj, "bits", bits) ); // bits per sample
	JL_CHK(JL_SetProperty(cx, bstrObj, "rate", info->rate) );
	JL_CHK(JL_SetProperty(cx, bstrObj, "channels", info->channels) );

	ov_clear(&descriptor); // beware: info must be valid

	// because the stack is LIFO, we have to start from the end.
	buf += totalSize;
	while( !jl::StackIsEnd(&stack) ) {

		char *buffer = (char *)jl::StackPop(&stack);
		int size = *(int*)buffer;
		buf = buf - size;
		memcpy( buf, buffer+sizeof(int), size );
		jl_free(buffer);
	}

	return JS_TRUE;
	JL_BAD;
}



sf_count_t SfGetFilelen(void *user_data) {

	StreamReadInfo *pv = (StreamReadInfo *)user_data;
	jsval tmpVal;

	int position;
	JS_GetProperty(pv->cx, pv->obj, "position", &tmpVal); // (TBD) manage error
	if ( JSVAL_IS_VOID( tmpVal ) )
		return -1;
	JL_JsvalToNative(pv->cx, tmpVal, &position); // (TBD) manage error

	int available;
	JS_GetProperty(pv->cx, pv->obj, "available", &tmpVal); // (TBD) manage error
	if ( JSVAL_IS_VOID( tmpVal ) )
		return -1;
	JL_JsvalToNative(pv->cx, tmpVal, &available); // (TBD) manage error

	return position + available;
}


sf_count_t SfSeek(sf_count_t offset, int whence, void *user_data) {

	StreamReadInfo *pv = (StreamReadInfo *)user_data;

	jsval tmpVal;
	size_t position, available;

	switch (whence) {
		case SEEK_SET:
			if ( offset < 0 )
				return -1;
			JL_NativeToJsval(pv->cx, offset, &tmpVal); // (TBD) manage error
			JS_SetProperty(pv->cx, pv->obj, "position", &tmpVal); // (TBD) manage error
			return 0;

		case SEEK_CUR:
			JS_GetProperty(pv->cx, pv->obj, "position", &tmpVal); // (TBD) manage error
			if ( JSVAL_IS_VOID( tmpVal ) )
				return -1;
			JL_JsvalToNative(pv->cx, tmpVal, &position); // (TBD) manage error

			position += size_t(offset);
			JL_NativeToJsval(pv->cx, position, &tmpVal); // (TBD) manage error
			JS_SetProperty(pv->cx, pv->obj, "position", &tmpVal); // (TBD) manage error
			return 0;

		case SEEK_END:
			JS_GetProperty(pv->cx, pv->obj, "available", &tmpVal);
			if ( JSVAL_IS_VOID( tmpVal ) )
				return -1;
			JL_JsvalToNative(pv->cx, tmpVal, &available);

			JS_GetProperty(pv->cx, pv->obj, "position", &tmpVal);
			if ( JSVAL_IS_VOID( tmpVal ) )
				return -1;
			JL_JsvalToNative(pv->cx, tmpVal, &position);

			if ( offset > 0 || -offset > position + available )
				return -1;
			JL_JsvalToNative(pv->cx, tmpVal, &position);
			JL_NativeToJsval(pv->cx, position + available + offset, &tmpVal); // the pointer is set to the size of the file plus offset.
			JS_SetProperty(pv->cx, pv->obj, "position", &tmpVal);
			return 0;

	}
	return -1;
}

sf_count_t SfTell(void *user_data) {

	StreamReadInfo *pv = (StreamReadInfo *)user_data;
	jsval tmpVal;

	int position;
	JS_GetProperty(pv->cx, pv->obj, "position", &tmpVal);
	if ( JSVAL_IS_VOID( tmpVal ) )
		return -1;
	JL_JsvalToNative(pv->cx, tmpVal, &position);

	return position;
}

sf_count_t SfRead(void *ptr, sf_count_t count, void *user_data) {

	StreamReadInfo *pv = (StreamReadInfo *)user_data;

	size_t amount = size_t(count);
//	if ( pv->streamRead( pv->cx, pv->obj, (char*)ptr, &amount ) != JS_TRUE )
//		return -1; // (TBD) find a better error
	if ( StreamReadInterface( pv->cx, pv->obj)( pv->cx, pv->obj, (char*)ptr, &amount ) != JS_TRUE )
		return -1; // (TBD) find a better error
	return amount;
}


static SF_VIRTUAL_IO sfCallbacks = { SfGetFilelen, SfSeek, SfRead, 0, SfTell };


/**   doc
$TOC_MEMBER $INAME
 $TYPE soundObject $INAME( stream ) $DEPRECATED
  Decodes a sample from any supported sound format to a sound object.
  $H arguments
   $ARG streamObject stream: any object that has a Read function or that supports the NIStreamRead Native Interface ( file, socket, new Stream(buffer), ... ). For further details about stream objects, see jslang::Stream object and native interface mechanism.
  $H return value
   A sound object in a 16-bit per sample format.
**/
DEFINE_FUNCTION( decodeSound ) {

	JL_ASSERT_ARGC_MIN( 1 );

	JL_ASSERT_ARG_IS_OBJECT(1);
	JSObject *streamObj = JSVAL_TO_OBJECT( JL_ARG(1) );

//	NIStreamRead streamReader;
//	JL_CHK( GetStreamReadInterface(cx, StreamObj, &streamReader) );
//	JL_ASSERT( streamReader != NULL, "Invalid stream." );

	StreamReadInfo pv;// = { cx, streamObj };
	pv.cx = cx;
	pv.obj = streamObj;
	SF_INFO info;// = {0};
	memset(&info, 0, sizeof(SF_INFO));

	SNDFILE *descriptor = sf_open_virtual(&sfCallbacks, SFM_READ, &info, &pv);

	JL_ASSERT( sf_error(descriptor) == SF_ERR_NO_ERROR, E_LIB, E_STR("sndfile"), E_OPERATION, E_ERRNO(sf_error(descriptor)) );
	JL_ASSERT( descriptor != NULL, E_ARG, E_NUM(1), E_INVALID ); // "Invalid stream."

	if ( JL_IsExceptionPending(cx) )
		return JS_FALSE;

	JL_CHKM( info.channels == 1 || info.channels == 2, E_NUM(info.channels), E_STR("channels"), E_FORMAT );

	sf_command(descriptor, SFC_SET_SCALE_FLOAT_INT_READ, NULL, SF_TRUE); // Doc. Set/clear the scale factor when integer (short/int) data is read from a file containing floating point data.

	void *stack;
	jl::StackInit(&stack);

	int bufferSize = 16384 - 16; // try to alloc less than one page

	long totalSize = 0;
	sf_count_t items;
	do {

		char *buffer = (char*)jl_malloc(bufferSize);
		JL_ASSERT_ALLOC(buffer);
		jl::StackPush(&stack, buffer);

		char *data = buffer+sizeof(int);
		int *len = (int*)buffer;
		int maxlen = bufferSize - sizeof(int);

		items = sf_read_short(descriptor, (short*)data, maxlen/sizeof(short)); // bits per sample

		JL_ASSERT( sf_error(descriptor) == SF_ERR_NO_ERROR, E_LIB, E_STR("sndfile"), E_OPERATION, E_ERRNO(sf_error(descriptor)) );

		if ( items <= 0 ) { // 0 indicates EOF

			*len = 0;

		} else {

			*len = size_t(items) * sizeof(short);
			totalSize += size_t(items) * sizeof(short);
		}

	} while (items > 0);


	// convert data chunks into a single memory buffer.
	char *buf = (char*)jl_malloc(totalSize);
	JL_CHK( buf );

//	JSObject *bstrObj = JL_NewBlob(cx, buf, totalSize);
//	JL_ASSERT( bstrObj != NULL, "Unable to create the Blob object.");
//	*JL_RVAL = OBJECT_TO_JSVAL(bstrObj);

	buf[totalSize] = '\0';
	JL_CHK( JL_NewBlob(cx, buf, totalSize, JL_RVAL) );
	JL_updateMallocCounter(cx, totalSize);
	JSObject *bstrObj;
	JL_CHK( JS_ValueToObject(cx, *JL_RVAL, &bstrObj) );
	JL_CHKM( bstrObj != NULL, E_STR("Blob"), E_CREATE );

	*JL_RVAL = OBJECT_TO_JSVAL(bstrObj);

	JL_CHK(JL_SetProperty(cx, bstrObj, "bits", 16) ); // bits per sample
	JL_CHK(JL_SetProperty(cx, bstrObj, "rate", info.samplerate) );
	JL_CHK(JL_SetProperty(cx, bstrObj, "channels", info.channels) );

	sf_close(descriptor);

	// because the stack is LIFO, we have to start from the end.
	buf += totalSize;
	while( !jl::StackIsEnd(&stack) ) {

		char *buffer = (char *)jl::StackPop(&stack);
		int size = *(int*)buffer;
		buf = buf - size;
		memcpy( buf, buffer+sizeof(int), size );
		jl_free(buffer);
	}

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME( sound )
  Split channels of the _sound_ into an Array of monaural sound object.
  $H arguments
   $ARG soundObject sound
  $H return value
   An array that contains each individual channel of the sound.
**/
DEFINE_FUNCTION( splitChannels ) {

	JLStr bufStr;
	JL_ASSERT_ARGC_MIN( 1 );
	JL_ASSERT_ARG_IS_OBJECT(1);

	unsigned int rate, channelCount, bits, frames;
	JSObject *srcBlobObj = JSVAL_TO_OBJECT(JL_ARG(1));
	JL_CHK(JL_GetProperty(cx, srcBlobObj, "rate", &rate) );
	JL_CHK(JL_GetProperty(cx, srcBlobObj, "channels", &channelCount) );
	JL_CHK(JL_GetProperty(cx, srcBlobObj, "bits", &bits) );
	JL_CHK(JL_GetProperty(cx, srcBlobObj, "frames", &frames) );

	JL_ASSERT( bits == 8 || bits == 16, E_ARG, E_NUM(1), E_FORMAT, E_COMMENT_BEGIN, E_NUM(bits), E_STR("bit"), E_COMMENT_END );

	const char *srcBuf;
	size_t srcBufLength;
	//JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(1), &srcBuf, &srcBufLength) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &bufStr) );
	srcBufLength = bufStr.Length();
	srcBuf = bufStr.GetConstStr();

	JSObject *destArray = JS_NewArrayObject(cx, 0, NULL);
	*JL_RVAL = OBJECT_TO_JSVAL(destArray);

	for ( size_t c = 0; c < channelCount; c++ ) {

		size_t totalSize = frames * (bits/8);
		char *buf = (char*)JS_malloc(cx, totalSize +1);

		if ( bits == 16 ) {

			for ( size_t frame = 0; frame < frames; frame++ )
				((int16_t*)buf)[frame] = ((int16_t*)srcBuf)[frame*channelCount+c];
		} else
		if ( bits == 8 ) {

			for ( size_t frame = 0; frame < frames; frame++ )
				((int8_t*)buf)[frame] = ((int8_t*)srcBuf)[frame*channelCount+c];
		}

		jsval blobVal;
		buf[totalSize] = '\0';
		JL_CHK( JL_NewBlob(cx, buf, totalSize, &blobVal) );
		JSObject *blobObj;
		JL_CHK( JS_ValueToObject(cx, blobVal, &blobObj) );
		JL_CHKM( blobObj != NULL, E_STR("Blob"), E_CREATE );
		blobVal = OBJECT_TO_JSVAL(blobObj);
		JL_CHK( JL_SetElement(cx, destArray, c, &blobVal) );

		JL_CHK(JL_SetProperty(cx, blobObj, "bits", bits) );
		JL_CHK(JL_SetProperty(cx, blobObj, "rate", rate) );
		JL_CHK(JL_SetProperty(cx, blobObj, "channels", 1) );
		JL_CHK(JL_SetProperty(cx, blobObj, "frames", frames ) );
	}

	return JS_TRUE;
	JL_BAD;
}


/**doc
=== Note ===
 SoundObject concatenation can be achieved using the concat() method. This works becuase a sound object is a Blob with some extra properties.
**/

CONFIGURE_STATIC

	REVISION(JL_SvnRevToInt("$Revision$"))
	BEGIN_STATIC_FUNCTION_SPEC
//		FUNCTION( decodeOggVorbis )
//		FUNCTION( decodeSound )
		FUNCTION( splitChannels )
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
	END_STATIC_PROPERTY_SPEC

END_STATIC

/**doc
=== Examples ===
 $H example 1
 {{{
 loadModule('jsstd');
 loadModule('jsio');
 loadModule('jssound');

 var file = new File('41_30secOgg-q0.wav');
 file.open('r');
 var pcm = decodeSound(file);
 file.close();
 print('sample length: '+pcm.length, '\n');
 }}}

 $H example 2
 {{{
 loadModule('jsstd');
 loadModule('jsio');
 loadModule('jssound');

 var file = new File('41_30secOgg-q0.wav');
 var buffer = file.content;
 var stream = new Stream(buffer);
 var pcm = decodeSound(stream);

 print('sample length: '+pcm.length, '\n');
 }}}
**/



/*

	Vorbisfile Documentation:
		http://xiph.org/vorbis/doc/vorbisfile/index.html

	ogg files:
		http://xiph.org/vorbis/listen.html

	ogg + OpenAL:
		http://www.gamedev.net/reference/articles/article2031.asp

	decoding wav files with libsndfile:
		http://www.mega-nerd.com/libsndfile/

	decoding mp3 files with liblame:
		http://sourceforge.net/projects/lame (dl: http://sourceforge.net/project/showfiles.php?group_id=290)

	external libraries used by audacity:
		http://audacity.cvs.sourceforge.net/audacity/audacity-src/win/Projects/

	other sound libraries:
		http://ccrma.stanford.edu/software/snd/sndlib/
		http://www.68k.org/~michael/audiofile/

	Reading/Writing .wav Files:
		http://people.msoe.edu/~taylor/examples/wav.htm
*/
