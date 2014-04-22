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

/*
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
    double d;
} jsdpun;

static inline int
JSDOUBLE_IS_NaN(double d)
{
    jsdpun u;
    u.d = d;
    return (u.u64 & JSDOUBLE_EXPMASK) == JSDOUBLE_EXPMASK &&
           (u.u64 & JSDOUBLE_MANTMASK) != 0;
}
*/

/*
// declared friend in jsiter.h

extern JS_FRIEND_API(bool)
js_ValueToIterator(JSContext *cx, unsigned flags, js::Value *vp);

extern JS_FRIEND_API(bool)
js_CloseIterator(JSContext *cx, JSObject *iterObj);
*/



/*
ALWAYS_INLINE JSObject *
JS_NewObjectForConstructor(JSContext *cx, const JSClass *cl, const jsval *vp) {

	return NULL;
}
*/



