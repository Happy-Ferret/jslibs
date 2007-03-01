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
#define TRAY_ID 1 // doc: Values from 0 to 12 are reserved and should not be used.

#define MSG_TRAY_CALLBACK (WM_USER + 1) // This message has two meanings: tray message + forward
#define MSG_POPUP_MENU (WM_USER + 2)


static HBITMAP MenuItemBitmapFromIcon(HICON hIcon) {

	HDC aHDC = GetDC(NULL);
	HDC aCHDC = CreateCompatibleDC(aHDC);
	int cx = GetSystemMetrics(SM_CXMENUCHECK);
	int cy = GetSystemMetrics(SM_CYMENUCHECK);
	HBITMAP hBMP = CreateCompatibleBitmap(aHDC, cx, cy);
	hBMP = (HBITMAP)SelectObject(aCHDC, hBMP);    
	RECT rect;
	SetRect(&rect, 0, 0, cx, cy); 
	FillRect(aCHDC,&rect,(HBRUSH)GetSysColorBrush(COLOR_WINDOW)); // COLOR_MENU // doc: http://msdn2.microsoft.com/en-us/library/ms724371.aspx
	if( DrawIconEx(aCHDC, 0, 0, hIcon, cx, cy, 0, NULL, DI_NORMAL ) == FALSE ) {

		DeleteObject(hBMP);
		hBMP = 0;
	} else {

		hBMP = (HBITMAP)SelectObject(aCHDC, hBMP);
	}
	DeleteDC(aCHDC);
	ReleaseDC(NULL, aHDC);
	return hBMP;
}

typedef struct MSGInfo {
	HWND        hwnd;
	UINT        message;
	WPARAM      wParam;
	LPARAM      lParam;
	BOOL lButton, rButton, mButton;
	BOOL shiftKey, controlKey, altKey;
} MSGInfo;


static LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

	switch ( message ) {
		case WM_DESTROY:
			PostQuitMessage(wParam);
			return 0;
		case WM_CLOSE:
			DestroyWindow(hWnd);
			return 0;
		case MSG_TRAY_CALLBACK:
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_CHAR:
		case WM_SETFOCUS:
		case WM_KILLFOCUS:
		case WM_COMMAND:
			{
				MSGInfo *msg = (MSGInfo*)malloc(sizeof(MSGInfo));
				BOOL swapButtons = GetSystemMetrics(SM_SWAPBUTTON);
				msg->lButton = GetAsyncKeyState(VK_LBUTTON)&0x8000 == 0x8000;
				msg->rButton = GetAsyncKeyState(VK_RBUTTON)&0x8000 == 0x8000;
				msg->mButton = GetAsyncKeyState(VK_MBUTTON)&0x8000 == 0x8000;
			
				msg->shiftKey = GetAsyncKeyState(VK_SHIFT)&0x8000 == 0x8000;
				msg->controlKey = GetAsyncKeyState(VK_CONTROL)&0x8000 == 0x8000;
				msg->altKey = GetAsyncKeyState(VK_MENU)&0x8000 == 0x8000;

				msg->hwnd = hWnd;
				msg->message = message;
				msg->wParam = wParam;
				msg->lParam = lParam;
				PostThreadMessage( GetWindowLong(hWnd, GWL_USERDATA), MSG_TRAY_CALLBACK, (WPARAM)msg, 0 );
			}
			return 0;
	}
	return DefWindowProc(hWnd, message, wParam, lParam); // We do not want to handle this message so pass back to Windows to handle it in a default way
}

struct ThreadPrivateData {

	HWND hWnd;
	HANDLE syncEvent;
};

DWORD WINAPI WinThread( LPVOID lpParam ) {

	ThreadPrivateData *tpd = (ThreadPrivateData*)lpParam;
	HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
	HMENU hMenu = CreatePopupMenu(); // doc: A menu that is assigned to a window is automatically destroyed when the application closes.
	HWND hWnd = CreateWindow( (LPSTR)WINDOW_CLASS_NAME, NULL, WS_OVERLAPPED, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, (HWND)NULL, (HMENU)hMenu, hInst, (LPVOID)NULL );
	tpd->hWnd = hWnd; // send back the hWnd value to the constructor
	SetEvent(tpd->syncEvent); // beware: tpd is invalid after this line ( static data )
	for ( MSG msg; GetMessage( &msg, NULL, 0, 0 ) != 0; ) {

		switch ( msg.message ) {
			case MSG_POPUP_MENU:
				{	
					POINT pos;
					GetCursorPos(&pos);
					SetForegroundWindow(hWnd);
					TrackPopupMenuEx(hMenu, GetSystemMetrics(SM_MENUDROPALIGNMENT) | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pos.x, pos.y, hWnd, NULL);
					PostMessage(hWnd, WM_NULL, 0, 0);
					// free menu data and menu items
					for ( int i = GetMenuItemCount(hMenu); i > 0; i-- ) {

						MENUITEMINFO mii = { sizeof(MENUITEMINFO) };
						mii.fType = MFT_RADIOCHECK; // doc: Displays selected menu items using a radio-button mark instead of a check mark if the hbmpChecked member is NULL.
						mii.fMask = MIIM_CHECKMARKS; // doc: Retrieves or sets the hbmpChecked and hbmpUnchecked members.
						BOOL st = GetMenuItemInfo(hMenu, 0, TRUE, &mii);
						if ( mii.hbmpChecked != NULL )
							DeleteObject(mii.hbmpChecked);
						DeleteMenu(hMenu, 0, MF_BYPOSITION);
					}
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
	RT_ASSERT( rc != 0 || GetLastError() == ERROR_CLASS_ALREADY_EXISTS, "Unable to RegisterClass." );

//    OSVERSIONINFO os = { sizeof(os) };
//    GetVersionx(&os);
//    isWin2K = ( VER_PLATFORM_WIN32_NT == os.dwPlatformId && os.dwMajorVersion >= 5 );

	ThreadPrivateData tpd;
	tpd.hWnd = NULL;
	tpd.syncEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
	
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
	nid->uID = TRAY_ID;
	nid->uFlags = NIF_MESSAGE;
	nid->uCallbackMessage = MSG_TRAY_CALLBACK; // doc: All Message Numbers below 0x0400 are RESERVED.

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
		BOOL status = Shell_NotifyIcon(NIM_DELETE, nid); // (TBD) error check
	}
}

DEFINE_FUNCTION( Close ) {

	NOTIFYICONDATA *nid = (NOTIFYICONDATA*)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(nid);
	SendMessage( nid->hWnd, WM_CLOSE, 0, 0 );
	BOOL status = Shell_NotifyIcon(NIM_DELETE, nid); // (TBD) error check
	RT_ASSERT( status == TRUE, "Unable to delete notification icon.");
	JS_SetPrivate(cx, obj, NULL);
}


DEFINE_FUNCTION( ProcessEvents ) {

	NOTIFYICONDATA *nid = (NOTIFYICONDATA*)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(nid);

	bool exitBool = false;
	MSG msg;
	while ( PeekMessage( &msg, (HWND)-1, MSG_TRAY_CALLBACK, MSG_TRAY_CALLBACK, PM_REMOVE ) != 0 ) { // doc: If hWnd is -1, PeekMessage retrieves only messages on the current thread's message queue whose hwnd value is NULL

		MSGInfo *trayWndMsg = (MSGInfo*)msg.wParam;
		if ( trayWndMsg->hwnd != nid->hWnd ) {

			PostMessage( msg.hwnd, msg.message, msg.wParam, msg.lParam ); // this message is not for this tray icon
		} else {

			UINT message = trayWndMsg->message;
			LPARAM lParam = trayWndMsg->lParam;
			WPARAM wParam = trayWndMsg->wParam;
			int mButton = trayWndMsg->lButton ? 1 : trayWndMsg->rButton ? 2 : 0;
			free(trayWndMsg);
			jsval functionVal;

			switch ( message ) {
				case WM_SETFOCUS:
					JS_GetProperty(cx, obj, "onfocus", &functionVal);
					if ( functionVal != JSVAL_VOID )
						RT_CHECK_CALL( CallFunction( cx, obj, functionVal, rval, 1, JSVAL_TRUE ) );
					break;
				case WM_KILLFOCUS:
					JS_GetProperty(cx, obj, "onblur", &functionVal);
					if ( functionVal != JSVAL_VOID )
						RT_CHECK_CALL( CallFunction( cx, obj, functionVal, rval, 1, JSVAL_FALSE ) );
					break;
				case WM_CHAR:
					JS_GetProperty(cx, obj, "onchar", &functionVal);
					if ( functionVal != JSVAL_VOID ) {
						
						char c = wParam;
						RT_CHECK_CALL( CallFunction( cx, obj, functionVal, rval, 1, STRING_TO_JSVAL( JS_NewStringCopyN(cx, &c, 1) ) ) );
					}
					break;
				case WM_COMMAND:
					JS_GetProperty(cx, obj, "oncommand", &functionVal);
					if ( functionVal != JSVAL_VOID ) {

						jsval key;
						RT_CHECK_CALL( JS_IdToValue(cx, (jsid)wParam, &key) );
						RT_CHECK_CALL( CallFunction( cx, obj, functionVal, rval, 2, key, INT_TO_JSVAL( mButton ) ) );
					}
					break;
				case MSG_TRAY_CALLBACK:
					switch ( lParam ) {
						case WM_MOUSEMOVE:
							JS_GetProperty(cx, obj, "onmousemove", &functionVal);
							if ( functionVal != JSVAL_VOID )
								RT_CHECK_CALL( CallFunction( cx, obj, functionVal, rval, 0 ) );
							break;
						case WM_LBUTTONDOWN:
						case WM_MBUTTONDOWN:
						case WM_RBUTTONDOWN:
							JS_GetProperty(cx, obj, "onmousedown", &functionVal);
							if ( functionVal != JSVAL_VOID )
								RT_CHECK_CALL( CallFunction( cx, obj, functionVal, rval, 2, INT_TO_JSVAL( lParam==WM_LBUTTONDOWN ? 1 : lParam==WM_RBUTTONDOWN ? 2 : lParam==WM_MBUTTONDOWN ? 3 : 0 ), JSVAL_TRUE ) );
							break;
						case WM_LBUTTONUP:
						case WM_MBUTTONUP:
						case WM_RBUTTONUP:
							JS_GetProperty(cx, obj, "onmouseup", &functionVal);
							if ( functionVal != JSVAL_VOID )
								RT_CHECK_CALL( CallFunction( cx, obj, functionVal, rval, 2, INT_TO_JSVAL( lParam==WM_LBUTTONUP ? 1 : lParam==WM_RBUTTONUP ? 2 : lParam==WM_MBUTTONUP ? 3 : 0 ), JSVAL_FALSE ) );
							break;
						case WM_LBUTTONDBLCLK:
						case WM_MBUTTONDBLCLK:
						case WM_RBUTTONDBLCLK:
							JS_GetProperty(cx, obj, "onmousedblclick", &functionVal);
							if ( functionVal != JSVAL_VOID )
								CallFunction( cx, obj, functionVal, rval, 1, INT_TO_JSVAL( lParam==WM_LBUTTONDBLCLK ? 1 : lParam==WM_RBUTTONDBLCLK ? 2 : lParam==WM_MBUTTONDBLCLK ? 3 : 0 ) );
							break;
					} // switch lParam
			} //  switch message
		}
		JSBool boolValue;
		JS_ValueToBoolean(cx, *rval, &boolValue );
		if ( boolValue == JS_TRUE )
			exitBool = true;
	} // while PeekMessage
	*rval = exitBool ? JSVAL_TRUE : JSVAL_FALSE;
	return JS_TRUE;
}


DEFINE_FUNCTION( Focus ) {

	PNOTIFYICONDATA nid = (PNOTIFYICONDATA)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(nid);
	SetForegroundWindow(nid->hWnd);
	return JS_TRUE;
}


DEFINE_FUNCTION( PopupMenu ) {

	PNOTIFYICONDATA nid = (PNOTIFYICONDATA)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(nid);
	HMENU hMenu = GetMenu(nid->hWnd);
	RT_ASSERT_RESOURCE(hMenu);
	//DestroyMenu(hMenu);
	//hMenu = CreatePopupMenu();
	//SetMenu(nid->hWnd, hMenu);

	jsval menu;
	JS_GetReservedSlot(cx, obj, SLOT_SYSTRAY_MENU, &menu);
	RT_ASSERT_DEFINED(menu);
	JSObject *menuObj = JSVAL_TO_OBJECT(menu);
	JSIdArray *list = JS_Enumerate(cx, menuObj);
	for ( int i = 0; i < list->length; i++ ) {

		jsval val;
		OBJ_GET_PROPERTY(cx, menuObj, list->vector[i], &val);

		LPCSTR newItem = NULL;
		UINT uFlags = MF_STRING;

		if ( JSVAL_IS_STRING(val) ) {

			RT_JSVAL_TO_STRING( val, newItem );
			AppendMenu(hMenu, uFlags, list->vector[i], newItem);
		} else if ( JSVAL_IS_OBJECT(val) && !JSVAL_IS_NULL(val) ) {
			
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

			AppendMenu(hMenu, uFlags, list->vector[i], newItem);

			JS_GetProperty(cx, itemObject, "icon", &tmp); // the menu item bitmap can only be added AFTER the item has been created
			if ( tmp != JSVAL_VOID ) {
				
				JSObject *iconObj = JSVAL_TO_OBJECT(tmp);
				RT_ASSERT_CLASS( iconObj, &classIcon );
				HICON *phIcon = (HICON*)JS_GetPrivate(cx, iconObj);
				RT_ASSERT_RESOURCE(phIcon);
				HBITMAP hBMP = MenuItemBitmapFromIcon(*phIcon);
				RT_ASSERT(hBMP != NULL, "Unable to create the menu item icon.");
				BOOL res = SetMenuItemBitmaps(hMenu, i, MF_BYPOSITION, hBMP, hBMP ); // doc: When the menu is destroyed, these bitmaps are not destroyed; it is up to the application to destroy them.
				RT_ASSERT( res != FALSE, "Unable to SetMenuItemBitmaps." );
			}
		}
	}
	JS_DestroyIdArray(cx, list);
	PostMessage(nid->hWnd, MSG_POPUP_MENU, 0, 0);
	return JS_TRUE;
}


DEFINE_FUNCTION( Flash ) {
/*
	PNOTIFYICONDATA nid = (PNOTIFYICONDATA)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(nid);
	FLASHWINFO fwi = { sizeof(FLASHWINFO) };
	fwi.hwnd = nid->hWnd;
	fwi.dwFlags = FLASHW_TIMER;
	fwi.uCount = 100;
	fwi.dwTimeout = 100;
	BOOL status = FlashWindowEx(&fwi);
//	DWORD e = GetLastError();
//	RT_ASSERT( status == TRUE, "Unable to flash systray icon." );
*/
	return JS_TRUE;
}


DEFINE_PROPERTY( icon ) {

	HICON hIcon;

	if ( JSVAL_IS_OBJECT(*vp) && *vp != JSVAL_NULL ) {

		JSObject *iconObj = JSVAL_TO_OBJECT(*vp);
		RT_ASSERT_CLASS( iconObj, &classIcon );
		HICON *phIcon = (HICON*)JS_GetPrivate(cx, iconObj);
		RT_ASSERT_RESOURCE( phIcon );
		hIcon = *phIcon;
	} else if ( *vp == JSVAL_NULL || *vp == JSVAL_VOID ) {

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

DEFINE_PROPERTY( visible ) {

	JSBool state;
	RT_CHECK_CALL( JS_ValueToBoolean(cx, *vp, &state ) );
	PNOTIFYICONDATA nid = (PNOTIFYICONDATA)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(nid);
	BOOL status = Shell_NotifyIcon( state == JS_TRUE ? NIM_ADD : NIM_DELETE, nid);
	RT_ASSERT( status == TRUE || GetLastError() == NO_ERROR, "Unable to setup systray icon." );
	return JS_TRUE;
}

DEFINE_PROPERTY( textSetter ) {

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

DEFINE_PROPERTY( textGetter ) {

	PNOTIFYICONDATA nid = (PNOTIFYICONDATA)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(nid);
	if ( nid->uFlags & NIF_TIP )
		*vp = STRING_TO_JSVAL( JS_NewStringCopyZ(cx, nid->szTip) );
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
		FUNCTION(Focus)
//		FUNCTION(Flash)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY(menu)
		PROPERTY_WRITE_STORE(icon) // _STORE  is needed to keep the reference to the image ( aboid GC )
		PROPERTY(text)
		PROPERTY_WRITE(visible)
	END_PROPERTY_SPEC

	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1)

END_CLASS


/*
http://msdn.microsoft.com/library/default.asp?url=/library/en-us/shellcc/platform/shell/reference/functions/shell_notifyicon.asp

Icons in Win32:
	http://msdn2.microsoft.com/en-us/library/ms997538.aspx

*/
