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

#include "icon.h"
#include <stdlib.h>

#include <Commdlg.h>
#include <shlobj.h>

BEGIN_STATIC

/**doc
=== Static functions ===
**/

/**doc
 * *ExtractIcon*( fileName [, iconIndex ] )
  TBD
**/
DEFINE_FUNCTION( _ExtractIcon ) {
	
	RT_ASSERT_ARGC(1);
	const char *fileName;
	UINT iconIndex = 0;
	RT_JSVAL_TO_STRING( argv[0], fileName );
	if ( argc >= 2 )
		RT_JSVAL_TO_INT32( argv[1], iconIndex );
	HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
	RT_ASSERT( hInst != NULL, "Unable to GetModuleHandle." );
	HICON hIcon = ExtractIcon( hInst, fileName, iconIndex ); // see SHGetFileInfo(
	if ( hIcon == NULL )
		return WinThrowError(cx, GetLastError());
	JSObject *icon = JS_NewObject(cx, &classIcon, NULL, NULL);
	HICON *phIcon = (HICON*)malloc(sizeof(HICON)); // this is needed because JS_SetPrivate stores ONLY alligned values
	RT_ASSERT_ALLOC( phIcon );
	*phIcon = hIcon;
	JS_SetPrivate(cx, icon, phIcon);
	*rval = OBJECT_TO_JSVAL(icon);
	return JS_TRUE;
}

/**doc
 * *MessageBox*( content [, caption [, style ] ] )
  TBD
**/
DEFINE_FUNCTION( _MessageBox ) {
	
	RT_ASSERT_ARGC(1);

	const char *text;
	RT_JSVAL_TO_STRING( argv[0], text );

	const char *caption = NULL;
	if ( argc >= 2 && argv[1] != JSVAL_VOID )
		RT_JSVAL_TO_STRING( argv[1], caption );
	
	UINT type = 0;
	if ( argc >= 3 )
		RT_JSVAL_TO_INT32( argv[2], type );

	int res = MessageBox(NULL, text, caption, type);
	RT_ASSERT( res != 0, "MessageBox call Failed." );
	*rval = INT_TO_JSVAL( res );
	return JS_TRUE;
}


/**doc
 * *CreateProcess*( applicationName , [ commandLine ], [ environment ], [ currentDirectory ] )
  TBD
 ===== example: =====
 {{{
 CreateProcess( 'C:\\WINDOWS\\system32\\calc.exe', undefined, undefined, 'c:\\' );
 }}}
**/
DEFINE_FUNCTION( _CreateProcess ) {

	RT_ASSERT_ARGC(1);

	const char *applicationName, *commandLine = NULL, *environment = NULL, *currentDirectory = NULL;

	RT_JSVAL_TO_STRING( argv[0], applicationName );

	if ( argc >= 2 && argv[1] != JSVAL_VOID )
		RT_JSVAL_TO_STRING( argv[1], commandLine );

	if ( argc >= 3 && argv[2] != JSVAL_VOID  )
		RT_JSVAL_TO_STRING( argv[2], environment );

	if ( argc >= 4 && argv[3] != JSVAL_VOID  )
		RT_JSVAL_TO_STRING( argv[3], currentDirectory );

	STARTUPINFO si = { sizeof(STARTUPINFO) };
	PROCESS_INFORMATION pi;
	BOOL st = CreateProcess( applicationName, (LPSTR)commandLine, NULL, NULL, FALSE, 0, (LPVOID)environment, currentDirectory, &si, &pi ); // doc: http://msdn2.microsoft.com/en-us/library/ms682425.aspx
	if ( st = FALSE )
		return WinThrowError(cx, GetLastError());
	return JS_TRUE;
}


/**doc
 * *FileOpenDialog*( filters | `undefined` [, defaultFileName ] );
  TBD
 ===== example: =====
 {{{
 FileOpenDialog( 'executable files|*.exe;*.com;*.cmd;*.bat|all files|*.*' );
 }}}
**/
DEFINE_FUNCTION( FileOpenDialog ) {

	OPENFILENAME ofn = { sizeof(OPENFILENAME) };
	char fileName[MAX_PATH];
	char filter[255];

	if ( argc >= 1 && argv[0] != JSVAL_VOID ) {
		
		const char *str;
		size_t len;
		RT_JSVAL_TO_STRING_AND_LENGTH( argv[0], str, len );
		strcpy( filter, str );
		for ( char *tmp = filter; tmp = strchr( tmp, '|' ); tmp++ )
			*tmp = '\0'; // doc: Pointer to a buffer containing pairs of null-terminated filter strings. 
		filter[len+1] = '\0'; // The last string in the buffer must be terminated by two NULL characters.
		ofn.lpstrFilter = filter;
	}

	if ( argc >= 2 && argv[1] != JSVAL_VOID ) {

		const char *tmp;
		RT_JSVAL_TO_STRING( argv[1], tmp );
		strcpy( fileName, tmp );
	} else {
		*fileName = '\0';
	}

	ofn.lpstrFile = fileName;
	ofn.nMaxFile = sizeof(fileName);
	ofn.Flags = OFN_NOCHANGEDIR | OFN_LONGNAMES | OFN_HIDEREADONLY;
	BOOL res = GetOpenFileName(&ofn); // doc: http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winui/winui/windowsuserinterface/userinput/commondialogboxlibrary/commondialogboxreference/commondialogboxstructures/openfilename.asp
	DWORD err = CommDlgExtendedError();

	RT_ASSERT( res == TRUE || err == 0, "Unable to GetOpenFileName." );

	if ( res == FALSE && err == 0 )
		*rval = JSVAL_VOID;
	else
		*rval = STRING_TO_JSVAL( JS_NewStringCopyZ(cx, fileName) );
	return JS_TRUE;
}


/**doc
 * string *ExpandEnvironmentStrings*( sourceString )
**/
DEFINE_FUNCTION( _ExpandEnvironmentStrings ) {

	RT_ASSERT_ARGC(1);
	const char *src;
	RT_JSVAL_TO_STRING( argv[0], src );
	TCHAR dst[MAX_PATH];
	DWORD res = ExpandEnvironmentStrings( src, dst, sizeof(dst) );
	RT_ASSERT( res != 0, "Unable to ExpandEnvironmentStrings." );
	*rval = STRING_TO_JSVAL( JS_NewStringCopyN(cx, dst, res) );
	return JS_TRUE;
}


/**doc
 * *Sleep*( time )
  TBD
**/
DEFINE_FUNCTION( _Sleep ) {

	RT_ASSERT_ARGC(1);
	uint32 timeout;
	RT_JSVAL_TO_INT32( argv[0], timeout );
	Sleep(timeout);
	return JS_TRUE;
}


/**doc
 * *MessageBeep*( value )
  Plays a waveform sound.
  The waveform sound for each sound type is identified by an entry in the registry.
**/
DEFINE_FUNCTION( _MessageBeep ) {

	UINT type = -1;
	if ( argc >= 1 )
		RT_JSVAL_TO_INT32( argv[0], type );
	MessageBeep(type);
	return JS_TRUE;
}


/**doc
 * *Beep*( freq, duration )
  TBD
**/
DEFINE_FUNCTION( _Beep ) {

	RT_ASSERT_ARGC(2);
	DWORD freq, duration;
	RT_JSVAL_TO_INT32( argv[0], freq );
	RT_JSVAL_TO_INT32( argv[1], duration );
	Beep(freq, duration);
	return JS_TRUE;
}

/**doc
=== Static properties ===
**/

/**doc
 * string *clipboard*
  TBD
**/
DEFINE_PROPERTY( clipboardGetter ) {

	BOOL res = OpenClipboard(NULL);
	RT_ASSERT( res != 0, "Unable to open the clipboard." );
	if ( IsClipboardFormatAvailable(CF_TEXT) == 0 ) {

		*vp = JSVAL_NULL;
	} else {

		HANDLE hglb = GetClipboardData(CF_TEXT);
		RT_ASSERT_RESOURCE( hglb );
		LPTSTR lptstr = (LPTSTR)GlobalLock(hglb);
		RT_ASSERT( lptstr != NULL, "Unable to lock memory." );
		JSString *str = JS_NewStringCopyZ(cx, lptstr);
		RT_ASSERT( str != NULL, "Unable to create the string.");
		*vp = STRING_TO_JSVAL(str);
		GlobalUnlock(hglb);
		CloseClipboard();
	}
	return JS_TRUE;
}

DEFINE_PROPERTY( clipboardSetter ) {

	BOOL res = OpenClipboard(NULL);
	RT_ASSERT( res != 0, "Unable to open the clipboard." );
	EmptyClipboard(); // doc: If the application specifies a NULL window handle when opening the clipboard, EmptyClipboard succeeds but sets the clipboard owner to NULL. Note that this causes SetClipboardData to fail.
	CloseClipboard();

	if ( *vp != JSVAL_VOID ) {

		res = OpenClipboard(NULL);
		RT_ASSERT( res != 0, "Unable to open the clipboard." );
		const char *str;
		size_t len;
		RT_JSVAL_TO_STRING_AND_LENGTH( *vp, str, len );
		HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, len + 1);
		RT_ASSERT_ALLOC( hglbCopy );
		LPTSTR lptstrCopy = (LPTSTR)GlobalLock(hglbCopy); 
		RT_ASSERT( lptstrCopy != NULL, "Unable to lock memory." );
		memcpy(lptstrCopy, str, len + 1);
		lptstrCopy[len] = 0;
		GlobalUnlock(hglbCopy);
		HANDLE h = SetClipboardData(CF_TEXT,hglbCopy);
		RT_ASSERT( h != NULL, "Unable to SetClipboardData." );
		CloseClipboard();
	}
	return JS_TRUE;
}

CONFIGURE_STATIC

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION2( MessageBox, _MessageBox )
		FUNCTION2( CreateProcess, _CreateProcess )
		FUNCTION2( ExtractIcon, _ExtractIcon )
		FUNCTION2( ExpandEnvironmentStrings, _ExpandEnvironmentStrings )
		FUNCTION( FileOpenDialog )
		FUNCTION2( Sleep, _Sleep )
		FUNCTION2( MessageBeep, _MessageBeep )
		FUNCTION2( Beep, _Beep )
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY( clipboard )
	END_STATIC_PROPERTY_SPEC

END_STATIC
