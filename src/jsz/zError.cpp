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
 You cannot construct this class.
 Its aim is to throw as an exception on any zlib runtime error.
**/

BEGIN_CLASS( ZError )


DEFINE_PROPERTY( code ) {

	JS_GetReservedSlot( cx, obj, 0, vp );
	return JS_TRUE;
}

DEFINE_PROPERTY( text ) {

	JS_GetReservedSlot( cx, obj, 1, vp );
	return JS_TRUE;
}

DEFINE_PROPERTY( const ) {

	JS_GetReservedSlot( cx, obj, 0, vp );
	int errorCode = JSVAL_TO_INT(*vp);

	char *errStr;
	switch (errorCode) {
		case 0:
			errStr = "Z_OK";
			break;
		case 1:
			errStr = "Z_STREAM_END";
			break;
		case 2:
			errStr = "Z_NEED_DICT";
			break;
		case -1:
			errStr = "Z_ERRNO";
			break;
		case -2:
			errStr = "Z_STREAM_ERROR";
			break;
		case -3:
			errStr = "Z_DATA_ERROR";
			break;
		case -4:
			errStr = "Z_MEM_ERROR";
			break;
		case -5:
			errStr = "Z_BUF_ERROR";
			break;
		case -6:
			errStr = "Z_VERSION_ERROR";
			break;
		default:
			errStr = "UNKNOWN_ERROR";
	}

	JSString *str = JS_NewStringCopyZ( cx, errStr );
	*vp = STRING_TO_JSVAL( str );
	return JS_TRUE;
}


DEFINE_FUNCTION( toString ) {

	J_CHK( _text(cx, obj, 0, rval) );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_HAS_INSTANCE() { // see issue#52

	*bp = !JSVAL_IS_PRIMITIVE(v) && OBJ_GET_CLASS(cx, JSVAL_TO_OBJECT(v)) == _class;
	return JS_TRUE;
}


DEFINE_XDR() {
	
	if ( xdr->mode == JSXDR_ENCODE ) {

		jsval tmp;
		J_CHK( JS_GetReservedSlot(xdr->cx, *objp, 0, &tmp) );
		JS_XDRValue(xdr, &tmp);
		J_CHK( JS_GetReservedSlot(xdr->cx, *objp, 1, &tmp) );
		JS_XDRValue(xdr, &tmp);
		return JS_TRUE;
	}

	if ( xdr->mode == JSXDR_DECODE ) {

		*objp = JS_NewObject(xdr->cx, _class, NULL, NULL);
		jsval tmp;
		JS_XDRValue(xdr, &tmp);
		J_CHK( JS_SetReservedSlot(xdr->cx, *objp, 0, tmp) );
		JS_XDRValue(xdr, &tmp);
		J_CHK( JS_SetReservedSlot(xdr->cx, *objp, 1, tmp) );
		return JS_TRUE;
	}

	if ( xdr->mode == JSXDR_FREE ) {

		// (TBD) nothing to free ?
		return JS_TRUE;
	}

	JL_BAD;
}


CONFIGURE_CLASS

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

	JSObject *error = JS_NewObject( cx, _class, NULL, NULL );
	JS_SetPendingException( cx, OBJECT_TO_JSVAL( error ) );
	JS_SetReservedSlot( cx, error, 0, INT_TO_JSVAL(errorCode) );
	JS_SetReservedSlot( cx, error, 1, errorMessage != NULL ? STRING_TO_JSVAL(JS_NewStringCopyZ( cx, errorMessage )) : JSVAL_VOID );
  return JS_FALSE;
}
