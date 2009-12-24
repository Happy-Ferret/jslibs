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

#include "jslibsModule.cpp"

#define NO_NED_NAMESPACE
#define NO_MALLINFO 1
#include "../../libs/nedmalloc/nedmalloc.h"

volatile bool disabledFree = false;

void nedfree_handlenull(void *mem) {
	
	if ( mem != NULL && !disabledFree )
		nedfree(mem);
}


// to be used in the main() function only
#define HOST_MAIN_ASSERT( condition, errorMessage ) if ( !(condition) ) { goto bad; }
//#define HOST_MAIN_ASSERT( condition, errorMessage ) if ( !(condition) ) { fprintf(stderr, errorMessage ); goto bad; }

static unsigned char embeddedBootstrapScript[] =
	#include "embeddedBootstrapScript.js.xdr.cres"
;

/*
int consoleStdOut( JSContext *cx, const char *data, int length ) {

	JSObject *obj = GetConfigurationObject(cx);
	JL_S_ASSERT( obj != NULL, "Unable to get GetConfigurationObject" );
	jsval functionVal;
	JS_GetProperty(cx, obj, "stdout", &functionVal);
	if ( !JSVAL_IS_VOID( functionVal ) ) {

		JL_S_ASSERT_FUNCTION( functionVal );
		JSString *str = JS_NewStringCopyN(cx, data, length);
		JL_CHK( str ); 
		jsval rval, arg = STRING_TO_JSVAL(str);
		JL_CHK ( JS_CallFunctionValue(cx, obj, functionVal, 1, &arg, &rval) );
	}
	return length;
}

int consoleStdErr( JSContext *cx, const char *data, int length ) {

	JSObject *obj = GetConfigurationObject(cx);
	JL_S_ASSERT( obj != NULL, "Unable to get GetConfigurationObject" );
	jsval functionVal;
	JS_GetProperty(cx, obj, "stderr", &functionVal);
	if ( !JSVAL_IS_VOID( functionVal ) ) {

		JL_S_ASSERT_FUNCTION( functionVal );
		JSString *str = JS_NewStringCopyN(cx, data, length);
		JL_CHK( str ); 
		jsval rval, arg = STRING_TO_JSVAL(str);
		JL_CHK( JS_CallFunctionValue(cx, obj, functionVal, 1, &arg, &rval) );
	}
	return length;
}
*/

/*
static JSBool stderrFunction(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	JSString *str;
	str = JS_ValueToString(cx, argv[0]);
	JL_S_ASSERT( str != NULL, "Unable to convert argument to string.");
	argv[0] = STRING_TO_JSVAL(str); // (TBD) needed ?
	consoleStdErr( cx, JS_GetStringBytes(str), JS_GetStringLength(str) );
	return JS_TRUE;
}

static JSBool stdoutFunction(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	JSString *str;
	str = JS_ValueToString(cx, argv[0]);
	JL_S_ASSERT( str != NULL, "Unable to convert argument to string.");
	argv[0] = STRING_TO_JSVAL(str); // (TBD) needed ?
	consoleStdOut( cx, JS_GetStringBytes(str), JS_GetStringLength(str) );
	return JS_TRUE;
}
*/

int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow ) {

	HANDLE heap = GetProcessHeap();
	ULONG enable = 2;
	HeapSetInformation(heap, HeapCompatibilityInformation, &enable, sizeof(enable));

	JSContext *cx = NULL;

	errno_t err;
	CHAR moduleName[PATH_MAX], scriptName[PATH_MAX], mutexName[PATH_MAX];
	DWORD moduleNameLen = GetModuleFileName(hInstance, moduleName, sizeof(moduleName));
	HOST_MAIN_ASSERT( moduleNameLen > 0, "Invalid module filename." );

	//doc: If you need to detect whether another instance already exists, create a uniquely named mutex using the CreateMutex function. 
	//CreateMutex will succeed even if the mutex already exists, but the function will return ERROR_ALREADY_EXISTS. 
	//This indicates that another instance of your application exists, because it created the mutex first.

	// (TBD) use file index as mutexName. note: If the file is on an NTFS volume, you can get a unique 64 bit identifier for it with GetFileInformationByHandle.  The 64 bit identifier is the "file index". 
	
	bool hasPrevInstance;
	err = strncpy_s(mutexName, sizeof(mutexName), moduleName, moduleNameLen);
	HOST_MAIN_ASSERT( err == 0, "Buffer overflow." );
	// normalize the mutex name
	char *pos;
	while ( pos = strchr(mutexName, '\\') )
		*pos = '/';
	SetLastError(0);
	HANDLE instanceCheckMutex = CreateMutex(NULL, TRUE, mutexName); // see Global\\ and Local\\ prefixes for mutex name.
	switch ( GetLastError() ) {
		case ERROR_ALREADY_EXISTS:
			hasPrevInstance = true;
			break;
		case ERROR_SUCCESS:
			hasPrevInstance = false;
			break;
		default: {
			char message[1024];
			JLLastSysetmErrorMessage(message, sizeof(message));
			HOST_MAIN_ASSERT( false,  message );
		}
	}

	jl_malloc = nedmalloc;
	jl_calloc = nedcalloc;
	jl_memalign = nedmemalign;
	jl_realloc = nedrealloc;
	jl_msize = nedblksize;
	jl_free = nedfree_handlenull;
/*
	jl_malloc = malloc;
	jl_calloc = calloc;
	jl_memalign = memalign;
	jl_realloc = realloc;
	jl_msize = msize;
	jl_free = free;
*/

	InitializeMemoryManager(&jl_malloc, &jl_calloc, &jl_memalign, &jl_realloc, &jl_msize, &jl_free);
#ifdef JS_HAS_JSLIBS_RegisterCustomAllocators
	JSLIBS_RegisterCustomAllocators(jl_malloc, jl_calloc, jl_memalign, jl_realloc, jl_msize, jl_free);
#endif // JS_HAS_JSLIBS_RegisterCustomAllocators

	cx = CreateHost(-1, -1, 0);
	JL_CHK( cx != NULL );

	MemoryManagerEnableGCEvent(cx);

	HostPrivate *hpv = GetHostPrivate(cx);

	// custom memory allocators are transfered to the modules through the HostPrivate structure:
	hpv->alloc.malloc = jl_malloc;
	hpv->alloc.calloc = jl_calloc;
	hpv->alloc.memalign = jl_memalign;
	hpv->alloc.realloc = jl_realloc;
	hpv->alloc.msize = jl_msize;
	hpv->alloc.free = jl_free;


	JL_CHK( InitHost(cx, true, NULL, NULL, NULL) );
	CHAR moduleFileName[PATH_MAX];
	strcpy(moduleFileName, moduleName);
	char *name = strrchr( moduleFileName, '\\' );
	JL_CHK( name );
	*name = '\0';
	name++;


	err = strncpy_s(scriptName, sizeof(scriptName), moduleName, moduleNameLen );
	JL_S_ASSERT( err == 0, "Buffer overflow." );
//	DWORD scriptNameLen = GetModuleFileName(hInstance, scriptName, sizeof(scriptName));
	char *dotPos = strrchr(scriptName, '.');
	JL_CHK( dotPos );
	*dotPos = '\0';
	err = strcat_s( scriptName, sizeof(scriptName), ".js" );
	JL_S_ASSERT( err == 0, "Buffer overflow." );


	JSObject *globalObject = JS_GetGlobalObject(cx);

	// arguments
//	JL_CHK( JS_DefineProperty(cx, globalObject, NAME_GLOBAL_ARGUMENT, STRING_TO_JSVAL(JS_NewStringCopyZ(cx, lpCmdLine)), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT) ); // see ExecuteScriptFileName()
	JL_CHK( JS_DefinePropertyById(cx, globalObject, JLID(cx, scripthostpath), STRING_TO_JSVAL(JS_NewStringCopyZ(cx, moduleFileName)), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT) );
	JL_CHK( JS_DefinePropertyById(cx, globalObject, JLID(cx, scripthostname), STRING_TO_JSVAL(JS_NewStringCopyZ(cx, name)), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT) );
	JL_CHK( JS_DefinePropertyById(cx, globalObject, JLID(cx, isfirstinstance), BOOLEAN_TO_JSVAL(hasPrevInstance?JS_FALSE:JS_TRUE), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT) );

	//#pragma comment (lib, "User32.lib")
	//MessageBox(NULL, scriptName, "script name", 0);

	if ( sizeof(embeddedBootstrapScript)-1 > 0 )
		JL_CHK( ExecuteBootstrapScript(cx, embeddedBootstrapScript, sizeof(embeddedBootstrapScript)-1) ); // -1 because sizeof("") == 1

	jsval rval;
	const char *argv[] = { scriptName, lpCmdLine };
	if ( ExecuteScriptFileName(cx, scriptName, false, COUNTOF(argv), argv, &rval) != JS_TRUE )
		if ( JS_IsExceptionPending(cx) )
			JS_ReportPendingException(cx); // see JSOPTION_DONT_REPORT_UNCAUGHT option.

	disabledFree = true;
	MemoryManagerDisableGCEvent(cx);
	FinalizeMemoryManager(!disabledFree, &jl_malloc, &jl_calloc, &jl_memalign, &jl_realloc, &jl_msize, &jl_free);

	JS_CommenceRuntimeShutDown(JS_GetRuntime(cx));
	JS_SetGCCallback(cx, NULL);
	DestroyHost(cx);
	cx = NULL;
	JS_ShutDown();

	CloseHandle( instanceCheckMutex ); //ReleaseMutex

	return 0;
bad:

	if ( cx ) {

		disabledFree = true;
		JS_CommenceRuntimeShutDown(JS_GetRuntime(cx));
		JS_SetGCCallback(cx, NULL);
		DestroyHost(cx);
	}
	JS_ShutDown();

	CloseHandle( instanceCheckMutex ); //ReleaseMutex
	return -1;
}

/**doc
#summary jswinhost executable
#labels doc

= jswinhost executable =
 [http://code.google.com/p/jslibs/ home] *>* [JSLibs] *>* [jswinhost] - [http://jslibs.googlecode.com/svn/trunk/jswinhost/jswinhost.cpp http://jslibs.googlecode.com/svn/wiki/source.png]

=== Description ===
 jswinhost (javascript windows host) is a small executable file that run javascript programs under a windows environment.
 The main difference with jshost is the jswinhost does not create any console windows.

=== Methods ===

 * status *LoadModule*( moduleFileName )
  see [jshost]

=== Properties ===

 * $OBJ *global* $READONLY
  is the global object.

 * $ARRAY *arguments* $READONLY
  is the host path [0] and whole command line [1].

 * $BOOL *isfirstinstance* $READONLY
  is true if the current instance is the first one. This can help to avoid jswinhost to be run twice at the same time.

=== Configuration object ===
 see [jshost]

=== Remarks ===
 There is no way to specify the script to execute using the command line. You have to create a .js file using the name of the host.
 By default, jswinhost.exe is the name of the host and jswinhost.js is the script the host execute.
 
Because jwinshost do not use a console window, errors and printed messages will not be displayed.

However, you can write your own output system:
{{{
LoadModule('jswinshell');
_configuration.stdout = new Console().Write;
_configuration.stderr = MessageBox;
LoadModule('jsstd');
Print('toto');
hkqjsfhkqsdu_error();
}}}

== Embedding JS scripts in your jswinhost binary ==
 see [jshost]

**/
