/* ***** BEGIN LICENSE BLOCK *****
 * Version: GNU GPL 2.0
 *
 * The contents of this file are subject to the
 * GNU Genergl Public License Version 2.0; you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at
 * http://www.gnu.org/licenses/gpl.html
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 * ***** END LICENSE BLOCK ***** */

#include "stdafx.h"

#ifdef _MACOSX // MacosX platform specific
	#include <AGL/agl.h>
	#include <OpenGL/gl.h>
#endif

//#define GL_GLEXT_PROTOTYPES

#include <gl/gl.h>
#include "glext.h"


#include "oglError.h"

/**doc fileIndex:bottom **/

/**doc
**/

BEGIN_CLASS( OglError )


DEFINE_PROPERTY( code ) {

	return JL_GetReservedSlot( cx, obj, 0, vp );
}


DEFINE_PROPERTY( text ) {

	JL_CHK( JL_GetReservedSlot( cx, obj, 0, vp ) );
	if ( JSVAL_IS_VOID(*vp) )
		return JS_TRUE;
	int errorCode;
	JL_CHK( JsvalToInt(cx, *vp, &errorCode) );
	char *errStr;
	switch (errorCode) {
		case GL_NO_ERROR:
			errStr = "No Error.";
			break;
		case GL_INVALID_ENUM:
			errStr = ".";
			break;
		case GL_INVALID_VALUE:
			errStr = ".";
			break;
		case GL_INVALID_OPERATION:
			errStr = ".";
			break;
		case GL_STACK_OVERFLOW:
			errStr = ".";
			break;
		case GL_STACK_UNDERFLOW:
			errStr = ".";
			break;
		case GL_OUT_OF_MEMORY:
			errStr = ".";
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


DEFINE_PROPERTY( const ) {

	JL_CHK( JL_GetReservedSlot( cx, obj, 0, vp ) );
	int errorCode;
	JL_CHK( JsvalToInt(cx, *vp, &errorCode) );
	char *errStr;
	switch (errorCode) {
		case GL_NO_ERROR:
			errStr = "GL_NO_ERROR";
			break;
		case GL_INVALID_ENUM:
			errStr = "GL_INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			errStr = "GL_INVALID_VALUE";
			break;
		case GL_INVALID_OPERATION:
			errStr = "GL_INVALID_OPERATION";
			break;
		case GL_STACK_OVERFLOW:
			errStr = "GL_STACK_OVERFLOW";
			break;
		case GL_STACK_UNDERFLOW:
			errStr = "GL_STACK_UNDERFLOW";
			break;
		case GL_OUT_OF_MEMORY:
			errStr = "GL_OUT_OF_MEMORY";
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


DEFINE_FUNCTION( toString ) {

	return _text(cx, obj, 0, rval);
}


DEFINE_HAS_INSTANCE() { // see issue#52

	*bp = !JSVAL_IS_PRIMITIVE(v) && JL_GetClass(JSVAL_TO_OBJECT(v)) == JL_THIS_CLASS;
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
		JL_CHK( JS_SetReservedSlot(xdr->cx, *objp, 0, tmp) );
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


JSBool ThrowOglError( JSContext *cx, GLenum err ) {

	JSObject *error = JS_NewObject( cx, JL_CLASS(OglError), NULL, NULL );
	JS_SetPendingException( cx, OBJECT_TO_JSVAL( error ) );
	jsval errVal;
	JL_CHK( IntToJsval(cx, err, &errVal) );
	JL_CHK( JS_SetReservedSlot( cx, error, 0, errVal ) );
	JL_SAFE( ExceptionSetScriptLocation(cx, error) );
	JL_BAD;
}
