#include "stdafx.h"
#include "jsgl.h"
#include "jswindow.h"

#define _USE_MATH_DEFINES
#include "math.h"

#include "gl/gl.h"
#include "gl/glu.h"

#define SLOT_WINDOW_OBJECT 0

BEGIN_CLASS

static JSBool ClassConstruct(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT( JS_IsConstructing(cx) && JS_GetClass(obj) == thisClass, RT_ERROR_INVALID_CLASS );
	RT_ASSERT_ARGC(1);
	RT_ASSERT_CLASS(JSVAL_TO_OBJECT(argv[0]), &classWindow);

	HWND hWnd = (HWND)JS_GetPrivate(cx, JSVAL_TO_OBJECT(argv[0]));
	RT_ASSERT( hWnd != NULL, RT_ERROR_NOT_INITIALIZED );

	JS_SetReservedSlot(cx, obj, SLOT_WINDOW_OBJECT, argv[0]); // avoid being GC while Gl is in use

	int colorDepth = 32;

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
		32,                              // 16Bit Z-Buffer (Depth Buffer)  
		0,                               // No Stencil Buffer
		0,                               // No Auxiliary Buffer
		PFD_MAIN_PLANE,                  // Main Drawing Layer
		0,                               // Reserved
		0, 0, 0                          // Layer Masks Ignored
	};

	BOOL res;
	HDC hDC = GetDC(hWnd);
	RT_ASSERT( hDC != NULL, "Could not get window Device Context." );

	int PixelFormat = ChoosePixelFormat(hDC, &pfd);
	RT_ASSERT( PixelFormat != 0, "Could not Find A Suitable OpenGL PixelFormat." );

	res = SetPixelFormat(hDC,PixelFormat,&pfd);
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

	RT_ASSERT( JS_GetClass(obj) == thisClass, RT_ERROR_INVALID_CLASS );
	HDC hDC = wglGetCurrentDC();
	RT_ASSERT( hDC != NULL, "Could not get the Current Device Context." );
	BOOL res = SwapBuffers(hDC);
	RT_ASSERT_1( res, "Unable to SwapBuffers.(%x)", GetLastError());
	return JS_TRUE;
}


JSBool Clear(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT( JS_GetClass(obj) == thisClass, RT_ERROR_INVALID_CLASS );
	RT_ASSERT_ARGC(1);
	int32 bitfield;
	JS_ValueToInt32(cx, argv[0], &bitfield);
	glClear(bitfield); // GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
	return JS_TRUE;
}

DEFINE_FUNCTION( Viewport ) {

	RT_ASSERT_ARGC(4);

	int32 x, y, width, height;
	JS_ValueToInt32(cx, argv[0], &x);
	JS_ValueToInt32(cx, argv[1], &y);
	JS_ValueToInt32(cx, argv[2], &width);
	JS_ValueToInt32(cx, argv[3], &height);

	glViewport(x, y, width, height);
	return JS_TRUE;
}

float i =0;

DEFINE_FUNCTION( Test ) {

/*
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);							// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);						// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);							// The Type Of Depth Test To Do

//	glViewport(0, 0, 100, 100);
*/

//	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);			// Really Nice Perspective Calculations

	glClearColor( 0, 0, 0, 0 );
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glShadeModel(GL_SMOOTH);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
//	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity();
  gluPerspective (45.0, 1, 0.1, 1000.0);

/*
	float fovy = 45.0;
	float zNear = 0.01;
	float zFar = 1000;
	float aspect = 1;

	float xmin, xmax, ymin, ymax;
	ymax = zNear * tan(fovy * M_PI / 360.0f);
	ymin = -ymax;
	xmin = ymin * aspect;
	xmax = ymax * aspect;
	glFrustum(xmin, xmax, ymin, ymax, zNear, zFar);
*/
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0, 0, -10.0);

	i += 1;
	glRotatef( i, 1.0, 1.0, 1.0);

	glBegin(GL_TRIANGLES);

		glColor3f( 1, 0, 0 );
		glVertex3f( 0.0f, 1.0f, 0.0f);
		glColor3f( 0, 1, 0 );
		glVertex3f(-1.0f,-1.0f, 0.0f);
		glColor3f( 0, 0, 1 );
		glVertex3f( 1.0f,-1.0f, 0.0f);

		glColor3f( 1, 0, 0 );
		glVertex3f( 0.0f, 1.0f, 0.0f);
		glColor3f( 0, 1, 0 );
		glVertex3f(-1.0f,-1.0f, 0.0f);
		glColor3f( 0, 0, 1 );
		glVertex3f( 0.0f, 0.0f, 1.0f);

	glEnd();

	glFlush();

	return JS_TRUE;
}

BEGIN_FUNCTION_MAP
	FUNCTION_ALIAS(SwapBuffers, _SwapBuffers)
	FUNCTION(Clear)
	FUNCTION(Viewport)
	FUNCTION(Test)
END_MAP

NO_PROPERTY_MAP
//	BEGIN_PROPERTY_MAP
//	END_MAP

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
