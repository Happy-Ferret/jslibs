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

#include "../common/jsNativeInterface.h"
#include "../jslang/bstringapi.h"
#include "../common/stack.h"


#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include	<sndfile.h>


// doc. For seek_func(), you *MUST* return -1 if the stream is unseekable

typedef struct {
	JSContext *cx;
	JSObject *obj;
	NIStreamRead streamRead;
} StreamReadInfo;


size_t readStream( void *ptr, size_t size, size_t nmemb, void *datasource ) {

	StreamReadInfo *info = (StreamReadInfo*)datasource;
	size_t amount = size * nmemb;

	if ( info->streamRead( info->cx, info->obj, (char*)ptr, &amount ) != JS_TRUE ) {
		// error
	}
	return amount;
}


BEGIN_STATIC

DEFINE_FUNCTION_FAST( DecodeOggVorbis ) {
	
	J_S_ASSERT_ARG_MIN( 1 );
	J_S_ASSERT_OBJECT( J_FARG(1) );

	JSObject *oggObj = JSVAL_TO_OBJECT( J_FARG(1) );
	ov_callbacks callbacks = { 0,0,0,0 };
	StreamReadInfo datasource;

	NIStreamRead streamReader;
	J_CHECK_CALL( GetStreamReadInterface(cx, oggObj, &streamReader) );
	if ( streamReader != NULL ) {

		callbacks.read_func = readStream;
		datasource.cx = cx;
		datasource.obj = oggObj;
		datasource.streamRead = streamReader;
	} else {

		REPORT_ERROR("Invalid data source.");
	}

	OggVorbis_File oggFile;
	ov_open_callbacks(&datasource, &oggFile, NULL, 0, callbacks);

// (TBD) if totalPcmSamples != OV_EINVAL, create the right buffer !
//	ogg_int64_t totalPcmSamples = ov_pcm_total(&oggFile, 0); // OV_EINVAL

	vorbis_info *info = ov_info(&oggFile, -1);
	int bits = 16;

	int bitStream;

	void *stack;
	StackInit(&stack);

	int bufferSize = 4096 - 32; // try to alloc not more than one page

	long totalSize = 0;
	long bytes;
	do {

		char *buffer = (char*)malloc(bufferSize);
		J_S_ASSERT_ALLOC(buffer);

		StackPush(&stack, buffer);

		bytes = ov_read(&oggFile, buffer+sizeof(int), bufferSize-sizeof(int), 0, bits/8, 1, &bitStream);

		if ( bytes < 0 ) { // 0 indicates EOF
		
			*(int*)buffer = 0;

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
	JSObject *bstrObj = NewBString(cx, buf, totalSize);
	J_S_ASSERT( bstrObj != NULL, "Unable to create the BString object.");
	*J_FRVAL = OBJECT_TO_JSVAL(bstrObj);
	SetPropertyInt32(cx, bstrObj, "bits", bits); // bits per sample
	SetPropertyInt32(cx, bstrObj, "rate", info->rate);
	SetPropertyInt32(cx, bstrObj, "channels", info->channels);

	ov_clear(&oggFile); // beware: info must be valid

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


sf_count_t vio_get_filelen(void *user_data) {
	return 0;
}

sf_count_t vio_seek(sf_count_t offset, int whence, void *user_data) {
	return 0;
}

sf_count_t vio_read(void *ptr, sf_count_t count, void *user_data) {
	return 0;
}

sf_count_t vio_write(const void *ptr, sf_count_t count, void *user_data) {
	return 0;
}

sf_count_t vio_tell(void *user_data) {
	return 0;
}


DEFINE_FUNCTION_FAST( DecodeSound ) {

	J_S_ASSERT_ARG_MIN( 1 );
	JSObject *inputObj = JSVAL_TO_OBJECT( J_FARG(1) );
	SF_VIRTUAL_IO callbacks = { 0,0,0,0,0 };
	void *datasource;

	callbacks.get_filelen = vio_get_filelen;
	callbacks.seek = vio_seek;
	callbacks.read = vio_read;
	callbacks.write = vio_write;
	callbacks.tell = vio_tell;

	SF_INFO sfinfo = {0};

	SNDFILE *sf = sf_open_virtual(&callbacks, SFM_READ, &sfinfo, datasource);


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