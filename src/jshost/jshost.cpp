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

// to be used in the main() function only
#define HOST_MAIN_ASSERT( condition, errorMessage ) if ( !(condition) ) { fprintf(stderr, errorMessage ); return EXIT_FAILURE; }


#ifdef XP_UNIX
void GetAbsoluteModulePath( char* moduleFileName, size_t size, char *modulePath ) {

	if ( modulePath[0] == PATH_SEPARATOR ) { //  /jshost

		strcpy(moduleFileName, modulePath);
		return;
	}

	if ( modulePath[0] == '.' && modulePath[1] == PATH_SEPARATOR ) { //  ./jshost

		getcwd(moduleFileName, size);
		strcat(moduleFileName, modulePath + 1 );
		return;
	}

	if ( modulePath[0] == '.' && modulePath[1] == '.' && modulePath[2] == PATH_SEPARATOR ) { //  ../jshost

		getcwd(moduleFileName, size);
		strcat(moduleFileName, PATH_SEPARATOR_STRING);
		strcat(moduleFileName, modulePath);
		return;
	}

	if ( strchr( modulePath, PATH_SEPARATOR ) != NULL ) { //  xxx/../jshost

		getcwd(moduleFileName, size);
		strcat(moduleFileName, PATH_SEPARATOR_STRING);
		strcat(moduleFileName, modulePath);
		return;
	}

	char *envPath = getenv("PATH");
	char *pos;

	do {

		pos = strchr( envPath, ':' );

		if ( envPath[0] == PATH_SEPARATOR ) {

			if ( pos == NULL ) {

				strcpy(moduleFileName, envPath);
			} else {

				strncpy(moduleFileName, envPath, pos-envPath);
				moduleFileName[pos-envPath] = '\0';
			}

			strcat(moduleFileName, PATH_SEPARATOR_STRING);
			strcat(moduleFileName, modulePath);

			if (access(moduleFileName, R_OK | X_OK ) == 0) // If the requested access is permitted, it returns 0.
				return;
		}

		envPath = pos+1;

	} while (pos != NULL);

	moduleFileName[0] = '\0';
	return;
}
#endif //XP_UNIX


bool gEndSignal = false;

JSBool EndSignalGetter(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	*vp = BOOLEAN_TO_JSVAL( gEndSignal ? JS_TRUE : JS_FALSE );
	return JS_TRUE;
}

JSBool EndSignalSetter(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	JSBool tmp;
	J_CHK( JS_ValueToBoolean(cx, *vp, &tmp) );
	gEndSignal = (tmp == JS_TRUE);
	return JS_TRUE;
}


#ifdef XP_WIN
BOOL Interrupt(DWORD CtrlType) {

// see. http://msdn2.microsoft.com/en-us/library/ms683242.aspx
//	if (CtrlType == CTRL_LOGOFF_EVENT || CtrlType == CTRL_SHUTDOWN_EVENT) // CTRL_C_EVENT, CTRL_BREAK_EVENT, CTRL_CLOSE_EVENT, CTRL_LOGOFF_EVENT, CTRL_SHUTDOWN_EVENT
//		return FALSE;
	gEndSignal = true;
	return TRUE;
}
#else
void Interrupt(int CtrlType) {

	gEndSignal = true;
}
#endif // XP_WIN


int HostStdout( const char *buffer, size_t length ) {

	return write(fileno(stdout), buffer, length);
}

int HostStderr( const char *buffer, size_t length ) {

	return write(fileno(stderr), buffer, length);
}


//////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) { // check int _tmain(int argc, _TCHAR* argv[]) for UNICODE

	#ifdef XP_WIN
	BOOL status = SetConsoleCtrlHandler((PHANDLER_ROUTINE)&Interrupt, TRUE);
	HOST_MAIN_ASSERT( status == TRUE, "Unable to set console handler" );
	#else
	signal(SIGINT,Interrupt);
	signal(SIGTERM,Interrupt);
	#endif // XP_WIN

	uint32 maxMem = (uint32)-1; // by default, there are no limit
	uint32 maxAlloc = (uint32)-1; // by default, there are no limit

	bool unsafeMode = false;
	bool compileOnly = false;

	char** argumentVector = argv;
	for ( argumentVector++; argumentVector[0] && argumentVector[0][0] == '-'; argumentVector++ )
		switch ( argumentVector[0][1] ) {
			case 'm': // maxbytes (GC)
				argumentVector++;
				maxMem = atol( *argumentVector ) * 1024L * 1024L;
				break;
			case 'n': // maxAlloc (GC)
				argumentVector++;
				maxAlloc = atol( *argumentVector ) * 1024L * 1024L;
				break;
			case 'u': // avoid any runtime checks
				argumentVector++;
				unsafeMode = ( atoi( *argumentVector ) != 0 );
				break;
			case 'c':
				compileOnly = true;
				break;
	}

	JSContext *cx = CreateHost(maxMem, maxAlloc);
	HOST_MAIN_ASSERT( cx != NULL, "unable to create a javascript execution context" );

	HOST_MAIN_ASSERT( InitHost(cx, unsafeMode, HostStdout, HostStderr), "unable to initialize the host." );

	JSObject *globalObject = JS_GetGlobalObject(cx);
	JS_DefineProperty(cx, globalObject, "endSignal", JSVAL_VOID, EndSignalGetter, EndSignalSetter, JSPROP_SHARED | JSPROP_PERMANENT );

// script name
	const char *scriptName = *argumentVector;
	HOST_MAIN_ASSERT( scriptName != NULL, "no script specified" );

	char hostFullPath[PATH_MAX +1];

#ifdef XP_WIN
// get hostpath and hostname
	HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(NULL);
	DWORD len = GetModuleFileName(hInstance, hostFullPath, sizeof(hostFullPath));
	HOST_MAIN_ASSERT( len != 0, "unable to GetModuleFileName." );
#else // XP_WIN
	GetAbsoluteModulePath(hostFullPath, sizeof(hostFullPath), argv[0]);
	HOST_MAIN_ASSERT( hostFullPath[0] != '\0', "unable to get module FileName." );
//	int len = readlink("/proc/self/exe", moduleFileName, sizeof(moduleFileName)); // doc: readlink does not append a NUL character to buf.
//	moduleFileName[len] = '\0';
//	strcpy(hostFullPath, argv[0]);
#endif // XP_WIN

	char *hostPath, *hostName;
	hostName = strrchr( hostFullPath, PATH_SEPARATOR );
	if ( hostName != NULL ) {

		*hostName = '\0';
		hostName++;
		hostPath = hostFullPath;
	} else {

		hostPath = ".";
		hostName = hostFullPath;
	}

//	RT_HOST_MAIN_ASSERT( name != NULL, "unable to get module FileName." );
	JS_DefineProperty(cx, globalObject, NAME_GLOBAL_SCRIPT_HOST_PATH, STRING_TO_JSVAL(JS_NewStringCopyZ(cx, hostPath)), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineProperty(cx, globalObject, NAME_GLOBAL_SCRIPT_HOST_NAME, STRING_TO_JSVAL(JS_NewStringCopyZ(cx, hostName)), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);

	int exitValue;
	jsval rval;
	if ( ExecuteScript(cx, scriptName, compileOnly, argc-1, argv+1, &rval) == JS_TRUE ) {

		if ( JSVAL_IS_INT(rval) && JSVAL_TO_INT(rval) >= 0 )
			exitValue = JSVAL_TO_INT(rval);
		else
			exitValue = EXIT_SUCCESS;
	} else {

		exitValue = EXIT_FAILURE;
	}

	DestroyHost(cx);

#ifdef XP_WIN
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)&Interrupt, FALSE);
//	RT_HOST_MAIN_ASSERT( status == TRUE, "Unable to remove console crtl handler" );
#else
	signal(SIGINT, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
#endif // XP_WIN
	
	return exitValue;
}

/**doc
#summary jshost executable
#labels doc

= jshost executable =
 [http://code.google.com/p/jslibs/ home] *>* [JSLibs] *>* [jshost] - [http://jslibs.googlecode.com/svn/trunk/jshost/jshost.cpp http://jslibs.googlecode.com/svn/wiki/source.png]

=== Description ===

jshost ( javascript host ) is a small executable file that run javascript programs.
The main features are:
 * Lightweight
  The binary executable file is less than 60KB
 * Minimalist internal API
  LoadModule is enough, everything else can be added using dynamic loadable modules.

=== Command line options ===
 * `-m` <size>
  Specifies the maximum memory usage of the script.
 * `-n`  <size>
  Specifies the number of allocated megabytes after which garbage collection is run.
 * `-u` <0 or 1>
  Avoid any runtime checks (this is a kind of "release mode") and disable the display of warnings.


=== Global functions ===

 * status *LoadModule*( moduleFileName )
  Loads and initialize the specified module.
  Do not provide the file extension in _moduleFileName_.
  ===== exemple: =
  {{{
  LoadModule('jsstd');
  Print( 'Unsafe mode: '+configuration.unsafeMode, '\n' );
  }}}
  ===== note: =
  You can avoid LoadModule to use the global object like this:
  {{{
  var std = {};
  LoadModule.call( std, 'jsstd' );
  std.Print( std.IdOf(1234), '\n' );
  std.Print( std.IdOf(1234), '\n' );
  }}}

=== Global properties ===

 * *arguments*
  The command-line arguments ( given after command line options ).
  {{{
  }}}
  Exemple:
  {{{
  for ( var i in arguments ) {
    
    Print( 'argument['+i+'] = '+arguments[i] ,'\n' );
  }
  ...
  c:\>jshost -w 0 -u 0 foo bar
  argument[0] = foo
  argument[1] = bar
  }}}

 * *endSignal* http://jslibs.googlecode.com/svn/wiki/readonly.png
  Is true if a break signal ( ctrl-c, .. ) has been sent. This event can be reset.

=== Configuration object ===

jshost create a global _configuration_ global to provide other modules 
some useful informations like `stdout` access and `unsafeMode` flag.

== Remarks ==

=== Generated filename extensions are ===
 * ".dll" : for windows
 * ".so" : for linux

=== Modules entry points signature are ===
|| `"ModuleInit"` || `JSBool (*ModuleInitFunction)(JSContext *, JSObject *)` ||
|| `"ModuleRelease"` || `void (*ModuleReleaseFunction)(JSContext *cx)` ||
|| `"ModuleFree"` || `void (*ModuleFreeFunction)(void)` ||


=== Exemple (win32) ===
{{{
extern "C" __declspec(dllexport) JSBool ModuleInit(JSContext *cx, JSObject *obj) {

 InitFileClass(cx, obj);
 InitDirectoryClass(cx, obj);
 InitSocketClass(cx, obj);
 InitErrorClass(cx, obj);
 InitGlobal(cx, obj);

 return JS_TRUE;
}
}}}
**/
