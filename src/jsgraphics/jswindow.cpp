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
win32, System Error Codes
	http://msdn.microsoft.com/library/default.asp?url=/library/en-us/debug/base/system_error_codes.asp

http://bob.developpez.com/tutapiwin/index.php

read this: http://egachine.berlios.de/embedding-sm-best-practice/embedding-sm-best-practice.html
*/

#include "stdafx.h"
#include "jswindow.h"

#define WINDOW_CLASS_NAME "jswindow"

typedef struct {
	JSContext *cx;
	JSObject *obj;
} CxObj;


//HINSTANCE GetInst() {
//
//	MEMORY_BASIC_INFORMATION mbi;
//	VirtualQuery(GetInst, &mbi, sizeof(mbi));
//	return (HINSTANCE)mbi.AllocationBase;
//}


/**XXXdoc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( Window )

//void Finalize(JSContext *cx, JSObject *obj) {
//	JL_GetPrivate(cx, obj);
//}
//static JSBool ClassConstruct(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
//	JL_S_ASSERT( JS_IsConstructing(cx) && JL_GetClass(obj) == thisClass, RT_ERROR_INVALID_CLASS );
//	return JS_TRUE;
//}
//	JSBool Call(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
//		return JS_TRUE;
//	}

static LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

	CxObj *cxobj = (CxObj*)GetWindowLong(hWnd, GWL_USERDATA);
	if ( cxobj == NULL )
		return DefWindowProc(hWnd, message, wParam, lParam);

	JSContext *cx = cxobj->cx;
	JSObject *obj = cxobj->obj;

	jsval rval;
	jsval functionVal;

	LRESULT returnValue = 0;

	switch (message) {

		//case WM_COMMAND:
		//case WM_PAINT

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		case WM_CHAR:
			JS_GetProperty(cx, obj, "onchar", &functionVal);
			if ( !JSVAL_IS_VOID( functionVal ) ) {

				JL_S_ASSERT_FUNCTION(functionVal); // (TBD) return value of assert is not compatible with this function (WndProc)
				char c = wParam;
				jsval argv[] = { STRING_TO_JSVAL(JS_NewStringCopyN(cx, &c, 1)), INT_TO_JSVAL(lParam) };
				JSTempValueRooter tvr;
				JS_PUSH_TEMP_ROOT(cx, COUNTOF(argv), argv, &tvr); // needed to protect the new string
				JSBool status = JS_CallFunctionValue(cx, obj, functionVal, COUNTOF(argv), argv, &rval);
				JS_POP_TEMP_ROOT(cx, &tvr);
				JL_CHK( status );
				return 0;
			}
			break;
		case WM_KEYUP:
			JS_GetProperty(cx, obj, "onkeyup", &functionVal);
			if ( !JSVAL_IS_VOID( functionVal ) ) {

				JL_S_ASSERT_FUNCTION(functionVal); // (TBD) return value of assert is not compatible with this function (WndProc)
				jsval argv[] = { INT_TO_JSVAL(wParam), INT_TO_JSVAL(lParam) };
				JSTempValueRooter tvr;
				JS_PUSH_TEMP_ROOT(cx, COUNTOF(argv), argv, &tvr); // not really needed
				JSBool status = JS_CallFunctionValue(cx, obj, functionVal, COUNTOF(argv), argv, &rval);
				JS_POP_TEMP_ROOT(cx, &tvr);
				JL_CHK( status );
				return 0;
			}
			break;
		case WM_KEYDOWN:
			JS_GetProperty(cx, obj, "onkeydown", &functionVal);
			if ( !JSVAL_IS_VOID( functionVal ) ) {

				JL_S_ASSERT_FUNCTION(functionVal); // (TBD) return value of assert is not compatible with this function (WndProc)
				jsval argv[] = { INT_TO_JSVAL(wParam), INT_TO_JSVAL(lParam) };
				JSTempValueRooter tvr;
				JS_PUSH_TEMP_ROOT(cx, COUNTOF(argv), argv, &tvr); // not really needed
				JSBool status = JS_CallFunctionValue(cx, obj, functionVal, COUNTOF(argv), argv, &rval);
				JS_POP_TEMP_ROOT(cx, &tvr);
				JL_CHK( status );
				return 0;
			}
			break;
		case WM_ACTIVATE:
			JS_GetProperty(cx, obj, "onactivate", &functionVal);
			if ( !JSVAL_IS_VOID( functionVal ) ) {

				JL_S_ASSERT_FUNCTION(functionVal); // (TBD) return value of assert is not compatible with this function (WndProc)
				jsval argv[] = { BOOLEAN_TO_JSVAL(wParam != WA_INACTIVE) };
				JSTempValueRooter tvr;
				JS_PUSH_TEMP_ROOT(cx, COUNTOF(argv), argv, &tvr); // not really needed
				JSBool status = JS_CallFunctionValue(cx, obj, functionVal, COUNTOF(argv), argv, &rval);
				JS_POP_TEMP_ROOT(cx, &tvr);
				JL_CHK( status );
				return 0;
			}
			break;
//		case WM_SIZING:
		case WM_SIZE:
			JS_GetProperty(cx, obj, "onsize", &functionVal);
			if ( !JSVAL_IS_VOID( functionVal ) ) {

				JL_S_ASSERT_FUNCTION(functionVal); // (TBD) return value of assert is not compatible with this function (WndProc)
				jsval argv[] = { INT_TO_JSVAL((short)LOWORD(lParam)), INT_TO_JSVAL((short)HIWORD(lParam)) };
				JSTempValueRooter tvr;
				JS_PUSH_TEMP_ROOT(cx, COUNTOF(argv), argv, &tvr); // not really needed
				JSBool status = JS_CallFunctionValue(cx, obj, functionVal, COUNTOF(argv), argv, &rval);
				JS_POP_TEMP_ROOT(cx, &tvr);
				JL_CHK( status );
				return 0;
			}
			break;
		case WM_MOUSEMOVE:
			JS_GetProperty(cx, obj, "onmousemove", &functionVal);
			if ( !JSVAL_IS_VOID( functionVal ) ) {

				JL_S_ASSERT_FUNCTION(functionVal); // (TBD) return value of assert is not compatible with this function (WndProc)
				jsval argv[] = { INT_TO_JSVAL(MAKEPOINTS(lParam).x), INT_TO_JSVAL(MAKEPOINTS(lParam).y), BOOLEAN_TO_JSVAL(wParam & MK_LBUTTON), BOOLEAN_TO_JSVAL(wParam & MK_RBUTTON), BOOLEAN_TO_JSVAL(wParam & MK_MBUTTON) };
				JSTempValueRooter tvr;
				JS_PUSH_TEMP_ROOT(cx, COUNTOF(argv), argv, &tvr); // not really needed
				JSBool status = JS_CallFunctionValue(cx, obj, functionVal, COUNTOF(argv), argv, &rval);
				JS_POP_TEMP_ROOT(cx, &tvr);
				JL_CHK( status );
				return 0;
			}
			break;
		case WM_MOUSEWHEEL:
			JS_GetProperty(cx, obj, "onmousewheel", &functionVal);
			if ( !JSVAL_IS_VOID( functionVal ) ) {

				JL_S_ASSERT_FUNCTION(functionVal); // (TBD) return value of assert is not compatible with this function (WndProc)
				jsval argv[] = { INT_TO_JSVAL( GET_WHEEL_DELTA_WPARAM(wParam)/WHEEL_DELTA ), BOOLEAN_TO_JSVAL(wParam & MK_LBUTTON), BOOLEAN_TO_JSVAL(wParam & MK_RBUTTON), BOOLEAN_TO_JSVAL(wParam & MK_MBUTTON) };
				JSTempValueRooter tvr;
				JS_PUSH_TEMP_ROOT(cx, COUNTOF(argv), argv, &tvr); // not really needed
				JSBool status = JS_CallFunctionValue(cx, obj, functionVal, COUNTOF(argv), argv, &rval);
				JS_POP_TEMP_ROOT(cx, &tvr);
				JL_CHK( status );
				return 0;
			}
			break;
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
			JS_GetProperty(cx, obj, "onmousedown", &functionVal);
			if ( !JSVAL_IS_VOID( functionVal ) ) {

				// xPos = GET_X_LPARAM(lParam);
				// yPos = GET_Y_LPARAM(lParam);

				JL_S_ASSERT_FUNCTION(functionVal); // (TBD) return value of assert is not compatible with this function (WndProc)
				jsval argv[] = { INT_TO_JSVAL( message==WM_LBUTTONDOWN ? 1 : message==WM_RBUTTONDOWN ? 2 : message==WM_MBUTTONDOWN ? 3 : 0 ), JSVAL_TRUE };
//					BOOLEAN_TO_JSVAL(wParam & MK_LBUTTON),
//					BOOLEAN_TO_JSVAL(wParam & MK_RBUTTON),
//					BOOLEAN_TO_JSVAL(wParam & MK_MBUTTON) };
				JSTempValueRooter tvr;
				JS_PUSH_TEMP_ROOT(cx, COUNTOF(argv), argv, &tvr); // not really needed
				JSBool status = JS_CallFunctionValue(cx, obj, functionVal, COUNTOF(argv), argv, &rval);
				JS_POP_TEMP_ROOT(cx, &tvr);
				JL_CHK( status );
				return 0;
			}
			break;
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
			JS_GetProperty(cx, obj, "onmouseup", &functionVal);
			if ( !JSVAL_IS_VOID( functionVal ) ) {

				// xPos = GET_X_LPARAM(lParam);
				// yPos = GET_Y_LPARAM(lParam);

				JL_S_ASSERT( JS_TypeOfValue( cx, functionVal ) == JSTYPE_FUNCTION, "Need a function." );
				jsval argv[] = { INT_TO_JSVAL( message==WM_LBUTTONUP ? 1 : message==WM_RBUTTONUP ? 2 : message==WM_MBUTTONUP ? 3 : 0 ), JSVAL_FALSE };
				JSTempValueRooter tvr;
				JS_PUSH_TEMP_ROOT(cx, COUNTOF(argv), argv, &tvr); // not really needed
				JSBool status = JS_CallFunctionValue(cx, obj, functionVal, COUNTOF(argv), argv, &rval);
				JS_POP_TEMP_ROOT(cx, &tvr);
				JL_CHK( status );
				return 0;
			}
			break;

		//case WM_MOUSELEAVE: // need TrackMouseEvent() ...
		//	JS_GetProperty(cx, obj, "onmouseleave", &functionVal);
		//	if ( !JSVAL_IS_VOID( functionVal ) ) {

		//		JS_CallFunctionValue(cx, obj, functionVal, 0, NULL, &rval);
		//	}
		//	break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam); // We do not want to handle this message so pass back to Windows to handle it in a default way
}

DEFINE_CONSTRUCTOR() {

//	JSClass *test = JS_GetClass(obj);
	JL_S_ASSERT_CLASS( obj, _class );
	HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
	JL_S_ASSERT( hInst != NULL, "Unable to GetModuleHandle." );

	// hCursor doc: To use a predefined cursors, the application must set the hInstance parameter to NULL and the lpCursorName parameter to one the cursor values.
	WNDCLASS wc = { CS_HREDRAW | CS_VREDRAW | CS_OWNDC, (WNDPROC)WndProc, 0, 0, hInst, LoadIcon((HINSTANCE)NULL, IDI_APPLICATION), LoadCursor((HINSTANCE) NULL, IDC_ARROW), NULL, NULL, WINDOW_CLASS_NAME };
	ATOM rc = RegisterClass(&wc);
	JL_S_ASSERT( rc != 0, "Unable to RegisterClass." );

// http://nehe.gamedev.net/data/lessons/lesson.asp?lesson=01
//	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);

	HWND hWnd = CreateWindow( (LPSTR)rc, NULL,    WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
	                          CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,    (HWND)NULL, (HMENU)NULL, hInst, (LPVOID)NULL);
	JL_S_ASSERT( hWnd != NULL, "Unable to CreateWindow." );
	JL_SetPrivate(cx, obj, hWnd);

	CxObj *cxobj = (CxObj*)jl_malloc(sizeof(CxObj));
	cxobj->cx = cx;
	cxobj->obj = obj;

	DWORD err;
	JL_SAFE(SetLastError(0));
	LONG prevWindowLong = SetWindowLong(hWnd, GWL_USERDATA, (LONG)cxobj );
	JL_S_ASSERT( prevWindowLong != 0 || (err=GetLastError()) == 0, "Unable to SetWindowLong. (error %d)", err );
	return JS_TRUE;
}

bool TrackMouseLeave( HWND hWnd ) {

	TRACKMOUSEEVENT trackMouseEvent;
	trackMouseEvent.cbSize = sizeof(TRACKMOUSEEVENT);
	trackMouseEvent.dwFlags = TME_LEAVE;
	trackMouseEvent.hwndTrack = hWnd;
	return TrackMouseEvent(&trackMouseEvent) != 0;
//	JL_S_ASSERT( status != 0, "Unable to TrackMouseEvent.(%d)", GetLastError() );
}

DEFINE_FUNCTION( Open ) {

	HWND hWnd = (HWND)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( hWnd );

	ShowWindow(hWnd, SW_SHOWNORMAL);
	SetForegroundWindow(hWnd);
	UpdateWindow(hWnd);
	SetFocus(hWnd);
	return JS_TRUE;
}


DEFINE_FUNCTION( ProcessEvents ) {

	HWND hWnd = (HWND)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( hWnd );

//	jsval functionVal;
	int msgCount;
	MSG msg;
//	bool quit = false;
//	do {

/*
		JS_GetProperty(cx, obj, "onidle", &functionVal);
		if ( !JSVAL_IS_VOID( functionVal ) ) {
			JL_S_ASSERT( JS_TypeOfValue( cx, functionVal ) == JSTYPE_FUNCTION, "Need a function." );
			JL_CHK( JS_CallFunctionValue(cx, obj, functionVal, 0, NULL, rval) );
		}
*/

		JL_SAFE( msgCount = 0 );
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) { //GetInputState() // determines whether there are mouse-button or keyboard messages in the calling thread's message queue.

			if (JS_IsExceptionPending(cx)) // need JS_ErrorFromException(...) ??
				return JS_FALSE;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT) {

				*rval = INT_TO_JSVAL((int)msg.wParam);
//				quit = true;
			}
			JL_S_ASSERT( ++msgCount < 100, "Message loop deadlock detected." );
		}

//	} while(!quit);

	return JS_TRUE;
}


DEFINE_FUNCTION( Close ) {

// some events can occur after this point, then we NUST keep cxobj as long as possible

	HWND hWnd = (HWND)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( hWnd );

	CxObj *cxobj = (CxObj*)GetWindowLong(hWnd, GWL_USERDATA);
	DestroyWindow(hWnd);
//	LONG status = SetWindowLong(hWnd, GWL_USERDATA, (LONG)NULL );  // If the function fails, the return value is zero.
	jl_free(cxobj);
	UnregisterClass(WINDOW_CLASS_NAME, GetModuleHandle(NULL));


	return JS_TRUE;
}


/*
static JSBool Exit(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	PostQuitMessage(0);
	return JS_TRUE;
}
*/

DEFINE_FUNCTION( WaitForMessage ) {

	JL_S_ASSERT_ARG_MIN(1);
	int32 val;
	JS_ValueToInt32(cx, argv[0], &val);

	DWORD status = MsgWaitForMultipleObjects(0, NULL, FALSE, val, QS_ALLEVENTS);
	*rval = (status == WAIT_TIMEOUT) ? JSVAL_FALSE : JSVAL_TRUE;
	return JS_TRUE;
}

/*
DEFINE_FUNCTION( CreateOpenGLBitmap ) {


	PIXELFORMATDESCRIPTOR pfd ;
	memset(&pfd,0,sizeof(PIXELFORMATDESCRIPTOR)) ;
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR) ;
	pfd.nVersion = 1 ;
	pfd.dwFlags = PFD_DRAW_TO_BITMAP | PFD_SUPPORT_OPENGL | PFD_SUPPORT_GDI;
	pfd.iPixelType = PFD_TYPE_RGBA ; 
	pfd.cColorBits = 8 ;
	pfd.cDepthBits = 16 ;
	pfd.iLayerType = PFD_MAIN_PLANE ; 

//	char *mem = (char*)jl_malloc( 100 * 100 * 3 * 3 );
//	HBITMAP bitmap = CreateBitmap(100, 100, 3, 32, mem );

	CreateDC( NULL, NULL, NULL, NULL );



	return JS_TRUE;
}
*/


/*
// The Effects of Double Buffering on Animation Frame Rates
//		http://www.futuretech.blinkenlights.nl/dbuffer.html
static JSBool _SwapBuffers(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	glFlush();
	glFinish();
	JL_S_ASSERT( JL_GetClass(obj) == _class, RT_ERROR_INVALID_CLASS );
	HDC hDC = wglGetCurrentDC();
	JL_S_ASSERT( hDC != NULL, "Could not get the Current Device Context." );
	BOOL res = SwapBuffers(hDC); // Doc: With multithread applications, flush the drawing commands in any other threads drawing to the same window before calling SwapBuffers.
	JL_S_ASSERT( res, "Unable to SwapBuffers.(%x)", GetLastError() );
	return JS_TRUE;
}
*/

DEFINE_FUNCTION( CreateOpenGLContext ) {

//	JL_S_ASSERT( JS_IsConstructing(cx) && JL_GetClass(obj) == _class, RT_ERROR_INVALID_CLASS );
//	JL_S_ASSERT_ARG_MIN(1);
//	JL_S_ASSERT_OBJECT(argv[0]);
//	JL_S_ASSERT_CLASS(JSVAL_TO_OBJECT(argv[0]), &classWindow);

	HWND hWnd = (HWND)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( hWnd );

//	JS_SetReservedSlot(cx, obj, SLOT_WINDOW_OBJECT, argv[0]); // avoid being GC while Gl is in use

	BOOL res;
	HDC hDC = GetDC(hWnd);
	JL_S_ASSERT( hDC != NULL, "Could not get window Device Context." );

	// PFD_DRAW_TO_BITMAP : http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dnopen/html/msdn_gl6.asp

	PIXELFORMATDESCRIPTOR pfd = {     // pfd Tells Windows How We Want Things To Be
		sizeof(PIXELFORMATDESCRIPTOR),   // Size Of This Pixel Format Descriptor
		1,                               // Version Number
		PFD_DRAW_TO_WINDOW |             // Format Must Support Window
		PFD_SUPPORT_OPENGL |             // Format Must Support OpenGL
		PFD_DOUBLEBUFFER,                // Must Support Double Buffering
		PFD_TYPE_RGBA,                   // Request An RGBA Format
		32,                              // Select Our Color Depth
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
	JL_S_ASSERT( pixelFormat != 0, "Could not Find A Suitable OpenGL PixelFormat." );

	// If you are using the Win32 interface (as opposed to GLUT), call DescribePixelFormat() and check the returned dwFlags bitfield.
	// If PFD_GENERIC_ACCELERATED is clear and PFD_GENERIC_FORMAT is set, then the pixel format is only supported by the generic implementation.
	// Hardware acceleration is not possible for this format. For hardware acceleration, you need to choose a different format.
	DescribePixelFormat(hDC, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd); // (TBD) check return value for error
	bool hasNoAccel = (pfd.dwFlags & PFD_GENERIC_FORMAT) != 0 && (pfd.dwFlags & PFD_GENERIC_ACCELERATED) == 0;
	JL_S_ASSERT( hasNoAccel == false, "Hardware acceleration is not possible for this format." );

	res = SetPixelFormat(hDC,pixelFormat,&pfd);
	JL_S_ASSERT( res, "Could not Set The PixelFormat." );

	HGLRC hRC = wglCreateContext(hDC);
	JL_S_ASSERT( hRC != NULL , "Cannot Create A GL Rendering Context. (%x)", GetLastError() );

	res = wglMakeCurrent(hDC,hRC);
	JL_S_ASSERT( res, "Cannot Activate The GL Rendering Context. (%x)", GetLastError());

//  wglMakeCurrent(NULL,NULL); // This step is not required, but it can help find errors, especially when you are using multiple rendering contexts.
//  wglDeleteContext(hRC);
	return JS_TRUE;
}


// The Effects of Double Buffering on Animation Frame Rates
//		http://www.futuretech.blinkenlights.nl/dbuffer.html
static JSBool _SwapBuffers(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

//	glFlush();
//	glFinish();
//	JL_S_ASSERT( JL_GetClass(obj) == _class, RT_ERROR_INVALID_CLASS );
	HDC hDC = wglGetCurrentDC(); // (TBD) un-specialize from OpenGL
	JL_S_ASSERT( hDC != NULL, "Could not get the Current Device Context." );
	BOOL res = SwapBuffers(hDC); // Doc: With multithread applications, flush the drawing commands in any other threads drawing to the same window before calling SwapBuffers.
	JL_S_ASSERT( res, "Unable to SwapBuffers.(%x)", GetLastError() );
	return JS_TRUE;
}


DEFINE_FUNCTION( Mode ) {

	LONG status;
	if ( argc > 0 ) {

		JL_S_ASSERT_ARG_MIN(3);
		int32 bits;
		JSBool fullscreen;

		unsigned int size[2];
//		IntArrayToVector(cx, 2, argv, size);
		size_t length;
		JL_CHK( JsvalToUIntVector(cx, argv[0], size, 2, &length) );
		JL_S_ASSERT( length == 2, "Invalid array size." );
		JL_CHK( JS_ValueToInt32(cx, argv[1], &bits) );
		JL_CHK( JS_ValueToBoolean(cx, argv[2], &fullscreen) );

		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = size[0];
		dmScreenSettings.dmPelsHeight = size[1];
		dmScreenSettings.dmBitsPerPel = bits;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// doc: CDS_FULLSCREEN Gets Rid Of Start Bar. The mode is temporary in nature.
		status = ChangeDisplaySettings(&dmScreenSettings, fullscreen == JS_TRUE ? CDS_FULLSCREEN : 0);
	} else {

		status = ChangeDisplaySettings(NULL, 0);
	}
	JL_S_ASSERT( status == DISP_CHANGE_SUCCESSFUL, "Unable to ChangeDisplaySettings.(%d)", status);
	return JS_TRUE;
}


DEFINE_PROPERTY( clipCursor ) {

	HWND hWnd = (HWND)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_INITIALIZED(hWnd);
	JSBool clip;
	JS_ValueToBoolean(cx, *vp, &clip);
	RECT r;
	GetWindowRect(hWnd, &r);
	BOOL sysStatus = ClipCursor( clip ? &r : NULL );
	JL_S_ASSERT( sysStatus != 0, "Unable to ClipCursor." );
	return JS_TRUE;
}


DEFINE_PROPERTY( absoluteClipCursor ) {

	BOOL sysStatus;
	if ( !JSVAL_IS_VOID( *vp ) ) {

		int v[4];
//		IntArrayToVector(cx, 4, vp, v);
		size_t length;
		JL_CHK( JsvalToIntVector(cx, *vp, v, 4, &length) );
		JL_S_ASSERT( length == 4, "Invalid array size." );

		JSBool clip;
		JS_ValueToBoolean(cx, *vp, &clip);
		RECT r = { v[0], v[1], v[2], v[3] };
		sysStatus = ClipCursor( &r );
	} else {
		sysStatus = ClipCursor( NULL );
	}
	JL_S_ASSERT( sysStatus != 0, "Unable to ClipCursor." );
	return JS_TRUE;
}



DEFINE_PROPERTY( showCursor ) {

	JSBool show;
	JS_ValueToBoolean(cx, *vp, &show);
	ShowCursor( show ? TRUE : FALSE );
	return JS_TRUE;
}




DEFINE_PROPERTY( clientRect ) {

	HWND hWnd = (HWND)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(hWnd);
	RECT r;
	GetClientRect(hWnd, &r);

	JSObject *arrayObj;
	if ( !JsvalIsArray(cx, *vp) ) {

		arrayObj = JS_NewArrayObject(cx, 0, NULL);
		JL_CHK(arrayObj);
		*vp = OBJECT_TO_JSVAL(arrayObj);
	} else { // reusing the stored array is a good idea.

		arrayObj = JSVAL_TO_OBJECT(*vp);
	}

	jsval value;
	value = INT_TO_JSVAL(r.left);
	JL_CHK( JS_SetElement(cx, arrayObj, 0, &value) );
	value = INT_TO_JSVAL(r.top);
	JL_CHK( JS_SetElement(cx, arrayObj, 1, &value) );
	value = INT_TO_JSVAL(r.right);
	JL_CHK( JS_SetElement(cx, arrayObj, 2, &value) );
	value = INT_TO_JSVAL(r.bottom);
	JL_CHK( JS_SetElement(cx, arrayObj, 3, &value) );
	return JS_TRUE;
}


DEFINE_PROPERTY( rectGetter ) {

	HWND hWnd = (HWND)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(hWnd);
	RECT r;
	GetWindowRect(hWnd, &r);
	//	IntVectorToArray(cx, COUNTOF(vector), vector, vp);
/*
	if ( J_VALUE_IS_ARRAY(*vp) ) { // reusing the stored array is NOT a good idea, because its reference may have been used elsewere

		JSObject *arrayObj = JSVAL_TO_OBJECT(*vp);
		jsval value;
		value = INT_TO_JSVAL(r.left);
		JL_CHK( JS_SetElement(cx, arrayObj, 0, &value) );
		value = INT_TO_JSVAL(r.top);
		JL_CHK( JS_SetElement(cx, arrayObj, 1, &value) );
		value = INT_TO_JSVAL(r.right);
		JL_CHK( JS_SetElement(cx, arrayObj, 2, &value) );
		value = INT_TO_JSVAL(r.bottom);
		JL_CHK( JS_SetElement(cx, arrayObj, 3, &value) );
	} else {
*/
	int vector[] = { r.left, r.top, r.right, r.bottom };
	JL_CHK( IntVectorToJsval(cx, vector, COUNTOF(vector), vp) );
//	}
	return JS_TRUE;
}


DEFINE_PROPERTY( rectSetter ) {

	HWND hWnd = (HWND)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(hWnd);
	int v[4];

//	IntArrayToVector(cx, 4, vp, v);
	unsigned int length;

//	J_JSVAL_TO_INT_VECTOR(*vp, v, length);
	JL_CHK( JsvalToIntVector(cx, *vp, v, 4, &length) );
	JL_S_ASSERT( length == 4, "Invalid array size." );

	SetWindowPos(hWnd, 0, v[0], v[1], v[2] - v[0], v[3] - v[1], SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE);
	return JS_TRUE;
}


DEFINE_PROPERTY( cursorAbsolutePositionSetter ) {

	HWND hWnd = (HWND)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(hWnd);
	int vec[2];

//	IntArrayToVector(cx, 2, vp, vec);
	size_t length;
	JL_CHK( JsvalToIntVector(cx, *vp, vec, 2, &length) );
	JL_S_ASSERT( length == 2, "Invalid array size." );

	BOOL sysStatus = SetCursorPos(vec[0], vec[1]); // http://windowssdk.msdn.microsoft.com/en-us/library/ms648394.aspx
	JL_S_ASSERT( sysStatus != 0, "Unable to SetCursorPos." );
	return JS_TRUE;
}


DEFINE_PROPERTY( cursorAbsolutePositionGetter ) {

	HWND hWnd = (HWND)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(hWnd);
	POINT pt;
	GetCursorPos( &pt );
	int vector[] = { pt.x, pt.y };
	//IntVectorToArray(cx, COUNTOF(vector), vector, vp);
	JL_CHK( IntVectorToJsval(cx, vector, COUNTOF(vector), vp) );
	return JS_TRUE;
}

DEFINE_PROPERTY( cursorPositionSetter ) {

	HWND hWnd = (HWND)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(hWnd);
	int vec[2];
//	IntArrayToVector(cx, 2, vp, vec);
	size_t length;
	JL_CHK( JsvalToIntVector(cx, *vp, vec, 2, &length) );
	JL_S_ASSERT( length == 2, "Invalid array size." );

	POINT pt = { vec[0], vec[1] };
	ClientToScreen(hWnd, &pt);
	BOOL sysStatus = SetCursorPos(pt.x, pt.y); // http://windowssdk.msdn.microsoft.com/en-us/library/ms648394.aspx
	JL_S_ASSERT( sysStatus != 0, "Unable to SetCursorPos." );
	return JS_TRUE;
}

DEFINE_PROPERTY( cursorPositionGetter ) {

	HWND hWnd = (HWND)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(hWnd);
	POINT pt;
	GetCursorPos( &pt );
	ScreenToClient(hWnd, &pt);
	int vector[] = { pt.x, pt.y };
//	IntVectorToArray(cx, COUNTOF(vector), vector, vp);
	JL_CHK( IntVectorToJsval(cx, vector, COUNTOF(vector), vp) );
	return JS_TRUE;
}

DEFINE_PROPERTY( title ) {

	HWND hWnd = (HWND)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(hWnd);
	const char *title;
	JL_CHK( JsvalToString(cx, vp, &title) );
	SetWindowText(hWnd, title);
	return JS_TRUE;
}


DEFINE_PROPERTY( showFrame ) {

	HWND hWnd = (HWND)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(hWnd);

	JSBool show;
	JS_ValueToBoolean(cx, *vp, &show);

	DWORD s = GetWindowLong(hWnd, GWL_STYLE);
	if ( show ) {
		s &= ~WS_POPUP;
		s |= WS_OVERLAPPEDWINDOW;
	} else {
		s &= ~WS_OVERLAPPEDWINDOW;
		s |= WS_POPUP;
	}
	SetWindowLong(hWnd, GWL_STYLE, s);
	// Certain window data is cached, so changes you make using SetWindowLong will not take effect until you call the SetWindowPos function.
	// Specifically, if you change any of the frame styles, you must call SetWindowPos with the SWP_FRAMECHANGED flag for the cache to be updated properly.
	SetWindowPos(hWnd, HWND_TOP, 0,0,0,0,  SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER); //HWND_TOPMOST
	return JS_TRUE;
}


DEFINE_PROPERTY( desktopRect ) {

	RECT r;
	GetWindowRect(GetDesktopWindow(), &r);
	int vector[] = { r.left, r.top, r.right, r.bottom };
//	IntVectorToArray(cx, COUNTOF(vector), vector, vp);
	JL_CHK( IntVectorToJsval(cx, vector, COUNTOF(vector), vp) );
	return JS_TRUE;
}


DEFINE_PROPERTY( captureMouse ) {

	HWND hWnd = (HWND)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(hWnd);
	JSBool capture;
	JS_ValueToBoolean(cx, *vp, &capture);

	// Only the foreground window can capture the mouse.
	// When a background window attempts to do so, the window receives messages only for mouse events that occur when the cursor hot spot is within the visible portion of the window.
	if ( capture )
		SetCapture(hWnd);
	else
		ReleaseCapture();
	return JS_TRUE;
}

DEFINE_PROPERTY( activeGetter ) {

	HWND hWnd = (HWND)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(hWnd);
	*vp = BOOLEAN_TO_JSVAL( GetActiveWindow() == hWnd );
	return JS_TRUE;
}

DEFINE_PROPERTY( activeSetter ) {

	HWND hWnd = (HWND)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(hWnd);
	JSBool active;
	JS_ValueToBoolean(cx, *vp, &active);
	if ( active )
		SetActiveWindow(hWnd);
	return JS_TRUE;
}


CONFIGURE_CLASS

	HAS_PRIVATE
//	HAS_RESERVED_SLOTS(1)

	HAS_CONSTRUCTOR
//	HAS_FINALIZE // (TBD) create it !

	BEGIN_FUNCTION_SPEC
		FUNCTION(Open)
		FUNCTION(Close)
		FUNCTION(ProcessEvents)
//		FUNCTION(Exit)
		FUNCTION(WaitForMessage)
		FUNCTION(CreateOpenGLContext)
		FUNCTION(SwapBuffers)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_WRITE_STORE(title)
		PROPERTY_WRITE_STORE(showFrame)
		PROPERTY_WRITE_STORE(captureMouse)
		PROPERTY_WRITE_STORE(clipCursor)
		PROPERTY_READ_STORE(clientRect)
		PROPERTY(rect)
		PROPERTY(active)
		PROPERTY(cursorPosition)
	END_PROPERTY_SPEC

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION(Mode)
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_WRITE_STORE(showCursor)
		PROPERTY_READ(desktopRect)
		PROPERTY(cursorAbsolutePosition)
		PROPERTY_WRITE_STORE(absoluteClipCursor)
	END_STATIC_PROPERTY_SPEC

	BEGIN_CONST_INTEGER_SPEC

		CONST_INTEGER_SINGLE(VK_LBUTTON)
		CONST_INTEGER_SINGLE(VK_RBUTTON)
		CONST_INTEGER_SINGLE(VK_CANCEL)
		CONST_INTEGER_SINGLE(VK_MBUTTON)
		#if(_WIN32_WINNT >= 0x0500)
		CONST_INTEGER_SINGLE(VK_XBUTTON1)
		CONST_INTEGER_SINGLE(VK_XBUTTON2)
		#endif /* _WIN32_WINNT >= 0x0500 */
		CONST_INTEGER_SINGLE(VK_BACK)
		CONST_INTEGER_SINGLE(VK_TAB)
		CONST_INTEGER_SINGLE(VK_CLEAR)
		CONST_INTEGER_SINGLE(VK_RETURN)
		CONST_INTEGER_SINGLE(VK_SHIFT)
		CONST_INTEGER_SINGLE(VK_CONTROL)
		CONST_INTEGER_SINGLE(VK_MENU)
		CONST_INTEGER_SINGLE(VK_PAUSE)
		CONST_INTEGER_SINGLE(VK_CAPITAL)
		CONST_INTEGER_SINGLE(VK_KANA)
		CONST_INTEGER_SINGLE(VK_HANGEUL)
		CONST_INTEGER_SINGLE(VK_HANGUL)
		CONST_INTEGER_SINGLE(VK_JUNJA)
		CONST_INTEGER_SINGLE(VK_FINAL)
		CONST_INTEGER_SINGLE(VK_HANJA)
		CONST_INTEGER_SINGLE(VK_KANJI)
		CONST_INTEGER_SINGLE(VK_ESCAPE)
		CONST_INTEGER_SINGLE(VK_CONVERT)
		CONST_INTEGER_SINGLE(VK_NONCONVERT)
		CONST_INTEGER_SINGLE(VK_ACCEPT)
		CONST_INTEGER_SINGLE(VK_MODECHANGE)
		CONST_INTEGER_SINGLE(VK_SPACE)
		CONST_INTEGER_SINGLE(VK_PRIOR)
		CONST_INTEGER_SINGLE(VK_NEXT)
		CONST_INTEGER_SINGLE(VK_END)
		CONST_INTEGER_SINGLE(VK_HOME)
		CONST_INTEGER_SINGLE(VK_LEFT)
		CONST_INTEGER_SINGLE(VK_UP)
		CONST_INTEGER_SINGLE(VK_RIGHT)
		CONST_INTEGER_SINGLE(VK_DOWN)
		CONST_INTEGER_SINGLE(VK_SELECT)
		CONST_INTEGER_SINGLE(VK_PRINT)
		CONST_INTEGER_SINGLE(VK_EXECUTE)
		CONST_INTEGER_SINGLE(VK_SNAPSHOT)
		CONST_INTEGER_SINGLE(VK_INSERT)
		CONST_INTEGER_SINGLE(VK_DELETE)
		CONST_INTEGER_SINGLE(VK_HELP)
		CONST_INTEGER_SINGLE(VK_LWIN)
		CONST_INTEGER_SINGLE(VK_RWIN)
		CONST_INTEGER_SINGLE(VK_APPS)
		CONST_INTEGER_SINGLE(VK_SLEEP)
		CONST_INTEGER_SINGLE(VK_NUMPAD0)
		CONST_INTEGER_SINGLE(VK_NUMPAD1)
		CONST_INTEGER_SINGLE(VK_NUMPAD2)
		CONST_INTEGER_SINGLE(VK_NUMPAD3)
		CONST_INTEGER_SINGLE(VK_NUMPAD4)
		CONST_INTEGER_SINGLE(VK_NUMPAD5)
		CONST_INTEGER_SINGLE(VK_NUMPAD6)
		CONST_INTEGER_SINGLE(VK_NUMPAD7)
		CONST_INTEGER_SINGLE(VK_NUMPAD8)
		CONST_INTEGER_SINGLE(VK_NUMPAD9)
		CONST_INTEGER_SINGLE(VK_MULTIPLY)
		CONST_INTEGER_SINGLE(VK_ADD)
		CONST_INTEGER_SINGLE(VK_SEPARATOR)
		CONST_INTEGER_SINGLE(VK_SUBTRACT)
		CONST_INTEGER_SINGLE(VK_DECIMAL)
		CONST_INTEGER_SINGLE(VK_DIVIDE)
		CONST_INTEGER_SINGLE(VK_F1)
		CONST_INTEGER_SINGLE(VK_F2)
		CONST_INTEGER_SINGLE(VK_F3)
		CONST_INTEGER_SINGLE(VK_F4)
		CONST_INTEGER_SINGLE(VK_F5)
		CONST_INTEGER_SINGLE(VK_F6)
		CONST_INTEGER_SINGLE(VK_F7)
		CONST_INTEGER_SINGLE(VK_F8)
		CONST_INTEGER_SINGLE(VK_F9)
		CONST_INTEGER_SINGLE(VK_F10)
		CONST_INTEGER_SINGLE(VK_F11)
		CONST_INTEGER_SINGLE(VK_F12)
		CONST_INTEGER_SINGLE(VK_F13)
		CONST_INTEGER_SINGLE(VK_F14)
		CONST_INTEGER_SINGLE(VK_F15)
		CONST_INTEGER_SINGLE(VK_F16)
		CONST_INTEGER_SINGLE(VK_F17)
		CONST_INTEGER_SINGLE(VK_F18)
		CONST_INTEGER_SINGLE(VK_F19)
		CONST_INTEGER_SINGLE(VK_F20)
		CONST_INTEGER_SINGLE(VK_F21)
		CONST_INTEGER_SINGLE(VK_F22)
		CONST_INTEGER_SINGLE(VK_F23)
		CONST_INTEGER_SINGLE(VK_F24)
		CONST_INTEGER_SINGLE(VK_NUMLOCK)
		CONST_INTEGER_SINGLE(VK_SCROLL)
		CONST_INTEGER_SINGLE(VK_OEM_NEC_EQUAL)
		CONST_INTEGER_SINGLE(VK_OEM_FJ_JISHO)
		CONST_INTEGER_SINGLE(VK_OEM_FJ_MASSHOU)
		CONST_INTEGER_SINGLE(VK_OEM_FJ_TOUROKU)
		CONST_INTEGER_SINGLE(VK_OEM_FJ_LOYA)
		CONST_INTEGER_SINGLE(VK_OEM_FJ_ROYA)
		CONST_INTEGER_SINGLE(VK_LSHIFT)
		CONST_INTEGER_SINGLE(VK_RSHIFT)
		CONST_INTEGER_SINGLE(VK_LCONTROL)
		CONST_INTEGER_SINGLE(VK_RCONTROL)
		CONST_INTEGER_SINGLE(VK_LMENU)
		CONST_INTEGER_SINGLE(VK_RMENU)
		#if(_WIN32_WINNT >= 0x0500)
		CONST_INTEGER_SINGLE(VK_BROWSER_BACK)
		CONST_INTEGER_SINGLE(VK_BROWSER_FORWARD)
		CONST_INTEGER_SINGLE(VK_BROWSER_REFRESH)
		CONST_INTEGER_SINGLE(VK_BROWSER_STOP)
		CONST_INTEGER_SINGLE(VK_BROWSER_SEARCH)
		CONST_INTEGER_SINGLE(VK_BROWSER_FAVORITES)
		CONST_INTEGER_SINGLE(VK_BROWSER_HOME)
		CONST_INTEGER_SINGLE(VK_VOLUME_MUTE)
		CONST_INTEGER_SINGLE(VK_VOLUME_DOWN)
		CONST_INTEGER_SINGLE(VK_VOLUME_UP)
		CONST_INTEGER_SINGLE(VK_MEDIA_NEXT_TRACK)
		CONST_INTEGER_SINGLE(VK_MEDIA_PREV_TRACK)
		CONST_INTEGER_SINGLE(VK_MEDIA_STOP)
		CONST_INTEGER_SINGLE(VK_MEDIA_PLAY_PAUSE)
		CONST_INTEGER_SINGLE(VK_LAUNCH_MAIL)
		CONST_INTEGER_SINGLE(VK_LAUNCH_MEDIA_SELECT)
		CONST_INTEGER_SINGLE(VK_LAUNCH_APP1)
		CONST_INTEGER_SINGLE(VK_LAUNCH_APP2)
		#endif /* _WIN32_WINNT >= 0x0500 */
		CONST_INTEGER_SINGLE(VK_OEM_1)
		CONST_INTEGER_SINGLE(VK_OEM_PLUS)
		CONST_INTEGER_SINGLE(VK_OEM_COMMA)
		CONST_INTEGER_SINGLE(VK_OEM_MINUS)
		CONST_INTEGER_SINGLE(VK_OEM_PERIOD)
		CONST_INTEGER_SINGLE(VK_OEM_2)
		CONST_INTEGER_SINGLE(VK_OEM_3)
		CONST_INTEGER_SINGLE(VK_OEM_4)
		CONST_INTEGER_SINGLE(VK_OEM_5)
		CONST_INTEGER_SINGLE(VK_OEM_6)
		CONST_INTEGER_SINGLE(VK_OEM_7)
		CONST_INTEGER_SINGLE(VK_OEM_8)
		CONST_INTEGER_SINGLE(VK_OEM_AX)
		CONST_INTEGER_SINGLE(VK_OEM_102)
		CONST_INTEGER_SINGLE(VK_ICO_HELP)
		CONST_INTEGER_SINGLE(VK_ICO_00)
		#if(WINVER >= 0x0400)
		CONST_INTEGER_SINGLE(VK_PROCESSKEY)
		#endif /* WINVER >= 0x0400 */
		CONST_INTEGER_SINGLE(VK_ICO_CLEAR)
		#if(_WIN32_WINNT >= 0x0500)
		CONST_INTEGER_SINGLE(VK_PACKET)
		#endif /* _WIN32_WINNT >= 0x0500 */
		CONST_INTEGER_SINGLE(VK_OEM_RESET)
		CONST_INTEGER_SINGLE(VK_OEM_JUMP)
		CONST_INTEGER_SINGLE(VK_OEM_PA1)
		CONST_INTEGER_SINGLE(VK_OEM_PA2)
		CONST_INTEGER_SINGLE(VK_OEM_PA3)
		CONST_INTEGER_SINGLE(VK_OEM_WSCTRL)
		CONST_INTEGER_SINGLE(VK_OEM_CUSEL)
		CONST_INTEGER_SINGLE(VK_OEM_ATTN)
		CONST_INTEGER_SINGLE(VK_OEM_FINISH)
		CONST_INTEGER_SINGLE(VK_OEM_COPY)
		CONST_INTEGER_SINGLE(VK_OEM_AUTO)
		CONST_INTEGER_SINGLE(VK_OEM_ENLW)
		CONST_INTEGER_SINGLE(VK_OEM_BACKTAB)
		CONST_INTEGER_SINGLE(VK_ATTN)
		CONST_INTEGER_SINGLE(VK_CRSEL)
		CONST_INTEGER_SINGLE(VK_EXSEL)
		CONST_INTEGER_SINGLE(VK_EREOF)
		CONST_INTEGER_SINGLE(VK_PLAY)
		CONST_INTEGER_SINGLE(VK_ZOOM)
		CONST_INTEGER_SINGLE(VK_NONAME)
		CONST_INTEGER_SINGLE(VK_PA1)
		CONST_INTEGER_SINGLE(VK_OEM_CLEAR)
	END_CONST_INTEGER_SPEC

END_CLASS

