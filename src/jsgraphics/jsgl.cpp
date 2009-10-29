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

#include "wglew.h"

#include "oglError.h"

// http://www.opengl.org/registry/api/glext.h

typedef void* (__cdecl *glGetProcAddress_t)(const char*);
static glGetProcAddress_t glGetProcAddress = NULL;

#define DECLARE_OPENGL_EXTENSION( name, proto ) static proto name = NULL;

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

/*
#define LOAD_OPENGL_EXTENSION( name, proto ) \
	if ( name != NULL ) \
		JL_REPORT_WARNING("OpenGL extension %s has already been loaded.", #name ); \
	name = (proto) glGetProcAddress( #name ); \
	if ( name == NULL ) \
		JL_REPORT_WARNING("OpenGL extension %s unavailable", #name );
*/

#define INIT_OPENGL_EXTENSION( name, proto ) \
JL_MACRO_BEGIN \
	if ( name == NULL ) \
		name = (proto)glGetProcAddress( #name ); \
JL_MACRO_END

#define JL_INIT_OPENGL_EXTENSION( name, proto ) \
JL_MACRO_BEGIN \
	INIT_OPENGL_EXTENSION(name, proto); \
	JL_S_ASSERT( name != NULL, "OpenGL extension %s is unavailable.", #name ); \
JL_MACRO_END


/* test for gl ext initialization. (TBD) apply this initialization method

void APIENTRY init_glPointParameteri(GLenum e, GLint i);
PFNGLPOINTPARAMETERIPROC glPointParameteri = init_glPointParameteri;
void APIENTRY init_glPointParameteri(GLenum e, GLint i) {

	glPointParameteri = (PFNGLPOINTPARAMETERIPROC)glGetProcAddress("glPointParameteri");
	if ( glPointParameteri == NULL )
		glPointParameteri = init_glPointParameteri;
	else
		return glPointParameteri(e, i);
}
*/






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
 $BOOL $INAME( procName )
  $H arguments
   $ARG string procName
  $H return value
   true if the extension proc is available.
**/
DEFINE_FUNCTION_FAST( HasExtension ) {
	
	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_STRING(JL_FARG(1));
	const char *procName;
	JL_S_ASSERT( glGetProcAddress != NULL, "OpenGL extensions unavailable." );
	JL_CHK( JsvalToString(cx, &JL_FARG(1), &procName) );
	void *procAddr = glGetProcAddress(procName);
	*JL_FRVAL = procAddr != NULL ? JSVAL_TRUE : JSVAL_FALSE;
	return JS_TRUE;
	JL_BAD;
}


/*
DEFINE_FUNCTION_FAST( Get ) {

	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT_INT(JL_FARG(1));

	int pname = JSVAL_TO_INT( JL_FARG(1) );

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
			GLboolean b;
			glGetBooleanv(pname, &b);
			return JS_TRUE;
		}


		case GL_COLOR_WRITEMASK:
		{
		//	4 bool
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
			GLint i;
			glGetIntegerv(pname, &i);
			return JS_TRUE;
		}

		case GL_MAP2_GRID_SEGMENTS:
		case GL_MAX_VIEWPORT_DIMS:
		case GL_POLYGON_MODE: // enum
		{
		//	2 int
		}

		case GL_SCISSOR_BOX:
		case GL_VIEWPORT:
		{
		//	4 int
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
		case GL_INDEX_BITS:
		case GL_LINE_WIDTH:
		case GL_LINE_WIDTH_GRANULARITY:
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
		case GL_SMOOTH_LINE_WIDTH_GRANULARITY:
		{
		//	1 real
		}
		case GL_ALIASED_POINT_SIZE_RANGE:
		case GL_ALIASED_LINE_WIDTH_RANGE:
		case GL_LINE_WIDTH_RANGE:
		case GL_MAP1_GRID_DOMAIN:
		case GL_POINT_SIZE_RANGE:
		case GL_SMOOTH_LINE_WIDTH_RANGE:
		case GL_SMOOTH_POINT_SIZE_RANGE:
		{
		//	2 real
		}
		case GL_CURRENT_NORMAL:
		case GL_POINT_DISTANCE_ATTENUATION:
		{
		//	3 real
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
		//	4 real
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
			16 real
		}
		case GL_COMPRESSED_TEXTURE_FORMATS: // enum
		{
		//	GL_NUM_COMPRESSED_TEXTURE_FORMATS uint
		}
	
	}

//	*JL_FRVAL = BOOLEAN_TO_JSVAL(params);
	return JS_TRUE;
	JL_BAD;
}
*/


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
DEFINE_FUNCTION_FAST( IsEnabled ) {

	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT_INT(JL_FARG(1));
	*JL_FRVAL = BOOLEAN_TO_JSVAL( glIsEnabled(JSVAL_TO_INT(JL_FARG(1))) == GL_TRUE );
	return JS_TRUE;
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
 $VOID $INAME( mode )
  $H OpenGL API
   glDrawBuffer
**/
DEFINE_FUNCTION_FAST( DrawBuffer ) {

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_INT(JL_FARG(1));
	GLenum mode = JSVAL_TO_INT(JL_FARG(1));
	
	glDrawBuffer(mode);
	
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( mode )
  $H OpenGL API
   glReadBuffer
**/
DEFINE_FUNCTION_FAST( ReadBuffer ) {

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_INT(JL_FARG(1));
	GLenum mode = JSVAL_TO_INT(JL_FARG(1));
	
	glReadBuffer(mode);
	
	*JL_FRVAL = JSVAL_VOID;
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

	JL_S_ASSERT_ARG(2);
	JL_S_ASSERT_INT(JL_FARG(1));
	GLenum op = JSVAL_TO_INT(JL_FARG(1));
	double value;
	JsvalToDouble(cx, JL_FARG(2), &value);
	
	glAccum(op, value);
	
	*JL_FRVAL = JSVAL_VOID;
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
  $H OpenGL API
   glStencilFunc
**/
DEFINE_FUNCTION_FAST( StencilFunc ) {

	JL_S_ASSERT_ARG(3);
	JL_S_ASSERT_INT(JL_FARG(1));
	JL_S_ASSERT_INT(JL_FARG(2));
	JL_S_ASSERT_INT(JL_FARG(3));

	glStencilFunc(JSVAL_TO_INT(JL_FARG(1)), JSVAL_TO_INT(JL_FARG(2)), JSVAL_TO_INT(JL_FARG(3)));

	*JL_FRVAL = JSVAL_VOID;
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
DEFINE_FUNCTION_FAST( StencilOp ) {

	JL_S_ASSERT_ARG(3);
	JL_S_ASSERT_INT(JL_FARG(1));
	JL_S_ASSERT_INT(JL_FARG(2));
	JL_S_ASSERT_INT(JL_FARG(3));

	glStencilOp(JSVAL_TO_INT(JL_FARG(1)), JSVAL_TO_INT(JL_FARG(2)), JSVAL_TO_INT(JL_FARG(3)));

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

	JL_S_ASSERT_ARG(2);
	JL_S_ASSERT_INT(JL_FARG(1));
	double ref;
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

	JL_S_ASSERT_ARG(2);
	JL_S_ASSERT_INT(JL_FARG(1));

	*JL_FRVAL = JSVAL_VOID;
	if ( JSVAL_IS_INT(JL_FARG(2)) ) {

		glFogi(JSVAL_TO_INT(JL_FARG(1)), JSVAL_TO_INT(JL_FARG(2)));

		return JS_TRUE;
	}
	if ( JSVAL_IS_DOUBLE(JL_FARG(2)) ) {

		double param;
		JsvalToDouble(cx, JL_FARG(2), &param);
		
		glFogf( JSVAL_TO_INT(JL_FARG(1)), param );
		
		return JS_TRUE;
	}
	if ( JsvalIsArray(cx, JL_FARG(2)) ) {

		GLfloat params[16];
		uint32 length;
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

	JL_S_ASSERT_ARG(2);
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

	JL_S_ASSERT_ARG_RANGE(2,3);
	*JL_FRVAL = JSVAL_VOID;

	double x, y, z;
	JsvalToDouble(cx, JL_FARG(1), &x);
	JsvalToDouble(cx, JL_FARG(2), &y);
	if ( JL_ARGC >= 3 ) {

		JsvalToDouble(cx, JL_FARG(3), &z);
		
		glVertex3d(x, y, z);
		
		return JS_TRUE;
	}

	glVertex2d(x, y);

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

	JL_S_ASSERT_ARG_RANGE(1,4);
	*JL_FRVAL = JSVAL_VOID;

	double r, g, b, a;
	JsvalToDouble(cx, JL_FARG(1), &r);
	if ( argc == 1 ) {
		
		glColor3d(r, r, r);
		return JS_TRUE;
	}		
	JsvalToDouble(cx, JL_FARG(2), &g);
	JsvalToDouble(cx, JL_FARG(3), &b);
	if ( argc == 3 ) {

		glColor3d(r, g, b);
		return JS_TRUE;
	}		
	JsvalToDouble(cx, JL_FARG(4), &a);
	glColor4d(r, g, b, a);
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

	JL_S_ASSERT_ARG(3);
	double nx, ny, nz;
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

	JL_S_ASSERT_ARG_RANGE(1,3);
	*JL_FRVAL = JSVAL_VOID;
	double s;
	JsvalToDouble(cx, JL_FARG(1), &s);
	if ( JL_ARGC == 1 ) {

		glTexCoord1d(s);

		return JS_TRUE;
	}
	double t;
	JsvalToDouble(cx, JL_FARG(2), &t);
	if ( JL_ARGC == 2 ) {

		glTexCoord2d(s, t);

		return JS_TRUE;
	}
	double r;
	JsvalToDouble(cx, JL_FARG(3), &r);

	glTexCoord3d(s, t, r);

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

	JL_S_ASSERT_ARG(3);
	JL_S_ASSERT_INT(JL_FARG(1));
	JL_S_ASSERT_INT(JL_FARG(2));

	*JL_FRVAL = JSVAL_VOID;
	if ( JSVAL_IS_INT(JL_FARG(3)) ) {

		glTexParameteri( JSVAL_TO_INT( JL_FARG(1) ), JSVAL_TO_INT( JL_FARG(2) ), JSVAL_TO_INT( JL_FARG(3) ) );

		return JS_TRUE;
	}
	if ( JSVAL_IS_DOUBLE(JL_FARG(3)) ) {

		double param;
		JsvalToDouble(cx, JL_FARG(3), &param);
		
		glTexParameterf( JSVAL_TO_INT( JL_FARG(1) ), JSVAL_TO_INT( JL_FARG(2) ), param );
		
		return JS_TRUE;
	}
	if ( JsvalIsArray(cx, JL_FARG(3)) ) {

		GLfloat params[16];
		uint32 length;
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

	JL_S_ASSERT_ARG(3);
	JL_S_ASSERT_INT(JL_FARG(1));
	JL_S_ASSERT_INT(JL_FARG(2));

	*JL_FRVAL = JSVAL_VOID;
	if ( JSVAL_IS_INT(JL_FARG(3)) ) {

		glTexEnvi( JSVAL_TO_INT( JL_FARG(1) ), JSVAL_TO_INT( JL_FARG(2) ), JSVAL_TO_INT( JL_FARG(3) ) );

		return JS_TRUE;
	}
	if ( JSVAL_IS_DOUBLE(JL_FARG(3)) ) {

		double param;
		JsvalToDouble(cx, JL_FARG(3), &param);

		glTexEnvf( JSVAL_TO_INT(JL_FARG(1)), JSVAL_TO_INT(JL_FARG(2)), param );

		return JS_TRUE;
	}
	if ( JsvalIsArray(cx, JL_FARG(3)) ) {

		GLfloat params[16];
		uint32 length;
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

	JL_S_ASSERT_ARG(2);
	JL_S_ASSERT_INT(JL_FARG(1));

	*JL_FRVAL = JSVAL_VOID;
	if ( JSVAL_IS_INT(JL_FARG(2)) ) {

		glLightModeli( JSVAL_TO_INT( JL_FARG(1) ), JSVAL_TO_INT( JL_FARG(2) ) );

		return JS_TRUE;
	}
	if ( JSVAL_IS_DOUBLE(JL_FARG(2)) ) {

		double param;
		JsvalToDouble(cx, JL_FARG(2), &param);

		glLightModelf( JSVAL_TO_INT( JL_FARG(1) ), param );
		
		return JS_TRUE;
	}
	if ( JsvalIsArray(cx, JL_FARG(2)) ) {

		GLfloat params[16];
		uint32 length;
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

	JL_S_ASSERT_ARG(3);
	JL_S_ASSERT_INT(JL_FARG(1));
	JL_S_ASSERT_INT(JL_FARG(2));

	*JL_FRVAL = JSVAL_VOID;
	if ( JSVAL_IS_INT(JL_FARG(3)) ) {

		glLighti( JSVAL_TO_INT( JL_FARG(1) ), JSVAL_TO_INT( JL_FARG(2) ), JSVAL_TO_INT( JL_FARG(3) ) );

		return JS_TRUE;
	}
	if ( JSVAL_IS_DOUBLE(JL_FARG(3)) ) {

		double param;
		JsvalToDouble(cx, JL_FARG(3), &param);

		glLightf( JSVAL_TO_INT( JL_FARG(1) ), JSVAL_TO_INT( JL_FARG(2) ), param );

		return JS_TRUE;
	}
	if ( JsvalIsArray(cx, JL_FARG(3)) ) {

		GLfloat params[16];
		uint32 length;
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

	JL_S_ASSERT_ARG(2);
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

	JL_S_ASSERT_ARG(3);
	JL_S_ASSERT_INT(JL_FARG(1));
	JL_S_ASSERT_INT(JL_FARG(2));

	*JL_FRVAL = JSVAL_VOID;
	if ( JSVAL_IS_INT(JL_FARG(3)) ) {

		glMateriali( JSVAL_TO_INT( JL_FARG(1) ), JSVAL_TO_INT( JL_FARG(2) ), JSVAL_TO_INT( JL_FARG(3) ) );
		return JS_TRUE;
	}
	if ( JSVAL_IS_DOUBLE(JL_FARG(3)) ) {

		double param;
		JsvalToDouble(cx, JL_FARG(3), &param);
		glMaterialf( JSVAL_TO_INT( JL_FARG(1) ), JSVAL_TO_INT( JL_FARG(2) ), param );
		return JS_TRUE;
	}
	if ( JsvalIsArray(cx, JL_FARG(3)) ) {

		GLfloat params[16];
		uint32 length;
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

	JL_S_ASSERT_ARG(1);
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

	JL_S_ASSERT_ARG(1);
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

	JL_S_ASSERT_ARG(1);
	double size;
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

	JL_S_ASSERT_ARG(1);
	double width;
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

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_INT(JL_FARG(1));

	glShadeModel(JSVAL_TO_INT( JL_FARG(1) ));
	
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

	JL_S_ASSERT_ARG(2);
	JL_S_ASSERT_INT(JL_FARG(1));
	JL_S_ASSERT_INT(JL_FARG(2));

	glBlendFunc(JSVAL_TO_INT( JL_FARG(1) ), JSVAL_TO_INT( JL_FARG(2) ));
	
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

	JL_S_ASSERT_ARG(1);
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

	JL_S_ASSERT_ARG(2);
	double zNear, zFar;
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

	JL_S_ASSERT_ARG(1);
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

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_INT(JL_FARG(1));

	glFrontFace(JSVAL_TO_INT( JL_FARG(1) ));
	
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

	JL_S_ASSERT_ARG(1);
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

	JL_S_ASSERT_ARG(1);
	double depth;
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

	JL_S_ASSERT_ARG(4);
	double r, g, b, a;
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

	JL_S_ASSERT_ARG(4);
	double r, g, b, a;
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

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_INT(JL_FARG(1));

	glClear(JSVAL_TO_INT(JL_FARG(1)));
	
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( red, green, blue, alpha )
  $H arguments
   $ARG boolean red
   $ARG boolean green
   $ARG boolean blue
   $ARG boolean alpha
  $H OpenGL API
   glColorMask
**/
DEFINE_FUNCTION_FAST( ColorMask ) {

	JL_S_ASSERT_ARG(4);
	JL_S_ASSERT_BOOLEAN(JL_FARG(1));
	JL_S_ASSERT_BOOLEAN(JL_FARG(2));
	JL_S_ASSERT_BOOLEAN(JL_FARG(3));
	JL_S_ASSERT_BOOLEAN(JL_FARG(4));

	glColorMask(JSVAL_TO_BOOLEAN(JL_FARG(1)), JSVAL_TO_BOOLEAN(JL_FARG(2)), JSVAL_TO_BOOLEAN(JL_FARG(3)), JSVAL_TO_BOOLEAN(JL_FARG(4)) );

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

	JL_S_ASSERT_ARG(2);
	JL_S_ASSERT_INT(JL_FARG(1));
	JL_S_ASSERT_ARRAY(JL_FARG(2));
	GLdouble equation[16];
	uint32 length;
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

	JL_S_ASSERT_ARG(4);
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

	JL_S_ASSERT_ARG(6);
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

	JL_S_ASSERT_ARG(6);
	jsdouble left, right, bottom, top, zNear, zFar;
	JsvalToDouble(cx, JL_FARG(1), &left);
	JsvalToDouble(cx, JL_FARG(2), &right);
	JsvalToDouble(cx, JL_FARG(3), &bottom);
	JsvalToDouble(cx, JL_FARG(4), &top);
	JsvalToDouble(cx, JL_FARG(5), &zNear);
	JsvalToDouble(cx, JL_FARG(6), &zFar);

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
	double fovy, zNear, zFar;
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

	JL_S_ASSERT_ARG(1);
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

	JL_S_ASSERT_ARG(0);

	glLoadIdentity();

	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  $H OpenGL API
   glPushMatrix
**/
DEFINE_FUNCTION_FAST( PushMatrix ) {

	JL_S_ASSERT_ARG(0);

	glPushMatrix();

	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  $H OpenGL API
   glPopMatrix
**/
DEFINE_FUNCTION_FAST( PopMatrix ) {

	JL_S_ASSERT_ARG(0);

	glPopMatrix();

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
DEFINE_FUNCTION_FAST( LoadMatrix ) {

	JL_S_ASSERT_ARG(1);
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

	JL_S_ASSERT_ARG(1);
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

	JL_S_ASSERT_ARG(4);
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

	JL_S_ASSERT_ARG_RANGE(2,3);
	double x, y, z;
	JsvalToDouble(cx, JL_FARG(1), &x);
	JsvalToDouble(cx, JL_FARG(2), &y);
	if ( argc >= 3 )
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
 $VOID $INAME( factor )
  $H arguments
   $ARG $REAL x
   $ARG $REAL y
   $ARG $REAL z
  $H OpenGL API
   glScaled
**/
DEFINE_FUNCTION_FAST( Scale ) {

	JL_S_ASSERT_ARG_RANGE(1,3);
	*JL_FRVAL = JSVAL_VOID;
	double x, y, z;
	JsvalToDouble(cx, JL_FARG(1), &x);

	if ( argc == 1 ) {

		glScaled(x, x, x);
		return JS_TRUE;
	}
	JsvalToDouble(cx, JL_FARG(2), &y);

	if ( argc >= 3 ) {

		JsvalToDouble(cx, JL_FARG(3), &z);
		glScaled(x, y, z);
		return JS_TRUE;
	}

	glScaled(x, y, 1);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME( [ compileOnly ] )
  Returns a new display-list.
  $H OpenGL API
   glGenLists, glNewList
**/
DEFINE_FUNCTION_FAST( NewList ) {

	JL_S_ASSERT_ARG_RANGE(0,1);
	bool compileOnly;
	if ( JL_FARG_ISDEF(1) )
		JsvalToBool(cx, JL_FARG(1), &compileOnly);
	else
		compileOnly = false;

	GLuint list = glGenLists(1);
	glNewList(list, compileOnly ? GL_COMPILE : GL_COMPILE_AND_EXECUTE);
	
	*JL_FRVAL = INT_TO_JSVAL(list);
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
DEFINE_FUNCTION_FAST( DeleteList ) {

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_INT(JL_FARG(1));
	
	glDeleteLists(JSVAL_TO_INT(JL_FARG(1)), 1);
	
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

	JL_S_ASSERT_ARG(0);

	glEndList();

	*JL_FRVAL = JSVAL_VOID;
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
DEFINE_FUNCTION_FAST( CallList ) {

	JL_S_ASSERT_ARG(1);
	*JL_FRVAL = JSVAL_VOID;

	if (JSVAL_IS_INT( JL_FARG(1) )) {

		glCallList(JSVAL_TO_INT(JL_FARG(1)));

		return JS_TRUE;
	}
	if (JsvalIsArray(cx, JL_FARG(1))) {

		JSObject *jsArray = JSVAL_TO_OBJECT(JL_FARG(1));
		jsuint length;
		JL_CHK( JS_GetArrayLength(cx, jsArray, &length) );

		GLuint *lists = (GLuint*)alloca(length * sizeof(GLuint));
		jsval value;
		for (jsuint i=0; i<length; ++i) {

			JL_CHK( JS_GetElement(cx, jsArray, i, &value) );
			lists[i] = JSVAL_TO_INT(value);
		}

		glCallLists(length, GL_UNSIGNED_INT, lists); // http://www.opengl.org/documentation/specs/man_pages/hardcopy/GL/html/gl/calllists.html

//		jl_free(lists); // alloca
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

	JL_S_ASSERT_ARG(1);
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

	JL_S_ASSERT_ARG(0);

	glEnd();
	
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
   glPushAttrib
**/
DEFINE_FUNCTION_FAST( PushAttrib ) {

	JL_S_ASSERT_ARG(1);
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

	JL_S_ASSERT_ARG(0);
	
	glPopAttrib();

	*JL_FRVAL = JSVAL_VOID;
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
DEFINE_FUNCTION_FAST( GenTexture ) {

	JL_S_ASSERT_ARG(0);
	GLuint texture;
	
	glGenTextures( 1, &texture );
	
	*JL_FRVAL = INT_TO_JSVAL(texture);
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
DEFINE_FUNCTION_FAST( BindTexture ) {

	JL_S_ASSERT_ARG(2);
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

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_INT(JL_FARG(1));
	GLuint texture = JSVAL_TO_INT( JL_FARG(1) );

	glDeleteTextures(1, &texture);

	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( target, level, internalFormat, x, y, width, height, [ border ] )
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
DEFINE_FUNCTION_FAST( CopyTexImage2D ) {

	JL_S_ASSERT_ARG_MIN(6);
	JL_S_ASSERT_INT(JL_FARG(1));
	JL_S_ASSERT_INT(JL_FARG(2));
	JL_S_ASSERT_INT(JL_FARG(3));
	JL_S_ASSERT_INT(JL_FARG(4));
	JL_S_ASSERT_INT(JL_FARG(5));
	JL_S_ASSERT_INT(JL_FARG(6));
	JL_S_ASSERT_INT(JL_FARG(7));

	GLint border;
	if ( JL_FARG_ISDEF(8) ) {

		JL_S_ASSERT_INT(JL_FARG(8));
		border = JSVAL_TO_INT(JL_FARG(8));
	} else {

		border = 0;
	}

	glCopyTexImage2D(JSVAL_TO_INT(JL_FARG(1)), JSVAL_TO_INT(JL_FARG(2)), JSVAL_TO_INT(JL_FARG(3)), JSVAL_TO_INT(JL_FARG(4)), JSVAL_TO_INT(JL_FARG(5)), JSVAL_TO_INT(JL_FARG(6)), JSVAL_TO_INT(JL_FARG(7)), border);

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


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( x, y [ , z [ , w ] ] )
  $H OpenGL API
   glRasterPos*
**/
DEFINE_FUNCTION_FAST( RasterPos ) {

	JL_S_ASSERT_ARG_RANGE(2,4);
	double x, y, z, w;

	*JL_FRVAL = JSVAL_VOID;

	JsvalToDouble(cx, JL_FARG(1), &x);
	JsvalToDouble(cx, JL_FARG(2), &y);
	if ( argc >= 3 ) {

		JsvalToDouble(cx, JL_FARG(3), &z);
		if ( argc >= 4 ) {

			JsvalToDouble(cx, JL_FARG(4), &w);
			glRasterPos4d(x, y, z, w);
			return JS_TRUE;
		}
		glRasterPos3d(x, y, z);
		return JS_TRUE;
	}
	glRasterPos2d(x, y);
 	return JS_TRUE;
	JL_BAD;
}


///////////////////////////////////////////////////////////////////////////////
// OpenGL extensions


/**doc
$TOC_MEMBER $INAME
 $INT $INAME()
  Returns a new buffer.
  $H OpenGL API
   glGenBuffers
**/
DEFINE_FUNCTION_FAST( GenBuffer ) {
	
	JL_INIT_OPENGL_EXTENSION( glGenBuffers, PFNGLGENBUFFERSPROC );
	
	JL_S_ASSERT_ARG(0);
	GLuint buffer;

	glGenBuffers(1, &buffer);
	
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
   glBindBuffer
**/
DEFINE_FUNCTION_FAST( BindBuffer ) {

	JL_INIT_OPENGL_EXTENSION( glBindBuffer, PFNGLBINDBUFFERPROC );

	JL_S_ASSERT_ARG(2);
	JL_S_ASSERT_INT(JL_FARG(1));
	JL_S_ASSERT_INT(JL_FARG(2));
	
	glBindBuffer(JSVAL_TO_INT(JL_FARG(1)), JSVAL_TO_INT(JL_FARG(2)));

	*JL_FRVAL = JSVAL_VOID;
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
DEFINE_FUNCTION_FAST( BufferData ) {

	LOAD_OPENGL_EXTENSION( glBufferDataARB, PFNGLBUFFERDATAARBPROC ); // glBufferDataARB (GLenum target, GLsizeiptrARB size, const GLvoid *data, GLenum usage);

	// see http://www.songho.ca/opengl/gl_pbo.html

	JL_S_ASSERT_ARG_MIN(2);
	JL_S_ASSERT_INT(JL_FARG(1));
	JL_S_ASSERT_INT(JL_FARG(2));
	GLenum target = JSVAL_TO_INT(JL_FARG(1));
	GLenum buffer = JSVAL_TO_INT(JL_FARG(2));
	
	glBufferDataARB(target, buffer);
	*JL_FRVAL = JSVAL_VOID;
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
DEFINE_FUNCTION_FAST( PointParameter ) {

	JL_INIT_OPENGL_EXTENSION( glPointParameteri, PFNGLPOINTPARAMETERIPROC );
	JL_INIT_OPENGL_EXTENSION( glPointParameterf, PFNGLPOINTPARAMETERFPROC );
	JL_INIT_OPENGL_EXTENSION( glPointParameterfv, PFNGLPOINTPARAMETERFVPROC );

	JL_S_ASSERT_ARG(2);
	JL_S_ASSERT_INT(JL_FARG(1));

	*JL_FRVAL = JSVAL_VOID;
	if ( JSVAL_IS_INT(JL_FARG(2)) ) {

		glPointParameteri(JSVAL_TO_INT(JL_FARG(1)), JSVAL_TO_INT(JL_FARG(2)));
		return JS_TRUE;
	}
	if ( JSVAL_IS_DOUBLE(JL_FARG(2)) ) {

		double param;
		JsvalToDouble(cx, JL_FARG(2), &param);

		glPointParameterf( JSVAL_TO_INT(JL_FARG(1)), param );
		
		return JS_TRUE;
	}
	if ( JsvalIsArray(cx, JL_FARG(2)) ) {

		GLfloat params[16];
		uint32 length;
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
   glActiveTexture
**/
DEFINE_FUNCTION_FAST( ActiveTexture ) {

	JL_INIT_OPENGL_EXTENSION( glActiveTexture, PFNGLACTIVETEXTUREPROC );

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_INT(JL_FARG(1));
	
	glActiveTexture(JSVAL_TO_INT(JL_FARG(1)));
	
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
   glClientActiveTexture
**/
DEFINE_FUNCTION_FAST( ClientActiveTexture ) {

	JL_INIT_OPENGL_EXTENSION( glClientActiveTexture, PFNGLCLIENTACTIVETEXTUREPROC );

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_INT(JL_FARG(1));

	glClientActiveTexture(JSVAL_TO_INT(JL_FARG(1)));
	
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

	JL_INIT_OPENGL_EXTENSION( glMultiTexCoord1d, PFNGLMULTITEXCOORD1DPROC );
	JL_INIT_OPENGL_EXTENSION( glMultiTexCoord2d, PFNGLMULTITEXCOORD2DPROC );
	JL_INIT_OPENGL_EXTENSION( glMultiTexCoord3d, PFNGLMULTITEXCOORD3DPROC );

	JL_S_ASSERT_ARG_RANGE(2,4);
	*JL_FRVAL = JSVAL_VOID;

	JL_S_ASSERT_INT(JL_FARG(1));
	GLenum target = JSVAL_TO_INT(JL_FARG(1));

	double s;
	JsvalToDouble(cx, JL_FARG(2), &s);
	if ( JL_ARGC == 2 ) {

		glMultiTexCoord1d(target, s);
		return JS_TRUE;
	}
	double t;
	JsvalToDouble(cx, JL_FARG(3), &t);
	if ( JL_ARGC == 3 ) {

		glMultiTexCoord2d(target, s, t);
		return JS_TRUE;
	}
	double r;
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
 $VOID $INAME( target, renderbuffer )
  $H arguments
   $ARG enum target
   $ARG uint renderbuffer
  $H API
   glBindRenderbufferEXT
**/
DEFINE_FUNCTION_FAST( BindRenderbuffer ) {

	JL_INIT_OPENGL_EXTENSION( glBindRenderbufferEXT, PFNGLBINDRENDERBUFFEREXTPROC );

	JL_S_ASSERT_ARG(2);
	JL_S_ASSERT_INT(JL_FARG(1));
	JL_S_ASSERT_INT(JL_FARG(2));
	
	glBindRenderbufferEXT(JSVAL_TO_INT(JL_FARG(1)), JSVAL_TO_INT(JL_FARG(2)) );
	
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME()
  $H OpenGL API
   glGenRenderbuffersEXT
**/
DEFINE_FUNCTION_FAST( GenRenderbuffer ) {

	JL_INIT_OPENGL_EXTENSION( glGenRenderbuffersEXT, PFNGLGENRENDERBUFFERSEXTPROC);
	
	JL_S_ASSERT_ARG(0);
	GLuint buffer;
	
	glGenRenderbuffersEXT(1, &buffer);
	
	*JL_FRVAL = INT_TO_JSVAL(buffer);
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
DEFINE_FUNCTION_FAST( DeleteRenderbuffer ) {

	JL_INIT_OPENGL_EXTENSION( glDeleteRenderbuffersEXT, PFNGLDELETERENDERBUFFERSEXTPROC );

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_INT(JL_FARG(1));
	GLuint buffer = JSVAL_TO_INT(JL_FARG(1));
	glDeleteRenderbuffersEXT(1, &buffer);
	*JL_FRVAL = JSVAL_VOID;
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
DEFINE_FUNCTION_FAST( RenderbufferStorage ) {

	JL_INIT_OPENGL_EXTENSION( glRenderbufferStorageEXT, PFNGLRENDERBUFFERSTORAGEEXTPROC );

	JL_S_ASSERT_ARG(4);
	JL_S_ASSERT_INT(JL_FARG(1));
	JL_S_ASSERT_INT(JL_FARG(2));
	JL_S_ASSERT_INT(JL_FARG(3));
	JL_S_ASSERT_INT(JL_FARG(4));
	glRenderbufferStorageEXT(JSVAL_TO_INT(JL_FARG(1)), JSVAL_TO_INT(JL_FARG(2)), JSVAL_TO_INT(JL_FARG(3)), JSVAL_TO_INT(JL_FARG(4)) );
	*JL_FRVAL = JSVAL_VOID;
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
DEFINE_FUNCTION_FAST( GetRenderbufferParameter ) {

	JL_INIT_OPENGL_EXTENSION( glGetRenderbufferParameterivEXT, PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC );

	JL_S_ASSERT_ARG_RANGE(2,3);
	JL_S_ASSERT_INT(JL_FARG(1));
	JL_S_ASSERT_INT(JL_FARG(2));

	GLint params[16]; // (TBD) check if it is the max amount of data that glGetRenderbufferParameterivEXT may returns.
	
	glGetRenderbufferParameterivEXT(JSVAL_TO_INT(JL_FARG(1)), JSVAL_TO_INT(JL_FARG(2)), params );

	if ( JL_FARG_ISDEF(3) ) {
		
		int count;
		JL_CHK( JsvalToInt(cx, JL_FARG(3), &count) );
		JL_S_ASSERT( count <= COUNTOF(params), "Too many params" );
		JL_CHK( IntVectorToJsval(cx, params, count, JL_FRVAL, false) );
	} else {

		*JL_FRVAL = INT_TO_JSVAL( params[0] );
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
DEFINE_FUNCTION_FAST( BindFramebuffer ) {

	JL_INIT_OPENGL_EXTENSION( glBindFramebufferEXT, PFNGLBINDFRAMEBUFFEREXTPROC );

	JL_S_ASSERT_ARG(2);
	JL_S_ASSERT_INT(JL_FARG(1));
	JL_S_ASSERT_INT(JL_FARG(2));
	glBindFramebufferEXT( JSVAL_TO_INT(JL_FARG(1)), JSVAL_TO_INT(JL_FARG(2)) );
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME()
  $H OpenGL API
   glGenFramebuffersEXT
**/
DEFINE_FUNCTION_FAST( GenFramebuffer ) {

	JL_INIT_OPENGL_EXTENSION( glGenFramebuffersEXT, PFNGLGENFRAMEBUFFERSEXTPROC );

	JL_S_ASSERT_ARG(0);
	GLuint buffer;
	glGenFramebuffersEXT(1, &buffer);
	*JL_FRVAL = INT_TO_JSVAL(buffer);
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
DEFINE_FUNCTION_FAST( DeleteFramebuffer ) {

	JL_INIT_OPENGL_EXTENSION( glDeleteFramebuffersEXT, PFNGLDELETEFRAMEBUFFERSEXTPROC );

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_INT(JL_FARG(1));
	GLuint buffer = JSVAL_TO_INT(JL_FARG(1));
	glDeleteFramebuffersEXT(1, &buffer);
	*JL_FRVAL = JSVAL_VOID;
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
DEFINE_FUNCTION_FAST( CheckFramebufferStatus ) {

	JL_INIT_OPENGL_EXTENSION( glCheckFramebufferStatusEXT, PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC );

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_INT(JL_FARG(1));
	*JL_FRVAL = INT_TO_JSVAL( glCheckFramebufferStatusEXT(JL_FARG(1)) );
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
DEFINE_FUNCTION_FAST( FramebufferTexture1D ) {

	JL_INIT_OPENGL_EXTENSION( glFramebufferTexture1DEXT, PFNGLFRAMEBUFFERTEXTURE1DEXTPROC );

	JL_S_ASSERT_ARG(5);
	JL_S_ASSERT_INT(JL_FARG(1));
	JL_S_ASSERT_INT(JL_FARG(2));
	JL_S_ASSERT_INT(JL_FARG(3));
	JL_S_ASSERT_INT(JL_FARG(4));
	JL_S_ASSERT_INT(JL_FARG(5));

	glFramebufferTexture1DEXT( JSVAL_TO_INT(JL_FARG(1)), JSVAL_TO_INT(JL_FARG(2)), JSVAL_TO_INT(JL_FARG(3)), JSVAL_TO_INT(JL_FARG(4)), JSVAL_TO_INT(JL_FARG(5)) );
	
	*JL_FRVAL = JSVAL_VOID;
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
DEFINE_FUNCTION_FAST( FramebufferTexture2D ) {

	JL_INIT_OPENGL_EXTENSION( glFramebufferTexture2DEXT, PFNGLFRAMEBUFFERTEXTURE2DEXTPROC );

	JL_S_ASSERT_ARG(5);
	JL_S_ASSERT_INT(JL_FARG(1));
	JL_S_ASSERT_INT(JL_FARG(2));
	JL_S_ASSERT_INT(JL_FARG(3));
	JL_S_ASSERT_INT(JL_FARG(4));
	JL_S_ASSERT_INT(JL_FARG(5));

	glFramebufferTexture2DEXT( JSVAL_TO_INT(JL_FARG(1)), JSVAL_TO_INT(JL_FARG(2)), JSVAL_TO_INT(JL_FARG(3)), JSVAL_TO_INT(JL_FARG(4)), JSVAL_TO_INT(JL_FARG(5)) );
	
	*JL_FRVAL = JSVAL_VOID;
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
DEFINE_FUNCTION_FAST( FramebufferTexture3D ) {

	JL_INIT_OPENGL_EXTENSION( glFramebufferTexture3DEXT, PFNGLFRAMEBUFFERTEXTURE3DEXTPROC );

	JL_S_ASSERT_ARG(5);
	JL_S_ASSERT_INT(JL_FARG(1));
	JL_S_ASSERT_INT(JL_FARG(2));
	JL_S_ASSERT_INT(JL_FARG(3));
	JL_S_ASSERT_INT(JL_FARG(4));
	JL_S_ASSERT_INT(JL_FARG(5));
	JL_S_ASSERT_INT(JL_FARG(6));

	glFramebufferTexture3DEXT( JSVAL_TO_INT(JL_FARG(1)), JSVAL_TO_INT(JL_FARG(2)), JSVAL_TO_INT(JL_FARG(3)), JSVAL_TO_INT(JL_FARG(4)), JSVAL_TO_INT(JL_FARG(5)), JSVAL_TO_INT(JL_FARG(6)) );
	
	*JL_FRVAL = JSVAL_VOID;
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
DEFINE_FUNCTION_FAST( FramebufferRenderbuffer ) {

	JL_INIT_OPENGL_EXTENSION( glFramebufferRenderbufferEXT, PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC );

	JL_S_ASSERT_ARG(5);
	JL_S_ASSERT_INT(JL_FARG(1));
	JL_S_ASSERT_INT(JL_FARG(2));
	JL_S_ASSERT_INT(JL_FARG(3));
	JL_S_ASSERT_INT(JL_FARG(4));

	glFramebufferRenderbufferEXT( JSVAL_TO_INT(JL_FARG(1)), JSVAL_TO_INT(JL_FARG(2)), JSVAL_TO_INT(JL_FARG(3)), JSVAL_TO_INT(JL_FARG(4)) );
	
	*JL_FRVAL = JSVAL_VOID;
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
DEFINE_FUNCTION_FAST( GetFramebufferAttachmentParameter ) {

	JL_INIT_OPENGL_EXTENSION( glGetFramebufferAttachmentParameterivEXT, PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC  );

	JL_S_ASSERT_ARG_RANGE(3,4);
	JL_S_ASSERT_INT(JL_FARG(1));
	JL_S_ASSERT_INT(JL_FARG(2));
	JL_S_ASSERT_INT(JL_FARG(3));

	GLint params[16]; // (TBD) check if it is the max amount of data that glGetRenderbufferParameterivEXT may returns.
	
	glGetFramebufferAttachmentParameterivEXT(JSVAL_TO_INT(JL_FARG(1)), JSVAL_TO_INT(JL_FARG(2)), JSVAL_TO_INT(JL_FARG(3)), params);

	if ( JL_FARG_ISDEF(4) ) {
		
		int count;
		JL_CHK( JsvalToInt(cx, JL_FARG(4), &count) );
		JL_S_ASSERT( count <= COUNTOF(params), "Too many params" );
		JL_CHK( IntVectorToJsval(cx, params, count, JL_FRVAL, false) );
	} else {

		*JL_FRVAL = INT_TO_JSVAL( params[0] );
	}

	return JS_TRUE;
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
DEFINE_FUNCTION_FAST( CreatePbuffer ) {


	return JS_TRUE;
	JL_BAD;
}
*/



/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz )
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
 $TYPE trimeshId $INAME( trimesh )
**/
DEFINE_FUNCTION_FAST( LoadTrimesh ) {

	JL_INIT_OPENGL_EXTENSION( glGenBuffers, PFNGLGENBUFFERSPROC );
	JL_INIT_OPENGL_EXTENSION( glBindBuffer, PFNGLBINDBUFFERPROC );
	JL_INIT_OPENGL_EXTENSION( glBufferData, PFNGLBUFFERDATAPROC );

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_OBJECT(JL_FARG(1));
	JSObject *trimeshObj = JSVAL_TO_OBJECT(JL_FARG(1));
	JL_S_ASSERT( JsvalIsTrimesh(cx, JL_FARG(1)), "Invalid Trimesh object" );
	Surface *srf = GetTrimeshSurface(cx, trimeshObj);
	JL_S_ASSERT_RESOURCE(srf);

	OpenGlTrimeshInfo *info;
	JL_CHK( CreateHandle(cx, TRIMESH_ID_NAME, sizeof(OpenGlTrimeshInfo), (void**)&info, FinalizeTrimesh, JL_FRVAL) );

	if ( srf->vertex ) {
	
		info->vertexCount = srf->vertexCount;
		glGenBuffers(1, &info->vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, info->vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, srf->vertexCount * 3 * sizeof(SURFACE_REAL_TYPE), srf->vertex, GL_STATIC_DRAW);
	} else
		info->vertexBuffer = 0;

	if ( srf->index ) {

		info->indexCount = srf->indexCount;
		glGenBuffers(1, &info->indexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, info->indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, srf->indexCount * sizeof(SURFACE_INDEX_TYPE), srf->index, GL_STATIC_DRAW);
	} else
		info->indexBuffer = 0;

	if ( srf->normal ) {

		glGenBuffers(1, &info->normalBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, info->normalBuffer);
		glBufferData(GL_ARRAY_BUFFER, srf->vertexCount * 3 * sizeof(SURFACE_REAL_TYPE), srf->normal, GL_STATIC_DRAW);
	} else
		info->normalBuffer = 0;

	if ( srf->textureCoordinate ) {

		glGenBuffers(1, &info->texCoordBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, info->texCoordBuffer);
		glBufferData(GL_ARRAY_BUFFER, srf->vertexCount * 3 * sizeof(SURFACE_REAL_TYPE), srf->textureCoordinate, GL_STATIC_DRAW);
	} else
		info->texCoordBuffer = 0;

	if ( srf->color ) {

		glGenBuffers(1, &info->colorBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, info->colorBuffer);
		glBufferData(GL_ARRAY_BUFFER, srf->vertexCount * 4 * sizeof(SURFACE_REAL_TYPE), srf->color, GL_STATIC_DRAW);
	} else
		info->colorBuffer = 0;

	JL_CHK( CheckThrowCurrentOglError(cx) );

	return JS_TRUE;
	JL_BAD;
}




/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( trimeshId [, mode ] )
  $H OpenGL API
   glVertexPointer
**/
DEFINE_FUNCTION_FAST( DrawTrimesh ) {

	JL_INIT_OPENGL_EXTENSION( glBindBuffer, PFNGLBINDBUFFERPROC );

	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT_OBJECT(JL_FARG(1));

	JL_S_ASSERT( IsHandleType(cx, JL_FARG(1), TRIMESH_ID_NAME), "Invalid Id." );

	OpenGlTrimeshInfo *info = (OpenGlTrimeshInfo*)GetHandlePrivate(cx, JL_FARG(1));

	GLenum dataType = sizeof(SURFACE_REAL_TYPE) == sizeof(float) ? GL_FLOAT : GL_DOUBLE;

	GLenum mode;
	if ( JL_FARG_ISDEF(2) )
		mode = JSVAL_TO_INT(JL_FARG(2));
	else
		mode = GL_TRIANGLES;

	if ( info->vertexBuffer ) {

		glEnableClientState(GL_VERTEX_ARRAY);
		glBindBuffer(GL_ARRAY_BUFFER, info->vertexBuffer);
		glVertexPointer(3, dataType, 0, 0);
	}

	if ( info->normalBuffer ) {

		glEnableClientState(GL_NORMAL_ARRAY);
		glBindBuffer(GL_ARRAY_BUFFER, info->normalBuffer);
		glNormalPointer(dataType, 0, 0);
	}

	if ( info->texCoordBuffer ) {

		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glBindBuffer(GL_ARRAY_BUFFER, info->texCoordBuffer);
		glTexCoordPointer(3, dataType, 0, 0);
	}

	if ( info->colorBuffer ) {

		glEnableClientState(GL_COLOR_ARRAY);
		glBindBuffer(GL_ARRAY_BUFFER, info->colorBuffer);
		glColorPointer(4, dataType, 0, 0);
	}

	if ( info->indexBuffer ) {

		//	glEnableClientState(GL_INDEX_ARRAY);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, info->indexBuffer);
		glDrawElements(mode, info->indexCount, GL_UNSIGNED_INT, 0); // 1 triangle = 3 vertex
	} else {
		
		if ( info->vertexBuffer )
			glDrawArrays(mode, 0, info->vertexCount);
	}

//	glDisableClientState(GL_INDEX_ARRAY);
	if ( info->colorBuffer )
		glDisableClientState(GL_COLOR_ARRAY);
	if ( info->texCoordBuffer )
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	if ( info->normalBuffer )
		glDisableClientState(GL_NORMAL_ARRAY);

	glDisableClientState(GL_VERTEX_ARRAY); // deactivate vertex array

	// bind with 0, so, switch back to normal pointer operation
//	glBindBuffer(GL_ARRAY_BUFFER, 0);
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	JL_CHK( CheckThrowCurrentOglError(cx) );

	*JL_FRVAL = JSVAL_VOID;
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


DEFINE_FUNCTION_FAST( CreateTextureBuffer ) {

	JL_INIT_OPENGL_EXTENSION( glGenBuffers, PFNGLGENBUFFERSPROC );
//	JL_INIT_OPENGL_EXTENSION( glBindBuffer, PFNGLBINDBUFFERPROC );

	TextureBuffer *tb;
	JL_CHK( CreateHandle(cx, 'TBUF', sizeof(TextureBuffer), (void**)&tb, TextureBufferFinalize, JL_FRVAL) );
	GLuint pbo;
	glGenBuffers(1, &pbo);
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
DEFINE_FUNCTION_FAST( DefineTextureImage ) {

	JL_S_ASSERT_ARG(3);
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

		JL_CHKM( GetPropertyInt(cx, tObj, "width", &width), "Invalid texture object." );
		JL_CHKM( GetPropertyInt(cx, tObj, "height", &height), "Invalid texture object." );
		JL_CHKM( GetPropertyInt(cx, tObj, "channels", &channels), "Invalid texture object." );
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

	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glTexImage2D( JSVAL_TO_INT(JL_FARG(1)), 0, format, width, height, 0, format, type, data );
//	GLenum err = glGetError();
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( internalformat | $UNDEF, texture )
  $H OpenGL API
   glDrawPixels
**/
DEFINE_FUNCTION_FAST( DrawImage ) {

	JL_S_ASSERT_ARG(2);
	JL_S_ASSERT_OBJECT(JL_FARG(2));

	GLsizei width, height;
	GLenum format, type;
	int channels;
	const GLvoid *data;

	JSObject *tObj = JSVAL_TO_OBJECT(JL_FARG(2));

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

	if ( JL_FARG_ISDEF(1) ) {

		JL_S_ASSERT_INT(JL_FARG(1));
		format = JSVAL_TO_INT(JL_FARG(1));
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

	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glDrawPixels(width, height, format, type, data);

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
	//glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPONENTS, &tComponents);
	//  glGet	with arguments GL_PACK_ALIGNMENT and others

	int length = tWidth * tHeight * 4; // RGBA
	JL_S_ASSERT( length > 0, "Invalid image size." );
	GLvoid *data = JS_malloc(cx, length);
	JL_CHK( data );
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glDeleteTextures(1, &texture);

	jsval blobVal;
	JL_CHK( JL_NewBlob(cx, data, length, &blobVal) );
	JSObject *blobObj;
	JL_CHK( JS_ValueToObject(cx, blobVal, &blobObj) );
	JL_S_ASSERT( blobObj, "Unable to create Blob object." );
	*JL_FRVAL = OBJECT_TO_JSVAL(blobObj);

	JS_DefineProperty(cx, blobObj, "channels", INT_TO_JSVAL(4), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT );
	JS_DefineProperty(cx, blobObj, "width", INT_TO_JSVAL(width), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT );
	JS_DefineProperty(cx, blobObj, "height", INT_TO_JSVAL(height), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT );

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $REAL $INAME()
  pixelWidth = PixelWidthFactor() * width / distance
**/
DEFINE_FUNCTION_FAST( PixelWidthFactor ) {

	// see. http://www.songho.ca/opengl/gl_projectionmatrix.html
	// see. engine_core.h

	JL_S_ASSERT_ARG(0);
	GLint viewport[4];
	GLfloat m[16];
	glGetIntegerv(GL_VIEWPORT, viewport);
	glGetFloatv(GL_PROJECTION_MATRIX, m);
	return FloatToJsval(cx, viewport[2] * m[0], JL_FRVAL); // viewportHeight = viewport[3];
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME( size )
**/
DEFINE_FUNCTION_FAST( DrawPoint ) {

	JL_S_ASSERT_ARG(1);
	double size;
	JL_CHK( JsvalToDouble(cx, JL_FARG(1), &size) );
	glPointSize(size); // get max with GL_POINT_SIZE_RANGE
	glBegin(GL_POINTS);
	glVertex2i(0,0);
	glEnd();
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
$INAME( radius [ , vertexCount = 12 ] )
**/
DEFINE_FUNCTION_FAST( DrawDisk ) {

	float s, c, angle, radius;
	int vertexCount;
	JL_S_ASSERT_ARG_RANGE(1,2);
	JL_CHK( JsvalToFloat(cx, JL_FARG(1), &radius) );
	if ( JL_FARG_ISDEF(2) )
		JL_CHK( JsvalToInt(cx, JL_FARG(2), &vertexCount) );
	else
		vertexCount = 12;
	angle = 2*M_PI / vertexCount;
	glBegin(GL_POLYGON);
	for (int i = 0; i < vertexCount; i++) {

		SinCos(i * angle, &s, &c);
		glTexCoord2f(c / 2.f + 0.5f, s / 2.f + 0.5f);
		glVertex2f(c * radius, s * radius);
	}
	glEnd();
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
$INAME( radius, slices, stacks );
**/
DEFINE_FUNCTION_FAST( DrawSphere ) {

	JL_S_ASSERT_ARG(3);
	double radius;
	int slices, stacks;
	JL_CHK( JsvalToDouble(cx, JL_FARG(1), &radius) );
	JL_CHK( JsvalToInt(cx, JL_FARG(2), &slices) );
	JL_CHK( JsvalToInt(cx, JL_FARG(3), &stacks) );

	GLUquadric *q = gluNewQuadric();
	gluQuadricTexture(q, GL_FALSE);
	gluQuadricNormals(q, GLU_SMOOTH); // GLU_FLAT / GLU_SMOOTH
	gluSphere(q, radius, slices, stacks);
	gluDeleteQuadric(q);

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME()
**/
DEFINE_FUNCTION_FAST( KeepTranslation ) {

	GLfloat m[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, m);

//	glLoadIdentity();
//	glTranslatef(m[12], m[13], m[14]);
// ... compare perf with:

	memset(m, 0, 12 * sizeof(GLfloat)); // 0..11
	m[0] = 1.f;
	m[5] = 1.f;
	m[10] = 1.f;
	m[15] = 1.f;
	glLoadMatrixf(m);

	return JS_TRUE;
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



/**doc
$TOC_MEMBER $INAME
 $STR $INAME $READONLY
**/
DEFINE_PROPERTY(vendor) {

	return StringToJsval(cx, (char*)glGetString(GL_VENDOR), vp);
}

/**doc
$TOC_MEMBER $INAME
 $STR $INAME $READONLY
**/
DEFINE_PROPERTY(renderer) {

	return StringToJsval(cx, (char*)glGetString(GL_RENDERER), vp);
}

/**doc
$TOC_MEMBER $INAME
 $STR $INAME $READONLY
**/
DEFINE_PROPERTY(version) {

	return StringToJsval(cx, (char*)glGetString(GL_VERSION), vp);
}

/**doc
$TOC_MEMBER $INAME
 $STR $INAME $READONLY
**/
DEFINE_PROPERTY(extensions) {

	return StringToJsval(cx, (char*)glGetString(GL_EXTENSIONS), vp);
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
		case GL_COLOR_MATRIX: // glext
			glGetFloatv(GL_COLOR_MATRIX, *m);
			return true;
	}
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
	JL_CHK( GetNativeFunction(cx, JS_GetGlobalObject(cx), "_glGetProcAddress", (void**)&glGetProcAddress) );
#endif
	JL_S_ASSERT( glGetProcAddress != NULL, "OpenGL extensions unavailable." );

/*

	LOAD_OPENGL_EXTENSION( glGenBuffersARB, PFNGLGENBUFFERSARBPROC );
	LOAD_OPENGL_EXTENSION( glBindBufferARB, PFNGLBINDBUFFERARBPROC );
	LOAD_OPENGL_EXTENSION( glPointParameteri, PFNGLPOINTPARAMETERIPROC );
	LOAD_OPENGL_EXTENSION( glPointParameterf, PFNGLPOINTPARAMETERFPROC );
	LOAD_OPENGL_EXTENSION( glPointParameterfv, PFNGLPOINTPARAMETERFVPROC );
	LOAD_OPENGL_EXTENSION( glActiveTextureARB, PFNGLACTIVETEXTUREARBPROC );
	LOAD_OPENGL_EXTENSION( glClientActiveTextureARB, PFNGLCLIENTACTIVETEXTUREARBPROC );
	LOAD_OPENGL_EXTENSION( glMultiTexCoord1d, PFNGLMULTITEXCOORD1DARBPROC );
	LOAD_OPENGL_EXTENSION( glMultiTexCoord2d, PFNGLMULTITEXCOORD2DARBPROC );
	LOAD_OPENGL_EXTENSION( glMultiTexCoord3d, PFNGLMULTITEXCOORD3DARBPROC );
	LOAD_OPENGL_EXTENSION( glBindRenderbufferEXT, PFNGLBINDRENDERBUFFEREXTPROC );
	LOAD_OPENGL_EXTENSION( glGenRenderbuffersEXT, PFNGLGENRENDERBUFFERSEXTPROC );
	LOAD_OPENGL_EXTENSION( glDeleteRenderbuffersEXT, PFNGLDELETERENDERBUFFERSEXTPROC );
	LOAD_OPENGL_EXTENSION( glRenderbufferStorageEXT, PFNGLRENDERBUFFERSTORAGEEXTPROC );
	LOAD_OPENGL_EXTENSION( glGetRenderbufferParameterivEXT, PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC );
	LOAD_OPENGL_EXTENSION( glBindFramebufferEXT, PFNGLBINDFRAMEBUFFEREXTPROC );
	LOAD_OPENGL_EXTENSION( glGenFramebuffersEXT, PFNGLGENFRAMEBUFFERSEXTPROC );
	LOAD_OPENGL_EXTENSION( glDeleteFramebuffersEXT, PFNGLDELETEFRAMEBUFFERSEXTPROC );
	LOAD_OPENGL_EXTENSION( glCheckFramebufferStatusEXT, PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC );
	LOAD_OPENGL_EXTENSION( glFramebufferTexture1DEXT, PFNGLFRAMEBUFFERTEXTURE1DEXTPROC );
	LOAD_OPENGL_EXTENSION( glFramebufferTexture2DEXT, PFNGLFRAMEBUFFERTEXTURE2DEXTPROC );
	LOAD_OPENGL_EXTENSION( glFramebufferTexture3DEXT, PFNGLFRAMEBUFFERTEXTURE3DEXTPROC );
	LOAD_OPENGL_EXTENSION( glFramebufferRenderbufferEXT, PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC );
	LOAD_OPENGL_EXTENSION( glGetFramebufferAttachmentParameterivEXT, PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC );
	LOAD_OPENGL_EXTENSION( glGenBuffersARB, PFNGLGENBUFFERSARBPROC );
	LOAD_OPENGL_EXTENSION( glBindBufferARB, PFNGLBINDBUFFERARBPROC );
	LOAD_OPENGL_EXTENSION( glBufferDataARB, PFNGLBUFFERDATAARBPROC );
	LOAD_OPENGL_EXTENSION( glBufferDataARB, PFNGLBUFFERDATAARBPROC );
	LOAD_OPENGL_EXTENSION( glMapBufferARB, PFNGLMAPBUFFERARBPROC );
	LOAD_OPENGL_EXTENSION( glUnmapBufferARB, PFNGLUNMAPBUFFERARBPROC );
	LOAD_OPENGL_EXTENSION( glIsBufferARB, PFNGLISBUFFERARBPROC );
*/

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
DEFINE_FUNCTION_FAST( Test ) {

	jsval id = JL_FARG(1);
	JL_S_ASSERT( IsHandleType(cx, id, 'TBUF'), "Invalid buffer." );
	TextureBuffer *tb = (TextureBuffer*)GetHandlePrivate(cx, id);

	tb->TextureBufferAlloc(tb, sizeof(float) * 3 * 32 * 32); // RGB 32x32
	//tb->TextureBufferFree(tb);

	for ( int i = 0; i < 3 * 32*32; i++ ) {
		
		tb->data[i] = 0.5;
	}


	return JS_TRUE;
	JL_BAD;
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
		FUNCTION_FAST_ARGC(Test, 1)
#endif // DEBUG


		FUNCTION_FAST_ARGC(HasExtension, 1) // procName
	
		FUNCTION_FAST_ARGC(IsEnabled, 1) // cap
		FUNCTION_FAST_ARGC(GetBoolean, 1) // pname
		FUNCTION_FAST_ARGC(GetInteger, 1) // pname
		FUNCTION_FAST_ARGC(GetDouble, 1) // pname
		FUNCTION_FAST_ARGC(DrawBuffer, 1) // mode
		FUNCTION_FAST_ARGC(ReadBuffer, 1) // mode
		FUNCTION_FAST_ARGC(Accum, 2) // op, value
		FUNCTION_FAST_ARGC(StencilFunc, 3) // func, ref, mask
		FUNCTION_FAST_ARGC(StencilOp, 3) // fail, zfail, zpass
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
		FUNCTION_FAST_ARGC(ColorMask, 4) // r,g,b,a
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
		FUNCTION_FAST_ARGC(CopyTexImage2D, 8) // target, level, internalFormat, x, y, width, height, border
		FUNCTION_FAST_ARGC(RasterPos, 4) // x,y,z,w

		FUNCTION_FAST_ARGC(DefineTextureImage, 3) // target, format, image (non-OpenGL API)

		FUNCTION_FAST_ARGC(BindRenderbuffer, 2) // target, renderbuffer
		FUNCTION_FAST_ARGC(GenRenderbuffer, 0)
		FUNCTION_FAST_ARGC(DeleteRenderbuffer, 1) // renderbuffer
		FUNCTION_FAST_ARGC(RenderbufferStorage, 4) // target, internalformat, width, height
		FUNCTION_FAST_ARGC(GetRenderbufferParameter, 3) // target, pname [, count]
		FUNCTION_FAST_ARGC(BindFramebuffer, 2) // target, renderbuffer
		FUNCTION_FAST_ARGC(GenFramebuffer, 0)
		FUNCTION_FAST_ARGC(DeleteFramebuffer, 1) // framebuffer
		FUNCTION_FAST_ARGC(CheckFramebufferStatus, 1) // target
		FUNCTION_FAST_ARGC(FramebufferTexture1D, 5) // target, attachment, textarget, texture, level
		FUNCTION_FAST_ARGC(FramebufferTexture2D, 5) // target, attachment, textarget, texture, level
		FUNCTION_FAST_ARGC(FramebufferTexture3D, 6) // target, attachment, textarget, texture, level, zoffset
		FUNCTION_FAST_ARGC(FramebufferRenderbuffer, 4) // target, attachment, renderbuffertarget, renderbuffer
		FUNCTION_FAST_ARGC(GetFramebufferAttachmentParameter, 4) // target, attachment, pname [, count]

		FUNCTION_FAST_ARGC(DrawImage, 3) // target, format, image (non-OpenGL API)
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
		FUNCTION_FAST_ARGC(DrawTrimesh, 2) // TrimeshId, mode

		FUNCTION_FAST_ARGC(PixelWidthFactor, 0)

		FUNCTION_FAST_ARGC(DrawPoint, 1)
		FUNCTION_FAST_ARGC(DrawDisk, 2)
		FUNCTION_FAST_ARGC(DrawSphere, 3)

		FUNCTION_FAST_ARGC(KeepTranslation, 0)
	END_STATIC_FUNCTION_SPEC


	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_READ(error)

		PROPERTY_READ(vendor)
		PROPERTY_READ(renderer)
		PROPERTY_READ(version)
		PROPERTY_READ(extensions)
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
