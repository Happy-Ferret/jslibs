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

#include "icon.h"

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

#define WINDOW_CLASS_NAME "systray"
#define TRAY_ID 1

#define MSG_TRAY_CALLBACK 1
#define MSG_POPUP_MENU 2

static LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

	switch ( message ) {
		case WM_DESTROY:
			PostQuitMessage(wParam);
			return 0;
		case WM_CLOSE:
			DestroyWindow(hWnd);
			return 0;
		case WM_USER + MSG_TRAY_CALLBACK:
		case WM_COMMAND:
			PostThreadMessage( GetWindowLong(hWnd, GWL_USERDATA), message, wParam, lParam );
			return 0;
	}
	return DefWindowProc(hWnd, message, wParam, lParam); // We do not want to handle this message so pass back to Windows to handle it in a default way
}

struct ThreadPrivateData {

	HWND hWnd;
	HANDLE syncEvent;
	ATOM registredWindowClass;
};

DWORD WINAPI WinThread( LPVOID lpParam ) {

	ThreadPrivateData *tpd = (ThreadPrivateData*)lpParam;
	HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
	HMENU hMenu = CreatePopupMenu(); // doc: A menu that is assigned to a window is automatically destroyed when the application closes.
	HWND hWnd = CreateWindow( (LPSTR)tpd->registredWindowClass, NULL, WS_OVERLAPPED, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, (HWND)NULL, (HMENU)hMenu, hInst, (LPVOID)NULL );
	tpd->hWnd = hWnd; // send back the hWnd value to the constructor
	SetEvent(tpd->syncEvent); // beware: tpd is invalid after this line ( static data )
	for ( MSG msg; GetMessage( &msg, NULL, 0, 0 ) != 0; ) {

		switch ( msg.message ) {
			case WM_USER + MSG_POPUP_MENU:
				{	
				POINT pos;
				GetCursorPos(&pos);
				SetForegroundWindow(hWnd);
				TrackPopupMenuEx(hMenu, GetSystemMetrics(SM_MENUDROPALIGNMENT) | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pos.x, pos.y, hWnd, NULL);
				PostMessage(hWnd, WM_NULL, 0, 0);
				}
				break;
			default:
				TranslateMessage(&msg);
				DispatchMessage(&msg);
		}
	}
	return 0;
}


BEGIN_CLASS( Systray )


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
	tpd.syncEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
	tpd.registredWindowClass = rc;
	
	MSG msg;
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE); // force the system to create the message queue for the current thread
	CreateThread( NULL, 0, WinThread, &tpd, 0, NULL );
	WaitForSingleObject( tpd.syncEvent, INFINITE );
	RT_ASSERT( tpd.hWnd != NULL, "Unable to create the window." );
	SetWindowLong(tpd.hWnd, GWL_USERDATA, GetCurrentThreadId() ); // thread where the messages will be routed
	
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

DEFINE_FINALIZE() {

	NOTIFYICONDATA *nid = (NOTIFYICONDATA*)JS_GetPrivate(cx, obj);
	if ( nid != NULL ) { // if not already closed

		SendMessage( nid->hWnd, WM_CLOSE, 0, 0 ); // PostMessage
//		if ( nid->hIcon != NULL )
//			DestroyIcon( nid->hIcon ); // doc: Before closing, your application must use DestroyIcon to destroy any icon it created by using CreateIconIndirect. It is not necessary to destroy icons created by other functions.
		BOOL status = Shell_NotifyIcon(NIM_DELETE, nid); // (TBD) error check
	}
}

DEFINE_FUNCTION( Close ) {

	NOTIFYICONDATA *nid = (NOTIFYICONDATA*)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(nid);
	SendMessage( nid->hWnd, WM_CLOSE, 0, 0 );
//	if ( nid->hIcon != NULL )
//		DestroyIcon( nid->hIcon ); // doc: Before closing, your application must use DestroyIcon to destroy any icon it created by using CreateIconIndirect. It is not necessary to destroy icons created by other functions.
	BOOL status = Shell_NotifyIcon(NIM_DELETE, nid); // (TBD) error check
	RT_ASSERT( status == TRUE, "Unable to delete notification icon.");
	JS_SetPrivate(cx, obj, NULL);
}


DEFINE_FUNCTION( ProcessEvents ) {

//	NOTIFYICONDATA *nid = (NOTIFYICONDATA*)JS_GetPrivate(cx, obj);
//	RT_ASSERT_RESOURCE(nid);

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
				RT_CHECK_CALL( JS_IdToValue(cx, (jsid)wParam, &key) );
				RT_ASSERT( JS_TypeOfValue( cx, functionVal ) == JSTYPE_FUNCTION, "Need a function." );
				jsval argv[] = { key };
				if ( JS_CallFunctionValue(cx, obj, functionVal, sizeof(argv)/sizeof(*argv), argv, &rval) == JS_FALSE )
					return JS_FALSE;
			}
			return JS_TRUE;
		}

		if ( message == WM_USER + MSG_TRAY_CALLBACK ) {

			switch ( lParam ) {

				case WM_MOUSEMOVE:
					JS_GetProperty(cx, obj, "onmousemove", &functionVal);
					if ( functionVal != JSVAL_VOID ) {

						RT_ASSERT( JS_TypeOfValue( cx, functionVal ) == JSTYPE_FUNCTION, "Need a function." );
						if ( JS_CallFunctionValue(cx, obj, functionVal, 0, NULL, &rval) == JS_FALSE )
							return JS_FALSE;
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
						if ( JS_CallFunctionValue(cx, obj, functionVal, 0, NULL, &rval) == JS_FALSE )
							return JS_FALSE;
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
						if ( JS_CallFunctionValue(cx, obj, functionVal, 0, NULL, &rval) == JS_FALSE )
							return JS_FALSE;
					}
					break;
			}
		}
	}
	return JS_TRUE;
}


/*
DEFINE_PROPERTY( blink ) {
	PNOTIFYICONDATA nid = (PNOTIFYICONDATA)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(nid);


	SetTimer

	nid->uFlags |= NIF_TIP;
	BOOL status = Shell_NotifyIcon(NIM_MODIFY, nid);
	RT_ASSERT( status == TRUE, "Unable to setup systray icon." );
	return JS_TRUE;
}
*/

DEFINE_PROPERTY( icon ) {

	HICON hIcon;

	if ( JSVAL_IS_OBJECT(*vp) && *vp != JSVAL_NULL ) {

		JSObject *iconObj = JSVAL_TO_OBJECT(*vp);
		RT_ASSERT_CLASS( iconObj, &classIcon );

		HICON *phIcon = (HICON*)JS_GetPrivate(cx, iconObj);
		RT_ASSERT_RESOURCE( phIcon );
		hIcon = *phIcon;
	} else
	if ( *vp == JSVAL_NULL || *vp == JSVAL_VOID ) {

		hIcon = NULL;
	} else {

		REPORT_ERROR("Invalid icon.");
	}

	PNOTIFYICONDATA nid = (PNOTIFYICONDATA)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(nid);
	nid->hIcon = hIcon;
	nid->uFlags |= NIF_ICON;
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

		jsval val;
		OBJ_GET_PROPERTY(cx, menuObj, list->vector[i], &val);

		LPCSTR newItem = NULL;
		UINT uFlags = MF_STRING;

		if ( JSVAL_IS_STRING(val) ) {

			RT_JSVAL_TO_STRING( val, newItem );
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
		FUNCTION(Close)
		FUNCTION(ProcessEvents)
		FUNCTION(PopupMenu)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_WRITE_STORE(icon) // _STORE  is needed to keep the reference to the image ( aboid GC )
		PROPERTY_WRITE(text)
		PROPERTY(menu)
//		PROPERTY(blink)
	END_PROPERTY_SPEC

	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1)

END_CLASS


/*
http://msdn.microsoft.com/library/default.asp?url=/library/en-us/shellcc/platform/shell/reference/functions/shell_notifyicon.asp

Icons in Win32:
	http://msdn2.microsoft.com/en-us/library/ms997538.aspx

*/
