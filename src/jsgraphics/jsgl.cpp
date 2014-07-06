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

#include "jstransformation.h"

#include <gl/gl.h>
#include "glext.h" // download at http://www.opengl.org/registry/api/glext.h (http://www.opengl.org/registry/#headers)

#include "oglError.h"


#include "../jsprotex/texture.h"
#include "../jsprotex/textureBuffer.h"
#include "../jstrimesh/trimeshPub.h"
#include "../jslang/handlePub.h"
#include "../jslang/imagePub.h"

#include <gl/glu.h> // gluPerspective, gluUnProject, GLUquadric, ...
//#include "wglew.h" // needed ?

#define MAX_PARAMS 16

typedef void* (__cdecl *glGetProcAddress_t)(const char*);
static glGetProcAddress_t glGetProcAddress = NULL;


//doc.
//  OpenGL matrices are 16-value arrays with base vectors laid out contiguously in memory.
//  The translation components occupy the 13th, 14th, and 15th elements of the 16-element matrix,
//  where indices are numbered from 1 to 16 as described in section 2.11.2 of the OpenGL 2.1 Specification.


/*
NEVER_INLINE double JsvalToDouble_Slow(JSContext * RESTRICT cx, const jsval &val) {

	double doubleValue;
	if ( JS::ToNumber(cx, val, &doubleValue) )
		return doubleValue;
	return 0;
}

ALWAYS_INLINE double JsvalToDouble(JSContext * RESTRICT cx, const jsval &val) {
	
	if ( JSVAL_IS_DOUBLE(val) )
		return JSVAL_TO_DOUBLE(val);
	if ( JSVAL_IS_INT(val) )
		return val.toInt32();
	return JsvalToDouble_Slow(cx, val);
}

#define ARG_DOUBLE(NUM) \
	JsvalToDouble(cx, JL_ARG(NUM))
*/



DECLARE_CLASS(Ogl)

/*
bool GetArgInt( JSContext *cx, unsigned *argc, jsval **argv, unsigned count, int *rval ) { // (TBD) jsval** = Conservative Stack Scanning issue ?

	size_t i;
	if ( JSVAL_IS_PRIMITIVE(**argv) || !JL_IsArray(cx, **argv) ) {

		JL_ASSERT( *argc >= count, "Not enough arguments." );
		for ( i = 0; i < count; ++i ) {

			JL_CHK( jl::getValue(cx, **argv, rval) );
			++rval;
			++*argv;
		}
		*argc -= count;
		return true;
	}
	unsigned len;
	JL_CHK( jl::getVector(cx, **argv, rval, count, &len) );
	JL_ASSERT( len == count, "Not enough elements." );
	++*argv;
	--*argc;
	return true;
	JL_BAD;
}

bool GetArgDouble( JSContext *cx, unsigned *argc, jsval **argv, unsigned count, double *rval ) { // (TBD) jsval** = Conservative Stack Scanning issue ?

	size_t i;
	if ( JSVAL_IS_PRIMITIVE(**argv) || !JL_IsArray(cx, **argv) ) {

		JL_ASSERT( *argc >= count, "Not enough arguments." );
		for ( i = 0; i < count; ++i ) {

			JL_CHK( jl::getValue(cx, **argv, rval) );
			++rval;
			++*argv;
		}
		*argc -= count;
		return true;
	}
	unsigned len;
	JL_CHK( jl::getVector(cx, **argv, rval, count, &len) );
	JL_ASSERT( len == count, "Not enough elements." );
	++*argv;
	--*argc;
	return true;
	JL_BAD;
}
*/


// The specification states that any command that is not valid is completely ignored and the proper error bit is set.
// Calling glGetError in a Begin/End-pair is not valid, and so the command is ignored the GL_INVALID_OPERATION error bit is set.
// Directly after the Begin/End-pair, the error is returned, because that's the first valid call to glGetError after the error occured.
#if defined(DEBUG)

static bool _inBeginOrEnd = false;

#define OGL_ERR_CHK \
	JL_MACRO_BEGIN \
		if ( JL_IS_SAFE && !_inBeginOrEnd ) { \
			GLenum err = glGetError(); \
			if ( err != GL_NO_ERROR ) \
				JL_WARN( E_LIB, E_STR("OpenGL"), E_OPERATION, E_DETAILS, E_STR(OpenGLErrorToConst(err)), E_STR("("), E_NUM(err), E_STR(")") ); \
		} \
	JL_MACRO_END

#define OGL_CX_CHK \
	JL_MACRO_BEGIN \
		if ( JL_IS_SAFE ) { \
			if ( !_inBeginOrEnd && glGetError() == GL_INVALID_OPERATION ) \
				JL_ERR( E_LIB, E_STR("OpenGL"), E_NOTINIT ); \
		} \
	JL_MACRO_END

#else // DBUG

#define OGL_ERR_CHK
#define OGL_CX_CHK

#endif // DBUG


#define DECLARE_OPENGL_EXTENSION( NAME, PROTOTYPE ) \
	static PROTOTYPE NAME = NULL;

#define INIT_OPENGL_EXTENSION( NAME, PROTOTYPE ) \
	JL_MACRO_BEGIN \
		if ( (NAME) == NULL ) \
			NAME = (PROTOTYPE)glGetProcAddress( #NAME ); \
	JL_MACRO_END

#define JL_INIT_OPENGL_EXTENSION( NAME, PROTOTYPE ) \
	JL_MACRO_BEGIN \
		INIT_OPENGL_EXTENSION( NAME, PROTOTYPE ); \
		if ( (NAME) == NULL ) \
			return ThrowOglError(cx, GL_INVALID_OPERATION); \
	JL_MACRO_END

//JL_ASSERT( NAME != NULL, "OpenGL extension %s is unavailable.", #NAME ); \


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

DECLARE_OPENGL_EXTENSION( glGetProgramiv, PFNGLGETPROGRAMIVPROC );

DECLARE_OPENGL_EXTENSION( glGetActiveUniformARB, PFNGLGETACTIVEUNIFORMARBPROC );

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


// GL_ARB_uniform_buffer_object
DECLARE_OPENGL_EXTENSION( glGetUniformBlockIndex, PFNGLGETUNIFORMBLOCKINDEXPROC );
DECLARE_OPENGL_EXTENSION( glUniformBlockBinding, PFNGLUNIFORMBLOCKBINDINGPROC );
DECLARE_OPENGL_EXTENSION( glGetActiveUniformsiv, PFNGLGETACTIVEUNIFORMSIVPROC );
DECLARE_OPENGL_EXTENSION( glGetUniformIndices, PFNGLGETUNIFORMINDICESPROC );




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

DECLARE_OPENGL_EXTENSION( glGenQueriesARB, PFNGLGENQUERIESARBPROC );
DECLARE_OPENGL_EXTENSION( glDeleteQueriesARB, PFNGLDELETEQUERIESARBPROC );
DECLARE_OPENGL_EXTENSION( glIsQueryARB, PFNGLISQUERYARBPROC );
DECLARE_OPENGL_EXTENSION( glBeginQueryARB, PFNGLBEGINQUERYARBPROC );
DECLARE_OPENGL_EXTENSION( glEndQueryARB, PFNGLENDQUERYARBPROC );
DECLARE_OPENGL_EXTENSION( glGetQueryivARB, PFNGLGETQUERYIVARBPROC );
DECLARE_OPENGL_EXTENSION( glGetQueryObjectivARB, PFNGLGETQUERYOBJECTIVARBPROC );
DECLARE_OPENGL_EXTENSION( glGetQueryObjectuivARB, PFNGLGETQUERYOBJECTUIVARBPROC );




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
DEFINE_FUNCTION( isEnabled ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	JL_RVAL.setBoolean( glIsEnabled(JL_ARG(1).toInt32()) );  OGL_ERR_CHK;
	return true;
	JL_BAD;
}




DEFINE_FUNCTION( get ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	int pname = JL_ARG(1).toInt32();

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
			glGetBooleanv(pname, params);  OGL_ERR_CHK;
			JL_RVAL.setBoolean( *params );
			return true;
		}

		case GL_COLOR_WRITEMASK:
		{
			GLboolean params[4];
			glGetBooleanv(pname, params);  OGL_ERR_CHK;
			/*
			jsval ret[] = {
					BOOLEAN_TO_JSVAL( params[0] ),
					BOOLEAN_TO_JSVAL( params[1] ),
					BOOLEAN_TO_JSVAL( params[2] ),
					BOOLEAN_TO_JSVAL( params[3] )
			};
			JL_RVAL.setObject(JS_NewArrayObject(cx, COUNTOF(ret), ret));
			*/
			JS::RootedObject arrayObj(cx, jl::newArray(cx, bool(params[0]), bool(params[1]), bool(params[2]), bool(params[3])));
			JL_CHK(arrayObj);
			JL_RVAL.setObject(*arrayObj);
			return true;
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
			glGetIntegerv(pname, params);  OGL_ERR_CHK;
			JL_RVAL.setInt32(*params);
			return true;
		}

		case GL_MAP2_GRID_SEGMENTS:
		case GL_MAX_VIEWPORT_DIMS:
		case GL_POLYGON_MODE: // enum
		{
			GLint params[2];
			glGetIntegerv(pname, params);  OGL_ERR_CHK;
			/*
			jsval jsparams[] = {
				INT_TO_JSVAL(params[0]),
				INT_TO_JSVAL(params[1])
			};
			JL_RVAL.setObject( JS_NewArrayObject(cx, COUNTOF(jsparams), jsparams) );
			JL_CHK( JL_RVAL );
			*/
			JS::RootedObject arrayObj(cx, jl::newArray(cx, params[0], params[1]));
			JL_CHK(arrayObj);
			JL_RVAL.setObject(*arrayObj);
			return true;
		}

		case GL_SCISSOR_BOX:
		case GL_VIEWPORT:
		{
			GLint params[4];
			glGetIntegerv(pname, params);  OGL_ERR_CHK;
			/*
			jsval jsparams[] = {
				INT_TO_JSVAL(params[0]),
				INT_TO_JSVAL(params[1]),
				INT_TO_JSVAL(params[2]),
				INT_TO_JSVAL(params[3])
			};
			JL_RVAL.setObject(JS_NewArrayObject(cx, COUNTOF(jsparams), jsparams));
			JL_CHK( JL_RVAL );
			*/

			JS::RootedObject arrayObj(cx, jl::newArray(cx, params[0], params[1], params[2], params[3]));
			JL_CHK(arrayObj);
			JL_RVAL.setObject(*arrayObj);

			return true;
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
			glGetDoublev(pname, params);  OGL_ERR_CHK;
			return jl::setValue(cx, JL_RVAL, *params);
		}

		case GL_ALIASED_POINT_SIZE_RANGE:
		case GL_ALIASED_LINE_WIDTH_RANGE:
		case GL_LINE_WIDTH_RANGE: // GL_SMOOTH_LINE_WIDTH_RANGE
		case GL_MAP1_GRID_DOMAIN:
		case GL_POINT_SIZE_RANGE: // GL_SMOOTH_POINT_SIZE_RANGE
		{
			GLdouble params[2];
			glGetDoublev(pname, params);  OGL_ERR_CHK;
			return jl::setVector(cx, JL_RVAL, params, COUNTOF(params));

		}

		case GL_CURRENT_NORMAL:
		case GL_POINT_DISTANCE_ATTENUATION:
		{
			GLdouble params[3];
			glGetDoublev(pname, params);  OGL_ERR_CHK;
			return jl::setVector(cx, JL_RVAL, params, COUNTOF(params));
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
			glGetDoublev(pname, params);  OGL_ERR_CHK;
			return jl::setVector(cx, JL_RVAL, params, COUNTOF(params));
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
			glGetDoublev(pname, params);  OGL_ERR_CHK;
			return jl::setVector(cx, JL_RVAL, params, COUNTOF(params));
		}

		case GL_COMPRESSED_TEXTURE_FORMATS: // enum
		{
			GLint count;
			glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &count);
			GLint *params = (GLint*)alloca(count * sizeof(GLenum));
			glGetIntegerv(pname, params);
			return jl::setVector(cx, JL_RVAL, params, count);
		}
	}

	ThrowOglError(cx, GL_INVALID_ENUM);
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
DEFINE_FUNCTION( getBoolean ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	GLboolean params;
	glGetBooleanv(JL_ARG(1).toInt32(), &params);  OGL_ERR_CHK;
	JL_RVAL.setBoolean(bool(params));
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT | $ARRAY $INAME( pname [, count | array] )
  $H arguments
   $ARG GLenum pname
   $ARG $INT count: is the number of expected values. If _count_ is defined, the function will returns an array of values, else it returns a single value.
  $H return value
   A value or an array of values of a selected parameter.
   If given, the _array_ argument is filled and then returned by the function.
   Important: _array_ length must match the number of arguments expected for the given _pname_.
  $H OpenGL API
   glGetIntegerv
**/
DEFINE_FUNCTION( getInteger ) {
	
	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	GLint params[MAX_PARAMS]; // (TBD) check if it is the max amount of data that glGetIntegerv may returns.

	if ( JL_IS_SAFE ) {

		memset(params, 0xAA, sizeof(params));
	}

	glGetIntegerv(JL_ARG(1).toInt32(), params);  OGL_ERR_CHK;

	if ( JL_ARG_ISDEF(2) ) {

		uint32_t count;
		JS::RootedObject arrayObj(cx);

		if ( JL_ARG(2).isInt32() ) {

			count = JL_ARG(2).toInt32();
			arrayObj = JS_NewArrayObject(cx, count);
			JL_CHK( arrayObj );
		} else {

			JL_ASSERT_ARG_IS_ARRAY(2);
			arrayObj = &JL_ARG(2).toObject();
			JL_CHK( JS_GetArrayLength(cx, arrayObj, &count) );
		}

		if ( JL_IS_SAFE ) {

			uint32_t argset = sizeof(params);
			while ( --argset >= 0 && ((unsigned char*)params)[argset] == 0xAA );
			argset = argset / sizeof(*params) + 1;
			JL_ASSERT_WARN( argset == count, E_ARG, E_NUM(2), E_EQUALS, E_NUM(argset) );
		}

		JL_RVAL.setObject(*arrayObj);
		count = jl::min(count, COUNTOF(params));
		while (count--) {

			jl::setElement(cx, arrayObj, count, params[count]);
		}

	} else {

		JL_RVAL.setInt32( params[0] );
	}

	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $REAL | $ARRAY $INAME( pname [, count | array] )
  $H arguments
   $ARG GLenum pname
   $ARG $INT count: is the number of expected values. If _count_ is defined, the function will returns an array of values, else a single value.
  $H return value
   A single value or an Array of values of the selected parameter.
   If given, the _array_ argument is filled and then returned by the function.
   Important: _array_ length must match the number of arguments expected for the given _pname_.
  $H OpenGL API
   glGetDoublev
**/
DEFINE_FUNCTION( getDouble ) {
	
	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	GLdouble params[MAX_PARAMS]; // (TBD) check if it is the max amount of data that glGetDoublev may returns.

	if ( JL_IS_SAFE ) {

		memset(params, 0xAA, sizeof(params));
	}

	glGetDoublev(JL_ARG(1).toInt32(), params);  OGL_ERR_CHK;

	if ( JL_ARG_ISDEF(2) ) {

		uint32_t count;
		JS::RootedObject arrayObj(cx);

		if ( JL_ARG(2).isInt32() ) {

			count = JL_ARG(2).toInt32();
			arrayObj = JS_NewArrayObject(cx, count);
			JL_CHK( arrayObj );
		} else {

			JL_ASSERT_ARG_IS_ARRAY(2);
			arrayObj = &JL_ARG(2).toObject();
			JL_CHK( JS_GetArrayLength(cx, arrayObj, &count) );
		}

		if ( JL_IS_SAFE ) {

			uint32_t argset = sizeof(params);
			while ( --argset >= 0 && ((unsigned char*)params)[argset] == 0xAA );
			argset = argset / sizeof(*params) + 1;
			JL_ASSERT_WARN( argset == count, E_ARGVALUE, E_NUM(2), E_EQUALS, E_NUM(argset) );
		}

		JL_RVAL.setObject(*arrayObj);
		count = jl::min(count, COUNTOF(params));
		while (count--) {

			JL_CHK(jl::setElement(cx, arrayObj, count, params[count]));
		}
	} else {

		JL_CHK(jl::setValue(cx, JL_RVAL, params[0]));
	}

	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
  $STR$INAME( name )
  $H arguments
   $ARG GLenum name Specifies a symbolic constant, one of VENDOR, RENDERER, VERSION, SHADING_LANGUAGE_VERSION, or EXTENSIONS.
  $H return value
   A string describing the current GL connection.
  $H OpenGL API
   glGetString
**/
DEFINE_FUNCTION( getString ) {
	
	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	return jl::setValue(cx, JL_RVAL, (char*)glGetString(JL_ARG(1).toInt32()));  OGL_ERR_CHK;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( mode )
  $H OpenGL API
   glDrawBuffer
**/
DEFINE_FUNCTION( drawBuffer ) {
	
	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	glDrawBuffer(JL_ARG(1).toInt32());  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( mode )
  $H OpenGL API
   glReadBuffer
**/
DEFINE_FUNCTION( readBuffer ) {
	
	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	glReadBuffer(JL_ARG(1).toInt32());  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( accum ) {
	
	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(2);
	JL_ASSERT_ARG_IS_INTEGER(1);
	JL_ASSERT_ARG_IS_NUMBER(2);

	float value;
	jl::getValue(cx, JL_ARG(2), &value);

	glAccum(JL_ARG(1).toInt32(), value);  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( stencilFunc ) {
	
	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(3);
	JL_ASSERT_ARG_IS_INTEGER(1);
	JL_ASSERT_ARG_IS_INTEGER(2);
	JL_ASSERT_ARG_IS_NUMBER(3);

	GLuint mask;
	if ( INT_TO_JSVAL(-1) == JL_ARG(3) )
		mask = 0xffffffff;
	else
		jl::getValue(cx, JL_ARG(3), &mask);

	glStencilFunc(JL_ARG(1).toInt32(), JL_ARG(2).toInt32(), mask);  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( stencilOp ) {
	
	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(3);
	JL_ASSERT_ARG_IS_INTEGER(1);
	JL_ASSERT_ARG_IS_INTEGER(2);
	JL_ASSERT_ARG_IS_INTEGER(3);

	glStencilOp(JL_ARG(1).toInt32(), JL_ARG(2).toInt32(), JL_ARG(3).toInt32());  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( stencilMask ) {
		
	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_NUMBER(1);

	GLuint mask;
	if ( INT_TO_JSVAL(-1) == JL_ARG(1) )
		mask = 0xffffffff;
	else
		jl::getValue(cx, JL_ARG(1), &mask);

	glStencilMask( mask );  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( alphaFunc ) {
		
	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(2);
	JL_ASSERT_ARG_IS_INTEGER(1);
	JL_ASSERT_ARG_IS_NUMBER(2);

	float ref;
	jl::getValue(cx, JL_ARG(2), &ref);

	glAlphaFunc( JL_ARG(1).toInt32(), ref );  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  $H OpenGL API
   glFlush
**/
DEFINE_FUNCTION( flush ) {
		
	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_IGNORE(argc, cx);

	glFlush();  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  $H OpenGL API
   glFinish
**/
DEFINE_FUNCTION( finish ) {
		
	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_IGNORE(argc, cx);

	glFinish();  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( fog ) {
		
	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(2);
	JL_ASSERT_ARG_IS_INTEGER(1);

	JL_RVAL.setUndefined();
	if ( JL_ARG(2).isInt32() ) {

		glFogi(JL_ARG(1).toInt32(), JL_ARG(2).toInt32());  OGL_ERR_CHK;

		return true;
	}
	if ( JL_ARG(2).isDouble() ) {

		float param;
		jl::getValue(cx, JL_ARG(2), &param);

		glFogf( JL_ARG(1).toInt32(), param );  OGL_ERR_CHK;

		return true;
	}
	if ( jl::isArrayLike(cx, JL_ARG(2)) ) {

		GLfloat params[MAX_PARAMS];
		uint32_t length;
		JL_CHK( jl::getVector(cx, JL_ARG(2), params, COUNTOF(params), &length ) );

		glFogfv( JL_ARG(1).toInt32(), params );  OGL_ERR_CHK;

		return true;
	}

	JL_ERR( E_ARG, E_NUM(2), E_TYPE, E_TY_NUMBER, E_OR, E_TY_ARRAY );
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
DEFINE_FUNCTION( hint ) {
		
	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(2);
	JL_ASSERT_ARG_IS_INTEGER(1);
	JL_ASSERT_ARG_IS_INTEGER(2);

	glHint( JL_ARG(1).toInt32(), JL_ARG(2).toInt32() );  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( vertex ) {
		
	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_RVAL.setUndefined();

	if ( argc > 1 && JL_ARG(1).isNumber() ) {

		JL_ASSERT_ARGC_RANGE(2,4);

		double x, y, z, w;
		jl::getValue(cx, JL_ARG(1), &x);
		JL_CHK( jl::getValue(cx, JL_ARG(2), &y) );
		if ( JL_ARGC >= 3 ) {

			JL_CHK( jl::getValue(cx, JL_ARG(3), &z) );
			if ( JL_ARGC >= 4 ) {

				JL_CHK( jl::getValue(cx, JL_ARG(4), &w) );
				glVertex4d(x, y, z, w);  OGL_ERR_CHK;
				return true;
			}
			glVertex3d(x, y, z);  OGL_ERR_CHK;
			return true;
		}
		glVertex2d(x, y);  OGL_ERR_CHK;
		return true;
	}

	JL_ASSERT_ARGC(1);

	GLdouble pos[4];
	uint32_t len;
	JL_CHK( jl::getVector(cx, JL_ARG(1), pos, COUNTOF(pos), &len) );
	if ( len == 2 ) {
		glVertex2dv(pos);  OGL_ERR_CHK;
	} else if ( len == 3 ) {
		glVertex3dv(pos);  OGL_ERR_CHK;
	} else if ( len == 4 ) {
		glVertex4dv(pos);  OGL_ERR_CHK;
	} else {
		JL_ERR( E_ARG, E_NUM(1), E_LENGTH, E_INTERVAL_NUM(2, 4) );
	}

	return true;
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
DEFINE_FUNCTION( edgeFlag ) {
		
	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(1);

	bool flag;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &flag) );

	glEdgeFlag(flag);

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( color ) {
		
	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC_MIN(1);
	JL_RVAL.setUndefined();

	if ( argc > 1 || JL_ARG(1).isNumber() ) {

		JL_ASSERT_ARGC_MAX(4);

		double r, g, b, a;
		jl::getValue(cx, JL_ARG(1), &r);
		if ( argc == 1 ) {

			glColor3d(r, r, r);  OGL_ERR_CHK;
			;
			return true;
		}
		jl::getValue(cx, JL_ARG(2), &g);
		jl::getValue(cx, JL_ARG(3), &b);
		if ( argc == 3 ) {

			glColor3d(r, g, b);  OGL_ERR_CHK;
			;
			return true;
		}
		jl::getValue(cx, JL_ARG(4), &a);
		glColor4d(r, g, b, a);  OGL_ERR_CHK;
		;
	} else {

		JL_ASSERT_ARGC(1);

		GLdouble color[4];
		uint32_t len;
		jl::getVector(cx, JL_ARG(1), color, 4, &len);
		if ( len == 3 ) {
			glColor3dv(color);  OGL_ERR_CHK;
		} else if ( len == 4 ) {
			glColor4dv(color);  OGL_ERR_CHK;
		} else {
			JL_ERR( E_ARG, E_NUM(1), E_LENGTH, E_INTERVAL_NUM(3, 4) );
		}
	}
	return true;
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
DEFINE_FUNCTION( normal ) {
		
	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(3);
	JL_ASSERT_ARG_IS_NUMBER(1);
	JL_ASSERT_ARG_IS_NUMBER(2);
	JL_ASSERT_ARG_IS_NUMBER(3);

	double nx, ny, nz;
	jl::getValue(cx, JL_ARG(1), &nx);
	jl::getValue(cx, JL_ARG(2), &ny);
	jl::getValue(cx, JL_ARG(3), &nz);

	glNormal3d(nx, ny, nz);  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	;
	return true;
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
DEFINE_FUNCTION( texCoord ) {
		
	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC_RANGE(1,3);

	JL_RVAL.setUndefined();
	double s;
	jl::getValue(cx, JL_ARG(1), &s);
	if ( JL_ARGC == 1 ) {

		glTexCoord1d(s);  OGL_ERR_CHK;

		;
		return true;
	}
	double t;
	jl::getValue(cx, JL_ARG(2), &t);
	if ( JL_ARGC == 2 ) {

		glTexCoord2d(s, t);  OGL_ERR_CHK;

		;
		return true;
	}
	double r;
	jl::getValue(cx, JL_ARG(3), &r);

	glTexCoord3d(s, t, r);  OGL_ERR_CHK;

	;
	return true;
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
DEFINE_FUNCTION( texParameter ) {
		
	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(3);
	JL_ASSERT_ARG_IS_INTEGER(1);
	JL_ASSERT_ARG_IS_INTEGER(2);
	JL_ASSERT_ARG_IS_NUMBER(3);

	JL_RVAL.setUndefined();
	if (JL_ARG(3).isInt32() ) {

		glTexParameteri( JL_ARG(1).toInt32(), JL_ARG(2).toInt32(), JL_ARG(3).toInt32() );  OGL_ERR_CHK;

		return true;
	}
	if ( JL_ARG(3).isDouble() ) {

		float param;
		jl::getValue(cx, JL_ARG(3), &param);

		glTexParameterf( JL_ARG(1).toInt32(), JL_ARG(2).toInt32(), param );  OGL_ERR_CHK;

		return true;
	}
	if ( jl::isArrayLike(cx, JL_ARG(3)) ) {

		GLfloat params[MAX_PARAMS];
		uint32_t length;
		JL_CHK( jl::getVector(cx, JL_ARG(3), params, COUNTOF(params), &length ) );

		glTexParameterfv( JL_ARG(1).toInt32(), JL_ARG(2).toInt32(), params );  OGL_ERR_CHK;

		return true;
	}

	JL_ERR( E_ARG, E_NUM(3), E_TYPE, E_TY_NUMBER, E_OR, E_TY_ARRAY );

	return true;
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
DEFINE_FUNCTION( texEnv ) {
		
	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC_MIN(3);
	JL_ASSERT_ARG_IS_INTEGER(1);
	JL_ASSERT_ARG_IS_INTEGER(2);

	JL_RVAL.setUndefined();
	if (argc == 3 && JL_ARG(3).isInt32() ) {

		glTexEnvi( JL_ARG(1).toInt32(), JL_ARG(2).toInt32(), JL_ARG(3).toInt32() );  OGL_ERR_CHK;

		return true;
	}
	if ( argc == 3 && JL_ARG(3).isDouble() ) {

		float param;
		jl::getValue(cx, JL_ARG(3), &param);

		glTexEnvf( JL_ARG(1).toInt32(), JL_ARG(2).toInt32(), param );  OGL_ERR_CHK;

		return true;
	}

	GLfloat params[MAX_PARAMS];
	if ( argc == 3 && jl::isArrayLike(cx, JL_ARG(3)) ) {

		uint32_t length;
		JL_CHK( jl::getVector(cx, JL_ARG(3), params, COUNTOF(params), &length ) );

		glTexEnvfv( JL_ARG(1).toInt32(), JL_ARG(2).toInt32(), params );  OGL_ERR_CHK;

		return true;
	}

	// process functions arguments [2..n]

	JL_ASSERT_ARGC_MIN( 3 ); // at least
	ASSERT( argc-2 < COUNTOF(params) );
	for ( unsigned int i = 2; i < argc; ++i )
		jl::getValue(cx, JL_ARG(i+1), &params[i-2]);

	glTexEnvfv( JL_ARG(1).toInt32(), JL_ARG(2).toInt32(), params );  OGL_ERR_CHK;

	return true;
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
DEFINE_FUNCTION( texGen ) {
		
	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC_MIN(3);
	JL_ASSERT_ARG_IS_INTEGER(1);
	JL_ASSERT_ARG_IS_INTEGER(2);

	JL_RVAL.setUndefined();
	if (argc == 3 && JL_ARG(3).isInt32()) {

		glTexGeni( JL_ARG(1).toInt32(), JL_ARG(2).toInt32(), JL_ARG(3).toInt32() );  OGL_ERR_CHK;

		return true;
	}
	if ( argc == 3 && JL_ARG(3).isDouble() ) {

		double param;
		jl::getValue(cx, JL_ARG(3), &param);

		glTexGend( JL_ARG(1).toInt32(), JL_ARG(2).toInt32(), param );  OGL_ERR_CHK;

		return true;
	}

	GLdouble params[MAX_PARAMS];
	if ( argc == 3 && jl::isArrayLike(cx, JL_ARG(3)) ) {

		uint32_t length;
		JL_CHK( jl::getVector(cx, JL_ARG(3), params, COUNTOF(params), &length ) );

		glTexGendv( JL_ARG(1).toInt32(), JL_ARG(2).toInt32(), params );  OGL_ERR_CHK;

		return true;
	}

	JL_ASSERT_ARGC_MIN( 3 ); // at least
	ASSERT( argc-2 < COUNTOF(params) );
	for ( unsigned int i = 2; i < argc; ++i )
		jl::getValue(cx, JL_ARG(i+1), &params[i-2]);

	glTexGendv( JL_ARG(1).toInt32(), JL_ARG(2).toInt32(), params );  OGL_ERR_CHK;

	return true;
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
DEFINE_FUNCTION( texImage2D ) {
		
	JL_DEFINE_ARGS;

	jl::BufString data;

	OGL_CX_CHK;

	JL_ASSERT_ARGC_RANGE(8, 9);
	JL_ASSERT_ARG_IS_INTEGER(1);
	JL_ASSERT_ARG_IS_INTEGER(2);
	JL_ASSERT_ARG_IS_INTEGER(3);
	JL_ASSERT_ARG_IS_INTEGER(4);
	JL_ASSERT_ARG_IS_INTEGER(5);
	JL_ASSERT_ARG_IS_INTEGER(6);
	JL_ASSERT_ARG_IS_INTEGER(7);
	JL_ASSERT_ARG_IS_INTEGER(8);

	if ( JL_ARG_ISDEF(9) && !JL_ARG(9).isNull() ) // same as !JSVAL_IS_PRIMITIVE
		JL_CHK( jl::getValue(cx, JL_ARG(9), &data) );

	glTexImage2D( JL_ARG(1).toInt32(), JL_ARG(2).toInt32(), JL_ARG(3).toInt32(), JL_ARG(4).toInt32(), JL_ARG(5).toInt32(), JL_ARG(6).toInt32(), JL_ARG(7).toInt32(), JL_ARG(8).toInt32(), (GLvoid*)data.toData<const uint8_t*>() );  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( copyTexSubImage2D ) {
		
	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(8);
	JL_ASSERT_ARG_IS_INTEGER(1);
	JL_ASSERT_ARG_IS_INTEGER(2);
	JL_ASSERT_ARG_IS_INTEGER(3);
	JL_ASSERT_ARG_IS_INTEGER(4);
	JL_ASSERT_ARG_IS_INTEGER(5);
	JL_ASSERT_ARG_IS_INTEGER(6);
	JL_ASSERT_ARG_IS_INTEGER(7);
	JL_ASSERT_ARG_IS_INTEGER(8);

	glCopyTexSubImage2D( JL_ARG(1).toInt32(), JL_ARG(2).toInt32(), JL_ARG(3).toInt32(), JL_ARG(4).toInt32(), JL_ARG(5).toInt32(), JL_ARG(6).toInt32(), JL_ARG(7).toInt32(), JL_ARG(8).toInt32() );  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, data )
  $H arguments
   $ARG GLenum target
   $ARG GLint level
   $ARG GLint xoffset
   $ARG GLint yoffset
   $ARG GLsizei width
   $ARG GLsizei height
   $ARG GLenum format
   $ARG GLenum type
   $ARG Blob data
  $H OpenGL API
   glTexSubImage2D
  $H OpenGL documentation
**/
DEFINE_FUNCTION( texSubImage2D ) {
		
	JL_DEFINE_ARGS;

	jl::BufString data;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(9);
	JL_ASSERT_ARG_IS_INTEGER(1);
	JL_ASSERT_ARG_IS_INTEGER(2);
	JL_ASSERT_ARG_IS_INTEGER(3);
	JL_ASSERT_ARG_IS_INTEGER(4);
	JL_ASSERT_ARG_IS_INTEGER(5);
	JL_ASSERT_ARG_IS_INTEGER(6);
	JL_ASSERT_ARG_IS_INTEGER(7);
	JL_ASSERT_ARG_IS_INTEGER(8);

	GLvoid *pixels;

	if (JL_ARG(9).isInt32()) {
			
		pixels = (GLvoid*)JL_ARG(9).toInt32();
	} else {

		JL_CHK(jl::getValue(cx, JL_ARG(9), &data));
		pixels = (GLvoid*)data.toData<const uint8_t*>();
	}

	glTexSubImage2D(JL_ARG(1).toInt32(), JL_ARG(2).toInt32(), JL_ARG(3).toInt32(), JL_ARG(4).toInt32(), JL_ARG(5).toInt32(), JL_ARG(6).toInt32(), JL_ARG(7).toInt32(), JL_ARG(8).toInt32(), pixels);

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( lightModel ) {
		
	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(2);
	JL_ASSERT_ARG_IS_INTEGER(1);

	JL_RVAL.setUndefined();
	if (JL_ARG(2).isInt32() ) {

		glLightModeli( JL_ARG(1).toInt32(), JL_ARG(2).toInt32() );  OGL_ERR_CHK;
		return true;
	}

	if ( JL_ARG(2).isDouble() ) {

		float param;
		jl::getValue(cx, JL_ARG(2), &param);
		glLightModelf( JL_ARG(1).toInt32(), param );  OGL_ERR_CHK;
		return true;
	}

	if ( jl::isArrayLike(cx, JL_ARG(2)) ) {

		GLfloat params[MAX_PARAMS];
		uint32_t length;
		JL_CHK( jl::getVector(cx, JL_ARG(2), params, COUNTOF(params), &length ) );
		glLightModelfv( JL_ARG(1).toInt32(), params );  OGL_ERR_CHK;
		return true;
	}

	JL_ERR( E_ARG, E_NUM(2), E_TYPE, E_TY_NUMBER, E_OR, E_TY_ARRAY );
	return true;
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
DEFINE_FUNCTION( light ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC_MIN(3);
	JL_ASSERT_ARG_IS_INTEGER(1);
	JL_ASSERT_ARG_IS_INTEGER(2);

	JL_RVAL.setUndefined();
	if (argc == 3 && JL_ARG(3).isInt32()) {

		glLighti( JL_ARG(1).toInt32(), JL_ARG(2).toInt32(), JL_ARG(3).toInt32() );  OGL_ERR_CHK;
		return true;
	}

	if ( argc == 3 && JL_ARG(3).isDouble() ) {

		float param;
		jl::getValue(cx, JL_ARG(3), &param);
		glLightf( JL_ARG(1).toInt32(), JL_ARG(2).toInt32(), param );  OGL_ERR_CHK;
		return true;
	}

	GLfloat params[MAX_PARAMS];
	if (argc == 3 && jl::isArrayLike(cx, JL_ARG(3))) {

		uint32_t length;
		JL_CHK( jl::getVector(cx, JL_ARG(3), params, COUNTOF(params), &length ) );
		glLightfv( JL_ARG(1).toInt32(), JL_ARG(2).toInt32(), params );  OGL_ERR_CHK;
		return true;
	}

	JL_ASSERT_ARGC_MIN( 3 ); // at least
	ASSERT( argc-2 < COUNTOF(params) );
	for ( unsigned int i = 2; i < argc; ++i )
		jl::getValue(cx, JL_ARG(i+1), &params[i-2]);
	glLightfv( JL_ARG(1).toInt32(), JL_ARG(2).toInt32(), params );  OGL_ERR_CHK;
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $REAL | $ARRAY $INAME( light, pname [, count] )
  Return light source parameter values.
  $H arguments
   $ARG GLenum light: Specifies a light source.
   $ARG GLenum pname: Specifies a light source parameter for light.
   $ARG $INT count: is the number of expected values. If _count_ is omitted, the function will automatically determine the right value for _count_.
  $H return value
   A single value or an Array of values of the selected parameter.
  $H OpenGL API
   glGetLightfv
**/
DEFINE_FUNCTION( getLight ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC_RANGE(2,3);
	JL_ASSERT_ARG_IS_INTEGER(1);
	JL_ASSERT_ARG_IS_INTEGER(2);

	GLfloat params[MAX_PARAMS];
	GLenum pname;
	pname = JL_ARG(2).toInt32();

	glGetLightfv(JL_ARG(1).toInt32(), pname, params);  OGL_ERR_CHK;

	int count;
	if ( JL_ARG_ISDEF(3) ) {

		JL_ASSERT_ARG_IS_INTEGER(3);
		count = jl::min(JL_ARG(3).toInt32(), (int)COUNTOF(params));
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
				JL_ERR( E_ARG, E_NUM(1), E_NOTSUPPORTED ); // "Unknown parameter count"
		}
	}

	if ( count > 1 ) {
		/*
		JS::RootedObject arrayObj(cx, JS_NewArrayObject(cx, count));
		JL_CHK( arrayObj );
		JL_RVAL.setObject(*arrayObj);

		JS::RootedValue tmpValue(cx);
		while ( count-- ) {

			JL_CHK(jl::setValue(cx, tmpValue, params[count]));
			JL_CHK( JL_SetElement(cx, arrayObj, count, tmpValue) );
		}
		*/
		JL_CHK(jl::setVector(cx, JL_RVAL, params, count));
	} else {

		JL_CHK(jl::setValue(cx, JL_RVAL, params[0]));
	}

	return true;
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
DEFINE_FUNCTION( colorMaterial ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(2);
	JL_ASSERT_ARG_IS_INTEGER(1);
	JL_ASSERT_ARG_IS_INTEGER(2);

	glColorMaterial(JL_ARG(1).toInt32(), JL_ARG(2).toInt32());  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( material ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC_MIN( 3 );
	JL_ASSERT_ARG_IS_INTEGER(1);
	JL_ASSERT_ARG_IS_INTEGER(2);

	// GL_AMBIENT: 4 int
	// GL_DIFFUSE: 4 int
	// GL_SPECULAR: 4 int
	// GL_EMISSION: 4 int
	// GL_SHININESS: 1 int
	// GL_AMBIENT_AND_DIFFUSE: ?
	// GL_COLOR_INDEXES: 3 int -OR- 3 float

	JL_RVAL.setUndefined();
	if (argc == 3 && JL_ARG(3).isInt32()) {

		glMateriali( JL_ARG(1).toInt32(), JL_ARG(2).toInt32(), JL_ARG(3).toInt32() );  OGL_ERR_CHK;
		;
		return true;
	}
	if ( argc == 3 && JL_ARG(3).isDouble() ) {

		float param;
		jl::getValue(cx, JL_ARG(3), &param);
		glMaterialf( JL_ARG(1).toInt32(), JL_ARG(2).toInt32(), param );  OGL_ERR_CHK;
		;
		return true;
	}

	GLfloat params[MAX_PARAMS]; // alloca ?
	if (argc == 3 && jl::isArrayLike(cx, JL_ARG(3))) {

		uint32_t length;
		JL_CHK( jl::getVector(cx, JL_ARG(3), params, COUNTOF(params), &length ) );
		glMaterialfv( JL_ARG(1).toInt32(), JL_ARG(2).toInt32(), params );  OGL_ERR_CHK;
		;
		return true;
	}

	JL_ASSERT_ARGC_MIN( 3 ); // at least
	ASSERT( argc-2 < COUNTOF(params) );
	for ( unsigned int i = 2; i < argc; ++i )
		jl::getValue(cx, JL_ARG(i+1), &params[i-2]);
	glMaterialfv( JL_ARG(1).toInt32(), JL_ARG(2).toInt32(), params );  OGL_ERR_CHK;
	;
	return true;
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
DEFINE_FUNCTION( enable ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC_MIN(1);

	for ( unsigned i = 0; i < JL_ARGC; ++i ) {

		JL_ASSERT_ARG_IS_INTEGER(i+1);
		glEnable( JL_ARG(i+1).toInt32() );  OGL_ERR_CHK;
	}
	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( disable ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC_MIN(1);

	for ( unsigned i = 0; i < JL_ARGC; ++i ) {

		JL_ASSERT_ARG_IS_INTEGER(i+1);
		glDisable( JL_ARG(i+1).toInt32() );  OGL_ERR_CHK;
	}
	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( pointSize ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(1);

	float size;
	jl::getValue(cx, JL_ARG(1), &size);

	glPointSize(size);  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( lineWidth ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(1);

	float width;
	jl::getValue(cx, JL_ARG(1), &width);

	glLineWidth(width);  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( shadeModel ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	glShadeModel(JL_ARG(1).toInt32());  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( blendFunc ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(2);
	JL_ASSERT_ARG_IS_INTEGER(1);
	JL_ASSERT_ARG_IS_INTEGER(2);

	glBlendFunc(JL_ARG(1).toInt32(), JL_ARG(2).toInt32());  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( depthFunc ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	glDepthFunc( JL_ARG(1).toInt32() );  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( depthMask ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_BOOLEAN(1);

	glDepthMask( JL_ARG(1).toBoolean() );  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( depthRange ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(2);
	JL_ASSERT_ARG_IS_NUMBER(1);
	JL_ASSERT_ARG_IS_NUMBER(2);

	double zNear, zFar;
	jl::getValue(cx, JL_ARG(1), &zNear);
	jl::getValue(cx, JL_ARG(2), &zFar);

	glDepthRange(zNear, zFar);  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( polygonOffset ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

//	JL_INIT_OPENGL_EXTENSION( glPolygonOffsetEXT, PFNGLPOLYGONOFFSETEXTPROC );

	JL_ASSERT_ARGC(2);
	JL_ASSERT_ARG_IS_NUMBER(1);
	JL_ASSERT_ARG_IS_NUMBER(2);

	GLfloat factor, units;

	jl::getValue(cx, JL_ARG(1), &factor);
	jl::getValue(cx, JL_ARG(2), &units);

	glPolygonOffset(factor, units);  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( cullFace ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	glCullFace(JL_ARG(1).toInt32());  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( frontFace ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	glFrontFace(JL_ARG(1).toInt32());  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( clearStencil ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	GLuint s;
	if ( INT_TO_JSVAL(-1) == JL_ARG(1) )
		s = 0xffffffff;
	else
		JL_CHK( jl::getValue(cx, JL_ARG(1), &s) );

	glClearStencil(s);  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( clearDepth ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_NUMBER(1);

	double depth;
	jl::getValue(cx, JL_ARG(1), &depth);

	glClearDepth(depth);  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( clearColor ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(4);
	JL_ASSERT_ARG_IS_NUMBER(1);
	JL_ASSERT_ARG_IS_NUMBER(2);
	JL_ASSERT_ARG_IS_NUMBER(3);
	JL_ASSERT_ARG_IS_NUMBER(4);

	float r, g, b, a;
	jl::getValue(cx, JL_ARG(1), &r);
	jl::getValue(cx, JL_ARG(2), &g);
	jl::getValue(cx, JL_ARG(3), &b);
	jl::getValue(cx, JL_ARG(4), &a);

	glClearColor(r, g, b, a);  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( clearAccum ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(4);
	JL_ASSERT_ARG_IS_NUMBER(1);
	JL_ASSERT_ARG_IS_NUMBER(2);
	JL_ASSERT_ARG_IS_NUMBER(3);
	JL_ASSERT_ARG_IS_NUMBER(4);

	float r, g, b, a;
	jl::getValue(cx, JL_ARG(1), &r);
	jl::getValue(cx, JL_ARG(2), &g);
	jl::getValue(cx, JL_ARG(3), &b);
	jl::getValue(cx, JL_ARG(4), &a);

	glClearAccum(r, g, b, a);  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( clear ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	glClear(JL_ARG(1).toInt32());  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( red, green, blue, alpha )
 $VOID $INAME( all )
  enable and disable writing of frame buffer color components.
  $H arguments
   $ARG boolean red
   $ARG boolean green
   $ARG boolean blue
   $ARG boolean alpha Specify whether red, green, blue, and alpha can or cannot be written into the frame buffer. The initial values are all TRUE, indicating that the color components can be written.
	$ARG boolean all If true then all components can be written, and false mean the opposite.
  $H OpenGL API
   glColorMask
**/
DEFINE_FUNCTION( colorMask ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_RVAL.setUndefined();
	if ( JL_ARG_ISDEF(1) ) {

		JL_ASSERT_ARG_IS_BOOLEAN(1);
		if ( JL_ARG(1).isFalse() ) {

			glColorMask(0,0,0,0);  OGL_ERR_CHK;
		} else {

			glColorMask(1,1,1,1);  OGL_ERR_CHK;
		}
		return true;
	}

	JL_ASSERT_ARGC(4);
	JL_ASSERT_ARG_IS_BOOLEAN(1);
	JL_ASSERT_ARG_IS_BOOLEAN(2);
	JL_ASSERT_ARG_IS_BOOLEAN(3);
	JL_ASSERT_ARG_IS_BOOLEAN(4);

	glColorMask( JL_ARG(1).toBoolean(), JL_ARG(2).toBoolean(), JL_ARG(3).toBoolean(), JL_ARG(4).toBoolean() );  OGL_ERR_CHK;

	return true;
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
DEFINE_FUNCTION( clipPlane ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(2);
	JL_ASSERT_ARG_IS_INTEGER(1);
	JL_ASSERT_ARG_IS_ARRAY(2);

	GLdouble equation[4];
	uint32_t len;
	JL_CHK( jl::getVector(cx, JL_ARG(2), equation, COUNTOF(equation), &len ) );
	JL_CHKM( len == 4, E_ARG, E_NUM(2), E_LENGTH, E_NUM(4) );
	glClipPlane(JL_ARG(1).toInt32(), equation);  OGL_ERR_CHK;
	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( viewport ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(4);
	JL_ASSERT_ARG_IS_INTEGER(1);
	JL_ASSERT_ARG_IS_INTEGER(2);
	JL_ASSERT_ARG_IS_INTEGER(3);
	JL_ASSERT_ARG_IS_INTEGER(4);

	glViewport(JL_ARG(1).toInt32(), JL_ARG(2).toInt32(), JL_ARG(3).toInt32(), JL_ARG(4).toInt32());  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( frustum ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(6);
	JL_ASSERT_ARG_IS_NUMBER(1);
	JL_ASSERT_ARG_IS_NUMBER(2);
	JL_ASSERT_ARG_IS_NUMBER(3);
	JL_ASSERT_ARG_IS_NUMBER(4);
	JL_ASSERT_ARG_IS_NUMBER(5);
	JL_ASSERT_ARG_IS_NUMBER(6);

	double left, right, bottom, top, zNear, zFar;
	jl::getValue(cx, JL_ARG(1), &left);
	jl::getValue(cx, JL_ARG(2), &right);
	jl::getValue(cx, JL_ARG(3), &bottom);
	jl::getValue(cx, JL_ARG(4), &top);
	jl::getValue(cx, JL_ARG(5), &zNear);
	jl::getValue(cx, JL_ARG(6), &zFar);

	glFrustum(left, right, bottom, top, zNear, zFar);  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( ortho ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(6);
	JL_ASSERT_ARG_IS_NUMBER(1);
	JL_ASSERT_ARG_IS_NUMBER(2);
	JL_ASSERT_ARG_IS_NUMBER(3);
	JL_ASSERT_ARG_IS_NUMBER(4);
	JL_ASSERT_ARG_IS_NUMBER(5);
	JL_ASSERT_ARG_IS_NUMBER(6);

	double left, right, bottom, top, zNear, zFar;
	jl::getValue(cx, JL_ARG(1), &left);
	jl::getValue(cx, JL_ARG(2), &right);
	jl::getValue(cx, JL_ARG(3), &bottom);
	jl::getValue(cx, JL_ARG(4), &top);
	jl::getValue(cx, JL_ARG(5), &zNear);
	jl::getValue(cx, JL_ARG(6), &zFar);

	glOrtho(left, right, bottom, top, zNear, zFar);  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( perspective ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	//cf. gluPerspective(fovy, float(viewport[2]) / float(viewport[3]), zNear, zFar);

	JL_ASSERT_ARGC(4);
	JL_ASSERT_ARG_IS_NUMBER(1);
	//JL_ASSERT_NUMBER(JL_ARG(2)); // may be undefined
	JL_ASSERT_ARG_IS_NUMBER(3);
	JL_ASSERT_ARG_IS_NUMBER(4);


	double fovy, zNear, zFar, aspect;
	jl::getValue(cx, JL_ARG(1), &fovy);
	//	jl::getValue(cx, JL_ARG(2), &aspect)
	jl::getValue(cx, JL_ARG(3), &zNear);
	jl::getValue(cx, JL_ARG(4), &zFar);

//	GLint prevMatrixMode;
//	glGetIntegerv(GL_MATRIX_MODE, &prevMatrixMode);  OGL_ERR_CHK; // GL_MODELVIEW

	if ( JL_ARG_ISDEF(2) ) {

		jl::getValue(cx, JL_ARG(2), &aspect);
	} else {

		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);  OGL_ERR_CHK;
		aspect = ((double)viewport[2]) / ((double)viewport[3]);
	}
/*
	double xmin, xmax, ymin, ymax;
	ymax = zNear * tan(fovy * M_PI / 360.0f);
	ymin = -ymax;
	xmin = ymin * aspect;
	xmax = ymax * aspect;
	glFrustum(xmin, xmax, ymin, ymax, zNear, zFar);  OGL_ERR_CHK;
*/
	gluPerspective(fovy, aspect, zNear, zFar);  OGL_ERR_CHK;

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

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( matrixMode ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	glMatrixMode(JL_ARG(1).toInt32());  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  $H OpenGL API
   glLoadIdentity
**/
DEFINE_FUNCTION( loadIdentity ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(0);

	glLoadIdentity();  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  $H OpenGL API
   glPushMatrix
**/
DEFINE_FUNCTION( pushMatrix ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(0);

	glPushMatrix();  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  $H OpenGL API
   glPopMatrix
**/
DEFINE_FUNCTION( popMatrix ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(0);

	glPopMatrix();  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( loadMatrix ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(1);

	float tmp[16], *m = tmp;

	JL_CHK( jl::getMatrix44(cx, JL_ARG(1), &m) );
	glLoadMatrixf(m);  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( multMatrix ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(1);

	float tmp[16], *m = tmp;

	JL_CHK( jl::getMatrix44(cx, JL_ARG(1), &m) );

	glMultMatrixf(m);  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( rotate ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(4);

	double angle, x, y, z;
	jl::getValue(cx, JL_ARG(1), &angle);
	jl::getValue(cx, JL_ARG(2), &x);
	jl::getValue(cx, JL_ARG(3), &y);
	jl::getValue(cx, JL_ARG(4), &z);

	glRotated(angle, x, y, z);  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( translate ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC_RANGE(2,3);
	JL_ASSERT_ARG_IS_NUMBER(1);
	JL_ASSERT_ARG_IS_NUMBER(2);

	double x, y, z;
	jl::getValue(cx, JL_ARG(1), &x);
	jl::getValue(cx, JL_ARG(2), &y);
	if ( JL_ARG_ISDEF(3) )
		JL_CHK( jl::getValue(cx, JL_ARG(3), &z) );
	else
		z = 0;

	glTranslated(x, y, z);  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( scale ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC_RANGE(1,3);

	JL_RVAL.setUndefined();
	double x, y, z;
	jl::getValue(cx, JL_ARG(1), &x);

	if ( argc == 1 ) {

		glScaled(x, x, x);  OGL_ERR_CHK;
		;
		return true;
	}
	jl::getValue(cx, JL_ARG(2), &y);

	if ( argc >= 3 ) {

		jl::getValue(cx, JL_ARG(3), &z);
		glScaled(x, y, z);  OGL_ERR_CHK;
		;
		return true;
	}

	glScaled(x, y, 1);  OGL_ERR_CHK;

	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME( [ $BOOL compileOnly ] )
  Returns a new display-list.
  $H OpenGL API
   glGenLists, glNewList
**/
DEFINE_FUNCTION( newList ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC_RANGE(0,1);

	bool compileOnly;
	if ( JL_ARG_ISDEF(1) )
		jl::getValue(cx, JL_ARG(1), &compileOnly);
	else
		compileOnly = false;

	GLuint list;
	list = glGenLists(1);  OGL_ERR_CHK;
	glNewList(list, compileOnly ? GL_COMPILE : GL_COMPILE_AND_EXECUTE);  OGL_ERR_CHK;

	JL_RVAL.setInt32(list);
	return true;
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
DEFINE_FUNCTION( deleteList ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	glDeleteLists(JL_ARG(1).toInt32(), 1);  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  $H OpenGL API
   glEndList
**/
DEFINE_FUNCTION( endList ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(0);

	glEndList();  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( callList ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(1);

	JL_RVAL.setUndefined();

	if (JL_ARG(1).isInt32()) {

		glCallList(JL_ARG(1).toInt32());  OGL_ERR_CHK;
		return true;
	}

	if ( jl::isArray(cx, JL_ARG(1)) ) { // no array-like. convert a string into an array of calllist does not make sense here.

		JS::RootedObject jsArray(cx, &JL_ARG(1).toObject());
		uint32_t length;
		JL_CHK( JS_GetArrayLength(cx, jsArray, &length) );

		GLuint *lists = (GLuint*)alloca(length * sizeof(GLuint));
		/*
		jsval value;
		for ( unsigned i = 0; i < length; ++i ) {

			JL_CHK( JL_GetElement(cx, jsArray, i, value) );
			lists[i] = value.toInt32();
		}
		*/
		JL_CHK(jl::getVector(cx, JL_ARG(1), lists, length));
		glCallLists(length, GL_UNSIGNED_INT, lists);  OGL_ERR_CHK; // http://www.opengl.org/documentation/specs/man_pages/hardcopy/GL/html/gl/calllists.html

//		jl_free(lists); // alloca
		return true;
	}

	JL_ERR( E_ARG, E_NUM(1), E_TYPE, E_TY_INTEGER, E_OR, E_TY_ARRAY );
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
DEFINE_FUNCTION( polygonMode ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(2);
	JL_ASSERT_ARG_IS_INTEGER(1);
	JL_ASSERT_ARG_IS_INTEGER(2);

	glPolygonMode(JL_ARG(1).toInt32(), JL_ARG(2).toInt32());  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( begin ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

#ifdef DEBUG
	_inBeginOrEnd = true; // see OGL_ERR_CHK
#endif // DEBUG

	glBegin(JL_ARG(1).toInt32());  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  $H OpenGL API
   glEnd
**/
DEFINE_FUNCTION( end ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(0);

	glEnd();  OGL_ERR_CHK;

#ifdef DEBUG
	_inBeginOrEnd = false; // see OGL_ERR_CHK
#endif // DEBUG

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( pushAttrib ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_NUMBER(1);

//	JL_ARG_GEN(1, GLbitfield);
	
	GLbitfield mask;
	jl::getValue(cx, JL_ARG(1), &mask);

	glPushAttrib(mask);  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  $H OpenGL API
   glPopAttrib
**/
DEFINE_FUNCTION( popAttrib ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(0);

	glPopAttrib();  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME()
  Returns a new texture name.
  $H OpenGL API
   glGenTextures
**/
DEFINE_FUNCTION( genTexture ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(0);

	GLuint texture;
	glGenTextures(1, &texture);  OGL_ERR_CHK;

	JL_RVAL.setInt32(texture);
	return true;
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
DEFINE_FUNCTION( bindTexture ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(2);
	JL_ASSERT_ARG_IS_INTEGER(1);
//	JL_ASSERT_ARG_IS_INTEGER(2);

	int texture;
	JL_CHK( jl::getValue(cx, JL_ARG(2), &texture) );

	glBindTexture( JL_ARG(1).toInt32(), texture );  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( deleteTexture ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	GLuint texture = JL_ARG(1).toInt32();
	glDeleteTextures(1, &texture);  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( copyTexImage2D ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC_MIN(6);
	JL_ASSERT_ARG_IS_INTEGER(1);
	JL_ASSERT_ARG_IS_INTEGER(2);
	JL_ASSERT_ARG_IS_INTEGER(3);
	JL_ASSERT_ARG_IS_INTEGER(4);
	JL_ASSERT_ARG_IS_INTEGER(5);
	JL_ASSERT_ARG_IS_INTEGER(6);
	JL_ASSERT_ARG_IS_INTEGER(7);

	GLint border;
	if ( JL_ARG_ISDEF(8) ) {

		JL_ASSERT_ARG_IS_INTEGER(8);
		border = JL_ARG(8).toInt32();
	} else {

		border = 0;
	}

	glCopyTexImage2D(JL_ARG(1).toInt32(), JL_ARG(2).toInt32(), JL_ARG(3).toInt32(), JL_ARG(4).toInt32(), JL_ARG(5).toInt32(), JL_ARG(6).toInt32(), JL_ARG(7).toInt32(), border);  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	;
	return true;
	JL_BAD;
}




/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( pname, param )
  $H OpenGL API
   glPixelTransferi, glPixelTransferf
**/
DEFINE_FUNCTION( pixelTransfer ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARG_IS_INTEGER(1);

	GLenum pname = JL_ARG(1).toInt32();

	if (JL_ARG(2).isInt32()) {

		glPixelTransferi(pname, JL_ARG(2).toInt32());  OGL_ERR_CHK;
	} else {

		JL_ASSERT_ARG_IS_NUMBER(2);
		float param;
		jl::getValue(cx, JL_ARG(2), &param);
		glPixelTransferf(pname, param);  OGL_ERR_CHK;
	}

	JL_RVAL.setUndefined();
 	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( pname, param )
  $H OpenGL API
   glPixelStorei, glPixelStoref
**/
DEFINE_FUNCTION( pixelStore ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARG_IS_INTEGER(1);

	GLenum pname = JL_ARG(1).toInt32();

	if (JL_ARG(2).isInt32()) {

		glPixelStorei(pname, JL_ARG(2).toInt32());  OGL_ERR_CHK;
	} else {

		JL_ASSERT_ARG_IS_NUMBER(2);
		float param;
		jl::getValue(cx, JL_ARG(2), &param);
		glPixelStoref(pname, param);  OGL_ERR_CHK;
	}

	JL_RVAL.setUndefined();
 	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( x, y [ , z [ , w ] ] )
  $H OpenGL API
   glRasterPos*
**/
DEFINE_FUNCTION( rasterPos ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC_RANGE(2,4);
	JL_ASSERT_ARG_IS_NUMBER(1);
	JL_ASSERT_ARG_IS_NUMBER(2);

	double x, y, z, w;

	JL_RVAL.setUndefined();

	jl::getValue(cx, JL_ARG(1), &x);
	jl::getValue(cx, JL_ARG(2), &y);
	if ( argc >= 3 ) {

		JL_CHK( jl::getValue(cx, JL_ARG(3), &z) );
		if ( argc >= 4 ) {

			JL_CHK( jl::getValue(cx, JL_ARG(4), &w) );
			glRasterPos4d(x, y, z, w);  OGL_ERR_CHK;
			;
			return true;
		}
		glRasterPos3d(x, y, z);  OGL_ERR_CHK;
		;
		return true;
	}
	glRasterPos2d(x, y);  OGL_ERR_CHK;
	;
 	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( x, y )
  $H OpenGL API
   glPixelZoom
**/
DEFINE_FUNCTION( pixelZoom ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(2);
	JL_ASSERT_ARG_IS_NUMBER(1);
	JL_ASSERT_ARG_IS_NUMBER(2);

	float x, y;

	jl::getValue(cx, JL_ARG(1), &x);
	jl::getValue(cx, JL_ARG(2), &y);
	glPixelZoom(x, y);  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
 	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( map, values )
  $H OpenGL API
   glPixelMapfv
**/
DEFINE_FUNCTION(pixelMap) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(2);
	JL_ASSERT_ARG_IS_ARRAY(2);

	{
		uint32_t mapsize;
		JS::RootedObject arrayObj(cx, &JL_ARG(2).toObject());
		JL_CHK(JS_GetArrayLength(cx, arrayObj, &mapsize));
		GLfloat *values = (GLfloat*)alloca(mapsize * sizeof(*values));
		JL_CHK(jl::getVector(cx, JL_ARG(2), values, mapsize, &mapsize));
		glPixelMapfv(JL_ARG(1).toInt32(), mapsize, values);  OGL_ERR_CHK;

		JL_RVAL.setUndefined();
	}
 	return true;
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
DEFINE_FUNCTION( hasExtensionProc ) {

	jl::BufString procName;
	JL_DEFINE_ARGS;

	OGL_CX_CHK;
	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT( glGetProcAddress != NULL, E_OS, E_INIT, E_STR("OpenGL"), E_COMMENT("extensions") );

	void *procAddr;
	for ( unsigned i = 0; i < JL_ARGC; ++i ) {

		JL_CHK( jl::getValue(cx, JL_ARG(i+1), &procName) );
		procAddr = glGetProcAddress(procName);
		if ( procAddr == NULL ) {

			JL_RVAL.setBoolean(false);
			return true;
		}
	}

	JL_RVAL.setBoolean(true);
	return true;
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
DEFINE_FUNCTION( hasExtensionName ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC_MIN(1);

	const char *extensions = (const char *)glGetString(GL_EXTENSIONS);
	ASSERT( extensions != NULL );

	for ( unsigned i = 0; i < JL_ARGC; ++i ) {

		jl::BufString name;
//		const char *name;
//		unsigned int nameLength;
//		JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARGV[i], &name, &nameLength) );
		JL_CHK( jl::getValue(cx, JL_ARG(i+1), &name) );

		const char *pos = strstr(extensions, name);
		size_t nameLen = name.length();
		if (pos == NULL || (pos[nameLen] != ' ' && pos[nameLen] != '\0')) {

			JL_RVAL.setBoolean(false);
			return true;
		}
	}
	JL_RVAL.setBoolean(true);
	return true;
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
DEFINE_FUNCTION( blendEquation ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glBlendEquation, PFNGLBLENDEQUATIONPROC );

	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	glBlendEquation(JL_ARG(1).toInt32());

	JL_RVAL.setUndefined();
 	return true;
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
DEFINE_FUNCTION( stencilFuncSeparate ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glStencilFuncSeparate, PFNGLSTENCILFUNCSEPARATEPROC ); // Opengl 2.0+

	JL_ASSERT_ARGC(4);
	JL_ASSERT_ARG_IS_INTEGER(1);
	JL_ASSERT_ARG_IS_INTEGER(2);
	JL_ASSERT_ARG_IS_INTEGER(3);
	JL_ASSERT_ARG_IS_NUMBER(4);

	GLuint mask;
	if ( INT_TO_JSVAL(-1) == JL_ARG(4) )
		mask = 0xffffffff;
	else
		JL_CHK( jl::getValue(cx, JL_ARG(4), &mask) );

	glStencilFuncSeparate(JL_ARG(1).toInt32(), JL_ARG(2).toInt32(), JL_ARG(3).toInt32(), mask);  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( stencilOpSeparate ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glStencilOpSeparate, PFNGLSTENCILOPSEPARATEPROC ); // Opengl 2.0+

	JL_ASSERT_ARGC(4);
	JL_ASSERT_ARG_IS_INTEGER(1);
	JL_ASSERT_ARG_IS_INTEGER(2);
	JL_ASSERT_ARG_IS_INTEGER(3);
	JL_ASSERT_ARG_IS_INTEGER(4);

	glStencilOpSeparate(JL_ARG(1).toInt32(), JL_ARG(2).toInt32(), JL_ARG(3).toInt32(), JL_ARG(4).toInt32());  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( activeStencilFaceEXT ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glActiveStencilFaceEXT, PFNGLACTIVESTENCILFACEEXTPROC ); // Opengl 2.0+

	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	glActiveStencilFaceEXT(JL_ARG(1).toInt32());  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( bindRenderbuffer ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glBindRenderbufferEXT, PFNGLBINDRENDERBUFFEREXTPROC );

	JL_ASSERT_ARGC(2);
	JL_ASSERT_ARG_IS_INTEGER(1);
	JL_ASSERT_ARG_IS_INTEGER(2);

	glBindRenderbufferEXT(JL_ARG(1).toInt32(), JL_ARG(2).toInt32());  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME()
  $H OpenGL API
   glGenRenderbuffersEXT
**/
DEFINE_FUNCTION( genRenderbuffer ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glGenRenderbuffersEXT, PFNGLGENRENDERBUFFERSEXTPROC);

	JL_ASSERT_ARGC(0);
	GLuint buffer;

	glGenRenderbuffersEXT(1, &buffer);  OGL_ERR_CHK;

	JL_RVAL.setInt32(buffer);

	return true;
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
DEFINE_FUNCTION( deleteRenderbuffer ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glDeleteRenderbuffersEXT, PFNGLDELETERENDERBUFFERSEXTPROC );

	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_INTEGER(1);
	GLuint buffer = JL_ARG(1).toInt32();
	glDeleteRenderbuffersEXT(1, &buffer);  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( renderbufferStorage ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glRenderbufferStorageEXT, PFNGLRENDERBUFFERSTORAGEEXTPROC );

	JL_ASSERT_ARGC(4);
	JL_ASSERT_ARG_IS_INTEGER(1);
	JL_ASSERT_ARG_IS_INTEGER(2);
	JL_ASSERT_ARG_IS_INTEGER(3);
	JL_ASSERT_ARG_IS_INTEGER(4);
	glRenderbufferStorageEXT(JL_ARG(1).toInt32(), JL_ARG(2).toInt32(), JL_ARG(3).toInt32(), JL_ARG(4).toInt32());  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( getRenderbufferParameter ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glGetRenderbufferParameterivEXT, PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC );

	JL_ASSERT_ARGC_RANGE(2,3);
	JL_ASSERT_ARG_IS_INTEGER(1);
	JL_ASSERT_ARG_IS_INTEGER(2);

	GLint params[MAX_PARAMS]; // (TBD) check if it is the max amount of data that glGetRenderbufferParameterivEXT may returns.

	glGetRenderbufferParameterivEXT(JL_ARG(1).toInt32(), JL_ARG(2).toInt32(), params );  OGL_ERR_CHK;

	if ( JL_ARG_ISDEF(3) ) {

		JL_ASSERT_ARG_IS_INTEGER(3);
		int count;
		count = jl::min(JL_ARG(3).toInt32(), (int32_t)COUNTOF(params));
		JL_CHK( jl::setVector(cx, JL_RVAL, params, count) );
	} else {

		JL_RVAL.setInt32( params[0] );
	}

	return true;
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
DEFINE_FUNCTION( bindFramebuffer ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glBindFramebufferEXT, PFNGLBINDFRAMEBUFFEREXTPROC );

	JL_ASSERT_ARGC(2);
	JL_ASSERT_ARG_IS_INTEGER(1);
	JL_ASSERT_ARG_IS_INTEGER(2);
	glBindFramebufferEXT( JL_ARG(1).toInt32(), JL_ARG(2).toInt32() );  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME()
  $H OpenGL API
   glGenFramebuffersEXT
**/
DEFINE_FUNCTION( genFramebuffer ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glGenFramebuffersEXT, PFNGLGENFRAMEBUFFERSEXTPROC );

	JL_ASSERT_ARGC(0);
	GLuint buffer;
	glGenFramebuffersEXT(1, &buffer);  OGL_ERR_CHK;
	JL_RVAL.setInt32(buffer);

	return true;
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
DEFINE_FUNCTION( deleteFramebuffer ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glDeleteFramebuffersEXT, PFNGLDELETEFRAMEBUFFERSEXTPROC );

	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	GLuint buffer = JL_ARG(1).toInt32();
	glDeleteFramebuffersEXT(1, &buffer);  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( checkFramebufferStatus ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glCheckFramebufferStatusEXT, PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC );

	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	JL_RVAL.setInt32( glCheckFramebufferStatusEXT(JL_ARG(1).toInt32()) );  OGL_ERR_CHK;

	return true;
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
DEFINE_FUNCTION( framebufferTexture1D ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glFramebufferTexture1DEXT, PFNGLFRAMEBUFFERTEXTURE1DEXTPROC );

	JL_ASSERT_ARGC(5);
	JL_ASSERT_ARG_IS_INTEGER(1);
	JL_ASSERT_ARG_IS_INTEGER(2);
	JL_ASSERT_ARG_IS_INTEGER(3);
	JL_ASSERT_ARG_IS_INTEGER(4);
	JL_ASSERT_ARG_IS_INTEGER(5);

	glFramebufferTexture1DEXT( JL_ARG(1).toInt32(), JL_ARG(2).toInt32(), JL_ARG(3).toInt32(), JL_ARG(4).toInt32(), JL_ARG(5).toInt32() );  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( framebufferTexture2D ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glFramebufferTexture2DEXT, PFNGLFRAMEBUFFERTEXTURE2DEXTPROC );

	JL_ASSERT_ARGC(5);
	JL_ASSERT_ARG_IS_INTEGER(1);
	JL_ASSERT_ARG_IS_INTEGER(2);
	JL_ASSERT_ARG_IS_INTEGER(3);
	JL_ASSERT_ARG_IS_INTEGER(4);
	JL_ASSERT_ARG_IS_INTEGER(5);

	glFramebufferTexture2DEXT( JL_ARG(1).toInt32(), JL_ARG(2).toInt32(), JL_ARG(3).toInt32(), JL_ARG(4).toInt32(), JL_ARG(5).toInt32() );  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( framebufferTexture3D ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glFramebufferTexture3DEXT, PFNGLFRAMEBUFFERTEXTURE3DEXTPROC );

	JL_ASSERT_ARGC(5);
	JL_ASSERT_ARG_IS_INTEGER(1);
	JL_ASSERT_ARG_IS_INTEGER(2);
	JL_ASSERT_ARG_IS_INTEGER(3);
	JL_ASSERT_ARG_IS_INTEGER(4);
	JL_ASSERT_ARG_IS_INTEGER(5);
	JL_ASSERT_ARG_IS_INTEGER(6);

	glFramebufferTexture3DEXT( JL_ARG(1).toInt32(), JL_ARG(2).toInt32(), JL_ARG(3).toInt32(), JL_ARG(4).toInt32(), JL_ARG(5).toInt32(), JL_ARG(6).toInt32() );  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( framebufferRenderbuffer ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glFramebufferRenderbufferEXT, PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC );

	JL_ASSERT_ARGC(5);
	JL_ASSERT_ARG_IS_INTEGER(1);
	JL_ASSERT_ARG_IS_INTEGER(2);
	JL_ASSERT_ARG_IS_INTEGER(3);
	JL_ASSERT_ARG_IS_INTEGER(4);

	glFramebufferRenderbufferEXT( JL_ARG(1).toInt32(), JL_ARG(2).toInt32(), JL_ARG(3).toInt32(), JL_ARG(4).toInt32() );  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( getFramebufferAttachmentParameter ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glGetFramebufferAttachmentParameterivEXT, PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC  );

	JL_ASSERT_ARGC_RANGE(3,4);
	JL_ASSERT_ARG_IS_INTEGER(1);
	JL_ASSERT_ARG_IS_INTEGER(2);
	JL_ASSERT_ARG_IS_INTEGER(3);

	GLint params[MAX_PARAMS]; // (TBD) check if it is the max amount of data that glGetRenderbufferParameterivEXT may returns.

	glGetFramebufferAttachmentParameterivEXT(JL_ARG(1).toInt32(), JL_ARG(2).toInt32(), JL_ARG(3).toInt32(), params);  OGL_ERR_CHK;

	if ( JL_ARG_ISDEF(4) ) {

		JL_ASSERT_ARG_IS_INTEGER(4);
		int count;
		count = jl::min(JL_ARG(4).toInt32(), (int)COUNTOF(params));
		JL_CHK( jl::setVector(cx, JL_RVAL, params, count) );
	} else {

		JL_RVAL.setInt32( params[0] );
	}

	return true;
	JL_BAD;
}




/**doc
$TOC_MEMBER $INAME
 $INT $INAME( shaderType )
  (TBD)
 $H OpenGL API
   glCreateShaderObjectARB
**/
DEFINE_FUNCTION( createShaderObject ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glCreateShaderObjectARB, PFNGLCREATESHADEROBJECTARBPROC );
	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	GLenum shaderType;
	shaderType = JL_ARG(1).toInt32();
	GLhandleARB handle = glCreateShaderObjectARB(shaderType);  OGL_ERR_CHK;
	JL_RVAL.setInt32(handle);

	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME( shader )
  (TBD)
 $H OpenGL API
   glDeleteObjectARB
**/
DEFINE_FUNCTION( deleteObject ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glDeleteObjectARB, PFNGLDELETEOBJECTARBPROC );
	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	GLhandleARB shaderHandle;
	shaderHandle = JL_ARG(1).toInt32();
	glDeleteObjectARB(shaderHandle);  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME( shader )
  (TBD)
 $H OpenGL API
   glGetInfoLogARB
**/
DEFINE_FUNCTION( getInfoLog ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glGetInfoLogARB, PFNGLGETINFOLOGARBPROC );
	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	GLhandleARB shaderHandle;
	shaderHandle = JL_ARG(1).toInt32();
	GLsizei length;
	glGetObjectParameterivARB(shaderHandle, GL_OBJECT_INFO_LOG_LENGTH_ARB, &length);  OGL_ERR_CHK;
	GLcharARB *buffer = (GLcharARB*)alloca(length+1);
	buffer[length] = '\0'; // needed ?
	glGetInfoLogARB(shaderHandle, length, &length, buffer);  OGL_ERR_CHK;
	JL_CHK(jl::setValue(cx, JL_RVAL, jl::CStrSpec(buffer, length)));

	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME()
  (TBD)
 $H OpenGL API
   glCreateProgramObjectARB
**/
DEFINE_FUNCTION( createProgramObject ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_IGNORE(argc);
	JL_INIT_OPENGL_EXTENSION( glCreateProgramObjectARB, PFNGLCREATEPROGRAMOBJECTARBPROC );

	GLhandleARB programHandle = glCreateProgramObjectARB();  OGL_ERR_CHK;
	JL_RVAL.setInt32(programHandle);
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME( shader, source )
  (TBD)
 $H OpenGL API
   glShaderSourceARB
**/
DEFINE_FUNCTION( shaderSource ) {

	jl::BufString source;
	JL_DEFINE_ARGS;

	OGL_CX_CHK;
	JL_INIT_OPENGL_EXTENSION( glShaderSourceARB, PFNGLSHADERSOURCEARBPROC );
	JL_ASSERT_ARGC(2);
	JL_ASSERT_ARG_IS_INTEGER(1);
	JL_ASSERT_ARG_IS_STRING(2);

	GLhandleARB shaderHandle;
	shaderHandle = JL_ARG(1).toInt32();
	JL_CHK( jl::getValue(cx, JL_ARG(2), &source) );

	const GLcharARB *buffer;
	GLint length;
	length = source.length();
	buffer = source.toData<const char*>();

	glShaderSourceARB(shaderHandle, 1, &buffer, &length);  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME( shader )
  (TBD)
 $H OpenGL API
   glCompileShaderARB
**/
DEFINE_FUNCTION( compileShader ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glCompileShaderARB, PFNGLCOMPILESHADERARBPROC );
	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	GLhandleARB shaderHandle;
	shaderHandle = JL_ARG(1).toInt32();
	glCompileShaderARB(shaderHandle);  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME( program, shaderHandle )
  (TBD)
 $H OpenGL API
   glCompileShaderARB
**/
DEFINE_FUNCTION( attachObject ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glAttachObjectARB, PFNGLATTACHOBJECTARBPROC );
	JL_ASSERT_ARGC(2);

//	JL_ASSERT_ARG_IS_INTEGER(1);
	GLhandleARB programHandle;
//	programHandle = JL_ARG(1).toInt32();
	JL_CHK( jl::getValue(cx, JL_ARG(1), &programHandle) );

	GLhandleARB shaderHandle;
	shaderHandle = JL_ARG(2).toInt32();

	glAttachObjectARB(programHandle, shaderHandle);  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME( program )
  (TBD)
 $H OpenGL API
   glLinkProgramARB
**/
DEFINE_FUNCTION( linkProgram ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glLinkProgramARB, PFNGLLINKPROGRAMARBPROC );
//	JL_INIT_OPENGL_EXTENSION( glUseProgramObjectARB, PFNGLUSEPROGRAMOBJECTARBPROC );
//	JL_INIT_OPENGL_EXTENSION( glGetProgramiv, PFNGLGETPROGRAMIVPROC );

	JL_ASSERT_ARGC(1);
//	JL_ASSERT_ARG_IS_INTEGER(1);

	GLhandleARB programHandle;
//	programHandle = JL_ARG(1).toInt32();
	JL_CHK( jl::getValue(cx, JL_ARG(1), &programHandle) );

	glLinkProgramARB(programHandle);  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME( programHandle )
  (TBD)
 $H OpenGL API
   glUseProgramObjectARB
**/
DEFINE_FUNCTION( useProgramObject ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glUseProgramObjectARB, PFNGLUSEPROGRAMOBJECTARBPROC );
	JL_ASSERT_ARGC(1);
//	JL_ASSERT_ARG_IS_INTEGER(1);

	GLhandleARB programHandle;
//	programHandle = JL_ARG(1).toInt32();
	JL_CHK( jl::getValue(cx, JL_ARG(1), &programHandle) );

	glUseProgramObjectARB(programHandle);  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME( program )
  (TBD)
 $H OpenGL API
   glGetActiveUniformARB, glGetUniformLocationARB, glGetProgramiv
**/
DEFINE_FUNCTION( getUniformInfo ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glGetActiveUniformARB, PFNGLGETACTIVEUNIFORMARBPROC );
	JL_INIT_OPENGL_EXTENSION( glGetUniformLocationARB, PFNGLGETUNIFORMLOCATIONARBPROC );
	JL_INIT_OPENGL_EXTENSION( glGetProgramiv, PFNGLGETPROGRAMIVPROC );

	JL_ASSERT_ARGC(1);

	GLhandleARB program;
	GLint activeUniform;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &program) );
	glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &activeUniform);  OGL_ERR_CHK;

	{
		JS::RootedObject info(cx, JL_NewObj(cx));
		JL_CHK( info );
		JL_RVAL.setObject(*info);

		GLcharARB name[256];
		GLsizei length;
		GLint size;
		GLenum type;
		GLint location;

		
		JS::RootedValue tmp(cx);
		JS::RootedId indexId(cx, jl::stringToJsid(cx, L("index")));
		JS::RootedId locationId(cx, jl::stringToJsid(cx, L("location")));
		JS::RootedId sizeId(cx, jl::stringToJsid(cx, L("size")));
		JS::RootedId typeId(cx, jl::stringToJsid(cx, L("type")));

		for ( int i = 0; i < activeUniform; ++i ) {

			glGetActiveUniformARB(program, i, sizeof(name), &length, &size, &type, name);  OGL_ERR_CHK;
			ASSERT( length < sizeof(name) );

			location = glGetUniformLocationARB(program, name);
			if ( location == -1 )
				continue;

			JS::RootedObject obj(cx, JL_NewObj(cx));
			JL_CHK( obj );

			tmp.setObject(*obj);
			JL_CHK( JS_SetProperty(cx, info, name, tmp) );

			tmp.setInt32(i);
			JL_CHK( JS_SetPropertyById(cx, obj, indexId, tmp) );
			tmp.setInt32(location);
			JL_CHK( JS_SetPropertyById(cx, obj, locationId, tmp) );
			tmp.setInt32(size);
			JL_CHK( JS_SetPropertyById(cx, obj, sizeId, tmp) );
			tmp.setInt32(type);
			JL_CHK( JS_SetPropertyById(cx, obj, typeId, tmp) );
		}
	}

	return true;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $INT $INAME( program, name )
  (TBD)
 $H OpenGL API
   glGetUniformLocationARB
**/
DEFINE_FUNCTION( getUniformLocation ) {

	jl::BufString name;

	JL_DEFINE_ARGS;

	OGL_CX_CHK;
	JL_INIT_OPENGL_EXTENSION( glGetUniformLocationARB, PFNGLGETUNIFORMLOCATIONARBPROC );

	JL_ASSERT_ARGC(2);
	//JL_ASSERT_ARG_IS_INTEGER(1);

	GLhandleARB programHandle;
	//programHandle = JL_ARG(1).toInt32();
	JL_CHK( jl::getValue(cx, JL_ARG(1), &programHandle) );

	JL_CHK( jl::getValue(cx, JL_ARG(2), &name) );
	GLint uniformLocation;

	uniformLocation = glGetUniformLocationARB(programHandle, name);  OGL_ERR_CHK;

	JL_RVAL.setInt32(uniformLocation);

	return true;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $INT $INAME( uniformLocation, value )
  (TBD)
 $H OpenGL API
  glUniform1fARB, glUniform1iARB
**/
DEFINE_FUNCTION( uniform ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glUniform1fARB, PFNGLUNIFORM1FARBPROC );
	JL_INIT_OPENGL_EXTENSION( glUniform2fARB, PFNGLUNIFORM2FARBPROC );
	JL_INIT_OPENGL_EXTENSION( glUniform3fARB, PFNGLUNIFORM3FARBPROC );
	JL_INIT_OPENGL_EXTENSION( glUniform4fARB, PFNGLUNIFORM4FARBPROC );

	JL_INIT_OPENGL_EXTENSION( glUniform1fvARB, PFNGLUNIFORM1FVARBPROC );
	JL_INIT_OPENGL_EXTENSION( glUniform2fvARB, PFNGLUNIFORM2FVARBPROC );
	JL_INIT_OPENGL_EXTENSION( glUniform3fvARB, PFNGLUNIFORM3FVARBPROC );
	JL_INIT_OPENGL_EXTENSION( glUniform4fvARB, PFNGLUNIFORM4FVARBPROC );

//	JL_INIT_OPENGL_EXTENSION( glGetUniformfvARB, PFNGLGETUNIFORMFVARBPROC );

	JL_INIT_OPENGL_EXTENSION( glUniform1iARB, PFNGLUNIFORM1IARBPROC );
	JL_INIT_OPENGL_EXTENSION( glUniform2iARB, PFNGLUNIFORM2IARBPROC );
	JL_INIT_OPENGL_EXTENSION( glUniform3iARB, PFNGLUNIFORM3IARBPROC );
	JL_INIT_OPENGL_EXTENSION( glUniform4iARB, PFNGLUNIFORM4IARBPROC );

	JL_INIT_OPENGL_EXTENSION( glUniform1ivARB, PFNGLUNIFORM1IVARBPROC );
	JL_INIT_OPENGL_EXTENSION( glUniform2ivARB, PFNGLUNIFORM2IVARBPROC );
	JL_INIT_OPENGL_EXTENSION( glUniform3ivARB, PFNGLUNIFORM3IVARBPROC );
	JL_INIT_OPENGL_EXTENSION( glUniform4ivARB, PFNGLUNIFORM4IVARBPROC );

//	JL_INIT_OPENGL_EXTENSION( glGetUniformivARB, PFNGLGETUNIFORMIVARBPROC );

	JL_INIT_OPENGL_EXTENSION( glUniformMatrix2fvARB, PFNGLUNIFORMMATRIX2FVARBPROC );
	JL_INIT_OPENGL_EXTENSION( glUniformMatrix3fvARB, PFNGLUNIFORMMATRIX3FVARBPROC );
	JL_INIT_OPENGL_EXTENSION( glUniformMatrix4fvARB, PFNGLUNIFORMMATRIX4FVARBPROC );

//GL_ARB_uniform_buffer_object extension || OpenGL >= 3.1 ?
//	JL_INIT_OPENGL_EXTENSION( glGetUniformBlockIndex, PFNGLGETUNIFORMBLOCKINDEXPROC );
//	JL_INIT_OPENGL_EXTENSION( glUniformBlockBinding, PFNGLUNIFORMBLOCKBINDINGPROC );
//	JL_INIT_OPENGL_EXTENSION( glGetActiveUniformsiv, PFNGLGETACTIVEUNIFORMSIVPROC );

	JL_ASSERT_ARGC_RANGE(2, 5);
	JL_ASSERT_ARG_IS_INTEGER(1);

	JL_RVAL.setUndefined();

	int uniformLocation;
	uniformLocation = JL_ARG(1).toInt32();






	/*
		// OpenGL 2.0 only ?, see glGetActiveUniformARB and glGetActiveUniform

		GLint program;
		glGetIntegerv(GL_CURRENT_PROGRAM, &program);  OGL_ERR_CHK;
		GLsizei length;
		GLint size;
		GLenum type;
		#ifdef DEBUG
		GLcharARB name[128];
		glGetActiveUniformARB(program, uniformLocation <-- incorrect! , sizeof(name), &length, &size, &type, name);  OGL_ERR_CHK;
		#else
		glGetActiveUniformARB(program, uniformLocation <-- incorrect! , 0, &length, &size, &type, NULL);  OGL_ERR_CHK;
		#endif // DEBUG

		bool isFloat;
		switch (type) {
		case GL_FLOAT:
		case GL_FLOAT_VEC2:
		case GL_FLOAT_VEC3:
		case GL_FLOAT_VEC4:
		case GL_FLOAT_MAT2:
		case GL_FLOAT_MAT3:
		case GL_FLOAT_MAT4:
		case GL_FLOAT_MAT2x3:
		case GL_FLOAT_MAT2x4:
		case GL_FLOAT_MAT3x2:
		case GL_FLOAT_MAT3x4:
		case GL_FLOAT_MAT4x2:
		case GL_FLOAT_MAT4x3:
		isFloat = true;
		break;
		default:
		isFloat = false;
		}

		// type:
		//   GL_FLOAT, GL_FLOAT_VEC2, GL_FLOAT_VEC3, GL_FLOAT_VEC4, GL_INT, GL_INT_VEC2, GL_INT_VEC3, GL_INT_VEC4,
		//   GL_BOOL, GL_BOOL_VEC2, GL_BOOL_VEC3, GL_BOOL_VEC4, GL_FLOAT_MAT2, GL_FLOAT_MAT3, GL_FLOAT_MAT4, GL_FLOAT_MAT2x3,
		//   GL_FLOAT_MAT2x4, GL_FLOAT_MAT3x2, GL_FLOAT_MAT3x4, GL_FLOAT_MAT4x2, GL_FLOAT_MAT4x3, GL_SAMPLER_1D, GL_SAMPLER_2D, GL_SAMPLER_3D, GL_SAMPLER_CUBE, GL_SAMPLER_1D_SHADOW, or GL_SAMPLER_2D_SHADOW
		//   GL_FLOAT_MAT2x3, GL_FLOAT_MAT2x4, GL_FLOAT_MAT3x2, GL_FLOAT_MAT3x4, GL_FLOAT_MAT4x2, and GL_FLOAT_MAT4x3 (OpenGL 2.1)
		*/


	//jsval staticArgs[4];
	//jsval *aargs;

	{
		JS::AutoValueArray<4> aargs(cx);

		uint32_t count;
		uint32_t i;
		if (JL_ARGC == 2 && jl::isArrayLike(cx, JL_ARG(2))) { // (TBD) check if array-like make sense here.

			JS::RootedObject arr(cx, &JL_ARG(2).toObject());
			JL_CHK(JS_GetArrayLength(cx, arr, &count));

			i = count;
			while (i--)
				JL_CHK(jl::getElement(cx, arr, i, aargs[i]));
		}
		else {

			count = JL_ARGC - 1;

			i = count;
			while (i--)
				aargs[i].set(JL_ARGV[i + 1]);
		}

		JL_CHKM(count >= 1 && count <= 4, E_ARGC, E_RANGE, E_INTERVAL_NUM(1, 4));

		if (aargs[0].isDouble()) {

			float v1, v2, v3, v4;
			JL_CHK(jl::getValue(cx, aargs[0], &v1));
			if (count > 1) {

				JL_CHK(jl::getValue(cx, aargs[1], &v2));
				if (count > 2) {

					JL_CHK(jl::getValue(cx, aargs[2], &v3));
					if (count > 3) {

						JL_CHK(jl::getValue(cx, aargs[3], &v4));
						glUniform4fARB(uniformLocation, v1, v2, v3, v4);  OGL_ERR_CHK;
						return true;
					}
					glUniform3fARB(uniformLocation, v1, v2, v3);  OGL_ERR_CHK;
					return true;
				}
				glUniform2fARB(uniformLocation, v1, v2);  OGL_ERR_CHK;
				return true;
			}
			glUniform1fARB(uniformLocation, v1);  OGL_ERR_CHK;
			return true;
		}
		else {

			int v1, v2, v3, v4;
			JL_CHK(jl::getValue(cx, aargs[0], &v1));
			if (count > 1) {

				JL_CHK(jl::getValue(cx, aargs[1], &v2));
				if (count > 2) {

					JL_CHK(jl::getValue(cx, aargs[2], &v3));
					if (count > 3) {

						JL_CHK(jl::getValue(cx, aargs[3], &v4));
						glUniform4iARB(uniformLocation, v1, v2, v3, v4);  OGL_ERR_CHK;
						if (glGetError() == GL_INVALID_OPERATION)
							glUniform4fARB(uniformLocation, (float)v1, (float)v2, (float)v3, (float)v4);  OGL_ERR_CHK;
						return true;
					}
					glUniform3iARB(uniformLocation, v1, v2, v3);  OGL_ERR_CHK;
					if (glGetError() == GL_INVALID_OPERATION)
						glUniform3fARB(uniformLocation, (float)v1, (float)v2, (float)v3);  OGL_ERR_CHK;
					return true;
				}
				glUniform2iARB(uniformLocation, v1, v2);  OGL_ERR_CHK;
				if (glGetError() == GL_INVALID_OPERATION)
					glUniform2fARB(uniformLocation, (float)v1, (float)v2);  OGL_ERR_CHK;
				return true;
			}
			glUniform1iARB(uniformLocation, v1);  OGL_ERR_CHK;
			if (glGetError() == GL_INVALID_OPERATION)
				glUniform1fARB(uniformLocation, (float)v1);  OGL_ERR_CHK;
			return true;
		}

	}

/*
	jsval arg2;
	arg2 = JL_ARG(2);

	float v1, v2, v3, v4;

	if ( JL_ARGC == 2 ) {

		if ( JSVAL_IS_INT(arg2) ) {

			glUniform1iARB(uniformLocation, arg2.toInt32());  OGL_ERR_CHK;
			return true;
		}
		if ( arg2.isBoolean() ) {

			glUniform1iARB(uniformLocation, arg2 == JSVAL_TRUE ? 1 : 0);  OGL_ERR_CHK;
			return true;
		}
	}

	if ( JSVAL_IS_NUMBER(arg2) ) {

		JL_CHK( jl::getValue(cx, arg2, &v1) );
		if ( JL_ARGC >= 3 ) {

			JL_CHK( jl::getValue(cx, JL_ARG(3), &v2) );
			if ( JL_ARGC >= 4 ) {

				JL_CHK( jl::getValue(cx, JL_ARG(4), &v3) );
				if ( JL_ARGC >= 5 ) {

					JL_CHK( jl::getValue(cx, JL_ARG(5), &v4) );
					glUniform4fARB(uniformLocation, v1, v2, v3, v4);  OGL_ERR_CHK;
					return true;
				}
				glUniform3fARB(uniformLocation, v1, v2, v3);  OGL_ERR_CHK;
				return true;
			}
			glUniform2fARB(uniformLocation, v1, v2);  OGL_ERR_CHK;
			return true;
		}
		glUniform1fARB(uniformLocation, v1);  OGL_ERR_CHK;
		return true;
	}



	if ( JL_IsArray(cx, arg2) ) {

		JS::RootedObject arrayObj;
		arrayObj = JSVAL_TO_OBJECT(JL_ARG(2));
		uint32_t currentLength;
		JL_CHK( JS_GetArrayLength(cx, arrayObj, &currentLength) );
// (TBD)
	}
*/

	JL_ERR( E_VALUE, E_INVALID );

	return true;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $INT $INAME( uniformLocation, matrix44 )
  (TBD)
 $H OpenGL API
  glUniformMatrix4fvARB
**/
DEFINE_FUNCTION( uniformMatrix ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glUniformMatrix4fvARB, PFNGLUNIFORMMATRIX4FVARBPROC );

	JL_ASSERT_ARGC(2);
	JL_ASSERT_ARG_IS_INTEGER(1);

	GLint uniformLocation;
	uniformLocation = JL_ARG(1).toInt32();
	JL_RVAL.setUndefined();

	float tmp[16], *m = tmp;
	JL_CHK( jl::getMatrix44(cx, JL_ARG(2), &m) );
	glUniformMatrix4fvARB(uniformLocation, 1, false, m);  OGL_ERR_CHK;

	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME( uniformLocation, vec [,vec2 [, vecN]] )
  (TBD)
 $H example
{{{
  $INAME( loc, [10,-1,5]);
  $INAME( loc1, [1,1,1,0]);
}}}
 $H OpenGL API
  glUniform1fARB, glUniform2fARB, glUniform3fARB, glUniform4fARB
**/
DEFINE_FUNCTION( uniformFloatVector ) {

	GLfloat *value = NULL;
	JL_DEFINE_ARGS;

	OGL_CX_CHK;
	JL_INIT_OPENGL_EXTENSION( glUniform1fvARB, PFNGLUNIFORM1FVARBPROC );
	JL_INIT_OPENGL_EXTENSION( glUniform2fvARB, PFNGLUNIFORM2FVARBPROC );
	JL_INIT_OPENGL_EXTENSION( glUniform3fvARB, PFNGLUNIFORM3FVARBPROC );
	JL_INIT_OPENGL_EXTENSION( glUniform4fvARB, PFNGLUNIFORM4FVARBPROC );

	JL_ASSERT_ARGC_MIN(2);
	JL_ASSERT_ARG_IS_INTEGER(1);

	GLint uniformLocation;
	uniformLocation = JL_ARG(1).toInt32();
	JL_RVAL.setUndefined();

	// doc. in glUniform*fvARB, count should be 1 if the targeted uniform variable is not an array, and 1 or more if it is an array.

	unsigned len;

	if ( JL_ARGC == 2 ) {

		JL_ASSERT_ARG_IS_ARRAY(2);
		GLfloat val[4]; // max for *4fv
//		JS::RootedObject arr = JSVAL_TO_OBJECT(JL_ARG(2));
		JL_CHK( jl::getVector(cx, JL_ARG(2), val, COUNTOF(val), &len) );
		JL_ASSERT_RANGE(len, (unsigned)0, (unsigned)4, "vec.length" );

		ASSERT( len >= 0 && len <= 4 );
		(len == 3 ? glUniform3fvARB : len == 4 ? glUniform4fvARB : len == 2 ? glUniform2fvARB : len == 1 ? glUniform1fvARB : NULL)(uniformLocation, 1, val);  OGL_ERR_CHK;
		return true;
	}


	ASSERT( JL_ARGC > 2 );

	unsigned firstLen = 0;
	int count = JL_ARGC - 1;
	value = (GLfloat*)jl_malloca(sizeof(GLfloat) * 4 * count); // allocate the max
	GLfloat *tmpVec = value;

	ASSERT( count >= 1 );

	IFDEBUG( len = 0 ); // avoid "potentially uninitialized local variable" warning

	for ( int i = 0; i < count; ++i ) {

		JL_CHK(jl::getVector(cx, JL_ARGV[i + 1], tmpVec, 4, &len));
		if ( firstLen == 0 )
			firstLen = len;
		ASSERT( len >= 0 && len <= 4 );

		JL_CHKM( len >= 0 && len <= 4, E_ARRAYLENGTH, E_RANGE, E_INTERVAL_NUM(0, 4) );
		//JL_ASSERT( len == firstLen, "Invalid variable vector length." );

		tmpVec += len;
	}

	ASSERT( len );

	(len == 3 ? glUniform3fvARB : len == 4 ? glUniform4fvARB : len == 2 ? glUniform2fvARB : len == 1 ? glUniform1fvARB : NULL)(uniformLocation, count, value);  OGL_ERR_CHK;
	jl_freea(value);

	return true;
bad:
	if (value)
		jl_freea(value);
	return false;
}



/**doc
$TOC_MEMBER $INAME
 $INT $INAME( uniformLocation, v1 [, v2 [, v3 [, v4]]] )
  (TBD)
 $H OpenGL API
  glUniform1fARB, glUniform2fARB, glUniform3fARB, glUniform4fARB
**/
DEFINE_FUNCTION( uniformFloat ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glUniform1fARB, PFNGLUNIFORM1FARBPROC );
	JL_INIT_OPENGL_EXTENSION( glUniform2fARB, PFNGLUNIFORM2FARBPROC );
	JL_INIT_OPENGL_EXTENSION( glUniform3fARB, PFNGLUNIFORM3FARBPROC );
	JL_INIT_OPENGL_EXTENSION( glUniform4fARB, PFNGLUNIFORM4FARBPROC );

	JL_ASSERT_ARGC_RANGE(2, 5);
	JL_ASSERT_ARG_IS_INTEGER(1);

	{
		GLint uniformLocation;
		uniformLocation = JL_ARG(1).toInt32();
		JL_RVAL.setUndefined();
		float v1, v2, v3, v4;

		if (JL_ARG(2).isNumber()) {

			JL_CHK(jl::getValue(cx, JL_ARG(2), &v1));
			if (JL_ARGC >= 3) {

				JL_CHK(jl::getValue(cx, JL_ARG(3), &v2));
				if (JL_ARGC >= 4) {

					JL_CHK(jl::getValue(cx, JL_ARG(4), &v3));
					if (JL_ARGC >= 5) {

						JL_CHK(jl::getValue(cx, JL_ARG(5), &v4));
						glUniform4fARB(uniformLocation, v1, v2, v3, v4);  OGL_ERR_CHK;
						return true;
					}
					glUniform3fARB(uniformLocation, v1, v2, v3);  OGL_ERR_CHK;
					return true;
				}
				glUniform2fARB(uniformLocation, v1, v2);  OGL_ERR_CHK;
				return true;
			}
			glUniform1fARB(uniformLocation, v1);  OGL_ERR_CHK;
			return true;
		}

		JL_ERR(E_ARG, E_INTERVAL_NUM(2, 5), E_TYPE, E_TY_NUMBER);
	}
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME( uniformLocation, v1 [, v2 [, v3 [, v4]]] )
  (TBD)
 $H OpenGL API
  glUniform1iARB, glUniform2iARB, glUniform3iARB, glUniform4iARB
**/
DEFINE_FUNCTION( uniformInteger ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glUniform1iARB, PFNGLUNIFORM1IARBPROC );
	JL_INIT_OPENGL_EXTENSION( glUniform2iARB, PFNGLUNIFORM2IARBPROC );
	JL_INIT_OPENGL_EXTENSION( glUniform3iARB, PFNGLUNIFORM3IARBPROC );
	JL_INIT_OPENGL_EXTENSION( glUniform4iARB, PFNGLUNIFORM4IARBPROC );

	JL_ASSERT_ARGC_RANGE(2, 5);
	JL_ASSERT_ARG_IS_INTEGER(1);

	{
		GLint uniformLocation;
		uniformLocation = JL_ARG(1).toInt32();
		JL_RVAL.setUndefined();
		int v1, v2, v3, v4;

		if (JL_ARG(2).isNumber()) {

			JL_CHK(jl::getValue(cx, JL_ARG(2), &v1));
			if (JL_ARGC >= 3) {

				JL_CHK(jl::getValue(cx, JL_ARG(3), &v2));
				if (JL_ARGC >= 4) {

					JL_CHK(jl::getValue(cx, JL_ARG(4), &v3));
					if (JL_ARGC >= 5) {

						JL_CHK(jl::getValue(cx, JL_ARG(5), &v4));
						glUniform4iARB(uniformLocation, v1, v2, v3, v4);  OGL_ERR_CHK;
						return true;
					}
					glUniform3iARB(uniformLocation, v1, v2, v3);  OGL_ERR_CHK;
					return true;
				}
				glUniform2iARB(uniformLocation, v1, v2);  OGL_ERR_CHK;
				return true;
			}
			glUniform1iARB(uniformLocation, v1);  OGL_ERR_CHK;
			return true;
		}

		JL_ERR(E_ARG, E_INTERVAL_NUM(2, 5), E_TYPE, E_TY_NUMBER);
	}
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME( obj, pname [, count] )
  (TBD)
 $H OpenGL API
  glGetObjectParameterfvARB, glGetObjectParameterivARB
**/
DEFINE_FUNCTION( getObjectParameter ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glGetObjectParameterfvARB, PFNGLGETOBJECTPARAMETERFVARBPROC );
	JL_INIT_OPENGL_EXTENSION( glGetObjectParameterivARB, PFNGLGETOBJECTPARAMETERIVARBPROC );

	JL_ASSERT_ARGC_RANGE(2,3);
	JL_ASSERT_ARG_IS_INTEGER(1);
	JL_ASSERT_ARG_IS_INTEGER(2);

	GLint params[MAX_PARAMS]; // (TBD) check if it is the max amount of data that glGetLightfv may returns.

	glGetObjectParameterivARB(JL_ARG(1).toInt32(), JL_ARG(2).toInt32(), params);  OGL_ERR_CHK;

	if ( JL_ARG_ISDEF(3) ) {

		JL_ASSERT_ARG_IS_INTEGER(3);
		int count = JL_ARG(3).toInt32();
		count = jl::min(count, (int)COUNTOF(params));

		JS::RootedObject arrayObj(cx, JS_NewArrayObject(cx, count));
		JL_CHK( arrayObj );
		JL_RVAL.setObject(*arrayObj);

		while (count--)
			jl::setElement(cx, arrayObj, count, params[count]);
	} else {

		JL_CHK(jl::setValue(cx, JL_RVAL, params[0]));
	}

	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME( programObj, index, name )
  (TBD)
 $H OpenGL API
  glBindAttribLocationARB
**/
DEFINE_FUNCTION( bindAttribLocation ) {

	jl::BufString name;
	JL_DEFINE_ARGS;

	OGL_CX_CHK;
	JL_INIT_OPENGL_EXTENSION( glBindAttribLocationARB, PFNGLBINDATTRIBLOCATIONARBPROC );

	JL_ASSERT_ARGC(3);
	JL_ASSERT_ARG_IS_INTEGER(1);
	JL_ASSERT_ARG_IS_INTEGER(2);

	JL_CHK( jl::getValue(cx, JL_ARG(3), &name) );
	glBindAttribLocationARB(JL_ARG(1).toInt32(), JL_ARG(2).toInt32(), name);  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME( programObj, name )
  (TBD)
 $H OpenGL API
  glGetAttribLocationARB
**/
DEFINE_FUNCTION( getAttribLocation ) {

	jl::BufString name;
	JL_DEFINE_ARGS;

	OGL_CX_CHK;
	JL_INIT_OPENGL_EXTENSION( glGetAttribLocationARB, PFNGLGETATTRIBLOCATIONARBPROC );

	JL_ASSERT_ARGC(2);
	JL_ASSERT_ARG_IS_INTEGER(1);

	JL_CHK( jl::getValue(cx, JL_ARG(2), &name) );
	int location;
	location = glGetAttribLocationARB(JL_ARG(1).toInt32(), name);  OGL_ERR_CHK;
	JL_RVAL.setInt32(location);

	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME( index, value )
  (TBD)
 $H OpenGL API
  glVertexAttrib1dARB
**/
DEFINE_FUNCTION( vertexAttrib ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glVertexAttrib1sARB, PFNGLVERTEXATTRIB1SARBPROC );

	JL_INIT_OPENGL_EXTENSION( glVertexAttrib1dARB, PFNGLVERTEXATTRIB1DARBPROC );
	JL_INIT_OPENGL_EXTENSION( glVertexAttrib2dARB, PFNGLVERTEXATTRIB2DARBPROC );
	JL_INIT_OPENGL_EXTENSION( glVertexAttrib3dARB, PFNGLVERTEXATTRIB3DARBPROC );
	JL_INIT_OPENGL_EXTENSION( glVertexAttrib4dARB, PFNGLVERTEXATTRIB4DARBPROC );

	JL_ASSERT_ARGC_RANGE(2, 5);
	JL_ASSERT_ARG_IS_INTEGER(1);

	int index;
	index = JL_ARG(1).toInt32();


	JL_RVAL.setUndefined();

	GLdouble v1, v2, v3, v4;

	if (JL_ARGC == 2 && JL_ARG(2).isInt32()) {

		glVertexAttrib1sARB(index, (GLshort)JL_ARG(2).toInt32());  OGL_ERR_CHK;
		return true;
	}

	if (JL_ARG(2).isNumber()) {

		JL_CHK(jl::getValue(cx, JL_ARG(2), &v1));
		if ( JL_ARGC >= 3 ) {

			JL_CHK( jl::getValue(cx, JL_ARG(3), &v2) );
			if ( JL_ARGC >= 4 ) {

				JL_CHK( jl::getValue(cx, JL_ARG(4), &v3) );
				if ( JL_ARGC >= 5 ) {

					JL_CHK( jl::getValue(cx, JL_ARG(5), &v4) );
					glVertexAttrib4dARB(index, v1, v2, v3, v4);  OGL_ERR_CHK;
					return true;
				}
				glVertexAttrib3dARB(index, v1, v2, v3);  OGL_ERR_CHK;
				return true;
			}
			glVertexAttrib2dARB(index, v1, v2);  OGL_ERR_CHK;
			return true;
		}
		glVertexAttrib1dARB(index, v1);  OGL_ERR_CHK;
		return true;
	}

	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME()
  Returns a new buffer.
  $H OpenGL API
   glGenBuffers
**/
DEFINE_FUNCTION( genBuffer ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glGenBuffers, PFNGLGENBUFFERSPROC );

	JL_ASSERT_ARGC(0);

	GLuint buffer;

	glGenBuffers(1, &buffer);  OGL_ERR_CHK;

	JL_RVAL.setInt32(buffer);

	return true;
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
DEFINE_FUNCTION( bindBuffer ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glBindBuffer, PFNGLBINDBUFFERPROC );

	JL_ASSERT_ARGC(2);
	JL_ASSERT_ARG_IS_INTEGER(1);
	JL_ASSERT_ARG_IS_INTEGER(2);

	glBindBuffer(JL_ARG(1).toInt32(), JL_ARG(2).toInt32());  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( bufferData ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	LOAD_OPENGL_EXTENSION( glBufferDataARB, PFNGLBUFFERDATAARBPROC ); // glBufferDataARB (GLenum target, GLsizeiptrARB size, const GLvoid *data, GLenum usage);

	// see http://www.songho.ca/opengl/gl_pbo.html

	JL_ASSERT_ARGC_MIN(2);
	JL_ASSERT_ARG_IS_INTEGER(1);
	JL_ASSERT_ARG_IS_INTEGER(2);

	GLenum target = JL_ARG(1).toInt32();
	GLenum buffer = JL_ARG(2).toInt32();

	glBufferDataARB(target, buffer);  OGL_ERR_CHK;
	JL_RVAL.setUndefined();
	;
	return true;
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
DEFINE_FUNCTION( pointParameter ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glPointParameteri, PFNGLPOINTPARAMETERIPROC );
	JL_INIT_OPENGL_EXTENSION( glPointParameterf, PFNGLPOINTPARAMETERFPROC );
	JL_INIT_OPENGL_EXTENSION( glPointParameterfv, PFNGLPOINTPARAMETERFVPROC );

	JL_ASSERT_ARGC(2);
	JL_ASSERT_ARG_IS_INTEGER(1);

	JL_RVAL.setUndefined();
	if (JL_ARG(2).isInt32()) {

		glPointParameteri(JL_ARG(1).toInt32(), JL_ARG(2).toInt32());  OGL_ERR_CHK;
		;
		return true;
	}
	if ( JL_ARG(2).isDouble() ) {

		float param;
		jl::getValue(cx, JL_ARG(2), &param);

		glPointParameterf( JL_ARG(1).toInt32(), param );  OGL_ERR_CHK;

		;
		return true;
	}
	if ( jl::isArray(cx, JL_ARG(2)) ) {  // (TBD) check if array-like make sense here.

		GLfloat params[MAX_PARAMS];
		uint32_t length;
		JL_CHK( jl::getVector(cx, JL_ARG(2), params, COUNTOF(params), &length ) );
		glPointParameterfv( JL_ARG(1).toInt32(), params );  OGL_ERR_CHK;
		;
		return true;
	}

	JL_ERR( E_ARG, E_NUM(2), E_TYPE, E_TY_NUMBER, E_OR, E_TY_ARRAY );
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( texture )
  Select server-side active texture unit.
  $H arguments
   $ARG GLenum texture
  $H OpenGL API
   glActiveTexture
**/
DEFINE_FUNCTION( activeTexture ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glActiveTexture, PFNGLACTIVETEXTUREPROC );

	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	glActiveTexture(JL_ARG(1).toInt32());  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( clientActiveTexture ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glClientActiveTexture, PFNGLCLIENTACTIVETEXTUREPROC );

	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	glClientActiveTexture(JL_ARG(1).toInt32());  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_FUNCTION( multiTexCoord ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glMultiTexCoord1d, PFNGLMULTITEXCOORD1DPROC );
	JL_INIT_OPENGL_EXTENSION( glMultiTexCoord2d, PFNGLMULTITEXCOORD2DPROC );
	JL_INIT_OPENGL_EXTENSION( glMultiTexCoord3d, PFNGLMULTITEXCOORD3DPROC );

	JL_ASSERT_ARGC_RANGE(2,4);
	JL_ASSERT_ARG_IS_INTEGER(1);

	JL_RVAL.setUndefined();

	GLenum target = JL_ARG(1).toInt32();

	double s;
	JL_CHK( jl::getValue(cx, JL_ARG(2), &s) );
	if ( JL_ARGC == 2 ) {

		glMultiTexCoord1d(target, s);  OGL_ERR_CHK;

		return true;
	}
	double t;
	JL_CHK( jl::getValue(cx, JL_ARG(3), &t) );
	if ( JL_ARGC == 3 ) {

		glMultiTexCoord2d(target, s, t);  OGL_ERR_CHK;

		return true;
	}
	double r;
	JL_CHK( jl::getValue(cx, JL_ARG(4), &r) );
	if ( JL_ARGC == 4 ) {

		glMultiTexCoord3d(target, s, t, r);  OGL_ERR_CHK;

		return true;
	}

	ASSERT(false);
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
DEFINE_FUNCTION( createPbuffer ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;


	;
	return true;
	JL_BAD;
}
*/




/**doc
$TOC_MEMBER $INAME
 $INT $INAME()
  Create a new query object.
  $H OpenGL API
   glGenQueriesARB
**/
DEFINE_FUNCTION( genQueries ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glGenQueriesARB, PFNGLGENQUERIESARBPROC );

	JL_ASSERT_ARGC(0);

	GLuint query;

	glGenQueriesARB(1, &query);  OGL_ERR_CHK;

	JL_RVAL.setInt32(query);
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( query )
  Deletes the given query object.
  $H arguments
   $ARG $INT query object id
  $H OpenGL API
   glDeleteQueriesARB
**/
DEFINE_FUNCTION( deleteQueries ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glDeleteQueriesARB, PFNGLDELETEQUERIESARBPROC );

	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	GLuint query = JL_ARG(1).toInt32();

	glDeleteQueriesARB(1, &query);  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( target, query )
  Delimit the boundaries of a query object.
  $H arguments
   $ARG $INT target Specifies the target type of query object established between BeginQuery and the subsequent EndQuery. The symbolic constant must be SAMPLES_PASSED.
   $ARG $INT query Specifies the name of a query object.
  $H OpenGL API
   glBeginQueryARB
**/
DEFINE_FUNCTION( beginQuery ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glBeginQueryARB, PFNGLBEGINQUERYARBPROC );

	JL_ASSERT_ARGC(2);
	JL_ASSERT_ARG_IS_INTEGER(1);
	JL_ASSERT_ARG_IS_INTEGER(2);

	glBeginQueryARB(JL_ARG(1).toInt32(), JL_ARG(2).toInt32());  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( target )
  Delimit the boundaries of a query object.
  $H arguments
   $ARG $INT target Specifies the target type of query object to be concluded. The symbolic constant must be SAMPLES_PASSED.
  $H OpenGL API
   glEndQueryARB
**/
DEFINE_FUNCTION( endQuery ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glEndQueryARB, PFNGLENDQUERYARBPROC );

	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	glEndQueryARB(JL_ARG(1).toInt32());  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( target, pname )
  Return parameters of a query object target
  $H arguments
   $ARG $INT target Specifies a query object target. Must be SAMPLES_PASSED.
	$ARG $INT pname Specifies the symbolic name of a query object target parameter. Accepted values are GL_CURRENT_QUERY or GL_QUERY_COUNTER_BITS.
  $H OpenGL API
   glGetQueryivARB
**/
DEFINE_FUNCTION( getQuery ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glGetQueryivARB, PFNGLGETQUERYIVARBPROC );

	JL_ASSERT_ARGC(2);
	JL_ASSERT_ARG_IS_INTEGER(1);
	JL_ASSERT_ARG_IS_INTEGER(2);

	// http://www.opengl.org/sdk/docs/man/xhtml/glGetQueryiv.xml

	GLint params;
	glGetQueryivARB(JL_ARG(1).toInt32(), JL_ARG(2).toInt32(), &params );  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( id, pname )
  Return parameters of a query object target
  $H arguments
   $ARG $INT id Specifies the name of a query object.
	$ARG $INT pname Specifies the symbolic name of a query object parameter. Accepted values are QUERY_RESULT or QUERY_RESULT_AVAILABLE.
  $H OpenGL API
   glGetQueryObjectARB, glGetQueryObjectuivARB
**/
DEFINE_FUNCTION( getQueryObject ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glGetQueryObjectivARB, PFNGLGETQUERYOBJECTIVARBPROC );
	JL_INIT_OPENGL_EXTENSION( glGetQueryObjectuivARB, PFNGLGETQUERYOBJECTUIVARBPROC );

	JL_ASSERT_ARGC(2);
	JL_ASSERT_ARG_IS_INTEGER(1);
	JL_ASSERT_ARG_IS_INTEGER(2);

	// gl doc. http://www.opengl.org/sdk/docs/man/xhtml/glGetQueryObject.xml
	// ext. doc. http://oss.sgi.com/projects/ogl-sample/registry/ARB/occlusion_query.txt

	GLenum pname = JL_ARG(2).toInt32();

	if ( pname == GL_QUERY_RESULT_AVAILABLE_ARB ) {

		GLint param;
		glGetQueryObjectivARB(JL_ARG(1).toInt32(), pname, &param);  OGL_ERR_CHK;
		JL_RVAL.setInt32(param);
	} else { // GL_QUERY_RESULT_ARB

		GLuint param;
		glGetQueryObjectuivARB(JL_ARG(1).toInt32(), pname, &param);  OGL_ERR_CHK;
		JL_CHK(jl::setValue(cx, JL_RVAL, param));
	}

	return true;
	JL_BAD;
}



///////////////////////////////////////////////////////////////////////////////
// non-OpenGL API


/**doc
$TOC_MEMBER $INAME
 $TYPE vec3 $INAME( x, y )
  $H API
   gluLookAt
**/
DEFINE_FUNCTION( unProject ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(2);
	JL_ASSERT_ARG_IS_INTEGER(1);
	JL_ASSERT_ARG_IS_INTEGER(2);

	int x, y;
	x = JL_ARG(1).toInt32();
	y = JL_ARG(2).toInt32();

   GLint viewport[4];
   GLdouble mvmatrix[16], projmatrix[16];
   GLint realy;
   GLdouble w[3];

	glGetIntegerv(GL_VIEWPORT, viewport);  OGL_ERR_CHK;
   glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix);  OGL_ERR_CHK;
   glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);  OGL_ERR_CHK;
	realy = viewport[3] - (GLint) y - 1;

	gluUnProject((GLdouble) x, (GLdouble) realy, 0.0, mvmatrix, projmatrix, viewport, w+0, w+1, w+2);

	JL_CHK( jl::setVector(cx, JL_RVAL, w, 3) );

	return true;
	JL_BAD;
}




/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( texture [, internalformat] )
  $H OpenGL API
   glDrawPixels
**/
DEFINE_FUNCTION( drawImage ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;
	JL_ASSERT_ARGC_RANGE(1,2);
	JL_ASSERT_ARG_IS_OBJECT(1);

	{

		GLsizei width, height;
		GLenum format, type;
		int channels;
		const GLvoid *data;

		JS::RootedObject tObj(cx, &JL_ARG(1).toObject());

		if (JL_GetClass(tObj) == JL_TextureJSClass(cx)) {

			TextureStruct *tex = (TextureStruct*)JL_GetPrivate(tObj);
			JL_ASSERT_OBJECT_STATE(tex, JL_GetClassName(tObj));

			data = tex->cbuffer;
			width = tex->width;
			height = tex->height;
			channels = tex->channels;
			type = GL_FLOAT;
		}
		else {

			ImageDataType dataType;
			jl::BufString image(JL_GetImageObject(cx, JL_ARG(1), &width, &height, &channels, &dataType));
			JL_ASSERT(image.hasData(), E_ARG, E_NUM(1), E_INVALID);
			switch (dataType) {
			case TYPE_INT8:
				type = GL_BYTE;
				break;
			case TYPE_UINT8:
				type = GL_UNSIGNED_BYTE;
				break;
			case TYPE_INT16:
				type = GL_SHORT;
				break;
			case TYPE_UINT16:
				type = GL_UNSIGNED_SHORT;
				break;
			case TYPE_INT32:
				type = GL_INT;
				break;
			case TYPE_UINT32:
				type = GL_UNSIGNED_INT;
				break;
			case TYPE_FLOAT32:
				type = GL_FLOAT;
				break;
			case TYPE_FLOAT64:
				type = GL_DOUBLE;
				break;
			default:
				JL_ERR(E_ARG, E_NUM(1), E_FORMAT);
			}
			data = image.toData<const uint8_t*>();
		}

		if (JL_ARG_ISDEF(2)) {

			JL_ASSERT_ARG_IS_INTEGER(2);
			format = JL_ARG(2).toInt32();
		}
		else { // guess

			//ASSERT( format == GL_LUMINANCE || format == GL_LUMINANCE_ALPHA || format == GL_RGB || format == GL_RGBA );

			switch (channels) {
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
				JL_ERR(E_PARAM, E_STR("channels"), E_RANGE, E_INTERVAL_NUM(1, 4));
			}
		}

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  OGL_ERR_CHK;
		glDrawPixels(width, height, format, type, data);  OGL_ERR_CHK;

		JL_RVAL.setUndefined();
	}
	return true;
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
DEFINE_FUNCTION( readImage ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC_RANGE(0,2);

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);  OGL_ERR_CHK;
	int x = viewport[0];
	int y = viewport[1];
	int width = viewport[2];
	int height = viewport[3];

	bool flipY;
	if ( JL_ARG_ISDEF(1) )
		JL_CHK( jl::getValue(cx, JL_ARG(1), &flipY) );
	else
		flipY = false;

	int channels;
	GLenum format;
	if ( JL_ARG_ISDEF(2) ) {

		format = JL_ARG(2).toInt32();
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
				JL_ERR( E_ARG, E_NUM(2), E_NOTSUPPORTED );
		}
	} else {

		format = GL_RGBA;
		channels = 4;  // 4 for RGBA
	}

	int lineLength = width * channels;
	int length = lineLength * height;
	ASSERT( length > 0 ); //, "Invalid image size." );
	uint8_t *data = JL_NewImageObject(cx, width, height, channels, TYPE_UINT8, JL_RVAL);
	JL_CHK( data );

/*
	GLuint texture;
	glGenTextures(1, &texture);  OGL_ERR_CHK;
	glBindTexture(GL_TEXTURE_2D, texture);  OGL_ERR_CHK;

	// see GL_ARB_texture_rectangle / ARB_texture_non_power_of_two

	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, width, height, 0);  OGL_ERR_CHK;

	GLint tWidth, tHeight;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &tWidth);  OGL_ERR_CHK;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &tHeight);  OGL_ERR_CHK;
	//glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPONENTS, &tComponents);  OGL_ERR_CHK;
	//  glGet	with arguments GL_PACK_ALIGNMENT and others

	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);  OGL_ERR_CHK;
	glDeleteTextures(1, &texture);  OGL_ERR_CHK;
*/

	glReadPixels(x, y, width, height, format, GL_UNSIGNED_BYTE, data);  OGL_ERR_CHK;

	if ( flipY ) { // Y-flip image

		void *tmp = alloca(lineLength);
		int mid = height / 2;
		for ( int line = 0; line < mid; ++line ) {

			jl::memcpy(tmp, data + (line*lineLength), lineLength);
			jl::memcpy(data + (line*lineLength), data + ((height-1-line)*lineLength), lineLength);
			jl::memcpy(data + ((height-1-line)*lineLength), tmp, lineLength);
		}
	}

	return true;
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

struct OpenGlTrimeshInfo : public HandlePrivate {

	GLuint indexBuffer, vertexBuffer, normalBuffer, texCoordBuffer, colorBuffer;
	size_t vertexCount, indexCount;

	JL_HANDLE_TYPE typeId() const {
		
		return TRIMESH_ID_NAME;
	}

	~OpenGlTrimeshInfo() {

		/* (TBD)!

		static PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC) GL_GET_PROC_ADDRESS( "PFNGLDELETEBUFFERSARBPROC" );

		OpenGlTrimeshInfo *info = (OpenGlTrimeshInfo*)pv;
		glDeleteBuffersARB( 1, &info->indexBuffer );  OGL_ERR_CHK;
		glDeleteBuffersARB( 1, &info->vertexBuffer );  OGL_ERR_CHK;
		if ( info->indexBuffer )
		glDeleteBuffersARB( 1, &info->indexBuffer );  OGL_ERR_CHK;
		if ( info->texCoordBuffer )
		glDeleteBuffersARB( 1, &info->texCoordBuffer );  OGL_ERR_CHK;
		if ( info->colorBuffer )
		glDeleteBuffersARB( 1, &info->colorBuffer );  OGL_ERR_CHK;
		*/
	}
};


/**doc
$TOC_MEMBER $INAME
 $TYPE trimeshId $INAME( trimesh )
**/
DEFINE_FUNCTION( loadTrimesh ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glGenBuffers, PFNGLGENBUFFERSPROC );
	JL_INIT_OPENGL_EXTENSION( glBindBuffer, PFNGLBINDBUFFERPROC );
	JL_INIT_OPENGL_EXTENSION( glBufferData, PFNGLBUFFERDATAPROC );

	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_OBJECT(1);

	{
		JS::RootedObject trimeshObj(cx, &JL_ARG(1).toObject());

		JL_ASSERT(JL_JsvalIsTrimesh(cx, JL_ARG(1)), E_ARG, E_NUM(1), E_TYPE, E_STR("Trimesh"));

		Surface *srf = GetTrimeshSurface(cx, trimeshObj);
		JL_ASSERT_OBJECT_STATE(srf, JL_GetClassName(trimeshObj));

		OpenGlTrimeshInfo *info = new OpenGlTrimeshInfo;
		//JL_CHK(HandleCreate(cx, TRIMESH_ID_NAME, &info, FinalizeTrimesh, JL_RVAL));
		JL_CHK( HandleCreate(cx, info, JL_RVAL) );

		if (srf->vertex) {

			info->vertexCount = srf->vertexCount;
			glGenBuffers(1, &info->vertexBuffer);  OGL_ERR_CHK;
			glBindBuffer(GL_ARRAY_BUFFER, info->vertexBuffer);  OGL_ERR_CHK;
			glBufferData(GL_ARRAY_BUFFER, srf->vertexCount * 3 * sizeof(SURFACE_REAL_TYPE), srf->vertex, GL_STATIC_DRAW);  OGL_ERR_CHK;
		}
		else
			info->vertexBuffer = 0;

		if (srf->index) {

			info->indexCount = srf->indexCount;
			glGenBuffers(1, &info->indexBuffer);  OGL_ERR_CHK;
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, info->indexBuffer);  OGL_ERR_CHK;
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, srf->indexCount * sizeof(SURFACE_INDEX_TYPE), srf->index, GL_STATIC_DRAW);  OGL_ERR_CHK;
		}
		else
			info->indexBuffer = 0;

		if (srf->normal) {

			glGenBuffers(1, &info->normalBuffer);  OGL_ERR_CHK;
			glBindBuffer(GL_ARRAY_BUFFER, info->normalBuffer);  OGL_ERR_CHK;
			glBufferData(GL_ARRAY_BUFFER, srf->vertexCount * 3 * sizeof(SURFACE_REAL_TYPE), srf->normal, GL_STATIC_DRAW);  OGL_ERR_CHK;
		}
		else
			info->normalBuffer = 0;

		if (srf->textureCoordinate) {

			glGenBuffers(1, &info->texCoordBuffer);  OGL_ERR_CHK;
			glBindBuffer(GL_ARRAY_BUFFER, info->texCoordBuffer);  OGL_ERR_CHK;
			glBufferData(GL_ARRAY_BUFFER, srf->vertexCount * 3 * sizeof(SURFACE_REAL_TYPE), srf->textureCoordinate, GL_STATIC_DRAW);  OGL_ERR_CHK;
		}
		else
			info->texCoordBuffer = 0;

		if (srf->color) {

			glGenBuffers(1, &info->colorBuffer);  OGL_ERR_CHK;
			glBindBuffer(GL_ARRAY_BUFFER, info->colorBuffer);  OGL_ERR_CHK;
			glBufferData(GL_ARRAY_BUFFER, srf->vertexCount * 4 * sizeof(SURFACE_REAL_TYPE), srf->color, GL_STATIC_DRAW);  OGL_ERR_CHK;
		}
		else
			info->colorBuffer = 0;

		JL_CHK(CheckThrowCurrentOglError(cx));
	}
	return true;
	JL_BAD;
}




/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( trimeshId [, mode ] )
  $H OpenGL API
   glVertexPointer
**/
DEFINE_FUNCTION( drawTrimesh ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_INIT_OPENGL_EXTENSION( glBindBuffer, PFNGLBINDBUFFERPROC );

	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT_ARG_IS_OBJECT(1);
	JL_ASSERT( IsHandleType(cx, JL_ARG(1), TRIMESH_ID_NAME), E_ARG, E_NUM(1), E_TYPE, E_STR("GlTr Handle") );

	//OpenGlTrimeshInfo *info = (OpenGlTrimeshInfo*)GetHandlePrivate(cx, JL_ARG(1));

	OpenGlTrimeshInfo *info;
	JL_CHK( GetHandlePrivate(cx, JL_ARG(1), info) );

	GLenum dataType = sizeof(SURFACE_REAL_TYPE) == sizeof(float) ? GL_FLOAT : GL_DOUBLE;

	GLenum mode;
	if ( JL_ARG_ISDEF(2) )
		mode = JL_ARG(2).toInt32();
	else
		mode = GL_TRIANGLES;

	if ( info->vertexBuffer ) {

		glEnableClientState(GL_VERTEX_ARRAY);  OGL_ERR_CHK;
		glBindBuffer(GL_ARRAY_BUFFER, info->vertexBuffer);  OGL_ERR_CHK;
		glVertexPointer(3, dataType, 0, 0);  OGL_ERR_CHK;
	}

	if ( info->normalBuffer ) {

		glEnableClientState(GL_NORMAL_ARRAY);  OGL_ERR_CHK;
		glBindBuffer(GL_ARRAY_BUFFER, info->normalBuffer);  OGL_ERR_CHK;
		glNormalPointer(dataType, 0, 0);  OGL_ERR_CHK;
	}

	if ( info->texCoordBuffer ) {

		glEnableClientState(GL_TEXTURE_COORD_ARRAY);  OGL_ERR_CHK;
		glBindBuffer(GL_ARRAY_BUFFER, info->texCoordBuffer);  OGL_ERR_CHK;
		glTexCoordPointer(3, dataType, 0, 0);  OGL_ERR_CHK;
	}

	if ( info->colorBuffer ) {

		glEnableClientState(GL_COLOR_ARRAY);  OGL_ERR_CHK;
		glBindBuffer(GL_ARRAY_BUFFER, info->colorBuffer);  OGL_ERR_CHK;
		glColorPointer(4, dataType, 0, 0);  OGL_ERR_CHK;
	}

	if ( info->indexBuffer ) {

		//	glEnableClientState(GL_INDEX_ARRAY);  OGL_ERR_CHK;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, info->indexBuffer);  OGL_ERR_CHK;
		glDrawElements(mode, (GLsizei)info->indexCount, GL_UNSIGNED_INT, 0);  OGL_ERR_CHK; // 1 triangle = 3 vertex
	} else {

		if ( info->vertexBuffer )
			glDrawArrays(mode, 0, (GLsizei)info->vertexCount);  OGL_ERR_CHK;
	}

//	glDisableClientState(GL_INDEX_ARRAY);  OGL_ERR_CHK;
	if ( info->colorBuffer )
		glDisableClientState(GL_COLOR_ARRAY);  OGL_ERR_CHK;
	if ( info->texCoordBuffer )
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);  OGL_ERR_CHK;
	if ( info->normalBuffer )
		glDisableClientState(GL_NORMAL_ARRAY);  OGL_ERR_CHK;

	glDisableClientState(GL_VERTEX_ARRAY);  OGL_ERR_CHK; // deactivate vertex array

	// bind with 0, so, switch back to normal pointer operation
//	glBindBuffer(GL_ARRAY_BUFFER, 0);  OGL_ERR_CHK;
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);  OGL_ERR_CHK;

	JL_CHK( CheckThrowCurrentOglError(cx) );

	JL_RVAL.setUndefined();
	return true;
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

/*
void TextureBufferFinalize(void* data) {

	TextureBuffer *tb = (TextureBuffer*)data;
	TextureBufferFree(tb);
}
*/

struct TextureBufferHandle : public HandlePrivate {

	TextureBuffer textureBuffer;
	GLuint pbo;

	JL_HANDLE_TYPE typeId() const {
	
		return JLHID(TBUF);
	}


	TextureBufferHandle() {

		glGenBuffers(1, &pbo); // OGL_ERR_CHK;
		ASSERT(glGetError() == GL_NO_ERROR);
		// TextureBufferAlloc ?
	}

	~TextureBufferHandle() {

		if (!glUnmapBuffer)
			return;

		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
		ASSERT(glGetError() == GL_NO_ERROR);

		glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
		ASSERT(glGetError() == GL_NO_ERROR);
	}
};

// OpenGL Pixel Buffer Object: http://www.songho.ca/opengl/gl_pbo.html


DEFINE_FUNCTION( createTextureBuffer ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_IGNORE(argc);

	JL_INIT_OPENGL_EXTENSION( glGenBuffers, PFNGLGENBUFFERSPROC );
//	JL_INIT_OPENGL_EXTENSION( glBindBuffer, PFNGLBINDBUFFERPROC );

	TextureBufferHandle *tb = new TextureBufferHandle;

	//JL_CHK( HandleCreate(cx, JLHID(TBUF), &tb, TextureBufferFinalize, JL_RVAL) );
	JL_CHK(HandleCreate(cx, tb, JL_RVAL));

	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( target, internalformat | $UNDEF, texture )
 $VOID $INAME( target, internalformat | $UNDEF, genericImage )
 $VOID $INAME( target, internalformat | $UNDEF, typedArray, width, height, channels )
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
DEFINE_FUNCTION( defineTextureImage ) {

	jl::BufString dataStr;
	JL_DEFINE_ARGS;

	OGL_CX_CHK;
	JL_ASSERT_ARGC_MIN(3);
	JL_ASSERT_ARG_IS_INTEGER(1);
//	JL_ASSERT_ARG_IS_INTEGER(2); // may be undefined
	JL_ASSERT_ARG_IS_OBJECT(3);

	GLsizei width, height;
	GLenum format, type;
	int channels;
	const GLvoid *data;

	{

		JS::RootedObject tObj(cx, &JL_ARG(3).toObject());

		if (JL_GetClass(tObj) == JL_TextureJSClass(cx)) {

			TextureStruct *tex = (TextureStruct*)JL_GetPrivate(tObj);
			JL_ASSERT_OBJECT_STATE(tex, JL_GetClassName(tObj));

			data = tex->cbuffer;
			width = tex->width;
			height = tex->height;
			channels = tex->channels;
			type = GL_FLOAT;
		}
		else if (JS_IsTypedArrayObject(tObj)) {

			JL_ASSERT_ARGC(6);

			switch (JS_GetArrayBufferViewType(tObj)) {
			case js::ArrayBufferView::TYPE_INT8:
				type = GL_BYTE;
				data = JS_GetInt8ArrayData(tObj);
				break;
			case js::ArrayBufferView::TYPE_UINT8:
				type = GL_UNSIGNED_BYTE;
				data = JS_GetUint8ArrayData(tObj);
				break;
			case js::ArrayBufferView::TYPE_INT16:
				type = GL_SHORT;
				data = JS_GetInt16ArrayData(tObj);
				break;
			case js::ArrayBufferView::TYPE_UINT16:
				type = GL_UNSIGNED_SHORT;
				data = JS_GetUint16ArrayData(tObj);
				break;
			case js::ArrayBufferView::TYPE_INT32:
				type = GL_INT;
				data = JS_GetInt32ArrayData(tObj);
				break;
			case js::ArrayBufferView::TYPE_UINT32:
				type = GL_UNSIGNED_INT;
				data = JS_GetUint32ArrayData(tObj);
				break;
			case js::ArrayBufferView::TYPE_FLOAT32:
				type = GL_FLOAT;
				data = JS_GetFloat32ArrayData(tObj);
				break;
			case js::ArrayBufferView::TYPE_FLOAT64:
				type = GL_DOUBLE;
				data = JS_GetFloat64ArrayData(tObj);
				break;
			default:
				JL_ERR(E_ARG, E_NUM(3), E_NOTSUPPORTED, E_COMMENT("ArrayBufferView.type"));
			}

			JL_CHK(jl::getValue(cx, JL_ARG(4), &width));
			JL_CHK(jl::getValue(cx, JL_ARG(5), &height));
			JL_CHK(jl::getValue(cx, JL_ARG(6), &channels));

			JL_ASSERT(width * height * channels == (int)JS_GetTypedArrayByteLength(tObj), E_DATASIZE, E_INVALID);
		}
		else {

			ImageDataType dataType;
			jl::BufString jlImage = JL_GetImageObject(cx, JL_ARG(3), &width, &height, &channels, &dataType);
			JL_ASSERT(jlImage.hasData(), E_ARG, E_NUM(3), E_INVALID);
			switch (dataType) {
			case TYPE_INT8:
				type = GL_BYTE;
				break;
			case TYPE_UINT8:
				type = GL_UNSIGNED_BYTE;
				break;
			case TYPE_INT16:
				type = GL_SHORT;
				break;
			case TYPE_UINT16:
				type = GL_UNSIGNED_SHORT;
				break;
			case TYPE_INT32:
				type = GL_INT;
				break;
			case TYPE_UINT32:
				type = GL_UNSIGNED_INT;
				break;
			case TYPE_FLOAT32:
				type = GL_FLOAT;
				break;
			case TYPE_FLOAT64:
				type = GL_DOUBLE;
				break;
			default:
				JL_ERR(E_ARG, E_NUM(3), E_FORMAT);
			}
			data = jlImage.toData<const uint8_t*>();
		}


		if (JL_ARG_ISDEF(2)) {

			JL_ASSERT_ARG_IS_INTEGER(2);
			format = JL_ARG(2).toInt32();
		}
		else { // guess

			switch (channels) {
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
				JL_ERR(E_PARAM, E_STR("channels"), E_RANGE, E_INTERVAL_NUM(1, 4));
				// JL_REPORT_ERROR("Invalid texture format."); // miss GL_COLOR_INDEX, GL_STENCIL_INDEX, GL_DEPTH_COMPONENT, GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA
			}
		}

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  OGL_ERR_CHK;

		glTexImage2D(JL_ARG(1).toInt32(), 0, format, width, height, 0, format, type, data);  OGL_ERR_CHK;

		JL_RVAL.setUndefined();

	}

	return true;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $REAL $INAME()
  pixelWidth = PixelWidthFactor() * width / distance
**/
DEFINE_FUNCTION( pixelWidthFactor ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	// see. http://www.songho.ca/opengl/gl_projectionmatrix.html
	// see. engine_core.h

	JL_ASSERT_ARGC(0);

	GLint viewport[4];
	GLfloat m[16];
	glGetIntegerv(GL_VIEWPORT, viewport);  OGL_ERR_CHK;
	glGetFloatv(GL_PROJECTION_MATRIX, m);  OGL_ERR_CHK;

	float w;
	w = viewport[2] * m[0];
//	float h;
//	h = viewport[3] * m[5];

	return jl::setValue(cx, JL_RVAL, w); // sqrt(w*w+h*h)
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME( size )
**/
DEFINE_FUNCTION( drawPoint ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(1);

	float size;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &size) );
	glPointSize(size);  OGL_ERR_CHK; // get max with GL_POINT_SIZE_RANGE
	glBegin(GL_POINTS);
	glVertex2i(0,0);
	glEnd();  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}

/*
/ **doc
$TOC_MEMBER $INAME
$INAME( radius [ , vertexCount = 12 ] )
** /
DEFINE_FUNCTION( drawDisk ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	float s, c, angle, radius;
	int vertexCount;
	JL_ASSERT_ARGC_RANGE(1,2);

	JL_CHK( jl::getValue(cx, JL_ARG(1), &radius) );
	if ( JL_ARG_ISDEF(2) )
		JL_CHK( jl::getValue(cx, JL_ARG(2), &vertexCount) );
	else
		vertexCount = 12;
	angle = 2*M_PI / vertexCount;
	glBegin(GL_POLYGON);  OGL_ERR_CHK;
	for (int i = 0; i < vertexCount; i++) {

		SinCos(i * angle, &s, &c);
		glTexCoord2f(c / 2.f + 0.5f, s / 2.f + 0.5f);  OGL_ERR_CHK;
		glVertex2f(c * radius, s * radius);  OGL_ERR_CHK;
	}
	glEnd();  OGL_ERR_CHK;
	;
	return true;
	JL_BAD;
}
*/


/**doc
$TOC_MEMBER $INAME
$INAME( radius, slices, stacks, smooth );
**/
DEFINE_FUNCTION( drawSphere ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(4);

	double radius;
	int slices, stacks;
	bool smooth;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &radius) );
	JL_CHK( jl::getValue(cx, JL_ARG(2), &slices) );
	JL_CHK( jl::getValue(cx, JL_ARG(3), &stacks) );
	JL_CHK( jl::getValue(cx, JL_ARG(4), &smooth) );

	GLUquadric *q = gluNewQuadric();
	gluQuadricTexture(q, GL_FALSE);
	gluQuadricNormals(q, smooth ? GLU_SMOOTH : GLU_FLAT);
	gluSphere(q, radius, slices, stacks);  OGL_ERR_CHK;
	gluDeleteQuadric(q);

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
$INAME( radius, slices, loops );
**/
DEFINE_FUNCTION( drawDisk ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(3);

	double radius;
	int slices, loops;

	JL_CHK( jl::getValue(cx, JL_ARG(1), &radius) );
	JL_CHK( jl::getValue(cx, JL_ARG(2), &slices) );
	JL_CHK( jl::getValue(cx, JL_ARG(3), &loops) );

	GLUquadric *q = gluNewQuadric();
	gluQuadricTexture(q, GL_FALSE);
	gluQuadricNormals(q, GLU_FLAT); // GLU_FLAT / GLU_SMOOTH
	gluDisk(q, 0, radius, slices, loops);  OGL_ERR_CHK;
	gluDeleteQuadric(q);

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
$INAME( baseRadius, topRadius, height, slices, stacks, smooth );
**/
DEFINE_FUNCTION( drawCylinder ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(6);

	double baseRadius, topRadius, height;
	int slices, stacks;
	bool smooth;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &baseRadius) );
	JL_CHK( jl::getValue(cx, JL_ARG(2), &topRadius) );
	JL_CHK( jl::getValue(cx, JL_ARG(3), &height) );
	JL_CHK( jl::getValue(cx, JL_ARG(4), &slices) );
	JL_CHK( jl::getValue(cx, JL_ARG(5), &stacks) );
	JL_CHK( jl::getValue(cx, JL_ARG(6), &smooth) );

	GLUquadric *q = gluNewQuadric();
	gluQuadricTexture(q, GL_FALSE); // GL_TRUE
	gluQuadricNormals(q, smooth ? GLU_SMOOTH : GLU_FLAT);  // GLU_NONE / GLU_FLAT / GLU_SMOOTH
	gluQuadricOrientation(q, GLU_OUTSIDE); //  GLU_INSIDE
	gluQuadricDrawStyle(q, GLU_FILL); // GLU_FILL / GLU_LINE / GLU_SILHOUETTE / GLU_POINT
	gluCylinder(q, baseRadius, topRadius, height, slices, stacks);  OGL_ERR_CHK;
	gluDeleteQuadric(q);

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
$INAME( lengthX, lengthY, lengthZ );
**/
DEFINE_FUNCTION( drawBox ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(3);

	float lengthX, lengthY, lengthZ;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &lengthX) );
	JL_CHK( jl::getValue(cx, JL_ARG(2), &lengthY) );
	JL_CHK( jl::getValue(cx, JL_ARG(3), &lengthZ) );

	lengthX /= 2.f;
	lengthY /= 2.f;
	lengthZ /= 2.f;

	glBegin(GL_QUADS);  OGL_ERR_CHK;

/* Cube with normals that points outside
	// right
	glNormal3f(1.0f,  1.0f,  1.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f( lengthX, lengthY, lengthZ);  OGL_ERR_CHK;
	glNormal3f(1.0f, -1.0f,  1.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f( lengthX,-lengthY, lengthZ);  OGL_ERR_CHK;
	glNormal3f(1.0f, -1.0f, -1.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f( lengthX,-lengthY,-lengthZ);  OGL_ERR_CHK;
	glNormal3f(1.0f,  1.0f, -1.0f); glTexCoord2f(1.0f, 1.0f); glVertex3f( lengthX, lengthY,-lengthZ);  OGL_ERR_CHK;
	// bottom
	glNormal3f( 1.0f,  1.0f, -1.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f( lengthX, lengthY,-lengthZ);  OGL_ERR_CHK;
	glNormal3f( 1.0f, -1.0f, -1.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f( lengthX,-lengthY,-lengthZ);  OGL_ERR_CHK;
	glNormal3f(-1.0f, -1.0f, -1.0f);	glTexCoord2f(1.0f, 0.0f); glVertex3f(-lengthX,-lengthY,-lengthZ);  OGL_ERR_CHK;
	glNormal3f(-1.0f,  1.0f, -1.0f); glTexCoord2f(1.0f, 1.0f); glVertex3f(-lengthX, lengthY,-lengthZ);  OGL_ERR_CHK;
	// left
	glNormal3f(-1.0f,  1.0f, -1.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f(-lengthX, lengthY,-lengthZ);  OGL_ERR_CHK;
	glNormal3f(-1.0f, -1.0f, -1.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(-lengthX,-lengthY,-lengthZ);  OGL_ERR_CHK;
	glNormal3f(-1.0f, -1.0f,  1.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f(-lengthX,-lengthY, lengthZ);  OGL_ERR_CHK;
	glNormal3f(-1.0f,  1.0f,  1.0f); glTexCoord2f(1.0f, 1.0f); glVertex3f(-lengthX, lengthY, lengthZ);  OGL_ERR_CHK;
	// top
	glNormal3f(-1.0f,  1.0f, 1.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f(-lengthX, lengthY, lengthZ);  OGL_ERR_CHK;
	glNormal3f(-1.0f, -1.0f, 1.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(-lengthX,-lengthY, lengthZ);  OGL_ERR_CHK;
	glNormal3f( 1.0f, -1.0f, 1.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f( lengthX,-lengthY, lengthZ);  OGL_ERR_CHK;
	glNormal3f( 1.0f,  1.0f, 1.0f); glTexCoord2f(1.0f, 1.0f); glVertex3f( lengthX, lengthY, lengthZ);  OGL_ERR_CHK;
	// back
	glNormal3f(-1.0f, 1.0f, -1.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f(-lengthX, lengthY,-lengthZ);  OGL_ERR_CHK;
	glNormal3f(-1.0f, 1.0f,  1.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(-lengthX, lengthY, lengthZ);  OGL_ERR_CHK;
	glNormal3f( 1.0f, 1.0f,  1.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f( lengthX, lengthY, lengthZ);  OGL_ERR_CHK;
	glNormal3f( 1.0f, 1.0f, -1.0f); glTexCoord2f(1.0f, 1.0f); glVertex3f( lengthX, lengthY,-lengthZ);  OGL_ERR_CHK;
	// front
	glNormal3f(-1.0f, -1.0f,  1.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f(-lengthX,-lengthY, lengthZ);  OGL_ERR_CHK;
	glNormal3f(-1.0f, -1.0f, -1.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(-lengthX,-lengthY,-lengthZ);  OGL_ERR_CHK;
	glNormal3f( 1.0f, -1.0f, -1.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f( lengthX,-lengthY,-lengthZ);  OGL_ERR_CHK;
	glNormal3f( 1.0f, -1.0f,  1.0f); glTexCoord2f(1.0f, 1.0f); glVertex3f( lengthX,-lengthY, lengthZ);  OGL_ERR_CHK;
	return;
*/

/*
	glNormal3f(1.0f, 0.0f,  0.0f); glVertex3f( lengthX, lengthY,-lengthZ);  OGL_ERR_CHK;
	glNormal3f(1.0f, 0.0f,  0.0f); glVertex3f( lengthX,-lengthY,-lengthZ);  OGL_ERR_CHK;
	glNormal3f(0.0f, 0.0f, -1.0f); glVertex3f( lengthX,-lengthY,-lengthZ);  OGL_ERR_CHK;
	glNormal3f(0.0f, 0.0f, -1.0f); glVertex3f( lengthX, lengthY,-lengthZ);  OGL_ERR_CHK;//

	glNormal3f(-1.0f, 0.0f, 0.0f); glVertex3f( -lengthX, lengthY,-lengthZ);  OGL_ERR_CHK;
	glNormal3f(-1.0f, 0.0f, 0.0f); glVertex3f( -lengthX,-lengthY,-lengthZ);  OGL_ERR_CHK;
	glNormal3f(0.0f, 0.0f, -1.0f); glVertex3f( -lengthX,-lengthY,-lengthZ);  OGL_ERR_CHK;
	glNormal3f(0.0f, 0.0f, -1.0f); glVertex3f( -lengthX, lengthY,-lengthZ);  OGL_ERR_CHK;

	glNormal3f(0.0f, 1.0f,  0.0f); glVertex3f(  lengthX, lengthY,-lengthZ);  OGL_ERR_CHK;
	glNormal3f(0.0f, 1.0f,  0.0f); glVertex3f( -lengthX, lengthY,-lengthZ);  OGL_ERR_CHK;
	glNormal3f(0.0f, 0.0f, -1.0f); glVertex3f( -lengthX, lengthY,-lengthZ);  OGL_ERR_CHK;
	glNormal3f(0.0f, 0.0f, -1.0f); glVertex3f(  lengthX, lengthY,-lengthZ);  OGL_ERR_CHK;

	glNormal3f(0.0f, -1.0f, 0.0f); glVertex3f(  lengthX, -lengthY,-lengthZ);  OGL_ERR_CHK;
	glNormal3f(0.0f, -1.0f, 0.0f); glVertex3f( -lengthX, -lengthY,-lengthZ);  OGL_ERR_CHK;
	glNormal3f(0.0f, 0.0f, -1.0f); glVertex3f( -lengthX, -lengthY,-lengthZ);  OGL_ERR_CHK;
	glNormal3f(0.0f, 0.0f, -1.0f); glVertex3f(  lengthX, -lengthY,-lengthZ);  OGL_ERR_CHK;


	glNormal3f(1.0f, 0.0f, 0.0f); glVertex3f( lengthX, lengthY, lengthZ);  OGL_ERR_CHK;
	glNormal3f(1.0f, 0.0f, 0.0f); glVertex3f( lengthX,-lengthY, lengthZ);  OGL_ERR_CHK;
	glNormal3f(0.0f, 0.0f, 1.0f); glVertex3f( lengthX,-lengthY, lengthZ);  OGL_ERR_CHK;
	glNormal3f(0.0f, 0.0f, 1.0f); glVertex3f( lengthX, lengthY, lengthZ);  OGL_ERR_CHK;

	glNormal3f(-1.0f, 0.0f, 0.0f); glVertex3f( -lengthX, lengthY, lengthZ);  OGL_ERR_CHK;
	glNormal3f(-1.0f, 0.0f, 0.0f); glVertex3f( -lengthX,-lengthY, lengthZ);  OGL_ERR_CHK;
	glNormal3f( 0.0f, 0.0f, 1.0f); glVertex3f( -lengthX,-lengthY, lengthZ);  OGL_ERR_CHK;
	glNormal3f( 0.0f, 0.0f, 1.0f); glVertex3f( -lengthX, lengthY, lengthZ);  OGL_ERR_CHK;

	glNormal3f(0.0f, 1.0f, 0.0f); glVertex3f(  lengthX, lengthY, lengthZ);  OGL_ERR_CHK;
	glNormal3f(0.0f, 1.0f, 0.0f); glVertex3f( -lengthX, lengthY, lengthZ);  OGL_ERR_CHK;
	glNormal3f(0.0f, 0.0f, 1.0f); glVertex3f( -lengthX, lengthY, lengthZ);  OGL_ERR_CHK;
	glNormal3f(0.0f, 0.0f, 1.0f); glVertex3f(  lengthX, lengthY, lengthZ);  OGL_ERR_CHK;

	glNormal3f(0.0f, -1.0f, 0.0f); glVertex3f(  lengthX, -lengthY, lengthZ);  OGL_ERR_CHK;
	glNormal3f(0.0f, -1.0f, 0.0f); glVertex3f( -lengthX, -lengthY, lengthZ);  OGL_ERR_CHK;
	glNormal3f(0.0f,  0.0f, 1.0f); glVertex3f( -lengthX, -lengthY, lengthZ);  OGL_ERR_CHK;
	glNormal3f(0.0f,  0.0f, 1.0f); glVertex3f(  lengthX, -lengthY, lengthZ);  OGL_ERR_CHK;



	glNormal3f(1.0f, 0.0f, 0.0f); glVertex3f( lengthX, lengthY, lengthZ);  OGL_ERR_CHK;
	glNormal3f(1.0f, 0.0f, 0.0f); glVertex3f( lengthX, lengthY, -lengthZ);  OGL_ERR_CHK;
	glNormal3f(0.0f, 1.0f, 0.0f); glVertex3f( lengthX, lengthY, -lengthZ);  OGL_ERR_CHK;
	glNormal3f(0.0f, 1.0f, 0.0f); glVertex3f( lengthX, lengthY, lengthZ);  OGL_ERR_CHK;

	glNormal3f(1.0f, 0.0f, 0.0f); glVertex3f( lengthX, -lengthY, lengthZ);  OGL_ERR_CHK;
	glNormal3f(1.0f, 0.0f, 0.0f); glVertex3f( lengthX, -lengthY, -lengthZ);  OGL_ERR_CHK;
	glNormal3f(0.0f, -1.0f, 0.0f); glVertex3f( lengthX, -lengthY, -lengthZ);  OGL_ERR_CHK;
	glNormal3f(0.0f, -1.0f, 0.0f); glVertex3f( lengthX, -lengthY, lengthZ);  OGL_ERR_CHK;

	glNormal3f(-1.0f, 0.0f, 0.0f); glVertex3f( -lengthX, -lengthY, lengthZ);  OGL_ERR_CHK;
	glNormal3f(-1.0f, 0.0f, 0.0f); glVertex3f( -lengthX, -lengthY, -lengthZ);  OGL_ERR_CHK;
	glNormal3f(0.0f, -1.0f, 0.0f); glVertex3f( -lengthX, -lengthY, -lengthZ);  OGL_ERR_CHK;
	glNormal3f(0.0f, -1.0f, 0.0f); glVertex3f( -lengthX, -lengthY, lengthZ);  OGL_ERR_CHK;

	glNormal3f(-1.0f, 0.0f, 0.0f); glVertex3f( -lengthX, lengthY, lengthZ);  OGL_ERR_CHK;
	glNormal3f(-1.0f, 0.0f, 0.0f); glVertex3f( -lengthX, lengthY, -lengthZ);  OGL_ERR_CHK;
	glNormal3f(0.0f, 1.0f, 0.0f); glVertex3f( -lengthX, lengthY, -lengthZ);  OGL_ERR_CHK;
	glNormal3f(0.0f, 1.0f, 0.0f); glVertex3f( -lengthX, lengthY, lengthZ);  OGL_ERR_CHK;
*/

	// right

	glNormal3f(1.0f, 0.0f, 0.0f);  OGL_ERR_CHK;
	glTexCoord2f(0.0f, 1.0f); glVertex3f( lengthX, lengthY, lengthZ);  OGL_ERR_CHK;
	glTexCoord2f(0.0f, 0.0f); glVertex3f( lengthX,-lengthY, lengthZ);  OGL_ERR_CHK;
	glTexCoord2f(1.0f, 0.0f); glVertex3f( lengthX,-lengthY,-lengthZ);  OGL_ERR_CHK;
	glTexCoord2f(1.0f, 1.0f); glVertex3f( lengthX, lengthY,-lengthZ);  OGL_ERR_CHK;
	// bottom
	glNormal3f(0.0f, 0.0f, -1.0f);  OGL_ERR_CHK;
	glTexCoord2f(0.0f, 1.0f); glVertex3f( lengthX, lengthY,-lengthZ);  OGL_ERR_CHK;
	glTexCoord2f(0.0f, 0.0f); glVertex3f( lengthX,-lengthY,-lengthZ);  OGL_ERR_CHK;
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-lengthX,-lengthY,-lengthZ);  OGL_ERR_CHK;
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-lengthX, lengthY,-lengthZ);  OGL_ERR_CHK;
	// left
	glNormal3f(-1.0f, 0.0f, 0.0f);  OGL_ERR_CHK;
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-lengthX, lengthY,-lengthZ);  OGL_ERR_CHK;
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-lengthX,-lengthY,-lengthZ);  OGL_ERR_CHK;
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-lengthX,-lengthY, lengthZ);  OGL_ERR_CHK;
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-lengthX, lengthY, lengthZ);  OGL_ERR_CHK;
	// top
	glNormal3f(0.0f, 0.0f, 1.0f);  OGL_ERR_CHK;
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-lengthX, lengthY, lengthZ);  OGL_ERR_CHK;
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-lengthX,-lengthY, lengthZ);  OGL_ERR_CHK;
	glTexCoord2f(1.0f, 0.0f); glVertex3f( lengthX,-lengthY, lengthZ);  OGL_ERR_CHK;
	glTexCoord2f(1.0f, 1.0f); glVertex3f( lengthX, lengthY, lengthZ);  OGL_ERR_CHK;
	// back
	glNormal3f(0.0f, 1.0f, 0.0f);  OGL_ERR_CHK;
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-lengthX, lengthY,-lengthZ);  OGL_ERR_CHK;
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-lengthX, lengthY, lengthZ);  OGL_ERR_CHK;
	glTexCoord2f(1.0f, 0.0f); glVertex3f( lengthX, lengthY, lengthZ);  OGL_ERR_CHK;
	glTexCoord2f(1.0f, 1.0f); glVertex3f( lengthX, lengthY,-lengthZ);  OGL_ERR_CHK;
	// right
	glNormal3f(1.0f, -1.0f, 0.0f);  OGL_ERR_CHK;
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-lengthX,-lengthY, lengthZ);  OGL_ERR_CHK;
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-lengthX,-lengthY,-lengthZ);  OGL_ERR_CHK;
	glTexCoord2f(1.0f, 0.0f); glVertex3f( lengthX,-lengthY,-lengthZ);  OGL_ERR_CHK;
	glTexCoord2f(1.0f, 1.0f); glVertex3f( lengthX,-lengthY, lengthZ);  OGL_ERR_CHK;

	glEnd();  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME()
**/
DEFINE_FUNCTION( fullQuad ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_IGNORE(argc, cx);

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

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz )
  $H API
   gluLookAt
**/
DEFINE_FUNCTION( lookAt ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC(9);
	JL_ASSERT_ARG_IS_NUMBER(1);
	JL_ASSERT_ARG_IS_NUMBER(2);
	JL_ASSERT_ARG_IS_NUMBER(3);
	JL_ASSERT_ARG_IS_NUMBER(4);
	JL_ASSERT_ARG_IS_NUMBER(5);
	JL_ASSERT_ARG_IS_NUMBER(6);
	JL_ASSERT_ARG_IS_NUMBER(7);
	JL_ASSERT_ARG_IS_NUMBER(8);
	JL_ASSERT_ARG_IS_NUMBER(9);

	double eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz;
	jl::getValue(cx, JL_ARG(1), &eyex);
	jl::getValue(cx, JL_ARG(2), &eyey);
	jl::getValue(cx, JL_ARG(3), &eyez);

	jl::getValue(cx, JL_ARG(4), &centerx);
	jl::getValue(cx, JL_ARG(5), &centery);
	jl::getValue(cx, JL_ARG(6), &centerz);

	jl::getValue(cx, JL_ARG(7), &upx);
	jl::getValue(cx, JL_ARG(8), &upy);
	jl::getValue(cx, JL_ARG(9), &upz);

	gluLookAt(eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz);  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( pointX, pointY, pointZ [, upx, upy, upz] )
**/
DEFINE_FUNCTION( aimAt ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_ASSERT_ARGC_RANGE(3,6);
	JL_ASSERT_ARG_IS_NUMBER(1);
	JL_ASSERT_ARG_IS_NUMBER(2);
	JL_ASSERT_ARG_IS_NUMBER(3);

	float px, py, pz, ux, uy, uz;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &px) );
	JL_CHK( jl::getValue(cx, JL_ARG(2), &py) );
	JL_CHK( jl::getValue(cx, JL_ARG(3), &pz) );

	if ( JL_ARGC == 6 ) {

		JL_ASSERT_ARG_IS_NUMBER(4);
		JL_ASSERT_ARG_IS_NUMBER(5);
		JL_ASSERT_ARG_IS_NUMBER(6);

		JL_CHK( jl::getValue(cx, JL_ARG(4), &ux) );
		JL_CHK( jl::getValue(cx, JL_ARG(5), &uy) );
		JL_CHK( jl::getValue(cx, JL_ARG(6), &uz) );
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

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME()
**/
DEFINE_FUNCTION( keepTranslation ) {

	JL_DEFINE_ARGS;

	OGL_CX_CHK;

	JL_IGNORE(argc, cx);

	GLfloat m[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, m);  OGL_ERR_CHK;

//	glLoadIdentity();  OGL_ERR_CHK;
//	glTranslatef(m[12], m[13], m[14]);  OGL_ERR_CHK;
// ... compare perf with:

	memset(m, 0, 12 * sizeof(GLfloat)); // 0..11
	m[0] = 1.f;
	m[5] = 1.f;
	m[10] = 1.f;
	m[15] = 1.f;
	glLoadMatrixf(m);  OGL_ERR_CHK;

	JL_RVAL.setUndefined();
	return true;
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
DEFINE_PROPERTY_GETTER( error ) {

	JL_DEFINE_PROP_ARGS;

	OGL_CX_CHK;

	// When an error occurs, the error flag is set to the appropriate error code value. No other errors are recorded
	// until glGetError is called, the error code is returned, and the flag is reset to GL_NO_ERROR.
	JL_RVAL.setInt32(glGetError());
	return true;
	JL_BAD;
}



bool MatrixGet(JSContext *cx, JS::HandleObject obj, float **m) {

	JL_IGNORE(obj, cx);

	GLint matrixMode;
	glGetIntegerv(GL_MATRIX_MODE, &matrixMode);  OGL_ERR_CHK;
	switch ( matrixMode ) {
		case GL_MODELVIEW:
			glGetFloatv(GL_MODELVIEW_MATRIX, *m);  OGL_ERR_CHK;
			return true;
		case GL_PROJECTION:
			glGetFloatv(GL_PROJECTION_MATRIX, *m);  OGL_ERR_CHK;
			return true;
		case GL_TEXTURE:
			glGetFloatv(GL_TEXTURE_MATRIX, *m);  OGL_ERR_CHK;
			return true;
		case GL_COLOR_MATRIX: // glext
			glGetFloatv(GL_COLOR_MATRIX, *m);  OGL_ERR_CHK;
			return true;
	}
	JL_ERR( E_STR("this matrix mode"), E_NOTSUPPORTED );
bad:
	return false;
}


void *windowsGLGetProcAddress(const char *procName) {

	return wglGetProcAddress(procName);
}


DEFINE_INIT() {

	JL_IGNORE(proto, cs);

#ifdef DEBUG

	// check GL const duplicates
	jl::ConstValueSpec *it2, *it1 = cs->static_const;

	int count;
	for ( it1 = cs->static_const; it1->name != NULL; ++it1 ) {

		count = 0;
		for ( it2 = cs->static_const; it2->name != NULL; ++it2 ) {

//			if ( strcmp(it1->name, it2->name) == 0 && it1->ival != it2->ival ) // detect duplicate name with different value !
			if ( it1->val == it2->val && strcmp(it1->name, it2->name) == 0 ) // detect duplicate name with same value.
				count++;
		}
		if ( count > 1 )
			fprintf(stderr, "Duplicate %dx GL_CONST( %s )\n", count, it1->name );
	}

#endif // DEBUG

	JL_CHK( jl::setMatrix44GetInterface(cx, obj, MatrixGet) );

	ASSERT( glGetProcAddress == NULL );

#ifdef WIN
	glGetProcAddress = windowsGLGetProcAddress;
#else
	//	JL_CHK( GetPrivateNativeFunction(cx, JL_GetGlobal(cx), "_glGetProcAddress", (void**)&glGetProcAddress) );
	JL_CHK( jl::getProperty(cx, GetHostObject(cx), "_glGetProcAddress", (void**)&glGetProcAddress) );
#endif

	ASSERT( glGetProcAddress != NULL );

	return true;
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
DEFINE_FUNCTION( test ) {
	
	JL_DEFINE_ARGS;

/*
	jsval id = JL_ARG(1);
	JL_ASSERT( IsHandleType(cx, id, 'TBUF'), "Invalid buffer." );
	TextureBuffer *tb = (TextureBuffer*)GetHandlePrivate(cx, id);
	tb->TextureBufferAlloc(tb, sizeof(float) * 3 * 32 * 32); // RGB 32x32
	//tb->TextureBufferFree(tb);
	for ( int i = 0; i < 3 * 32*32; i++ )
		tb->data[i] = 0.5;
*/

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}
#endif // DEBUG


CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision$"))
	HAS_INIT

	BEGIN_CONST
		// OpenGL constants
		#include "jsglconst.h"
	END_CONST

	BEGIN_STATIC_FUNCTION_SPEC
#ifdef DEBUG
		FUNCTION_ARGC(test, 1)
#endif // DEBUG
	// OpenGL 1.1 functions
		FUNCTION_ARGC(isEnabled, 1) // cap
		FUNCTION_ARGC(get, 1) // pname
		FUNCTION_ARGC(getBoolean, 2) // pname [,count]
		FUNCTION_ARGC(getInteger, 2) // pname [,count]
		FUNCTION_ARGC(getDouble, 2) // pname [,count]
		FUNCTION_ARGC(getString, 1)
		FUNCTION_ARGC(drawBuffer, 1) // mode
		FUNCTION_ARGC(readBuffer, 1) // mode
		FUNCTION_ARGC(accum, 2) // op, value
		FUNCTION_ARGC(stencilFunc, 3) // func, ref, mask
		FUNCTION_ARGC(stencilOp, 3) // fail, zfail, zpass
		FUNCTION_ARGC(stencilMask, 1) // mask
		FUNCTION_ARGC(alphaFunc, 2) // func, ref
		FUNCTION_ARGC(flush, 0)
		FUNCTION_ARGC(finish, 0)
		FUNCTION_ARGC(fog, 2) // pname, param | array of params
		FUNCTION_ARGC(hint, 2) // target, mode
		FUNCTION_ARGC(vertex, 4) // x, y [, z [, w]]
		FUNCTION_ARGC(edgeFlag, 1) // flag
		FUNCTION_ARGC(color, 4) // r, g, b [, a]
		FUNCTION_ARGC(normal, 3) // nx, ny, nz
		FUNCTION_ARGC(texCoord, 3) // s [, t [,r ]]
		FUNCTION_ARGC(texParameter, 3) // target, pname, param | array of params
		FUNCTION_ARGC(texEnv, 3) // target, pname, param | array of params
		FUNCTION_ARGC(texGen, 3) // coord, pname, params
		FUNCTION_ARGC(texImage2D, 9) // target, level, internalFormat, width, height, border, format, type, data
		FUNCTION_ARGC(copyTexSubImage2D, 8) // target, level, xoffset, yoffset, x, y, width, height
		FUNCTION_ARGC(texSubImage2D, 9) // target, level, xoffset, yoffset, width, height, format, type, data
		FUNCTION_ARGC(lightModel, 2) // pname, param
		FUNCTION_ARGC(light, 3) // light, pname, param
		FUNCTION_ARGC(getLight, 3) // light, pname, count
		FUNCTION_ARGC(colorMaterial, 2) // face, mode
		FUNCTION_ARGC(material, 3) // face, pname, param
		FUNCTION_ARGC(enable, 1) // cap
		FUNCTION_ARGC(disable ,1) // cap
		FUNCTION_ARGC(pointSize, 1) // size
		FUNCTION_ARGC(lineWidth, 1) // width
		FUNCTION_ARGC(shadeModel, 1) // mode
		FUNCTION_ARGC(blendFunc, 2) // sfactor, dfactor
		FUNCTION_ARGC(depthFunc, 1) // func
		FUNCTION_ARGC(depthMask, 1) // mask
		FUNCTION_ARGC(depthRange, 2) // zNear, zFar
		FUNCTION_ARGC(polygonOffset, 2) // factor, units

		FUNCTION_ARGC(cullFace, 1) // mode
		FUNCTION_ARGC(frontFace, 1) // mode
		FUNCTION_ARGC(clearStencil, 1) // s
		FUNCTION_ARGC(clearDepth, 1) // depth
		FUNCTION_ARGC(clearColor, 4) // r, g, b, alpha
		FUNCTION_ARGC(clearAccum, 4) // r, g, b, alpha
		FUNCTION_ARGC(clear, 1) // mask
		FUNCTION_ARGC(colorMask, 4) // r,g,b,a
		FUNCTION_ARGC(clipPlane, 2) // plane, equation
		FUNCTION_ARGC(viewport, 4) // x, y, width, height
		FUNCTION_ARGC(frustum, 6) // left, right, bottom, top, zNear, zFar
		FUNCTION_ARGC(perspective, 4) // fovY, aspectRatio, zNear, zFar (non-OpenGL API)
		FUNCTION_ARGC(ortho, 6) // left, right, bottom, top, zNear, zFar
		FUNCTION_ARGC(matrixMode, 1) // mode
		FUNCTION_ARGC(loadIdentity, 0)
		FUNCTION_ARGC(pushMatrix, 0)
		FUNCTION_ARGC(popMatrix, 0)
		FUNCTION_ARGC(loadMatrix, 1) // matrix
		FUNCTION_ARGC(multMatrix, 1) // matrix
		FUNCTION_ARGC(rotate, 4) // angle, x, y, z
		FUNCTION_ARGC(translate, 3) // x, y, z
		FUNCTION_ARGC(scale, 3) // x, y, z
		FUNCTION_ARGC(newList, 0)
		FUNCTION_ARGC(deleteList, 1) // listId
		FUNCTION_ARGC(endList, 0)
		FUNCTION_ARGC(callList, 1) // listId | array of listId
		FUNCTION_ARGC(polygonMode, 2) // face, mode
		FUNCTION_ARGC(begin, 1) // mode
		FUNCTION_ARGC(end, 0)
		FUNCTION_ARGC(pushAttrib, 1) // mask
		FUNCTION_ARGC(popAttrib, 0)
		FUNCTION_ARGC(genTexture, 0)
		FUNCTION_ARGC(bindTexture, 2) // target, texture
		FUNCTION_ARGC(deleteTexture, 1) // textureId
		FUNCTION_ARGC(copyTexImage2D, 8) // target, level, internalFormat, x, y, width, height, border
		FUNCTION_ARGC(pixelTransfer, 2) // pname, param
		FUNCTION_ARGC(pixelStore, 2) // pname, param
		FUNCTION_ARGC(rasterPos, 4) // x,y,z,w
		FUNCTION_ARGC(pixelZoom, 2) // x,y
		FUNCTION_ARGC(pixelMap, 2) // map,<array>

		FUNCTION_ARGC(createTextureBuffer, 0)
		FUNCTION_ARGC(defineTextureImage, 3) // target, format, image (non-OpenGL API)


// OpenGL extensions
		FUNCTION_ARGC(hasExtensionProc, 1) // procName
		FUNCTION_ARGC(hasExtensionName, 1) // name

		FUNCTION_ARGC(blendEquation, 1) // mode
		FUNCTION_ARGC(stencilFuncSeparate, 4) // func, ref, mask
		FUNCTION_ARGC(stencilOpSeparate, 4) // fail, zfail, zpass
		FUNCTION_ARGC(activeStencilFaceEXT, 1) // face

		FUNCTION_ARGC(bindRenderbuffer, 2) // target, renderbuffer
		FUNCTION_ARGC(genRenderbuffer, 0)
		FUNCTION_ARGC(deleteRenderbuffer, 1) // renderbuffer
		FUNCTION_ARGC(renderbufferStorage, 4) // target, internalformat, width, height
		FUNCTION_ARGC(getRenderbufferParameter, 3) // target, pname [, count]
		FUNCTION_ARGC(bindFramebuffer, 2) // target, renderbuffer
		FUNCTION_ARGC(genFramebuffer, 0)
		FUNCTION_ARGC(deleteFramebuffer, 1) // framebuffer
		FUNCTION_ARGC(checkFramebufferStatus, 1) // target
		FUNCTION_ARGC(framebufferTexture1D, 5) // target, attachment, textarget, texture, level
		FUNCTION_ARGC(framebufferTexture2D, 5) // target, attachment, textarget, texture, level
		FUNCTION_ARGC(framebufferTexture3D, 6) // target, attachment, textarget, texture, level, zoffset
		FUNCTION_ARGC(framebufferRenderbuffer, 4) // target, attachment, renderbuffertarget, renderbuffer
		FUNCTION_ARGC(getFramebufferAttachmentParameter, 4) // target, attachment, pname [, count]

		FUNCTION_ARGC(createShaderObject, 1)
		FUNCTION_ARGC(deleteObject, 1)
		FUNCTION_ARGC(getInfoLog, 1)
		FUNCTION_ARGC(createProgramObject, 0)
		FUNCTION_ARGC(shaderSource, 2)
		FUNCTION_ARGC(compileShader, 1)
		FUNCTION_ARGC(attachObject, 2)
		FUNCTION_ARGC(linkProgram, 1)
		FUNCTION_ARGC(useProgramObject, 1)
		FUNCTION_ARGC(getUniformLocation, 2)
		FUNCTION_ARGC(uniform, 5)
		FUNCTION_ARGC(uniformMatrix, 2)
		FUNCTION_ARGC(uniformFloatVector, 3)
		FUNCTION_ARGC(uniformFloat, 5)
		FUNCTION_ARGC(uniformInteger, 5)
		FUNCTION_ARGC(getObjectParameter, 2)
		FUNCTION_ARGC(bindAttribLocation, 3)
		FUNCTION_ARGC(getAttribLocation, 2)
		FUNCTION_ARGC(vertexAttrib, 2)
		FUNCTION_ARGC(genBuffer, 0)
		FUNCTION_ARGC(bindBuffer, 2) // target, buffer

		FUNCTION_ARGC(pointParameter, 2) // pname, param | Array of param
		FUNCTION_ARGC(activeTexture, 1) // texture
		FUNCTION_ARGC(clientActiveTexture, 1) // texture
		FUNCTION_ARGC(multiTexCoord, 4) // target, s, t, r

		FUNCTION_ARGC(genQueries, 0)
		FUNCTION_ARGC(deleteQueries, 1) // query id
		FUNCTION_ARGC(beginQuery, 2) // target, query id
		FUNCTION_ARGC(endQuery, 1) // query id
		FUNCTION_ARGC(getQuery, 2) // target, pname
		FUNCTION_ARGC(getQueryObject, 3) // id, pname, length


// Helper functions

		FUNCTION_ARGC(getUniformInfo, 1) // (non-OpenGL API)

		FUNCTION_ARGC(unProject, 2) // (non-OpenGL API)

		FUNCTION_ARGC(drawImage, 3) // target, format, image (non-OpenGL API)
		FUNCTION_ARGC(readImage, 0) // (non-OpenGL API)

		FUNCTION_ARGC(loadTrimesh, 1) // Trimesh object
		FUNCTION_ARGC(drawTrimesh, 2) // TrimeshId, mode

		FUNCTION_ARGC(pixelWidthFactor, 0)

		FUNCTION_ARGC(drawPoint, 1)
		FUNCTION_ARGC(drawDisk, 2)
		FUNCTION_ARGC(drawSphere, 4)
		FUNCTION_ARGC(drawCylinder, 6)
		FUNCTION_ARGC(drawBox, 3)
		FUNCTION_ARGC(fullQuad, 0)

		FUNCTION_ARGC(lookAt, 9) // (non-OpenGL API)
		FUNCTION_ARGC(aimAt, 6)
		FUNCTION_ARGC(keepTranslation, 0)
	END_STATIC_FUNCTION_SPEC


	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_GETTER( error )
	END_STATIC_PROPERTY_SPEC

END_CLASS


/**

== Example ==
{{{

loadModule('jsstd');
loadModule('jssdl');
loadModule('jsgraphics');

glSetAttribute( GL_SWAP_CONTROL, 1 ); // vsync
glSetAttribute( GL_DOUBLEBUFFER, 1 );
glSetAttribute( GL_DEPTH_SIZE, 16 );
setVideoMode( 320, 200, 32, HWSURFACE | OPENGL | RESIZABLE );

var listeners = {
	onQuit: function() { end = true },
	onKeyDown: function(key, mod) { end = key == K_ESCAPE }
}

Ogl.matrixMode(Ogl.PROJECTION);
Ogl.perspective(60, 0.001, 1000);
Ogl.matrixMode(Ogl.MODELVIEW);

for (var end = false; !end ;) {

	pollEvent(listeners);

	with (Ogl) { // beware: slower than Ogl.*

		clear(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);
		loadIdentity();

		lookAt(-1,-1,1, 0,0,0, 0,0,1);

		begin(QUADS);
		color(1,0,0);
		vertex(-0.5, -0.5, 0);
		vertex(-0.5,  0.5, 0);
		vertex( 0.5,  0.5, 0);
		vertex( 0.5, -0.5, 0);
		end(QUADS);
   }

	glSwapBuffers();
	sleep(10);
}

}}}
**/

