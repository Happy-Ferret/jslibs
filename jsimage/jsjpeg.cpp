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

/*
API:
	http://refspecs.freestandards.org/LSB_3.1.0/LSB-Desktop-generic/LSB-Desktop-generic/libjpegman.html

example:
	.\src\example.c

steps:

	Allocate and initialize a JPEG decompression object
	Specify the source of the compressed data (eg, a file)
	Call jpeg_read_header() to obtain image info
	Set parameters for decompression
	jpeg_start_decompress(...);
	while (scan lines remain to be read)
		jpeg_read_scanlines(...);
	jpeg_finish_decompress(...);
	Release the JPEG decompression object

*/

#include "stdafx.h"
#include "../common/jsNativeInterface.h"

#include "image.h"

#ifndef INT32
	#define XMD_H // avoid: jslibs\libjpeg\src\jmorecfg.h(161) : error C2371: 'INT32' : redefinition; different basic types /n Microsoft Platform SDK\Include\basetsd.h(62) : see declaration of 'INT32'
#endif // INT32

#undef FAR

extern "C" {
	#include <jpeglib.h>
}
#include <jerror.h>

#include <stdlib.h>
#include <memory.h>

//#include <setjmp.h>

#define INPUT_BUF_SIZE 4096

typedef struct {
  struct jpeg_source_mgr pub;	// public fields
  JOCTET * buffer;
  void *pv;
  NIResourceRead read;
} SourceMgr;
typedef SourceMgr *SourceMgrPtr;

METHODDEF(void) init_source(j_decompress_ptr cinfo) {
}

METHODDEF(boolean) fill_input_buffer(j_decompress_ptr cinfo) {

	SourceMgrPtr src = (SourceMgrPtr) cinfo->src;

	unsigned int amount = INPUT_BUF_SIZE;
	src->read(src->pv, src->buffer, &amount);
	// (TBD) manage errors

	if ( amount == 0 ) {
		WARNMS(cinfo, JWRN_JPEG_EOF);
		src->buffer[0] = (JOCTET) 0xFF;
		src->buffer[1] = (JOCTET) JPEG_EOI;
		amount = 2;
	}
	src->pub.bytes_in_buffer = amount;
	src->pub.next_input_byte = src->buffer;
	return TRUE;
}

METHODDEF(void) skip_input_data(j_decompress_ptr cinfo, long num_bytes) {

	SourceMgrPtr src = (SourceMgrPtr) cinfo->src;

	if (num_bytes > 0) {

		while (num_bytes > (long) src->pub.bytes_in_buffer) {

			num_bytes -= (long) src->pub.bytes_in_buffer;
			fill_input_buffer(cinfo); // (TBD) change this
		}
		src->pub.next_input_byte += (size_t) num_bytes;
		src->pub.bytes_in_buffer -= (size_t) num_bytes;
	}
}

//METHODDEF(boolean) resync_to_restart(j_decompress_ptr cinfo, int desired) {
//
//	return TRUE;
//}

METHODDEF(void) term_source(j_decompress_ptr cinfo) {
}


//typedef struct {
//  struct jpeg_error_mgr pub;	// public fields
//  jmp_buf setjmp_buffer;	/* for return to caller */
//} ErrorMgr;
//typedef ErrorMgr *ErrorMgrPtr;

typedef struct {
	JSContext *cx;
	JSObject *obj;
} CxObj;

/*
void ReadUsingJsMethod( void *pv, unsigned char *data, unsigned int *length ) {

	jsval rval, len = INT_TO_JSVAL(*length);
	JS_CallFunctionName(((CxObj*)pv)->cx, ((CxObj*)pv)->obj, "Read", 1, &len, &rval ); // (TBD) check if function exists
	//if ( !JSVAL_IS_STRING(rval) )
	// (TBD) manage this error

//	JSString *str = JSVAL_TO_STRING(rval);
//	*length = JS_GetStringLength(str);
//	char *stringData = JS_GetStringBytes(str);

	char *stringData;
	RT_JSVAL_TO_STRING_AND_LENGTH( rval, stringData, *length );

	// (TBD) check if not NULL
	memcpy(data, stringData, *length); // (TBD) hard but try to avoid the useless copy of data
}
*/

BEGIN_CLASS( Jpeg )

DEFINE_FINALIZE() {

	j_decompress_ptr cinfo = (j_decompress_ptr)JS_GetPrivate(cx, obj);
	if ( cinfo != NULL ) {
		free(cinfo->err);
		free(cinfo);
	}
}

DEFINE_CONSTRUCTOR() {

	RT_ASSERT_CONSTRUCTING(_class);
	RT_ASSERT_ARGC(1);

	j_decompress_ptr cinfo = (j_decompress_ptr)malloc(sizeof(jpeg_decompress_struct)); // (TBD) free
	RT_ASSERT_ALLOC(cinfo);

// setup error structure
	jpeg_error_mgr *err = (jpeg_error_mgr *)malloc(sizeof(jpeg_error_mgr));
	cinfo->err = jpeg_std_error(err);

	jpeg_create_decompress(cinfo);

// alloc reader structure & buffer ( both are freed by jpeg_destroy_decompress )
	cinfo->src = (struct jpeg_source_mgr *) (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT, sizeof(SourceMgr));
	SourceMgrPtr src = (SourceMgrPtr)cinfo->src;
	src->buffer = (JOCTET *)(*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT, INPUT_BUF_SIZE * sizeof(JOCTET));

// setup reader structure
	src = (SourceMgrPtr) cinfo->src;
	src->pub.init_source = init_source;
	src->pub.fill_input_buffer = fill_input_buffer;
	src->pub.skip_input_data = skip_input_data;
	src->pub.resync_to_restart = jpeg_resync_to_restart; // use default method
	src->pub.term_source = term_source;

	src->pub.bytes_in_buffer = 0; // forces fill_input_buffer on first read
	src->pub.next_input_byte = NULL; // until buffer loaded

// try to use a fast way to read the data
	GetNativeInterface(cx, JSVAL_TO_OBJECT(argv[0]), NI_READ_RESOURCE, (FunctionPointer*)&src->read, &src->pv);
	RT_ASSERT( src->read != NULL && src->pv != NULL, "Unable to GetNativeResource." );

// else use a 'classic' method to read the data ( like var data = resourceObject.Read(amount); )
	if ( src->read == NULL || src->pv == NULL ) {
		RT_ASSERT( false, "TO BE DONE" ); // (TBD) read without NI_READ_RESOURCE
//		CxObj
//		src->pv = resourceObject;
//		src->read = ReadUsingJsMethod;
	}

//	JSBool status = GetNativeResource(cx, JSVAL_TO_OBJECT(argv[0]), &src->pv, &src->read, NULL );
//	RT_ASSERT( status == JS_TRUE, "Unable to GetNativeResource." );

// read image headers
	jpeg_read_header(cinfo, TRUE); //we passed TRUE to reject a tables-only JPEG file as an error.

	// the default is to produce full color output from a color file.

	jpeg_calc_output_dimensions(cinfo);

	JS_SetPrivate(cx, obj, cinfo);

	return JS_TRUE;
}


DEFINE_FUNCTION( Load ) {

	j_decompress_ptr cinfo = (j_decompress_ptr)JS_GetPrivate(cx, obj);
	RT_ASSERT( cinfo != NULL, RT_ERROR_NOT_INITIALIZED );

// read the image
	jpeg_start_decompress(cinfo);

	int width = cinfo->output_width;
	int height = cinfo->output_height;
	int bytePerRow = cinfo->output_width * cinfo->output_components;
	int size = height * bytePerRow;
	int channels = cinfo->output_components;

	JOCTET * data = (JOCTET *)malloc(height * bytePerRow);
	RT_ASSERT_ALLOC(data);
	JSObject *image = NewImage(cx, INT_TO_JSVAL(width), INT_TO_JSVAL(height), INT_TO_JSVAL(channels), data);
	*rval = OBJECT_TO_JSVAL(image);


	// cinfo->rec_outbuf_height : recomanded scanline height ( 1, 2 or 4 )
	while (cinfo->output_scanline < cinfo->output_height) {

		jpeg_read_scanlines(cinfo, &data, 1);
		data += bytePerRow;
	}

	jpeg_finish_decompress(cinfo);
	jpeg_destroy_decompress(cinfo);
	// jpeg_destroy(); ??

	return JS_TRUE;
}



DEFINE_PROPERTY( width ) {

	j_decompress_ptr cinfo = (j_decompress_ptr)JS_GetPrivate(cx, obj);
	RT_ASSERT( cinfo != NULL, RT_ERROR_NOT_INITIALIZED );
	int value = cinfo->output_width;
	RT_ASSERT( value != 0, RT_ERROR_NOT_INITIALIZED );
	*vp = INT_TO_JSVAL(value);
	return JS_TRUE;
}

DEFINE_PROPERTY( height ) {

	j_decompress_ptr cinfo = (j_decompress_ptr)JS_GetPrivate(cx, obj);
	RT_ASSERT( cinfo != NULL, RT_ERROR_NOT_INITIALIZED );
	int value = cinfo->output_height;
	RT_ASSERT( value != 0, RT_ERROR_NOT_INITIALIZED );
	*vp = INT_TO_JSVAL(value);
	return JS_TRUE;
}

DEFINE_PROPERTY( channels ) {

	j_decompress_ptr cinfo = (j_decompress_ptr)JS_GetPrivate(cx, obj);
	RT_ASSERT( cinfo != NULL, RT_ERROR_NOT_INITIALIZED );
	int value = cinfo->output_components;
	RT_ASSERT( value != 0, RT_ERROR_NOT_INITIALIZED );
	*vp = INT_TO_JSVAL(value);
	return JS_TRUE;
}

// output_gamma

CONFIGURE_CLASS

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION(Load)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ(width)
		PROPERTY_READ(height)
		PROPERTY_READ(channels)
	END_PROPERTY_SPEC

	HAS_PRIVATE

END_CLASS
