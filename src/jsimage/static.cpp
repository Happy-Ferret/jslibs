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
$SVN_REVISION $Revision: 3533 $
**/
BEGIN_STATIC



//////////////////////////////////////////////////////////////////////////////
//// JPEG

// doc: http://uw714doc.sco.com/en/jpeg/libjpeg.txt


#define INPUT_BUF_SIZE 4096

typedef struct {
	struct jpeg_source_mgr pub;	// public fields
	JOCTET *buffer;
	JSContext *cx;
	JSObject *obj;
} SourceMgr;


METHODDEF(void) init_source(j_decompress_ptr cinfo) {
	
	JL_IGNORE(cinfo);
}

METHODDEF(boolean) fill_input_buffer(j_decompress_ptr cinfo) {

	SourceMgr * src = (SourceMgr *) cinfo->src;
	size_t amount = INPUT_BUF_SIZE;
	NIStreamRead streamRead = StreamReadInterface( src->cx, src->obj );
	if ( streamRead == NULL )
		return false;
	streamRead(src->cx, src->obj, (char*)src->buffer, &amount);

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

	SourceMgr * src = (SourceMgr *) cinfo->src;

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
	
	JL_IGNORE(cinfo);
}


struct JpegErrorManager {
  struct jpeg_error_mgr pub;
  JSContext *cx;
  jmp_buf setjmp_buffer;
};

METHODDEF(void)
JpegErrorExit(j_common_ptr cinfo) {

	JpegErrorManager *myerr = (JpegErrorManager*)cinfo->err;
	longjmp(myerr->setjmp_buffer, 1);
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
DEFINE_FUNCTION( decodeJpegImage ) {

	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_OBJECT(1);

	// see: jslibs/libs/libjpeg/src/example.c

	jpeg_decompress_struct cinfo;
	JpegErrorManager jerr;
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = JpegErrorExit;
	
	int longJumpValue;

	#ifdef _MSC_VER
	#pragma warning( push, 0 )
	#pragma warning(disable : 4611) // warning C4611: interaction between '_setjmp' and C++ object destruction is non-portable
	#endif // _MSC_VER

	longJumpValue = setjmp(jerr.setjmp_buffer);

	#ifdef _MSC_VER
	#pragma warning( pop )
	#endif // _MSC_VER

	if ( longJumpValue ) {

		char message[JMSG_LENGTH_MAX];
		cinfo.err->format_message((j_common_ptr)&cinfo, message);
		jpeg_destroy_decompress(&cinfo);
		JL_ERR( E_LIB, E_STR("JPEG"), E_INTERNAL, E_DETAILS, E_STR(message) );
	}

	jpeg_create_decompress(&cinfo);

// alloc reader structure & buffer ( both are freed by jpeg_destroy_decompress )
	cinfo.src = (struct jpeg_source_mgr*)cinfo.mem->alloc_small((j_common_ptr) &cinfo, JPOOL_PERMANENT, sizeof(SourceMgr));
	SourceMgr * src = (SourceMgr *)cinfo.src;

//	src->buffer = (JOCTET *)(*cinfo.mem->alloc_small) ((j_common_ptr) &cinfo, JPOOL_PERMANENT, INPUT_BUF_SIZE * sizeof(JOCTET));
	JOCTET buffer[INPUT_BUF_SIZE];
	src->buffer = buffer;

// setup reader structure
	src = (SourceMgr *) cinfo.src;
	src->pub.init_source = init_source;
	src->pub.fill_input_buffer = fill_input_buffer;
	src->pub.skip_input_data = skip_input_data;
	src->pub.resync_to_restart = jpeg_resync_to_restart; // use default method
	src->pub.term_source = term_source;

	src->pub.bytes_in_buffer = 0; // forces fill_input_buffer on first read
	src->pub.next_input_byte = NULL; // until buffer loaded

	src->obj = JSVAL_TO_OBJECT( JL_ARG(1) ); // required by NIStreamRead
	src->cx = cx; // required by NIStreamRead

// try to use a fast way to read the data
//	JL_CHK( GetStreamReadInterface(cx, JSVAL_TO_OBJECT(argv[0]), &src->read) );

//	JL_ASSERT( src->read != NULL, "Unable to GetNativeResource." );

// else use a 'classic' method to read the data ( like var data = resourceObject.Read(amount); )
//	if ( src->read == NULL ) {

//		JL_ASSERT( false, "TO BE DONE" ); // (TBD) read without NI_READ_RESOURCE
//		CxObj
//		src->pv = resourceObject;
//		src->read = ReadUsingJsMethod;
//	}

//	JSBool status = GetNativeResource(cx, JSVAL_TO_OBJECT(argv[0]), &src->pv, &src->read, NULL );
//	JL_ASSERT( status == JS_TRUE, "Unable to GetNativeResource." );

// read image headers
	jpeg_read_header(&cinfo, TRUE); //we passed TRUE to reject a tables-only JPEG file as an error.

	// the default is to produce full color output from a color file.

	jpeg_calc_output_dimensions(&cinfo);

// read the image
	jpeg_start_decompress(&cinfo);

	int width, height, bytePerRow, channels, size, length;
	width = cinfo.output_width;
	height = cinfo.output_height;
	bytePerRow = cinfo.output_width * cinfo.output_components;
	size = height * bytePerRow;
	channels = cinfo.output_components;

	length = height * bytePerRow;
	JOCTET * data;
	data = (JOCTET *)JL_NewImageObject(cx, width, height, channels, TYPE_UINT8, JL_RVAL);
	JL_CHK( data );

	// cinfo->rec_outbuf_height : recomanded scanline height ( 1, 2 or 4 )
	while (cinfo.output_scanline < cinfo.output_height) {

		jpeg_read_scanlines(&cinfo, &data, 1);
		data += bytePerRow;
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	//jpeg_destroy((j_common_ptr)&cinfo);

	return JS_TRUE;
	JL_BAD;
}



#define INITIAL_OUTPUT_BUF_SIZE 8192

struct jpegDestination {
  jpeg_destination_mgr pub; // public fields
  JOCTET *buffer;
  size_t bufferLength;
  size_t dataLength;
};

METHODDEF(void) jpegDestInit( j_compress_ptr cinfo ) {

	jpegDestination *dest = (jpegDestination*)cinfo->dest;

	dest->bufferLength = INITIAL_OUTPUT_BUF_SIZE;
	dest->buffer = (JOCTET*)jl_malloc(dest->bufferLength);

	dest->pub.next_output_byte = dest->buffer;
	dest->pub.free_in_buffer = dest->bufferLength;
}

METHODDEF(boolean) jpegDestEmptyBuffer( j_compress_ptr cinfo ) {

	// beware:
	//  In typical applications, this should write the entire output buffer (ignoring the current state of next_output_byte & free_in_buffer),
	//  reset the pointer & count to the start of the buffer, and return TRUE indicating that the buffer has been dumped.

	jpegDestination *dest = (jpegDestination*)cinfo->dest;
	
	size_t length = dest->bufferLength;
	dest->bufferLength = length * 2;
	dest->buffer = (JOCTET*)jl_realloc(dest->buffer, dest->bufferLength);
	if ( dest->buffer == NULL )
		return FALSE;

	dest->pub.next_output_byte = dest->buffer + length;
	dest->pub.free_in_buffer = dest->bufferLength - length;

	return TRUE;
}

METHODDEF(void) jpegDestTerm( j_compress_ptr cinfo ) {

	jpegDestination *dest = (jpegDestination*)cinfo->dest;

	dest->dataLength = dest->pub.next_output_byte - dest->buffer;

	if ( JL_MaybeRealloc(dest->bufferLength, dest->dataLength) )
		dest->buffer = (JOCTET*)jl_realloc(dest->buffer, dest->dataLength);
}


DEFINE_FUNCTION( encodeJpegImage ) {

	jpegDestination dest;
	dest.buffer = NULL;
	jpeg_compress_struct cinfo;
	cinfo.global_state = 0;

	JL_ASSERT_ARGC_RANGE(1, 2);
	JL_ASSERT_ARG_IS_OBJECT(1);

	int quality;
	if ( JL_ARG_ISDEF(2) ) {

		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &quality) );
		JL_ASSERT_ARG_VAL_RANGE(quality, 0, 100, 2);
	} else {
	
		quality = -1; // use default. see below.
	}

	JpegErrorManager jerr;
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = JpegErrorExit;

	// error management

	int longJumpValue;

	#ifdef _MSC_VER
	#pragma warning( push, 0 )
	#pragma warning(disable : 4611) // warning C4611: interaction between '_setjmp' and C++ object destruction is non-portable
	#endif // _MSC_VER

	longJumpValue = setjmp(jerr.setjmp_buffer);

	#ifdef _MSC_VER
	#pragma warning( pop )
	#endif // _MSC_VER

	if ( longJumpValue ) {

		char message[JMSG_LENGTH_MAX];
		cinfo.err->format_message((j_common_ptr)&cinfo, message);
		JL_ERR( E_LIB, E_STR("JPEG"), E_INTERNAL, E_DETAILS, E_STR(message) );
	}

	// initialization

	jpeg_create_compress(&cinfo);

	cinfo.dest = (jpeg_destination_mgr*)&dest;

	dest.pub.init_destination = jpegDestInit;
	dest.pub.empty_output_buffer = jpegDestEmptyBuffer;
	dest.pub.term_destination = jpegDestTerm;

	// setup source data

	const JSAMPLE *sImageData;

	{
	ImageDataType dataType;
	JLData buffer = JL_GetImageObject(cx, JL_ARG(1), &cinfo.image_width, &cinfo.image_height, &cinfo.input_components, &dataType); // source
	JL_ASSERT( dataType == TYPE_UINT8, E_ARG, E_NUM(1), E_DATATYPE, E_INVALID );
	ASSERT( buffer.IsSet() );
	sImageData = (const JSAMPLE*)buffer.GetConstStr();
	}

	JL_ASSERT( cinfo.input_components == 1 || cinfo.input_components == 3, E_STR("image"), E_FORMAT );

	cinfo.in_color_space = cinfo.input_components == 3 ? JCS_RGB : JCS_GRAYSCALE;

	jpeg_set_defaults(&cinfo);
  
	if ( quality >= 0 ) {

		// The quality value ranges from 0..100. If "force_baseline" is TRUE, the computed quantization table entries are limited to 1..255 for JPEG baseline compatibility.
		jpeg_set_quality(&cinfo, quality, TRUE);
	}

	// compress

	jpeg_start_compress(&cinfo, TRUE);

	int rowStride;
	rowStride = cinfo.image_width * cinfo.input_components;

	while ( cinfo.next_scanline < cinfo.image_height ) {

		// jpeg_write_scanlines expects an array of pointers to scanlines. Here the array is only one element long,
		// but you could pass more than one scanline at a time if that's more convenient.
		if ( jpeg_write_scanlines(&cinfo, const_cast<JSAMPARRAY>(&sImageData), 1) != 1 )
			break;
		sImageData += rowStride;
	}

	jpeg_finish_compress(&cinfo);

	// store compressed data

	JL_CHK( JL_NewBufferGetOwnership(cx, dest.buffer, dest.dataLength, JL_RVAL) );

	jpeg_destroy_compress(&cinfo);
	return JS_TRUE;

bad:
	if ( dest.buffer != NULL )
		jl_free(dest.buffer);
	if ( cinfo.global_state != 0 )
		jpeg_destroy_compress(&cinfo);
	return JS_FALSE;
}




//////////////////////////////////////////////////////////////////////////////
//// PNG



struct PngReadUserStruct {

	png_structp png;
	png_infop info;
	JSContext *cx;
	JSObject *obj;
};


void _png_read( png_structp png_ptr, png_bytep data, png_size_t length ) {

	PngReadUserStruct *desc = (PngReadUserStruct*)png_get_io_ptr(png_ptr);
//	desc->read(desc->cx, desc->obj, (char*)data, &length);

	NIStreamRead streamRead = StreamReadInterface(desc->cx, desc->obj);

	ASSERT( streamRead );
	if ( streamRead == NULL )
		return;

	streamRead(desc->cx, desc->obj, (char*)data, &length);

//	if ( length == 0 )
//		png_error(png_ptr, "Trying to read after EOF."); // png_warning()
}


NOALIAS png_voidp
PngMalloc(png_structp, png_size_t size) NOTHROW {

	return jl_malloc(size);
}

void
PngFree(png_structp, png_voidp ptr) NOTHROW {

	jl_free(ptr);
}

static void
PngError(png_structp png_ptr, png_const_charp message) {

	JSContext *cx = (JSContext*)png_get_error_ptr(png_ptr);
	JL_ERR( E_LIB, E_STR("PNG"), E_INTERNAL, E_STR(message) );
bad:
	longjmp(png_ptr->jmpbuf, 1);
}

static void
PngWarning(png_structp png_ptr, png_const_charp message) {

	JSContext *cx = (JSContext*)png_get_error_ptr(png_ptr);
	JL_WARN( E_STR(message) );
bad:
	longjmp(png_ptr->jmpbuf, 1);
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
DEFINE_FUNCTION( decodePngImage ) {

	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_OBJECT(1);

	PngReadUserStruct desc;

	desc.png = png_create_read_struct_2(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL, NULL, PngMalloc, PngFree);
	JL_ASSERT( desc.png != NULL, E_LIB, E_STR("PNG"), E_INTERNAL, E_COMMENT("png_create_read_struct") );

	png_set_error_fn(desc.png, (png_voidp)cx, PngError, PngWarning);

	desc.info = png_create_info_struct(desc.png);
	JL_ASSERT( desc.info != NULL, E_LIB, E_STR("PNG"), E_INTERNAL, E_COMMENT("png_create_info_struct") );

	int longJumpValue;

	#ifdef _MSC_VER
	#pragma warning( push, 0 )
	#pragma warning(disable : 4611) // warning C4611: interaction between '_setjmp' and C++ object destruction is non-portable
	#endif // _MSC_VER

	longJumpValue = setjmp(png_jmpbuf(desc.png));

	#ifdef _MSC_VER
	#pragma warning( pop )
	#endif // _MSC_VER

	if ( longJumpValue ) {

        png_destroy_read_struct(&desc.png, &desc.info, NULL);
        goto bad;
    }


//	JL_CHK( GetStreamReadInterface(cx, JSVAL_TO_OBJECT(argv[0]), &desc.read) );
//	JL_ASSERT( desc.read != NULL, "Unable to GetNativeResource." );

	desc.obj = JSVAL_TO_OBJECT( JL_ARG(1) );
	desc.cx = cx;

	png_set_read_fn( desc.png, (voidp)&desc, _png_read );
	png_read_info(desc.png, desc.info);
	JL_ASSERT( desc.info->height <= PNG_UINT_32_MAX / png_sizeof(png_bytep), E_NAME("height"), E_MAX, E_NUM(PNG_UINT_32_MAX / png_sizeof(png_bytep)) );
	
	JL_CHKM( png_set_interlace_handling(desc.png) == 1, E_ARG, E_NUM(1), E_NOTSUPPORTED, E_COMMENT("Interleaved") );

// Load
	png_set_strip_16(desc.png);
	png_set_packing(desc.png);
	if ( (desc.png->bit_depth < 8) || (desc.png->color_type == PNG_COLOR_TYPE_PALETTE) || (png_get_valid(desc.png, desc.info, PNG_INFO_tRNS)) ) {

		png_set_expand(desc.png);
	}

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
	data = (png_bytep)JL_NewImageObject(cx, width, height, channels, TYPE_UINT8, JL_RVAL);
	JL_CHK( data );

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
	jl::memcpy((char*)desc->buffer + desc->pos, data, size);
	desc->pos += size;
}

void output_flush_fn(png_structp png_ptr) {

	JL_IGNORE(png_ptr);
}

/**doc
$TOC_MEMBER $INAME
 $TYPE data $INAME( imageObject [, compressionLevel = default] )
**/
DEFINE_FUNCTION( encodePngImage ) {

	PngWriteUserStruct desc;
	desc.buffer = NULL; // see bad:
	JLData buffer;

	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT_ARG_IS_OBJECT(1);

	int compressionLevel;
	if ( JL_ARG_ISDEF(2) ) {

		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &compressionLevel) );
		JL_ASSERT_ARG_VAL_RANGE( compressionLevel, 0, 9, 2 );
	} else {

		compressionLevel = Z_DEFAULT_COMPRESSION;
	}


	int sWidth, sHeight, sChannels;
	ImageDataType dataType;
	buffer = JL_GetImageObject(cx, JL_ARG(1), &sWidth, &sHeight, &sChannels, &dataType); // source
	JL_ASSERT( dataType == TYPE_UINT8, E_ARG, E_NUM(1), E_DATATYPE, E_INVALID );

	desc.buffer = JL_DataBufferAlloc(cx, sWidth * sHeight * sChannels + 1024); // destination
	JL_ASSERT_ALLOC( desc.buffer );
	desc.pos = 0;

	desc.png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	JL_ASSERT( desc.png != NULL, E_LIB, E_STR("PNG"), E_INTERNAL, E_COMMENT("png_create_write_struct") );
	desc.info = png_create_info_struct(desc.png);
	JL_ASSERT( desc.info != NULL, E_LIB, E_STR("PNG"), E_INTERNAL, E_COMMENT("png_create_info_struct") );

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
			JL_ERR( E_NAME("channels"), E_RANGE, E_INTERVAL_NUM(1, 4) );
	}

	png_set_IHDR(desc.png, desc.info, sWidth, sHeight, 8, color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_set_compression_level(desc.png, compressionLevel);

	png_write_info(desc.png, desc.info);
	for ( int r = 0; r < sHeight; r++ )
		png_write_row(desc.png, (png_bytep)buffer.GetConstStr() + sWidth * r * sChannels);
	png_write_end(desc.png, desc.info);

	png_destroy_write_struct(&desc.png, &desc.info);

	// (TBD) use maybeRealloc here ?
	desc.buffer = (void*)JL_DataBufferRealloc(cx, (uint8_t*)desc.buffer, desc.pos); // usually, compressed data is smaller that original one.
	JL_ASSERT_ALLOC( desc.buffer );
	JL_CHK( JL_NewBufferGetOwnership(cx, desc.buffer, desc.pos, JL_RVAL) );
	return JS_TRUE;
bad:
	JL_DataBufferFree(cx, (uint8_t*)desc.buffer);
	return JS_FALSE;
}

// (TBD) use these compilation options: PNG_SETJMP_NOT_SUPPORTED, PNG_NO_CONSOLE_IO, PNG_NO_STDIO, ...

CONFIGURE_STATIC

	REVISION(jl::SvnRevToInt("$Revision: 3533 $"))
	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION_ARGC( decodePngImage, 1 )
		FUNCTION_ARGC( encodePngImage, 2 )
		FUNCTION_ARGC( decodeJpegImage, 1 )
		FUNCTION_ARGC( encodeJpegImage, 2 )
	END_STATIC_FUNCTION_SPEC

END_STATIC
