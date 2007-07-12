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

#include "static.h"

#include "jsxdrapi.h"
#include "jscntxt.h"

#include <limits.h>

#ifndef PATH_MAX
	#define PATH_MAX FILENAME_MAX
#endif

#ifdef XP_UNIX
	#define MAX_PATH PATH_MAX
#endif

extern JSFunction *stdoutFunction;

BEGIN_STATIC

DEFINE_FUNCTION( ASSERT ) {

	RT_ASSERT_ARGC( 1 );

	bool assert;
	RT_JSVAL_TO_BOOL( argv[0], assert );

	if ( !assert ) {

		char *message;
		if ( argc >= 2 )
			RT_JSVAL_TO_STRING( argv[1], message )
		else
			message = "Assertion failed.";
		JS_ReportError( cx, message );
		return JS_FALSE;
	}
	return JS_TRUE;
}


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
	JSBool deep;
	RT_ASSERT_OBJECT(argv[0]);
	//RT_CHECK_CALL( JS_ValueToObject(cx, argv[0], &obj) );
	if ( argc >= 2 ) { // strange: js> seal(o) => deep == true : it's because nargs in JS_DefineFunction

		RT_CHECK_CALL( JS_ValueToBoolean( cx, argv[1], &deep ) )
	} else {

		deep = JS_FALSE;
	}
	return JS_SealObject(cx, JSVAL_TO_OBJECT(argv[0]), deep);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DEFINE_FUNCTION( Clear ) {

	RT_ASSERT_ARGC(1);
	RT_ASSERT_OBJECT(argv[0]);
	JS_ClearScope(cx, JSVAL_TO_OBJECT(argv[0]));
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DEFINE_FUNCTION( SetScope ) {

	RT_ASSERT_ARGC(2);

	JSObject *o;
	JS_ValueToObject(cx, argv[0], &o);

	JSObject *p;
	JS_ValueToObject(cx, argv[1], &p);

	*rval = OBJECT_TO_JSVAL(JS_GetParent(cx, o));

	RT_CHECK_CALL( JS_SetParent(cx, o, p) );

	return JS_TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DEFINE_FUNCTION( IsConstructing ) {
	
	*rval = JS_IsConstructing(cx) == JS_TRUE ? JSVAL_TRUE : JSVAL_FALSE;
	return JS_TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DEFINE_FUNCTION( HideProperties ) {

	RT_ASSERT_ARGC(2);
	JSObject *object;
	RT_CHECK_CALL( JS_ValueToObject( cx, argv[0], &object ) );
//	const char *propertyName;
//	uintN attributes;
	for ( uintN i=1; i<argc; i++ ) {

		jsid id;
		uintN attrs;
		JSObject *obj2;
		JSProperty *prop;

		RT_CHECK_CALL( JS_ValueToId(cx, argv[i], &id) );
		RT_CHECK_CALL( OBJ_LOOKUP_PROPERTY(cx, object, id, &obj2, &prop) );
		if (!prop || object != obj2) { // not found

			if (prop)
				OBJ_DROP_PROPERTY(cx, obj2, prop);
			REPORT_ERROR( "Invalid property name." );
		}
		RT_CHECK_CALL( OBJ_GET_ATTRIBUTES(cx, object, id, prop, &attrs) );
		attrs &= ~JSPROP_ENUMERATE;
		RT_CHECK_CALL( OBJ_SET_ATTRIBUTES(cx, object, id, prop, &attrs) );
		OBJ_DROP_PROPERTY(cx, object, prop);

/*
	JSBool found;
		...
		propertyName = JS_GetStringBytes( JS_ValueToString( cx, argv[i] ) );
		RT_ASSERT_1( propertyName != NULL, "Invalid property name (%s).", propertyName );
		RT_CHECK_CALL( JS_GetPropertyAttributes( cx, object, propertyName, &attributes, &found ) );
		if ( found == JS_FALSE )
			continue;
		attributes &= ~JSPROP_ENUMERATE;
		RT_CHECK_CALL( JS_SetPropertyAttributes( cx, object, propertyName, attributes, &found ) );
*/
	}
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DEFINE_FUNCTION( IdOf ) {

	jsid id;
	if ( JSVAL_IS_OBJECT(argv[0]) )
		JS_GetObjectId(cx, JSVAL_TO_OBJECT(argv[0]), &id);
	else
		JS_ValueToId(cx, argv[0], &id);
	JS_NewNumberValue(cx,(JSUword)id,rval); // src: ... JSDOUBLE_IS_INT(d, i) && INT_FITS_IN_JSVAL(i) ...
	return JS_TRUE;
}

/*
DEFINE_FUNCTION( test ) {

// toto(123) = 'test'
	*rval
	JS_SetCallReturnValue2(cx,
	return JS_TRUE;
}
*/

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


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DEFINE_PROPERTY( gcByte ) {

	JSRuntime *rt = cx->runtime;
	*vp = INT_TO_JSVAL(rt->gcBytes);
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DEFINE_FUNCTION( CollectGarbage ) {

	#ifdef JS_THREADSAFE
	JS_BeginRequest( cx ); // http://developer.mozilla.org/en/docs/JS_BeginRequest
	#endif

	JS_GC( cx );

	#ifdef JS_THREADSAFE
	JS_EndRequest( cx );
	#endif

	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DEFINE_FUNCTION( Print ) {

	if ( stdoutFunction == NULL )
		return JS_TRUE; // nowhere to write, but don't failed
	for (uintN i = 0; i<argc; i++)
		RT_CHECK_CALL( JS_CallFunction(cx, obj, stdoutFunction, 1, &argv[i], rval) );
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

	struct stat srcFileStat;
	bool hasSrcFile = ( stat( fileName, &srcFileStat ) != -1 ) ; // errno == ENOENT

	struct stat compFileStat;
	bool hasCompFile = ( stat( compiledFileName, &compFileStat ) != -1 );

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

		size_t compFileSize = compFileStat.st_size;
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
	if ( argc >= 2 && argv[1] == JSVAL_FALSE )
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

//		if ( argc >= 3 )
//			obj = JSVAL_TO_OBJECT( argv[2] ); // try Exec.call( obj1, 'test.js' ); ...
// see: http://groups.google.com/group/mozilla.dev.tech.js-engine/browse_thread/thread/97269b31d65d493d/be8a4f9c4e805bef

		ok = JS_ExecuteScript(cx, obj, script, rval); // Doc: On successful completion, rval is a pointer to a variable that holds the value from the last executed expression statement processed in the script.
		JS_DestroyScript(cx, script);
	}
	JS_SetOptions(cx, oldopts);
	//        JS_SetErrorReporter(cx, older);
	if (!ok)
		return JS_FALSE;
	return JS_TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
DEFINE_FUNCTION( IsStatementValid ) {

	RT_ASSERT_ARGC( 1 );
	char *buffer;
	int length;
	RT_JSVAL_TO_STRING_AND_LENGTH( argv[0], buffer, length );
	*rval = JS_BufferIsCompilableUnit(cx, obj, buffer, length) == JS_TRUE ? JSVAL_TRUE : JSVAL_FALSE;
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
DEFINE_FUNCTION( Halt ) {

	return JS_FALSE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CONFIGURE_STATIC

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION( ASSERT )
		FUNCTION( Expand )
		FUNCTION( Seal )
		FUNCTION( Clear )
		FUNCTION( SetScope )
		FUNCTION( IsConstructing )
		FUNCTION( HideProperties )
		FUNCTION( Exec )
		FUNCTION( IsStatementValid )
		FUNCTION( Print )
		FUNCTION( CollectGarbage )
		FUNCTION( Warning )
		FUNCTION( IdOf )
		FUNCTION( Halt )
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_READ( gcByte )
	END_STATIC_PROPERTY_SPEC

END_STATIC
