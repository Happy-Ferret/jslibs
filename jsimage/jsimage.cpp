#include "stdafx.h"
#include "jsimage.h"

#include "../smtools/smtools.h"

#include <stdlib.h>


BEGIN_CLASS

DEFINE_FINALIZE() {

	void *data = JS_GetPrivate(cx, obj);
	if ( data != NULL )
		free(data);
}

DEFINE_FUNCTION( Alloc ) {

	RT_ASSERT_CLASS(obj, thisClass);
	RT_ASSERT_ARGC(1);

	void *data = JS_GetPrivate(cx, obj);
	if ( data != NULL )
		free(data);

	unsigned int size = JSVAL_TO_INT(argv[0]);
	data = malloc(size);
	JS_SetPrivate(cx, obj, data);

	return JS_TRUE;
}


DEFINE_FUNCTION( ClassConstruct ) {

	RT_ASSERT_CONSTRUCTING(thisClass);
	JSFunction *allocFunction = JS_NewFunction(cx, Alloc, 0, 0, NULL, "Alloc");
	RT_ASSERT( allocFunction != NULL, "Unable to create allocation function." );
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

	RT_ASSERT_ARGC(1);
	int vect[4];
	IntArrayToVector(cx, 4, &argv[0], vect);
	int x = vect[0];
	int y = vect[1];
	int x1 = vect[2];
	int y1 = vect[3];

	jsval tmp;
	JS_GetProperty(cx, obj, "width", &tmp);
	RT_ASSERT(JSVAL_IS_INT(tmp), RT_ERROR_UNEXPECTED_TYPE);
	int width = JSVAL_TO_INT(tmp);

	JS_GetProperty(cx, obj, "height", &tmp);
	RT_ASSERT(JSVAL_IS_INT(tmp), RT_ERROR_UNEXPECTED_TYPE);
	int height = JSVAL_TO_INT(tmp);

	JS_GetProperty(cx, obj, "channels", &tmp);
	RT_ASSERT(JSVAL_IS_INT(tmp), RT_ERROR_UNEXPECTED_TYPE);
	int channels = JSVAL_TO_INT(tmp);
	// assume that we have 1 Byte/channel

	RT_ASSERT( !(x<0 || x1<0 || x>width || x1>width || y<0 || y1<0 || y>height || y1>height), "Invalid size." );

	JSBool reuseBuffer = false; // default
	if ( argc >= 2 )
		JS_ValueToBoolean(cx, argv[1], &reuseBuffer);

	char *data = (char*)JS_GetPrivate(cx, obj);
	RT_ASSERT(data!=NULL, RT_ERROR_NOT_INITIALIZED);
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

	RT_ASSERT_ARGC(1);

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

	BEGIN_FUNCTION_MAP
		FUNCTION(Trim)
		FUNCTION(Gamma)
//		FUNCTION(Alloc) // see SLOT_FUNCTION_ALLOC
		FUNCTION(Free)
	END_MAP
	BEGIN_PROPERTY_MAP
		CREATE(width)
		CREATE(height)
		CREATE(channels)
//		CREATE(pixelSize)
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

END_CLASS(Image, HAS_PRIVATE, 1)

