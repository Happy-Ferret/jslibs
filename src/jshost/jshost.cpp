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


volatile bool gEndSignal = false;

JSBool EndSignalGetter(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	J_CHK( BoolToJsval(cx, gEndSignal, vp) );
	return JS_TRUE;
	JL_BAD;
}

JSBool EndSignalSetter(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	bool tmp;
	J_CHK( JsvalToBool(cx, *vp, &tmp) );
	gEndSignal = tmp;
	return JS_TRUE;
	JL_BAD;
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


int HostStdout( void *privateData, const char *buffer, size_t length ) {

	return write(fileno(stdout), buffer, length);
}

int HostStderr( void *privateData, const char *buffer, size_t length ) {

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
	size_t maybeGCInterval = 30; // 30 seconds
	int camelCase = 0; // 0:default, 1:lower, 2:upper

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
			case 'g': // operationLimitGC
				argumentVector++;
				maybeGCInterval = atol( *argumentVector ) * 1000; // s to ms
				break;
			case 'c':
				compileOnly = true;
				break;
			case 'l':
				argumentVector++;
				camelCase = atoi( *argumentVector );
				break;
	}

	JSContext *cx = CreateHost(maxMem, maxAlloc, maybeGCInterval);
	HOST_MAIN_ASSERT( cx != NULL, "unable to create a javascript execution context" );

	GetHostPrivate(cx)->camelCase = camelCase;

	HOST_MAIN_ASSERT( InitHost(cx, unsafeMode, HostStdout, HostStderr, NULL), "unable to initialize the host." );

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
	if ( ExecuteScriptFileName(cx, scriptName, compileOnly, argc - (argumentVector-argv), argumentVector, &rval) == JS_TRUE ) {

		if ( JSVAL_IS_INT(rval) && JSVAL_TO_INT(rval) >= 0 )
			exitValue = JSVAL_TO_INT(rval);
		else
			exitValue = EXIT_SUCCESS;
	} else {

		if ( JS_IsExceptionPending(cx) ) { // see JSOPTION_DONT_REPORT_UNCAUGHT option.

			jsval ex;
			JS_GetPendingException(cx, &ex);
			JL_ValueOf(cx, &ex, &ex);
			if ( JSVAL_IS_INT(ex) ) {

				exitValue = JSVAL_TO_INT(ex);
		} else {

				JS_ReportPendingException(cx);
				exitValue = EXIT_FAILURE;
			}
		} else {

			exitValue = EXIT_FAILURE;
		}
	}

	DestroyHost(cx);
	JS_ShutDown();

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
 * `-c` <0 or 1> (default = 0)
  Compile-only. The script is compiled but not executed. This is useful to detect syntax errors.
 * `-u` <0 or 1> (default = 0)
  Unsafe-mode is a kind of 'release mode'. If 1, any runtime checks is avoid and warnings display is disabled. This mode allow to increase performances.
 * `-m` <size> (default: no limit)
  Specifies the maximum memory usage of the script in megabytes.
 * `-n`  <size> (default: no limit)
  Specifies the number of allocated megabytes after which garbage collection is run.
 * `-g` <loops> (default = 8192)
  This is the frequency at witch the GarbageCollector is launched.

=== Exit code ===
 * The exit code of jshost is 1 on error. On success, exit code is the last evaluated expression of the script.
   If this last expression is a positive integer, its value is returned, in any other case, 0 is returned.
 * If there is a pending uncatched exception and if this exception can be converted into a number (see valueOf), this numeric value is used as exit code.
 $H example
  {{{
  function Exit(code) {
   throw code;
  }
  Exit(2);
  }}}

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
  c:\>jshost -g 8192 -u 0 foo.js bar
  argument[0] = foo.js
  argument[1] = bar
  }}}

 * *endSignal* $READONLY
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


