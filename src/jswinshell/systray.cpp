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
#include "error.h"
#include "systray.h"
#include <stdlib.h>

#include <jsobj.h>

#include "icon.h"

#include <commctrl.h>

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

#define WINDOW_CLASS_NAME "systray"
#define TRAY_ID 1 // doc: Values from 0 to 12 are reserved and should not be used.

#define MSG_TRAY_CALLBACK (WM_USER + 1) // This message has two meanings: tray message + forward
#define MSG_POPUP_MENU (WM_USER + 2)

// source: http://www.codeproject.com/shell/ctrayiconposition.asp
BOOL CALLBACK FindTrayWnd(HWND hwnd, LPARAM lParam) {    

	TCHAR szClassName[256];
	GetClassName(hwnd, szClassName, 255);
	if (strcmp(szClassName, "TrayNotifyWnd") == 0) {

		*(HWND*)lParam = hwnd;
      return FALSE;    
	}    
	return TRUE;
}

// source: http://www.codeproject.com/shell/ctrayiconposition.asp
BOOL CALLBACK FindToolBarInTrayWnd(HWND hwnd, LPARAM lParam) {    

	TCHAR szClassName[256];
	GetClassName(hwnd, szClassName, 255);    // Did we find the Main System Tray? If so, then get its size and quit
	if (strcmp(szClassName, "ToolbarWindow32") == 0) {        

		*(HWND*)lParam = hwnd;
		return FALSE;
	}    
	return TRUE;
}

// source: http://www.codeproject.com/shell/ctrayiconposition.asp
HWND GetTrayNotifyWnd() {

	HWND hWndTrayNotifyWnd = NULL;
	HWND hWndShellTrayWnd = FindWindow("Shell_TrayWnd", NULL);
	if (hWndShellTrayWnd) {        

		EnumChildWindows(hWndShellTrayWnd, FindTrayWnd, (LPARAM)&hWndTrayNotifyWnd);   
		if (hWndTrayNotifyWnd && IsWindow(hWndTrayNotifyWnd)) {

			HWND hWndToolBarWnd = NULL;
			EnumChildWindows(hWndTrayNotifyWnd, FindToolBarInTrayWnd, (LPARAM)&hWndToolBarWnd);   
			if(hWndToolBarWnd)
				return hWndToolBarWnd;
		}
		return hWndTrayNotifyWnd;
	}
	return hWndShellTrayWnd;
}

// source: http://www.codeproject.com/shell/ctrayiconposition.asp
BOOL FindOutPositionOfIconDirectly(const HWND a_hWndOwner, const int a_iButtonID, RECT *a_rcIcon) {

	HWND hWndTray = GetTrayNotifyWnd();
   if (hWndTray == NULL)
		return FALSE;
	DWORD dwTrayProcessID = -1;
	GetWindowThreadProcessId(hWndTray, &dwTrayProcessID);
	if(dwTrayProcessID <= 0)
		return FALSE;
	HANDLE hTrayProc = OpenProcess(PROCESS_ALL_ACCESS, 0, dwTrayProcessID);
	if(hTrayProc == NULL)
		return FALSE;
 	int iButtonsCount = SendMessage(hWndTray, TB_BUTTONCOUNT, 0, 0);
	LPVOID lpData = VirtualAllocEx(hTrayProc, NULL, sizeof(TBBUTTON), MEM_COMMIT, PAGE_READWRITE);
	if( lpData == NULL || iButtonsCount < 1 ) {

		CloseHandle(hTrayProc);
		return FALSE;
	}
	BOOL bIconFound = FALSE;
	for(int iButton=0; iButton<iButtonsCount; iButton++) {

		DWORD dwBytesRead = -1;
		TBBUTTON buttonData;
		SendMessage(hWndTray, TB_GETBUTTON, iButton, (LPARAM)lpData);
		ReadProcessMemory(hTrayProc, lpData, &buttonData, sizeof(TBBUTTON), &dwBytesRead);
		if(dwBytesRead < sizeof(TBBUTTON))
			continue;
		DWORD dwExtraData[2] = { 0,0 };
		ReadProcessMemory(hTrayProc, (LPVOID)buttonData.dwData, dwExtraData, sizeof(dwExtraData), &dwBytesRead);
		if(dwBytesRead < sizeof(dwExtraData))
			continue;
		HWND hWndOfIconOwner = (HWND) dwExtraData[0];
		int  iIconId		 = (int)  dwExtraData[1];
		if(hWndOfIconOwner != a_hWndOwner || iIconId != a_iButtonID)
			continue;
		if( buttonData.fsState & TBSTATE_HIDDEN )
			break;
		RECT rcPosition = {0,0};
		SendMessage(hWndTray, TB_GETITEMRECT, iButton, (LPARAM)lpData);
		ReadProcessMemory(hTrayProc, lpData, &rcPosition, sizeof(RECT), &dwBytesRead);
		if(dwBytesRead < sizeof(RECT))
			continue;
		MapWindowPoints(hWndTray, NULL, (LPPOINT)&rcPosition, 2);
		*a_rcIcon = rcPosition;
		bIconFound = TRUE;
		break;
	}
	VirtualFreeEx(hTrayProc, lpData, NULL, MEM_RELEASE);
	CloseHandle(hTrayProc);
	return bIconFound;
}



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
	int mouseX, mouseY;
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
		case WM_COMMAND: {
			MSGInfo *msg = (MSGInfo*)malloc(sizeof(MSGInfo));
			// BOOL swapButtons = GetSystemMetrics(SM_SWAPBUTTON); // (TBD) use it

			msg->lButton = GetAsyncKeyState(VK_LBUTTON) > 0; // (TBD) check !!
			msg->rButton = GetAsyncKeyState(VK_RBUTTON) > 0; // (TBD) check !!
			msg->mButton = GetAsyncKeyState(VK_MBUTTON) > 0; // (TBD) check !!
		
			msg->shiftKey   = GetAsyncKeyState(VK_SHIFT)   > 0; // (TBD) check !!
			msg->controlKey = GetAsyncKeyState(VK_CONTROL) > 0; // (TBD) check !!
			msg->altKey     = GetAsyncKeyState(VK_MENU)    > 0; // (TBD) check !!

			if ( message == MSG_TRAY_CALLBACK ) {
				
				POINT pt;
				GetCursorPos(&pt);
				msg->mouseX = pt.x;
				msg->mouseY = pt.y;
			} else {
				
				msg->mouseX = -1;
				msg->mouseY = -1;
			}

			msg->hwnd = hWnd;
			msg->message = message;
			msg->wParam = wParam;
			msg->lParam = lParam;
			PostThreadMessage( GetWindowLong(hWnd, GWL_USERDATA), MSG_TRAY_CALLBACK, (WPARAM)msg, 0 );
			return 0;
		}
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
			case MSG_POPUP_MENU: {	
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
				break;
			}
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
	return JS_TRUE;
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
			int mouseX = trayWndMsg->mouseX;
			int mouseY = trayWndMsg->mouseY;
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
								RT_CHECK_CALL( CallFunction( cx, obj, functionVal, rval, 2, INT_TO_JSVAL( mouseX ), INT_TO_JSVAL( mouseY ) ) );
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

		jsval item;
		OBJ_GET_PROPERTY(cx, menuObj, list->vector[i], &item);
		LPCSTR newItem = NULL;
		UINT uFlags = MF_STRING;

		if ( JSVAL_IS_STRING(item) ) {

			RT_JSVAL_TO_STRING( item, newItem );
			AppendMenu(hMenu, uFlags, list->vector[i], newItem);
		} else if ( JSVAL_IS_OBJECT(item) && !JSVAL_IS_NULL(item) ) {
			
			JSObject *itemObject = JSVAL_TO_OBJECT(item);
			jsval key, itemVal;

			JS_GetProperty(cx, itemObject, "text", &itemVal);
			if ( itemVal != JSVAL_VOID ) {
				if ( JS_TypeOfValue(cx, itemVal) == JSTYPE_FUNCTION ) {

					RT_CHECK_CALL( JS_IdToValue(cx, list->vector[i], &key) );
					RT_CHECK_CALL( CallFunction(cx, obj, itemVal, &itemVal, 2, item, key) );
				}
				if ( itemVal != JSVAL_VOID ) {
					RT_JSVAL_TO_STRING( itemVal, newItem );
				}
			}

			JS_GetProperty(cx, itemObject, "checked", &itemVal);
			if ( itemVal != JSVAL_VOID ) {
				if ( JS_TypeOfValue(cx, itemVal) == JSTYPE_FUNCTION ) {

					RT_CHECK_CALL( JS_IdToValue(cx, list->vector[i], &key) );
					RT_CHECK_CALL( CallFunction(cx, obj, itemVal, &itemVal, 2, item, key) );
				}
				if ( itemVal != JSVAL_VOID ) {
					JSBool boolVal;
					RT_CHECK_CALL( JS_ValueToBoolean(cx, itemVal, &boolVal) );
					if ( boolVal == JS_TRUE )
						uFlags |= MF_CHECKED;
				}
			}

			JS_GetProperty(cx, itemObject, "grayed", &itemVal);
			if ( itemVal != JSVAL_VOID ) {
				if ( JS_TypeOfValue(cx, itemVal) == JSTYPE_FUNCTION ) {

					RT_CHECK_CALL( JS_IdToValue(cx, list->vector[i], &key) );
					RT_CHECK_CALL( CallFunction(cx, obj, itemVal, &itemVal, 2, item, key) );
				}

				if ( itemVal != JSVAL_VOID ) {
					JSBool boolVal;
					RT_CHECK_CALL( JS_ValueToBoolean(cx, itemVal, &boolVal) );
					if ( boolVal == JS_TRUE )
						uFlags |= MF_GRAYED;
				}
			}

			JS_GetProperty(cx, itemObject, "separator", &itemVal);
			if ( itemVal == JSVAL_TRUE || itemVal == JSVAL_ONE )
				uFlags |= MF_SEPARATOR;

			AppendMenu(hMenu, uFlags, list->vector[i], newItem);

			JS_GetProperty(cx, itemObject, "default", &itemVal);
			if ( itemVal == JSVAL_TRUE || itemVal == JSVAL_ONE ) {

				SetMenuDefaultItem(hMenu, i, TRUE);
			}

			JS_GetProperty(cx, itemObject, "icon", &itemVal); // the menu item bitmap can only be added AFTER the item has been created
			if ( itemVal != JSVAL_VOID ) {
				if ( JS_TypeOfValue(cx, itemVal) == JSTYPE_FUNCTION ) {

					RT_CHECK_CALL( JS_IdToValue(cx, list->vector[i], &key) );
					RT_CHECK_CALL( CallFunction(cx, obj, itemVal, &itemVal, 2, item, key) );
				}
				if ( itemVal != JSVAL_VOID ) {
					JSObject *iconObj = JSVAL_TO_OBJECT(itemVal);
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
	}
	JS_DestroyIdArray(cx, list);
	PostMessage(nid->hWnd, MSG_POPUP_MENU, 0, 0);
	return JS_TRUE;
}

/*
DEFINE_FUNCTION( CallDefault ) {

	PNOTIFYICONDATA nid = (PNOTIFYICONDATA)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(nid);
	HMENU hMenu = GetMenu(nid->hWnd);
	RT_ASSERT_RESOURCE(hMenu);
!!! menu is empty !!!
	jsid id = GetMenuDefaultItem( hMenu, FALSE, GMDI_USEDISABLED ); // http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winui/winui/windowsuserinterface/resources/menus/menureference/menufunctions/getmenudefaultitem.asp
	DWORD err = GetLastError();

	jsval functionVal;
	JS_GetProperty(cx, obj, "oncommand", &functionVal);
	if ( functionVal != JSVAL_VOID ) {

		jsval key;
		RT_CHECK_CALL( JS_IdToValue(cx, id, &key) );
		RT_CHECK_CALL( CallFunction( cx, obj, functionVal, rval, 1, key ) );
	}
	return JS_TRUE;
}
*/

DEFINE_FUNCTION( Position ) {

	PNOTIFYICONDATA nid = (PNOTIFYICONDATA)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE(nid);
	RECT r;
	BOOL res = FindOutPositionOfIconDirectly( nid->hWnd, nid->uID, &r );
	RT_ASSERT( res == TRUE, "Unable to FindOutPositionOfIconDirectly." );
	jsval v[] = { INT_TO_JSVAL(r.left), INT_TO_JSVAL(r.top) };

	JSObject *point;
	if ( argc >= 1 && JSVAL_IS_OBJECT(argv[0]) && !JSVAL_IS_NULL(argv[0]) ) { // reuse

		point = JSVAL_TO_OBJECT(argv[0]); // (TBD) check this
		
		JS_SetElement(cx, point, 0, &v[0]);
		JS_SetElement(cx, point, 1, &v[1]);
	} else {

		point = JS_NewArrayObject(cx, 2, v);
	}
	*rval = OBJECT_TO_JSVAL(point);
	return JS_TRUE;
}


DEFINE_FUNCTION( Rect ) {

	HWND hWndTrayWnd = GetTrayNotifyWnd();
	RT_ASSERT( hWndTrayWnd != NULL, "Unable to GetTrayNotifyWnd." );
	RECT rect;
	BOOL st = GetWindowRect(hWndTrayWnd, &rect);
	RT_ASSERT( st == TRUE, "Unable to GetWindowRect." );
	jsval v[] = { INT_TO_JSVAL(rect.left), INT_TO_JSVAL(rect.top), INT_TO_JSVAL(rect.right-rect.left), INT_TO_JSVAL(rect.bottom-rect.top) };

	JSObject *point;
	if ( argc >= 1 && JSVAL_IS_OBJECT(argv[0]) && !JSVAL_IS_NULL(argv[0]) ) { // reuse

		point = JSVAL_TO_OBJECT(argv[0]); // (TBD) check this
		
		JS_SetElement(cx, point, 0, &v[0]);
		JS_SetElement(cx, point, 1, &v[1]);
		JS_SetElement(cx, point, 2, &v[2]);
		JS_SetElement(cx, point, 3, &v[3]);
	} else {

		point = JS_NewArrayObject(cx, 4, v);
	}
	*rval = OBJECT_TO_JSVAL(point);
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
	const char *tipText;
	size_t tipLen;
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
		FUNCTION(Position)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY(menu)
		PROPERTY_WRITE_STORE(icon) // _STORE  is needed to keep the reference to the image ( aboid GC )
		PROPERTY(text)
		PROPERTY_WRITE(visible)
	END_PROPERTY_SPEC

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION(Rect)
	END_STATIC_FUNCTION_SPEC


	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1)

END_CLASS


/*
http://msdn.microsoft.com/library/default.asp?url=/library/en-us/shellcc/platform/shell/reference/functions/shell_notifyicon.asp

Icons in Win32:
	http://msdn2.microsoft.com/en-us/library/ms997538.aspx

*/