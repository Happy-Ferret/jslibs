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

#include "stdafx.h"
#include "systray.h"
#include <stdlib.h>

#include <jsobj.h>


#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

#define WINDOW_CLASS_NAME "systray"
#define TRAY_ID 1

#define MSG_TRAY_CALLBACK 1
#define MSG_FORWARD 2
#define MSG_POPUP_MENU 3
#define MSG_QUIT 4


void WinErrorText() {

  LPVOID lpMsgBuf;
  DWORD result = ::FormatMessage(
    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | 
    FORMAT_MESSAGE_MAX_WIDTH_MASK,
    NULL,
	 GetLastError(),
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
    (LPTSTR) &lpMsgBuf,
    0,
    NULL );

  printf( "%s", lpMsgBuf );
  LocalFree( lpMsgBuf ); // Free the buffer.
}


struct ThreadPrivateData {

	HWND hWnd;
	HANDLE syncEvent;
	DWORD parentThreadId;
	ATOM registredWindowClass;
	HINSTANCE hInstance;
};

static LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	
	if ( message == WM_USER + MSG_TRAY_CALLBACK ) { //  && wParam == (12) + TRAY_ID 
		
		PostMessage(hWnd, WM_USER + MSG_FORWARD, wParam, lParam );
		return 0;
	}
	return DefWindowProc(hWnd, message, wParam, lParam); // We do not want to handle this message so pass back to Windows to handle it in a default way
}


DWORD WINAPI WinThread( LPVOID lpParam ) {

	ThreadPrivateData *tpd = (ThreadPrivateData*)lpParam;

	HMENU hMenu = CreatePopupMenu();
	HWND hWnd = CreateWindow( (LPSTR)tpd->registredWindowClass, NULL, WS_OVERLAPPED, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, (HWND)NULL, (HMENU)hMenu, tpd->hInstance, (LPVOID)NULL );
	DWORD parentThreadId = tpd->parentThreadId;
	tpd->hWnd = hWnd;
	SetEvent(tpd->syncEvent); // beware: tpd is invalid after this line !!!!!!!!!

	MSG msg;
	while ( GetMessage( &msg, NULL, 0, 0 ) != 0 ) {

		switch ( msg.message ) {
			case WM_USER + MSG_QUIT:
				PostQuitMessage(msg.wParam);
				break;
			case WM_USER + MSG_FORWARD:
				PostThreadMessage( parentThreadId, msg.message, msg.wParam, msg.lParam );
				break;
			case WM_USER + MSG_POPUP_MENU:
				{	
				POINT pos;
				GetCursorPos(&pos);
				SetForegroundWindow(hWnd);
				TrackPopupMenuEx(hMenu, GetSystemMetrics(SM_MENUDROPALIGNMENT) | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pos.x, pos.y, hWnd, NULL);
				PostMessage(hWnd, WM_NULL, 0, 0);
				}
				break;
			case WM_COMMAND:
				PostThreadMessage( parentThreadId, msg.message, msg.wParam, msg.lParam );
				break;
			default:
				TranslateMessage(&msg);
				DispatchMessage(&msg);
		}
	}

	DestroyMenu(hMenu);
	DestroyWindow(hWnd);
	return 0;
}


BEGIN_CLASS( Systray )

DEFINE_FINALIZE() {

	NOTIFYICONDATA *nid = (NOTIFYICONDATA*)JS_GetPrivate(cx, obj);
	if ( nid != NULL ) {

		PostMessage( nid->hWnd, WM_USER + MSG_QUIT, 0/*quit*/, 0 );

		//DWORD r = WaitForSingleObject( ???, INFINITE );
		// (TBD) exit the message loop

		if ( nid->hIcon != NULL )
			DestroyIcon( nid->hIcon ); // doc: Before closing, your application must use DestroyIcon to destroy any icon it created by using CreateIconIndirect. It is not necessary to destroy icons created by other functions.
		BOOL status = Shell_NotifyIcon(NIM_DELETE, nid); // (TBD) error check
	}
}


DEFINE_CONSTRUCTOR() {

	RT_ASSERT_CONSTRUCTING(_class);

	HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
	RT_ASSERT( hInst != NULL, "Unable to GetModuleHandle." );
	WNDCLASS wc = { 0, (WNDPROC)WndProc, 0, 0, hInst, NULL, NULL, NULL, NULL, WINDOW_CLASS_NAME };
	ATOM rc = RegisterClass(&wc);
	RT_ASSERT( rc != 0, "Unable to RegisterClass." );

//    OSVERSIONINFO os = { sizeof(os) };
//    GetVersionx(&os);
//    isWin2K = ( VER_PLATFORM_WIN32_NT == os.dwPlatformId && os.dwMajorVersion >= 5 );

	ThreadPrivateData tpd;
	tpd.hWnd = NULL;
	tpd.hInstance = hInst;
	tpd.syncEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
	tpd.parentThreadId = GetCurrentThreadId();
	tpd.registredWindowClass = rc;
	
	MSG msg;
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE); // force the system to create the message queue.
	DWORD dwThreadId;
	HANDLE thread = CreateThread( NULL, 0, WinThread, &tpd, 0, &dwThreadId );
	WaitForSingleObject( tpd.syncEvent, INFINITE );
	
	NOTIFYICONDATA *nid = (NOTIFYICONDATA*)malloc( sizeof(NOTIFYICONDATA) );
	memset(nid, 0, sizeof(NOTIFYICONDATA));
	nid->cbSize = sizeof(NOTIFYICONDATA);
	nid->hWnd = tpd.hWnd;
	nid->uID = (12) + TRAY_ID; // doc: Values from 0 to 12 are reserved and should not be used.
	nid->uFlags = NIF_MESSAGE;
	nid->uCallbackMessage = WM_USER + MSG_TRAY_CALLBACK; // doc: All Message Numbers below 0x0400 are RESERVED.

	BOOL status = Shell_NotifyIcon(NIM_ADD, nid);
	RT_ASSERT( status == TRUE, "Unable to setup systray icon." );

	JS_SetPrivate(cx, obj, (void*)nid);

// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/shellcc/platform/shell/reference/structures/notifyicondata.asp
//	_tcscpy(nid.szTip, "tooltip");

	return JS_TRUE;
}

DEFINE_FUNCTION( ProcessEvents ) {

	NOTIFYICONDATA *nid = (NOTIFYICONDATA*)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(nid);

	MSG msg;
	while ( PeekMessage( &msg, (HWND)-1, 0, 0, PM_REMOVE ) != 0 ) { // doc: If hWnd is -1, PeekMessage retrieves only messages on the current thread's message queue whose hwnd value is NULL

		UINT message = msg.message;
		LPARAM lParam = msg.lParam;
		WPARAM wParam = msg.wParam;
		
		jsval functionVal, rval;

		if ( message == WM_COMMAND ) {

			JS_GetProperty(cx, obj, "oncommand", &functionVal);
			if ( functionVal != JSVAL_VOID ) {

				jsval key;
				RT_ASSERT_RETURN( JS_IdToValue(cx, (jsid)wParam, &key) );
				RT_ASSERT( JS_TypeOfValue( cx, functionVal ) == JSTYPE_FUNCTION, "Need a function." );
				jsval argv[] = { key };
				JS_CallFunctionValue(cx, obj, functionVal, sizeof(argv)/sizeof(*argv), argv, &rval);
			}
			return JS_TRUE;
		}

		if ( message == WM_USER + MSG_FORWARD ) {

			switch ( lParam ) {

				case WM_MOUSEMOVE:
					JS_GetProperty(cx, obj, "onmousemove", &functionVal);
					if ( functionVal != JSVAL_VOID ) {

						RT_ASSERT( JS_TypeOfValue( cx, functionVal ) == JSTYPE_FUNCTION, "Need a function." );
						//	POINT pt;
						//	GetCursorPos( &pt );
						//	ScreenToClient(hWnd, &pt);
						JS_CallFunctionValue(cx, obj, functionVal, 0, NULL, &rval);
					}
					break;
				case WM_LBUTTONDOWN:
				case WM_MBUTTONDOWN:
				case WM_RBUTTONDOWN:
					JS_GetProperty(cx, obj, "onmousedown", &functionVal);
					if ( functionVal != JSVAL_VOID ) {

						RT_ASSERT( JS_TypeOfValue( cx, functionVal ) == JSTYPE_FUNCTION, "Need a function." );
						jsval argv[] = { 
							INT_TO_JSVAL( lParam==WM_LBUTTONDOWN ? 1 : lParam==WM_RBUTTONDOWN ? 2 : lParam==WM_MBUTTONDOWN ? 3 : 0 ), 
							JSVAL_TRUE 
						};
						JS_CallFunctionValue(cx, obj, functionVal, sizeof(argv)/sizeof(*argv), argv, &rval);
					}
					break;
				case WM_LBUTTONUP:
				case WM_MBUTTONUP:
				case WM_RBUTTONUP:
					JS_GetProperty(cx, obj, "onmouseup", &functionVal);
					if ( functionVal != JSVAL_VOID ) {

						RT_ASSERT( JS_TypeOfValue( cx, functionVal ) == JSTYPE_FUNCTION, "Need a function." );
						jsval argv[] = { 
							INT_TO_JSVAL( lParam==WM_LBUTTONUP ? 1 : lParam==WM_RBUTTONUP ? 2 : lParam==WM_MBUTTONUP ? 3 : 0 ), 
							JSVAL_FALSE 
						};
						JS_CallFunctionValue(cx, obj, functionVal, sizeof(argv)/sizeof(*argv), argv, &rval);
					}
					break;
			}
		}
	}

	return JS_TRUE;
}

DEFINE_PROPERTY( icon ) {

	JSObject *imgObj = JSVAL_TO_OBJECT(*vp);

	RT_ASSERT_CLASS_NAME(imgObj, "Image"); // (TBD) need something better/safer ?
	HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);

	jsval tmp;
	JS_GetProperty(cx, imgObj, "width", &tmp);
	RT_ASSERT(JSVAL_IS_INT(tmp), RT_ERROR_UNEXPECTED_TYPE);
	int width = JSVAL_TO_INT(tmp);
	JS_GetProperty(cx, imgObj, "height", &tmp);
	RT_ASSERT(JSVAL_IS_INT(tmp), RT_ERROR_UNEXPECTED_TYPE);
	int height = JSVAL_TO_INT(tmp);
	JS_GetProperty(cx, imgObj, "channels", &tmp);
	RT_ASSERT(JSVAL_IS_INT(tmp), RT_ERROR_UNEXPECTED_TYPE);
	int channels = JSVAL_TO_INT(tmp);
	unsigned char *imageData = (unsigned char*)JS_GetPrivate(cx, imgObj);

	// http://groups.google.com/group/microsoft.public.win32.programmer.gdi/browse_frm/thread/adaf38d715cef81/3825af9edde28cdc?lnk=st&q=RGB+CreateIcon&rnum=9&hl=en#3825af9edde28cdc
	HDC screenDC = GetDC(NULL); // doc: If this value is NULL, GetDC retrieves the DC for the entire screen.
	HDC colorDC = CreateCompatibleDC(screenDC);
	HDC maskDC = CreateCompatibleDC(screenDC);
	HBITMAP colorBMP = CreateCompatibleBitmap(screenDC, width, height);
	HBITMAP maskBMP = CreateCompatibleBitmap(screenDC, width, height);
	HBITMAP oldColorBMP = (HBITMAP)SelectObject(colorDC, colorBMP);
	HBITMAP oldMaskBMP = (HBITMAP)SelectObject(maskDC, maskBMP);

	for ( int x = 0; x < width; x++ )
		for ( int y = 0; y < width; y++ ) {

			unsigned char *offset = imageData + channels*(x + y * width);
			SetPixel(colorDC, x,y, RGB(offset[0],offset[1],offset[2]) );
			if ( channels == 4 ) // image has alpha channel ?
				SetPixel(maskDC, x,y, RGB( 255-offset[3], 255-offset[3], 255-offset[3] ) );
			else
				SetPixel(maskDC, x,y, RGB(0,0,0) );
		}

	SelectObject(colorDC, oldColorBMP); 
	SelectObject(maskDC, oldMaskBMP);
	DeleteDC(colorDC);
	DeleteDC(maskDC);

	ICONINFO ii = { TRUE, 0, 0, maskBMP, colorBMP };
	HICON icon = CreateIconIndirect( &ii );
	DeleteObject(colorBMP);
	DeleteObject(maskBMP); 
	RT_ASSERT( icon != NULL, "Unable to create the icon." );

	PNOTIFYICONDATA nid = (PNOTIFYICONDATA)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(nid);
	nid->uFlags |= NIF_ICON;
	if ( nid->hIcon != NULL ) // free the previous icon
		DestroyIcon( nid->hIcon );
	nid->hIcon = icon;
	BOOL status = Shell_NotifyIcon(NIM_MODIFY, nid);
	RT_ASSERT( status == TRUE, "Unable to setup systray icon." );
	return JS_TRUE;
}


DEFINE_PROPERTY( text ) {

	PNOTIFYICONDATA nid = (PNOTIFYICONDATA)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(nid);

	char *tipText;
	int tipLen;
	RT_JSVAL_TO_STRING_AND_LENGTH( *vp, tipText, tipLen );
	strncpy( nid->szTip, tipText, MIN(sizeof(nid->szTip)-1,tipLen) );
	nid->uFlags |= NIF_TIP;
	BOOL status = Shell_NotifyIcon(NIM_MODIFY, nid);
	RT_ASSERT( status == TRUE, "Unable to setup systray icon." );
	return JS_TRUE;
}


DEFINE_FUNCTION( PopupMenu ) {

	PNOTIFYICONDATA nid = (PNOTIFYICONDATA)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(nid);
	HMENU hMenu = GetMenu(nid->hWnd);
	RT_ASSERT_RESOURCE(hMenu);
	//DestroyMenu(hMenu);
	while ( DeleteMenu(hMenu, 0, MF_BYPOSITION) != 0 ); // remove existing items
	//hMenu = CreatePopupMenu();
	//SetMenu(nid->hWnd, hMenu);

	jsval menu;
	JS_GetReservedSlot(cx, obj, SLOT_SYSTRAY_MENU, &menu);
	JSObject *menuObj = JSVAL_TO_OBJECT(menu);
	JSIdArray *list = JS_Enumerate(cx, menuObj);
	for ( int i = 0; i < list->length; i++ ) {

		jsval key, val;
		OBJ_GET_PROPERTY(cx, menuObj, list->vector[i], &val);

		LPCSTR newItem = NULL;
		UINT uFlags = MF_STRING;

		if ( JSVAL_IS_STRING(val) ) {

			 newItem = JS_GetStringBytes(JS_ValueToString(cx,val));
		} else

		if ( JSVAL_IS_OBJECT(val) && !JSVAL_IS_NULL(val) ) {
			
			JSObject *itemObject = JSVAL_TO_OBJECT(val);
			jsval tmp;
			JS_GetProperty(cx, itemObject, "text", &tmp);
			if ( tmp != JSVAL_VOID )
				RT_JSVAL_TO_STRING( tmp, newItem );
			JS_GetProperty(cx, itemObject, "checked", &tmp);
			if ( tmp == JSVAL_TRUE || tmp == JSVAL_ONE ) // allows to write: s.menu[id].checked ^= 1; // toggle
				uFlags |= MF_CHECKED;
			JS_GetProperty(cx, itemObject, "grayed", &tmp);
			if ( tmp == JSVAL_TRUE || tmp == JSVAL_ONE )
				uFlags |= MF_GRAYED;
			JS_GetProperty(cx, itemObject, "separator", &tmp);
			if ( tmp == JSVAL_TRUE || tmp == JSVAL_ONE )
				uFlags |= MF_SEPARATOR;
		}
		AppendMenu(hMenu, uFlags, list->vector[i], newItem);
	}
	JS_DestroyIdArray(cx, list);
	PostMessage(nid->hWnd, WM_USER + MSG_POPUP_MENU, 0, 0);
	return JS_TRUE;
}


DEFINE_PROPERTY( menuSetter ) {

	JS_SetReservedSlot(cx, obj, SLOT_SYSTRAY_MENU, *vp);
	return JS_TRUE;
}

DEFINE_PROPERTY( menuGetter ) {

	JS_GetReservedSlot(cx, obj, SLOT_SYSTRAY_MENU, vp);
	return JS_TRUE;
}

CONFIGURE_CLASS

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION(ProcessEvents)
		FUNCTION(PopupMenu)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_WRITE(icon) // (TBD) _STORE ? is needed to keep the reference to the image ( aboid GC ) ???
		PROPERTY_WRITE(text)
		PROPERTY(menu)
	END_PROPERTY_SPEC

	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1)

END_CLASS


/*
http://msdn.microsoft.com/library/default.asp?url=/library/en-us/shellcc/platform/shell/reference/functions/shell_notifyicon.asp

Icons in Win32:
	http://msdn2.microsoft.com/en-us/library/ms997538.aspx

*/
