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

#include "../common/jsNames.h"

#include "static.h"

#include "jsxdrapi.h"
#include "jscntxt.h"
#include <jsdbgapi.h>

#include "../common/buffer.h"


DECLARE_CLASS( OperationLimit )
DECLARE_CLASS( Sandbox )

/**doc fileIndex:topmost **/

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
DEFINE_FUNCTION_FAST( Expand ) {

	J_S_ASSERT_ARG_MIN( 1 );

	const char *srcBegin;
	size_t srcLen;

	J_CHK( JsvalToStringAndLength(cx, &J_FARG(1), &srcBegin, &srcLen) );
	J_S_ASSERT( srcBegin[srcLen] == '\0', "Invalid input string." ); // else strstr may failed.
	const char *srcEnd;
	srcEnd = srcBegin + srcLen;

	JSObject *table;
	if ( J_FARG_ISDEF(2) ) {

		J_S_ASSERT_OBJECT( J_FARG(2) );
		table = JSVAL_TO_OBJECT( J_FARG(2) );
	} else {

		table = J_FOBJ;
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
	J_CHK( expandedString );
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
	J_CHK( jsstr );
	*J_FRVAL = STRING_TO_JSVAL( jsstr );
	return JS_TRUE;
	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( string )
  Make an interned string, a string that is automatically shared with other code that needs a string with the same value. Use this function with care.
**/
// source: http://mxr.mozilla.org/mozilla/source/js/src/js.c
DEFINE_FUNCTION_FAST( InternString ) {

	JSString *str = JS_ValueToString(cx, vp[2]);
	J_CHK( str );
	J_CHK( JS_InternUCStringN(cx, JS_GetStringChars(str), J_STRING_LENGTH(str)) );
	*vp = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( obj [ , recursively  ] )
  Prevent modification of object fields. ie. all write access to the object, either to add a new property, delete an existing property, or set the value or attributes of an existing property.
  If _recursively_ is true, the function seal any non-null objects in the graph connected to obj's slots.
  $H example
  {{{
  LoadModule('jsstd');

  var obj = { a:1 };
  obj.b = 2;
  Seal(obj);
  obj.c = 3; // Error: obj.c is read-only
}}}
**/
DEFINE_FUNCTION_FAST( Seal ) {

	J_S_ASSERT_ARG_MIN(1);
	J_S_ASSERT_OBJECT( J_FARG(1) );
	//J_CHK( JS_ValueToObject(cx, J_ARG(1), &obj) );
	JSBool deep;
	if ( J_FARG_ISDEF(2) )
		J_CHK( JS_ValueToBoolean(cx, J_FARG(2), &deep) );
	else
		deep = JS_FALSE;
	*J_FRVAL = JSVAL_VOID;
	return JS_SealObject(cx, JSVAL_TO_OBJECT(J_FARG(1)), deep);
	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( obj )
  Remove all properties associated with an object.
  $H note
   $INAME removes all of obj's own properties, except the special __proto__ and __parent__ properties, in a single operation. Properties belonging to objects on obj's prototype chain are not affected.
  $H example
  {{{
  LoadModule('jsstd');

  var obj = { a:1, b:[2,3,4], c:{} };
  Print( uneval(obj) ); // prints: ({a:1, b:[2, 3, 4], c:{}})

  Clear(obj);
  Print( uneval(obj) ); // prints: ({})
  }}}
**/
DEFINE_FUNCTION_FAST( Clear ) {

	J_S_ASSERT_ARG_MIN( 1 );
	J_S_ASSERT_OBJECT( J_FARG(1) );
	JS_ClearScope(cx, JSVAL_TO_OBJECT( J_FARG(1) ));
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $OBJ $INAME( obj, scopeObject )
  Set the scope object of _obj_. Use this function with care.
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
DEFINE_FUNCTION_FAST( SetScope ) {

	J_S_ASSERT_ARG_MIN( 2 );
	JSObject *o, *p;
	J_CHK( JS_ValueToObject(cx, J_FARG(1), &o) ); // o = JSVAL_TO_OBJECT(J_FARG(1));
	J_CHK( JS_ValueToObject(cx, J_FARG(2), &p) ); // p = JSVAL_TO_OBJECT(J_FARG(2));
	*J_FRVAL = OBJECT_TO_JSVAL( JS_GetParent(cx, o) );
	J_CHK( JS_SetParent(cx, o, p) );
	return JS_TRUE;
	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** xx doc
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
/*
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


	//JSBool found;
	//	...
	//	propertyName = JS_GetStringBytes( JS_ValueToString( cx, J_ARG(i+1) ) );
	//	J_S_ASSERT_1( propertyName != NULL, "Invalid property name (%s).", propertyName );
	//	J_CHK( JS_GetPropertyAttributes( cx, object, propertyName, &attributes, &found ) );
	//	if ( found == JS_FALSE )
	//		continue;
	//	attributes &= ~JSPROP_ENUMERATE;
	//	J_CHK( JS_SetPropertyAttributes( cx, object, propertyName, attributes, &found ) );

	}
	return JS_TRUE;
	JL_BAD;
}
*/


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( object, propertyName, polarity )
  Show/Hide a property to for-in loop.
  $H note
   Using this function may change the order of the properties within the object.
  $H example
  {{{
  var obj = { a:1, b:2, c:3 };
  for ( var p in obj )
   Print(p, ', '); // prints: a, b, c

  SetPropertyEnumerate(obj, 'b', false);
  for ( var p in obj )
   Print(p, ', '); // prints: a, c

  SetPropertyEnumerate(obj, 'b', true);
  for ( var p in obj )
   Print(p, ', '); // prints: a, c, b
  }}}
**/
DEFINE_FUNCTION_FAST( SetPropertyEnumerate ) {

	J_S_ASSERT_ARG_MIN( 3 );
	J_S_ASSERT_OBJECT( J_FARG(1) );

	JSObject *object;
	object = JSVAL_TO_OBJECT( J_FARG(1) );
	jsid id;
	J_CHK( JS_ValueToId(cx, J_FARG(2), &id) );
	bool polarity;
	J_CHK( JsvalToBool(cx, J_FARG(3), &polarity) );

	uintN attrs;
	JSObject *obj2;
	JSProperty *prop;

	J_CHK( OBJ_LOOKUP_PROPERTY(cx, object, id, &obj2, &prop) );
	if (!prop || object != obj2) { // not found

		if (prop)
			OBJ_DROP_PROPERTY(cx, obj2, prop);
		J_REPORT_ERROR( "Property not found" );
	}
	J_CHK( OBJ_GET_ATTRIBUTES(cx, object, id, prop, &attrs) );
	if ( polarity )
		attrs |= JSPROP_ENUMERATE;
	else
		attrs &= ~JSPROP_ENUMERATE;
	J_CHK( OBJ_SET_ATTRIBUTES(cx, object, id, prop, &attrs) );
	OBJ_DROP_PROPERTY(cx, object, prop);

	return JS_TRUE;
	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( object, propertyName, polarity )
  Make a property not settable. Any attempt to modify a read-only property fail silently.
  $H example
  {{{
  var obj = { a:1 }

  Print( obj.a ); // prints: 1

  obj.a = 2;
  Print( obj.a ); // prints: 2

  SetPropertyReadonly( obj, 'a', true );
  obj.a = 3;
  Print( obj.a ); // prints: 2

  SetPropertyReadonly( obj, 'a', false );
  obj.a = 4;
  Print( obj.a ); // prints: 4
  }}}
**/
DEFINE_FUNCTION_FAST( SetPropertyReadonly ) {

	J_S_ASSERT_ARG_MIN( 3 );
	J_S_ASSERT_OBJECT( J_FARG(1) );

	JSObject *object;
	object = JSVAL_TO_OBJECT( J_FARG(1) );
	jsid id;
	J_CHK( JS_ValueToId(cx, J_FARG(2), &id) );
	bool polarity;
	J_CHK( JsvalToBool(cx, J_FARG(3), &polarity) );

	uintN attrs;
	JSObject *obj2;
	JSProperty *prop;

	J_CHK( OBJ_LOOKUP_PROPERTY(cx, object, id, &obj2, &prop) );
	if (!prop || object != obj2) { // not found

		if (prop)
			OBJ_DROP_PROPERTY(cx, obj2, prop);
		J_REPORT_ERROR( "Property not found" );
	}
	J_CHK( OBJ_GET_ATTRIBUTES(cx, object, id, prop, &attrs) );
	if ( polarity )
		attrs |= JSPROP_READONLY;
	else
		attrs &= ~JSPROP_READONLY;
	J_CHK( OBJ_SET_ATTRIBUTES(cx, object, id, prop, &attrs) );
	OBJ_DROP_PROPERTY(cx, object, prop);

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
	JSObject *obj;
	obj = JSVAL_TO_OBJECT( J_FARG(1) );

	ObjId *freeSlot;
	freeSlot = NULL;
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
  Returns the object with the identifier _id_ or undefined if the identifier do not exist or the object has been GCed. It is up to you to keep a reference to the object if you want to keep it through GC cycles.
  $H example 1
  {{{
  var myObj = {};
  Print( IdToObject( ObjectToId(myObj) ) === myObj ); // prints true
  }}}
  $H example 2
  {{{
  var id = ObjectToId({});
  Print( IdToObject(id) ); // prints: [object Object]
  CollectGarbage();
  Print( IdToObject(id) ); // prints: undefined
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
 $BOOL $INAME()
  Returns $TRUE if the value is a boolean.
**/
DEFINE_FUNCTION_FAST( IsBoolean ) {

	J_S_ASSERT_ARG_MIN(1);
	*J_FRVAL = JSVAL_IS_BOOLEAN(J_FARG(1)) ? JSVAL_TRUE : JSVAL_FALSE;
	return JS_TRUE;
	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME()
  Returns $TRUE if the value is a primitive ( null or not an object ).
**/
DEFINE_FUNCTION_FAST( IsPrimitive ) {

	J_S_ASSERT_ARG_MIN(1);
	*J_FRVAL = JSVAL_IS_PRIMITIVE(J_FARG(1)) ? JSVAL_TRUE : JSVAL_FALSE;
	return JS_TRUE;
	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME()
  Returns $TRUE if the value is a function.
**/
DEFINE_FUNCTION_FAST( IsFunction ) {

	J_S_ASSERT_ARG_MIN(1);
	*J_FRVAL = VALUE_IS_FUNCTION(cx, J_FARG(1)) ? JSVAL_TRUE : JSVAL_FALSE;
	return JS_TRUE;
	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME()
  Returns $TRUE if the value is undefined (ie. void 0).
  $H example
  {{{
  Print( IsVoid(undefined) ); // prints: true
  }}}
**/
DEFINE_FUNCTION_FAST( IsVoid ) {

	J_S_ASSERT_ARG_MIN(1);
	*J_FRVAL = JSVAL_IS_VOID(J_FARG(1)) ? JSVAL_TRUE : JSVAL_FALSE;
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
	J_CHK( xdr );
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
  $H beware
   Decoding malformed XDR data can lead the program to crash. This may be an important security issue. Decode only trusted data.
**/
#ifdef JS_HAS_XDR
DEFINE_FUNCTION_FAST( XdrDecode ) {

	J_S_ASSERT_ARG_MIN( 1 );

	JSXDRState *xdr;
	xdr = JS_XDRNewMem(cx, JSXDR_DECODE);
	J_CHK( xdr );
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
	const char *message;
	J_CHK( JsvalToString(cx, &J_FARG(1), &message) );
	J_CHK( JS_ReportWarning(cx, "%s", message) );
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
  $INAME( i >= 0 || i < 3, 'Invalid value.' );
  Print( foo[i] );
  }}}
  $H note
   The error output can be redirected by redefining the _configuration.stderr function. see the Print() function.
**/
DEFINE_FUNCTION( Assert ) {

	J_S_ASSERT_ARG_MIN(1);
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
/**doc
$TOC_MEMBER $INAME
 $REAL $INAME()
  Returns the current value of a high-resolution time counter in millisecond.
  The returned value is a relative time value.
  $H example
  {{{
  LoadModule('jsstd');
  LoadModule('jsio');
  Print( 't0: '+TimeCounter(), '\n' ); // prints: 1743731894.4259675
  Print( 't1: '+TimeCounter(), '\n' ); // prints: 1743731896.1083043
  Sleep(100);
  Print( 't2: '+TimeCounter(), '\n' ); // prints: 1743732003.6174989
  }}}
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
	J_CHK( newBuf );
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
	J_CHK( jsstr );
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
   $H example 1
   {{{
   LoadModule('jsstd');
   Print('Hello', 'World'); // prints: HelloWorld
   }}}
	$H example 2
   {{{
   LoadModule('jsstd');
   Print('foo\n'); // prints: foo
   _configuration.stdout = function() {}
   Print('bar\n'); // prints nothing
   }}}
**/
DEFINE_FUNCTION_FAST( Print ) {

	jsval fval;
	J_CHK( GetConfigurationValue(cx, NAME_CONFIGURATION_STDOUT, &fval) );
	*J_FRVAL = JSVAL_VOID;
	if ( JsvalIsFunction(cx, fval) )
		return JS_CallFunctionValue(cx, JS_GetGlobalObject(cx), fval, J_ARGC, J_FARGV, J_FRVAL);
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

#ifndef JS_HAS_XDR
	return JS_CompileFile(cx, obj, fileName);
#endif // JS_HAS_XDR

	JSScript *script = NULL;

	char compiledFileName[PATH_MAX];
	strcpy( compiledFileName, fileName );
	strcat( compiledFileName, "xdr" );

	struct stat srcFileStat, compFileStat;
	bool hasSrcFile = stat(fileName, &srcFileStat) != -1; // errno == ENOENT
	bool hasCompFile = stat(compiledFileName, &compFileStat) != -1;
	bool compFileUpToDate = hasCompFile && !hasSrcFile || hasSrcFile && hasCompFile && (compFileStat.st_mtime > srcFileStat.st_mtime); // true if comp file is up to date or alone

	J_CHKM2( hasSrcFile || hasCompFile, "Unable to load Script, file \"%s\" or \"%s\" not found.", fileName, compiledFileName );

	if ( useCompFile && compFileUpToDate ) {

		int file = open(compiledFileName, O_RDONLY | O_BINARY | O_SEQUENTIAL);
		J_CHKM1( file != -1, "Unable to open file \"%s\" for reading.", compiledFileName );

		size_t compFileSize = compFileStat.st_size; // filelength(file); ?
		void *data = malloc(compFileSize); // (TBD) free on error
		int readCount = read( file, data, compFileSize ); // here we can use "Memory-Mapped I/O Functions" ( http://developer.mozilla.org/en/docs/NSPR_API_Reference:I/O_Functions#Memory-Mapped_I.2FO_Functions )
		J_CHKM1( readCount != -1 && readCount == compFileSize, "Unable to read the file \"%s\" ", compiledFileName );
		close( file );

		JSXDRState *xdr = JS_XDRNewMem(cx, JSXDR_DECODE);
		J_CHK( xdr );
		JS_XDRMemSetData(xdr, data, compFileSize);
		J_CHK( JS_XDRScript(xdr, &script) );
		// (TBD) manage BIG_ENDIAN here ?
		JS_XDRMemSetData(xdr, NULL, 0);
		JS_XDRDestroy(xdr);
		free(data);
		if ( JS_GetScriptVersion(cx, script) < JS_GetVersion(cx) )
			J_REPORT_WARNING_1("Trying to xdr-decode an old script (%s).", compiledFileName);
		return script; // Done.
	}

// script = JS_CompileFile(cx, obj, fileName);

// shebang support
	FILE *scriptFile;
	scriptFile = fopen(fileName, "r");
	J_CHKM1( scriptFile != NULL, "Script file \"%s\" cannot be opened.", fileName );

	char s, b;
	s = getc(scriptFile);
	if ( s == '#' ) {

		b = getc(scriptFile);
		if ( b == '!' ) {

			ungetc('/', scriptFile);
			ungetc('/', scriptFile);
		} else {

			ungetc(b, scriptFile);
			ungetc(s, scriptFile);
		}
	} else {

		ungetc(s, scriptFile);
	}

	script = JS_CompileFileHandle(cx, obj, fileName, scriptFile);
	fclose(scriptFile);

	J_CHK( script );
	if ( !useCompFile )
		return script; // Done.

	int file = open(compiledFileName, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY | O_SEQUENTIAL, 0700);
	if ( file == -1 ) // if the file cannot be write, this is not an error ( eg. read-only drive )
		return script;

	JSXDRState *xdr = JS_XDRNewMem(cx, JSXDR_ENCODE);
	J_CHK( xdr );
	J_CHK( JS_XDRScript(xdr, &script) );

	uint32 length;
	void *buf;
	buf = JS_XDRMemGetData(xdr, &length);
	J_CHK( buf );
	// manage BIG_ENDIAN here ?
	J_CHK( write(file, buf, length) != -1 ); // On error, -1 is returned, and errno is set appropriately.
	J_CHK( close(file) == 0 );
	JS_XDRDestroy(xdr);
	return script;

bad:
	// report a warning ?
	return script;
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

	J_S_ASSERT_ARG_MIN( 1 );

	bool saveCompiledScripts;
	saveCompiledScripts = !J_FARG_ISDEF(2) || J_FARG(2) == JSVAL_TRUE;
	const char *filename;
	J_CHK( JsvalToString(cx, &J_FARG(1), &filename) );

	uint32 oldopts;
	oldopts = JS_SetOptions(cx, JS_GetOptions(cx) | JSOPTION_COMPILE_N_GO);
	JSScript *script;
	script = LoadScript( cx, J_FOBJ, filename, saveCompiledScripts );
	JS_SetOptions(cx, oldopts);
	J_CHK( script );

	JSTempValueRooter tvr;
	JSObject *scrobj = JS_NewScriptObject(cx, script);
	JS_PUSH_TEMP_ROOT_OBJECT(cx, scrobj, &tvr);
	JSBool ok;
	ok = JS_ExecuteScript(cx, J_FOBJ, script, J_FRVAL); // Doc: On successful completion, rval is a pointer to a variable that holds the value from the last executed expression statement processed in the script.
	JS_POP_TEMP_ROOT(cx, &tvr);
	J_CHK( ok );

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
	JSContext *cx;
	jsval queryFunctionValue;
};

JSBool SandboxMaxOperationCallback(JSContext *cx) {

	JSObject *branchLimitExceptionObj = JS_NewObject(cx, classOperationLimit, NULL, NULL);
	J_CHK( branchLimitExceptionObj );
	JS_SetPendingException(cx, OBJECT_TO_JSVAL( branchLimitExceptionObj ));
	JL_BAD;
}

JLThreadFuncDecl SandboxWatchDogThreadProc(void *threadArg) {

	JSContext *scx = (JSContext*)threadArg;
	SandboxContextPrivate *pv = (SandboxContextPrivate*)JS_GetContextPrivate(scx);
	SleepMilliseconds(pv->maxExecutionTime);
	JS_TriggerOperationCallback(scx);
	return 0;
}

JSBool SandboxQueryFunction(JSContext *scx, uintN argc, jsval *vp) {

	SandboxContextPrivate *pv = (SandboxContextPrivate*)JS_GetContextPrivate(scx);
	JSContext *cx = pv->cx; // needed to send errors in the right context.
	if ( JSVAL_IS_VOID( pv->queryFunctionValue ) ) {

		*J_FRVAL = JSVAL_VOID;
	} else {

		J_CHK( JS_CallFunctionValue(scx, J_FOBJ, pv->queryFunctionValue, J_ARGC, J_FARGV, J_FRVAL) );
		J_CHKM( JSVAL_IS_PRIMITIVE(*J_FRVAL), "Only primitive value can be returned." );
	}
	return JS_TRUE;
	JL_BAD;
}

DEFINE_FUNCTION_FAST( SandboxEval ) {

	JSContext *scx = NULL;

	J_S_ASSERT_ARG_MIN(1);

	scx = JS_NewContext(JS_GetRuntime(cx), 8192L); // see host/host.cpp
	J_CHK( scx );
	JS_SetOptions(scx, JS_GetOptions(scx) | JSOPTION_DONT_REPORT_UNCAUGHT | JSOPTION_VAROBJFIX | JSOPTION_XML | JSOPTION_COMPILE_N_GO | JSOPTION_RELIMIT | JSOPTION_JIT);

	JSObject *globalObject;
	globalObject = JS_NewObject(scx, classSandbox, NULL, NULL);
	J_CHK( globalObject );

	SandboxContextPrivate pv;
	pv.cx = cx;

	if ( J_FARG_ISDEF(2) ) {

		J_S_ASSERT_FUNCTION( J_FARG(2) );
		pv.queryFunctionValue = J_FARG(2);
		J_CHK( JS_DefineFunction(scx, globalObject, "Query", (JSNative)SandboxQueryFunction, 1, JSFUN_FAST_NATIVE | JSPROP_PERMANENT | JSPROP_READONLY) );
	} else {

		pv.queryFunctionValue = JSVAL_VOID;
	}

	if ( J_FARG_ISDEF(3) )
		J_CHK( JsvalToUInt(cx, J_FARG(3), &pv.maxExecutionTime) );
	else
		pv.maxExecutionTime = 1000; // default value

	JSString *jsstr;
	jsstr = JS_ValueToString(cx, J_FARG(1));
	J_CHK( jsstr );
	uintN srclen;
	srclen = JS_GetStringLength(jsstr);
	jschar *src;
	src = JS_GetStringChars(jsstr);

	JSOperationCallback prev;
	prev = JS_SetOperationCallback(scx, SandboxMaxOperationCallback);

	JS_SetGlobalObject(scx, globalObject);
	JSStackFrame *fp;
	fp = JS_GetScriptedCaller(cx, NULL);
	J_CHK( fp );
	JSScript *script;
	script = JS_GetFrameScript(cx, fp);
	J_CHK( script );
	const char *filename;
	filename = JS_GetScriptFilename(cx, script);
	jsbytecode *pc;
	pc = JS_GetFramePC(cx, fp);
	uintN lineno;
	lineno = JS_PCToLineNumber(cx, script, pc);

	JS_SetContextPrivate(scx, &pv);

	JLThreadHandler sandboxWatchDogThread;
	sandboxWatchDogThread = JLThreadStart(SandboxWatchDogThreadProc, scx);
	if ( !JLThreadOk(sandboxWatchDogThread) ) {

		char reason[1024];
		JLLastSysetmErrorMessage(reason, sizeof(reason));
		J_REPORT_ERROR_1( "Unable to create the thread (%s).", reason );
	}

	JSBool ok;
	ok = JS_EvaluateUCScript(scx, globalObject, src, srclen, filename, lineno, J_FRVAL);

	J_CHK( JLThreadCancel(sandboxWatchDogThread) ); // <- this line make troubles
	J_CHK( JLWaitThread(sandboxWatchDogThread) );
	JLFreeThread(&sandboxWatchDogThread);

	JS_SetContextPrivate(scx, NULL);

	prev = JS_SetOperationCallback(scx, prev);
	J_S_ASSERT( prev == SandboxMaxOperationCallback, "Invalid SandboxMaxOperationCallback handler." );


//	JSPrincipals principals = { "sandbox context", NULL, NULL, 1, NULL, NULL };
//	JSBool ok = JS_EvaluateUCScriptForPrincipals(scx, globalObject, &principals, src, srclen, fp->script->filename, JS_PCToLineNumber(cx, fp->script, fp->regs->pc), J_FRVAL);

	if (!ok) {

		jsval ex;
		if ( JS_GetPendingException(scx, &ex) )
			JS_SetPendingException(cx, ex);
		else
			JS_ReportError(cx, "Unexpected error.");
	}

	JS_DestroyContextNoGC(scx);
	return ok;

bad:
	JLFreeThread(&sandboxWatchDogThread);
	if ( scx )
		JS_DestroyContextNoGC(scx);
	return JS_FALSE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME( statementString )
  Returns true if _statementString_ is a valid Javascript statement.
  The intent is to support interactive compilation, accumulate lines in a buffer until IsStatementValid returns true, then pass it to an eval.
  This function is useful to write an interactive console.
  $H example
  {{{
  LoadModule('jsstd');
  LoadModule('jsio');

  global.__defineGetter__('quit', Halt);

  for (;;) {

   Print('js> ');

   var code = '';
   do {

    code += File.stdin.Read();
	} while( !IsStatementValid( code ) );

   try {

    var res = eval( code );
	} catch(ex) {

    Print( ex, '\n' );
	}

   if ( res != undefined )
    Print( res, '\n' );
  }
  }}}
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
JSBool VetoingCallback(JSContext *cx, JSGCStatus status) {

	// doc. JSGC_BEGIN: Start of GC. The callback may prevent GC from starting by returning JS_FALSE.
	//      But even if the callback returns JS_TRUE, the garbage collector may determine that GC is not necessary,
	//      in which case the other three callbacks are skipped.
	if ( status == JSGC_BEGIN )
		return JS_FALSE;
	return JS_TRUE;
}

JSGCCallback prevJSGCCallback = NULL;

DEFINE_PROPERTY_SETTER( disableGarbageCollection ) {

	// <shaver>	you could install a vetoing callback!
	// <crowder>	oh, true
	bool disableGC;
	J_CHK( JsvalToBool(cx, *vp, &disableGC) );
	if ( disableGC ) {

		JSGCCallback tmp = JS_SetGCCallback(cx, VetoingCallback);
		if ( tmp != VetoingCallback )
			prevJSGCCallback = tmp;
	} else {

		JSGCCallback tmp = JS_SetGCCallback(cx, prevJSGCCallback);
		if ( tmp != VetoingCallback )
			JS_SetGCCallback(cx, tmp);
	}
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( disableGarbageCollection ) {

	JSGCCallback cb = JS_SetGCCallback(cx, NULL);
	JS_SetGCCallback(cx, cb);
	*vp = cb == VetoingCallback ? JSVAL_TRUE : JSVAL_FALSE;
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

	return JS_FALSE;
}
#endif // _DEBUG


CONFIGURE_STATIC

	REVISION(SvnRevToInt("$Revision$"))

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION_FAST( Expand )
		FUNCTION_FAST( InternString )
		FUNCTION_FAST( Seal )
		FUNCTION_FAST( Clear )
		FUNCTION_FAST( SetScope )
//		FUNCTION( HideProperties )
		FUNCTION_FAST_ARGC( SetPropertyEnumerate, 3 )
		FUNCTION_FAST_ARGC( SetPropertyReadonly, 3 )

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
		FUNCTION_FAST_ARGC( IsBoolean, 1 )
		FUNCTION_FAST_ARGC( IsPrimitive, 1 )
		FUNCTION_FAST_ARGC( IsFunction, 1 )
		FUNCTION_FAST_ARGC( IsVoid, 1 )
#ifdef JS_HAS_XDR
		FUNCTION_FAST( XdrEncode )
		FUNCTION_FAST( XdrDecode )
#endif // JS_HAS_XDR
		FUNCTION_FAST( Warning )
		FUNCTION( Assert )
		FUNCTION_FAST( Halt )
//		FUNCTION( StrSet )
#ifdef _DEBUG
		FUNCTION_FAST( Test )
#endif // _DEBUG
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_READ( isConstructing )
		PROPERTY( disableGarbageCollection )
//		PROPERTY( processPriority )
	END_STATIC_PROPERTY_SPEC

END_STATIC
