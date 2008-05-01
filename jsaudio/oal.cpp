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
//#include "jsobj.h"

#include "oal.h"

//#include "jstransformation.h"

#include "../jslang/bstring.h"

//TextureJSClass


//#define _USE_MATH_DEFINES
//#include "math.h"

#include <AL/al.h>

#define LOAD_OPENAL_EXTENSION( name, proto ) \
	static proto name = (proto) alGetProcAddress( #name ); \
	RT_ASSERT_1( name != NULL, "OpenAL extension %s unavailable.", #name );


BEGIN_CLASS( Oal )

/*
DEFINE_FUNCTION_FAST( GetBoolean ) {

	RT_ASSERT_ARGC(1);
	RT_ASSERT_INT(J_FARG(1));
	GLboolean params;
	glGetBooleanv(JSVAL_TO_INT(J_FARG(1)), &params);
	*J_FRVAL = BOOLEAN_TO_JSVAL(params);
	return JS_TRUE;
}
*/

/*
DEFINE_FUNCTION_FAST( MultiTexCoord ) {

	LOAD_OPENGL_EXTENSION( glMultiTexCoord1d, PFNGLMULTITEXCOORD1DARBPROC );
	LOAD_OPENGL_EXTENSION( glMultiTexCoord2d, PFNGLMULTITEXCOORD2DARBPROC );
	LOAD_OPENGL_EXTENSION( glMultiTexCoord3d, PFNGLMULTITEXCOORD3DARBPROC );

	RT_ASSERT_ARGC(2);

	RT_ASSERT_INT(J_FARG(1));
	GLenum target = JSVAL_TO_INT(J_FARG(1));
	
	*J_FRVAL = JSVAL_VOID;
	jsdouble s;
	JS_ValueToNumber(cx, J_FARG(2), &s);
	if ( J_ARGC == 1 ) {
	
		glMultiTexCoord1d(target, s);
		return JS_TRUE;	
	}
	jsdouble t;
	JS_ValueToNumber(cx, J_FARG(3), &t);
	if ( J_ARGC == 2 ) {

		glMultiTexCoord2d(target, s, t);
		return JS_TRUE;	
	}
	jsdouble r;
	JS_ValueToNumber(cx, J_FARG(4), &r);
	if ( J_ARGC == 3 ) {

		glMultiTexCoord3d(target, s, t, r);
		return JS_TRUE;	
	}
	REPORT_ERROR("Invalid argument.");
	return JS_TRUE;
}
*/


DEFINE_PROPERTY(error) {

	*vp = INT_TO_JSVAL(alGetError());
	return JS_TRUE;
}


JSBool Init( JSContext *cx, JSObject *obj ) {

	return JS_TRUE;
}


CONFIGURE_CLASS

	HAS_INIT

	BEGIN_CONST_INTEGER_SPEC
//		CONST_INTEGER( ACCUM  , GL_ACCUM  )
//		CONST_INTEGER( LOAD   , GL_LOAD   )
	END_CONST_INTEGER_SPEC

	BEGIN_STATIC_FUNCTION_SPEC
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_READ(error)
	END_STATIC_PROPERTY_SPEC

END_CLASS
