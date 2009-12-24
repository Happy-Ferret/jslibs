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

#include "error.h"


/**doc fileIndex:bottom **/

/**doc
----
== jssdl::SdlError class ==
 You cannot construct this class.$LF
 Its aim is to catch jssdl runtime error exception.
**/

BEGIN_CLASS( SdlError )


/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Is the text of the error.
**/
DEFINE_PROPERTY( text ) {

	return JL_GetReservedSlot( cx, obj, 0, vp );
}

DEFINE_FUNCTION( toString ) {

	return _text(cx, obj, 0, rval);
}

DEFINE_HAS_INSTANCE() { // see issue#52

	*bp = !JSVAL_IS_PRIMITIVE(v) && JL_GetClass(JSVAL_TO_OBJECT(v)) == JL_THIS_CLASS;
	return JS_TRUE;
}


DEFINE_XDR() {

	if ( xdr->mode == JSXDR_ENCODE ) {

		jsval tmp;
		JL_CHK( JL_GetReservedSlot(xdr->cx, *objp, 0, &tmp) );
		JS_XDRValue(xdr, &tmp);
		return JS_TRUE;
	}

	if ( xdr->mode == JSXDR_DECODE ) {

		*objp = JS_NewObject(xdr->cx, JL_THIS_CLASS, NULL, NULL);
		jsval tmp;
		JS_XDRValue(xdr, &tmp);
		JL_CHK( JS_SetReservedSlot(xdr->cx, *objp, 0, tmp) );
		return JS_TRUE;
	}

	if ( xdr->mode == JSXDR_FREE ) {

		// (TBD) nothing to free ?
		return JS_TRUE;
	}

	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))

	HAS_XDR
	HAS_HAS_INSTANCE // see issue#52

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ( text )
	END_PROPERTY_SPEC

	BEGIN_FUNCTION_SPEC
		FUNCTION(toString)
	END_FUNCTION_SPEC

	HAS_RESERVED_SLOTS(1)

END_CLASS


JSBool ThrowSdlError( JSContext *cx ) {

	JSObject *errorObj = JS_NewObject( cx, JL_CLASS(SdlError), NULL, NULL );
	JS_SetPendingException( cx, OBJECT_TO_JSVAL( errorObj ) );
	const char *errorMessage = SDL_GetError();
	SDL_ClearError();
//	if ( errorMessage == NULL || *errorMessage == '\0' )
//		errorMessage = "Undefined error";
	JS_SetReservedSlot( cx, errorObj, 0, errorMessage != NULL && *errorMessage != '\0' ? STRING_TO_JSVAL(JS_NewStringCopyZ( cx, errorMessage )) : JSVAL_VOID );
	JL_SAFE( ExceptionSetScriptLocation(cx, errorObj) );
	JL_BAD;
}
