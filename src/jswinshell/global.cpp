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

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_MIN(1);

	UINT iconIndex = 0;
	if ( argc >= 2 )
		JL_CHK( jl::getValue(cx, JL_ARG(2), &iconIndex) );
	HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
	if ( hInst == NULL )
		return jl::throwOSError(cx);
	
	HICON hIcon;

	{

		jl::StrData fileName(cx);
		JL_CHK( jl::getValue(cx, JL_ARG(1), &fileName) );
		hIcon = ExtractIcon( hInst, fileName, iconIndex ); // see SHGetFileInfo(
		if ( hIcon == NULL ) {

	//		if ( GetLastError() != 0 )
	//			return WinThrowError(cx, GetLastError());
			JL_RVAL.setUndefined();
			return true;
		}
	
	}


	{

		JS::RootedObject icon(cx, jl::newObjectWithGivenProto(cx, JL_CLASS(Icon), JL_CLASS_PROTOTYPE(cx, Icon)));
		HICON *phIcon = (HICON*)jl_malloc(sizeof(HICON)); // this is needed because JL_SetPrivate stores ONLY alligned values
		JL_ASSERT_ALLOC( phIcon );
		*phIcon = hIcon;
		JL_SetPrivate(icon, phIcon);
		JL_RVAL.setObject(*icon);

	}

	return true;
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

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_MIN(1);

	UINT type = 0;
	if ( argc >= 3 )
		JL_CHK( jl::getValue(cx, JL_ARG(3), &type) );

	{

		jl::StrData caption(cx);
		jl::StrData text(cx);

		if ( argc >= 2 && !JL_ARG(2).isUndefined() )
			JL_CHK( jl::getValue(cx, JL_ARG(2), &caption) );

		JL_CHK( jl::getValue(cx, JL_ARG(1), &text) );

		int res = MessageBox(NULL, text, caption, type);
		if ( res == 0 )
			return jl::throwOSError(cx);
		JL_RVAL.setInt32(res);

	}

	return true;
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

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_MIN(1);

	{

		jl::StrData applicationName(cx);
		jl::StrData commandLine(cx);
		jl::StrData environment(cx);
		jl::StrData currentDirectory(cx);

		if ( JL_ARG_ISDEF(1) )
			JL_CHK( jl::getValue(cx, JL_ARG(1), &applicationName) ); // warning: GC on the returned buffer !

		if ( JL_ARG_ISDEF(2) )
			JL_CHK( jl::getValue(cx, JL_ARG(2), &commandLine) ); // warning: GC on the returned buffer !

		if ( JL_ARG_ISDEF(3) )
			JL_CHK( jl::getValue(cx, JL_ARG(3), &environment) ); // warning: GC on the returned buffer !

		if ( JL_ARG_ISDEF(4) )
			JL_CHK( jl::getValue(cx, JL_ARG(4), &currentDirectory) ); // warning: GC on the returned buffer !

		STARTUPINFO si;
		ZeroMemory(&si, sizeof(STARTUPINFO));
		si.cb = sizeof(STARTUPINFO);

		PROCESS_INFORMATION pi;

		// doc. commandLine parameter: The Unicode version of CreateProcess function, CreateProcessW, may modify the contents of this string !
		TCHAR *tmpCommandLine = commandLine.toOwnWStrZ(); // The Unicode version, CreateProcessW, can modify the contents of this string.
		DWORD creationFlags = sizeof( TCHAR ) == sizeof( WCHAR ) ? CREATE_UNICODE_ENVIRONMENT : 0;

		// doc: http://msdn2.microsoft.com/en-us/library/ms682425.aspx
		BOOL st = ::CreateProcess( applicationName, tmpCommandLine, NULL, NULL, FALSE, creationFlags, (LPVOID)environment.toWStrZ(), currentDirectory, &si, &pi );
		jl_free( tmpCommandLine );

		if ( st == FALSE )
			return WinThrowError(cx, GetLastError());

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	
	}

	JL_RVAL.setUndefined();
	return true;
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

	JL_DEFINE_ARGS;

	OPENFILENAME ofn = { sizeof(OPENFILENAME) };
	TCHAR fileName[PATH_MAX];
	TCHAR filter[255];

	if ( argc >= 1 && !JL_ARG(1).isUndefined() ) {

		jl::StrData str(cx);
		JL_CHK( jl::getValue(cx, JL_ARG(1), &str) );
		jl::strcpy( filter, str );
		for ( TCHAR *tmp = filter; (tmp = jl::strchr(tmp, '|')) != 0; tmp++ )
			*tmp = TEXT('\0'); // doc: Pointer to a buffer containing pairs of null-terminated filter strings.
		filter[str.length() + 1] = TEXT( '\0' ); // The last string in the buffer must be terminated by two NULL characters.
		ofn.lpstrFilter = filter;
	}

	if ( argc >= 2 && !JL_ARG(2).isUndefined() ) {

		jl::StrData tmp(cx);
		JL_CHK( jl::getValue(cx, JL_ARG(2), &tmp) );
		jl::strcpy( fileName, tmp );
	} else {

		*fileName = '\0';
	}

	ofn.lpstrFile = fileName;
	ofn.nMaxFile = COUNTOF(fileName);
	ofn.Flags = OFN_NOCHANGEDIR | OFN_LONGNAMES | OFN_HIDEREADONLY;
	BOOL res = GetOpenFileName(&ofn); // doc: http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winui/winui/windowsuserinterface/userinput/commondialogboxlibrary/commondialogboxreference/commondialogboxstructures/openfilename.asp

	if ( res == 0 ) {

		DWORD err = CommDlgExtendedError();
		JL_ERR( E_OS, E_OPERATION, E_ERRNO(err) );
	}

	JL_CHK( jl::setValue( cx, JL_RVAL, fileName ) );
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( sourceString )
  Expands environment-variable strings and replaces them with the values defined for the current user.
**/
DEFINE_FUNCTION( expandEnvironmentStrings ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_MIN(1);

	{

		jl::StrData src(cx);

		JL_CHK( jl::getValue(cx, JL_ARG(1), &src) );
		TCHAR dst[PATH_MAX];
		DWORD res = ExpandEnvironmentStrings( src, dst, COUNTOF(dst) );
		if ( res == 0 )
			return jl::throwOSError(cx);
		JL_CHK( jl::setValue( cx, JL_RVAL, jl::strSpec( dst, res ) ) );

	}
	
	return true;
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

	JL_DEFINE_ARGS;

	UINT type = (UINT)-1;
	if ( argc >= 1 )
		JL_CHK( jl::getValue(cx, JL_ARG(1), &type) );
	MessageBeep(type);

	JL_RVAL.setUndefined();
	return true;
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

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_MIN(2);

	unsigned int freq, duration;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &freq) );
	JL_CHK( jl::getValue(cx, JL_ARG(2), &duration) );
	Beep(freq, duration);

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( id )
  Creates a new COM object by object name or CLSID.
**/
DEFINE_FUNCTION( createComObject ) {

	JL_DEFINE_ARGS;

	IUnknown *punk = NULL;
	IDispatch *pdisp = NULL;

	HRESULT hr;

	JL_ASSERT_ARGC( 1 );
	JL_ASSERT_ARG_IS_STRING(1);

	//JSString *idStr = JS::ToString(cx, JL_ARG(1));
	//LPOLESTR name = (LPOLESTR)JS_GetStringCharsZ(cx, idStr);
	
	CLSID clsid;

	{

		jl::StrData name(cx);
		JL_CHK( jl::getValue(cx, JL_ARG(1), &name) );

		hr = name.getWCharAt(0) == L('{') ? CLSIDFromString(name, &clsid) : CLSIDFromProgID(name, &clsid);
		if ( FAILED(hr) )
			JL_CHK( WinThrowError(cx, hr) );
	
	}

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
	JL_CHK( NewComDispatch(cx, pdisp, args.rval()) );
	pdisp->Release();
	punk->Release();
	return true;

bad:
	if ( pdisp )
		pdisp->Release();
	if ( punk )
		punk->Release();
	return false;
}



HKEY
ParseRootKey(IN const TCHAR *path, OUT size_t *length) {

	if ( !jl::strncmp(path, TEXT("HKEY_CURRENT_USER"), 17) ) {
		*length = 17;
		return HKEY_CURRENT_USER;
	} else
		if ( !jl::strncmp( path, TEXT( "HKCU"), 4 ) ) {
		*length = 4;
		return HKEY_CURRENT_USER;
	} else
		if ( !jl::strncmp( path, TEXT( "HKEY_LOCAL_MACHINE"), 18 ) ) {
		*length = 18;
		return HKEY_LOCAL_MACHINE;
	} else
		if ( !jl::strncmp( path, TEXT( "HKLM"), 4 ) ) {
		*length = 4;
		return HKEY_LOCAL_MACHINE;
	} else
		if ( !jl::strncmp( path, TEXT( "HKEY_CLASSES_ROOT"), 17 ) ) {
		*length = 17;
		return HKEY_CLASSES_ROOT;
	} else
		if ( !jl::strncmp( path, TEXT( "HKCR"), 4 ) ) {
		*length = 4;
		return HKEY_CLASSES_ROOT;
	} else
		if ( !jl::strncmp( path, TEXT( "HKEY_CURRENT_CONFIG"), 19 ) ) {
		*length = 19;
		return HKEY_CURRENT_CONFIG;
	} else
		if ( !jl::strncmp( path, TEXT( "HKCC"), 4 ) ) {
		*length = 4;
		return HKEY_CURRENT_CONFIG;
	} else
		if ( !jl::strncmp( path, TEXT( "HKEY_USERS"), 10 ) ) {
		*length = 10;
		return HKEY_USERS;
	} else
		if ( !jl::strncmp( path, TEXT( "HKU"), 3 ) ) {
		*length = 3;
		return HKEY_USERS;
	} else
		if ( !jl::strncmp( path, TEXT( "HKEY_PERFORMANCE_DATA"), 21 ) ) {
		*length = 21;
		return HKEY_PERFORMANCE_DATA;
	} else
		if ( !jl::strncmp( path, TEXT( "HKPD"), 4 ) ) {
		*length = 4;
		return HKEY_PERFORMANCE_DATA;
	} else
		if ( !jl::strncmp( path, TEXT( "HKEY_DYN_DATA"), 13 ) ) {
		*length = 13;
		return HKEY_DYN_DATA;
	} else
		if ( !jl::strncmp( path, TEXT( "HKDD"), 4 ) ) {
		*length = 4;
		return HKEY_DYN_DATA;
	} else {

		return NULL;
	}
}

DEFINE_FUNCTION( registrySet ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC(3);

	{

		JS::RootedValue value(cx);
		jl::StrData subKeyStr(cx);
		const TCHAR *subKey;

		JL_CHK( jl::getValue(cx, JL_ARG(1), &subKeyStr) );
		subKey = subKeyStr;

		size_t length;
		HKEY rootHKey = ParseRootKey(subKey, &length);
		JL_ASSERT( subKey != NULL, E_ARG, E_NUM(1), E_INVALID, E_COMMENT("root key") );
		subKey += length;
		if ( subKey[0] == '\\' )
			subKey += 1;

		LONG st;

		value = JL_ARG(3);

		if ( JL_ARG(2).isUndefined() ) {

			if ( value.isUndefined() ) {

				st = ::RegDeleteKey(rootHKey, subKey);
				if ( st != ERROR_SUCCESS )
					return WinThrowError(cx, st);
			}
		} else {

			HKEY hKey;
			st = RegCreateKeyEx(rootHKey, subKey, 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL); // http://msdn.microsoft.com/en-us/library/windows/desktop/ms724844(v=vs.85).aspx
			if ( st != ERROR_SUCCESS )
				return WinThrowError(cx, st);
		
			jl::StrData valueNameStr(cx);

			JL_CHK( jl::getValue(cx, JL_ARG(2), &valueNameStr) );

			if ( value.isUndefined() ) {

				st = RegDeleteValue(hKey, valueNameStr);
			} else
			if ( value.isNull() ) {

				st = RegSetValueEx(hKey, valueNameStr, 0, REG_NONE, NULL, 0);
			} else
			if ( value.isInt32() ) {

				DWORD num;
				JL_CHK( jl::getValue(cx, value, &num) );
				st = RegSetValueEx(hKey, valueNameStr, 0, REG_DWORD, (LPBYTE)&num, sizeof(DWORD));
			} else
			if ( value.isDouble() ) {

				uint64_t num;
				JL_CHK( jl::getValue(cx, value, &num) );
				st = RegSetValueEx(hKey, valueNameStr, 0, REG_QWORD, (LPBYTE)&num, sizeof(uint64_t));
			} else
			if ( value.isString() ) {

				jl::StrData tmp(cx);
				JL_CHK( jl::getValue(cx, value, &tmp) );
				// doc: When writing a string to the registry, you must specify the length of the string, including the terminating null character (\0).
				st = RegSetValueEx(hKey, valueNameStr, 0, REG_SZ, reinterpret_cast<const BYTE *>(tmp.toStrZ()), tmp.length() + 1);
			} else
			if ( jl::isData(cx, value) ) {

				jl::BufString tmp;
				JL_CHK( jl::getValue(cx, value, &tmp) );
				st = RegSetValueEx(hKey, valueNameStr, 0, REG_BINARY, tmp.toBytes(), tmp.length());
			}

			if ( st != ERROR_SUCCESS )
				return WinThrowError(cx, st);

			st = RegCloseKey(hKey);
			if ( st != ERROR_SUCCESS )
				return WinThrowError(cx, st);
		}

	}

	JL_RVAL.setUndefined();

	return true;
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

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_RANGE(1,2);

	{

		jl::BufBase buffer;

		jl::StrData pathStr(cx);
		jl::StrData valueName(cx);

		const TCHAR *path;
		JL_CHK( jl::getValue(cx, JL_ARG(1), &pathStr) );
		path = pathStr;

		size_t length;
		HKEY rootHKey = ParseRootKey(path, &length);
		JL_ASSERT( rootHKey != NULL, E_ARG, E_NUM(1), E_INVALID, E_COMMENT("root key") );
		path += length;

		if ( path[0] == '\\' )
			path++;

		HKEY hKey;
		LONG st;

		st = ::RegOpenKeyEx(rootHKey, path, 0, KEY_READ, &hKey); // http://msdn.microsoft.com/en-us/library/ms724897%28VS.85%29.aspx
		if ( st != ERROR_SUCCESS )
			return WinThrowError(cx, st);

		if ( (argc == 1) || (argc >= 2 && JL_ARG(2).isUndefined()) ) {

			JS::RootedObject arrObj(cx, JS_NewArrayObject(cx, 0));
			JL_CHK( arrObj );
			JL_RVAL.setObject(*arrObj);

			TCHAR name[16384]; // http://msdn.microsoft.com/en-us/library/ms724872%28VS.85%29.aspx
			DWORD nameLength, index;
			index = 0;
			for (;;) {

				nameLength = COUNTOF(name);
				if ( argc == 1 ) // enum keys
					st = RegEnumKeyEx(hKey, index, name, &nameLength, NULL, NULL, NULL, NULL);
				else
					st = RegEnumValue(hKey, index, name, &nameLength, NULL, NULL, NULL, NULL); // doc. http://msdn.microsoft.com/en-us/library/ms724865(VS.85).aspx

				if ( st != ERROR_SUCCESS )
					break;
				JS::RootedValue strName(cx);
				JL_CHK( jl::setValue(cx, &strName, jl::strSpec(name, nameLength)) );
				JL_CHK( JL_SetElement(cx, arrObj, index, strName) );
				index++;
			}
			if ( st != ERROR_NO_MORE_ITEMS )
				return WinThrowError(cx, st);

			RegCloseKey(hKey);
			return true;
		}

		JL_CHK( jl::getValue(cx, JL_ARG(2), &valueName) );

		DWORD type, size;

		// doc. http://msdn.microsoft.com/en-us/library/ms724911(VS.85).aspx
		st = RegQueryValueEx(hKey, valueName, NULL, &type, NULL, &size);

		if ( st == ERROR_FILE_NOT_FOUND ) {

			JL_RVAL.setUndefined();
			RegCloseKey(hKey);
			return true;
		}

		if ( st != ERROR_SUCCESS ) {

			RegCloseKey(hKey);
			return WinThrowError(cx, st);
		}

		//buffer = JL_DataBufferAlloc(cx, size);
		buffer.alloc(size, true);
		JL_ASSERT_ALLOC( buffer );

		st = RegQueryValueEx(hKey, valueName, NULL, NULL, buffer.data(), &size);

		// doc. http://msdn.microsoft.com/en-us/library/ms724884(VS.85).aspx
		switch (type) {
			case REG_NONE:
				JL_RVAL.setNull();
				//JL_DataBufferFree(cx, buffer);
				break;
			case REG_BINARY:
				//JL_CHK( JL_NewBufferGetOwnership(cx, buffer, size, JL_RVAL) );
				JL_CHK( BlobCreate(cx, buffer, JL_RVAL) );
				break;
			case REG_DWORD:
				JL_CHK( jl::setValue(cx, JL_RVAL, *(DWORD*)buffer.data()) );
				//JL_DataBufferFree(cx, buffer);
				break;
			case REG_QWORD:
				JL_CHK( jl::setValue(cx, JL_RVAL, (double)*(DWORD64*)buffer.data()) );
				break;
			case REG_LINK: {
				//JSString *jsstr = JL_NewUCString(cx, (jschar*)buffer, size/2);
				//JL_CHK( jsstr );
				//JL_RVAL.setString(jsstr);
				//JL_CHK( buffer.toExternalStringUC(cx, JL_RVAL) );

				jl::BufString(buffer).setCharSize(2).toString(cx, JL_RVAL);
				break;
			}
			case REG_EXPAND_SZ:
			case REG_MULTI_SZ:
			case REG_SZ: {
				//JSString *jsstr = JL_NewString(cx, (char*)buffer, size-1); // note: ((char*)buffer)[size] already == '\0'
				//JL_CHK( JLData((char*)buffer, true, size-1).GetJSString(cx, JL_RVAL) );
				buffer.setUsed(size-1);
				//JL_CHK( buffer.toExternalStringUC(cx, JL_RVAL) );
				jl::BufString(buffer).setCharSize(1).toString(cx, JL_RVAL);
				break;
			}
		}

		RegCloseKey(hKey);
		
	}

	return true;
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
  var dch = directoryChangesInit('C:\\WINDOWS', 0x10|0x40, true);
  while (!host.endSignal) {

    print( uneval( directoryChangesLookup(dch) ), '\n');
    sleep(1000);
  }
  }}}
**/
// (TBD) Linux version using inotify: http://en.wikipedia.org/wiki/Inotify / try: man inotify
struct DirectoryChanges : public HandlePrivate {
	JL_HANDLE_TYPE typeId() const {

		return JLHID(dmon);
	}
	HANDLE hDirectory;
	OVERLAPPED overlapped;
	BYTE buffer[2][2048];
	int currentBuffer;
	BOOL watchSubtree;
	DWORD notifyFilter;
	~DirectoryChanges() {

		CloseHandle(hDirectory);
	}
};


DEFINE_FUNCTION( directoryChangesInit ) {
	

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_RANGE(2,3);


	unsigned int notifyFilter;
	JL_CHK( jl::getValue(cx, JL_ARG(2), &notifyFilter) );

	bool watchSubtree;
	if ( JL_ARG_ISDEF(3) )
		JL_CHK( jl::getValue(cx, JL_ARG(3), &watchSubtree) );
	else
		watchSubtree = false;

	DirectoryChanges *dc = new DirectoryChanges();
	JL_ASSERT_ALLOC(dc);
	JL_CHK( HandleCreate(cx, dc, JL_RVAL) );

	dc->watchSubtree = watchSubtree;
	dc->notifyFilter = notifyFilter;
	
	{

		jl::StrData pathName(cx);
		JL_CHK( jl::getValue(cx, JL_ARG(1), &pathName) );

		dc->hDirectory = CreateFile(pathName, FILE_LIST_DIRECTORY, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);
		if ( dc->hDirectory == INVALID_HANDLE_VALUE )
			return WinThrowError(cx, GetLastError());
	
	}

	dc->currentBuffer = 0;
	dc->overlapped.hEvent = NULL;

	// http://msdn.microsoft.com/en-us/library/windows/desktop/aa365465(v=vs.85).aspx
	if ( !ReadDirectoryChangesW(dc->hDirectory, &dc->buffer[dc->currentBuffer], sizeof(*dc->buffer), dc->watchSubtree, dc->notifyFilter, NULL, &dc->overlapped, NULL) )
		return WinThrowError(cx, GetLastError());

	return true;
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

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_RANGE(1,2);
	JL_ASSERT_ARG_TYPE( IsHandleType(cx, JL_ARG(1), JLHID(dmon)), 1, "(dmon) Handle" );

	DirectoryChanges *dc;
	JL_CHK( GetHandlePrivate(cx, JL_ARG(1), dc) );
	JL_ASSERT( dc, E_ARG, E_NUM(1), E_STATE, E_INVALID );

	bool wait;
	if ( JL_ARG_ISDEF(2) )
		JL_CHK( jl::getValue(cx, JL_ARG(2), &wait) );
	else
		wait = false;

	if ( !wait ) {

		DWORD res = WaitForSingleObject(dc->hDirectory, 0);
		if ( res == -1 )
			return WinThrowError(cx,  GetLastError());

		if ( res != WAIT_OBJECT_0 ) { // non signaled

			JSObject *arrObj = JS_NewArrayObject(cx, 0);
			JL_CHK( arrObj );
			JL_RVAL.setObject(*arrObj);
			return true;
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

	{

		JS::RootedObject arrObj(cx, JS_NewArrayObject(cx, 0));
		JL_CHK( arrObj );
		JL_RVAL.setObject(*arrObj);

		int index = 0;
		// see http://www.google.fr/codesearch/p?hl=fr&sa=N&cd=17&ct=rc#8WOCRDPt-u8/trunk/src/FileWatch.cc&q=ReadDirectoryChangesW
		while ( pFileNotify ) {

			JS::RootedValue eltVal(cx);
			// pFileNotify->FileNameLength is the size of the file name portion of the record, in bytes. Note that this value does not include the terminating null character.
			eltVal.setObjectOrNull( jl::newArray( cx, jl::strSpec( pFileNotify->FileName, pFileNotify->FileNameLength / sizeof( *pFileNotify->FileName ) ), pFileNotify->Action ) );
			JL_CHK( !eltVal.isNull() );
			JL_CHK( JL_SetElement(cx, arrObj, index, eltVal) );
			index++;

			if ( pFileNotify->NextEntryOffset )
				pFileNotify = (FILE_NOTIFY_INFORMATION*) ((PBYTE)pFileNotify + pFileNotify->NextEntryOffset) ;
			else
				pFileNotify = NULL;
		}

	}

	return true;
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

struct DirectoryUserProcessEvent : public ProcessEvent2 {

	DirectoryChanges *dc;
	HANDLE cancelEvent;

	bool prepareWait(JSContext *cx, JS::HandleObject obj) {

		return true;
	}

	void startWait() {

		const HANDLE events[] = { cancelEvent, dc->hDirectory };
		DWORD status = WaitForMultipleObjects(COUNTOF(events), events, FALSE, INFINITE);
		ASSERT( status != WAIT_FAILED );
	}

	bool cancelWait() {

		BOOL status = SetEvent(cancelEvent);
		ASSERT( status );

		return true;
	}

	bool endWait(bool *hasEvent, JSContext *cx, JS::HandleObject obj) {

		DWORD st = WaitForSingleObject(dc->hDirectory, 0);
		*hasEvent = (st == WAIT_OBJECT_0);

		if ( !*hasEvent )
			return true;

		JS::RootedValue fct(cx, getSlot(2));
		if ( !fct.isUndefined() ) {
			
			JS::RootedValue calleeThis(cx, getSlot(0));
			JS::RootedValue dmon(cx, getSlot(1));
			JL_CHK( jl::callNoRval( cx, calleeThis, fct, dmon ) );
		}
		return true;
		JL_BAD;
	}

	~DirectoryUserProcessEvent() {

		BOOL status = CloseHandle(cancelEvent);
		ASSERT( status );
	}
};



DEFINE_FUNCTION( directoryChangesEvents ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_RANGE(1, 2);
	JL_ASSERT_ARG_TYPE( IsHandleType(cx, JL_ARG(1), JLHID(dmon)), 1, "(dmon) Handle" );

	DirectoryChanges *dc;
	JL_CHK( GetHandlePrivate(cx, JL_ARG(1), dc) );
	JL_ASSERT( dc, E_ARG, E_NUM(1), E_STATE, E_INVALID );

	DirectoryUserProcessEvent *upe = new DirectoryUserProcessEvent();
	JL_ASSERT_ALLOC(upe);
	JL_CHK( HandleCreate(cx, upe, JL_RVAL) );

	if ( JL_ARG_ISDEF(2) ) {

		JL_ASSERT_ARG_IS_CALLABLE(2);
		upe->setSlot(0, JL_OBJVAL); // store "this" object.
		upe->setSlot(1, JL_ARG(1)); // dmon handle (argument 1 of the callback function)
		upe->setSlot(2, JL_ARG(2)); // onChange function
	}

	upe->dc = dc;
	upe->cancelEvent = CreateEvent(NULL, FALSE, FALSE, NULL); // auto-reset

//	if ( upe->cancelEvent == NULL )
//		jl::throwOSError(cx);

	return true;
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

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC(1);

	{

		jl::StrData str(cx);
		JL_CHK( jl::getValue(cx, JL_ARG(1), &str) );

		JL_ASSERT( str.length() == sizeof(GUID), E_ARG, E_NUM(1), E_LENGTH, E_NUM(sizeof(GUID)) );

		GUID guid;
		CopyMemory(&guid, str.toStr(), sizeof(GUID));
		WCHAR szGuid[39];
		int len = StringFromGUID2(guid, szGuid, COUNTOF(szGuid));
		ASSERT( len == COUNTOF(szGuid) );
		ASSERT( szGuid[COUNTOF(szGuid)-1] == 0 );

		JL_CHK( jl::setValue(cx, JL_RVAL, jl::strSpec(szGuid, COUNTOF(szGuid)-1)) );

	}

	return true;
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
		return jl::throwOSError(cx);

	if ( IsClipboardFormatAvailable(CF_TEXT) == 0 ) {

		vp.setNull();
	} else {

		HANDLE hglb = GetClipboardData(CF_TEXT);
		if ( !hglb )
			return jl::throwOSError(cx);

		LPTSTR lptstr = (LPTSTR)GlobalLock(hglb);
		if ( lptstr == NULL )
			return jl::throwOSError(cx);
		JL_CHK( jl::setValue( cx, vp, lptstr ) );
		GlobalUnlock(hglb);
		CloseClipboard();
	}
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( clipboard ) {

	JL_IGNORE( strict, id, obj );

	BOOL res = OpenClipboard(NULL);
	if ( res == 0 )
		return jl::throwOSError(cx);
	res = EmptyClipboard(); // doc: If the application specifies a NULL window handle when opening the clipboard, EmptyClipboard succeeds but sets the clipboard owner to NULL. Note that this causes SetClipboardData to fail.
	if ( res == 0 )
		return jl::throwOSError(cx);
	res = CloseClipboard();
	if ( res == 0 )
		return jl::throwOSError(cx);


	if ( !vp.isNullOrUndefined() ) {

		jl::StrData str(cx);

		res = OpenClipboard(NULL);
		if ( res == 0 )
			return jl::throwOSError(cx);
		JL_CHK( jl::getValue(cx, vp, &str) );
		HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, str.length() + 1);
		JL_ASSERT_ALLOC( hglbCopy );
		LPTSTR lptstrCopy = (LPTSTR)GlobalLock(hglbCopy);
		if ( lptstrCopy == NULL )
			return jl::throwOSError(cx);
		jl::memcpy(lptstrCopy, str.toStr(), str.length() + 1);
		lptstrCopy[str.length()] = 0;
		GlobalUnlock(hglbCopy);
		HANDLE h = SetClipboardData(CF_TEXT, hglbCopy);
		if ( h == NULL )
			return jl::throwOSError(cx);
		CloseClipboard();
	}
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME
  current Windows ANSI code page identifier for the operating system.
**/
DEFINE_PROPERTY_GETTER( systemCodepage ) {

	JL_IGNORE( id, obj, cx );

	vp.setInt32(GetACP());
	return true;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME
  current Windows ANSI code page identifier for the console.
**/
DEFINE_PROPERTY_GETTER( consoleCodepage ) {

	JL_IGNORE( id, obj, cx );

	vp.setInt32(GetOEMCP());
	return true;
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

	JL_DEFINE_PROP_ARGS;

	return jl::setValue(cx, JL_RVAL, GetKeyState(VK_NUMLOCK) & 1);
}

DEFINE_PROPERTY_SETTER( numlockState ) {

	JL_DEFINE_PROP_ARGS;

	bool state;
	JL_CHK( jl::getValue(cx, JL_RVAL, &state) );
	SetKeyState(VK_NUMLOCK, state);
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME
  Gets or sets the capslock key state.
**/
DEFINE_PROPERTY_GETTER( capslockState ) {

	JL_DEFINE_PROP_ARGS;
	return jl::setValue(cx, JL_RVAL, GetKeyState(VK_CAPITAL) & 1);
}

DEFINE_PROPERTY_SETTER( capslockState ) {

	JL_DEFINE_PROP_ARGS;
	bool state;
	JL_CHK( jl::getValue(cx, JL_RVAL, &state) );
	SetKeyState(VK_CAPITAL, state);
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME
  Gets or sets the scrolllock key state.
**/
DEFINE_PROPERTY_GETTER( scrolllockState ) {

	JL_DEFINE_PROP_ARGS;

	return jl::setValue(cx, JL_RVAL, GetKeyState(VK_SCROLL) & 1);
}

DEFINE_PROPERTY_SETTER( scrolllockState ) {

	JL_DEFINE_PROP_ARGS;

	bool state;
	JL_CHK( jl::getValue(cx, vp, &state) );
	SetKeyState(VK_SCROLL, state);
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME
  is the elapsed time since the last input event (in milliseconds).
**/
DEFINE_PROPERTY_GETTER( lastInputTime ) {

	JL_DEFINE_PROP_ARGS;

	LASTINPUTINFO lastInputInfo = {0};
	lastInputInfo.cbSize = sizeof(LASTINPUTINFO);
	if ( ::GetLastInputInfo(&lastInputInfo) == FALSE )
		return WinThrowError(cx, GetLastError());
	return jl::setValue(cx, JL_RVAL, ::GetTickCount() - lastInputInfo.dwTime);
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


DEFINE_PROPERTY_GETTER( folderPath ) {

	JL_DEFINE_PROP_ARGS;

	TCHAR path[PATH_MAX];
	if ( SUCCEEDED( SHGetFolderPath(NULL, JSID_TO_INT(id), NULL, 0, path) ) ) // |CSIDL_FLAG_CREATE
		return jl::setValue(cx, JL_RVAL, path);
	vp.setUndefined();
	return true;
}


DEFINE_PROPERTY_GETTER_SWITCH(ADMINTOOLS               , folderPath, CSIDL_ADMINTOOLS);
DEFINE_PROPERTY_GETTER_SWITCH(ALTSTARTUP               , folderPath, CSIDL_ALTSTARTUP);
DEFINE_PROPERTY_GETTER_SWITCH(APPDATA                  , folderPath, CSIDL_APPDATA);
DEFINE_PROPERTY_GETTER_SWITCH(BITBUCKET                , folderPath, CSIDL_BITBUCKET);
DEFINE_PROPERTY_GETTER_SWITCH(CDBURN_AREA              , folderPath, CSIDL_CDBURN_AREA);
DEFINE_PROPERTY_GETTER_SWITCH(COMMON_ADMINTOOLS        , folderPath, CSIDL_COMMON_ADMINTOOLS);
DEFINE_PROPERTY_GETTER_SWITCH(COMMON_ALTSTARTUP        , folderPath, CSIDL_COMMON_ALTSTARTUP);
DEFINE_PROPERTY_GETTER_SWITCH(COMMON_APPDATA           , folderPath, CSIDL_COMMON_APPDATA);
DEFINE_PROPERTY_GETTER_SWITCH(COMMON_DESKTOPDIRECTORY  , folderPath, CSIDL_COMMON_DESKTOPDIRECTORY);
DEFINE_PROPERTY_GETTER_SWITCH(COMMON_DOCUMENTS         , folderPath, CSIDL_COMMON_DOCUMENTS);
DEFINE_PROPERTY_GETTER_SWITCH(COMMON_FAVORITES         , folderPath, CSIDL_COMMON_FAVORITES);
DEFINE_PROPERTY_GETTER_SWITCH(COMMON_MUSIC             , folderPath, CSIDL_COMMON_MUSIC);
DEFINE_PROPERTY_GETTER_SWITCH(COMMON_OEM_LINKS         , folderPath, CSIDL_COMMON_OEM_LINKS);
DEFINE_PROPERTY_GETTER_SWITCH(COMMON_PICTURES          , folderPath, CSIDL_COMMON_PICTURES);
DEFINE_PROPERTY_GETTER_SWITCH(COMMON_PROGRAMS          , folderPath, CSIDL_COMMON_PROGRAMS);
DEFINE_PROPERTY_GETTER_SWITCH(COMMON_STARTMENU         , folderPath, CSIDL_COMMON_STARTMENU);
DEFINE_PROPERTY_GETTER_SWITCH(COMMON_STARTUP           , folderPath, CSIDL_COMMON_STARTUP);
DEFINE_PROPERTY_GETTER_SWITCH(COMMON_TEMPLATES         , folderPath, CSIDL_COMMON_TEMPLATES);
DEFINE_PROPERTY_GETTER_SWITCH(COMMON_VIDEO             , folderPath, CSIDL_COMMON_VIDEO);
DEFINE_PROPERTY_GETTER_SWITCH(COMPUTERSNEARME          , folderPath, CSIDL_COMPUTERSNEARME);
DEFINE_PROPERTY_GETTER_SWITCH(CONNECTIONS              , folderPath, CSIDL_CONNECTIONS);
DEFINE_PROPERTY_GETTER_SWITCH(CONTROLS                 , folderPath, CSIDL_CONTROLS);
DEFINE_PROPERTY_GETTER_SWITCH(COOKIES                  , folderPath, CSIDL_COOKIES);
DEFINE_PROPERTY_GETTER_SWITCH(DESKTOP                  , folderPath, CSIDL_DESKTOP);
DEFINE_PROPERTY_GETTER_SWITCH(DESKTOPDIRECTORY         , folderPath, CSIDL_DESKTOPDIRECTORY);
DEFINE_PROPERTY_GETTER_SWITCH(DRIVES                   , folderPath, CSIDL_DRIVES);
DEFINE_PROPERTY_GETTER_SWITCH(FAVORITES                , folderPath, CSIDL_FAVORITES);
DEFINE_PROPERTY_GETTER_SWITCH(FONTS                    , folderPath, CSIDL_FONTS);
DEFINE_PROPERTY_GETTER_SWITCH(HISTORY                  , folderPath, CSIDL_HISTORY);
DEFINE_PROPERTY_GETTER_SWITCH(INTERNET                 , folderPath, CSIDL_INTERNET);
DEFINE_PROPERTY_GETTER_SWITCH(INTERNET_CACHE           , folderPath, CSIDL_INTERNET_CACHE);
DEFINE_PROPERTY_GETTER_SWITCH(LOCAL_APPDATA            , folderPath, CSIDL_LOCAL_APPDATA);
DEFINE_PROPERTY_GETTER_SWITCH(MYDOCUMENTS              , folderPath, CSIDL_MYDOCUMENTS);
DEFINE_PROPERTY_GETTER_SWITCH(MYMUSIC                  , folderPath, CSIDL_MYMUSIC);
DEFINE_PROPERTY_GETTER_SWITCH(MYPICTURES               , folderPath, CSIDL_MYPICTURES);
DEFINE_PROPERTY_GETTER_SWITCH(MYVIDEO                  , folderPath, CSIDL_MYVIDEO);
DEFINE_PROPERTY_GETTER_SWITCH(NETHOOD                  , folderPath, CSIDL_NETHOOD);
DEFINE_PROPERTY_GETTER_SWITCH(NETWORK                  , folderPath, CSIDL_NETWORK);
DEFINE_PROPERTY_GETTER_SWITCH(PERSONAL                 , folderPath, CSIDL_PERSONAL);
DEFINE_PROPERTY_GETTER_SWITCH(PRINTERS                 , folderPath, CSIDL_PRINTERS);
DEFINE_PROPERTY_GETTER_SWITCH(PRINTHOOD                , folderPath, CSIDL_PRINTHOOD);
DEFINE_PROPERTY_GETTER_SWITCH(PROFILE                  , folderPath, CSIDL_PROFILE);
DEFINE_PROPERTY_GETTER_SWITCH(PROGRAM_FILES            , folderPath, CSIDL_PROGRAM_FILES);
DEFINE_PROPERTY_GETTER_SWITCH(PROGRAM_FILESX86         , folderPath, CSIDL_PROGRAM_FILESX86);
DEFINE_PROPERTY_GETTER_SWITCH(PROGRAM_FILES_COMMON     , folderPath, CSIDL_PROGRAM_FILES_COMMON);
DEFINE_PROPERTY_GETTER_SWITCH(PROGRAM_FILES_COMMONX86  , folderPath, CSIDL_PROGRAM_FILES_COMMONX86);
DEFINE_PROPERTY_GETTER_SWITCH(PROGRAMS                 , folderPath, CSIDL_PROGRAMS);
DEFINE_PROPERTY_GETTER_SWITCH(RECENT                   , folderPath, CSIDL_RECENT);
DEFINE_PROPERTY_GETTER_SWITCH(RESOURCES                , folderPath, CSIDL_RESOURCES);
DEFINE_PROPERTY_GETTER_SWITCH(RESOURCES_LOCALIZED      , folderPath, CSIDL_RESOURCES_LOCALIZED);
DEFINE_PROPERTY_GETTER_SWITCH(SENDTO                   , folderPath, CSIDL_SENDTO);
DEFINE_PROPERTY_GETTER_SWITCH(STARTMENU                , folderPath, CSIDL_STARTMENU);
DEFINE_PROPERTY_GETTER_SWITCH(STARTUP                  , folderPath, CSIDL_STARTUP);
DEFINE_PROPERTY_GETTER_SWITCH(SYSTEM                   , folderPath, CSIDL_SYSTEM);
DEFINE_PROPERTY_GETTER_SWITCH(SYSTEMX86                , folderPath, CSIDL_SYSTEMX86);
DEFINE_PROPERTY_GETTER_SWITCH(TEMPLATES                , folderPath, CSIDL_TEMPLATES);
DEFINE_PROPERTY_GETTER_SWITCH(WINDOWS                  , folderPath, CSIDL_WINDOWS);




#ifdef DEBUG

#include <Mmsystem.h>

void CALLBACK waveOutProc( HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2 ) {
	

}


DEFINE_FUNCTION( jswinshelltest ) {

	JL_DEFINE_ARGS;
	JL_RVAL.setUndefined();

	MMRESULT res;

	WAVEFORMATEX wfx;
	wfx.nSamplesPerSec = 44100;
	wfx.wBitsPerSample = 16;
	wfx.nChannels = 2;


	wfx.cbSize = 0;
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nBlockAlign = (wfx.wBitsPerSample * wfx.nChannels) / 8;
	wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;


//	res = waveOutOpen(&hwo, WAVE_MAPPER, &wfx, NULL, NULL, CALLBACK_NULL);


	UINT uDeviceID;
	MMRESULT mmResult;
	UINT uNumDevs = waveInGetNumDevs();
	for (uDeviceID = 0; uDeviceID < uNumDevs; uDeviceID++) {
		
		// Take a look at the driver capabilities.
		WAVEINCAPS wic;
		mmResult = waveInGetDevCaps(uDeviceID, &wic, sizeof(wic));

		if ( jl::strcmp(wic.szPname, TEXT("Rec. Playback (IDT High Definit")) == 0 )
			break;
		ASSERT( mmResult == MMSYSERR_NOERROR );
	}

	HWAVEIN hwi;
	res = waveInOpen(&hwi, uDeviceID, &wfx, NULL, NULL, WAVE_FORMAT_DIRECT);

	const int NUMPTS = 44100 * 2 * 10;   // 3 seconds
    short int waveIn[NUMPTS];

	WAVEHDR waveHeader;

	// Set up and prepare header for input
    waveHeader.lpData = (LPSTR)waveIn;
    waveHeader.dwBufferLength = NUMPTS*2;
    waveHeader.dwBytesRecorded=0;
    waveHeader.dwUser = 0L;
    waveHeader.dwFlags = 0L;
    waveHeader.dwLoops = 0L;
    waveInPrepareHeader(hwi, &waveHeader, sizeof(WAVEHDR));

	res = waveInAddBuffer(hwi, &waveHeader, sizeof(WAVEHDR));

	res = waveInStart(hwi);

    do {

		waveHeader.dwBytesRecorded = 0;
	
	} while ((res = waveInUnprepareHeader(hwi, &waveHeader, sizeof(WAVEHDR))) == WAVERR_STILLPLAYING);


	waveInClose(hwi);




//	ASSERT( res == MMSYSERR_NOERROR );

	return true;
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

		PROPERTY_GETTER( ADMINTOOLS )
		PROPERTY_GETTER( ALTSTARTUP )
		PROPERTY_GETTER( APPDATA )
		PROPERTY_GETTER( BITBUCKET )
		PROPERTY_GETTER( CDBURN_AREA )
		PROPERTY_GETTER( COMMON_ADMINTOOLS )
		PROPERTY_GETTER( COMMON_ALTSTARTUP )
		PROPERTY_GETTER( COMMON_APPDATA )
		PROPERTY_GETTER( COMMON_DESKTOPDIRECTORY )
		PROPERTY_GETTER( COMMON_DOCUMENTS )
		PROPERTY_GETTER( COMMON_FAVORITES )
		PROPERTY_GETTER( COMMON_MUSIC )
		PROPERTY_GETTER( COMMON_OEM_LINKS )
		PROPERTY_GETTER( COMMON_PICTURES )
		PROPERTY_GETTER( COMMON_PROGRAMS )
		PROPERTY_GETTER( COMMON_STARTMENU )
		PROPERTY_GETTER( COMMON_STARTUP )
		PROPERTY_GETTER( COMMON_TEMPLATES )
		PROPERTY_GETTER( COMMON_VIDEO )
		PROPERTY_GETTER( COMPUTERSNEARME )
		PROPERTY_GETTER( CONNECTIONS )
		PROPERTY_GETTER( CONTROLS )
		PROPERTY_GETTER( COOKIES )
		PROPERTY_GETTER( DESKTOP )
		PROPERTY_GETTER( DESKTOPDIRECTORY )
		PROPERTY_GETTER( DRIVES )
		PROPERTY_GETTER( FAVORITES )
		PROPERTY_GETTER( FONTS )
		PROPERTY_GETTER( HISTORY )
		PROPERTY_GETTER( INTERNET )
		PROPERTY_GETTER( INTERNET_CACHE )
		PROPERTY_GETTER( LOCAL_APPDATA )
		PROPERTY_GETTER( MYDOCUMENTS )
		PROPERTY_GETTER( MYMUSIC )
		PROPERTY_GETTER( MYPICTURES )
		PROPERTY_GETTER( MYVIDEO )
		PROPERTY_GETTER( NETHOOD )
		PROPERTY_GETTER( NETWORK )
		PROPERTY_GETTER( PERSONAL )
		PROPERTY_GETTER( PRINTERS )
		PROPERTY_GETTER( PRINTHOOD )
		PROPERTY_GETTER( PROFILE )
		PROPERTY_GETTER( PROGRAM_FILES )
		PROPERTY_GETTER( PROGRAM_FILESX86 )
		PROPERTY_GETTER( PROGRAM_FILES_COMMON )
		PROPERTY_GETTER( PROGRAM_FILES_COMMONX86 )
		PROPERTY_GETTER( PROGRAMS )
		PROPERTY_GETTER( RECENT )
		PROPERTY_GETTER( RESOURCES )
		PROPERTY_GETTER( RESOURCES_LOCALIZED )
		PROPERTY_GETTER( SENDTO )
		PROPERTY_GETTER( STARTMENU )
		PROPERTY_GETTER( STARTUP )
		PROPERTY_GETTER( SYSTEM )
		PROPERTY_GETTER( SYSTEMX86 )
		PROPERTY_GETTER( TEMPLATES )
		PROPERTY_GETTER( WINDOWS )

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
