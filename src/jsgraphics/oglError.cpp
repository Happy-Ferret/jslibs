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
#include <jsvalserializer.h>

#include <gl/gl.h>
#include "glext.h"

const char *
OpenGLErrorToConst(GLenum errorCode) {

	switch (errorCode) {
		case GL_NO_ERROR:
			return "GL_NO_ERROR";
		case GL_INVALID_ENUM:
			return "GL_INVALID_ENUM";
		case GL_INVALID_VALUE:
			return "GL_INVALID_VALUE";
		case GL_INVALID_OPERATION:
			return "GL_INVALID_OPERATION";
		case GL_STACK_OVERFLOW:
			return "GL_STACK_OVERFLOW";
		case GL_STACK_UNDERFLOW:
			return "GL_STACK_UNDERFLOW";
		case GL_OUT_OF_MEMORY:
			return "GL_OUT_OF_MEMORY";
// GL_EXT_histogram / GL_ARB_imaging_DEPRECATED
		case GL_TABLE_TOO_LARGE:
			return "GL_TABLE_TOO_LARGE";
#if defined(GL_ARB_framebuffer_object) || defined(GL_EXT_framebuffer_object)
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			return "GL_INVALID_FRAMEBUFFER_OPERATION";
#endif
	}
	return "???";
}


/**doc fileIndex:bottom **/

/**doc
**/


BEGIN_CLASS( OglError )


DEFINE_PROPERTY_GETTER( code ) {
	
	JL_IGNORE(id, cx);
	return JL_GetReservedSlot(  obj, 0, vp );
}


DEFINE_PROPERTY_GETTER( text ) {

	JL_IGNORE(id);
	JL_CHK( JL_GetReservedSlot(  obj, 0, vp ) );
	if ( vp.isUndefined() )
		return true;
	int errorCode;
	JL_CHK( jl::getValue(cx, vp, &errorCode) );
	char *errStr;
	switch (errorCode) {
		case GL_NO_ERROR:
			errStr = "no error";
			break;
		case GL_INVALID_ENUM:
			errStr = "invalid enumerant";
			break;
		case GL_INVALID_VALUE:
			errStr = "invalid value";
			break;
		case GL_INVALID_OPERATION:
			errStr = "invalid operation";
			break;
		case GL_STACK_OVERFLOW:
			errStr = "stack overflow";
			break;
		case GL_STACK_UNDERFLOW:
			errStr = "stack underflow";
			break;
		case GL_OUT_OF_MEMORY:
			errStr = "out of memory";
			break;
		case GL_TABLE_TOO_LARGE:
			errStr = "table too large";
			break;
		default:
			errStr = "unknown";
			break;
	}
	return JL_NativeToJsval(cx, errStr, vp);
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( const ) {

	JL_IGNORE(id);
	JL_CHK( JL_GetReservedSlot(  obj, 0, vp ) );
	int errorCode;
	JL_CHK( jl::getValue(cx, vp, &errorCode) );
	return JL_NativeToJsval(cx, OpenGLErrorToConst(errorCode), vp);
	JL_BAD;
}


DEFINE_FUNCTION( toString ) {

	JL_IGNORE(argc);

	JL_DEFINE_ARGS;

	JL_DEFINE_FUNCTION_OBJ;

	JS::RootedObject rtobj(cx, obj);
	JS::RootedId rtid(cx, JSID_EMPTY);
	JS::RootedValue hval(cx);

	return _textGetter(cx, rtobj, rtid, &hval);
}


DEFINE_FUNCTION( _serialize ) {

	JL_DEFINE_ARGS;

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_TYPE( jl::JsvalIsSerializer(cx, JL_ARG(1)), 1, "Serializer" );

	jl::Serializer *ser;
	ser = jl::JsvalToSerializer(cx, JL_ARG(1));

	JL_CHK( JS_GetPropertyById(cx, JL_OBJ, JLID(cx, fileName), JL_RVAL) );
	JL_CHK( ser->Write(cx, *JL_RVAL) );
	JL_CHK( JS_GetPropertyById(cx, JL_OBJ, JLID(cx, lineNumber), JL_RVAL) );
	JL_CHK( ser->Write(cx, *JL_RVAL) );
	JL_CHK( JL_GetReservedSlot( JL_OBJ, 0, *JL_RVAL) );
	JL_CHK( ser->Write(cx, *JL_RVAL) );

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
		PROPERTY_GETTER( code )
		PROPERTY_GETTER( text )
		PROPERTY_GETTER( const )
	END_PROPERTY_SPEC

	BEGIN_FUNCTION_SPEC
		FUNCTION(toString)
		FUNCTION_ARGC(_serialize, 1)
		FUNCTION_ARGC(_unserialize, 1)
	END_FUNCTION_SPEC

END_CLASS



NEVER_INLINE bool FASTCALL
ThrowOglError( JSContext *cx, GLenum err ) {

	JS::RootedObject error(cx, jl::newObjectWithGivenProto( cx, JL_CLASS(OglError), JL_CLASS_PROTOTYPE(cx, OglError)));
	JS_SetPendingException( cx, OBJECT_TO_JSVAL( error ) );
	JS::RootedValue errVal(cx);
	JL_CHK( JL_NativeToJsval(cx, err, errVal) );
	JL_CHK( JL_SetReservedSlot(  error, 0, errVal ) );
	JL_SAFE( jl::setScriptLocation(cx, error) );
	JL_BAD;
}
