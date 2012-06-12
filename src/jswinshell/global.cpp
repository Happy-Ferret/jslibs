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
#include "com.h"

DECLARE_CLASS( Icon )


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
DEFINE_FUNCTION( extractIcon ) {

	JLData fileName;
	JL_ASSERT_ARGC_MIN(1);

	UINT iconIndex = 0;
	if ( argc >= 2 )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &iconIndex) );
	HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
	if ( hInst == NULL )
		return JL_ThrowOSError(cx);
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &fileName) );
	HICON hIcon = ExtractIcon( hInst, fileName, iconIndex ); // see SHGetFileInfo(
	if ( hIcon == NULL ) {

//		if ( GetLastError() != 0 )
//			return WinThrowError(cx, GetLastError());
		*JL_RVAL = JSVAL_VOID;
		return JS_TRUE;
	}
	JSObject *icon = JL_NewObjectWithGivenProto(cx, JL_CLASS(Icon), JL_CLASS_PROTOTYPE(cx, Icon), NULL);
	HICON *phIcon = (HICON*)jl_malloc(sizeof(HICON)); // this is needed because JL_SetPrivate stores ONLY alligned values
	JL_ASSERT_ALLOC( phIcon );
	*phIcon = hIcon;
	JL_SetPrivate( icon, phIcon);
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
DEFINE_FUNCTION( messageBox ) {

	JLData caption, text;

	JL_ASSERT_ARGC_MIN(1);

	UINT type = 0;
	if ( argc >= 3 )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &type) );

	if ( argc >= 2 && !JSVAL_IS_VOID( JL_ARG(2) ) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &caption) );

	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &text) );

	int res = MessageBox(NULL, text.GetConstStrZ(), caption.GetConstStrZOrNULL(), type);
	if ( res == 0 )
		return JL_ThrowOSError(cx);
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
 createProcess( 'C:\\WINDOWS\\system32\\calc.exe', undefined, undefined, 'c:\\' );
 }}}
**/
DEFINE_FUNCTION( createProcess ) {

	JLData applicationName, commandLine, environment, currentDirectory;

	JL_ASSERT_ARGC_MIN(1);

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
 fileOpenDialog( 'executable files|*.exe;*.com;*.cmd;*.bat|all files|*.*' );
 }}}
**/
DEFINE_FUNCTION( fileOpenDialog ) {

	OPENFILENAME ofn = { sizeof(OPENFILENAME) };
	char fileName[PATH_MAX];
	char filter[255];

	if ( argc >= 1 && !JSVAL_IS_VOID( JL_ARG(1) ) ) {

		JLData str;
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &str) );
		strcpy( filter, str );
		for ( char *tmp = filter; (tmp = strchr(tmp, '|')) != 0; tmp++ )
			*tmp = '\0'; // doc: Pointer to a buffer containing pairs of null-terminated filter strings.
		filter[str.Length() + 1] = '\0'; // The last string in the buffer must be terminated by two NULL characters.
		ofn.lpstrFilter = filter;
	}

	if ( argc >= 2 && !JSVAL_IS_VOID( JL_ARG(2) ) ) {

		JLData tmp;
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &tmp) );
		strcpy( fileName, tmp );
	} else {
		*fileName = '\0';
	}

	ofn.lpstrFile = fileName;
	ofn.nMaxFile = sizeof(fileName);
	ofn.Flags = OFN_NOCHANGEDIR | OFN_LONGNAMES | OFN_HIDEREADONLY;
	BOOL res = GetOpenFileName(&ofn); // doc: http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winui/winui/windowsuserinterface/userinput/commondialogboxlibrary/commondialogboxreference/commondialogboxstructures/openfilename.asp
	
	if ( res == 0 ) {
	
		DWORD err = CommDlgExtendedError();
		JL_ERR( E_OS, E_OPERATION, E_ERRNO(err) );
	}

	*JL_RVAL = STRING_TO_JSVAL( JS_NewStringCopyZ(cx, fileName) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( sourceString )
  Expands environment-variable strings and replaces them with the values defined for the current user.
**/
DEFINE_FUNCTION( expandEnvironmentStrings ) {

	JLData src;
	JL_ASSERT_ARGC_MIN(1);

	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &src) );
	TCHAR dst[PATH_MAX];
	DWORD res = ExpandEnvironmentStrings( src, dst, sizeof(dst) );
	if ( res == 0 )
		return JL_ThrowOSError(cx);
	*JL_RVAL = STRING_TO_JSVAL( JS_NewStringCopyN(cx, dst, res) );
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
DEFINE_FUNCTION( messageBeep ) {

	UINT type = (UINT)-1;
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
DEFINE_FUNCTION( beep ) {

	JL_ASSERT_ARGC_MIN(2);

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
DEFINE_FUNCTION( createComObject ) {

	IUnknown *punk = NULL;
	IDispatch *pdisp = NULL;

	HRESULT hr;

	JL_ASSERT_ARGC( 1 );
	JL_ASSERT_ARG_IS_STRING(1);

	JSString *idStr = JS_ValueToString(cx, JL_ARG(1));
	LPOLESTR name = (LPOLESTR)JS_GetStringCharsZ(cx, idStr);

	CLSID clsid;
	hr = name[0] == L('{') ? CLSIDFromString(name, &clsid) : CLSIDFromProgID(name, &clsid);
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



HKEY
ParseRootKey(IN const char *path, OUT size_t *length) {

	if ( !strncmp(path, "HKEY_CURRENT_USER", 17) ) {
		*length = 17;
		return HKEY_CURRENT_USER;
	} else
	if ( !strncmp(path, "HKCU", 4) ) {
		*length = 4;
		return HKEY_CURRENT_USER;
	} else
	if ( !strncmp(path, "HKEY_LOCAL_MACHINE", 18) ) {
		*length = 18;
		return HKEY_LOCAL_MACHINE;
	} else
	if ( !strncmp(path, "HKLM", 4) ) {
		*length = 4;
		return HKEY_LOCAL_MACHINE;
	} else
	if ( !strncmp(path, "HKEY_CLASSES_ROOT", 17) ) {
		*length = 17;
		return HKEY_CLASSES_ROOT;
	} else
	if ( !strncmp(path, "HKCR", 4) ) {
		*length = 4;
		return HKEY_CLASSES_ROOT;
	} else
	if ( !strncmp(path, "HKEY_CURRENT_CONFIG", 19) ) {
		*length = 19;
		return HKEY_CURRENT_CONFIG;
	} else
	if ( !strncmp(path, "HKCC", 4) ) {
		*length = 4;
		return HKEY_CURRENT_CONFIG;
	} else
	if ( !strncmp(path, "HKEY_USERS", 10) ) {
		*length = 10;
		return HKEY_USERS;
	} else
	if ( !strncmp(path, "HKU", 3) ) {
		*length = 3;
		return HKEY_USERS;
	} else
	if ( !strncmp(path, "HKEY_PERFORMANCE_DATA", 21) ) {
		*length = 21;
		return HKEY_PERFORMANCE_DATA;
	} else
	if ( !strncmp(path, "HKPD", 4) ) {
		*length = 4;
		return HKEY_PERFORMANCE_DATA;
	} else
	if ( !strncmp(path, "HKEY_DYN_DATA", 13) ) {
		*length = 13;
		return HKEY_DYN_DATA;
	} else
	if ( !strncmp(path, "HKDD", 4) ) {
		*length = 4;
		return HKEY_DYN_DATA;
	} else {
		
		return NULL;
	}
}

DEFINE_FUNCTION( registrySet ) {
	
	jsval value;
	JLData subKeyStr, valueNameStr;
	const char *subKey;

	JL_ASSERT_ARGC(3);
	
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &subKeyStr) );
	subKey = subKeyStr.GetConstStrZ();

	size_t length;
	HKEY rootHKey = ParseRootKey(subKey, &length);
	JL_ASSERT( subKey != NULL, E_ARG, E_NUM(1), E_INVALID, E_COMMENT("root key") );
	subKey += length;
	if ( subKey[0] == '\\' )
		subKey += 1;

	LONG st;

	value = JL_ARG(3);

	if ( JSVAL_IS_VOID(JL_ARG(2)) ) {

		if ( JSVAL_IS_VOID(value) ) {

			st = RegDeleteKey(rootHKey, subKey);
			if ( st != ERROR_SUCCESS )
				return WinThrowError(cx, st);
		}
	} else {

		HKEY hKey;
		st = RegCreateKeyEx(rootHKey, subKey, 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL); // http://msdn.microsoft.com/en-us/library/windows/desktop/ms724844(v=vs.85).aspx
		if ( st != ERROR_SUCCESS )
			return WinThrowError(cx, st);

		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &valueNameStr) );

		if ( JSVAL_IS_VOID(value) ) {
			
			st = RegDeleteValue(hKey, valueNameStr);
		} else
		if ( JSVAL_IS_NULL(value) ) {

			st = RegSetValueEx(hKey, valueNameStr, 0, REG_NONE, NULL, 0);
		} else
		if ( JSVAL_IS_INT(value) ) {

			DWORD num;
			JL_CHK( JL_JsvalToNative(cx, value, &num) );
			st = RegSetValueEx(hKey, valueNameStr, 0, REG_DWORD, (LPBYTE)&num, sizeof(DWORD));
		} else
		if ( JSVAL_IS_DOUBLE(value) ) {

			uint64_t num;
			JL_CHK( JL_JsvalToNative(cx, value, &num) );
			st = RegSetValueEx(hKey, valueNameStr, 0, REG_QWORD, (LPBYTE)&num, sizeof(uint64_t));
		} else
		if ( JSVAL_IS_STRING(value) ) {

			JLData tmp;
			JL_CHK( JL_JsvalToNative(cx, value, &tmp) );
			// doc: When writing a string to the registry, you must specify the length of the string, including the terminating null character (\0).
			st = RegSetValueEx(hKey, valueNameStr, 0, REG_SZ, (LPBYTE)tmp.GetConstStrZ(), tmp.Length() + 1);
		} else
		if ( JL_ValueIsData(cx, value) ) {

			JLData tmp;
			JL_CHK( JL_JsvalToNative(cx, value, &tmp) );
			st = RegSetValueEx(hKey, valueNameStr, 0, REG_BINARY, (LPBYTE)tmp.GetConstStr(), tmp.Length());
		}

		if ( st != ERROR_SUCCESS )
			return WinThrowError(cx, st);

		st = RegCloseKey(hKey);
		if ( st != ERROR_SUCCESS )
			return WinThrowError(cx, st);
	}

	*JL_RVAL = JSVAL_VOID;

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
  registryGet('HKEY_CURRENT_USER\\Software\\7-Zip'); // returns ["FM"]
  registryGet('HKEY_CURRENT_USER\\Software\\7-Zip', undefined); // returns ["Path", "Lang"]
  registryGet('HKEY_CURRENT_USER\\Software\\7-Zip', 'path') // returns "C:\\Program Files\\7-Zip"
  }}}
  $H example 2
  {{{
  var path = 'HKEY_LOCAL_MACHINE\\Software\\Clients\\StartMenuInternet';
  var defaultBrowser = registryGet(path+'\\' + registryGet(path, '') + '\\shell\\open\\command', '');
  createProcess(undefined, defaultBrowser + ' http://jslibs.googlecode.com/');
  }}}
**/
DEFINE_FUNCTION( registryGet ) {

	uint8_t *buffer = NULL;
	JLData pathStr, valueName;

	JL_ASSERT_ARGC_RANGE(1,2);
	
	const char *path;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &pathStr) );
	path = pathStr.GetConstStrZ();

	size_t length;
	HKEY rootHKey = ParseRootKey(path, &length);
	JL_ASSERT( rootHKey != NULL, E_ARG, E_NUM(1), E_INVALID, E_COMMENT("root key") );
	path += length;

	if ( path[0] == '\\' )
		path++;

	HKEY hKey;
	LONG st;

	st = RegOpenKeyEx(rootHKey, path, 0, KEY_READ, &hKey); // http://msdn.microsoft.com/en-us/library/ms724897%28VS.85%29.aspx
	if ( st != ERROR_SUCCESS )
		return WinThrowError(cx, st);

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
				st = RegEnumKeyEx(hKey, index, name, &nameLength, NULL, NULL, NULL, NULL);
			else
				st = RegEnumValue(hKey, index, name, &nameLength, NULL, NULL, NULL, NULL); // doc. http://msdn.microsoft.com/en-us/library/ms724865(VS.85).aspx

			if ( st != ERROR_SUCCESS )
				break;
			jsval strName;
			JL_CHK( JL_NativeToJsval(cx, name, nameLength, &strName) );
			JL_CHK( JL_SetElement(cx, arrObj, index, &strName) );
			index++;
		}
		if ( st != ERROR_NO_MORE_ITEMS )
			return WinThrowError(cx, st);

		RegCloseKey(hKey);
		return JS_TRUE;
	}

	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &valueName) );

	DWORD type, size;

	// doc. http://msdn.microsoft.com/en-us/library/ms724911(VS.85).aspx
	st = RegQueryValueEx(hKey, valueName, NULL, &type, NULL, &size);

	if ( st == ERROR_FILE_NOT_FOUND ) {
		
		*JL_RVAL = JSVAL_VOID;
		RegCloseKey(hKey);
		return JS_TRUE;
	}

	if ( st != ERROR_SUCCESS ) {

		RegCloseKey(hKey);
		return WinThrowError(cx, st);
	}

	buffer = JL_DataBufferAlloc(cx, size);
	st = RegQueryValueEx(hKey, valueName, NULL, NULL, buffer, &size);

	// doc. http://msdn.microsoft.com/en-us/library/ms724884(VS.85).aspx
	switch (type) {
		case REG_NONE:
			*JL_RVAL = JSVAL_NULL;
			JL_DataBufferFree(cx, buffer);
			break;
		case REG_BINARY:
			JL_CHK( JL_NewBufferGetOwnership(cx, buffer, size, JL_RVAL) );
			break;
		case REG_DWORD:
			JL_CHK( JL_NativeToJsval(cx, *(DWORD*)buffer, JL_RVAL) );
			JL_DataBufferFree(cx, buffer);
			break;
		case REG_QWORD:
			JL_CHK( JL_NativeToJsval(cx, (double)*(DWORD64*)buffer, JL_RVAL) );
			break;
		case REG_LINK: {
			JSString *jsstr = JL_NewUCString(cx, (jschar*)buffer, size/2);
			JL_CHK( jsstr );
			*JL_RVAL = STRING_TO_JSVAL(jsstr);
			break;
		}
		case REG_EXPAND_SZ:
		case REG_MULTI_SZ:
		case REG_SZ: {
			//JSString *jsstr = JL_NewString(cx, (char*)buffer, size-1); // note: ((char*)buffer)[size] already == '\0'
			JL_CHK( JLData((char*)buffer, true, size-1).GetJSString(cx, JL_RVAL) );
			break;
		}
	}

	RegCloseKey(hKey);

	return JS_TRUE;
bad:
	JL_DataBufferFree(cx, buffer);
	return JS_FALSE;
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
  var dch = directoryChangesInit('C:\\WINDOWS', 0x10|0x40, true);
  while (!host.endSignal) {

    print( uneval( directoryChangesLookup(dch) ), '\n');
    sleep(1000);
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

DEFINE_FUNCTION( directoryChangesInit ) {

	JLData pathName;

	JL_ASSERT_ARGC_RANGE(2,3);

	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &pathName) );

	unsigned int notifyFilter;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &notifyFilter) );

	bool watchSubtree;
	if ( JL_ARG_ISDEF(3) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &watchSubtree) );
	else
		watchSubtree = false;

	DirectoryChanges *dc;
	JL_CHK( HandleCreate(cx, JLHID(dmon), &dc, FinalizeDirectoryHandle, JL_RVAL) );

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
DEFINE_FUNCTION( directoryChangesLookup ) {

	JL_ASSERT_ARGC_RANGE(1,2);
	JL_ASSERT_ARG_TYPE( IsHandleType(cx, JL_ARG(1), JLHID(dmon)), 1, "(dmon) Handle" );

	DirectoryChanges *dc = (DirectoryChanges*)GetHandlePrivate(cx, JL_ARG(1));
	JL_ASSERT( dc, E_ARG, E_NUM(1), E_STATE );

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
	int index = 0;
	// see http://www.google.fr/codesearch/p?hl=fr&sa=N&cd=17&ct=rc#8WOCRDPt-u8/trunk/src/FileWatch.cc&q=ReadDirectoryChangesW
	while ( pFileNotify ) {

		JSString *str = JS_NewUCStringCopyN(cx, (jschar*)pFileNotify->FileName, pFileNotify->FileNameLength / sizeof(WCHAR));
		JL_CHK( str );
		eltContent[0] = STRING_TO_JSVAL( str );
		eltContent[1] = INT_TO_JSVAL( pFileNotify->Action );
		JSObject *elt = JS_NewArrayObject(cx, COUNTOF(eltContent), eltContent);
		JL_CHK( elt );
		tmp = OBJECT_TO_JSVAL( elt );
		JL_CHK( JL_SetElement(cx, arrObj, index, &tmp) );
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
  Passively waits for directory changes through the processEvents function.
  $H example:
{{{
loadModule('jsstd');
loadModule('jswinshell');

var dch = directoryChangesInit('C:\\', 0x10, true); // 0x10: FILE_NOTIFY_CHANGE_LAST_WRITE

function onChanges() {

	print( directoryChangesLookup(dch).join('\n'), '\n');
}

while ( !host.endSignal )
	processEvents( directoryChangesEvents(dch, onChanges), host.endSignalEvents() );
}}}
**/

struct DirectoryUserProcessEvent {
	
	ProcessEvent pe;

	DirectoryChanges *dc;
	HANDLE cancelEvent;

	JSObject *callbackFunctionThis;
	jsval callbackFunction;

	jsval dcVal;
};

S_ASSERT( offsetof(DirectoryUserProcessEvent, pe) == 0 );

static JSBool DirectoryChangesPrepareWait( volatile ProcessEvent *, JSContext *, JSObject * ) {
	
	return JS_TRUE;
}

static void DirectoryChangesStartWait( volatile ProcessEvent *pe ) {

	DirectoryUserProcessEvent *upe = (DirectoryUserProcessEvent*)pe;
	
	const HANDLE events[] = { upe->cancelEvent, upe->dc->hDirectory };
	DWORD status = WaitForMultipleObjects(COUNTOF(events), events, FALSE, INFINITE);
	ASSERT( status != WAIT_FAILED );
}

static bool DirectoryChangesCancelWait( volatile ProcessEvent *pe ) {

	DirectoryUserProcessEvent *upe = (DirectoryUserProcessEvent*)pe;

	BOOL status = SetEvent(upe->cancelEvent);
	ASSERT( status );

	return true;
}

static JSBool DirectoryChangesEndWait( volatile ProcessEvent *pe, bool *hasEvent, JSContext *cx, JSObject * ) {
	
	DirectoryUserProcessEvent *upe = (DirectoryUserProcessEvent*)pe;

	DWORD st = WaitForSingleObject(upe->dc->hDirectory, 0);
	*hasEvent = (st == WAIT_OBJECT_0);

	if ( !*hasEvent )
		return JS_TRUE;

	if ( JSVAL_IS_VOID(upe->callbackFunction) )
		return JS_TRUE;

	jsval rval;
	JL_CHK( JL_CallFunctionVA(cx, upe->callbackFunctionThis, upe->callbackFunction, &rval, upe->dcVal) );
	return JS_TRUE;
	JL_BAD;
}

static void FinalizeDirectoryChangesEvents( void *data ) {
	
	DirectoryUserProcessEvent *upe = (DirectoryUserProcessEvent*)data;

	BOOL status = CloseHandle(upe->cancelEvent);
	ASSERT( status );
}


DEFINE_FUNCTION( directoryChangesEvents ) {
	
	JL_ASSERT_ARGC_RANGE(1, 2);
	JL_ASSERT_ARG_TYPE( IsHandleType(cx, JL_ARG(1), JLHID(dmon)), 1, "(dmon) Handle" );

	DirectoryChanges *dc = (DirectoryChanges*)GetHandlePrivate(cx, JL_ARG(1));
	JL_ASSERT( dc, E_ARG, E_NUM(1), E_STATE );

	DirectoryUserProcessEvent *upe;
	JL_CHK( HandleCreate(cx, JLHID(pev), &upe, FinalizeDirectoryChangesEvents, JL_RVAL) );
	upe->pe.prepareWait = DirectoryChangesPrepareWait;
	upe->pe.startWait = DirectoryChangesStartWait;
	upe->pe.cancelWait = DirectoryChangesCancelWait;
	upe->pe.endWait = DirectoryChangesEndWait;

	if ( JL_ARG_ISDEF(2) ) {

		JL_ASSERT_ARG_IS_CALLABLE(2);
		JL_CHK( SetHandleSlot(cx, *JL_RVAL, 0, JL_OBJVAL) ); // GC protection only
		JL_CHK( SetHandleSlot(cx, *JL_RVAL, 1, JL_ARG(1)) ); // GC protection only
		JL_CHK( SetHandleSlot(cx, *JL_RVAL, 2, JL_ARG(2) ) ); // GC protection only

		upe->callbackFunctionThis = JSVAL_TO_OBJECT(JL_OBJVAL); // store "this" object.
		upe->callbackFunction = JL_ARG(2);
		upe->dcVal = JL_ARG(1); // dmon handle (argument 1 of the callback function)
	} else {
	
		upe->callbackFunction = JSVAL_VOID;
	}

	upe->dc = dc;
	upe->cancelEvent = CreateEvent(NULL, FALSE, FALSE, NULL); // auto-reset

//	if ( upe->cancelEvent == NULL )
//		JL_ThrowOSError(cx);

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( $STR )
  Converts a globally unique identifier (GUID) into a string of printable characters.
  $H example:
{{{
var file = new File("C:\\tmp");
print(guidToString(f.id).quote());
}}}
**/
DEFINE_FUNCTION( guidToString ) {

	JLData str;

	JL_ASSERT_ARGC(1);

	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &str) );

	JL_ASSERT( str.Length() == sizeof(GUID), E_ARG, E_NUM(1), E_LENGTH, E_NUM(sizeof(GUID)) );

	GUID guid;
	CopyMemory(&guid, str.GetConstStr(), sizeof(GUID));
	WCHAR szGuid[39];
	int len = StringFromGUID2(guid, szGuid, 39);
	ASSERT( len == 39 );
	ASSERT( szGuid[38] == 0 );

	JL_CHK( JL_NativeToJsval(cx, szGuid, 38, JL_RVAL) );

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
DEFINE_PROPERTY_GETTER( clipboard ) {

	JL_IGNORE( id, obj );

	BOOL res = OpenClipboard(NULL);
	if ( res == 0 )
		return JL_ThrowOSError(cx);

	if ( IsClipboardFormatAvailable(CF_TEXT) == 0 ) {

		*vp = JSVAL_NULL;
	} else {

		HANDLE hglb = GetClipboardData(CF_TEXT);
		if ( !hglb )
			return JL_ThrowOSError(cx);

		LPTSTR lptstr = (LPTSTR)GlobalLock(hglb);
		if ( lptstr == NULL )
			return JL_ThrowOSError(cx);
		JSString *str = JS_NewStringCopyZ(cx, lptstr);
		JL_ASSERT_ALLOC( str );
		*vp = STRING_TO_JSVAL(str);
		GlobalUnlock(hglb);
		CloseClipboard();
	}
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( clipboard ) {

	JL_IGNORE( strict, id, obj );

	BOOL res = OpenClipboard(NULL);
	if ( res == 0 )
		return JL_ThrowOSError(cx);
	res = EmptyClipboard(); // doc: If the application specifies a NULL window handle when opening the clipboard, EmptyClipboard succeeds but sets the clipboard owner to NULL. Note that this causes SetClipboardData to fail.
	if ( res == 0 )
		return JL_ThrowOSError(cx);
	res = CloseClipboard();
	if ( res == 0 )
		return JL_ThrowOSError(cx);


	if ( !JSVAL_IS_VOID( *vp ) ) {

		JLData str;

		res = OpenClipboard(NULL);
		if ( res == 0 )
			return JL_ThrowOSError(cx);
		JL_CHK( JL_JsvalToNative(cx, *vp, &str) );
		HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, str.Length() + 1);
		JL_ASSERT_ALLOC( hglbCopy );
		LPTSTR lptstrCopy = (LPTSTR)GlobalLock(hglbCopy);
		if ( lptstrCopy == NULL )
			return JL_ThrowOSError(cx);
		jl::memcpy(lptstrCopy, str.GetConstStr(), str.Length() + 1);
		lptstrCopy[str.Length()] = 0;
		GlobalUnlock(hglbCopy);
		HANDLE h = SetClipboardData(CF_TEXT, hglbCopy);
		if ( h == NULL )
			return JL_ThrowOSError(cx);
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
DEFINE_PROPERTY_GETTER( systemCodepage ) {

	JL_IGNORE( id, obj, cx );

	*vp = INT_TO_JSVAL(GetACP());
	return JS_TRUE;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME
  current Windows ANSI code page identifier for the console.
**/
DEFINE_PROPERTY_GETTER( consoleCodepage ) {

	JL_IGNORE( id, obj, cx );

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

	JL_IGNORE( id, obj );

	return JL_NativeToJsval(cx, GetKeyState(VK_NUMLOCK) & 1, vp);
}

DEFINE_PROPERTY_SETTER( numlockState ) {

	JL_IGNORE( strict, id, obj );

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

	JL_IGNORE( id, obj );
	return JL_NativeToJsval(cx, GetKeyState(VK_CAPITAL) & 1, vp);
}

DEFINE_PROPERTY_SETTER( capslockState ) {

	JL_IGNORE( strict, id, obj );

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

	JL_IGNORE( id, obj );

	return JL_NativeToJsval(cx, GetKeyState(VK_SCROLL) & 1, vp);
}

DEFINE_PROPERTY_SETTER( scrolllockState ) {

	JL_IGNORE( strict, id, obj );

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
DEFINE_PROPERTY_GETTER( lastInputTime ) {

	JL_IGNORE( id, obj );

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
	ADMINTOOLS               = CSIDL_ADMINTOOLS,
	ALTSTARTUP               = CSIDL_ALTSTARTUP,
	APPDATA                  = CSIDL_APPDATA,
	BITBUCKET                = CSIDL_BITBUCKET,
	CDBURN_AREA              = CSIDL_CDBURN_AREA,
	COMMON_ADMINTOOLS        = CSIDL_COMMON_ADMINTOOLS,
	COMMON_ALTSTARTUP        = CSIDL_COMMON_ALTSTARTUP,
	COMMON_APPDATA           = CSIDL_COMMON_APPDATA,
	COMMON_DESKTOPDIRECTORY  = CSIDL_COMMON_DESKTOPDIRECTORY,
	COMMON_DOCUMENTS         = CSIDL_COMMON_DOCUMENTS,
	COMMON_FAVORITES         = CSIDL_COMMON_FAVORITES,
	COMMON_MUSIC             = CSIDL_COMMON_MUSIC,
	COMMON_OEM_LINKS         = CSIDL_COMMON_OEM_LINKS,
	COMMON_PICTURES          = CSIDL_COMMON_PICTURES,
	COMMON_PROGRAMS          = CSIDL_COMMON_PROGRAMS,
	COMMON_STARTMENU         = CSIDL_COMMON_STARTMENU,
	COMMON_STARTUP           = CSIDL_COMMON_STARTUP,
	COMMON_TEMPLATES         = CSIDL_COMMON_TEMPLATES,
	COMMON_VIDEO             = CSIDL_COMMON_VIDEO,
	COMPUTERSNEARME          = CSIDL_COMPUTERSNEARME,
	CONNECTIONS              = CSIDL_CONNECTIONS,
	CONTROLS                 = CSIDL_CONTROLS,
	COOKIES                  = CSIDL_COOKIES,
	DESKTOP                  = CSIDL_DESKTOP,
	DESKTOPDIRECTORY         = CSIDL_DESKTOPDIRECTORY,
	DRIVES                   = CSIDL_DRIVES,
	FAVORITES                = CSIDL_FAVORITES,
	FONTS                    = CSIDL_FONTS,
	HISTORY                  = CSIDL_HISTORY,
	INTERNET                 = CSIDL_INTERNET,
	INTERNET_CACHE           = CSIDL_INTERNET_CACHE,
	LOCAL_APPDATA            = CSIDL_LOCAL_APPDATA,
	MYDOCUMENTS              = CSIDL_MYDOCUMENTS,
	MYMUSIC                  = CSIDL_MYMUSIC,
	MYPICTURES               = CSIDL_MYPICTURES,
	MYVIDEO                  = CSIDL_MYVIDEO,
	NETHOOD                  = CSIDL_NETHOOD,
	NETWORK                  = CSIDL_NETWORK,
	PERSONAL                 = CSIDL_PERSONAL,
	PRINTERS                 = CSIDL_PRINTERS,
	PRINTHOOD                = CSIDL_PRINTHOOD,
	PROFILE                  = CSIDL_PROFILE,
	PROGRAM_FILES            = CSIDL_PROGRAM_FILES,
	PROGRAM_FILESX86         = CSIDL_PROGRAM_FILESX86,
	PROGRAM_FILES_COMMON     = CSIDL_PROGRAM_FILES_COMMON,
	PROGRAM_FILES_COMMONX86  = CSIDL_PROGRAM_FILES_COMMONX86,
	PROGRAMS                 = CSIDL_PROGRAMS,
	RECENT                   = CSIDL_RECENT,
	RESOURCES                = CSIDL_RESOURCES,
	RESOURCES_LOCALIZED      = CSIDL_RESOURCES_LOCALIZED,
	SENDTO                   = CSIDL_SENDTO,
	STARTMENU                = CSIDL_STARTMENU,
	STARTUP                  = CSIDL_STARTUP,
	SYSTEM                   = CSIDL_SYSTEM,
	SYSTEMX86                = CSIDL_SYSTEMX86,
	TEMPLATES                = CSIDL_TEMPLATES,
	WINDOWS                  = CSIDL_WINDOWS,
};

DEFINE_PROPERTY_GETTER( folderPath ) {

	JL_IGNORE( obj );

	TCHAR path[PATH_MAX];
	if ( SUCCEEDED( SHGetFolderPath(NULL, JSID_TO_INT(id), NULL, 0, path) ) ) // |CSIDL_FLAG_CREATE
		return JL_NativeToJsval(cx, path, vp);
	*vp = JSVAL_VOID;
	return JS_TRUE;
}


#ifdef DEBUG
DEFINE_FUNCTION( jswinshelltest ) {

	JL_IGNORE( argc, cx );

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
}
#endif //DEBUG


CONFIGURE_STATIC

	REVISION(jl::SvnRevToInt("$Revision$"))
	BEGIN_STATIC_FUNCTION_SPEC
		#ifdef DEBUG
		FUNCTION( jswinshelltest )
		#endif //DEBUG

		FUNCTION( messageBox )
		FUNCTION( createProcess )
		FUNCTION( extractIcon )
		FUNCTION( expandEnvironmentStrings )
		FUNCTION( fileOpenDialog )
		FUNCTION( messageBeep )
		FUNCTION( beep )
		FUNCTION( registrySet )
		FUNCTION( registryGet )
		FUNCTION( createComObject )

		FUNCTION( directoryChangesInit )
		FUNCTION( directoryChangesLookup )
		FUNCTION( directoryChangesEvents )

		FUNCTION_ARGC( guidToString, 1 )
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY( clipboard )
		PROPERTY_GETTER( systemCodepage )
		PROPERTY_GETTER( consoleCodepage )

		PROPERTY( numlockState )
		PROPERTY( capslockState )
		PROPERTY( scrolllockState )

		PROPERTY_GETTER( lastInputTime )

		PROPERTY_SWITCH_GETTER( ADMINTOOLS, folderPath )
		PROPERTY_SWITCH_GETTER( ALTSTARTUP, folderPath )
		PROPERTY_SWITCH_GETTER( APPDATA, folderPath )
		PROPERTY_SWITCH_GETTER( BITBUCKET, folderPath )
		PROPERTY_SWITCH_GETTER( CDBURN_AREA, folderPath )
		PROPERTY_SWITCH_GETTER( COMMON_ADMINTOOLS, folderPath )
		PROPERTY_SWITCH_GETTER( COMMON_ALTSTARTUP, folderPath )
		PROPERTY_SWITCH_GETTER( COMMON_APPDATA, folderPath )
		PROPERTY_SWITCH_GETTER( COMMON_DESKTOPDIRECTORY, folderPath )
		PROPERTY_SWITCH_GETTER( COMMON_DOCUMENTS, folderPath )
		PROPERTY_SWITCH_GETTER( COMMON_FAVORITES, folderPath )
		PROPERTY_SWITCH_GETTER( COMMON_MUSIC, folderPath )
		PROPERTY_SWITCH_GETTER( COMMON_OEM_LINKS, folderPath )
		PROPERTY_SWITCH_GETTER( COMMON_PICTURES, folderPath )
		PROPERTY_SWITCH_GETTER( COMMON_PROGRAMS, folderPath )
		PROPERTY_SWITCH_GETTER( COMMON_STARTMENU, folderPath )
		PROPERTY_SWITCH_GETTER( COMMON_STARTUP, folderPath )
		PROPERTY_SWITCH_GETTER( COMMON_TEMPLATES, folderPath )
		PROPERTY_SWITCH_GETTER( COMMON_VIDEO, folderPath )
		PROPERTY_SWITCH_GETTER( COMPUTERSNEARME, folderPath )
		PROPERTY_SWITCH_GETTER( CONNECTIONS, folderPath )
		PROPERTY_SWITCH_GETTER( CONTROLS, folderPath )
		PROPERTY_SWITCH_GETTER( COOKIES, folderPath )
		PROPERTY_SWITCH_GETTER( DESKTOP, folderPath )
		PROPERTY_SWITCH_GETTER( DESKTOPDIRECTORY, folderPath )
		PROPERTY_SWITCH_GETTER( DRIVES, folderPath )
		PROPERTY_SWITCH_GETTER( FAVORITES, folderPath )
		PROPERTY_SWITCH_GETTER( FONTS, folderPath )
		PROPERTY_SWITCH_GETTER( HISTORY, folderPath )
		PROPERTY_SWITCH_GETTER( INTERNET, folderPath )
		PROPERTY_SWITCH_GETTER( INTERNET_CACHE, folderPath )
		PROPERTY_SWITCH_GETTER( LOCAL_APPDATA, folderPath )
		PROPERTY_SWITCH_GETTER( MYDOCUMENTS, folderPath )
		PROPERTY_SWITCH_GETTER( MYMUSIC, folderPath )
		PROPERTY_SWITCH_GETTER( MYPICTURES, folderPath )
		PROPERTY_SWITCH_GETTER( MYVIDEO, folderPath )
		PROPERTY_SWITCH_GETTER( NETHOOD, folderPath )
		PROPERTY_SWITCH_GETTER( NETWORK, folderPath )
		PROPERTY_SWITCH_GETTER( PERSONAL, folderPath )
		PROPERTY_SWITCH_GETTER( PRINTERS, folderPath )
		PROPERTY_SWITCH_GETTER( PRINTHOOD, folderPath )
		PROPERTY_SWITCH_GETTER( PROFILE, folderPath )
		PROPERTY_SWITCH_GETTER( PROGRAM_FILES, folderPath )
		PROPERTY_SWITCH_GETTER( PROGRAM_FILESX86, folderPath )
		PROPERTY_SWITCH_GETTER( PROGRAM_FILES_COMMON, folderPath )
		PROPERTY_SWITCH_GETTER( PROGRAM_FILES_COMMONX86, folderPath )
		PROPERTY_SWITCH_GETTER( PROGRAMS, folderPath )
		PROPERTY_SWITCH_GETTER( RECENT, folderPath )
		PROPERTY_SWITCH_GETTER( RESOURCES, folderPath )
		PROPERTY_SWITCH_GETTER( RESOURCES_LOCALIZED, folderPath )
		PROPERTY_SWITCH_GETTER( SENDTO, folderPath )
		PROPERTY_SWITCH_GETTER( STARTMENU, folderPath )
		PROPERTY_SWITCH_GETTER( STARTUP, folderPath )
		PROPERTY_SWITCH_GETTER( SYSTEM, folderPath )
		PROPERTY_SWITCH_GETTER( SYSTEMX86, folderPath )
		PROPERTY_SWITCH_GETTER( TEMPLATES, folderPath )
		PROPERTY_SWITCH_GETTER( WINDOWS, folderPath )
		
	END_STATIC_PROPERTY_SPEC

	BEGIN_CONST
        CONST_INTEGER(MB_ABORTRETRYIGNORE            ,MB_ABORTRETRYIGNORE)
        CONST_INTEGER(MB_CANCELTRYCONTINUE           ,MB_CANCELTRYCONTINUE)
        CONST_INTEGER(MB_HELP                        ,MB_HELP)
        CONST_INTEGER(MB_OK                          ,MB_OK)
        CONST_INTEGER(MB_OKCANCEL                    ,MB_OKCANCEL)
        CONST_INTEGER(MB_RETRYCANCEL                 ,MB_RETRYCANCEL)
        CONST_INTEGER(MB_YESNO                       ,MB_YESNO)
        CONST_INTEGER(MB_YESNOCANCEL                 ,MB_YESNOCANCEL)
        CONST_INTEGER(MB_ICONEXCLAMATION             ,MB_ICONEXCLAMATION)
        CONST_INTEGER(MB_ICONWARNING                 ,MB_ICONWARNING)
        CONST_INTEGER(MB_ICONINFORMATION             ,MB_ICONINFORMATION)
        CONST_INTEGER(MB_ICONASTERISK                ,MB_ICONASTERISK)
        CONST_INTEGER(MB_ICONQUESTION                ,MB_ICONQUESTION)
        CONST_INTEGER(MB_ICONSTOP                    ,MB_ICONSTOP)
        CONST_INTEGER(MB_ICONERROR                   ,MB_ICONERROR)
        CONST_INTEGER(MB_ICONHAND                    ,MB_ICONHAND)
        CONST_INTEGER(MB_DEFBUTTON1                  ,MB_DEFBUTTON1)
        CONST_INTEGER(MB_DEFBUTTON2                  ,MB_DEFBUTTON2)
        CONST_INTEGER(MB_DEFBUTTON3                  ,MB_DEFBUTTON3)
        CONST_INTEGER(MB_DEFBUTTON4                  ,MB_DEFBUTTON4)
        CONST_INTEGER(MB_APPLMODAL                   ,MB_APPLMODAL)
        CONST_INTEGER(MB_SYSTEMMODAL                 ,MB_SYSTEMMODAL)
        CONST_INTEGER(MB_TASKMODAL                   ,MB_TASKMODAL)
        CONST_INTEGER(MB_DEFAULT_DESKTOP_ONLY        ,MB_DEFAULT_DESKTOP_ONLY)
        CONST_INTEGER(MB_RIGHT                       ,MB_RIGHT)
        CONST_INTEGER(MB_RTLREADING                  ,MB_RTLREADING)
        CONST_INTEGER(MB_SETFOREGROUND               ,MB_SETFOREGROUND)
        CONST_INTEGER(MB_TOPMOST                     ,MB_TOPMOST)
        //CONST_INTEGER(MB_SERVICE_NOTIFICATION        ,MB_SERVICE_NOTIFICATION)
        //CONST_INTEGER(MB_SERVICE_NOTIFICATION_NT3X   ,MB_SERVICE_NOTIFICATION_NT3X)
		
		CONST_INTEGER(IDABORT     ,IDABORT    )
		CONST_INTEGER(IDCANCEL	  ,IDCANCEL	  )
		CONST_INTEGER(IDCONTINUE  ,IDCONTINUE )
		CONST_INTEGER(IDIGNORE	  ,IDIGNORE	  )
		CONST_INTEGER(IDNO		  ,IDNO		  )
		CONST_INTEGER(IDOK		  ,IDOK		  )
		CONST_INTEGER(IDRETRY	  ,IDRETRY	  )
		CONST_INTEGER(IDTRYAGAIN  ,IDTRYAGAIN )
		CONST_INTEGER(IDYES		  ,IDYES	  )
	END_CONST

END_STATIC
