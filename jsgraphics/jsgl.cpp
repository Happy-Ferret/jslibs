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

#include "jsgl.h"
#include "jswindow.h"
#include "../common/jsNativeInterface.h"

#include "jstransformation.h"

#include "../jsimage/image.h"
#include "../jslang/bstring.h"
#include "../jsprotex/texture.h"
//TextureJSClass


#define _USE_MATH_DEFINES
#include "math.h"

#include "../common/matrix44.h"

#ifdef _MACOSX // MacosX platform specific
	#include <AGL/agl.h>
	#include <OpenGL/gl.h>
#endif

#include "gl/gl.h"


BEGIN_CLASS( Ogl )


DEFINE_FUNCTION_FAST( Flush ) {

	glFlush();
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Hint ) {

	RT_ASSERT_ARGC(2);
	RT_ASSERT_INT(J_FARG(1));
	RT_ASSERT_INT(J_FARG(2));
	glHint( JSVAL_TO_INT(J_FARG(1)), JSVAL_TO_INT(J_FARG(2)) );
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Vertex ) {

	RT_ASSERT_ARGC(2);

//	if ( J_VALUE_IS_ARRAY(J_FARG(1)) ) {
//	}

//	float vec[3];
//	FloatArrayToVector(cx, 3, &argv[0], vec);
	jsdouble x, y, z;
	JS_ValueToNumber(cx, J_FARG(1), &x);
	JS_ValueToNumber(cx, J_FARG(2), &y);
	if ( J_ARGC >= 3 ) {

		JS_ValueToNumber(cx, J_FARG(2), &z);
		glVertex3d(x, y, z);
	} else {

		glVertex2d(x, y);
	}
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Color ) {

	RT_ASSERT_ARGC(3);
//	float vec[3];
//	FloatArrayToVector(cx, 3, &argv[0], vec);
	jsdouble r, g, b, a;
	JS_ValueToNumber(cx, J_FARG(1), &r);
	JS_ValueToNumber(cx, J_FARG(2), &g);
	JS_ValueToNumber(cx, J_FARG(3), &b);
	if ( J_ARGC >= 4 ) {
		
		JS_ValueToNumber(cx, J_FARG(4), &a);
		glColor4d(r, g, b, a);
	} else {

		glColor3d(r, g, b);
	}
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Normal ) {

	RT_ASSERT_ARGC(3);
//	float vec[3];
//	FloatArrayToVector(cx, 3, &argv[0], vec);
	jsdouble nx, ny, nz;
	JS_ValueToNumber(cx, J_FARG(1), &nx);
	JS_ValueToNumber(cx, J_FARG(2), &ny);
	JS_ValueToNumber(cx, J_FARG(2), &nz);
	glNormal3d(nx, ny, nz);
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( TexCoord ) {

	RT_ASSERT_ARGC(2);
//	float vec[3];
//	FloatArrayToVector(cx, 3, &argv[0], vec);
	jsdouble s, t, r;
	JS_ValueToNumber(cx, J_FARG(1), &s);
	JS_ValueToNumber(cx, J_FARG(2), &t);
	if ( J_ARGC >= 3 ) {

		JS_ValueToNumber(cx, J_FARG(2), &r);
		glTexCoord3d(s, t, r);
	} else {

		glTexCoord2d(s, t);
	}
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( TexParameter ) {

	RT_ASSERT_ARGC(3);
	RT_ASSERT_INT(J_FARG(1));
	RT_ASSERT_INT(J_FARG(2));

	*J_FRVAL = JSVAL_VOID;
	if ( JSVAL_IS_INT(J_FARG(3)) ) {

		glTexParameteri( JSVAL_TO_INT( J_FARG(1) ), JSVAL_TO_INT( J_FARG(2) ), JSVAL_TO_INT( J_FARG(3) ) );
		return JS_TRUE;
	}
	if ( JSVAL_IS_DOUBLE(J_FARG(3)) ) {
	
		jsdouble param;
		RT_CHECK_CALL( JS_ValueToNumber(cx, J_FARG(3), &param) );
		glTexParameterf( JSVAL_TO_INT( J_FARG(1) ), JSVAL_TO_INT( J_FARG(2) ), param );
		return JS_TRUE;
	}
	REPORT_ERROR("Invalid argument.");
	return JS_TRUE;
}



DEFINE_FUNCTION_FAST( LightModel ) {

	RT_ASSERT_ARGC(2);
	RT_ASSERT_INT(J_FARG(1));

	*J_FRVAL = JSVAL_VOID;
	if ( JSVAL_IS_INT(J_FARG(3)) ) {

		glLightModeli( JSVAL_TO_INT( J_FARG(1) ), JSVAL_TO_INT( J_FARG(2) ) );
		return JS_TRUE;
	}
	if ( JSVAL_IS_DOUBLE(J_FARG(3)) ) {
	
		jsdouble param;
		RT_CHECK_CALL( JS_ValueToNumber(cx, J_FARG(2), &param) );
		glLightModelf( JSVAL_TO_INT( J_FARG(1) ), param );
		return JS_TRUE;
	}
	REPORT_ERROR("Invalid argument.");
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Light ) {

	RT_ASSERT_ARGC(3);
	RT_ASSERT_INT(J_FARG(1));
	RT_ASSERT_INT(J_FARG(2));

	*J_FRVAL = JSVAL_VOID;
	if ( JSVAL_IS_INT(J_FARG(3)) ) {

		glLighti( JSVAL_TO_INT( J_FARG(1) ), JSVAL_TO_INT( J_FARG(2) ), JSVAL_TO_INT( J_FARG(3) ) );
		return JS_TRUE;
	}
	if ( JSVAL_IS_DOUBLE(J_FARG(3)) ) {
	
		jsdouble param;
		RT_CHECK_CALL( JS_ValueToNumber(cx, J_FARG(3), &param) );
		glLightf( JSVAL_TO_INT( J_FARG(1) ), JSVAL_TO_INT( J_FARG(2) ), param );
		return JS_TRUE;
	}
	REPORT_ERROR("Invalid argument.");
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Material ) {

	RT_ASSERT_ARGC(3);
	RT_ASSERT_INT(J_FARG(1));
	RT_ASSERT_INT(J_FARG(2));

	*J_FRVAL = JSVAL_VOID;
	if ( JSVAL_IS_INT(J_FARG(3)) ) {

		glMateriali( JSVAL_TO_INT( J_FARG(1) ), JSVAL_TO_INT( J_FARG(2) ), JSVAL_TO_INT( J_FARG(3) ) );
		return JS_TRUE;
	}
	if ( JSVAL_IS_DOUBLE(J_FARG(3)) ) {
	
		jsdouble param;
		RT_CHECK_CALL( JS_ValueToNumber(cx, J_FARG(3), &param) );
		glMaterialf( JSVAL_TO_INT( J_FARG(1) ), JSVAL_TO_INT( J_FARG(2) ), param );
		return JS_TRUE;
	}
	REPORT_ERROR("Invalid argument.");
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Enable ) {
	
	RT_ASSERT_ARGC(1);
	RT_ASSERT_INT(J_FARG(1));
	glEnable( JSVAL_TO_INT( J_FARG(1) ) );
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Disable ) {
	
	RT_ASSERT_ARGC(1);
	RT_ASSERT_INT(J_FARG(1));
	glDisable( JSVAL_TO_INT( J_FARG(1) ) );
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( ShadeModel ) {

	RT_ASSERT_INT(J_FARG(1));
	glShadeModel( JSVAL_TO_INT( J_FARG(1) ) );
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( BlendFunc ) {

	RT_ASSERT_ARGC(2);
	RT_ASSERT_INT(J_FARG(1));
	RT_ASSERT_INT(J_FARG(2));
	glBlendFunc( JSVAL_TO_INT( J_FARG(1) ), JSVAL_TO_INT( J_FARG(2) ) );
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( DepthFunc ) {

	RT_ASSERT_ARGC(1);
	RT_ASSERT_INT(J_FARG(1));
	glDepthFunc( JSVAL_TO_INT( J_FARG(1) ) );
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( DepthRange ) {

	RT_ASSERT_ARGC(2);
	jsdouble zNear, zFar;
	JS_ValueToNumber(cx, J_FARG(1), &zNear);
	JS_ValueToNumber(cx, J_FARG(2), &zFar);
	glDepthRange(zNear, zFar);
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( CullFace ) {

	RT_ASSERT_ARGC(1);
	RT_ASSERT_INT(J_FARG(1));
	glCullFace(JSVAL_TO_INT( J_FARG(1) ));
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( FrontFace ) {

	RT_ASSERT_ARGC(1);
	RT_ASSERT_INT(J_FARG(1));
	glFrontFace( JSVAL_TO_INT( J_FARG(1) ) );
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( ClearStencil ) {

	RT_ASSERT_ARGC(1);
	RT_ASSERT_INT(J_FARG(1));
	glClearStencil(JSVAL_TO_INT( J_FARG(1) ));
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( ClearDepth ) {

	RT_ASSERT_ARGC(1);
	jsdouble depth;
	JS_ValueToNumber(cx, J_FARG(1), &depth);
	glClearDepth(depth);
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( ClearColor ) {

	RT_ASSERT_ARGC(4);
	jsdouble r, g, b, a;
	JS_ValueToNumber(cx, J_FARG(1), &r);
	JS_ValueToNumber(cx, J_FARG(2), &g);
	JS_ValueToNumber(cx, J_FARG(3), &b);
	JS_ValueToNumber(cx, J_FARG(4), &a);
	glClearColor(r, g, b, a);
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Clear ) {

	RT_ASSERT_ARGC(1);
	RT_ASSERT_INT(J_FARG(1));
	glClear(JSVAL_TO_INT(J_FARG(1)));
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Viewport ) {

	RT_ASSERT_ARGC(4);
	RT_ASSERT_INT(J_FARG(1));
	RT_ASSERT_INT(J_FARG(2));
	RT_ASSERT_INT(J_FARG(3));
	RT_ASSERT_INT(J_FARG(4));
	glViewport(JSVAL_TO_INT(J_FARG(1)), JSVAL_TO_INT(J_FARG(2)), JSVAL_TO_INT(J_FARG(3)), JSVAL_TO_INT(J_FARG(4)));
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Frustum ) {

	RT_ASSERT_ARGC(6);
	jsdouble left, right, bottom, top, zNear, zFar;
	JS_ValueToNumber(cx, J_FARG(1), &left);
	JS_ValueToNumber(cx, J_FARG(2), &right);
	JS_ValueToNumber(cx, J_FARG(3), &bottom);
	JS_ValueToNumber(cx, J_FARG(4), &top);
	JS_ValueToNumber(cx, J_FARG(5), &zNear);
	JS_ValueToNumber(cx, J_FARG(6), &zFar);
	glFrustum(left, right, bottom, top, zNear, zFar);
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Ortho ) {

	RT_ASSERT_ARGC(6);
	jsdouble left, right, bottom, top, zNear, zFar;
	JS_ValueToNumber(cx, J_FARG(1), &left);
	JS_ValueToNumber(cx, J_FARG(2), &right);
	JS_ValueToNumber(cx, J_FARG(3), &bottom);
	JS_ValueToNumber(cx, J_FARG(4), &top);
	JS_ValueToNumber(cx, J_FARG(5), &zNear);
	JS_ValueToNumber(cx, J_FARG(6), &zFar);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(left, right, bottom, top, zNear, zFar);
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Perspective ) {

	//cf. gluPerspective(fovy, float(viewport[2]) / float(viewport[3]), zNear, zFar);

	RT_ASSERT_ARGC(3);
	jsdouble fovy, zNear, zFar;
	JS_ValueToNumber(cx, J_FARG(1), &fovy);
	JS_ValueToNumber(cx, J_FARG(2), &zNear);
	JS_ValueToNumber(cx, J_FARG(3), &zFar);

//	GLint prevMatrixMode;
//	glGetIntegerv(GL_MATRIX_MODE, &prevMatrixMode); // GL_MODELVIEW
	
	GLint viewport[4];
	glGetIntegerv( GL_VIEWPORT, viewport );
	float aspect = float(viewport[2]) / float(viewport[3]);

	float xmin, xmax, ymin, ymax;
	ymax = zNear * tan(fovy * M_PI / 360.0f);
	ymin = -ymax;
	xmin = ymin * aspect;
	xmax = ymax * aspect;

	glFrustum(xmin, xmax, ymin, ymax, zNear, zFar);

	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( MatrixMode ) {

	RT_ASSERT_ARGC(1);
	RT_ASSERT_INT(J_FARG(1));
	glMatrixMode(JSVAL_TO_INT( J_FARG(1) ));
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( LoadIdentity ) {

	glLoadIdentity();
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( PushMatrix ) {

	glPushMatrix();
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( PopMatrix ) {

	glPopMatrix();
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( LoadMatrix ) {

	RT_ASSERT_ARGC(1);
	Matrix44 tmp, *m = &tmp;
	if (GetMatrixHelper(cx, J_FARG(1), &m) == JS_FALSE)
		return JS_FALSE;
	glLoadMatrixf(m->raw);
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Rotate ) {

	RT_ASSERT_ARGC(4);
	jsdouble angle, x, y, z;
	JS_ValueToNumber(cx, J_FARG(1), &angle);
	JS_ValueToNumber(cx, J_FARG(2), &x);
	JS_ValueToNumber(cx, J_FARG(3), &y);
	JS_ValueToNumber(cx, J_FARG(4), &z);
	glRotated(angle, x, y, z);
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Translate ) {

	RT_ASSERT_ARGC(3);
	jsdouble x, y, z;
	JS_ValueToNumber(cx, J_FARG(1), &x);
	JS_ValueToNumber(cx, J_FARG(2), &y);
	JS_ValueToNumber(cx, J_FARG(3), &z);
	glTranslated(x, y, z);
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}

DEFINE_FUNCTION_FAST( Scale ) {

	RT_ASSERT_ARGC(3);
	jsdouble x, y, z;
	JS_ValueToNumber(cx, J_FARG(1), &x);
	JS_ValueToNumber(cx, J_FARG(2), &y);
	JS_ValueToNumber(cx, J_FARG(3), &z);
	glScaled(x, y, z);
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( NewList ) {

	GLuint list = glGenLists(1);
	glNewList(list, GL_COMPILE);
	*J_FRVAL = INT_TO_JSVAL(list);
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( DeleteList ) {

	RT_ASSERT_ARGC(1);
	RT_ASSERT_INT(J_FARG(1));
	GLuint list = JSVAL_TO_INT(J_FARG(1));
	glDeleteLists(list, 1);
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( EndList ) {

	glEndList();
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( CallList ) {

	RT_ASSERT_ARGC(1);
	RT_ASSERT_INT(J_FARG(1));
	GLuint list = JSVAL_TO_INT(J_FARG(1));
	glCallList(list);
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( CallLists ) {

	RT_ASSERT_ARGC(1);
	RT_ASSERT_ARRAY( J_FARG(1) );

	JSObject *jsArray = JSVAL_TO_OBJECT(J_FARG(1));
	jsuint length;
	RT_CHECK_CALL( JS_GetArrayLength(cx, jsArray, &length) );

	GLuint *lists = (GLuint*)malloc(length * sizeof(GLuint));
	jsval value;
	for (jsuint i=0; i<length; ++i) {

		RT_CHECK_CALL( JS_GetElement(cx, jsArray, i, &value) );
		lists[i] = JSVAL_TO_INT(value);
	}
	glCallLists(length, GL_UNSIGNED_INT, lists); // http://www.opengl.org/documentation/specs/man_pages/hardcopy/GL/html/gl/calllists.html
	free(lists);

	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Begin ) {

	RT_ASSERT_ARGC(1);
	RT_ASSERT_INT(J_FARG(1));
	glBegin(JSVAL_TO_INT( J_FARG(1) ));
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( End ) {

	glEnd();
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( PushAttrib ) {

	RT_ASSERT_ARGC(1);
	RT_ASSERT_INT(J_FARG(1));
	glPushAttrib(JSVAL_TO_INT( J_FARG(1) ));
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( PopAttrib ) {

	glPopAttrib();
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( GenTexture ) {

	GLuint texture;
	glGenTextures( 1, &texture );
	*J_FRVAL = INT_TO_JSVAL(texture);
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( BindTexture ) {

	RT_ASSERT_ARGC(2);
	RT_ASSERT_INT(J_FARG(1));
	RT_ASSERT_INT(J_FARG(2));
	glBindTexture( JSVAL_TO_INT( J_FARG(1) ), JSVAL_TO_INT( J_FARG(2) ));
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( DeleteTexture ) {

	RT_ASSERT_ARGC(1);
	RT_ASSERT_INT(J_FARG(1));
	GLuint texture = JSVAL_TO_INT( J_FARG(1) );
	glDeleteTextures(1, &texture);
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


// (TBD) manage compression: http://www.opengl.org/registry/specs/ARB/texture_compression.txt
DEFINE_FUNCTION_FAST( DefineTextureImage ) {

	RT_ASSERT_ARGC(2);
	RT_ASSERT_INT(J_FARG(1));
	RT_ASSERT_OBJECT(J_FARG(2));

	JSObject *tObj = JSVAL_TO_OBJECT(J_FARG(2));
	const char * className = JS_GET_CLASS(cx, tObj)->name;

	GLsizei width, height;
	GLenum format, type;
	GLvoid *data;
	int channels;

//	if ( strcmp(className, "Texture" ) == 0 ) {
	if ( TextureJSClass(cx) == JS_GET_CLASS(cx, tObj) ) {

		Texture *tex = (Texture *)JS_GetPrivate(cx, tObj);
		RT_ASSERT_RESOURCE(tex);

		data = tex->cbuffer;
		width = tex->width;
		height = tex->height;
		channels = tex->channels;
		type = GL_FLOAT;
	} else
	if ( strcmp(className, "Image" ) == 0 ) {

		data = JS_GetPrivate(cx, tObj);
		RT_ASSERT_RESOURCE(data);

		GetIntProperty(cx, tObj, "width", &width);
		GetIntProperty(cx, tObj, "height", &height);
		GetIntProperty(cx, tObj, "channels", &channels);
		type = GL_UNSIGNED_BYTE;
	} else {
		
		REPORT_ERROR("Invalid texture type.");
	}

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

	glTexImage2D( JSVAL_TO_INT(J_FARG(1)), 0, 3, width, height, 0, format, type, data );
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( RenderToImage ) {

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	GLint viewport[4];
	glGetIntegerv( GL_VIEWPORT, viewport );
	int width = viewport[2];
	int height = viewport[3];

	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, width, height, 0);

	GLint tWidth, tHeight, tComponents;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &tWidth);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &tHeight);
//	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPONENTS, &tComponents);
	//  glGet	with arguments GL_PACK_ALIGNMENT and others

	int size = tWidth * tHeight * 4; // RGBA
	void *pixels = malloc(size);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	JSObject *bstr = NewImage(cx, tWidth, tHeight, 4, pixels);
	RT_ASSERT_ALLOC(bstr);
	*J_FRVAL = OBJECT_TO_JSVAL(bstr);

	glDeleteTextures(1, &texture);
	return JS_TRUE;
}


DEFINE_PROPERTY(error) {

	*vp = INT_TO_JSVAL(glGetError());
	return JS_TRUE;
}

CONFIGURE_CLASS

	BEGIN_CONST_INTEGER_SPEC

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
	END_CONST_INTEGER_SPEC


	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION_FAST(Flush)
		FUNCTION_FAST(Hint)
		FUNCTION_FAST(Vertex)
		FUNCTION_FAST(Color)
		FUNCTION_FAST(Normal)
		FUNCTION_FAST(TexCoord)
		FUNCTION_FAST(TexParameter)
		FUNCTION_FAST(LightModel)
		FUNCTION_FAST(Light)
		FUNCTION_FAST(Material)
		FUNCTION_FAST(Enable)
		FUNCTION_FAST(ShadeModel)
		FUNCTION_FAST(BlendFunc)
		FUNCTION_FAST(DepthFunc)
		FUNCTION_FAST(DepthRange)
		FUNCTION_FAST(CullFace)
		FUNCTION_FAST(FrontFace)
		FUNCTION_FAST(ClearStencil)
		FUNCTION_FAST(ClearDepth)
		FUNCTION_FAST(ClearColor)
		FUNCTION_FAST(Clear)
		FUNCTION_FAST(Viewport)
		FUNCTION_FAST(Frustum)
		FUNCTION_FAST(Perspective)
		FUNCTION_FAST(Ortho)
		FUNCTION_FAST(MatrixMode)
		FUNCTION_FAST(LoadIdentity)
		FUNCTION_FAST(PushMatrix)
		FUNCTION_FAST(PopMatrix)
		FUNCTION_FAST(LoadMatrix)
		FUNCTION_FAST(Rotate)
		FUNCTION_FAST(Translate)
		FUNCTION_FAST(Scale)
		FUNCTION_FAST(NewList)
		FUNCTION_FAST(DeleteList)
		FUNCTION_FAST(EndList)
		FUNCTION_FAST(CallList)
		FUNCTION_FAST(CallLists)
		FUNCTION_FAST(Begin)
		FUNCTION_FAST(End)
		FUNCTION_FAST(PushAttrib)
		FUNCTION_FAST(PopAttrib)
		FUNCTION_FAST(GenTexture)
		FUNCTION_FAST(BindTexture)
		FUNCTION_FAST(DeleteTexture)
		FUNCTION_FAST(DefineTextureImage)
		FUNCTION_FAST(RenderToImage)
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_READ(error)
	END_STATIC_PROPERTY_SPEC


END_CLASS
