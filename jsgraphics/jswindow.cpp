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


BEGIN_CLASS( Window )

//void Finalize(JSContext *cx, JSObject *obj) {
//	JS_GetPrivate(cx, obj);
//}
//static JSBool ClassConstruct(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
//	RT_ASSERT( JS_IsConstructing(cx) && JS_GetClass(obj) == thisClass, RT_ERROR_INVALID_CLASS );
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
			if ( functionVal != JSVAL_VOID ) {

				RT_ASSERT( JS_TypeOfValue( cx, functionVal ) == JSTYPE_FUNCTION, "Need a function." ); // (TBD) return value of assert is not compatible with this function (WndProc)
				char c = wParam;
				jsval argv[] = { STRING_TO_JSVAL(JS_NewStringCopyN(cx, &c, 1)), INT_TO_JSVAL(lParam) };
				JS_CallFunctionValue(cx, obj, functionVal, sizeof(argv)/sizeof(*argv), argv, &rval);
				return 0;
			}
			break;
		case WM_KEYUP:
			JS_GetProperty(cx, obj, "onkeyup", &functionVal);
			if ( functionVal != JSVAL_VOID ) {

				RT_ASSERT( JS_TypeOfValue( cx, functionVal ) == JSTYPE_FUNCTION, "Need a function." );
				jsval argv[] = { INT_TO_JSVAL(wParam), INT_TO_JSVAL(lParam) };
				JS_CallFunctionValue(cx, obj, functionVal, sizeof(argv)/sizeof(*argv), argv, &rval);
				return 0;
			}
			break;
		case WM_KEYDOWN:
			JS_GetProperty(cx, obj, "onkeydown", &functionVal);
			if ( functionVal != JSVAL_VOID ) {

				RT_ASSERT( JS_TypeOfValue( cx, functionVal ) == JSTYPE_FUNCTION, "Need a function." );
				jsval argv[] = { INT_TO_JSVAL(wParam), INT_TO_JSVAL(lParam) };
				JS_CallFunctionValue(cx, obj, functionVal, sizeof(argv)/sizeof(*argv), argv, &rval);
				return 0;
			}
			break;
		case WM_ACTIVATE:
			JS_GetProperty(cx, obj, "onactivate", &functionVal);
			if ( functionVal != JSVAL_VOID ) {

				RT_ASSERT( JS_TypeOfValue( cx, functionVal ) == JSTYPE_FUNCTION, "Need a function." );
				jsval argv[] = { BOOLEAN_TO_JSVAL(wParam != WA_INACTIVE) };
				JS_CallFunctionValue(cx, obj, functionVal, sizeof(argv)/sizeof(*argv), argv, &rval);
				return 0;
			}
			break;
//		case WM_SIZING:
		case WM_SIZE:
			JS_GetProperty(cx, obj, "onsize", &functionVal);
			if ( functionVal != JSVAL_VOID ) {

				RT_ASSERT( JS_TypeOfValue( cx, functionVal ) == JSTYPE_FUNCTION, "Need a function." );
				jsval argv[] = { INT_TO_JSVAL((short)LOWORD(lParam)), INT_TO_JSVAL((short)HIWORD(lParam)) };
				JS_CallFunctionValue(cx, obj, functionVal, sizeof(argv)/sizeof(*argv), argv, &rval);
				return 0;
			}
			break;
		case WM_MOUSEMOVE:
			JS_GetProperty(cx, obj, "onmousemove", &functionVal);
			if ( functionVal != JSVAL_VOID ) {

				RT_ASSERT( JS_TypeOfValue( cx, functionVal ) == JSTYPE_FUNCTION, "Need a function." );
				jsval argv[] = { INT_TO_JSVAL(MAKEPOINTS(lParam).x), INT_TO_JSVAL(MAKEPOINTS(lParam).y), BOOLEAN_TO_JSVAL(wParam & MK_LBUTTON), BOOLEAN_TO_JSVAL(wParam & MK_RBUTTON), BOOLEAN_TO_JSVAL(wParam & MK_MBUTTON) };
				JS_CallFunctionValue(cx, obj, functionVal, sizeof(argv)/sizeof(*argv), argv, &rval);
				return 0;
			}
			break;
		case WM_MOUSEWHEEL:
			JS_GetProperty(cx, obj, "onmousewheel", &functionVal);
			if ( functionVal != JSVAL_VOID ) {

				RT_ASSERT( JS_TypeOfValue( cx, functionVal ) == JSTYPE_FUNCTION, "Need a function." );
				jsval argv[] = { INT_TO_JSVAL( GET_WHEEL_DELTA_WPARAM(wParam)/WHEEL_DELTA ), BOOLEAN_TO_JSVAL(wParam & MK_LBUTTON), BOOLEAN_TO_JSVAL(wParam & MK_RBUTTON), BOOLEAN_TO_JSVAL(wParam & MK_MBUTTON) };
				JS_CallFunctionValue(cx, obj, functionVal, sizeof(argv)/sizeof(*argv), argv, &rval);
				return 0;
			}
			break;
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
			JS_GetProperty(cx, obj, "onmousedown", &functionVal);
			if ( functionVal != JSVAL_VOID ) {

				// xPos = GET_X_LPARAM(lParam);
				// yPos = GET_Y_LPARAM(lParam);

				RT_ASSERT( JS_TypeOfValue( cx, functionVal ) == JSTYPE_FUNCTION, "Need a function." );
				jsval argv[] = {
					INT_TO_JSVAL( message==WM_LBUTTONDOWN ? 1 : message==WM_RBUTTONDOWN ? 2 : message==WM_MBUTTONDOWN ? 3 : 0 ), JSVAL_TRUE };
//					BOOLEAN_TO_JSVAL(wParam & MK_LBUTTON),
//					BOOLEAN_TO_JSVAL(wParam & MK_RBUTTON),
//					BOOLEAN_TO_JSVAL(wParam & MK_MBUTTON) };
				JS_CallFunctionValue(cx, obj, functionVal, sizeof(argv)/sizeof(*argv), argv, &rval);
				return 0;
			}
			break;
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
			JS_GetProperty(cx, obj, "onmouseup", &functionVal);
			if ( functionVal != JSVAL_VOID ) {

				// xPos = GET_X_LPARAM(lParam);
				// yPos = GET_Y_LPARAM(lParam);

				RT_ASSERT( JS_TypeOfValue( cx, functionVal ) == JSTYPE_FUNCTION, "Need a function." );
				jsval argv[] = { INT_TO_JSVAL( message==WM_LBUTTONUP ? 1 : message==WM_RBUTTONUP ? 2 : message==WM_MBUTTONUP ? 3 : 0 ), JSVAL_FALSE };
					JS_CallFunctionValue(cx, obj, functionVal, sizeof(argv)/sizeof(*argv), argv, &rval);
				return 0;
			}
			break;

		//case WM_MOUSELEAVE: // need TrackMouseEvent() ...
		//	JS_GetProperty(cx, obj, "onmouseleave", &functionVal);
		//	if ( functionVal != JSVAL_VOID ) {

		//		JS_CallFunctionValue(cx, obj, functionVal, 0, NULL, &rval);
		//	}
		//	break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam); // We do not want to handle this message so pass back to Windows to handle it in a default way
}

DEFINE_CONSTRUCTOR() {

	RT_ASSERT_CLASS( obj, _class );
	HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
	RT_ASSERT( hInst != NULL, "Unable to GetModuleHandle." );

	// hCursor doc: To use a predefined cursors, the application must set the hInstance parameter to NULL and the lpCursorName parameter to one the cursor values.
	WNDCLASS wc = { CS_HREDRAW | CS_VREDRAW | CS_OWNDC, (WNDPROC)WndProc, 0, 0, hInst, LoadIcon((HINSTANCE)NULL, IDI_APPLICATION), LoadCursor((HINSTANCE) NULL, IDC_ARROW), NULL, NULL, WINDOW_CLASS_NAME };
	ATOM rc = RegisterClass(&wc);
	RT_ASSERT( rc != 0, "Unable to RegisterClass." );

// http://nehe.gamedev.net/data/lessons/lesson.asp?lesson=01
//	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);

	HWND hWnd = CreateWindow( WINDOW_CLASS_NAME, NULL,    WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
	                          CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,    (HWND)NULL, (HMENU)NULL, hInst, (LPVOID)NULL);
	RT_ASSERT( hWnd != NULL, "Unable to CreateWindow." );
	JS_SetPrivate(cx, obj, hWnd);

	CxObj *cxobj = (CxObj*)malloc(sizeof(CxObj));
	cxobj->cx = cx;
	cxobj->obj = obj;

	DWORD err;
	RT_SAFE(SetLastError(0));
	LONG prevWindowLong = SetWindowLong(hWnd, GWL_USERDATA, (LONG)cxobj );
	RT_ASSERT_1( prevWindowLong != 0 || (err=GetLastError()) == 0, "Unable to SetWindowLong. (error %d)", err );
	return JS_TRUE;
}

bool TrackMouseLeave( HWND hWnd ) {

	TRACKMOUSEEVENT trackMouseEvent;
	trackMouseEvent.cbSize = sizeof(TRACKMOUSEEVENT);
	trackMouseEvent.dwFlags = TME_LEAVE;
	trackMouseEvent.hwndTrack = hWnd;
	return TrackMouseEvent(&trackMouseEvent) != 0;
//	RT_ASSERT_1( status != 0, "Unable to TrackMouseEvent.(%d)", GetLastError() );
}

DEFINE_FUNCTION( ProcessEvents ) {

	HWND hWnd = (HWND)JS_GetPrivate(cx, obj);
	RT_ASSERT( hWnd != NULL, RT_ERROR_NOT_INITIALIZED );

	ShowWindow(hWnd, SW_SHOWNORMAL);
	SetForegroundWindow(hWnd);
	UpdateWindow(hWnd);
	SetFocus(hWnd);

	jsval functionVal;
	int msgCount;
	MSG msg;
	bool quit = false;
	do {

		JS_GetProperty(cx, obj, "onidle", &functionVal);

		if ( functionVal != JSVAL_VOID ) {
			RT_ASSERT( JS_TypeOfValue( cx, functionVal ) == JSTYPE_FUNCTION, "Need a function." );
			if ( JS_CallFunctionValue(cx, obj, functionVal, 0, NULL, rval) == JS_FALSE )
				return JS_FALSE;
		}

		RT_SAFE( msgCount = 0 );
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) { //GetInputState() // determines whether there are mouse-button or keyboard messages in the calling thread's message queue.

			if (JS_IsExceptionPending(cx)) // need JS_ErrorFromException(...) ??
				return JS_FALSE;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT) {

				*rval = INT_TO_JSVAL((int)msg.wParam);
				quit = true;
			}
			RT_ASSERT( ++msgCount < 100, "Message loop deadlock detected." );
		}
	} while(!quit);

	// some events can occur after this point, then we NUST keep cxobj as long as possible

	CxObj *cxobj = (CxObj*)GetWindowLong(hWnd, GWL_USERDATA);
	DestroyWindow(hWnd);
//	LONG status = SetWindowLong(hWnd, GWL_USERDATA, (LONG)NULL );  // If the function fails, the return value is zero.
	free(cxobj);
	UnregisterClass(WINDOW_CLASS_NAME, GetModuleHandle(NULL));
	return JS_TRUE;
}


static JSBool Exit(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	PostQuitMessage(0);
	return JS_TRUE;
}


DEFINE_FUNCTION( WaitForMessage ) {

	RT_ASSERT_ARGC(1);
	int32 val;
	JS_ValueToInt32(cx, argv[0], &val);

	DWORD status = MsgWaitForMultipleObjects(0, NULL, FALSE, val, QS_ALLEVENTS);
	*rval = (status == WAIT_TIMEOUT) ? JSVAL_FALSE : JSVAL_TRUE;
	return JS_TRUE;
}


DEFINE_FUNCTION( Mode ) {

	LONG status;
	if ( argc > 0 ) {

		RT_ASSERT_ARGC(3);
		int32 bits;
		JSBool fullscreen;

		int size[2];
		IntArrayToVector(cx, 2, argv, size);
		JS_ValueToInt32(cx, argv[1], &bits);
		JS_ValueToBoolean(cx, argv[2], &fullscreen);

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
	RT_ASSERT_1( status == DISP_CHANGE_SUCCESSFUL, "Unable to ChangeDisplaySettings.(%d)", status);
	return JS_TRUE;
}


DEFINE_PROPERTY( clipCursor ) {

	HWND hWnd = (HWND)JS_GetPrivate(cx, obj);
	RT_ASSERT( hWnd != NULL, RT_ERROR_NOT_INITIALIZED );
	JSBool clip;
	JS_ValueToBoolean(cx, *vp, &clip);
	RECT r;
	GetWindowRect(hWnd, &r);
	BOOL sysStatus = ClipCursor( clip ? &r : NULL );
	RT_ASSERT( sysStatus != 0, "Unable to ClipCursor." );
	return JS_TRUE;
}


DEFINE_PROPERTY( absoluteClipCursor ) {

	BOOL sysStatus;
	if ( *vp != JSVAL_VOID ) {

		int v[4];
		IntArrayToVector(cx, 4, vp, v);

		JSBool clip;
		JS_ValueToBoolean(cx, *vp, &clip);
		RECT r = { v[0], v[1], v[2], v[3] };
		sysStatus = ClipCursor( &r );
	} else {
		sysStatus = ClipCursor( NULL );
	}
	RT_ASSERT( sysStatus != 0, "Unable to ClipCursor." );
	return JS_TRUE;
}



DEFINE_PROPERTY( showCursor ) {

	JSBool show;
	JS_ValueToBoolean(cx, *vp, &show);
	ShowCursor( show ? TRUE : FALSE );
	return JS_TRUE;
}


DEFINE_PROPERTY( rectGetter ) {

	HWND hWnd = (HWND)JS_GetPrivate(cx, obj);
	RT_ASSERT( hWnd != NULL, RT_ERROR_NOT_INITIALIZED );
	RECT r;
	GetWindowRect(hWnd, &r);
	int vector[] = { r.left, r.top, r.right, r.bottom };
	IntVectorToArray(cx, 4, vector, vp);
	return JS_TRUE;
}


DEFINE_PROPERTY( rectSetter ) {

	HWND hWnd = (HWND)JS_GetPrivate(cx, obj);
	RT_ASSERT( hWnd != NULL, RT_ERROR_NOT_INITIALIZED );
	int v[4];
	IntArrayToVector(cx, 4, vp, v);
	SetWindowPos(hWnd, 0, v[0], v[1], v[2] - v[0], v[3] - v[1], SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE);
	return JS_TRUE;
}


DEFINE_PROPERTY( cursorAbsolutePositionSetter ) {

	HWND hWnd = (HWND)JS_GetPrivate(cx, obj);
	RT_ASSERT( hWnd != NULL, RT_ERROR_NOT_INITIALIZED );
	int vec[2];
	IntArrayToVector(cx, 2, vp, vec);
	BOOL sysStatus = SetCursorPos(vec[0], vec[1]); // http://windowssdk.msdn.microsoft.com/en-us/library/ms648394.aspx
	RT_ASSERT( sysStatus != 0, "Unable to SetCursorPos." );
	return JS_TRUE;
}

DEFINE_PROPERTY( cursorAbsolutePositionGetter ) {

	HWND hWnd = (HWND)JS_GetPrivate(cx, obj);
	RT_ASSERT( hWnd != NULL, RT_ERROR_NOT_INITIALIZED );
	POINT pt;
	GetCursorPos( &pt );
	int vector[] = { pt.x, pt.y };
	IntVectorToArray(cx, 4, vector, vp);
	return JS_TRUE;
}

DEFINE_PROPERTY( cursorPositionSetter ) {

	HWND hWnd = (HWND)JS_GetPrivate(cx, obj);
	RT_ASSERT( hWnd != NULL, RT_ERROR_NOT_INITIALIZED );
	int vec[2];
	IntArrayToVector(cx, 2, vp, vec);
	POINT pt = { vec[0], vec[1] };
	ClientToScreen(hWnd, &pt);
	BOOL sysStatus = SetCursorPos(pt.x, pt.y); // http://windowssdk.msdn.microsoft.com/en-us/library/ms648394.aspx
	RT_ASSERT( sysStatus != 0, "Unable to SetCursorPos." );
	return JS_TRUE;
}

DEFINE_PROPERTY( cursorPositionGetter ) {

	HWND hWnd = (HWND)JS_GetPrivate(cx, obj);
	RT_ASSERT( hWnd != NULL, RT_ERROR_NOT_INITIALIZED );
	POINT pt;
	GetCursorPos( &pt );
	ScreenToClient(hWnd, &pt);
	int vector[] = { pt.x, pt.y };
	IntVectorToArray(cx, 4, vector, vp);
	return JS_TRUE;
}

DEFINE_PROPERTY( title ) {

	HWND hWnd = (HWND)JS_GetPrivate(cx, obj);
	RT_ASSERT( hWnd != NULL, RT_ERROR_NOT_INITIALIZED );
	char *title;
	RT_JSVAL_TO_STRING( *vp, title );
	SetWindowText(hWnd, title);
	return JS_TRUE;
}


DEFINE_PROPERTY( showFrame ) {

	HWND hWnd = (HWND)JS_GetPrivate(cx, obj);
	RT_ASSERT( hWnd != NULL, RT_ERROR_NOT_INITIALIZED );

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
	IntVectorToArray(cx, 4, vector, vp);
	return JS_TRUE;
}


DEFINE_PROPERTY( captureMouse ) {

	HWND hWnd = (HWND)JS_GetPrivate(cx, obj);
	RT_ASSERT( hWnd != NULL, RT_ERROR_NOT_INITIALIZED );
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

	HWND hWnd = (HWND)JS_GetPrivate(cx, obj);
	RT_ASSERT( hWnd != NULL, RT_ERROR_NOT_INITIALIZED );
	*vp = BOOLEAN_TO_JSVAL( GetActiveWindow() == hWnd );
	return JS_TRUE;
}

DEFINE_PROPERTY( activeSetter ) {

	HWND hWnd = (HWND)JS_GetPrivate(cx, obj);
	RT_ASSERT( hWnd != NULL, RT_ERROR_NOT_INITIALIZED );
	JSBool active;
	JS_ValueToBoolean(cx, *vp, &active);
	if ( active )
		SetActiveWindow(hWnd);
	return JS_TRUE;
}


CONFIGURE_CLASS

	HAS_CONSTRUCTOR
//	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION(ProcessEvents)
		FUNCTION(Exit)
		FUNCTION(WaitForMessage)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_WRITE_STORE(title)
		PROPERTY_WRITE_STORE(showFrame)
		PROPERTY_WRITE_STORE(captureMouse)
		PROPERTY_WRITE_STORE(clipCursor)
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

	HAS_PRIVATE  // private: BodyID
//	HAS_RESERVED_SLOTS(1)

END_CLASS

