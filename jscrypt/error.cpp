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


BEGIN_CLASS( CryptError )

DEFINE_CONSTRUCTOR() {

	REPORT_ERROR( "This object cannot be construct." ); // but constructor must be defined
	return JS_TRUE;
}

DEFINE_PROPERTY( code ) {

	JS_GetReservedSlot( cx, obj, 0, vp );
	return JS_TRUE;
}

DEFINE_PROPERTY( text ) {

	JS_GetReservedSlot( cx, obj, 0, vp );
	int errorCode = JSVAL_TO_INT(*vp);
	JSString *str = JS_NewStringCopyZ( cx, error_to_string(errorCode) );
	*vp = STRING_TO_JSVAL( str );
	return JS_TRUE;
}

DEFINE_FUNCTION( toString ) {

	RT_CHECK_CALL( text(cx, obj, 0, rval) );
	return JS_TRUE;
}

CONFIGURE_CLASS

	HAS_CONSTRUCTOR

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ( code )
		PROPERTY_READ( text )
	END_PROPERTY_SPEC

	BEGIN_FUNCTION_SPEC
		FUNCTION(toString)
	END_FUNCTION_SPEC

	HAS_RESERVED_SLOTS(1)

END_CLASS



JSBool ThrowCryptError( JSContext *cx, int errorCode ) {

	JS_ReportWarning( cx, "CryptError exception" );
	JSObject *error = JS_NewObject( cx, _class, NULL, NULL );
	JS_SetReservedSlot( cx, error, 0, INT_TO_JSVAL(errorCode) );
//	JS_SetReservedSlot( cx, error, 1, errorMessage != NULL ? STRING_TO_JSVAL(JS_NewStringCopyZ( cx, errorMessage )) : JSVAL_VOID );
	JS_SetPendingException( cx, OBJECT_TO_JSVAL( error ) );
  return JS_FALSE;
}