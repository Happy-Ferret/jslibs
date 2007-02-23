#include "stdafx.h"

#ifdef WIN32
 #define DLL_EXT ".dll"
#else
 #define DLL_EXT ".so"
#endif

#include "jsstddef.h"
#include <jsapi.h>

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
	RT_ASSERT( i < 32, "unable to load more libraries" );
	_moduleList[i] = module;
	typedef JSBool (*ModuleInitFunction)(JSContext *, JSObject *);
	ModuleInitFunction moduleInit = (ModuleInitFunction)::GetProcAddress( module, "ModuleInit" ); // (argc>1) ? JS_GetStringBytes(JS_ValueToString(cx, argv[1])) : 
	RT_ASSERT_1( moduleInit != NULL, "Module initialization function not found in %s.", libFileName );
	*rval = moduleInit( cx, obj ) == JS_TRUE ? JSVAL_TRUE : JSVAL_FALSE;
	return JS_TRUE;
}


JSBool global_getter_global(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
	
	*vp = OBJECT_TO_JSVAL(JS_GetGlobalObject(cx));
	return JS_TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////////
JSPropertySpec Global_PropertySpec[] = { // *name, tinyid, flags, getter, setter
	{ "global"    , 0    , JSPROP_SHARED | JSPROP_READONLY | JSPROP_PERMANENT, global_getter_global, NULL },
	{ 0 }
};

static JSBool stdoutFunction(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	JSString *str;
	str = JS_ValueToString(cx, argv[0]);
	RT_ASSERT( str != NULL, "Unable to convert argument to string.");
	argv[0] = STRING_TO_JSVAL(str); // (TBD) needed ?
//	consoleStdOut( JS_GetStringBytes(str), JS_GetStringLength(str) );
	return JS_TRUE;
}

int WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow ) {

	JSRuntime *rt;
	JSContext *cx;
	JSObject *globalObject;

	unsigned long maxbytes = 32L * 1024L * 1024L;
	unsigned long stackSize = 1L * 1024L * 1024L;

	rt = JS_NewRuntime(maxbytes); // maxbytes specifies the number of allocated bytes after which garbage collection is run.
	cx = JS_NewContext(rt, stackSize); // A context specifies a stack size for the script, the amount, in bytes, of private memory to allocate to the execution stack for the script.

	JS_SetVersion( cx, JSVERSION_1_7 );
	// (TBD) set into configuration file

	JSBool jsStatus;
// global object
	JSClass global_class = { "global", 0, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub };
	globalObject = JS_NewObject(cx, &global_class, NULL, NULL);

// Standard classes
	jsStatus = JS_InitStandardClasses(cx, globalObject);

// global functions & properties
	JS_DefineProperties( cx, globalObject, Global_PropertySpec );
	JS_DefineFunction( cx, globalObject, "LoadModule", global_loadModule, 0, 0 );

// Global configuration object
	JSObject *configObject;
	jsStatus = GetConfigurationObject(cx, &configObject);
	jsval value = OBJECT_TO_JSVAL(JS_GetFunctionObject(JS_NewFunction(cx, stdoutFunction, 1, 0, NULL, NULL))); // If you do not assign a name to the function, it is assigned the name "anonymous".
	jsStatus = JS_SetProperty(cx, configObject, "stdout", &value);
	value = JSVAL_TRUE;
	jsStatus = JS_SetProperty(cx, configObject, "unsafeMode", &value);

// arguments
	jsStatus = JS_DefineProperty(cx, globalObject, "arguments", STRING_TO_JSVAL(JS_NewStringCopyZ(cx, lpCmdLine)), NULL, NULL, 0);

	JS_SetOptions(cx, JSOPTION_VAROBJFIX | JSOPTION_STRICT | JSOPTION_XML | JSOPTION_COMPILE_N_GO );

	CHAR moduleFileName[MAX_PATH];
	DWORD len = GetModuleFileName(hInstance, moduleFileName, sizeof(moduleFileName));
	strcpy( moduleFileName + len - 3, "js" ); // construct the script name

	JSScript *script = JS_CompileFile( cx, globalObject, moduleFileName );
	if ( script == NULL )
		return -1; // script not found
	jsval rval;
	jsStatus = JS_ExecuteScript( cx, globalObject, script, &rval ); // MUST be executed only once ( JSOPTION_COMPILE_N_GO )
	JS_DestroyScript( cx, script );

	typedef void (*ModuleReleaseFunction)(JSContext *cx);
	for ( int i = sizeof(_moduleList) / sizeof(*_moduleList) - 1; i >= 0; --i ) // beware: 'i' must be signed // start from the end
		if ( _moduleList[i] != NULL ) {

			ModuleReleaseFunction moduleRelease = (ModuleReleaseFunction)::GetProcAddress( _moduleList[i], "ModuleRelease" );
			if ( moduleRelease != NULL )
				moduleRelease(cx);
		}

// cleanup
  // For each context you've created
  JS_DestroyContext(cx); // (TBD) is JS_DestroyContextNoGC faster ?
  // For each runtime
  JS_DestroyRuntime(rt);
  // And finally
  JS_ShutDown();
// Beware: because JS engine allocate memory from the DLL, all memory must be disallocated before releasing the DLL
	// free used modules
	typedef void (*ModuleFreeFunction)(void);
	for ( int i = sizeof(_moduleList) / sizeof(*_moduleList) - 1; i >= 0; --i ) // beware: 'i' must be signed // start from the end
		if ( _moduleList[i] != NULL ) {

			ModuleFreeFunction moduleFree = (ModuleFreeFunction)::GetProcAddress( _moduleList[i], "ModuleFree" );
			if ( moduleFree != NULL )
				moduleFree();
			::FreeLibrary(_moduleList[i]);
		}

  return 0;
}