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
	JL_CHK( JS_GetProperty(cx, obj, "onImage", &onImageFct) );
	if ( JsvalIsFunction(cx, onImageFct) ) {

		jsval nameVal;
		JL_CHK( StringToJsval(cx, name, &nameVal) );
		JL_CHK( JL_CallFunction(cx, obj, onImageFct, &image, 1, nameVal) );
		if ( JSVAL_IS_OBJECT( image ) ) {

			JSObject *imageObj = JSVAL_TO_OBJECT( image );
			int sWidth, sHeight, sChannels;
			JL_CHK( GetPropertyInt(cx, imageObj, "width", &sWidth) );
			JL_CHK( GetPropertyInt(cx, imageObj, "height", &sHeight) );
			JL_CHK( GetPropertyInt(cx, imageObj, "channels", &sChannels) );

			JL_S_ASSERT_1( sChannels == 3 || sChannels == 4, "Unsupported image format for %s.", name );
			const char *sBuffer;
			size_t bufferLength;
			JL_CHK( JsvalToStringAndLength(cx, &image, &sBuffer, &bufferLength ) ); // warning: GC on the returned buffer !
			JL_S_ASSERT( bufferLength == sWidth * sHeight * sChannels * 1, "Invalid image format." );
			*pixbuf = gdk_pixbuf_new_from_data((const guchar *)sBuffer, GDK_COLORSPACE_RGB, sChannels == 4, 8, sWidth, sHeight, sWidth*sChannels, NULL, NULL);
			JL_S_ASSERT( *pixbuf == NULL, "Unable to create the pixbuf." );
		}
	}
	return JS_TRUE;
	JL_BAD;
}



// see jslibs/libs/librsvg/win32_config/rsvg-image.c at rsvg_pixbuf_new_from_href function.
extern "C" GdkPixbuf *rsvg_pixbuf_new_from_href(const char *href, const char *base_uri, GError ** error) {

	JSContext *cx = ((CxObj*)base_uri)->cx;
	JSObject *obj = ((CxObj*)base_uri)->obj;
	GdkPixbuf *pixbuf;
	RequestPixbufImage(cx, obj, href, &pixbuf); // error is managed in Write(), after rsvg_handle_write() call.
	return pixbuf;
}

struct Private {
	RsvgHandle *handle;
	cairo_matrix_t transformation;
};



/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( SVG ) // Start the definition of the class. It defines some symbols: _name, _class, _prototype

DEFINE_FINALIZE() { // called when the Garbage Collector is running if there are no remaing references to this object.

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	if ( !pv )
		return;

	g_object_unref(pv->handle);
	JS_free(cx, pv);
}


/**doc
$TOC_MEMBER $INAME
 $INAME()
  Constructs a new SVG object.
**/
DEFINE_CONSTRUCTOR() {

	JL_S_ASSERT_CONSTRUCTING();
	JL_S_ASSERT_THIS_CLASS();

	Private *pv = (Private*)JS_malloc(cx, sizeof(Private));
	JL_CHK( pv );
	pv->handle = rsvg_handle_new();
	JL_S_ASSERT( pv->handle != NULL, "Unable to create rsvg handler." );
	cairo_matrix_init_identity(&pv->transformation);
	JL_CHK( JL_SetPrivate(cx, obj, pv) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
=== Methods ===
**/

/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( xmlString )
  Adds XML data to the current SVG context.
  $H note
   calls to callback function 'onImage' may occur during this call.
  $H example
  {{{
  svg.Write(<svg><circle cx="50" cy="50" r="25" fill="red"/></svg>);
  // or
  svg.Write('<svg><circle cx="50" cy="50" r="25" fill="red"/></svg>');
  }}}
**/
DEFINE_FUNCTION_FAST( Write ) {

	Private *pv = (Private*)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE(pv);
	RsvgHandle *handle = pv->handle;

	const char *data;
	size_t length;
	JL_CHK( JsvalToStringAndLength(cx, &JL_FARG(1), &data, &length) );

	GError *error = NULL;
	gboolean status;

//	rsvg_handle_new_from_file
//	xmlResetLastError();

//	rsvg_handle_set_base_uri(handle, "123"); // This can only be called before rsvg_handle_write()

	gchar *tmp = handle->priv->base_uri;
	CxObj cxobj;
	cxobj.cx = cx;
	cxobj.obj = JL_FOBJ;
	handle->priv->base_uri = (gchar*)&cxobj; // hack base_uri to store cx and obj for rsvg_pixbuf_new_from_href()
	status = rsvg_handle_write(handle, (const guchar *)data, length, &error);
	handle->priv->base_uri = tmp;

	if ( JS_IsExceptionPending(cx) )
		return JS_FALSE;

	if ( !status ) {

		xmlErrorPtr xmlErr = xmlGetLastError();
		if ( xmlErr != NULL ) { // XML error
			JL_REPORT_ERROR_2("SVG error: %s. %s", error->message, xmlErr->message);
		} else
			JL_REPORT_ERROR_1("SVG error: %s", error->message);
	}

	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/*
DEFINE_PROPERTY( xmlData ) {

	RsvgHandle *handle = (RsvgHandle*)JL_GetPrivate(cx, obj);
	if ( handle )
		g_object_unref(handle);
	handle = rsvg_handle_new();
	JL_S_ASSERT( handle != NULL, "Unable to create rsvg handler." );
	JL_CHK( JL_SetPrivate(cx, obj, handle) );

	const char *data;
	size_t length;
	JL_CHK( JsvalToStringAndLength(cx, &JL_FARG(1), &data, &length) );

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
			JL_REPORT_ERROR_2("SVG error: %s. %s", error->message, xmlErr->message);
		} else
			JL_REPORT_ERROR_1("SVG error: %s", error->message);
	}
	return JS_TRUE;
}
*/

/**doc
$TOC_MEMBER $INAME
 $TYPE ImageObject $INAME( [ imageWidth , imageHeight ] [ , channels ] [ , fit ] [ , elementId ] )
  Draws the SVG to an image object.
  $H arguments
   $ARG $INT imageWidth: override default SVG's  width.
   $ARG $INT imageHeight: override default SVG's  width.
   $ARG $INT channels: 1 (Alpha only), 3 (RGB) or 4 (RGBA).
   $ARG $BOOL fit: fit the SVG dimensions to [imageWidth, imageHeight].
   $ARG $STR elementId: draws a subset of a SVG starting from an element's id. For example, if you have a layer called "layer1" that you wish to render, pass "#layer1" as the id.
  $H example
  {{{
  var svg = new SVG();
  svg.Write(<svg><circle cx="50" cy="50" r="25" fill="red"/></svg>);
  svg.Rotate(Math.PI/4); // +45 deg
  svgimage = svg.RenderImage(100, 100, true);
  new File('test.png').content = EncodePngImage( svgimage )
  }}}
**/
DEFINE_FUNCTION_FAST( RenderImage ) { // using cairo

	Private *pv = (Private*)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE(pv);
	RsvgHandle *handle = pv->handle;

	gboolean status;

	GError *error = NULL;
	status = rsvg_handle_close(handle, &error);
	if ( !status )
		JL_REPORT_ERROR_1( "SVG error: %s", error->message );

	RsvgDimensionData dim;
	rsvg_handle_get_dimensions(handle, &dim);

	size_t imageWidth, imageHeight;
	if ( JL_FARG_ISDEF(1) ) {

		JL_S_ASSERT_ARG_MIN(2);
		JL_CHK( JsvalToUInt(cx, JL_FARG(1), &imageWidth) );
		JL_CHK( JsvalToUInt(cx, JL_FARG(2), &imageHeight) );
	} else {

		imageWidth = dim.width;
		imageHeight = dim.height;
	}

	size_t channels;
	if ( JL_FARG_ISDEF(3) ) {

		JL_CHK( JsvalToUInt(cx, JL_FARG(3), &channels) );
		JL_S_ASSERT( channels == 1 || channels == 3 || channels == 4, "Invalid format." );
	} else
		channels = 4;

	if ( JL_FARG_ISDEF(4) ) { // fit

		bool fit;
		JL_CHK( JsvalToBool(cx, JL_FARG(4), &fit) );
		if ( fit ) {

			cairo_matrix_t tmp;
			cairo_matrix_init_identity(&tmp);
			cairo_matrix_scale(&tmp, (double)imageWidth / (double)dim.width, (double)imageHeight / (double)dim.height);
			cairo_matrix_multiply(&pv->transformation, &pv->transformation, &tmp);
//			cairo_matrix_scale(&pv->transformation, (double)imageWidth / (double)dim.width, (double)imageHeight / (double)dim.height);
		}
	}

/*
	if ( JL_FARG_ISDEF(4) ) {

		JL_S_ASSERT_ARRAY( JL_FARG(4) );
		double trVector[6];
		size_t currentLength;
		JL_CHK( JsvalToDoubleVector(cx, JL_FARG(4), trVector, COUNTOF(trVector), &currentLength ) );
		JL_S_ASSERT( currentLength == 6, "Invalid transformation matrix size." );
		cairo_matrix_t tmp = *(cairo_matrix_t*)&trVector;
		cairo_matrix_multiply(&tr, &tmp, &tr);
	}
*/

	const char *id;
	if ( JL_FARG_ISDEF(5) ) {

		JL_CHK( JsvalToString(cx, &JL_FARG(5), &id) );
		JL_S_ASSERT( id != NULL && id[0] == '#', "Invalid id." );
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
			JL_REPORT_ERROR("Unsupported output image format.");
	}

	cairo_surface_t *surface = cairo_image_surface_create(surfaceFormat, imageWidth, imageHeight);
	JL_S_ASSERT( cairo_surface_status(surface) == CAIRO_STATUS_SUCCESS, "Unable to create a drawing surface." );
	cairo_t *cr = cairo_create(surface);
	JL_S_ASSERT( cairo_status(cr) == CAIRO_STATUS_SUCCESS, "Unable to create drawing state." );

	//cairo_set_antialias(cr, CAIRO_ANTIALIAS_DEFAULT);
	//cairo_set_tolerance(cr, 0.1);

	cairo_set_matrix(cr, &pv->transformation);
	status = rsvg_handle_render_cairo_sub(handle, cr, id);

	JL_S_ASSERT_1( status == TRUE, "Unable to render the SVG. %s", cairo_status_to_string(cairo_status(cr)) );

//	cairo_format_t surfaceFormat = cairo_image_surface_get_format(surface);

	int width = cairo_image_surface_get_width(surface);
	int height = cairo_image_surface_get_height(surface);
	void *buffer = cairo_image_surface_get_data(surface);

	size_t pixelCount = width * height;
	size_t length = pixelCount * channels;

	void *image = JS_malloc(cx, length);
	JL_CHK( image );


	switch ( surfaceFormat ) {
		case CAIRO_FORMAT_RGB24:
			for ( size_t i = 0; i < pixelCount; i++ ) { // 3 2 1 X  ->  1 2 3

				uint32_t src = ((uint32_t*)buffer)[i];
				uint8_t* dest = (uint8_t*)image + i*3;
				*(dest++) = src >> 16 & 0xff;
				*(dest++) = src >> 8 & 0xff;
				*(dest++) = src & 0xff;
			}
			break;
		case CAIRO_FORMAT_ARGB32:
			for ( size_t i = 0; i < pixelCount; i++ ) { // 3 2 1 4  ->  1 2 3 4

				uint32_t p = ((uint32_t*)buffer)[i];
				((uint32_t*)image)[i] = p & 0xff00ff00 | _rotl(p, 16) & 0x00ff00ff; // use SSE2 instructions ?
			}
			break;
		case CAIRO_FORMAT_A8:
			memcpy(image, buffer, length);
			break;
	}

	cairo_destroy(cr);
	cairo_surface_destroy(surface);

	jsval blobVal;
	JL_CHK( JL_NewBlob(cx, image, length, &blobVal) );
	JSObject *blobObj;
	JL_CHK( JS_ValueToObject(cx, blobVal, &blobObj) );
	JL_S_ASSERT( blobObj, "Unable to create Blob object." );
	*JL_FRVAL = OBJECT_TO_JSVAL(blobObj);
	JL_CHK( JS_DefineProperty(cx, blobObj, "channels", INT_TO_JSVAL(channels), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT ) );
	JL_CHK( JS_DefineProperty(cx, blobObj, "width", INT_TO_JSVAL(width), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT ) );
	JL_CHK( JS_DefineProperty(cx, blobObj, "height", INT_TO_JSVAL(height), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT ) );
	return JS_TRUE;
	JL_BAD;
}


/*
DEFINE_FUNCTION_FAST( GetImage ) { // using pixbuf

	RsvgHandle *handle = (RsvgHandle*)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE(handle);

	GdkPixbuf *pb = rsvg_handle_get_pixbuf(handle);

	JL_S_ASSERT( pb != NULL, "Insufficient data has been read to create the pixbuf." );
	JL_S_ASSERT( gdk_pixbuf_get_bits_per_sample(pb) == 8, "Unsupported bits_per_sample." );

	int width = gdk_pixbuf_get_width(pb);
	int height = gdk_pixbuf_get_height(pb);
	int channels = gdk_pixbuf_get_n_channels(pb);
	void *buffer = gdk_pixbuf_get_pixels(pb);

	jsval blobVal;
	JL_CHK( JL_NewBlob(cx, buffer, width*height*channels, &blobVal) ); // (TBD) copy the buffer before !!

	JSObject *blobObj;
	JL_CHK( JS_ValueToObject(cx, blobVal, &blobObj) );
	JL_S_ASSERT( blobObj, "Unable to create Blob object." );
	*JL_FRVAL = OBJECT_TO_JSVAL(blobObj);

	JL_CHK( JS_DefineProperty(cx, blobObj, "channels", INT_TO_JSVAL(channels), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT ) );
	JL_CHK( JS_DefineProperty(cx, blobObj, "width", INT_TO_JSVAL(width), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT ) );
	JL_CHK( JS_DefineProperty(cx, blobObj, "height", INT_TO_JSVAL(height), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT ) );

	return JS_TRUE;
}
*/


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME( elementId, polarity )
  $H arguments
   $ARG $STR elementId: the id of the element with '#' prefix (eg. '#circle1').
   $ARG $BOOL polarity: false to hide, true to show.
  $H return value
   true if the element visibility has been set, otherwise false.
**/
DEFINE_FUNCTION_FAST( SetVisible ) {

	JL_S_ASSERT_ARG_MIN(2);

	Private *pv = (Private*)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE(pv);
	RsvgHandle *handle = pv->handle;

	const char *id;
	JL_CHK( JsvalToString(cx, &JL_FARG(1), &id) );
	JL_S_ASSERT( id != NULL && id[0] == '#', "Invalid id." );

	bool visible;
	JsvalToBool(cx, JL_FARG(2), &visible);

	RsvgNode *drawsub = rsvg_defs_lookup (handle->priv->defs, id);

	if ( drawsub == NULL ) {

		*JL_FRVAL = JSVAL_FALSE;
		return JS_TRUE;
	}

	drawsub->state->visible = visible ? TRUE : FALSE;

	*JL_FRVAL = JSVAL_TRUE;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( sx, sy )
  Applies scaling by _sx_, _sy_ to the current transformation.
  The effect of the new transformation is to first scale the coordinates by _sx_ and _sy_,
  then apply the original transformation to the coordinates.
**/
DEFINE_FUNCTION_FAST( Scale ) {

	JL_S_ASSERT_ARG_MIN(2);
	Private *pv = (Private*)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE(pv);
	double sx, sy;
	JL_CHK( JsvalToDouble(cx, JL_FARG(1), &sx) );
	JL_CHK( JsvalToDouble(cx, JL_FARG(2), &sy) );
	cairo_matrix_scale(&pv->transformation, sx, sy);
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( radians )
  Applies rotation by _radians_ to the current transformation.
  The effect of the new transformation is to first rotate the coordinates by _radians_,
  then apply the original transformation to the coordinates.
**/
DEFINE_FUNCTION_FAST( Rotate ) {

	JL_S_ASSERT_ARG_MIN(1);
	Private *pv = (Private*)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE(pv);
	double angle;
	JL_CHK( JsvalToDouble(cx, JL_FARG(1), &angle) );
	cairo_matrix_rotate(&pv->transformation, angle);
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( tx, ty )
  Applies a translation by _tx_, _ty_ to the current transformation.
  The effect of the new transformation is to first translate the coordinates by _tx_ and _ty_,
  then apply the original transformation to the coordinates.
**/
DEFINE_FUNCTION_FAST( Translate ) {

	JL_S_ASSERT_ARG_MIN(2);
	Private *pv = (Private*)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE(pv);
	double tx, ty;
	JL_CHK( JsvalToDouble(cx, JL_FARG(1), &tx) );
	JL_CHK( JsvalToDouble(cx, JL_FARG(2), &ty) );
	cairo_matrix_translate(&pv->transformation, tx, ty);
	return JS_TRUE;
	JL_BAD;
}


/**doc
=== Properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $INT | $ARRAY $INAME $WRITEONLY
  Sets the dpi of the resulting image. If the argument is an Array (like [ dpiX, dpiY ]) X and Y dpi can be set aside.
**/
DEFINE_PROPERTY(dpi) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	RsvgHandle *handle = pv->handle;

	if ( JSVAL_IS_VOID(*vp) ) {

		rsvg_handle_set_dpi(handle, -1);
	} else
	if ( JSVAL_IS_NUMBER(*vp) ) {

		size_t dpi;
		JL_CHK( JsvalToUInt(cx, *vp, &dpi) );
		rsvg_handle_set_dpi(handle, dpi);
	} else
	if ( JsvalIsArray(cx, *vp) ) {

		size_t dpiX, dpiY;
		jsval tmp;
		JL_CHK( JS_GetElement(cx, JSVAL_TO_OBJECT(*vp), 0, &tmp) );
		JL_CHK( JsvalToUInt(cx, tmp, &dpiX) );
		JL_CHK( JS_GetElement(cx, JSVAL_TO_OBJECT(*vp), 1, &tmp) );
		JL_CHK( JsvalToUInt(cx, tmp, &dpiY) );
		rsvg_handle_set_dpi_x_y(handle, dpiX, dpiY);
	} else
		JL_REPORT_ERROR("Invalid case.");
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Is the default width of the SVG.
**/
DEFINE_PROPERTY(width) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	RsvgHandle *handle = pv->handle;

	RsvgDimensionData dim;
	rsvg_handle_get_dimensions(handle, &dim);
	JL_CHK( UIntToJsval(cx, dim.width, vp) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Is the default height of the SVG.
**/
DEFINE_PROPERTY(height) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	RsvgHandle *handle = pv->handle;

	RsvgDimensionData dim;
	rsvg_handle_get_dimensions(handle, &dim);
	JL_CHK( UIntToJsval(cx, dim.height, vp) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Is the title of the SVG.
**/
DEFINE_PROPERTY(title) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	RsvgHandle *handle = pv->handle;

	const char *title = rsvg_handle_get_title(handle);
	if ( title != NULL )
		JL_CHK( StringToJsval(cx, title, vp) );
	else
		*vp = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Is the metadata string of the SVG.
**/
DEFINE_PROPERTY(metadata) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	RsvgHandle *handle = pv->handle;

	const char *metadata = rsvg_handle_get_metadata(handle);
	if ( metadata != NULL )
		JL_CHK( StringToJsval(cx, metadata, vp) );
	else
		*vp = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Is the description string of the SVG.
**/
DEFINE_PROPERTY(description) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	RsvgHandle *handle = pv->handle;

	const char *description = rsvg_handle_get_desc(handle);
	if ( description != NULL )
		JL_CHK( StringToJsval(cx, description, vp) );
	else
		*vp = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
**/
/*
DEFINE_PROPERTY(images) {

	RsvgHandle *handle = (RsvgHandle*)JL_GetPrivate(cx, JL_OBJ);
	JL_S_ASSERT_RESOURCE(handle);

	JL_CHK( JS_GetReservedSlot(cx, JL_OBJ, SLOT_IMAGES_OBJECT, vp) );
	if ( JSVAL_IS_VOID( *vp ) ) {

		*vp = OBJECT_TO_JSVAL( JS_NewObject(cx, NULL, NULL, NULL) );
		JL_CHK( JS_SetReservedSlot(cx, JL_OBJ, SLOT_IMAGES_OBJECT, *vp) );
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
  svg.Write(<svg><image x="0" y="0" path="img.png"/></svg>);
  svg.onImage = function(href) {
   return DecodePngImage( new File('myImage.png').Open('r') );
  }
  }}}
**/

CONFIGURE_CLASS

	REVISION(SvnRevToInt("$Revision$"))
	HAS_PRIVATE

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST(Write)
		FUNCTION_FAST(RenderImage)
		FUNCTION_FAST(SetVisible)
//		FUNCTION_FAST(GetImage)
		FUNCTION_FAST(Scale)
		FUNCTION_FAST(Rotate)
		FUNCTION_FAST(Translate)
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
