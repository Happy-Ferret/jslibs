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

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( WinError )

/* see issue#52
DEFINE_CONSTRUCTOR() {

	JL_REPORT_ERROR( "This object cannot be construct." ); // (TBD) remove constructor and define HAS_HAS_INSTANCE
	return JS_TRUE;
}
*/

DEFINE_PROPERTY( code ) {

	return JS_GetReservedSlot( cx, obj, SLOT_WIN_ERROR_CODE, vp );
}

DEFINE_PROPERTY( text ) {

	JL_CHK( JS_GetReservedSlot(cx, obj, SLOT_WIN_ERROR_CODE, vp) );
	if ( JSVAL_IS_VOID(*vp) )
		return JS_TRUE;
	DWORD messageId = JSVAL_TO_INT(*vp);
	LPVOID lpvMessageBuffer;
	DWORD res = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, messageId, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&lpvMessageBuffer, 0, NULL);
	if ( res == 0 )
		return JS_FALSE;

	if ( ((char*)lpvMessageBuffer)[res-2] == '\r' ) { // remove the trailing CRLF

		res -= 2;
		((char*)lpvMessageBuffer)[res] = '\0';
	}

	*vp = STRING_TO_JSVAL(JS_NewStringCopyN( cx, (char*)lpvMessageBuffer, res+1 )); // doc: If the function succeeds, the return value is the number of TCHARs stored in the output buffer, excluding the terminating null character.
	LocalFree(lpvMessageBuffer);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_HAS_INSTANCE() { // see issue#52

	*bp = !JSVAL_IS_PRIMITIVE(v) && JL_GetClass(JSVAL_TO_OBJECT(v)) == _class;
	return JS_TRUE;
}

DEFINE_XDR() {

	if ( xdr->mode == JSXDR_ENCODE ) {

		jsval tmp;
		JL_CHK( JS_GetReservedSlot(xdr->cx, *objp, SLOT_WIN_ERROR_CODE, &tmp) );
		JS_XDRValue(xdr, &tmp);
		return JS_TRUE;
	}

	if ( xdr->mode == JSXDR_DECODE ) {

		*objp = JS_NewObject(xdr->cx, _class, NULL, NULL);
		jsval tmp;
		JS_XDRValue(xdr, &tmp);
		JL_CHK( JS_SetReservedSlot(xdr->cx, *objp, SLOT_WIN_ERROR_CODE, tmp) );
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
//	HAS_CONSTRUCTOR // see issue#52
	HAS_HAS_INSTANCE // see issue#52

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ( code )
		PROPERTY_READ( text )
	END_PROPERTY_SPEC

	HAS_RESERVED_SLOTS(1)

END_CLASS


JSBool WinThrowError( JSContext *cx, DWORD errorCode ) {

//	JL_SAFE(	JS_ReportWarning( cx, "WinError exception" ) );
	JSObject *error = JS_NewObject( cx, classWinError, NULL, NULL ); // (TBD) understand why it must have a constructor to be throwed in an exception
//	JL_S_ASSERT( error != NULL, "Unable to create WinError object." );
	JS_SetPendingException( cx, OBJECT_TO_JSVAL( error ) );
	JS_SetReservedSlot( cx, error, SLOT_WIN_ERROR_CODE, INT_TO_JSVAL(errorCode) );
	JL_SAFE( ExceptionSetScriptLocation(cx, error) );
	JL_BAD;
}


/*

Sure, you can JS_GetProperty(cx, global, "Error", &v) to get the constructor, then JS_GetProperty(cx, JSVAL_TO_OBJECT(v), "prototype", &v), then JL_GetClass(JSVAL_TO_OBJECT(v)).  Error and type checking elided, as usual.

*/
