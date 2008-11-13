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
#include "idPub.h"

static int globalKey = 0;

BEGIN_CLASS( Id )

DEFINE_FINALIZE() {

	void **pv = (void **)JS_GetPrivate(cx, obj);
	if (pv != NULL) {
	
		if ( pv[1] ) // callback function is present
			((IdFinalizeCallback_t)pv[1])(pv + 2);

		JS_free(cx, pv);
	}
}

DEFINE_HAS_INSTANCE() { // see issue#52

	*bp = !JSVAL_IS_PRIMITIVE(v) && OBJ_GET_CLASS(cx, JSVAL_TO_OBJECT(v)) == _class;
	return JS_TRUE;
}

DEFINE_INIT() {

	while ( globalKey == 0 ) {
		
		globalKey = rand(); // (TBD) more random !!!
	}
	return JS_TRUE;	
}

DEFINE_XDR() {
	
	jsid id;
	jsval key, value;

	if ( xdr->mode == JSXDR_ENCODE ) {

		return JS_TRUE;
	}
	
	if ( xdr->mode == JSXDR_DECODE ) {

		return JS_TRUE;
	}

	if ( xdr->mode == JSXDR_FREE ) {

		// (TBD) nothing to free ?
		return JS_TRUE;
	}

	return JS_FALSE;
}



CONFIGURE_CLASS

	HAS_INIT
	HAS_PRIVATE
	HAS_FINALIZE
	HAS_HAS_INSTANCE
	HAS_XDR

END_CLASS
