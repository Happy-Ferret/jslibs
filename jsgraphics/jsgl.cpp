#include "stdafx.h"
//#include "jsobj.h"

#include "jsgl.h"
#include "jswindow.h"
#include "../common/jsNativeInterface.h"

#include "../jsimage/jsimage.h"

#define _USE_MATH_DEFINES
#include "math.h"

#include "matrix44.h"

#include "gl/gl.h"
#include "gl/glu.h"

#define SLOT_WINDOW_OBJECT 0

BEGIN_CLASS

/*
void SetupBitmapDC() {

	PIXELFORMATDESCRIPTOR pfd ;
	memset(&pfd,0,sizeof(PIXELFORMATDESCRIPTOR)) ;
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR) ;
	pfd.nVersion = 1 ;
	pfd.dwFlags = PFD_DRAW_TO_BITMAP | // replaces PFD_DRAW_TO_WINDOW
					  PFD_SUPPORT_OPENGL |
					  PFD_SUPPORT_GDI ;
	pfd.iPixelType = PFD_TYPE_RGBA ; 
	pfd.cColorBits = 8 ;
	pfd.cDepthBits = 16 ;
	pfd.iLayerType = PFD_MAIN_PLANE ; 


// ChoosePixelFormat  : gives you the number of bits per pixel you asked for

}
*/


DEFINE_FUNCTION( ClassConstruct ) {

	RT_ASSERT( JS_IsConstructing(cx) && JS_GetClass(obj) == thisClass, RT_ERROR_INVALID_CLASS );
	RT_ASSERT_ARGC(1);
	RT_ASSERT_CLASS(JSVAL_TO_OBJECT(argv[0]), &classWindow);

	HWND hWnd = (HWND)JS_GetPrivate(cx, JSVAL_TO_OBJECT(argv[0]));
	RT_ASSERT( hWnd != NULL, RT_ERROR_NOT_INITIALIZED );

	JS_SetReservedSlot(cx, obj, SLOT_WINDOW_OBJECT, argv[0]); // avoid being GC while Gl is in use

	BOOL res;
	HDC hDC = GetDC(hWnd);
	RT_ASSERT( hDC != NULL, "Could not get window Device Context." );

	int colorDepth = 16;

	// PFD_DRAW_TO_BITMAP : http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dnopen/html/msdn_gl6.asp

	PIXELFORMATDESCRIPTOR pfd = {     // pfd Tells Windows How We Want Things To Be
		sizeof(PIXELFORMATDESCRIPTOR),   // Size Of This Pixel Format Descriptor
		1,                               // Version Number
		PFD_DRAW_TO_WINDOW |             // Format Must Support Window
		PFD_SUPPORT_OPENGL |             // Format Must Support OpenGL
		PFD_DOUBLEBUFFER,                // Must Support Double Buffering
		PFD_TYPE_RGBA,                   // Request An RGBA Format
		colorDepth,                      // Select Our Color Depth
		0, 0, 0, 0, 0, 0,                // Color Bits Ignored
		0,                               // No Alpha Buffer
		0,                               // Shift Bit Ignored
		0,                               // No Accumulation Buffer
		0, 0, 0, 0,                      // Accumulation Bits Ignored
		16,                              // 16Bit Z-Buffer (Depth Buffer)  
		0,                               // No Stencil Buffer
		0,                               // No Auxiliary Buffer
		PFD_MAIN_PLANE,                  // Main Drawing Layer
		0,                               // Reserved
		0, 0, 0                          // Layer Masks Ignored
	};


	int pixelFormat = ChoosePixelFormat(hDC, &pfd);
	RT_ASSERT( pixelFormat != 0, "Could not Find A Suitable OpenGL PixelFormat." );

	// If you are using the Win32 interface (as opposed to GLUT), call DescribePixelFormat() and check the returned dwFlags bitfield. 
	// If PFD_GENERIC_ACCELERATED is clear and PFD_GENERIC_FORMAT is set, then the pixel format is only supported by the generic implementation. 
	// Hardware acceleration is not possible for this format. For hardware acceleration, you need to choose a different format.
	DescribePixelFormat(hDC, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd); // [TBD] check return value for error
	bool hasNoAccel = (pfd.dwFlags & PFD_GENERIC_FORMAT) != 0 && (pfd.dwFlags & PFD_GENERIC_ACCELERATED) == 0;
	RT_ASSERT( hasNoAccel == false, "Hardware acceleration is not possible for this format." );

	res = SetPixelFormat(hDC,pixelFormat,&pfd);
	RT_ASSERT( res, "Could not Set The PixelFormat." );

	HGLRC hRC = wglCreateContext(hDC);
	RT_ASSERT_1( hRC != NULL , "Cannot Create A GL Rendering Context. (%x)", GetLastError() );

	res = wglMakeCurrent(hDC,hRC);
	RT_ASSERT_1( res, "Cannot Activate The GL Rendering Context. (%x)", GetLastError());

//  wglMakeCurrent(NULL,NULL); // This step is not required, but it can help find errors, especially when you are using multiple rendering contexts.
//  wglDeleteContext(hRC);

	return JS_TRUE;
}


// The Effects of Double Buffering on Animation Frame Rates
//		http://www.futuretech.blinkenlights.nl/dbuffer.html
static JSBool _SwapBuffers(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	
	glFlush();
	glFinish();
	RT_ASSERT( JS_GetClass(obj) == thisClass, RT_ERROR_INVALID_CLASS );
	HDC hDC = wglGetCurrentDC();
	RT_ASSERT( hDC != NULL, "Could not get the Current Device Context." );
	BOOL res = SwapBuffers(hDC); // Doc: With multithread applications, flush the drawing commands in any other threads drawing to the same window before calling SwapBuffers.
	RT_ASSERT_1( res, "Unable to SwapBuffers.(%x)", GetLastError());
	return JS_TRUE;
}


DEFINE_FUNCTION( Viewport ) {

	RT_ASSERT_ARGC(1);
	int view[4];
	IntArrayToVector(cx, 4, argv, view);
	glViewport(view[0], view[1], view[2], view[3]);
	return JS_TRUE;
}


DEFINE_FUNCTION( Init ) {

	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); // GL_LEQUAL
	glClearDepth(1.0f);
//	glDepthRange( 0.01, 1000 );

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

//	glEnable(GL_LIGHTING);
//	glEnable(GL_LIGHT0);


	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // Really Nice Perspective Calculations

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

//    glBlendFunc(GL_SRC_ALPHA,GL_ONE); 
	return JS_TRUE;
}


DEFINE_FUNCTION( Perspective ) {

	RT_ASSERT_ARGC(3);
	jsdouble fovy, zNear, zFar;
	JS_ValueToNumber(cx, argv[0], &fovy);
	JS_ValueToNumber(cx, argv[1], &zNear);
	JS_ValueToNumber(cx, argv[2], &zFar);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	GLint viewport[4];
	glGetIntegerv( GL_VIEWPORT, viewport );

	//float xmin, xmax, ymin, ymax;
	//ymax = zNear * tan(fovy * M_PI / 360.0f);
	//ymin = -ymax;
	//xmin = ymin * aspect;
	//xmax = ymax * aspect;
	//glFrustum(xmin, xmax, ymin, ymax, zNear, zFar);
	gluPerspective(fovy, float(viewport[2]) / float(viewport[3]), zNear, zFar);
	return JS_TRUE;
}


DEFINE_FUNCTION( Clear ) {

//	RT_ASSERT( JS_GetClass(obj) == thisClass, RT_ERROR_INVALID_CLASS );
	RT_ASSERT_ARGC(1);
	int32 bitfield;
	JS_ValueToInt32(cx, argv[0], &bitfield);
	
	glClear(bitfield); // GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	return JS_TRUE;
}

DEFINE_FUNCTION( LoadMatrix ) {

	RT_ASSERT_ARGC(1);
	RT_ASSERT( JSVAL_IS_OBJECT(argv[0]), "Argument must be an object." );
	NIMatrix44Read ReadMatrix;
	void *descriptor;
	GetNativeInterface(cx, JSVAL_TO_OBJECT(argv[0]), NI_READ_MATRIX44, (void**)&ReadMatrix, &descriptor);
	if ( ReadMatrix != NULL && descriptor != NULL ) {

		Matrix44 m;
		float *tmp = m.raw;
		ReadMatrix(descriptor, &tmp);
		glLoadMatrixf(	tmp );
		return JS_TRUE;
	}
	REPORT_ERROR( "Unable to read a matrix." );
}


DEFINE_FUNCTION( Rotate ) {
	
	RT_ASSERT_ARGC(4);
	jsdouble angle;
	JS_ValueToNumber(cx, argv[0], &angle);
	jsdouble x, y, z;
	JS_ValueToNumber(cx, argv[1], &x);
	JS_ValueToNumber(cx, argv[2], &y);
	JS_ValueToNumber(cx, argv[3], &z);

//	float vec[3];
//	FloatArrayToVector(cx, 3, &argv[1], vec);
//	glRotatef(angle, vec[0], vec[1], vec[2]);
	glRotatef(angle, x, y, z);
	return JS_TRUE;
}


DEFINE_FUNCTION( Translate ) {
	
	RT_ASSERT_ARGC(3);
//	float vec[3];
//	FloatArrayToVector(cx, 3, &argv[0], vec);
//	glTranslatef(vec[0], vec[1], vec[2]);
	jsdouble x, y, z;
	JS_ValueToNumber(cx, argv[0], &x);
	JS_ValueToNumber(cx, argv[1], &y);
	JS_ValueToNumber(cx, argv[2], &z);
	glTranslatef(x,y,z);
	return JS_TRUE;
}


DEFINE_FUNCTION( Axis ) {

	glBegin(GL_LINES);
	glColor3f( 1,0,0 );
	glVertex3i( 0,0,0 );
	glVertex3i( 1,0,0 );
	glColor3f( 0,1,0 );
	glVertex3i( 0,0,0 );
	glVertex3i( 0,1,0 );
	glColor3f( 0,0,1 );
	glVertex3i( 0,0,0 );
	glVertex3i( 0,0,1 );
	glEnd();
	return JS_TRUE;
}


DEFINE_FUNCTION( Quad ) {

	RT_ASSERT_ARGC(4);

	jsdouble x0,y0, x1,y1;
	JS_ValueToNumber(cx, argv[0], &x0);
	JS_ValueToNumber(cx, argv[1], &y0);
	JS_ValueToNumber(cx, argv[2], &x1);
	JS_ValueToNumber(cx, argv[3], &y1);

	glBegin(GL_QUADS);
	glTexCoord2f( 0, 0 ); 
	glVertex2f( x0, y0 );
	glTexCoord2f( 1, 0 ); 
	glVertex2f( x1, y0 );
	glTexCoord2f( 1, 1 ); 
	glVertex2f( x1, y1 );
	glTexCoord2f( 0, 1 ); 
	glVertex2f( x0, y1 );
	glEnd();

	return JS_TRUE;
}

/*
DEFINE_FUNCTION( Color ) {

	GLfloat color[3];
	FloatArrayToVector(cx, 3, argv, color);
	glColor3fv(color);

//	GLint color[3];
//	IntArrayToVector(cx, 3, argv, color);
//	glColor3iv(color);
	return JS_TRUE;
}
*/

DEFINE_FUNCTION( Color ) {

	RT_ASSERT_ARGC(3);

	jsdouble r, g, b;
	JS_ValueToNumber(cx, argv[0], &r);
	JS_ValueToNumber(cx, argv[1], &g);
	JS_ValueToNumber(cx, argv[2], &b);
	glColor3f(r,g,b);
	return JS_TRUE;
}

DEFINE_FUNCTION( Cube ) {

//	RT_ASSERT_ARGC(1);
//	jsdouble len;
//	JS_ValueToNumber(cx, argv[0], &len);

 glBegin(GL_QUADS);		// Draw The Cube Using quads
    glColor3f(0.0f,1.0f,0.0f);	// Color Blue
    glVertex3f( 1.0f, 1.0f,-1.0f);	// Top Right Of The Quad (Top)
    glVertex3f(-1.0f, 1.0f,-1.0f);	// Top Left Of The Quad (Top)
    glVertex3f(-1.0f, 1.0f, 1.0f);	// Bottom Left Of The Quad (Top)
    glVertex3f( 1.0f, 1.0f, 1.0f);	// Bottom Right Of The Quad (Top)
    glColor3f(1.0f,0.5f,0.0f);	// Color Orange
    glVertex3f( 1.0f,-1.0f, 1.0f);	// Top Right Of The Quad (Bottom)
    glVertex3f(-1.0f,-1.0f, 1.0f);	// Top Left Of The Quad (Bottom)
    glVertex3f(-1.0f,-1.0f,-1.0f);	// Bottom Left Of The Quad (Bottom)
    glVertex3f( 1.0f,-1.0f,-1.0f);	// Bottom Right Of The Quad (Bottom)
    glColor3f(1.0f,0.0f,0.0f);	// Color Red	
    glVertex3f( 1.0f, 1.0f, 1.0f);	// Top Right Of The Quad (Front)
    glVertex3f(-1.0f, 1.0f, 1.0f);	// Top Left Of The Quad (Front)
    glVertex3f(-1.0f,-1.0f, 1.0f);	// Bottom Left Of The Quad (Front)
    glVertex3f( 1.0f,-1.0f, 1.0f);	// Bottom Right Of The Quad (Front)
    glColor3f(1.0f,1.0f,0.0f);	// Color Yellow
    glVertex3f( 1.0f,-1.0f,-1.0f);	// Top Right Of The Quad (Back)
    glVertex3f(-1.0f,-1.0f,-1.0f);	// Top Left Of The Quad (Back)
    glVertex3f(-1.0f, 1.0f,-1.0f);	// Bottom Left Of The Quad (Back)
    glVertex3f( 1.0f, 1.0f,-1.0f);	// Bottom Right Of The Quad (Back)
    glColor3f(0.0f,0.0f,1.0f);	// Color Blue
    glVertex3f(-1.0f, 1.0f, 1.0f);	// Top Right Of The Quad (Left)
    glVertex3f(-1.0f, 1.0f,-1.0f);	// Top Left Of The Quad (Left)
    glVertex3f(-1.0f,-1.0f,-1.0f);	// Bottom Left Of The Quad (Left)
    glVertex3f(-1.0f,-1.0f, 1.0f);	// Bottom Right Of The Quad (Left)
    glColor3f(1.0f,0.0f,1.0f);	// Color Violet
    glVertex3f( 1.0f, 1.0f,-1.0f);	// Top Right Of The Quad (Right)
    glVertex3f( 1.0f, 1.0f, 1.0f);	// Top Left Of The Quad (Right)
    glVertex3f( 1.0f,-1.0f, 1.0f);	// Bottom Left Of The Quad (Right)
    glVertex3f( 1.0f,-1.0f,-1.0f);	// Bottom Right Of The Quad (Right)
  glEnd();	

	return JS_TRUE;
}

DEFINE_FUNCTION( Line ) { // 2D

	jsdouble x0,y0, x1,y1;
	JS_ValueToNumber(cx, argv[0], &x0);
	JS_ValueToNumber(cx, argv[1], &y0);
//	JS_ValueToNumber(cx, argv[2], &z0);
	JS_ValueToNumber(cx, argv[3], &x1);
	JS_ValueToNumber(cx, argv[4], &y1);
//	JS_ValueToNumber(cx, argv[5], &z1);

	glBegin(GL_LINES);
	glVertex2f( x0,y0 );
	glVertex2f( x1,y1 );
	glEnd();

	//RT_ASSERT_ARGC(2);
	//float point0[3], point1[3];
	//FloatArrayToVector(cx, 3, &argv[0], point0);
	//FloatArrayToVector(cx, 3, &argv[1], point1);

	//glBegin(GL_LINES);
	//glVertex3i( point0[0], point0[1], point0[2] );
	//glVertex3i( point1[0], point1[1], point1[2] );
	//glEnd();
	return JS_TRUE;
}


DEFINE_FUNCTION( Texture ) {

	RT_ASSERT_ARGC(1);

	// get Image object
	JSObject *image;
	JSBool status = JS_ValueToObject(cx, argv[0], &image);
	RT_ASSERT_CLASS_NAME(image, "Image");
	void *data = JS_GetPrivate(cx, image);
	RT_ASSERT_RESOURCE(data);
	
	int width, height, channels;
	GetIntProperty(cx, image, "width", &width);
	GetIntProperty(cx, image, "height", &height);
	GetIntProperty(cx, image, "channels", &channels);

	GLuint texture;
	glGenTextures( 1, &texture ); // [TBD] free with glDeleteTextures
	glBindTexture( GL_TEXTURE_2D, texture ); // Doc: glBindTexture is included in display lists.
   
	glTexImage2D( GL_TEXTURE_2D, 0, 3, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	*rval = JSVAL_TO_INT(texture);
	return JS_TRUE;
}


DEFINE_FUNCTION( Test ) {
	
	return JS_TRUE;
}


BEGIN_FUNCTION_MAP
	FUNCTION_ALIAS(SwapBuffers, _SwapBuffers)
	FUNCTION(Viewport)
	FUNCTION(Perspective)
	FUNCTION(Clear)
	FUNCTION(Color)
	FUNCTION(Texture)
	FUNCTION(Axis)
	FUNCTION(Cube)
	FUNCTION(Quad)
	FUNCTION(Line)
	FUNCTION(LoadMatrix)
	FUNCTION(Rotate)
	FUNCTION(Translate)
	FUNCTION(Init)
	FUNCTION(Test)
END_MAP

BEGIN_PROPERTY_MAP
END_MAP


NO_STATIC_FUNCTION_MAP
//	BEGIN_STATIC_FUNCTION_MAP
//	END_MAP

NO_STATIC_PROPERTY_MAP
//	BEGIN_STATIC_PROPERTY_MAP
//	END_MAP

//	NO_CLASS_CONSTRUCT
NO_OBJECT_CONSTRUCT
NO_FINALIZE
NO_CALL
NO_PROTOTYPE
NO_CONSTANT_MAP
NO_INITCLASSAUX

END_CLASS(Gl, HAS_PRIVATE, 1/*NO_RESERVED_SLOT*/)

/*


Manage GL extensions:
	http://www.libsdl.org/cgi/viewvc.cgi/trunk/SDL/src/video/win32/SDL_win32opengl.c?view=markup&sortby=date

*/