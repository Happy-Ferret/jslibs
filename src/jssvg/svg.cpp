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
#include "svg.h"
#include <libxml/xmlerror.h>


/**doc
$CLASS_HEADER
**/
BEGIN_CLASS( SVG ) // Start the definition of the class. It defines some symbols: _name, _class, _prototype

DEFINE_FINALIZE() { // called when the Garbage Collector is running if there are no remaing references to this object.

	RsvgHandle *handle = (RsvgHandle*)JS_GetPrivate(cx, obj);

	if ( handle ) {

		g_object_unref(handle);
	}
}


/**doc
 * $INAME()
**/
DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();

	RsvgHandle *handle;
	handle = rsvg_handle_new();

	J_S_ASSERT_RESOURCE(handle);

	J_CHK( JS_SetPrivate(cx, obj, handle) );


	return JS_TRUE;
}


/**doc
=== Methods ===
**/

/**doc
 * $THIS $INAME()
**/
DEFINE_FUNCTION_FAST( Write ) {

	RsvgHandle *handle = (RsvgHandle*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(handle);

	const char *data;
	size_t length;
	J_CHK( JsvalToStringAndLength(cx, &J_FARG(1), &data, &length) );

	GError *error = NULL;
	gboolean status;

//		rsvg_handle_new_from_file
//	xmlResetLastError();

	status = rsvg_handle_write(handle, (const guchar *)data, length, &error);
	if ( !status ) {

		xmlErrorPtr xmlErr = xmlGetLastError();
		if ( xmlErr != NULL ) { // XML error
			J_REPORT_ERROR_2("SVG error: %s. %s", error->message, xmlErr->message);
		} else
			J_REPORT_ERROR_1("SVG error: %s", error->message);
	}

	status = rsvg_handle_close(handle, &error);
	if (!status) {

		J_REPORT_ERROR_1("SVG error: %s", error->message);
	}
	
	return JS_TRUE;
}


/**doc
 * $THIS $INAME( [ imageWidth , imageHeight [ [ , surfaceWidth, surfaceHeight ] , id] ] )
  $H arguments
   $ARG integer imageWidth
   $ARG integer imageHeight
   $ARG integer surfaceWidth
   $ARG integer surfaceHeight
   $ARG string id: Draws a subset of a SVG starting from an element's id. For example, if you have a layer called "layer1" that you wish to render, pass "#layer1" as the id.
**/
DEFINE_FUNCTION_FAST( RenderImage ) { // using cairo

	RsvgHandle *handle = (RsvgHandle*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(handle);

	RsvgDimensionData dim;
	rsvg_handle_get_dimensions(handle, &dim);

	cairo_matrix_t tr = { // identity matrix
		1, 0, // x
		0, 1, // y
		0, 0 // translation
	};

	size_t imageWidth, imageHeight;
	if ( J_FARG_ISDEF(1) && J_FARG_ISDEF(2) ) {

		J_CHK( JsvalToUInt(cx, J_FARG(1), &imageWidth) );
		J_CHK( JsvalToUInt(cx, J_FARG(2), &imageHeight) );

		tr.xx = (double)imageWidth / (double)dim.width;
		tr.yy = (double)imageHeight / (double)dim.height;
	} else {

		imageWidth = dim.width;
		imageHeight = dim.height;
	}

	size_t surfaceWidth, surfaceHeight;	
	if ( J_FARG_ISDEF(3) && J_FARG_ISDEF(4) ) {

		J_CHK( JsvalToUInt(cx, J_FARG(3), &surfaceWidth) );
		J_CHK( JsvalToUInt(cx, J_FARG(4), &surfaceHeight) );
	} else {

		surfaceWidth = imageWidth;
		surfaceHeight = imageHeight;
	}

	const char *id;
	if ( J_FARG_ISDEF(5) ) {

		J_CHK( JsvalToString(cx, &J_FARG(5), &id) );
		J_S_ASSERT( id != NULL && *id == '#', "Invalid id." );
	} else {

		id = NULL;
	}

	// CAIRO_FORMAT_ARGB32: Pre-multiplied alpha is used. (That is, 50% transparent red is 0x80800000, not 0x80ff0000.)
	cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, surfaceWidth, surfaceHeight);
	J_S_ASSERT( cairo_surface_status(surface) == CAIRO_STATUS_SUCCESS, "Unable to create cairo surface." );

	cairo_t *cr = cairo_create(surface);
	J_S_ASSERT( cairo_status(cr) == CAIRO_STATUS_SUCCESS, "Unable to create cairo state." );

//	cairo_set_antialias(cr, CAIRO_ANTIALIAS_SUBPIXEL);
//	cairo_set_antialias(cr, CAIRO_ANTIALIAS_GRAY);
	
	cairo_set_matrix(cr, &tr);
	gboolean status = rsvg_handle_render_cairo_sub(handle, cr, id);
	J_S_ASSERT( status == TRUE, "Unable to rsvg_handle_render_cairo." );

	cairo_format_t surfaceFormat = cairo_image_surface_get_format(surface);
	
	int channels;
	switch ( surfaceFormat ) {
		case CAIRO_FORMAT_ARGB32:
		case CAIRO_FORMAT_RGB24:
			channels = 4;
			break;
		case CAIRO_FORMAT_A8:
			channels = 1;
			break;
		default:
			J_REPORT_ERROR( "Invalid format." ); // see. cairo_format_t
	}

	int width = cairo_image_surface_get_width(surface);
	int height = cairo_image_surface_get_height(surface);
	void *buffer = cairo_image_surface_get_data(surface);

	size_t pixelCount = width * height;
	size_t length = pixelCount * channels;

	void *image = JS_malloc(cx, length);
	J_S_ASSERT_ALLOC(image);

	switch ( surfaceFormat ) {
		case CAIRO_FORMAT_ARGB32:
			// cancel Pre-multiplied Alpha ?
		case CAIRO_FORMAT_RGB24:
			for ( size_t i = 0; i < pixelCount; i++ ) { // 3 2 1 4  ->  1 2 3 4

				u_int32_t p = ((u_int32_t*)buffer)[i];
				((u_int32_t*)image)[i] = p & 0xff00ff00 | _rotl(p, 16) & 0x00ff00ff; // use SSE2 instructions ?
			}
			break;
		case CAIRO_FORMAT_A8:
			memcpy(image, buffer, length);
			break;
	}

	cairo_destroy(cr);
	cairo_surface_destroy(surface);


	jsval blobVal;
	J_CHK( J_NewBlob(cx, image, length, &blobVal) );

	JSObject *blobObj;
	J_CHK( JS_ValueToObject(cx, blobVal, &blobObj) );
	J_S_ASSERT( blobObj, "Unable to create Blob object." );
	*J_FRVAL = OBJECT_TO_JSVAL(blobObj);

	J_CHK( JS_DefineProperty(cx, blobObj, "channels", INT_TO_JSVAL(channels), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT ) );
	J_CHK( JS_DefineProperty(cx, blobObj, "width", INT_TO_JSVAL(width), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT ) );
	J_CHK( JS_DefineProperty(cx, blobObj, "height", INT_TO_JSVAL(height), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT ) );

	return JS_TRUE;
}


/*
DEFINE_FUNCTION_FAST( GetImage ) { // using pixbuf

	RsvgHandle *handle = (RsvgHandle*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(handle);

	GdkPixbuf *pb = rsvg_handle_get_pixbuf(handle);

	J_S_ASSERT( pb != NULL, "Insufficient data has been read to create the pixbuf." );
	J_S_ASSERT( gdk_pixbuf_get_bits_per_sample(pb) == 8, "Unsupported bits_per_sample." );

	int width = gdk_pixbuf_get_width(pb);
	int height = gdk_pixbuf_get_height(pb);
	int channels = gdk_pixbuf_get_n_channels(pb);
	void *buffer = gdk_pixbuf_get_pixels(pb);

	jsval blobVal;
	J_CHK( J_NewBlob(cx, buffer, width*height*channels, &blobVal) ); // (TBD) copy the buffer before !!

	JSObject *blobObj;
	J_CHK( JS_ValueToObject(cx, blobVal, &blobObj) );
	J_S_ASSERT( blobObj, "Unable to create Blob object." );
	*J_FRVAL = OBJECT_TO_JSVAL(blobObj);

	J_CHK( JS_DefineProperty(cx, blobObj, "channels", INT_TO_JSVAL(channels), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT ) );
	J_CHK( JS_DefineProperty(cx, blobObj, "width", INT_TO_JSVAL(width), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT ) );
	J_CHK( JS_DefineProperty(cx, blobObj, "height", INT_TO_JSVAL(height), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT ) );

	return JS_TRUE;
}
*/



/**doc
=== Properties ===
**/

/**doc
 * $INAME $READONLY
**/
DEFINE_PROPERTY(dpi) {

	RsvgHandle *handle = (RsvgHandle*)JS_GetPrivate(cx, J_OBJ);
	J_S_ASSERT_RESOURCE(handle);
	if ( JSVAL_IS_VOID(*vp) ) {

		rsvg_handle_set_dpi(handle, -1);
	} else
	if ( JSVAL_IS_NUMBER(*vp) ) {

		size_t dpi;
		J_CHK( JsvalToUInt(cx, *vp, &dpi) );
		rsvg_handle_set_dpi(handle, dpi);
	} else
	if ( JsvalIsArray(cx, *vp) ) {

		size_t dpiX, dpiY;
		jsval tmp;
		J_CHK( JS_GetElement(cx, JSVAL_TO_OBJECT(*vp), 0, &tmp) );
		J_CHK( JsvalToUInt(cx, tmp, &dpiX) );
		J_CHK( JS_GetElement(cx, JSVAL_TO_OBJECT(*vp), 1, &tmp) );
		J_CHK( JsvalToUInt(cx, tmp, &dpiY) );
		rsvg_handle_set_dpi_x_y(handle, dpiX, dpiY);
	} else
		J_REPORT_ERROR("Invalid case.");
	return JS_TRUE;
}

/**doc
 * $INAME $READONLY
**/
DEFINE_PROPERTY(width) {

	RsvgHandle *handle = (RsvgHandle*)JS_GetPrivate(cx, J_OBJ);
	J_S_ASSERT_RESOURCE(handle);
	RsvgDimensionData dim;
	rsvg_handle_get_dimensions(handle, &dim);
	J_CHK( UIntToJsval(cx, dim.width, vp) );
	return JS_TRUE;
}

/**doc
 * $INAME $READONLY
**/
DEFINE_PROPERTY(height) {

	RsvgHandle *handle = (RsvgHandle*)JS_GetPrivate(cx, J_OBJ);
	J_S_ASSERT_RESOURCE(handle);
	RsvgDimensionData dim;
	rsvg_handle_get_dimensions(handle, &dim);
	J_CHK( UIntToJsval(cx, dim.height, vp) );
	return JS_TRUE;
}

/**doc
 * $INAME $READONLY
**/
DEFINE_PROPERTY(title) {

	RsvgHandle *handle = (RsvgHandle*)JS_GetPrivate(cx, J_OBJ);
	J_S_ASSERT_RESOURCE(handle);
	const char *title = rsvg_handle_get_title(handle);
	if ( title != NULL )
		J_CHK( StringToJsval(cx, title, vp) );
	else
		*vp = JSVAL_VOID;
	return JS_TRUE;
}

/**doc
 * $INAME $READONLY
**/
DEFINE_PROPERTY(metadata) {

	RsvgHandle *handle = (RsvgHandle*)JS_GetPrivate(cx, J_OBJ);
	J_S_ASSERT_RESOURCE(handle);
	const char *metadata = rsvg_handle_get_metadata(handle);
	if ( metadata != NULL )
		J_CHK( StringToJsval(cx, metadata, vp) );
	else
		*vp = JSVAL_VOID;
	return JS_TRUE;
}

/**doc
 * $INAME $READONLY
**/
DEFINE_PROPERTY(description) {

	RsvgHandle *handle = (RsvgHandle*)JS_GetPrivate(cx, J_OBJ);
	J_S_ASSERT_RESOURCE(handle);
	const char *description = rsvg_handle_get_desc(handle);
	if ( description != NULL )
		J_CHK( StringToJsval(cx, description, vp) );
	else
		*vp = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_INIT() {

	rsvg_init(); // see rsvg_term()
	return JS_TRUE;
}


CONFIGURE_CLASS // This section containt the declaration and the configuration of the class

	HAS_PRIVATE
	HAS_CONSTRUCTOR
	HAS_FINALIZE

	HAS_INIT

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST(Write)
		FUNCTION_FAST(RenderImage)
//		FUNCTION_FAST(GetImage)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_WRITE(dpi)
		PROPERTY_READ(width)
		PROPERTY_READ(height)
		PROPERTY_READ(title)
		PROPERTY_READ(metadata)
		PROPERTY_READ(description)
	END_PROPERTY_SPEC

END_CLASS
