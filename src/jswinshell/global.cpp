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

/**doc fileIndex:topmost **/

BEGIN_STATIC


/**doc
=== Static functions ===
**/

/**doc
 * $TYPE Icon $INAME( fileName [, iconIndex ] )
  Retrieves an icon from the specified executable file, DLL, or icon file.
  $H beware
   This function is not supported for icons in 16-bit executables and DLLs.
**/
DEFINE_FUNCTION( ExtractIcon_ ) {

	J_S_ASSERT_ARG_MIN(1);
	const char *fileName;
	UINT iconIndex = 0;
	J_CHK( JsvalToString(cx, argv[0], &fileName) );
	if ( argc >= 2 )
		J_JSVAL_TO_INT32( argv[1], iconIndex );
	HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
	J_S_ASSERT( hInst != NULL, "Unable to GetModuleHandle." );
	HICON hIcon = ExtractIcon( hInst, fileName, iconIndex ); // see SHGetFileInfo(
	if ( hIcon == NULL )
		return WinThrowError(cx, GetLastError());
	JSObject *icon = JS_NewObject(cx, classIcon, NULL, NULL);
	HICON *phIcon = (HICON*)malloc(sizeof(HICON)); // this is needed because JS_SetPrivate stores ONLY alligned values
	J_S_ASSERT_ALLOC( phIcon );
	*phIcon = hIcon;
	JS_SetPrivate(cx, icon, phIcon);
	*rval = OBJECT_TO_JSVAL(icon);
	return JS_TRUE;
}

/**doc
 * $INT $INAME( content [, caption [, style ] ] )
  Displays a modal dialog box that contains a system icon, a set of buttons, and a brief application-specific message, such as status or error information.
  The message box returns an integer value that indicates which button the user clicked.
  $H arguments
   $ARG string content: the message to be displayed.
   $ARG string caption: the dialog box title.
   $ARG integer style: specifies the contents and behavior of the dialog box. This parameter can be a combination of flags from the following groups of flags:
    * MB_ABORTRETRYIGNORE
    * MB_CANCELTRYCONTINUE
    * MB_HELP
    * MB_OK
    * MB_OKCANCEL
    * MB_RETRYCANCEL
    * MB_YESNO
    * MB_YESNOCANCEL
    * MB_ICONEXCLAMATION
    * MB_ICONWARNING
    * MB_ICONINFORMATION
    * MB_ICONASTERISK
    * MB_ICONQUESTION
    * MB_ICONSTOP
    * MB_ICONERROR
    * MB_ICONHAND
    * MB_DEFBUTTON1
    * MB_DEFBUTTON2
    * MB_DEFBUTTON3
    * MB_DEFBUTTON4
    * MB_APPLMODAL
    * MB_SYSTEMMODAL
    * MB_TASKMODAL
    * MB_DEFAULT_DESKTOP_ONLY
    * MB_RIGHT
    * MB_RTLREADING
    * MB_SETFOREGROUND
    * MB_TOPMOST
  $H return value
   * IDABORT: Abort button was selected.
   * IDCANCEL: Cancel button was selected.
   * IDCONTINUE: Continue button was selected.
   * IDIGNORE: Ignore button was selected.
   * IDNO: No button was selected.
   * IDOK: OK button was selected.
   * IDRETRY: Retry button was selected.
   * IDTRYAGAIN: Try Again button was selected.
   * IDYES: Yes button was selected.
   * 32000: timeout ???
**/
DEFINE_FUNCTION( MessageBox_ ) {

	J_S_ASSERT_ARG_MIN(1);

	const char *text;
	J_CHK( JsvalToString(cx, argv[0], &text) );

	const char *caption = NULL;
	if ( argc >= 2 && argv[1] != JSVAL_VOID )
		J_CHK( JsvalToString(cx, argv[1], &caption) );

	UINT type = 0;
	if ( argc >= 3 )
		J_JSVAL_TO_INT32( argv[2], type );

	int res = MessageBox(NULL, text, caption, type);
	J_S_ASSERT( res != 0, "MessageBox call Failed." );
	*rval = INT_TO_JSVAL( res );
	return JS_TRUE;
}


/**doc
 * $VOID $INAME( applicationPath , [ commandLine ], [ environment ], [ currentDirectory ] )
  Creates a new process.
 $H arguments
  $ARG string applicationPath
  $ARG string commandLine
  $ARG string environment
  $ARG string currentDirectory
 $H example
 {{{
 CreateProcess( 'C:\\WINDOWS\\system32\\calc.exe', undefined, undefined, 'c:\\' );
 }}}
**/
DEFINE_FUNCTION( CreateProcess_ ) {

	J_S_ASSERT_ARG_MIN(1);

	const char *applicationName, *commandLine = NULL, *environment = NULL, *currentDirectory = NULL;

	J_CHK( JsvalToString(cx, argv[0], &applicationName) );

	if ( argc >= 2 && argv[1] != JSVAL_VOID )
		J_CHK( JsvalToString(cx, argv[1], &commandLine) );

	if ( argc >= 3 && argv[2] != JSVAL_VOID  )
		J_CHK( JsvalToString(cx, argv[2], &environment) );

	if ( argc >= 4 && argv[3] != JSVAL_VOID  )
		J_CHK( JsvalToString(cx, argv[3], &currentDirectory) );

	STARTUPINFO si = { sizeof(STARTUPINFO) };
	PROCESS_INFORMATION pi;
	BOOL st = CreateProcess( applicationName, (LPSTR)commandLine, NULL, NULL, FALSE, 0, (LPVOID)environment, currentDirectory, &si, &pi ); // doc: http://msdn2.microsoft.com/en-us/library/ms682425.aspx
	if ( st = FALSE )
		return WinThrowError(cx, GetLastError());
	return JS_TRUE;
}


/**doc
 * $STR | $TYPE undefined $INAME( [ filters ] [, defaultFileName ] )
  Creates an Open dialog box that lets the user specify the drive, directory, and the name of a file.
 $H example
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
		J_CHK( JsvalToStringAndLength(cx, argv[0], &str, &len) );
		strcpy( filter, str );
		for ( char *tmp = filter; tmp = strchr( tmp, '|' ); tmp++ )
			*tmp = '\0'; // doc: Pointer to a buffer containing pairs of null-terminated filter strings.
		filter[len+1] = '\0'; // The last string in the buffer must be terminated by two NULL characters.
		ofn.lpstrFilter = filter;
	}

	if ( argc >= 2 && argv[1] != JSVAL_VOID ) {

		const char *tmp;
		J_CHK( JsvalToString(cx, argv[1], &tmp) );
		strcpy( fileName, tmp );
	} else {
		*fileName = '\0';
	}

	ofn.lpstrFile = fileName;
	ofn.nMaxFile = sizeof(fileName);
	ofn.Flags = OFN_NOCHANGEDIR | OFN_LONGNAMES | OFN_HIDEREADONLY;
	BOOL res = GetOpenFileName(&ofn); // doc: http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winui/winui/windowsuserinterface/userinput/commondialogboxlibrary/commondialogboxreference/commondialogboxstructures/openfilename.asp
	DWORD err = CommDlgExtendedError();

	J_S_ASSERT( res == TRUE || err == 0, "Unable to GetOpenFileName." );

	if ( res == FALSE && err == 0 )
		*rval = JSVAL_VOID;
	else
		*rval = STRING_TO_JSVAL( JS_NewStringCopyZ(cx, fileName) );
	return JS_TRUE;
}


/**doc
 * $STR $INAME( sourceString )
  Expands environment-variable strings and replaces them with the values defined for the current user.
**/
DEFINE_FUNCTION( ExpandEnvironmentStrings_ ) {

	J_S_ASSERT_ARG_MIN(1);
	const char *src;
	J_CHK( JsvalToString(cx, argv[0], &src) );
	TCHAR dst[MAX_PATH];
	DWORD res = ExpandEnvironmentStrings( src, dst, sizeof(dst) );
	J_S_ASSERT( res != 0, "Unable to ExpandEnvironmentStrings." );
	*rval = STRING_TO_JSVAL( JS_NewStringCopyN(cx, dst, res) );
	return JS_TRUE;
}


/**doc
 * $VOID $INAME( milliseconds )
  Suspends the execution of the current process until the time-out interval elapses.
**/
DEFINE_FUNCTION( Sleep_ ) {

	J_S_ASSERT_ARG_MIN(1);
	uint32 timeout;
	J_JSVAL_TO_INT32( argv[0], timeout );
	Sleep(timeout);
	return JS_TRUE;
}


/**doc
 * $VOID $INAME( type )
  Plays a waveform sound. The waveform sound for each sound type is identified by an entry in the registry.
  $H arguments
   $ARG integer type:
    * -1 : Simple beep. If the sound card is not available, the sound is generated using the speaker.
    * 0 : MB_OK SystemDefault
    * ICONASTERISK: SystemAsterisk
    * ICONEXCLAMATION: SystemExclamation
    * ICONHAND: SystemHand
    * ICONQUESTION: SystemQuestion
**/
DEFINE_FUNCTION( MessageBeep_ ) {

	UINT type = -1;
	if ( argc >= 1 )
		J_JSVAL_TO_INT32( argv[0], type );
	MessageBeep(type);
	return JS_TRUE;
}


/**doc
 * $VOID $INAME( hertzFrequency, millisecondsDuration )
  Generates simple tones on the speaker.
  $H note
   The function is synchronous, it does not return control to its caller until the sound finishes.
**/
DEFINE_FUNCTION( Beep_ ) {

	J_S_ASSERT_ARG_MIN(2);
	DWORD freq, duration;
	J_JSVAL_TO_INT32( argv[0], freq );
	J_JSVAL_TO_INT32( argv[1], duration );
	Beep(freq, duration);
	return JS_TRUE;
}

/**doc
=== Static properties ===
**/

/**doc
 * $STR $INAME
  Places or retrieves text data from the clipboard.
**/
DEFINE_PROPERTY( clipboardGetter ) {

	BOOL res = OpenClipboard(NULL);
	J_S_ASSERT( res != 0, "Unable to open the clipboard." );
	if ( IsClipboardFormatAvailable(CF_TEXT) == 0 ) {

		*vp = JSVAL_NULL;
	} else {

		HANDLE hglb = GetClipboardData(CF_TEXT);
		J_S_ASSERT_RESOURCE( hglb );
		LPTSTR lptstr = (LPTSTR)GlobalLock(hglb);
		J_S_ASSERT( lptstr != NULL, "Unable to lock memory." );
		JSString *str = JS_NewStringCopyZ(cx, lptstr);
		J_S_ASSERT( str != NULL, "Unable to create the string.");
		*vp = STRING_TO_JSVAL(str);
		GlobalUnlock(hglb);
		CloseClipboard();
	}
	return JS_TRUE;
}

DEFINE_PROPERTY( clipboardSetter ) {

	BOOL res = OpenClipboard(NULL);
	J_S_ASSERT( res != 0, "Unable to open the clipboard." );
	EmptyClipboard(); // doc: If the application specifies a NULL window handle when opening the clipboard, EmptyClipboard succeeds but sets the clipboard owner to NULL. Note that this causes SetClipboardData to fail.
	CloseClipboard();

	if ( *vp != JSVAL_VOID ) {

		res = OpenClipboard(NULL);
		J_S_ASSERT( res != 0, "Unable to open the clipboard." );
		const char *str;
		size_t len;
		J_CHK( JsvalToStringAndLength(cx, *vp, &str, &len) );
		HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, len + 1);
		J_S_ASSERT_ALLOC( hglbCopy );
		LPTSTR lptstrCopy = (LPTSTR)GlobalLock(hglbCopy);
		J_S_ASSERT( lptstrCopy != NULL, "Unable to lock memory." );
		memcpy(lptstrCopy, str, len + 1);
		lptstrCopy[len] = 0;
		GlobalUnlock(hglbCopy);
		HANDLE h = SetClipboardData(CF_TEXT,hglbCopy);
		J_S_ASSERT( h != NULL, "Unable to SetClipboardData." );
		CloseClipboard();
	}
	return JS_TRUE;
}

CONFIGURE_STATIC

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION2( MessageBox, MessageBox_ )
		FUNCTION2( CreateProcess, CreateProcess_ )
		FUNCTION2( ExtractIcon, ExtractIcon_ )
		FUNCTION2( ExpandEnvironmentStrings, ExpandEnvironmentStrings_ )
		FUNCTION( FileOpenDialog )
		FUNCTION2( Sleep, Sleep_ )
		FUNCTION2( MessageBeep, MessageBeep_ )
		FUNCTION2( Beep, Beep_ )
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY( clipboard )
	END_STATIC_PROPERTY_SPEC

	BEGIN_CONST_INTEGER_SPEC
		CONST_INTEGER(MB_ABORTRETRYIGNORE				,MB_ABORTRETRYIGNORE           )
		CONST_INTEGER(MB_CANCELTRYCONTINUE				,MB_CANCELTRYCONTINUE			 )
		CONST_INTEGER(MB_HELP								,MB_HELP								 )
		CONST_INTEGER(MB_OK									,MB_OK								 )
		CONST_INTEGER(MB_OKCANCEL							,MB_OKCANCEL						 )
		CONST_INTEGER(MB_RETRYCANCEL						,MB_RETRYCANCEL					 )
		CONST_INTEGER(MB_YESNO								,MB_YESNO							 )
		CONST_INTEGER(MB_YESNOCANCEL						,MB_YESNOCANCEL					 )
		CONST_INTEGER(MB_ICONEXCLAMATION					,MB_ICONEXCLAMATION				 )
		CONST_INTEGER(MB_ICONWARNING						,MB_ICONWARNING					 )
		CONST_INTEGER(MB_ICONINFORMATION					,MB_ICONINFORMATION				 )
		CONST_INTEGER(MB_ICONASTERISK						,MB_ICONASTERISK					 )
		CONST_INTEGER(MB_ICONQUESTION						,MB_ICONQUESTION					 )
		CONST_INTEGER(MB_ICONSTOP							,MB_ICONSTOP						 )
		CONST_INTEGER(MB_ICONERROR							,MB_ICONERROR						 )
		CONST_INTEGER(MB_ICONHAND							,MB_ICONHAND						 )
		CONST_INTEGER(MB_DEFBUTTON1						,MB_DEFBUTTON1						 )
		CONST_INTEGER(MB_DEFBUTTON2						,MB_DEFBUTTON2						 )
		CONST_INTEGER(MB_DEFBUTTON3						,MB_DEFBUTTON3						 )
		CONST_INTEGER(MB_DEFBUTTON4						,MB_DEFBUTTON4						 )
		CONST_INTEGER(MB_APPLMODAL							,MB_APPLMODAL						 )
		CONST_INTEGER(MB_SYSTEMMODAL						,MB_SYSTEMMODAL					 )
		CONST_INTEGER(MB_TASKMODAL							,MB_TASKMODAL						 )
		CONST_INTEGER(MB_DEFAULT_DESKTOP_ONLY			,MB_DEFAULT_DESKTOP_ONLY		 )
		CONST_INTEGER(MB_RIGHT								,MB_RIGHT							 )
		CONST_INTEGER(MB_RTLREADING						,MB_RTLREADING						 )
		CONST_INTEGER(MB_SETFOREGROUND					,MB_SETFOREGROUND					 )
		CONST_INTEGER(MB_TOPMOST							,MB_TOPMOST							 )
//		CONST_INTEGER(MB_SERVICE_NOTIFICATION			,MB_SERVICE_NOTIFICATION		 )
//		CONST_INTEGER(MB_SERVICE_NOTIFICATION_NT3X	,MB_SERVICE_NOTIFICATION_NT3X	 )
		
		CONST_INTEGER(IDABORT     ,IDABORT    )
		CONST_INTEGER(IDCANCEL	  ,IDCANCEL	  )
		CONST_INTEGER(IDCONTINUE  ,IDCONTINUE )
		CONST_INTEGER(IDIGNORE	  ,IDIGNORE	  )
		CONST_INTEGER(IDNO		  ,IDNO		  )
		CONST_INTEGER(IDOK		  ,IDOK		  )
		CONST_INTEGER(IDRETRY	  ,IDRETRY	  )
		CONST_INTEGER(IDTRYAGAIN  ,IDTRYAGAIN )
		CONST_INTEGER(IDYES		  ,IDYES		  )
	END_CONST_INTEGER_SPEC

END_STATIC
