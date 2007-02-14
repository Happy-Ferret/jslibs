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


/*
static LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

	return DefWindowProc(hWnd, message, wParam, lParam); // We do not want to handle this message so pass back to Windows to handle it in a default way
}
*/

BEGIN_CLASS( Systray )

DEFINE_FINALIZE() {

	NOTIFYICONDATA *nid = (NOTIFYICONDATA*)JS_GetPrivate(cx, obj);
	if ( nid != NULL ) {

		BOOL status = Shell_NotifyIcon(NIM_DELETE, nid);
//		RT_ASSERT( status == TRUE, "Unable to setup systray icon." );

	}
}

DEFINE_CONSTRUCTOR() {

	RT_ASSERT_CONSTRUCTING(_class);

	HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
	RT_ASSERT( hInst != NULL, "Unable to GetModuleHandle." );
	WNDCLASS wc = { 0, (WNDPROC)DefWindowProc, 0, 0, hInst, NULL, NULL, NULL, NULL, L"systray" };
	ATOM rc = RegisterClass(&wc);
	RT_ASSERT( rc != 0, "Unable to RegisterClass." );
	HWND hWnd = CreateWindow( (LPCWSTR)rc, NULL, WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, (HWND)NULL, (HMENU)NULL, hInst, (LPVOID)NULL );
	RT_ASSERT( hWnd != NULL, "Unable to CreateWindow." );
//	JS_SetPrivate(cx, obj, hWnd);

	NOTIFYICONDATA *nid = (NOTIFYICONDATA*)malloc( sizeof(NOTIFYICONDATA) );
	memset(nid, 0, sizeof(NOTIFYICONDATA));
	nid->cbSize = sizeof(NOTIFYICONDATA);
	nid->hWnd = hWnd;
	nid->uID = (12) + 0; // doc: Values from 0 to 12 are reserved and should not be used.
	nid->uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	nid->uCallbackMessage = (WM_USER) + 0; // doc: All Message Numbers below 0x0400 are RESERVED.
	nid->hIcon = LoadIcon(NULL,MAKEINTRESOURCE(IDI_APPLICATION));
	wcscpy( nid->szTip, L"text" );

	BOOL status = Shell_NotifyIcon(NIM_ADD, nid);
	RT_ASSERT( status == TRUE, "Unable to setup systray icon." );

	JS_SetPrivate(cx, obj, (void*)nid);

//	Shell_NotifyIcon(NIM_MODIFY, &nid);
//	Shell_NotifyIcon(NIM_DELETE, &nid);

// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/shellcc/platform/shell/reference/structures/notifyicondata.asp
//	_tcscpy(nid.szTip, "tooltip");

	return JS_TRUE;
}


CONFIGURE_CLASS

	HAS_CONSTRUCTOR
	HAS_FINALIZE

//DEFINE_FUNCTION( Call ) {
//	return JS_TRUE;
//}

//DEFINE_PROPERTY( prop ) {
//	return JS_TRUE;
//}

//DEFINE_FUNCTION( Func ) {
//	return JS_TRUE;
//}

	BEGIN_FUNCTION_SPEC
//		FUNCTION(Func)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
//		PROPERTY(prop)
	END_PROPERTY_SPEC

	HAS_PRIVATE
//	HAS_RESERVED_SLOTS(1)

END_CLASS

/*


http://msdn.microsoft.com/library/default.asp?url=/library/en-us/shellcc/platform/shell/reference/functions/shell_notifyicon.asp


*/
