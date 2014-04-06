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


#pragma once

#include "vector3.h"
#include "matrix44.h"

DECLARE_CLASS(Transformation)

struct TransformationPrivate {
	bool isIdentity;
	Matrix44 *mat;
};

// This function tries to read a matrix44 from a Transformation object OR a NI_READ_MATRIX44 interface OR a javascript Array.
inline bool GetMatrixHelper( JSContext *cx, JS::MutableHandleValue val, float **m ) {

	if ( val.isObject() ) {

		JS::RootedObject matrixObj(cx, &val.toObject());
		if ( JL_GetClass(matrixObj) == JL_CLASS(Transformation) ) {

			TransformationPrivate *pv = (TransformationPrivate *)JL_GetPrivate(matrixObj);
			JL_ASSERT_OBJECT_STATE( pv, JL_CLASS_NAME(Transformation) );
			*m = pv->mat->raw;
			return true;
		}
	}
	return jl::getMatrix44(cx, val, m);
	JL_BAD;
}
