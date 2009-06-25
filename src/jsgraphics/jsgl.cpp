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

/*
Manage GL extensions:
	http://www.libsdl.org/cgi/viewvc.cgi/trunk/SDL/src/video/win32/SDL_win32opengl.c?view=markup&sortby=date

*/

#include "stdafx.h"
//#include "jsobj.h"

DECLARE_CLASS(Ogl)

//#include "../common/jsNativeInterface.h"

//#include "jstransformation.h"

#include "../jsimage/image.h"
#include "../jslang/blob.h"
#include "../jsprotex/texture.h"
//TextureJSClass

#include "../jslang/idPub.h"

#include "../jstrimesh/trimeshPub.h"

#define _USE_MATH_DEFINES
#include "math.h"

#include "../common/matrix44.h"
#include "../common/vector3.h"

/* doc.
  OpenGL matrices are 16-value arrays with base vectors laid out contiguously in memory. 
  The translation components occupy the 13th, 14th, and 15th elements of the 16-element matrix, 
  where indices are numbered from 1 to 16 as described in section 2.11.2 of the OpenGL 2.1 Specification.
*/
#ifdef _MACOSX // MacosX platform specific
	#include <AGL/agl.h>
	#include <OpenGL/gl.h>
#endif

//#define GL_GLEXT_PROTOTYPES

#include <gl/gl.h>
#include "glext.h"

#include <gl/glu.h>

#include "oglError.h"

// http://www.opengl.org/registry/api/glext.h

#ifdef XP_WIN
#define GL_GET_PROC_ADDRESS wglGetProcAddress
#endif // XP_WIN

/*
#define LOAD_OPENGL_EXTENSION( name, proto ) \
	static proto name = NULL; \
	if ( name == NULL ) { \
		name = (proto) GL_GET_PROC_ADDRESS( #name ); \
		JL_S_ASSERT( name != NULL, "OpenGL extension %s unavailable.", #name ); \
	}
*/

 // (TBD) check static keyword issue
#define LOAD_OPENGL_EXTENSION( name, proto ) \
	static proto name = (proto) GL_GET_PROC_ADDRESS( #name ); \
	JL_S_ASSERT( name != NULL, "OpenGL extension %s unavailable.", #name );



/**doc fileIndex:top
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( Ogl )

/**doc
=== Static functions ===
**/


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME( pname )
  $H arguments
   $ARG GLenum pname
  $H return value
   value of a selected parameter.
  $H OpenGL API
   glGetBooleanv
**/
DEFINE_FUNCTION_FAST( GetBoolean ) {

	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT_INT(JL_FARG(1));
	GLboolean params;
	glGetBooleanv(JSVAL_TO_INT(JL_FARG(1)), &params);
	*JL_FRVAL = BOOLEAN_TO_JSVAL(params);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT | $ARRAY $INAME( pname [, count] )
  $H arguments
   $ARG GLenum pname
   $ARG $INT count: is the number of expected values. If _count_ is defined, the function will returns an array of values, else it returns a single value.
  $H return value
   A value or an array of values of a selected parameter.
  $H OpenGL API
   glGetIntegerv
**/
DEFINE_FUNCTION_FAST( GetInteger ) {

	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT_INT(JL_FARG(1));

	GLint params[16]; // (TBD) check if it is the max amount of data that glGetIntegerv may returns.
	glGetIntegerv(JSVAL_TO_INT( JL_FARG(1) ), params);

	if ( JL_FARG_ISDEF(2) ) {

		JL_S_ASSERT_INT( JL_FARG(2) );
		int count = JSVAL_TO_INT( JL_FARG(2) );
		JSObject *arrayObj = JS_NewArrayObject(cx, 0, NULL);
		JL_CHK(arrayObj);
		*JL_FRVAL = OBJECT_TO_JSVAL(arrayObj);
		jsval tmpValue;
		while (count--) {

			tmpValue = INT_TO_JSVAL( params[count] );
			JL_CHK( JS_SetElement(cx, arrayObj, count, &tmpValue) );
		}
	} else {

		*JL_FRVAL = INT_TO_JSVAL( params[0] );
	}
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $REAL | $ARRAY $INAME( pname [, count] )
  $H arguments
   $ARG GLenum pname
   $ARG $INT count: is the number of expected values. If _count_ is defined, the function will returns an array of values, else a single value.
  $H return value
   A single value or an Array of values of the selected parameter.
  $H OpenGL API
   glGetDoublev
**/
DEFINE_FUNCTION_FAST( GetDouble ) {

	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT_INT(JL_FARG(1));

	GLdouble params[16]; // (TBD) check if it is the max amount of data that glGetDoublev may returns.
	glGetDoublev(JSVAL_TO_INT(JL_FARG(1)), params);

	if ( JL_FARG_ISDEF(2) ) {

		JL_S_ASSERT_INT( JL_FARG(2) );
		int count = JSVAL_TO_INT( JL_FARG(2) );
		JSObject *arrayObj = JS_NewArrayObject(cx, 0, NULL);
		JL_CHK(arrayObj);
		*JL_FRVAL = OBJECT_TO_JSVAL(arrayObj);
		jsval tmpValue;
		while (count--) {

			JL_CHK( DoubleToJsval(cx, params[count], &tmpValue) );
			JL_CHK( JS_SetElement(cx, arrayObj, count, &tmpValue) );
		}
	} else {

		JL_CHK( DoubleToJsval(cx, params[0], JL_FRVAL) );
	}
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( op, value )
  $H arguments
   $ARG GLenum op
   $ARG $REAL value
  $H OpenGL API
   glAccum
**/
DEFINE_FUNCTION_FAST( Accum ) {

	JL_S_ASSERT_ARG_MIN(2);
	JL_S_ASSERT_INT(JL_FARG(1));
	GLenum op = JSVAL_TO_INT(JL_FARG(1));
	jsdouble value;
	JsvalToDouble(cx, JL_FARG(2), &value);
	glAccum(op, value);
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( func, ref )
  $H arguments
   $ARG GLenum op
   $ARG $REAL ref
  $H OpenGL API
   glAlphaFunc
**/
DEFINE_FUNCTION_FAST( AlphaFunc ) {

	JL_S_ASSERT_ARG_MIN(2);
	JL_S_ASSERT_INT(JL_FARG(1));
	JL_S_ASSERT_NUMBER(JL_FARG(2));
	jsdouble ref;
	JsvalToDouble(cx, JL_FARG(2), &ref);
	glAlphaFunc( JSVAL_TO_INT(JL_FARG(1)), ref );
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  $H OpenGL API
   glFlush
**/
DEFINE_FUNCTION_FAST( Flush ) {

	glFlush();
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  $H OpenGL API
   glFinish
**/
DEFINE_FUNCTION_FAST( Finish ) {

	glFinish();
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( pname, params )
  $H arguments
   $ARG GLenum pname
   $ARG $VAL params: is either a number or an array of numbers.
  $H OpenGL API
   glFogi, glFogf, glFogfv
**/
DEFINE_FUNCTION_FAST( Fog ) {

	JL_S_ASSERT_ARG_MIN(2);
	JL_S_ASSERT_INT(JL_FARG(1));

	*JL_FRVAL = JSVAL_VOID;
	if ( JSVAL_IS_INT(JL_FARG(2)) ) {

		glFogi(JSVAL_TO_INT(JL_FARG(1)), JSVAL_TO_INT(JL_FARG(2)));
		return JS_TRUE;
	}
	if ( JSVAL_IS_NUMBER(JL_FARG(2)) ) {

		jsdouble param;
		JsvalToDouble(cx, JL_FARG(2), &param);
		glFogf( JSVAL_TO_INT(JL_FARG(1)), param );
		return JS_TRUE;
	}
	if ( JsvalIsArray(cx, JL_FARG(2)) ) {

		GLfloat params[16];
		size_t length;
		JL_CHK( JsvalToFloatVector(cx, JL_FARG(2), params, COUNTOF(params), &length ) );
		glFogfv( JSVAL_TO_INT(JL_FARG(1)), params );
		return JS_TRUE;
	}
	JL_REPORT_ERROR("Invalid argument.");
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( target, mode )
  $H arguments
   $ARG GLenum target
   $ARG GLenum mode
  $H OpenGL API
   glHint
**/
DEFINE_FUNCTION_FAST( Hint ) {

	JL_S_ASSERT_ARG_MIN(2);
	JL_S_ASSERT_INT(JL_FARG(1));
	JL_S_ASSERT_INT(JL_FARG(2));
	glHint( JSVAL_TO_INT(JL_FARG(1)), JSVAL_TO_INT(JL_FARG(2)) );
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( x, y [, z] )
  $H arguments
   $ARG $REAL x
   $ARG $REAL y
   $ARG $REAL z
  $H OpenGL API
   glVertex3d, glVertex2d
**/
DEFINE_FUNCTION_FAST( Vertex ) {

	JL_S_ASSERT_ARG_MIN(2);

//	if ( JsvalIsArray(cx, JL_FARG(1)) ) {
//	}

//	float vec[3];
//	FloatArrayToVector(cx, 3, &argv[0], vec);
	jsdouble x, y, z;
	JsvalToDouble(cx, JL_FARG(1), &x);
	JsvalToDouble(cx, JL_FARG(2), &y);
	if ( JL_ARGC >= 3 ) {

		JsvalToDouble(cx, JL_FARG(3), &z);
		glVertex3d(x, y, z);
	} else {

		glVertex2d(x, y);
	}
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( red, green, blue [, alpha] )
  $H arguments
   $ARG $REAL red
   $ARG $REAL green
   $ARG $REAL blue
   $ARG $REAL alpha
  $H OpenGL API
   glColor4d, glColor3d
**/
DEFINE_FUNCTION_FAST( Color ) {

	JL_S_ASSERT_ARG_MIN(3);
//	float vec[3];
//	FloatArrayToVector(cx, 3, &argv[0], vec);
	jsdouble r, g, b, a;
	JsvalToDouble(cx, JL_FARG(1), &r);
	JsvalToDouble(cx, JL_FARG(2), &g);
	JsvalToDouble(cx, JL_FARG(3), &b);
	if ( JL_FARG_ISDEF(4) ) {

		JsvalToDouble(cx, JL_FARG(4), &a);
		glColor4d(r, g, b, a);
	} else {

		glColor3d(r, g, b);
	}
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( nx, ny, nz )
  $H arguments
   $ARG $REAL nx
   $ARG $REAL ny
   $ARG $REAL nz
  $H OpenGL API
   glNormal3d
**/
DEFINE_FUNCTION_FAST( Normal ) {

	JL_S_ASSERT_ARG_MIN(3);
//	float vec[3];
//	FloatArrayToVector(cx, 3, &argv[0], vec);
	jsdouble nx, ny, nz;
	JsvalToDouble(cx, JL_FARG(1), &nx);
	JsvalToDouble(cx, JL_FARG(2), &ny);
	JsvalToDouble(cx, JL_FARG(3), &nz);
	glNormal3d(nx, ny, nz);
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( s [, t [, r]] )
  $H arguments
   $ARG $REAL s
   $ARG $REAL t
   $ARG $REAL r
  $H OpenGL API
   glTexCoord1d, glTexCoord2d, glTexCoord3d
**/
DEFINE_FUNCTION_FAST( TexCoord ) {

	JL_S_ASSERT_ARG_MIN(1);
	*JL_FRVAL = JSVAL_VOID;
	jsdouble s;
	JsvalToDouble(cx, JL_FARG(1), &s);
	if ( JL_ARGC == 1 ) {

		glTexCoord1d(s);
		return JS_TRUE;
	}
	jsdouble t;
	JsvalToDouble(cx, JL_FARG(2), &t);
	if ( JL_ARGC == 2 ) {

		glTexCoord2d(s, t);
		return JS_TRUE;
	}
	jsdouble r;
	JsvalToDouble(cx, JL_FARG(3), &r);
	if ( JL_ARGC == 3 ) {

		glTexCoord3d(s, t, r);
		return JS_TRUE;
	}
	JL_REPORT_ERROR("Invalid argument.");
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( target, pname, params )
  $H arguments
   $ARG GLenum target
   $ARG GLenum pname
   $ARG $VAL params: is either a number or an array of numbers.
  $H OpenGL API
   glTexParameteri, glTexParameterf, glTexParameterfv
**/
DEFINE_FUNCTION_FAST( TexParameter ) {

	JL_S_ASSERT_ARG_MIN(3);
	JL_S_ASSERT_INT(JL_FARG(1));
	JL_S_ASSERT_INT(JL_FARG(2));

	*JL_FRVAL = JSVAL_VOID;
	if ( JSVAL_IS_INT(JL_FARG(3)) ) {

		glTexParameteri( JSVAL_TO_INT( JL_FARG(1) ), JSVAL_TO_INT( JL_FARG(2) ), JSVAL_TO_INT( JL_FARG(3) ) );
		return JS_TRUE;
	}
	if ( JSVAL_IS_DOUBLE(JL_FARG(3)) ) {

		jsdouble param;
		JL_CHK( JsvalToDouble(cx, JL_FARG(3), &param) );
		glTexParameterf( JSVAL_TO_INT( JL_FARG(1) ), JSVAL_TO_INT( JL_FARG(2) ), param );
		return JS_TRUE;
	}
	if ( JsvalIsArray(cx, JL_FARG(3)) ) {

		GLfloat params[16];
		size_t length;
		JL_CHK( JsvalToFloatVector(cx, JL_FARG(3), params, COUNTOF(params), &length ) );
		glTexParameterfv( JSVAL_TO_INT(JL_FARG(1)), JSVAL_TO_INT(JL_FARG(2)), params );
		return JS_TRUE;
	}

	JL_REPORT_ERROR("Invalid argument.");
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( target, pname, params )
  $H arguments
   $ARG GLenum target
   $ARG GLenum pname
   $ARG $VAL params: is either a number or an array of numbers.
  $H OpenGL API
   glTexEnvi, glTexEnvf, glTexEnvfv
**/
DEFINE_FUNCTION_FAST( TexEnv ) {

	JL_S_ASSERT_ARG_MIN(3);
	JL_S_ASSERT_INT(JL_FARG(1));
	JL_S_ASSERT_INT(JL_FARG(2));

	*JL_FRVAL = JSVAL_VOID;
	if ( JSVAL_IS_INT(JL_FARG(3)) ) {

		glTexEnvi( JSVAL_TO_INT( JL_FARG(1) ), JSVAL_TO_INT( JL_FARG(2) ), JSVAL_TO_INT( JL_FARG(3) ) );
		return JS_TRUE;
	}
	if ( JSVAL_IS_NUMBER(JL_FARG(3)) ) {

		jsdouble param;
		JsvalToDouble(cx, JL_FARG(3), &param);
		glTexEnvf( JSVAL_TO_INT(JL_FARG(1)), JSVAL_TO_INT(JL_FARG(2)), param );
		return JS_TRUE;
	}
	if ( JsvalIsArray(cx, JL_FARG(3)) ) {

		GLfloat params[16];
		size_t length;
		JL_CHK( JsvalToFloatVector(cx, JL_FARG(3), params, COUNTOF(params), &length ) );
		glTexEnvfv( JSVAL_TO_INT(JL_FARG(1)), JSVAL_TO_INT(JL_FARG(2)), params );
		return JS_TRUE;
	}
	JL_REPORT_ERROR("Invalid argument.");
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( pname, params )
  $H arguments
   $ARG GLenum pname
   $ARG $VAL params: is either a number or an array of numbers.
  $H OpenGL API
   glLightModeli, glLightModelf, glLightModelfv
**/
DEFINE_FUNCTION_FAST( LightModel ) {

	JL_S_ASSERT_ARG_MIN(2);
	JL_S_ASSERT_INT(JL_FARG(1));

	*JL_FRVAL = JSVAL_VOID;
	if ( JSVAL_IS_INT(JL_FARG(2)) ) {

		glLightModeli( JSVAL_TO_INT( JL_FARG(1) ), JSVAL_TO_INT( JL_FARG(2) ) );
		return JS_TRUE;
	}
	if ( JSVAL_IS_DOUBLE(JL_FARG(2)) ) {

		jsdouble param;
		JL_CHK( JsvalToDouble(cx, JL_FARG(2), &param) );
		glLightModelf( JSVAL_TO_INT( JL_FARG(1) ), param );
		return JS_TRUE;
	}
	if ( JsvalIsArray(cx, JL_FARG(2)) ) {

		GLfloat params[16];
		size_t length;
		JL_CHK( JsvalToFloatVector(cx, JL_FARG(2), params, COUNTOF(params), &length ) );
		glLightModelfv( JSVAL_TO_INT(JL_FARG(1)), params );
		return JS_TRUE;
	}
	JL_REPORT_ERROR("Invalid argument.");
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( light, pname, params )
  $H arguments
   $ARG GLenum light
   $ARG GLenum pname
   $ARG $VAL params: is either a number or an array of numbers.
  $H OpenGL API
   glLighti, glLightf, glLightfv
**/
DEFINE_FUNCTION_FAST( Light ) {

	JL_S_ASSERT_ARG_MIN(3);
	JL_S_ASSERT_INT(JL_FARG(1));
	JL_S_ASSERT_INT(JL_FARG(2));

	*JL_FRVAL = JSVAL_VOID;
	if ( JSVAL_IS_INT(JL_FARG(3)) ) {

		glLighti( JSVAL_TO_INT( JL_FARG(1) ), JSVAL_TO_INT( JL_FARG(2) ), JSVAL_TO_INT( JL_FARG(3) ) );
		return JS_TRUE;
	}
	if ( JSVAL_IS_DOUBLE(JL_FARG(3)) ) {

		jsdouble param;
		JL_CHK( JsvalToDouble(cx, JL_FARG(3), &param) );
		glLightf( JSVAL_TO_INT( JL_FARG(1) ), JSVAL_TO_INT( JL_FARG(2) ), param );
		return JS_TRUE;
	}
	if ( JsvalIsArray(cx, JL_FARG(3)) ) {

		GLfloat params[16];
		size_t length;
		JL_CHK( JsvalToFloatVector(cx, JL_FARG(3), params, COUNTOF(params), &length ) );
		glLightfv( JSVAL_TO_INT(JL_FARG(1)), JSVAL_TO_INT(JL_FARG(2)), params );
		return JS_TRUE;
	}
	JL_REPORT_ERROR("Invalid argument.");
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( face, mode )
  $H arguments
   $ARG GLenum face
   $ARG GLenum mode
  $H OpenGL API
   glColorMaterial
**/
DEFINE_FUNCTION_FAST( ColorMaterial ) {

	JL_S_ASSERT_ARG_MIN(2);
	JL_S_ASSERT_INT(JL_FARG(1));
	JL_S_ASSERT_INT(JL_FARG(2));
	glColorMaterial(JSVAL_TO_INT( JL_FARG(1) ), JSVAL_TO_INT( JL_FARG(2) ));
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( face, pname, params )
  $H arguments
   $ARG GLenum face
   $ARG GLenum pname
   $ARG $VAL params: is either a number or an array of numbers.
  $H OpenGL API
   glMateriali, glMaterialf, glMaterialfv
**/
DEFINE_FUNCTION_FAST( Material ) {

	JL_S_ASSERT_ARG_MIN(3);
	JL_S_ASSERT_INT(JL_FARG(1));
	JL_S_ASSERT_INT(JL_FARG(2));

	*JL_FRVAL = JSVAL_VOID;
	if ( JSVAL_IS_INT(JL_FARG(3)) ) {

		glMateriali( JSVAL_TO_INT( JL_FARG(1) ), JSVAL_TO_INT( JL_FARG(2) ), JSVAL_TO_INT( JL_FARG(3) ) );
		return JS_TRUE;
	}
	if ( JSVAL_IS_DOUBLE(JL_FARG(3)) ) {

		jsdouble param;
		JL_CHK( JsvalToDouble(cx, JL_FARG(3), &param) );
		glMaterialf( JSVAL_TO_INT( JL_FARG(1) ), JSVAL_TO_INT( JL_FARG(2) ), param );
		return JS_TRUE;
	}
	if ( JsvalIsArray(cx, JL_FARG(3)) ) {

		GLfloat params[16];
		size_t length;
		JL_CHK( JsvalToFloatVector(cx, JL_FARG(3), params, COUNTOF(params), &length ) );
		glMaterialfv( JSVAL_TO_INT(JL_FARG(1)), JSVAL_TO_INT(JL_FARG(2)), params );
		return JS_TRUE;
	}
	JL_REPORT_ERROR("Invalid argument.");
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( cap )
  $H arguments
   $ARG GLenum cap
  $H OpenGL API
   glEnable
**/
DEFINE_FUNCTION_FAST( Enable ) {

	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT_INT(JL_FARG(1));
	glEnable( JSVAL_TO_INT(JL_FARG(1)) );
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( cap )
  $H arguments
   $ARG GLenum cap
  $H OpenGL API
   glDisable
**/
DEFINE_FUNCTION_FAST( Disable ) {

	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT_INT(JL_FARG(1));
	glDisable( JSVAL_TO_INT(JL_FARG(1)) );
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( size )
  $H arguments
   $ARG $REAL size
  $H OpenGL API
   glPointSize
**/
DEFINE_FUNCTION_FAST( PointSize ) {

	JL_S_ASSERT_ARG_MIN(1);
	jsdouble size;
	JsvalToDouble(cx, JL_FARG(1), &size);
	glPointSize(size);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( width )
  $H arguments
   $ARG $REAL width
  $H OpenGL API
   glLineWidth
**/
DEFINE_FUNCTION_FAST( LineWidth ) {

	JL_S_ASSERT_ARG_MIN(1);
	jsdouble width;
	JsvalToDouble(cx, JL_FARG(1), &width);
	glLineWidth(width);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( mode )
  $H arguments
   $ARG GLenum mode
  $H OpenGL API
   glShadeModel
**/
DEFINE_FUNCTION_FAST( ShadeModel ) {

	JL_S_ASSERT_INT(JL_FARG(1));
	glShadeModel( JSVAL_TO_INT( JL_FARG(1) ) );
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( fFactor, dFactor )
  $H arguments
   $ARG GLenum fFactor
   $ARG GLenum dFactor
  $H OpenGL API
   glBlendFunc
**/
DEFINE_FUNCTION_FAST( BlendFunc ) {

	JL_S_ASSERT_ARG_MIN(2);
	JL_S_ASSERT_INT(JL_FARG(1));
	JL_S_ASSERT_INT(JL_FARG(2));
	glBlendFunc( JSVAL_TO_INT( JL_FARG(1) ), JSVAL_TO_INT( JL_FARG(2) ) );
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( func )
  $H arguments
   $ARG GLenum func
  $H OpenGL API
   glDepthFunc
**/
DEFINE_FUNCTION_FAST( DepthFunc ) {

	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT_INT(JL_FARG(1));
	glDepthFunc( JSVAL_TO_INT( JL_FARG(1) ) );
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( zNear, zFar )
  $H arguments
   $ARG $REAL zNear
   $ARG $REAL zFar
  $H OpenGL API
   glDepthRange
**/
DEFINE_FUNCTION_FAST( DepthRange ) {

	JL_S_ASSERT_ARG_MIN(2);
	jsdouble zNear, zFar;
	JsvalToDouble(cx, JL_FARG(1), &zNear);
	JsvalToDouble(cx, JL_FARG(2), &zFar);
	glDepthRange(zNear, zFar);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( mode )
  $H arguments
   $ARG GLenum mode
  $H OpenGL API
   glCullFace
**/
DEFINE_FUNCTION_FAST( CullFace ) {

	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT_INT(JL_FARG(1));
	glCullFace(JSVAL_TO_INT( JL_FARG(1) ));
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( mode )
  $H arguments
   $ARG GLenum mode
  $H OpenGL API
   glFrontFace
**/
DEFINE_FUNCTION_FAST( FrontFace ) {

	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT_INT(JL_FARG(1));
	glFrontFace( JSVAL_TO_INT( JL_FARG(1) ) );
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( s )
  $H arguments
   $ARG $INT s
  $H OpenGL API
   glClearStencil
**/
DEFINE_FUNCTION_FAST( ClearStencil ) {

	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT_INT(JL_FARG(1));
	glClearStencil(JSVAL_TO_INT( JL_FARG(1) ));
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( depth )
  $H arguments
   $ARG $REAL depth
  $H OpenGL API
   glClearDepth
**/
DEFINE_FUNCTION_FAST( ClearDepth ) {

	JL_S_ASSERT_ARG_MIN(1);
	jsdouble depth;
	JsvalToDouble(cx, JL_FARG(1), &depth);
	glClearDepth(depth);
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( red, green, blue, alpha )
  $H arguments
   $ARG $REAL red
   $ARG $REAL green
   $ARG $REAL blue
   $ARG $REAL alpha
  $H OpenGL API
   glClearColor
**/
DEFINE_FUNCTION_FAST( ClearColor ) {

	JL_S_ASSERT_ARG_MIN(4);
	jsdouble r, g, b, a;
	JsvalToDouble(cx, JL_FARG(1), &r);
	JsvalToDouble(cx, JL_FARG(2), &g);
	JsvalToDouble(cx, JL_FARG(3), &b);
	JsvalToDouble(cx, JL_FARG(4), &a);
	glClearColor(r, g, b, a);
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( red, green, blue, alpha )
  $H arguments
   $ARG $REAL red
   $ARG $REAL green
   $ARG $REAL blue
   $ARG $REAL alpha
  $H OpenGL API
   glClearAccum
**/
DEFINE_FUNCTION_FAST( ClearAccum ) {

	JL_S_ASSERT_ARG_MIN(4);
	jsdouble r, g, b, a;
	JsvalToDouble(cx, JL_FARG(1), &r);
	JsvalToDouble(cx, JL_FARG(2), &g);
	JsvalToDouble(cx, JL_FARG(3), &b);
	JsvalToDouble(cx, JL_FARG(4), &a);
	glClearAccum(r, g, b, a);
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( mask )
  $H arguments
   $ARG GLbitfield mask
  $H OpenGL API
   glClear
**/
DEFINE_FUNCTION_FAST( Clear ) {

	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT_INT(JL_FARG(1));
	glClear(JSVAL_TO_INT(JL_FARG(1)));
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( plane, equation )
  $H arguments
   $ARG GLenum plane
   $ARG $ARRAY equation: array of real
  $H OpenGL API
   glClipPlane
**/
DEFINE_FUNCTION_FAST( ClipPlane ) {

	JL_S_ASSERT_ARG_MIN(2);
	JL_S_ASSERT_INT(JL_FARG(1));
	JL_S_ASSERT_ARRAY(JL_FARG(2));
	GLdouble equation[16];
	size_t length;
	JL_CHK( JsvalToDoubleVector(cx, JL_FARG(2), equation, COUNTOF(equation), &length ) );
	glClipPlane(JSVAL_TO_INT(JL_FARG(1)), equation);
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( x, y, width, height )
  $H arguments
   $ARG $INT x
   $ARG $INT y
   $ARG $INT width
   $ARG $INT height
  $H OpenGL API
   glViewport
**/
DEFINE_FUNCTION_FAST( Viewport ) {

	JL_S_ASSERT_ARG_MIN(4);
	JL_S_ASSERT_INT(JL_FARG(1));
	JL_S_ASSERT_INT(JL_FARG(2));
	JL_S_ASSERT_INT(JL_FARG(3));
	JL_S_ASSERT_INT(JL_FARG(4));
	glViewport(JSVAL_TO_INT(JL_FARG(1)), JSVAL_TO_INT(JL_FARG(2)), JSVAL_TO_INT(JL_FARG(3)), JSVAL_TO_INT(JL_FARG(4)));
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( left, right, bottom, top, zNear, zFar )
  $H arguments
   $ARG $REAL left
   $ARG $REAL right
   $ARG $REAL bottom
   $ARG $REAL top
   $ARG $REAL zNear
   $ARG $REAL zFar
  $H OpenGL API
   glFrustum
**/
DEFINE_FUNCTION_FAST( Frustum ) {

	JL_S_ASSERT_ARG_MIN(6);
	jsdouble left, right, bottom, top, zNear, zFar;
	JsvalToDouble(cx, JL_FARG(1), &left);
	JsvalToDouble(cx, JL_FARG(2), &right);
	JsvalToDouble(cx, JL_FARG(3), &bottom);
	JsvalToDouble(cx, JL_FARG(4), &top);
	JsvalToDouble(cx, JL_FARG(5), &zNear);
	JsvalToDouble(cx, JL_FARG(6), &zFar);
	glFrustum(left, right, bottom, top, zNear, zFar);
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( left, right, bottom, top, zNear, zFar )
  $H arguments
   $ARG $REAL left
   $ARG $REAL right
   $ARG $REAL bottom
   $ARG $REAL top
   $ARG $REAL zNear
   $ARG $REAL zFar
  $H OpenGL API
   glOrtho
**/
DEFINE_FUNCTION_FAST( Ortho ) {

	JL_S_ASSERT_ARG_MIN(6);
	jsdouble left, right, bottom, top, zNear, zFar;
	JsvalToDouble(cx, JL_FARG(1), &left);
	JsvalToDouble(cx, JL_FARG(2), &right);
	JsvalToDouble(cx, JL_FARG(3), &bottom);
	JsvalToDouble(cx, JL_FARG(4), &top);
	JsvalToDouble(cx, JL_FARG(5), &zNear);
	JsvalToDouble(cx, JL_FARG(6), &zFar);

//	glMatrixMode(GL_PROJECTION);
//	glLoadIdentity();
	glOrtho(left, right, bottom, top, zNear, zFar);
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( fovy, zNear, zFar )
  Set up a perspective projection matrix.
  $H arguments
   $ARG $REAL fovy
   $ARG $REAL zNear
   $ARG $REAL zFar
  $H note
   This is not an OpenGL API function.
  $H OpenGL API
   glGetIntegerv, glFrustum
**/
DEFINE_FUNCTION_FAST( Perspective ) {

	//cf. gluPerspective(fovy, float(viewport[2]) / float(viewport[3]), zNear, zFar);

	JL_S_ASSERT_ARG(3);
	jsdouble fovy, zNear, zFar;
	JsvalToDouble(cx, JL_FARG(1), &fovy);
	JsvalToDouble(cx, JL_FARG(2), &zNear);
	JsvalToDouble(cx, JL_FARG(3), &zFar);

//	GLint prevMatrixMode;
//	glGetIntegerv(GL_MATRIX_MODE, &prevMatrixMode); // GL_MODELVIEW

	GLint viewport[4];
	glGetIntegerv( GL_VIEWPORT, viewport );
	double aspect = double(viewport[2]) / double(viewport[3]);

	double xmin, xmax, ymin, ymax;
	ymax = zNear * tan(fovy * M_PI / 360.0f);
	ymin = -ymax;
	xmin = ymin * aspect;
	xmax = ymax * aspect;

	glFrustum(xmin, xmax, ymin, ymax, zNear, zFar);

/*
   float x = (2.0F*nearZ) / (right-left);
   float y = (2.0F*nearZ) / (top-bottom);
   float a = (right+left) / (right-left);
   float b = (top+bottom) / (top-bottom);
   float c = -(farZ+nearZ) / ( farZ-nearZ);
   float d = -(2.0F*farZ*nearZ) / (farZ-nearZ);

#define M(row,col)  m[col*4+row]
   M(0,0) = x;     M(0,1) = 0.0F;  M(0,2) = a;      M(0,3) = 0.0F;
   M(1,0) = 0.0F;  M(1,1) = y;     M(1,2) = b;      M(1,3) = 0.0F;
   M(2,0) = 0.0F;  M(2,1) = 0.0F;  M(2,2) = c;      M(2,3) = d;
   M(3,0) = 0.0F;  M(3,1) = 0.0F;  M(3,2) = -1.0F;  M(3,3) = 0.0F;
#undef M
*/

	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( mode )
  $H arguments
   $ARG GLenum mode
  $H OpenGL API
   glMatrixMode
**/
DEFINE_FUNCTION_FAST( MatrixMode ) {

	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT_INT(JL_FARG(1));
	glMatrixMode(JSVAL_TO_INT( JL_FARG(1) ));
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  $H OpenGL API
   glLoadIdentity
**/
DEFINE_FUNCTION_FAST( LoadIdentity ) {

	glLoadIdentity();
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  $H OpenGL API
   glPushMatrix
**/
DEFINE_FUNCTION_FAST( PushMatrix ) {

	glPushMatrix();
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  $H OpenGL API
   glPopMatrix
**/
DEFINE_FUNCTION_FAST( PopMatrix ) {

	glPopMatrix();
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( matrix )
  $H arguments
   $ARG $VAL matrix: either a matrix object or an Array
  $H OpenGL API
   glLoadMatrixf
**/
DEFINE_FUNCTION_FAST( LoadMatrix ) {

	JL_S_ASSERT_ARG_MIN(1);
	float tmp[16], *m = tmp;

	JL_S_ASSERT_OBJECT( JL_FARG(1) );
	JSObject *matrixObj = JSVAL_TO_OBJECT( JL_FARG(1) );
	NIMatrix44Get fct = Matrix44GetInterface(cx, matrixObj);
	JL_S_ASSERT( fct, "Invalid Matrix44 interface." );
	JL_CHK( fct(cx, matrixObj, &m) );
	glLoadMatrixf(m);
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( matrix )
  $H arguments
   $ARG $VAL matrix: either a matrix object or an Array
  $H OpenGL API
   glLoadMatrixf
**/
DEFINE_FUNCTION_FAST( MultMatrix ) {

	JL_S_ASSERT_ARG_MIN(1);
	float tmp[16], *m = tmp;

	JL_S_ASSERT_OBJECT( JL_FARG(1) );
	JSObject *matrixObj = JSVAL_TO_OBJECT( JL_FARG(1) );
	NIMatrix44Get fct = Matrix44GetInterface(cx, matrixObj);
	JL_S_ASSERT( fct, "Invalid Matrix44 interface." );
	JL_CHK( fct(cx, matrixObj, &m) );
	glMultMatrixf(m);
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( angle, x, y, z )
  $H arguments
   $ARG $REAL angle
   $ARG $REAL x
   $ARG $REAL y
   $ARG $REAL z
  $H OpenGL API
   glRotated
**/
DEFINE_FUNCTION_FAST( Rotate ) {

	JL_S_ASSERT_ARG_MIN(4);
	jsdouble angle, x, y, z;
	JsvalToDouble(cx, JL_FARG(1), &angle);
	JsvalToDouble(cx, JL_FARG(2), &x);
	JsvalToDouble(cx, JL_FARG(3), &y);
	JsvalToDouble(cx, JL_FARG(4), &z);
	glRotated(angle, x, y, z);
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( x, y [, z = 0] )
  $H arguments
   $ARG $REAL x
   $ARG $REAL y
   $ARG $REAL z
  $H OpenGL API
   glTranslated
**/
DEFINE_FUNCTION_FAST( Translate ) {

	JL_S_ASSERT_ARG_MIN(3);
	jsdouble x, y, z;
	JsvalToDouble(cx, JL_FARG(1), &x);
	JsvalToDouble(cx, JL_FARG(2), &y);
	if ( JL_FARG_ISDEF(3) )
		JsvalToDouble(cx, JL_FARG(3), &z);
	else
		z = 0;
	glTranslated(x, y, z);
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( x, y [, z = 1] )
  $H arguments
   $ARG $REAL x
   $ARG $REAL y
   $ARG $REAL z
  $H OpenGL API
   glScaled
**/
DEFINE_FUNCTION_FAST( Scale ) {

	JL_S_ASSERT_ARG_MIN(2);
	jsdouble x, y, z;
	JsvalToDouble(cx, JL_FARG(1), &x);
	JsvalToDouble(cx, JL_FARG(2), &y);
	if ( JL_FARG_ISDEF(3) )
		JsvalToDouble(cx, JL_FARG(3), &z);
	else
		z = 1;
	glScaled(x, y, z);
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME()
  Returns a new display-list.
  $H OpenGL API
   glNewList
**/
DEFINE_FUNCTION_FAST( NewList ) {

	GLuint list = glGenLists(1);
	glNewList(list, GL_COMPILE);
	*JL_FRVAL = INT_TO_JSVAL(list);
	return JS_TRUE;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( list )
  Deletes a display-list.
  $H arguments
   $ARG $INT list
  $H OpenGL API
   glDeleteLists
**/
DEFINE_FUNCTION_FAST( DeleteList ) {

	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT_INT(JL_FARG(1));
	GLuint list = JSVAL_TO_INT(JL_FARG(1));
	glDeleteLists(list, 1);
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  $H OpenGL API
   glEndList
**/
DEFINE_FUNCTION_FAST( EndList ) {

	glEndList();
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( lists )
  Calls one or more display-list.
  $H arguments
   $ARG $VAL lists: is a single list name or an Array of list name.
  $H OpenGL API
   glCallList, glCallLists
**/
DEFINE_FUNCTION_FAST( CallList ) {

	JL_S_ASSERT_ARG_MIN(1);
	*JL_FRVAL = JSVAL_VOID;

	if (JSVAL_IS_INT( JL_FARG(1) )) {

		GLuint list = JSVAL_TO_INT(JL_FARG(1));
		glCallList(list);
		return JS_TRUE;
	}
	else if (JsvalIsArray(cx, JL_FARG(1))) {

		JSObject *jsArray = JSVAL_TO_OBJECT(JL_FARG(1));
		jsuint length;
		JL_CHK( JS_GetArrayLength(cx, jsArray, &length) );

		GLuint *lists = (GLuint*)malloc(length * sizeof(GLuint));
		jsval value;
		for (jsuint i=0; i<length; ++i) {

			JL_CHK( JS_GetElement(cx, jsArray, i, &value) );
			lists[i] = JSVAL_TO_INT(value);
		}
		glCallLists(length, GL_UNSIGNED_INT, lists); // http://www.opengl.org/documentation/specs/man_pages/hardcopy/GL/html/gl/calllists.html
		free(lists);
		return JS_TRUE;
	}

	JL_REPORT_ERROR("Invalid argument");
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( mode )
  $H arguments
   $ARG GLenum mode
  $H OpenGL API
   glBegin
**/
DEFINE_FUNCTION_FAST( Begin ) {

	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT_INT(JL_FARG(1));
	glBegin(JSVAL_TO_INT( JL_FARG(1) ));
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  $H OpenGL API
   glEnd
**/
DEFINE_FUNCTION_FAST( End ) {

	glEnd();
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( mask )
  $H arguments
   $ARG GLbitfield mask
  $H OpenGL API
   glPushAttrib
**/
DEFINE_FUNCTION_FAST( PushAttrib ) {

	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT_INT(JL_FARG(1));
	glPushAttrib(JSVAL_TO_INT( JL_FARG(1) ));
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  $H OpenGL API
   glPopAttrib
**/
DEFINE_FUNCTION_FAST( PopAttrib ) {

	glPopAttrib();
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME()
  Returns a new texture name.
  $H OpenGL API
   glGenTextures
**/
DEFINE_FUNCTION_FAST( GenTexture ) {

	GLuint texture;
	glGenTextures( 1, &texture );
	*JL_FRVAL = INT_TO_JSVAL(texture);
	return JS_TRUE;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( target, texture )
  $H arguments
   $ARG GLenum target
   $ARG $INT texture
  $H OpenGL API
   glBindTexture
**/
DEFINE_FUNCTION_FAST( BindTexture ) {

	JL_S_ASSERT_ARG_MIN(2);
	JL_S_ASSERT_INT(JL_FARG(1));
	JL_S_ASSERT_INT(JL_FARG(2));
	glBindTexture( JSVAL_TO_INT( JL_FARG(1) ), JSVAL_TO_INT( JL_FARG(2) ));
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( texture )
  Deletes the given texture.
  $H arguments
   $ARG $INT texture
  $H OpenGL API
   glDeleteTextures
**/
DEFINE_FUNCTION_FAST( DeleteTexture ) {

	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT_INT(JL_FARG(1));
	GLuint texture = JSVAL_TO_INT( JL_FARG(1) );
	glDeleteTextures(1, &texture);
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( level, internalFormat, x, y, width, height, [ border ] )
  $H arguments
   $ARG $INT level
   $ARG $INT internalFormat
   $ARG $INT x
   $ARG $INT y
   $ARG $INT width
   $ARG $INT height
   $ARG $INT border
  $H note
   The target is always a GL_TEXTURE_2D
  $H OpenGL API
   glCopyTexImage2D
**/
DEFINE_FUNCTION_FAST( CopyTexImage2D ) {

	JL_S_ASSERT_ARG_MIN(6);
	JL_S_ASSERT_INT(JL_FARG(1));
	JL_S_ASSERT_INT(JL_FARG(2));
	JL_S_ASSERT_INT(JL_FARG(3));
	JL_S_ASSERT_INT(JL_FARG(4));
	JL_S_ASSERT_INT(JL_FARG(5));
	JL_S_ASSERT_INT(JL_FARG(6));

	GLint level = JSVAL_TO_INT(JL_FARG(1));
	GLenum internalFormat = JSVAL_TO_INT(JL_FARG(2));

	GLint x = JSVAL_TO_INT(JL_FARG(3));
	GLint y = JSVAL_TO_INT(JL_FARG(4));
	GLint width = JSVAL_TO_INT(JL_FARG(5));
	GLint height = JSVAL_TO_INT(JL_FARG(6));

	GLint border;
	if ( JL_FARG_ISDEF(7) )
		border = JSVAL_TO_INT(JL_FARG(7));
	else
		border = 0;

	glCopyTexImage2D( GL_TEXTURE_2D, level, internalFormat, x, y, width, height, border );

	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/*
DEFINE_FUNCTION_FAST( TexSubImage2D ) {

	JL_S_ASSERT_ARG_MIN(7);
	JL_S_ASSERT_INT(JL_FARG(1));
	JL_S_ASSERT_INT(JL_FARG(2));
	JL_S_ASSERT_INT(JL_FARG(3));
	JL_S_ASSERT_INT(JL_FARG(4));
	JL_S_ASSERT_INT(JL_FARG(5));
	JL_S_ASSERT_INT(JL_FARG(6));

	GLint level = JSVAL_TO_INT(JL_FARG(1));
	GLenum internalFormat = JSVAL_TO_INT(JL_FARG(2));

	GLint xoffset = JSVAL_TO_INT(JL_FARG(3));
	GLint yoffset = JSVAL_TO_INT(JL_FARG(4));
	GLint x = JSVAL_TO_INT(JL_FARG(5));
	GLint y = JSVAL_TO_INT(JL_FARG(6));
	GLint width = JSVAL_TO_INT(JL_FARG(7));
	GLint height = JSVAL_TO_INT(JL_FARG(8));

	GLint border;
	if ( JL_FARG_ISDEF(7) )
		border = JSVAL_TO_INT(JL_FARG(7));
	else
		border = 0;

	glTexSubImage2D( GL_TEXTURE_2D, level, xoffset, yoffset, width, height, format,  border );

	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}
*/



///////////////////////////////////////////////////////////////////////////////
// OpenGL extensions


/**doc
$TOC_MEMBER $INAME
 $INT $INAME()
  Returns a new buffer.
  $H OpenGL API
   glGenBuffersARB
**/
DEFINE_FUNCTION_FAST( GenBuffer ) {

	LOAD_OPENGL_EXTENSION( glGenBuffersARB, PFNGLGENBUFFERSARBPROC );

	GLuint buffer;
	glGenBuffersARB(1, &buffer);
	*JL_FRVAL = INT_TO_JSVAL(buffer);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( target, buffer )
  $H arguments
   $ARG GLenum target
   $ARG $INT buffer
  $H OpenGL API
   glBindBufferARB
**/
DEFINE_FUNCTION_FAST( BindBuffer ) {

	LOAD_OPENGL_EXTENSION( glBindBufferARB, PFNGLBINDBUFFERARBPROC );

	JL_S_ASSERT_ARG_MIN(2);
	JL_S_ASSERT_INT(JL_FARG(1));
	JL_S_ASSERT_INT(JL_FARG(2));
	GLenum target = JSVAL_TO_INT(JL_FARG(1));
	GLenum buffer = JSVAL_TO_INT(JL_FARG(2));
	glBindBufferARB(target, buffer);
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( pname, params )
  $H arguments
   $ARG GLenum pname
   $ARG $VAL params: is a real or an Array of real.
  $H OpenGL API
   glPointParameterf, glPointParameterfv
**/
DEFINE_FUNCTION_FAST( PointParameter ) {

	LOAD_OPENGL_EXTENSION( glPointParameterf, PFNGLPOINTPARAMETERFPROC );
	LOAD_OPENGL_EXTENSION( glPointParameterfv, PFNGLPOINTPARAMETERFVPROC );

	JL_S_ASSERT_ARG_MIN(2);
	JL_S_ASSERT_INT(JL_FARG(1));

	*JL_FRVAL = JSVAL_VOID;
//	if ( JSVAL_IS_INT(JL_FARG(2)) ) {
//
//		glPointParameteri(JSVAL_TO_INT(JL_FARG(1)), JSVAL_TO_INT(JL_FARG(2)));
//		return JS_TRUE;
//	}
	if ( JSVAL_IS_NUMBER(JL_FARG(2)) ) {

		jsdouble param;
		JsvalToDouble(cx, JL_FARG(2), &param);
		glPointParameterf( JSVAL_TO_INT(JL_FARG(1)), param );
		return JS_TRUE;
	}
	if ( JsvalIsArray(cx, JL_FARG(2)) ) {

		GLfloat params[16];
		size_t length;
		JL_CHK( JsvalToFloatVector(cx, JL_FARG(2), params, COUNTOF(params), &length ) );
		glPointParameterfv( JSVAL_TO_INT(JL_FARG(1)), params );
		return JS_TRUE;
	}
	JL_REPORT_ERROR("Invalid argument.");
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( texture )
  $H arguments
   $ARG GLenum texture
  $H OpenGL API
   glActiveTextureARB
**/
DEFINE_FUNCTION_FAST( ActiveTexture ) {

	LOAD_OPENGL_EXTENSION( glActiveTextureARB, PFNGLACTIVETEXTUREARBPROC );

	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT_INT(JL_FARG(1));
	GLenum texture = JSVAL_TO_INT(JL_FARG(1));
	glActiveTextureARB(texture);
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( texture )
  $H arguments
   $ARG GLenum texture
  $H OpenGL API
   glClientActiveTextureARB
**/
DEFINE_FUNCTION_FAST( ClientActiveTexture ) {

	LOAD_OPENGL_EXTENSION( glClientActiveTextureARB, PFNGLCLIENTACTIVETEXTUREARBPROC );

	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT_INT(JL_FARG(1));
	GLenum texture = JSVAL_TO_INT(JL_FARG(1));
	glClientActiveTextureARB(texture);
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( target, s [, t [, r]] )
  $H arguments
   $ARG GLenum target
   $ARG $REAL s
   $ARG $REAL t
   $ARG $REAL r
  $H OpenGL API
   glMultiTexCoord1d, glMultiTexCoord2d, glMultiTexCoord3d
**/
DEFINE_FUNCTION_FAST( MultiTexCoord ) {

	LOAD_OPENGL_EXTENSION( glMultiTexCoord1d, PFNGLMULTITEXCOORD1DARBPROC );
	LOAD_OPENGL_EXTENSION( glMultiTexCoord2d, PFNGLMULTITEXCOORD2DARBPROC );
	LOAD_OPENGL_EXTENSION( glMultiTexCoord3d, PFNGLMULTITEXCOORD3DARBPROC );

	JL_S_ASSERT_ARG_RANGE(2,4);

	JL_S_ASSERT_INT(JL_FARG(1));
	GLenum target = JSVAL_TO_INT(JL_FARG(1));

	*JL_FRVAL = JSVAL_VOID;
	jsdouble s;
	JsvalToDouble(cx, JL_FARG(2), &s);
	if ( JL_ARGC == 2 ) {

		glMultiTexCoord1d(target, s);
		return JS_TRUE;
	}
	jsdouble t;
	JsvalToDouble(cx, JL_FARG(3), &t);
	if ( JL_ARGC == 3 ) {

		glMultiTexCoord2d(target, s, t);
		return JS_TRUE;
	}
	jsdouble r;
	JsvalToDouble(cx, JL_FARG(4), &r);
	if ( JL_ARGC == 4 ) {

		glMultiTexCoord3d(target, s, t, r);
		return JS_TRUE;
	}
	JL_REPORT_ERROR("Invalid argument.");
	JL_BAD;
}




/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz )
  $H arguments
   $ARG $REAL x
   $ARG $REAL y
   $ARG $REAL z
  $H API
   gluLookAt
**/
DEFINE_FUNCTION_FAST( LookAt ) {

	JL_S_ASSERT_ARG(9);
	double eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz;
	JsvalToDouble(cx, JL_FARG(1), &eyex);
	JsvalToDouble(cx, JL_FARG(2), &eyey);
	JsvalToDouble(cx, JL_FARG(3), &eyez);

	JsvalToDouble(cx, JL_FARG(4), &centerx);
	JsvalToDouble(cx, JL_FARG(5), &centery);
	JsvalToDouble(cx, JL_FARG(6), &centerz);

	JsvalToDouble(cx, JL_FARG(7), &upx);
	JsvalToDouble(cx, JL_FARG(8), &upy);
	JsvalToDouble(cx, JL_FARG(9), &upz);
	gluLookAt(eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz);
	return JS_TRUE;
	JL_BAD;
}


///////////////////////////////////////////////////////////////////////////////
// non-OpenGL API


	// doc: http://www.songho.ca/opengl/gl_vbo.html#create
/*
	LOAD_OPENGL_EXTENSION( glBindBufferARB, PFNGLBINDBUFFERARBPROC );
	LOAD_OPENGL_EXTENSION( glDeleteBuffersARB, PFNGLDELETEBUFFERSARBPROC );
	LOAD_OPENGL_EXTENSION( glGenBuffersARB, PFNGLGENBUFFERSARBPROC );
	LOAD_OPENGL_EXTENSION( glIsBufferARB, PFNGLISBUFFERARBPROC );
	LOAD_OPENGL_EXTENSION( glBufferDataARB, PFNGLBUFFERDATAARBPROC );
	LOAD_OPENGL_EXTENSION( glBufferSubDataARB, PFNGLBUFFERSUBDATAARBPROC );
	LOAD_OPENGL_EXTENSION( glGetBufferSubDataARB, PFNGLGETBUFFERSUBDATAARBPROC );
	LOAD_OPENGL_EXTENSION( glMapBufferARB, PFNGLMAPBUFFERARBPROC );
	LOAD_OPENGL_EXTENSION( glUnmapBufferARB, PFNGLUNMAPBUFFERARBPROC );
	LOAD_OPENGL_EXTENSION( glGetBufferParameterivARB, PFNGLGETBUFFERPARAMETERIVARBPROC );
	LOAD_OPENGL_EXTENSION( glGetBufferPointervARB, PFNGLGETBUFFERPOINTERVARBPROC );
*/


#define TRIMESH_ID_NAME 'GlTr'

struct OpenGlTrimeshInfo {

	GLuint indexBuffer, vertexBuffer, normalBuffer, texCoordBuffer, colorBuffer;
	int vertexCount, indexCount;
};

void FinalizeTrimesh(void *pv) {

/* (TBD)!
	static PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC) GL_GET_PROC_ADDRESS( "PFNGLDELETEBUFFERSARBPROC" );

	OpenGlTrimeshInfo *info = (OpenGlTrimeshInfo*)pv;
	glDeleteBuffersARB( 1, &info->indexBuffer );
	glDeleteBuffersARB( 1, &info->vertexBuffer );
	if ( info->indexBuffer )
		glDeleteBuffersARB( 1, &info->indexBuffer );
	if ( info->texCoordBuffer )
		glDeleteBuffersARB( 1, &info->texCoordBuffer );
	if ( info->colorBuffer )
		glDeleteBuffersARB( 1, &info->colorBuffer );
*/
}

/**doc
$TOC_MEMBER $INAME
 $TYPE Id $INAME( trimesh )
**/
DEFINE_FUNCTION_FAST( LoadTrimesh ) {

	LOAD_OPENGL_EXTENSION( glGenBuffersARB, PFNGLGENBUFFERSARBPROC );
	LOAD_OPENGL_EXTENSION( glBindBufferARB, PFNGLBINDBUFFERARBPROC );
	LOAD_OPENGL_EXTENSION( glBufferDataARB, PFNGLBUFFERDATAARBPROC );

	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT_OBJECT(JL_FARG(1));
	JSObject *trimeshObj = JSVAL_TO_OBJECT(JL_FARG(1));
	JL_S_ASSERT( JsvalIsTrimesh(cx, JL_FARG(1)), "Invalid Trimesh object" );
	Surface *srf = GetTrimeshSurface(cx, trimeshObj);
	JL_S_ASSERT_RESOURCE(srf);

	JL_S_ASSERT( srf->vertex && srf->vertexCount && srf->index && srf->indexCount, "No enough data" );

	OpenGlTrimeshInfo *info;
	JL_CHK( CreateId(cx, TRIMESH_ID_NAME, sizeof(OpenGlTrimeshInfo), (void**)&info, FinalizeTrimesh, JL_FRVAL) );

	info->vertexCount = srf->vertexCount;
	info->indexCount = srf->indexCount;

	glGenBuffersARB(1, &info->indexBuffer);

	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, info->indexBuffer);

	glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, srf->indexCount * sizeof(SURFACE_INDEX_TYPE), srf->index, GL_STATIC_DRAW_ARB);

	glGenBuffersARB(1, &info->vertexBuffer);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, info->vertexBuffer);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, srf->vertexCount * 3 * sizeof(SURFACE_REAL_TYPE), srf->vertex, GL_STATIC_DRAW_ARB);

	if ( srf->normal ) {

		glGenBuffersARB(1, &info->normalBuffer);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, info->normalBuffer);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB, srf->vertexCount * 3 * sizeof(SURFACE_REAL_TYPE), srf->normal, GL_STATIC_DRAW_ARB);
	} else
		info->normalBuffer = 0;

	if ( srf->textureCoordinate ) {

		glGenBuffersARB(1, &info->texCoordBuffer);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, info->texCoordBuffer);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB, srf->vertexCount * 3 * sizeof(SURFACE_REAL_TYPE), srf->textureCoordinate, GL_STATIC_DRAW_ARB);
	} else
		info->texCoordBuffer = 0;

	if ( srf->color ) {

		glGenBuffersARB(1, &info->colorBuffer);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, info->colorBuffer);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB, srf->vertexCount * 4 * sizeof(SURFACE_REAL_TYPE), srf->color, GL_STATIC_DRAW_ARB);
	} else
		info->colorBuffer = 0;

	JL_CHK( CheckThrowCurrentOglError(cx) );

	return JS_TRUE;
	JL_BAD;
}




/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  $H OpenGL API
   glVertexPointer
**/
DEFINE_FUNCTION_FAST( DrawTrimesh ) {

	LOAD_OPENGL_EXTENSION( glBindBufferARB, PFNGLBINDBUFFERARBPROC );

	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT_OBJECT(JL_FARG(1));

	JL_S_ASSERT( IsIdType(cx, JL_FARG(1), TRIMESH_ID_NAME), "Invalid Id." );

	OpenGlTrimeshInfo *info = (OpenGlTrimeshInfo*)GetIdPrivate(cx, JL_FARG(1));

	GLenum dataType = sizeof(SURFACE_REAL_TYPE) == sizeof(float) ? GL_FLOAT : GL_DOUBLE;

	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, info->vertexBuffer);
	glVertexPointer(3, dataType, 0, 0);

	if ( info->normalBuffer ) {

		glEnableClientState(GL_NORMAL_ARRAY);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, info->normalBuffer);
		glNormalPointer(dataType, 0, 0);
	}

	if ( info->texCoordBuffer ) {

		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, info->texCoordBuffer);
		glTexCoordPointer(3, dataType, 0, 0);
	}

	if ( info->colorBuffer ) {

		glEnableClientState(GL_COLOR_ARRAY);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, info->colorBuffer);
		glColorPointer(4, dataType, 0, 0);
	}

//	glEnableClientState(GL_INDEX_ARRAY);
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, info->indexBuffer);
	glDrawElements(GL_TRIANGLES, info->indexCount, GL_UNSIGNED_INT, 0); // 1 triangle = 3 vertex

//	glDisableClientState(GL_INDEX_ARRAY);
	if ( info->colorBuffer )
		glDisableClientState(GL_COLOR_ARRAY);
	if ( info->texCoordBuffer )
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	if ( info->normalBuffer )
		glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY); // deactivate vertex array

	// bind with 0, so, switch back to normal pointer operation
//	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
//	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

	JL_CHK( CheckThrowCurrentOglError(cx) );

	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}




/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( target, [internalformat], texture )
  $H arguments
   $ARG GLenum target
   $ARG $INT internalformat: is the internal PixelFormat. If undefined, the function will use the format of _texture_.
   $ARG $VAL texture: either a Texture object or an image object.
  $H note
   This is not an OpenGL API function.
  $H OpenGL API
   glPixelStorei, glTexImage2D
**/
// (TBD) manage compression: http://www.opengl.org/registry/specs/ARB/texture_compression.txt
DEFINE_FUNCTION_FAST( DefineTextureImage ) {

	JL_S_ASSERT_ARG_MIN(3);
	JL_S_ASSERT_INT(JL_FARG(1));
//	JL_S_ASSERT_INT(JL_FARG(2)); // may be undefined
	JL_S_ASSERT_OBJECT(JL_FARG(3));

	GLsizei width, height;
	GLenum format, type;
	int channels;
	const GLvoid *data;

	JSObject *tObj = JSVAL_TO_OBJECT(JL_FARG(3));

	if ( JL_GetClass(tObj) == TextureJSClass(cx) ) {

		Texture *tex = (Texture *)JL_GetPrivate(cx, tObj);
		JL_S_ASSERT_RESOURCE(tex);

		data = tex->cbuffer;
		width = tex->width;
		height = tex->height;
		channels = tex->channels;
		type = GL_FLOAT;
	} else {

		JL_CHK( GetPropertyInt(cx, tObj, "width", &width) );
		JL_CHK( GetPropertyInt(cx, tObj, "height", &height) );
		JL_CHK( GetPropertyInt(cx, tObj, "channels", &channels) );
		size_t bufferLength;
		jsval tVal = OBJECT_TO_JSVAL(tObj);
		JL_CHK( JsvalToStringAndLength(cx, &tVal, (const char**)&data, &bufferLength ) );
		JL_S_ASSERT( bufferLength == width * height * channels * 1, "Invalid image format." );
		JL_S_ASSERT_RESOURCE(data);
		type = GL_UNSIGNED_BYTE;
	}
// else
//		JL_REPORT_ERROR("Invalid texture type.");

	if ( JL_FARG_ISDEF(2) ) {

		JL_S_ASSERT_INT(JL_FARG(2));
		format = JSVAL_TO_INT(JL_FARG(2));
	} else {

		switch ( channels ) {
			case 1:
				format = GL_LUMINANCE;
				break;
			case 2:
				format = GL_LUMINANCE_ALPHA;
				break;
			case 3:
				format = GL_RGB;
				break;
			case 4:
				format = GL_RGBA;
				break;
		}
	}

	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glTexImage2D( JSVAL_TO_INT(JL_FARG(1)), 0, format, width, height, 0, format, type, data );
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $TYPE image $INAME()
  Returns the current contain of the viewport.
  $H arguments
   $ARG GLenum target
   $ARG $INT internalformat: is the internal PixelFormat. If undefined, the function will use the format of _texture_.
   $ARG $VAL texture: either a Texture object or an image object.
  $H return value
   An image object.
  $H note
   This is not an OpenGL API function.
  $H OpenGL API
   glGenTextures, glBindTexture, glGetIntegerv, glCopyTexImage2D, glGetTexLevelParameteriv, glGetTexImage, glDeleteTextures
**/
DEFINE_FUNCTION_FAST( RenderToImage ) {

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	GLint viewport[4];
	glGetIntegerv( GL_VIEWPORT, viewport );
	int width = viewport[2];
	int height = viewport[3];

	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, width, height, 0);

	GLint tWidth, tHeight;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &tWidth);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &tHeight);
//	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPONENTS, &tComponents);
	//  glGet	with arguments GL_PACK_ALIGNMENT and others

	int size = tWidth * tHeight * 4; // RGBA
	void *pixels = malloc(size);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	// (TBD) render to a blob !
	JSObject *blobObj = NewImage(cx, tWidth, tHeight, 4, pixels);
	JL_CHK( blobObj );
	*JL_FRVAL = OBJECT_TO_JSVAL(blobObj);

	glDeleteTextures(1, &texture);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME()
**/
DEFINE_FUNCTION_FAST( PixelWidth ) {

	// see. http://www.songho.ca/opengl/gl_projectionmatrix.html
	// see. engine_core.h
	JL_S_ASSERT_ARG(2);
	float width, distance;
	JL_CHK( JsvalToFloat(cx, JL_FARG(1), &width) );
	JL_CHK( JsvalToFloat(cx, JL_FARG(2), &distance) );
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	int viewportWidth = viewport[2]; // int viewportHeight = viewport[3];
	GLfloat m[16];
	glGetFloatv(GL_PROJECTION_MATRIX, m);
	float pixelWidth = width * viewportWidth * m[0] / distance;
	return FloatToJsval(cx, pixelWidth, JL_FRVAL);
	JL_BAD;
}


/**doc
=== Static properties ===
**/


/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  $H OpenGL API
   glGetError
**/
DEFINE_PROPERTY(error) {

	// When an error occurs, the error flag is set to the appropriate error code value. No other errors are recorded
	// until glGetError is called, the error code is returned, and the flag is reset to GL_NO_ERROR.
	*vp = INT_TO_JSVAL(glGetError());
	return JS_TRUE;
}


static int MatrixGet(JSContext *cx, JSObject *obj, float **m) {

	GLint matrixMode;
	glGetIntegerv(GL_MATRIX_MODE, &matrixMode);
	switch ( matrixMode ) {
		case GL_MODELVIEW:
			glGetFloatv(GL_MODELVIEW_MATRIX, *m);
			return true;
		case GL_PROJECTION:
			glGetFloatv(GL_PROJECTION_MATRIX, *m);
			return true;
		case GL_TEXTURE:
			glGetFloatv(GL_TEXTURE_MATRIX, *m);
			return true;
	}
	return false; // JL_REPORT_ERROR( "Unsupported matrix mode." );
}


JSBool Init( JSContext *cx, JSObject *obj ) {

	return SetMatrix44GetInterface(cx, obj, MatrixGet);
}


/**doc
=== Native Interface ===
 * *NIMatrix44Read*
  the current OpenGL matrix. See MatrixMode() to specifiy which matrix stack is the target forsubsequent matrix operations.
**/


/**doc
== more information ==
 [http://www.glprogramming.com/blue/ OpenGL API Documentation]
**/

CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_INIT

	BEGIN_CONST_INTEGER_SPEC
// OpenGL constants
		CONST_INTEGER( ACCUM  , GL_ACCUM  )
		CONST_INTEGER( LOAD   , GL_LOAD   )
		CONST_INTEGER( RETURN , GL_RETURN )
		CONST_INTEGER( MULT   , GL_MULT   )
		CONST_INTEGER( ADD    , GL_ADD    )

		CONST_INTEGER( NEVER    , GL_NEVER    )
		CONST_INTEGER( LESS     , GL_LESS     )
		CONST_INTEGER( EQUAL    , GL_EQUAL    )
		CONST_INTEGER( LEQUAL   , GL_LEQUAL   )
		CONST_INTEGER( GREATER  , GL_GREATER  )
		CONST_INTEGER( NOTEQUAL , GL_NOTEQUAL )
		CONST_INTEGER( GEQUAL   , GL_GEQUAL   )
		CONST_INTEGER( ALWAYS   , GL_ALWAYS   )

		CONST_INTEGER( CURRENT_BIT         , GL_CURRENT_BIT         )
		CONST_INTEGER( POINT_BIT           , GL_POINT_BIT           )
		CONST_INTEGER( LINE_BIT            , GL_LINE_BIT            )
		CONST_INTEGER( POLYGON_BIT         , GL_POLYGON_BIT         )
		CONST_INTEGER( POLYGON_STIPPLE_BIT , GL_POLYGON_STIPPLE_BIT )
		CONST_INTEGER( PIXEL_MODE_BIT      , GL_PIXEL_MODE_BIT      )
		CONST_INTEGER( LIGHTING_BIT        , GL_LIGHTING_BIT        )
		CONST_INTEGER( FOG_BIT             , GL_FOG_BIT             )
		CONST_INTEGER( DEPTH_BUFFER_BIT    , GL_DEPTH_BUFFER_BIT    )
		CONST_INTEGER( ACCUM_BUFFER_BIT    , GL_ACCUM_BUFFER_BIT    )
		CONST_INTEGER( STENCIL_BUFFER_BIT  , GL_STENCIL_BUFFER_BIT  )
		CONST_INTEGER( VIEWPORT_BIT        , GL_VIEWPORT_BIT        )
		CONST_INTEGER( TRANSFORM_BIT       , GL_TRANSFORM_BIT       )
		CONST_INTEGER( ENABLE_BIT          , GL_ENABLE_BIT          )
		CONST_INTEGER( COLOR_BUFFER_BIT    , GL_COLOR_BUFFER_BIT    )
		CONST_INTEGER( HINT_BIT            , GL_HINT_BIT            )
		CONST_INTEGER( EVAL_BIT            , GL_EVAL_BIT            )
		CONST_INTEGER( LIST_BIT            , GL_LIST_BIT            )
		CONST_INTEGER( TEXTURE_BIT         , GL_TEXTURE_BIT         )
		CONST_INTEGER( SCISSOR_BIT         , GL_SCISSOR_BIT         )
		CONST_INTEGER( ALL_ATTRIB_BITS     , GL_ALL_ATTRIB_BITS     )

		CONST_INTEGER( POINTS         , GL_POINTS         )
		CONST_INTEGER( LINES          , GL_LINES          )
		CONST_INTEGER( LINE_LOOP      , GL_LINE_LOOP      )
		CONST_INTEGER( LINE_STRIP     , GL_LINE_STRIP     )
		CONST_INTEGER( TRIANGLES      , GL_TRIANGLES      )
		CONST_INTEGER( TRIANGLE_STRIP , GL_TRIANGLE_STRIP )
		CONST_INTEGER( TRIANGLE_FAN   , GL_TRIANGLE_FAN   )
		CONST_INTEGER( QUADS          , GL_QUADS          )
		CONST_INTEGER( QUAD_STRIP     , GL_QUAD_STRIP     )
		CONST_INTEGER( POLYGON        , GL_POLYGON        )

		CONST_INTEGER( ZERO                , GL_ZERO                )
		CONST_INTEGER( ONE                 , GL_ONE                 )
		CONST_INTEGER( SRC_COLOR           , GL_SRC_COLOR           )
		CONST_INTEGER( ONE_MINUS_SRC_COLOR , GL_ONE_MINUS_SRC_COLOR )
		CONST_INTEGER( SRC_ALPHA           , GL_SRC_ALPHA           )
		CONST_INTEGER( ONE_MINUS_SRC_ALPHA , GL_ONE_MINUS_SRC_ALPHA )
		CONST_INTEGER( DST_ALPHA           , GL_DST_ALPHA           )
		CONST_INTEGER( ONE_MINUS_DST_ALPHA , GL_ONE_MINUS_DST_ALPHA )

		CONST_INTEGER( DST_COLOR           , GL_DST_COLOR           )
		CONST_INTEGER( ONE_MINUS_DST_COLOR , GL_ONE_MINUS_DST_COLOR )
		CONST_INTEGER( SRC_ALPHA_SATURATE  , GL_SRC_ALPHA_SATURATE  )

		CONST_INTEGER( TRUE  , GL_TRUE  )
		CONST_INTEGER( FALSE , GL_FALSE )

		CONST_INTEGER( CLIP_PLANE0 , GL_CLIP_PLANE0 )
		CONST_INTEGER( CLIP_PLANE1 , GL_CLIP_PLANE1 )
		CONST_INTEGER( CLIP_PLANE2 , GL_CLIP_PLANE2 )
		CONST_INTEGER( CLIP_PLANE3 , GL_CLIP_PLANE3 )
		CONST_INTEGER( CLIP_PLANE4 , GL_CLIP_PLANE4 )
		CONST_INTEGER( CLIP_PLANE5 , GL_CLIP_PLANE5 )

		CONST_INTEGER( BYTE           , GL_BYTE           )
		CONST_INTEGER( UNSIGNED_BYTE  , GL_UNSIGNED_BYTE  )
		CONST_INTEGER( SHORT          , GL_SHORT          )
		CONST_INTEGER( UNSIGNED_SHORT , GL_UNSIGNED_SHORT )
		CONST_INTEGER( INT            , GL_INT            )
		CONST_INTEGER( UNSIGNED_INT   , GL_UNSIGNED_INT   )
		CONST_INTEGER( FLOAT          , GL_FLOAT          )
		CONST_INTEGER( 2_BYTES        , GL_2_BYTES        )
		CONST_INTEGER( 3_BYTES        , GL_3_BYTES        )
		CONST_INTEGER( 4_BYTES        , GL_4_BYTES        )
		CONST_INTEGER( DOUBLE         , GL_DOUBLE         )

		CONST_INTEGER( NONE           , GL_NONE           )
		CONST_INTEGER( FRONT_LEFT     , GL_FRONT_LEFT     )
		CONST_INTEGER( FRONT_RIGHT    , GL_FRONT_RIGHT    )
		CONST_INTEGER( BACK_LEFT      , GL_BACK_LEFT      )
		CONST_INTEGER( BACK_RIGHT     , GL_BACK_RIGHT     )
		CONST_INTEGER( FRONT          , GL_FRONT          )
		CONST_INTEGER( BACK           , GL_BACK           )
		CONST_INTEGER( LEFT           , GL_LEFT           )
		CONST_INTEGER( RIGHT          , GL_RIGHT          )
		CONST_INTEGER( FRONT_AND_BACK , GL_FRONT_AND_BACK )
		CONST_INTEGER( AUX0           , GL_AUX0           )
		CONST_INTEGER( AUX1           , GL_AUX1           )
		CONST_INTEGER( AUX2           , GL_AUX2           )
		CONST_INTEGER( AUX3           , GL_AUX3           )

		CONST_INTEGER( NO_ERROR          , GL_NO_ERROR          )
		CONST_INTEGER( INVALID_ENUM      , GL_INVALID_ENUM      )
		CONST_INTEGER( INVALID_VALUE     , GL_INVALID_VALUE     )
		CONST_INTEGER( INVALID_OPERATION , GL_INVALID_OPERATION )
		CONST_INTEGER( STACK_OVERFLOW    , GL_STACK_OVERFLOW    )
		CONST_INTEGER( STACK_UNDERFLOW   , GL_STACK_UNDERFLOW   )
		CONST_INTEGER( OUT_OF_MEMORY     , GL_OUT_OF_MEMORY     )

		CONST_INTEGER( 2D               , GL_2D               )
		CONST_INTEGER( 3D               , GL_3D               )
		CONST_INTEGER( 3D_COLOR         , GL_3D_COLOR         )
		CONST_INTEGER( 3D_COLOR_TEXTURE , GL_3D_COLOR_TEXTURE )
		CONST_INTEGER( 4D_COLOR_TEXTURE , GL_4D_COLOR_TEXTURE )

		CONST_INTEGER( PASS_THROUGH_TOKEN , GL_PASS_THROUGH_TOKEN )
		CONST_INTEGER( POINT_TOKEN        , GL_POINT_TOKEN        )
		CONST_INTEGER( LINE_TOKEN         , GL_LINE_TOKEN         )
		CONST_INTEGER( POLYGON_TOKEN      , GL_POLYGON_TOKEN      )
		CONST_INTEGER( BITMAP_TOKEN       , GL_BITMAP_TOKEN       )
		CONST_INTEGER( DRAW_PIXEL_TOKEN   , GL_DRAW_PIXEL_TOKEN   )
		CONST_INTEGER( COPY_PIXEL_TOKEN   , GL_COPY_PIXEL_TOKEN   )
		CONST_INTEGER( LINE_RESET_TOKEN   , GL_LINE_RESET_TOKEN   )

		CONST_INTEGER( EXP  , GL_EXP  )
		CONST_INTEGER( EXP2 , GL_EXP2 )

		CONST_INTEGER( CW  , GL_CW  )
		CONST_INTEGER( CCW , GL_CCW )

		CONST_INTEGER( COEFF  , GL_COEFF  )
		CONST_INTEGER( ORDER  , GL_ORDER  )
		CONST_INTEGER( DOMAIN , GL_DOMAIN )

		CONST_INTEGER( CURRENT_COLOR                 , GL_CURRENT_COLOR                 )
		CONST_INTEGER( CURRENT_INDEX                 , GL_CURRENT_INDEX                 )
		CONST_INTEGER( CURRENT_NORMAL                , GL_CURRENT_NORMAL                )
		CONST_INTEGER( CURRENT_TEXTURE_COORDS        , GL_CURRENT_TEXTURE_COORDS        )
		CONST_INTEGER( CURRENT_RASTER_COLOR          , GL_CURRENT_RASTER_COLOR          )
		CONST_INTEGER( CURRENT_RASTER_INDEX          , GL_CURRENT_RASTER_INDEX          )
		CONST_INTEGER( CURRENT_RASTER_TEXTURE_COORDS , GL_CURRENT_RASTER_TEXTURE_COORDS )
		CONST_INTEGER( CURRENT_RASTER_POSITION       , GL_CURRENT_RASTER_POSITION       )
		CONST_INTEGER( CURRENT_RASTER_POSITION_VALID , GL_CURRENT_RASTER_POSITION_VALID )
		CONST_INTEGER( CURRENT_RASTER_DISTANCE       , GL_CURRENT_RASTER_DISTANCE       )
		CONST_INTEGER( POINT_SMOOTH                  , GL_POINT_SMOOTH                  )
		CONST_INTEGER( POINT_SIZE                    , GL_POINT_SIZE                    )
		CONST_INTEGER( POINT_SIZE_RANGE              , GL_POINT_SIZE_RANGE              )
		CONST_INTEGER( POINT_SIZE_GRANULARITY        , GL_POINT_SIZE_GRANULARITY        )
		CONST_INTEGER( LINE_SMOOTH                   , GL_LINE_SMOOTH                   )
		CONST_INTEGER( LINE_WIDTH                    , GL_LINE_WIDTH                    )
		CONST_INTEGER( LINE_WIDTH_RANGE              , GL_LINE_WIDTH_RANGE              )
		CONST_INTEGER( LINE_WIDTH_GRANULARITY        , GL_LINE_WIDTH_GRANULARITY        )
		CONST_INTEGER( LINE_STIPPLE                  , GL_LINE_STIPPLE                  )
		CONST_INTEGER( LINE_STIPPLE_PATTERN          , GL_LINE_STIPPLE_PATTERN          )
		CONST_INTEGER( LINE_STIPPLE_REPEAT           , GL_LINE_STIPPLE_REPEAT           )
		CONST_INTEGER( LIST_MODE                     , GL_LIST_MODE                     )
		CONST_INTEGER( MAX_LIST_NESTING              , GL_MAX_LIST_NESTING              )
		CONST_INTEGER( LIST_BASE                     , GL_LIST_BASE                     )
		CONST_INTEGER( LIST_INDEX                    , GL_LIST_INDEX                    )
		CONST_INTEGER( POLYGON_MODE                  , GL_POLYGON_MODE                  )
		CONST_INTEGER( POLYGON_SMOOTH                , GL_POLYGON_SMOOTH                )
		CONST_INTEGER( POLYGON_STIPPLE               , GL_POLYGON_STIPPLE               )
		CONST_INTEGER( EDGE_FLAG                     , GL_EDGE_FLAG                     )
		CONST_INTEGER( CULL_FACE                     , GL_CULL_FACE                     )
		CONST_INTEGER( CULL_FACE_MODE                , GL_CULL_FACE_MODE                )
		CONST_INTEGER( FRONT_FACE                    , GL_FRONT_FACE                    )
		CONST_INTEGER( LIGHTING                      , GL_LIGHTING                      )
		CONST_INTEGER( LIGHT_MODEL_LOCAL_VIEWER      , GL_LIGHT_MODEL_LOCAL_VIEWER      )
		CONST_INTEGER( LIGHT_MODEL_TWO_SIDE          , GL_LIGHT_MODEL_TWO_SIDE          )
		CONST_INTEGER( LIGHT_MODEL_AMBIENT           , GL_LIGHT_MODEL_AMBIENT           )
		CONST_INTEGER( SHADE_MODEL                   , GL_SHADE_MODEL                   )
		CONST_INTEGER( COLOR_MATERIAL_FACE           , GL_COLOR_MATERIAL_FACE           )
		CONST_INTEGER( COLOR_MATERIAL_PARAMETER      , GL_COLOR_MATERIAL_PARAMETER      )
		CONST_INTEGER( COLOR_MATERIAL                , GL_COLOR_MATERIAL                )
		CONST_INTEGER( FOG                           , GL_FOG                           )
		CONST_INTEGER( FOG_INDEX                     , GL_FOG_INDEX                     )
		CONST_INTEGER( FOG_DENSITY                   , GL_FOG_DENSITY                   )
		CONST_INTEGER( FOG_START                     , GL_FOG_START                     )
		CONST_INTEGER( FOG_END                       , GL_FOG_END                       )
		CONST_INTEGER( FOG_MODE                      , GL_FOG_MODE                      )
		CONST_INTEGER( FOG_COLOR                     , GL_FOG_COLOR                     )
		CONST_INTEGER( DEPTH_RANGE                   , GL_DEPTH_RANGE                   )
		CONST_INTEGER( DEPTH_TEST                    , GL_DEPTH_TEST                    )
		CONST_INTEGER( DEPTH_WRITEMASK               , GL_DEPTH_WRITEMASK               )
		CONST_INTEGER( DEPTH_CLEAR_VALUE             , GL_DEPTH_CLEAR_VALUE             )
		CONST_INTEGER( DEPTH_FUNC                    , GL_DEPTH_FUNC                    )
		CONST_INTEGER( ACCUM_CLEAR_VALUE             , GL_ACCUM_CLEAR_VALUE             )
		CONST_INTEGER( STENCIL_TEST                  , GL_STENCIL_TEST                  )
		CONST_INTEGER( STENCIL_CLEAR_VALUE           , GL_STENCIL_CLEAR_VALUE           )
		CONST_INTEGER( STENCIL_FUNC                , GL_STENCIL_FUNC                )
		CONST_INTEGER( STENCIL_VALUE_MASK          , GL_STENCIL_VALUE_MASK          )
		CONST_INTEGER( STENCIL_FAIL                , GL_STENCIL_FAIL                )
		CONST_INTEGER( STENCIL_PASS_DEPTH_FAIL     , GL_STENCIL_PASS_DEPTH_FAIL     )
		CONST_INTEGER( STENCIL_PASS_DEPTH_PASS     , GL_STENCIL_PASS_DEPTH_PASS     )
		CONST_INTEGER( STENCIL_REF                 , GL_STENCIL_REF                 )
		CONST_INTEGER( STENCIL_WRITEMASK           , GL_STENCIL_WRITEMASK           )
		CONST_INTEGER( MATRIX_MODE                 , GL_MATRIX_MODE                 )
		CONST_INTEGER( NORMALIZE                   , GL_NORMALIZE                   )
		CONST_INTEGER( VIEWPORT                    , GL_VIEWPORT                    )
		CONST_INTEGER( MODELVIEW_STACK_DEPTH       , GL_MODELVIEW_STACK_DEPTH       )
		CONST_INTEGER( PROJECTION_STACK_DEPTH      , GL_PROJECTION_STACK_DEPTH      )
		CONST_INTEGER( TEXTURE_STACK_DEPTH         , GL_TEXTURE_STACK_DEPTH         )
		CONST_INTEGER( MODELVIEW_MATRIX            , GL_MODELVIEW_MATRIX            )
		CONST_INTEGER( PROJECTION_MATRIX           , GL_PROJECTION_MATRIX           )
		CONST_INTEGER( TEXTURE_MATRIX              , GL_TEXTURE_MATRIX              )
		CONST_INTEGER( ATTRIB_STACK_DEPTH          , GL_ATTRIB_STACK_DEPTH          )
		CONST_INTEGER( CLIENT_ATTRIB_STACK_DEPTH   , GL_CLIENT_ATTRIB_STACK_DEPTH   )
		CONST_INTEGER( ALPHA_TEST                  , GL_ALPHA_TEST                  )
		CONST_INTEGER( ALPHA_TEST_FUNC             , GL_ALPHA_TEST_FUNC             )
		CONST_INTEGER( ALPHA_TEST_REF              , GL_ALPHA_TEST_REF              )
		CONST_INTEGER( DITHER                      , GL_DITHER                      )
		CONST_INTEGER( BLEND_DST                   , GL_BLEND_DST                   )
		CONST_INTEGER( BLEND_SRC                   , GL_BLEND_SRC                   )
		CONST_INTEGER( BLEND                       , GL_BLEND                       )
		CONST_INTEGER( LOGIC_OP_MODE               , GL_LOGIC_OP_MODE               )
		CONST_INTEGER( INDEX_LOGIC_OP              , GL_INDEX_LOGIC_OP              )
		CONST_INTEGER( COLOR_LOGIC_OP              , GL_COLOR_LOGIC_OP              )
		CONST_INTEGER( AUX_BUFFERS                 , GL_AUX_BUFFERS                 )
		CONST_INTEGER( DRAW_BUFFER                 , GL_DRAW_BUFFER                 )
		CONST_INTEGER( READ_BUFFER                 , GL_READ_BUFFER                 )
		CONST_INTEGER( SCISSOR_BOX                 , GL_SCISSOR_BOX                 )
		CONST_INTEGER( SCISSOR_TEST                , GL_SCISSOR_TEST                )
		CONST_INTEGER( INDEX_CLEAR_VALUE           , GL_INDEX_CLEAR_VALUE           )
		CONST_INTEGER( INDEX_WRITEMASK             , GL_INDEX_WRITEMASK             )
		CONST_INTEGER( COLOR_CLEAR_VALUE           , GL_COLOR_CLEAR_VALUE           )
		CONST_INTEGER( COLOR_WRITEMASK             , GL_COLOR_WRITEMASK             )
		CONST_INTEGER( INDEX_MODE                  , GL_INDEX_MODE                  )
		CONST_INTEGER( RGBA_MODE                   , GL_RGBA_MODE                   )
		CONST_INTEGER( DOUBLEBUFFER                , GL_DOUBLEBUFFER                )
		CONST_INTEGER( STEREO                      , GL_STEREO                      )
		CONST_INTEGER( RENDER_MODE                 , GL_RENDER_MODE                 )
		CONST_INTEGER( PERSPECTIVE_CORRECTION_HINT , GL_PERSPECTIVE_CORRECTION_HINT )
		CONST_INTEGER( POINT_SMOOTH_HINT           , GL_POINT_SMOOTH_HINT           )
		CONST_INTEGER( LINE_SMOOTH_HINT            , GL_LINE_SMOOTH_HINT            )
		CONST_INTEGER( POLYGON_SMOOTH_HINT         , GL_POLYGON_SMOOTH_HINT         )
		CONST_INTEGER( FOG_HINT                    , GL_FOG_HINT                    )
		CONST_INTEGER( TEXTURE_GEN_S               , GL_TEXTURE_GEN_S               )
		CONST_INTEGER( TEXTURE_GEN_T               , GL_TEXTURE_GEN_T               )
		CONST_INTEGER( TEXTURE_GEN_R               , GL_TEXTURE_GEN_R               )
		CONST_INTEGER( TEXTURE_GEN_Q               , GL_TEXTURE_GEN_Q               )
		CONST_INTEGER( PIXEL_MAP_I_TO_I            , GL_PIXEL_MAP_I_TO_I            )
		CONST_INTEGER( PIXEL_MAP_S_TO_S            , GL_PIXEL_MAP_S_TO_S            )
		CONST_INTEGER( PIXEL_MAP_I_TO_R            , GL_PIXEL_MAP_I_TO_R            )
		CONST_INTEGER( PIXEL_MAP_I_TO_G            , GL_PIXEL_MAP_I_TO_G            )
		CONST_INTEGER( PIXEL_MAP_I_TO_B            , GL_PIXEL_MAP_I_TO_B            )
		CONST_INTEGER( PIXEL_MAP_I_TO_A            , GL_PIXEL_MAP_I_TO_A            )
		CONST_INTEGER( PIXEL_MAP_R_TO_R            , GL_PIXEL_MAP_R_TO_R            )
		CONST_INTEGER( PIXEL_MAP_G_TO_G              , GL_PIXEL_MAP_G_TO_G              )
		CONST_INTEGER( PIXEL_MAP_B_TO_B              , GL_PIXEL_MAP_B_TO_B              )
		CONST_INTEGER( PIXEL_MAP_A_TO_A              , GL_PIXEL_MAP_A_TO_A              )
		CONST_INTEGER( PIXEL_MAP_I_TO_I_SIZE         , GL_PIXEL_MAP_I_TO_I_SIZE         )
		CONST_INTEGER( PIXEL_MAP_S_TO_S_SIZE         , GL_PIXEL_MAP_S_TO_S_SIZE         )
		CONST_INTEGER( PIXEL_MAP_I_TO_R_SIZE         , GL_PIXEL_MAP_I_TO_R_SIZE         )
		CONST_INTEGER( PIXEL_MAP_I_TO_G_SIZE         , GL_PIXEL_MAP_I_TO_G_SIZE         )
		CONST_INTEGER( PIXEL_MAP_I_TO_B_SIZE         , GL_PIXEL_MAP_I_TO_B_SIZE         )
		CONST_INTEGER( PIXEL_MAP_I_TO_A_SIZE         , GL_PIXEL_MAP_I_TO_A_SIZE         )
		CONST_INTEGER( PIXEL_MAP_R_TO_R_SIZE         , GL_PIXEL_MAP_R_TO_R_SIZE         )
		CONST_INTEGER( PIXEL_MAP_G_TO_G_SIZE         , GL_PIXEL_MAP_G_TO_G_SIZE         )
		CONST_INTEGER( PIXEL_MAP_B_TO_B_SIZE         , GL_PIXEL_MAP_B_TO_B_SIZE         )
		CONST_INTEGER( PIXEL_MAP_A_TO_A_SIZE         , GL_PIXEL_MAP_A_TO_A_SIZE         )
		CONST_INTEGER( UNPACK_SWAP_BYTES             , GL_UNPACK_SWAP_BYTES             )
		CONST_INTEGER( UNPACK_LSB_FIRST              , GL_UNPACK_LSB_FIRST              )
		CONST_INTEGER( UNPACK_ROW_LENGTH             , GL_UNPACK_ROW_LENGTH             )
		CONST_INTEGER( UNPACK_SKIP_ROWS              , GL_UNPACK_SKIP_ROWS              )
		CONST_INTEGER( UNPACK_SKIP_PIXELS            , GL_UNPACK_SKIP_PIXELS            )
		CONST_INTEGER( UNPACK_ALIGNMENT              , GL_UNPACK_ALIGNMENT              )
		CONST_INTEGER( PACK_SWAP_BYTES               , GL_PACK_SWAP_BYTES               )
		CONST_INTEGER( PACK_LSB_FIRST                , GL_PACK_LSB_FIRST                )
		CONST_INTEGER( PACK_ROW_LENGTH               , GL_PACK_ROW_LENGTH               )
		CONST_INTEGER( PACK_SKIP_ROWS                , GL_PACK_SKIP_ROWS                )
		CONST_INTEGER( PACK_SKIP_PIXELS              , GL_PACK_SKIP_PIXELS              )
		CONST_INTEGER( PACK_ALIGNMENT                , GL_PACK_ALIGNMENT                )
		CONST_INTEGER( MAP_COLOR                     , GL_MAP_COLOR                     )
		CONST_INTEGER( MAP_STENCIL                   , GL_MAP_STENCIL                   )
		CONST_INTEGER( INDEX_SHIFT                   , GL_INDEX_SHIFT                   )
		CONST_INTEGER( INDEX_OFFSET                  , GL_INDEX_OFFSET                  )
		CONST_INTEGER( RED_SCALE                     , GL_RED_SCALE                     )
		CONST_INTEGER( RED_BIAS                      , GL_RED_BIAS                      )
		CONST_INTEGER( ZOOM_X                        , GL_ZOOM_X                        )
		CONST_INTEGER( ZOOM_Y                        , GL_ZOOM_Y                        )
		CONST_INTEGER( GREEN_SCALE                   , GL_GREEN_SCALE                   )
		CONST_INTEGER( GREEN_BIAS                    , GL_GREEN_BIAS                    )
		CONST_INTEGER( BLUE_SCALE                    , GL_BLUE_SCALE                    )
		CONST_INTEGER( BLUE_BIAS                     , GL_BLUE_BIAS                     )
		CONST_INTEGER( ALPHA_SCALE                   , GL_ALPHA_SCALE                   )
		CONST_INTEGER( ALPHA_BIAS                    , GL_ALPHA_BIAS                    )
		CONST_INTEGER( DEPTH_SCALE                   , GL_DEPTH_SCALE                   )
		CONST_INTEGER( DEPTH_BIAS                    , GL_DEPTH_BIAS                    )
		CONST_INTEGER( MAX_EVAL_ORDER                , GL_MAX_EVAL_ORDER                )
		CONST_INTEGER( MAX_LIGHTS                    , GL_MAX_LIGHTS                    )
		CONST_INTEGER( MAX_CLIP_PLANES               , GL_MAX_CLIP_PLANES               )
		CONST_INTEGER( MAX_TEXTURE_SIZE              , GL_MAX_TEXTURE_SIZE              )
		CONST_INTEGER( MAX_PIXEL_MAP_TABLE           , GL_MAX_PIXEL_MAP_TABLE           )
		CONST_INTEGER( MAX_ATTRIB_STACK_DEPTH        , GL_MAX_ATTRIB_STACK_DEPTH        )
		CONST_INTEGER( MAX_MODELVIEW_STACK_DEPTH     , GL_MAX_MODELVIEW_STACK_DEPTH     )
		CONST_INTEGER( MAX_NAME_STACK_DEPTH          , GL_MAX_NAME_STACK_DEPTH          )
		CONST_INTEGER( MAX_PROJECTION_STACK_DEPTH    , GL_MAX_PROJECTION_STACK_DEPTH    )
		CONST_INTEGER( MAX_TEXTURE_STACK_DEPTH       , GL_MAX_TEXTURE_STACK_DEPTH       )
		CONST_INTEGER( MAX_VIEWPORT_DIMS             , GL_MAX_VIEWPORT_DIMS             )
		CONST_INTEGER( MAX_CLIENT_ATTRIB_STACK_DEPTH , GL_MAX_CLIENT_ATTRIB_STACK_DEPTH )
		CONST_INTEGER( SUBPIXEL_BITS                 , GL_SUBPIXEL_BITS                 )
		CONST_INTEGER( INDEX_BITS                    , GL_INDEX_BITS                    )
		CONST_INTEGER( RED_BITS                      , GL_RED_BITS                      )
		CONST_INTEGER( GREEN_BITS               , GL_GREEN_BITS               )
		CONST_INTEGER( BLUE_BITS                , GL_BLUE_BITS                )
		CONST_INTEGER( ALPHA_BITS               , GL_ALPHA_BITS               )
		CONST_INTEGER( DEPTH_BITS               , GL_DEPTH_BITS               )
		CONST_INTEGER( STENCIL_BITS             , GL_STENCIL_BITS             )
		CONST_INTEGER( ACCUM_RED_BITS           , GL_ACCUM_RED_BITS           )
		CONST_INTEGER( ACCUM_GREEN_BITS         , GL_ACCUM_GREEN_BITS         )
		CONST_INTEGER( ACCUM_BLUE_BITS          , GL_ACCUM_BLUE_BITS          )
		CONST_INTEGER( ACCUM_ALPHA_BITS         , GL_ACCUM_ALPHA_BITS         )
		CONST_INTEGER( NAME_STACK_DEPTH         , GL_NAME_STACK_DEPTH         )
		CONST_INTEGER( AUTO_NORMAL              , GL_AUTO_NORMAL              )
		CONST_INTEGER( MAP1_COLOR_4             , GL_MAP1_COLOR_4             )
		CONST_INTEGER( MAP1_INDEX               , GL_MAP1_INDEX               )
		CONST_INTEGER( MAP1_NORMAL              , GL_MAP1_NORMAL              )
		CONST_INTEGER( MAP1_TEXTURE_COORD_1     , GL_MAP1_TEXTURE_COORD_1     )
		CONST_INTEGER( MAP1_TEXTURE_COORD_2     , GL_MAP1_TEXTURE_COORD_2     )
		CONST_INTEGER( MAP1_TEXTURE_COORD_3     , GL_MAP1_TEXTURE_COORD_3     )
		CONST_INTEGER( MAP1_TEXTURE_COORD_4     , GL_MAP1_TEXTURE_COORD_4     )
		CONST_INTEGER( MAP1_VERTEX_3            , GL_MAP1_VERTEX_3            )
		CONST_INTEGER( MAP1_VERTEX_4            , GL_MAP1_VERTEX_4            )
		CONST_INTEGER( MAP2_COLOR_4             , GL_MAP2_COLOR_4             )
		CONST_INTEGER( MAP2_INDEX               , GL_MAP2_INDEX               )
		CONST_INTEGER( MAP2_NORMAL              , GL_MAP2_NORMAL              )
		CONST_INTEGER( MAP2_TEXTURE_COORD_1     , GL_MAP2_TEXTURE_COORD_1     )
		CONST_INTEGER( MAP2_TEXTURE_COORD_2     , GL_MAP2_TEXTURE_COORD_2     )
		CONST_INTEGER( MAP2_TEXTURE_COORD_3     , GL_MAP2_TEXTURE_COORD_3     )
		CONST_INTEGER( MAP2_TEXTURE_COORD_4     , GL_MAP2_TEXTURE_COORD_4     )
		CONST_INTEGER( MAP2_VERTEX_3            , GL_MAP2_VERTEX_3            )
		CONST_INTEGER( MAP2_VERTEX_4            , GL_MAP2_VERTEX_4            )
		CONST_INTEGER( MAP1_GRID_DOMAIN         , GL_MAP1_GRID_DOMAIN         )
		CONST_INTEGER( MAP1_GRID_SEGMENTS       , GL_MAP1_GRID_SEGMENTS       )
		CONST_INTEGER( MAP2_GRID_DOMAIN         , GL_MAP2_GRID_DOMAIN         )
		CONST_INTEGER( MAP2_GRID_SEGMENTS       , GL_MAP2_GRID_SEGMENTS       )
		CONST_INTEGER( TEXTURE_1D               , GL_TEXTURE_1D               )
		CONST_INTEGER( TEXTURE_2D               , GL_TEXTURE_2D               )
		CONST_INTEGER( FEEDBACK_BUFFER_POINTER  , GL_FEEDBACK_BUFFER_POINTER  )
		CONST_INTEGER( FEEDBACK_BUFFER_SIZE     , GL_FEEDBACK_BUFFER_SIZE     )
		CONST_INTEGER( FEEDBACK_BUFFER_TYPE     , GL_FEEDBACK_BUFFER_TYPE     )
		CONST_INTEGER( SELECTION_BUFFER_POINTER , GL_SELECTION_BUFFER_POINTER )
		CONST_INTEGER( SELECTION_BUFFER_SIZE    , GL_SELECTION_BUFFER_SIZE    )

		CONST_INTEGER( TEXTURE_WIDTH           , GL_TEXTURE_WIDTH           )
		CONST_INTEGER( TEXTURE_HEIGHT          , GL_TEXTURE_HEIGHT          )
		CONST_INTEGER( TEXTURE_INTERNAL_FORMAT , GL_TEXTURE_INTERNAL_FORMAT )
		CONST_INTEGER( TEXTURE_BORDER_COLOR    , GL_TEXTURE_BORDER_COLOR    )
		CONST_INTEGER( TEXTURE_BORDER          , GL_TEXTURE_BORDER          )

		CONST_INTEGER( DONT_CARE , GL_DONT_CARE )
		CONST_INTEGER( FASTEST   , GL_FASTEST   )
		CONST_INTEGER( NICEST    , GL_NICEST    )

		CONST_INTEGER( LIGHT0 , GL_LIGHT0 )
		CONST_INTEGER( LIGHT1 , GL_LIGHT1 )
		CONST_INTEGER( LIGHT2 , GL_LIGHT2 )
		CONST_INTEGER( LIGHT3 , GL_LIGHT3 )
		CONST_INTEGER( LIGHT4 , GL_LIGHT4 )
		CONST_INTEGER( LIGHT5 , GL_LIGHT5 )
		CONST_INTEGER( LIGHT6 , GL_LIGHT6 )
		CONST_INTEGER( LIGHT7 , GL_LIGHT7 )

		CONST_INTEGER( AMBIENT               , GL_AMBIENT               )
		CONST_INTEGER( DIFFUSE               , GL_DIFFUSE               )
		CONST_INTEGER( SPECULAR              , GL_SPECULAR              )
		CONST_INTEGER( POSITION              , GL_POSITION              )
		CONST_INTEGER( SPOT_DIRECTION        , GL_SPOT_DIRECTION        )
		CONST_INTEGER( SPOT_EXPONENT         , GL_SPOT_EXPONENT         )
		CONST_INTEGER( SPOT_CUTOFF           , GL_SPOT_CUTOFF           )
		CONST_INTEGER( CONSTANT_ATTENUATION  , GL_CONSTANT_ATTENUATION  )
		CONST_INTEGER( LINEAR_ATTENUATION    , GL_LINEAR_ATTENUATION    )
		CONST_INTEGER( QUADRATIC_ATTENUATION , GL_QUADRATIC_ATTENUATION )

		CONST_INTEGER( COMPILE             , GL_COMPILE             )
		CONST_INTEGER( COMPILE_AND_EXECUTE , GL_COMPILE_AND_EXECUTE )

		CONST_INTEGER( CLEAR         , GL_CLEAR         )
		CONST_INTEGER( AND           , GL_AND           )
		CONST_INTEGER( AND_REVERSE   , GL_AND_REVERSE   )
		CONST_INTEGER( COPY          , GL_COPY          )
		CONST_INTEGER( AND_INVERTED  , GL_AND_INVERTED  )
		CONST_INTEGER( NOOP          , GL_NOOP          )
		CONST_INTEGER( XOR           , GL_XOR           )
		CONST_INTEGER( OR            , GL_OR            )
		CONST_INTEGER( NOR           , GL_NOR           )
		CONST_INTEGER( EQUIV         , GL_EQUIV         )
		CONST_INTEGER( INVERT        , GL_INVERT        )
		CONST_INTEGER( OR_REVERSE    , GL_OR_REVERSE    )
		CONST_INTEGER( COPY_INVERTED , GL_COPY_INVERTED )
		CONST_INTEGER( OR_INVERTED   , GL_OR_INVERTED   )
		CONST_INTEGER( NAND          , GL_NAND          )
		CONST_INTEGER( SET           , GL_SET           )

		CONST_INTEGER( EMISSION            , GL_EMISSION            )
		CONST_INTEGER( SHININESS           , GL_SHININESS           )
		CONST_INTEGER( AMBIENT_AND_DIFFUSE , GL_AMBIENT_AND_DIFFUSE )
		CONST_INTEGER( COLOR_INDEXES       , GL_COLOR_INDEXES       )

		CONST_INTEGER( MODELVIEW  , GL_MODELVIEW  )
		CONST_INTEGER( PROJECTION , GL_PROJECTION )
		CONST_INTEGER( TEXTURE    , GL_TEXTURE    )

		CONST_INTEGER( COLOR   , GL_COLOR   )
		CONST_INTEGER( DEPTH   , GL_DEPTH   )
		CONST_INTEGER( STENCIL , GL_STENCIL )

		CONST_INTEGER( COLOR_INDEX     , GL_COLOR_INDEX     )
		CONST_INTEGER( STENCIL_INDEX   , GL_STENCIL_INDEX   )
		CONST_INTEGER( DEPTH_COMPONENT , GL_DEPTH_COMPONENT )
		CONST_INTEGER( RED             , GL_RED             )
		CONST_INTEGER( GREEN           , GL_GREEN           )
		CONST_INTEGER( BLUE            , GL_BLUE            )
		CONST_INTEGER( ALPHA           , GL_ALPHA           )
		CONST_INTEGER( RGB             , GL_RGB             )
		CONST_INTEGER( RGBA            , GL_RGBA            )
		CONST_INTEGER( LUMINANCE       , GL_LUMINANCE       )
		CONST_INTEGER( LUMINANCE_ALPHA , GL_LUMINANCE_ALPHA )

		CONST_INTEGER( BITMAP , GL_BITMAP )

		CONST_INTEGER( POINT , GL_POINT )
		CONST_INTEGER( LINE  , GL_LINE  )
		CONST_INTEGER( FILL  , GL_FILL  )

		CONST_INTEGER( RENDER   , GL_RENDER   )
		CONST_INTEGER( FEEDBACK , GL_FEEDBACK )
		CONST_INTEGER( SELECT   , GL_SELECT   )

		CONST_INTEGER( FLAT   , GL_FLAT   )
		CONST_INTEGER( SMOOTH , GL_SMOOTH )

		CONST_INTEGER( KEEP    , GL_KEEP    )
		CONST_INTEGER( REPLACE , GL_REPLACE )
		CONST_INTEGER( INCR    , GL_INCR    )
		CONST_INTEGER( DECR    , GL_DECR    )

		CONST_INTEGER( VENDOR     , GL_VENDOR     )
		CONST_INTEGER( RENDERER   , GL_RENDERER   )
		CONST_INTEGER( VERSION    , GL_VERSION    )
		CONST_INTEGER( EXTENSIONS , GL_EXTENSIONS )

		CONST_INTEGER( S , GL_S )
		CONST_INTEGER( T , GL_T )
		CONST_INTEGER( R , GL_R )
		CONST_INTEGER( Q , GL_Q )

		CONST_INTEGER( MODULATE , GL_MODULATE )
		CONST_INTEGER( DECAL    , GL_DECAL    )

		CONST_INTEGER( TEXTURE_ENV_MODE  , GL_TEXTURE_ENV_MODE  )
		CONST_INTEGER( TEXTURE_ENV_COLOR , GL_TEXTURE_ENV_COLOR )

		CONST_INTEGER( TEXTURE_ENV , GL_TEXTURE_ENV )

		CONST_INTEGER( EYE_LINEAR    , GL_EYE_LINEAR    )
		CONST_INTEGER( OBJECT_LINEAR , GL_OBJECT_LINEAR )
		CONST_INTEGER( SPHERE_MAP    , GL_SPHERE_MAP    )

		CONST_INTEGER( TEXTURE_GEN_MODE , GL_TEXTURE_GEN_MODE )
		CONST_INTEGER( OBJECT_PLANE     , GL_OBJECT_PLANE     )
		CONST_INTEGER( EYE_PLANE        , GL_EYE_PLANE        )

		CONST_INTEGER( NEAREST , GL_NEAREST )
		CONST_INTEGER( LINEAR  , GL_LINEAR  )

		CONST_INTEGER( NEAREST_MIPMAP_NEAREST , GL_NEAREST_MIPMAP_NEAREST )
		CONST_INTEGER( LINEAR_MIPMAP_NEAREST  , GL_LINEAR_MIPMAP_NEAREST  )
		CONST_INTEGER( NEAREST_MIPMAP_LINEAR  , GL_NEAREST_MIPMAP_LINEAR  )
		CONST_INTEGER( LINEAR_MIPMAP_LINEAR   , GL_LINEAR_MIPMAP_LINEAR   )

		CONST_INTEGER( TEXTURE_MAG_FILTER , GL_TEXTURE_MAG_FILTER )
		CONST_INTEGER( TEXTURE_MIN_FILTER , GL_TEXTURE_MIN_FILTER )
		CONST_INTEGER( TEXTURE_WRAP_S     , GL_TEXTURE_WRAP_S     )
		CONST_INTEGER( TEXTURE_WRAP_T     , GL_TEXTURE_WRAP_T     )

		CONST_INTEGER( CLAMP  , GL_CLAMP  )
		CONST_INTEGER( REPEAT , GL_REPEAT )

		CONST_INTEGER( CLIENT_PIXEL_STORE_BIT  , GL_CLIENT_PIXEL_STORE_BIT  )
		CONST_INTEGER( CLIENT_VERTEX_ARRAY_BIT , GL_CLIENT_VERTEX_ARRAY_BIT )
//		CONST_INTEGER( CLIENT_ALL_ATTRIB_BITS  , GL_CLIENT_ALL_ATTRIB_BITS  )

		CONST_INTEGER( POLYGON_OFFSET_FACTOR , GL_POLYGON_OFFSET_FACTOR )
		CONST_INTEGER( POLYGON_OFFSET_UNITS  , GL_POLYGON_OFFSET_UNITS  )
		CONST_INTEGER( POLYGON_OFFSET_POINT  , GL_POLYGON_OFFSET_POINT  )
		CONST_INTEGER( POLYGON_OFFSET_LINE   , GL_POLYGON_OFFSET_LINE   )
		CONST_INTEGER( POLYGON_OFFSET_FILL   , GL_POLYGON_OFFSET_FILL   )

		CONST_INTEGER( ALPHA4                 , GL_ALPHA4                 )
		CONST_INTEGER( ALPHA8                 , GL_ALPHA8                 )
		CONST_INTEGER( ALPHA12                , GL_ALPHA12                )
		CONST_INTEGER( ALPHA16                , GL_ALPHA16                )
		CONST_INTEGER( LUMINANCE4             , GL_LUMINANCE4             )
		CONST_INTEGER( LUMINANCE8             , GL_LUMINANCE8             )
		CONST_INTEGER( LUMINANCE12            , GL_LUMINANCE12            )
		CONST_INTEGER( LUMINANCE16            , GL_LUMINANCE16            )
		CONST_INTEGER( LUMINANCE4_ALPHA4      , GL_LUMINANCE4_ALPHA4      )
		CONST_INTEGER( LUMINANCE6_ALPHA2      , GL_LUMINANCE6_ALPHA2      )
		CONST_INTEGER( LUMINANCE8_ALPHA8      , GL_LUMINANCE8_ALPHA8      )
		CONST_INTEGER( LUMINANCE12_ALPHA4     , GL_LUMINANCE12_ALPHA4     )
		CONST_INTEGER( LUMINANCE12_ALPHA12    , GL_LUMINANCE12_ALPHA12    )
		CONST_INTEGER( LUMINANCE16_ALPHA16    , GL_LUMINANCE16_ALPHA16    )
		CONST_INTEGER( INTENSITY              , GL_INTENSITY              )
		CONST_INTEGER( INTENSITY4             , GL_INTENSITY4             )
		CONST_INTEGER( INTENSITY8             , GL_INTENSITY8             )
		CONST_INTEGER( INTENSITY12            , GL_INTENSITY12            )
		CONST_INTEGER( INTENSITY16            , GL_INTENSITY16            )
		CONST_INTEGER( R3_G3_B2               , GL_R3_G3_B2               )
		CONST_INTEGER( RGB4                   , GL_RGB4                   )
		CONST_INTEGER( RGB5                   , GL_RGB5                   )
		CONST_INTEGER( RGB8                   , GL_RGB8                   )
		CONST_INTEGER( RGB10                  , GL_RGB10                  )
		CONST_INTEGER( RGB12                  , GL_RGB12                  )
		CONST_INTEGER( RGB16                  , GL_RGB16                  )
		CONST_INTEGER( RGBA2                  , GL_RGBA2                  )
		CONST_INTEGER( RGBA4                  , GL_RGBA4                  )
		CONST_INTEGER( RGB5_A1                , GL_RGB5_A1                )
		CONST_INTEGER( RGBA8                  , GL_RGBA8                  )
		CONST_INTEGER( RGB10_A2               , GL_RGB10_A2               )
		CONST_INTEGER( RGBA12                 , GL_RGBA12                 )
		CONST_INTEGER( RGBA16                 , GL_RGBA16                 )
		CONST_INTEGER( TEXTURE_RED_SIZE       , GL_TEXTURE_RED_SIZE       )
		CONST_INTEGER( TEXTURE_GREEN_SIZE     , GL_TEXTURE_GREEN_SIZE     )
		CONST_INTEGER( TEXTURE_BLUE_SIZE      , GL_TEXTURE_BLUE_SIZE      )
		CONST_INTEGER( TEXTURE_ALPHA_SIZE     , GL_TEXTURE_ALPHA_SIZE     )
		CONST_INTEGER( TEXTURE_LUMINANCE_SIZE , GL_TEXTURE_LUMINANCE_SIZE )
		CONST_INTEGER( TEXTURE_INTENSITY_SIZE , GL_TEXTURE_INTENSITY_SIZE )
		CONST_INTEGER( PROXY_TEXTURE_1D       , GL_PROXY_TEXTURE_1D       )
		CONST_INTEGER( PROXY_TEXTURE_2D       , GL_PROXY_TEXTURE_2D       )

		CONST_INTEGER( TEXTURE_PRIORITY   , GL_TEXTURE_PRIORITY   )
		CONST_INTEGER( TEXTURE_RESIDENT   , GL_TEXTURE_RESIDENT   )
		CONST_INTEGER( TEXTURE_BINDING_1D , GL_TEXTURE_BINDING_1D )
		CONST_INTEGER( TEXTURE_BINDING_2D , GL_TEXTURE_BINDING_2D )

		CONST_INTEGER( VERTEX_ARRAY                , GL_VERTEX_ARRAY                )
		CONST_INTEGER( NORMAL_ARRAY                , GL_NORMAL_ARRAY                )
		CONST_INTEGER( COLOR_ARRAY                 , GL_COLOR_ARRAY                 )
		CONST_INTEGER( INDEX_ARRAY                 , GL_INDEX_ARRAY                 )
		CONST_INTEGER( TEXTURE_COORD_ARRAY         , GL_TEXTURE_COORD_ARRAY         )
		CONST_INTEGER( EDGE_FLAG_ARRAY             , GL_EDGE_FLAG_ARRAY             )
		CONST_INTEGER( VERTEX_ARRAY_SIZE           , GL_VERTEX_ARRAY_SIZE           )
		CONST_INTEGER( VERTEX_ARRAY_TYPE           , GL_VERTEX_ARRAY_TYPE           )
		CONST_INTEGER( VERTEX_ARRAY_STRIDE         , GL_VERTEX_ARRAY_STRIDE         )
		CONST_INTEGER( NORMAL_ARRAY_TYPE           , GL_NORMAL_ARRAY_TYPE           )
		CONST_INTEGER( NORMAL_ARRAY_STRIDE         , GL_NORMAL_ARRAY_STRIDE         )
		CONST_INTEGER( COLOR_ARRAY_SIZE            , GL_COLOR_ARRAY_SIZE            )
		CONST_INTEGER( COLOR_ARRAY_TYPE            , GL_COLOR_ARRAY_TYPE            )
		CONST_INTEGER( COLOR_ARRAY_STRIDE          , GL_COLOR_ARRAY_STRIDE          )
		CONST_INTEGER( INDEX_ARRAY_TYPE            , GL_INDEX_ARRAY_TYPE            )
		CONST_INTEGER( INDEX_ARRAY_STRIDE          , GL_INDEX_ARRAY_STRIDE          )
		CONST_INTEGER( TEXTURE_COORD_ARRAY_SIZE    , GL_TEXTURE_COORD_ARRAY_SIZE    )
		CONST_INTEGER( TEXTURE_COORD_ARRAY_TYPE    , GL_TEXTURE_COORD_ARRAY_TYPE    )
		CONST_INTEGER( TEXTURE_COORD_ARRAY_STRIDE  , GL_TEXTURE_COORD_ARRAY_STRIDE  )
		CONST_INTEGER( EDGE_FLAG_ARRAY_STRIDE      , GL_EDGE_FLAG_ARRAY_STRIDE      )
		CONST_INTEGER( VERTEX_ARRAY_POINTER        , GL_VERTEX_ARRAY_POINTER        )
		CONST_INTEGER( NORMAL_ARRAY_POINTER        , GL_NORMAL_ARRAY_POINTER        )
		CONST_INTEGER( COLOR_ARRAY_POINTER         , GL_COLOR_ARRAY_POINTER         )
		CONST_INTEGER( INDEX_ARRAY_POINTER         , GL_INDEX_ARRAY_POINTER         )
		CONST_INTEGER( TEXTURE_COORD_ARRAY_POINTER , GL_TEXTURE_COORD_ARRAY_POINTER )
		CONST_INTEGER( EDGE_FLAG_ARRAY_POINTER     , GL_EDGE_FLAG_ARRAY_POINTER     )
		CONST_INTEGER( V2F                         , GL_V2F                         )
		CONST_INTEGER( V3F                         , GL_V3F                         )
		CONST_INTEGER( C4UB_V2F                    , GL_C4UB_V2F                    )
		CONST_INTEGER( C4UB_V3F                    , GL_C4UB_V3F                    )
		CONST_INTEGER( C3F_V3F                     , GL_C3F_V3F                     )
		CONST_INTEGER( N3F_V3F                     , GL_N3F_V3F                     )
		CONST_INTEGER( C4F_N3F_V3F                 , GL_C4F_N3F_V3F                 )
		CONST_INTEGER( T2F_V3F                     , GL_T2F_V3F                     )
		CONST_INTEGER( T4F_V4F                     , GL_T4F_V4F                     )
		CONST_INTEGER( T2F_C4UB_V3F                , GL_T2F_C4UB_V3F                )
		CONST_INTEGER( T2F_C3F_V3F                 , GL_T2F_C3F_V3F                 )
		CONST_INTEGER( T2F_N3F_V3F                 , GL_T2F_N3F_V3F                 )
		CONST_INTEGER( T2F_C4F_N3F_V3F             , GL_T2F_C4F_N3F_V3F             )
		CONST_INTEGER( T4F_C4F_N3F_V4F             , GL_T4F_C4F_N3F_V4F             )

		CONST_INTEGER( EXT_vertex_array        , GL_EXT_vertex_array        )
		CONST_INTEGER( EXT_bgra                , GL_EXT_bgra                )
		CONST_INTEGER( EXT_paletted_texture    , GL_EXT_paletted_texture    )
		CONST_INTEGER( WIN_swap_hint           , GL_WIN_swap_hint           )
		CONST_INTEGER( WIN_draw_range_elements , GL_WIN_draw_range_elements )

		CONST_INTEGER( VERTEX_ARRAY_EXT                , GL_VERTEX_ARRAY_EXT                )
		CONST_INTEGER( NORMAL_ARRAY_EXT                , GL_NORMAL_ARRAY_EXT                )
		CONST_INTEGER( COLOR_ARRAY_EXT                 , GL_COLOR_ARRAY_EXT                 )
		CONST_INTEGER( INDEX_ARRAY_EXT                 , GL_INDEX_ARRAY_EXT                 )
		CONST_INTEGER( TEXTURE_COORD_ARRAY_EXT         , GL_TEXTURE_COORD_ARRAY_EXT         )
		CONST_INTEGER( EDGE_FLAG_ARRAY_EXT             , GL_EDGE_FLAG_ARRAY_EXT             )
		CONST_INTEGER( VERTEX_ARRAY_SIZE_EXT           , GL_VERTEX_ARRAY_SIZE_EXT           )
		CONST_INTEGER( VERTEX_ARRAY_TYPE_EXT           , GL_VERTEX_ARRAY_TYPE_EXT           )
		CONST_INTEGER( VERTEX_ARRAY_STRIDE_EXT         , GL_VERTEX_ARRAY_STRIDE_EXT         )
		CONST_INTEGER( VERTEX_ARRAY_COUNT_EXT          , GL_VERTEX_ARRAY_COUNT_EXT          )
		CONST_INTEGER( NORMAL_ARRAY_TYPE_EXT           , GL_NORMAL_ARRAY_TYPE_EXT           )
		CONST_INTEGER( NORMAL_ARRAY_STRIDE_EXT         , GL_NORMAL_ARRAY_STRIDE_EXT         )
		CONST_INTEGER( NORMAL_ARRAY_COUNT_EXT          , GL_NORMAL_ARRAY_COUNT_EXT          )
		CONST_INTEGER( COLOR_ARRAY_SIZE_EXT            , GL_COLOR_ARRAY_SIZE_EXT            )
		CONST_INTEGER( COLOR_ARRAY_TYPE_EXT            , GL_COLOR_ARRAY_TYPE_EXT            )
		CONST_INTEGER( COLOR_ARRAY_STRIDE_EXT          , GL_COLOR_ARRAY_STRIDE_EXT          )
		CONST_INTEGER( COLOR_ARRAY_COUNT_EXT           , GL_COLOR_ARRAY_COUNT_EXT           )
		CONST_INTEGER( INDEX_ARRAY_TYPE_EXT            , GL_INDEX_ARRAY_TYPE_EXT            )
		CONST_INTEGER( INDEX_ARRAY_STRIDE_EXT          , GL_INDEX_ARRAY_STRIDE_EXT          )
		CONST_INTEGER( INDEX_ARRAY_COUNT_EXT           , GL_INDEX_ARRAY_COUNT_EXT           )
		CONST_INTEGER( TEXTURE_COORD_ARRAY_SIZE_EXT    , GL_TEXTURE_COORD_ARRAY_SIZE_EXT    )
		CONST_INTEGER( TEXTURE_COORD_ARRAY_TYPE_EXT    , GL_TEXTURE_COORD_ARRAY_TYPE_EXT    )
		CONST_INTEGER( TEXTURE_COORD_ARRAY_STRIDE_EXT  , GL_TEXTURE_COORD_ARRAY_STRIDE_EXT  )
		CONST_INTEGER( TEXTURE_COORD_ARRAY_COUNT_EXT   , GL_TEXTURE_COORD_ARRAY_COUNT_EXT   )
		CONST_INTEGER( EDGE_FLAG_ARRAY_STRIDE_EXT      , GL_EDGE_FLAG_ARRAY_STRIDE_EXT      )
		CONST_INTEGER( EDGE_FLAG_ARRAY_COUNT_EXT       , GL_EDGE_FLAG_ARRAY_COUNT_EXT       )
		CONST_INTEGER( VERTEX_ARRAY_POINTER_EXT        , GL_VERTEX_ARRAY_POINTER_EXT        )
		CONST_INTEGER( NORMAL_ARRAY_POINTER_EXT        , GL_NORMAL_ARRAY_POINTER_EXT        )
		CONST_INTEGER( COLOR_ARRAY_POINTER_EXT         , GL_COLOR_ARRAY_POINTER_EXT         )
		CONST_INTEGER( INDEX_ARRAY_POINTER_EXT         , GL_INDEX_ARRAY_POINTER_EXT         )
		CONST_INTEGER( TEXTURE_COORD_ARRAY_POINTER_EXT , GL_TEXTURE_COORD_ARRAY_POINTER_EXT )
		CONST_INTEGER( EDGE_FLAG_ARRAY_POINTER_EXT     , GL_EDGE_FLAG_ARRAY_POINTER_EXT     )
		CONST_INTEGER( DOUBLE_EXT                      , GL_DOUBLE_EXT                      )

		CONST_INTEGER( BGR_EXT  , GL_BGR_EXT  )
		CONST_INTEGER( BGRA_EXT , GL_BGRA_EXT )

		CONST_INTEGER( COLOR_TABLE_FORMAT_EXT         , GL_COLOR_TABLE_FORMAT_EXT         )
		CONST_INTEGER( COLOR_TABLE_WIDTH_EXT          , GL_COLOR_TABLE_WIDTH_EXT          )
		CONST_INTEGER( COLOR_TABLE_RED_SIZE_EXT       , GL_COLOR_TABLE_RED_SIZE_EXT       )
		CONST_INTEGER( COLOR_TABLE_GREEN_SIZE_EXT     , GL_COLOR_TABLE_GREEN_SIZE_EXT     )
		CONST_INTEGER( COLOR_TABLE_BLUE_SIZE_EXT      , GL_COLOR_TABLE_BLUE_SIZE_EXT      )
		CONST_INTEGER( COLOR_TABLE_ALPHA_SIZE_EXT     , GL_COLOR_TABLE_ALPHA_SIZE_EXT     )
		CONST_INTEGER( COLOR_TABLE_LUMINANCE_SIZE_EXT , GL_COLOR_TABLE_LUMINANCE_SIZE_EXT )
		CONST_INTEGER( COLOR_TABLE_INTENSITY_SIZE_EXT , GL_COLOR_TABLE_INTENSITY_SIZE_EXT )

		CONST_INTEGER( COLOR_INDEX1_EXT  , GL_COLOR_INDEX1_EXT  )
		CONST_INTEGER( COLOR_INDEX2_EXT  , GL_COLOR_INDEX2_EXT  )
		CONST_INTEGER( COLOR_INDEX4_EXT  , GL_COLOR_INDEX4_EXT  )
		CONST_INTEGER( COLOR_INDEX8_EXT  , GL_COLOR_INDEX8_EXT  )
		CONST_INTEGER( COLOR_INDEX12_EXT , GL_COLOR_INDEX12_EXT )
		CONST_INTEGER( COLOR_INDEX16_EXT , GL_COLOR_INDEX16_EXT )

		CONST_INTEGER( MAX_ELEMENTS_VERTICES_WIN , GL_MAX_ELEMENTS_VERTICES_WIN )
		CONST_INTEGER( MAX_ELEMENTS_INDICES_WIN  , GL_MAX_ELEMENTS_INDICES_WIN  )

		CONST_INTEGER( PHONG_WIN      , GL_PHONG_WIN      )
		CONST_INTEGER( PHONG_HINT_WIN , GL_PHONG_HINT_WIN )

		CONST_INTEGER( FOG_SPECULAR_TEXTURE_WIN , GL_FOG_SPECULAR_TEXTURE_WIN )

		CONST_INTEGER( LOGIC_OP , GL_LOGIC_OP )
		CONST_INTEGER( TEXTURE_COMPONENTS , GL_TEXTURE_COMPONENTS )

//OpenGL extensions
		CONST_INTEGER( POINT_SIZE_MIN             , GL_POINT_SIZE_MIN             )
		CONST_INTEGER( POINT_SIZE_MAX             , GL_POINT_SIZE_MAX             )
		CONST_INTEGER( POINT_FADE_THRESHOLD_SIZE  , GL_POINT_FADE_THRESHOLD_SIZE  )
		CONST_INTEGER( POINT_DISTANCE_ATTENUATION , GL_POINT_DISTANCE_ATTENUATION )

		CONST_INTEGER( POINT_SPRITE, GL_POINT_SPRITE )
		CONST_INTEGER( COORD_REPLACE, GL_COORD_REPLACE )

		#ifndef GL_ARB_multitexture
		CONST_INTEGER( TEXTURE0_ARB             ,GL_TEXTURE0_ARB               )
		CONST_INTEGER( TEXTURE1_ARB             ,GL_TEXTURE1_ARB               )
		CONST_INTEGER( TEXTURE2_ARB             ,GL_TEXTURE2_ARB               )
		CONST_INTEGER( TEXTURE3_ARB             ,GL_TEXTURE3_ARB               )
		CONST_INTEGER( TEXTURE4_ARB             ,GL_TEXTURE4_ARB               )
		CONST_INTEGER( TEXTURE5_ARB             ,GL_TEXTURE5_ARB               )
		CONST_INTEGER( TEXTURE6_ARB             ,GL_TEXTURE6_ARB               )
		CONST_INTEGER( TEXTURE7_ARB             ,GL_TEXTURE7_ARB               )
		CONST_INTEGER( TEXTURE8_ARB             ,GL_TEXTURE8_ARB               )
		CONST_INTEGER( TEXTURE9_ARB             ,GL_TEXTURE9_ARB               )
		CONST_INTEGER( TEXTURE10_ARB            ,GL_TEXTURE10_ARB              )
		CONST_INTEGER( TEXTURE11_ARB            ,GL_TEXTURE11_ARB              )
		CONST_INTEGER( TEXTURE12_ARB            ,GL_TEXTURE12_ARB              )
		CONST_INTEGER( TEXTURE13_ARB            ,GL_TEXTURE13_ARB              )
		CONST_INTEGER( TEXTURE14_ARB            ,GL_TEXTURE14_ARB              )
		CONST_INTEGER( TEXTURE15_ARB            ,GL_TEXTURE15_ARB              )
		CONST_INTEGER( TEXTURE16_ARB            ,GL_TEXTURE16_ARB              )
		CONST_INTEGER( TEXTURE17_ARB            ,GL_TEXTURE17_ARB              )
		CONST_INTEGER( TEXTURE18_ARB            ,GL_TEXTURE18_ARB              )
		CONST_INTEGER( TEXTURE19_ARB            ,GL_TEXTURE19_ARB              )
		CONST_INTEGER( TEXTURE20_ARB            ,GL_TEXTURE20_ARB              )
		CONST_INTEGER( TEXTURE21_ARB            ,GL_TEXTURE21_ARB              )
		CONST_INTEGER( TEXTURE22_ARB            ,GL_TEXTURE22_ARB              )
		CONST_INTEGER( TEXTURE23_ARB            ,GL_TEXTURE23_ARB              )
		CONST_INTEGER( TEXTURE24_ARB            ,GL_TEXTURE24_ARB              )
		CONST_INTEGER( TEXTURE25_ARB            ,GL_TEXTURE25_ARB              )
		CONST_INTEGER( TEXTURE26_ARB            ,GL_TEXTURE26_ARB              )
		CONST_INTEGER( TEXTURE27_ARB            ,GL_TEXTURE27_ARB              )
		CONST_INTEGER( TEXTURE28_ARB            ,GL_TEXTURE28_ARB              )
		CONST_INTEGER( TEXTURE29_ARB            ,GL_TEXTURE29_ARB              )
		CONST_INTEGER( TEXTURE30_ARB            ,GL_TEXTURE30_ARB              )
		CONST_INTEGER( TEXTURE31_ARB            ,GL_TEXTURE31_ARB              )
		CONST_INTEGER( ACTIVE_TEXTURE_ARB       ,GL_ACTIVE_TEXTURE_ARB         )
		CONST_INTEGER( CLIENT_ACTIVE_TEXTURE_ARB,GL_CLIENT_ACTIVE_TEXTURE_ARB  )
		CONST_INTEGER( MAX_TEXTURE_UNITS_ARB    ,GL_MAX_TEXTURE_UNITS_ARB      )
		#endif

		#ifdef GL_ARB_vertex_buffer_object
		CONST_INTEGER( BUFFER_SIZE_ARB                             ,GL_BUFFER_SIZE_ARB                           )
		CONST_INTEGER( BUFFER_USAGE_ARB									  ,GL_BUFFER_USAGE_ARB								  )
		CONST_INTEGER( ARRAY_BUFFER_ARB									  ,GL_ARRAY_BUFFER_ARB								  )
		CONST_INTEGER( ELEMENT_ARRAY_BUFFER_ARB						  ,GL_ELEMENT_ARRAY_BUFFER_ARB						  )
		CONST_INTEGER( ARRAY_BUFFER_BINDING_ARB						  ,GL_ARRAY_BUFFER_BINDING_ARB						  )
		CONST_INTEGER( ELEMENT_ARRAY_BUFFER_BINDING_ARB				  ,GL_ELEMENT_ARRAY_BUFFER_BINDING_ARB			  )
		CONST_INTEGER( VERTEX_ARRAY_BUFFER_BINDING_ARB				  ,GL_VERTEX_ARRAY_BUFFER_BINDING_ARB			  )
		CONST_INTEGER( NORMAL_ARRAY_BUFFER_BINDING_ARB				  ,GL_NORMAL_ARRAY_BUFFER_BINDING_ARB			  )
		CONST_INTEGER( COLOR_ARRAY_BUFFER_BINDING_ARB				  ,GL_COLOR_ARRAY_BUFFER_BINDING_ARB				  )
		CONST_INTEGER( INDEX_ARRAY_BUFFER_BINDING_ARB				  ,GL_INDEX_ARRAY_BUFFER_BINDING_ARB				  )
		CONST_INTEGER( TEXTURE_COORD_ARRAY_BUFFER_BINDING_ARB		  ,GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING_ARB	  )
		CONST_INTEGER( EDGE_FLAG_ARRAY_BUFFER_BINDING_ARB			  ,GL_EDGE_FLAG_ARRAY_BUFFER_BINDING_ARB		  )
		CONST_INTEGER( SECONDARY_COLOR_ARRAY_BUFFER_BINDING_ARB	  ,GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING_ARB  )
		CONST_INTEGER( FOG_COORDINATE_ARRAY_BUFFER_BINDING_ARB	  ,GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING_ARB	  )
		CONST_INTEGER( WEIGHT_ARRAY_BUFFER_BINDING_ARB				  ,GL_WEIGHT_ARRAY_BUFFER_BINDING_ARB			  )
		CONST_INTEGER( VERTEX_ATTRIB_ARRAY_BUFFER_BINDING_ARB		  ,GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING_ARB	  )
		CONST_INTEGER( READ_ONLY_ARB										  ,GL_READ_ONLY_ARB									  )
		CONST_INTEGER( WRITE_ONLY_ARB										  ,GL_WRITE_ONLY_ARB									  )
		CONST_INTEGER( READ_WRITE_ARB										  ,GL_READ_WRITE_ARB									  )
		CONST_INTEGER( BUFFER_ACCESS_ARB									  ,GL_BUFFER_ACCESS_ARB								  )
		CONST_INTEGER( BUFFER_MAPPED_ARB									  ,GL_BUFFER_MAPPED_ARB								  )
		CONST_INTEGER( BUFFER_MAP_POINTER_ARB							  ,GL_BUFFER_MAP_POINTER_ARB						  )
		CONST_INTEGER( STREAM_DRAW_ARB									  ,GL_STREAM_DRAW_ARB									  )
		CONST_INTEGER( STREAM_READ_ARB									  ,GL_STREAM_READ_ARB									  )
		CONST_INTEGER( STREAM_COPY_ARB									  ,GL_STREAM_COPY_ARB									  )
		CONST_INTEGER( STATIC_DRAW_ARB									  ,GL_STATIC_DRAW_ARB									  )
		CONST_INTEGER( STATIC_READ_ARB									  ,GL_STATIC_READ_ARB									  )
		CONST_INTEGER( STATIC_COPY_ARB									  ,GL_STATIC_COPY_ARB									  )
		CONST_INTEGER( DYNAMIC_DRAW_ARB									  ,GL_DYNAMIC_DRAW_ARB								  )
		CONST_INTEGER( DYNAMIC_READ_ARB									  ,GL_DYNAMIC_READ_ARB								  )
		CONST_INTEGER( DYNAMIC_COPY_ARB									  ,GL_DYNAMIC_COPY_ARB								  )
		#endif


	END_CONST_INTEGER_SPEC


	BEGIN_STATIC_FUNCTION_SPEC

		FUNCTION_FAST_ARGC(GetBoolean, 1) // pname
		FUNCTION_FAST_ARGC(GetInteger, 1) // pname
		FUNCTION_FAST_ARGC(GetDouble, 1) // pname
		FUNCTION_FAST_ARGC(Accum, 2) // op, value
		FUNCTION_FAST_ARGC(AlphaFunc, 2) // func, ref
		FUNCTION_FAST_ARGC(Flush, 0)
		FUNCTION_FAST_ARGC(Finish, 0)
		FUNCTION_FAST_ARGC(Fog, 2) // pname, param | array of params
		FUNCTION_FAST_ARGC(Hint, 2) // target, mode
		FUNCTION_FAST_ARGC(Vertex, 3) // x, y [, z]
		FUNCTION_FAST_ARGC(Color, 4) // r, g, b [, a]
		FUNCTION_FAST_ARGC(Normal, 3) // nx, ny, nz
		FUNCTION_FAST_ARGC(TexCoord, 3) // s [, t [,r ]]
		FUNCTION_FAST_ARGC(TexParameter, 3) // target, pname, param | array of params
		FUNCTION_FAST_ARGC(TexEnv, 3) // target, pname, param | array of params
		FUNCTION_FAST_ARGC(LightModel, 2) // pname, param
		FUNCTION_FAST_ARGC(Light, 3) // light, pname, param
		FUNCTION_FAST_ARGC(ColorMaterial, 2) // face, mode
		FUNCTION_FAST_ARGC(Material, 3) // face, pname, param
		FUNCTION_FAST_ARGC(Enable, 1) // cap
		FUNCTION_FAST_ARGC(Disable ,1) // cap
		FUNCTION_FAST_ARGC(PointSize, 1) // size
		FUNCTION_FAST_ARGC(LineWidth, 1) // width
		FUNCTION_FAST_ARGC(ShadeModel, 1) // mode
		FUNCTION_FAST_ARGC(BlendFunc, 2) // sfactor, dfactor
		FUNCTION_FAST_ARGC(DepthFunc, 1) // func
		FUNCTION_FAST_ARGC(DepthRange, 2) // zNear, zFar
		FUNCTION_FAST_ARGC(CullFace, 1) // mode
		FUNCTION_FAST_ARGC(FrontFace, 1) // mode
		FUNCTION_FAST_ARGC(ClearStencil, 1) // s
		FUNCTION_FAST_ARGC(ClearDepth, 1) // depth
		FUNCTION_FAST_ARGC(ClearColor, 4) // r, g, b, alpha
		FUNCTION_FAST_ARGC(ClearAccum, 4) // r, g, b, alpha
		FUNCTION_FAST_ARGC(Clear, 1) // mask
		FUNCTION_FAST_ARGC(ClipPlane, 2) // plane, equation
		FUNCTION_FAST_ARGC(Viewport, 4) // x, y, width, height
		FUNCTION_FAST_ARGC(Frustum, 6) // left, right, bottom, top, zNear, zFar
		FUNCTION_FAST_ARGC(Perspective, 3) // fovY, zNear, zFar (non-OpenGL API)
		FUNCTION_FAST_ARGC(Ortho, 6) // left, right, bottom, top, zNear, zFar
		FUNCTION_FAST_ARGC(MatrixMode, 1) // mode
		FUNCTION_FAST_ARGC(LoadIdentity, 0)
		FUNCTION_FAST_ARGC(PushMatrix, 0)
		FUNCTION_FAST_ARGC(PopMatrix, 0)
		FUNCTION_FAST_ARGC(LoadMatrix, 1) // matrix
		FUNCTION_FAST_ARGC(MultMatrix, 1) // matrix
		FUNCTION_FAST_ARGC(Rotate, 4) // angle, x, y, z
		FUNCTION_FAST_ARGC(Translate, 3) // x, y, z
		FUNCTION_FAST_ARGC(Scale, 3) // x, y, z
		FUNCTION_FAST_ARGC(NewList, 0)
		FUNCTION_FAST_ARGC(DeleteList, 1) // listId
		FUNCTION_FAST_ARGC(EndList, 0)
		FUNCTION_FAST_ARGC(CallList, 1) // listId | array of listId
		FUNCTION_FAST_ARGC(Begin, 1) // mode
		FUNCTION_FAST_ARGC(End, 0)
		FUNCTION_FAST_ARGC(PushAttrib, 1) // mask
		FUNCTION_FAST_ARGC(PopAttrib, 0)
		FUNCTION_FAST_ARGC(GenTexture, 0)
		FUNCTION_FAST_ARGC(BindTexture, 2) // target, texture
		FUNCTION_FAST_ARGC(DeleteTexture, 1) // textureId
		FUNCTION_FAST_ARGC(CopyTexImage2D, 7) // level, internalFormat, x, y, width, height, border
		FUNCTION_FAST_ARGC(DefineTextureImage, 3) // target, format, image (non-OpenGL API)
		FUNCTION_FAST_ARGC(RenderToImage, 0) // (non-OpenGL API)

		FUNCTION_FAST_ARGC(LookAt, 9) // (non-OpenGL API)

		// OpenGL extensions
		FUNCTION_FAST_ARGC(GenBuffer, 0)
		FUNCTION_FAST_ARGC(BindBuffer, 2) // target, buffer
		FUNCTION_FAST_ARGC(PointParameter, 2) // pname, param | Array of param

		FUNCTION_FAST_ARGC(ActiveTexture, 1) // texture
		FUNCTION_FAST_ARGC(ClientActiveTexture, 1) // texture

		FUNCTION_FAST_ARGC(MultiTexCoord, 4) // target, s, t, r

		FUNCTION_FAST_ARGC(LoadTrimesh, 1) // Trimesh object
		FUNCTION_FAST_ARGC(DrawTrimesh, 1) // Trimesh id

		FUNCTION_FAST_ARGC(PixelWidth, 2)
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_READ(error)
	END_STATIC_PROPERTY_SPEC

END_CLASS


/**

== Example ==
{{{

LoadModule('jsstd');
LoadModule('jssdl');
LoadModule('jsgraphics');

GlSetAttribute( GL_SWAP_CONTROL, 1 ); // vsync
GlSetAttribute( GL_DOUBLEBUFFER, 1 );
GlSetAttribute( GL_DEPTH_SIZE, 16 );
SetVideoMode( 320, 200, 32, HWSURFACE | OPENGL | RESIZABLE ); // | ASYNCBLIT // RESIZABLE FULLSCREEN

var listeners = {
	onQuit: function() { end = true },
	onKeyDown: function(key, mod) { end = key == K_ESCAPE }
}

Ogl.MatrixMode(Ogl.PROJECTION);
Ogl.Perspective(60, 0.001, 1000);
Ogl.MatrixMode(Ogl.MODELVIEW);

for (var end = false; !end ;) {
	
	PollEvent(listeners);
	
	with (Ogl) {

		Clear(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);
		LoadIdentity();
		
		LookAt(-1,-1,1, 0,0,0, 0,0,1);
		
		Begin(QUADS);
		Color(1,0,0);
		Vertex( -0.5, -0.5, 0);
		Vertex( -0.5, 0.5, 0);
		Vertex( 0.5, 0.5, 0);
		Vertex( 0.5, -0.5, 0);
		End(QUADS);
 }
	
	GlSwapBuffers();
	Sleep(10);
}

}}}
**/
