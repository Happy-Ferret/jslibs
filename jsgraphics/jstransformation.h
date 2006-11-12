#include "vector3.h"
#include "matrix44.h"
#include "../common/jsNativeInterface.h"

DECLARE_CLASS(Transformation)



/* This function tries to read a matrix44 from a Transformation object OR a NI_READ_MATRIX44 interface
 * *m MUST be a valid matrix pointer BUT m MAY be modified and replaced by a private matrix pointer ( in this case, you MUST copy the data )
 * see Load por an example
 */
inline JSBool GetMatrixHelper( JSContext *cx, jsval val, Matrix44 **m ) {

	RT_ASSERT( JSVAL_IS_OBJECT(val), "Object expected." );
	JSObject *obj = JSVAL_TO_OBJECT(val);
	if ( JS_GetClass(obj) == &classTransformation ) { // ok, we know this object and the content of its jsprivate

		*m = (Matrix44*)JS_GetPrivate(cx, obj);
		RT_ASSERT_RESOURCE(m); // [TBD] good place to throw an error ? ( shouldn't be the caller's job ? )
	} else { // try to read the matrix using the NativeInterface system

		NIMatrix44Read ReadMatrix;
		void *descriptor;
		GetNativeInterface(cx, obj, NI_READ_MATRIX44, (FunctionPointer*)&ReadMatrix, &descriptor);
		RT_ASSERT( ReadMatrix != NULL && descriptor != NULL, "Invalid matrix interface." );
		ReadMatrix(descriptor, (float**)m);
	}
	return JS_TRUE;
}
