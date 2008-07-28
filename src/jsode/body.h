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

DECLARE_CLASS( Body )

#define BODY_SLOT_WORLD 0 // the world

inline JSBool ValToBodyID( JSContext *cx, jsval val, ode::dBodyID *bodyId ) {

	J_S_ASSERT_OBJECT(val);
	JSObject *obj = JSVAL_TO_OBJECT(val);
	J_S_ASSERT_CLASS(obj, classBody);
	*bodyId = (ode::dBodyID)JS_GetPrivate(cx,obj); // (TBD) ! manage null body ( environment connected; see world.body property )
	// *bodyId == NULL is not an error !
	// (TBD) use another way to detect if body is correct
	return JS_TRUE;
}
