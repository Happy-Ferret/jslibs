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

#include "../jslang/blobPub.h"

//#include "jlnativeinterface.h"
#include "static.h"


#ifndef INT32
	#define XMD_H // avoid: jslibs\libjpeg\src\jmorecfg.h(161) : error C2371: 'INT32' : redefinition; different basic types /n Microsoft Platform SDK\Include\basetsd.h(62) : see declaration of 'INT32'
#endif // INT32

#undef FAR

EXTERN_C {
	#include <jpeglib.h>
	#include <jerror.h>
}



#include <png.h>


/**doc fileIndex:topmost **/

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_STATIC



//////////////////////////////////////////////////////////////////////////////
//// JPEG

#define INPUT_BUF_SIZE 4096

typedef struct {
	struct jpeg_source_mgr pub;	// public fields
	JOCTET *buffer;
	JSContext *cx;
	JSObject *obj;
//	NIStreamRead read;
} SourceMgr;
typedef SourceMgr *SourceMgrPtr;


METHODDEF(void) init_source(j_decompress_ptr cinfo) {
}


METHODDEF(boolean) fill_input_buffer(j_decompress_ptr cinfo) {

	SourceMgrPtr src = (SourceMgrPtr) cinfo->src;

	size_t amount = INPUT_BUF_SIZE;
//	src->read(src->pv, src->buffer, &amount);
//	src->read( src->cx, src->obj, (char*)src->buffer, &amount);

//	StreamReadInterface( src->cx, src->obj, (char*)src->buffer, &amount);

	StreamReadInterface( src->cx, src->obj )(src->cx, src->obj, (char*)src->buffer, &amount);


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

METHODDEF(void) term_source(j_decompress_ptr cinfo) {
}

/**doc
$TOC_MEMBER $INAME
 $TYPE imageObject $INAME( streamObject )
  This function returns an image object that represents the decompressed jpeg image given as argument.
  $LF
  The streamObject argument is any object that supports the NIStreamRead Native Interface ( file, socket, new Stream(buffer), ... )
  $LF
  For further details about stream objects, see jslang::Stream object and NativeInterface mechanism.
**/
DEFINE_FUNCTION( DecodeJpegImage ) {

	jpeg_decompress_struct cinfo;
	jpeg_error_mgr errMgr;
	cinfo.err = jpeg_std_error(&errMgr);

	jpeg_create_decompress(&cinfo);

// alloc reader structure & buffer ( both are freed by jpeg_destroy_decompress )
	cinfo.src = (struct jpeg_source_mgr *) (*cinfo.mem->alloc_small) ((j_common_ptr) &cinfo, JPOOL_PERMANENT, sizeof(SourceMgr));
	SourceMgrPtr src = (SourceMgrPtr)cinfo.src;

//	src->buffer = (JOCTET *)(*cinfo.mem->alloc_small) ((j_common_ptr) &cinfo, JPOOL_PERMANENT, INPUT_BUF_SIZE * sizeof(JOCTET));
	JOCTET buffer[INPUT_BUF_SIZE];
	src->buffer = buffer;

// setup reader structure
	src = (SourceMgrPtr) cinfo.src;
	src->pub.init_source = init_source;
	src->pub.fill_input_buffer = fill_input_buffer;
	src->pub.skip_input_data = skip_input_data;
	src->pub.resync_to_restart = jpeg_resync_to_restart; // use default method
	src->pub.term_source = term_source;

	src->pub.bytes_in_buffer = 0; // forces fill_input_buffer on first read
	src->pub.next_input_byte = NULL; // until buffer loaded

	JL_S_ASSERT_OBJECT( JL_ARG(1) );
	src->obj = JSVAL_TO_OBJECT( JL_ARG(1) ); // required by NIStreamRead
	src->cx = cx; // required by NIStreamRead

// try to use a fast way to read the data
//	JL_CHK( GetStreamReadInterface(cx, JSVAL_TO_OBJECT(argv[0]), &src->read) );

//	JL_S_ASSERT( src->read != NULL, "Unable to GetNativeResource." );

// else use a 'classic' method to read the data ( like var data = resourceObject.Read(amount); )
//	if ( src->read == NULL ) {

//		JL_S_ASSERT( false, "TO BE DONE" ); // (TBD) read without NI_READ_RESOURCE
//		CxObj
//		src->pv = resourceObject;
//		src->read = ReadUsingJsMethod;
//	}

//	JSBool status = GetNativeResource(cx, JSVAL_TO_OBJECT(argv[0]), &src->pv, &src->read, NULL );
//	JL_S_ASSERT( status == JS_TRUE, "Unable to GetNativeResource." );

// read image headers
	jpeg_read_header(&cinfo, TRUE); //we passed TRUE to reject a tables-only JPEG file as an error.

	// the default is to produce full color output from a color file.

	jpeg_calc_output_dimensions(&cinfo);

// read the image
	jpeg_start_decompress(&cinfo);

	int width;
	width = cinfo.output_width;
	int height;
	height = cinfo.output_height;
	int bytePerRow;
	bytePerRow = cinfo.output_width * cinfo.output_components;
	int size;
	size = height * bytePerRow;
	int channels;
	channels = cinfo.output_components;

	int length;
	length = height * bytePerRow;
	JOCTET * data;
	data = (JOCTET *)JS_malloc(cx, length +1);
	JL_CHK( data );

	data[length] = 0;
	JL_CHK( JL_NewBlob(cx, data, length, JL_RVAL) );
	JSObject *blobObj;
	JL_CHK( JS_ValueToObject(cx, *JL_RVAL, &blobObj) );
	JL_S_ASSERT( blobObj, "Unable to create Blob object." );
	*JL_RVAL = OBJECT_TO_JSVAL(blobObj);

	JS_DefineProperty(cx, blobObj, "channels", INT_TO_JSVAL(channels), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT );
	JS_DefineProperty(cx, blobObj, "width", INT_TO_JSVAL(width), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT );
	JS_DefineProperty(cx, blobObj, "height", INT_TO_JSVAL(height), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT );

	// cinfo->rec_outbuf_height : recomanded scanline height ( 1, 2 or 4 )
	while (cinfo.output_scanline < cinfo.output_height) {

		jpeg_read_scanlines(&cinfo, &data, 1);
		data += bytePerRow;
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	// jpeg_destroy(); ??

	return JS_TRUE;
	JL_BAD;
}


/*
DEFINE_FUNCTION( EncodeJpegImage ) {

	JL_REPORT_ERROR("TBD!"); // (TBD)
	return JS_TRUE;
	JL_BAD;
}
*/


//////////////////////////////////////////////////////////////////////////////
//// PNG

typedef struct {
	png_structp png;
	png_infop info;
	JSContext *cx;
	JSObject *obj;
//	NIStreamRead read;
} PngReadUserStruct;


static void _png_read( png_structp png_ptr, png_bytep data, png_size_t length ) {

	PngReadUserStruct *desc = (PngReadUserStruct*)png_get_io_ptr(png_ptr);
//	desc->read(desc->cx, desc->obj, (char*)data, &length);
	StreamReadInterface(desc->cx, desc->obj)(desc->cx, desc->obj, (char*)data, &length);

//	if ( length == 0 )
//		png_error(png_ptr, "Trying to read after EOF."); // png_warning()
}

png_voidp malloc_wrapper(png_structp png_ptr, png_size_t size) {

	return jl_malloc(size);
}

void free_wrapper(png_structp png_ptr, png_voidp ptr) {
	
	jl_free(ptr);
}

/**doc
$TOC_MEMBER $INAME
 $TYPE imageObject $INAME( streamObject )
  This function returns an image object that represents the decompressed png image given as argument.
  $LF
  The streamObject argument is any object that supports the NIStreamRead Native Interface ( file, socket, new Stream(buffer), ... )
  $LF
  For further details about stream objects, see jslang::Stream object and NativeInterface mechanism.
**/
DEFINE_FUNCTION( DecodePngImage ) {

	PngReadUserStruct desc;

//	desc.png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
	desc.png = png_create_read_struct_2(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL, NULL, malloc_wrapper, free_wrapper);
	JL_S_ASSERT( desc.png != NULL, "Unable to png_create_read_struct.");
	desc.info = png_create_info_struct(desc.png);
	JL_S_ASSERT( desc.info != NULL, "Unable to png_create_info_struct.");

//	JL_CHK( GetStreamReadInterface(cx, JSVAL_TO_OBJECT(argv[0]), &desc.read) );
//	JL_S_ASSERT( desc.read != NULL, "Unable to GetNativeResource." );

	JL_S_ASSERT_OBJECT( JL_ARG(1) );
	desc.obj = JSVAL_TO_OBJECT( JL_ARG(1) );
	desc.cx = cx;

	png_set_read_fn( desc.png, (voidp)&desc, _png_read );
   png_read_info(desc.png, desc.info);
	JL_S_ASSERT( desc.info->height <= PNG_UINT_32_MAX/png_sizeof(png_bytep), "Image is too high to process with png_read_png()");

	JL_S_ASSERT( png_set_interlace_handling(desc.png) == 1, "Cannot read interlaced image yet." );

// Load
	png_set_strip_16(desc.png);
	png_set_packing(desc.png);
	if ((desc.png->bit_depth < 8) || (desc.png->color_type == PNG_COLOR_TYPE_PALETTE) || (png_get_valid(desc.png, desc.info, PNG_INFO_tRNS)))
		png_set_expand(desc.png);
   png_read_update_info(desc.png, desc.info); // optional call to update the users info_ptr structure

	png_uint_32 width;
	width = png_get_image_width(desc.png, desc.info);
	png_uint_32 height;
	height = png_get_image_height(desc.png, desc.info);
	png_uint_32 bytePerRow;
	bytePerRow = png_get_rowbytes(desc.png, desc.info);
	png_byte channels;
	channels = png_get_channels(desc.png, desc.info);

// read & store

	int length;
	length = height * bytePerRow;
	png_bytep data;
	data = (png_bytep)JS_malloc(cx, length +1);
	JL_CHK( data );

	data[length] = 0;
	JL_CHK( JL_NewBlob(cx, data, length, JL_RVAL) );
	JSObject *blobObj;
	JL_CHK( JS_ValueToObject(cx, *JL_RVAL, &blobObj) );
	JL_S_ASSERT( blobObj, "Unable to create Blob object." );
	*JL_RVAL = OBJECT_TO_JSVAL(blobObj);

	JS_DefineProperty(cx, blobObj, "channels", INT_TO_JSVAL(channels), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT );
	JS_DefineProperty(cx, blobObj, "width", INT_TO_JSVAL(width), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT );
	JS_DefineProperty(cx, blobObj, "height", INT_TO_JSVAL(height), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT );


	// int number_of_passes = png_set_interlace_handling(desc.png);
// for ( int p = 0; p < number_of_passes; p++ )
	for( unsigned int y = 0; y < height; ++y ) {

		png_read_row(desc.png, data, png_bytep_NULL);
		data += bytePerRow;
	}
   png_read_end(desc.png, desc.info); // read rest of file, and get additional chunks in desc.info - REQUIRED
	png_destroy_read_struct(&desc.png, &desc.info, NULL);

	return JS_TRUE;
	JL_BAD;
}



typedef struct {
	png_structp png;
	png_infop info;
	size_t pos;
	void *buffer;
} PngWriteUserStruct;


void write_row_callback(png_structp png_ptr, png_bytep data, png_size_t size) {

	PngWriteUserStruct *desc = (PngWriteUserStruct*)png_get_io_ptr(png_ptr);
	memcpy((char*)desc->buffer + desc->pos, data, size);
	desc->pos += size;
}

void output_flush_fn(png_structp png_ptr) {
}

DEFINE_FUNCTION( EncodePngImage ) {

	JLStr buffer;

	JL_S_ASSERT_ARG_MIN(1);

	int compressionLevel;
	if ( JL_ARG_ISDEF(2) ) {

		JL_S_ASSERT_INT( JL_ARG(2) );
		compressionLevel = JSVAL_TO_INT( JL_ARG(2) );
		JL_S_ASSERT( compressionLevel >= 0 && compressionLevel <= 9, "Invalid compression level." );
	} else {

		compressionLevel = Z_DEFAULT_COMPRESSION;
	}

	JL_S_ASSERT_OBJECT( JL_ARG(1) );
	JSObject *image;
	image = JSVAL_TO_OBJECT( JL_ARG(1) );
	int sWidth, sHeight, sChannels;
	JL_CHK( JL_GetProperty(cx, image, "width", &sWidth) );
	JL_CHK( JL_GetProperty(cx, image, "height", &sHeight) );
	JL_CHK( JL_GetProperty(cx, image, "channels", &sChannels) );

	PngWriteUserStruct desc;

	desc.buffer = JS_malloc(cx, sWidth * sHeight * sChannels + 1024);
	JL_CHK( desc.buffer );
	desc.pos = 0;

//	const char *sBuffer;
//	size_t bufferLength;
//	JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(1), &sBuffer, &bufferLength ) ); // warning: GC on the returned buffer !
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), buffer) );

	JL_S_ASSERT( buffer.Length() == (size_t)(sWidth * sHeight * sChannels * 1), "Invalid image format." );

	desc.png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	JL_S_ASSERT( desc.png != NULL, "Unable to png_create_write_struct.");
	desc.info = png_create_info_struct(desc.png);
	JL_S_ASSERT( desc.info != NULL, "Unable to png_create_info_struct.");

	png_set_write_fn( desc.png, (voidp)&desc, write_row_callback, output_flush_fn );

	int color_type;
	switch (sChannels) {
		case 1:
			color_type = PNG_COLOR_TYPE_GRAY;
			break;
		case 2:
			color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
			break;
		case 3:
			color_type = PNG_COLOR_TYPE_RGB;
			break;
		case 4:
			color_type = PNG_COLOR_TYPE_RGB_ALPHA;
			break;
		default:
			JL_REPORT_ERROR("Format not supported.");
	}

	png_set_IHDR(desc.png, desc.info, sWidth, sHeight, 8, color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_set_compression_level(desc.png, compressionLevel);

	png_write_info(desc.png, desc.info);
	for ( int r = 0; r < sHeight; r++ )
		png_write_row(desc.png, (png_bytep)buffer.GetConstStr() + sWidth * r * sChannels);
	png_write_end(desc.png, desc.info);

	png_destroy_write_struct(&desc.png, &desc.info);

	desc.buffer = JS_realloc(cx, desc.buffer, desc.pos +1);
	JL_CHK( desc.buffer );

	((uint8_t*)desc.buffer)[desc.pos] = 0;
	JL_CHK( JL_NewBlob(cx, desc.buffer, desc.pos, JL_RVAL) );

	return JS_TRUE;
	JL_BAD;
}

// (TBD) use these compilation options: PNG_SETJMP_NOT_SUPPORTED, PNG_NO_CONSOLE_IO, PNG_NO_STDIO, ...

CONFIGURE_STATIC

	REVISION(JL_SvnRevToInt("$Revision$"))
	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION( DecodePngImage )
		FUNCTION( EncodePngImage )
		FUNCTION( DecodeJpegImage )
//		FUNCTION( EncodeJpegImage )
	END_STATIC_FUNCTION_SPEC

END_STATIC
