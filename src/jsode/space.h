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

DECLARE_CLASS( Space );

inline JSBool ValToSpaceID( JSContext *cx, jsval val, ode::dSpaceID *spaceId ) {

	RT_ASSERT_OBJECT(val);
	JSObject *obj = JSVAL_TO_OBJECT(val);
	RT_ASSERT_CLASS(obj,&classSpace);
	*spaceId = (ode::dSpaceID)JS_GetPrivate(cx,obj);
	return JS_TRUE;
}