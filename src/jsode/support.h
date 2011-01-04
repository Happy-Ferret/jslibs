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


#define JL_JsvalToODERealVector JL_JsvalToNativeVector

#define ODERealVectorToJsval JL_NativeVectorToJsval

ALWAYS_INLINE JSBool JL_JsvalToODEReal( JSContext *cx, const jsval val, ode::dReal *real ) {

	if ( JL_IsPInfinity(cx, val) ) {
		
		*real = dInfinity;
		return JS_TRUE;
	}

	if ( JL_IsNInfinity(cx, val) ) {
		
		*real = -dInfinity; 
		return JS_TRUE;
	}

#if defined(dSINGLE)
	JL_CHK( JL_JsvalToNative(cx, val, real) );
#else
	JL_CHK( JL_JsvalToNative(cx, val, real) );
#endif

	if ( *real > dInfinity ) {
		
		*real = dInfinity;
		return JS_TRUE;
	}

	if ( *real < -dInfinity ) {

		*real = -dInfinity;
		return JS_TRUE;
	}

	return JS_TRUE;
	JL_BAD;
}


ALWAYS_INLINE JSBool ODERealToJsval( JSContext *cx, const ode::dReal real, jsval *rval ) {
	
	if ( real >= dInfinity ) {
		
		*rval = js::Jsvalify(cx->runtime->positiveInfinityValue); // return JS_GetPositiveInfinityValue(cx);
		return JS_TRUE;
	}

	if ( real <= -dInfinity ) {
	
		*rval = js::Jsvalify(cx->runtime->negativeInfinityValue); // return JS_GetNegativeInfinityValue(cx);
		return JS_TRUE;
	}

#if defined(dSINGLE)
	JL_CHK( JL_NativeToJsval(cx, real, rval) );
#else
	JL_CHK( JL_NativeToJsval(cx, real, rval) );
#endif

	return JS_TRUE;
	JL_BAD;
}
