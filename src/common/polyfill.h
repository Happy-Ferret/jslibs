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


// from jsnum.h

typedef union jsdpun {
    struct {
#if defined(IS_LITTLE_ENDIAN) && !defined(FPU_IS_ARM_FPA)
        uint32_t lo, hi;
#else
        uint32_t hi, lo;
#endif
    } s;
    uint64_t u64;
    jsdouble d;
} jsdpun;

static inline int
JSDOUBLE_IS_NaN(jsdouble d)
{
    jsdpun u;
    u.d = d;
    return (u.u64 & JSDOUBLE_EXPMASK) == JSDOUBLE_EXPMASK &&
           (u.u64 & JSDOUBLE_MANTMASK) != 0;
}


// declared friend in jsiter.h

extern JS_FRIEND_API(JSBool)
js_ValueToIterator(JSContext *cx, uintN flags, js::Value *vp);

extern JS_FRIEND_API(JSBool)
js_CloseIterator(JSContext *cx, JSObject *iterObj);


// only defined as JS_FRIEND_API in js/src/js/src/jsobj.h

extern JS_FRIEND_API(JSBool)
js_GetClassPrototype(JSContext *cx, JSObject *scope, JSProtoKey protoKey,
                     JSObject **protop, js::Class *clasp = NULL);


/*
// only defined as JS_FRIEND_API in js/src/js/src/jstypedarray.h

JS_FRIEND_API(JSObject *)
js_CreateTypedArray(JSContext *cx, jsint atype, uint32_t nelements);
*/

// only defined as JS_FRIEND_API in js/src/js/src/jsdate.h

extern JS_FRIEND_API(int)
js_DateGetYear(JSContext *cx, JSObject* obj);

extern JS_FRIEND_API(int)
js_DateGetMonth(JSContext *cx, JSObject* obj);

extern JS_FRIEND_API(int)
js_DateGetDate(JSContext *cx, JSObject* obj);

extern JS_FRIEND_API(int)
js_DateGetHours(JSContext *cx, JSObject* obj);

extern JS_FRIEND_API(int)
js_DateGetMinutes(JSContext *cx, JSObject* obj);

extern JS_FRIEND_API(int)
js_DateGetSeconds(JSContext *cx, JSObject* obj);



ALWAYS_INLINE JSBool
JS_DescribeTopFrame(JSContext *cx, JSScript **script, unsigned *lineno) {

	JSStackFrame *frame = JS_GetScriptedCaller(cx, NULL);
	ASSERT( frame != NULL );
	*script = JS_GetFrameScript(cx, frame);
	*lineno = JS_PCToLineNumber(cx, *script, JS_GetFramePC(cx, frame));
	return JS_TRUE;
}
