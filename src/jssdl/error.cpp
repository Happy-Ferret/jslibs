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
----
== jssdl::SdlError class ==
 You cannot construct this class.
 Its aim is to catch jssdl runtime error exception.
**/

BEGIN_CLASS( SdlError )


/**doc
 * $INAME $READONLY
  Is the text of the error.
**/
DEFINE_PROPERTY( text ) {

	JS_GetReservedSlot( cx, obj, 0, vp );
	return JS_TRUE;
}

DEFINE_FUNCTION( toString ) {

	J_CHK( _text(cx, obj, 0, rval) );
	return JS_TRUE;
}

DEFINE_HAS_INSTANCE() { // see issue#52

	*bp = !JSVAL_IS_PRIMITIVE(v) && OBJ_GET_CLASS(cx, JSVAL_TO_OBJECT(v)) == _class;
	return JS_TRUE;
}


CONFIGURE_CLASS

	HAS_HAS_INSTANCE // see issue#52

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ( text )
	END_PROPERTY_SPEC

	BEGIN_FUNCTION_SPEC
		FUNCTION(toString)
	END_FUNCTION_SPEC

	HAS_RESERVED_SLOTS(1)

END_CLASS


JSBool ThrowSdlError( JSContext *cx ) {

	JSObject *errorObj = JS_NewObject( cx, _class, NULL, NULL );
	JS_SetPendingException( cx, OBJECT_TO_JSVAL( errorObj ) );
	const char *errorMessage = SDL_GetError();
	SDL_ClearError();
//	if ( errorMessage == NULL || *errorMessage == '\0' )
//		errorMessage = "Undefined error";
	JS_SetReservedSlot( cx, errorObj, 0, errorMessage != NULL && *errorMessage != '\0' ? STRING_TO_JSVAL(JS_NewStringCopyZ( cx, errorMessage )) : JSVAL_VOID );
  return JS_FALSE;
}
