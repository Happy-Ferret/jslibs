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

#include "image.h"

#include "../common/jsNativeInterface.h"

#include "png.h"

#include <stdlib.h>


typedef struct {
	png_structp png;
	png_infop info;
	JSContext *cx;
	JSObject *obj;
	NIStreamRead read;
} PngDescriptor;


void _png_read( png_structp png_ptr, png_bytep data, png_size_t length ) {

	PngDescriptor *desc = (PngDescriptor*)png_get_io_ptr(png_ptr);
	desc->read(desc->cx, desc->obj, NULL, (char*)data, &length);
//	if ( length == 0 )
//		png_error(png_ptr, "Trying to read after EOF."); // png_warning()
}


BEGIN_CLASS( Png )


DEFINE_FINALIZE() {

	PngDescriptor *desc = (PngDescriptor*)JS_GetPrivate(cx, obj);
	if ( desc != NULL ) {

		png_destroy_read_struct(&desc->png, &desc->info, NULL);
		free(desc);
	}
}

DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();
	RT_ASSERT_ARGC(1);

	PngDescriptor *desc = (PngDescriptor*)malloc(sizeof(PngDescriptor));
	RT_ASSERT_ALLOC(desc);
	JS_SetPrivate(cx, obj, desc);

	desc->png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
	RT_ASSERT( desc->png != NULL, "Unable to png_create_read_struct.");
	desc->info = png_create_info_struct(desc->png);
	RT_ASSERT( desc->info != NULL, "Unable to png_create_info_struct.");

	void *tmp;
	GetNativeInterface(cx, JSVAL_TO_OBJECT(argv[0]), NI_STREAM_READ, (FunctionPointer*)&desc->read, &tmp);
	RT_ASSERT( desc->read != NULL, "Unable to GetNativeResource." );

	png_set_read_fn( desc->png, (voidp)desc, _png_read );
   png_read_info(desc->png, desc->info);
	RT_ASSERT( desc->info->height <= PNG_UINT_32_MAX/png_sizeof(png_bytep), "Image is too high to process with png_read_png()");

	RT_ASSERT( png_set_interlace_handling(desc->png) == 1, "Cannot read interlaced image yet." );
	return JS_TRUE;
}


DEFINE_FUNCTION( Load ) {

	PngDescriptor *desc = (PngDescriptor*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( desc );

	desc->cx = cx;
	desc->obj = ???

	png_set_strip_16(desc->png);
	png_set_packing(desc->png);
	if ((desc->png->bit_depth < 8) || (desc->png->color_type == PNG_COLOR_TYPE_PALETTE) || (png_get_valid(desc->png, desc->info, PNG_INFO_tRNS)))
		png_set_expand(desc->png);
   png_read_update_info(desc->png, desc->info); // optional call to update the users info_ptr structure

	png_uint_32 width = png_get_image_width(desc->png, desc->info);
	png_uint_32 height = png_get_image_height(desc->png, desc->info);
	png_uint_32 bytePerRow = png_get_rowbytes(desc->png, desc->info);
	png_byte channels = png_get_channels(desc->png, desc->info);

// read & store

	png_bytep data = (png_bytep)malloc(height * bytePerRow);
	RT_ASSERT_ALLOC(data);
	JSObject *image = NewImage(cx, width, height, channels, data);
	*rval = OBJECT_TO_JSVAL(image);

// int number_of_passes = png_set_interlace_handling(desc->png);
// for ( int p = 0; p < number_of_passes; p++ )
	for( unsigned int y = 0; y < height; ++y ) {

		png_read_row(desc->png, data, png_bytep_NULL);
		data += bytePerRow;
	}
   png_read_end(desc->png, desc->info); // read rest of file, and get additional chunks in desc->info - REQUIRED

	return JS_TRUE;
}


//	JSBool Call(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
//		return JS_TRUE;
//	}

DEFINE_PROPERTY( width ) {

	PngDescriptor *desc = (PngDescriptor*) JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( desc );
	png_uint_32 width = png_get_image_width(desc->png, desc->info);
	J_S_ASSERT( width != 0, "Invalid width." );
	*vp = INT_TO_JSVAL(width);
	return JS_TRUE;
}

DEFINE_PROPERTY( height ) {

	PngDescriptor *desc = (PngDescriptor*) JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( desc );
	png_uint_32 height = png_get_image_height(desc->png, desc->info);
	J_S_ASSERT( width != 0, "Invalid height." );
	*vp = INT_TO_JSVAL(height);
	return JS_TRUE;
}

DEFINE_PROPERTY( channels ) {

	PngDescriptor *desc = (PngDescriptor*) JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( desc );
	png_uint_32 value = png_get_channels(desc->png, desc->info);
	J_S_ASSERT( value != 0, "Invalid channel count." );
	*vp = INT_TO_JSVAL(value);
	return JS_TRUE;
}

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
