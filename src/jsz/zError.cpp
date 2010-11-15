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

#include "zError.h"

#include <zlib.h>

/**doc fileIndex:bottom **/

/**doc
----
== jsz::ZError class ==
 You cannot construct this class.$LF
 Its aim is to throw as an exception on any zlib runtime error.
**/

const char *ZConstString( int errorCode ) {

	switch (errorCode) {
		case Z_OK: return "Z_OK";
		case Z_STREAM_END: return "Z_STREAM_END";
		case Z_NEED_DICT: return "Z_NEED_DICT";
		case Z_ERRNO: return "Z_ERRNO";
		case Z_STREAM_ERROR: return "Z_STREAM_ERROR";
		case Z_DATA_ERROR: return "Z_DATA_ERROR";
		case Z_MEM_ERROR: return "Z_MEM_ERROR";
		case Z_BUF_ERROR: return "Z_BUF_ERROR";
		case Z_VERSION_ERROR: return "Z_VERSION_ERROR";
	}
	return "UNKNOWN_ERROR";
}



BEGIN_CLASS( ZError )


DEFINE_PROPERTY( code ) {

	return JL_GetReservedSlot( cx, obj, 0, vp );
}

DEFINE_PROPERTY( text ) {

	return JL_GetReservedSlot( cx, obj, 1, vp );
}

DEFINE_PROPERTY( const ) {

	JL_GetReservedSlot( cx, obj, 0, vp );
	if ( JSVAL_IS_VOID(*vp) )
		return JS_TRUE;
	int errorCode = JSVAL_TO_INT(*vp);
	JSString *str = JS_NewStringCopyZ( cx, ZConstString(errorCode) );
	*vp = STRING_TO_JSVAL( str );
	return JS_TRUE;
}


DEFINE_FUNCTION( toString ) {

	JL_DEFINE_FUNCTION_OBJ;
	return _text(cx, obj, JSID_EMPTY, JL_RVAL);
}

DEFINE_HAS_INSTANCE() { // see issue#52

	*bp = !JSVAL_IS_PRIMITIVE(*v) && JL_InheritFrom(cx, JSVAL_TO_OBJECT(*v), JL_THIS_CLASS);
	return JS_TRUE;
}


DEFINE_XDR() {

	if ( xdr->mode == JSXDR_ENCODE ) {

		jsval tmp;
		JL_CHK( JL_GetReservedSlot(xdr->cx, *objp, 0, &tmp) );
		JS_XDRValue(xdr, &tmp);
		JL_CHK( JL_GetReservedSlot(xdr->cx, *objp, 1, &tmp) );
		JS_XDRValue(xdr, &tmp);
		return JS_TRUE;
	}

	if ( xdr->mode == JSXDR_DECODE ) {

		*objp = JS_NewObject(xdr->cx, JL_THIS_CLASS, NULL, NULL);
		jsval tmp;
		JS_XDRValue(xdr, &tmp);
		JL_CHK( JL_SetReservedSlot(xdr->cx, *objp, 0, tmp) );
		JS_XDRValue(xdr, &tmp);
		JL_CHK( JL_SetReservedSlot(xdr->cx, *objp, 1, tmp) );
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
		PROPERTY_READ( code )
		PROPERTY_READ( text )
		PROPERTY_READ( const )
	END_PROPERTY_SPEC

	BEGIN_FUNCTION_SPEC
		FUNCTION(toString)
	END_FUNCTION_SPEC

	HAS_RESERVED_SLOTS(2)

END_CLASS


JSBool ThrowZError( JSContext *cx, int errorCode, const char *errorMessage ) {

	JSObject *error = JS_NewObjectWithGivenProto( cx, JL_CLASS(ZError), JL_PROTOTYPE(cx, ZError), NULL );
	JS_SetPendingException( cx, OBJECT_TO_JSVAL( error ) );
	JL_CHK( JL_SetReservedSlot( cx, error, 0, INT_TO_JSVAL(errorCode) ) );
	JL_CHK( JL_SetReservedSlot( cx, error, 1, STRING_TO_JSVAL(JS_NewStringCopyZ( cx, errorMessage != NULL ? errorMessage : ZConstString(errorCode) )) ) );
	JL_SAFE( JL_ExceptionSetScriptLocation(cx, error) );
	return JS_FALSE;
	JL_BAD;
}
