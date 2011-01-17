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

DECLARE_CLASS( OglError )

#define GLERR_EXTENSION_NOT_FOUND 0x0001

JSBool ThrowOglError( JSContext *cx, GLenum err );

JSBool ThrowOglCustomError( JSContext *cx, int err );

inline JSBool CheckThrowCurrentOglError( JSContext *cx ) {

	GLenum err = glGetError();
	if (unlikely( err != GL_NO_ERROR ))
		return ThrowOglError(cx, err);
	return JS_TRUE;
}

