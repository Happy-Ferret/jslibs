#include "stdafx.h"
#include "jsimage.h"

#include "png.h"

#include <stdlib.h>
#include <memory.h>

#include "../smtools/nativeresource.h"


typedef struct {
	png_structp png;
	png_infop info;
	void *pv;
	NativeResourceFunction read;
} PngDescriptor;


void _png_read( png_structp png_ptr, png_bytep data, png_size_t length ) {
	
	PngDescriptor *desc = (PngDescriptor*)png_get_io_ptr(png_ptr);
	desc->read(desc->pv, data, &length);
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

DEFINE_FUNCTION( ClassConstruct ) {

	RT_ASSERT_CONSTRUCTING(thisClass);
	RT_ASSERT_ARGC(1);

	PngDescriptor *desc = (PngDescriptor*)malloc(sizeof(PngDescriptor));
	RT_ASSERT_ALLOC(desc);
	JS_SetPrivate(cx, obj, desc);

	desc->png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
	RT_ASSERT( desc->png != NULL, "Unable to png_create_read_struct.");
	desc->info = png_create_info_struct(desc->png);
	RT_ASSERT( desc->info != NULL, "Unable to png_create_info_struct.");

	JSBool status = GetNativeResource(cx, JSVAL_TO_OBJECT(argv[0]), &desc->pv, &desc->read, NULL );
	RT_ASSERT( status == JS_TRUE, "Unable to GetNativeResource." );
	
	png_set_read_fn( desc->png, (voidp)desc, _png_read );
   png_read_info(desc->png, desc->info);
	RT_ASSERT( desc->info->height <= PNG_UINT_32_MAX/png_sizeof(png_bytep), "Image is too high to process with png_read_png()");

	RT_ASSERT( !desc->png->interlaced, "Cannot read interlaced image." );
	return JS_TRUE;
}


DEFINE_FUNCTION( Load ) {


	PngDescriptor *desc = (PngDescriptor*)JS_GetPrivate(cx, obj);
	RT_ASSERT( desc != NULL, RT_ERROR_NOT_INITIALIZED );


	png_set_strip_16(desc->png);
	png_set_packing(desc->png);
	if ((desc->png->bit_depth < 8) || (desc->png->color_type == PNG_COLOR_TYPE_PALETTE) || (png_get_valid(desc->png, desc->info, PNG_INFO_tRNS)))
		png_set_expand(desc->png);
   png_read_update_info(desc->png, desc->info); // optional call to update the users info_ptr structure

	png_uint_32 height = png_get_image_height(desc->png, desc->info);
	png_uint_32 bytePerRow = png_get_rowbytes(desc->png, desc->info);

// read & store
	unsigned int size = height * bytePerRow;
	png_bytep data = (png_bytep)JS_malloc(cx, height * bytePerRow);
	png_bytep buffer = data;

	for( unsigned int y = 0; y < height; ++y ) {

		png_read_row(desc->png, buffer, png_bytep_NULL);
		buffer += bytePerRow;
	}

   png_read_end(desc->png, desc->info); // read rest of file, and get additional chunks in desc->info - REQUIRED

// return
	JSString *str = JS_NewString( cx, (char*)data, size );
	RT_ASSERT( str != NULL, "Unable to create the inage string." );
	if ( str != NULL )
		JS_free(cx, str);
	*rval = STRING_TO_JSVAL(str); // GC protection is ok with this ?
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

