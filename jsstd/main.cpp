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

#include <sys/stat.h>

#include "jsxdrapi.h"

#include "jscntxt.h"

#define JSHELPER_UNSAFE_DEFINED
#include "../common/jshelper.h"
#include "../common/jsclass.h"
#include "../configuration/configuration.h"

#include "../common/stack.h"


DEFINE_UNSAFE_MODE;

extern JSFunction *stdoutFunction = NULL;


BEGIN_STATIC


DEFINE_FUNCTION( Expand ) {

	RT_ASSERT_ARGC( 1 );

	char *srcBegin;
	int srcLen;
	RT_JSVAL_TO_STRING_AND_LENGTH( argv[0], srcBegin, srcLen );
	char *srcEnd = srcBegin + srcLen;

	JSObject *table;
	if ( argc >= 2 ) {

		RT_ASSERT_OBJECT( argv[1] );
		table = JSVAL_TO_OBJECT( argv[1] );
	} else {

		table = obj;
	}

	typedef struct {

		const char *data;
		long int length;
	} Chunk;

	int totalLength = 0;

	void *stack;
	StackInit( &stack );
	Chunk *chunk;
	char *tok;
	jsval val;

	while (true) {

		tok = strstr(srcBegin, "$(");

		if ( tok == NULL ) {

			chunk = (Chunk*)malloc(sizeof(Chunk));
			chunk->data = srcBegin;
			chunk->length = srcEnd - srcBegin;
			totalLength += chunk->length;
			StackPush( &stack, chunk );
			break;
		}

		chunk = (Chunk*)malloc(sizeof(Chunk));
		chunk->data = srcBegin;
		chunk->length = tok - srcBegin;
		totalLength += chunk->length;
		StackPush( &stack, chunk );

		srcBegin = tok + 2;
//		if ( srcBegin >= srcEnd )
//			break;

		tok = strstr(srcBegin, ")");

		if ( tok == NULL )
			break;

		char tmp = *tok;
		*tok = 0;
		JS_GetProperty(cx, table, srcBegin, &val);
		*tok = tmp;

		chunk = (Chunk*)malloc(sizeof(Chunk));
		RT_JSVAL_TO_STRING_AND_LENGTH( val, chunk->data, chunk->length );
		totalLength += chunk->length;
		StackPush( &stack, chunk );

		srcBegin = tok + 1; // (TBD) check buffer overflow
//		if ( srcBegin >= srcEnd )
//			break;
	}

	StackReverse(&stack);

	char *expandedString = (char*)JS_malloc(cx, totalLength);

	char *tmp = expandedString;
	while ( !StackIsEnd(&stack) ) {

		Chunk *chunk = (Chunk*)StackPop(&stack);
		memcpy(tmp, chunk->data, chunk->length);
		tmp += chunk->length;
		free(chunk);
	}

	*rval = STRING_TO_JSVAL( JS_NewString(cx, expandedString, totalLength) );


//	js_GetFrameCallObject
	return JS_TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DEFINE_FUNCTION( Seal ) {

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
DEFINE_FUNCTION( Clear ) {

	RT_ASSERT_ARGC(1);
	JSBool err;
	err = JS_ValueToObject(cx, argv[0], &obj);
	RT_ASSERT( err == JS_TRUE, "unable to clear this object." );
	JS_ClearScope(cx, obj);
  return JS_TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DEFINE_FUNCTION( HideProperties ) {

	RT_ASSERT_ARGC(2);
	JSObject *object;
	JSBool err;
	err = JS_ValueToObject( cx, argv[0], &object );
	RT_ASSERT( err == JS_TRUE, "unable to seal this object." );

	const char *propertyName;
	uintN attributes;
	JSBool found;
	for ( uintN i=1; i<argc; i++ ) {

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
//  warning is reported on stderr ( jshost.exe test.js 2>NUL )
// (TBD) update this note ?
DEFINE_FUNCTION( Warning ) {

	JSString *jssMesage = JS_ValueToString(cx, argv[0]);
	argv[0] = STRING_TO_JSVAL(jssMesage);
	JS_ReportWarning( cx, "%s", JS_GetStringBytes(jssMesage) );
	return JS_TRUE;
}


DEFINE_PROPERTY( gcByte ) {

    JSRuntime *rt = cx->runtime;
	*vp = INT_TO_JSVAL(rt->gcBytes);
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DEFINE_FUNCTION( CollectGarbage ) {
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
DEFINE_FUNCTION( Print ) {

	if ( stdoutFunction == NULL )
		return JS_TRUE; // nowhere to write, but don't failed

	for (uintN i = 0; i<argc; i++)
		if (JS_CallFunction(cx, obj, stdoutFunction, 1, &argv[i], rval) == JS_FALSE)
			return JS_FALSE;
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
		// (TBD) use open/close/read/... instead of fopen/fclose/fread/...
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

		fclose( file ); // (TBD) use open/close/read/... instead of fopen/fclose/fread/...

		JSXDRState *xdr = JS_XDRNewMem(cx,JSXDR_DECODE);
		if ( xdr == NULL )
			return NULL;
		JS_XDRMemSetData( xdr, data, compFileSize );
		JSBool xdrSuccess = JS_XDRScript( xdr, &script );
		if ( xdrSuccess != JS_TRUE )
			return NULL;
		// (TBD) manage BIG_ENDIAN here ?
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
			FILE *file  = fopen( compiledFileName, "wb" ); // (TBD) use open/close/read/... instead of fopen/fclose/fread/...
			if ( file != NULL ) { // if the file cannot be write, this is not an error ( eg. read-only drive )

				uint32 length;
				void *buf = JS_XDRMemGetData( xdr, &length );
				if ( buf == NULL )
					return NULL;
				// manage BIG_ENDIAN here ?
				fwrite( buf, 1, length, file ); // (TBD) use open/close/read/... instead of fopen/fclose/fread/...
				fclose(file); // (TBD) use open/close/read/... instead of fopen/fclose/fread/...
			}
			JS_XDRDestroy( xdr );
		}
	}
	return script;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// function copied from mozilla/js/src/js.c
DEFINE_FUNCTION( Exec ) {

//  uintN i;
  JSString *str;
  const char *filename;
  JSScript *script;
  JSBool ok;
//  JSErrorReporter older;
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

CONFIGURE_STATIC

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION( Expand )
		FUNCTION( Seal )
		FUNCTION( Clear )
		FUNCTION( HideProperties )
		FUNCTION( Exec )
		FUNCTION( Print )
		FUNCTION( CollectGarbage )
		FUNCTION( Warning )
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_READ( gcByte )
	END_STATIC_PROPERTY_SPEC

END_STATIC



extern "C" __declspec(dllexport) JSBool ModuleInit(JSContext *cx, JSObject *obj) {

	INIT_STATIC(cx, obj);

// read configuration
	jsval stdoutFunctionValue;
	JSBool jsStatus = GetConfigurationValue(cx, "stdout", &stdoutFunctionValue );
	RT_ASSERT( jsStatus != JS_FALSE, "Unable to read stdout function from configuration object." );

	stdoutFunction = JS_ValueToFunction(cx, stdoutFunctionValue); // returns NULL if the function is not defined
//	_unsafeMode = JSVAL_TO_BOOLEAN(GetConfigurationValue(cx, "unsafeMode")) == JS_TRUE;

	jsval unsafeModeValue;
	jsStatus = GetConfigurationValue(cx, "unsafeMode", &unsafeModeValue);
	RT_ASSERT( jsStatus != JS_FALSE, "Unable to read unsafeMode state from configuration object." );

	if ( unsafeModeValue != JSVAL_VOID && JSVAL_IS_BOOLEAN(unsafeModeValue) )
		SET_UNSAFE_MODE( JSVAL_TO_BOOLEAN(unsafeModeValue) == JS_TRUE );

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

