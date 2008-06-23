#include "stdafx.h"


/*
int consoleStdOut( JSContext *cx, const char *data, int length ) {

	JSObject *obj = GetConfigurationObject(cx);
	RT_ASSERT( obj != NULL, "Unable to get GetConfigurationObject" );
	jsval functionVal;
	JS_GetProperty(cx, obj, "stdout", &functionVal);
	if ( functionVal != JSVAL_VOID ) {

		RT_ASSERT_FUNCTION( functionVal );
		JSString *str = JS_NewStringCopyN(cx, data, length);
		RT_ASSERT_ALLOC( str ); 
		jsval rval, arg = STRING_TO_JSVAL(str);
		RT_CHECK_CALL ( JS_CallFunctionValue(cx, obj, functionVal, 1, &arg, &rval) );
	}
	return length;
}

int consoleStdErr( JSContext *cx, const char *data, int length ) {

	JSObject *obj = GetConfigurationObject(cx);
	RT_ASSERT( obj != NULL, "Unable to get GetConfigurationObject" );
	jsval functionVal;
	JS_GetProperty(cx, obj, "stderr", &functionVal);
	if ( functionVal != JSVAL_VOID ) {

		RT_ASSERT_FUNCTION( functionVal );
		JSString *str = JS_NewStringCopyN(cx, data, length);
		RT_ASSERT_ALLOC( str ); 
		jsval rval, arg = STRING_TO_JSVAL(str);
		RT_CHECK_CALL( JS_CallFunctionValue(cx, obj, functionVal, 1, &arg, &rval) );
	}
	return length;
}
*/

/*
static JSBool stderrFunction(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	JSString *str;
	str = JS_ValueToString(cx, argv[0]);
	RT_ASSERT( str != NULL, "Unable to convert argument to string.");
	argv[0] = STRING_TO_JSVAL(str); // (TBD) needed ?
	consoleStdErr( cx, JS_GetStringBytes(str), JS_GetStringLength(str) );
	return JS_TRUE;
}

static JSBool stdoutFunction(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	JSString *str;
	str = JS_ValueToString(cx, argv[0]);
	RT_ASSERT( str != NULL, "Unable to convert argument to string.");
	argv[0] = STRING_TO_JSVAL(str); // (TBD) needed ?
	consoleStdOut( cx, JS_GetStringBytes(str), JS_GetStringLength(str) );
	return JS_TRUE;
}
*/

int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow ) {

	JSContext *cx = CreateHost(-1, -1);
	if ( cx == NULL )
		return -1;

	if ( !InitHost(cx, true, NULL, NULL) )
		return -1;

	errno_t err;
	CHAR moduleFileName[PATH_MAX];
	DWORD len = GetModuleFileName(hInstance, moduleFileName, sizeof(moduleFileName));
	if ( len == 0 )
		return -1;
	char *name = strrchr( moduleFileName, '\\' );
	if ( name == NULL )
		return -1;
	*name = '\0';
	name++;


	CHAR moduleName[PATH_MAX];
	DWORD moduleNameLen = GetModuleFileName(hInstance, moduleName, sizeof(moduleName));

	CHAR scriptName[PATH_MAX];
	err = strncpy_s(scriptName, sizeof(scriptName), moduleName, moduleNameLen );
	RT_ASSERT( err == 0, "Buffer overflow." );
//	DWORD scriptNameLen = GetModuleFileName(hInstance, scriptName, sizeof(scriptName));
	char *dotPos = strrchr(scriptName, '.');
	if ( dotPos == NULL )
		return -1;
	*dotPos = '\0';
	err = strcat_s( scriptName, sizeof(scriptName), ".js" );
	RT_ASSERT( err == 0, "Buffer overflow." );

	//If you need to detect whether another instance already exists, create a uniquely named mutex using the CreateMutex function. 
	//CreateMutex will succeed even if the mutex already exists, but the function will return ERROR_ALREADY_EXISTS. 
	//This indicates that another instance of your application exists, because it created the mutex first.


	// (TBD) use file index as mutexName. note: If the file is on an NTFS volume, you can get a unique 64 bit identifier for it with GetFileInformationByHandle.  The 64 bit identifier is the "file index". 
	char mutexName[PATH_MAX];// = "jswinhost_";
	err = strncpy_s(mutexName, sizeof(mutexName), moduleName, moduleNameLen );
	RT_ASSERT( err == 0, "Buffer overflow." );
	err = strcat_s(mutexName, sizeof(mutexName), name);
	RT_ASSERT( err == 0, "Buffer overflow." );
	SetLastError(0);
	HANDLE instanceCheckMutex = CreateMutex( NULL, TRUE, mutexName );
	bool hasPrevInstance = GetLastError() == ERROR_ALREADY_EXISTS;


	JSObject *globalObject = JS_GetGlobalObject(cx);

	// arguments
//	JS_DefineProperty(cx, globalObject, NAME_GLOBAL_ARGUMENT, STRING_TO_JSVAL(JS_NewStringCopyZ(cx, lpCmdLine)), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineProperty(cx, globalObject, NAME_GLOBAL_SCRIPT_HOST_NAME, STRING_TO_JSVAL(JS_NewStringCopyZ(cx, name)), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineProperty(cx, globalObject, NAME_GLOBAL_SCRIPT_HOST_PATH, STRING_TO_JSVAL(JS_NewStringCopyZ(cx, moduleFileName)), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineProperty(cx, globalObject, NAME_GLOBAL_FIRST_INSTANCE, BOOLEAN_TO_JSVAL(hasPrevInstance?JS_FALSE:JS_TRUE), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);

	const char * argv[] = { scriptName, lpCmdLine };
	jsval rval;
	ExecuteScript(cx, scriptName, false, 2, argv, &rval);

	//ReleaseMutex
	CloseHandle( instanceCheckMutex );

	return 0;
}

/**doc
#summary jswinhost executable
#labels doc

= jswinhost executable =
 [http://code.google.com/p/jslibs/ home] *>* [JSLibs] *>* [jswinhost] - [http://jslibs.googlecode.com/svn/trunk/jswinhost/jswinhost.cpp http://jslibs.googlecode.com/svn/wiki/source.png]

=== Description ===

jswinhost ( javascript windows host ) is a small executable file that run javascript programs under a windows environment.
The main difference with jshost is the jswinhost do not popup any windows

=== Functions ===

 * status *LoadModule*( moduleFileName )
  same as [jshost]

=== Properties ===

 * object *global* http://jslibs.googlecode.com/svn/wiki/readonly.png
 * string *argument*
  the whole command line

=== Configuration object ===
 see [jshost]

=== Remarks ===

There is no way to specify the script to execute using the command line. You have to create a .js file using the name of the host.
By default, jswinhost.exe is the name of the host and jswinhost.js is the script the host execute.
 
Because jwinshost do not use a console window, errors and printed messages will not be displayed.

However, you can write your own output system:
{{{
LoadModule('jswinshell');
configuration.stdout = new Console().Write;
configuration.stderr = MessageBox;
LoadModule('jsstd');
Print('toto');
hkqjsfhkqsdu_error();
}}}
**/