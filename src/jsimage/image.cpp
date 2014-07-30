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

	if ( jl::Host::getJLHost(fop->runtime())->canSkipCleanup )
		return;

	void *data = JL_GetPrivate(obj);
	if ( data != NULL )
		jl_free(data); // jl_free(NULL) is legal
}

DEFINE_FUNCTION( alloc ) {

		JL_ASSERT_INSTANCE(obj, JL_THIS_CLASS);
	JL_ASSERT_ARGC_MIN(1);

	void *data;
	data = JL_GetPrivate(obj);
	if ( data != NULL )
		jl_free(data);

	unsigned int size;
	size = JL_ARG(1).toInt32();
	data = jl_malloc(size);
	JL_ASSERT_ALLOC( data );
	JL_updateMallocCounter(cx, size);
	JL_SetPrivate( obj, data);

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


DEFINE_CONSTRUCTOR() {

	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

	JSFunction *allocFunction;
	allocFunction = JS_NewFunction(cx, _alloc, 0, 0, NULL, "alloc");
	JL_ASSERT_ALLOC( allocFunction ); // "Unable to create allocation function."
	JS::RootedObject functionObject(cx, JS_GetFunctionObject(allocFunction));
	JL_CHK( JL_SetReservedSlot( obj, SLOT_FUNCTION_ALLOC, OBJECT_TO_JSVAL(functionObject)) );

	return true;
	JL_BAD;
}


DEFINE_FUNCTION( free ) {

	JL_IGNORE(argc);
		JL_RVAL.setUndefined();

	void *data = JL_GetPrivate(obj);
	if ( data != NULL ) {

		JL_SetPrivate( obj, NULL);
		jl_free(data);
	}
	return true;
	JL_BAD;
}

DEFINE_FUNCTION( trim ) {

		JL_ASSERT_ARGC_MIN(1);

	int vect[4];
	//IntArrayToVector(cx, 4, &argv[0], vect);
	uint32_t length;
	JL_CHK( jl::getVector(cx, JL_ARG(1), vect, 4, &length) );
	JL_ASSERT( length == 4, E_ARG, E_NUM(1), E_TYPE, E_TY_NARRAY(4) );

	int x, y;
	int x1, y1;
	x = vect[0];
	y = vect[1];
	x1 = vect[2];
	y1 = vect[3];

	int width, height, channels;
	JL_CHK( jl::getProperty(cx, obj, JLID(cx, width), &width) );
	JL_CHK( jl::getProperty(cx, obj, JLID(cx, height), &height) );
	JL_CHK( jl::getProperty(cx, obj, JLID(cx, channels), &channels) );

// assume that we have 1 Byte/channel !

	//JL_ASSERT( !(x<0 || x1<0 || x>width || x1>width || y<0 || y1<0 || y>height || y1>height), "Invalid size." );
	JL_ASSERT( !(x<0 || x1<0 || x>width || x1>width || y<0 || y1<0 || y>height || y1>height), E_ARG, E_NUM(1), E_INVALID );

	bool reuseBuffer;
	reuseBuffer = false; // default
	if ( argc >= 2 )
		JS::ToBoolean(cx, JL_ARG(2), &reuseBuffer);

	uint8_t *data;
	data = (uint8_t*)JL_GetPrivate( obj );
	JL_ASSERT_THIS_OBJECT_STATE( data );

	uint8_t *tmpDataPtr;
	tmpDataPtr = data;

	uint8_t *newData;
	if (reuseBuffer)
		newData = data;
	else {
		newData = (uint8_t*)jl_malloc( channels * (x1 - x) * (y1 - y) );
		JL_ASSERT_ALLOC( newData );
		JL_SetPrivate( obj, newData);
	}

	int newWidth, newHeight;
	newWidth = x1-x;
	newHeight = y1-y;

	data += channels * (width * y + x); // now, data points to the first byte to displace
	for ( int i=0; i<newHeight; i++) {

		jl::memcpy(newData, data, channels * newWidth);
		newData += channels * newWidth;
		data += channels * width;
	}

	//JS_DefineProperty(cx, obj, "width", INT_TO_JSVAL(newWidth), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT );
	//JS_DefineProperty(cx, obj, "height", INT_TO_JSVAL(newHeight), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT );

	JL_CHK( jl::setProperty(cx, obj, JLID(cx, width), newWidth) );
	JL_CHK( jl::setProperty(cx, obj, JLID(cx, height), newHeight) );

	*JL_RVAL = OBJECT_TO_JSVAL(obj); // allows to write: var texture = new Jpeg(f).Load().Trim(...)

	if ( !reuseBuffer )
		jl_free(tmpDataPtr);

	return true;
	JL_BAD;
}

DEFINE_FUNCTION( gamma ) {

	JL_ASSERT_ARGC_MIN(1);

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}



//	bool Call(JSContext *cx, JSObject *obj, unsigned argc, jsval *argv, jsval *rval) {
//		return true;
//	}

//	bool prop(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
//		return true;
//	}

//	bool Func(JSContext *cx, JSObject *obj, unsigned argc, jsval *argv, jsval *rval) {
//		return true;
//	}


CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision: 3533 $"))
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1)

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

END_CLASS

