#include "stdafx.h"
#include "jsimage.h"

#include "png.h"

#include <stdlib.h>
#include <memory.h>

#include "../smtools/nativeresource.h"


typedef struct {
	JSContext *cx;
	JSObject *obj;
} CxObj;


typedef struct {
	void *pv;
	NativeResourceFunction read;
} NativeResource;


typedef struct {
	png_structp png;
	png_infop info;
} PngDescriptor;


void _png_read( png_structp png_ptr, png_bytep data, png_size_t length ) {
	
	NativeResource *nnr = (NativeResource*)png_get_io_ptr(png_ptr);
	nnr->read(nnr->pv, data, &length);
//	if ( length == 0 )
//		png_error(png_ptr, "Trying to read after EOF."); // png_warning()
}


BEGIN_CLASS

	static void Finalize(JSContext *cx, JSObject *obj) {

		PngDescriptor *desc = (PngDescriptor*)JS_GetPrivate(cx, obj);
		if ( desc != NULL ) {
			
			png_destroy_read_struct(&desc->png, &desc->info, NULL);
			free(desc);
		}
	}

	static JSBool ClassConstruct(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

		RT_ASSERT_CONSTRUCTING(thisClass);
		RT_ASSERT_ARGC(1);
		PngDescriptor *desc = (PngDescriptor*)malloc(sizeof(PngDescriptor));
		RT_ASSERT_ALLOC(desc);
		JS_SetPrivate(cx, obj, desc);

		png_infop end_info;

//		desc->png = png_create_read_struct_2(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL, (png_voidp)this, ImagePng::_png_alloc, ImagePng::_png_free );
		desc->png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
		RT_ASSERT( desc->png != NULL, "Unable to png_create_read_struct.");

		desc->info = png_create_info_struct(desc->png);
		RT_ASSERT( desc->info != NULL, "Unable to png_create_info_struct.");

		end_info = png_create_info_struct(desc->png);

		NativeResource *res = (NativeResource*)malloc(sizeof(NativeResource)); // [TBD] free
		
		JSBool status = GetNativeResource(cx, JSVAL_TO_OBJECT(argv[0]), &res->pv, &res->read, NULL );
		RT_ASSERT( status == JS_TRUE, "Unable to GetNativeResource." );
		png_set_read_fn( desc->png, (voidp)res, _png_read );

		png_byte color_type = png_get_color_type(desc->png, desc->info);

		png_byte bit_depth = png_get_bit_depth(desc->png, desc->info);


/* nsPNGDecoder.cpp

	if (color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_expand(png_ptr);

  if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
    png_set_expand(png_ptr);

  if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
    png_get_tRNS(png_ptr, info_ptr, &trans, &num_trans, NULL);
    png_set_expand(png_ptr);
  }

  if (bit_depth == 16)
    png_set_strip_16(png_ptr);

  if (color_type == PNG_COLOR_TYPE_GRAY ||
      color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
      png_set_gray_to_rgb(png_ptr);

if (png_get_gAMA(png_ptr, info_ptr, &aGamma)) {
      if ((aGamma <= 0.0) || (aGamma > 21474.83)) {
          aGamma = 0.45455;
          png_set_gAMA(png_ptr, info_ptr, aGamma);
      }
      png_set_gamma(png_ptr, 2.2, aGamma);
  }
  else
      png_set_gamma(png_ptr, 2.2, 0.45455);

  // let libpng expand interlaced images
  if (interlace_type == PNG_INTERLACE_ADAM7) {
    // number_passes = 
    png_set_interlace_handling(png_ptr);
  }

*/

		if (color_type == PNG_COLOR_TYPE_PALETTE)
			png_set_palette_to_rgb(desc->png);

		if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) 
			png_set_gray_1_2_4_to_8(desc->png);

		if (png_get_valid(desc->png, desc->info, PNG_INFO_tRNS)) 
			png_set_tRNS_to_alpha(desc->png);

		if (bit_depth == 16)
			png_set_strip_16(desc->png);

		if (bit_depth < 8)
			png_set_packing(desc->png);


		png_read_update_info(desc->png, desc->info);

	
		return JS_TRUE;
	}

//	JSBool Call(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
//		return JS_TRUE;
//	}

DEFINE_PROPERTY( width ) {

	PngDescriptor *desc = (PngDescriptor*) JS_GetPrivate(cx, obj);
	RT_ASSERT( desc != NULL, RT_ERROR_NOT_INITIALIZED );
	png_uint_32 width = png_get_image_width(desc->png, desc->info);
	RT_ASSERT( width != 0, RT_ERROR_NOT_INITIALIZED );
	*vp = INT_TO_JSVAL(width);
	return JS_TRUE;
}

DEFINE_PROPERTY( height ) {

	PngDescriptor *desc = (PngDescriptor*) JS_GetPrivate(cx, obj);
	RT_ASSERT( desc != NULL, RT_ERROR_NOT_INITIALIZED );
	png_uint_32 height = png_get_image_height(desc->png, desc->info);
	RT_ASSERT( height != 0, RT_ERROR_NOT_INITIALIZED );
	*vp = INT_TO_JSVAL(height);
	return JS_TRUE;
}

DEFINE_PROPERTY( channels ) {

	PngDescriptor *desc = (PngDescriptor*) JS_GetPrivate(cx, obj);
	RT_ASSERT( desc != NULL, RT_ERROR_NOT_INITIALIZED );
	png_uint_32 value = png_get_channels(desc->png, desc->info);
	RT_ASSERT( value != 0, RT_ERROR_NOT_INITIALIZED );
	*vp = INT_TO_JSVAL(value);
	return JS_TRUE;
}



DEFINE_FUNCTION( Load ) {

	PngDescriptor *desc = (PngDescriptor*) JS_GetPrivate(cx, obj);
	RT_ASSERT( desc != NULL, RT_ERROR_NOT_INITIALIZED );

	png_uint_32 height = png_get_image_height(desc->png, desc->info);
	png_uint_32 bytePerRow = png_get_rowbytes(desc->png, desc->info);



//	png_read_image(desc->png,

/*
   row_pointers = png_malloc(png_ptr, height*png_sizeof(png_bytep));
   for (int i=0; i<height, i++)
      row_pointers[i]=png_malloc(png_ptr, width*pixel_size);
   png_set_rows(png_ptr, info_ptr, &row_pointers);
*/


// png_read_row(desc->png, png_bytep row, NULL); // [TBD]

	png_read_png(desc->png, desc->info, PNG_TRANSFORM_IDENTITY, NULL);
	png_bytep *row_pointers = png_get_rows(desc->png, desc->info); // cf. png_read_rows(png_ptr, row_pointers, NULL, number_of_rows);


	unsigned int size = height * bytePerRow;
	unsigned char *data = (unsigned char*)JS_malloc(cx, size);
	png_bytep buffer = (png_bytep)data;

	for( unsigned int y = 0; y < height; ++y ) {

		memcpy( data, row_pointers[y], bytePerRow );
		buffer += bytePerRow;
	}

	png_read_end(desc->png, desc->info);

	JSString *str = JS_NewString( cx, (char*)data, size );
	RT_ASSERT( str != NULL, "Unable to create the inage string." );

	*rval = STRING_TO_JSVAL(str); // GC protection is ok with this ?
	return JS_TRUE;
}


	BEGIN_FUNCTION_MAP
		FUNCTION(Load)
	END_MAP
	BEGIN_PROPERTY_MAP
		READONLY(width)
		READONLY(height)
		READONLY(channels)
	END_MAP
	NO_STATIC_FUNCTION_MAP
	//BEGIN_STATIC_FUNCTION_MAP
	//END_MAP
	NO_STATIC_PROPERTY_MAP
	//BEGIN_STATIC_PROPERTY_MAP
	//END_MAP
//	NO_CLASS_CONSTRUCT
	NO_OBJECT_CONSTRUCT
//	NO_FINALIZE
	NO_CALL
	NO_PROTOTYPE
	NO_CONSTANT_MAP
	NO_INITCLASSAUX

END_CLASS(Image, HAS_PRIVATE, NO_RESERVED_SLOT)

