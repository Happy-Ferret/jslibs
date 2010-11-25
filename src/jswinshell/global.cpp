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

#include "com.h"


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

	JLStr fileName;
	JL_S_ASSERT_ARG_MIN(1);
	UINT iconIndex = 0;
	if ( argc >= 2 )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &iconIndex) );
	HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
	JL_S_ASSERT( hInst != NULL, "Unable to GetModuleHandle." );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &fileName) );
	HICON hIcon = ExtractIcon( hInst, fileName, iconIndex ); // see SHGetFileInfo(
	if ( hIcon == NULL ) {

//		if ( GetLastError() != 0 )
//			return WinThrowError(cx, GetLastError());
		*JL_RVAL = JSVAL_VOID;
		return JS_TRUE;
	}
	JSObject *icon = JS_NewObjectWithGivenProto(cx, JL_CLASS(Icon), JL_PROTOTYPE(cx, Icon), NULL);
	HICON *phIcon = (HICON*)jl_malloc(sizeof(HICON)); // this is needed because JL_SetPrivate stores ONLY alligned values
	JL_S_ASSERT_ALLOC( phIcon );
	*phIcon = hIcon;
	JL_SetPrivate(cx, icon, phIcon);
	*JL_RVAL = OBJECT_TO_JSVAL(icon);
	
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

	JLStr caption, text;

	JL_S_ASSERT_ARG_MIN(1);

	UINT type = 0;
	if ( argc >= 3 )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &type) );

	if ( argc >= 2 && !JSVAL_IS_VOID( JL_ARG(2) ) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &caption) );

	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &text) );

	int res = MessageBox(NULL, text.GetConstStr(), caption.GetStrConstOrNull(), type);
	JL_S_ASSERT( res != 0, "MessageBox call Failed." );
	*JL_RVAL = INT_TO_JSVAL( res );
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

	JLStr applicationName, commandLine, environment, currentDirectory;

	JL_S_ASSERT_ARG_MIN(1);

	if ( JL_ARG_ISDEF(1) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &applicationName) ); // warning: GC on the returned buffer !

	if ( JL_ARG_ISDEF(2) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &commandLine) ); // warning: GC on the returned buffer !

	if ( JL_ARG_ISDEF(3) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &environment) ); // warning: GC on the returned buffer !

	if ( JL_ARG_ISDEF(4) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(4), &currentDirectory) ); // warning: GC on the returned buffer !

	STARTUPINFO si;
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);


	// doc. commandLine parameter: The Unicode version of CreateProcess function, CreateProcessW, can modify the contents of this string !
	PROCESS_INFORMATION pi;
	BOOL st = CreateProcess( applicationName.GetStrConstOrNull(), (LPSTR)commandLine.GetStrConstOrNull(), NULL, NULL, FALSE, 0, (LPVOID)environment.GetStrConstOrNull(), currentDirectory.GetStrConstOrNull(), &si, &pi ); // doc: http://msdn2.microsoft.com/en-us/library/ms682425.aspx
	if ( st == FALSE )
		return WinThrowError(cx, GetLastError());

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	*JL_RVAL = JSVAL_VOID;
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

	if ( argc >= 1 && !JSVAL_IS_VOID( JL_ARG(1) ) ) {

		JLStr str;
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &str) );
		strcpy( filter, str );
		for ( char *tmp = filter; tmp = strchr( tmp, '|' ); tmp++ )
			*tmp = '\0'; // doc: Pointer to a buffer containing pairs of null-terminated filter strings.
		filter[str.Length() + 1] = '\0'; // The last string in the buffer must be terminated by two NULL characters.
		ofn.lpstrFilter = filter;
	}

	if ( argc >= 2 && !JSVAL_IS_VOID( JL_ARG(2) ) ) {

		JLStr tmp;
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &tmp) );
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
		*JL_RVAL = JSVAL_VOID;
	else
		*JL_RVAL = STRING_TO_JSVAL( JS_NewStringCopyZ(cx, fileName) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( sourceString )
  Expands environment-variable strings and replaces them with the values defined for the current user.
**/
DEFINE_FUNCTION( ExpandEnvironmentStrings ) {

	JLStr src;
	JL_S_ASSERT_ARG_MIN(1);
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &src) );
	TCHAR dst[MAX_PATH];
	DWORD res = ExpandEnvironmentStrings( src, dst, sizeof(dst) );
	JL_S_ASSERT( res != 0, "Unable to ExpandEnvironmentStrings." );
	*JL_RVAL = STRING_TO_JSVAL( JS_NewStringCopyN(cx, dst, res) );
	
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
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &timeout) );
	Sleep(timeout);

	*JL_RVAL = JSVAL_VOID;
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
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &type) );
	MessageBeep(type);

	*JL_RVAL = JSVAL_VOID;
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
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &freq) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &duration) );
	Beep(freq, duration);

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( id )
  Creates a new COM object by object name or CLSID.
**/
DEFINE_FUNCTION( CreateComObject ) {

	IUnknown *punk = NULL;
	IDispatch *pdisp = NULL;

	HRESULT hr;

	JL_S_ASSERT_ARG( 1 );
	JL_S_ASSERT_STRING( JL_ARG(1) );

	LPOLESTR name = (LPOLESTR)JS_GetStringCharsZ(cx, JS_ValueToString(cx, JL_ARG(1)));

	CLSID clsid;
	hr = name[0] == L'{' ? CLSIDFromString(name, &clsid) : CLSIDFromProgID(name, &clsid);
	if ( FAILED(hr) )
		JL_CHK( WinThrowError(cx, hr) );

	hr = GetActiveObject(clsid, NULL, &punk);
	if ( FAILED(hr) ) {
		
		hr = CoCreateInstance(clsid, NULL, CLSCTX_SERVER, IID_IUnknown, (void **)&punk); // | CLSCTX_INPROC_HANDLER ???
		if ( FAILED(hr) )
			JL_CHK( WinThrowError(cx, hr) );
	}

//	punk->AddRef(); // see http://stackoverflow.com/questions/645268/in-com-should-i-call-addref-after-cocreateinstance
	hr = punk->QueryInterface(IID_IDispatch, (void **)&pdisp);
	if ( FAILED(hr) )
		JL_CHK( WinThrowError(cx, hr) );
//	pdisp->AddRef();
	JL_CHK( NewComDispatch(cx, pdisp, JL_RVAL) );
	pdisp->Release();
	punk->Release();
	return JS_TRUE;

bad:
	if ( pdisp )
		pdisp->Release();
	if ( punk )
		punk->Release();
	return JS_FALSE;
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
DEFINE_FUNCTION( RegistryGet ) {

	JLStr pathStr, valueName;
	JL_S_ASSERT_ARG_RANGE(1,2);
	
	const char *path;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &pathStr) );
	path = pathStr.GetConstStr();

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

	if ( (argc == 1) || (argc >= 2 && JL_ARG(2) == JSVAL_VOID) ) {

		JSObject *arrObj = JS_NewArrayObject(cx, 0, NULL);
		JL_CHK( arrObj );
		*JL_RVAL = OBJECT_TO_JSVAL(arrObj);

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
			JL_CHK( JL_NativeToJsval(cx, name, nameLength, &strName) );
			JL_CHK( JS_SetElement(cx, arrObj, index, &strName) );
			index++;
		}
		if ( error != ERROR_NO_MORE_ITEMS )
			return WinThrowError(cx, error);

		RegCloseKey(hKey);
		return JS_TRUE;
	}

	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &valueName) );

	DWORD type, size;

	// doc. http://msdn.microsoft.com/en-us/library/ms724911(VS.85).aspx
	error = RegQueryValueEx(hKey, valueName, NULL, &type, NULL, &size);
	if ( error != ERROR_SUCCESS ) {

		RegCloseKey(hKey);
		return WinThrowError(cx, error);
	}

	void *buffer = JS_malloc(cx, size +1);
	error = RegQueryValueEx(hKey, valueName, NULL, NULL, (LPBYTE)buffer, &size);

	// doc. http://msdn.microsoft.com/en-us/library/ms724884(VS.85).aspx
	switch (type) {
		case REG_NONE:
			*JL_RVAL = JSVAL_VOID;
			JS_free(cx, buffer);
			break;
		case REG_BINARY:
			((uint8_t*)buffer)[size] = 0;
			JL_CHK( JL_NewBlob(cx, buffer, size, JL_RVAL) );
			break;
		case REG_DWORD:
			JL_CHK( JL_NativeToJsval(cx, *(DWORD*)buffer, JL_RVAL) );
			JS_free(cx, buffer);
			break;
		case REG_QWORD:
			JL_CHK( JL_NativeToJsval(cx, (double)*(DWORD64*)buffer, JL_RVAL) );
			break;
		case REG_LINK: {
			JSString *jsstr = JS_NewUCString(cx, (jschar*)buffer, size/2);
			JL_CHK( jsstr );
			*JL_RVAL = STRING_TO_JSVAL(jsstr);
			break;
		}
		case REG_EXPAND_SZ:
		case REG_MULTI_SZ:
		case REG_SZ: {
			JSString *jsstr = JS_NewString(cx, (char*)buffer, size-1); // note: ((char*)buffer)[size] already == '\0'
			JL_CHK( jsstr );
			*JL_RVAL = STRING_TO_JSVAL(jsstr);
			break;
		}
	}

	RegCloseKey(hKey);

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE DirectoryChangesHandle $INAME( pathName, notifyFilter [ , watchSubtree = false ] )
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
	BYTE buffer[2][2048];
	int currentBuffer;
	BOOL watchSubtree;
	DWORD	notifyFilter;
};

void FinalizeDirectoryHandle(void *data) {
	
	DirectoryChanges *dc = (DirectoryChanges*)data;
	CloseHandle(dc->hDirectory);
}

DEFINE_FUNCTION( DirectoryChangesInit ) {

	JLStr pathName;
	JL_S_ASSERT_ARG_RANGE(2,3);

	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &pathName) );

	unsigned int notifyFilter;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &notifyFilter) );

	bool watchSubtree;
	if ( JL_ARG_ISDEF(3) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &watchSubtree) );
	else
		watchSubtree = false;

	DirectoryChanges *dc;
	JL_CHK( HandleCreate(cx, JLHID(dmon), sizeof(DirectoryChanges), (void**)&dc, FinalizeDirectoryHandle, JL_RVAL) );

	dc->watchSubtree = watchSubtree;
	dc->notifyFilter = notifyFilter;

	dc->hDirectory = CreateFile(pathName, FILE_LIST_DIRECTORY, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);
	if ( dc->hDirectory == INVALID_HANDLE_VALUE )
		return WinThrowError(cx, GetLastError());

	dc->currentBuffer = 0;
	dc->overlapped.hEvent = NULL;

	if ( !ReadDirectoryChangesW(dc->hDirectory, &dc->buffer[dc->currentBuffer], sizeof(*dc->buffer), dc->watchSubtree, dc->notifyFilter, NULL, &dc->overlapped, NULL) )
		return WinThrowError(cx, GetLastError());

	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $ARR $INAME( directoryChangesHandle [, wait] )
  Returns the list of changed files based on the filter provided to the DirectoryChangesInit() function.
  Each element of the list is a 2-element array that contain the filename and the action.$LF
  actions:
  * 1: The file was added to the directory.
  * 2: The file was removed from the directory.
  * 3: The file was modified. This can be a change in the time stamp or attributes.
  * 4: The file was renamed and this is the old name.
  * 5: The file was renamed and this is the new name.
**/
DEFINE_FUNCTION( DirectoryChangesLookup ) {

	JL_S_ASSERT_ARG_RANGE(1,2);
	JL_S_ASSERT( IsHandleType(cx, JL_ARG(1), JLHID("dmon")), "Unexpected argument type." );
	DirectoryChanges *dc = (DirectoryChanges*)GetHandlePrivate(cx, JL_ARG(1));
	JL_S_ASSERT_RESOURCE( dc );

	bool wait;
	if ( JL_ARG_ISDEF(2) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &wait) );
	else
		wait = false;

	if ( !wait ) {

		DWORD res = WaitForSingleObject(dc->hDirectory, 0);
		if ( res == -1 )
			return WinThrowError(cx,  GetLastError());

		if ( res != WAIT_OBJECT_0 ) { // non signaled
			
			JSObject *arrObj = JS_NewArrayObject(cx, 0, NULL);
			JL_CHK( arrObj );
			*JL_RVAL = OBJECT_TO_JSVAL( arrObj );
			return JS_TRUE;
		}
	}

	DWORD dwNumberbytes;
	if ( !GetOverlappedResult(dc->hDirectory, &dc->overlapped, &dwNumberbytes, wait) )
		return WinThrowError(cx, GetLastError());

	dc->currentBuffer = 1 - dc->currentBuffer; // swap buffers

	if ( !ReadDirectoryChangesW(dc->hDirectory, &dc->buffer[dc->currentBuffer], sizeof(*dc->buffer), dc->watchSubtree, dc->notifyFilter, NULL, &dc->overlapped, NULL) )
		return WinThrowError(cx, GetLastError());

	FILE_NOTIFY_INFORMATION *pFileNotify;
	pFileNotify = (PFILE_NOTIFY_INFORMATION)&dc->buffer[1-dc->currentBuffer];

	JSObject *arrObj = JS_NewArrayObject(cx, 0, NULL);
	JL_CHK( arrObj );
	*JL_RVAL = OBJECT_TO_JSVAL( arrObj );

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
$TOC_MEMBER $INAME
 $TYPE HANDLE $INAME( directoryChangesHandle [, onChanges] )
  Passively waits for directory changes through the ProcessEvents function.
  $H example:
{{{
LoadModule('jsstd');
LoadModule('jswinshell');

var dch = DirectoryChangesInit('C:\\', 0x10, true); // 0x10: FILE_NOTIFY_CHANGE_LAST_WRITE

function onChanges() {

	Print( DirectoryChangesLookup(dch).join('\n'), '\n');
}

while ( !endSignal )
	ProcessEvents( DirectoryChangesEvents(dch, onChanges), EndSignalEvents() );
}}}
**/

struct UserProcessEvent {
	
	ProcessEvent pe;

	DirectoryChanges *dc;
	HANDLE cancelEvent;
};

JL_STATIC_ASSERT( offsetof(UserProcessEvent, pe) == 0 );

static void DirectoryChangesStartWait( volatile ProcessEvent *pe ) {

	UserProcessEvent *upe = (UserProcessEvent*)pe;
	
	const HANDLE events[2] = { upe->cancelEvent, upe->dc->hDirectory };
	DWORD status = WaitForMultipleObjects(COUNTOF(events), events, FALSE, INFINITE);
	JL_ASSERT( status != WAIT_FAILED );
}

static bool DirectoryChangesCancelWait( volatile ProcessEvent *pe ) {

	UserProcessEvent *upe = (UserProcessEvent*)pe;

	BOOL status = SetEvent(upe->cancelEvent);
	JL_ASSERT( status );

	return true;
}

static JSBool DirectoryChangesEndWait( volatile ProcessEvent *pe, bool *hasEvent, JSContext *cx, JSObject *obj ) {
	
	UserProcessEvent *upe = (UserProcessEvent*)pe;

	BOOL status = CloseHandle(upe->cancelEvent);
	JL_ASSERT( status );

	DWORD st = WaitForSingleObject(upe->dc->hDirectory, 0);
	*hasEvent = (st == WAIT_OBJECT_0);

	if ( !*hasEvent )
		return JS_TRUE;

	jsval fct, argv[2];
	JL_CHK( GetHandleSlot(cx, OBJECT_TO_JSVAL(obj), 0, &fct) );
	if ( JSVAL_IS_VOID( fct ) )
		return JS_TRUE;

	JL_CHK( GetHandleSlot(cx, OBJECT_TO_JSVAL(obj), 1, &argv[1]) );
	JL_CHK( JS_CallFunctionValue(cx, JS_GetGlobalObject(cx), fct, COUNTOF(argv)-1, argv+1, argv) );

	return JS_TRUE;
	JL_BAD;
}

DEFINE_FUNCTION( DirectoryChangesEvents ) {
	
	JL_S_ASSERT_ARG_RANGE(1,2);

	JL_S_ASSERT( IsHandleType(cx, JL_ARG(1), JLHID("dmon")), "Unexpected argument type." );

	if ( JL_ARG_ISDEF(2) )
		JL_S_ASSERT_FUNCTION( JL_ARG(2) );

	DirectoryChanges *dc = (DirectoryChanges*)GetHandlePrivate(cx, JL_ARG(1));
	JL_S_ASSERT_RESOURCE( dc );

	UserProcessEvent *upe;
	JL_CHK( HandleCreate(cx, JLHID(pev), sizeof(UserProcessEvent), (void**)&upe, NULL, JL_RVAL) );
	upe->pe.startWait = DirectoryChangesStartWait;
	upe->pe.cancelWait = DirectoryChangesCancelWait;
	upe->pe.endWait = DirectoryChangesEndWait;

	JL_CHK( SetHandleSlot(cx, *JL_RVAL, 0, JL_ARG(2) ) );
	JL_CHK( SetHandleSlot(cx, *JL_RVAL, 1, JL_ARG(1) ) );

	upe->cancelEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	upe->dc = dc;

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

		JLStr str;

		res = OpenClipboard(NULL);
		JL_S_ASSERT( res != 0, "Unable to open the clipboard." );
		JL_CHK( JL_JsvalToNative(cx, *vp, &str) );
		HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, str.Length() + 1);
		JL_S_ASSERT_ALLOC( hglbCopy );
		LPTSTR lptstrCopy = (LPTSTR)GlobalLock(hglbCopy);
		JL_S_ASSERT( lptstrCopy != NULL, "Unable to lock memory." );
		memcpy(lptstrCopy, str.GetConstStr(), str.Length() + 1);
		lptstrCopy[str.Length()] = 0;
		GlobalUnlock(hglbCopy);
		HANDLE h = SetClipboardData(CF_TEXT, hglbCopy);
		JL_S_ASSERT( h != NULL, "Unable to SetClipboardData." );
		CloseClipboard();
	}
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME
  current Windows ANSI code page identifier for the operating system.
**/
DEFINE_PROPERTY( systemCodepage ) {

	*vp = INT_TO_JSVAL(GetACP());
	return JS_TRUE;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME
  current Windows ANSI code page identifier for the console.
**/
DEFINE_PROPERTY( consoleCodepage ) {

	*vp = INT_TO_JSVAL(GetOEMCP());
	return JS_TRUE;
}


///

ALWAYS_INLINE void SetKeyState( BYTE vkey, bool state ) {

	if ( (GetKeyState(vkey) & 1) != state ) {

		keybd_event( vkey, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0 );
		keybd_event( vkey, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	}
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME
  Gets or sets the numlock key state.
**/
DEFINE_PROPERTY_GETTER( numlockState ) {

	return JL_NativeToJsval(cx, GetKeyState(VK_NUMLOCK) & 1, vp);
}

DEFINE_PROPERTY_SETTER( numlockState ) {

	bool state;
	JL_CHK( JL_JsvalToNative(cx, *vp, &state) );
	SetKeyState(VK_NUMLOCK, state);
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME
  Gets or sets the capslock key state.
**/
DEFINE_PROPERTY_GETTER( capslockState ) {

	return JL_NativeToJsval(cx, GetKeyState(VK_CAPITAL) & 1, vp);
}

DEFINE_PROPERTY_SETTER( capslockState ) {

	bool state;
	JL_CHK( JL_JsvalToNative(cx, *vp, &state) );
	SetKeyState(VK_CAPITAL, state);
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME
  Gets or sets the scrolllock key state.
**/
DEFINE_PROPERTY_GETTER( scrolllockState ) {

	return JL_NativeToJsval(cx, GetKeyState(VK_SCROLL) & 1, vp);
}

DEFINE_PROPERTY_SETTER( scrolllockState ) {

	bool state;
	JL_CHK( JL_JsvalToNative(cx, *vp, &state) );
	SetKeyState(VK_SCROLL, state);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME
  is the elapsed time since the last input event (in milliseconds).
**/
DEFINE_PROPERTY( lastInputTime ) {

	LASTINPUTINFO lastInputInfo = {0};
	lastInputInfo.cbSize = sizeof(LASTINPUTINFO);
	if ( ::GetLastInputInfo(&lastInputInfo) == FALSE )
		return WinThrowError(cx, GetLastError());
	return JL_NativeToJsval(cx, ::GetTickCount() - lastInputInfo.dwTime, JL_RVAL);
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME
  ADMINTOOLS              
  ALTSTARTUP					
  APPDATA						
  BITBUCKET					
  CDBURN_AREA					
  COMMON_ADMINTOOLS			
  COMMON_ALTSTARTUP			
  COMMON_APPDATA				
  COMMON_DESKTOPDIRECTORY	
  COMMON_DOCUMENTS			
  COMMON_FAVORITES			
  COMMON_MUSIC				
  COMMON_OEM_LINKS			
  COMMON_PICTURES			
  COMMON_PROGRAMS			
  COMMON_STARTMENU			
  COMMON_STARTUP				
  COMMON_TEMPLATES			
  COMMON_VIDEO				
  COMPUTERSNEARME			
  CONNECTIONS					
  CONTROLS						
  COOKIES						
  DESKTOP						
  DESKTOPDIRECTORY			
  DRIVES						
  FAVORITES					
  FONTS							
  HISTORY						
  INTERNET						
  INTERNET_CACHE				
  LOCAL_APPDATA				
  MYDOCUMENTS					
  MYMUSIC						
  MYPICTURES					
  MYVIDEO						
  NETHOOD						
  NETWORK						
  PERSONAL						
  PRINTERS						
  PRINTHOOD					
  PROFILE						
  PROGRAM_FILES				
  PROGRAM_FILESX86			
  PROGRAM_FILES_COMMON		
  PROGRAM_FILES_COMMONX86	
  PROGRAMS						
  RECENT						
  RESOURCES					
  RESOURCES_LOCALIZED		
  SENDTO						
  STARTMENU					
  STARTUP						
  SYSTEM						
  SYSTEMX86					
  TEMPLATES					
  WINDOWS						
 $H example
{{{
new File( DESKTOP+'\\test.txt' ).content = '1234';
}}}
**/

enum {
	ADMINTOOLS              = CSIDL_ADMINTOOLS ,
	ALTSTARTUP					= CSIDL_ALTSTARTUP ,
	APPDATA						= CSIDL_APPDATA ,
	BITBUCKET					= CSIDL_BITBUCKET ,
	CDBURN_AREA					= CSIDL_CDBURN_AREA ,
	COMMON_ADMINTOOLS			= CSIDL_COMMON_ADMINTOOLS ,
	COMMON_ALTSTARTUP			= CSIDL_COMMON_ALTSTARTUP ,
	COMMON_APPDATA				= CSIDL_COMMON_APPDATA ,
	COMMON_DESKTOPDIRECTORY	= CSIDL_COMMON_DESKTOPDIRECTORY ,
	COMMON_DOCUMENTS			= CSIDL_COMMON_DOCUMENTS ,
	COMMON_FAVORITES			= CSIDL_COMMON_FAVORITES ,
	COMMON_MUSIC				= CSIDL_COMMON_MUSIC ,
	COMMON_OEM_LINKS			= CSIDL_COMMON_OEM_LINKS ,
	COMMON_PICTURES			= CSIDL_COMMON_PICTURES ,
	COMMON_PROGRAMS			= CSIDL_COMMON_PROGRAMS ,
	COMMON_STARTMENU			= CSIDL_COMMON_STARTMENU ,
	COMMON_STARTUP				= CSIDL_COMMON_STARTUP ,
	COMMON_TEMPLATES			= CSIDL_COMMON_TEMPLATES ,
	COMMON_VIDEO				= CSIDL_COMMON_VIDEO ,
	COMPUTERSNEARME			= CSIDL_COMPUTERSNEARME ,
	CONNECTIONS					= CSIDL_CONNECTIONS ,
	CONTROLS						= CSIDL_CONTROLS ,
	COOKIES						= CSIDL_COOKIES ,
	DESKTOP						= CSIDL_DESKTOP ,
	DESKTOPDIRECTORY			= CSIDL_DESKTOPDIRECTORY ,
	DRIVES						= CSIDL_DRIVES ,
	FAVORITES					= CSIDL_FAVORITES ,
	FONTS							= CSIDL_FONTS ,
	HISTORY						= CSIDL_HISTORY ,
	INTERNET						= CSIDL_INTERNET ,
	INTERNET_CACHE				= CSIDL_INTERNET_CACHE ,
	LOCAL_APPDATA				= CSIDL_LOCAL_APPDATA ,
	MYDOCUMENTS					= CSIDL_MYDOCUMENTS ,
	MYMUSIC						= CSIDL_MYMUSIC ,
	MYPICTURES					= CSIDL_MYPICTURES ,
	MYVIDEO						= CSIDL_MYVIDEO ,
	NETHOOD						= CSIDL_NETHOOD ,
	NETWORK						= CSIDL_NETWORK ,
	PERSONAL						= CSIDL_PERSONAL ,
	PRINTERS						= CSIDL_PRINTERS ,
	PRINTHOOD					= CSIDL_PRINTHOOD ,
	PROFILE						= CSIDL_PROFILE ,
	PROGRAM_FILES				= CSIDL_PROGRAM_FILES ,
	PROGRAM_FILESX86			= CSIDL_PROGRAM_FILESX86 ,
	PROGRAM_FILES_COMMON		= CSIDL_PROGRAM_FILES_COMMON ,
	PROGRAM_FILES_COMMONX86	= CSIDL_PROGRAM_FILES_COMMONX86 ,
	PROGRAMS						= CSIDL_PROGRAMS ,
	RECENT						= CSIDL_RECENT ,
	RESOURCES					= CSIDL_RESOURCES ,
	RESOURCES_LOCALIZED		= CSIDL_RESOURCES_LOCALIZED ,
	SENDTO						= CSIDL_SENDTO ,
	STARTMENU					= CSIDL_STARTMENU ,
	STARTUP						= CSIDL_STARTUP ,
	SYSTEM						= CSIDL_SYSTEM ,
	SYSTEMX86					= CSIDL_SYSTEMX86 ,
	TEMPLATES					= CSIDL_TEMPLATES ,
	WINDOWS						= CSIDL_WINDOWS ,
};

DEFINE_PROPERTY( folderPath ) {

	TCHAR path[MAX_PATH];
	if ( SUCCEEDED( SHGetFolderPath(NULL, JSID_TO_INT(id), NULL, 0, path) ) ) // |CSIDL_FLAG_CREATE
		return JL_NativeToJsval(cx, path, vp);
	*vp = JSVAL_VOID;
	return JS_TRUE;
}


#ifdef DEBUG
DEFINE_FUNCTION( jswinshelltest ) {

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
}
#endif //DEBUG


CONFIGURE_STATIC

	REVISION(JL_SvnRevToInt("$Revision$"))
	BEGIN_STATIC_FUNCTION_SPEC
		#ifdef DEBUG
		FUNCTION( jswinshelltest )
		#endif //DEBUG

		FUNCTION( MessageBox )
		FUNCTION( CreateProcess )
		FUNCTION( ExtractIcon )
		FUNCTION( ExpandEnvironmentStrings )
		FUNCTION( FileOpenDialog )
		FUNCTION( Sleep )
		FUNCTION( MessageBeep )
		FUNCTION( Beep )
		FUNCTION( RegistryGet )
		FUNCTION( CreateComObject )

		FUNCTION( DirectoryChangesInit )
		FUNCTION( DirectoryChangesLookup )
		FUNCTION( DirectoryChangesEvents )
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY( clipboard )
		PROPERTY_READ( systemCodepage )
		PROPERTY_READ( consoleCodepage )

		PROPERTY( numlockState )
		PROPERTY( capslockState )
		PROPERTY( scrolllockState )

		PROPERTY_READ( lastInputTime )

		PROPERTY_SWITCH_READ( ADMINTOOLS, folderPath )
		PROPERTY_SWITCH_READ( ALTSTARTUP, folderPath )
		PROPERTY_SWITCH_READ( APPDATA, folderPath )
		PROPERTY_SWITCH_READ( BITBUCKET, folderPath )
		PROPERTY_SWITCH_READ( CDBURN_AREA, folderPath )
		PROPERTY_SWITCH_READ( COMMON_ADMINTOOLS, folderPath )
		PROPERTY_SWITCH_READ( COMMON_ALTSTARTUP, folderPath )
		PROPERTY_SWITCH_READ( COMMON_APPDATA, folderPath )
		PROPERTY_SWITCH_READ( COMMON_DESKTOPDIRECTORY, folderPath )
		PROPERTY_SWITCH_READ( COMMON_DOCUMENTS, folderPath )
		PROPERTY_SWITCH_READ( COMMON_FAVORITES, folderPath )
		PROPERTY_SWITCH_READ( COMMON_MUSIC, folderPath )
		PROPERTY_SWITCH_READ( COMMON_OEM_LINKS, folderPath )
		PROPERTY_SWITCH_READ( COMMON_PICTURES, folderPath )
		PROPERTY_SWITCH_READ( COMMON_PROGRAMS, folderPath )
		PROPERTY_SWITCH_READ( COMMON_STARTMENU, folderPath )
		PROPERTY_SWITCH_READ( COMMON_STARTUP, folderPath )
		PROPERTY_SWITCH_READ( COMMON_TEMPLATES, folderPath )
		PROPERTY_SWITCH_READ( COMMON_VIDEO, folderPath )
		PROPERTY_SWITCH_READ( COMPUTERSNEARME, folderPath )
		PROPERTY_SWITCH_READ( CONNECTIONS, folderPath )
		PROPERTY_SWITCH_READ( CONTROLS, folderPath )
		PROPERTY_SWITCH_READ( COOKIES, folderPath )
		PROPERTY_SWITCH_READ( DESKTOP, folderPath )
		PROPERTY_SWITCH_READ( DESKTOPDIRECTORY, folderPath )
		PROPERTY_SWITCH_READ( DRIVES, folderPath )
		PROPERTY_SWITCH_READ( FAVORITES, folderPath )
		PROPERTY_SWITCH_READ( FONTS, folderPath )
		PROPERTY_SWITCH_READ( HISTORY, folderPath )
		PROPERTY_SWITCH_READ( INTERNET, folderPath )
		PROPERTY_SWITCH_READ( INTERNET_CACHE, folderPath )
		PROPERTY_SWITCH_READ( LOCAL_APPDATA, folderPath )
		PROPERTY_SWITCH_READ( MYDOCUMENTS, folderPath )
		PROPERTY_SWITCH_READ( MYMUSIC, folderPath )
		PROPERTY_SWITCH_READ( MYPICTURES, folderPath )
		PROPERTY_SWITCH_READ( MYVIDEO, folderPath )
		PROPERTY_SWITCH_READ( NETHOOD, folderPath )
		PROPERTY_SWITCH_READ( NETWORK, folderPath )
		PROPERTY_SWITCH_READ( PERSONAL, folderPath )
		PROPERTY_SWITCH_READ( PRINTERS, folderPath )
		PROPERTY_SWITCH_READ( PRINTHOOD, folderPath )
		PROPERTY_SWITCH_READ( PROFILE, folderPath )
		PROPERTY_SWITCH_READ( PROGRAM_FILES, folderPath )
		PROPERTY_SWITCH_READ( PROGRAM_FILESX86, folderPath )
		PROPERTY_SWITCH_READ( PROGRAM_FILES_COMMON, folderPath )
		PROPERTY_SWITCH_READ( PROGRAM_FILES_COMMONX86, folderPath )
		PROPERTY_SWITCH_READ( PROGRAMS, folderPath )
		PROPERTY_SWITCH_READ( RECENT, folderPath )
		PROPERTY_SWITCH_READ( RESOURCES, folderPath )
		PROPERTY_SWITCH_READ( RESOURCES_LOCALIZED, folderPath )
		PROPERTY_SWITCH_READ( SENDTO, folderPath )
		PROPERTY_SWITCH_READ( STARTMENU, folderPath )
		PROPERTY_SWITCH_READ( STARTUP, folderPath )
		PROPERTY_SWITCH_READ( SYSTEM, folderPath )
		PROPERTY_SWITCH_READ( SYSTEMX86, folderPath )
		PROPERTY_SWITCH_READ( TEMPLATES, folderPath )
		PROPERTY_SWITCH_READ( WINDOWS, folderPath )
		
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
