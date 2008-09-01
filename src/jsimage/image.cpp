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

//DECLARE_CLASS(Image)

#include "image.h"

#include <stdlib.h>


BEGIN_CLASS( Image )

DEFINE_FINALIZE() {

	void *data = JS_GetPrivate(cx, obj);
	if ( data != NULL )
		free(data);
}

DEFINE_FUNCTION( Alloc ) {

	J_S_ASSERT_CLASS(obj, _class);
	J_S_ASSERT_ARG_MIN(1);

	void *data = JS_GetPrivate(cx, obj);
	if ( data != NULL )
		free(data);

	unsigned int size = JSVAL_TO_INT(argv[0]);
	data = malloc(size);
	JS_SetPrivate(cx, obj, data);

	return JS_TRUE;
}


DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();
	JSFunction *allocFunction = JS_NewFunction(cx, _Alloc, 0, 0, NULL, "Alloc");
	J_S_ASSERT( allocFunction != NULL, "Unable to create allocation function." );
	JSObject *functionObject = JS_GetFunctionObject(allocFunction);
	JS_SetReservedSlot(cx, obj, SLOT_FUNCTION_ALLOC, OBJECT_TO_JSVAL(functionObject));
	return JS_TRUE;
}


DEFINE_FUNCTION( Free ) {

	void *data = JS_GetPrivate(cx, obj);
	if ( data != NULL ) {

		JS_SetPrivate(cx, obj, NULL);
		free(data);
	}
	return JS_TRUE;
}

DEFINE_FUNCTION( Trim ) {

	J_S_ASSERT_ARG_MIN(1);
	int vect[4];
	//IntArrayToVector(cx, 4, &argv[0], vect);
	size_t length;
	J_CHK( JsvalToIntVector(cx, argv[0], vect, 4, &length) );
	J_S_ASSERT( length == 4, "Invalid array size." );

	int x = vect[0];
	int y = vect[1];
	int x1 = vect[2];
	int y1 = vect[3];

	jsval tmp;
	JS_GetProperty(cx, obj, "width", &tmp);
	J_S_ASSERT_INT( tmp );
	int width = JSVAL_TO_INT(tmp);

	JS_GetProperty(cx, obj, "height", &tmp);
	J_S_ASSERT_INT( tmp );
	int height = JSVAL_TO_INT(tmp);

	JS_GetProperty(cx, obj, "channels", &tmp);
	J_S_ASSERT_INT( tmp );
	int channels = JSVAL_TO_INT(tmp);
	// assume that we have 1 Byte/channel

	J_S_ASSERT( !(x<0 || x1<0 || x>width || x1>width || y<0 || y1<0 || y>height || y1>height), "Invalid size." );

	JSBool reuseBuffer = false; // default
	if ( argc >= 2 )
		JS_ValueToBoolean(cx, argv[1], &reuseBuffer);

	char *data = (char*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( data );

	char *tmpDataPtr = data;

	char *newData;
	if (reuseBuffer)
		newData = data;
	else {
		newData = (char*)malloc( channels * (x1-x) * (y1-y) );
		JS_SetPrivate(cx, obj, newData);
	}

	int newWidth = x1-x;
	int newHeight = y1-y;

	data += channels * (width * y + x); // now, data points to the first byte to displace
	for ( int i=0; i<newHeight; i++) {
		memcpy(newData, data, channels * newWidth);
		newData += channels * newWidth;
		data += channels * width;
	}

	JS_DefineProperty(cx, obj, "width", INT_TO_JSVAL(newWidth), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT );
	JS_DefineProperty(cx, obj, "height", INT_TO_JSVAL(newHeight), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT );

	*rval = OBJECT_TO_JSVAL(obj); // allows to write: var texture = new Jpeg(f).Load().Trim(...)

	if ( !reuseBuffer )
		free(tmpDataPtr);

	return JS_TRUE;
}

DEFINE_FUNCTION( Gamma ) {

	J_S_ASSERT_ARG_MIN(1);

	return JS_TRUE;
}



//	JSBool Call(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
//		return JS_TRUE;
//	}

//	JSBool prop(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
//		return JS_TRUE;
//	}

//	JSBool Func(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
//		return JS_TRUE;
//	}


CONFIGURE_CLASS

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION(Trim)
		FUNCTION(Gamma)
//		FUNCTION(Alloc) // see SLOT_FUNCTION_ALLOC
		FUNCTION(Free)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_DEFINE(width)
		PROPERTY_DEFINE(height)
		PROPERTY_DEFINE(channels)
//		CREATE(pixelSize)
	END_PROPERTY_SPEC

	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1)

END_CLASS

