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

//#include "jlnativeinterface.h"

//#include "jstransformation.h"

#include "../jsimage/image.h"
#include "../jslang/blob.h"
#include "../jsprotex/texture.h"
#include "../jsprotex/textureBuffer.h"
//TextureJSClass

#include "../jslang/handlePub.h"

#include "../jstrimesh/trimeshPub.h"

#define _USE_MATH_DEFINES
#include "math.h"

#include "matrix44.h"
#include "vector3.h"

/* 
JSBool GetArgInt( JSContext *cx, uintN *argc, jsval **argv, uintN count, int *rval ) { // (TBD) jsval** = Conservative Stack Scanning issue ?
	
	size_t i;
	if ( JSVAL_IS_PRIMITIVE(**argv) || !JL_IsArray(cx, **argv) ) {

		JL_S_ASSERT( *argc >= count, "Not enough arguments." );
		for ( i = 0; i < count; ++i ) {

			JL_CHK( JL_JsvalToNative(cx, **argv, rval) );
			++rval;
			++*argv;
		}
		*argc -= count;
		return JS_TRUE;
	}
	jsuint len;
	JL_CHK( JL_JsvalToCValVector(cx, **argv, rval, count, &len) );
	JL_S_ASSERT( len == count, "Not enough elements." );
	++*argv;
	--*argc;
	return JS_TRUE;
	JL_BAD;
}

JSBool GetArgDouble( JSContext *cx, uintN *argc, jsval **argv, uintN count, double *rval ) { // (TBD) jsval** = Conservative Stack Scanning issue ?
	
	size_t i;
	if ( JSVAL_IS_PRIMITIVE(**argv) || !JL_IsArray(cx, **argv) ) {

		JL_S_ASSERT( *argc >= count, "Not enough arguments." );
		for ( i = 0; i < count; ++i ) {

			JL_CHK( JL_JsvalToNative(cx, **argv, rval) );
			++rval;
			++*argv;
		}
		*argc -= count;
		return JS_TRUE;
	}
	jsuint len;
	JL_CHK( JL_JsvalToCValVector(cx, **argv, rval, count, &len) );
	JL_S_ASSERT( len == count, "Not enough elements." );
	++*argv;
	--*argc;
	return JS_TRUE;
	JL_BAD;
}
*/




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
#include "glext.h" // download at http://www.opengl.org/registry/api/glext.h (http://www.opengl.org/registry/#headers)

#include <gl/glu.h>

#include "wglew.h"

#include "oglError.h"

// http://www.opengl.org/registry/api/glext.h

typedef void* (__cdecl *glGetProcAddress_t)(const char*);
static glGetProcAddress_t glGetProcAddress = NULL;


// The specification states that any command that is not valid is completely ignored and the proper error bit is set.
// Calling glGetError in a Begin/End-pair is not valid, and so the command is ignored the GL_INVALID_OPERATION error bit is set.
// Directly after the Begin/End-pair, the error is returned, because that's the first valid call to glGetError after the error occured.
#if defined(DEBUG)

static bool _inBeginEnd = false;

#define OGL_CHK \
JL_MACRO_BEGIN \
	if ( !_unsafeMode && !_inBeginEnd /*&& false*/ ) { \
		GLenum err = glGetError(); \
		if ( err != GL_NO_ERROR ) \
			JL_REPORT_WARNING("OpenGL error %d", err); \
	} \
JL_MACRO_END

//#undef OGL_CHK
//#define OGL_CHK


#else // DBUG

#define OGL_CHK

#endif // DBUG

#define DECLARE_OPENGL_EXTENSION( NAME, PROTOTYPE ) static PROTOTYPE NAME = NULL;

#define INIT_OPENGL_EXTENSION( NAME, PROTOTYPE ) \
JL_MACRO_BEGIN \
	if ( NAME == NULL ) \
		NAME = (PROTOTYPE)glGetProcAddress( #NAME ); \
JL_MACRO_END

#define JL_INIT_OPENGL_EXTENSION( NAME, PROTOTYPE ) \
JL_MACRO_BEGIN \
	INIT_OPENGL_EXTENSION(NAME, PROTOTYPE); \
	JL_S_ASSERT( NAME != NULL, "OpenGL extension %s is unavailable.", #NAME ); \
JL_MACRO_END


DECLARE_OPENGL_EXTENSION( glBlendColor, PFNGLBLENDCOLORPROC);
DECLARE_OPENGL_EXTENSION( glBlendEquation, PFNGLBLENDEQUATIONPROC);
DECLARE_OPENGL_EXTENSION( glDrawRangeElements, PFNGLDRAWRANGEELEMENTSPROC);
DECLARE_OPENGL_EXTENSION( glTexImage3D, PFNGLTEXIMAGE3DPROC);
DECLARE_OPENGL_EXTENSION( glTexSubImage3D, PFNGLTEXSUBIMAGE3DPROC);
DECLARE_OPENGL_EXTENSION( glCopyTexSubImage3D, PFNGLCOPYTEXSUBIMAGE3DPROC)

DECLARE_OPENGL_EXTENSION( glPointParameteri, PFNGLPOINTPARAMETERIPROC );
DECLARE_OPENGL_EXTENSION( glPointParameterf, PFNGLPOINTPARAMETERFPROC );
DECLARE_OPENGL_EXTENSION( glPointParameterfv, PFNGLPOINTPARAMETERFVPROC );
DECLARE_OPENGL_EXTENSION( glActiveTexture, PFNGLACTIVETEXTUREPROC );
DECLARE_OPENGL_EXTENSION( glClientActiveTexture, PFNGLCLIENTACTIVETEXTUREPROC );
DECLARE_OPENGL_EXTENSION( glMultiTexCoord1d, PFNGLMULTITEXCOORD1DPROC );
DECLARE_OPENGL_EXTENSION( glMultiTexCoord2d, PFNGLMULTITEXCOORD2DPROC );
DECLARE_OPENGL_EXTENSION( glMultiTexCoord3d, PFNGLMULTITEXCOORD3DPROC );
DECLARE_OPENGL_EXTENSION( glBindRenderbufferEXT, PFNGLBINDRENDERBUFFEREXTPROC );
DECLARE_OPENGL_EXTENSION( glGenRenderbuffersEXT, PFNGLGENRENDERBUFFERSEXTPROC );
DECLARE_OPENGL_EXTENSION( glDeleteRenderbuffersEXT, PFNGLDELETERENDERBUFFERSEXTPROC );
DECLARE_OPENGL_EXTENSION( glRenderbufferStorageEXT, PFNGLRENDERBUFFERSTORAGEEXTPROC );
DECLARE_OPENGL_EXTENSION( glGetRenderbufferParameterivEXT, PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC );
DECLARE_OPENGL_EXTENSION( glBindFramebufferEXT, PFNGLBINDFRAMEBUFFEREXTPROC );
DECLARE_OPENGL_EXTENSION( glGenFramebuffersEXT, PFNGLGENFRAMEBUFFERSEXTPROC );
DECLARE_OPENGL_EXTENSION( glDeleteFramebuffersEXT, PFNGLDELETEFRAMEBUFFERSEXTPROC );
DECLARE_OPENGL_EXTENSION( glCheckFramebufferStatusEXT, PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC );
DECLARE_OPENGL_EXTENSION( glFramebufferTexture1DEXT, PFNGLFRAMEBUFFERTEXTURE1DEXTPROC );
DECLARE_OPENGL_EXTENSION( glFramebufferTexture2DEXT, PFNGLFRAMEBUFFERTEXTURE2DEXTPROC );
DECLARE_OPENGL_EXTENSION( glFramebufferTexture3DEXT, PFNGLFRAMEBUFFERTEXTURE3DEXTPROC );
DECLARE_OPENGL_EXTENSION( glFramebufferRenderbufferEXT, PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC );
DECLARE_OPENGL_EXTENSION( glGetFramebufferAttachmentParameterivEXT, PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC );
DECLARE_OPENGL_EXTENSION( glIsBuffer, PFNGLISBUFFERPROC );
DECLARE_OPENGL_EXTENSION( glGenBuffers, PFNGLGENBUFFERSPROC );
DECLARE_OPENGL_EXTENSION( glBindBuffer, PFNGLBINDBUFFERPROC );
DECLARE_OPENGL_EXTENSION( glBufferData, PFNGLBUFFERDATAPROC );
DECLARE_OPENGL_EXTENSION( glMapBuffer, PFNGLMAPBUFFERPROC );
DECLARE_OPENGL_EXTENSION( glUnmapBuffer, PFNGLUNMAPBUFFERPROC );
//DECLARE_OPENGL_EXTENSION( glPolygonOffsetEXT, PFNGLPOLYGONOFFSETEXTPROC );

DECLARE_OPENGL_EXTENSION( glCreateShaderObjectARB, PFNGLCREATESHADEROBJECTARBPROC );
DECLARE_OPENGL_EXTENSION( glDeleteObjectARB, PFNGLDELETEOBJECTARBPROC );
DECLARE_OPENGL_EXTENSION( glGetInfoLogARB, PFNGLGETINFOLOGARBPROC );
DECLARE_OPENGL_EXTENSION( glCreateProgramObjectARB, PFNGLCREATEPROGRAMOBJECTARBPROC );
DECLARE_OPENGL_EXTENSION( glShaderSourceARB, PFNGLSHADERSOURCEARBPROC );
DECLARE_OPENGL_EXTENSION( glCompileShaderARB, PFNGLCOMPILESHADERARBPROC );
DECLARE_OPENGL_EXTENSION( glAttachObjectARB, PFNGLATTACHOBJECTARBPROC );
DECLARE_OPENGL_EXTENSION( glLinkProgramARB, PFNGLLINKPROGRAMARBPROC );
DECLARE_OPENGL_EXTENSION( glUseProgramObjectARB, PFNGLUSEPROGRAMOBJECTARBPROC );
DECLARE_OPENGL_EXTENSION( glGetUniformLocationARB, PFNGLGETUNIFORMLOCATIONARBPROC );

DECLARE_OPENGL_EXTENSION( glUniform1fARB, PFNGLUNIFORM1FARBPROC );
DECLARE_OPENGL_EXTENSION( glUniform2fARB, PFNGLUNIFORM2FARBPROC );
DECLARE_OPENGL_EXTENSION( glUniform3fARB, PFNGLUNIFORM3FARBPROC );
DECLARE_OPENGL_EXTENSION( glUniform4fARB, PFNGLUNIFORM4FARBPROC );

DECLARE_OPENGL_EXTENSION( glUniform1fvARB, PFNGLUNIFORM1FVARBPROC );
DECLARE_OPENGL_EXTENSION( glUniform2fvARB, PFNGLUNIFORM2FVARBPROC );
DECLARE_OPENGL_EXTENSION( glUniform3fvARB, PFNGLUNIFORM3FVARBPROC );
DECLARE_OPENGL_EXTENSION( glUniform4fvARB, PFNGLUNIFORM4FVARBPROC );

DECLARE_OPENGL_EXTENSION( glGetUniformfvARB, PFNGLGETUNIFORMFVARBPROC );

DECLARE_OPENGL_EXTENSION( glUniform1iARB, PFNGLUNIFORM1IARBPROC );
DECLARE_OPENGL_EXTENSION( glUniform2iARB, PFNGLUNIFORM2IARBPROC );
DECLARE_OPENGL_EXTENSION( glUniform3iARB, PFNGLUNIFORM3IARBPROC );
DECLARE_OPENGL_EXTENSION( glUniform4iARB, PFNGLUNIFORM4IARBPROC );

DECLARE_OPENGL_EXTENSION( glUniform1ivARB, PFNGLUNIFORM1IVARBPROC );
DECLARE_OPENGL_EXTENSION( glUniform2ivARB, PFNGLUNIFORM2IVARBPROC );
DECLARE_OPENGL_EXTENSION( glUniform3ivARB, PFNGLUNIFORM3IVARBPROC );
DECLARE_OPENGL_EXTENSION( glUniform4ivARB, PFNGLUNIFORM4IVARBPROC );

DECLARE_OPENGL_EXTENSION( glGetUniformivARB, PFNGLGETUNIFORMIVARBPROC );

DECLARE_OPENGL_EXTENSION( glUniformMatrix2fvARB, PFNGLUNIFORMMATRIX2FVARBPROC );
DECLARE_OPENGL_EXTENSION( glUniformMatrix3fvARB, PFNGLUNIFORMMATRIX3FVARBPROC );
DECLARE_OPENGL_EXTENSION( glUniformMatrix4fvARB, PFNGLUNIFORMMATRIX4FVARBPROC );

DECLARE_OPENGL_EXTENSION( glGetObjectParameterfvARB, PFNGLGETOBJECTPARAMETERFVARBPROC );
DECLARE_OPENGL_EXTENSION( glGetObjectParameterivARB, PFNGLGETOBJECTPARAMETERIVARBPROC );

DECLARE_OPENGL_EXTENSION( glBindAttribLocationARB, PFNGLBINDATTRIBLOCATIONARBPROC );
DECLARE_OPENGL_EXTENSION( glGetAttribLocationARB, PFNGLGETATTRIBLOCATIONARBPROC );

DECLARE_OPENGL_EXTENSION( glVertexAttrib1sARB, PFNGLVERTEXATTRIB1SARBPROC );

DECLARE_OPENGL_EXTENSION( glVertexAttrib1dARB, PFNGLVERTEXATTRIB1DARBPROC );
DECLARE_OPENGL_EXTENSION( glVertexAttrib2dARB, PFNGLVERTEXATTRIB2DARBPROC );
DECLARE_OPENGL_EXTENSION( glVertexAttrib3dARB, PFNGLVERTEXATTRIB3DARBPROC );
DECLARE_OPENGL_EXTENSION( glVertexAttrib4dARB, PFNGLVERTEXATTRIB4DARBPROC );

DECLARE_OPENGL_EXTENSION( glStencilOpSeparate, PFNGLSTENCILOPSEPARATEPROC );
DECLARE_OPENGL_EXTENSION( glStencilFuncSeparate, PFNGLSTENCILFUNCSEPARATEPROC );
DECLARE_OPENGL_EXTENSION( glActiveStencilFaceEXT, PFNGLACTIVESTENCILFACEEXTPROC );



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
 $BOOL $INAME( cap )
  $H arguments
   $ARG GLenum cap
  $H return value
   test whether a capability is enabled.
  $H OpenGL API
   glIsEnabled
**/
DEFINE_FUNCTION( IsEnabled ) {

	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT_INT(JL_ARG(1));
	*JL_RVAL = glIsEnabled(JSVAL_TO_INT(JL_ARG(1))) ? JSVAL_TRUE : JSVAL_FALSE;  OGL_CHK;
	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION( Get ) {

	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT_INT(JL_ARG(1));

	int pname = JSVAL_TO_INT( JL_ARG(1) );

	switch ( pname ) { // http://www.opengl.org/sdk/docs/man/xhtml/glGet.xml

		case GL_ALPHA_TEST:
		case GL_AUTO_NORMAL:
		case GL_BLEND:
		case GL_CLIP_PLANE0:
		case GL_CLIP_PLANE1:
		case GL_CLIP_PLANE2:
		case GL_CLIP_PLANE3:
		case GL_CLIP_PLANE4:
		case GL_CLIP_PLANE5:
		case GL_COLOR_ARRAY:
		case GL_COLOR_LOGIC_OP:
		case GL_COLOR_MATERIAL:
		case GL_COLOR_SUM:
		case GL_COLOR_TABLE:
		case GL_CONVOLUTION_1D:
		case GL_CONVOLUTION_2D:
		case GL_CULL_FACE:
		case GL_CURRENT_RASTER_POSITION_VALID:
		case GL_DEPTH_TEST:
		case GL_DEPTH_WRITEMASK:
		case GL_DITHER:
		case GL_DOUBLEBUFFER:
		case GL_EDGE_FLAG:
		case GL_EDGE_FLAG_ARRAY:
		case GL_FOG:
		case GL_FOG_COORD_ARRAY:
		case GL_HISTOGRAM:
		case GL_INDEX_ARRAY:
		case GL_INDEX_LOGIC_OP:
		case GL_INDEX_MODE:
		case GL_LIGHT0:
		case GL_LIGHT1:
		case GL_LIGHT2:
		case GL_LIGHT3:
		case GL_LIGHT4:
		case GL_LIGHT5:
		case GL_LIGHT6:
		case GL_LIGHT7:
		case GL_LIGHTING:
		case GL_LIGHT_MODEL_LOCAL_VIEWER:
		case GL_LIGHT_MODEL_TWO_SIDE:
		case GL_LINE_SMOOTH:
		case GL_LINE_STIPPLE:
		case GL_MAP1_COLOR_4:
		case GL_MAP1_INDEX:
		case GL_MAP1_NORMAL:
		case GL_MAP1_TEXTURE_COORD_1:
		case GL_MAP1_TEXTURE_COORD_2:
		case GL_MAP1_TEXTURE_COORD_3:
		case GL_MAP1_TEXTURE_COORD_4:
		case GL_MAP1_VERTEX_3:
		case GL_MAP1_VERTEX_4:
		case GL_MAP2_COLOR_4:
		case GL_MAP2_INDEX:
		case GL_MAP2_NORMAL:
		case GL_MAP2_TEXTURE_COORD_1:
		case GL_MAP2_TEXTURE_COORD_2:
		case GL_MAP2_TEXTURE_COORD_3:
		case GL_MAP2_TEXTURE_COORD_4:
		case GL_MAP2_VERTEX_3:
		case GL_MAP2_VERTEX_4:
		case GL_MAP_COLOR:
		case GL_MAP_STENCIL:
		case GL_MINMAX:
		case GL_NORMAL_ARRAY:
		case GL_NORMALIZE:
		case GL_PACK_LSB_FIRST:
		case GL_PACK_SWAP_BYTES:
		case GL_POINT_SMOOTH:
		case GL_POINT_SPRITE:
		case GL_POLYGON_OFFSET_FILL:
		case GL_POLYGON_OFFSET_LINE:
		case GL_POLYGON_OFFSET_POINT:
		case GL_POLYGON_SMOOTH:
		case GL_POLYGON_STIPPLE:
		case GL_POST_COLOR_MATRIX_COLOR_TABLE:
		case GL_POST_CONVOLUTION_COLOR_TABLE:
		case GL_RESCALE_NORMAL:
		case GL_RGBA_MODE:
		case GL_SAMPLE_COVERAGE_INVERT:
		case GL_SCISSOR_TEST:
		case GL_SECONDARY_COLOR_ARRAY:
		case GL_SEPARABLE_2D:
		case GL_STENCIL_TEST:
		case GL_STEREO:
		case GL_TEXTURE_1D:
		case GL_TEXTURE_2D:
		case GL_TEXTURE_3D:
		case GL_TEXTURE_COORD_ARRAY:
		case GL_TEXTURE_CUBE_MAP:
		case GL_TEXTURE_GEN_Q:
		case GL_TEXTURE_GEN_R:
		case GL_TEXTURE_GEN_S:
		case GL_TEXTURE_GEN_T:
		case GL_UNPACK_LSB_FIRST:
		case GL_UNPACK_SWAP_BYTES:
		case GL_VERTEX_ARRAY:
		case GL_VERTEX_PROGRAM_POINT_SIZE:
		case GL_VERTEX_PROGRAM_TWO_SIDE:
		{
			GLboolean params[1];
			glGetBooleanv(pname, params);  OGL_CHK;
			*JL_RVAL = *params ? JSVAL_TRUE : JSVAL_FALSE;
			return JS_TRUE;
		}

		case GL_COLOR_WRITEMASK:
		{
			GLboolean params[4];
			glGetBooleanv(pname, params);  OGL_CHK;
			jsval ret[] = {
					params[0] ? JSVAL_TRUE : JSVAL_FALSE,
					params[1] ? JSVAL_TRUE : JSVAL_FALSE,
					params[2] ? JSVAL_TRUE : JSVAL_FALSE,
					params[3] ? JSVAL_TRUE : JSVAL_FALSE
			};
			*JL_RVAL = OBJECT_TO_JSVAL(JS_NewArrayObject(cx, COUNTOF(ret), ret));
			JL_CHK( *JL_RVAL != JSVAL_NULL );
			return JS_TRUE;
		}

		case GL_ACCUM_ALPHA_BITS: // uint
		case GL_ACCUM_BLUE_BITS: // uint
		case GL_ACCUM_GREEN_BITS: // uint
		case GL_ACCUM_RED_BITS: // uint
		case GL_ACTIVE_TEXTURE: // enum
		case GL_ALPHA_BITS: // uint
		case GL_ALPHA_TEST_FUNC: // enum
		case GL_ARRAY_BUFFER_BINDING: // name
		case GL_ATTRIB_STACK_DEPTH: // uint
		case GL_AUX_BUFFERS: // uint
		case GL_BLEND_DST_ALPHA: // enum
		case GL_BLEND_DST_RGB: // enum
		case GL_BLEND_EQUATION_RGB: // enum
		case GL_BLEND_EQUATION_ALPHA: // enum
		case GL_BLEND_SRC_ALPHA: // enum
		case GL_BLEND_SRC_RGB: // enum
		case GL_BLUE_BITS: // uint
		case GL_CLIENT_ACTIVE_TEXTURE: // enum
		case GL_CLIENT_ATTRIB_STACK_DEPTH: // uint
		case GL_COLOR_ARRAY_BUFFER_BINDING: // name
		case GL_COLOR_ARRAY_SIZE: // uint
		case GL_COLOR_ARRAY_STRIDE: // uint
		case GL_COLOR_ARRAY_TYPE: // enum
		case GL_COLOR_MATERIAL_FACE: // enum
		case GL_COLOR_MATERIAL_PARAMETER: // enum
		case GL_COLOR_MATRIX_STACK_DEPTH: // uint
		case GL_CULL_FACE_MODE: // enum
		case GL_CURRENT_PROGRAM: // uint
		case GL_DEPTH_BITS: // uint
		case GL_DEPTH_FUNC: // enum
		case GL_DRAW_BUFFER0: // enum
		case GL_DRAW_BUFFER1: // enum
		case GL_DRAW_BUFFER2: // enum
		case GL_DRAW_BUFFER3: // enum
		case GL_DRAW_BUFFER4: // enum
		case GL_DRAW_BUFFER5: // enum
		case GL_DRAW_BUFFER6: // enum
		case GL_DRAW_BUFFER7: // enum
		case GL_DRAW_BUFFER8: // enum
		case GL_DRAW_BUFFER9: // enum
		case GL_DRAW_BUFFER10: // enum
		case GL_DRAW_BUFFER11: // enum
		case GL_DRAW_BUFFER12: // enum
		case GL_DRAW_BUFFER13: // enum
		case GL_DRAW_BUFFER14: // enum
		case GL_DRAW_BUFFER15: // enum
		case GL_EDGE_FLAG_ARRAY_BUFFER_BINDING: // enum
		case GL_EDGE_FLAG_ARRAY_STRIDE: // uint
		case GL_ELEMENT_ARRAY_BUFFER_BINDING: // name
		case GL_FEEDBACK_BUFFER_SIZE: // uint
		case GL_FEEDBACK_BUFFER_TYPE: // enum
		case GL_FOG_COORD_ARRAY_BUFFER_BINDING:
		case GL_FOG_COORD_ARRAY_STRIDE: // uint
		case GL_FOG_COORD_ARRAY_TYPE: // enum
		case GL_FOG_COORD_SRC: // enum
		case GL_FOG_HINT: // enum
		case GL_FOG_MODE: // enum
		case GL_FRAGMENT_SHADER_DERIVATIVE_HINT: // enum
		case GL_FRONT_FACE: // enum
		case GL_GENERATE_MIPMAP_HINT: // enum
		case GL_GREEN_BITS: // uint
		case GL_INDEX_ARRAY_BUFFER_BINDING: // name
		case GL_INDEX_ARRAY_STRIDE: // uint
		case GL_INDEX_ARRAY_TYPE: // enum
		case GL_INDEX_BITS: // uint
		case GL_INDEX_OFFSET: // uint
		case GL_INDEX_SHIFT: // int
		case GL_INDEX_WRITEMASK: // uint
		case GL_LIGHT_MODEL_COLOR_CONTROL: // enum
		case GL_LINE_SMOOTH_HINT: // enum
		case GL_LINE_STIPPLE_PATTERN: // uint
		case GL_LINE_STIPPLE_REPEAT:
		case GL_LIST_BASE: // uint
		case GL_LIST_INDEX: // name
		case GL_LIST_MODE : // enum
		case GL_LOGIC_OP_MODE: // enum
		case GL_MAP1_GRID_SEGMENTS: // uint
		case GL_MATRIX_MODE: // enum
		case GL_MAX_3D_TEXTURE_SIZE: // uint
		case GL_MAX_CLIENT_ATTRIB_STACK_DEPTH: // uint
		case GL_MAX_ATTRIB_STACK_DEPTH: // uint
		case GL_MAX_CLIP_PLANES: // uint
		case GL_MAX_COLOR_MATRIX_STACK_DEPTH: // uint
		case GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS: // uint
		case GL_MAX_CUBE_MAP_TEXTURE_SIZE: // uint
		case GL_MAX_DRAW_BUFFERS: // uint
		case GL_MAX_ELEMENTS_INDICES: // uint
		case GL_MAX_ELEMENTS_VERTICES: // uint
		case GL_MAX_EVAL_ORDER : // uint
		case GL_MAX_FRAGMENT_UNIFORM_COMPONENTS : //uint
		case GL_MAX_LIGHTS: // uint
		case GL_MAX_LIST_NESTING: // uint
		case GL_MAX_MODELVIEW_STACK_DEPTH: // uint
		case GL_MAX_NAME_STACK_DEPTH: // uint
		case GL_MAX_PIXEL_MAP_TABLE: // uint
		case GL_MAX_PROJECTION_STACK_DEPTH: // uint
		case GL_MAX_TEXTURE_COORDS: // uint
		case GL_MAX_TEXTURE_IMAGE_UNITS: // uint
		case GL_MAX_TEXTURE_SIZE: // uint
		case GL_MAX_TEXTURE_STACK_DEPTH: // uint
		case GL_MAX_TEXTURE_UNITS: // uint
		case GL_MAX_VARYING_FLOATS: // uint
		case GL_MAX_VERTEX_ATTRIBS: // uint
		case GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS: // uint
		case GL_MAX_VERTEX_UNIFORM_COMPONENTS: // uint
		case GL_MODELVIEW_STACK_DEPTH: // uint
		case GL_NAME_STACK_DEPTH: // uint
		case GL_NORMAL_ARRAY_BUFFER_BINDING: // name
		case GL_NORMAL_ARRAY_STRIDE: // uint
		case GL_NORMAL_ARRAY_TYPE: // enum
		case GL_NUM_COMPRESSED_TEXTURE_FORMATS: // uint
		case GL_PACK_ALIGNMENT: // uint
		case GL_PACK_IMAGE_HEIGHT: // uint
		case GL_PACK_ROW_LENGTH: // uint
		case GL_PACK_SKIP_IMAGES: // uint
		case GL_PACK_SKIP_PIXELS: // uint
		case GL_PACK_SKIP_ROWS: // uint
		case GL_PERSPECTIVE_CORRECTION_HINT: // enum
		case GL_PIXEL_MAP_A_TO_A_SIZE: // uint
		case GL_PIXEL_MAP_B_TO_B_SIZE: // uint
		case GL_PIXEL_MAP_G_TO_G_SIZE: // uint
		case GL_PIXEL_MAP_I_TO_A_SIZE: // uint
		case GL_PIXEL_MAP_I_TO_B_SIZE: // uint
		case GL_PIXEL_MAP_I_TO_G_SIZE: // uint
		case GL_PIXEL_MAP_I_TO_I_SIZE: // uint
		case GL_PIXEL_MAP_I_TO_R_SIZE: // uint
		case GL_PIXEL_MAP_R_TO_R_SIZE: // uint
		case GL_PIXEL_MAP_S_TO_S_SIZE: // uint
		case GL_PIXEL_PACK_BUFFER_BINDING: // name
		case GL_PIXEL_UNPACK_BUFFER_BINDING: // name
		case GL_POINT_SMOOTH_HINT: // enum
		case GL_POLYGON_SMOOTH_HINT: // enum
		case GL_PROJECTION_STACK_DEPTH: // uint
		case GL_READ_BUFFER: // enum
		case GL_RED_BITS: // uint
		case GL_RENDER_MODE: // enum
		case GL_SAMPLE_BUFFERS: // uint
		case GL_SAMPLES: // uint
		case GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING: // name
		case GL_SECONDARY_COLOR_ARRAY_SIZE: // uint
		case GL_SECONDARY_COLOR_ARRAY_STRIDE: // int
		case GL_SECONDARY_COLOR_ARRAY_TYPE: // enum
		case GL_SELECTION_BUFFER_SIZE: // uint
		case GL_SHADE_MODEL: // enum
		case GL_STENCIL_BACK_FAIL: // enum
		case GL_STENCIL_BACK_FUNC: // enum
		case GL_STENCIL_BACK_PASS_DEPTH_FAIL: // enum
		case GL_STENCIL_BACK_PASS_DEPTH_PASS: // enum
		case GL_STENCIL_BACK_REF: // uint
		case GL_STENCIL_BACK_VALUE_MASK: // uint
		case GL_STENCIL_BACK_WRITEMASK: // uint
		case GL_STENCIL_BITS: // uint
		case GL_STENCIL_CLEAR_VALUE:
		case GL_STENCIL_FAIL: // enum
		case GL_STENCIL_FUNC: // enum
		case GL_STENCIL_PASS_DEPTH_FAIL: // enum
		case GL_STENCIL_PASS_DEPTH_PASS: // enum
		case GL_STENCIL_REF: // enum
		case GL_STENCIL_VALUE_MASK: // enum
		case GL_STENCIL_WRITEMASK: // uint
		case GL_SUBPIXEL_BITS:
		case GL_TEXTURE_BINDING_1D: // name
		case GL_TEXTURE_BINDING_2D: // name
		case GL_TEXTURE_BINDING_3D: // name
		case GL_TEXTURE_BINDING_CUBE_MAP: // name
		case GL_TEXTURE_COMPRESSION_HINT: // enum
		case GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING: // name
		case GL_TEXTURE_COORD_ARRAY_SIZE:
		case GL_TEXTURE_COORD_ARRAY_STRIDE:
		case GL_TEXTURE_COORD_ARRAY_TYPE: // enum
		case GL_TEXTURE_STACK_DEPTH:
		case GL_UNPACK_ALIGNMENT:
		case GL_UNPACK_IMAGE_HEIGHT:
		case GL_UNPACK_ROW_LENGTH:
		case GL_UNPACK_SKIP_IMAGES:
		case GL_UNPACK_SKIP_PIXELS:
		case GL_UNPACK_SKIP_ROWS:
		case GL_VERTEX_ARRAY_BUFFER_BINDING:
		case GL_VERTEX_ARRAY_SIZE:
		case GL_VERTEX_ARRAY_STRIDE:
		case GL_VERTEX_ARRAY_TYPE: // enum
		case GL_ZOOM_X:
		case GL_ZOOM_Y:
		{
			GLint params[1];
			glGetIntegerv(pname, params);  OGL_CHK;
			*JL_RVAL = INT_TO_JSVAL(*params);
			return JS_TRUE;
		}

		case GL_MAP2_GRID_SEGMENTS:
		case GL_MAX_VIEWPORT_DIMS:
		case GL_POLYGON_MODE: // enum
		{
			GLint params[2];
			glGetIntegerv(pname, params);  OGL_CHK;
			jsval jsparams[] = {
				INT_TO_JSVAL(params[0]),
				INT_TO_JSVAL(params[1])
			};
			*JL_RVAL = OBJECT_TO_JSVAL( JS_NewArrayObject(cx, COUNTOF(jsparams), jsparams) );
			JL_CHK( *JL_RVAL != JSVAL_NULL );
			return JS_TRUE;
		}

		case GL_SCISSOR_BOX:
		case GL_VIEWPORT:
		{
			GLint params[4];
			glGetIntegerv(pname, params);  OGL_CHK;
			jsval jsparams[] = {
				INT_TO_JSVAL(params[0]),
				INT_TO_JSVAL(params[1]),
				INT_TO_JSVAL(params[2]),
				INT_TO_JSVAL(params[3])
			};
			*JL_RVAL = OBJECT_TO_JSVAL( JS_NewArrayObject(cx, COUNTOF(jsparams), jsparams) );
			JL_CHK( *JL_RVAL != JSVAL_NULL );
			return JS_TRUE;
		}

		case GL_ALPHA_BIAS:
		case GL_ALPHA_SCALE:
		case GL_ALPHA_TEST_REF:
		case GL_BLUE_BIAS:
		case GL_BLUE_SCALE:
		case GL_CURRENT_FOG_COORD:
		case GL_CURRENT_INDEX:
		case GL_CURRENT_RASTER_DISTANCE:
		case GL_CURRENT_RASTER_INDEX:
		case GL_DEPTH_BIAS:
		case GL_DEPTH_CLEAR_VALUE:
		case GL_DEPTH_RANGE:
		case GL_DEPTH_SCALE:
		case GL_FOG_DENSITY:
		case GL_FOG_END:
		case GL_FOG_INDEX:
		case GL_FOG_START:
		case GL_GREEN_BIAS:
		case GL_GREEN_SCALE:
		case GL_LINE_WIDTH:
		case GL_LINE_WIDTH_GRANULARITY: // GL_SMOOTH_LINE_WIDTH_GRANULARITY
		case GL_MAX_TEXTURE_LOD_BIAS:
		case GL_POINT_FADE_THRESHOLD_SIZE:
		case GL_POINT_SIZE:
		case GL_POINT_SIZE_GRANULARITY:
		case GL_POINT_SIZE_MAX:
		case GL_POINT_SIZE_MIN:
		case GL_POLYGON_OFFSET_FACTOR:
		case GL_POLYGON_OFFSET_UNITS:
		case GL_POST_COLOR_MATRIX_RED_BIAS:
		case GL_POST_COLOR_MATRIX_GREEN_BIAS:
		case GL_POST_COLOR_MATRIX_BLUE_BIAS:
		case GL_POST_COLOR_MATRIX_ALPHA_BIAS:
		case GL_POST_COLOR_MATRIX_RED_SCALE:
		case GL_POST_COLOR_MATRIX_GREEN_SCALE:
		case GL_POST_COLOR_MATRIX_BLUE_SCALE:
		case GL_POST_COLOR_MATRIX_ALPHA_SCALE:
		case GL_POST_CONVOLUTION_RED_BIAS:
		case GL_POST_CONVOLUTION_GREEN_BIAS:
		case GL_POST_CONVOLUTION_BLUE_BIAS:
		case GL_POST_CONVOLUTION_ALPHA_BIAS:
		case GL_POST_CONVOLUTION_RED_SCALE:
		case GL_POST_CONVOLUTION_GREEN_SCALE:
		case GL_POST_CONVOLUTION_BLUE_SCALE:
		case GL_POST_CONVOLUTION_ALPHA_SCALE:
		case GL_RED_BIAS:
		case GL_RED_SCALE:
		case GL_SAMPLE_COVERAGE_VALUE:
		{
			GLdouble params[1];
			glGetDoublev(pname, params);  OGL_CHK;
			return JL_NativeToJsval(cx, *params, JL_RVAL);
		}

		case GL_ALIASED_POINT_SIZE_RANGE:
		case GL_ALIASED_LINE_WIDTH_RANGE:
		case GL_LINE_WIDTH_RANGE: // GL_SMOOTH_LINE_WIDTH_RANGE
		case GL_MAP1_GRID_DOMAIN:
		case GL_POINT_SIZE_RANGE: // GL_SMOOTH_POINT_SIZE_RANGE
		{
			GLdouble params[2];
			glGetDoublev(pname, params);  OGL_CHK;
			return JL_CValVectorToJsval(cx, params, COUNTOF(params), JL_RVAL);
		}

		case GL_CURRENT_NORMAL:
		case GL_POINT_DISTANCE_ATTENUATION:
		{
			GLdouble params[3];
			glGetDoublev(pname, params);  OGL_CHK;
			return JL_CValVectorToJsval(cx, params, COUNTOF(params), JL_RVAL);
		}

		case GL_ACCUM_CLEAR_VALUE:
		case GL_BLEND_COLOR:
		case GL_COLOR_CLEAR_VALUE:
		case GL_CURRENT_COLOR:
		case GL_CURRENT_RASTER_COLOR:
		case GL_CURRENT_RASTER_POSITION:
		case GL_CURRENT_RASTER_SECONDARY_COLOR:
		case GL_CURRENT_RASTER_TEXTURE_COORDS:
		case GL_CURRENT_SECONDARY_COLOR:
		case GL_CURRENT_TEXTURE_COORDS:
		case GL_FOG_COLOR:
		case GL_LIGHT_MODEL_AMBIENT:
		case GL_MAP2_GRID_DOMAIN:
		{
			GLdouble params[4];
			glGetDoublev(pname, params);  OGL_CHK;
			return JL_CValVectorToJsval(cx, params, COUNTOF(params), JL_RVAL);
		}

		case GL_COLOR_MATRIX:
		case GL_MODELVIEW_MATRIX:
		case GL_PROJECTION_MATRIX:
		case GL_TEXTURE_MATRIX:
		case GL_TRANSPOSE_COLOR_MATRIX:
		case GL_TRANSPOSE_MODELVIEW_MATRIX:
		case GL_TRANSPOSE_PROJECTION_MATRIX:
		case GL_TRANSPOSE_TEXTURE_MATRIX:
		{
			GLdouble params[16];
			glGetDoublev(pname, params);  OGL_CHK;
			return JL_CValVectorToJsval(cx, params, COUNTOF(params), JL_RVAL);
		}

		case GL_COMPRESSED_TEXTURE_FORMATS: // enum
		{
			GLint count;
			glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &count);
			GLint *params = (GLint*)alloca(count * sizeof(GLenum));
			glGetIntegerv(pname, params);
			return JL_CValVectorToJsval(cx, params, count, JL_RVAL);
		}
	}

	JL_REPORT_ERROR("Unknown pname");
	JL_BAD;
}


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
DEFINE_FUNCTION( GetBoolean ) {

	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT_INT(JL_ARG(1));
	GLboolean params;
	glGetBooleanv(JSVAL_TO_INT(JL_ARG(1)), &params);  OGL_CHK;
	*JL_RVAL = BOOLEAN_TO_JSVAL(params);
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
DEFINE_FUNCTION( GetInteger ) {

	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT_INT(JL_ARG(1));

	GLint params[16]; // (TBD) check if it is the max amount of data that glGetIntegerv may returns.
	glGetIntegerv(JSVAL_TO_INT( JL_ARG(1) ), params);  OGL_CHK;

	if ( JL_ARG_ISDEF(2) ) {

		JL_S_ASSERT_INT( JL_ARG(2) );
		int count = JSVAL_TO_INT( JL_ARG(2) );
		JSObject *arrayObj = JS_NewArrayObject(cx, 0, NULL);
		JL_CHK(arrayObj);
		*JL_RVAL = OBJECT_TO_JSVAL(arrayObj);
		jsval tmpValue;
		while (count--) {

			tmpValue = INT_TO_JSVAL( params[count] );
			JL_CHK( JS_SetElement(cx, arrayObj, count, &tmpValue) );
		}
	} else {

		*JL_RVAL = INT_TO_JSVAL( params[0] );
	}
	;
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
DEFINE_FUNCTION( GetDouble ) {

	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT_INT(JL_ARG(1));

	GLdouble params[16]; // (TBD) check if it is the max amount of data that glGetDoublev may returns.
	glGetDoublev(JSVAL_TO_INT(JL_ARG(1)), params);  OGL_CHK;

	if ( JL_ARG_ISDEF(2) ) {

		JL_S_ASSERT_INT( JL_ARG(2) );
		int count = JSVAL_TO_INT( JL_ARG(2) );
		JSObject *arrayObj = JS_NewArrayObject(cx, 0, NULL);
		JL_CHK(arrayObj);
		*JL_RVAL = OBJECT_TO_JSVAL(arrayObj);
		jsval tmpValue;
		while (count--) {

			JL_CHK( JL_NativeToJsval(cx, params[count], &tmpValue) );
			JL_CHK( JS_SetElement(cx, arrayObj, count, &tmpValue) );
		}
	} else {

		JL_CHK( JL_NativeToJsval(cx, params[0], JL_RVAL) );
	}
	;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
  $STR$INAME( name )
  $H arguments
   $ARG GLenum name
  $H return value
   A string describing the current GL connection.
  $H OpenGL API
   glGetDoublev

**/
DEFINE_FUNCTION( GetString ) {

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_INT(JL_ARG(1));
	return JL_NativeToJsval(cx, (char*)glGetString(JSVAL_TO_INT(JL_ARG(1))), JL_RVAL);  OGL_CHK;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( mode )
  $H OpenGL API
   glDrawBuffer
**/
DEFINE_FUNCTION( DrawBuffer ) {

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_INT(JL_ARG(1));
	GLenum mode = JSVAL_TO_INT(JL_ARG(1));
	
	glDrawBuffer(mode);  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
	;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( mode )
  $H OpenGL API
   glReadBuffer
**/
DEFINE_FUNCTION( ReadBuffer ) {

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_INT(JL_ARG(1));
	GLenum mode = JSVAL_TO_INT(JL_ARG(1));
	
	glReadBuffer(mode);  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;

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
DEFINE_FUNCTION( Accum ) {

	JL_S_ASSERT_ARG(2);
	JL_S_ASSERT_INT(JL_ARG(1));
	GLenum op = JSVAL_TO_INT(JL_ARG(1));
	float value;
	JL_JsvalToNative(cx, JL_ARG(2), &value);
	
	glAccum(op, value);  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( func, ref, mask )
  $H arguments
   $ARG GLenum func
   $ARG $INT ref
   $ARG $UINT mask
  $H note
   if mask is -1, 0xffffffff value is used as mask.
  $H OpenGL API
   glStencilFunc
**/
DEFINE_FUNCTION( StencilFunc ) {

	JL_S_ASSERT_ARG(3);
	JL_S_ASSERT_INT(JL_ARG(1));
	JL_S_ASSERT_INT(JL_ARG(2));
	JL_S_ASSERT_NUMBER(JL_ARG(3));

	GLuint mask;
	if ( JL_ARG(3) == INT_TO_JSVAL(-1) )
		mask = 0xffffffff;
	else
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &mask) );

	glStencilFunc(JSVAL_TO_INT(JL_ARG(1)), JSVAL_TO_INT(JL_ARG(2)), mask);  OGL_CHK;

	*JL_RVAL = JSVAL_VOID;

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( fail, zfail, zpass )
  $H arguments
   $ARG GLenum fail
   $ARG GLenum zfail
   $ARG GLenum zpass
  $H OpenGL API
   glStencilOp
**/
DEFINE_FUNCTION( StencilOp ) {

	JL_S_ASSERT_ARG(3);
	JL_S_ASSERT_INT(JL_ARG(1));
	JL_S_ASSERT_INT(JL_ARG(2));
	JL_S_ASSERT_INT(JL_ARG(3));

	glStencilOp(JSVAL_TO_INT(JL_ARG(1)), JSVAL_TO_INT(JL_ARG(2)), JSVAL_TO_INT(JL_ARG(3)));  OGL_CHK;

	*JL_RVAL = JSVAL_VOID;

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( mask )
  $H arguments
   $ARG GLenum func
  $H note
   if mask is -1, 0xffffffff value is used as mask.
  $H OpenGL API
   glStencilMask
**/
DEFINE_FUNCTION( StencilMask ) {

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_NUMBER(JL_ARG(1));

	GLuint mask;
	if ( JL_ARG(1) == INT_TO_JSVAL(-1) )
		mask = 0xffffffff;
	else
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &mask) );
	
	glStencilMask( mask );  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
	;
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
DEFINE_FUNCTION( AlphaFunc ) {

	JL_S_ASSERT_ARG(2);
	JL_S_ASSERT_INT(JL_ARG(1));
	float ref;
	JL_JsvalToNative(cx, JL_ARG(2), &ref);

	glAlphaFunc( JSVAL_TO_INT(JL_ARG(1)), ref );  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  $H OpenGL API
   glFlush
**/
DEFINE_FUNCTION( Flush ) {

	glFlush();  OGL_CHK;

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  $H OpenGL API
   glFinish
**/
DEFINE_FUNCTION( Finish ) {

	glFinish();  OGL_CHK;

	*JL_RVAL = JSVAL_VOID;
	;
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
   glFogi, glFogf, glFogfv
**/
DEFINE_FUNCTION( Fog ) {

	JL_S_ASSERT_ARG(2);
	JL_S_ASSERT_INT(JL_ARG(1));

	*JL_RVAL = JSVAL_VOID;
	if ( JSVAL_IS_INT(JL_ARG(2)) ) {

		glFogi(JSVAL_TO_INT(JL_ARG(1)), JSVAL_TO_INT(JL_ARG(2)));  OGL_CHK;

		;
		return JS_TRUE;
	}
	if ( JSVAL_IS_DOUBLE(JL_ARG(2)) ) {

		float param;
		JL_JsvalToNative(cx, JL_ARG(2), &param);
		
		glFogf( JSVAL_TO_INT(JL_ARG(1)), param );  OGL_CHK;
		
		;
		return JS_TRUE;
	}
	if ( JL_IsArray(cx, JL_ARG(2)) ) {

		GLfloat params[16];
		uint32 length;
		JL_CHK( JL_JsvalToCValVector(cx, JL_ARG(2), params, COUNTOF(params), &length ) );

		glFogfv( JSVAL_TO_INT(JL_ARG(1)), params );  OGL_CHK;

		;
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
DEFINE_FUNCTION( Hint ) {

	JL_S_ASSERT_ARG(2);
	JL_S_ASSERT_INT(JL_ARG(1));
	JL_S_ASSERT_INT(JL_ARG(2));

	glHint( JSVAL_TO_INT(JL_ARG(1)), JSVAL_TO_INT(JL_ARG(2)) );  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
	;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( x, y [, z [, w]] )
 $VOID $INAME( $TYPE vec3 )
  $H arguments
   $ARG $REAL x
   $ARG $REAL y
   $ARG $REAL z
   $ARG $REAL w
  $H OpenGL API
   glVertex3d, glVertex2d
**/
DEFINE_FUNCTION( Vertex ) {
	
	*JL_RVAL = JSVAL_VOID;

	if ( argc > 1 || JSVAL_IS_NUMBER(JL_ARG(1)) ) {
		
		JL_S_ASSERT_ARG_RANGE(2,4);

		double x, y, z, w;
		JL_JsvalToNative(cx, JL_ARG(1), &x);
		JL_JsvalToNative(cx, JL_ARG(2), &y);
		if ( JL_ARGC >= 3 ) {

			JL_JsvalToNative(cx, JL_ARG(3), &z);
			if ( JL_ARGC >= 4 ) {
	
				JL_JsvalToNative(cx, JL_ARG(4), &w);
				glVertex4d(x, y, z, w);  OGL_CHK;
				return JS_TRUE;
			}
			glVertex3d(x, y, z);  OGL_CHK;
			return JS_TRUE;
		}
		glVertex2d(x, y);  OGL_CHK;
		return JS_TRUE;
	}
	
	JL_S_ASSERT_ARG(1);

	GLdouble pos[4];
	uint32 len;
	JL_JsvalToCValVector(cx, JL_ARG(1), pos, COUNTOF(pos), &len);
	if ( len == 2 ) {
		glVertex2dv(pos);  OGL_CHK;
	} else if ( len == 3 ) {
		glVertex3dv(pos);  OGL_CHK;
	} else if ( len == 4 ) {
		glVertex4dv(pos);  OGL_CHK;
	} else {
		JL_REPORT_ERROR("Unexpected array length.");
	}

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( flag )
  $H arguments
   $ARG $BOOL flag
  $H OpenGL API
   glEdgeFlag
**/
DEFINE_FUNCTION( EdgeFlag ) {

	bool flag;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &flag) );
	glEdgeFlag(flag);
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( red, green, blue [, alpha] )
 $VOID $INAME( colorArray )
  $H arguments
   $ARG $REAL red
   $ARG $REAL green
   $ARG $REAL blue
   $ARG $REAL alpha
  $H OpenGL API
   glColor4d, glColor3d
**/
DEFINE_FUNCTION( Color ) {

	JL_S_ASSERT_ARG_MIN(1);
	*JL_RVAL = JSVAL_VOID;

	if ( argc > 1 || JSVAL_IS_NUMBER(JL_ARG(1)) ) {

		JL_S_ASSERT_ARG_MAX(4);

		double r, g, b, a;
		JL_JsvalToNative(cx, JL_ARG(1), &r);
		if ( argc == 1 ) {
			
			glColor3d(r, r, r);  OGL_CHK;
			;
			return JS_TRUE;
		}		
		JL_JsvalToNative(cx, JL_ARG(2), &g);
		JL_JsvalToNative(cx, JL_ARG(3), &b);
		if ( argc == 3 ) {

			glColor3d(r, g, b);  OGL_CHK;
			;
			return JS_TRUE;
		}		
		JL_JsvalToNative(cx, JL_ARG(4), &a);
		glColor4d(r, g, b, a);  OGL_CHK;
		;
	} else {

		JL_S_ASSERT_ARG(1);

		GLdouble color[4];
		uint32 len;
		JL_JsvalToCValVector(cx, JL_ARG(1), color, 4, &len);
		if ( len == 3 ) {
			glColor3dv(color);  OGL_CHK;
		} else if ( len == 4 ) {
			glColor4dv(color);  OGL_CHK;
		} else {
			JL_REPORT_ERROR("Unexpected array length.");
		}
	}
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
DEFINE_FUNCTION( Normal ) {

	JL_S_ASSERT_ARG(3);
	double nx, ny, nz;
	JL_JsvalToNative(cx, JL_ARG(1), &nx);
	JL_JsvalToNative(cx, JL_ARG(2), &ny);
	JL_JsvalToNative(cx, JL_ARG(3), &nz);

	glNormal3d(nx, ny, nz);  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
	;
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
DEFINE_FUNCTION( TexCoord ) {

	JL_S_ASSERT_ARG_RANGE(1,3);
	*JL_RVAL = JSVAL_VOID;
	double s;
	JL_JsvalToNative(cx, JL_ARG(1), &s);
	if ( JL_ARGC == 1 ) {

		glTexCoord1d(s);  OGL_CHK;

		;
		return JS_TRUE;
	}
	double t;
	JL_JsvalToNative(cx, JL_ARG(2), &t);
	if ( JL_ARGC == 2 ) {

		glTexCoord2d(s, t);  OGL_CHK;

		;
		return JS_TRUE;
	}
	double r;
	JL_JsvalToNative(cx, JL_ARG(3), &r);

	glTexCoord3d(s, t, r);  OGL_CHK;

	;
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
DEFINE_FUNCTION( TexParameter ) {

	JL_S_ASSERT_ARG(3);
	JL_S_ASSERT_INT(JL_ARG(1));
	JL_S_ASSERT_INT(JL_ARG(2));
	JL_S_ASSERT_NUMBER(JL_ARG(3));

	*JL_RVAL = JSVAL_VOID;
	if ( JSVAL_IS_INT(JL_ARG(3)) ) {

		glTexParameteri( JSVAL_TO_INT( JL_ARG(1) ), JSVAL_TO_INT( JL_ARG(2) ), JSVAL_TO_INT( JL_ARG(3) ) );  OGL_CHK;
		return JS_TRUE;
	}
	if ( JSVAL_IS_DOUBLE(JL_ARG(3)) ) {

		float param;
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &param) );
		glTexParameterf( JSVAL_TO_INT( JL_ARG(1) ), JSVAL_TO_INT( JL_ARG(2) ), param );  OGL_CHK;
		return JS_TRUE;
	}
	if ( JL_IsArray(cx, JL_ARG(3)) ) {

		GLfloat params[16];
		uint32 length;
		JL_CHK( JL_JsvalToCValVector(cx, JL_ARG(3), params, COUNTOF(params), &length ) );
		glTexParameterfv( JSVAL_TO_INT(JL_ARG(1)), JSVAL_TO_INT(JL_ARG(2)), params );  OGL_CHK;
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
DEFINE_FUNCTION( TexEnv ) {

	JL_S_ASSERT_ARG_MIN(3);
	JL_S_ASSERT_INT(JL_ARG(1));
	JL_S_ASSERT_INT(JL_ARG(2));

	*JL_RVAL = JSVAL_VOID;
	if ( argc == 3 && JSVAL_IS_INT(JL_ARG(3)) ) {

		glTexEnvi( JSVAL_TO_INT( JL_ARG(1) ), JSVAL_TO_INT( JL_ARG(2) ), JSVAL_TO_INT( JL_ARG(3) ) );  OGL_CHK;
		return JS_TRUE;
	}
	if ( argc == 3 && JSVAL_IS_DOUBLE(JL_ARG(3)) ) {

		float param;
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &param) );
		glTexEnvf( JSVAL_TO_INT(JL_ARG(1)), JSVAL_TO_INT(JL_ARG(2)), param );  OGL_CHK;
		return JS_TRUE;
	}

	GLfloat params[16];
	if ( argc == 3 && JL_IsArray(cx, JL_ARG(3)) ) {

		uint32 length;
		JL_CHK( JL_JsvalToCValVector(cx, JL_ARG(3), params, COUNTOF(params), &length ) );
		glTexEnvfv( JSVAL_TO_INT(JL_ARG(1)), JSVAL_TO_INT(JL_ARG(2)), params );  OGL_CHK;
		return JS_TRUE;
	}

	JL_S_ASSERT_ARG_MIN( 3 ); // at least
	JL_ASSERT( argc-2 < COUNTOF(params) );
	for ( unsigned int i = 2; i < argc; ++i )
		JL_JsvalToNative(cx, JL_ARGV[i], &params[i-2]);
	glTexEnvfv( JSVAL_TO_INT(JL_ARG(1)), JSVAL_TO_INT(JL_ARG(2)), params );  OGL_CHK;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( coord, pname, [ param | paramArray | param1, ..., paramN  ] )
  $H arguments
   $ARG GLenum target
   $ARG GLenum pname
   $ARG $VAL params: is either a number or an array of numbers.
  $H OpenGL API
   glTexGeni, glTexGend, glTexGendv
**/
DEFINE_FUNCTION( TexGen ) {

	JL_S_ASSERT_ARG_MIN(3);
	JL_S_ASSERT_INT(JL_ARG(1));
	JL_S_ASSERT_INT(JL_ARG(2));

	*JL_RVAL = JSVAL_VOID;
	if ( argc == 3 && JSVAL_IS_INT(JL_ARG(3)) ) {

		glTexGeni( JSVAL_TO_INT( JL_ARG(1) ), JSVAL_TO_INT( JL_ARG(2) ), JSVAL_TO_INT( JL_ARG(3) ) );  OGL_CHK;
		return JS_TRUE;
	}
	if ( argc == 3 && JSVAL_IS_DOUBLE(JL_ARG(3)) ) {

		double param;
		JL_JsvalToNative(cx, JL_ARG(3), &param);
		glTexGend( JSVAL_TO_INT(JL_ARG(1)), JSVAL_TO_INT(JL_ARG(2)), param );  OGL_CHK;
		return JS_TRUE;
	}

	GLdouble params[16];
	if ( argc == 3 && JL_IsArray(cx, JL_ARG(3)) ) {

		uint32 length;
		JL_CHK( JL_JsvalToCValVector(cx, JL_ARG(3), params, COUNTOF(params), &length ) );
		glTexGendv( JSVAL_TO_INT(JL_ARG(1)), JSVAL_TO_INT(JL_ARG(2)), params );  OGL_CHK;
		return JS_TRUE;
	}

	JL_S_ASSERT_ARG_MIN( 3 ); // at least
	JL_ASSERT( argc-2 < COUNTOF(params) );
	for ( unsigned int i = 2; i < argc; ++i )
		JL_JsvalToNative(cx, JL_ARGV[i], &params[i-2]);
	glTexGendv( JSVAL_TO_INT(JL_ARG(1)), JSVAL_TO_INT(JL_ARG(2)), params );  OGL_CHK;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( target, level, internalFormat, width, height, border, format, type [, data] )
  $H arguments
	$ARG GLenum target
 	$ARG GLint level
 	$ARG GLint internalFormat
 	$ARG GLsizei width
 	$ARG GLsizei height
 	$ARG GLint border
 	$ARG GLenum format
 	$ARG GLenum type
 	$ARG Blob data
  $H OpenGL API
   glTexImage2D
  $H OpenGL documentation
   http://www.opengl.org/sdk/docs/man/xhtml/glTexImage2D.xml
**/
DEFINE_FUNCTION( TexImage2D ) {

	JLStr data;
	JL_S_ASSERT_ARG_RANGE(8, 9);
	JL_S_ASSERT_INT(JL_ARG(1));
	JL_S_ASSERT_INT(JL_ARG(2));
	JL_S_ASSERT_INT(JL_ARG(3));
	JL_S_ASSERT_INT(JL_ARG(4));
	JL_S_ASSERT_INT(JL_ARG(5));
	JL_S_ASSERT_INT(JL_ARG(6));
	JL_S_ASSERT_INT(JL_ARG(7));
	JL_S_ASSERT_INT(JL_ARG(8));

	if ( JL_ARG_ISDEF(9) && !JSVAL_IS_NULL(JL_ARG(9)) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(9), &data) );

	glTexImage2D( JSVAL_TO_INT(JL_ARG(1)), JSVAL_TO_INT(JL_ARG(2)), JSVAL_TO_INT(JL_ARG(3)), JSVAL_TO_INT(JL_ARG(4)), JSVAL_TO_INT(JL_ARG(5)), JSVAL_TO_INT(JL_ARG(6)), JSVAL_TO_INT(JL_ARG(7)), JSVAL_TO_INT(JL_ARG(8)), (GLvoid*)data.GetConstStr() );  OGL_CHK;

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( target, level, xoffset, yoffset, x, y, width, height )
  $H arguments
 	$ARG GLenum target
 	$ARG GLint level
 	$ARG GLint xoffset
 	$ARG GLint yoffset
 	$ARG GLint x
 	$ARG GLint y
 	$ARG GLsizei width
 	$ARG GLsizei height
  $H OpenGL API
   glCopyTexSubImage2D
  $H OpenGL documentation
**/
DEFINE_FUNCTION( CopyTexSubImage2D ) {

	JL_S_ASSERT_ARG(8);
	JL_S_ASSERT_INT(JL_ARG(1));
	JL_S_ASSERT_INT(JL_ARG(2));
	JL_S_ASSERT_INT(JL_ARG(3));
	JL_S_ASSERT_INT(JL_ARG(4));
	JL_S_ASSERT_INT(JL_ARG(5));
	JL_S_ASSERT_INT(JL_ARG(6));
	JL_S_ASSERT_INT(JL_ARG(7));
	JL_S_ASSERT_INT(JL_ARG(8));

	glCopyTexSubImage2D( JSVAL_TO_INT(JL_ARG(1)), JSVAL_TO_INT(JL_ARG(2)), JSVAL_TO_INT(JL_ARG(3)), JSVAL_TO_INT(JL_ARG(4)), JSVAL_TO_INT(JL_ARG(5)), JSVAL_TO_INT(JL_ARG(6)), JSVAL_TO_INT(JL_ARG(7)), JSVAL_TO_INT(JL_ARG(8)) );  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
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
DEFINE_FUNCTION( LightModel ) {

	JL_S_ASSERT_ARG(2);
	JL_S_ASSERT_INT(JL_ARG(1));

	*JL_RVAL = JSVAL_VOID;
	if ( JSVAL_IS_INT(JL_ARG(2)) ) {

		glLightModeli( JSVAL_TO_INT( JL_ARG(1) ), JSVAL_TO_INT( JL_ARG(2) ) );  OGL_CHK;
		return JS_TRUE;
	}

	if ( JSVAL_IS_DOUBLE(JL_ARG(2)) ) {

		float param;
		JL_JsvalToNative(cx, JL_ARG(2), &param);
		glLightModelf( JSVAL_TO_INT( JL_ARG(1) ), param );  OGL_CHK;
		return JS_TRUE;
	}

	if ( JL_IsArray(cx, JL_ARG(2)) ) {

		GLfloat params[16];
		uint32 length;
		JL_CHK( JL_JsvalToCValVector(cx, JL_ARG(2), params, COUNTOF(params), &length ) );
		glLightModelfv( JSVAL_TO_INT(JL_ARG(1)), params );  OGL_CHK;
		return JS_TRUE;
	}

	JL_REPORT_ERROR("Invalid argument.");
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( light, pname, [ paramArray | param1, ... paramN ] )
  $H arguments
   $ARG GLenum light
   $ARG GLenum pname
   $ARG $VAL params: is either a number or an array of numbers.
  $H OpenGL API
   glLighti, glLightf, glLightfv
**/
DEFINE_FUNCTION( Light ) {

	JL_S_ASSERT_ARG_MIN(3);
	JL_S_ASSERT_INT(JL_ARG(1));
	JL_S_ASSERT_INT(JL_ARG(2));

	*JL_RVAL = JSVAL_VOID;
	if ( argc == 3 && JSVAL_IS_INT(JL_ARG(3)) ) {

		glLighti( JSVAL_TO_INT( JL_ARG(1) ), JSVAL_TO_INT( JL_ARG(2) ), JSVAL_TO_INT( JL_ARG(3) ) );  OGL_CHK;
		return JS_TRUE;
	}

	if ( argc == 3 && JSVAL_IS_DOUBLE(JL_ARG(3)) ) {

		float param;
		JL_JsvalToNative(cx, JL_ARG(3), &param);
		glLightf( JSVAL_TO_INT( JL_ARG(1) ), JSVAL_TO_INT( JL_ARG(2) ), param );  OGL_CHK;
		return JS_TRUE;
	}

	GLfloat params[16];
	if ( argc == 3 && JL_IsArray(cx, JL_ARG(3)) ) {

		uint32 length;
		JL_CHK( JL_JsvalToCValVector(cx, JL_ARG(3), params, COUNTOF(params), &length ) );
		glLightfv( JSVAL_TO_INT(JL_ARG(1)), JSVAL_TO_INT(JL_ARG(2)), params );  OGL_CHK;
		return JS_TRUE;
	}

	JL_S_ASSERT_ARG_MIN( 3 ); // at least
	JL_ASSERT( argc-2 < COUNTOF(params) );
	for ( unsigned int i = 2; i < argc; ++i )
		JL_JsvalToNative(cx, JL_ARGV[i], &params[i-2]);
	glLightfv( JSVAL_TO_INT(JL_ARG(1)), JSVAL_TO_INT(JL_ARG(2)), params );  OGL_CHK;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $REAL | $ARRAY $INAME( light, pname [, count] )
  Return light source parameter values.
  $H arguments
   $ARG GLenum light: Specifies a light source.
   $ARG GLenum pname: Specifies a light source parameter for light.
   $ARG $INT count: is the number of expected values. If _count_ is defined, the function will returns an array of values, else a single value.
  $H return value
   A single value or an Array of values of the selected parameter.
  $H OpenGL API
   glGetLightfv
**/
DEFINE_FUNCTION( GetLight ) {

	JL_S_ASSERT_ARG_RANGE(2,3);
	JL_S_ASSERT_INT(JL_ARG(1));
	JL_S_ASSERT_INT(JL_ARG(2));

	GLfloat params[16]; // max ?  4 ?
	GLenum pname;
	pname = JSVAL_TO_INT(JL_ARG(2));
	glGetLightfv(JSVAL_TO_INT(JL_ARG(1)), pname, params);  OGL_CHK;

	int count;
	if ( JL_ARG_ISDEF(3) ) {
		
		JL_S_ASSERT_INT(JL_ARG(3));
		count = JSVAL_TO_INT(JL_ARG(3));
	} else {

		switch ( pname ) {
			case GL_AMBIENT:
			case GL_DIFFUSE:
			case GL_SPECULAR:
			case GL_POSITION:
				count = 4;
				break;
			case GL_SPOT_DIRECTION:
				count = 3;
				break;
			case GL_SPOT_EXPONENT:
			case GL_SPOT_CUTOFF:
			case GL_CONSTANT_ATTENUATION:
			case GL_LINEAR_ATTENUATION:
			case GL_QUADRATIC_ATTENUATION:
				count = 1;
				break;
			default:
				JL_REPORT_ERROR("Unknown parameter count.");
		}
	}

	if ( count > 1 ) {

		JSObject *arrayObj = JS_NewArrayObject(cx, count, NULL);
		JL_CHK( arrayObj );
		*JL_RVAL = OBJECT_TO_JSVAL(arrayObj);
		jsval tmpValue;
		while ( count-- ) {

			JL_CHK(JL_NativeToJsval(cx, params[count], &tmpValue) );
			JL_CHK( JS_SetElement(cx, arrayObj, count, &tmpValue) );
		}
	} else {

		JL_CHK(JL_NativeToJsval(cx, params[0], JL_RVAL) );
	}

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
DEFINE_FUNCTION( ColorMaterial ) {

	JL_S_ASSERT_ARG(2);
	JL_S_ASSERT_INT(JL_ARG(1));
	JL_S_ASSERT_INT(JL_ARG(2));

	glColorMaterial(JSVAL_TO_INT( JL_ARG(1) ), JSVAL_TO_INT( JL_ARG(2) ));  OGL_CHK;

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( face, pname, [ paramArray | param1, ... paramN ] )
  $H arguments
   $ARG GLenum face
   $ARG GLenum pname
   $ARG $VAL params: is either a number or an array of numbers.
  $H OpenGL API
   glMateriali, glMaterialf, glMaterialfv
**/
DEFINE_FUNCTION( Material ) {

	JL_S_ASSERT_ARG_MIN( 3 );
	JL_S_ASSERT_INT(JL_ARG(1));
	JL_S_ASSERT_INT(JL_ARG(2));

	*JL_RVAL = JSVAL_VOID;
	if ( argc == 3 && JSVAL_IS_INT(JL_ARG(3)) ) {

		glMateriali( JSVAL_TO_INT( JL_ARG(1) ), JSVAL_TO_INT( JL_ARG(2) ), JSVAL_TO_INT( JL_ARG(3) ) );  OGL_CHK;
		;
		return JS_TRUE;
	}
	if ( argc == 3 && JSVAL_IS_DOUBLE(JL_ARG(3)) ) {

		float param;
		JL_JsvalToNative(cx, JL_ARG(3), &param);
		glMaterialf( JSVAL_TO_INT( JL_ARG(1) ), JSVAL_TO_INT( JL_ARG(2) ), param );  OGL_CHK;
		;
		return JS_TRUE;
	}

	GLfloat params[16]; // alloca ?
	if ( argc == 3 && JL_IsArray(cx, JL_ARG(3)) ) {

		uint32 length;
		JL_CHK( JL_JsvalToCValVector(cx, JL_ARG(3), params, COUNTOF(params), &length ) );
		glMaterialfv( JSVAL_TO_INT(JL_ARG(1)), JSVAL_TO_INT(JL_ARG(2)), params );  OGL_CHK;
		;
		return JS_TRUE;
	}

	JL_S_ASSERT_ARG_MIN( 3 ); // at least
	JL_ASSERT( argc-2 < COUNTOF(params) );
	for ( unsigned int i = 2; i < argc; ++i )
		JL_JsvalToNative(cx, JL_ARGV[i], &params[i-2]);
	glMaterialfv( JSVAL_TO_INT(JL_ARG(1)), JSVAL_TO_INT(JL_ARG(2)), params );  OGL_CHK;
	;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( cap [, ..., capN] )
  $H arguments
   $ARG GLenum cap
  $H OpenGL API
   glEnable
**/
DEFINE_FUNCTION( Enable ) {

	JL_S_ASSERT_ARG_MIN(1);
	for ( uintN i = 0; i < JL_ARGC; ++i ) {

		JL_S_ASSERT_INT(JL_ARGV[i]);
		glEnable( JSVAL_TO_INT(JL_ARGV[i]) );  OGL_CHK;
	}
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( cap [, ..., capN] )
  $H arguments
   $ARG GLenum cap
  $H OpenGL API
   glDisable
**/
DEFINE_FUNCTION( Disable ) {

	JL_S_ASSERT_ARG_MIN(1);
	for ( uintN i = 0; i < JL_ARGC; ++i ) {

		JL_S_ASSERT_INT(JL_ARGV[i]);
		glDisable( JSVAL_TO_INT(JL_ARGV[i]) );  OGL_CHK;
	}
	*JL_RVAL = JSVAL_VOID;
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
DEFINE_FUNCTION( PointSize ) {

	JL_S_ASSERT_ARG(1);
	float size;
	JL_JsvalToNative(cx, JL_ARG(1), &size);
	
	glPointSize(size);  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
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
DEFINE_FUNCTION( LineWidth ) {

	JL_S_ASSERT_ARG(1);
	float width;
	JL_JsvalToNative(cx, JL_ARG(1), &width);
	
	glLineWidth(width);  OGL_CHK;

	*JL_RVAL = JSVAL_VOID;
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
DEFINE_FUNCTION( ShadeModel ) {

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_INT(JL_ARG(1));

	glShadeModel(JSVAL_TO_INT( JL_ARG(1) ));  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
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
DEFINE_FUNCTION( BlendFunc ) {

	JL_S_ASSERT_ARG(2);
	JL_S_ASSERT_INT(JL_ARG(1));
	JL_S_ASSERT_INT(JL_ARG(2));

	glBlendFunc(JSVAL_TO_INT( JL_ARG(1) ), JSVAL_TO_INT( JL_ARG(2) ));  OGL_CHK;

	*JL_RVAL = JSVAL_VOID;
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
DEFINE_FUNCTION( DepthFunc ) {

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_INT(JL_ARG(1));
	
	glDepthFunc( JSVAL_TO_INT( JL_ARG(1) ) );  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( func )
  $H arguments
   $ARG GLenum func
  $H OpenGL API
   glDepthMask
**/
DEFINE_FUNCTION( DepthMask ) {

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_BOOLEAN(JL_ARG(1));
	
	glDepthMask( JSVAL_TO_BOOLEAN( JL_ARG(1) ) );  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
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
DEFINE_FUNCTION( DepthRange ) {

	JL_S_ASSERT_ARG(2);
	double zNear, zFar;
	JL_JsvalToNative(cx, JL_ARG(1), &zNear);
	JL_JsvalToNative(cx, JL_ARG(2), &zFar);
	
	glDepthRange(zNear, zFar);  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( factor, units )
  $H arguments
   $ARG $REAL factor
   $ARG $REAL units
  $H OpenGL API
   glPolygonOffset
**/
DEFINE_FUNCTION( PolygonOffset ) {

//	JL_INIT_OPENGL_EXTENSION( glPolygonOffsetEXT, PFNGLPOLYGONOFFSETEXTPROC );

	JL_S_ASSERT_ARG(2);

	GLfloat factor, units;

	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &factor) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &units) );
	
	glPolygonOffset(factor, units);  OGL_CHK;

	*JL_RVAL = JSVAL_VOID;
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
DEFINE_FUNCTION( CullFace ) {

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_INT(JL_ARG(1));
	
	glCullFace(JSVAL_TO_INT( JL_ARG(1) ));  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
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
DEFINE_FUNCTION( FrontFace ) {

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_INT(JL_ARG(1));

	glFrontFace(JSVAL_TO_INT( JL_ARG(1) ));  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( s )
  $H arguments
   $ARG $INT s
  $H note
   if s is -1, 0xffffffff value is used.
  $H OpenGL API
   glClearStencil
**/
DEFINE_FUNCTION( ClearStencil ) {

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_INT(JL_ARG(1));

	GLuint s;
	if ( JL_ARG(1) == INT_TO_JSVAL(-1) )
		s = 0xffffffff;
	else
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &s) );

	glClearStencil(s);  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
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
DEFINE_FUNCTION( ClearDepth ) {

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_NUMBER(JL_ARG(1));

	double depth;
	JL_JsvalToNative(cx, JL_ARG(1), &depth);

	glClearDepth(depth);  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
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
DEFINE_FUNCTION( ClearColor ) {

	JL_S_ASSERT_ARG(4);
	JL_S_ASSERT_NUMBER(JL_ARG(1));
	JL_S_ASSERT_NUMBER(JL_ARG(2));
	JL_S_ASSERT_NUMBER(JL_ARG(3));
	JL_S_ASSERT_NUMBER(JL_ARG(4));

	float r, g, b, a;
	JL_JsvalToNative(cx, JL_ARG(1), &r);
	JL_JsvalToNative(cx, JL_ARG(2), &g);
	JL_JsvalToNative(cx, JL_ARG(3), &b);
	JL_JsvalToNative(cx, JL_ARG(4), &a);

	glClearColor(r, g, b, a);  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
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
DEFINE_FUNCTION( ClearAccum ) {

	JL_S_ASSERT_ARG(4);
	JL_S_ASSERT_NUMBER(JL_ARG(1));
	JL_S_ASSERT_NUMBER(JL_ARG(2));
	JL_S_ASSERT_NUMBER(JL_ARG(3));
	JL_S_ASSERT_NUMBER(JL_ARG(4));

	float r, g, b, a;
	JL_JsvalToNative(cx, JL_ARG(1), &r);
	JL_JsvalToNative(cx, JL_ARG(2), &g);
	JL_JsvalToNative(cx, JL_ARG(3), &b);
	JL_JsvalToNative(cx, JL_ARG(4), &a);
	
	glClearAccum(r, g, b, a);  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
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
DEFINE_FUNCTION( Clear ) {

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_INT(JL_ARG(1));

	glClear(JSVAL_TO_INT(JL_ARG(1)));  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( red, green, blue, alpha )
 $VOID $INAME( all )
  $H arguments
   $ARG boolean red
   $ARG boolean green
   $ARG boolean blue
   $ARG boolean alpha
  $H OpenGL API
   glColorMask
**/
DEFINE_FUNCTION( ColorMask ) {

	*JL_RVAL = JSVAL_VOID;
	if ( JL_ARGC == 1 ) {

		JL_S_ASSERT_BOOLEAN(JL_ARG(1));
		if ( JL_ARG(1) == JSVAL_FALSE ) {

			glColorMask(0,0,0,0);  OGL_CHK;
		} else {

			glColorMask(1,1,1,1);  OGL_CHK;
		}
		return JS_TRUE;
	}

	JL_S_ASSERT_ARG(4);
	JL_S_ASSERT_BOOLEAN(JL_ARG(1));
	JL_S_ASSERT_BOOLEAN(JL_ARG(2));
	JL_S_ASSERT_BOOLEAN(JL_ARG(3));
	JL_S_ASSERT_BOOLEAN(JL_ARG(4));

	glColorMask(JSVAL_TO_BOOLEAN(JL_ARG(1)), JSVAL_TO_BOOLEAN(JL_ARG(2)), JSVAL_TO_BOOLEAN(JL_ARG(3)), JSVAL_TO_BOOLEAN(JL_ARG(4)) );  OGL_CHK;

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
DEFINE_FUNCTION( ClipPlane ) {

	JL_S_ASSERT_ARG(2);
	JL_S_ASSERT_INT(JL_ARG(1));
	JL_S_ASSERT_ARRAY(JL_ARG(2));
	GLdouble equation[4];
	uint32 len;
	JL_CHK( JL_JsvalToCValVector(cx, JL_ARG(2), equation, COUNTOF(equation), &len ) );
	JL_S_ASSERT( len == 4, "Invalid plane equation.");
	glClipPlane(JSVAL_TO_INT(JL_ARG(1)), equation);  OGL_CHK;

	*JL_RVAL = JSVAL_VOID;
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
DEFINE_FUNCTION( Viewport ) {

	JL_S_ASSERT_ARG(4);
	JL_S_ASSERT_INT(JL_ARG(1));
	JL_S_ASSERT_INT(JL_ARG(2));
	JL_S_ASSERT_INT(JL_ARG(3));
	JL_S_ASSERT_INT(JL_ARG(4));

	glViewport(JSVAL_TO_INT(JL_ARG(1)), JSVAL_TO_INT(JL_ARG(2)), JSVAL_TO_INT(JL_ARG(3)), JSVAL_TO_INT(JL_ARG(4)));  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
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
DEFINE_FUNCTION( Frustum ) {

	JL_S_ASSERT_ARG(6);
	JL_S_ASSERT_NUMBER(JL_ARG(1));
	JL_S_ASSERT_NUMBER(JL_ARG(2));
	JL_S_ASSERT_NUMBER(JL_ARG(3));
	JL_S_ASSERT_NUMBER(JL_ARG(4));
	JL_S_ASSERT_NUMBER(JL_ARG(5));
	JL_S_ASSERT_NUMBER(JL_ARG(6));

	jsdouble left, right, bottom, top, zNear, zFar;
	JL_JsvalToNative(cx, JL_ARG(1), &left);
	JL_JsvalToNative(cx, JL_ARG(2), &right);
	JL_JsvalToNative(cx, JL_ARG(3), &bottom);
	JL_JsvalToNative(cx, JL_ARG(4), &top);
	JL_JsvalToNative(cx, JL_ARG(5), &zNear);
	JL_JsvalToNative(cx, JL_ARG(6), &zFar);

	glFrustum(left, right, bottom, top, zNear, zFar);  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
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
DEFINE_FUNCTION( Ortho ) {

	JL_S_ASSERT_ARG(6);
	JL_S_ASSERT_NUMBER(JL_ARG(1));
	JL_S_ASSERT_NUMBER(JL_ARG(2));
	JL_S_ASSERT_NUMBER(JL_ARG(3));
	JL_S_ASSERT_NUMBER(JL_ARG(4));
	JL_S_ASSERT_NUMBER(JL_ARG(5));
	JL_S_ASSERT_NUMBER(JL_ARG(6));

	jsdouble left, right, bottom, top, zNear, zFar;
	JL_JsvalToNative(cx, JL_ARG(1), &left);
	JL_JsvalToNative(cx, JL_ARG(2), &right);
	JL_JsvalToNative(cx, JL_ARG(3), &bottom);
	JL_JsvalToNative(cx, JL_ARG(4), &top);
	JL_JsvalToNative(cx, JL_ARG(5), &zNear);
	JL_JsvalToNative(cx, JL_ARG(6), &zFar);

	glOrtho(left, right, bottom, top, zNear, zFar);  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( fovY, [aspectRatio | $UNDEF], zNear, zFar )
  Set up a perspective projection matrix.
  $H arguments
   $ARG $REAL fovy
   $ARG $REAL aspectRatio: If omitted (viewportWidth/viewportHeight) is used by default.
   $ARG $REAL zNear
   $ARG $REAL zFar
  $H API
   gluPerspective
**/
DEFINE_FUNCTION( Perspective ) {

	//cf. gluPerspective(fovy, float(viewport[2]) / float(viewport[3]), zNear, zFar);

	JL_S_ASSERT_ARG(4);
	JL_S_ASSERT_NUMBER(JL_ARG(1));
	//JL_S_ASSERT_NUMBER(JL_ARG(2)); // may be undefined
	JL_S_ASSERT_NUMBER(JL_ARG(3));
	JL_S_ASSERT_NUMBER(JL_ARG(4));


	double fovy, zNear, zFar, aspect;
	JL_JsvalToNative(cx, JL_ARG(1), &fovy);
	//	JL_JsvalToNative(cx, JL_ARG(2), &aspect)
	JL_JsvalToNative(cx, JL_ARG(3), &zNear);
	JL_JsvalToNative(cx, JL_ARG(4), &zFar);

//	GLint prevMatrixMode;
//	glGetIntegerv(GL_MATRIX_MODE, &prevMatrixMode);  OGL_CHK; // GL_MODELVIEW

	if ( JL_ARG_ISDEF(2) ) {

		JL_JsvalToNative(cx, JL_ARG(2), &aspect);
	} else {

		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);  OGL_CHK;
		aspect = ((double)viewport[2]) / ((double)viewport[3]);
	}
/*
	double xmin, xmax, ymin, ymax;
	ymax = zNear * tan(fovy * M_PI / 360.0f);
	ymin = -ymax;
	xmin = ymin * aspect;
	xmax = ymax * aspect;
	glFrustum(xmin, xmax, ymin, ymax, zNear, zFar);  OGL_CHK;
*/
	gluPerspective(fovy, aspect, zNear, zFar);  OGL_CHK;

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

	*JL_RVAL = JSVAL_VOID;
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
DEFINE_FUNCTION( MatrixMode ) {

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_INT(JL_ARG(1));
	
	glMatrixMode(JSVAL_TO_INT( JL_ARG(1) ));  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  $H OpenGL API
   glLoadIdentity
**/
DEFINE_FUNCTION( LoadIdentity ) {

	JL_S_ASSERT_ARG(0);

	glLoadIdentity();  OGL_CHK;

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  $H OpenGL API
   glPushMatrix
**/
DEFINE_FUNCTION( PushMatrix ) {

	JL_S_ASSERT_ARG(0);

	glPushMatrix();  OGL_CHK;

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  $H OpenGL API
   glPopMatrix
**/
DEFINE_FUNCTION( PopMatrix ) {

	JL_S_ASSERT_ARG(0);

	glPopMatrix();  OGL_CHK;

	*JL_RVAL = JSVAL_VOID;
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
DEFINE_FUNCTION( LoadMatrix ) {

	JL_S_ASSERT_ARG(1);
	float tmp[16], *m = tmp;

	JL_CHK( JL_JsvalToMatrix44(cx, JL_ARG(1), &m) );
	glLoadMatrixf(m);  OGL_CHK;

	*JL_RVAL = JSVAL_VOID;
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
DEFINE_FUNCTION( MultMatrix ) {

	JL_S_ASSERT_ARG(1);
	float tmp[16], *m = tmp;

	JL_CHK( JL_JsvalToMatrix44(cx, JL_ARG(1), &m) );

	glMultMatrixf(m);  OGL_CHK;

	*JL_RVAL = JSVAL_VOID;
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
DEFINE_FUNCTION( Rotate ) {

	JL_S_ASSERT_ARG(4);
	jsdouble angle, x, y, z;
	JL_JsvalToNative(cx, JL_ARG(1), &angle);
	JL_JsvalToNative(cx, JL_ARG(2), &x);
	JL_JsvalToNative(cx, JL_ARG(3), &y);
	JL_JsvalToNative(cx, JL_ARG(4), &z);
	
	glRotated(angle, x, y, z);  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
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
DEFINE_FUNCTION( Translate ) {

	JL_S_ASSERT_ARG_RANGE(2,3);
	double x, y, z;
	JL_JsvalToNative(cx, JL_ARG(1), &x);
	JL_JsvalToNative(cx, JL_ARG(2), &y);
	if ( argc >= 3 )
		JL_JsvalToNative(cx, JL_ARG(3), &z);
	else
		z = 0;
	
	glTranslated(x, y, z);  OGL_CHK;

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( x, y [, z = 1] )
 $VOID $INAME( factor )
  $H arguments
   $ARG $REAL x
   $ARG $REAL y
   $ARG $REAL z
  $H OpenGL API
   glScaled
**/
DEFINE_FUNCTION( Scale ) {

	JL_S_ASSERT_ARG_RANGE(1,3);
	*JL_RVAL = JSVAL_VOID;
	double x, y, z;
	JL_JsvalToNative(cx, JL_ARG(1), &x);

	if ( argc == 1 ) {

		glScaled(x, x, x);  OGL_CHK;
		;
		return JS_TRUE;
	}
	JL_JsvalToNative(cx, JL_ARG(2), &y);

	if ( argc >= 3 ) {

		JL_JsvalToNative(cx, JL_ARG(3), &z);
		glScaled(x, y, z);  OGL_CHK;
		;
		return JS_TRUE;
	}

	glScaled(x, y, 1);  OGL_CHK;

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME( [ $BOOL compileOnly ] )
  Returns a new display-list.
  $H OpenGL API
   glGenLists, glNewList
**/
DEFINE_FUNCTION( NewList ) {

	JL_S_ASSERT_ARG_RANGE(0,1);
	bool compileOnly;
	if ( JL_ARG_ISDEF(1) )
		JL_JsvalToNative(cx, JL_ARG(1), &compileOnly);
	else
		compileOnly = false;

	GLuint list;
	list = glGenLists(1);  OGL_CHK;
	glNewList(list, compileOnly ? GL_COMPILE : GL_COMPILE_AND_EXECUTE);  OGL_CHK;
	
	*JL_RVAL = INT_TO_JSVAL(list);
	return JS_TRUE;
	JL_BAD;
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
DEFINE_FUNCTION( DeleteList ) {

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_INT(JL_ARG(1));
	glDeleteLists(JSVAL_TO_INT(JL_ARG(1)), 1);  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  $H OpenGL API
   glEndList
**/
DEFINE_FUNCTION( EndList ) {

	JL_S_ASSERT_ARG(0);
	glEndList();  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
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
DEFINE_FUNCTION( CallList ) {

	JL_S_ASSERT_ARG(1);
	*JL_RVAL = JSVAL_VOID;

	if ( JSVAL_IS_INT( JL_ARG(1) ) ) {

		glCallList(JSVAL_TO_INT(JL_ARG(1)));  OGL_CHK;
		return JS_TRUE;
	}

	if ( JL_IsArray(cx, JL_ARG(1)) ) {

		JSObject *jsArray = JSVAL_TO_OBJECT(JL_ARG(1));
		jsuint length;
		JL_CHK( JS_GetArrayLength(cx, jsArray, &length) );

		GLuint *lists = (GLuint*)alloca(length * sizeof(GLuint));
		jsval value;
		for (jsuint i=0; i<length; ++i) {

			JL_CHK( JS_GetElement(cx, jsArray, i, &value) );
			lists[i] = JSVAL_TO_INT(value);
		}

		glCallLists(length, GL_UNSIGNED_INT, lists);  OGL_CHK; // http://www.opengl.org/documentation/specs/man_pages/hardcopy/GL/html/gl/calllists.html

//		jl_free(lists); // alloca
		return JS_TRUE;
	}
	JL_REPORT_ERROR("Invalid argument");
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( face, mode )
  $H arguments
   $ARG GLenum face
   $ARG GLenum mode
  $H OpenGL API
   glPolygonMode
**/
DEFINE_FUNCTION( PolygonMode ) {

	JL_S_ASSERT_ARG(2);
	JL_S_ASSERT_INT(JL_ARG(1));
	JL_S_ASSERT_INT(JL_ARG(2));
	glPolygonMode(JSVAL_TO_INT( JL_ARG(1) ), JSVAL_TO_INT( JL_ARG(2) ));  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
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
DEFINE_FUNCTION( Begin ) {

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_INT(JL_ARG(1));
#ifdef DEBUG
	_inBeginEnd = true;
#endif // DEBUG
	glBegin(JSVAL_TO_INT( JL_ARG(1) ));  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  $H OpenGL API
   glEnd
**/
DEFINE_FUNCTION( End ) {

	JL_S_ASSERT_ARG(0);
	glEnd();  OGL_CHK;
#ifdef DEBUG
	_inBeginEnd = false;
#endif // DEBUG
	
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( mask )
  $H arguments
   $ARG GLbitfield mask
  $H OpenGL API
   glPushAttrib
**/
DEFINE_FUNCTION( PushAttrib ) {

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_NUMBER(JL_ARG(1));
	GLbitfield mask;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &mask) );
	glPushAttrib(mask);  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  $H OpenGL API
   glPopAttrib
**/
DEFINE_FUNCTION( PopAttrib ) {

	JL_S_ASSERT_ARG(0);
	glPopAttrib();  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME()
  Returns a new texture name.
  $H OpenGL API
   glGenTextures
**/
DEFINE_FUNCTION( GenTexture ) {

	JL_S_ASSERT_ARG(0);
	GLuint texture;
	glGenTextures(1, &texture);  OGL_CHK;
	
	*JL_RVAL = INT_TO_JSVAL(texture);
	return JS_TRUE;
	JL_BAD;
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
DEFINE_FUNCTION( BindTexture ) {

	JL_S_ASSERT_ARG(2);
	JL_S_ASSERT_INT(JL_ARG(1));
//	JL_S_ASSERT_INT(JL_ARG(2));
	int texture;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &texture) );
	glBindTexture( JSVAL_TO_INT( JL_ARG(1) ), texture);  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
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
DEFINE_FUNCTION( DeleteTexture ) {

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_INT(JL_ARG(1));
	GLuint texture = JSVAL_TO_INT( JL_ARG(1) );
	glDeleteTextures(1, &texture);  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( target, level, internalFormat, x, y, width, height [, border = false ] )
  $H arguments
   $ARG $INT target
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
DEFINE_FUNCTION( CopyTexImage2D ) {

	JL_S_ASSERT_ARG_MIN(6);
	JL_S_ASSERT_INT(JL_ARG(1));
	JL_S_ASSERT_INT(JL_ARG(2));
	JL_S_ASSERT_INT(JL_ARG(3));
	JL_S_ASSERT_INT(JL_ARG(4));
	JL_S_ASSERT_INT(JL_ARG(5));
	JL_S_ASSERT_INT(JL_ARG(6));
	JL_S_ASSERT_INT(JL_ARG(7));

	GLint border;
	if ( JL_ARG_ISDEF(8) ) {

		JL_S_ASSERT_INT(JL_ARG(8));
		border = JSVAL_TO_INT(JL_ARG(8));
	} else {

		border = 0;
	}

	glCopyTexImage2D(JSVAL_TO_INT(JL_ARG(1)), JSVAL_TO_INT(JL_ARG(2)), JSVAL_TO_INT(JL_ARG(3)), JSVAL_TO_INT(JL_ARG(4)), JSVAL_TO_INT(JL_ARG(5)), JSVAL_TO_INT(JL_ARG(6)), JSVAL_TO_INT(JL_ARG(7)), border);  OGL_CHK;

	*JL_RVAL = JSVAL_VOID;
	;
	return JS_TRUE;
	JL_BAD;
}


/*
DEFINE_FUNCTION( TexSubImage2D ) {

	JL_S_ASSERT_ARG_MIN(7);
	JL_S_ASSERT_INT(JL_ARG(1));
	JL_S_ASSERT_INT(JL_ARG(2));
	JL_S_ASSERT_INT(JL_ARG(3));
	JL_S_ASSERT_INT(JL_ARG(4));
	JL_S_ASSERT_INT(JL_ARG(5));
	JL_S_ASSERT_INT(JL_ARG(6));

	GLint level = JSVAL_TO_INT(JL_ARG(1));
	GLenum internalFormat = JSVAL_TO_INT(JL_ARG(2));

	GLint xoffset = JSVAL_TO_INT(JL_ARG(3));
	GLint yoffset = JSVAL_TO_INT(JL_ARG(4));
	GLint x = JSVAL_TO_INT(JL_ARG(5));
	GLint y = JSVAL_TO_INT(JL_ARG(6));
	GLint width = JSVAL_TO_INT(JL_ARG(7));
	GLint height = JSVAL_TO_INT(JL_ARG(8));

	GLint border;
	if ( JL_ARG_ISDEF(7) )
		border = JSVAL_TO_INT(JL_ARG(7));
	else
		border = 0;

	glTexSubImage2D( GL_TEXTURE_2D, level, xoffset, yoffset, width, height, format,  border );  OGL_CHK;

	*JL_RVAL = JSVAL_VOID;
	;
	return JS_TRUE;
	JL_BAD;
}
*/

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( pname, param )
  $H OpenGL API
   glPixelTransferi, glPixelTransferf
**/
DEFINE_FUNCTION( PixelTransfer ) {

	JL_S_ASSERT_INT(JL_ARG(1));

	GLenum pname = JSVAL_TO_INT( JL_ARG(1) );

	if ( JSVAL_IS_INT(JL_ARG(2)) ) {
		
		glPixelTransferi(pname, JSVAL_TO_INT( JL_ARG(2) ));  OGL_CHK;
	} else {

		JL_S_ASSERT_NUMBER(JL_ARG(2));
		float param;
		JL_JsvalToNative(cx, JL_ARG(2), &param);
		glPixelTransferf(pname, param);  OGL_CHK;
	}

	*JL_RVAL = JSVAL_VOID;
 	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( pname, param )
  $H OpenGL API
   glPixelStorei, glPixelStoref
**/
DEFINE_FUNCTION( PixelStore ) {

	JL_S_ASSERT_INT(JL_ARG(1));

	GLenum pname = JSVAL_TO_INT( JL_ARG(1) );

	if ( JSVAL_IS_INT(JL_ARG(2)) ) {
		
		glPixelStorei(pname, JSVAL_TO_INT( JL_ARG(2) ));  OGL_CHK;
	} else {

		JL_S_ASSERT_NUMBER(JL_ARG(2));
		float param;
		JL_JsvalToNative(cx, JL_ARG(2), &param);
		glPixelStoref(pname, param);  OGL_CHK;
	}

	*JL_RVAL = JSVAL_VOID;
 	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( x, y [ , z [ , w ] ] )
  $H OpenGL API
   glRasterPos*
**/
DEFINE_FUNCTION( RasterPos ) {

	JL_S_ASSERT_ARG_RANGE(2,4);

	double x, y, z, w;

	*JL_RVAL = JSVAL_VOID;

	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &x) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &y) );
	if ( argc >= 3 ) {

		JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &z) );
		if ( argc >= 4 ) {

			JL_CHK( JL_JsvalToNative(cx, JL_ARG(4), &w) );
			glRasterPos4d(x, y, z, w);  OGL_CHK;
			;
			return JS_TRUE;
		}
		glRasterPos3d(x, y, z);  OGL_CHK;
		;
		return JS_TRUE;
	}
	glRasterPos2d(x, y);  OGL_CHK;
	;
 	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( x, y )
  $H OpenGL API
   glPixelZoom
**/
DEFINE_FUNCTION( PixelZoom ) {

	JL_S_ASSERT_ARG(2);
	float x, y;

	*JL_RVAL = JSVAL_VOID;

	JL_JsvalToNative(cx, JL_ARG(1), &x);
	JL_JsvalToNative(cx, JL_ARG(2), &y);
	glPixelZoom(x, y);  OGL_CHK;
	;
 	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( map, values )
  $H OpenGL API
   glPixelMapfv
**/
DEFINE_FUNCTION( PixelMap ) {

	JL_S_ASSERT_ARG(2);
	*JL_RVAL = JSVAL_VOID;
	JL_S_ASSERT_ARRAY( JL_ARG(2) );

	jsuint mapsize;
	JL_CHK( JS_GetArrayLength(cx, JSVAL_TO_OBJECT(JL_ARG(2)), &mapsize) );
	GLfloat *values = (GLfloat*)alloca(mapsize*sizeof(*values));
	JL_CHK( JL_JsvalToCValVector(cx, JL_ARG(2), values, mapsize, &mapsize ) );
	glPixelMapfv(JSVAL_TO_INT(JL_ARG(1)), mapsize, values);  OGL_CHK;

	;
 	return JS_TRUE;
	JL_BAD;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// OpenGL extensions


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME( procName [, procName, ...] )
  $H arguments
   $ARG string procName
  $H return value
   true if all extension proc are found.
**/
DEFINE_FUNCTION( HasExtensionProc ) {
	
	JLStr procName;
	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT( glGetProcAddress != NULL, "OpenGL extensions unavailable." );

	void *procAddr;
	for ( uintN i = 0; i < JL_ARGC; ++i ) {

		JL_CHK( JL_JsvalToNative(cx, JL_ARGV[i], &procName) );
		procAddr = glGetProcAddress(procName);
		if ( procAddr == NULL ) {

			*JL_RVAL = JSVAL_FALSE;
			return JS_TRUE;
		}
	}
	*JL_RVAL = JSVAL_TRUE;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME( name [, name, ...] )
  $H arguments
   $ARG string name
  $H return value
   true if all extensions are available.
**/
DEFINE_FUNCTION( HasExtensionName ) {
	
	JL_S_ASSERT_ARG_MIN(1);

	const char *extensions = (const char *)glGetString(GL_EXTENSIONS);
	JL_ASSERT( extensions != NULL );

	for ( uintN i = 0; i < JL_ARGC; ++i ) {

		JLStr name;
//		const char *name;
//		unsigned int nameLength;
//		JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARGV[i], &name, &nameLength) );
		JL_CHK( JL_JsvalToNative(cx, JL_ARGV[i], &name) );

		const char *pos = strstr(extensions, name);
		if ( pos == NULL || ( pos[name.Length()] != ' ' && pos[name.Length()] != '\0' ) ) {

			*JL_RVAL = JSVAL_FALSE;
			return JS_TRUE;
		}
	}
	*JL_RVAL = JSVAL_TRUE;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME( mode )
  $H arguments
   $ARG GLenum mode
  $H OpenGL API
   glBlendEquation
**/
DEFINE_FUNCTION( BlendEquation ) {

	INIT_OPENGL_EXTENSION( glBlendEquation, PFNGLBLENDEQUATIONPROC );
	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT_INT(JL_ARG(1));

	glBlendEquation(JSVAL_TO_INT(JL_ARG(1)));

	*JL_RVAL = JSVAL_VOID;
 	return JS_TRUE;
	JL_BAD;
}


 
/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( face, func, ref, mask )
  $H arguments
   $ARG GLenum face
   $ARG GLenum func
   $ARG $INT ref
   $ARG $UINT mask
  $H note
   if mask is -1, 0xffffffff value is used as mask.
  $H OpenGL API
   glStencilFunc
**/
DEFINE_FUNCTION( StencilFuncSeparate ) {

	INIT_OPENGL_EXTENSION( glStencilFuncSeparate, PFNGLSTENCILFUNCSEPARATEPROC ); // Opengl 2.0+

	JL_S_ASSERT_ARG(4);
	JL_S_ASSERT_INT(JL_ARG(1));
	JL_S_ASSERT_INT(JL_ARG(2));
	JL_S_ASSERT_INT(JL_ARG(3));
	JL_S_ASSERT_NUMBER(JL_ARG(4));

	GLuint mask;
	if ( JL_ARG(4) == INT_TO_JSVAL(-1) )
		mask = 0xffffffff;
	else
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(4), &mask) );

	glStencilFuncSeparate(JSVAL_TO_INT(JL_ARG(1)), JSVAL_TO_INT(JL_ARG(2)), JSVAL_TO_INT(JL_ARG(3)), mask);  OGL_CHK;

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( face, fail, zfail, zpass )
  $H arguments
   $ARG GLenum face
   $ARG GLenum fail
   $ARG GLenum zfail
   $ARG GLenum zpass
  $H OpenGL API
   glStencilOp
**/
DEFINE_FUNCTION( StencilOpSeparate ) {

	INIT_OPENGL_EXTENSION( glStencilOpSeparate, PFNGLSTENCILOPSEPARATEPROC ); // Opengl 2.0+

	JL_S_ASSERT_ARG(4);
	JL_S_ASSERT_INT(JL_ARG(1));
	JL_S_ASSERT_INT(JL_ARG(2));
	JL_S_ASSERT_INT(JL_ARG(3));
	JL_S_ASSERT_INT(JL_ARG(4));
	glStencilOpSeparate(JSVAL_TO_INT(JL_ARG(1)), JSVAL_TO_INT(JL_ARG(2)), JSVAL_TO_INT(JL_ARG(3)), JSVAL_TO_INT(JL_ARG(4)));  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( face )
  $H arguments
   $ARG GLenum face
  $H OpenGL API
   glActiveStencilFaceEXT
**/
DEFINE_FUNCTION( ActiveStencilFaceEXT ) {

	INIT_OPENGL_EXTENSION( glActiveStencilFaceEXT, PFNGLACTIVESTENCILFACEEXTPROC ); // Opengl 2.0+

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_INT(JL_ARG(1));
	glActiveStencilFaceEXT(JSVAL_TO_INT( JL_ARG(1) ));  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( target, renderbuffer )
  $H arguments
   $ARG enum target
   $ARG uint renderbuffer
  $H API
   glBindRenderbufferEXT
**/
DEFINE_FUNCTION( BindRenderbuffer ) {

	JL_INIT_OPENGL_EXTENSION( glBindRenderbufferEXT, PFNGLBINDRENDERBUFFEREXTPROC );

	JL_S_ASSERT_ARG(2);
	JL_S_ASSERT_INT(JL_ARG(1));
	JL_S_ASSERT_INT(JL_ARG(2));
	
	glBindRenderbufferEXT(JSVAL_TO_INT(JL_ARG(1)), JSVAL_TO_INT(JL_ARG(2)) );  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME()
  $H OpenGL API
   glGenRenderbuffersEXT
**/
DEFINE_FUNCTION( GenRenderbuffer ) {

	JL_INIT_OPENGL_EXTENSION( glGenRenderbuffersEXT, PFNGLGENRENDERBUFFERSEXTPROC);
	
	JL_S_ASSERT_ARG(0);
	GLuint buffer;
	
	glGenRenderbuffersEXT(1, &buffer);  OGL_CHK;
	
	*JL_RVAL = INT_TO_JSVAL(buffer);

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME( renderbuffer )
  $H arguments
   $ARG int renderbuffer
  $H OpenGL API
   glDeleteRenderbuffersEXT
**/
DEFINE_FUNCTION( DeleteRenderbuffer ) {

	JL_INIT_OPENGL_EXTENSION( glDeleteRenderbuffersEXT, PFNGLDELETERENDERBUFFERSEXTPROC );

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_INT(JL_ARG(1));
	GLuint buffer = JSVAL_TO_INT(JL_ARG(1));
	glDeleteRenderbuffersEXT(1, &buffer);  OGL_CHK;

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME( target, internalformat, width, height )
  $H arguments
   $ARG enum target
   $ARG enum internalformat
   $ARG int width
   $ARG int height
  $H OpenGL API
   glRenderbufferStorageEXT
**/
DEFINE_FUNCTION( RenderbufferStorage ) {

	JL_INIT_OPENGL_EXTENSION( glRenderbufferStorageEXT, PFNGLRENDERBUFFERSTORAGEEXTPROC );

	JL_S_ASSERT_ARG(4);
	JL_S_ASSERT_INT(JL_ARG(1));
	JL_S_ASSERT_INT(JL_ARG(2));
	JL_S_ASSERT_INT(JL_ARG(3));
	JL_S_ASSERT_INT(JL_ARG(4));
	glRenderbufferStorageEXT(JSVAL_TO_INT(JL_ARG(1)), JSVAL_TO_INT(JL_ARG(2)), JSVAL_TO_INT(JL_ARG(3)), JSVAL_TO_INT(JL_ARG(4)) );  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME( target, pname [, count] )
  $H arguments
   $ARG enum target
   $ARG enum pname
   $ARG int|Array params
  $H OpenGL API
   glGetRenderbufferParameterivEXT
**/
DEFINE_FUNCTION( GetRenderbufferParameter ) {

	JL_INIT_OPENGL_EXTENSION( glGetRenderbufferParameterivEXT, PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC );

	JL_S_ASSERT_ARG_RANGE(2,3);
	JL_S_ASSERT_INT(JL_ARG(1));
	JL_S_ASSERT_INT(JL_ARG(2));

	GLint params[16]; // (TBD) check if it is the max amount of data that glGetRenderbufferParameterivEXT may returns.
	
	glGetRenderbufferParameterivEXT(JSVAL_TO_INT(JL_ARG(1)), JSVAL_TO_INT(JL_ARG(2)), params );  OGL_CHK;

	if ( JL_ARG_ISDEF(3) ) {
		
		JL_S_ASSERT_INT(JL_ARG(3));
		int count;
		count = JSVAL_TO_INT(JL_ARG(3));
		JL_S_ASSERT( count <= COUNTOF(params), "Too many params" );
		JL_CHK( JL_CValVectorToJsval(cx, params, count, JL_RVAL, false) );
	} else {

		*JL_RVAL = INT_TO_JSVAL( params[0] );
	}

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( target, renderbuffer )
  $H arguments
   $ARG enum target
   $ARG uint renderbuffer
  $H API
   glBindFramebufferEXT
**/
DEFINE_FUNCTION( BindFramebuffer ) {

	JL_INIT_OPENGL_EXTENSION( glBindFramebufferEXT, PFNGLBINDFRAMEBUFFEREXTPROC );

	JL_S_ASSERT_ARG(2);
	JL_S_ASSERT_INT(JL_ARG(1));
	JL_S_ASSERT_INT(JL_ARG(2));
	glBindFramebufferEXT( JSVAL_TO_INT(JL_ARG(1)), JSVAL_TO_INT(JL_ARG(2)) );  OGL_CHK;

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME()
  $H OpenGL API
   glGenFramebuffersEXT
**/
DEFINE_FUNCTION( GenFramebuffer ) {

	JL_INIT_OPENGL_EXTENSION( glGenFramebuffersEXT, PFNGLGENFRAMEBUFFERSEXTPROC );

	JL_S_ASSERT_ARG(0);
	GLuint buffer;
	glGenFramebuffersEXT(1, &buffer);  OGL_CHK;
	*JL_RVAL = INT_TO_JSVAL(buffer);

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME( renderbuffer )
  $H arguments
   $ARG int renderbuffer
  $H OpenGL API
   glDeleteFranebuffersEXT
**/
DEFINE_FUNCTION( DeleteFramebuffer ) {

	JL_INIT_OPENGL_EXTENSION( glDeleteFramebuffersEXT, PFNGLDELETEFRAMEBUFFERSEXTPROC );

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_INT(JL_ARG(1));
	GLuint buffer = JSVAL_TO_INT(JL_ARG(1));
	glDeleteFramebuffersEXT(1, &buffer);  OGL_CHK;

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME( target )
  $H arguments
   $ARG enum target
  $H OpenGL API
   glCheckFramebufferStatusEXT
**/
DEFINE_FUNCTION( CheckFramebufferStatus ) {

	JL_INIT_OPENGL_EXTENSION( glCheckFramebufferStatusEXT, PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC );

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_INT(JL_ARG(1));
	*JL_RVAL = INT_TO_JSVAL( glCheckFramebufferStatusEXT(JSVAL_TO_INT(JL_ARG(1))) );  OGL_CHK;

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME( target, attachment, textarget, texture, level )
  $H arguments
   $ARG enum target
   $ARG enum attachment
   $ARG enum textarget
   $ARG uint texture
   $ARG int level
  $H OpenGL API
   glFramebufferTexture1DEXT
**/
DEFINE_FUNCTION( FramebufferTexture1D ) {

	JL_INIT_OPENGL_EXTENSION( glFramebufferTexture1DEXT, PFNGLFRAMEBUFFERTEXTURE1DEXTPROC );

	JL_S_ASSERT_ARG(5);
	JL_S_ASSERT_INT(JL_ARG(1));
	JL_S_ASSERT_INT(JL_ARG(2));
	JL_S_ASSERT_INT(JL_ARG(3));
	JL_S_ASSERT_INT(JL_ARG(4));
	JL_S_ASSERT_INT(JL_ARG(5));

	glFramebufferTexture1DEXT( JSVAL_TO_INT(JL_ARG(1)), JSVAL_TO_INT(JL_ARG(2)), JSVAL_TO_INT(JL_ARG(3)), JSVAL_TO_INT(JL_ARG(4)), JSVAL_TO_INT(JL_ARG(5)) );  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME( target, attachment, textarget, texture, level )
  $H arguments
   $ARG enum target
   $ARG enum attachment
   $ARG enum textarget
   $ARG uint texture
   $ARG int level
  $H OpenGL API
   glFramebufferTexture2DEXT
**/
DEFINE_FUNCTION( FramebufferTexture2D ) {

	JL_INIT_OPENGL_EXTENSION( glFramebufferTexture2DEXT, PFNGLFRAMEBUFFERTEXTURE2DEXTPROC );

	JL_S_ASSERT_ARG(5);
	JL_S_ASSERT_INT(JL_ARG(1));
	JL_S_ASSERT_INT(JL_ARG(2));
	JL_S_ASSERT_INT(JL_ARG(3));
	JL_S_ASSERT_INT(JL_ARG(4));
	JL_S_ASSERT_INT(JL_ARG(5));

	glFramebufferTexture2DEXT( JSVAL_TO_INT(JL_ARG(1)), JSVAL_TO_INT(JL_ARG(2)), JSVAL_TO_INT(JL_ARG(3)), JSVAL_TO_INT(JL_ARG(4)), JSVAL_TO_INT(JL_ARG(5)) );  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME( target, attachment, textarget, texture, level, zoffset )
  $H arguments
   $ARG enum target
   $ARG enum attachment
   $ARG enum textarget
   $ARG uint texture
   $ARG int level
   $ARG int zoffset
  $H OpenGL API
   glFramebufferTexture3DEXT
**/
DEFINE_FUNCTION( FramebufferTexture3D ) {

	JL_INIT_OPENGL_EXTENSION( glFramebufferTexture3DEXT, PFNGLFRAMEBUFFERTEXTURE3DEXTPROC );

	JL_S_ASSERT_ARG(5);
	JL_S_ASSERT_INT(JL_ARG(1));
	JL_S_ASSERT_INT(JL_ARG(2));
	JL_S_ASSERT_INT(JL_ARG(3));
	JL_S_ASSERT_INT(JL_ARG(4));
	JL_S_ASSERT_INT(JL_ARG(5));
	JL_S_ASSERT_INT(JL_ARG(6));

	glFramebufferTexture3DEXT( JSVAL_TO_INT(JL_ARG(1)), JSVAL_TO_INT(JL_ARG(2)), JSVAL_TO_INT(JL_ARG(3)), JSVAL_TO_INT(JL_ARG(4)), JSVAL_TO_INT(JL_ARG(5)), JSVAL_TO_INT(JL_ARG(6)) );  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $INT $INAME( target, attachment, renderbuffertarget, renderbuffer )
  $H arguments
   $ARG enum target
   $ARG enum attachment
   $ARG enum renderbuffertarget
   $ARG uint renderbuffer
  $H OpenGL API
   glFramebufferRenderbufferEXT
**/
DEFINE_FUNCTION( FramebufferRenderbuffer ) {

	JL_INIT_OPENGL_EXTENSION( glFramebufferRenderbufferEXT, PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC );

	JL_S_ASSERT_ARG(5);
	JL_S_ASSERT_INT(JL_ARG(1));
	JL_S_ASSERT_INT(JL_ARG(2));
	JL_S_ASSERT_INT(JL_ARG(3));
	JL_S_ASSERT_INT(JL_ARG(4));

	glFramebufferRenderbufferEXT( JSVAL_TO_INT(JL_ARG(1)), JSVAL_TO_INT(JL_ARG(2)), JSVAL_TO_INT(JL_ARG(3)), JSVAL_TO_INT(JL_ARG(4)) );  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}




/**doc
$TOC_MEMBER $INAME
 $INT $INAME( target, attachment, pname [, count] )
  $H arguments
   $ARG enum target
   $ARG enum attachment
   $ARG enum pname
   $ARG int|Array params
  $H OpenGL API
   glGetFramebufferAttachmentParameterivEXT
**/
DEFINE_FUNCTION( GetFramebufferAttachmentParameter ) {

	JL_INIT_OPENGL_EXTENSION( glGetFramebufferAttachmentParameterivEXT, PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC  );

	JL_S_ASSERT_ARG_RANGE(3,4);
	JL_S_ASSERT_INT(JL_ARG(1));
	JL_S_ASSERT_INT(JL_ARG(2));
	JL_S_ASSERT_INT(JL_ARG(3));

	GLint params[16]; // (TBD) check if it is the max amount of data that glGetRenderbufferParameterivEXT may returns.
	
	glGetFramebufferAttachmentParameterivEXT(JSVAL_TO_INT(JL_ARG(1)), JSVAL_TO_INT(JL_ARG(2)), JSVAL_TO_INT(JL_ARG(3)), params);  OGL_CHK;

	if ( JL_ARG_ISDEF(4) ) {
		
		JL_S_ASSERT_INT( JL_ARG(4) );
		int count;
		count = JSVAL_TO_INT( JL_ARG(4) );
		JL_S_ASSERT( count <= COUNTOF(params), "Too many params" );
		JL_CHK( JL_CValVectorToJsval(cx, params, count, JL_RVAL, false) );
	} else {

		*JL_RVAL = INT_TO_JSVAL( params[0] );
	}

	return JS_TRUE;
	JL_BAD;
}




/**doc
$TOC_MEMBER $INAME
 $INT $INAME( shaderType )
  (TBD)
 $H OpenGL API
   glCreateShaderObjectARB
**/
DEFINE_FUNCTION( CreateShaderObjectARB ) {

	JL_INIT_OPENGL_EXTENSION( glCreateShaderObjectARB, PFNGLCREATESHADEROBJECTARBPROC );
	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_INT(JL_ARG(1));
	GLenum shaderType;
	shaderType = JSVAL_TO_INT( JL_ARG(1) );
	GLhandleARB handle = glCreateShaderObjectARB(shaderType);  OGL_CHK;
	*JL_RVAL = INT_TO_JSVAL(handle);
	
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME( shaderHandle )
  (TBD)
 $H OpenGL API
   glDeleteObjectARB
**/
DEFINE_FUNCTION( DeleteObjectARB ) {

	JL_INIT_OPENGL_EXTENSION( glDeleteObjectARB, PFNGLDELETEOBJECTARBPROC );
	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_INT(JL_ARG(1));
	GLhandleARB shaderHandle;
	shaderHandle = JSVAL_TO_INT( JL_ARG(1) );
	glDeleteObjectARB(shaderHandle);  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME( shaderHandle )
  (TBD)
 $H OpenGL API
   glGetInfoLogARB
**/
DEFINE_FUNCTION( GetInfoLogARB ) {

	JL_INIT_OPENGL_EXTENSION( glGetInfoLogARB, PFNGLGETINFOLOGARBPROC );
	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_INT(JL_ARG(1));
	GLhandleARB shaderHandle;
	shaderHandle = JSVAL_TO_INT( JL_ARG(1) );
	GLsizei length;
	glGetObjectParameterivARB(shaderHandle, GL_OBJECT_INFO_LOG_LENGTH_ARB, &length);  OGL_CHK;
	GLcharARB *buffer = (GLcharARB*)alloca(length+1);
	buffer[length] = '\0'; // needed ?
	glGetInfoLogARB(shaderHandle, length, &length, buffer);  OGL_CHK;
//	JL_CHK( JL_StringAndLengthToJsval(cx, JL_RVAL, buffer, length+1) );
	JL_CHK( JL_NativeToJsval(cx, buffer, length+1, JL_RVAL) );
	
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME()
  (TBD)
 $H OpenGL API
   glCreateProgramObjectARB
**/
DEFINE_FUNCTION( CreateProgramObjectARB ) {

	JL_INIT_OPENGL_EXTENSION( glCreateProgramObjectARB, PFNGLCREATEPROGRAMOBJECTARBPROC );
	GLhandleARB programHandle = glCreateProgramObjectARB();  OGL_CHK;
	*JL_RVAL = INT_TO_JSVAL(programHandle);
	
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME( shaderHandle, source )
  (TBD)
 $H OpenGL API
   glShaderSourceARB
**/
DEFINE_FUNCTION( ShaderSourceARB ) {

	JLStr source;
	JL_INIT_OPENGL_EXTENSION( glShaderSourceARB, PFNGLSHADERSOURCEARBPROC );
	JL_S_ASSERT_ARG(2);
	JL_S_ASSERT_INT(JL_ARG(1));
	JL_S_ASSERT_STRING(JL_ARG(2));
	GLhandleARB shaderHandle;
	shaderHandle = JSVAL_TO_INT( JL_ARG(1) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &source) );

	const GLcharARB *buffer;
	GLint length;
	length = source.Length();
	buffer = source.GetConstStr();

	glShaderSourceARB(shaderHandle, 1, &buffer, &length);  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME( shaderHandle )
  (TBD)
 $H OpenGL API
   glCompileShaderARB
**/
DEFINE_FUNCTION( CompileShaderARB ) {

	JL_INIT_OPENGL_EXTENSION( glCompileShaderARB, PFNGLCOMPILESHADERARBPROC );
	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_INT(JL_ARG(1));
	GLhandleARB shaderHandle;
	shaderHandle = JSVAL_TO_INT( JL_ARG(1) );
	glCompileShaderARB(shaderHandle);  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME( programHandle, shaderHandle )
  (TBD)
 $H OpenGL API
   glCompileShaderARB
**/
DEFINE_FUNCTION( AttachObjectARB ) {

	JL_INIT_OPENGL_EXTENSION( glAttachObjectARB, PFNGLATTACHOBJECTARBPROC );
	JL_S_ASSERT_ARG(2);
	JL_S_ASSERT_INT(JL_ARG(1));
	GLhandleARB programHandle;
	programHandle = JSVAL_TO_INT( JL_ARG(1) );
	GLhandleARB shaderHandle;
	shaderHandle = JSVAL_TO_INT( JL_ARG(2) );
	glAttachObjectARB(programHandle, shaderHandle);  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME( programHandle )
  (TBD)
 $H OpenGL API
   glLinkProgramARB
**/
DEFINE_FUNCTION( LinkProgramARB ) {

	JL_INIT_OPENGL_EXTENSION( glLinkProgramARB, PFNGLLINKPROGRAMARBPROC );
	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_INT(JL_ARG(1));
	GLhandleARB programHandle;
	programHandle = JSVAL_TO_INT( JL_ARG(1) );
	glLinkProgramARB(programHandle);  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME( programHandle )
  (TBD)
 $H OpenGL API
   glUseProgramObjectARB
**/
DEFINE_FUNCTION( UseProgramObjectARB ) {

	JL_INIT_OPENGL_EXTENSION( glUseProgramObjectARB, PFNGLUSEPROGRAMOBJECTARBPROC );
	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_INT(JL_ARG(1));
	GLhandleARB programHandle;
	programHandle = JSVAL_TO_INT( JL_ARG(1) );
	glUseProgramObjectARB(programHandle);  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME( programHandle, name )
  (TBD)
 $H OpenGL API
   glGetUniformLocationARB
**/
DEFINE_FUNCTION( GetUniformLocationARB ) {

	JLStr name;

	JL_INIT_OPENGL_EXTENSION( glGetUniformLocationARB, PFNGLGETUNIFORMLOCATIONARBPROC );
	JL_S_ASSERT_ARG(2);
	JL_S_ASSERT_INT(JL_ARG(1));
	JL_S_ASSERT_STRING(JL_ARG(2));
	GLhandleARB programHandle;
	programHandle = JSVAL_TO_INT( JL_ARG(1) );

	JL_JsvalToNative(cx, JL_ARG(2), &name);
	int uniformLocation;
	uniformLocation = glGetUniformLocationARB(programHandle, name);  OGL_CHK;
	*JL_RVAL = INT_TO_JSVAL(uniformLocation);
	
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME( uniformLocation, value )
  (TBD)
 $H OpenGL API
  glUniform1fARB, glUniform1iARB
**/
DEFINE_FUNCTION( UniformARB ) {

	JL_INIT_OPENGL_EXTENSION( glUniform1fARB, PFNGLUNIFORM1FARBPROC );
	JL_INIT_OPENGL_EXTENSION( glUniform2fARB, PFNGLUNIFORM2FARBPROC );
	JL_INIT_OPENGL_EXTENSION( glUniform3fARB, PFNGLUNIFORM3FARBPROC );
	JL_INIT_OPENGL_EXTENSION( glUniform4fARB, PFNGLUNIFORM4FARBPROC );

	JL_INIT_OPENGL_EXTENSION( glUniform1fvARB, PFNGLUNIFORM1FVARBPROC );
	JL_INIT_OPENGL_EXTENSION( glUniform2fvARB, PFNGLUNIFORM2FVARBPROC );
	JL_INIT_OPENGL_EXTENSION( glUniform3fvARB, PFNGLUNIFORM3FVARBPROC );
	JL_INIT_OPENGL_EXTENSION( glUniform4fvARB, PFNGLUNIFORM4FVARBPROC );

	JL_INIT_OPENGL_EXTENSION( glGetUniformfvARB, PFNGLGETUNIFORMFVARBPROC );

	JL_INIT_OPENGL_EXTENSION( glUniform1iARB, PFNGLUNIFORM1IARBPROC );
	JL_INIT_OPENGL_EXTENSION( glUniform2iARB, PFNGLUNIFORM2IARBPROC );
	JL_INIT_OPENGL_EXTENSION( glUniform3iARB, PFNGLUNIFORM3IARBPROC );
	JL_INIT_OPENGL_EXTENSION( glUniform4iARB, PFNGLUNIFORM4IARBPROC );

	JL_INIT_OPENGL_EXTENSION( glUniform1ivARB, PFNGLUNIFORM1IVARBPROC );
	JL_INIT_OPENGL_EXTENSION( glUniform2ivARB, PFNGLUNIFORM2IVARBPROC );
	JL_INIT_OPENGL_EXTENSION( glUniform3ivARB, PFNGLUNIFORM3IVARBPROC );
	JL_INIT_OPENGL_EXTENSION( glUniform4ivARB, PFNGLUNIFORM4IVARBPROC );

	JL_INIT_OPENGL_EXTENSION( glGetUniformivARB, PFNGLGETUNIFORMIVARBPROC );

	JL_INIT_OPENGL_EXTENSION( glUniformMatrix2fvARB, PFNGLUNIFORMMATRIX2FVARBPROC );
	JL_INIT_OPENGL_EXTENSION( glUniformMatrix3fvARB, PFNGLUNIFORMMATRIX3FVARBPROC );
	JL_INIT_OPENGL_EXTENSION( glUniformMatrix4fvARB, PFNGLUNIFORMMATRIX4FVARBPROC );


	JL_S_ASSERT_ARG_RANGE(2, 5);
	JL_S_ASSERT_INT(JL_ARG(1));
	int uniformLocation;
	uniformLocation = JSVAL_TO_INT(JL_ARG(1));
	*JL_RVAL = JSVAL_VOID;
	
	jsval arg2;
	arg2 = JL_ARG(2);

	float v1, v2, v3, v4;

	if ( JL_ARGC == 2 ) {
		
		if ( JSVAL_IS_INT(arg2) ) {

			glUniform1iARB(uniformLocation, JSVAL_TO_INT(arg2));  OGL_CHK;
			return JS_TRUE;
		}
		if ( JSVAL_IS_BOOLEAN(arg2) ) {

			glUniform1iARB(uniformLocation, arg2 == JSVAL_TRUE ? 1 : 0);  OGL_CHK;
			return JS_TRUE;
		}
	}

	if ( JSVAL_IS_NUMBER(arg2) ) {

		JL_CHK( JL_JsvalToNative(cx, arg2, &v1) );
		if ( JL_ARGC >= 3 ) {
		
			JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &v2) );
			if ( JL_ARGC >= 4 ) {
			
				JL_CHK( JL_JsvalToNative(cx, JL_ARG(4), &v3) );
				if ( JL_ARGC >= 5 ) {
				
					JL_CHK( JL_JsvalToNative(cx, JL_ARG(5), &v4) );
					glUniform4fARB(uniformLocation, v1, v2, v3, v4);  OGL_CHK;
					return JS_TRUE;
				}
				glUniform3fARB(uniformLocation, v1, v2, v3);  OGL_CHK;
				return JS_TRUE;
			}
			glUniform2fARB(uniformLocation, v1, v2);  OGL_CHK;
			return JS_TRUE;
		}
		glUniform1fARB(uniformLocation, v1);  OGL_CHK;
		return JS_TRUE;
	}
	
	if ( JL_IsArray(cx, arg2) ) {

		JSObject *arrayObj;
		arrayObj = JSVAL_TO_OBJECT(JL_ARG(2));
		uint32 currentLength;
		JL_CHK( JS_GetArrayLength(cx, arrayObj, &currentLength) );
// (TBD)		
	}

	JL_REPORT_ERROR("Invalid argument.");
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME( uniformLocation, v1 [, v2 [, v3 [, v4]]] )
  (TBD)
 $H OpenGL API
  glUniformMatrix4fvARB
**/
DEFINE_FUNCTION( UniformMatrixARB ) {

	JL_INIT_OPENGL_EXTENSION( glUniformMatrix4fvARB, PFNGLUNIFORMMATRIX4FVARBPROC );

	JL_S_ASSERT_ARG(2);
	JL_S_ASSERT_INT(JL_ARG(1));
	int uniformLocation;
	uniformLocation = JSVAL_TO_INT(JL_ARG(1));
	*JL_RVAL = JSVAL_VOID;

	float tmp[16], *m = tmp;
	JL_CHK( JL_JsvalToMatrix44(cx, JL_ARG(2), &m) );
	glUniformMatrix4fvARB(uniformLocation, 1, false, m);  OGL_CHK;

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME( uniformLocation, v1 [, v2 [, v3 [, v4]]] )
  (TBD)
 $H OpenGL API
  glUniform1fARB, glUniform2fARB, glUniform3fARB, glUniform4fARB
**/
DEFINE_FUNCTION( UniformFloatARB ) {

	JL_INIT_OPENGL_EXTENSION( glUniform1fARB, PFNGLUNIFORM1FARBPROC );
	JL_INIT_OPENGL_EXTENSION( glUniform2fARB, PFNGLUNIFORM2FARBPROC );
	JL_INIT_OPENGL_EXTENSION( glUniform3fARB, PFNGLUNIFORM3FARBPROC );
	JL_INIT_OPENGL_EXTENSION( glUniform4fARB, PFNGLUNIFORM4FARBPROC );

	JL_S_ASSERT_ARG_RANGE(2, 5);
	JL_S_ASSERT_INT(JL_ARG(1));
	int uniformLocation;
	uniformLocation = JSVAL_TO_INT(JL_ARG(1));
	*JL_RVAL = JSVAL_VOID;
	jsval arg2;
	arg2 = JL_ARG(2);
	float v1, v2, v3, v4;
	if ( JSVAL_IS_NUMBER(arg2) ) {

		JL_CHK( JL_JsvalToNative(cx, arg2, &v1) );
		if ( JL_ARGC >= 3 ) {
		
			JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &v2) );
			if ( JL_ARGC >= 4 ) {
			
				JL_CHK( JL_JsvalToNative(cx, JL_ARG(4), &v3) );
				if ( JL_ARGC >= 5 ) {
				
					JL_CHK( JL_JsvalToNative(cx, JL_ARG(5), &v4) );
					glUniform4fARB(uniformLocation, v1, v2, v3, v4);  OGL_CHK;
					return JS_TRUE;
				}
				glUniform3fARB(uniformLocation, v1, v2, v3);  OGL_CHK;
				return JS_TRUE;
			}
			glUniform2fARB(uniformLocation, v1, v2);  OGL_CHK;
			return JS_TRUE;
		}
		glUniform1fARB(uniformLocation, v1);  OGL_CHK;
		return JS_TRUE;
	}

	JL_REPORT_ERROR("Invalid argument.");
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME( uniformLocation, v1 [, v2 [, v3 [, v4]]] )
  (TBD)
 $H OpenGL API
  glUniform1iARB, glUniform2iARB, glUniform3iARB, glUniform4iARB
**/
DEFINE_FUNCTION( UniformIntegerARB ) {

	JL_INIT_OPENGL_EXTENSION( glUniform1iARB, PFNGLUNIFORM1IARBPROC );
	JL_INIT_OPENGL_EXTENSION( glUniform2iARB, PFNGLUNIFORM2IARBPROC );
	JL_INIT_OPENGL_EXTENSION( glUniform3iARB, PFNGLUNIFORM3IARBPROC );
	JL_INIT_OPENGL_EXTENSION( glUniform4iARB, PFNGLUNIFORM4IARBPROC );

	JL_S_ASSERT_ARG_RANGE(2, 5);
	JL_S_ASSERT_INT(JL_ARG(1));
	int uniformLocation;
	uniformLocation = JSVAL_TO_INT(JL_ARG(1));
	*JL_RVAL = JSVAL_VOID;
	jsval arg2;
	arg2 = JL_ARG(2);
	int v1, v2, v3, v4;

	if ( JSVAL_IS_NUMBER(arg2) ) {

		JL_CHK( JL_JsvalToNative(cx, arg2, &v1) );
		if ( JL_ARGC >= 3 ) {
		
			JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &v2) );
			if ( JL_ARGC >= 4 ) {
			
				JL_CHK( JL_JsvalToNative(cx, JL_ARG(4), &v3) );
				if ( JL_ARGC >= 5 ) {
				
					JL_CHK( JL_JsvalToNative(cx, JL_ARG(5), &v4) );
					glUniform4iARB(uniformLocation, v1, v2, v3, v4);  OGL_CHK;
					return JS_TRUE;
				}
				glUniform3iARB(uniformLocation, v1, v2, v3);  OGL_CHK;
				return JS_TRUE;
			}
			glUniform2iARB(uniformLocation, v1, v2);  OGL_CHK;
			return JS_TRUE;
		}
		glUniform1iARB(uniformLocation, v1);  OGL_CHK;
		return JS_TRUE;
	}

	JL_REPORT_ERROR("Invalid argument.");
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME( obj, pname )
  (TBD)
 $H OpenGL API
  glGetObjectParameterfvARB, glGetObjectParameterivARB
**/
DEFINE_FUNCTION( GetObjectParameterARB ) {

	JL_INIT_OPENGL_EXTENSION( glGetObjectParameterfvARB, PFNGLGETOBJECTPARAMETERFVARBPROC );
	JL_INIT_OPENGL_EXTENSION( glGetObjectParameterivARB, PFNGLGETOBJECTPARAMETERIVARBPROC );

	JL_S_ASSERT_ARG_RANGE(2,3);
	JL_S_ASSERT_INT(JL_ARG(1));
	JL_S_ASSERT_INT(JL_ARG(2));

	GLint params[16]; // (TBD) check if it is the max amount of data that glGetLightfv may returns.
	glGetObjectParameterivARB(JSVAL_TO_INT(JL_ARG(1)), JSVAL_TO_INT(JL_ARG(2)), params);  OGL_CHK;

	if ( JL_ARG_ISDEF(3) ) {

		JL_S_ASSERT_INT( JL_ARG(3) );
		int count = JSVAL_TO_INT( JL_ARG(3) );
		JSObject *arrayObj = JS_NewArrayObject(cx, count, NULL);
		JL_CHK( arrayObj );
		*JL_RVAL = OBJECT_TO_JSVAL(arrayObj);
		jsval tmpValue;
		while (count--) {

			JL_CHK( JL_NativeToJsval(cx, params[count], &tmpValue) );
			JL_CHK( JS_SetElement(cx, arrayObj, count, &tmpValue) );
		}
	} else {

		JL_CHK( JL_NativeToJsval(cx, params[0], JL_RVAL) );
	}

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME( programObj, index, name )
  (TBD)
 $H OpenGL API
  glBindAttribLocationARB
**/
DEFINE_FUNCTION( BindAttribLocationARB ) {

	JLStr name;

	JL_INIT_OPENGL_EXTENSION( glBindAttribLocationARB, PFNGLBINDATTRIBLOCATIONARBPROC );

	JL_S_ASSERT_ARG(3);
	JL_S_ASSERT_INT(JL_ARG(1));
	JL_S_ASSERT_INT(JL_ARG(2));

	JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &name) );
	glBindAttribLocationARB(JSVAL_TO_INT(JL_ARG(1)), JSVAL_TO_INT(JL_ARG(2)), name);  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME( programObj, name )
  (TBD)
 $H OpenGL API
  glGetAttribLocationARB
**/
DEFINE_FUNCTION( GetAttribLocationARB ) {

	JLStr name;

	JL_INIT_OPENGL_EXTENSION( glGetAttribLocationARB, PFNGLGETATTRIBLOCATIONARBPROC );

	JL_S_ASSERT_ARG(2);
	JL_S_ASSERT_INT(JL_ARG(1));

	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &name) );
	int location;
	location = glGetAttribLocationARB(JSVAL_TO_INT(JL_ARG(1)), name);  OGL_CHK;
	*JL_RVAL = INT_TO_JSVAL(location);

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME( index, value )
  (TBD)
 $H OpenGL API
  glVertexAttrib1dARB
**/
DEFINE_FUNCTION( VertexAttribARB ) {

	JL_INIT_OPENGL_EXTENSION( glVertexAttrib1sARB, PFNGLVERTEXATTRIB1SARBPROC );

	JL_INIT_OPENGL_EXTENSION( glVertexAttrib1dARB, PFNGLVERTEXATTRIB1DARBPROC );
	JL_INIT_OPENGL_EXTENSION( glVertexAttrib2dARB, PFNGLVERTEXATTRIB2DARBPROC );
	JL_INIT_OPENGL_EXTENSION( glVertexAttrib3dARB, PFNGLVERTEXATTRIB3DARBPROC );
	JL_INIT_OPENGL_EXTENSION( glVertexAttrib4dARB, PFNGLVERTEXATTRIB4DARBPROC );

	JL_S_ASSERT_ARG_RANGE(2, 5);
	JL_S_ASSERT_INT(JL_ARG(1));
	int index;
	index = JSVAL_TO_INT(JL_ARG(1));


	*JL_RVAL = JSVAL_VOID;
	
	jsval arg2;
	arg2 = JL_ARG(2);

	GLdouble v1, v2, v3, v4;

	if ( JL_ARGC == 2 && JSVAL_IS_INT(arg2) ) {

		glVertexAttrib1sARB(index, (GLshort)JSVAL_TO_INT(arg2));  OGL_CHK;
		return JS_TRUE;
	}

	if ( JSVAL_IS_NUMBER(arg2) ) {

		JL_CHK( JL_JsvalToNative(cx, arg2, &v1) );
		if ( JL_ARGC >= 3 ) {
		
			JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &v2) );
			if ( JL_ARGC >= 4 ) {
			
				JL_CHK( JL_JsvalToNative(cx, JL_ARG(4), &v3) );
				if ( JL_ARGC >= 5 ) {
				
					JL_CHK( JL_JsvalToNative(cx, JL_ARG(5), &v4) );
					glVertexAttrib4dARB(index, v1, v2, v3, v4);  OGL_CHK;
					return JS_TRUE;
				}
				glVertexAttrib3dARB(index, v1, v2, v3);  OGL_CHK;
				return JS_TRUE;
			}
			glVertexAttrib2dARB(index, v1, v2);  OGL_CHK;
			return JS_TRUE;
		}
		glVertexAttrib1dARB(index, v1);  OGL_CHK;
		return JS_TRUE;
	}

	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME()
  Returns a new buffer.
  $H OpenGL API
   glGenBuffers
**/
DEFINE_FUNCTION( GenBuffer ) {
	
	JL_INIT_OPENGL_EXTENSION( glGenBuffers, PFNGLGENBUFFERSPROC );
	
	JL_S_ASSERT_ARG(0);
	GLuint buffer;

	glGenBuffers(1, &buffer);  OGL_CHK;
	
	*JL_RVAL = INT_TO_JSVAL(buffer);

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
   glBindBuffer
**/
DEFINE_FUNCTION( BindBuffer ) {

	JL_INIT_OPENGL_EXTENSION( glBindBuffer, PFNGLBINDBUFFERPROC );

	JL_S_ASSERT_ARG(2);
	JL_S_ASSERT_INT(JL_ARG(1));
	JL_S_ASSERT_INT(JL_ARG(2));
	
	glBindBuffer(JSVAL_TO_INT(JL_ARG(1)), JSVAL_TO_INT(JL_ARG(2)));  OGL_CHK;

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/* *doc
$TOC_MEMBER $INAME
 $VOID $INAME( target, size )
  $H arguments
   $ARG GLenum target
   $ARG $INT buffer
  $H OpenGL API
   glBindBufferARB
**/
/*
DEFINE_FUNCTION( BufferData ) {

	LOAD_OPENGL_EXTENSION( glBufferDataARB, PFNGLBUFFERDATAARBPROC ); // glBufferDataARB (GLenum target, GLsizeiptrARB size, const GLvoid *data, GLenum usage);

	// see http://www.songho.ca/opengl/gl_pbo.html

	JL_S_ASSERT_ARG_MIN(2);
	JL_S_ASSERT_INT(JL_ARG(1));
	JL_S_ASSERT_INT(JL_ARG(2));
	GLenum target = JSVAL_TO_INT(JL_ARG(1));
	GLenum buffer = JSVAL_TO_INT(JL_ARG(2));
	
	glBufferDataARB(target, buffer);  OGL_CHK;
	*JL_RVAL = JSVAL_VOID;
	;
	return JS_TRUE;
	JL_BAD;
}
*/


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( pname, params )
  $H arguments
   $ARG GLenum pname
   $ARG $VAL params: is a real or an Array of real.
  $H OpenGL API
   glPointParameterf, glPointParameterfv
**/
DEFINE_FUNCTION( PointParameter ) {

	JL_INIT_OPENGL_EXTENSION( glPointParameteri, PFNGLPOINTPARAMETERIPROC );
	JL_INIT_OPENGL_EXTENSION( glPointParameterf, PFNGLPOINTPARAMETERFPROC );
	JL_INIT_OPENGL_EXTENSION( glPointParameterfv, PFNGLPOINTPARAMETERFVPROC );

	JL_S_ASSERT_ARG(2);
	JL_S_ASSERT_INT(JL_ARG(1));

	*JL_RVAL = JSVAL_VOID;
	if ( JSVAL_IS_INT(JL_ARG(2)) ) {

		glPointParameteri(JSVAL_TO_INT(JL_ARG(1)), JSVAL_TO_INT(JL_ARG(2)));  OGL_CHK;
		;
		return JS_TRUE;
	}
	if ( JSVAL_IS_DOUBLE(JL_ARG(2)) ) {

		float param;
		JL_JsvalToNative(cx, JL_ARG(2), &param);

		glPointParameterf( JSVAL_TO_INT(JL_ARG(1)), param );  OGL_CHK;
		
		;
		return JS_TRUE;
	}
	if ( JL_IsArray(cx, JL_ARG(2)) ) {

		GLfloat params[16];
		uint32 length;
		JL_CHK( JL_JsvalToCValVector(cx, JL_ARG(2), params, COUNTOF(params), &length ) );
		glPointParameterfv( JSVAL_TO_INT(JL_ARG(1)), params );  OGL_CHK;
		;
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
   glActiveTexture
**/
DEFINE_FUNCTION( ActiveTexture ) {

	JL_INIT_OPENGL_EXTENSION( glActiveTexture, PFNGLACTIVETEXTUREPROC );

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_INT(JL_ARG(1));
	
	glActiveTexture(JSVAL_TO_INT(JL_ARG(1)));  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( texture )
  $H arguments
   $ARG GLenum texture
  $H OpenGL API
   glClientActiveTexture
**/
DEFINE_FUNCTION( ClientActiveTexture ) {

	JL_INIT_OPENGL_EXTENSION( glClientActiveTexture, PFNGLCLIENTACTIVETEXTUREPROC );

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_INT(JL_ARG(1));

	glClientActiveTexture(JSVAL_TO_INT(JL_ARG(1)));  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
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
DEFINE_FUNCTION( MultiTexCoord ) {

	JL_INIT_OPENGL_EXTENSION( glMultiTexCoord1d, PFNGLMULTITEXCOORD1DPROC );
	JL_INIT_OPENGL_EXTENSION( glMultiTexCoord2d, PFNGLMULTITEXCOORD2DPROC );
	JL_INIT_OPENGL_EXTENSION( glMultiTexCoord3d, PFNGLMULTITEXCOORD3DPROC );

	JL_S_ASSERT_ARG_RANGE(2,4);
	*JL_RVAL = JSVAL_VOID;

	JL_S_ASSERT_INT(JL_ARG(1));
	GLenum target = JSVAL_TO_INT(JL_ARG(1));

	double s;
	JL_JsvalToNative(cx, JL_ARG(2), &s);
	if ( JL_ARGC == 2 ) {

		glMultiTexCoord1d(target, s);  OGL_CHK;
		;
		return JS_TRUE;
	}
	double t;
	JL_JsvalToNative(cx, JL_ARG(3), &t);
	if ( JL_ARGC == 3 ) {

		glMultiTexCoord2d(target, s, t);  OGL_CHK;
		;
		return JS_TRUE;
	}
	double r;
	JL_JsvalToNative(cx, JL_ARG(4), &r);
	if ( JL_ARGC == 4 ) {

		glMultiTexCoord3d(target, s, t, r);  OGL_CHK;
		;
		return JS_TRUE;
	}
	JL_REPORT_ERROR("Invalid argument.");
	JL_BAD;
}



/* *doc
$TOC_MEMBER $INAME
 $INT $INAME( target, attachment, pname [, count] )
  $H arguments
   $ARG enum target
  $H OpenGL API
**/
/*
DEFINE_FUNCTION( CreatePbuffer ) {


	;
	return JS_TRUE;
	JL_BAD;
}
*/



///////////////////////////////////////////////////////////////////////////////
// non-OpenGL API




/**doc
$TOC_MEMBER $INAME
 $TYPE vec3 $INAME( x, y )
  $H API
   gluLookAt
**/
DEFINE_FUNCTION( UnProject ) {

	JL_S_ASSERT_ARG(2);
	JL_S_ASSERT_INT( JL_ARG(1) );
	JL_S_ASSERT_INT( JL_ARG(2) );

	int x, y;
	x = JSVAL_TO_INT( JL_ARG(1) );
	y = JSVAL_TO_INT( JL_ARG(2) );

   GLint viewport[4];
   GLdouble mvmatrix[16], projmatrix[16];
   GLint realy;
   GLdouble w[3];

	glGetIntegerv(GL_VIEWPORT, viewport);  OGL_CHK;
   glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix);  OGL_CHK;
   glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);  OGL_CHK;
	realy = viewport[3] - (GLint) y - 1;

	gluUnProject((GLdouble) x, (GLdouble) realy, 0.0, mvmatrix, projmatrix, viewport, w+0, w+1, w+2);

	JL_CHK( JL_CValVectorToJsval(cx, w, 3, JL_RVAL, false) );

	return JS_TRUE;
	JL_BAD;
}




/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( $UNDEF, texture [, internalformat] )
  $H OpenGL API
   glDrawPixels
**/
DEFINE_FUNCTION( DrawImage ) {

	JLStr dataStr;

	JL_S_ASSERT_ARG_RANGE(1,2);
	JL_S_ASSERT_OBJECT(JL_ARG(1));

	GLsizei width, height;
	GLenum format, type;
	int channels;
	const GLvoid *data;
	
	JL_S_ASSERT_OBJECT( JL_ARG(1) );
	JSObject *tObj = JSVAL_TO_OBJECT( JL_ARG(1) );

	if ( JL_GetClass(tObj) == JL_TextureJSClass(cx) ) {

		TextureStruct *tex = (TextureStruct*)JL_GetPrivate(cx, tObj);
		JL_S_ASSERT_RESOURCE(tex);

		data = tex->cbuffer;
		width = tex->width;
		height = tex->height;
		channels = tex->channels;
		type = GL_FLOAT;
	} else {

		JL_CHK( JL_GetProperty(cx, tObj, "width", &width) );
		JL_CHK( JL_GetProperty(cx, tObj, "height", &height) );
		JL_CHK( JL_GetProperty(cx, tObj, "channels", &channels) );
		
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &dataStr) );
		data = dataStr.GetConstStr();

		JL_S_ASSERT( dataStr.Length() == width * height * channels * 1, "Invalid image format." );
		JL_S_ASSERT_RESOURCE(data);
		type = GL_UNSIGNED_BYTE;
	}

	if ( JL_ARG_ISDEF(2) ) {

		JL_S_ASSERT_INT(JL_ARG(2));
		format = JSVAL_TO_INT(JL_ARG(2));
	} else { // guess

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

	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );  OGL_CHK;
	glDrawPixels(width, height, format, type, data);  OGL_CHK;

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE image $INAME( [ flipY = true ] [, format = Ogl.RGBA ] )
  Returns the current contain of the viewport.
  $H return value
   An image object.
  $H note
   This is not an OpenGL API function.
  $H OpenGL API
   glGenTextures, glBindTexture, glGetIntegerv, glCopyTexImage2D, glGetTexLevelParameteriv, glGetTexImage, glDeleteTextures
**/
DEFINE_FUNCTION( ReadImage ) {

	JL_S_ASSERT_ARG_RANGE(0,2);

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);  OGL_CHK;
	int x = viewport[0];
	int y = viewport[1];
	int width = viewport[2];
	int height = viewport[3];

	bool flipY;
	if ( JL_ARG_ISDEF(1) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &flipY) );
	else 
		flipY = false;

	int channels;
	GLenum format;
	if ( JL_ARG_ISDEF(2) ) {

		format = JSVAL_TO_INT( JL_ARG(2) );
		switch ( format ) {
			case GL_RGBA:
				channels = 4;
				break;
			case GL_RGB:
				channels = 3;
				break;
			case GL_LUMINANCE_ALPHA:
				channels = 2;
				break;
			case GL_DEPTH_COMPONENT:
			case GL_RED:
			case GL_GREEN:
			case GL_BLUE:
			case GL_ALPHA:
			case GL_LUMINANCE:
			case GL_STENCIL_INDEX:
				channels = 1;
				break;
			default:
				JL_REPORT_ERROR("Unsupported format.");
		}
	} else {

		format = GL_RGBA;
		channels = 4;  // 4 for RGBA
	}

	int lineLength = width * channels;
	int length = lineLength * height;
	JL_S_ASSERT( length > 0, "Invalid image size." );
	GLvoid *data = JS_malloc(cx, length +1);
	JL_CHK( data );

/*
	GLuint texture;
	glGenTextures(1, &texture);  OGL_CHK;
	glBindTexture(GL_TEXTURE_2D, texture);  OGL_CHK;

	// see GL_ARB_texture_rectangle / ARB_texture_non_power_of_two

	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, width, height, 0);  OGL_CHK;

	GLint tWidth, tHeight;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &tWidth);  OGL_CHK;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &tHeight);  OGL_CHK;
	//glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPONENTS, &tComponents);  OGL_CHK;
	//  glGet	with arguments GL_PACK_ALIGNMENT and others

	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);  OGL_CHK;
	glDeleteTextures(1, &texture);  OGL_CHK;
*/
	
	glReadPixels(x, y, width, height, format, GL_UNSIGNED_BYTE, data);  OGL_CHK;

	if ( flipY ) { // Y-flip image

		void *tmp = alloca(lineLength);
		int mid = height / 2;
		for ( int line = 0; line < mid; ++line ) {

			memcpy(tmp, (char*)data + (line*lineLength), lineLength);
			memcpy((char*)data + (line*lineLength), (char*)data + ((height-1-line)*lineLength), lineLength);
			memcpy((char*)data + ((height-1-line)*lineLength), tmp, lineLength);
		}
	}

	((uint8_t*)data)[length] = 0;
	JL_CHK( JL_NewBlob(cx, data, length, JL_RVAL) );
	JSObject *blobObj;
	JL_CHK( JS_ValueToObject(cx, *JL_RVAL, &blobObj) );
	JL_S_ASSERT( blobObj, "Unable to create Blob object." );
	*JL_RVAL = OBJECT_TO_JSVAL(blobObj);

	JS_DefineProperty(cx, blobObj, "channels", INT_TO_JSVAL(channels), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT );
	JS_DefineProperty(cx, blobObj, "width", INT_TO_JSVAL(width), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT );
	JS_DefineProperty(cx, blobObj, "height", INT_TO_JSVAL(height), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT );

	return JS_TRUE;
	JL_BAD;
}





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


#define TRIMESH_ID_NAME JLHID(GlTr)

struct OpenGlTrimeshInfo {

	GLuint indexBuffer, vertexBuffer, normalBuffer, texCoordBuffer, colorBuffer;
	size_t vertexCount, indexCount;
};

void FinalizeTrimesh(void *pv) {

/* (TBD)!

	static PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC) GL_GET_PROC_ADDRESS( "PFNGLDELETEBUFFERSARBPROC" );

	OpenGlTrimeshInfo *info = (OpenGlTrimeshInfo*)pv;
	glDeleteBuffersARB( 1, &info->indexBuffer );  OGL_CHK;
	glDeleteBuffersARB( 1, &info->vertexBuffer );  OGL_CHK;
	if ( info->indexBuffer )
		glDeleteBuffersARB( 1, &info->indexBuffer );  OGL_CHK;
	if ( info->texCoordBuffer )
		glDeleteBuffersARB( 1, &info->texCoordBuffer );  OGL_CHK;
	if ( info->colorBuffer )
		glDeleteBuffersARB( 1, &info->colorBuffer );  OGL_CHK;
*/
}

/**doc
$TOC_MEMBER $INAME
 $TYPE trimeshId $INAME( trimesh )
**/
DEFINE_FUNCTION( LoadTrimesh ) {

	JL_INIT_OPENGL_EXTENSION( glGenBuffers, PFNGLGENBUFFERSPROC );
	JL_INIT_OPENGL_EXTENSION( glBindBuffer, PFNGLBINDBUFFERPROC );
	JL_INIT_OPENGL_EXTENSION( glBufferData, PFNGLBUFFERDATAPROC );

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_OBJECT(JL_ARG(1));
	JSObject *trimeshObj = JSVAL_TO_OBJECT(JL_ARG(1));
	JL_S_ASSERT( JL_JsvalIsTrimesh(cx, JL_ARG(1)), "Invalid Trimesh object" );
	Surface *srf = GetTrimeshSurface(cx, trimeshObj);
	JL_S_ASSERT_RESOURCE(srf);

	OpenGlTrimeshInfo *info;
	JL_CHK( HandleCreate(cx, TRIMESH_ID_NAME, sizeof(OpenGlTrimeshInfo), (void**)&info, FinalizeTrimesh, JL_RVAL) );

	if ( srf->vertex ) {
	
		info->vertexCount = srf->vertexCount;
		glGenBuffers(1, &info->vertexBuffer);  OGL_CHK;
		glBindBuffer(GL_ARRAY_BUFFER, info->vertexBuffer);  OGL_CHK;
		glBufferData(GL_ARRAY_BUFFER, srf->vertexCount * 3 * sizeof(SURFACE_REAL_TYPE), srf->vertex, GL_STATIC_DRAW);  OGL_CHK;
	} else
		info->vertexBuffer = 0;

	if ( srf->index ) {

		info->indexCount = srf->indexCount;
		glGenBuffers(1, &info->indexBuffer);  OGL_CHK;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, info->indexBuffer);  OGL_CHK;
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, srf->indexCount * sizeof(SURFACE_INDEX_TYPE), srf->index, GL_STATIC_DRAW);  OGL_CHK;
	} else
		info->indexBuffer = 0;

	if ( srf->normal ) {

		glGenBuffers(1, &info->normalBuffer);  OGL_CHK;
		glBindBuffer(GL_ARRAY_BUFFER, info->normalBuffer);  OGL_CHK;
		glBufferData(GL_ARRAY_BUFFER, srf->vertexCount * 3 * sizeof(SURFACE_REAL_TYPE), srf->normal, GL_STATIC_DRAW);  OGL_CHK;
	} else
		info->normalBuffer = 0;

	if ( srf->textureCoordinate ) {

		glGenBuffers(1, &info->texCoordBuffer);  OGL_CHK;
		glBindBuffer(GL_ARRAY_BUFFER, info->texCoordBuffer);  OGL_CHK;
		glBufferData(GL_ARRAY_BUFFER, srf->vertexCount * 3 * sizeof(SURFACE_REAL_TYPE), srf->textureCoordinate, GL_STATIC_DRAW);  OGL_CHK;
	} else
		info->texCoordBuffer = 0;

	if ( srf->color ) {

		glGenBuffers(1, &info->colorBuffer);  OGL_CHK;
		glBindBuffer(GL_ARRAY_BUFFER, info->colorBuffer);  OGL_CHK;
		glBufferData(GL_ARRAY_BUFFER, srf->vertexCount * 4 * sizeof(SURFACE_REAL_TYPE), srf->color, GL_STATIC_DRAW);  OGL_CHK;
	} else
		info->colorBuffer = 0;

	JL_CHK( CheckThrowCurrentOglError(cx) );

	;
	return JS_TRUE;
	JL_BAD;
}




/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( trimeshId [, mode ] )
  $H OpenGL API
   glVertexPointer
**/
DEFINE_FUNCTION( DrawTrimesh ) {

	JL_INIT_OPENGL_EXTENSION( glBindBuffer, PFNGLBINDBUFFERPROC );

	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT_OBJECT(JL_ARG(1));

	JL_S_ASSERT( IsHandleType(cx, JL_ARG(1), TRIMESH_ID_NAME), "Invalid Id." );

	OpenGlTrimeshInfo *info = (OpenGlTrimeshInfo*)GetHandlePrivate(cx, JL_ARG(1));

	GLenum dataType = sizeof(SURFACE_REAL_TYPE) == sizeof(float) ? GL_FLOAT : GL_DOUBLE;

	GLenum mode;
	if ( JL_ARG_ISDEF(2) )
		mode = JSVAL_TO_INT(JL_ARG(2));
	else
		mode = GL_TRIANGLES;

	if ( info->vertexBuffer ) {

		glEnableClientState(GL_VERTEX_ARRAY);  OGL_CHK;
		glBindBuffer(GL_ARRAY_BUFFER, info->vertexBuffer);  OGL_CHK;
		glVertexPointer(3, dataType, 0, 0);  OGL_CHK;
	}

	if ( info->normalBuffer ) {

		glEnableClientState(GL_NORMAL_ARRAY);  OGL_CHK;
		glBindBuffer(GL_ARRAY_BUFFER, info->normalBuffer);  OGL_CHK;
		glNormalPointer(dataType, 0, 0);  OGL_CHK;
	}

	if ( info->texCoordBuffer ) {

		glEnableClientState(GL_TEXTURE_COORD_ARRAY);  OGL_CHK;
		glBindBuffer(GL_ARRAY_BUFFER, info->texCoordBuffer);  OGL_CHK;
		glTexCoordPointer(3, dataType, 0, 0);  OGL_CHK;
	}

	if ( info->colorBuffer ) {

		glEnableClientState(GL_COLOR_ARRAY);  OGL_CHK;
		glBindBuffer(GL_ARRAY_BUFFER, info->colorBuffer);  OGL_CHK;
		glColorPointer(4, dataType, 0, 0);  OGL_CHK;
	}

	if ( info->indexBuffer ) {

		//	glEnableClientState(GL_INDEX_ARRAY);  OGL_CHK;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, info->indexBuffer);  OGL_CHK;
		glDrawElements(mode, (GLsizei)info->indexCount, GL_UNSIGNED_INT, 0);  OGL_CHK; // 1 triangle = 3 vertex
	} else {
		
		if ( info->vertexBuffer )
			glDrawArrays(mode, 0, (GLsizei)info->vertexCount);  OGL_CHK;
	}

//	glDisableClientState(GL_INDEX_ARRAY);  OGL_CHK;
	if ( info->colorBuffer )
		glDisableClientState(GL_COLOR_ARRAY);  OGL_CHK;
	if ( info->texCoordBuffer )
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);  OGL_CHK;
	if ( info->normalBuffer )
		glDisableClientState(GL_NORMAL_ARRAY);  OGL_CHK;

	glDisableClientState(GL_VERTEX_ARRAY);  OGL_CHK; // deactivate vertex array

	// bind with 0, so, switch back to normal pointer operation
//	glBindBuffer(GL_ARRAY_BUFFER, 0);  OGL_CHK;
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);  OGL_CHK;

	JL_CHK( CheckThrowCurrentOglError(cx) );

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( target, access )
  $H arguments
   $ARG enum target
   $ARG enum access
  $H OpenGL API
   glMapBuffer
**/

bool TextureBufferAlloc(TextureBuffer *tb, unsigned int size) {

	INIT_OPENGL_EXTENSION( glBufferData, PFNGLBUFFERDATAPROC );
	INIT_OPENGL_EXTENSION( glMapBuffer, PFNGLMAPBUFFERPROC );
	INIT_OPENGL_EXTENSION( glBindBuffer, PFNGLBINDBUFFERPROC );

	if ( glBufferData == NULL || glMapBuffer == NULL || glBindBuffer == NULL )
		return false;

	GLuint pbo = (GLuint)tb->pv;
	
	// the target tokens clearly specify the bound PBO will be used in one of 2 different operations; 
	//GL_PIXEL_PACK_BUFFER to transfer pixel data to a PBO, or GL_PIXEL_UNPACK_BUFFER to transfer pixel data from PBO. 
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
	
	// VBO memory manager will choose the best memory places for the buffer object based on these usage flags,for example, 
	// GL_STATIC_DRAW and GL_STREAM_DRAW may use video memory, and GL_DYNAMIC_DRAW may use AGP memory.
	// Any _READ_ related buffers would be fine in system or AGP memory because the data should be easy to access.
	glBufferData(GL_PIXEL_UNPACK_BUFFER, size, NULL, GL_DYNAMIC_READ);
	
	// glMapBuffer() returns the pointer to the buffer object if success. Otherwise it returns NULL.
	// The target parameter is GL_PIXEL_PACK_BUFFER or GL_PIXEL_UNPACK_BUFFER. The second parameter,
	// access specifies what to do with the mapped buffer; read data from the PBO (GL_READ_ONLY),
	// write data to the PBO (GL_WRITE_ONLY), or both (GL_READ_WRITE). 
	tb->data = (float*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_READ_WRITE);
	return true;
}

bool TextureBufferFree(TextureBuffer *tb) {

	if ( !glUnmapBuffer )
		return false;
	
	if ( tb->pv != NULL ) {
		
		GLuint pbo = (GLuint)tb->pv;
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
		glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
		tb->pv = NULL;
	}
	return true;
}

void TextureBufferFinalize(void* data) {

	TextureBuffer *tb = (TextureBuffer*)data;
	TextureBufferFree(tb);
}

// OpenGL Pixel Buffer Object: http://www.songho.ca/opengl/gl_pbo.html


DEFINE_FUNCTION( CreateTextureBuffer ) {

	JL_INIT_OPENGL_EXTENSION( glGenBuffers, PFNGLGENBUFFERSPROC );
//	JL_INIT_OPENGL_EXTENSION( glBindBuffer, PFNGLBINDBUFFERPROC );

	TextureBuffer *tb;
	JL_CHK( HandleCreate(cx, JLHID(TBUF), sizeof(TextureBuffer), (void**)&tb, TextureBufferFinalize, JL_RVAL) );
	GLuint pbo;
	glGenBuffers(1, &pbo);  OGL_CHK;
	tb->pv = (void*)pbo;

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( target, internalformat | $UNDEF, texture )
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
DEFINE_FUNCTION( DefineTextureImage ) {

	JLStr dataStr;
	JL_S_ASSERT_ARG(3);
	JL_S_ASSERT_INT(JL_ARG(1));
//	JL_S_ASSERT_INT(JL_ARG(2)); // may be undefined
	JL_S_ASSERT_OBJECT(JL_ARG(3));

	GLsizei width, height;
	GLenum format, type;
	int channels;
	const GLvoid *data;

	JSObject *tObj = JSVAL_TO_OBJECT(JL_ARG(3));

	if ( JL_GetClass(tObj) == JL_TextureJSClass(cx) ) {

		TextureStruct *tex = (TextureStruct*)JL_GetPrivate(cx, tObj);
		JL_S_ASSERT_RESOURCE(tex);

		data = tex->cbuffer;
		width = tex->width;
		height = tex->height;
		channels = tex->channels;
		type = GL_FLOAT;
	} else {

		JL_CHKM( JL_GetProperty(cx, tObj, "width", &width), "Invalid texture object." );
		JL_CHKM( JL_GetProperty(cx, tObj, "height", &height), "Invalid texture object." );
		JL_CHKM( JL_GetProperty(cx, tObj, "channels", &channels), "Invalid texture object." );
		
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &dataStr) );
		data = dataStr.GetConstStr();

		JL_S_ASSERT( dataStr.Length() == width * height * channels * 1, "Invalid image format." );
		JL_S_ASSERT_RESOURCE(data);
		type = GL_UNSIGNED_BYTE;
	}
// else
//		JL_REPORT_ERROR("Invalid texture type.");

	if ( JL_ARG_ISDEF(2) ) {

		JL_S_ASSERT_INT(JL_ARG(2));
		format = JSVAL_TO_INT(JL_ARG(2));
	} else { // guess

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
			default:
				JL_REPORT_ERROR("Invalid texture format.");
		}
	}

	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );  OGL_CHK;
	glTexImage2D( JSVAL_TO_INT(JL_ARG(1)), 0, format, width, height, 0, format, type, data );  OGL_CHK;
//	GLenum err = glGetError();

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $REAL $INAME()
  pixelWidth = PixelWidthFactor() * width / distance
**/
DEFINE_FUNCTION( PixelWidthFactor ) {

	// see. http://www.songho.ca/opengl/gl_projectionmatrix.html
	// see. engine_core.h

	JL_S_ASSERT_ARG(0);
	GLint viewport[4];
	GLfloat m[16];
	glGetIntegerv(GL_VIEWPORT, viewport);  OGL_CHK;
	glGetFloatv(GL_PROJECTION_MATRIX, m);  OGL_CHK;

	float w;
	w = viewport[2] * m[0];
//	float h;
//	h = viewport[3] * m[5];

	return JL_NativeToJsval(cx, w, JL_RVAL); // sqrt(w*w+h*h)
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME( size )
**/
DEFINE_FUNCTION( DrawPoint ) {

	JL_S_ASSERT_ARG(1);
	float size;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &size) );
	glPointSize(size);  OGL_CHK; // get max with GL_POINT_SIZE_RANGE
	glBegin(GL_POINTS);
	glVertex2i(0,0);
	glEnd();  OGL_CHK;

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

/*
/ **doc
$TOC_MEMBER $INAME
$INAME( radius [ , vertexCount = 12 ] )
** /
DEFINE_FUNCTION( DrawDisk ) {

	float s, c, angle, radius;
	int vertexCount;
	JL_S_ASSERT_ARG_RANGE(1,2);
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &radius) );
	if ( JL_ARG_ISDEF(2) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &vertexCount) );
	else
		vertexCount = 12;
	angle = 2*M_PI / vertexCount;
	glBegin(GL_POLYGON);  OGL_CHK;
	for (int i = 0; i < vertexCount; i++) {

		SinCos(i * angle, &s, &c);
		glTexCoord2f(c / 2.f + 0.5f, s / 2.f + 0.5f);  OGL_CHK;
		glVertex2f(c * radius, s * radius);  OGL_CHK;
	}
	glEnd();  OGL_CHK;
	;
	return JS_TRUE;
	JL_BAD;
}
*/


/**doc
$TOC_MEMBER $INAME
$INAME( radius, slices, stacks );
**/
DEFINE_FUNCTION( DrawSphere ) {

	JL_S_ASSERT_ARG(3);
	double radius;
	int slices, stacks;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &radius) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &slices) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &stacks) );

	GLUquadric *q = gluNewQuadric();
	gluQuadricTexture(q, GL_FALSE);
	gluQuadricNormals(q, GLU_SMOOTH); // GLU_FLAT / GLU_SMOOTH
	gluSphere(q, radius, slices, stacks);  OGL_CHK;
	gluDeleteQuadric(q);

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
$INAME( radius, slices, loops );
**/
DEFINE_FUNCTION( DrawDisk ) {

	JL_S_ASSERT_ARG(3);
	double radius;
	int slices, loops;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &radius) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &slices) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &loops) );

	GLUquadric *q = gluNewQuadric();
	gluQuadricTexture(q, GL_FALSE);
	gluQuadricNormals(q, GLU_SMOOTH); // GLU_FLAT / GLU_SMOOTH
	gluDisk(q, 0, radius, slices, loops);  OGL_CHK;
	gluDeleteQuadric(q);

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
$INAME( baseRadius, topRadius, height, slices, stacks );
**/
DEFINE_FUNCTION( DrawCylinder ) {

	JL_S_ASSERT_ARG(5);
	double baseRadius, topRadius, height;
	int slices, stacks;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &baseRadius) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &topRadius) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &height) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(4), &slices) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(5), &stacks) );

	GLUquadric *q = gluNewQuadric();
	gluQuadricTexture(q, GL_FALSE); // GL_TRUE
	gluQuadricNormals(q, GLU_SMOOTH); // GLU_NONE / GLU_FLAT / GLU_SMOOTH
	gluQuadricOrientation(q, GLU_OUTSIDE); //  GLU_INSIDE
	gluQuadricDrawStyle(q, GLU_FILL); // GLU_FILL / GLU_LINE / GLU_SILHOUETTE / GLU_POINT
	gluCylinder(q, baseRadius, topRadius, height, slices, stacks);  OGL_CHK;
	gluDeleteQuadric(q);

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
$INAME( lengthX, lengthY, lengthZ );
**/
DEFINE_FUNCTION( DrawBox ) {

	JL_S_ASSERT_ARG(3);
	float lengthX, lengthY, lengthZ;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &lengthX) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &lengthY) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &lengthZ) );

	lengthX /= 2.f;
	lengthY /= 2.f;
	lengthZ /= 2.f;

	glBegin(GL_QUADS);  OGL_CHK;

/* Cube with normals that points outside
	// right
	glNormal3f(1.0f,  1.0f,  1.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f( lengthX, lengthY, lengthZ);  OGL_CHK;
	glNormal3f(1.0f, -1.0f,  1.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f( lengthX,-lengthY, lengthZ);  OGL_CHK;
	glNormal3f(1.0f, -1.0f, -1.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f( lengthX,-lengthY,-lengthZ);  OGL_CHK;
	glNormal3f(1.0f,  1.0f, -1.0f); glTexCoord2f(1.0f, 1.0f); glVertex3f( lengthX, lengthY,-lengthZ);  OGL_CHK;
	// bottom
	glNormal3f( 1.0f,  1.0f, -1.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f( lengthX, lengthY,-lengthZ);  OGL_CHK;
	glNormal3f( 1.0f, -1.0f, -1.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f( lengthX,-lengthY,-lengthZ);  OGL_CHK;
	glNormal3f(-1.0f, -1.0f, -1.0f);	glTexCoord2f(1.0f, 0.0f); glVertex3f(-lengthX,-lengthY,-lengthZ);  OGL_CHK;
	glNormal3f(-1.0f,  1.0f, -1.0f); glTexCoord2f(1.0f, 1.0f); glVertex3f(-lengthX, lengthY,-lengthZ);  OGL_CHK;
	// left
	glNormal3f(-1.0f,  1.0f, -1.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f(-lengthX, lengthY,-lengthZ);  OGL_CHK;
	glNormal3f(-1.0f, -1.0f, -1.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(-lengthX,-lengthY,-lengthZ);  OGL_CHK;
	glNormal3f(-1.0f, -1.0f,  1.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f(-lengthX,-lengthY, lengthZ);  OGL_CHK;
	glNormal3f(-1.0f,  1.0f,  1.0f); glTexCoord2f(1.0f, 1.0f); glVertex3f(-lengthX, lengthY, lengthZ);  OGL_CHK;
	// top
	glNormal3f(-1.0f,  1.0f, 1.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f(-lengthX, lengthY, lengthZ);  OGL_CHK;
	glNormal3f(-1.0f, -1.0f, 1.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(-lengthX,-lengthY, lengthZ);  OGL_CHK;
	glNormal3f( 1.0f, -1.0f, 1.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f( lengthX,-lengthY, lengthZ);  OGL_CHK;
	glNormal3f( 1.0f,  1.0f, 1.0f); glTexCoord2f(1.0f, 1.0f); glVertex3f( lengthX, lengthY, lengthZ);  OGL_CHK;
	// back
	glNormal3f(-1.0f, 1.0f, -1.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f(-lengthX, lengthY,-lengthZ);  OGL_CHK;
	glNormal3f(-1.0f, 1.0f,  1.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(-lengthX, lengthY, lengthZ);  OGL_CHK;
	glNormal3f( 1.0f, 1.0f,  1.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f( lengthX, lengthY, lengthZ);  OGL_CHK;
	glNormal3f( 1.0f, 1.0f, -1.0f); glTexCoord2f(1.0f, 1.0f); glVertex3f( lengthX, lengthY,-lengthZ);  OGL_CHK;
	// front
	glNormal3f(-1.0f, -1.0f,  1.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f(-lengthX,-lengthY, lengthZ);  OGL_CHK;
	glNormal3f(-1.0f, -1.0f, -1.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(-lengthX,-lengthY,-lengthZ);  OGL_CHK;
	glNormal3f( 1.0f, -1.0f, -1.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f( lengthX,-lengthY,-lengthZ);  OGL_CHK;
	glNormal3f( 1.0f, -1.0f,  1.0f); glTexCoord2f(1.0f, 1.0f); glVertex3f( lengthX,-lengthY, lengthZ);  OGL_CHK;
	return;
*/

/*
	glNormal3f(1.0f, 0.0f,  0.0f); glVertex3f( lengthX, lengthY,-lengthZ);  OGL_CHK;
	glNormal3f(1.0f, 0.0f,  0.0f); glVertex3f( lengthX,-lengthY,-lengthZ);  OGL_CHK;
	glNormal3f(0.0f, 0.0f, -1.0f); glVertex3f( lengthX,-lengthY,-lengthZ);  OGL_CHK;
	glNormal3f(0.0f, 0.0f, -1.0f); glVertex3f( lengthX, lengthY,-lengthZ);  OGL_CHK;//

	glNormal3f(-1.0f, 0.0f, 0.0f); glVertex3f( -lengthX, lengthY,-lengthZ);  OGL_CHK;
	glNormal3f(-1.0f, 0.0f, 0.0f); glVertex3f( -lengthX,-lengthY,-lengthZ);  OGL_CHK;
	glNormal3f(0.0f, 0.0f, -1.0f); glVertex3f( -lengthX,-lengthY,-lengthZ);  OGL_CHK;
	glNormal3f(0.0f, 0.0f, -1.0f); glVertex3f( -lengthX, lengthY,-lengthZ);  OGL_CHK;

	glNormal3f(0.0f, 1.0f,  0.0f); glVertex3f(  lengthX, lengthY,-lengthZ);  OGL_CHK;
	glNormal3f(0.0f, 1.0f,  0.0f); glVertex3f( -lengthX, lengthY,-lengthZ);  OGL_CHK;
	glNormal3f(0.0f, 0.0f, -1.0f); glVertex3f( -lengthX, lengthY,-lengthZ);  OGL_CHK;
	glNormal3f(0.0f, 0.0f, -1.0f); glVertex3f(  lengthX, lengthY,-lengthZ);  OGL_CHK;

	glNormal3f(0.0f, -1.0f, 0.0f); glVertex3f(  lengthX, -lengthY,-lengthZ);  OGL_CHK;
	glNormal3f(0.0f, -1.0f, 0.0f); glVertex3f( -lengthX, -lengthY,-lengthZ);  OGL_CHK;
	glNormal3f(0.0f, 0.0f, -1.0f); glVertex3f( -lengthX, -lengthY,-lengthZ);  OGL_CHK;
	glNormal3f(0.0f, 0.0f, -1.0f); glVertex3f(  lengthX, -lengthY,-lengthZ);  OGL_CHK;


	glNormal3f(1.0f, 0.0f, 0.0f); glVertex3f( lengthX, lengthY, lengthZ);  OGL_CHK;
	glNormal3f(1.0f, 0.0f, 0.0f); glVertex3f( lengthX,-lengthY, lengthZ);  OGL_CHK;
	glNormal3f(0.0f, 0.0f, 1.0f); glVertex3f( lengthX,-lengthY, lengthZ);  OGL_CHK;
	glNormal3f(0.0f, 0.0f, 1.0f); glVertex3f( lengthX, lengthY, lengthZ);  OGL_CHK;

	glNormal3f(-1.0f, 0.0f, 0.0f); glVertex3f( -lengthX, lengthY, lengthZ);  OGL_CHK;
	glNormal3f(-1.0f, 0.0f, 0.0f); glVertex3f( -lengthX,-lengthY, lengthZ);  OGL_CHK;
	glNormal3f( 0.0f, 0.0f, 1.0f); glVertex3f( -lengthX,-lengthY, lengthZ);  OGL_CHK;
	glNormal3f( 0.0f, 0.0f, 1.0f); glVertex3f( -lengthX, lengthY, lengthZ);  OGL_CHK;

	glNormal3f(0.0f, 1.0f, 0.0f); glVertex3f(  lengthX, lengthY, lengthZ);  OGL_CHK;
	glNormal3f(0.0f, 1.0f, 0.0f); glVertex3f( -lengthX, lengthY, lengthZ);  OGL_CHK;
	glNormal3f(0.0f, 0.0f, 1.0f); glVertex3f( -lengthX, lengthY, lengthZ);  OGL_CHK;
	glNormal3f(0.0f, 0.0f, 1.0f); glVertex3f(  lengthX, lengthY, lengthZ);  OGL_CHK;

	glNormal3f(0.0f, -1.0f, 0.0f); glVertex3f(  lengthX, -lengthY, lengthZ);  OGL_CHK;
	glNormal3f(0.0f, -1.0f, 0.0f); glVertex3f( -lengthX, -lengthY, lengthZ);  OGL_CHK;
	glNormal3f(0.0f,  0.0f, 1.0f); glVertex3f( -lengthX, -lengthY, lengthZ);  OGL_CHK;
	glNormal3f(0.0f,  0.0f, 1.0f); glVertex3f(  lengthX, -lengthY, lengthZ);  OGL_CHK;



	glNormal3f(1.0f, 0.0f, 0.0f); glVertex3f( lengthX, lengthY, lengthZ);  OGL_CHK;
	glNormal3f(1.0f, 0.0f, 0.0f); glVertex3f( lengthX, lengthY, -lengthZ);  OGL_CHK;
	glNormal3f(0.0f, 1.0f, 0.0f); glVertex3f( lengthX, lengthY, -lengthZ);  OGL_CHK;
	glNormal3f(0.0f, 1.0f, 0.0f); glVertex3f( lengthX, lengthY, lengthZ);  OGL_CHK;

	glNormal3f(1.0f, 0.0f, 0.0f); glVertex3f( lengthX, -lengthY, lengthZ);  OGL_CHK;
	glNormal3f(1.0f, 0.0f, 0.0f); glVertex3f( lengthX, -lengthY, -lengthZ);  OGL_CHK;
	glNormal3f(0.0f, -1.0f, 0.0f); glVertex3f( lengthX, -lengthY, -lengthZ);  OGL_CHK;
	glNormal3f(0.0f, -1.0f, 0.0f); glVertex3f( lengthX, -lengthY, lengthZ);  OGL_CHK;

	glNormal3f(-1.0f, 0.0f, 0.0f); glVertex3f( -lengthX, -lengthY, lengthZ);  OGL_CHK;
	glNormal3f(-1.0f, 0.0f, 0.0f); glVertex3f( -lengthX, -lengthY, -lengthZ);  OGL_CHK;
	glNormal3f(0.0f, -1.0f, 0.0f); glVertex3f( -lengthX, -lengthY, -lengthZ);  OGL_CHK;
	glNormal3f(0.0f, -1.0f, 0.0f); glVertex3f( -lengthX, -lengthY, lengthZ);  OGL_CHK;

	glNormal3f(-1.0f, 0.0f, 0.0f); glVertex3f( -lengthX, lengthY, lengthZ);  OGL_CHK;
	glNormal3f(-1.0f, 0.0f, 0.0f); glVertex3f( -lengthX, lengthY, -lengthZ);  OGL_CHK;
	glNormal3f(0.0f, 1.0f, 0.0f); glVertex3f( -lengthX, lengthY, -lengthZ);  OGL_CHK;
	glNormal3f(0.0f, 1.0f, 0.0f); glVertex3f( -lengthX, lengthY, lengthZ);  OGL_CHK;
*/



	// right
	glNormal3f(1.0f, 0.0f, 0.0f);  OGL_CHK;
	glTexCoord2f(0.0f, 1.0f); glVertex3f( lengthX, lengthY, lengthZ);  OGL_CHK;
	glTexCoord2f(0.0f, 0.0f); glVertex3f( lengthX,-lengthY, lengthZ);  OGL_CHK;
	glTexCoord2f(1.0f, 0.0f); glVertex3f( lengthX,-lengthY,-lengthZ);  OGL_CHK;
	glTexCoord2f(1.0f, 1.0f); glVertex3f( lengthX, lengthY,-lengthZ);  OGL_CHK;

	// bottom
	glNormal3f(0.0f, 0.0f, -1.0f);  OGL_CHK;
	glTexCoord2f(0.0f, 1.0f); glVertex3f( lengthX, lengthY,-lengthZ);  OGL_CHK;
	glTexCoord2f(0.0f, 0.0f); glVertex3f( lengthX,-lengthY,-lengthZ);  OGL_CHK;
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-lengthX,-lengthY,-lengthZ);  OGL_CHK;
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-lengthX, lengthY,-lengthZ);  OGL_CHK;

	// left
	glNormal3f(-1.0f, 0.0f, 0.0f);  OGL_CHK;
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-lengthX, lengthY,-lengthZ);  OGL_CHK;
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-lengthX,-lengthY,-lengthZ);  OGL_CHK;
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-lengthX,-lengthY, lengthZ);  OGL_CHK;
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-lengthX, lengthY, lengthZ);  OGL_CHK;

	// top
	glNormal3f(0.0f, 0.0f, 1.0f);  OGL_CHK;
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-lengthX, lengthY, lengthZ);  OGL_CHK;
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-lengthX,-lengthY, lengthZ);  OGL_CHK;
	glTexCoord2f(1.0f, 0.0f); glVertex3f( lengthX,-lengthY, lengthZ);  OGL_CHK;
	glTexCoord2f(1.0f, 1.0f); glVertex3f( lengthX, lengthY, lengthZ);  OGL_CHK;

	// back
	glNormal3f(0.0f, 1.0f, 0.0f);  OGL_CHK;
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-lengthX, lengthY,-lengthZ);  OGL_CHK;
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-lengthX, lengthY, lengthZ);  OGL_CHK;
	glTexCoord2f(1.0f, 0.0f); glVertex3f( lengthX, lengthY, lengthZ);  OGL_CHK;
	glTexCoord2f(1.0f, 1.0f); glVertex3f( lengthX, lengthY,-lengthZ);  OGL_CHK;

	// right
	glNormal3f(1.0f, -1.0f, 0.0f);  OGL_CHK;
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-lengthX,-lengthY, lengthZ);  OGL_CHK;
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-lengthX,-lengthY,-lengthZ);  OGL_CHK;
	glTexCoord2f(1.0f, 0.0f); glVertex3f( lengthX,-lengthY,-lengthZ);  OGL_CHK;
	glTexCoord2f(1.0f, 1.0f); glVertex3f( lengthX,-lengthY, lengthZ);  OGL_CHK;

	glEnd();  OGL_CHK;

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME()
**/
DEFINE_FUNCTION( FullQuad ) {

	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	// Begin(QUADS); Vertex(-1,-1);  Vertex(1,-1);  Vertex(1,1);  Vertex(-1,1); End();
	glBegin(GL_TRIANGLES);
	glVertex2i(-1,-1);
	glVertex2i(3,-1);
	glVertex2i(-1,3);
	glEnd();
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
//	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz )
  $H API
   gluLookAt
**/
DEFINE_FUNCTION( LookAt ) {

	JL_S_ASSERT_ARG(9);
	JL_S_ASSERT_NUMBER(JL_ARG(1));
	JL_S_ASSERT_NUMBER(JL_ARG(2));
	JL_S_ASSERT_NUMBER(JL_ARG(3));
	JL_S_ASSERT_NUMBER(JL_ARG(4));
	JL_S_ASSERT_NUMBER(JL_ARG(5));
	JL_S_ASSERT_NUMBER(JL_ARG(6));
	JL_S_ASSERT_NUMBER(JL_ARG(7));
	JL_S_ASSERT_NUMBER(JL_ARG(8));
	JL_S_ASSERT_NUMBER(JL_ARG(9));

	double eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz;
	JL_JsvalToNative(cx, JL_ARG(1), &eyex);
	JL_JsvalToNative(cx, JL_ARG(2), &eyey);
	JL_JsvalToNative(cx, JL_ARG(3), &eyez);

	JL_JsvalToNative(cx, JL_ARG(4), &centerx);
	JL_JsvalToNative(cx, JL_ARG(5), &centery);
	JL_JsvalToNative(cx, JL_ARG(6), &centerz);

	JL_JsvalToNative(cx, JL_ARG(7), &upx);
	JL_JsvalToNative(cx, JL_ARG(8), &upy);
	JL_JsvalToNative(cx, JL_ARG(9), &upz);

	gluLookAt(eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz);  OGL_CHK;
	
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( pointX, pointY, pointZ [, upx, upy, upz] )
**/
DEFINE_FUNCTION( AimAt ) {

	JL_S_ASSERT_ARG_RANGE(3,6);

	JL_S_ASSERT_NUMBER(JL_ARG(1));
	JL_S_ASSERT_NUMBER(JL_ARG(2));
	JL_S_ASSERT_NUMBER(JL_ARG(3));

	float px, py, pz, ux, uy, uz;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &px) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &py) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &pz) );

	if ( JL_ARGC == 6 ) {

		JL_S_ASSERT_NUMBER(JL_ARG(4));
		JL_S_ASSERT_NUMBER(JL_ARG(5));
		JL_S_ASSERT_NUMBER(JL_ARG(6));

		JL_CHK( JL_JsvalToNative(cx, JL_ARG(4), &ux) );
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(5), &uy) );
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(6), &uz) );
	} else {

		ux = 0.00001f;
		uy = 0.00001f;
		uz = 1.f;
	}

	Vector3 to, up, t;
	Vector3Set(&to, px,py,pz);
	Vector3Set(&up, ux,uy,uz);
	Vector3Normalize(&to, &to);
	Vector3Cross(&t, &up, &to);
	Vector3Normalize(&t, &t);
	Vector3Cross(&up, &to, &t);

	float m[16] = {
		t.x,  t.y,  t.z,  0,
		up.x, up.y, up.z, 0,
		to.x, to.y, to.z, 0,
		0, 0, 0, 1
	};

	glMultMatrixf(m);

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME()
**/
DEFINE_FUNCTION( KeepTranslation ) {

	GLfloat m[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, m);  OGL_CHK;

//	glLoadIdentity();  OGL_CHK;
//	glTranslatef(m[12], m[13], m[14]);  OGL_CHK;
// ... compare perf with:

	memset(m, 0, 12 * sizeof(GLfloat)); // 0..11
	m[0] = 1.f;
	m[5] = 1.f;
	m[10] = 1.f;
	m[15] = 1.f;
	glLoadMatrixf(m);  OGL_CHK;

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
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
DEFINE_PROPERTY( error ) {

	// When an error occurs, the error flag is set to the appropriate error code value. No other errors are recorded
	// until glGetError is called, the error code is returned, and the flag is reset to GL_NO_ERROR.
	*vp = INT_TO_JSVAL(glGetError());
	return JS_TRUE;
}



static int MatrixGet(JSContext *cx, JSObject *obj, float **m) {

	GLint matrixMode;
	glGetIntegerv(GL_MATRIX_MODE, &matrixMode);  OGL_CHK;
	switch ( matrixMode ) {
		case GL_MODELVIEW:
			glGetFloatv(GL_MODELVIEW_MATRIX, *m);  OGL_CHK;
			return true;
		case GL_PROJECTION:
			glGetFloatv(GL_PROJECTION_MATRIX, *m);  OGL_CHK;
			return true;
		case GL_TEXTURE:
			glGetFloatv(GL_TEXTURE_MATRIX, *m);  OGL_CHK;
			return true;
		case GL_COLOR_MATRIX: // glext
			glGetFloatv(GL_COLOR_MATRIX, *m);  OGL_CHK;
			return true;
	}
bad:
	return false; // JL_REPORT_ERROR( "Unsupported matrix mode." );
}


void *windowsGLGetProcAddress(const char *procName) {
	
	return wglGetProcAddress(procName);
}

DEFINE_INIT() {

	JL_CHK( SetMatrix44GetInterface(cx, obj, MatrixGet) );

#ifdef XP_WIN
	glGetProcAddress = windowsGLGetProcAddress;
#else
	JL_CHK( GetPrivateNativeFunction(cx, JL_GetGlobalObject(cx), "_glGetProcAddress", (void**)&glGetProcAddress) );
#endif
	JL_S_ASSERT( glGetProcAddress != NULL, "OpenGL extensions unavailable." );

	return JS_TRUE;
	JL_BAD;
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

#ifdef DEBUG
DEFINE_FUNCTION( Test ) {

/*
	jsval id = JL_ARG(1);
	JL_S_ASSERT( IsHandleType(cx, id, 'TBUF'), "Invalid buffer." );
	TextureBuffer *tb = (TextureBuffer*)GetHandlePrivate(cx, id);
	tb->TextureBufferAlloc(tb, sizeof(float) * 3 * 32 * 32); // RGB 32x32
	//tb->TextureBufferFree(tb);
	for ( int i = 0; i < 3 * 32*32; i++ )
		tb->data[i] = 0.5;
*/

/*
	jsdouble d;
	jsval val = JSVAL_NULL;
	JL_CHK( JS_ValueToNumber(cx, val, &d) );
*/

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
//	JL_BAD;
}
#endif // DEBUG


CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_INIT

	BEGIN_CONST_INTEGER_SPEC
		// OpenGL constants
		#include "jsglconst.h"
	END_CONST_INTEGER_SPEC


	BEGIN_STATIC_FUNCTION_SPEC

#ifdef DEBUG
		FUNCTION_ARGC(Test, 1)
#endif // DEBUG

// OpenGL 1.1 functions

		FUNCTION_ARGC(IsEnabled, 1) // cap
		FUNCTION_ARGC(Get, 1) // pname
		FUNCTION_ARGC(GetBoolean, 2) // pname [,count]
		FUNCTION_ARGC(GetInteger, 2) // pname [,count]
		FUNCTION_ARGC(GetDouble, 2) // pname [,count]
		FUNCTION_ARGC(GetString, 1)
		FUNCTION_ARGC(DrawBuffer, 1) // mode
		FUNCTION_ARGC(ReadBuffer, 1) // mode
		FUNCTION_ARGC(Accum, 2) // op, value
		FUNCTION_ARGC(StencilFunc, 3) // func, ref, mask
		FUNCTION_ARGC(StencilOp, 3) // fail, zfail, zpass
		FUNCTION_ARGC(StencilMask, 1) // mask
		FUNCTION_ARGC(AlphaFunc, 2) // func, ref
		FUNCTION_ARGC(Flush, 0)
		FUNCTION_ARGC(Finish, 0)
		FUNCTION_ARGC(Fog, 2) // pname, param | array of params
		FUNCTION_ARGC(Hint, 2) // target, mode
		FUNCTION_ARGC(Vertex, 4) // x, y [, z [, w]]
		FUNCTION_ARGC(EdgeFlag, 1) // flag
		FUNCTION_ARGC(Color, 4) // r, g, b [, a]
		FUNCTION_ARGC(Normal, 3) // nx, ny, nz
		FUNCTION_ARGC(TexCoord, 3) // s [, t [,r ]]
		FUNCTION_ARGC(TexParameter, 3) // target, pname, param | array of params
		FUNCTION_ARGC(TexEnv, 3) // target, pname, param | array of params
		FUNCTION_ARGC(TexGen, 3) // coord, pname, params
		FUNCTION_ARGC(TexImage2D, 9) // target, level, internalFormat, width, height, border, format, type, data
		FUNCTION_ARGC(CopyTexSubImage2D, 8) // target, level, xoffset, yoffset, x, y, width, height
		FUNCTION_ARGC(LightModel, 2) // pname, param
		FUNCTION_ARGC(Light, 3) // light, pname, param
		FUNCTION_ARGC(GetLight, 3) // light, pname, count
		FUNCTION_ARGC(ColorMaterial, 2) // face, mode
		FUNCTION_ARGC(Material, 3) // face, pname, param
		FUNCTION_ARGC(Enable, 1) // cap
		FUNCTION_ARGC(Disable ,1) // cap
		FUNCTION_ARGC(PointSize, 1) // size
		FUNCTION_ARGC(LineWidth, 1) // width
		FUNCTION_ARGC(ShadeModel, 1) // mode
		FUNCTION_ARGC(BlendFunc, 2) // sfactor, dfactor
		FUNCTION_ARGC(DepthFunc, 1) // func
		FUNCTION_ARGC(DepthMask, 1) // mask
		FUNCTION_ARGC(DepthRange, 2) // zNear, zFar
		FUNCTION_ARGC(PolygonOffset, 2) // factor, units

		FUNCTION_ARGC(CullFace, 1) // mode
		FUNCTION_ARGC(FrontFace, 1) // mode
		FUNCTION_ARGC(ClearStencil, 1) // s
		FUNCTION_ARGC(ClearDepth, 1) // depth
		FUNCTION_ARGC(ClearColor, 4) // r, g, b, alpha
		FUNCTION_ARGC(ClearAccum, 4) // r, g, b, alpha
		FUNCTION_ARGC(Clear, 1) // mask
		FUNCTION_ARGC(ColorMask, 4) // r,g,b,a
		FUNCTION_ARGC(ClipPlane, 2) // plane, equation
		FUNCTION_ARGC(Viewport, 4) // x, y, width, height
		FUNCTION_ARGC(Frustum, 6) // left, right, bottom, top, zNear, zFar
		FUNCTION_ARGC(Perspective, 4) // fovY, aspectRatio, zNear, zFar (non-OpenGL API)
		FUNCTION_ARGC(Ortho, 6) // left, right, bottom, top, zNear, zFar
		FUNCTION_ARGC(MatrixMode, 1) // mode
		FUNCTION_ARGC(LoadIdentity, 0)
		FUNCTION_ARGC(PushMatrix, 0)
		FUNCTION_ARGC(PopMatrix, 0)
		FUNCTION_ARGC(LoadMatrix, 1) // matrix
		FUNCTION_ARGC(MultMatrix, 1) // matrix
		FUNCTION_ARGC(Rotate, 4) // angle, x, y, z
		FUNCTION_ARGC(Translate, 3) // x, y, z
		FUNCTION_ARGC(Scale, 3) // x, y, z
		FUNCTION_ARGC(NewList, 0)
		FUNCTION_ARGC(DeleteList, 1) // listId
		FUNCTION_ARGC(EndList, 0)
		FUNCTION_ARGC(CallList, 1) // listId | array of listId
		FUNCTION_ARGC(PolygonMode, 2) // face, mode
		FUNCTION_ARGC(Begin, 1) // mode
		FUNCTION_ARGC(End, 0)
		FUNCTION_ARGC(PushAttrib, 1) // mask
		FUNCTION_ARGC(PopAttrib, 0)
		FUNCTION_ARGC(GenTexture, 0)
		FUNCTION_ARGC(BindTexture, 2) // target, texture
		FUNCTION_ARGC(DeleteTexture, 1) // textureId
		FUNCTION_ARGC(CopyTexImage2D, 8) // target, level, internalFormat, x, y, width, height, border
		FUNCTION_ARGC(PixelTransfer, 2) // pname, param
		FUNCTION_ARGC(PixelStore, 2) // pname, param
		FUNCTION_ARGC(RasterPos, 4) // x,y,z,w
		FUNCTION_ARGC(PixelZoom, 2) // x,y
		FUNCTION_ARGC(PixelMap, 2) // map,<array>

		FUNCTION_ARGC(DefineTextureImage, 3) // target, format, image (non-OpenGL API)


// OpenGL extensions
		FUNCTION_ARGC(HasExtensionProc, 1) // procName
		FUNCTION_ARGC(HasExtensionName, 1) // name

		FUNCTION_ARGC(BlendEquation, 1) // mode
		FUNCTION_ARGC(StencilFuncSeparate, 4) // func, ref, mask
		FUNCTION_ARGC(StencilOpSeparate, 4) // fail, zfail, zpass
		FUNCTION_ARGC(ActiveStencilFaceEXT, 1) // face

		FUNCTION_ARGC(BindRenderbuffer, 2) // target, renderbuffer
		FUNCTION_ARGC(GenRenderbuffer, 0)
		FUNCTION_ARGC(DeleteRenderbuffer, 1) // renderbuffer
		FUNCTION_ARGC(RenderbufferStorage, 4) // target, internalformat, width, height
		FUNCTION_ARGC(GetRenderbufferParameter, 3) // target, pname [, count]
		FUNCTION_ARGC(BindFramebuffer, 2) // target, renderbuffer
		FUNCTION_ARGC(GenFramebuffer, 0)
		FUNCTION_ARGC(DeleteFramebuffer, 1) // framebuffer
		FUNCTION_ARGC(CheckFramebufferStatus, 1) // target
		FUNCTION_ARGC(FramebufferTexture1D, 5) // target, attachment, textarget, texture, level
		FUNCTION_ARGC(FramebufferTexture2D, 5) // target, attachment, textarget, texture, level
		FUNCTION_ARGC(FramebufferTexture3D, 6) // target, attachment, textarget, texture, level, zoffset
		FUNCTION_ARGC(FramebufferRenderbuffer, 4) // target, attachment, renderbuffertarget, renderbuffer
		FUNCTION_ARGC(GetFramebufferAttachmentParameter, 4) // target, attachment, pname [, count]

		FUNCTION_ARGC(CreateShaderObjectARB, 1)
		FUNCTION_ARGC(DeleteObjectARB, 1)
		FUNCTION_ARGC(GetInfoLogARB, 1)
		FUNCTION_ARGC(CreateProgramObjectARB, 0)
		FUNCTION_ARGC(ShaderSourceARB, 2)
		FUNCTION_ARGC(CompileShaderARB, 1)
		FUNCTION_ARGC(AttachObjectARB, 2)
		FUNCTION_ARGC(LinkProgramARB, 1)
		FUNCTION_ARGC(UseProgramObjectARB, 1)
		FUNCTION_ARGC(GetUniformLocationARB, 2)
		FUNCTION_ARGC(UniformARB, 5)
		FUNCTION_ARGC(UniformMatrixARB, 2)
		FUNCTION_ARGC(UniformFloatARB, 5)
		FUNCTION_ARGC(UniformIntegerARB, 5)
		FUNCTION_ARGC(GetObjectParameterARB, 2)
		FUNCTION_ARGC(GenBuffer, 0)
		FUNCTION_ARGC(BindBuffer, 2) // target, buffer

		FUNCTION_ARGC(PointParameter, 2) // pname, param | Array of param
		FUNCTION_ARGC(ActiveTexture, 1) // texture
		FUNCTION_ARGC(ClientActiveTexture, 1) // texture
		FUNCTION_ARGC(MultiTexCoord, 4) // target, s, t, r


// Helper functions

		FUNCTION_ARGC(UnProject, 2) // (non-OpenGL API)

		FUNCTION_ARGC(DrawImage, 3) // target, format, image (non-OpenGL API)
		FUNCTION_ARGC(ReadImage, 0) // (non-OpenGL API)

		FUNCTION_ARGC(LoadTrimesh, 1) // Trimesh object
		FUNCTION_ARGC(DrawTrimesh, 2) // TrimeshId, mode

		FUNCTION_ARGC(PixelWidthFactor, 0)

		FUNCTION_ARGC(DrawPoint, 1)
		FUNCTION_ARGC(DrawDisk, 2)
		FUNCTION_ARGC(DrawSphere, 3)
		FUNCTION_ARGC(DrawCylinder, 5)
		FUNCTION_ARGC(DrawBox, 3)
		FUNCTION_ARGC(FullQuad, 0)

		FUNCTION_ARGC(LookAt, 9) // (non-OpenGL API)
		FUNCTION_ARGC(AimAt, 6)
		FUNCTION_ARGC(KeepTranslation, 0)
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
SetVideoMode( 320, 200, 32, HWSURFACE | OPENGL | RESIZABLE );

var listeners = {
	onQuit: function() { end = true },
	onKeyDown: function(key, mod) { end = key == K_ESCAPE }
}

Ogl.MatrixMode(Ogl.PROJECTION);
Ogl.Perspective(60, 0.001, 1000);
Ogl.MatrixMode(Ogl.MODELVIEW);

for (var end = false; !end ;) {
   
	PollEvent(listeners);
   
	with (Ogl) { // beware: slower than Ogl.*
      
		Clear(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);
		LoadIdentity();
      
		LookAt(-1,-1,1, 0,0,0, 0,0,1);
      
		Begin(QUADS);
		Color(1,0,0);
		Vertex(-0.5, -0.5, 0);
		Vertex(-0.5,  0.5, 0);
		Vertex( 0.5,  0.5, 0);
		Vertex( 0.5, -0.5, 0);
		End(QUADS);
   }
   
	GlSwapBuffers();
	Sleep(10);
}

}}}
**/

