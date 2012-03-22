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
$SVN_REVISION $Revision: 3524 $
**/
BEGIN_CLASS( OalError )

/**doc
$TOC_MEMBER $INAME
 $INT $INAME
  OpenAL error number.
**/
DEFINE_PROPERTY_GETTER( code ) {

	JL_IGNORE(cx, id);
	return JL_GetReservedSlot( cx, obj, 0, vp );
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME
  OpenAL error literal.
**/
DEFINE_PROPERTY_GETTER( text ) {

	JL_IGNORE(cx, id);
	JL_CHK( JL_GetReservedSlot( cx, obj, 0, vp ) );
	if ( JSVAL_IS_VOID(*vp) )
		return JS_TRUE;
	int errorCode;
	JL_CHK( JL_JsvalToNative(cx, *vp, &errorCode) );
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
DEFINE_PROPERTY_GETTER( const ) {

	JL_IGNORE(id);
	JL_CHK( JL_GetReservedSlot( cx, obj, 0, vp ) );
	if ( JSVAL_IS_VOID(*vp) )
		return JS_TRUE;
	int errorCode;
	JL_CHK( JL_JsvalToNative(cx, *vp, &errorCode) );
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

	JL_IGNORE(argc);
	JL_DEFINE_FUNCTION_OBJ;
	return _textGetter(cx, obj, JSID_EMPTY, JL_RVAL);
}


/*
DEFINE_FUNCTION( _serialize ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_TYPE( jl::JsvalIsSerializer(cx, JL_ARG(1)), 1, "Serializer" );

	jl::Serializer *ser;
	ser = jl::JsvalToSerializer(cx, JL_ARG(1));

	JL_CHK( JS_GetPropertyById(cx, JL_OBJ, JLID(cx, fileName), JL_RVAL) );
	JL_CHK( ser->Write(cx, *JL_RVAL) );
	JL_CHK( JS_GetPropertyById(cx, JL_OBJ, JLID(cx, lineNumber), JL_RVAL) );
	JL_CHK( ser->Write(cx, *JL_RVAL) );
	JL_CHK( JL_GetReservedSlot(cx, JL_OBJ, 0, JL_RVAL) );
	JL_CHK( ser->Write(cx, *JL_RVAL) );

	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION( _unserialize ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_TYPE( jl::JsvalIsUnserializer(cx, JL_ARG(1)), 1, "Unserializer" );

	jl::Unserializer *unser;
	unser = jl::JsvalToUnserializer(cx, JL_ARG(1));

	JL_CHK( unser->Read(cx, *JL_RVAL) );
	JL_CHK( JS_SetPropertyById(cx, obj, JLID(cx, fileName), JL_RVAL) );
	JL_CHK( unser->Read(cx, *JL_RVAL) );
	JL_CHK( JS_SetPropertyById(cx, obj, JLID(cx, lineNumber), JL_RVAL) );
	JL_CHK( unser->Read(cx, *JL_RVAL) );
	JL_CHK( JL_SetReservedSlot(cx, JL_OBJ, 0, *JL_RVAL) );

	return JS_TRUE;
	JL_BAD;
}
*/

CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision: 3524 $"))
	HAS_RESERVED_SLOTS(1)
	
	IS_UNCONSTRUCTIBLE

	BEGIN_PROPERTY_SPEC
		PROPERTY_GETTER( code )
		PROPERTY_GETTER( text )
		PROPERTY_GETTER( const )
	END_PROPERTY_SPEC

	BEGIN_FUNCTION_SPEC
		FUNCTION(toString)
//		FUNCTION_ARGC(_serialize, 1)
//		FUNCTION_ARGC(_unserialize, 1)
	END_FUNCTION_SPEC

END_CLASS


NEVER_INLINE JSBool FASTCALL
ThrowOalError( JSContext *cx, ALenum err ) {

	JSObject *error = JL_NewObjectWithGivenProto( cx, JL_CLASS(OalError), JL_CLASS_PROTOTYPE(cx, OalError), NULL );
	JS_SetPendingException( cx, OBJECT_TO_JSVAL( error ) );
	jsval errVal;
	JL_CHK( JL_NativeToJsval(cx, err, &errVal) );
	JL_CHK( JL_SetReservedSlot( cx, error, 0, errVal ) );
	JL_SAFE( JL_ExceptionSetScriptLocation(cx, error) );
	JL_BAD;
}
