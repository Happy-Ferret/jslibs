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

BEGIN_CLASS( SqliteError )

DEFINE_CONSTRUCTOR() {

	REPORT_ERROR( "This object cannot be construct." );
	return JS_TRUE;
}

DEFINE_PROPERTY( code ) {

	JS_GetReservedSlot( cx, obj, SLOT_SQLITE_ERROR_CODE, vp );
	return JS_TRUE;
}

DEFINE_PROPERTY( text ) {

	JS_GetReservedSlot( cx, obj, SLOT_SQLITE_ERROR_TEXT, vp );
	return JS_TRUE;
}

CONFIGURE_CLASS

	HAS_CONSTRUCTOR

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ( code )
		PROPERTY_READ( text )
	END_PROPERTY_SPEC

	HAS_RESERVED_SLOTS(2)

END_CLASS


JSBool SqliteThrowError( JSContext *cx, int status, int errorCode, const char *errorMsg ) {

	RT_SAFE(	JS_ReportWarning( cx, "SqliteError exception" ) );
	JSObject *error = JS_NewObject( cx, &classSqliteError, NULL, NULL ); // (TBD) understand why classSqliteError must have a constructor to be throwed in an exception
	RT_ASSERT( error != NULL, "Unable to create SqliteError object." );
	JS_SetReservedSlot( cx, error, SLOT_SQLITE_ERROR_CODE, INT_TO_JSVAL(errorCode) );
	JS_SetReservedSlot( cx, error, SLOT_SQLITE_ERROR_TEXT, STRING_TO_JSVAL(JS_NewStringCopyZ( cx, errorMsg )) );
	JS_SetPendingException( cx, OBJECT_TO_JSVAL( error ) );
	return JS_FALSE;
}


/*

Sure, you can JS_GetProperty(cx, global, "Error", &v) to get the constructor, then JS_GetProperty(cx, JSVAL_TO_OBJECT(v), "prototype", &v), then JS_GET_CLASS(cx, JSVAL_TO_OBJECT(v)).  Error and type checking elided, as usual.

*/
