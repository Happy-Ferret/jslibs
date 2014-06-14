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
DEFINE_PROPERTY_GETTER( text ) {

	JL_IGNORE(id);
	return JL_GetReservedSlot(  obj, 0, vp );
}

DEFINE_FUNCTION( toString ) {

	JL_IGNORE(argc);
		return _textGetter(cx, obj, JSID_EMPTY, JL_RVAL);
}


DEFINE_FUNCTION( _serialize ) {

		JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_TYPE( jl::JsvalIsSerializer(cx, JL_ARG(1)), 1, "Serializer" );

	jl::Serializer *ser;
	ser = jl::JsvalToSerializer(cx, JL_ARG(1));

	JL_CHK( JS_GetPropertyById(cx, JL_OBJ, JLID(cx, fileName), JL_RVAL) );
	JL_CHK( ser->Write(cx, *JL_RVAL) );
	JL_CHK( JS_GetPropertyById(cx, JL_OBJ, JLID(cx, lineNumber), JL_RVAL) );
	JL_CHK( ser->Write(cx, *JL_RVAL) );
	JL_CHK( JL_GetReservedSlot( JL_OBJ, 0, JL_RVAL) );
	JL_CHK( ser->Write(cx, *JL_RVAL) );

	return true;
	JL_BAD;
}


DEFINE_FUNCTION( _unserialize ) {

		JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_TYPE( jl::JsvalIsUnserializer(cx, JL_ARG(1)), 1, "Unserializer" );

	jl::Unserializer *unser;
	unser = jl::JsvalToUnserializer(cx, JL_ARG(1));

	JL_CHK( unser->Read(cx, *JL_RVAL) );
	JL_CHK( JS_SetPropertyById(cx, obj, JLID(cx, fileName), JL_RVAL) );
	JL_CHK( unser->Read(cx, *JL_RVAL) );
	JL_CHK( JS_SetPropertyById(cx, obj, JLID(cx, lineNumber), JL_RVAL) );
	JL_CHK( unser->Read(cx, *JL_RVAL) );
	JL_CHK( JL_SetReservedSlot( JL_OBJ, 0, *JL_RVAL) );

	return true;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision: 3524 $"))
	HAS_RESERVED_SLOTS(1)
	IS_UNCONSTRUCTIBLE

	BEGIN_PROPERTY_SPEC
		PROPERTY_GETTER( text )
	END_PROPERTY_SPEC

	BEGIN_FUNCTION_SPEC
		FUNCTION(toString)
		FUNCTION_ARGC(_serialize, 1)
		FUNCTION_ARGC(_unserialize, 1)
	END_FUNCTION_SPEC

END_CLASS


NEVER_INLINE bool FASTCALL
ThrowSdlError( JSContext *cx ) {

	JSObject *errorObj = jl::newObjectWithGivenProto(cx, JL_CLASS(SdlError), JL_CLASS_PROTOTYPE(cx, SdlError));
	JS_SetPendingException( cx, OBJECT_TO_JSVAL( errorObj ) );
	const char *errorMessage = SDL_GetError();
	SDL_ClearError();
//	if ( errorMessage == NULL || *errorMessage == '\0' )
//		errorMessage = "Undefined error";
	JL_CHK( JL_SetReservedSlot(  errorObj, 0, errorMessage != NULL && *errorMessage != '\0' ? STRING_TO_JSVAL(JS_NewStringCopyZ( cx, errorMessage )) : JSVAL_VOID ) );
	JL_SAFE( jl::addScriptLocation(cx, errorObj) );
	JL_BAD;
}
