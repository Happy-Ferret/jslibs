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

#include <js/TracingAPI.h>


DECLARE_CLASS( Icon )


#define SYSTRAY_WINDOW_CLASS_NAME "jslibs_systray"
#define SYSTRAY_ID 13 // doc: Values from 0 to 12 are reserved and should not be used.

#define MSG_TRAY_CALLBACK (WM_USER + 1) // This message has two meanings: tray message + forward
#define MSG_POPUP_MENU (WM_USER + 2)


BOOL FASTCALL
Shell_NotifyIconA_retry(DWORD dwMessage, PNOTIFYICONDATA lpData) {

	BOOL status;
	IFDEBUG( status = FALSE );
	for ( int retry = 4; retry; --retry ) {
		
		status = Shell_NotifyIcon(dwMessage, lpData);
		if ( status == TRUE )
			break;
		Sleep(10);
	};
	return status;
}

S_ASSERT( sizeof(WPARAM) >= sizeof(JS::Value*) );
S_ASSERT( sizeof(UINT_PTR) >= sizeof(JS::Value*) );

struct Private : public jl::CppAllocators {
	NOTIFYICONDATA nid;
	HANDLE thread;
	HANDLE systrayEvent;
	jl::Queue msgQueue;
	CRITICAL_SECTION cs; // protects msgQueue
	jl::Stack<JS::Value> popupMenuRoots;
	POINT lastMousePos;
	bool mouseIn;
};


struct MSGInfo {
	UINT message;
	WPARAM wParam;
	LPARAM lParam;
	BOOL lButton, rButton, mButton;
	BOOL shiftKey, controlKey, altKey;
	POINT mousePos;
};


JS::Value* AddPopupMenuRoot(Private *pv, JS::HandleValue value) {

	*++(pv->popupMenuRoots) = value;
	return &pv->popupMenuRoots;
}

void FreePopupMenuRoots(Private *pv) {

	(pv->popupMenuRoots).Clear();
}


// source: http://www.codeproject.com/shell/ctrayiconposition.asp
BOOL CALLBACK FindTrayWnd(HWND hwnd, LPARAM lParam) {

	TCHAR szClassName[256];
	::GetClassName( hwnd, szClassName, COUNTOF( szClassName )-1);
	if ( jl::strcmp( szClassName, TEXT( "TrayNotifyWnd" ) ) == 0 ) {

		*(HWND*)lParam = hwnd;
		return FALSE;
	}
	return TRUE;
}

// source: http://www.codeproject.com/shell/ctrayiconposition.asp
BOOL CALLBACK FindToolBarInTrayWnd(HWND hwnd, LPARAM lParam) {

	TCHAR szClassName[256];
	GetClassName( hwnd, szClassName, COUNTOF( szClassName ) - 1 );    // Did we find the Main System Tray? If so, then get its size and quit
	if ( jl::strcmp( szClassName, TEXT( "ToolbarWindow32" ) ) == 0 ) {

		*(HWND*)lParam = hwnd;
		return FALSE;
	}
	return TRUE;
}

// source: http://www.codeproject.com/shell/ctrayiconposition.asp
HWND GetTrayNotifyWnd() {

	HWND hWndTrayNotifyWnd = NULL;
	HWND hWndShellTrayWnd = ::FindWindow(TEXT("Shell_TrayWnd"), NULL);
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
	DWORD dwTrayProcessID = (DWORD)-1;
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

		DWORD dwBytesRead = (DWORD)-1;
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

HBITMAP MenuItemBitmapFromIcon(HICON hIcon) {

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


void 
ForwardMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

	MSGInfo *msg = (MSGInfo*)jl_malloc(sizeof(MSGInfo));
	ASSERT( msg != NULL );

	msg->message = message;
	msg->wParam = wParam;
	msg->lParam = lParam;

	// BOOL swapButtons = GetSystemMetrics(SM_SWAPBUTTON); // (TBD) use it ?
	msg->lButton = GetAsyncKeyState(VK_LBUTTON) > 0; // (TBD) check !!
	msg->rButton = GetAsyncKeyState(VK_RBUTTON) > 0; // (TBD) check !!
	msg->mButton = GetAsyncKeyState(VK_MBUTTON) > 0; // (TBD) check !!

	msg->shiftKey   = GetAsyncKeyState(VK_SHIFT)   > 0; // (TBD) check !!
	msg->controlKey = GetAsyncKeyState(VK_CONTROL) > 0; // (TBD) check !!
	msg->altKey     = GetAsyncKeyState(VK_MENU)    > 0; // (TBD) check !!

	Private *pv = (Private*)GetWindowLongPtr(hWnd, GWL_USERDATA);
	ASSERT( pv != NULL );

	if ( message == MSG_TRAY_CALLBACK ) {

		GetCursorPos(&msg->mousePos);
	} else {

		// n/a
		msg->mousePos.x = -1;
		msg->mousePos.y = -1;
	}

	EnterCriticalSection(&pv->cs);
	jl::QueuePush(&pv->msgQueue, msg);
	LeaveCriticalSection(&pv->cs);

	PulseEvent(pv->systrayEvent);
}


BOOL 
FreeMenu( HMENU menu ) {

	for ( int i = GetMenuItemCount(menu); i > 0; i-- ) {

		MENUITEMINFO mii = { sizeof(MENUITEMINFO) };
		mii.fType = MFT_RADIOCHECK; // doc: Displays selected menu items using a radio-button mark instead of a check mark if the hbmpChecked member is NULL.
		mii.fMask = MIIM_CHECKMARKS | MIIM_SUBMENU;
		BOOL st = GetMenuItemInfo(menu, 0, TRUE, &mii);
		JL_IGNORE(st);
		if ( mii.hbmpChecked != NULL )
			DeleteObject(mii.hbmpChecked);
		if ( mii.hSubMenu != NULL )
			FreeMenu(mii.hSubMenu);
		DeleteMenu(menu, 0, MF_BYPOSITION);
	}
	return TRUE;
}


LRESULT CALLBACK 
SystrayWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

	// char dbg[65535];  sprintf(dbg, "message:%x wParam:%x lParam:%x\n", message, wParam, lParam);  OutputDebugString(dbg);

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
		case WM_TIMER: {
			
			Private *pv = (Private*)GetWindowLongPtr(hWnd, GWL_USERDATA);
			ASSERT( pv != NULL );
			ASSERT( pv->mouseIn == true );

			POINT pos;
			GetCursorPos(&pos);
			if ( pos.x != pv->lastMousePos.x && pos.y != pv->lastMousePos.y ) {

				BOOL st = KillTimer(hWnd, 1);
				ASSERT( st );
				pv->mouseIn = false;
				ForwardMessage(hWnd, MSG_TRAY_CALLBACK, 0, WM_MOUSELEAVE);
			}
			break;
		}
		case MSG_TRAY_CALLBACK:

			if ( lParam == WM_MOUSEMOVE ) {

				Private *pv = (Private*)GetWindowLongPtr(hWnd, GWL_USERDATA);
				ASSERT( pv != NULL );
				if ( pv->mouseIn ) {

					GetCursorPos(&pv->lastMousePos);
				} else {

					UINT_PTR timerId = SetTimer(pv->nid.hWnd, 1, 100, NULL);
					ASSERT( timerId );
					pv->mouseIn = true;
					ForwardMessage(hWnd, MSG_TRAY_CALLBACK, 0, WM_MOUSEHOVER);
				}
			}
			ForwardMessage(hWnd, message, wParam, lParam);
			break;

		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_CHAR:
		case WM_SETFOCUS:
		case WM_KILLFOCUS:
		case WM_COMMAND:
			ForwardMessage(hWnd, message, wParam, lParam);
			break;

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



DWORD WINAPI 
SystrayThread( LPVOID lpParam ) {

	Private *pv = (Private*)lpParam;

	HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
	ASSERT( hInst != NULL ); // JL_ASSERT( hInst != NULL, "Unable to GetModuleHandle." );

	WNDCLASS wc = { 0, SystrayWndProc, 0, 0, hInst, NULL, NULL, NULL, NULL, TEXT(SYSTRAY_WINDOW_CLASS_NAME) };
	ATOM rc = RegisterClass(&wc);	// (TBD) do UnregisterClass at the end ?
	ASSERT( rc != 0 || GetLastError() == ERROR_CLASS_ALREADY_EXISTS ); // JL_ASSERT( rc != 0 || GetLastError() == ERROR_CLASS_ALREADY_EXISTS, "Unable to RegisterClass." );

	memset(&pv->nid, 0, sizeof(NOTIFYICONDATA)); // http://msdn.microsoft.com/library/default.asp?url=/library/en-us/shellcc/platform/shell/reference/structures/notifyicondata.asp
	pv->nid.cbSize = sizeof(NOTIFYICONDATA);
	pv->nid.uID = SYSTRAY_ID;
	pv->nid.uFlags = NIF_MESSAGE;
	pv->nid.uCallbackMessage = MSG_TRAY_CALLBACK; // doc: All Message Numbers below 0x0400 are RESERVED.

	// doc: The message loop and window procedure for the window must be in the thread that created the window.
	pv->nid.hWnd = CreateWindow( TEXT( SYSTRAY_WINDOW_CLASS_NAME), NULL, WS_OVERLAPPED, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, HWND_MESSAGE, CreatePopupMenu(), hInst, NULL ); // (TBD) use HWND_MESSAGE ?
	SetWindowLongPtr(pv->nid.hWnd, GWL_USERDATA, (LONG)pv); // make pv available for SystrayWndProc

	BOOL status = Shell_NotifyIconA_retry(NIM_ADD, &pv->nid);
	ASSERT( status );

	PulseEvent(pv->systrayEvent); // first pulse

	BOOL st;
	MSG msg;
	while ( (st = GetMessage(&msg, pv->nid.hWnd, 0, 0)) != 0 ) {

		if ( st == -1 ) // (TBD) manage the error
			break;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	status = Shell_NotifyIconA_retry(NIM_DELETE, &pv->nid);
	ASSERT( status ); // JL_ASSERT( status == TRUE, "Unable to delete notification icon.");

	pv->nid.hWnd = NULL; // see finalizer

	// doc: Before calling UnregisterClass, an application must destroy all windows created with the specified class.
	st = UnregisterClass( TEXT( SYSTRAY_WINDOW_CLASS_NAME ), hInst );
	ASSERT( st || GetLastError() == ERROR_CLASS_HAS_WINDOWS );

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

	JL_DEFINE_ARGS;

	Private *pv = NULL;

	{
	JL_DEFINE_ARGS;
	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

	//Private *pv = (Private*)jl_malloc(sizeof(Private));

	pv = new Private();
	JL_ASSERT_ALLOC( pv );
	JL_updateMallocCounter(cx, sizeof(Private));

	InitializeCriticalSection(&pv->cs);
	jl::QueueInitialize(&pv->msgQueue);
//	jl::QueueInitialize(&pv->popupMenuRoots);

	pv->mouseIn = false;
	pv->systrayEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	JL_ASSERT(pv->systrayEvent, E_OS, E_OBJ, E_CREATE, E_COMMENT("systray event") );
	pv->thread = CreateThread(NULL, 0, SystrayThread, pv, 0, NULL);
	JL_ASSERT(pv->thread, E_OS, E_OBJ, E_CREATE, E_COMMENT("thread"));
	SetThreadPriority(pv->thread, THREAD_PRIORITY_ABOVE_NORMAL);
	WaitForSingleObject(pv->systrayEvent, INFINITE); // first pulse

	JL_SetPrivate(JL_OBJ, pv);
	return true;
	}

bad:
	if ( pv ) {

		if ( pv->thread )
			CloseHandle(pv->thread);
		if ( pv->systrayEvent )
			CloseHandle(pv->systrayEvent);
		delete pv;
	}
	return false;
}


void
CloseSystray(JSRuntime *rt, Private *pv) {

	ASSERT(pv);

	if ( pv->nid.hWnd != NULL ) { // pv->nid.hWnd may have already been closed.

		PostMessage(pv->nid.hWnd, WM_CLOSE, 0, 0); // SendMessage ?
	}

	WaitForSingleObject(pv->thread, INFINITE);
	CloseHandle(pv->thread); // doc: The thread object remains in the system until the thread has terminated and all handles to it have been closed through a call to CloseHandle.
	CloseHandle(pv->systrayEvent);
	
	if ( jl::HostRuntime::getJLRuntime( rt ).skipCleanup() ) { // do not cleanup in unsafe mode ?

		while ( !jl::QueueIsEmpty(&pv->msgQueue) )
			jl_free(jl::QueuePop(&pv->msgQueue));
	}

	DeleteCriticalSection(&pv->cs);
	FreePopupMenuRoots(pv);

	delete pv;
}


DEFINE_FINALIZE() {

	Private *pv = (Private*)js::GetObjectPrivate(obj);
	if ( !pv )
		return;
	CloseSystray(fop->runtime(), pv);
}


/**doc
=== Methods ===
**/

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Close the Systray.
**/
DEFINE_FUNCTION( close ) {

	JL_IGNORE( argc );

	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_SetPrivate(JL_OBJ, NULL);
	CloseSystray(JL_GetRuntime(cx), pv);
	
	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


bool ProcessSystrayMessage( JSContext *cx, JS::HandleObject obj, const MSGInfo *trayMsg, JS::MutableHandleValue rval ) {

	UINT message = trayMsg->message;
	LPARAM lParam = trayMsg->lParam;
	WPARAM wParam = trayMsg->wParam;

	JS::RootedValue functionVal(cx);

//	char dbg[65535];  sprintf(dbg, "msg: %x\n", message);  OutputDebugString(dbg);

	switch ( message ) {

		case WM_SETFOCUS:
			JL_CHK( jl::getProperty(cx, obj, "onfocus", &functionVal) );
			if ( jl::isCallable(cx, functionVal) )
				JL_CHK( jl::call(cx, obj, functionVal, rval, true) );
			break;

		case WM_KILLFOCUS:
			JL_CHK( jl::getProperty(cx, obj, "onblur", &functionVal) );
			if ( jl::isCallable(cx, functionVal) )
				JL_CHK( jl::call(cx, obj, functionVal, rval, false) );
			break;

		case WM_CHAR:
			JL_CHK( jl::getProperty(cx, obj, "onchar", &functionVal) );
			if ( jl::isCallable(cx, functionVal) ) {

				wchar_t c = wParam & 0xffff;
				JL_CHK( jl::call(cx, obj, functionVal, rval, jl::strSpec(&c,1)) );
			}
			break;

		case WM_ENDSESSION: // case WM_QUERYENDSESSION:
		case WM_CLOSE:
			JL_CHK( jl::getProperty(cx, obj, "onclose", &functionVal) );
			if ( jl::isCallable(cx, functionVal) ) {

				JL_CHK( jl::call(cx, obj, functionVal, rval, message == WM_ENDSESSION && lParam == 0) );
			}
			break;

		case WM_COMMAND:
			JL_CHK( jl::getProperty(cx, obj, "oncommand", &functionVal) );
			if ( jl::isCallable(cx, functionVal) ) {

				JS::RootedValue key(cx, *(JS::Value*)wParam);
				JL_CHK( jl::call(cx, obj, functionVal, rval, key, trayMsg->lButton ? 1 : trayMsg->rButton ? 2 : 0) );
				Private *pv = (Private*)JL_GetPrivate(obj);
				if ( pv )
					FreePopupMenuRoots(pv);
			}
			break;

		case WM_SYSCOMMAND: // avoid any system command (including Alt-F4) from systray icon.

			switch ( wParam ) {
				
				case SC_SCREENSAVE:
				case SC_MONITORPOWER:
					JL_CHK( jl::getProperty(cx, obj, "onidle", &functionVal) );
					if ( jl::isCallable(cx, functionVal) ) {

						JS::RootedValue key(cx, *(JS::Value*)wParam);
						JL_CHK( jl::call(cx, obj, functionVal, rval, key, trayMsg->lButton ? 1 : trayMsg->rButton ? 2 : 0) );
					}
				break;
			}
			break;

		case MSG_TRAY_CALLBACK:

			switch ( lParam ) {

				case WM_MOUSEHOVER:
					JL_CHK( jl::getProperty(cx, obj, "onmouseenter", &functionVal) );
					if ( jl::isCallable(cx, functionVal) )
						JL_CHK( jl::call(cx, obj, functionVal, rval, true) );
					break;

				case WM_MOUSELEAVE:
					JL_CHK( jl::getProperty(cx, obj, "onmouseleave", &functionVal) );
					if ( jl::isCallable(cx, functionVal) )
						JL_CHK( jl::call(cx, obj, functionVal, rval, false) );
					break;

				case WM_MOUSEMOVE:
					JL_CHK( jl::getProperty(cx, obj, "onmousemove", &functionVal) );
					if ( jl::isCallable(cx, functionVal) )
						JL_CHK( jl::call(cx, obj, functionVal, rval, trayMsg->mousePos.x, trayMsg->mousePos.y) );
					break;

				case WM_LBUTTONDOWN:
				case WM_MBUTTONDOWN:
				case WM_RBUTTONDOWN:
					JL_CHK( jl::getProperty(cx, obj, "onmousedown", &functionVal) );
					if ( jl::isCallable(cx, functionVal) )
						JL_CHK( jl::call(cx, obj, functionVal, rval, lParam==WM_LBUTTONDOWN ? 1 : lParam==WM_RBUTTONDOWN ? 2 : lParam==WM_MBUTTONDOWN ? 3 : 0, true) );
					break;

				case WM_LBUTTONUP:
				case WM_MBUTTONUP:
				case WM_RBUTTONUP:
					JL_CHK( jl::getProperty(cx, obj, "onmouseup", &functionVal) );
					if ( jl::isCallable(cx, functionVal) )
						JL_CHK( jl::call(cx, obj, functionVal, rval, lParam==WM_LBUTTONUP ? 1 : lParam==WM_RBUTTONUP ? 2 : lParam==WM_MBUTTONUP ? 3 : 0, false) );
					break;

				case WM_LBUTTONDBLCLK:
				case WM_MBUTTONDBLCLK:
				case WM_RBUTTONDBLCLK:
					JL_CHK( jl::getProperty(cx, obj, "onmousedblclick", &functionVal) );
					if ( jl::isCallable(cx, functionVal) )
						JL_CHK( jl::call(cx, obj, functionVal, rval, lParam==WM_LBUTTONDBLCLK ? 1 : lParam==WM_RBUTTONDBLCLK ? 2 : lParam==WM_MBUTTONDBLCLK ? 3 : 0) );
					break;
			} // switch lParam
	} //  switch message

	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME()
  Precess all pending events of the systray.
  The function returns true if at least one of the event function ( see Remarks below ) returns true.
**/
DEFINE_FUNCTION( processEvents ) {

	JL_IGNORE( argc );

	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	MSGInfo *trayMsg;
	while ( !jl::QueueIsEmpty(&pv->msgQueue) ) {

		EnterCriticalSection(&pv->cs);
		trayMsg = (MSGInfo*)jl::QueueShift(&pv->msgQueue);
		LeaveCriticalSection(&pv->cs);
		JL_CHK( ProcessSystrayMessage(cx, JL_OBJ, trayMsg, JL_RVAL) );
		jl_free(trayMsg);
	}	
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE HANDLE $INAME()
  Passively waits for Systray events through the processEvents function.
**/

struct SystrayEvent : public ProcessEvent2 {
	HANDLE cancelEvent;
	HANDLE systrayEvent;

	~SystrayEvent() {

		CloseHandle(cancelEvent);
		CloseHandle(systrayEvent);
	}

	bool prepareWait(JSContext *cx, JS::HandleObject obj) {
	
		return true;
	}

	void startWait() {

		HANDLE events[] = { systrayEvent, cancelEvent };
		DWORD status = WaitForMultipleObjects(COUNTOF(events), events, FALSE, INFINITE);
		ASSERT( status != WAIT_FAILED );
	}

	bool cancelWait() {

		SetEvent(cancelEvent);
		return true;
	}

	bool endWait(bool *hasEvent, JSContext *cx, JS::HandleObject) {

		JS::RootedObject systrayObj(cx, &slot(0).toObject());

		Private *pv = (Private*)JL_GetPrivate(systrayObj);
	
		if ( pv == NULL ) { // maybe Systray::close() has been called in between

			return true;
		}

		*hasEvent = !jl::QueueIsEmpty(&pv->msgQueue);

		bool ok;
		JS::RootedValue rval(cx);
		MSGInfo *trayMsg;
		while ( !jl::QueueIsEmpty(&pv->msgQueue) ) {

			EnterCriticalSection(&pv->cs);
			trayMsg = (MSGInfo*)jl::QueueShift(&pv->msgQueue);
			LeaveCriticalSection(&pv->cs);
			ok = ProcessSystrayMessage(cx, systrayObj, trayMsg, &rval);
			jl_free(trayMsg);
			JL_CHK( ok );

			if ( JL_GetPrivate(systrayObj) == NULL ) // maybe Systray::close() has been called in between
				break;
		}	
		return true;
		JL_BAD;
	}
};


DEFINE_FUNCTION( events ) {
	
	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC(0);

	Private *pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	SystrayEvent *upe = new SystrayEvent();
	JL_ASSERT_ALLOC(upe);
	JL_CHK( HandleCreate(cx, upe, JL_RVAL) );

	upe->slot(0).set(JL_OBJVAL);

	upe->cancelEvent = CreateEvent(NULL, FALSE, FALSE, NULL); // auto-reset
	if ( upe->cancelEvent == NULL )
		JL_CHK( jl::throwOSError(cx) );

	// need to dup. the handle because the original one may be closed in Systray::close()
	HANDLE currentProcess = GetCurrentProcess();
	BOOL st = DuplicateHandle(currentProcess, pv->systrayEvent, currentProcess, &upe->systrayEvent, 0, FALSE, DUPLICATE_SAME_ACCESS);
	if ( !st )
		JL_CHK( jl::throwOSError(cx) );

	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Puts the systray into the foreground. Keyboard input is directed to the systray.
**/
DEFINE_FUNCTION( focus ) {

	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	SetForegroundWindow(pv->nid.hWnd);

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}

ALWAYS_INLINE bool NormalizeMenuInfo( JSContext *cx, JS::HandleObject obj, JS::HandleValue key, JS::MutableHandleValue value ) {

	if ( jl::isCallable(cx, value) )
		return jl::call(cx, obj, value, value, key);
	return true;
}


bool FillMenu( JSContext *cx, JS::HandleObject systrayObj, JS::HandleObject menuObj, HMENU *hMenu ) {

//	*hMenu = CreatePopupMenu();
	ASSERT( *hMenu != NULL );

	unsigned menuLength;
	JL_CHK( JS_GetArrayLength(cx, menuObj, &menuLength) );

	for ( unsigned i = 0; i < menuLength; ++i ) {
		
		UINT uFlags = 0;
		bool isDefault = false;
		HBITMAP hBMP = NULL;
		HMENU popupMenu;
		IFDEBUG( popupMenu = NULL ); // avoid "potentially uninitialized local variable" warning

		JS::RootedValue cmdid(cx);
		JS::RootedValue label(cx);
		JS::RootedValue key(cx); // JS::NumberValue(i);
		JS::RootedValue item(cx);

		JL_CHK( jl::getElement(cx, menuObj, i, &item) );
		key.setInt32(i);
		JL_CHK( NormalizeMenuInfo(cx, menuObj, key, &item) );

		if ( item.isNull() || item.isUndefined() ) {

			uFlags |= MF_SEPARATOR;
		} else 
		if ( jl::isString(cx, item) ) {

			label = item;
			cmdid = item;
		} else
		if ( item.isObject() ) {

			JS::RootedObject itemObj(cx, &item.toObject());
			JS::RootedValue value(cx);

			JS::AutoIdArray ida(cx, JS_Enumerate(cx, itemObj));
			JL_CHK( ida );

			size_t length = ida.length();
			for ( size_t j = 0; j < length; ++j ) {

				jl::BufString keyStr;
				JS::RootedId itemId(cx, ida[j]);

				JL_CHK( JS_IdToValue(cx, itemId, &key) );
				JL_CHK( jl::getProperty(cx, itemObj, itemId, &value) );
		
				JL_CHK( jl::getValue(cx, key, &keyStr) );

				if ( jl::strcmp(keyStr, "id") == 0 ) {

					JL_CHK( NormalizeMenuInfo(cx, itemObj, key, &value) );
					cmdid = value;
					continue;
				}
				if ( jl::strcmp( keyStr, "label" ) == 0 ) {

					JL_CHK( NormalizeMenuInfo(cx, itemObj, key, &value) );
					label = value;
					continue;
				}
				if ( jl::strcmp( keyStr, "break" ) == 0 ) {
					
					JL_CHK( NormalizeMenuInfo(cx, itemObj, key, &value) );
					bool b;
					JL_CHK( jl::getValue(cx, value, &b) );
					if ( b )
						uFlags |= MF_MENUBARBREAK;
					continue;
				}
				if ( jl::strcmp( keyStr, "checked" ) == 0 ) {
					
					JL_CHK( NormalizeMenuInfo(cx, itemObj, key, &value) );
					bool b;
					JL_CHK( jl::getValue(cx, value, &b) );
					if ( b )
						uFlags |= MF_CHECKED;
					continue;
				}
				if ( jl::strcmp( keyStr, "grayed" ) == 0 ) {
					
					JL_CHK( NormalizeMenuInfo(cx, itemObj, key, &value) );
					bool b;
					JL_CHK( jl::getValue(cx, value, &b) );
					if ( b )
						uFlags |= MF_GRAYED;
					continue;
				}
				if ( jl::strcmp( keyStr, "disabled" ) == 0 ) {
					
					JL_CHK( NormalizeMenuInfo(cx, itemObj, key, &value) );
					bool b;
					JL_CHK( jl::getValue(cx, value, &b) );
					if ( b )
						uFlags |= MF_DISABLED;
					continue;
				}
				if ( jl::strcmp( keyStr, "default" ) == 0 ) {
					
					JL_CHK( NormalizeMenuInfo(cx, itemObj, key, &value) );
					JL_CHK( jl::getValue(cx, value, &isDefault) );
					continue;
				}
				if ( jl::strcmp( keyStr, "icon" ) == 0 ) {

					JL_CHK( NormalizeMenuInfo(cx, itemObj, key, &value) );
					JL_ASSERT_IS_OBJECT(value, (const char*)keyStr);
					
					JS::RootedObject iconObj(cx, &value.toObject());
					JL_ASSERT_INSTANCE( iconObj, JL_CLASS(Icon) );
					HICON *phIcon = (HICON*)JL_GetPrivate(iconObj);
					JL_ASSERT_OBJECT_STATE(phIcon, JL_CLASS_NAME(Icon));
					hBMP = MenuItemBitmapFromIcon(*phIcon);
					JL_ASSERT( hBMP != NULL, E_STR("icon"), E_CREATE );
					continue;
				}
				if ( jl::strcmp( keyStr, "popup" ) == 0 ) {

					JL_CHK( NormalizeMenuInfo(cx, itemObj, key, &value) );
					JL_ASSERT_IS_OBJECT(value, (const char*)keyStr);
					uFlags |= MF_POPUP;
					popupMenu = CreatePopupMenu();
					JS::RootedObject tmp(cx, &value.toObject());
					JL_CHK( FillMenu(cx, systrayObj, tmp, &popupMenu) );
					continue;
				}
			}
		} else {

			JL_ERR( E_STR("menu item"), E_INVALID );
		}


		jl::BufString newItemStr;
		LPCTSTR lpNewItem;
		UINT_PTR uIDNewItem;

		if ( uFlags & MF_SEPARATOR ) {
			
			lpNewItem = NULL;
		} else
		if ( jl::isClass(cx, label, JL_CLASS(Icon) ) ) {

			uFlags |= MF_BITMAP;
			HICON *phIcon = (HICON*)JL_GetPrivate(label);
			JL_ASSERT_OBJECT_STATE(phIcon, JL_CLASS_NAME(Icon));
			hBMP = MenuItemBitmapFromIcon(*phIcon);
			JL_ASSERT( hBMP != NULL, E_STR("icon"), E_CREATE );
			lpNewItem = (LPCTSTR)phIcon;
		} else {

			if ( label != JSVAL_VOID ) {

				JL_CHK( jl::getValue(cx, label, &newItemStr) );
				
				lpNewItem = newItemStr.toStringZ<PCTSTR>();
				uFlags |= MF_STRING;
			} else {

				JL_ERR( E_PROP, E_NAME("label"), E_DEFINED );
			}
		}



		if ( uFlags & MF_SEPARATOR ) {
			
			uIDNewItem = 0;
		} else
		if ( uFlags & MF_POPUP ) {

			uIDNewItem = (UINT_PTR)popupMenu; // ignore warning C4703
		} else {

			if ( cmdid == JSVAL_VOID )
				cmdid = label;
			uIDNewItem = (UINT_PTR)AddPopupMenuRoot((Private*)JL_GetPrivate(systrayObj), item);
		}

		BOOL res = AppendMenu(*hMenu, uFlags, uIDNewItem, lpNewItem);
		ASSERT( res );

		if ( isDefault )
			SetMenuDefaultItem(*hMenu, i, TRUE);

		if ( hBMP != NULL ) {

			res = SetMenuItemBitmaps(*hMenu, i, MF_BYPOSITION, hBMP, hBMP); // doc: When the menu is destroyed, these bitmaps are not destroyed; it is up to the application to destroy them.
			if ( res == 0 )
				return jl::throwOSError(cx);
		}
	}

	return true;
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
systray.popupMenu([ 
  'Start',
  'Stop',
  '---',
  function() { return 'uptime: '+new Date() },
  { status:'sub', popup:function() { return [ 1,2,3,4,5 ] } },
  '---',
  { text:'Exit', id:'do_exit', icon:iconRedCross }
]);
}}}
**/
DEFINE_FUNCTION( popupMenu ) {

	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_OBJECT(1);

	Private *pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	
	BOOL st;

	HMENU hMenu = GetMenu(pv->nid.hWnd);
	JL_ASSERT_THIS_OBJECT_STATE( hMenu );

//	st = DestroyMenu(hMenu);
//	ASSERT( st );
	
	// there is no way to detect the menu popup has been closed. Here we free previous items.
	{
	FreePopupMenuRoots(pv);
	JS::RootedObject tmp(cx, &JL_ARG(1).toObject());
	JL_CHK( FillMenu(cx, JL_OBJ, tmp, &hMenu) );
	st = PostMessage(pv->nid.hWnd, MSG_POPUP_MENU, 0, 0);
	ASSERT( st );
	}

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( balloonObject )
  Displays a systray balloon.
*/
DEFINE_FUNCTION( popupBalloon ) {

	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	BOOL status;

	if ( JL_ARG_ISDEF(1) ) {
	
		JL_ASSERT_ARG_IS_OBJECT(1);
		JS::RootedObject infoObj(cx, &JL_ARG(1).toObject());

		pv->nid.dwInfoFlags = NIIF_NONE;

		JL_CHK( jl::getProperty(cx, infoObj, "infoTitle", JL_RVAL) );
		if ( !JL_RVAL.isUndefined() ) {

			jl::BufString infoTitle;
			JL_CHK( jl::getValue(cx, JL_RVAL, &infoTitle) );

			//size_t len = jl::min(sizeof(pv->nid.szInfo)-1, infoTitle.length());
			//jl::memcpy( pv->nid.szInfoTitle, infoTitle.toData<const char*>(), jl::min(sizeof(pv->nid.szInfoTitle)-1, infoTitle.length()) );

			size_t copiedLength;
			copiedLength = infoTitle.copyTo(pv->nid.szInfoTitle, sizeof(pv->nid.szInfoTitle)-1);
			pv->nid.szInfoTitle[copiedLength] = 0;
		}

		JL_CHK( jl::getProperty(cx, infoObj, "info", JL_RVAL) );
		if ( !JL_RVAL.isUndefined() ) {

			jl::BufString infoStr;
			JL_CHK( jl::getValue(cx, JL_RVAL, &infoStr) );
			//size_t len = jl::min(sizeof(pv->nid.szInfo)-1, infoStr.length());
			//JL_IGNORE(len);
			//jl::memcpy( pv->nid.szInfo, infoStr.toData<const char*>(), infoStr.length() );
			//JL_ASSERT( infoStr.length() < sizeof(pv->nid.szInfo), E_

			size_t copiedLength;
			copiedLength = infoStr.copyTo(pv->nid.szInfo, sizeof(pv->nid.szInfo)-1);
			pv->nid.szInfo[copiedLength] = 0;
		}

		JL_CHK( jl::getProperty(cx, infoObj, "icon", JL_RVAL) );
		if ( !JL_RVAL.isUndefined() ) {

			jl::BufString iconNameStr;
			JL_CHK( jl::getValue(cx, JL_RVAL, &iconNameStr) );
			
			if ( strcmp(iconNameStr, TEXT("info") ) == 0 )
				pv->nid.dwInfoFlags |= NIIF_INFO;
			else
			if ( strcmp( iconNameStr, TEXT( "warning") ) == 0 )
				pv->nid.dwInfoFlags |= NIIF_WARNING;
			else
			if ( strcmp( iconNameStr, TEXT( "error") ) == 0 )
				pv->nid.dwInfoFlags |= NIIF_ERROR;
		}

//		pv->nid.uTimeout = 10000;

		pv->nid.uFlags |= NIF_INFO;
		status = Shell_NotifyIconA_retry(NIM_MODIFY, &pv->nid);
	} else {

		pv->nid.szInfo[0] = '\0';
		pv->nid.uFlags |= NIF_INFO;
		status = Shell_NotifyIconA_retry(NIM_MODIFY, &pv->nid);
	}
	JL_ASSERT( status == TRUE, E_THISOBJ, E_INTERNAL );

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/*
DEFINE_FUNCTION( callDefault ) {

	PNOTIFYICONDATA nid = (PNOTIFYICONDATA)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(nid);
	HMENU hMenu = GetMenu(nid->hWnd);
	JL_ASSERT_THIS_OBJECT_STATE(hMenu);
!!! menu is empty !!!
	jsid id = GetMenuDefaultItem( hMenu, FALSE, GMDI_USEDISABLED ); // http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winui/winui/windowsuserinterface/resources/menus/menureference/menufunctions/getmenudefaultitem.asp
	DWORD err = GetLastError();

	jsval functionVal;
	jl::getProperty(cx, obj, "oncommand", &functionVal);
	if ( !JSVAL_IS_VOID( functionVal ) ) {

		jsval key;
		JL_CHK( JS_IdToValue(cx, id, &key) );
		JL_CHK( CallFunction( cx, obj, functionVal, rval, 1, key ) );
	}
	return true;
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

   var pos = systray.position();
   print( x-pos[0], ', ', y-pos[1], '\n' );
  }
  }}}
**/
DEFINE_FUNCTION( position ) {

	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	RECT r;
	BOOL res = FindOutPositionOfIconDirectly( pv->nid.hWnd, pv->nid.uID, &r );
	if ( res != TRUE )
		return jl::throwOSError(cx);

	LONG v[] = { r.left, r.top };
	JL_CHK( jl::setVector(cx, JL_RVAL, v, COUNTOF(v), JL_ARGC >= 1 && JL_ARG(1).isObject()) );

	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME( [reusableArray] )
  Returns the dimensions [left, top, width, height] of the systray rectangle.
  $H note
   If you provide a _reusableArray_, the function will use it to store the values.
**/
DEFINE_FUNCTION( rect ) {

	JL_DEFINE_ARGS;
	HWND hWndTrayWnd = GetTrayNotifyWnd();
	if ( !hWndTrayWnd )
		return jl::throwOSError(cx);
	RECT rect;
	BOOL st = GetWindowRect(hWndTrayWnd, &rect);
	if ( !st )
		return jl::throwOSError(cx);

	LONG v[] = { rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top };
	JL_CHK( jl::setVector(cx, JL_RVAL, v, COUNTOF(v), JL_ARGC >= 1 && JL_ARG(1).isObject()) );

	return true;
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
DEFINE_PROPERTY_SETTER( icon ) {

	JL_DEFINE_PROP_ARGS;

	JL_ASSERT_THIS_INSTANCE();

	HICON hIcon;
	if ( vp.isObject() ) {

		JS::RootedObject iconObj(cx, &vp.toObject());
		JL_ASSERT_INSTANCE( iconObj, JL_CLASS(Icon) );
		HICON *phIcon = (HICON*)JL_GetPrivate(iconObj);
		JL_ASSERT_OBJECT_STATE( phIcon, JL_CLASS_NAME(Icon) );
		hIcon = *phIcon;
	} else if ( vp.isNullOrUndefined() ) {

		hIcon = NULL;
	} else {

		JL_ERR( E_STR("icon"), E_INVALID );
	}

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	pv->nid.hIcon = hIcon;
	pv->nid.uFlags |= NIF_ICON;

	BOOL status = Shell_NotifyIconA_retry(NIM_MODIFY, &pv->nid);
	JL_ASSERT( status == TRUE, E_THISOBJ, E_INTERNAL );

	return jl::StoreProperty(cx, obj, id, vp, false);
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $WRITEONLY
  Show or hide the systray icon.
  $H beware
   you cannot use this property to get the current visibility of the icon.
**/
DEFINE_PROPERTY_SETTER( visible ) {

	JL_DEFINE_PROP_ARGS;

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	bool state;
	state = JS::ToBoolean(vp);
	
	BOOL status = Shell_NotifyIconA_retry( state == true ? NIM_ADD : NIM_DELETE, &pv->nid);
	JL_ASSERT( status == TRUE, E_THISOBJ, E_INTERNAL );

	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $STR $INAME
  Get of set the tooltip text of the systray icon.
**/
DEFINE_PROPERTY_SETTER( text ) {

	JL_DEFINE_PROP_ARGS;

	jl::BufString tipText;

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JL_CHK( jl::getValue(cx, vp, &tipText) );

	//size_t len = jl::min(sizeof(pv->nid.szTip)-1, tipText.length());
	//jl::memcpy(pv->nid.szTip, tipText.GetConstStr(), tipText.length());
	
	size_t copiedLength;
	copiedLength = tipText.copyTo(pv->nid.szTip, sizeof(pv->nid.szTip)-1);
	pv->nid.szTip[copiedLength] = 0;

	pv->nid.uFlags |= NIF_TIP;

	BOOL status = Shell_NotifyIconA_retry(NIM_MODIFY, &pv->nid);
	JL_ASSERT( status == TRUE, E_THISOBJ, E_INTERNAL );

	return true;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( text ) {

	JL_DEFINE_PROP_ARGS;

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	if ( pv->nid.uFlags & NIF_TIP )
		JL_CHK( jl::setValue( cx, vp, pv->nid.szTip ) );
	return true;
	JL_BAD;
}


DEFINE_TRACER() {

	Private *pv = (Private*)js::GetObjectPrivate(obj);
	if ( pv ) {

		struct Tmp {
			JSTracer *_trc;
			Tmp( JSTracer *trc ) : _trc(trc) {
			}

			bool operator()( JS::Value &value ) {

				JS_CallValueTracer(_trc, &value, "jswinshell/Systray/popupMenuRoots");
				return false;
			}
		} tmp(trc);
		pv->popupMenuRoots.BackForEach( tmp );
	}
}


/**doc
=== Callback functions ===
 The following functions are called when you call processEvents() according the events received by the tray icon.
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

  messageBeep();
  s.popupMenu();
 }
 }}}
**/

CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision$"))
	HAS_PRIVATE

	HAS_CONSTRUCTOR
	HAS_FINALIZE
	HAS_TRACER

	BEGIN_FUNCTION_SPEC
		FUNCTION(close)
		FUNCTION(processEvents)
		FUNCTION(events)
		FUNCTION(popupMenu)
		FUNCTION(popupBalloon)
		FUNCTION(focus)
		FUNCTION(position)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_SETTER(icon) // _STORE  is needed to keep the reference to the image ( aboid GC )
		PROPERTY(text)
		PROPERTY_SETTER(visible)
	END_PROPERTY_SPEC

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION(rect)
	END_STATIC_FUNCTION_SPEC

END_CLASS

/**doc tab:2
=== Examples ===
 $H example 1
 {{{
loadModule('jswinshell');
loadModule('jsio');
loadModule('jsimage');

var s = new Systray();

s.icon = new Icon( decodePngImage(new File('calendar.png').open(File.RDONLY)) );
s.text = 'calendar';
s.menu = { exit_cmd:'exit' }

s.onmousedown = function(button) {

  if ( button == 2 )
    s.popupMenu();
}

s.oncommand = function(id) {

  if ( id == 'exit_cmd' )
    host.endSignal = true;
}

while ( !host.endSignal ) {

  s.processEvents();
  sleep(100);
}
 }}}

 $H example 2
 {{{
 loadModule('jsstd');
 loadModule('jswinshell');

 var s = new Systray();
 s.icon = new Icon( 0 );
 s.menu = { add:'Add', exit:'Exit', s1:{ separator:true } };
 s.onmousedown = function( button ) {

 	s.popupMenu();
 }

 s.oncommand = function( id, button ) {

 	switch ( id ) {
 		case 'exit':
 			return true;
 		case 'add':
 			var fileName = fileOpenDialog( 'executable files|*.exe;*.com;*.cmd;*.bat|all files|*.*' );
 			if ( !fileName )
 				return;
 			var icon = extractIcon( fileName );
 			var text = fileName.substr( fileName.lastIndexOf( '\\' ) + 1 );
 			s.menu[fileName] = { icon:icon, text:text };
 			break;
 		default:
 			if ( button == 1 )
 				createProcess( id );
 			else
 				if ( messageBox( 'Remove item: ' + id + '? ', 'Question', MB_YESNO) == IDYES )
 					delete s.menu[id];
 		}
 }

 do { sleep(100) } while ( !s.processEvents() );
 }}}
**/


/*
http://msdn.microsoft.com/library/default.asp?url=/library/en-us/shellcc/platform/shell/reference/functions/shell_notifyicon.asp

Icons in Win32:
	http://msdn2.microsoft.com/en-us/library/ms997538.aspx

*/
