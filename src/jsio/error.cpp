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

/**doc fileIndex:bottom **/

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
 You cannot construct this class.$LF
 Its aim is to throw as an exception on any NSPR runtime error.
**/
BEGIN_CLASS( IoError )


/**doc
=== Properties ===
**/


/* see issue#52
DEFINE_CONSTRUCTOR() {

	JL_REPORT_ERROR( "This object cannot be construct." ); // (TBD) remove constructor and define HAS_HAS_INSTANCE
	return JS_TRUE;
}
*/

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
*/
DEFINE_PROPERTY( code ) {

	JS_GetReservedSlot( cx, obj, 0, vp );  // (TBD) use the obj.name proprety directly instead of slot 0 ?
	return JS_TRUE;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
*/
DEFINE_PROPERTY( os ) {

	JS_GetReservedSlot( cx, obj, 1, vp );  // (TBD) use the obj.name proprety directly instead of slot 1 ?
	return JS_TRUE;
}

/**doc
$TOC_MEMBER $INAME
 $STR $INAME $READONLY
*/
DEFINE_PROPERTY( text ) {

	JS_GetReservedSlot( cx, obj, 0, vp );  // (TBD) use the obj.name proprety directly instead of slot 0 ?
	PRErrorCode errorCode = JSVAL_TO_INT(*vp);
	JSString *str = JS_NewStringCopyZ( cx, PR_ErrorToString( errorCode, PR_LANGUAGE_EN ) );
	*vp = STRING_TO_JSVAL( str );
	return JS_TRUE;
}

DEFINE_FUNCTION( toString ) {

	JL_CHK( _text(cx, obj, 0, rval) );
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
		JL_CHK( JS_GetReservedSlot(xdr->cx, *objp, 0, &tmp) );
		JS_XDRValue(xdr, &tmp);
		JL_CHK( JS_GetReservedSlot(xdr->cx, *objp, 1, &tmp) );
		JS_XDRValue(xdr, &tmp);
		return JS_TRUE;
	}

	if ( xdr->mode == JSXDR_DECODE ) {

		*objp = JS_NewObject(xdr->cx, classIoError, NULL, NULL);
		jsval tmp;
		JS_XDRValue(xdr, &tmp);
		JL_CHK( JS_SetReservedSlot(xdr->cx, *objp, 0, tmp) );
		JS_XDRValue(xdr, &tmp);
		JL_CHK( JS_SetReservedSlot(xdr->cx, *objp, 1, tmp) );
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
		PROPERTY_READ( os )
		PROPERTY_READ( text )
	END_PROPERTY_SPEC

	BEGIN_FUNCTION_SPEC
		FUNCTION(toString)
	END_FUNCTION_SPEC

	HAS_RESERVED_SLOTS(2)

END_CLASS



JSBool ThrowIoErrorArg( JSContext *cx, PRErrorCode errorCode, PRInt32 osError ) {

	JSObject *error = JS_NewObject( cx, classIoError, NULL, NULL );
	JS_SetPendingException( cx, OBJECT_TO_JSVAL( error ) );
	JS_SetReservedSlot( cx, error, 0, INT_TO_JSVAL(errorCode) );
	JS_SetReservedSlot( cx, error, 1, INT_TO_JSVAL(osError) );
	JL_SAFE( ExceptionSetScriptLocation(cx, error) );
	JL_BAD;
}


JSBool ThrowIoError( JSContext *cx ) {

	return ThrowIoErrorArg(cx, PR_GetError(), PR_GetOSError());
}
