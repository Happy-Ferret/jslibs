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

#include "../common/jsNativeInterface.h"

#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>


// doc. For seek_func(), you *MUST* return -1 if the stream is unseekable

typedef struct {

	JSContext *cx;
	JSObject *obj;
	NIStreamRead streamRead;
} OggInfo;

size_t read_func( void *ptr, size_t size, size_t nmemb, void *datasource ) {

	OggInfo *info = (OggInfo*)datasource;

	size_t sizeToRead = size * nmemb;

	if ( info->streamRead( info->cx, info->obj, (char*)ptr, &sizeToRead ) != JS_TRUE ) {
		// error
	}

	return sizeToRead;
}


//int seek_func(void *datasource, ogg_int64_t offset, int whence) {
//	return -1;
//}

//int close_func( void *datasource ) {
//
//	return 0;
//}

//  long   (*tell_func)  (void *datasource);




BEGIN_STATIC

DEFINE_FUNCTION_FAST( DecodeVorbis ) {
	
	J_S_ASSERT_ARG_MIN( 1 );

	JSObject *oggStreamObj = JSVAL_TO_OBJECT( J_FARG(1) );
	
	ov_callbacks callbacks = { read_func, NULL, NULL, NULL } ;

	OggVorbis_File oggFile;



	OggInfo descriptorInfo = { cx, oggStreamObj };
	J_CHECK_CALL( GetStreamReadInterface(cx, oggStreamObj, &descriptorInfo.streamRead) );
	J_S_ASSERT( descriptorInfo.streamRead != NULL, "Invalid stream" );
	ov_open_callbacks( &descriptorInfo, &oggFile, NULL, 0, callbacks );

	long bytes;
	int bitStream;

	char buffer[4096]; // Local fixed size array

	do {
		// Read up to a buffer's worth of decoded sound data
		bytes = ov_read(&oggFile, buffer, sizeof(buffer), 0, 2, 1, &bitStream);
		// Append to end of buffer
		//    buffer.insert(buffer.end(), array, array + bytes);
	} while (bytes > 0);

	ov_clear(&oggFile);

	return JS_TRUE;
}

CONFIGURE_STATIC

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION_FAST( DecodeVorbis )
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
	END_STATIC_PROPERTY_SPEC

END_STATIC

/*

	ogg files: http://xiph.org/vorbis/listen.html

	ogg + OpenAL: http://www.gamedev.net/reference/articles/article2031.asp

*/