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

BEGIN_CLASS( SoundFileDecoder )

typedef struct {
	JSContext *cx; // temporary set to the current JSContext while sndfile API is called !
//	JSObject *thisObj; // the SoundFileDecoder object
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

DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();
	J_S_ASSERT_ARG_MIN(1);
	J_S_ASSERT_OBJECT( J_ARG(1) );

	Private *pv = (Private*)malloc(sizeof(Private));

	J_CHK( JS_SetReservedSlot(cx, obj, SLOT_INPUT_STREAM, J_ARG(1) ) ); // used to keep a root on it only.
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
	J_CHK( JS_SetPrivate(cx, obj, pv) );

	return JS_TRUE;
}

/**doc
 * $TYPE soundObject $INAME( frames )
  Decodes _frames_ frapes from the stream.
**/
DEFINE_FUNCTION_FAST( Read ) {

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

		if ( MaybeRealloc(amount, totalSize) ) {

			buf = (char*)realloc(buf, totalSize);
			J_S_ASSERT_ALLOC(buf);
		}

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
	J_S_ASSERT( blobVal != NULL, "Unable to create the Blob object.");
	*J_FRVAL = OBJECT_TO_JSVAL(blobVal);

	J_CHK( SetPropertyInt(cx, blobObj, "bits", pv->bits) ); // bits per sample
	J_CHK( SetPropertyInt(cx, blobObj, "rate", pv->sfInfo.samplerate) ); // samples per second
	J_CHK( SetPropertyInt(cx, blobObj, "channels", pv->sfInfo.channels) ); // 1:mono, 2:stereo
	J_CHK( SetPropertyInt(cx, blobObj, "frames", totalSize / (pv->sfInfo.channels * pv->bits / 8) ) );

	return JS_TRUE;
}


DEFINE_PROPERTY( bits ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(pv);
	*vp = INT_TO_JSVAL( pv->bits );
	return JS_TRUE;
}

DEFINE_PROPERTY( rate ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(pv);
	*vp = INT_TO_JSVAL( pv->sfInfo.samplerate );
	return JS_TRUE;
}

DEFINE_PROPERTY( channels ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(pv);
	*vp = INT_TO_JSVAL( pv->sfInfo.channels );
	return JS_TRUE;
}

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
