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
#define HOST_MAIN_ASSERT( condition, errorMessage ) \
	if ( !(condition) ) { fprintf(stderr, errorMessage ); return EXIT_FAILURE; }


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


//////////////////////////////////////////////////////////////////////////////////////////////


bool gEndSignal = false;

JSBool EndSignalGetter(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	*vp = BOOLEAN_TO_JSVAL( gEndSignal == true );
	return JS_TRUE;
}

JSBool EndSignalSetter(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	JSBool tmp;
	JS_ValueToBoolean(cx, *vp, &tmp);
	gEndSignal = tmp == JS_TRUE;
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


//////////////////////////////////////////////////////////////////////////////////////////////



static JSBool stderrFunction(JSContext *cx, uintN argc, jsval *vp) {

	const char *buffer;
	size_t length;
	J_CHK( JsvalToStringAndLength(cx, J_FARG(1), &buffer, &length) );
	write(2, buffer, length);
	return JS_TRUE;
bad:
	return JS_FALSE;
}

static JSBool stdoutFunction(JSContext *cx, uintN argc, jsval *vp) {

	const char *buffer;
	size_t length;
	J_CHK( JsvalToStringAndLength(cx, J_FARG(1), &buffer, &length) );
	write(1, buffer, length);
	return JS_TRUE;
bad:
	return JS_FALSE;
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

	HOST_MAIN_ASSERT( InitHost(cx, unsafeMode, stdoutFunction, stderrFunction), "unable to initialize the host.") ;

	JSObject *globalObject = JS_GetGlobalObject(cx);
	JS_DefineProperty(cx, globalObject, "endSignal", JSVAL_VOID, EndSignalGetter, EndSignalSetter, JSPROP_SHARED | JSPROP_PERMANENT );

// script name
  const char *scriptName = *argumentVector;
  HOST_MAIN_ASSERT( scriptName != NULL, "no script specified" );

/*
// arguments
	JSObject *argsObj = JS_NewArrayObject(cx, 0, NULL);
	RT_HOST_MAIN_ASSERT( argsObj != NULL, "unable to create argument array." );

	jsStatus = JS_DefineProperty(cx, globalObject, NAME_GLOBAL_ARGUMENTS, OBJECT_TO_JSVAL(argsObj), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
	RT_HOST_MAIN_ASSERT( jsStatus == JS_TRUE, "unable to store the argument array." );
	int index = 0;
	for ( char** argumentVectorit = argumentVector; *argumentVectorit; argumentVectorit++ ) {

		JSString *str = JS_NewStringCopyZ(cx, *argumentVectorit);
		RT_HOST_MAIN_ASSERT( str != NULL, "unable to store the argument." );
		jsStatus = JS_DefineElement(cx, argsObj, index++, STRING_TO_JSVAL(str), NULL, NULL, JSPROP_ENUMERATE);
		RT_HOST_MAIN_ASSERT( jsStatus == JS_TRUE, "unable to define the argument." );
	}
*/

	char hostFullPath[PATH_MAX +1];

#ifdef XP_WIN
// get hostpath and hostname
	HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(NULL);
	DWORD len = GetModuleFileName(hInstance, hostFullPath, sizeof(hostFullPath));
	HOST_MAIN_ASSERT( len != 0, "unable to GetModuleFileName." );
#else // XP_WIN
	GetAbsoluteModulePath(hostFullPath, sizeof(hostFullPath), argv[0]);
	RT_HOST_MAIN_ASSERT( hostFullPath[0] != '\0', "unable to get module FileName." );
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

// compile & executes the script

	int exitValue;

	jsval rval;
	if ( !ExecuteScript(cx, scriptName, compileOnly, argc-1, argv+1, &rval) ) {

		exitValue = EXIT_FAILURE;
	} else {

		if ( JSVAL_IS_INT(rval) && JSVAL_TO_INT(rval) >= 0 )
			exitValue = JSVAL_TO_INT(rval);
		else
			exitValue = EXIT_SUCCESS;
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



/*
jshost VERSION:
--------------
	0.1


README:
------
	jshost is strongly based on the Spidermonkey interactive shell : mozilla/js/src/js.c


Spidermonkey configuration:
--------------------------
	I have enabled XDR api by modifying mozilla/js/src/config.h
		#define JS_HAS_XDR_FREEZE_THAW  1

	"If you are on windows make sure you also define JS_USE_ONLY_NSPR_LOCKS in your js builds. That solved a lock hang problem for me."
		cf. http://groups.google.fr/group/mozilla.dev.tech.js-engine/browse_thread/thread/c59a6b91bd072c1e


Spidermonkey compilation:
------------------------
	call "C:\Program Files\Microsoft Platform SDK\SetEnv.Cmd" /XP32 /RETAIL
	call "C:\Program Files\Microsoft Visual Studio 8\VC\vcvarsall.bat" x86
	set path=%path%;C:\tools\cygwin\bin
	cd .\mozilla\js\src
	make -f Makefile.ref clean all BUILD_OPT=1 XCFLAGS=/MT


Directories structure:
---------------------
	.
	..
	jshost <--
	libfile
	libffi
	libjsni
	libsocket
	mozilla
		js
			src


jhost documentation:
-------------------
  Command-line switchs:
		-w <1 or 0> : enable/disable warning reporting
		-m <number> : allocated bytes after which garbage collection is run
		-s <number> : stack size
		-c <1 or 0> : enable/disable compiled scripts to saved on disk
		-u <1 or 0> : enable/disable unsafe mode. Some logical checks are no more done. Use this switch carefully !

	Global object
		.seal(object,deep) : set <object> read-only, <deep> set sub-objects too
		.exec(file) : load and executes <file> ( see -c switch )
		.print(string, ...) : display strings on the standard output (stdout)
		.collectGarbage() : force the garbage collector to run
		.warning(string) : retport the warning <string> to strerr ( see -w switch )


Spidermonkey API DOC reminder:
-----------------------------
  JS_SetPendingException
	  ???
  JS_SetBranchCallback
	  specifies a callback function that is automatically called when a script branches backward during execution, when a function returns,
	  and at the end of the script. One typical use for a callback is in a client application to enable a user to abort an operation.
	JSFunction * JS_DefineFunction(JSContext *cx, JSObject *obj, const char *name, JSNative call, uintN nargs, uintN flags);
		nargs indicates the number of arguments the function expects to receive. JS uses this information to allocate storage space for each argument.

FAQ
---
What's the nargs member of JSFunctionSpec actually used for?
	It's not minimum requirement in the	report-an-error/throw-an-exception-if-fewer-actuals sense.  It's the minimum the callee can count on dereferencing with non-negative indexes via argumentVector.
	If fewer actuals are passed, the engine will push undefined until nargs arguments are available.
	...
	Just because argc reflects the actual parameter count does not mean that you cannot dereference argumentVector[argc] or argumentVector[argc+1] safely -- you can, all the way up to argumentVector[NARGS-1],
	for the value of NARGS you stored in the JSFunctionSpec.nargs initializer, or passed to JS_DefineFunction.
  ...
	Ah, I think I understand now. I can specify 0 for nargs for *all* functions just as long as I check argc's value before dereferencing argumentVector[] elements.
	But if I want guarantee that argumentVector[0], as an example, can be dereferenced (regardless of the number of arguments actually passed), then I need to specify an nargs value of 1 (or higher). Right?


Useful Links:
------------
  jshost web sites:
		http://soubok.googlepages.com/javascript
		http://code.google.com/p/jshost/
  Spidermonkey Web Site
    http://www.mozilla.org/js/spidermonkey/
  Spidermonkey bonsai
    http://bonsai.mozilla.org/rview.cgi?dir=mozilla/js/src&cvsroot=/cvsroot&module=default
  Spidermonkey release-notes
    http://www.mozilla.org/js/spidermonkey/release-notes/
  Spidermonkey API DOC
    http://www.mozilla.org/js/spidermonkey/apidoc/complete-frameset.html
    http://www.sterlingbates.com/jsref/sparse-frameset.html
  socket doc
    http://www.synchro.net/
  tutorial
    http://egachine.berlios.de/embedding-sm-best-practice/embedding-sm-best-practice.html
    http://egachine.berlios.de/embedding-sm-best-practice/
  xpconnect
    http://lxr.mozilla.org/seamonkey/source/js/src/xpconnect/src/xpcwrappednativejsops.cpp
  ???
    http://users.skynet.be/saw/SpiderMonkey.htm
  WXjs
    http://wxjs.sourceforge.net/
  JSFILE
    http://www.mozilla.org/js/js-file-object.html#playwithfire
  NSPR 4.6 - src/bin
    ftp://ftp.mozilla.org/pub/mozilla.org/nspr/releases/v4.6

	Dynamic-Link Library Search Order
		http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dllproc/base/dynamic-link_library_search_order.asp

	Dynamic-Link Library Redirection
		http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dllproc/base/dynamic_link_library_redirection.asp

*/
