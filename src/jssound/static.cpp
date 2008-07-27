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

#include "static.h"

#include <cstring>

#include "../jslang/bstringapi.h"
#include "../common/stack.h"

#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include	<sndfile.h>


// doc. For seek_func(), you *MUST* return -1 if the stream is unseekable

typedef struct {
	JSContext *cx;
	JSObject *obj;
//	NIStreamRead streamRead;
} StreamReadInfo;


/**doc fileIndex:topmost **/

BEGIN_STATIC

size_t readStream( void *ptr, size_t size, size_t nmemb, void *pv ) {

	StreamReadInfo *info = (StreamReadInfo*)pv;
	size_t amount = size * nmemb;
//	if ( info->streamRead( info->cx, info->obj, (char*)ptr, &amount ) != JS_TRUE )
//		return -1; // (TBD) check for a better error
	if ( StreamReadInterface(info->cx, info->obj)(info->cx, info->obj, (char*)ptr, &amount) != JS_TRUE )
		return -1; // (TBD) check for a better error

	return amount;
}

static ov_callbacks ovCallbacks = { readStream,0,0,0 };


/**doc
 * $TYPE soundObject $INAME( stream )
  Decodes a ogg vorbis sample to a sound object.
  $H arguments
   $ARG streamObject stream: any object that has a Read function or that supports the NIStreamRead Native Interface ( file, socket, new Stream(buffer), ... ). For further details about stream objects, see jslang::Stream object and NativeInterface mechanism.
  $H return value
   returns a sound object in a 16-bit per sample format.
  $H example
  {{{
  LoadModule('jsstd');
  LoadModule('jsio');
  LoadModule('jssound');

  var file = new File('41_30secOgg-q0.ogg');
  var buffer = file.content;
  var stream = new Stream(buffer);
  var mySoundObject = DecodeOggVorbis(stream);
  Print('sample length: ', mySoundObject.length, '\n');
  }}}
**/
DEFINE_FUNCTION_FAST( DecodeOggVorbis ) {

	J_S_ASSERT_ARG_MIN( 1 );
	J_S_ASSERT_OBJECT( J_FARG(1) );
	JSObject *StreamObj = JSVAL_TO_OBJECT( J_FARG(1) );

//	NIStreamRead streamReader;
//	J_CHK( GetStreamReadInterface(cx, StreamObj, &streamReader) );
//	J_S_ASSERT( streamReader != NULL, "Invalid stream." );

	StreamReadInfo pv = { cx, StreamObj };
	OggVorbis_File descriptor;
	ov_open_callbacks(&pv, &descriptor, NULL, 0, ovCallbacks);

	// (TBD) manage errors

	vorbis_info *info = ov_info(&descriptor, -1);
	int bits = 16;

	J_S_ASSERT( bits != 8 || bits == 16, "Unsupported bits count." );
	J_S_ASSERT( info->channels == 1 || info->channels == 2, "Unsupported channel count." );

	int bitStream;
	void *stack;
	StackInit(&stack);

	int bufferSize = 4096 - 16; // try to alloc less than one page

	long totalSize = 0;
	long bytes;
	do {

		char *buffer = (char*)malloc(bufferSize);
		J_S_ASSERT_ALLOC(buffer);

		StackPush(&stack, buffer);

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
	char *buf = (char*)JS_malloc(cx, totalSize);
	J_S_ASSERT_ALLOC(buf);

	jsval bstr;
	J_CHK( J_NewBinaryString(cx, buf, totalSize, &bstr) );
	JSObject *bstrObj;
	J_CHK( JS_ValueToObject(cx, bstr, &bstrObj) );
	J_S_ASSERT( bstrObj != NULL, "Unable to create the BString object.");
	*J_FRVAL = OBJECT_TO_JSVAL(bstrObj);

	J_CHK( SetPropertyInt(cx, bstrObj, "bits", bits) ); // bits per sample
	J_CHK( SetPropertyInt(cx, bstrObj, "rate", info->rate) );
	J_CHK( SetPropertyInt(cx, bstrObj, "channels", info->channels) );

	ov_clear(&descriptor); // beware: info must be valid

	// because the stack is LIFO, we have to start from the end.
	buf += totalSize;
	while( !StackIsEnd(&stack) ) {

		char *buffer = (char *)StackPop(&stack);
		int size = *(int*)buffer;
		buf = buf - size;
		memcpy( buf, buffer+sizeof(int), size );
		free(buffer);
	}

	return JS_TRUE;
}



sf_count_t SfGetFilelen(void *user_data) {

	StreamReadInfo *pv = (StreamReadInfo *)user_data;
	jsval tmpVal;

	int position;
	JS_GetProperty(pv->cx, pv->obj, "position", &tmpVal); // (TBD) manage error
	if ( tmpVal == JSVAL_VOID )
		return -1;
	JsvalToInt(pv->cx, tmpVal, &position); // (TBD) manage error

	int available;
	JS_GetProperty(pv->cx, pv->obj, "available", &tmpVal); // (TBD) manage error
	if ( tmpVal == JSVAL_VOID )
		return -1;
	JsvalToInt(pv->cx, tmpVal, &available); // (TBD) manage error

	return position + available;
}


sf_count_t SfSeek(sf_count_t offset, int whence, void *user_data) {

	StreamReadInfo *pv = (StreamReadInfo *)user_data;

	jsval tmpVal;
	int position, available;

	switch (whence) {
		case SEEK_SET:
			if ( offset < 0 )
				return -1;
			IntToJsval(pv->cx, offset, &tmpVal); // (TBD) manage error
			JS_SetProperty(pv->cx, pv->obj, "position", &tmpVal); // (TBD) manage error
			return 0;

		case SEEK_CUR:
			JS_GetProperty(pv->cx, pv->obj, "position", &tmpVal); // (TBD) manage error
			if ( tmpVal == JSVAL_VOID )
				return -1;
			JsvalToInt(pv->cx, tmpVal, &position); // (TBD) manage error

			position += offset;
			IntToJsval(pv->cx, position, &tmpVal); // (TBD) manage error
			JS_SetProperty(pv->cx, pv->obj, "position", &tmpVal); // (TBD) manage error
			return 0;

		case SEEK_END:
			JS_GetProperty(pv->cx, pv->obj, "available", &tmpVal);
			if ( tmpVal == JSVAL_VOID )
				return -1;
			JsvalToInt(pv->cx, tmpVal, &available);

			JS_GetProperty(pv->cx, pv->obj, "position", &tmpVal);
			if ( tmpVal == JSVAL_VOID )
				return -1;
			JsvalToInt(pv->cx, tmpVal, &position);

			if ( offset > 0 || -offset > position + available )
				return -1;
			JsvalToInt(pv->cx, tmpVal, &position);
			IntToJsval(pv->cx, position + available + offset, &tmpVal); // the pointer is set to the size of the file plus offset.
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
	if ( tmpVal == JSVAL_VOID )
		return -1;
	JsvalToInt(pv->cx, tmpVal, &position);

	return position;
}

sf_count_t SfRead(void *ptr, sf_count_t count, void *user_data) {

	StreamReadInfo *pv = (StreamReadInfo *)user_data;

	size_t amount = count;
//	if ( pv->streamRead( pv->cx, pv->obj, (char*)ptr, &amount ) != JS_TRUE )
//		return -1; // (TBD) find a better error
	if ( StreamReadInterface( pv->cx, pv->obj)( pv->cx, pv->obj, (char*)ptr, &amount ) != JS_TRUE )
		return -1; // (TBD) find a better error
	return amount;
}


static SF_VIRTUAL_IO sfCallbacks = { SfGetFilelen, SfSeek, SfRead, 0, SfTell };


/**doc
 * $TYPE soundObject $INAME( stream )
  Decodes a sample from any supported sound format to a sound object.
  $H arguments
   $ARG streamObject stream: any object that has a Read function or that supports the NIStreamRead Native Interface ( file, socket, new Stream(buffer), ... ). For further details about stream objects, see jslang::Stream object and native interface mechanism.
  $H return value
   returns a sound object in a 16-bit per sample format.
**/
DEFINE_FUNCTION_FAST( DecodeSound ) {

	J_S_ASSERT_ARG_MIN( 1 );

	J_S_ASSERT_OBJECT( J_FARG(1) );
	JSObject *StreamObj = JSVAL_TO_OBJECT( J_FARG(1) );

//	NIStreamRead streamReader;
//	J_CHK( GetStreamReadInterface(cx, StreamObj, &streamReader) );
//	J_S_ASSERT( streamReader != NULL, "Invalid stream." );

	StreamReadInfo pv = { cx, StreamObj };
	SF_INFO info = {0};
	SNDFILE *descriptor = sf_open_virtual(&sfCallbacks, SFM_READ, &info, &pv);

	J_S_ASSERT_1( sf_error(descriptor) == SF_ERR_NO_ERROR, "sndfile error: %d", sf_error(descriptor) );
	J_S_ASSERT( descriptor != NULL, "Invalid stream." );

	if ( JS_IsExceptionPending(cx) )
		return JS_FALSE;

	J_S_ASSERT( info.channels == 1 || info.channels == 2, "Unsupported channel count." );

	sf_command(descriptor, SFC_SET_SCALE_FLOAT_INT_READ, NULL, SF_TRUE); // Doc. Set/clear the scale factor when integer (short/int) data is read from a file containing floating point data.

	void *stack;
	StackInit(&stack);

	int bufferSize = 16384 - 16; // try to alloc less than one page

	long totalSize = 0;
	sf_count_t items;
	do {

		char *buffer = (char*)malloc(bufferSize);
		J_S_ASSERT_ALLOC(buffer);
		StackPush(&stack, buffer);

		char *data = buffer+sizeof(int);
		int *len = (int*)buffer;
		int maxlen = bufferSize - sizeof(int);

		items = sf_read_short(descriptor, (short*)data, maxlen/sizeof(short)); // bits per sample

		J_S_ASSERT_1( sf_error(descriptor) == SF_ERR_NO_ERROR, "sndfile error: %d", sf_error(descriptor) );

		if ( items <= 0 ) { // 0 indicates EOF

			*len = 0;

		} else {

			*len = items * sizeof(short);
			totalSize += items * sizeof(short);
		}

	} while (items > 0);


	// convert data chunks into a single memory buffer.
	char *buf = (char*)JS_malloc(cx, totalSize);
	J_S_ASSERT_ALLOC(buf);

//	JSObject *bstrObj = J_NewBinaryString(cx, buf, totalSize);
//	J_S_ASSERT( bstrObj != NULL, "Unable to create the BString object.");
//	*J_FRVAL = OBJECT_TO_JSVAL(bstrObj);

	jsval bstr;
	J_CHK( J_NewBinaryString(cx, buf, totalSize, &bstr) );
	JSObject *bstrObj;
	J_CHK( JS_ValueToObject(cx, bstr, &bstrObj) );
	J_S_ASSERT( bstrObj != NULL, "Unable to create the BString object.");
	*J_FRVAL = OBJECT_TO_JSVAL(bstrObj);

	J_CHK( SetPropertyInt(cx, bstrObj, "bits", 16) ); // bits per sample
	J_CHK( SetPropertyInt(cx, bstrObj, "rate", info.samplerate) );
	J_CHK( SetPropertyInt(cx, bstrObj, "channels", info.channels) );

	sf_close(descriptor);

	// because the stack is LIFO, we have to start from the end.
	buf += totalSize;
	while( !StackIsEnd(&stack) ) {

		char *buffer = (char *)StackPop(&stack);
		int size = *(int*)buffer;
		buf = buf - size;
		memcpy( buf, buffer+sizeof(int), size );
		free(buffer);
	}

	return JS_TRUE;
}


CONFIGURE_STATIC

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION_FAST( DecodeOggVorbis )
		FUNCTION_FAST( DecodeSound )
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
	END_STATIC_PROPERTY_SPEC

END_STATIC

/**doc
=== Examples ===
 $H example 1
 {{{
 LoadModule('jsstd');
 LoadModule('jsio');
 LoadModule('jssound');
 
 var file = new File('41_30secOgg-q0.wav');
 file.Open('r');
 var pcm = DecodeSound(file);
 file.Close();
 Print('sample length: '+pcm.length, '\n');
 }}}

 $H example 2
 {{{
 LoadModule('jsstd');
 LoadModule('jsio');
 LoadModule('jssound');
 
 var file = new File('41_30secOgg-q0.wav');
 var buffer = file.content;
 var stream = new Stream(buffer);
 var pcm = DecodeSound(stream);
 
 Print('sample length: '+pcm.length, '\n');
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