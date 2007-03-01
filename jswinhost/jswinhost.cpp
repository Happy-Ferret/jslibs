#include "stdafx.h"

#ifdef WIN32
 #define DLL_EXT ".dll"
#else
 #define DLL_EXT ".so"
#endif

#include "jsstddef.h"
#include <jsapi.h>

#include "jsprf.h"

#include "../common/jsNames.h"
#include "../common/jshelper.h"
#include "../configuration/configuration.h"

HMODULE _moduleList[32] = {NULL}; // do not manage the module list dynamicaly, we allow a maximum of 32 modules

static JSBool global_loadModule(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT_ARGC(1);
	char *fileName;
	RT_JSVAL_TO_STRING( argv[0], fileName );
	char libFileName[MAX_PATH];
	strcpy( libFileName, fileName );
	strcat( libFileName, DLL_EXT );
	HMODULE module = ::LoadLibrary(libFileName);
	RT_ASSERT_2( module != NULL, "Unable to load the library %s (error:%d).", libFileName, GetLastError() );
	int i;
	for ( i = 0; _moduleList[i]; ++i ); // find a free module slot
	RT_ASSERT( i < sizeof(_moduleList)/sizeof(HMODULE), "unable to load more libraries" );
	_moduleList[i] = module;
	typedef JSBool (*ModuleInitFunction)(JSContext *, JSObject *);
	ModuleInitFunction moduleInit = (ModuleInitFunction)::GetProcAddress( module, NAME_MODULE_INIT ); // (argc>1) ? JS_GetStringBytes(JS_ValueToString(cx, argv[1])) : 
	RT_ASSERT_1( moduleInit != NULL, "Module initialization function not found in %s.", libFileName );
	*rval = moduleInit( cx, obj ) == JS_TRUE ? JSVAL_TRUE : JSVAL_FALSE;
	return JS_TRUE;
}

static JSBool noop(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	return JS_TRUE;
}

int consoleStdOut( JSContext *cx, const char *data, int length ) {

	JSObject *obj;
	RT_CHECK_CALL( GetConfigurationObject(cx, &obj) );
	jsval functionVal;
	JS_GetProperty(cx, obj, "stdout", &functionVal);
	if ( functionVal != JSVAL_VOID ) {

		RT_ASSERT_FUNCTION( functionVal );
		JSString *str = JS_NewStringCopyN(cx, data, length);
		RT_ASSERT_ALLOC( str ); 
		jsval rval, arg = STRING_TO_JSVAL(str);
		RT_CHECK_CALL ( JS_CallFunctionValue(cx, obj, functionVal, 1, &arg, &rval) )
	}
	return length;
}

int consoleStdErr( JSContext *cx, const char *data, int length ) {

	JSObject *obj;
	RT_CHECK_CALL( GetConfigurationObject(cx, &obj) );
	jsval functionVal;
	JS_GetProperty(cx, obj, "stderr", &functionVal);
	if ( functionVal != JSVAL_VOID ) {

		RT_ASSERT_FUNCTION( functionVal );
		JSString *str = JS_NewStringCopyN(cx, data, length);
		RT_ASSERT_ALLOC( str ); 
		jsval rval, arg = STRING_TO_JSVAL(str);
		RT_CHECK_CALL( JS_CallFunctionValue(cx, obj, functionVal, 1, &arg, &rval) )
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

	unsigned long maxbytes = 32L * 1024L * 1024L;
	unsigned long stackSize = 1L * 1024L * 1024L;

	rt = JS_NewRuntime(maxbytes); // maxbytes specifies the number of allocated bytes after which garbage collection is run.
	cx = JS_NewContext(rt, stackSize); // A context specifies a stack size for the script, the amount, in bytes, of private memory to allocate to the execution stack for the script.

	JS_SetVersion( cx, JSVERSION_1_7 );
	// (TBD) set into configuration file

	JS_SetErrorReporter(cx, ErrorReporter);

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
	JS_DefineFunction(cx, globalObject, NAME_GLOBAL_FUNCTION_LOAD_MODULE, global_loadModule, 0, 0);

// Global configuration object
	JSObject *configObject;
	jsStatus = GetConfigurationObject(cx, &configObject);
	if ( jsStatus == JS_FALSE )
		return -1;

	jsval value;
	
	value = OBJECT_TO_JSVAL(JS_GetFunctionObject(JS_NewFunction(cx, noop, 1, 0, NULL, NULL))); // doc: If you do not assign a name to the function, it is assigned the name "anonymous".
	JS_SetProperty(cx, configObject, NAME_CONFIGURATION_STDERR, &value);

	value = OBJECT_TO_JSVAL(JS_GetFunctionObject(JS_NewFunction(cx, noop, 1, 0, NULL, NULL))); // doc: If you do not assign a name to the function, it is assigned the name "anonymous".
	JS_SetProperty(cx, configObject, NAME_CONFIGURATION_STDOUT, &value);

	value = JSVAL_TRUE; // enable unsafe mode
	JS_SetProperty(cx, configObject, NAME_CONFIGURATION_UNSAFE_MODE, &value);

// arguments
	JS_DefineProperty(cx, globalObject, NAME_GLOBAL_ARGUMENT, STRING_TO_JSVAL(JS_NewStringCopyZ(cx, lpCmdLine)), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
	
	CHAR moduleFileName[MAX_PATH];
	DWORD len = GetModuleFileName(hInstance, moduleFileName, sizeof(moduleFileName));
	if ( len == 0 )
		return -1;
	char *name = strrchr( moduleFileName, '\\' );
	if ( name == NULL )
		return -1;
	*name = '\0';
	name++;
	
	JS_DefineProperty(cx, globalObject, NAME_GLOBAL_SCRIPT_HOST_NAME, STRING_TO_JSVAL(JS_NewStringCopyZ(cx, name)), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineProperty(cx, globalObject, NAME_GLOBAL_SCRIPT_HOST_PATH, STRING_TO_JSVAL(JS_NewStringCopyZ(cx, moduleFileName)), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);

// options
	uint32 options = JSOPTION_VAROBJFIX | JSOPTION_XML | JSOPTION_COMPILE_N_GO;
	if ( /*!unsafeMode*/ false )
		options |= JSOPTION_STRICT;
	JS_SetOptions(cx, options );

	CHAR scriptName[MAX_PATH];
	DWORD scriptNameLen = GetModuleFileName(hInstance, scriptName, sizeof(scriptName));
	strcpy( scriptName + scriptNameLen - 3, "js" ); // construct the script name

	JSScript *script = JS_CompileFile( cx, globalObject, scriptName );
	if ( script == NULL )
		return -1; // script not found

	jsval rval;
	jsStatus = JS_ExecuteScript( cx, globalObject, script, &rval ); // the script MUST be executed only once because JSOPTION_COMPILE_N_GO is set.
	if ( jsStatus == JS_FALSE )
		return -2;
	JS_DestroyScript( cx, script );

	typedef void (*ModuleReleaseFunction)(JSContext *cx);
	for ( int i = sizeof(_moduleList) / sizeof(*_moduleList) - 1; i >= 0; --i ) // beware: 'i' must be signed because we start from the end
		if ( _moduleList[i] != NULL ) {

			ModuleReleaseFunction moduleRelease = (ModuleReleaseFunction)::GetProcAddress( _moduleList[i], NAME_MODULE_RELEASE );
			if ( moduleRelease != NULL )
				moduleRelease(cx);
		}

  JS_DestroyContext(cx);
  JS_DestroyRuntime(rt);
  JS_ShutDown();

  // Beware: because JS engine allocate memory from the DLL, all memory must be disallocated before releasing the DLL
	typedef void (*ModuleFreeFunction)(void);
	for ( int i = sizeof(_moduleList) / sizeof(*_moduleList) - 1; i >= 0; --i ) // beware: 'i' must be signed // start from the end
		if ( _moduleList[i] != NULL ) {

			ModuleFreeFunction moduleFree = (ModuleFreeFunction)::GetProcAddress( _moduleList[i], NAME_MODULE_FREE );
			if ( moduleFree != NULL )
				moduleFree();
			::FreeLibrary(_moduleList[i]);
		}

  return 0;
}