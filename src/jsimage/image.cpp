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


BEGIN_CLASS( Image )

DEFINE_FINALIZE() {

	if ( JL_GetHostPrivate(cx)->canSkipCleanup )
		return;

	void *data = JL_GetPrivate(cx, obj);
	if ( data != NULL )
		jl_free(data); // jl_free(NULL) is legal
}

DEFINE_FUNCTION( alloc ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_INSTANCE(obj, JL_THIS_CLASS);
	JL_ASSERT_ARGC_MIN(1);

	void *data;
	data = JL_GetPrivate(cx, obj);
	if ( data != NULL )
		jl_free(data);

	unsigned int size;
	size = JSVAL_TO_INT(JL_ARG(1));
	data = jl_malloc(size);
	JL_ASSERT_ALLOC( data );
	JL_updateMallocCounter(cx, size);
	JL_SetPrivate(cx, obj, data);

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


DEFINE_CONSTRUCTOR() {

	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

	JSFunction *allocFunction;
	allocFunction = JS_NewFunction(cx, _alloc, 0, 0, NULL, "alloc");
	JL_ASSERT_ALLOC( allocFunction ); // "Unable to create allocation function."
	JSObject *functionObject;
	functionObject = JS_GetFunctionObject(allocFunction);
	JL_CHK( JL_SetReservedSlot(cx, obj, SLOT_FUNCTION_ALLOC, OBJECT_TO_JSVAL(functionObject)) );
	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION( free ) {

	JL_IGNORE(argc);
	JL_DEFINE_FUNCTION_OBJ;
	*JL_RVAL = JSVAL_VOID;

	void *data = JL_GetPrivate(cx, obj);
	if ( data != NULL ) {

		JL_SetPrivate(cx, obj, NULL);
		jl_free(data);
	}
	return JS_TRUE;
}

DEFINE_FUNCTION( trim ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARGC_MIN(1);
	int vect[4];
	//IntArrayToVector(cx, 4, &argv[0], vect);
	uint32_t length;
	JL_CHK( JL_JsvalToNativeVector(cx, JL_ARG(1), vect, 4, &length) );
	JL_ASSERT( length == 4, E_ARG, E_NUM(1), E_TYPE, E_TY_NARRAY(4) );

	int x;
	x = vect[0];
	int y;
	y = vect[1];
	int x1;
	x1 = vect[2];
	int y1;
	y1= vect[3];

	int width, height, channels;
	JL_CHK( JL_GetProperty(cx, obj, JLID(cx, width), &width) && JL_GetProperty(cx, obj, JLID(cx, height), &height) && JL_GetProperty(cx, obj, JLID(cx, channels), &channels) );

// assume that we have 1 Byte/channel !

	//JL_ASSERT( !(x<0 || x1<0 || x>width || x1>width || y<0 || y1<0 || y>height || y1>height), "Invalid size." );
	JL_ASSERT( !(x<0 || x1<0 || x>width || x1>width || y<0 || y1<0 || y>height || y1>height), E_ARG, E_NUM(1), E_INVALID );

	JSBool reuseBuffer;
	reuseBuffer = false; // default
	if ( argc >= 2 )
		JS_ValueToBoolean(cx, JL_ARG(2), &reuseBuffer);

	char *data;
	data = (char*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( data );

	char *tmpDataPtr;
	tmpDataPtr = data;

	char *newData;
	if (reuseBuffer)
		newData = data;
	else {
		newData = (char*)jl_malloc( channels * (x1-x) * (y1-y) );
		JL_ASSERT_ALLOC( newData );
		JL_SetPrivate(cx, obj, newData);
	}

	int newWidth;
	newWidth = x1-x;
	int newHeight;
	newHeight = y1-y;

	data += channels * (width * y + x); // now, data points to the first byte to displace
	for ( int i=0; i<newHeight; i++) {
		jl_memcpy(newData, data, channels * newWidth);
		newData += channels * newWidth;
		data += channels * width;
	}

	//JS_DefineProperty(cx, obj, "width", INT_TO_JSVAL(newWidth), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT );
	//JS_DefineProperty(cx, obj, "height", INT_TO_JSVAL(newHeight), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT );

	JL_CHK( JL_SetProperty(cx, obj, JLID(cx, width), newWidth) && JL_SetProperty(cx, obj, JLID(cx, width), newHeight) );

	*JL_RVAL = OBJECT_TO_JSVAL(obj); // allows to write: var texture = new Jpeg(f).Load().Trim(...)

	if ( !reuseBuffer )
		jl_free(tmpDataPtr);

	return JS_TRUE;
	JL_BAD;
}

DEFINE_FUNCTION( gamma ) {

	JL_ASSERT_ARGC_MIN(1);

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
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

	REVISION(JL_SvnRevToInt("$Revision: 3533 $"))
	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION(trim)
		FUNCTION(gamma)
//		FUNCTION(alloc) // see SLOT_FUNCTION_ALLOC
		FUNCTION(free)
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

