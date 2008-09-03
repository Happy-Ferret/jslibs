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

DECLARE_CLASS( OalError )
JSBool ThrowOalError( JSContext *cx, ALenum err );

inline JSBool CheckThrowCurrentOalError( JSContext *cx ) {

	ALenum err = alGetError();
	if (unlikely( err != AL_NO_ERROR ))
		return ThrowOalError(cx, err);
	return JS_TRUE;
}

