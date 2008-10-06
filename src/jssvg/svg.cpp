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

#include <rsvg.h>
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

DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();

	RsvgHandle *handle;
	handle = rsvg_handle_new();

	J_S_ASSERT_RESOURCE(handle);

	J_CHK( JS_SetPrivate(cx, obj, handle) );



	return JS_TRUE;
}

//DEFINE_FUNCTION( Call ) {
//	return JS_TRUE;
//}

//DEFINE_PROPERTY( prop ) {
//	return JS_TRUE;
//}

DEFINE_FUNCTION_FAST( Write ) {

	RsvgHandle *handle = (RsvgHandle*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(handle);

	const char *data;
	size_t length;
	J_CHK( JsvalToStringAndLength(cx, &J_FARG(1), &data, &length) );

	GError *error = NULL;
	gboolean status;

//		rsvg_handle_new_from_file

	xmlResetLastError();

	status = rsvg_handle_write(handle, (const guchar *)data, length, &error);
	if ( !status ) {

		xmlErrorPtr xmlErr = xmlGetLastError();
		if ( xmlErr != NULL ) { // XML error
			J_REPORT_ERROR_2("SVG error: %s. %s", error->message, xmlErr->message);
		} else
			J_REPORT_ERROR_1("SVG error: %s", error->message);
	}

	status = rsvg_handle_close(handle, &error);
	if (!status)
		J_REPORT_ERROR_1("SVG error: %s", error->message);
	

	RsvgDimensionData dim;
	rsvg_handle_get_dimensions(handle, &dim);

	GdkPixbuf *pb = rsvg_handle_get_pixbuf(handle);
	J_S_ASSERT( gdk_pixbuf_get_bits_per_sample(pb) == 8, "Unsupported bits_per_sample." );

	int channels = gdk_pixbuf_get_n_channels(pb);

	int width = gdk_pixbuf_get_width(pb);
	int height = gdk_pixbuf_get_height(pb);

	void *buffer = gdk_pixbuf_get_pixels(pb);

	jsval blobVal;
	J_CHK( J_NewBlob(cx, buffer, width*height*channels, &blobVal) );

	JSObject *blobObj;
	J_CHK( JS_ValueToObject(cx, blobVal, &blobObj) );
	J_S_ASSERT( blobObj, "Unable to create Blob object." );
	*J_FRVAL = OBJECT_TO_JSVAL(blobObj);

	JS_DefineProperty(cx, blobObj, "channels", INT_TO_JSVAL(channels), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT );
	JS_DefineProperty(cx, blobObj, "width", INT_TO_JSVAL(width), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT );
	JS_DefineProperty(cx, blobObj, "height", INT_TO_JSVAL(height), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT );

	return JS_TRUE;
}

DEFINE_INIT() {

	rsvg_init(); // see rsvg_term()
	return JS_TRUE;
}

CONFIGURE_CLASS // This section containt the declaration and the configuration of the class

	HAS_INIT

	HAS_PRIVATE
//	HAS_RESERVED_SLOTS(1)

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST(Write)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
//		PROPERTY(prop)
	END_PROPERTY_SPEC

END_CLASS
