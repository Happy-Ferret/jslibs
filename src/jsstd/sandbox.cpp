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

#include "stdafx.h"


// source: http://mxr.mozilla.org/mozilla/source/js/src/js.c

BEGIN_CLASS( Sandbox )

DEFINE_NEW_RESOLVE() {

	JSBool resolved;
   if ( (flags & JSRESOLVE_ASSIGNING) == 0) {

//		jsval tmp;
//		JS_IdToValue(cx, id, &tmp);
//		char *str = JL_GetStringBytes(JS_ValueToString(cx, tmp));

		JL_CHK( JS_ResolveStandardClass(cx, obj, id, &resolved) );
		if (resolved) {

			*objp = obj;
			return JS_TRUE;
		}
    }
    *objp = NULL;
    return JS_TRUE;
	 JL_BAD;
}

CONFIGURE_CLASS

	IS_GLOBAL
	HAS_NEW_RESOLVE

END_CLASS
