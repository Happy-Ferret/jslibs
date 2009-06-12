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


ALWAYS_INLINE ode::dReal JSValToODEReal( JSContext *cx, jsval val ) {

	if ( JsvalIsPInfinity(cx, val) )
		return dInfinity;
	if ( JsvalIsNInfinity(cx, val) )
		return -dInfinity;

	ode::dReal value;
#if defined(dSINGLE)
	JL_CHK( JsvalToFloat(cx, val, &value) );
#else
	JL_CHK( JsvalToDouble(cx, val, &value) );
#endif

	if ( value > dInfinity )
		return dInfinity;
	if ( value < -dInfinity )
		return -dInfinity;
	return value;

bad:
	return 0;
}

ALWAYS_INLINE jsval ODERealToJsval( JSContext *cx, ode::dReal val ) {
	
	if ( val >= dInfinity )
		return JS_GetPositiveInfinityValue(cx);
	if ( val <= -dInfinity )
		return JS_GetNegativeInfinityValue(cx);

	jsval value;
#if defined(dSINGLE)
	JL_CHK( FloatToJsval(cx, val, &value) );
#else
	JL_CHK( DoubleVectorToJsval(cx, val, &value) );
#endif
	return value;

bad:
	return 0;
}

