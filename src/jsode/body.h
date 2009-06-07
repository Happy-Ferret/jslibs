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

extern bool _odeFinalization;

DECLARE_CLASS( Body )

JSBool ReconstructBody(JSContext *cx, ode::dBodyID bodyId, JSObject **obj);

ALWAYS_INLINE JSBool JsvalToBody( JSContext *cx, jsval val, ode::dBodyID *bodyId ) {
	
	JL_S_ASSERT_OBJECT(val);
	JSObject *obj = JSVAL_TO_OBJECT(val);
	JL_S_ASSERT_CLASS(obj, classBody);
	*bodyId = (ode::dBodyID)JL_GetPrivate(cx,obj); // may be null if body is world.env
	return JS_TRUE;
	JL_BAD;
}


ALWAYS_INLINE JSBool BodyToJsval( JSContext *cx, ode::dBodyID bodyId, jsval *val ) {

	JSObject *obj = (JSObject*)ode::dBodyGetData(bodyId);
	if (unlikely( !obj ))
		JL_CHK( ReconstructBody(cx, bodyId, &obj) );
	*val = OBJECT_TO_JSVAL( obj );
	return JS_TRUE;
	JL_BAD;
}
