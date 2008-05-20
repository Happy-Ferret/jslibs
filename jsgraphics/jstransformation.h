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
#include "../common/jsNativeInterface.h"

DECLARE_CLASS(Transformation)


/* This function tries to read a matrix44 from a Transformation object OR a NI_READ_MATRIX44 interface
 * *m MUST be a valid matrix pointer BUT m MAY be modified and replaced by a private matrix pointer ( in this case, you MUST copy the data )
 * see Load for an example
 */
inline JSBool GetMatrixHelper( JSContext *cx, jsval val, Matrix44 **m ) {

	RT_ASSERT( JSVAL_IS_OBJECT(val), "Object expected." );

	if ( J_JSVAL_IS_CLASS( val, &classTransformation ) ) { // ok, we know this object and the content of its jsprivate

		*m = (Matrix44*)JS_GetPrivate(cx, JSVAL_TO_OBJECT(val));
		RT_ASSERT_RESOURCE(m); // (TBD) good place to throw an error ? ( shouldn't be the caller's job ? )
		return JS_TRUE;
	}
	
	if ( J_JSVAL_IS_ARRAY( val ) ) {

		jsuint length = 16;
		J_JSVAL_TO_REAL_VECTOR( val, (*m)->raw, length );
		J_S_ASSERT( length == 16, "Too few elements in the array." );
		return JS_TRUE;
	}

	if ( JSVAL_IS_OBJECT(val) && !JSVAL_IS_NULL(val) ) { // try to read the matrix using the NativeInterface system

		NIMatrix44Read ReadMatrix;
		J_CHECK_CALL( GetMatrix44ReadInterface(cx, JSVAL_TO_OBJECT(val), &ReadMatrix ) );
		RT_ASSERT( ReadMatrix != NULL, "Invalid matrix interface." ); // the descriptor is not always required
		J_CHECK_CALL( ReadMatrix(cx, JSVAL_TO_OBJECT(val), (float**)m) );
		return JS_TRUE;
	}

	J_REPORT_ERROR("Unable to read a 4x4 matrix.");
}
