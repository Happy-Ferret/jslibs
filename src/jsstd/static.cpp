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

#include <string.h>

#include "static.h"

#include "jsxdrapi.h"
#include "jscntxt.h"
#include <jsdbgapi.h>

#include "../common/buffer.h"


//#ifndef PATH_MAX
//	#define PATH_MAX FILENAME_MAX
//#endif

#ifdef XP_UNIX
	#define MAX_PATH PATH_MAX
	#define O_BINARY 0
#endif

/**doc fileIndex:topmost **/

DECLARE_CLASS( OperationLimit )
DECLARE_CLASS( Sandbox )


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_STATIC


/**doc
=== Static functions ===
**/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $STR $INAME( str [, obj] )
  Return an expanded string using key/value stored in _obj_.
  $LF
  If _obj_ is omitted, the current object is used to look for key/value.
  $H example
  {{{
  function Test() {
   this.Expand = Expand;
   this.a = 123;
  }
  Print( new Test().Expand('$(a)') );
  }}}
  $H note
   undefined values are ignored in the resulting string.
  $H example
  {{{
  Expand(' $(h) $(w)', { h:'Hello', w:'World' }); // returns "Hello World"
  }}}
**/
DEFINE_FUNCTION( Expand ) {

	J_S_ASSERT_ARG_MIN( 1 );

	const char *srcBegin;
	size_t srcLen;

	J_CHK( JsvalToStringAndLength(cx, &J_ARG(1), &srcBegin, &srcLen) );
	J_S_ASSERT( srcBegin[srcLen] == '\0', "Invalid input string." ); // else strstr may failed.
	const char *srcEnd;
	srcEnd = srcBegin + srcLen;

	JSObject *table;
	if ( J_ARG_ISDEF(2) ) {

		J_S_ASSERT_OBJECT( J_ARG(2) );
		table = JSVAL_TO_OBJECT( J_ARG(2) );
	} else {

		table = obj;
	}

	typedef struct {

		const char *data;
		size_t length;
	} Chunk;

	void *stack;
	jl::StackInit( &stack );
	Chunk *chunk;
	const char *tok;
	jsval val;
	int totalLength;
	totalLength = 0;

	while ( *srcBegin != '\0' ) {

		tok = strstr(srcBegin, "$(");
		if ( tok == NULL ) { // not found

			chunk = (Chunk*)malloc(sizeof(Chunk));
			chunk->data = srcBegin;
			chunk->length = srcEnd - srcBegin;
			totalLength += chunk->length;
			jl::StackPush( &stack, chunk );
			break;
		}

		chunk = (Chunk*)malloc(sizeof(Chunk));
		chunk->data = srcBegin;
		chunk->length = tok - srcBegin;
		totalLength += chunk->length;
		jl::StackPush( &stack, chunk );

		srcBegin = tok + 2; // length of "$("
		tok = strchr(srcBegin, ')'); // tok = strstr(srcBegin, ")"); // slower for only one char
		if ( tok == NULL ) // not found
			break;

		// (TBD) try to replace the following code
		char tmp = *tok;
		*((char*)tok) = 0;
		J_CHK( JS_GetProperty(cx, table, srcBegin, &val) );
		*((char*)tok) = tmp;

		if ( !JSVAL_IS_VOID( val ) ) {

			chunk = (Chunk*)malloc(sizeof(Chunk));
			J_CHK( JsvalToStringAndLength(cx, &val, &chunk->data, &chunk->length) ); // warning: GC on the returned buffer !
			totalLength += chunk->length;
			jl::StackPush( &stack, chunk );
		}

		srcBegin = tok + 1; // length of ")"
	}

	char *expandedString;
	expandedString = (char*)JS_malloc(cx, totalLength +1);
	J_S_ASSERT_ALLOC( expandedString );
	expandedString[totalLength] = '\0';

	expandedString += totalLength;
	while ( !jl::StackIsEnd(&stack) ) {

		Chunk *chunk = (Chunk*)jl::StackPop(&stack);
		expandedString -= chunk->length;
		memcpy(expandedString, chunk->data, chunk->length);
		free(chunk);
	}

	JSString *jsstr;
	jsstr = JS_NewString(cx, expandedString, totalLength);
	J_S_ASSERT_ALLOC( jsstr );
	*rval = STRING_TO_JSVAL( jsstr );
	return JS_TRUE;
	JL_BAD;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( string )
  Make an interned string, a string that is automatically shared with other code that needs a string with the same value.
**/
// source: http://mxr.mozilla.org/mozilla/source/js/src/js.c
DEFINE_FUNCTION_FAST( InternString ) {

	JSString *str;
	str = JS_ValueToString(cx, vp[2]);
	if (!str)
		return JS_FALSE;
	if (!JS_InternUCStringN(cx, JS_GetStringChars(str), J_STRING_LENGTH(str))) {

		return JS_FALSE;
	}
	*vp = JSVAL_VOID;
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( obj [ , recursively  ] )
  Prevents all write access to the object, either to add a new property, delete an existing property, or set the value or attributes of an existing property.
  If _recursively_ is true, the function seal any non-null objects in the graph connected to obj's slots.
**/
DEFINE_FUNCTION( Seal ) {

	J_S_ASSERT_ARG_MIN(1);
	J_S_ASSERT_OBJECT( J_ARG(1) );
	//J_CHK( JS_ValueToObject(cx, J_ARG(1), &obj) );
	JSBool deep;
	if ( J_ARG_ISDEF(2) )
		J_CHK( JS_ValueToBoolean(cx, J_ARG(2), &deep) );
	else
		deep = JS_FALSE;
	return JS_SealObject(cx, JSVAL_TO_OBJECT(J_ARG(1)), deep);
	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( obj )
  Removes all properties and elements from _obj_ in a single operation.
**/
DEFINE_FUNCTION( Clear ) {

	J_S_ASSERT_ARG_MIN( 1 );
//	J_S_ASSERT_OBJECT(J_ARG(1));
	if ( JSVAL_IS_OBJECT( J_ARG(1) ) ) {

		JS_ClearScope(cx, JSVAL_TO_OBJECT( J_ARG(1) ));
		*rval = JSVAL_TRUE;
	} else
		*rval = JSVAL_FALSE;
	return JS_TRUE;
	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $OBJ $INAME( obj, scopeObject )
  Set the scope object of _obj_.
  $H example 1
  {{{
  function foo() {

   var data = 55;
   function bar() { Print( data, '\n' ); }
   bar();
   var old = SetScope( bar, {data:7} );
   bar();
   var old = SetScope( bar, old );
   bar();
  }
  foo();
  }}}
  prints:
   $F 55
   $F 7
   $F 55

  $H example 2
  {{{
  LoadModule('jsstd');

  function Bind(fun, obj) {

   SetScope(obj, SetScope(fun, obj));
  }


  function MyClass() {

   this.Test = function() {

    foo();
   }

   this.foo = function() {

    Print('foo\n');
   }

   Bind(this.Test, this);
  }

  var myObj = new MyClass();
  myObj.Test(); // prints: foo
  var fct = myObj.Test;
  fct(); // prints: foo
  }}}
**/
DEFINE_FUNCTION( SetScope ) {

	J_S_ASSERT_ARG_MIN( 2 );
	JSObject *o, *p;
	J_CHK( JS_ValueToObject(cx, J_ARG(1), &o) );
	J_CHK( JS_ValueToObject(cx, J_ARG(2), &p) );
	*rval = OBJECT_TO_JSVAL(JS_GetParent(cx, o));
	J_CHK( JS_SetParent(cx, o, p) );
	return JS_TRUE;
	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( obj, propertyName1 [, propertyName2 [, ... ] ] )
  Hide properties from for-in loop.
  $H example
  {{{
  var obj = { a:11, b:22, c:33 };
  for ( var p in obj ) Print(p, ', '); // prints: a, b, c
  HideProperties(obj, 'b');
  for ( var p in obj ) Print(p, ', '); // prints: a, c
  }}}
**/
DEFINE_FUNCTION( HideProperties ) {

	J_S_ASSERT_ARG_MIN( 2 );
	JSObject *object;
	J_CHK( JS_ValueToObject( cx, J_ARG(1), &object ) );
//	const char *propertyName;
//	uintN attributes;
	for ( uintN i = 1; i < J_ARGC; i++ ) {

		jsid id;
		uintN attrs;
		JSObject *obj2;
		JSProperty *prop;

		J_CHK( JS_ValueToId(cx, J_ARG(1+i), &id) );
		J_CHK( OBJ_LOOKUP_PROPERTY(cx, object, id, &obj2, &prop) );
		if (!prop || object != obj2) { // not found

			if (prop)
				OBJ_DROP_PROPERTY(cx, obj2, prop);
			J_REPORT_ERROR( "Invalid property name." );
		}
		J_CHK( OBJ_GET_ATTRIBUTES(cx, object, id, prop, &attrs) );
		attrs &= ~JSPROP_ENUMERATE;
		J_CHK( OBJ_SET_ATTRIBUTES(cx, object, id, prop, &attrs) );
		OBJ_DROP_PROPERTY(cx, object, prop);

/*
	JSBool found;
		...
		propertyName = JS_GetStringBytes( JS_ValueToString( cx, J_ARG(i+1) ) );
		J_S_ASSERT_1( propertyName != NULL, "Invalid property name (%s).", propertyName );
		J_CHK( JS_GetPropertyAttributes( cx, object, propertyName, &attributes, &found ) );
		if ( found == JS_FALSE )
			continue;
		attributes &= ~JSPROP_ENUMERATE;
		J_CHK( JS_SetPropertyAttributes( cx, object, propertyName, attributes, &found ) );
*/
	}
	return JS_TRUE;
	JL_BAD;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $INT $INAME( object )
  Returns an integer value that is a unique identifier of the object _object_ .
  $H example
  {{{
  var myObj = {};
  Print( ObjectToId(myObj), '\n' );
  }}}
**/

struct ObjId {
	JSObject *obj;
	unsigned int id;
};

ObjId *objIdList = NULL;
unsigned int lastObjectId = 0;
unsigned int objectIdAlloced = 0;

JSGCCallback prevObjectIdGCCallback = NULL;

JSBool ObjectIdGCCallback(JSContext *cx, JSGCStatus status) {
	
	if ( status == JSGC_MARK_END ) {

		for ( ObjId *it = objIdList, *end = objIdList + objectIdAlloced; it < end; ++it ) {

			if ( it->obj && JS_IsAboutToBeFinalized(cx, it->obj) ) {

				it->id = 0;
				it->obj = NULL;
			}
		}
	}

	return prevObjectIdGCCallback ? prevObjectIdGCCallback(cx, status) : JS_TRUE;
}


DEFINE_FUNCTION_FAST( ObjectToId ) {

	J_S_ASSERT_ARG_RANGE( 1, 1 );
	J_S_ASSERT_OBJECT( J_FARG(1) );
	JSObject *obj = JSVAL_TO_OBJECT( J_FARG(1) );
	
	ObjId *freeSlot = NULL;
	for ( ObjId *it = objIdList, *end = objIdList + objectIdAlloced; it < end; ++it ) {

		if ( it->obj == obj )
			return UIntToJsval(cx, it->id, J_FRVAL);
		if ( !freeSlot && it->id == 0 )
			freeSlot = it;
	}
	
	if ( !freeSlot ) {

		unsigned int prevAlloced = objectIdAlloced;

		if ( !objIdList ) {
			
			prevObjectIdGCCallback = JS_SetGCCallback(cx, ObjectIdGCCallback);
			objectIdAlloced = 32;
		} else {

			objectIdAlloced *= 2;
		}
		objIdList = (ObjId*)JS_realloc(cx, objIdList, sizeof(ObjId) * objectIdAlloced); // (TBD) free objIdList at the end
		freeSlot = objIdList + prevAlloced;
		memset(freeSlot, 0,(objectIdAlloced - prevAlloced) * sizeof(ObjId)); // init only new slots
	}

	freeSlot->id = ++lastObjectId;
	freeSlot->obj = obj;

	return UIntToJsval(cx, lastObjectId, J_FRVAL);
	JL_BAD;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $INT $INAME( id )
  Returns the object with the identifier _id_ or undefined if the identifier do not exist or the object has been GCed. 
  $H example 1
  {{{
  var myObj = {};
  Print( IdToObject(ObjectToId(myObj)) === myObj, '\n' ); // prints true
  }}}
  $H example 2
  {{{
  var id = ObjectToId({});
  Print( IdToObject(id), '\n' ); // prints: [object Object]
  CollectGarbage();
  Print( IdToObject(id), '\n' ); // prints: undefined
  }}}
**/
DEFINE_FUNCTION_FAST( IdToObject ) {

	J_S_ASSERT_ARG_RANGE( 1, 1 );
	J_S_ASSERT_NUMBER( J_FARG(1) );

	unsigned int id;
	J_CHK( JsvalToUInt(cx, J_FARG(1), &id) );

	if ( id > 0 && id <= lastObjectId ) {

		for ( ObjId *it = objIdList, *end = objIdList + objectIdAlloced; it < end; ++it ) {

			if ( it->id == id ) {

				*J_FRVAL = OBJECT_TO_JSVAL( it->obj );
				return JS_TRUE;
			}
		}
	}

	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $TYPE Blob $INAME( value )
  Encode (serialize) a JavaScript value into an XDR (eXternal Data Representation) blob.
  $H note
   All JavaScript values cannot be encoded into XDR. If the function failed to encode a value, an error is raised. The Map object can help you to encode Object and Array.
**/
#ifdef JS_HAS_XDR
DEFINE_FUNCTION_FAST( XdrEncode ) {

	J_S_ASSERT_ARG_MIN( 1 );
	JSXDRState *xdr;
	xdr = JS_XDRNewMem(cx, JSXDR_ENCODE);
	J_S_ASSERT_ALLOC(xdr);
	J_CHK( JS_XDRValue(xdr, &J_FARG(1)) );
	uint32 length;
	void *buffer;
	buffer = JS_XDRMemGetData(xdr, &length);
	J_S_ASSERT( buffer != NULL, "Invalid xdr data." );
	J_CHK( J_NewBlobCopyN(cx, buffer, length, J_FRVAL) );
	JS_XDRDestroy(xdr);
	return JS_TRUE;
	JL_BAD;
}
#endif // JS_HAS_XDR


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $VAL $INAME( xdrBlob )
  Decode (deserialize) XDR (eXternal Data Representation) blob to a JavaScript value.
  $H note
   Decoding malformed XDR data can lead the program to crash. This may be a security issue.
**/
#ifdef JS_HAS_XDR
DEFINE_FUNCTION_FAST( XdrDecode ) {

	J_S_ASSERT_ARG_MIN( 1 );

	JSXDRState *xdr;
	xdr = JS_XDRNewMem(cx, JSXDR_DECODE);
	J_S_ASSERT_ALLOC(xdr);
	const char *buffer;
	size_t length;
	J_CHK( JsvalToStringAndLength(cx, &J_FARG(1), &buffer, &length) );
	JS_XDRMemSetData(xdr, (void*)buffer, length); // safe de-const cast: we are JSXDR_DECODE from the buffer.
	J_CHK( JS_XDRValue(xdr, J_FRVAL) );
	JS_XDRMemSetData(xdr, NULL, 0);
	JS_XDRDestroy(xdr);
	return JS_TRUE;
	JL_BAD;
}
#endif // JS_HAS_XDR


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( text )
  Report the given _text_ as warning. The warning is reported on the stderr. Warnings ignored in unsafeMode.
**/
DEFINE_FUNCTION_FAST( Warning ) {

	J_S_ASSERT_ARG_MIN( 1 );
	JSString *jssMesage;
	jssMesage = JS_ValueToString(cx, J_FARG(1));
	J_S_ASSERT_ALLOC( jssMesage );
	J_FARG(1) = STRING_TO_JSVAL(jssMesage);
	JS_ReportWarning( cx, "%s", JS_GetStringBytes(jssMesage) );
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( expression [, failureMessage ] )
  If the argument expression compares equal to zero, the failureMessage is written to the standard error device and the program stops its execution.
  $H example
  {{{
  var foo = ['a', 'b', 'c'];
  ASSERT( i >= 0 || i < 3, 'Invalid value.' );
  Print( foo[i] );
  }}}
  $H note
   The error output can be redirected by redefining the _configuration.stderr function. see the Print() function.
**/
DEFINE_FUNCTION( ASSERT ) {

	bool assert;
	J_CHK( JsvalToBool(cx, J_ARG(1), &assert) );
	if ( !assert ) {

		const char *message;
		if ( J_ARG_ISDEF(2) )
			J_CHK( JsvalToString(cx, &J_ARG(2), &message) );
		else
			message = "Assertion failed.";
		JS_ReportError( cx, message );
		return JS_FALSE;
	}
	return JS_TRUE;
	JL_BAD;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Performs garbage collection in the JS memory pool.
**/
DEFINE_FUNCTION_FAST( CollectGarbage ) {

	#ifdef JS_THREADSAFE
	JS_BeginRequest( cx ); // http://developer.mozilla.org/en/docs/JS_BeginRequest
	#endif

	JS_GC( cx );

	#ifdef JS_THREADSAFE
	JS_EndRequest( cx );
	#endif

	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Performs a conditional garbage collection of JS objects, doubles, and strings that are no longer needed by a script executing.
  This offers the JavaScript engine an opportunity to perform garbage collection if needed.
**/
DEFINE_FUNCTION_FAST( MaybeCollectGarbage ) {

	#ifdef JS_THREADSAFE
	JS_BeginRequest( cx ); // http://developer.mozilla.org/en/docs/JS_BeginRequest
	#endif

	JS_MaybeGC( cx );

	#ifdef JS_THREADSAFE
	JS_EndRequest( cx );
	#endif

	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//DEFINE_FUNCTION_FAST( Now ) {
//
//	JS_NewNumberValue(cx, JS_Now(), &JS_RVAL(cx,vp));
//	return JS_TRUE;
//}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $REAL $INAME()
  Returns the current value of a high-resolution time counter in millisecond.
  The returned value is a relative time value.
**/
DEFINE_FUNCTION_FAST( TimeCounter ) {

	JS_NewNumberValue(cx, AccurateTimeCounter(), J_FRVAL);
	return JS_TRUE;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $STR $INAME( $STR str, $INT count )
  Returns the string that is _count_ times _str_.
  $H example
  {{{
  Print( StringRepeat('foo', 3) ); // prints: foofoofoo
  }}}
**/
DEFINE_FUNCTION_FAST( StringRepeat ) {

	J_S_ASSERT_ARG_MIN(2);

	size_t count;
	J_CHK( JsvalToUInt(cx, J_FARG(2), &count) );
	if ( count == 0 ) {

		*J_FRVAL = JS_GetEmptyStringValue(cx);
		return JS_TRUE;
	}

	const char *buf;
	size_t len;
	J_CHK( JsvalToStringAndLength(cx, &J_FARG(1), &buf, &len) ); // warning: GC on the returned buffer !

	if ( len == 0 ) {

		*J_FRVAL = JS_GetEmptyStringValue(cx);
		return JS_TRUE;
	}

	if ( count == 1 ) {

		*J_FRVAL = STRING_TO_JSVAL( JS_ValueToString(cx, J_FARG(1)) ); // force string conversion because we must return a string.
		return JS_TRUE;
	}

	size_t newLen;
	newLen = len * count;

	char *newBuf;
	newBuf = (char *)JS_malloc(cx, newLen +1);
	J_S_ASSERT_ALLOC(newBuf);
	newBuf[newLen] = '\0';

	if ( len == 1 ) {

		memset(newBuf, *buf, newLen);
	} else {

		char *tmp = newBuf;
		size_t i, j;
		for ( i=0; i<count; i++ )
			for ( j=0; j<len; j++ )
				*(tmp++) = buf[j];
	}

	JSString *jsstr;
	jsstr = JS_NewString(cx, newBuf, newLen);
	J_S_ASSERT_ALLOC(jsstr);
	*J_FRVAL = STRING_TO_JSVAL( jsstr );
	return JS_TRUE;
	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( value1 [, value2 [, ...]] )
  Display _val_ to the output (the screen by default).
  $H example
  {{{
  Print( 'Hello', ' ', 'World', '\n' ); // prints: Hello World
  }}}
  $H note
   The output can be redirected by redefining the _configuration.stdout function.
   $H example
   {{{
   LoadModule('jsstd');
   Print('foo\n'); // prints: foo
   _configuration.stdout = function() {}
   Print('bar\n'); // prints nothing
   }}}
**/
DEFINE_FUNCTION_FAST( Print ) {

	JSObject *globalObject = JS_GetGlobalObject(cx);
	jsval fct;
	J_CHK( GetConfigurationValue(cx, NAME_CONFIGURATION_STDOUT, &fct) );
	if ( JS_TypeOfValue(cx, fct) == JSTYPE_FUNCTION )
		J_CHK( JS_CallFunctionValue(cx, globalObject, fct, J_ARGC, J_FARGV, J_FRVAL) );
	*J_FRVAL = JSVAL_VOID;

/*
	if ( stdoutFunction == NULL )
		return JS_TRUE; // nowhere to write, but don't failed

	JS_SET_RVAL(cx, vp, JSVAL_VOID);

	for (uintN i = 0; i < J_ARGC; i++)
		J_CHK( JS_CallFunction(cx, J_FOBJ, stdoutFunction, 1, &J_FARG(1+i), J_FRVAL) );
*/
	return JS_TRUE;
	JL_BAD;
}



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

#ifdef JS_HAS_XDR

	char compiledFileName[PATH_MAX];
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

		int file = open(compiledFileName, O_RDONLY | O_BINARY);
		if ( file == -1 ) {

			JS_ReportError( cx, "Unable to open file \"%s\" for reading.", compiledFileName );
			return NULL;
		}

		size_t compFileSize = compFileStat.st_size;
//		size_t compFileSize = filelength(file);
		void *data = JS_malloc( cx, compFileSize );

		int readCount = read( file, data, compFileSize ); // here we can use "Memory-Mapped I/O Functions" ( http://developer.mozilla.org/en/docs/NSPR_API_Reference:I/O_Functions#Memory-Mapped_I.2FO_Functions )

		if ( readCount == -1 || readCount != compFileSize ) {

			JS_ReportError( cx, "Unable to read the file \"%s\" ", compiledFileName );
			return NULL;
		}

		close( file );

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

//		JS_GC(cx); // ...and also just before doing anything that requires compilation (since compilation disables GC until complete).
		script = JS_CompileFile(cx, obj, fileName);

		if ( useCompFile && script != NULL ) {

			JSXDRState *xdr = JS_XDRNewMem(cx,JSXDR_ENCODE);
			if (!xdr)
				return NULL;
			JSBool xdrSuccess = JS_XDRScript( xdr, &script );
			if ( xdrSuccess != JS_TRUE )
				return NULL;
			int file = open( compiledFileName, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0700 ); // (TBD) use open/close/read/... instead of fopen/fclose/fread/...
			if ( file != -1 ) { // if the file cannot be write, this is not an error ( eg. read-only drive )

				uint32 length;
				void *buf = JS_XDRMemGetData( xdr, &length );
				if ( buf == NULL )
					return NULL;
				// manage BIG_ENDIAN here ?
				write( file, buf, length );
				close(file);
			}
			JS_XDRDestroy( xdr );
		}
	}
	return script;

#else // JS_HAS_XDR

//	JS_GC(cx);  // ...and also just before doing anything that requires compilation (since compilation disables GC until complete).
	return JS_CompileFile(cx, obj, fileName);

#endif // JS_HAS_XDR
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $VAL $INAME( fileName [, useAndSaveCompiledScript = true] )
  Executes the script specified by _fileName_.
  If _useAndSaveCompiledScript_ is true, the function load and save a compiled version (using XDR format) of the script on the disk ( adding 'xrd' to _fileName_ ).
  If the compiled file is not found, the uncompiled version is used instead.
  $H return value
   returns the last evaluated statement of the script.
  $H example
  {{{
  var foo = Exec('constants.js'); // loads constants.js or constants.jsxrd if available.
  }}}
**/
// function copied from mozilla/js/src/js.c
DEFINE_FUNCTION_FAST( Exec ) {

	//  uintN i;
	JSScript *script;
	JSBool ok;
	//  JSErrorReporter older;
	uint32 oldopts;

	J_S_ASSERT_ARG_MIN( 1 );
	bool saveCompiledScripts;
	saveCompiledScripts = !( J_FARG_ISDEF(2) && J_FARG(2) == JSVAL_FALSE );

	errno = 0;
	//        older = JS_SetErrorReporter(cx, LoadErrorReporter);
	oldopts = JS_GetOptions(cx);
	JS_SetOptions(cx, oldopts | JSOPTION_COMPILE_N_GO);
	// script = JS_CompileFile(cx, obj, filename);
	const char *filename;
	J_CHK( JsvalToString(cx, &J_FARG(1), &filename) );

	script = LoadScript( cx, J_FOBJ, filename, saveCompiledScripts );
	if (!script) {
		ok = JS_FALSE;
	} else {

//		if ( argc >= 3 )
//			obj = JSVAL_TO_OBJECT( J_ARG(3) ); // try Exec.call( obj1, 'test.js' ); ...
// see: http://groups.google.com/group/mozilla.dev.tech.js-engine/browse_thread/thread/97269b31d65d493d/be8a4f9c4e805bef
		ok = JS_ExecuteScript(cx, J_FOBJ, script, J_FRVAL); // Doc: On successful completion, rval is a pointer to a variable that holds the value from the last executed expression statement processed in the script.
		JS_DestroyScript(cx, script);
	}
	JS_SetOptions(cx, oldopts);
	//        JS_SetErrorReporter(cx, older);
	if (!ok)
		return JS_FALSE;
	return JS_TRUE;
	JL_BAD;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $VAL $INAME( scriptCode [ , queryCallback ] [ , maxExecutionTime = 1000 ] )
  Evaluates the JavaScript code in a sandbox with a new set of standard classes (Object, Math, ...).
  $H arguments
   $ARG $STR scriptCode: the unsecured script code to be executed.
   $ARG $FUN queryCallback: this function may be called by the unsecured script to query miscellaneous information to the host script. For security reasons, the function can only return primitive values (no objects).
   $ARG $INT maxExecutionTime: if defined, an OperationLimit exception is thrown when _maxExecutionTime_ milliseconds are elapsed.
  $H return value
   the value of the last-executed expression statement processed in the script.
  $H example 1
  {{{
  function QueryCallback(val) {
   return val;
  }
  var res = SandboxEval('1 + 2 + Query(3)', QueryCallback);
  Print( res ); // prints: 6
  }}}
  $H example 2
  {{{
  var res = SandboxEval('Math');
  Print( res == Math ); // prints: false
  }}}
  $H example 3
   abort very-long-running scripts.
  {{{
  try {
   var res = SandboxEval('while (true);', undefined, 1000);
  } catch (ex if ex instanceof OperationLimit) {
   Print( 'script execution too long !' );
  }
  }}}
**/

struct SandboxContextPrivate {

	size_t maxExecutionTime;
	JSFunction *queryFunction;
};

static JSBool SandboxMaxOperationCallback(JSContext *cx) {

	JSObject *branchLimitExceptionObj = JS_NewObject( cx, classOperationLimit, NULL, NULL );
	JS_SetPendingException( cx, OBJECT_TO_JSVAL( branchLimitExceptionObj ) );
	return JS_FALSE;
}

JLThreadFuncDecl WatchDogThreadProc(void *threadArg) {

	JSContext *scx = (JSContext*)threadArg;
	SandboxContextPrivate *pv = (SandboxContextPrivate*)JS_GetContextPrivate(scx);
	size_t maxExecutionTime = pv->maxExecutionTime;
	SleepMilliseconds(maxExecutionTime);
	JS_TriggerOperationCallback(scx);
	return 0;
}


static JSBool SandboxQueryFunction(JSContext *cx, uintN argc, jsval *vp) {

	SandboxContextPrivate *pv = (SandboxContextPrivate*)JS_GetContextPrivate(cx);
	JSFunction *fun = pv->queryFunction;
	if ( fun ) {

		J_CHK( JS_CallFunction(cx, J_FOBJ, fun, argc, J_FARGV, J_FRVAL) );
		if ( !JSVAL_IS_PRIMITIVE(*J_FRVAL) ) { // for security reasons, you must return primitive values.

			JS_ReportError(cx, "Only primitive value can be used.");
			return JS_FALSE;
		}
	} else
		*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

DEFINE_FUNCTION_FAST( SandboxEval ) {

	J_S_ASSERT_ARG_MIN(1);

	JSContext *scx;
	scx = JS_NewContext(JS_GetRuntime(cx), 8192L); // see host/host.cpp
	if ( !scx ) {

		JS_ReportOutOfMemory(cx); // the only error that is not catchable
		return JS_FALSE;
	}
	JS_ToggleOptions(scx, JSOPTION_DONT_REPORT_UNCAUGHT | JSOPTION_VAROBJFIX | JSOPTION_COMPILE_N_GO | JSOPTION_RELIMIT | JSOPTION_JIT);

	JSObject *globalObject;
	globalObject = JS_NewObject(scx, classSandbox, NULL, NULL);
	if ( !globalObject ) {

		JS_DestroyContextNoGC(scx);
		JS_ReportOutOfMemory(cx); // the only error that is not catchable
		return JS_FALSE;
	}

	SandboxContextPrivate pv;
	JS_SetContextPrivate(scx, &pv);

	if ( J_FARG_ISDEF(2) ) {

		J_S_ASSERT_FUNCTION( J_FARG(2) );
		pv.queryFunction = JS_ValueToFunction(cx, J_FARG(2));
		J_CHK( JS_DefineFunction(scx, globalObject, "Query", (JSNative)SandboxQueryFunction, 1, JSFUN_FAST_NATIVE | JSPROP_PERMANENT | JSPROP_READONLY) );
	} else {

		pv.queryFunction = NULL;
	}

	if ( J_FARG_ISDEF(3) )
		J_CHK( JsvalToUInt(cx, J_FARG(3), &pv.maxExecutionTime) );
	else
		pv.maxExecutionTime = 1000; // default value

/*
	if ( J_FARG_ISDEF(2) ) {

		JSObject *aobj = JSVAL_TO_OBJECT( J_FARG(2) );
		JS_SetParent(scx, aobj, globalObject);
		globalObject = aobj;
	}
*/

	JSString *jsstr;
	jsstr = JS_ValueToString(cx, J_FARG(1));
	uintN srclen;
	srclen = JS_GetStringLength(jsstr);
	jschar *src;
	src = JS_GetStringChars(jsstr);

	JS_SetOperationCallback(scx, SandboxMaxOperationCallback);
	JLThreadHandler watchDogThread;
	watchDogThread = JLThreadStart(WatchDogThreadProc, scx); // (TBD) check the restult

	JS_SetGlobalObject(scx, globalObject);
	JSStackFrame *fp;
	fp = JS_GetScriptedCaller(cx, NULL);
	JSBool ok;
	ok = JS_EvaluateUCScript(scx, globalObject, src, srclen, fp->script->filename, JS_PCToLineNumber(cx, fp->script, fp->regs->pc), J_FRVAL);

	JLThreadCancel(watchDogThread);
	JLFreeThread(&watchDogThread);

//	JSPrincipals principals = { "sandbox context", NULL, NULL, 1, NULL, NULL };
//	JSBool ok = JS_EvaluateUCScriptForPrincipals(scx, globalObject, &principals, src, srclen, fp->script->filename, JS_PCToLineNumber(cx, fp->script, fp->regs->pc), J_FRVAL);

	if (!ok) {

		jsval ex;
		if (JS_GetPendingException(scx, &ex))
			JS_SetPendingException(cx, ex);
		else
			JS_ReportError(cx, "Unexpected error.");
	}

	JS_DestroyContextMaybeGC(scx);
	return ok;
	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME( statementString )
  Returns true if _statementString_ is a valid Javascript statement.
  The intent is to support interactive compilation, accumulate lines in a buffer until IsStatementValid returns true, then pass it to an eval.
  This function is useful to write an interactive console.
**/
DEFINE_FUNCTION( IsStatementValid ) {

	J_S_ASSERT_ARG_MIN( 1 );
	const char *buffer;
	size_t length;
	J_CHK( JsvalToStringAndLength(cx, &J_ARG(1), &buffer, &length) );
	J_CHK( BoolToJsval(cx, JS_BufferIsCompilableUnit(cx, obj, buffer, length) == JS_TRUE, rval) );
	return JS_TRUE;
	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Stop the execution of the program. This is a ungraceful way to finish a program and should only be used in critical cases.
**/
DEFINE_FUNCTION( Halt ) {

	return JS_FALSE;
}

/*
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
DEFINE_FUNCTION( StrSet ) {

	J_S_ASSERT_ARG_MIN( 2 );

	const char *chr;
	size_t charLen;
	J_JSVAL_TO_STRING_AND_LENGTH( J_ARG(1), chr, charLen );

	unsigned int count;
	J_CHK( JsvalToUInt(cx, J_ARG(2), &count) );

	char *buf = (char*)JS_malloc(cx, count +1);
	J_S_ASSERT_ALLOC( buf );
	buf[count] = '\0';

	memset(buf, chr[0], count);

	JSString *jsstr = JS_NewString(cx, buf, count);

	J_S_ASSERT_ALLOC( jsstr );
	*J_RVAL = STRING_TO_JSVAL( jsstr );

	return JS_TRUE;
}
*/




/**doc
=== Static properties ===
**/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME
  Determines whether or not the function currently executing was called as a constructor.
**/
DEFINE_PROPERTY( isConstructing ) {

	*vp = BOOLEAN_TO_JSVAL( JS_IsConstructing(cx) );
	return JS_TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME
  Set to $TRUE, this property desactivates the garbage collector.
**/
DEFINE_PROPERTY( disableGarbageCollection ) {
// <shaver>	you could install a vetoing callback!
// <crowder>	oh, true

//	JS_SetGCCallback(cx, ..

	bool disableGC;
	J_CHK( JsvalToBool(cx, *vp, &disableGC) );

	if ( disableGC ) {

		JS_LOCK_GC(JS_GetRuntime(cx));
	} else {

		JS_UNLOCK_GC(JS_GetRuntime(cx));
	}

	return JS_TRUE;
	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
/*
DEFINE_PROPERTY( processPriorityGetter ) {

	return JS_TRUE;
}


DEFINE_PROPERTY( processPrioritySetter ) {

	HANDLE hProcess = OpenProcess(PROCESS_DUP_HANDLE, TRUE, GetCurrentProcessId());
	SetPriorityClass(hProcess, BELOW_NORMAL_PRIORITY_CLASS);

	DWORD dwError = GetLastError();
   CloseHandle(hProcess);

	return JS_TRUE;
}
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

DEFINE_FUNCTION_FAST( Test ) {

	char *ref = (char*)malloc(2000000);
	for ( int i = 0; i < 2000000; i++ )
		ref[i] = rand() & 0xff; // 0->255
	int refPos = 0;

	using namespace jl;
	Buffer b;
	BufferInitialize(&b, bufferTypeAuto, bufferGrowTypePage);

	for ( int i = 0; i < 1000; i++ ) {

		int rnd = rand() & 0xff; // 0->127
		char *tmp = BufferNewChunk(&b, rnd);
		memcpy(tmp, ref + refPos, rnd);
		BufferConfirm(&b, rnd);
		refPos += rnd;
	}

	int l = BufferGetLength(&b);

	char *tmp = (char*)malloc(l);
	BufferCopyData(&b, tmp, l);

	const char *d = BufferGetData(&b);
	bool success = memcmp(ref, d, l) == 0 && memcmp(ref, tmp, l) == 0;

	BoolToJsval(cx, success, J_FRVAL);


	return JS_TRUE;
}
#endif // _DEBUG


CONFIGURE_STATIC

	REVISION(SvnRevToInt("$Revision$"))

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION( Expand )
		FUNCTION_FAST( InternString )
		FUNCTION( Seal )
		FUNCTION( Clear )
		FUNCTION( SetScope )
		FUNCTION( HideProperties )
		FUNCTION_FAST( Exec )
		FUNCTION_FAST(SandboxEval)
		FUNCTION( IsStatementValid )
		FUNCTION_FAST( StringRepeat )
		FUNCTION_FAST( Print )
		FUNCTION_FAST( TimeCounter )
		FUNCTION_FAST( CollectGarbage )
		FUNCTION_FAST( MaybeCollectGarbage )
		FUNCTION_FAST( ObjectToId )
		FUNCTION_FAST( IdToObject )

#ifdef JS_HAS_XDR
		FUNCTION_FAST( XdrEncode )
		FUNCTION_FAST( XdrDecode )
#endif // JS_HAS_XDR
		FUNCTION_FAST( Warning )
		FUNCTION( ASSERT )
		FUNCTION_FAST( Halt )
//		FUNCTION( StrSet )
#ifdef _DEBUG
		FUNCTION_FAST( Test )
#endif // _DEBUG
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_READ( isConstructing )
		PROPERTY_WRITE( disableGarbageCollection )
//		PROPERTY( processPriority )
	END_STATIC_PROPERTY_SPEC

END_STATIC
