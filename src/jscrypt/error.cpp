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
#include <jsvalserializer.h>


/**doc fileIndex:bottom
$CLASS_HEADER
$SVN_REVISION $Revision: 3524 $
**/
BEGIN_CLASS( CryptError )

DEFINE_PROPERTY_GETTER( code ) {

	JL_IGNORE(id);

	return JL_GetReservedSlot(  obj, 0, vp );
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

	JL_DEFINE_PROP_ARGS;

	JL_CHK( JL_GetReservedSlot(  obj, 0, vp ) );
	if ( vp.isUndefined() )
		return true;
	int errorCode;
	errorCode = vp.toInt32();
	JSString *str;
	str = JS_NewStringCopyZ( cx, ConstString(errorCode) );
	JL_RVAL.setString(str);
	return true;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( text ) {

	JL_DEFINE_PROP_ARGS;

	JL_CHK( JL_GetReservedSlot(JL_OBJ, 0, JL_RVAL) );
	if ( JL_RVAL.isUndefined() )
		return true;
	int errorCode;
	errorCode = JL_RVAL.toInt32();
	JSString *str;
	str = JS_NewStringCopyZ(cx, error_to_string(errorCode));
	JL_RVAL.setString(str);
	return true;
	JL_BAD;
}


DEFINE_FUNCTION( toString ) {

	JL_DEFINE_ARGS;

	JL_CHK( JL_GetReservedSlot(JL_OBJ, 0, JL_RVAL) );
	if ( JL_RVAL.isUndefined() )
		return true;
	int errorCode;
	errorCode = JL_RVAL.toInt32();
	JSString *str;
	str = JS_NewStringCopyZ(cx, error_to_string(errorCode));
	JL_RVAL.setString(str);
	return true;
	JL_BAD;
}


DEFINE_FUNCTION( _serialize ) {

	JL_DEFINE_ARGS;

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_TYPE( jl::JsvalIsSerializer(cx, JL_ARG(1)), 1, "Serializer" );

	jl::Serializer *ser;
	ser = jl::JsvalToSerializer(cx, JL_ARG(1));

	JL_CHK( JS_GetPropertyById(cx, JL_OBJ, JLID(cx, fileName), JL_RVAL) );
	JL_CHK( ser->Write(cx, JL_RVAL) );
	JL_CHK( JS_GetPropertyById(cx, JL_OBJ, JLID(cx, lineNumber), JL_RVAL) );
	JL_CHK( ser->Write(cx, JL_RVAL) );
	JL_CHK( JL_GetReservedSlot( JL_OBJ, 0, JL_RVAL) );
	JL_CHK( ser->Write(cx, JL_RVAL) );

	return true;
	JL_BAD;
}


DEFINE_FUNCTION( _unserialize ) {

	JL_DEFINE_ARGS;

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_TYPE( jl::JsvalIsUnserializer(cx, JL_ARG(1)), 1, "Unserializer" );

	jl::Unserializer *unser;
	unser = jl::JsvalToUnserializer(cx, JL_ARG(1));

	JL_CHK( unser->Read(cx, JL_RVAL) );
	JL_CHK( JS_SetPropertyById(cx, JL_OBJ, JLID(cx, fileName), JL_RVAL) );
	JL_CHK( unser->Read(cx, JL_RVAL) );
	JL_CHK( JS_SetPropertyById(cx, JL_OBJ, JLID(cx, lineNumber), JL_RVAL) );
	JL_CHK( unser->Read(cx, JL_RVAL) );
	JL_CHK( JL_SetReservedSlot( JL_OBJ, 0, JL_RVAL) );

	return true;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision: 3524 $"))

	HAS_RESERVED_SLOTS(1)
	IS_UNCONSTRUCTIBLE

	BEGIN_PROPERTY_SPEC
		PROPERTY_GETTER( code )
		PROPERTY_GETTER( const )
		PROPERTY_GETTER( text )
	END_PROPERTY_SPEC

	BEGIN_FUNCTION_SPEC
		FUNCTION(toString)
		FUNCTION_ARGC(_serialize, 1)
		FUNCTION_ARGC(_unserialize, 1)
	END_FUNCTION_SPEC

END_CLASS



NEVER_INLINE bool FASTCALL
ThrowCryptError( JSContext *cx, int errorCode ) {

//	JS_ReportWarning( cx, "CryptError exception" );
	JS::RootedObject error(cx, jl::newObjectWithGivenProto( cx, JL_CLASS(CryptError), JL_CLASS_PROTOTYPE(cx, CryptError)));
	JS::RootedValue errorVal(cx);
	errorVal.setObject(*error);
	JL_CHK( jl::setSlot(cx, error, 0, errorCode) );
	JL_SAFE( jl::setScriptLocation(cx, &error) );

	JS_SetPendingException(cx, errorVal);

	//	JL_SetReservedSlot(  error, 1, errorMessage != NULL ? STRING_TO_JSVAL(JS_NewStringCopyZ( cx, errorMessage )) : JSVAL_VOID );
	JL_BAD;
}
