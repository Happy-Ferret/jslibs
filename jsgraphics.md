<b>If something seems wrong or incomplete, please enter <a href='#commentform.md'>a comment at the bottom of this page</a>.</b><br /><br />- [source](http://jslibs.googlecode.com/svn/trunk/./src/jsgraphics/) - [main](JSLibs.md) - [QA](http://jslibs.googlecode.com/svn/trunk/./src/jsgraphics/qa.js) -
# jsgraphics module #




---

## class jsgraphics::Ogl ##
- [top](#jsgraphics_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsgraphics/jsgl.cpp?r=2555) -

### Static functions ###

#### <font color='white' size='1'><b>GetBoolean</b></font> ####
> <sub>boolean</sub> <b>GetBoolean</b>( pname )
> > ##### arguments: #####
      1. <sub>GLenum</sub> _pname_
> > ##### return value: #####
> > > value of a selected parameter.

> > ##### OpenGL API: #####
> > > glGetBooleanv

#### <font color='white' size='1'><b>GetInteger</b></font> ####

> <sub>integer</sub> | <sub>Array</sub> <b>GetInteger</b>( pname [[.md](.md), count] )
> > ##### arguments: #####
      1. <sub>GLenum</sub> _pname_
      1. <sub>integer</sub> _count_: is the number of expected values. If _count_ is defined, the function will returns an array of values, else it returns a single value.
> > ##### return value: #####
> > > A value or an array of values of a selected parameter.

> > ##### OpenGL API: #####
> > > glGetIntegerv

#### <font color='white' size='1'><b>GetDouble</b></font> ####

> <sub>real</sub> | <sub>Array</sub> <b>GetDouble</b>( pname [[.md](.md), count] )
> > ##### arguments: #####
      1. <sub>GLenum</sub> _pname_
      1. <sub>integer</sub> _count_: is the number of expected values. If _count_ is defined, the function will returns an array of values, else a single value.
> > ##### return value: #####
> > > A single value or an Array of values of the selected parameter.

> > ##### OpenGL API: #####
> > > glGetDoublev

#### <font color='white' size='1'><b>Accum</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Accum</b>( op, value )
> > ##### arguments: #####
      1. <sub>GLenum</sub> _op_
      1. <sub>real</sub> _value_
> > ##### OpenGL API: #####
> > > glAccum

#### <font color='white' size='1'><b>AlphaFunc</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>AlphaFunc</b>( func, ref )
> > ##### arguments: #####
      1. <sub>GLenum</sub> _op_
      1. <sub>real</sub> _ref_
> > ##### OpenGL API: #####
> > > glAlphaFunc

#### <font color='white' size='1'><b>Flush</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Flush</b>()
> > ##### OpenGL API: #####
> > > glFlush

#### <font color='white' size='1'><b>Finish</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Finish</b>()
> > ##### OpenGL API: #####
> > > glFinish

#### <font color='white' size='1'><b>Fog</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Fog</b>( pname, params )
> > ##### arguments: #####
      1. <sub>GLenum</sub> _pname_
      1. <sub>value</sub> _params_: is either a number or an array of numbers.
> > ##### OpenGL API: #####
> > > glFogi, glFogf, glFogfv

#### <font color='white' size='1'><b>Hint</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Hint</b>( target, mode )
> > ##### arguments: #####
      1. <sub>GLenum</sub> _target_
      1. <sub>GLenum</sub> _mode_
> > ##### OpenGL API: #####
> > > glHint

#### <font color='white' size='1'><b>Vertex</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Vertex</b>( x, y [[.md](.md), z] )
> > ##### arguments: #####
      1. <sub>real</sub> _x_
      1. <sub>real</sub> _y_
      1. <sub>real</sub> _z_
> > ##### OpenGL API: #####
> > > glVertex3d, glVertex2d

#### <font color='white' size='1'><b>Color</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Color</b>( red, green, blue [[.md](.md), alpha] )
> > ##### arguments: #####
      1. <sub>real</sub> _red_
      1. <sub>real</sub> _green_
      1. <sub>real</sub> _blue_
      1. <sub>real</sub> _alpha_
> > ##### OpenGL API: #####
> > > glColor4d, glColor3d

#### <font color='white' size='1'><b>Normal</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Normal</b>( nx, ny, nz )
> > ##### arguments: #####
      1. <sub>real</sub> _nx_
      1. <sub>real</sub> _ny_
      1. <sub>real</sub> _nz_
> > ##### OpenGL API: #####
> > > glNormal3d

#### <font color='white' size='1'><b>TexCoord</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>TexCoord</b>( s [[.md](.md), t [[.md](.md), r]] )
> > ##### arguments: #####
      1. <sub>real</sub> _s_
      1. <sub>real</sub> _t_
      1. <sub>real</sub> _r_
> > ##### OpenGL API: #####
> > > glTexCoord1d, glTexCoord2d, glTexCoord3d

#### <font color='white' size='1'><b>TexParameter</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>TexParameter</b>( target, pname, params )
> > ##### arguments: #####
      1. <sub>GLenum</sub> _target_
      1. <sub>GLenum</sub> _pname_
      1. <sub>value</sub> _params_: is either a number or an array of numbers.
> > ##### OpenGL API: #####
> > > glTexParameteri, glTexParameterf, glTexParameterfv

#### <font color='white' size='1'><b>TexEnv</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>TexEnv</b>( target, pname, params )
> > ##### arguments: #####
      1. <sub>GLenum</sub> _target_
      1. <sub>GLenum</sub> _pname_
      1. <sub>value</sub> _params_: is either a number or an array of numbers.
> > ##### OpenGL API: #####
> > > glTexEnvi, glTexEnvf, glTexEnvfv

#### <font color='white' size='1'><b>LightModel</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>LightModel</b>( pname, params )
> > ##### arguments: #####
      1. <sub>GLenum</sub> _pname_
      1. <sub>value</sub> _params_: is either a number or an array of numbers.
> > ##### OpenGL API: #####
> > > glLightModeli, glLightModelf, glLightModelfv

#### <font color='white' size='1'><b>Light</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Light</b>( light, pname, params )
> > ##### arguments: #####
      1. <sub>GLenum</sub> _light_
      1. <sub>GLenum</sub> _pname_
      1. <sub>value</sub> _params_: is either a number or an array of numbers.
> > ##### OpenGL API: #####
> > > glLighti, glLightf, glLightfv

#### <font color='white' size='1'><b>Material</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Material</b>( face, pname, params )
> > ##### arguments: #####
      1. <sub>GLenum</sub> _face_
      1. <sub>GLenum</sub> _pname_
      1. <sub>value</sub> _params_: is either a number or an array of numbers.
> > ##### OpenGL API: #####
> > > glMateriali, glMaterialf, glMaterialfv

#### <font color='white' size='1'><b>Enable</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Enable</b>( cap )
> > ##### arguments: #####
      1. <sub>GLenum</sub> _cap_
> > ##### OpenGL API: #####
> > > glEnable

#### <font color='white' size='1'><b>Disable</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Disable</b>( cap )
> > ##### arguments: #####
      1. <sub>GLenum</sub> _cap_
> > ##### OpenGL API: #####
> > > glDisable

#### <font color='white' size='1'><b>PointSize</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>PointSize</b>( size )
> > ##### arguments: #####
      1. <sub>real</sub> _size_
> > ##### OpenGL API: #####
> > > glPointSize

#### <font color='white' size='1'><b>LineWidth</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>LineWidth</b>( width )
> > ##### arguments: #####
      1. <sub>real</sub> _width_
> > ##### OpenGL API: #####
> > > glLineWidth

#### <font color='white' size='1'><b>ShadeModel</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>ShadeModel</b>( mode )
> > ##### arguments: #####
      1. <sub>GLenum</sub> _mode_
> > ##### OpenGL API: #####
> > > glShadeModel

#### <font color='white' size='1'><b>BlendFunc</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>BlendFunc</b>( fFactor, dFactor )
> > ##### arguments: #####
      1. <sub>GLenum</sub> _fFactor_
      1. <sub>GLenum</sub> _dFactor_
> > ##### OpenGL API: #####
> > > glBlendFunc

#### <font color='white' size='1'><b>DepthFunc</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>DepthFunc</b>( func )
> > ##### arguments: #####
      1. <sub>GLenum</sub> _func_
> > ##### OpenGL API: #####
> > > glDepthFunc

#### <font color='white' size='1'><b>DepthRange</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>DepthRange</b>( zNear, zFar )
> > ##### arguments: #####
      1. <sub>real</sub> _zNear_
      1. <sub>real</sub> _zFar_
> > ##### OpenGL API: #####
> > > glDepthRange

#### <font color='white' size='1'><b>CullFace</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>CullFace</b>( mode )
> > ##### arguments: #####
      1. <sub>GLenum</sub> _mode_
> > ##### OpenGL API: #####
> > > glCullFace

#### <font color='white' size='1'><b>FrontFace</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>FrontFace</b>( mode )
> > ##### arguments: #####
      1. <sub>GLenum</sub> _mode_
> > ##### OpenGL API: #####
> > > glFrontFace

#### <font color='white' size='1'><b>ClearStencil</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>ClearStencil</b>( s )
> > ##### arguments: #####
      1. <sub>integer</sub> _s_
> > ##### OpenGL API: #####
> > > glClearStencil

#### <font color='white' size='1'><b>ClearDepth</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>ClearDepth</b>( depth )
> > ##### arguments: #####
      1. <sub>real</sub> _depth_
> > ##### OpenGL API: #####
> > > glClearDepth

#### <font color='white' size='1'><b>ClearColor</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>ClearColor</b>( red, green, blue, alpha )
> > ##### arguments: #####
      1. <sub>real</sub> _red_
      1. <sub>real</sub> _green_
      1. <sub>real</sub> _blue_
      1. <sub>real</sub> _alpha_
> > ##### OpenGL API: #####
> > > glClearColor

#### <font color='white' size='1'><b>ClearAccum</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>ClearAccum</b>( red, green, blue, alpha )
> > ##### arguments: #####
      1. <sub>real</sub> _red_
      1. <sub>real</sub> _green_
      1. <sub>real</sub> _blue_
      1. <sub>real</sub> _alpha_
> > ##### OpenGL API: #####
> > > glClearAccum

#### <font color='white' size='1'><b>Clear</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Clear</b>( mask )
> > ##### arguments: #####
      1. <sub>GLbitfield</sub> _mask_
> > ##### OpenGL API: #####
> > > glClear

#### <font color='white' size='1'><b>ClipPlane</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>ClipPlane</b>( plane, equation )
> > ##### arguments: #####
      1. <sub>GLenum</sub> _plane_
      1. <sub>Array</sub> _equation_: array of real
> > ##### OpenGL API: #####
> > > glClipPlane

#### <font color='white' size='1'><b>Viewport</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Viewport</b>( x, y, width, height )
> > ##### arguments: #####
      1. <sub>integer</sub> _x_
      1. <sub>integer</sub> _y_
      1. <sub>integer</sub> _width_
      1. <sub>integer</sub> _height_
> > ##### OpenGL API: #####
> > > glViewport

#### <font color='white' size='1'><b>Frustum</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Frustum</b>( left, right, bottom, top, zNear, zFar )
> > ##### arguments: #####
      1. <sub>real</sub> _left_
      1. <sub>real</sub> _right_
      1. <sub>real</sub> _bottom_
      1. <sub>real</sub> _top_
      1. <sub>real</sub> _zNear_
      1. <sub>real</sub> _zFar_
> > ##### OpenGL API: #####
> > > glFrustum

#### <font color='white' size='1'><b>Ortho</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Ortho</b>( left, right, bottom, top, zNear, zFar )
> > ##### arguments: #####
      1. <sub>real</sub> _left_
      1. <sub>real</sub> _right_
      1. <sub>real</sub> _bottom_
      1. <sub>real</sub> _top_
      1. <sub>real</sub> _zNear_
      1. <sub>real</sub> _zFar_
> > ##### OpenGL API: #####
> > > glOrtho

#### <font color='white' size='1'><b>Perspective</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Perspective</b>( fovy, zNear, zFar )
> > Set up a perspective projection matrix.
> > ##### arguments: #####
      1. <sub>real</sub> _fovy_
      1. <sub>real</sub> _zNear_
      1. <sub>real</sub> _zFar_
> > ##### note: #####
> > > This is not an OpenGL API function.

> > ##### OpenGL API: #####
> > > glGetIntegerv, glFrustum

#### <font color='white' size='1'><b>MatrixMode</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>MatrixMode</b>( mode )
> > ##### arguments: #####
      1. <sub>GLenum</sub> _mode_
> > ##### OpenGL API: #####
> > > glMatrixMode

#### <font color='white' size='1'><b>LoadIdentity</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>LoadIdentity</b>()
> > ##### OpenGL API: #####
> > > glLoadIdentity

#### <font color='white' size='1'><b>PushMatrix</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>PushMatrix</b>()
> > ##### OpenGL API: #####
> > > glPushMatrix

#### <font color='white' size='1'><b>PopMatrix</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>PopMatrix</b>()
> > ##### OpenGL API: #####
> > > glPopMatrix

#### <font color='white' size='1'><b>LoadMatrix</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>LoadMatrix</b>( matrix )
> > ##### arguments: #####
      1. <sub>value</sub> _matrix_: either a matrix object or an Array
> > ##### OpenGL API: #####
> > > glLoadMatrixf

#### <font color='white' size='1'><b>Rotate</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Rotate</b>( angle, x, y, z )
> > ##### arguments: #####
      1. <sub>real</sub> _angle_
      1. <sub>real</sub> _x_
      1. <sub>real</sub> _y_
      1. <sub>real</sub> _z_
> > ##### OpenGL API: #####
> > > glRotated

#### <font color='white' size='1'><b>Translate</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Translate</b>( x, y [[.md](.md), z = 0] )
> > ##### arguments: #####
      1. <sub>real</sub> _x_
      1. <sub>real</sub> _y_
      1. <sub>real</sub> _z_
> > ##### OpenGL API: #####
> > > glTranslated

#### <font color='white' size='1'><b>Scale</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Scale</b>( x, y [[.md](.md), z = 1] )
> > ##### arguments: #####
      1. <sub>real</sub> _x_
      1. <sub>real</sub> _y_
      1. <sub>real</sub> _z_
> > ##### OpenGL API: #####
> > > glScaled

#### <font color='white' size='1'><b>NewList</b></font> ####

> <sub>integer</sub> <b>NewList</b>()
> > Returns a new display-list.
> > ##### OpenGL API: #####
> > > glNewList

#### <font color='white' size='1'><b>DeleteList</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>DeleteList</b>( list )
> > Deletes a display-list.
> > ##### arguments: #####
      1. <sub>integer</sub> _list_
> > ##### OpenGL API: #####
> > > glDeleteLists

#### <font color='white' size='1'><b>EndList</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>EndList</b>()
> > ##### OpenGL API: #####
> > > glEndList

#### <font color='white' size='1'><b>CallList</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>CallList</b>( lists )
> > Calls one or more display-list.
> > ##### arguments: #####
      1. <sub>value</sub> _lists_: is a single list name or an Array of list name.
> > ##### OpenGL API: #####
> > > glCallList, glCallLists

#### <font color='white' size='1'><b>Begin</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Begin</b>( mode )
> > ##### arguments: #####
      1. <sub>GLenum</sub> _mode_
> > ##### OpenGL API: #####
> > > glBegin

#### <font color='white' size='1'><b>End</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>End</b>()
> > ##### OpenGL API: #####
> > > glEnd

#### <font color='white' size='1'><b>PushAttrib</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>PushAttrib</b>( mask )
> > ##### arguments: #####
      1. <sub>GLbitfield</sub> _mask_
> > ##### OpenGL API: #####
> > > glPushAttrib

#### <font color='white' size='1'><b>PopAttrib</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>PopAttrib</b>()
> > ##### OpenGL API: #####
> > > glPopAttrib

#### <font color='white' size='1'><b>GenTexture</b></font> ####

> <sub>integer</sub> <b>GenTexture</b>()
> > Returns a new texture name.
> > ##### OpenGL API: #####
> > > glGenTextures

#### <font color='white' size='1'><b>BindTexture</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>BindTexture</b>( target, texture )
> > ##### arguments: #####
      1. <sub>GLenum</sub> _target_
      1. <sub>integer</sub> _texture_
> > ##### OpenGL API: #####
> > > glBindTexture

#### <font color='white' size='1'><b>DeleteTexture</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>DeleteTexture</b>( texture )
> > Deletes the given texture.
> > ##### arguments: #####
      1. <sub>integer</sub> _texture_
> > ##### OpenGL API: #####
> > > glDeleteTextures

#### <font color='white' size='1'><b>CopyTexImage2D</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>CopyTexImage2D</b>( level, internalFormat, x, y, width, height, [[.md](.md) border ] )
> > ##### arguments: #####
      1. <sub>integer</sub> _level_
      1. <sub>integer</sub> _internalFormat_
      1. <sub>integer</sub> _x_
      1. <sub>integer</sub> _y_
      1. <sub>integer</sub> _width_
      1. <sub>integer</sub> _height_
      1. <sub>integer</sub> _border_
> > ##### note: #####
> > > The target is always a GL\_TEXTURE\_2D

> > ##### OpenGL API: #####
> > > glCopyTexImage2D

#### <font color='white' size='1'><b>GenBuffer</b></font> ####

> <sub>integer</sub> <b>GenBuffer</b>()
> > Returns a new buffer.
> > ##### OpenGL API: #####
> > > glGenBuffersARB

#### <font color='white' size='1'><b>BindBuffer</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>BindBuffer</b>( target, buffer )
> > ##### arguments: #####
      1. <sub>GLenum</sub> _target_
      1. <sub>integer</sub> _buffer_
> > ##### OpenGL API: #####
> > > glBindBufferARB

#### <font color='white' size='1'><b>PointParameter</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>PointParameter</b>( pname, params )
> > ##### arguments: #####
      1. <sub>GLenum</sub> _pname_
      1. <sub>value</sub> _params_: is a real or an Array of real.
> > ##### OpenGL API: #####
> > > glPointParameterf, glPointParameterfv

#### <font color='white' size='1'><b>ActiveTexture</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>ActiveTexture</b>( texture )
> > ##### arguments: #####
      1. <sub>GLenum</sub> _texture_
> > ##### OpenGL API: #####
> > > glActiveTextureARB

#### <font color='white' size='1'><b>ClientActiveTexture</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>ClientActiveTexture</b>( texture )
> > ##### arguments: #####
      1. <sub>GLenum</sub> _texture_
> > ##### OpenGL API: #####
> > > glClientActiveTextureARB

#### <font color='white' size='1'><b>MultiTexCoord</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>MultiTexCoord</b>( target, s [[.md](.md), t [[.md](.md), r]] )
> > ##### arguments: #####
      1. <sub>GLenum</sub> _target_
      1. <sub>real</sub> _s_
      1. <sub>real</sub> _t_
      1. <sub>real</sub> _r_
> > ##### OpenGL API: #####
> > > glMultiTexCoord1d, glMultiTexCoord2d, glMultiTexCoord3d

#### <font color='white' size='1'><b>LoadTrimesh</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>LoadTrimesh</b>( trimesh )

#### <font color='white' size='1'><b>DrawTrimesh</b></font> ####
> <font color='gray' size='1'><sub>void</sub></font> <b>DrawTrimesh</b>()
> > ##### OpenGL API: #####
> > > glVertexPointer

#### <font color='white' size='1'><b>DefineTextureImage</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>DefineTextureImage</b>( target, [[.md](.md)internalformat], texture )
> > ##### arguments: #####
      1. <sub>GLenum</sub> _target_
      1. <sub>integer</sub> _internalformat_: is the internal PixelFormat. If undefined, the function will use the format of _texture_.
      1. <sub>value</sub> _texture_: either a Texture object or an image object.
> > ##### note: #####
> > > This is not an OpenGL API function.

> > ##### OpenGL API: #####
> > > glPixelStorei, glTexImage2D

#### <font color='white' size='1'><b>RenderToImage</b></font> ####

> <sub>image</sub> <b>RenderToImage</b>()
> > Returns the current contain of the viewport.
> > ##### arguments: #####
      1. <sub>GLenum</sub> _target_
      1. <sub>integer</sub> _internalformat_: is the internal PixelFormat. If undefined, the function will use the format of _texture_.
      1. <sub>value</sub> _texture_: either a Texture object or an image object.
> > ##### return value: #####
> > > An image object.

> > ##### note: #####
> > > This is not an OpenGL API function.

> > ##### OpenGL API: #####
> > > glGenTextures, glBindTexture, glGetIntegerv, glCopyTexImage2D, glGetTexLevelParameteriv, glGetTexImage, glDeleteTextures

### Static properties ###

#### <font color='white' size='1'><b>error</b></font> ####

> <sub>integer</sub> <b>error</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > ##### OpenGL API: #####
> > > glGetError

### Native Interface ###
  * **NIMatrix44Read**

> > the current OpenGL matrix. See MatrixMode() to specifiy which matrix stack is the target forsubsequent matrix operations.

## more information ##

> [OpenGL API Documentation](http://www.glprogramming.com/blue/)


---

## class jsgraphics::Transformation ##
- [top](#jsgraphics_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsgraphics/jstransformation.cpp?r=2555) -

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####
> <i><b>constructor</b></i>( [[.md](.md)initToIdentity] )
> > Creates a new Transformation object. If _initToIdentity_ is given and true, the transformation is initialized to identity. Else the transformation is not initialized.

#### <font color='white' size='1'><b>Clear</b></font> ####

> <sub>this</sub> <b>Clear</b>()
> > Reset the current transformation (set to identity).

#### <font color='white' size='1'><b>ClearRotation</b></font> ####

> <sub>this</sub> <b>ClearRotation</b>()
> > Clear the rotation part of the current transformation.

#### <font color='white' size='1'><b>ClearTranslation</b></font> ####

> <sub>this</sub> <b>ClearTranslation</b>()
> > Clear the translation part of the current transformation.

#### <font color='white' size='1'><b>Load</b></font> ####

> <sub>this</sub> <b>Load</b>( matrix )
> > Load a 4x4 matrix as the current transformation.
> > ##### arguments: #####
      1. <sub>value</sub> _matrix_: an Array or an object that supports NIMatrix44Read native interface.

#### <font color='white' size='1'><b>LoadRotation</b></font> ####

> <sub>this</sub> <b>LoadRotation</b>( matrix )
> > Load the rotation part of another matrix to the current matrix.
> > ##### arguments: #####
      1. <sub>value</sub> _matrix_: an Array or an object that supports NIMatrix44Read native interface.

#### <font color='white' size='1'><b>LoadTranslation</b></font> ####

> <sub>this</sub> <b>LoadTranslation</b>( matrix )
> > Load the translation part of another matrix to the current matrix.
> > ##### arguments: #####
      1. <sub>value</sub> _matrix_: an Array or an object that supports NIMatrix44Read native interface.

#### <font color='white' size='1'><b>Translation</b></font> ####

> <sub>this</sub> <b>Translation</b>( x, y, z )
> > Sets the translation part of the current transformation.
> > ##### arguments: #####
      1. <sub>real</sub> _x_
      1. <sub>real</sub> _y_
      1. <sub>real</sub> _z_

#### <font color='white' size='1'><b>Translate</b></font> ####

> <sub>this</sub> <b>Translate</b>( x, y, z )
> > Apply a translation to the current ransformation.
> > ##### arguments: #####
      1. <sub>real</sub> _x_
      1. <sub>real</sub> _y_
      1. <sub>real</sub> _z_

#### <font color='white' size='1'><b>RotationFromQuaternion</b></font> ####

> <sub>this</sub> <b>RotationFromQuaternion</b>( w, x, y, z )
> > Sets the rotation part from a quaternion.
> > ##### arguments: #####
      1. <sub>real</sub> _w_
      1. <sub>real</sub> _x_
      1. <sub>real</sub> _y_
      1. <sub>real</sub> _z_

#### <font color='white' size='1'><b>TaitBryanRotation</b></font> ####

> <sub>this</sub> <b>TaitBryanRotation</b>( roll, pitch, yaw )
> > Sets the Tait-Bryan rotation.
> > ##### arguments: #####
      1. <sub>real</sub> _roll_
      1. <sub>real</sub> _pitch_
      1. <sub>real</sub> _yaw_

#### <font color='white' size='1'><b>Rotation</b></font> ####

> <sub>this</sub> <b>Rotation</b>( angle, x, y, z )
> > Sets the rotation part of the current transformation.
> > ##### arguments: #####
      1. <sub>real</sub> _angle_ in degres
      1. <sub>real</sub> _x_
      1. <sub>real</sub> _y_
      1. <sub>real</sub> _z_

#### <font color='white' size='1'><b>Rotate</b></font> ####

> <sub>this</sub> <b>Rotate</b>( angle, x, y, z )
> > Apply a rotation to the current ransformation.
> > ##### arguments: #####
      1. <sub>real</sub> _angle_ in degres
      1. <sub>real</sub> _x_
      1. <sub>real</sub> _y_
      1. <sub>real</sub> _z_

#### <font color='white' size='1'><b>RotationX</b></font> ####

> <sub>this</sub> <b>RotationX</b>( angle )
> > Set the rotation around the X axis.
> > ##### arguments: #####
      1. <sub>real</sub> _angle_ in degres

#### <font color='white' size='1'><b>RotationY</b></font> ####

> <sub>this</sub> <b>RotationY</b>( angle )
> > Set the rotation around the Y axis.
> > ##### arguments: #####
      1. <sub>real</sub> _angle_ in degres

#### <font color='white' size='1'><b>RotationZ</b></font> ####

> <sub>this</sub> <b>RotationZ</b>( angle )
> > Set the rotation around the Z axis.
> > ##### arguments: #####
      1. <sub>real</sub> _angle_ in degres

#### <font color='white' size='1'><b>LookAt</b></font> ####

> <sub>this</sub> <b>LookAt</b>( x, y, z )
> > unavailable: need to be fixed.
> > ##### arguments: #####
      1. <sub>real</sub> _x_
      1. <sub>real</sub> _y_
      1. <sub>real</sub> _z_

#### <font color='white' size='1'><b>RotateToVector</b></font> ####

> <sub>this</sub> <b>RotateToVector</b>( x, y, z )
> > Apply the (0,0,1)-(x,y,z) angle to the current transformation.

#### <font color='white' size='1'><b>Invert</b></font> ####

> <sub>this</sub> <b>Invert</b>()
> > invert the current transformation.

#### <font color='white' size='1'><b>Product</b></font> ####

> <sub>this</sub> <b>Product</b>( newTransformation )
> > Apply a _newTransformation_ to the current transformation.
> > this = this . new
> > ##### arguments: #####
      1. <sub>value</sub> _newTransformation_: an Array or an object that supports NIMatrix44Read native interface.

#### <font color='white' size='1'><b>ReverseProduct</b></font> ####

> <sub>this</sub> <b>ReverseProduct</b>( otherTransformation )
> > Apply the current transformation to the _otherTransformation_ and stores the result to the current transformation.
> > this = new . this
> > ##### arguments: #####
      1. <sub>value</sub> _otherTransformation_: an Array or an object that supports NIMatrix44Read native interface.

#### <font color='white' size='1'><b>TransformVector</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>TransformVector</b>( vector )
> > Transforms the 3D or 4D _vector_ by the current transformation.
> > ##### arguments: #####
      1. <sub>Array</sub> _vector_

### Properties ###

#### <font color='white' size='1'><i><b><a href='N.md'>N</a> operator</b></i></font> ####

> <sub>real</sub> <i><b><a href='N.md'>N</a> operator</b></i>
> > Get or set a element of the current transformation matrix.
> > ##### example: #####
```
  var t = new Transformation();
  t.Clear();
  t[3] = 1;
  t[7] = 2;
  t[11] = 3;
```

### Native Interface ###
  * **NIMatrix44Read**
> > The current transformation matrix.




---

- [top](#jsgraphics_module.md) - [main](JSLibs.md) -
