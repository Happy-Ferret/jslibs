#include "stdafx.h"

//#include <stdio.h>
//#include <stdlib.h>
//#include <sys/types.h>
#include <sys/stat.h>
//#include <string.h>
//#include <time.h>
//#include <errno.h>
//#include <windows.h>

#define XP_WIN
#include <jsapi.h>

#include "jsxdrapi.h"

#include "../common/jshelper.h"

#include "configuration.h"

Configuration *configuration;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static JSBool global_seal(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT_ARGC(1);

	JSObject *target;
	JSBool deep = JS_FALSE;

	JSBool err;
	err = JS_ValueToObject( cx, argv[0], &target );
	RT_ASSERT( err == JS_TRUE, "unable to seal this object." );

	argv[0] = OBJECT_TO_JSVAL(target);

	if ( argc >= 2 ) { // strange: js> seal(o) => deep == true : it's because nargs in JS_DefineFunction

		err = JS_ValueToBoolean( cx, argv[1], &deep );
		RT_ASSERT( err == JS_TRUE, "unable to convert arg2 to bool." );
	}

	return JS_SealObject(cx, target, deep);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static JSBool global_clear(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT_ARGC(1);
	JSBool err;
	err = JS_ValueToObject(cx, argv[0], &obj);
	RT_ASSERT( err == JS_TRUE, "unable to clear this object." );
	JS_ClearScope(cx, obj);
  return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static JSBool global_hideProperties(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT_ARGC(2);
	JSObject *object;
	JSBool err;
	err = JS_ValueToObject( cx, argv[0], &object );
	RT_ASSERT( err == JS_TRUE, "unable to seal this object." );

	const char *propertyName;
	uintN attributes;
	JSBool found;
	for ( int i=1; i<argc; i++ ) {

		propertyName = JS_GetStringBytes( JS_ValueToString( cx, argv[i] ) );
		RT_ASSERT( propertyName != NULL, "invalid property." );
		JS_GetPropertyAttributes( cx, object, propertyName, &attributes, &found );
		if ( found == JS_FALSE )
			continue;
		attributes &= ~JSPROP_ENUMERATE;
		JS_SetPropertyAttributes( cx, object, propertyName, attributes, &found );
	}
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// note:
//  warning is reported on stderr ( jshost.exe test.js 2>NUL ) [TBD] update this note ?
static JSBool global_warning(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	JSString *jssMesage = JS_ValueToString(cx, argv[0]);
	argv[0] = STRING_TO_JSVAL(jssMesage);
  JS_ReportWarning( cx, "%s", JS_GetStringBytes(jssMesage) );
  return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static JSBool global_collectGarbage(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
/* needed ???
  #ifdef JS_THREADSAFE
	JS_BeginRequest( cx );
  #endif
*/
	JS_GC( cx );
/* needed ???
  #ifdef JS_THREADSAFE
	JS_EndRequest( cx );
  #endif
*/
  return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static JSBool global_print(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	JSString *str;
	uintN i;
	for (i = 0; i < argc; i++) {

		str = JS_ValueToString(cx, argv[i]);
		RT_ASSERT_1( str != NULL, "Unable to convert argument %d to string.", i );
		argv[i] = STRING_TO_JSVAL(str); // [TBD] needed ?
		configuration->stdOut( JS_GetStringBytes(str), JS_GetStringLength(str) );
	}
	return JS_TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// XDR and bytecode compatibility:
//   Backward compatibility is when you run old bytecode on a new engine, and that should work.
//   What you seem to want is forward compatibility, which is new bytecode on an old engine, which is nothing we've ever promised.
// year 2038 bug :
//   Later than midnight, January 1, 1970, and before 19:14:07 January 18, 2038, UTC ( see _stat64 )
// note:
//	You really want to use Script.prototype.thaw and Script.prototype.freeze.  At
//	least imitate their implementations in jsscript.c (script_thaw and
//	script_freeze).  But you might do better to call these via JS_CallFunctionName
//	on your script object.
//
//	/be
static JSScript* LoadScript(JSContext *cx, JSObject *obj, const char *fileName, bool useCompFile) {

  char compiledFileName[MAX_PATH];
  strcpy( compiledFileName, fileName );
  strcat( compiledFileName, "xdr" );

  struct _stat srcFileStat;
  bool hasSrcFile = ( _stat( fileName, &srcFileStat ) != -1 ) ; // errno == ENOENT

  struct _stat compFileStat;
  bool hasCompFile = ( _stat( compiledFileName, &compFileStat ) != -1 );

  bool compFileUpToDate = ( hasSrcFile && hasCompFile && (srcFileStat.st_mtime < compFileStat.st_mtime) ) || ( hasCompFile && !hasSrcFile );	// true if comp file is up to date or alone

  if ( !hasSrcFile && !hasCompFile ) {

    JS_ReportError( cx, "Unable to load Script, file \"%s\" or \"%s\" not found.", fileName, compiledFileName );
    return NULL;
  }

  JSScript *script;

  if ( useCompFile && compFileUpToDate ) {

    FILE *file = fopen(compiledFileName, "rb"); // b for binary ( win32 )
    if ( !file ) {

      JS_ReportError( cx, "Unable to open file \"%s\" for reading.", compiledFileName );
      return NULL;
    }

    int compFileSize = compFileStat.st_size;
    void *data = JS_malloc( cx, compFileSize );

    size_t readCount = fread( data, 1, compFileSize, file ); // here we can use "Memory-Mapped I/O Functions" ( http://developer.mozilla.org/en/docs/NSPR_API_Reference:I/O_Functions#Memory-Mapped_I.2FO_Functions )

		if ( readCount != compFileSize ) {
      JS_ReportError( cx, "Unable to read the file \"%s\" ", compiledFileName );
      return NULL;
    }

    fclose( file );

    JSXDRState *xdr = JS_XDRNewMem(cx,JSXDR_DECODE);
		if ( xdr == NULL )
			return NULL;
		JS_XDRMemSetData( xdr, data, compFileSize );
		JSBool xdrSuccess = JS_XDRScript( xdr, &script );
		if ( xdrSuccess != JS_TRUE )
			return NULL;
		// manage BIG_ENDIAN here ?
		JS_XDRMemSetData(xdr, NULL, 0);
    JS_XDRDestroy(xdr);
    JS_free( cx, data );
  } else {

    script = JS_CompileFile(cx, obj, fileName);

		if ( useCompFile && script != NULL ) {

			JSXDRState *xdr = JS_XDRNewMem(cx,JSXDR_ENCODE);
	    if (!xdr)
				return NULL;
			JSBool xdrSuccess = JS_XDRScript( xdr, &script );
			if ( xdrSuccess != JS_TRUE )
				return NULL;
			FILE *file  = fopen( compiledFileName, "wb" );
			if ( file != NULL ) { // if the file cannot be write, this is not an error ( eg. read-only drive )
				uint32 length;
				void *buf = JS_XDRMemGetData( xdr, &length );
				if ( buf == NULL )
					return NULL;
				// manage BIG_ENDIAN here ?
				fwrite( buf, 1, length, file );
				fclose(file);
			}
			JS_XDRDestroy( xdr );
		}
	}
  return script;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// function copied from mozilla/js/src/js.c
static JSBool global_exec(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

  uintN i;
  JSString *str;
  const char *filename;
  JSScript *script;
  JSBool ok;
  JSErrorReporter older;
  uint32 oldopts;

  RT_ASSERT_ARGC(1);
	bool saveCompiledScripts = true; // default
	if ( argc >= 2 && argv[1] != JSVAL_FALSE )
		saveCompiledScripts = false;

  str = JS_ValueToString(cx, argv[0]);
  RT_ASSERT( str != NULL, "unable to get the filename." );
  argv[0] = STRING_TO_JSVAL(str);
  filename = JS_GetStringBytes(str);
  errno = 0;
//        older = JS_SetErrorReporter(cx, LoadErrorReporter);
  oldopts = JS_GetOptions(cx);
  JS_SetOptions(cx, oldopts | JSOPTION_COMPILE_N_GO);
  // script = JS_CompileFile(cx, obj, filename);
  script = LoadScript( cx, obj, filename, saveCompiledScripts );
  if (!script) {
    ok = JS_FALSE;
  } else {
    ok = JS_ExecuteScript(cx, obj, script, rval); // rval = Pointer to the value from the last executed expression statement processed in the script.
    JS_DestroyScript(cx, script);
  }
  JS_SetOptions(cx, oldopts);
//        JS_SetErrorReporter(cx, older);
  if (!ok)
		return JS_FALSE;
  return JS_TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSFunctionSpec Global_FunctionSpec[] = { // *name, call, nargs, flags, extra
	{ "Seal"            , global_seal            , 0, 0, 0 },
	{ "Clear"           , global_clear           , 0, 0, 0 },
	{ "HideProperties"  , global_hideProperties  , 0, 0, 0 },
	{ "Exec"            , global_exec            , 0, 0, 0 },
	{ "Print"           , global_print           , 0, 0, 0 },
	{ "CollectGarbage"  , global_collectGarbage  , 0, 0, 0 },
	{ "Warning"         , global_warning         , 0, 0, 0 },
	{ 0 }
};


extern "C" __declspec(dllexport) JSBool ModuleInit(JSContext *cx, JSObject *obj) {

	configuration = GetConfiguration( cx );
	RT_ASSERT( configuration != NULL, "unable to read the configuration." );
	JS_DefineFunctions( cx, obj, Global_FunctionSpec );
	return JS_TRUE;
}


BOOL APIENTRY DllMain( HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved ) {

  switch (ul_reason_for_call) {

	  case DLL_PROCESS_ATTACH:
	  case DLL_THREAD_ATTACH:
	  case DLL_THREAD_DETACH:
	  case DLL_PROCESS_DETACH:
		  break;
  }
  return TRUE;
}

