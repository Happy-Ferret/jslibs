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


DEFINE_PROPERTY_GETTER( code ) {

	JL_INGORE(id);

	return JL_GetReservedSlot( cx, obj, 0, vp );
}

DEFINE_PROPERTY_GETTER( text ) {

	JL_INGORE(id);

	return JL_GetReservedSlot( cx, obj, 1, vp );
}

DEFINE_PROPERTY_GETTER( const ) {

	JL_INGORE(id);

	JL_GetReservedSlot( cx, obj, 0, vp );
	if ( JSVAL_IS_VOID(*vp) )
		return JS_TRUE;
	int errorCode = JSVAL_TO_INT(*vp);
	JSString *str = JS_NewStringCopyZ( cx, ZConstString(errorCode) );
	*vp = STRING_TO_JSVAL( str );
	return JS_TRUE;
}


DEFINE_FUNCTION( toString ) {

	JL_INGORE(argc);

	JL_DEFINE_FUNCTION_OBJ;
	return _textGetter(cx, obj, JSID_EMPTY, JL_RVAL);
}

/*
DEFINE_HAS_INSTANCE() { // see issue#52

	JL_INGORE(obj);

	*bp = !JSVAL_IS_PRIMITIVE(*v) && JL_InheritFrom(cx, JSVAL_TO_OBJECT(*v), JL_THIS_CLASS);
	return JS_TRUE;
}
*/


DEFINE_FUNCTION( _serialize ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARG_COUNT(1);
	JL_ASSERT_ARG_TYPE( jl::JsvalIsSerializer(cx, JL_ARG(1)), 1, "Serializer" );

	jl::Serializer *ser;
	ser = jl::JsvalToSerializer(cx, JL_ARG(1));

	JL_CHK( JS_GetPropertyById(cx, JL_OBJ, JL_ATOMJSID(cx, fileName), JL_RVAL) );
	JL_CHK( ser->Write(cx, *JL_RVAL) );
	JL_CHK( JS_GetPropertyById(cx, JL_OBJ, JL_ATOMJSID(cx, lineNumber), JL_RVAL) );
	JL_CHK( ser->Write(cx, *JL_RVAL) );
	JL_CHK( JL_GetReservedSlot(cx, JL_OBJ, 0, JL_RVAL) );
	JL_CHK( ser->Write(cx, *JL_RVAL) );
	JL_CHK( JL_GetReservedSlot(cx, JL_OBJ, 1, JL_RVAL) );
	JL_CHK( ser->Write(cx, *JL_RVAL) );

	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION( _unserialize ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARG_COUNT(1);
	JL_ASSERT_ARG_TYPE( jl::JsvalIsUnserializer(cx, JL_ARG(1)), 1, "Unserializer" );

	jl::Unserializer *unser;
	unser = jl::JsvalToUnserializer(cx, JL_ARG(1));

	JL_CHK( unser->Read(cx, *JL_RVAL) );
	JL_CHK( JS_SetPropertyById(cx, obj, JL_ATOMJSID(cx, fileName), JL_RVAL) );
	JL_CHK( unser->Read(cx, *JL_RVAL) );
	JL_CHK( JS_SetPropertyById(cx, obj, JL_ATOMJSID(cx, lineNumber), JL_RVAL) );
	JL_CHK( unser->Read(cx, *JL_RVAL) );
	JL_CHK( JL_SetReservedSlot(cx, JL_OBJ, 0, *JL_RVAL) );
	JL_CHK( unser->Read(cx, *JL_RVAL) );
	JL_CHK( JL_SetReservedSlot(cx, JL_OBJ, 1, *JL_RVAL) );

	return JS_TRUE;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))

	//HAS_HAS_INSTANCE // see issue#52
	IS_INCONSTRUCTIBLE

	BEGIN_PROPERTY_SPEC
		PROPERTY_GETTER( code )
		PROPERTY_GETTER( text )
		PROPERTY_GETTER( const )
	END_PROPERTY_SPEC

	BEGIN_FUNCTION_SPEC
		FUNCTION(toString)
		FUNCTION_ARGC(_serialize, 1)
		FUNCTION_ARGC(_unserialize, 1)
	END_FUNCTION_SPEC

	HAS_RESERVED_SLOTS(2)

END_CLASS


NEVER_INLINE JSBool FASTCALL
ThrowZError( JSContext *cx, int errorCode, const char *errorMessage ) {

	JSObject *error = JS_NewObjectWithGivenProto( cx, JL_CLASS(ZError), JL_PROTOTYPE(cx, ZError), NULL );
	JS_SetPendingException( cx, OBJECT_TO_JSVAL( error ) );
	JL_CHK( JL_SetReservedSlot( cx, error, 0, INT_TO_JSVAL(errorCode) ) );
	JL_CHK( JL_SetReservedSlot( cx, error, 1, STRING_TO_JSVAL(JS_NewStringCopyZ( cx, errorMessage != NULL ? errorMessage : ZConstString(errorCode) )) ) );
	JL_SAFE( JL_ExceptionSetScriptLocation(cx, error) );
	return JS_FALSE;
	JL_BAD;
}
