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

DECLARE_CLASS( Body )

struct BodyPrivate {
	JSObject *obj;
	bool hasMoved;
};


JSBool ReconstructBody(JSContext *cx, ode::dBodyID bodyId, JSObject **obj);

ALWAYS_INLINE JSBool JL_JsvalToBody( JSContext *cx, jsval val, ode::dBodyID *bodyId ) {
	
	JL_S_ASSERT_OBJECT(val);
	JSObject *obj = JSVAL_TO_OBJECT(val);
	JL_S_ASSERT_CLASS(obj, JL_CLASS(Body));
	*bodyId = (ode::dBodyID)JL_GetPrivate(cx,obj); // may be null if body is world.env
	return JS_TRUE;
	JL_BAD;
}


ALWAYS_INLINE JSBool BodyToJsval( JSContext *cx, ode::dBodyID bodyId, jsval *val ) {

	JSObject *obj;
	if (unlikely( !bodyId )) { // bodyId may be null if body is world.env
		
		JL_CHK( ReconstructBody(cx, bodyId, &obj) );
	} else {
		
		obj = (JSObject*)ode::dBodyGetData(bodyId);
		if (unlikely( !obj ))
			JL_CHK( ReconstructBody(cx, bodyId, &obj) );
	}
	JL_S_ASSERT_CLASS(obj, JL_CLASS(Body));
	*val = OBJECT_TO_JSVAL( obj );
	return JS_TRUE;
	JL_BAD;
}
