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
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( OalError )

/**doc
$TOC_MEMBER $INAME
 $INT $INAME
  OpenAL error number.
**/
DEFINE_PROPERTY( code ) {

	return JL_GetReservedSlot( cx, obj, 0, vp );
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME
  OpenAL error literal.
**/
DEFINE_PROPERTY( text ) {

	JL_CHK( JL_GetReservedSlot( cx, obj, 0, vp ) );
	if ( JSVAL_IS_VOID(*vp) )
		return JS_TRUE;
	int errorCode;
	JL_CHK( 	JL_JsvalToCVal(cx, *vp, &errorCode) );
	char *errStr;
	switch (errorCode) {
		case AL_NO_ERROR:
			errStr = "No Error.";
			break;
		case AL_INVALID_NAME:
			errStr = "Invalid Name paramater passed to AL call.";
			break;
		case AL_ILLEGAL_ENUM: // AL_INVALID_ENUM
			errStr = "Invalid parameter passed to AL call.";
			break;
		case AL_INVALID_VALUE:
			errStr = "Invalid enum parameter value.";
			break;
		case AL_ILLEGAL_COMMAND: // AL_INVALID_OPERATION
			errStr = "Illegal call.";
			break;
		case AL_OUT_OF_MEMORY:
			errStr = "No mojo.";
			break;
		default:
			errStr = "Unknown error.";
			break;
	}
	JSString *str = JS_NewStringCopyZ( cx, errStr );
	*vp = STRING_TO_JSVAL( str );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $STR $INAME
  Const name of the OpenAL error.
**/
DEFINE_PROPERTY( const ) {

	JL_CHK( JL_GetReservedSlot( cx, obj, 0, vp ) );
	if ( JSVAL_IS_VOID(*vp) )
		return JS_TRUE;
	int errorCode;
	JL_CHK( 	JL_JsvalToCVal(cx, *vp, &errorCode) );
	char *errStr;
	switch (errorCode) {
		case AL_NO_ERROR:
			errStr = "AL_NO_ERROR";
			break;
		case AL_INVALID_NAME:
			errStr = "AL_INVALID_NAME";
			break;
		case AL_ILLEGAL_ENUM: // AL_INVALID_ENUM
			errStr = "AL_ILLEGAL_ENUM";
			break;
		case AL_INVALID_VALUE:
			errStr = "AL_INVALID_VALUE";
			break;
		case AL_ILLEGAL_COMMAND: // AL_INVALID_OPERATION
			errStr = "AL_ILLEGAL_COMMAND";
			break;
		case AL_OUT_OF_MEMORY:
			errStr = "AL_OUT_OF_MEMORY";
			break;
		default:
			errStr = "???";
			break;
	}
	JSString *str = JS_NewStringCopyZ( cx, errStr );
	*vp = STRING_TO_JSVAL( str );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $STR $INAME
  see Text().
**/
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
		return JS_TRUE;
	}

	if ( xdr->mode == JSXDR_DECODE ) {

		*objp = JS_NewObject(xdr->cx, JL_THIS_CLASS, NULL, NULL);
		jsval tmp;
		JS_XDRValue(xdr, &tmp);
		JL_CHK( JL_SetReservedSlot(xdr->cx, *objp, 0, tmp) );
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
	HAS_RESERVED_SLOTS(1)
	HAS_HAS_INSTANCE // see issue#52

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ( code )
		PROPERTY_READ( text )
		PROPERTY_READ( const )
	END_PROPERTY_SPEC

	BEGIN_FUNCTION_SPEC
		FUNCTION(toString)
	END_FUNCTION_SPEC

END_CLASS


JSBool ThrowOalError( JSContext *cx, ALenum err ) {

	JSObject *error = JS_NewObjectWithGivenProto( cx, JL_CLASS(OalError), JL_PROTOTYPE(cx, OalError), NULL );
	JS_SetPendingException( cx, OBJECT_TO_JSVAL( error ) );
	jsval errVal;
	JL_CHK( JL_CValToJsval(cx, err, &errVal) );
	JL_CHK( JL_SetReservedSlot( cx, error, 0, errVal ) );
	JL_SAFE( JL_ExceptionSetScriptLocation(cx, error) );
	JL_BAD;
}
