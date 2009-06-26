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

#include "../common/vector3.h"
#include "../common/matrix44.h"

DECLARE_CLASS(Transformation)

struct TransformationPrivate {
	bool isIdentity;
	Matrix44 *mat;
};

// This function tries to read a matrix44 from a Transformation object OR a NI_READ_MATRIX44 interface OR a javascript Array.
inline JSBool GetMatrixHelper( JSContext *cx, jsval val, float **m ) {

	JL_S_ASSERT( JSVAL_IS_OBJECT(val), J__ERRMSG_UNEXPECTED_TYPE " Object expected." );

	JSObject *matrixObj = JSVAL_TO_OBJECT(val);

	if ( JL_GetClass(matrixObj) == classTransformation ) {
		
		TransformationPrivate *pv = (TransformationPrivate *)JL_GetPrivate(cx, matrixObj);
		*m = pv->mat->raw;
		return JS_TRUE;
	}

	if ( JSVAL_IS_NULL(matrixObj) ) {
		
		memcpy(*m, &Matrix44IdentityValue, sizeof(Matrix44));
		return JS_TRUE;
	}

	NIMatrix44Get Matrix44Get = Matrix44GetInterface(cx, matrixObj);
	if ( Matrix44Get )
		return Matrix44Get(cx, matrixObj, m);

	if ( JS_IsArrayObject(cx, matrixObj) ) {

		uint32 length;
		jsval element;
		JL_CHK( JS_GetElement(cx, JSVAL_TO_OBJECT(val), 0, &element) );
		if ( JsvalIsArray(cx, element) ) { // support for: [ [1,1,1,1], [2,2,2,2], [3,3,3,3], [4,4,4,4] ] matrix

			JL_CHK( JsvalToFloatVector(cx, element, (*m)+0, 4, &length ) );
			JL_S_ASSERT( length == 4, "Too few (%d) elements in the array.", length );
			
			JL_CHK( JS_GetElement(cx, JSVAL_TO_OBJECT(val), 1, &element) );
			JL_S_ASSERT_ARRAY( element );
			JL_CHK( JsvalToFloatVector(cx, element, (*m)+4, 4, &length ) );
			JL_S_ASSERT( length == 4, "Too few (%d) elements in the array.", length );

			JL_CHK( JS_GetElement(cx, JSVAL_TO_OBJECT(val), 2, &element) );
			JL_S_ASSERT_ARRAY( element );
			JL_CHK( JsvalToFloatVector(cx, element, (*m)+8, 4, &length ) );
			JL_S_ASSERT( length == 4, "Too few (%d) elements in the array.", length );

			JL_CHK( JS_GetElement(cx, JSVAL_TO_OBJECT(val), 3, &element) );
			JL_S_ASSERT_ARRAY( element );
			JL_CHK( JsvalToFloatVector(cx, element, (*m)+12, 4, &length ) );
			JL_S_ASSERT( length == 4, "Too few (%d) elements in the array.", length );
			return JS_TRUE;
		}

		JL_CHK( JsvalToFloatVector(cx, val, *m, 16, &length ) );
		JL_S_ASSERT( length == 16, "Too few (%d) elements in the array.", length );
		return JS_TRUE;
	}

	JL_REPORT_ERROR("Unable to read a 4x4 matrix.");
	JL_BAD;
}
