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

#include "../jslang/handlePub.h"

#include "icon.h"
#include <stdlib.h>

#include <Commdlg.h>
#include <shlobj.h>

/**doc fileIndex:topmost **/

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_STATIC


/**doc
=== Static functions ===
**/

/**doc
$TOC_MEMBER $INAME
 $TYPE Icon $INAME( fileName [, iconIndex ] )
  Retrieves an icon from the specified executable file, DLL, or icon file.$LF
  The function returns $UNDEF if no icon is found.
  $H beware
   This function is not supported for icons in 16-bit executables and DLLs.
**/
DEFINE_FUNCTION( ExtractIcon ) {

	JL_S_ASSERT_ARG_MIN(1);
	UINT iconIndex = 0;
	if ( argc >= 2 )
		JL_CHK( JsvalToUInt(cx, argv[1], &iconIndex) );
	HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
	JL_S_ASSERT( hInst != NULL, "Unable to GetModuleHandle." );
	const char *fileName;
	JL_CHK( JsvalToString(cx, &argv[0], &fileName) );
	HICON hIcon = ExtractIcon( hInst, fileName, iconIndex ); // see SHGetFileInfo(
	if ( hIcon == NULL ) {

//		if ( GetLastError() != 0 )
//			return WinThrowError(cx, GetLastError());
		*rval = JSVAL_VOID;
		return JS_TRUE;
	}
	JSObject *icon = JS_NewObject(cx, classIcon, NULL, NULL);
	HICON *phIcon = (HICON*)jl_malloc(sizeof(HICON)); // this is needed because JL_SetPrivate stores ONLY alligned values
	JL_S_ASSERT_ALLOC( phIcon );
	*phIcon = hIcon;
	JL_SetPrivate(cx, icon, phIcon);
	*rval = OBJECT_TO_JSVAL(icon);
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME( content [, caption [, style ] ] )
  Displays a modal dialog box that contains a system icon, a set of buttons, and a brief application-specific message, such as status or error information.
  The message box returns an integer value that indicates which button the user clicked.
  $H arguments
   $ARG $STR content: the message to be displayed.
   $ARG $STR caption: the dialog box title.
   $ARG $INT style: specifies the contents and behavior of the dialog box. This parameter can be a combination of flags from the following groups of flags:
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
DEFINE_FUNCTION( MessageBox ) {

	JL_S_ASSERT_ARG_MIN(1);

	UINT type = 0;
	if ( argc >= 3 )
		JL_CHK( JsvalToUInt(cx, argv[2], &type) );

	const char *caption = NULL;
	if ( argc >= 2 && !JSVAL_IS_VOID( argv[1] ) )
		JL_CHK( JsvalToString(cx, &argv[1], &caption) );

	const char *text;
	JL_CHK( JsvalToString(cx, &argv[0], &text) );

	int res = MessageBox(NULL, text, caption, type);
	JL_S_ASSERT( res != 0, "MessageBox call Failed." );
	*rval = INT_TO_JSVAL( res );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( applicationPath , [ commandLine ], [ environment ], [ currentDirectory ] )
  Creates a new process.
 $H arguments
  $ARG $STR applicationPath
  $ARG $STR commandLine
  $ARG $STR environment
  $ARG $STR currentDirectory
 $H example
 {{{
 CreateProcess( 'C:\\WINDOWS\\system32\\calc.exe', undefined, undefined, 'c:\\' );
 }}}
**/
DEFINE_FUNCTION( CreateProcess ) {

	JL_S_ASSERT_ARG_MIN(1);

	const char *applicationName, *commandLine, *environment, *currentDirectory;

	if ( JL_ARG_ISDEF(1) )
		JL_CHK( JsvalToString(cx, &argv[0], &applicationName) ); // warning: GC on the returned buffer !
	else
		applicationName = NULL;

	if ( JL_ARG_ISDEF(2) )
		JL_CHK( JsvalToString(cx, &argv[1], &commandLine) ); // warning: GC on the returned buffer !
	else
		commandLine = NULL;

	if ( JL_ARG_ISDEF(3) )
		JL_CHK( JsvalToString(cx, &argv[2], &environment) ); // warning: GC on the returned buffer !
	else
		environment = NULL;

	if ( JL_ARG_ISDEF(4) )
		JL_CHK( JsvalToString(cx, &argv[3], &currentDirectory) ); // warning: GC on the returned buffer !
	else
		currentDirectory = NULL;

	STARTUPINFO si;
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);


	// doc. commandLine parameter: The Unicode version of CreateProcess function, CreateProcessW, can modify the contents of this string !
	PROCESS_INFORMATION pi;
	BOOL st = CreateProcess( applicationName, (LPSTR)commandLine, NULL, NULL, FALSE, 0, (LPVOID)environment, currentDirectory, &si, &pi ); // doc: http://msdn2.microsoft.com/en-us/library/ms682425.aspx
	if ( st == FALSE )
		return WinThrowError(cx, GetLastError());

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR | $UNDEF $INAME( [ filters ] [, defaultFileName ] )
  Creates an Open dialog box that lets the user specify the drive, directory, and the name of a file. The function returns $UNDEF if the dialog is canceled.
 $H example
 {{{
 FileOpenDialog( 'executable files|*.exe;*.com;*.cmd;*.bat|all files|*.*' );
 }}}
**/
DEFINE_FUNCTION( FileOpenDialog ) {

	OPENFILENAME ofn = { sizeof(OPENFILENAME) };
	char fileName[MAX_PATH];
	char filter[255];

	if ( argc >= 1 && !JSVAL_IS_VOID( argv[0] ) ) {

		const char *str;
		size_t len;
		JL_CHK( JsvalToStringAndLength(cx, &argv[0], &str, &len) );
		strcpy( filter, str );
		for ( char *tmp = filter; tmp = strchr( tmp, '|' ); tmp++ )
			*tmp = '\0'; // doc: Pointer to a buffer containing pairs of null-terminated filter strings.
		filter[len+1] = '\0'; // The last string in the buffer must be terminated by two NULL characters.
		ofn.lpstrFilter = filter;
	}

	if ( argc >= 2 && !JSVAL_IS_VOID( argv[1] ) ) {

		const char *tmp;
		JL_CHK( JsvalToString(cx, &argv[1], &tmp) );
		strcpy( fileName, tmp );
	} else {
		*fileName = '\0';
	}

	ofn.lpstrFile = fileName;
	ofn.nMaxFile = sizeof(fileName);
	ofn.Flags = OFN_NOCHANGEDIR | OFN_LONGNAMES | OFN_HIDEREADONLY;
	BOOL res = GetOpenFileName(&ofn); // doc: http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winui/winui/windowsuserinterface/userinput/commondialogboxlibrary/commondialogboxreference/commondialogboxstructures/openfilename.asp
	DWORD err = CommDlgExtendedError();

	JL_S_ASSERT( res == TRUE || err == 0, "Unable to GetOpenFileName." );

	if ( res == FALSE && err == 0 )
		*rval = JSVAL_VOID;
	else
		*rval = STRING_TO_JSVAL( JS_NewStringCopyZ(cx, fileName) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( sourceString )
  Expands environment-variable strings and replaces them with the values defined for the current user.
**/
DEFINE_FUNCTION( ExpandEnvironmentStrings ) {

	JL_S_ASSERT_ARG_MIN(1);
	const char *src;
	JL_CHK( JsvalToString(cx, &argv[0], &src) );
	TCHAR dst[MAX_PATH];
	DWORD res = ExpandEnvironmentStrings( src, dst, sizeof(dst) );
	JL_S_ASSERT( res != 0, "Unable to ExpandEnvironmentStrings." );
	*rval = STRING_TO_JSVAL( JS_NewStringCopyN(cx, dst, res) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( milliseconds )
  Suspends the execution of the current process until the time-out interval elapses.
**/
DEFINE_FUNCTION( Sleep ) {

	JL_S_ASSERT_ARG_MIN(1);
	unsigned int timeout;
	JL_CHK( JsvalToUInt(cx, argv[0], &timeout) );
	Sleep(timeout);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( type )
  Plays a waveform sound. The waveform sound for each sound type is identified by an entry in the registry.
  $H arguments
   $ARG $INT type:
    * -1 : Simple beep. If the sound card is not available, the sound is generated using the speaker.
    * 0 : MB_OK SystemDefault
    * ICONASTERISK: SystemAsterisk
    * ICONEXCLAMATION: SystemExclamation
    * ICONHAND: SystemHand
    * ICONQUESTION: SystemQuestion
**/
DEFINE_FUNCTION( MessageBeep ) {

	UINT type = -1;
	if ( argc >= 1 )
		JL_CHK( JsvalToUInt(cx, argv[0], &type) );
	MessageBeep(type);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( hertzFrequency, millisecondsDuration )
  Generates simple tones on the speaker.
  $H note
   The function is synchronous, it does not return control to its caller until the sound finishes.
**/
DEFINE_FUNCTION( Beep ) {

	JL_S_ASSERT_ARG_MIN(2);
	unsigned int freq, duration;
	JL_CHK( JsvalToUInt(cx, argv[0], &freq) );
	JL_CHK( JsvalToUInt(cx, argv[1], &duration) );
	Beep(freq, duration);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( path [, valueName] )
  Query a value or a list of valueName from the system registry.
  $BR
  If valueName is ommited, the returned value is the list of sub-keys.
  $BR
  If valueName is given but is $UNDEF, the returned value is the list of available values.
  $H example 1
  {{{
  RegistryGet('HKEY_CURRENT_USER\\Software\\7-Zip'); // returns ["FM"]
  RegistryGet('HKEY_CURRENT_USER\\Software\\7-Zip', undefined); // returns ["Path", "Lang"]
  RegistryGet('HKEY_CURRENT_USER\\Software\\7-Zip', 'path') // returns "C:\\Program Files\\7-Zip"
  }}}
  $H example 2
  {{{
  var path = 'HKEY_LOCAL_MACHINE\\Software\\Clients\\StartMenuInternet';
  var defaultBrowser = RegistryGet(path+'\\'+RegistryGet(path, '')+'\\shell\\open\\command', '');
  CreateProcess(undefined, defaultBrowser + ' http://jslibs.googlecode.com/');
  }}}
**/
DEFINE_FUNCTION_FAST( RegistryGet ) {

	JL_S_ASSERT_ARG_RANGE(1,2);
	
	const char *path, *valueName;
	JL_CHK( JsvalToString(cx, &JL_FARG(1), &path) );

	HKEY rootHKey;
	if ( !strncmp(path, "HKEY_CURRENT_USER", 17) ) {
		rootHKey = HKEY_CURRENT_USER;
		path += 17;
	} else
	if ( !strncmp(path, "HKCU", 4) ) {
		rootHKey = HKEY_CURRENT_USER;
		path += 4;
	} else
	if ( !strncmp(path, "HKEY_LOCAL_MACHINE", 18) ) {
		rootHKey = HKEY_LOCAL_MACHINE;
		path += 18;
	} else
	if ( !strncmp(path, "HKLM", 4) ) {
		rootHKey = HKEY_LOCAL_MACHINE;
		path += 4;
	} else
	if ( !strncmp(path, "HKEY_CLASSES_ROOT", 17) ) {
		rootHKey = HKEY_CLASSES_ROOT;
		path += 17;
	} else
	if ( !strncmp(path, "HKCR", 4) ) {
		rootHKey = HKEY_CLASSES_ROOT;
		path += 4;
	} else
	if ( !strncmp(path, "HKEY_CURRENT_CONFIG", 19) ) {
		rootHKey = HKEY_CURRENT_CONFIG;
		path += 19;
	} else
	if ( !strncmp(path, "HKCC", 4) ) {
		rootHKey = HKEY_CURRENT_CONFIG;
		path += 4;
	} else
	if ( !strncmp(path, "HKEY_USERS", 10) ) {
		rootHKey = HKEY_USERS;
		path += 10;
	} else
	if ( !strncmp(path, "HKU", 3) ) {
		rootHKey = HKEY_USERS;
		path += 3;
	} else
	if ( !strncmp(path, "HKEY_PERFORMANCE_DATA", 21) ) {
		rootHKey = HKEY_PERFORMANCE_DATA;
		path += 21;
	} else
	if ( !strncmp(path, "HKPD", 4) ) {
		rootHKey = HKEY_PERFORMANCE_DATA;
		path += 4;
	} else
	if ( !strncmp(path, "HKEY_DYN_DATA", 13) ) {
		rootHKey = HKEY_DYN_DATA;
		path += 13;
	} else
	if ( !strncmp(path, "HKDD", 4) ) {
		rootHKey = HKEY_DYN_DATA;
		path += 4;
	}

	if ( path[0] == '\\' )
		path++;

	HKEY hKey;
	LONG error;
	error = RegOpenKeyEx(rootHKey, path, 0, KEY_READ, &hKey); // http://msdn.microsoft.com/en-us/library/ms724897%28VS.85%29.aspx
	if ( error != ERROR_SUCCESS )
		return WinThrowError(cx, error);

	if ( (argc == 1) || (argc >= 2 && JL_FARG(2) == JSVAL_VOID) ) {

		JSObject *arrObj = JS_NewArrayObject(cx, 0, NULL);
		JL_CHK( arrObj );
		*JL_FRVAL = OBJECT_TO_JSVAL(arrObj);

		char name[16384]; // http://msdn.microsoft.com/en-us/library/ms724872%28VS.85%29.aspx
		DWORD nameLength, index;
		index = 0;
		for (;;) {

			nameLength = sizeof(name);
			if ( argc == 1 ) // enum keys
				error = RegEnumKeyEx(hKey, index, name, &nameLength, NULL, NULL, NULL, NULL);
			else
				error = RegEnumValue(hKey, index, name, &nameLength, NULL, NULL, NULL, NULL); // doc. http://msdn.microsoft.com/en-us/library/ms724865(VS.85).aspx

			if ( error != ERROR_SUCCESS )
				break;
			jsval strName;
			JL_CHK( StringAndLengthToJsval(cx, &strName, name, nameLength) );
			JL_CHK( JS_SetElement(cx, arrObj, index, &strName) );
			index++;
		}
		if ( error != ERROR_NO_MORE_ITEMS )
			return WinThrowError(cx, error);

		RegCloseKey(hKey);
		return JS_TRUE;
	}

	JL_CHK( JsvalToString(cx, &JL_FARG(2), &valueName) );

	DWORD type, size;

	// doc. http://msdn.microsoft.com/en-us/library/ms724911(VS.85).aspx
	error = RegQueryValueEx(hKey, valueName, NULL, &type, NULL, &size);
	if ( error != ERROR_SUCCESS ) {

		RegCloseKey(hKey);
		return WinThrowError(cx, error);
	}

	void *buffer = JS_malloc(cx, size);
	error = RegQueryValueEx(hKey, valueName, NULL, NULL, (LPBYTE)buffer, &size);

	// doc. http://msdn.microsoft.com/en-us/library/ms724884(VS.85).aspx
	switch (type) {
		case REG_NONE:
			*JL_FRVAL = JSVAL_VOID;
			JS_free(cx, buffer);
			break;
		case REG_BINARY:
			JL_CHK( JL_NewBlob(cx, buffer, size, JL_FRVAL) );
			break;
		case REG_DWORD:
			JL_CHK( UIntToJsval(cx, *(DWORD*)buffer, JL_FRVAL) );
			JS_free(cx, buffer);
			break;
		case REG_QWORD:
			JL_CHK( DoubleToJsval(cx, (double)*(DWORD64*)buffer, JL_FRVAL) );
			break;
		case REG_LINK: {
			JSString *jsstr = JS_NewUCString(cx, (jschar*)buffer, size/2);
			JL_CHK( jsstr );
			*JL_FRVAL = STRING_TO_JSVAL(jsstr);
			break;
		}
		case REG_EXPAND_SZ:
		case REG_MULTI_SZ:
		case REG_SZ: {
			JSString *jsstr = JS_NewString(cx, (char*)buffer, size-1); // note: ((char*)buffer)[size] already == '\0'
			JL_CHK( jsstr );
			*JL_FRVAL = STRING_TO_JSVAL(jsstr);
			break;
		}
	}

	RegCloseKey(hKey);

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( pathName, notifyFilter [ , watchSubtree = false ] )
  Creates a change notification handle and sets up initial change notification filter conditions.$LF
  _notifyFilter_ can be a combination of the following flags:
  * 0x01: Any file name change in the watched directory or subtree. Changes include renaming, creating, or deleting a file.
  * 0x02: Any directory-name change in the watched directory or subtree. Changes include creating or deleting a directory.
  * 0x04: Any attribute change in the watched directory or subtree.
  * 0x08: Any file-size change in the watched directory or subtree. The operating system detects a change in file size only when the file is written to the disk. For operating systems that use extensive caching, detection occurs only when the cache is sufficiently flushed.
  * 0x10: Any change to the last write-time of files in the watched directory or subtree. The operating system detects a change to the last write-time only when the file is written to the disk. For operating systems that use extensive caching, detection occurs only when the cache is sufficiently flushed.
  * 0x20: Any change to the last access time of files in the watched directory or subtree.
  * 0x40: Any change to the creation time of files in the watched directory or subtree.
  * 0x100: Any security-descriptor change in the watched directory or subtree.
  $H example 1
  {{{
  var dch = DirectoryChangesInit('C:\\WINDOWS', 0x10|0x40, true);
  while (!endSignal) {
    Print( uneval( DirectoryChangesLookup(dch) ), '\n');
    Sleep(1000);
  }
  }}}
**/
// (TBD) Linux version using inotify: http://en.wikipedia.org/wiki/Inotify / try: man inotify
struct DirectoryChanges {
	HANDLE hDirectory;
	OVERLAPPED overlapped;
	BYTE buffer[2][512*sizeof(FILE_NOTIFY_INFORMATION)];
	int currentBuffer;
	BOOL watchSubtree;
	DWORD	notifyFilter;
};

void FinalizeDirectoryHandle(void *data) {
	
	DirectoryChanges *dc = (DirectoryChanges*)data;
	CloseHandle(dc->overlapped.hEvent);
	dc->overlapped.hEvent = NULL;
	CloseHandle(dc->hDirectory);
}

DEFINE_FUNCTION_FAST( DirectoryChangesInit ) {

	JL_S_ASSERT_ARG_RANGE(2,3);

	const char *pathName;
	JL_CHK( JsvalToString(cx, &JL_FARG(1), &pathName) );

	unsigned int notifyFilter;
	JL_CHK( JsvalToUInt(cx, JL_FARG(2), &notifyFilter) );

	bool watchSubtree;
	if ( JL_FARG_ISDEF(3) )
		JL_CHK( JsvalToBool(cx, JL_FARG(3), &watchSubtree) );
	else
		watchSubtree = false;

	DirectoryChanges *dc;
	JL_CHK( CreateHandle(cx, 'dmon', sizeof(DirectoryChanges), (void**)&dc, FinalizeDirectoryHandle, JL_FRVAL) );

	dc->watchSubtree = watchSubtree;
	dc->notifyFilter = notifyFilter;

	dc->hDirectory = CreateFile(pathName, FILE_LIST_DIRECTORY, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);
	if ( dc->hDirectory == INVALID_HANDLE_VALUE )
		return WinThrowError(cx, GetLastError());

//	memset(&dc->overlapped, 0, sizeof(dc->overlapped));
//   memset(&dc->buffer, 0, sizeof(dc->buffer));
	dc->currentBuffer = 0;
	dc->overlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	if ( !ReadDirectoryChangesW(dc->hDirectory, &dc->buffer[dc->currentBuffer], sizeof(*dc->buffer), dc->watchSubtree, dc->notifyFilter, NULL, &dc->overlapped, NULL) )
		return WinThrowError(cx, GetLastError());

	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $ARR | $VOID $INAME( changeNotificationHandle )
  Returns the list of changed files based on the filter provided to the DirectoryChangesInit() function.
  Each element of the list is a 2-element array that contain the filename and the action.$LF
  actions:
  * 1: The file was added to the directory.
  * 2: The file was removed from the directory.
  * 3: The file was modified. This can be a change in the time stamp or attributes.
  * 4: The file was renamed and this is the old name.
  * 5: The file was renamed and this is the new name.
**/
DEFINE_FUNCTION_FAST( DirectoryChangesLookup ) {

	JL_S_ASSERT_ARG_RANGE(1,1);
	JL_S_ASSERT( IsHandleType(cx, JL_FARG(1), 'dmon'), "Unexpected argument type." );
	DirectoryChanges *dc = (DirectoryChanges*)GetHandlePrivate(cx, JL_FARG(1));

	DWORD res = WaitForSingleObject(dc->overlapped.hEvent, 0);
	if ( res == -1 )
		return WinThrowError(cx,  GetLastError());

	if ( res != WAIT_OBJECT_0 ) {
		
		*JL_FRVAL = JSVAL_VOID;
		return JS_TRUE;
	}

	DWORD dwNumberbytes;
	if ( !GetOverlappedResult(dc->hDirectory, &dc->overlapped, &dwNumberbytes, FALSE) )
		return WinThrowError(cx, GetLastError());

	dc->currentBuffer = 1 - dc->currentBuffer;

	if ( !ReadDirectoryChangesW(dc->hDirectory, &dc->buffer[dc->currentBuffer], sizeof(*dc->buffer), dc->watchSubtree, dc->notifyFilter, NULL, &dc->overlapped, NULL) )
		return WinThrowError(cx, GetLastError());

	FILE_NOTIFY_INFORMATION *pFileNotify;
	pFileNotify = (PFILE_NOTIFY_INFORMATION)&dc->buffer[1-dc->currentBuffer];

	JSObject *arrObj = JS_NewArrayObject(cx, 0, NULL);
	JL_CHK( arrObj );
	*JL_FRVAL = OBJECT_TO_JSVAL( arrObj );

	jsval tmp;
	jsval eltContent[2];
	jsint index = 0;
	// see http://www.google.fr/codesearch/p?hl=fr&sa=N&cd=17&ct=rc#8WOCRDPt-u8/trunk/src/FileWatch.cc&q=ReadDirectoryChangesW
	while ( pFileNotify ) {

		JSString *str = JS_NewUCStringCopyN(cx, (jschar*)pFileNotify->FileName, pFileNotify->FileNameLength / sizeof(WCHAR));
		JL_CHK( str );
		eltContent[0] = STRING_TO_JSVAL( str );
		eltContent[1] = INT_TO_JSVAL( pFileNotify->Action );
		JSObject *elt = JS_NewArrayObject(cx, COUNTOF(eltContent), eltContent);
		JL_CHK( elt );
		tmp = OBJECT_TO_JSVAL( elt );
		JL_CHK( JS_SetElement(cx, arrObj, index, &tmp) );
		index++;

		if ( pFileNotify->NextEntryOffset )
			pFileNotify = (FILE_NOTIFY_INFORMATION*) ((PBYTE)pFileNotify + pFileNotify->NextEntryOffset) ;
		else
			pFileNotify = NULL;
	}

	return JS_TRUE;
	JL_BAD;
}





/**doc
=== Static properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $STR $INAME
  Places or retrieves text data from the clipboard.
**/
DEFINE_PROPERTY( clipboardGetter ) {

	BOOL res = OpenClipboard(NULL);
	JL_S_ASSERT( res != 0, "Unable to open the clipboard." );
	if ( IsClipboardFormatAvailable(CF_TEXT) == 0 ) {

		*vp = JSVAL_NULL;
	} else {

		HANDLE hglb = GetClipboardData(CF_TEXT);
		JL_S_ASSERT_RESOURCE( hglb );
		LPTSTR lptstr = (LPTSTR)GlobalLock(hglb);
		JL_S_ASSERT( lptstr != NULL, "Unable to lock memory." );
		JSString *str = JS_NewStringCopyZ(cx, lptstr);
		JL_S_ASSERT( str != NULL, "Unable to create the string.");
		*vp = STRING_TO_JSVAL(str);
		GlobalUnlock(hglb);
		CloseClipboard();
	}
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( clipboardSetter ) {

	BOOL res = OpenClipboard(NULL);
	JL_S_ASSERT( res != 0, "Unable to open the clipboard." );
	EmptyClipboard(); // doc: If the application specifies a NULL window handle when opening the clipboard, EmptyClipboard succeeds but sets the clipboard owner to NULL. Note that this causes SetClipboardData to fail.
	CloseClipboard();

	if ( !JSVAL_IS_VOID( *vp ) ) {

		res = OpenClipboard(NULL);
		JL_S_ASSERT( res != 0, "Unable to open the clipboard." );
		const char *str;
		size_t len;
		JL_CHK( JsvalToStringAndLength(cx, vp, &str, &len) );
		HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, len + 1);
		JL_S_ASSERT_ALLOC( hglbCopy );
		LPTSTR lptstrCopy = (LPTSTR)GlobalLock(hglbCopy);
		JL_S_ASSERT( lptstrCopy != NULL, "Unable to lock memory." );
		memcpy(lptstrCopy, str, len + 1);
		lptstrCopy[len] = 0;
		GlobalUnlock(hglbCopy);
		HANDLE h = SetClipboardData(CF_TEXT,hglbCopy);
		JL_S_ASSERT( h != NULL, "Unable to SetClipboardData." );
		CloseClipboard();
	}
	return JS_TRUE;
	JL_BAD;
}

CONFIGURE_STATIC

	REVISION(JL_SvnRevToInt("$Revision$"))
	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION( MessageBox )
		FUNCTION( CreateProcess )
		FUNCTION( ExtractIcon )
		FUNCTION( ExpandEnvironmentStrings )
		FUNCTION( FileOpenDialog )
		FUNCTION( Sleep )
		FUNCTION( MessageBeep )
		FUNCTION( Beep )
		FUNCTION_FAST( RegistryGet )

		FUNCTION_FAST( DirectoryChangesInit )
		FUNCTION_FAST( DirectoryChangesLookup )
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
