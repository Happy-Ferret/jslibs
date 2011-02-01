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
#include "../common/jsvalserializer.h"

#include "error.h"


/**doc fileIndex:bottom **/

/**doc
----
== jssdl::SdlError class ==
 You cannot construct this class.$LF
 Its aim is to catch jssdl runtime error exception.
**/

BEGIN_CLASS( SdlError )


/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Is the text of the error.
**/
DEFINE_PROPERTY( text ) {

	return JL_GetReservedSlot( cx, obj, 0, vp );
}

DEFINE_FUNCTION( toString ) {

	JL_DEFINE_FUNCTION_OBJ;
	return _text(cx, obj, JSID_EMPTY, JL_RVAL);
}

DEFINE_HAS_INSTANCE() { // see issue#52

	*bp = !JSVAL_IS_PRIMITIVE(*v) && JL_InheritFrom(cx, JSVAL_TO_OBJECT(*v), JL_THIS_CLASS);
	return JS_TRUE;
}

DEFINE_FUNCTION( _serialize ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_ARG(1);
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
	JL_S_ASSERT_ARG(1);
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
ThrowSdlError( JSContext *cx ) {

	JSObject *errorObj = JS_NewObjectWithGivenProto( cx, JL_CLASS(SdlError), JL_PROTOTYPE(cx, SdlError), NULL );
	JS_SetPendingException( cx, OBJECT_TO_JSVAL( errorObj ) );
	const char *errorMessage = SDL_GetError();
	SDL_ClearError();
//	if ( errorMessage == NULL || *errorMessage == '\0' )
//		errorMessage = "Undefined error";
	JL_CHK( JL_SetReservedSlot( cx, errorObj, 0, errorMessage != NULL && *errorMessage != '\0' ? STRING_TO_JSVAL(JS_NewStringCopyZ( cx, errorMessage )) : JSVAL_VOID ) );
	JL_SAFE( JL_ExceptionSetScriptLocation(cx, errorObj) );
	JL_BAD;
}
