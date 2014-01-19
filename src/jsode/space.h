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

DECLARE_CLASS( Space );

ALWAYS_INLINE bool JL_JsvalIsSpace( const jsval val ) {

	return !JSVAL_IS_PRIMITIVE(val) && JL_GetClass(JSVAL_TO_OBJECT( val )) == JL_CLASS(Space);
}

ALWAYS_INLINE bool JL_JsvalToSpaceID( JSContext *cx, jsval val, ode::dSpaceID *spaceId ) {

	JL_ASSERT_IS_OBJECT(val, JL_CLASS_NAME(Space));
	JSObject *obj = JSVAL_TO_OBJECT(val);
	JL_ASSERT_INSTANCE(obj, JL_CLASS(Space));
	*spaceId = (ode::dSpaceID)JL_GetPrivate(obj);
	return true;
	JL_BAD;
}
