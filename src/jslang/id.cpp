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

// the aim of *globalKey* is to ensure that a pointer in the process's virtual memory space can be serialized and unserializes safely.
static uint32 globalKey = 0;

BEGIN_CLASS( Id )

DEFINE_FINALIZE() {

	IdPrivate *pv = (IdPrivate*)JL_GetPrivate(cx, obj);
	if (!pv)
		return;
	if ( pv->finalizeCallback ) // callback function is present
		pv->finalizeCallback((char*)pv + sizeof(IdPrivate)); // (TBD) test it !
	JS_free(cx, pv);
}


DEFINE_FUNCTION_FAST( toString ) {

	IdPrivate *pv = (IdPrivate*)JL_GetPrivate(cx, JL_FOBJ);
	JSString *idStr;
	char str[] = "[Id XXXX]";

	if ( DetectSystemEndianType() == LittleEndian ) {

		str[4] = ((char*)&pv->idType)[3];
		str[5] = ((char*)&pv->idType)[2];
		str[6] = ((char*)&pv->idType)[1];
		str[7] = ((char*)&pv->idType)[0];
	} else {

		*((ID_TYPE*)str+4) = pv->idType;
	}

	idStr = JS_NewStringCopyN(cx, str, sizeof(str));
	JL_CHK(idStr);
	*JL_FRVAL = STRING_TO_JSVAL(idStr);
	return JS_TRUE;
	JL_BAD;
}


DEFINE_HAS_INSTANCE() { // see issue#52

	*bp = !JSVAL_IS_PRIMITIVE(v) && JL_GetClass(JSVAL_TO_OBJECT(v)) == _class;
	return JS_TRUE;
}

DEFINE_INIT() {

	JL_SAFE_BEGIN
		globalKey = JLSessionId();
	JL_SAFE_END
	return JS_TRUE;
}

/*
DEFINE_XDR() {
	
	JSContext *cx = xdr->cx;

	jsid id;
	jsval key, value;

	if ( xdr->mode == JSXDR_ENCODE ) {

		JS_XDRUint32(xdr, &globalKey);

		return JS_TRUE;
	}
	
	if ( xdr->mode == JSXDR_DECODE ) {

		uint32 gKey;
		JS_XDRUint32(xdr, &gKey);
		JL_S_ASSERT( gKey == globalKey, "Incompatible Id." );

		return JS_TRUE;
	}

	if ( xdr->mode == JSXDR_FREE ) {

		// (TBD) nothing to free ?
		return JS_TRUE;
	}

	return JS_FALSE;
}
*/


CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_INIT
	HAS_PRIVATE
	HAS_FINALIZE
	HAS_HAS_INSTANCE
//	HAS_XDR

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST( toString )
	END_FUNCTION_SPEC
	
END_CLASS
