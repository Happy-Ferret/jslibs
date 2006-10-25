#include "stdafx.h"
#include "jswindow.h"
#include "../smtools/smtools.h"

#include "stdlib.h"


#define WINDOW_CLASS_NAME "jswindow"

// #define MSG_JS_CALL_ERROR 1

#define SLOT_PREV_WINDOW_INFO 0

typedef struct {
	JSContext *cx;
	JSObject *obj;
} CxObj;
	

BEGIN_CLASS

void Finalize(JSContext *cx, JSObject *obj) {

	JS_GetPrivate(cx, obj);
}


//static JSBool ClassConstruct(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
//
//	RT_ASSERT( JS_IsConstructing(cx) && JS_GetClass(obj) == thisClass, RT_ERROR_INVALID_CLASS );
//	return JS_TRUE;
//}

//	JSBool Call(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
//		return JS_TRUE;
//	}


//HINSTANCE GetInst() {
//
//	MEMORY_BASIC_INFORMATION mbi;
//	VirtualQuery(GetInst, &mbi, sizeof(mbi));
//	return (HINSTANCE)mbi.AllocationBase;
//}

JSBool FireEvent( JSContext *cx, JSObject *obj, const char *name, int argc, jsval *argv) {

	jsval rval;
	jsval functionVal;
	JS_GetProperty(cx, obj, name, &functionVal);
	if ( functionVal != JSVAL_VOID )
		return JS_CallFunctionValue(cx, obj, functionVal, argc, argv, &rval);
	return JS_TRUE;
}

static LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

	CxObj *cxobj = (CxObj*)GetWindowLong(hWnd, GWL_USERDATA);
	if ( cxobj == NULL )
		return DefWindowProc(hWnd, message, wParam, lParam);

	JSContext *cx = cxobj->cx;
	JSObject *obj = cxobj->obj;

	switch (message) {

//		 case WM_COMMAND:
//		  // handle menu selections etc.
//		 break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		case WM_CHAR: {
			char c = wParam;
			jsval argv[] = { STRING_TO_JSVAL(JS_NewStringCopyN(cx, &c, 1)), INT_TO_JSVAL(lParam) };
			FireEvent(cx, obj, "onchar", 2, argv );
			break;
		}
		case WM_KEYUP: {
			jsval argv[] = { INT_TO_JSVAL(wParam), INT_TO_JSVAL(lParam) };
			FireEvent(cx, obj, "onkeyup", 2, argv );
			break;
		}
		case WM_KEYDOWN: {
			jsval argv[] = { INT_TO_JSVAL(wParam), INT_TO_JSVAL(lParam) };
			FireEvent(cx, obj, "onkeydown", 2, argv );
			break;
		}
//		case WM_PAINT:
		case WM_MOVE: {
			jsval argv[] = { INT_TO_JSVAL((short)LOWORD(lParam)), INT_TO_JSVAL((short)HIWORD(lParam)) };
			FireEvent(cx, obj, "onmove", 2, argv );
			break;
		}
		case WM_SIZE: {
			jsval argv[] = { INT_TO_JSVAL((short)LOWORD(lParam)), INT_TO_JSVAL((short)HIWORD(lParam)) };
			FireEvent(cx, obj, "onsize", 2, argv );
			break;
		}
		//case WM_MOUSEWHEEL: {
		//	break;
		//}
		case WM_MOUSEMOVE: {
			jsval argv[] = { INT_TO_JSVAL((short)LOWORD(lParam)), INT_TO_JSVAL((short)HIWORD(lParam)), BOOLEAN_TO_JSVAL(wParam & MK_LBUTTON), BOOLEAN_TO_JSVAL(wParam & MK_RBUTTON) };
			FireEvent(cx, obj, "onmousemove", 4, argv );
			break;
		}

		default:
			return DefWindowProc(hWnd, message, wParam, lParam); // We do not want to handle this message so pass back to Windows to handle it in a default way
	}
	return 0;
}


static JSBool ClassConstruct(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT( JS_GetClass(obj) == thisClass, RT_ERROR_INVALID_CLASS );

	RT_ASSERT_ARGC(1);

	HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);

	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC; // OWNDC:Allocates a unique device context for each window in the class.
	wc.lpfnWndProc = (WNDPROC)WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInst;
	wc.hIcon = LoadIcon((HINSTANCE) NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor((HINSTANCE) NULL, IDC_ARROW);
	wc.hbrBackground = NULL; //(HBRUSH) (COLOR_WINDOW +1); //(HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;// "MainMenu";
	wc.lpszClassName = WINDOW_CLASS_NAME;
	wc.hIconSm = NULL;

	ATOM rc = RegisterClassEx(&wc);
	RT_ASSERT( rc != 0, "Unable to RegisterClass." );

	char *windowName;
	RT_JSVAL_TO_STRING(argv[0], windowName);

// http://nehe.gamedev.net/data/lessons/lesson.asp?lesson=01
//	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);

	HWND hWnd = CreateWindowEx( 0, WINDOW_CLASS_NAME, windowName,
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
//		10, 10, 500, 500,
		(HWND) NULL, (HMENU) NULL, hInst, (LPVOID) NULL);

	RT_ASSERT( hWnd != NULL, "Unable to CreateWindow." );
	
	JS_SetPrivate(cx, obj, hWnd);
	SetLastError(0);

	CxObj *cxobj = (CxObj*)malloc(sizeof(CxObj));
	cxobj->cx = cx;
	cxobj->obj = obj;
	LONG prevWindowLong = SetWindowLong(hWnd, GWL_USERDATA, (LONG)cxobj );
	DWORD err;
	RT_ASSERT_1( prevWindowLong != 0 || (err=GetLastError()) == 0, "Unable to SetWindowLong. (error %d)", err );
	return JS_TRUE;
}


static JSBool ProcessEvents(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	HWND hWnd = (HWND)JS_GetPrivate(cx, obj);
	RT_ASSERT( hWnd != NULL, RT_ERROR_NOT_INITIALIZED );

	ShowWindow(hWnd, SW_SHOWNORMAL);
	SetForegroundWindow(hWnd);
	UpdateWindow(hWnd);
	SetFocus(hWnd);

	MSG msg;
	bool quit = false;
	do {

		jsval functionVal;
		JS_GetProperty(cx, obj, "onidle", &functionVal);
		if ( functionVal != JSVAL_VOID )
			JSBool ret = JS_CallFunctionValue(cx, obj, functionVal, 0, NULL, rval);

		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) { //GetInputState() // determines whether there are mouse-button or keyboard messages in the calling thread's message queue.

//			if ( msg.message == WM_USER + MSG_JS_CALL_ERROR )
//				return JS_FALSE;

			if ( JS_IsExceptionPending(cx) ) // need JS_ErrorFromException(...) ??
				return JS_FALSE;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
				quit = true;
		}
	} while(!quit);

	CxObj *cxobj = (CxObj*)GetWindowLong(hWnd, GWL_USERDATA);

//	if ( cxobj != NULL ) // invalid case
	free(cxobj);

	DestroyWindow(hWnd);
	UnregisterClass(WINDOW_CLASS_NAME, GetModuleHandle(NULL));

	return JS_TRUE;
}

static JSBool Exit(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

//	HWND hWnd = (HWND)JS_GetPrivate(cx, obj);
//	RT_ASSERT( hWnd != NULL, RT_ERROR_NOT_INITIALIZED );
	PostQuitMessage(0);
	return JS_TRUE;
}

static JSBool WaitForMessage(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	DWORD status = MsgWaitForMultipleObjects(0, NULL, FALSE, 1, QS_ALLEVENTS);
	*rval = (status == WAIT_TIMEOUT) ? JSVAL_FALSE : JSVAL_TRUE;
	return JS_TRUE;
}

/*
JSBool fullScreenGetter(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	jsval v;
	JS_GetReservedSlot(cx, obj, SLOT_PREV_WINDOW_INFO, &v);
	*vp = (v == JSVAL_VOID) ? JSVAL_FALSE : JSVAL_TRUE;
	return JS_TRUE;
}
*/

DEFINE_PROPERTY( fullScreen ) {

	HWND hWnd = (HWND)JS_GetPrivate(cx, obj);
	RT_ASSERT( hWnd != NULL, RT_ERROR_NOT_INITIALIZED );

	typedef struct {
		RECT prevRect;
		DWORD prevStyle;
	} Prev;

	if ( JSVAL_TO_BOOLEAN(*vp) ) {

		Prev *p = (Prev*)JS_malloc(cx, sizeof(Prev));
		JS_SetReservedSlot(cx, obj, SLOT_PREV_WINDOW_INFO, PRIVATE_TO_JSVAL(p));

		GetWindowRect(hWnd, &p->prevRect);
		p->prevStyle = GetWindowLong(hWnd, GWL_STYLE);

		RECT r;
		GetWindowRect(GetDesktopWindow(), &r);
		DWORD s = GetWindowLong(hWnd, GWL_STYLE);
		s &= ~WS_OVERLAPPEDWINDOW;
		s |= WS_POPUP;
		SetWindowLong(hWnd, GWL_STYLE, s);
		SetWindowPos(hWnd, HWND_TOP, r.left, r.top, r.right-r.left, r.bottom-r.top,  SWP_FRAMECHANGED); //HWND_TOPMOST
		// ClientToScreen(hWnd, &p);
	} else {

		jsval v;
		JS_GetReservedSlot(cx, obj, SLOT_PREV_WINDOW_INFO, &v);
		Prev *p = (Prev*)JSVAL_TO_PRIVATE(v);
		// AdjustWindowRect(&changes, style, FALSE);
		SetWindowLong( hWnd, GWL_STYLE, p->prevStyle );
		SetWindowPos( hWnd, NULL, 
		              p->prevRect.left, p->prevRect.top, p->prevRect.right - p->prevRect.left, p->prevRect.bottom - p->prevRect.top, 
		              SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER ); // SWP_NOSENDCHANGING
		JS_free(cx,p);
		JS_SetReservedSlot(cx, obj, SLOT_PREV_WINDOW_INFO, JSVAL_VOID);
	}
	return JS_TRUE;
}


DEFINE_FUNCTION( Mode ) {

	if ( argc == 0 ) {

		LONG status = ChangeDisplaySettings( NULL, NULL );
		RT_ASSERT_1( status == DISP_CHANGE_SUCCESSFUL, "Unable to ChangeDisplaySettings.(%d)", status);
	} else {
		
		RT_ASSERT_ARGC(3);
		int32 width, height, bits;
		JS_ValueToInt32(cx, argv[0], &width);
		JS_ValueToInt32(cx, argv[1], &height);
		JS_ValueToInt32(cx, argv[2], &bits);
		
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = width;
		dmScreenSettings.dmPelsHeight = height;
		dmScreenSettings.dmBitsPerPel = bits;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		LONG status = ChangeDisplaySettings(&dmScreenSettings, 0); // CDS_FULLSCREEN Gets Rid Of Start Bar. 
		RT_ASSERT_1( status == DISP_CHANGE_SUCCESSFUL, "Unable to ChangeDisplaySettings.(%d)", status);
	}
	return JS_TRUE;
}

DEFINE_PROPERTY( showCursor ) {

	ShowCursor( *vp == JSVAL_TRUE ? TRUE : FALSE );
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
	SetWindowPos(hWnd, 0, v[0], v[1], v[2] - v[0], v[3] - v[1], SWP_NOZORDER | SWP_NOACTIVATE);
	return JS_TRUE;
}


DEFINE_FUNCTION( SetCursorPosition ) {

	HWND hWnd = (HWND)JS_GetPrivate(cx, obj);
	RT_ASSERT( hWnd != NULL, RT_ERROR_NOT_INITIALIZED );

	RT_ASSERT_ARGC(2);
	int32 x, y;
	JS_ValueToInt32(cx, argv[0], &x);
	JS_ValueToInt32(cx, argv[0], &y);
	POINT pt = { x, y };
	ClientToScreen( hWnd, &pt );
	SetCursorPos(pt.x, pt.y); // http://windowssdk.msdn.microsoft.com/en-us/library/ms648394.aspx
	return JS_TRUE;
}


DEFINE_PROPERTY( title ) {

	HWND hWnd = (HWND)JS_GetPrivate(cx, obj);
	RT_ASSERT( hWnd != NULL, RT_ERROR_NOT_INITIALIZED );
	char *title;
	RT_JSVAL_TO_STRING( *vp, title );
    SetWindowText(hWnd, title); // TEXT("")
	return JS_TRUE;
}


///////////

BEGIN_FUNCTION_MAP
//	FUNCTION(Create)
	FUNCTION(ProcessEvents)
	FUNCTION(Exit)
	FUNCTION(WaitForMessage)
	FUNCTION(SetCursorPosition)
END_MAP

BEGIN_PROPERTY_MAP
	SET_AND_STORE(fullScreen)
	SET_AND_STORE(showCursor)
	SET_AND_STORE(title)
	READWRITE( rect )
	//		READONLY(prop)

END_MAP

//NO_STATIC_FUNCTION_MAP
BEGIN_STATIC_FUNCTION_MAP
	FUNCTION(Mode)
END_MAP

NO_STATIC_PROPERTY_MAP
//BEGIN_STATIC_PROPERTY_MAP
//END_MAP

NO_OBJECT_CONSTRUCT
//	NO_CLASS_CONSTRUCT
//NO_FINALIZE
NO_CALL
NO_PROTOTYPE
NO_CONSTANT_MAP
NO_INITCLASSAUX

END_CLASS(Window, HAS_PRIVATE, 1/*NO_RESERVED_SLOT*/)

/*
win32, System Error Codes
	http://msdn.microsoft.com/library/default.asp?url=/library/en-us/debug/base/system_error_codes.asp


http://bob.developpez.com/tutapiwin/index.php

*/