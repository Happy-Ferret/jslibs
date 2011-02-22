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


/**doc fileIndex:bottom
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( CryptError )

DEFINE_PROPERTY_GETTER( code ) {

	JL_USE(id);

	return JL_GetReservedSlot( cx, obj, 0, vp );
}



const char *ConstString( int errorCode ) {

	switch (errorCode) {

		case CRYPT_OK: return "CRYPT_OK";
		case CRYPT_ERROR: return "CRYPT_ERROR";
		case CRYPT_NOP: return "CRYPT_NOP";
		case CRYPT_INVALID_KEYSIZE: return "CRYPT_INVALID_KEYSIZE";
		case CRYPT_INVALID_ROUNDS: return "CRYPT_INVALID_ROUNDS";
		case CRYPT_FAIL_TESTVECTOR: return "CRYPT_FAIL_TESTVECTOR";
		case CRYPT_BUFFER_OVERFLOW: return "CRYPT_BUFFER_OVERFLOW";
		case CRYPT_INVALID_PACKET: return "CRYPT_INVALID_PACKET";
		case CRYPT_INVALID_PRNGSIZE: return "CRYPT_INVALID_PRNGSIZE";
		case CRYPT_ERROR_READPRNG: return "CRYPT_ERROR_READPRNG";
		case CRYPT_INVALID_CIPHER: return "CRYPT_INVALID_CIPHER";
		case CRYPT_INVALID_HASH: return "CRYPT_INVALID_HASH";
		case CRYPT_INVALID_PRNG: return "CRYPT_INVALID_PRNG";
		case CRYPT_MEM: return "CRYPT_MEM";
		case CRYPT_PK_TYPE_MISMATCH: return "CRYPT_PK_TYPE_MISMATCH";
		case CRYPT_PK_NOT_PRIVATE: return "CRYPT_PK_NOT_PRIVATE";
		case CRYPT_INVALID_ARG: return "CRYPT_INVALID_ARG";
		case CRYPT_FILE_NOTFOUND: return "CRYPT_FILE_NOTFOUND";
		case CRYPT_PK_INVALID_TYPE: return "CRYPT_PK_INVALID_TYPE";
		case CRYPT_PK_INVALID_SYSTEM: return "CRYPT_PK_INVALID_SYSTEM";
		case CRYPT_PK_DUP: return "CRYPT_PK_DUP";
		case CRYPT_PK_NOT_FOUND: return "CRYPT_PK_NOT_FOUND";
		case CRYPT_PK_INVALID_SIZE: return "CRYPT_PK_INVALID_SIZE";
		case CRYPT_INVALID_PRIME_SIZE: return "CRYPT_INVALID_PRIME_SIZE";
		case CRYPT_PK_INVALID_PADDING : return "CRYPT_PK_INVALID_PADDING ";
	}
	return "UNKNOWN_ERROR";
}


DEFINE_PROPERTY_GETTER( const ) {

	JL_USE(id);

	JL_CHK( JL_GetReservedSlot( cx, obj, 0, vp ) );
	if ( JSVAL_IS_VOID(*vp) )
		return JS_TRUE;
	int errorCode;
	errorCode = JSVAL_TO_INT(*vp);
	JSString *str;
	str = JS_NewStringCopyZ( cx, ConstString(errorCode) );
	*vp = STRING_TO_JSVAL( str );
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( text ) {

	JL_USE(id);

	JL_CHK( JL_GetReservedSlot( cx, obj, 0, vp ) );
	if ( JSVAL_IS_VOID(*vp) )
		return JS_TRUE;
	int errorCode;
	errorCode = JSVAL_TO_INT(*vp);
	JSString *str;
	str = JS_NewStringCopyZ( cx, error_to_string(errorCode) );
	*vp = STRING_TO_JSVAL( str );
	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION( toString ) {

	JL_USE(argc);

	JL_DEFINE_FUNCTION_OBJ;
	return _textGetter(cx, obj, JSID_EMPTY, JL_RVAL);
}

DEFINE_HAS_INSTANCE() { // see issue#52

	JL_USE(obj);

	*bp = !JSVAL_IS_PRIMITIVE(*v) && JL_InheritFrom(cx, JSVAL_TO_OBJECT(*v), JL_THIS_CLASS);
	return JS_TRUE;
}




DEFINE_FUNCTION( _serialize ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_ARG_COUNT(1);
	JL_S_ASSERT( jl::JsvalIsSerializer(cx, JL_ARG(1)), "Invalid serializer object." );
	jl::Serializer *ser;
	ser = jl::JsvalToSerializer(cx, JL_ARG(1));

	JL_CHK( JS_GetPropertyById(cx, JL_OBJ, JL_ATOMJSID(cx, fileName), JL_RVAL) );
	JL_CHK( ser->Write(cx, *JL_RVAL) );
	JL_CHK( JS_GetPropertyById(cx, JL_OBJ, JL_ATOMJSID(cx, lineNumber), JL_RVAL) );
	JL_CHK( ser->Write(cx, *JL_RVAL) );
	JL_CHK( JL_GetReservedSlot(cx, JL_OBJ, 0, JL_RVAL) );
	JL_CHK( ser->Write(cx, *JL_RVAL) );

	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION( _unserialize ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_ARG_COUNT(1);
	JL_S_ASSERT( jl::JsvalIsUnserializer(cx, JL_ARG(1)), "Invalid unserializer object." );
	jl::Unserializer *unser;
	unser = jl::JsvalToUnserializer(cx, JL_ARG(1));

	JL_CHK( unser->Read(cx, *JL_RVAL) );
	JL_CHK( JS_SetPropertyById(cx, obj, JL_ATOMJSID(cx, fileName), JL_RVAL) );
	JL_CHK( unser->Read(cx, *JL_RVAL) );
	JL_CHK( JS_SetPropertyById(cx, obj, JL_ATOMJSID(cx, lineNumber), JL_RVAL) );
	JL_CHK( unser->Read(cx, *JL_RVAL) );
	JL_CHK( JL_SetReservedSlot(cx, JL_OBJ, 0, *JL_RVAL) );

	return JS_TRUE;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))

	HAS_HAS_INSTANCE // see issue#52

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ( code )
		PROPERTY_READ( const )
		PROPERTY_READ( text )
	END_PROPERTY_SPEC

	BEGIN_FUNCTION_SPEC
		FUNCTION(toString)
		FUNCTION_ARGC(_serialize, 1)
		FUNCTION_ARGC(_unserialize, 1)
	END_FUNCTION_SPEC

	HAS_RESERVED_SLOTS(1)

END_CLASS



NEVER_INLINE JSBool FASTCALL
ThrowCryptError( JSContext *cx, int errorCode ) {

//	JS_ReportWarning( cx, "CryptError exception" );
	JSObject *error = JS_NewObjectWithGivenProto( cx, JL_CLASS(CryptError), JL_PROTOTYPE(cx, CryptError), NULL );
	JS_SetPendingException( cx, OBJECT_TO_JSVAL( error ) );
	JL_CHK( JL_SetReservedSlot( cx, error, 0, INT_TO_JSVAL(errorCode) ) );
//	JL_SetReservedSlot( cx, error, 1, errorMessage != NULL ? STRING_TO_JSVAL(JS_NewStringCopyZ( cx, errorMessage )) : JSVAL_VOID );
	JL_SAFE( JL_ExceptionSetScriptLocation(cx, error) );
	JL_BAD;
}
