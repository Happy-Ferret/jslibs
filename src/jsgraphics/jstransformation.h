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


/* This function tries to read a matrix44 from a Transformation object OR a NI_READ_MATRIX44 interface
 * *m MUST be a valid matrix pointer BUT m MAY be modified and replaced by a private matrix pointer ( in this case, you MUST copy the data )
 * see Load for an example
 */
inline JSBool GetMatrixHelper( JSContext *cx, jsval val, Matrix44 **m ) {

	JL_S_ASSERT_OBJECT(val);
	JSObject *matrixObj = JSVAL_TO_OBJECT(val);

	NIMatrix44Get MatrixGet = Matrix44GetInterface(cx, matrixObj);
	if ( MatrixGet != NULL ) {

		JL_CHK( MatrixGet(cx, matrixObj, (float**)m));
		return JS_TRUE;
	}

	if ( JsvalIsArray(cx, val) ) {

		size_t length;
		JL_CHK( JsvalToFloatVector(cx, val, (*m)->raw, 16, &length ) );
		JL_S_ASSERT( length == 16, "Too few (%d) elements in the array.", length );
		return JS_TRUE;
	}

	JL_REPORT_ERROR("Unable to read a 4x4 matrix.");
	JL_BAD;
}
