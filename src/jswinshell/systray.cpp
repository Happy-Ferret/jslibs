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

#include "../jslang/handlePub.h"

#include "queue.h"

#include "icon.h"

#include <commctrl.h>

#define SYSTRAY_WINDOW_CLASS_NAME "jslibs_systray"
#define SYSTRAY_ID 13 // doc: Values from 0 to 12 are reserved and should not be used.

#define MSG_TRAY_CALLBACK (WM_USER + 1) // This message has two meanings: tray message + forward
#define MSG_POPUP_MENU (WM_USER + 2)


struct Private {
	
	NOTIFYICONDATA nid;
	HANDLE thread;
	HANDLE event;
	jl::Queue msgQueue;
	CRITICAL_SECTION cs; // protects msgQueue
	jl::Queue popupMenuRoots;
};


typedef struct MSGInfo {
	HWND hwnd;
	UINT message;
	WPARAM wParam;
	LPARAM lParam;
	BOOL lButton, rButton, mButton;
	BOOL shiftKey, controlKey, altKey;
	int mouseX, mouseY;
} MSGInfo;


JL_STATIC_ASSERT(sizeof(jsval) <= sizeof(/*jl::QueueCell::data aka.*/void*));

void AddPopupMenuRoot(JSContext *cx, JSObject *obj, jsval value) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	jl::QueuePush(&pv->popupMenuRoots, (void*)value);
}

void FreePopupMenuRoots(JSContext *cx, JSObject *obj) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	while ( !jl::QueueIsEmpty(&pv->popupMenuRoots) )
		jl::QueuePop(&pv->popupMenuRoots);
}


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


void ForwardMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

	MSGInfo *msg = (MSGInfo*)jl_malloc(sizeof(MSGInfo));
	JL_ASSERT( msg != NULL );
	// BOOL swapButtons = GetSystemMetrics(SM_SWAPBUTTON); // (TBD) use it

	msg->hwnd = hWnd;
	msg->message = message;
	msg->wParam = wParam;
	msg->lParam = lParam;

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

	Private *pv = (Private*)GetWindowLongPtr(hWnd, GWL_USERDATA);
	JL_ASSERT( pv != NULL );

	EnterCriticalSection(&pv->cs);
	jl::QueuePush(&pv->msgQueue, msg);
	LeaveCriticalSection(&pv->cs);

	PulseEvent(pv->event);
}


BOOL FreeMenu( HMENU menu ) {

	for ( int i = GetMenuItemCount(menu); i > 0; i-- ) {

		MENUITEMINFO mii = { sizeof(MENUITEMINFO) };
		mii.fType = MFT_RADIOCHECK; // doc: Displays selected menu items using a radio-button mark instead of a check mark if the hbmpChecked member is NULL.
		mii.fMask = MIIM_CHECKMARKS | MIIM_SUBMENU;
		BOOL st = GetMenuItemInfo(menu, 0, TRUE, &mii);
		if ( mii.hbmpChecked != NULL )
			DeleteObject(mii.hbmpChecked);
		if ( mii.hSubMenu != NULL )
			FreeMenu(mii.hSubMenu);
		DeleteMenu(menu, 0, MF_BYPOSITION);
	}
	return TRUE;
}


static LRESULT CALLBACK SystrayWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

//	char dbg[65535];  sprintf(dbg, "message:%x wParam:%x\n", message, wParam);  OutputDebugString(dbg);

	switch ( message ) {

		case MSG_POPUP_MENU: {

			POINT pos;
			GetCursorPos(&pos);
			SetForegroundWindow(hWnd);
			TrackPopupMenuEx(GetMenu(hWnd), GetSystemMetrics(SM_MENUDROPALIGNMENT) | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pos.x, pos.y, hWnd, NULL);
			PostMessage(hWnd, WM_NULL, 0, 0);
			FreeMenu(GetMenu(hWnd)); // free menu data and menu items
			break;
		}
		case MSG_TRAY_CALLBACK:
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_CHAR:
		case WM_SETFOCUS:
		case WM_KILLFOCUS:
		case WM_COMMAND: {
			ForwardMessage(hWnd, message, wParam, lParam);
			break;
		}
		case WM_SYSCOMMAND: // avoid any system command (including Alt-F4) from systray icon.

			switch ( wParam ) {
				case SC_CLOSE:
					break;
				case SC_SCREENSAVE: // not detected !
				case SC_MONITORPOWER:

					ForwardMessage(hWnd, message, wParam, lParam);
					return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;

		case WM_DESTROY:
			PostQuitMessage(wParam);
			break;

//		case WM_QUERYENDSESSION:
		case WM_ENDSESSION: // test with: http://code.google.com/p/sendmessage/downloads/detail?name=SendMessage-1.0.1.exe&can=2&q=
			ForwardMessage(hWnd, message, wParam, lParam);
			 // doc: The application need not call the DestroyWindow or PostQuitMessage function when the session is ending.
			 // doc: The system does not send any messages to gracefully close your app (such as WM_CLOSE, WM_DESTROY, or WM_QUIT). You must do it yourself. 
			return DefWindowProc(hWnd, message, wParam, lParam);

		case WM_CLOSE: // test with: taskkill /IM jswinhost.exe
			ForwardMessage(hWnd, message, wParam, lParam);
			DestroyMenu(GetMenu(hWnd)); // (TBD) needed ?
			// DestroyWindow(hWnd);
			return DefWindowProc(hWnd, message, wParam, lParam); // doc: By default, the DefWindowProc function calls the DestroyWindow function to destroy the window.

		default:
			return DefWindowProc(hWnd, message, wParam, lParam); // We do not want to handle this message so pass back to Windows to handle it in a default way
	}
	return NULL;
}



DWORD WINAPI SystrayThread( LPVOID lpParam ) {

	Private *pv = (Private*)lpParam;

	HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
	JL_ASSERT( hInst != NULL ); // JL_S_ASSERT( hInst != NULL, "Unable to GetModuleHandle." );

	WNDCLASS wc = { 0, SystrayWndProc, 0, 0, hInst, NULL, NULL, NULL, NULL, SYSTRAY_WINDOW_CLASS_NAME };
	ATOM rc = RegisterClass(&wc);	// (TBD) do UnregisterClass at the end ?
	JL_ASSERT( rc != 0 || GetLastError() == ERROR_CLASS_ALREADY_EXISTS ); // JL_S_ASSERT( rc != 0 || GetLastError() == ERROR_CLASS_ALREADY_EXISTS, "Unable to RegisterClass." );

	memset(&pv->nid, 0, sizeof(NOTIFYICONDATA)); // http://msdn.microsoft.com/library/default.asp?url=/library/en-us/shellcc/platform/shell/reference/structures/notifyicondata.asp
	pv->nid.cbSize = sizeof(NOTIFYICONDATA);
	pv->nid.uID = SYSTRAY_ID;
	pv->nid.uFlags = NIF_MESSAGE;
	pv->nid.uCallbackMessage = MSG_TRAY_CALLBACK; // doc: All Message Numbers below 0x0400 are RESERVED.

	// doc: The message loop and window procedure for the window must be in the thread that created the window.
	pv->nid.hWnd = CreateWindow(SYSTRAY_WINDOW_CLASS_NAME, NULL, WS_OVERLAPPED, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, HWND_MESSAGE, CreatePopupMenu(), hInst, NULL); // (TBD) use HWND_MESSAGE ?
	SetWindowLongPtr(pv->nid.hWnd, GWL_USERDATA, (LONG)pv); // make pv available for SystrayWndProc

	BOOL status = Shell_NotifyIcon(NIM_ADD, &pv->nid);
	JL_ASSERT( status ); // JL_S_ASSERT( status == TRUE, "Unable to setup systray icon." );

	PulseEvent(pv->event); // first pulse

	BOOL st;
	MSG msg;
	while ( (st = GetMessage(&msg, pv->nid.hWnd, 0, 0)) != 0 ) {

		if ( st == -1 ) // (TBD) manage the error
			break;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	status = Shell_NotifyIcon(NIM_DELETE, &pv->nid);
	JL_ASSERT( status ); // JL_S_ASSERT( status == TRUE, "Unable to delete notification icon.");

	pv->nid.hWnd = NULL; // see finalizer

	// (TBD) call DestroyWindow() ???

	return msg.wParam;
}


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( Systray )

/**doc
$TOC_MEMBER $INAME
 $INAME()
  Creates a Systray object.
**/
DEFINE_CONSTRUCTOR() {

	JL_S_ASSERT_CONSTRUCTING();
	JL_S_ASSERT_THIS_CLASS();

	Private *pv = (Private*)jl_malloc(sizeof(Private));
	JL_S_ASSERT_ALLOC( pv );
	JL_SetPrivate(cx, obj, pv);

	InitializeCriticalSection(&pv->cs);

	jl::QueueInitialize(&pv->msgQueue);
	jl::QueueInitialize(&pv->popupMenuRoots);

	pv->event = CreateEvent(NULL, FALSE, FALSE, NULL);
	pv->thread = CreateThread(NULL, 0, SystrayThread, pv, 0, NULL);
	SetThreadPriority(pv->thread, THREAD_PRIORITY_ABOVE_NORMAL);
	WaitForSingleObject(pv->event, INFINITE); // first pulse

	return JS_TRUE;
	JL_BAD;
}


DEFINE_FINALIZE() {

//	if ( obj == JL_PROTOTYPE(cx, Systray) )
//		return;

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	if ( !pv )
		return;

	if ( pv->nid.hWnd != NULL ) { // pv->nid.hWnd may have already been closed.

		LRESULT res = SendMessage( pv->nid.hWnd, WM_CLOSE, 0, 0 ); // PostMessage
	}

	WaitForSingleObject(pv->thread, INFINITE);
	CloseHandle(pv->thread); // doc: The thread object remains in the system until the thread has terminated and all handles to it have been closed through a call to CloseHandle.
	CloseHandle(pv->event);
	DeleteCriticalSection(&pv->cs);
	
	while ( !jl::QueueIsEmpty(&pv->msgQueue) )
		jl_free(jl::QueuePop(&pv->msgQueue));

	FreePopupMenuRoots(cx, obj);

	jl_free(pv);
}


/**doc
=== Methods ===
**/

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Close the Systray.
**/
DEFINE_FUNCTION( Close ) {

	Finalize(cx, obj);
	JL_SetPrivate(cx, obj, NULL);

	return JS_TRUE;
	JL_BAD;
}


JSBool ProcessSystrayMessage( JSContext *cx, JSObject *obj, MSGInfo *trayMsg, jsval *rval ) {

	UINT message = trayMsg->message;
	LPARAM lParam = trayMsg->lParam;
	WPARAM wParam = trayMsg->wParam;
	int mButton = trayMsg->lButton ? 1 : trayMsg->rButton ? 2 : 0;
	int mouseX = trayMsg->mouseX;
	int mouseY = trayMsg->mouseY;

	jsval functionVal;

//	char dbg[65535];  sprintf(dbg, "msg: %x\n", message);  OutputDebugString(dbg);

	switch ( message ) {
		case WM_SETFOCUS:
			JL_CHK( JS_GetProperty(cx, obj, "onfocus", &functionVal) );
			if ( JsvalIsFunction(cx, functionVal) )
				JL_CHK( JL_CallFunction( cx, obj, functionVal, rval, 1, JSVAL_TRUE ) );
			break;
		case WM_KILLFOCUS:
			JL_CHK( JS_GetProperty(cx, obj, "onblur", &functionVal) );
			if ( JsvalIsFunction(cx, functionVal) )
				JL_CHK( JL_CallFunction( cx, obj, functionVal, rval, 1, JSVAL_FALSE ) );
			break;
		case WM_CHAR:
			JL_CHK( JS_GetProperty(cx, obj, "onchar", &functionVal) );
			if ( JsvalIsFunction(cx, functionVal) ) {

				char c = wParam;
				JL_CHK( JL_CallFunction( cx, obj, functionVal, rval, 1, STRING_TO_JSVAL( JS_NewStringCopyN(cx, &c, 1) ) ) );
			}
			break;

		case WM_ENDSESSION: // case WM_QUERYENDSESSION:
		case WM_CLOSE:
			JL_CHK( JS_GetProperty(cx, obj, "onclose", &functionVal) );
			if ( JsvalIsFunction(cx, functionVal) ) {

				bool endCase = message == WM_ENDSESSION && lParam == 0;
				JL_CHK( JL_CallFunction( cx, obj, functionVal, rval, 1, BOOLEAN_TO_JSVAL(endCase) ) );
			}
			break;

		case WM_COMMAND:
			JL_CHK( JS_GetProperty(cx, obj, "oncommand", &functionVal) );
			if ( JsvalIsFunction(cx, functionVal) ) {

				jsval key;
				JL_CHK( JS_IdToValue(cx, (jsid)wParam, &key) );
				JL_CHK( JL_CallFunction( cx, obj, functionVal, rval, 2, key, INT_TO_JSVAL( mButton ) ) );
				FreePopupMenuRoots(cx, obj);
			}
			break;

		case WM_SYSCOMMAND: // avoid any system command (including Alt-F4) from systray icon.

			switch ( wParam ) {
				case SC_SCREENSAVE:
				case SC_MONITORPOWER:

					JL_CHK( JS_GetProperty(cx, obj, "onidle", &functionVal) );
					if ( JsvalIsFunction(cx, functionVal) ) {

						jsval key;
						JL_CHK( JS_IdToValue(cx, (jsid)wParam, &key) );
						JL_CHK( JL_CallFunction( cx, obj, functionVal, rval, 2, key, INT_TO_JSVAL( mButton ) ) );
					}
				break;
			}
			break;

		case MSG_TRAY_CALLBACK:
			switch ( lParam ) {
				case WM_MOUSEMOVE:
					JL_CHK( JS_GetProperty(cx, obj, "onmousemove", &functionVal) );
					if ( JsvalIsFunction(cx, functionVal) )
						JL_CHK( JL_CallFunction( cx, obj, functionVal, rval, 2, INT_TO_JSVAL( mouseX ), INT_TO_JSVAL( mouseY ) ) );
					break;
				case WM_LBUTTONDOWN:
				case WM_MBUTTONDOWN:
				case WM_RBUTTONDOWN:
					JL_CHK( JS_GetProperty(cx, obj, "onmousedown", &functionVal) );
					if ( JsvalIsFunction(cx, functionVal) )
						JL_CHK( JL_CallFunction( cx, obj, functionVal, rval, 2, INT_TO_JSVAL( lParam==WM_LBUTTONDOWN ? 1 : lParam==WM_RBUTTONDOWN ? 2 : lParam==WM_MBUTTONDOWN ? 3 : 0 ), JSVAL_TRUE ) );
					break;
				case WM_LBUTTONUP:
				case WM_MBUTTONUP:
				case WM_RBUTTONUP:
					JL_CHK( JS_GetProperty(cx, obj, "onmouseup", &functionVal) );
					if ( JsvalIsFunction(cx, functionVal) )
						JL_CHK( JL_CallFunction( cx, obj, functionVal, rval, 2, INT_TO_JSVAL( lParam==WM_LBUTTONUP ? 1 : lParam==WM_RBUTTONUP ? 2 : lParam==WM_MBUTTONUP ? 3 : 0 ), JSVAL_FALSE ) );
					break;
				case WM_LBUTTONDBLCLK:
				case WM_MBUTTONDBLCLK:
				case WM_RBUTTONDBLCLK:
					JL_CHK( JS_GetProperty(cx, obj, "onmousedblclick", &functionVal) );
					if ( JsvalIsFunction(cx, functionVal) )
						JL_CHK( JL_CallFunction( cx, obj, functionVal, rval, 1, INT_TO_JSVAL( lParam==WM_LBUTTONDBLCLK ? 1 : lParam==WM_RBUTTONDBLCLK ? 2 : lParam==WM_MBUTTONDBLCLK ? 3 : 0 ) ) );
					break;
			} // switch lParam
	} //  switch message

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME()
  Precess all pending events of the systray.
  The function returns true if at least one of the event function ( see Remarks below ) returns true.
**/
DEFINE_FUNCTION( ProcessEvents ) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);

	MSGInfo *trayMsg;
	while ( !jl::QueueIsEmpty(&pv->msgQueue) ) {

		EnterCriticalSection(&pv->cs);
		trayMsg = (MSGInfo*)jl::QueueShift(&pv->msgQueue);
		LeaveCriticalSection(&pv->cs);
		JL_CHK( ProcessSystrayMessage(cx, obj, trayMsg, JL_RVAL) );
		jl_free(trayMsg);
	}	
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE HANDLE $INAME()
  Passively waits for Systray events through the ProcessEvents function.
**/

struct UserProcessEvent {
	
	ProcessEvent pe;
	Private *systrayPrivate;
	HANDLE cancelEvent;
	JSObject *systrayObj;
};

JL_STATIC_ASSERT( offsetof(UserProcessEvent, pe) == 0 );

static void SystrayStartWait( volatile ProcessEvent *pe ) {

	UserProcessEvent *upe = (UserProcessEvent*)pe;

	HANDLE events[] = { upe->systrayPrivate->event, upe->cancelEvent };
	DWORD status = WaitForMultipleObjects(COUNTOF(events), events, FALSE, INFINITE);
	JL_ASSERT( status != WAIT_FAILED );

}

static bool SystrayCancelWait( volatile ProcessEvent *pe ) {

	UserProcessEvent *upe = (UserProcessEvent*)pe;

	SetEvent(upe->cancelEvent);
	return true;
}

static JSBool SystrayEndWait( volatile ProcessEvent *pe, bool *hasEvent, JSContext *cx, JSObject *obj ) {

	UserProcessEvent *upe = (UserProcessEvent*)pe;

	Private *pv = upe->systrayPrivate;

	CloseHandle(upe->cancelEvent);
	
	*hasEvent = !jl::QueueIsEmpty(&pv->msgQueue);

	jsval rval;
	MSGInfo *trayMsg;
	while ( !jl::QueueIsEmpty(&pv->msgQueue) ) {

		EnterCriticalSection(&pv->cs);
		trayMsg = (MSGInfo*)jl::QueueShift(&pv->msgQueue);
		LeaveCriticalSection(&pv->cs);
		if ( ProcessSystrayMessage(cx, upe->systrayObj, trayMsg, &rval) != JS_TRUE ) {
			
			jl_free(trayMsg);
			goto bad;
		}
		jl_free(trayMsg);
	}	

	return JS_TRUE;
	JL_BAD;
}

DEFINE_FUNCTION_FAST( Events ) {
	
	JL_S_ASSERT_ARG(0);
	JSObject *obj = JL_FOBJ;
	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);

	UserProcessEvent *upe;
	JL_CHK( CreateHandle(cx, 'pev', sizeof(UserProcessEvent), (void**)&upe, NULL, JL_FRVAL) );
	upe->pe.startWait = SystrayStartWait;
	upe->pe.cancelWait = SystrayCancelWait;
	upe->pe.endWait = SystrayEndWait;

	upe->cancelEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	upe->systrayPrivate = pv;
	upe->systrayObj = obj;


	JL_CHK( SetHandleSlot(cx, *JL_FRVAL, 0, OBJECT_TO_JSVAL(obj)) ); // GC protection

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Puts the systray into the foreground. Keyboard input is directed to the systray.
**/
DEFINE_FUNCTION( Focus ) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	SetForegroundWindow(pv->nid.hWnd);
	return JS_TRUE;
	JL_BAD;
}

ALWAYS_INLINE JSBool NormalizeMenuInfo( JSContext *cx, JSObject *obj, const jsval key, jsval *value ) {

	if ( JsvalIsFunction(cx, *value) )
		return JL_CallFunction(cx, obj, *value, value, 1, key);
	return JS_TRUE;
}

JSBool MakeMenu( JSContext *cx, JSObject *systrayObj, JSObject *menuObj, HMENU *hMenu ) {

	*hMenu = CreatePopupMenu();

	jsuint menuLength;
	JL_CHK( JS_GetArrayLength(cx, menuObj, &menuLength) );

	for ( jsuint i = 0; i < menuLength; ++i ) {
		
		UINT uFlags = 0;
		bool isDefault = false;
		HBITMAP hBMP = NULL;
		jsval cmdid = JSVAL_VOID;
		jsval label = JSVAL_VOID;
		HMENU popupMenu;

		jsval item;
		JL_CHK( JS_GetElement(cx, menuObj, i, &item) );

		JL_CHK( NormalizeMenuInfo(cx, menuObj, INT_TO_JSVAL(i), &item) );

		if ( JSVAL_IS_NULL(item) || JSVAL_IS_VOID(item) ) {

			uFlags |= MF_SEPARATOR;
		} else 
		if ( JSVAL_IS_STRING(item) || JL_VALUE_IS_STRING_OBJECT(cx, item) ) {

			label = item;
			cmdid = item;
		} else
		if ( !JSVAL_IS_PRIMITIVE(item) ) {

			JSObject *itemObj = JSVAL_TO_OBJECT(item);
			jsval key, value;
			JSIdArray *list = JS_Enumerate(cx, itemObj);
			JL_CHK( list );
			for ( jsint j = 0; j < list->length; ++j ) {
			
				JL_CHK( JS_IdToValue(cx, list->vector[j], &key) );
				JL_CHK( JS_GetPropertyById(cx, itemObj, list->vector[j], &value) );
				
				const char *keyStr;
				JL_CHK( JsvalToString(cx, &key, &keyStr) );

				if ( strcmp(keyStr, "id") == 0 ) {

					JL_CHK( NormalizeMenuInfo(cx, itemObj, key, &value) );
					cmdid = value;
					continue;
				}
				if ( strcmp(keyStr, "label") == 0 ) {

					JL_CHK( NormalizeMenuInfo(cx, itemObj, key, &value) );
					label = value;
					continue;
				}
				if ( strcmp(keyStr, "break") == 0 ) {
					
					JL_CHK( NormalizeMenuInfo(cx, itemObj, key, &value) );
					bool b;
					JL_CHK( JsvalToBool(cx, value, &b) );
					if ( b )
						uFlags |= MF_MENUBARBREAK;
					continue;
				}
				if ( strcmp(keyStr, "checked") == 0 ) {
					
					JL_CHK( NormalizeMenuInfo(cx, itemObj, key, &value) );
					bool b;
					JL_CHK( JsvalToBool(cx, value, &b) );
					if ( b )
						uFlags |= MF_CHECKED;
					continue;
				}
				if ( strcmp(keyStr, "grayed") == 0 ) {
					
					JL_CHK( NormalizeMenuInfo(cx, itemObj, key, &value) );
					bool b;
					JL_CHK( JsvalToBool(cx, value, &b) );
					if ( b )
						uFlags |= MF_GRAYED;
					continue;
				}
				if ( strcmp(keyStr, "disabled") == 0 ) {
					
					JL_CHK( NormalizeMenuInfo(cx, itemObj, key, &value) );
					bool b;
					JL_CHK( JsvalToBool(cx, value, &b) );
					if ( b )
						uFlags |= MF_DISABLED;
					continue;
				}
				if ( strcmp(keyStr, "default") == 0 ) {
					
					JL_CHK( NormalizeMenuInfo(cx, itemObj, key, &value) );
					JL_CHK( JsvalToBool(cx, value, &isDefault) );
					continue;
				}
				if ( strcmp(keyStr, "icon") == 0 ) {

					JL_CHK( NormalizeMenuInfo(cx, itemObj, key, &value) );
					JL_S_ASSERT_OBJECT( value );
					JSObject *iconObj = JSVAL_TO_OBJECT(value);
					JL_S_ASSERT_CLASS( iconObj, JL_CLASS(Icon) );
					HICON *phIcon = (HICON*)JL_GetPrivate(cx, iconObj);
					JL_S_ASSERT_RESOURCE(phIcon);
					hBMP = MenuItemBitmapFromIcon(*phIcon);
					JL_S_ASSERT(hBMP != NULL, "Unable to create the menu item icon.");
					continue;
				}
				if ( strcmp(keyStr, "popup") == 0 ) {

					JL_CHK( NormalizeMenuInfo(cx, itemObj, key, &value) );
					JL_S_ASSERT_OBJECT( value );
					uFlags |= MF_POPUP;
					JL_CHK( MakeMenu(cx, systrayObj, JSVAL_TO_OBJECT(value), &popupMenu) );
					continue;
				}
			}
			JS_DestroyIdArray(cx, list);
		}

		LPCTSTR lpNewItem;
		UINT_PTR uIDNewItem;

		if ( uFlags & MF_SEPARATOR ) {
			
			lpNewItem = NULL;
		} else
		if ( !JSVAL_IS_PRIMITIVE(label) && JL_GetClass(JSVAL_TO_OBJECT(label)) == JL_CLASS(Icon) ) {

			uFlags |= MF_BITMAP;
			HICON *phIcon = (HICON*)JL_GetPrivate(cx, JSVAL_TO_OBJECT(label));
			JL_S_ASSERT_RESOURCE(phIcon);
			hBMP = MenuItemBitmapFromIcon(*phIcon);
			JL_S_ASSERT(hBMP != NULL, "Unable to create the menu item icon.");
			lpNewItem = (LPCTSTR)phIcon;
		} else {

			if ( label != JSVAL_VOID ) {

				JL_CHK( JsvalToString(cx, &label, &lpNewItem) );
				uFlags |= MF_STRING;
			} else {

				JL_REPORT_ERROR("Menu label must be defined.");
			}
		}

		if ( uFlags & MF_SEPARATOR ) {
			
			uIDNewItem = 0;
		} else
		if ( uFlags & MF_POPUP ) {

			uIDNewItem = (UINT_PTR)popupMenu;
		} else {

			if ( cmdid == JSVAL_VOID )
				cmdid = label;
//			AddPopupMenuRoot(cx, systrayObj, cmdid);
//			uIDNewItem = (UINT_PTR)cmdid;
			AddPopupMenuRoot(cx, systrayObj, item);
			uIDNewItem = (UINT_PTR)item;
		}

		AppendMenu(*hMenu, uFlags, uIDNewItem, lpNewItem);

		if ( isDefault )
			SetMenuDefaultItem(*hMenu, i, TRUE);

		if ( hBMP != NULL ) {

			BOOL res = SetMenuItemBitmaps(*hMenu, i, MF_BYPOSITION, hBMP, hBMP ); // doc: When the menu is destroyed, these bitmaps are not destroyed; it is up to the application to destroy them.
			JL_S_ASSERT( res != FALSE, "Unable to SetMenuItemBitmaps." );
		}
	}

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( menuObject )
  Opens the systray menu.
 $H the menu object
  The menuObject is an array that describes the content of the menu.
  string item: the given string is used as id and label.
  '---' string: an horizontal separator.
  object item: the object contains detailed information about the menu item:
   * ,,string,, *id*
    The command id of this item.
   * ,,string,, *label*
    The text of the menu item.
   * ,,boolean,, *checked*
    Add a checked icon before the item.
   * ,,boolean,, *grayed*
    Make the menu item appear grayed and disabled.
   * ,,boolean,, *disabled*
    Make the menu item disabled.
   * ,,boolean,, *default*
    Set the menu item as the default item.
   * ,,boolean,, *break*
    Add a vertical separator before the item.
   * ,,[Icon],, *icon*
    The icon of the menu item.
	* ,,object,, *popup*
    A nested popup menu.
  $H note
   If a function is used to define a menu item or the value of an object item, its returned value is used as item value.
  $H example
{{{ 
[ 
  'Start',
  'Stop',
  '---',
  function() { return 'uptime: '+new Date() }
  { status:'sub', popup:function() { return [ 1,2,3,4,5 ] }
  '---',
  { text:'Exit', id:'do_exit', icon:iconRedCross },
  }
]
}}}
**/
DEFINE_FUNCTION( PopupMenu ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	JL_S_ASSERT_ARG(1);

	JL_S_ASSERT_OBJECT( JL_ARG(1) );

	HMENU hMenu = GetMenu(pv->nid.hWnd);
	JL_S_ASSERT_RESOURCE(hMenu);
	DestroyMenu(hMenu);
	FreePopupMenuRoots(cx, obj);
	JL_CHK( MakeMenu(cx, obj, JSVAL_TO_OBJECT( JL_ARG(1) ), &hMenu) );
	SetMenu(pv->nid.hWnd, hMenu);

	PostMessage(pv->nid.hWnd, MSG_POPUP_MENU, 0, 0);

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( balloonObject )
  Displays a systray balloon.
*/
DEFINE_FUNCTION( PopupBalloon ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	BOOL status;

	if ( JL_ARG_ISDEF(1) ) {
	
		JL_S_ASSERT_OBJECT( JL_ARG(1) );
		JSObject *infoObj = JSVAL_TO_OBJECT(JL_ARG(1));

		pv->nid.dwInfoFlags = NIIF_NONE;

		JL_CHK( JS_GetProperty(cx, infoObj, "infoTitle", rval) );
		if ( !JSVAL_IS_VOID(*rval) ) {

			const char *infoTitleStr;
			unsigned int infoTitleLen;
			JL_CHK( JsvalToStringAndLength(cx, rval, &infoTitleStr, &infoTitleLen) );
			int len = JL_MIN(sizeof(pv->nid.szInfo)-1, infoTitleLen);
			memcpy( pv->nid.szInfoTitle, infoTitleStr, JL_MIN(sizeof(pv->nid.szInfoTitle)-1, infoTitleLen) );
			pv->nid.szInfoTitle[len] = '\0';
		}

		JL_CHK( JS_GetProperty(cx, infoObj, "info", rval) );
		if ( !JSVAL_IS_VOID(*rval) ) {

			const char *infoStr;
			unsigned int infoLen;
			JL_CHK( JsvalToStringAndLength(cx, rval, &infoStr, &infoLen) );
			int len = JL_MIN(sizeof(pv->nid.szInfo)-1, infoLen);
			memcpy( pv->nid.szInfo, infoStr, len );
			pv->nid.szInfo[len] = '\0';
		}

		JL_CHK( JS_GetProperty(cx, infoObj, "icon", rval) );
		if ( !JSVAL_IS_VOID(*rval) ) {

			const char *iconNameStr;
			JL_CHK( JsvalToString(cx, rval, &iconNameStr) );
			
			if ( strcmp(iconNameStr, "info") == 0 )
				pv->nid.dwInfoFlags |= NIIF_INFO;
			else
			if ( strcmp(iconNameStr, "warning") == 0 )
				pv->nid.dwInfoFlags |= NIIF_WARNING;
			else
			if ( strcmp(iconNameStr, "error") == 0 )
				pv->nid.dwInfoFlags |= NIIF_ERROR;
		}

//		pv->nid.uTimeout = 10000;

		pv->nid.uFlags |= NIF_INFO;
		status = Shell_NotifyIcon(NIM_MODIFY, &pv->nid);
	} else {

		pv->nid.szInfo[0] = '\0';
		pv->nid.uFlags |= NIF_INFO;
		status = Shell_NotifyIcon(NIM_MODIFY, &pv->nid);
	}
	if ( status == FALSE )
		return WinThrowError(cx, GetLastError());

	*rval = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/*
DEFINE_FUNCTION( CallDefault ) {

	PNOTIFYICONDATA nid = (PNOTIFYICONDATA)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(nid);
	HMENU hMenu = GetMenu(nid->hWnd);
	JL_S_ASSERT_RESOURCE(hMenu);
!!! menu is empty !!!
	jsid id = GetMenuDefaultItem( hMenu, FALSE, GMDI_USEDISABLED ); // http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winui/winui/windowsuserinterface/resources/menus/menureference/menufunctions/getmenudefaultitem.asp
	DWORD err = GetLastError();

	jsval functionVal;
	JS_GetProperty(cx, obj, "oncommand", &functionVal);
	if ( !JSVAL_IS_VOID( functionVal ) ) {

		jsval key;
		JL_CHK( JS_IdToValue(cx, id, &key) );
		JL_CHK( CallFunction( cx, obj, functionVal, rval, 1, key ) );
	}
	return JS_TRUE;
	JL_BAD;
}
*/

/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME( [reusableArray] )
  Returns the [x,y] position pointed by the mouse pointer in the systray icon.
  $H note
   If you provide a _reusableArray_, the function will use it to store the values.
  $H example
  {{{
  systray.onmousemove = function( x, y ) {

   var pos = systray.Position();
   Print( x-pos[0], ', ', y-pos[1], '\n' );
  }
  }}}
**/
DEFINE_FUNCTION( Position ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);

	RECT r;
	BOOL res = FindOutPositionOfIconDirectly( pv->nid.hWnd, pv->nid.uID, &r );
	JL_S_ASSERT( res == TRUE, "Unable to FindOutPositionOfIconDirectly." );
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
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME( [reusableArray] )
  Returns the dimensions [left, top, width, height] of the systray rectangle.
  $H note
   If you provide a _reusableArray_, the function will use it to store the values.
**/
DEFINE_FUNCTION( Rect ) {

	HWND hWndTrayWnd = GetTrayNotifyWnd();
	JL_S_ASSERT( hWndTrayWnd != NULL, "Unable to GetTrayNotifyWnd." );
	RECT rect;
	BOOL st = GetWindowRect(hWndTrayWnd, &rect);
	JL_S_ASSERT( st == TRUE, "Unable to GetWindowRect." );
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
	JL_BAD;
}


/**doc
=== Properties ===
**/


/**doc
$TOC_MEMBER $INAME
 $TYPE Icon | $TYPE null $INAME $WRITEONLY
  This is the Icon to be used as systray icon.
**/
DEFINE_PROPERTY( icon ) {

	HICON hIcon;
	if ( JSVAL_IS_OBJECT(*vp) && !JSVAL_IS_NULL( *vp ) ) {

		JSObject *iconObj = JSVAL_TO_OBJECT(*vp);
		JL_S_ASSERT_CLASS( iconObj, JL_CLASS(Icon) );
		HICON *phIcon = (HICON*)JL_GetPrivate(cx, iconObj);
		JL_S_ASSERT_RESOURCE( phIcon );
		hIcon = *phIcon;
	} else if ( JSVAL_IS_NULL( *vp ) || JSVAL_IS_VOID( *vp ) ) {

		hIcon = NULL;
	} else {

		JL_REPORT_ERROR("Invalid icon.");
	}

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);

	pv->nid.hIcon = hIcon;
	pv->nid.uFlags |= NIF_ICON;
	BOOL status = Shell_NotifyIcon(NIM_MODIFY, &pv->nid);
	JL_S_ASSERT( status == TRUE, "Unable to setup systray icon." );
	return JL_StoreProperty(cx, obj, id, vp, false);
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $WRITEONLY
  Show or hide the systray icon.
  $H beware
   you cannot use this property to get the current visibility of the icon.
**/
DEFINE_PROPERTY( visible ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);

	JSBool state;
	JL_CHK( JS_ValueToBoolean(cx, *vp, &state ) );
	BOOL status = Shell_NotifyIcon( state == JS_TRUE ? NIM_ADD : NIM_DELETE, &pv->nid);
	JL_S_ASSERT( status == TRUE || GetLastError() == NO_ERROR, "Unable to setup systray icon." );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $STR $INAME
  Get of set the tooltip text of the systray icon.
**/
DEFINE_PROPERTY( textSetter ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	const char *tipText;
	size_t tipLen;
	JL_CHK( JsvalToStringAndLength(cx, vp, &tipText, &tipLen) );
	int len = JL_MIN(sizeof(pv->nid.szTip)-1, tipLen);
	memcpy(pv->nid.szTip, tipText, len);
	pv->nid.szTip[len] = '\0';

	pv->nid.uFlags |= NIF_TIP;
	BOOL status = Shell_NotifyIcon(NIM_MODIFY, &pv->nid);
	JL_S_ASSERT( status == TRUE, "Unable to setup systray icon." );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( textGetter ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	if ( pv->nid.uFlags & NIF_TIP )
		*vp = STRING_TO_JSVAL( JS_NewStringCopyZ(cx, pv->nid.szTip) );
	return JS_TRUE;
	JL_BAD;
}


DEFINE_TRACER() {

	Private *pv = (Private*)JL_GetPrivate(trc->context, obj);
	if ( pv )
		for ( jl::QueueCell *it = jl::QueueBegin(&pv->popupMenuRoots); it; it = jl::QueueNext(it) )
			JS_CALL_VALUE_TRACER(trc, (jsval)QueueGetData(it), "jswinshell/Systray/popupMenuRoots");
}


/**doc
=== Callback functions ===
 The following functions are called when you call ProcessEvents() according the events received by the tray icon.
  * *onfocus*( $TRUE )
  * *onblur*( $FALSE )
  * *onchar*( $STR char )
  * *oncommand*( $STR id, $INT mouseButton )
  * *onmousemove*( $INT mouseX, $INT mouseY )
  * *onmousedown*( $INT mouseButton, $TRUE )
  * *onmouseup*( $INT mouseButton, $FALSE )
  * *onmousedblclick*( $INT mouseButton )
 $H example
 {{{
 var s = new Systray();
 s.icon = new Icon( 0 );
 s.onmousedown = function( button ) {

  MessageBeep();
  s.PopupMenu();
 }
 }}}
**/

CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1)

	HAS_CONSTRUCTOR
	HAS_FINALIZE
	HAS_TRACER

	BEGIN_FUNCTION_SPEC
		FUNCTION(Close)
		FUNCTION(ProcessEvents)
		FUNCTION_FAST(Events)
		FUNCTION(PopupMenu)
		FUNCTION(PopupBalloon)
		FUNCTION(Focus)
		FUNCTION(Position)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_WRITE(icon) // _STORE  is needed to keep the reference to the image ( aboid GC )
		PROPERTY(text)
		PROPERTY_WRITE(visible)
	END_PROPERTY_SPEC

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION(Rect)
	END_STATIC_FUNCTION_SPEC

END_CLASS

/**doc tab:2
=== Examples ===
 $H example 1
 {{{
LoadModule('jswinshell');
LoadModule('jsio');
LoadModule('jsimage');

var s = new Systray();

s.icon = new Icon( DecodePngImage(new File('calendar.png').Open(File.RDONLY)) );
s.text = 'calendar';
s.menu = { exit_cmd:'exit' }

s.onmousedown = function(button) {

  if ( button == 2 )
    s.PopupMenu();
}

s.oncommand = function(id) {

  if ( id == 'exit_cmd' )
    endSignal = true;
}

while ( !endSignal ) {

  s.ProcessEvents();
  Sleep(100);
}
 }}}

 $H example 2
 {{{
 LoadModule('jsstd');
 LoadModule('jswinshell');

 var s = new Systray();
 s.icon = new Icon( 0 );
 s.menu = { add:'Add', exit:'Exit', s1:{ separator:true } };
 s.onmousedown = function( button ) {

 	s.PopupMenu();
 }

 s.oncommand = function( id, button ) {

 	switch ( id ) {
 		case 'exit':
 			return true;
 		case 'add':
 			var fileName = FileOpenDialog( 'executable files|*.exe;*.com;*.cmd;*.bat|all files|*.*' );
 			if ( !fileName )
 				return;
 			var icon = ExtractIcon( fileName );
 			var text = fileName.substr( fileName.lastIndexOf( '\\' ) + 1 );
 			s.menu[fileName] = { icon:icon, text:text };
 			break;
 		default:
 			if ( button == 1 )
 				CreateProcess( id );
 			else
 				if ( MessageBox( 'Remove item: ' + id + '? ', 'Question', MB_YESNO) == IDYES )
 					delete s.menu[id];
 		}
 }

 do { Sleep(100) } while ( !s.ProcessEvents() );
 }}}
**/


/*
http://msdn.microsoft.com/library/default.asp?url=/library/en-us/shellcc/platform/shell/reference/functions/shell_notifyicon.asp

Icons in Win32:
	http://msdn2.microsoft.com/en-us/library/ms997538.aspx

*/
