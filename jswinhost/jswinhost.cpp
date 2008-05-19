#include "stdafx.h"

#include "../common/platform.h"

#include "jsstddef.h"
#include <jsapi.h>

#include "jsprf.h"

#include "../common/jsNames.h"
#include "../common/jshelper.h"
#include "../moduleManager/moduleManager.h"
#include "../common/jsConfiguration.h"

static JSBool LoadModule(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT_ARGC(1);
	const char *fileName;
	RT_JSVAL_TO_STRING( argv[0], fileName );
	char libFileName[PATH_MAX];
	errno_t err = strcpy_s(libFileName, sizeof(libFileName), fileName);
	RT_ASSERT( err == 0, "Buffer overflow." );
	err = strcat_s(libFileName, sizeof(libFileName), DLL_EXT);
	RT_ASSERT( err == 0, "Buffer overflow." );

	ModuleId id = ModuleLoad(libFileName, cx, obj);
	RT_ASSERT_2( id != 0, "Unable to load the module %s (error:%d).", libFileName, GetLastError() );
	RT_CHECK_CALL( JS_NewNumberValue(cx, id, rval) );

	return JS_TRUE;
}

static JSBool UnloadModule(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT_ARGC(1);
	jsdouble dVal;
	RT_CHECK_CALL( JS_ValueToNumber(cx, argv[0], &dVal) );
	ModuleId id = dVal;
	bool st = ModuleUnload(id, cx);
	RT_ASSERT( st == true, "Unable to unload the module" );
	return JS_TRUE;
}

static JSBool noop(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	return JS_TRUE;
}

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

bool reportWarnings = true;

// function copied from ../js/src/js.c
static void ErrorReporter(JSContext *cx, const char *message, JSErrorReport *report) {

    int i, j, k, n;
    char *prefix, *tmp;
    const char *ctmp;

    if (!report) {

		consoleStdOut( cx, message, strlen(message) );
		consoleStdOut( cx, "\n", 1 );
		return;
    }

    /* Conditionally ignore reported warnings. */
    if (JSREPORT_IS_WARNING(report->flags) && !reportWarnings)
        return;

    prefix = NULL;
    if (report->filename)
        prefix = JS_smprintf("%s:", report->filename);
    if (report->lineno) {
        tmp = prefix;
        prefix = JS_smprintf("%s%u: ", tmp ? tmp : "", report->lineno);
        JS_free(cx, tmp);
    }
    if (JSREPORT_IS_WARNING(report->flags)) {
        tmp = prefix;
        prefix = JS_smprintf("%s%swarning: ",
                             tmp ? tmp : "",
                             JSREPORT_IS_STRICT(report->flags) ? "strict " : "");
        JS_free(cx, tmp);
    }

    /* embedded newlines -- argh! */
    while ((ctmp = strchr(message, '\n')) != 0) {
        ctmp++;
        if (prefix)
            consoleStdErr( cx, prefix, strlen(prefix) );
        consoleStdErr( cx, message, ctmp - message );
        message = ctmp;
    }

    /* If there were no filename or lineno, the prefix might be empty */
    if (prefix)
	    consoleStdErr( cx, prefix, strlen(prefix) );
    consoleStdErr( cx, message, strlen(message) );

    if (!report->linebuf) {
        consoleStdErr(cx, "\n", 1);
        goto out;
    }

    /* report->linebuf usually ends with a newline. */
    n = strlen(report->linebuf);
    char *msg = JS_smprintf(":\n%s%s%s%s",
            prefix,
            report->linebuf,
            (n > 0 && report->linebuf[n-1] == '\n') ? "" : "\n",
            prefix);
	 consoleStdErr( cx, msg, strlen(msg) );
    n = PTRDIFF(report->tokenptr, report->linebuf, char);
    for (i = j = 0; i < n; i++) {
        if (report->linebuf[i] == '\t') {
            for (k = (j + 8) & ~7; j < k; j++) {
                consoleStdErr( cx, ".", 1);
            }
            continue;
        }
        consoleStdErr( cx, ".", 1);
        j++;
    }
    consoleStdErr( cx, "^\n", 2);
 out:
//    if (!JSREPORT_IS_WARNING(report->flags))
//        gExitCode = EXITCODE_RUNTIME_ERROR;
    JS_free(cx, prefix);
}



int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow ) {

	JSRuntime *rt;
	JSContext *cx;
	JSObject *globalObject;

	unsigned long maxbytes = 16L * 1024L * 1024L;
	unsigned long stackSize = 8192; //1L * 1024L * 1024L; // http://groups.google.com/group/mozilla.dev.tech.js-engine/browse_thread/thread/be9f404b623acf39/9efdfca81be99ca3

	rt = JS_NewRuntime(maxbytes); // maxbytes specifies the number of allocated bytes after which garbage collection is run.
	cx = JS_NewContext(rt, stackSize); // A context specifies a stack size for the script, the amount, in bytes, of private memory to allocate to the execution stack for the script.

	JS_SetVersion( cx, (JSVersion)JS_VERSION );
	// (TBD) set into configuration file

	JS_SetErrorReporter(cx, ErrorReporter);

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

	JSBool jsStatus;
// global object
	JSClass global_class = { NAME_GLOBAL_CLASS, 0, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub };
	globalObject = JS_NewObject(cx, &global_class, NULL, NULL);

// Standard classes
	jsStatus = JS_InitStandardClasses(cx, globalObject);
	if ( jsStatus == JS_FALSE )
		return -1;

// global functions & properties
	JS_DefineProperty(cx, globalObject, NAME_GLOBAL_GLOBAL_OBJECT, OBJECT_TO_JSVAL(JS_GetGlobalObject(cx)), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineFunction(cx, globalObject, NAME_GLOBAL_FUNCTION_LOAD_MODULE, LoadModule, 0, 0);
	JS_DefineFunction(cx, globalObject, NAME_GLOBAL_FUNCTION_UNLOAD_MODULE, UnloadModule, 0, 0);

// Global configuration object
	JSObject *configObject = GetConfigurationObject(cx);
	if ( configObject == NULL )
		return -1;

	jsval value;
	
	value = OBJECT_TO_JSVAL(JS_GetFunctionObject(JS_NewFunction(cx, noop, 1, 0, NULL, NULL))); // doc: If you do not assign a name to the function, it is assigned the name "anonymous".
	JS_SetProperty(cx, configObject, NAME_CONFIGURATION_STDERR, &value);

	value = OBJECT_TO_JSVAL(JS_GetFunctionObject(JS_NewFunction(cx, noop, 1, 0, NULL, NULL))); // doc: If you do not assign a name to the function, it is assigned the name "anonymous".
	JS_SetProperty(cx, configObject, NAME_CONFIGURATION_STDOUT, &value);

	value = JSVAL_TRUE; // enable unsafe mode by default
	JS_SetProperty(cx, configObject, NAME_CONFIGURATION_UNSAFE_MODE, &value);

// arguments
	JS_DefineProperty(cx, globalObject, NAME_GLOBAL_ARGUMENT, STRING_TO_JSVAL(JS_NewStringCopyZ(cx, lpCmdLine)), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineProperty(cx, globalObject, NAME_GLOBAL_SCRIPT_HOST_NAME, STRING_TO_JSVAL(JS_NewStringCopyZ(cx, name)), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineProperty(cx, globalObject, NAME_GLOBAL_SCRIPT_HOST_PATH, STRING_TO_JSVAL(JS_NewStringCopyZ(cx, moduleFileName)), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineProperty(cx, globalObject, NAME_GLOBAL_FIRST_INSTANCE, BOOLEAN_TO_JSVAL(hasPrevInstance?JS_FALSE:JS_TRUE), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);

// options
	uint32 options = JSOPTION_VAROBJFIX | JSOPTION_XML | JSOPTION_COMPILE_N_GO;
	if ( /*!unsafeMode*/ false )
		options |= JSOPTION_STRICT;
	JS_SetOptions(cx, options );

	JSScript *script = JS_CompileFile( cx, globalObject, scriptName );
	if ( script == NULL )
		return -1; // script not found

	jsval rval;
	jsStatus = JS_ExecuteScript( cx, globalObject, script, &rval ); // the script MUST be executed only once because JSOPTION_COMPILE_N_GO is set.
	if ( jsStatus == JS_FALSE )
		return -2;
	JS_DestroyScript( cx, script );

	ModuleReleaseAll(cx);

  JS_DestroyContext(cx);
  JS_DestroyRuntime(rt);
  JS_ShutDown();

  // Beware: because JS engine allocate memory from the DLL, all memory must be disallocated before releasing the DLL
  ModuleFreeAll();

  //ReleaseMutex
  CloseHandle( instanceCheckMutex );

  return 0;
}