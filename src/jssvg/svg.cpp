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
#include <rsvg-private.h>
#include "rsvg-structure.h"

//#include <libxml/xpath.h>
//#include <libxml/parser.h>

#include "svg.h"
#include <libxml/xmlerror.h>


struct CxObj {

	JSContext *cx;
	JSObject *obj;
};


JSBool RequestPixbufImage(JSContext *cx, JSObject *obj, const char *name, GdkPixbuf **pixbuf) {

	*pixbuf = NULL;
	jsval onImageFct, image;
	J_CHK( JS_GetProperty(cx, obj, "onImage", &onImageFct) );
	if ( JsvalIsFunction(cx, onImageFct) ) {

		jsval nameVal;
		J_CHK( StringToJsval(cx, name, &nameVal) );
		J_CHK( JL_CallFunction(cx, obj, onImageFct, &image, 1, nameVal) );
		if ( JSVAL_IS_OBJECT( image ) ) {

			JSObject *imageObj = JSVAL_TO_OBJECT( image );
			int sWidth, sHeight, sChannels;
			J_CHK( GetPropertyInt(cx, imageObj, "width", &sWidth) );
			J_CHK( GetPropertyInt(cx, imageObj, "height", &sHeight) );
			J_CHK( GetPropertyInt(cx, imageObj, "channels", &sChannels) );

			J_S_ASSERT_1( sChannels == 3 || sChannels == 4, "Unsupported image format for %s.", name );
			const char *sBuffer;
			size_t bufferLength;
			J_CHK( JsvalToStringAndLength(cx, &image, &sBuffer, &bufferLength ) ); // warning: GC on the returned buffer !
			J_S_ASSERT( bufferLength == sWidth * sHeight * sChannels * 1, "Invalid image format." );
			*pixbuf = gdk_pixbuf_new_from_data((const guchar *)sBuffer, GDK_COLORSPACE_RGB, sChannels == 4, 8, sWidth, sHeight, sWidth*sChannels, NULL, NULL);
			J_S_ASSERT( *pixbuf == NULL, "Unable to create the pixbuf." );
		}
	}
	return JS_TRUE;
}



// see jslibs/libs/librsvg/win32_config/rsvg-image.c at rsvg_pixbuf_new_from_href function.
extern "C" GdkPixbuf *rsvg_pixbuf_new_from_href(const char *href, const char *base_uri, GError ** error) {

	JSContext *cx = ((CxObj*)base_uri)->cx;
	JSObject *obj = ((CxObj*)base_uri)->obj;
	GdkPixbuf *pixbuf;
	RequestPixbufImage(cx, obj, href, &pixbuf); // error is managed in Write(), after rsvg_handle_write() call.
	return pixbuf;
}


/**doc
$CLASS_HEADER
**/
BEGIN_CLASS( SVG ) // Start the definition of the class. It defines some symbols: _name, _class, _prototype

DEFINE_FINALIZE() { // called when the Garbage Collector is running if there are no remaing references to this object.

	RsvgHandle *handle = (RsvgHandle*)JS_GetPrivate(cx, obj);
	if ( handle )
		g_object_unref(handle);
}


/**doc
 * $INAME()
  Constructs a new SVG object.
**/
DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();

	RsvgHandle *handle = rsvg_handle_new();
	J_S_ASSERT( handle != NULL, "Unable to create rsvg handler." );
	J_CHK( JS_SetPrivate(cx, obj, handle) );
	return JS_TRUE;
}


/**doc
=== Methods ===
**/

/**doc
 * $THIS $INAME( xmlString )
  Adds XML data to the current SVG context.
  $H note
   calls to callback function 'onImage' may occur during this call.
**/
DEFINE_FUNCTION_FAST( Write ) {

	RsvgHandle *handle = (RsvgHandle*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(handle);

	const char *data;
	size_t length;
	J_CHK( JsvalToStringAndLength(cx, &J_FARG(1), &data, &length) );

	GError *error = NULL;
	gboolean status;

//	rsvg_handle_new_from_file
//	xmlResetLastError();

//	rsvg_handle_set_base_uri(handle, "123"); // This can only be called before rsvg_handle_write()

	gchar *tmp = handle->priv->base_uri;
	CxObj cxobj = { cx, J_FOBJ };
	handle->priv->base_uri = (gchar*)&cxobj; // hack base_uri to store cx and obj for rsvg_pixbuf_new_from_href()
	status = rsvg_handle_write(handle, (const guchar *)data, length, &error);
	handle->priv->base_uri = tmp;

	if ( JS_IsExceptionPending(cx) )
		return JS_FALSE;

	if ( !status ) {

		xmlErrorPtr xmlErr = xmlGetLastError();
		if ( xmlErr != NULL ) { // XML error
			J_REPORT_ERROR_2("SVG error: %s. %s", error->message, xmlErr->message);
		} else
			J_REPORT_ERROR_1("SVG error: %s", error->message);
	}

	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


/*
DEFINE_PROPERTY( xmlData ) {

	RsvgHandle *handle = (RsvgHandle*)JS_GetPrivate(cx, obj);
	if ( handle )
		g_object_unref(handle);
	handle = rsvg_handle_new();
	J_S_ASSERT( handle != NULL, "Unable to create rsvg handler." );
	J_CHK( JS_SetPrivate(cx, obj, handle) );

	const char *data;
	size_t length;
	J_CHK( JsvalToStringAndLength(cx, &J_FARG(1), &data, &length) );

	GError *error = NULL;
	gboolean status;

//	xmlResetLastError();
//	rsvg_handle_set_base_uri(handle, "123"); // This can only be called before rsvg_handle_write()

	gchar *tmp = handle->priv->base_uri;
	CxObj cxobj = { cx, obj };
	handle->priv->base_uri = (gchar*)&cxobj; // hack base_uri to store cx and obj for rsvg_pixbuf_new_from_href()
	status = rsvg_handle_write(handle, (const guchar *)data, length, &error);
	handle->priv->base_uri = tmp;

	if ( JS_IsExceptionPending(cx) )
		return JS_FALSE;

	if ( !status ) {

		xmlErrorPtr xmlErr = xmlGetLastError();
		if ( xmlErr != NULL ) { // XML error
			J_REPORT_ERROR_2("SVG error: %s. %s", error->message, xmlErr->message);
		} else
			J_REPORT_ERROR_1("SVG error: %s", error->message);
	}
	return JS_TRUE;
}
*/

/**doc
 * $ImageObject $INAME( [ imageWidth , imageHeight ] [ , channels ] [ , transformationMatrix ] [ , elementId ] )
  $H arguments
   $ARG integer imageWidth: override default SVG's  width.
   $ARG integer imageHeight: override default SVG's  width.
   $ARG integer channels: 1 (Alpha only), 3 (RGB) or 4 (RGBA).
   $ARG Array transformationMatrix: array of reals.
   $ARG string elementId: Draws a subset of a SVG starting from an element's id. For example, if you have a layer called "layer1" that you wish to render, pass "#layer1" as the id.
  $H example
  {{{
  var svg = new SVG();
  svg.Write('<svg><circle cx="50" cy="50" r="25" fill="red"/></svg>');
  svgimage = svg.RenderImage(100, 100, [2,0,0,1,0,0]);
  new File('test.png').content = EncodePngImage( svgimage )
  }}}
**/
DEFINE_FUNCTION_FAST( RenderImage ) { // using cairo

	RsvgHandle *handle = (RsvgHandle*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(handle);

	gboolean status;

	GError *error = NULL;
	status = rsvg_handle_close(handle, &error);
	if ( !status )
		J_REPORT_ERROR_1( "SVG error: %s", error->message );

	RsvgDimensionData dim;
	rsvg_handle_get_dimensions(handle, &dim);

	cairo_matrix_t tr;
	cairo_matrix_init_identity(&tr);

	size_t imageWidth, imageHeight;
	if ( J_FARG_ISDEF(1) ) {

		J_CHK( JsvalToUInt(cx, J_FARG(1), &imageWidth) );
		tr.xx = (double)imageWidth / (double)dim.width;
	} else
		imageWidth = dim.width;

	if ( J_FARG_ISDEF(2) ) {

		J_CHK( JsvalToUInt(cx, J_FARG(2), &imageHeight) );
		tr.yy = (double)imageHeight / (double)dim.height;
	} else
		imageHeight = dim.height;


	size_t channels;
	if ( J_FARG_ISDEF(3) ) {

		J_CHK( JsvalToUInt(cx, J_FARG(3), &channels) );
		J_S_ASSERT( channels == 1 || channels == 3 || channels == 4, "Invalid format." );
	} else
		channels = 4;


	if ( J_FARG_ISDEF(4) ) {
		
		J_S_ASSERT_ARRAY( J_FARG(4) );
		double trVector[6];
		size_t currentLength;
		J_CHK( JsvalToDoubleVector(cx, J_FARG(4), trVector, COUNTOF(trVector), &currentLength ) );
		J_S_ASSERT( currentLength == 6, "Invalid transformation matrix size." );
		cairo_matrix_t tmp = *(cairo_matrix_t*)&trVector;
		cairo_matrix_multiply(&tr, &tmp, &tr);
	}

	const char *id;
	if ( J_FARG_ISDEF(5) ) {

		J_CHK( JsvalToString(cx, &J_FARG(5), &id) );
		J_S_ASSERT( id != NULL && id[0] == '#', "Invalid id." );
	} else
		id = NULL;


	cairo_format_t surfaceFormat;
	switch ( channels ) {
		case 1:
			surfaceFormat = CAIRO_FORMAT_A8;
			break;
		case 3:
			surfaceFormat = CAIRO_FORMAT_RGB24;
			break;
		case 4:
			surfaceFormat = CAIRO_FORMAT_ARGB32; // Pre-multiplied alpha is used. (That is, 50% transparent red is 0x80800000, not 0x80ff0000.)
			break;
		default:
			J_REPORT_ERROR("Unsupported output image format.");
	}

	cairo_surface_t *surface = cairo_image_surface_create(surfaceFormat, imageWidth, imageHeight);
	J_S_ASSERT( cairo_surface_status(surface) == CAIRO_STATUS_SUCCESS, "Unable to create a drawing surface." );
	cairo_t *cr = cairo_create(surface);
	J_S_ASSERT( cairo_status(cr) == CAIRO_STATUS_SUCCESS, "Unable to create drawing state." );

	//cairo_set_antialias(cr, CAIRO_ANTIALIAS_DEFAULT);
	//cairo_set_tolerance(cr, 0.1);
	
	cairo_set_matrix(cr, &tr);
	status = rsvg_handle_render_cairo_sub(handle, cr, id);
	
	J_S_ASSERT_1( status == TRUE, "Unable to render the SVG. %s", cairo_status_to_string(cairo_status(cr)) );

//	cairo_format_t surfaceFormat = cairo_image_surface_get_format(surface);

	int width = cairo_image_surface_get_width(surface);
	int height = cairo_image_surface_get_height(surface);
	void *buffer = cairo_image_surface_get_data(surface);

	size_t pixelCount = width * height;
	size_t length = pixelCount * channels;

	void *image = JS_malloc(cx, length);
	J_S_ASSERT_ALLOC(image);


	switch ( surfaceFormat ) {
		case CAIRO_FORMAT_RGB24:
			for ( size_t i = 0; i < pixelCount; i++ ) { // 3 2 1 X  ->  1 2 3

				u_int32_t src = ((u_int32_t*)buffer)[i];
				u_int8_t* dest = (u_int8_t*)image + i*3;
				*(dest++) = src >> 16 & 0xff;
				*(dest++) = src >> 8 & 0xff;
				*(dest++) = src & 0xff;
			}
			break;
		case CAIRO_FORMAT_ARGB32:
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
 * $BOOL $INAME( elementId, polarity )
  $H arguments
   $ARG string elementId: the id of the element with '#' prefix (eg. '#circle1').
	$ARG boolean polarity: false to hide, true to show.
  $H return value
   true if the element visibility has been set, otherwise false.
**/
DEFINE_FUNCTION_FAST( SetVisible ) {
	
	J_S_ASSERT_ARG_MIN(2);

	RsvgHandle *handle = (RsvgHandle*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(handle);

	const char *id;
	J_CHK( JsvalToString(cx, &J_FARG(1), &id) );
	J_S_ASSERT( id != NULL && id[0] == '#', "Invalid id." );

	bool visible;
	JsvalToBool(cx, J_FARG(2), &visible);

	RsvgNode *drawsub = rsvg_defs_lookup (handle->priv->defs, id);

	if ( drawsub == NULL ) {

		*J_FRVAL = JSVAL_FALSE;
		return JS_TRUE;
	}

	drawsub->state->visible = visible ? TRUE : FALSE;

	*J_FRVAL = JSVAL_TRUE;
	return JS_TRUE;
}



/**doc
=== Properties ===
**/

/**doc
 * $INT|$ARRAY $INAME $WRITEONLY
  Sets the dpi of the resulting image. If the argument is an Array (like [ dpiX, dpiY ]) X and Y dpi can be set aside.
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
  Is the default width of the SVG.
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
  Is the default height of the SVG.
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
  Is the title of the SVG.
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
  Is the metadata string of the SVG.
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
  Is the description string of the SVG.
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


/**doc
 * $INAME $READONLY
**/
/*
DEFINE_PROPERTY(images) {

	RsvgHandle *handle = (RsvgHandle*)JS_GetPrivate(cx, J_OBJ);
	J_S_ASSERT_RESOURCE(handle);

	J_CHK( JS_GetReservedSlot(cx, J_OBJ, SLOT_IMAGES_OBJECT, vp) );
	if ( JSVAL_IS_VOID( *vp ) ) {

		*vp = OBJECT_TO_JSVAL( JS_NewObject(cx, NULL, NULL, NULL) );
		J_CHK( JS_SetReservedSlot(cx, J_OBJ, SLOT_IMAGES_OBJECT, *vp) );
	}
	return JS_TRUE;
}
*/


/**doc
=== callback functions ===
 * $TYPE ImageObject *onImage*( uri )
  Called when the SVG renderer need to draw an image element. _uri_ is the name of the requested image.
  $H example
   {{{
   var svg = new SVG();
	svg.Write('<svg><image x="0" y="0" path="img.png"/></svg>');
   svg.onImage = function(href) {
    return DecodePngImage( new File('myImage.png').Open('r') );
   }
	}}}
**/

CONFIGURE_CLASS

	HAS_PRIVATE

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST(Write)
		FUNCTION_FAST(RenderImage)
		FUNCTION_FAST(SetVisible)
//		FUNCTION_FAST(GetImage)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
//		PROPERTY_WRITE_STORE(xmlData)
//		PROPERTY_READ(images)
		PROPERTY_WRITE(dpi)
		PROPERTY_READ(width)
		PROPERTY_READ(height)
		PROPERTY_READ(title)
		PROPERTY_READ(metadata)
		PROPERTY_READ(description)
	END_PROPERTY_SPEC

END_CLASS
