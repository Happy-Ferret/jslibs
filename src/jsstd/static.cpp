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

#include "jsxdrapi.h"
#include "jscntxt.h"
#include "jsnum.h"
#include <jsdbgapi.h>

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
 $STR $INAME( str , object|function )
  Return an expanded string using key/value stored in _obj_ or returned by the function.
  $H note
   $UNDEF values are ignored in the resulting string.
  $H example 1
  {{{
  Expand('$(h) $(xxx) $(w)', { h:'Hello', w:'World' }); // returns "Hello  World"
  }}}
  $H example 2
  {{{
  Expand('$(foo)-$(bar)', function(id) '<'+id+'>' ); // returns "<foo>-<bar>"
  }}}
**/

#define EXPAND_SOURCE_ARG 1
#define EXPAND_SOURCE_ARG_FUNCTION 2
//#define EXPAND_SOURCE_SCOPE 3

DEFINE_FUNCTION_FAST( Expand ) {

	JSObject *obj = JL_FOBJ;

	JL_S_ASSERT_ARG_RANGE(1, 2);

	const char *srcBegin;
	unsigned int srcLen;

	JL_CHK( JsvalToStringAndLength(cx, &JL_FARG(1), &srcBegin, &srcLen) );
	JL_S_ASSERT( srcBegin[srcLen] == '\0', "Invalid input string." ); // else strstr may failed.
	const char *srcEnd;
	srcEnd = srcBegin + srcLen;

	int mapSource;
	jsval map;

	//if ( argc < 2 ) {

	//	map = JSVAL_VOID;
	//	mapSource = EXPAND_SOURCE_SCOPE;
	//	goto next;
	//}

	if ( JsvalIsFunction(cx, JL_FARG(2)) ) {

		map = JL_FARG(2);
		mapSource = EXPAND_SOURCE_ARG_FUNCTION;
		goto next;
	}

	if ( !JSVAL_IS_PRIMITIVE( JL_FARG(2) ) ) {

		map = JL_FARG(2);
		mapSource = EXPAND_SOURCE_ARG;
		goto next;
	}

	JL_REPORT_ERROR( "Invalid argument." );

next:


	typedef struct {
		const char *data;
		unsigned int length;
		JSTempValueRooter tvr;
		bool hasTvr;
	} Chunk;

	void *stack;
	jl::StackInit( &stack );
	Chunk *chunk;
	const char *tok;
	int totalLength;
	totalLength = 0;

	while ( *srcBegin != '\0' ) {

		tok = strstr(srcBegin, "$(");
		if ( tok == NULL ) { // not found

			chunk = (Chunk*)jl_malloc(sizeof(Chunk));
			chunk->data = srcBegin;
			chunk->length = srcEnd - srcBegin;
			chunk->hasTvr = false;
			totalLength += chunk->length;
			jl::StackPush( &stack, chunk );
			break;
		}

		chunk = (Chunk*)jl_malloc(sizeof(Chunk));
		chunk->data = srcBegin;
		chunk->length = tok - srcBegin;
		chunk->hasTvr = false;
		totalLength += chunk->length;
		jl::StackPush( &stack, chunk );

		srcBegin = tok + 2; // length of "$("
		tok = strchr(srcBegin, ')'); // tok = strstr(srcBegin, ")"); // slower for only one char
		if ( tok == NULL ) // not found
			break;

		//if ( mapSource == EXPAND_SOURCE_SCOPE ) {

		//	char tmp = *tok; // (TBD) try to replace this trick
		//	*((char*)tok) = '\0';
		//	JL_CHKB( JL_GetVariableValue(cx, srcBegin, JL_FRVAL), bad_free_stack );
		//	*((char*)tok) = tmp;
		//} else
		if ( mapSource == EXPAND_SOURCE_ARG_FUNCTION ) {

			JL_CHKB( StringAndLengthToJsval(cx, JL_FRVAL, srcBegin, tok-srcBegin), bad_free_stack );
			JL_CHKB( JS_CallFunctionValue(cx, obj, map, 1, JL_FRVAL, JL_FRVAL), bad_free_stack );
		} else {

			char tmp = *tok; // (TBD) try to replace this trick
			*((char*)tok) = '\0';
			JL_CHKB( JS_GetProperty(cx, JSVAL_TO_OBJECT(map), srcBegin, JL_FRVAL), bad_free_stack );
			*((char*)tok) = tmp;
		}

		if ( !JSVAL_IS_VOID( *JL_FRVAL ) ) {

			chunk = (Chunk*)jl_malloc(sizeof(Chunk));
			JS_PUSH_SINGLE_TEMP_ROOT(cx, *JL_FRVAL, &chunk->tvr);
			chunk->hasTvr = true;
			JL_CHKB( JsvalToStringAndLength(cx, &chunk->tvr.u.value, &chunk->data, &chunk->length), bad_free_stack );
			totalLength += chunk->length;
			jl::StackPush( &stack, chunk );
		}

		srcBegin = tok + 1; // length of ")"
	}

	char *expandedString;
	expandedString = (char*)JS_malloc(cx, totalLength +1);
	JL_CHKB( expandedString, bad_free_stack );
	expandedString[totalLength] = '\0';

	expandedString += totalLength;
	while ( !jl::StackIsEnd(&stack) ) {

		Chunk *chunk = (Chunk*)jl::StackPop(&stack);
		expandedString -= chunk->length;
		memcpy(expandedString, chunk->data, chunk->length);
		if ( chunk->hasTvr )
			JS_POP_TEMP_ROOT(cx, &chunk->tvr);
		jl_free(chunk);
	}

	JSString *jsstr;
	jsstr = JS_NewString(cx, expandedString, totalLength);
	JL_CHK( jsstr );
	*JL_FRVAL = STRING_TO_JSVAL( jsstr );
	return JS_TRUE;

bad_free_stack:

	while ( !jl::StackIsEnd(&stack) ) {

		Chunk *chunk = (Chunk*)jl::StackPop(&stack);
		if ( chunk->hasTvr )
			JS_POP_TEMP_ROOT(cx, &chunk->tvr);
		jl_free(chunk);
	}
bad:
	return JS_FALSE;
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
	JL_CHK( str );
	JL_CHK( JS_InternUCStringN(cx, JS_GetStringChars(str), JL_GetStringLength(str)) );
	*vp = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( obj [ , recursively  ] )
  Prevent modification of object fields. ie. all write access to the object, either to add a new property, delete an existing property, or set the value or attributes of an existing property.
  If _recursively_ is $TRUE, the function seal any non-null objects in the graph connected to obj's slots.
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

	JL_S_ASSERT_ARG_RANGE(1, 2);
	JL_S_ASSERT_OBJECT( JL_FARG(1) );
	//JL_CHK( JS_ValueToObject(cx, JL_ARG(1), &obj) );
	JSBool deep;
	if ( JL_FARG_ISDEF(2) )
		JL_CHK( JS_ValueToBoolean(cx, JL_FARG(2), &deep) );
	else
		deep = JS_FALSE;
	*JL_FRVAL = JSVAL_VOID;
	return JS_SealObject(cx, JSVAL_TO_OBJECT(JL_FARG(1)), deep);
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

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_OBJECT( JL_FARG(1) );
	JS_ClearScope(cx, JSVAL_TO_OBJECT( JL_FARG(1) ));
	*JL_FRVAL = JSVAL_VOID;
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
   $F 55 $LF
   $F 7 $LF
   $F 55 $LF
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

	JL_S_ASSERT_ARG(2);
	JSObject *o, *p;
	JL_CHK( JS_ValueToObject(cx, JL_FARG(1), &o) ); // o = JSVAL_TO_OBJECT(JL_FARG(1));
	JL_CHK( JS_ValueToObject(cx, JL_FARG(2), &p) ); // p = JSVAL_TO_OBJECT(JL_FARG(2));
	*JL_FRVAL = OBJECT_TO_JSVAL( JS_GetParent(cx, o) );
	JL_CHK( JS_SetParent(cx, o, p) );
	return JS_TRUE;
	JL_BAD;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* *doc
$TOC_MEMBER $INAME
 $OBJ $INAME()
  Get the currentScope.
  }}}
**/
/*
DEFINE_FUNCTION_FAST( GetCurrentScope ) {

	JL_S_ASSERT_ARG(0);

	JSStackFrame *fp = JL_CurrentStackFrame(cx);
	*JL_FRVAL = OBJECT_TO_JSVAL( fp->??? );
	return JS_TRUE;
	JL_BAD;
}
*/

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

	JL_S_ASSERT_ARG_MIN( 2 );
	JSObject *object;
	JL_CHK( JS_ValueToObject( cx, JL_ARG(1), &object ) );
//	const char *propertyName;
//	uintN attributes;
	for ( uintN i = 1; i < JL_ARGC; i++ ) {

		jsid id;
		uintN attrs;
		JSObject *obj2;
		JSProperty *prop;

		JL_CHK( JS_ValueToId(cx, JL_ARG(1+i), &id) );
		JL_CHK( OBJ_LOOKUP_PROPERTY(cx, object, id, &obj2, &prop) );
		if (!prop || object != obj2) { // not found

			if (prop)
				OBJ_DROP_PROPERTY(cx, obj2, prop);
			JL_REPORT_ERROR( "Invalid property name." );
		}
		JL_CHK( OBJ_GET_ATTRIBUTES(cx, object, id, prop, &attrs) );
		attrs &= ~JSPROP_ENUMERATE;
		JL_CHK( OBJ_SET_ATTRIBUTES(cx, object, id, prop, &attrs) );
		OBJ_DROP_PROPERTY(cx, object, prop);


	//JSBool found;
	//	...
	//	propertyName = JS_GetStringBytes( JS_ValueToString( cx, JL_ARG(i+1) ) );
	//	JL_S_ASSERT( propertyName != NULL, "Invalid property name (%s).", propertyName );
	//	JL_CHK( JS_GetPropertyAttributes( cx, object, propertyName, &attributes, &found ) );
	//	if ( found == JS_FALSE )
	//		continue;
	//	attributes &= ~JSPROP_ENUMERATE;
	//	JL_CHK( JS_SetPropertyAttributes( cx, object, propertyName, attributes, &found ) );

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

	JL_S_ASSERT_ARG(3);
	JL_S_ASSERT_OBJECT( JL_FARG(1) );

	JSObject *object;
	object = JSVAL_TO_OBJECT( JL_FARG(1) );
	jsid id;
	JL_CHK( JS_ValueToId(cx, JL_FARG(2), &id) );
	bool polarity;
	JL_CHK( JsvalToBool(cx, JL_FARG(3), &polarity) );

	uintN attrs;
	JSObject *obj2;
	JSProperty *prop;

	JL_CHK( object->lookupProperty(cx, id, &obj2, &prop) );
	if (!prop || object != obj2) { // not found

		if (prop)
			obj2->dropProperty(cx, prop);
		JL_REPORT_ERROR( "Property not found" );
	}
	JL_CHK( object->getAttributes(cx, id, prop, &attrs) );
	if ( polarity )
		attrs |= JSPROP_ENUMERATE;
	else
		attrs &= ~JSPROP_ENUMERATE;
	JL_CHK( object->setAttributes(cx, id, prop, &attrs) );
	object->dropProperty(cx, prop);

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

	JL_S_ASSERT_ARG(3);
	JL_S_ASSERT_OBJECT( JL_FARG(1) );

	JSObject *object;
	object = JSVAL_TO_OBJECT( JL_FARG(1) );
	jsid id;
	JL_CHK( JS_ValueToId(cx, JL_FARG(2), &id) );
	bool polarity;
	JL_CHK( JsvalToBool(cx, JL_FARG(3), &polarity) );

	uintN attrs;
	JSObject *obj2;
	JSProperty *prop;

	JL_CHK( object->lookupProperty(cx, id, &obj2, &prop) );
	if (!prop || object != obj2) { // not found

		if (prop)
			obj2->dropProperty(cx, prop);
		JL_REPORT_ERROR( "Property not found" );
	}
	JL_CHK( object->getAttributes(cx, id, prop, &attrs) );
	if ( polarity )
		attrs |= JSPROP_READONLY;
	else
		attrs &= ~JSPROP_READONLY;
	JL_CHK( object->setAttributes(cx, id, prop, &attrs) );
	object->dropProperty(cx, prop);

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
unsigned int objectIdAllocated = 0;

JSGCCallback prevObjectIdGCCallback = NULL;

JSBool ObjectIdGCCallback(JSContext *cx, JSGCStatus status) {

	if ( status == JSGC_MARK_END ) {

		for ( ObjId *it = objIdList, *end = objIdList + objectIdAllocated; it < end; ++it ) {

			if ( it->obj && JS_IsAboutToBeFinalized(cx, it->obj) ) {

				it->id = 0;
				it->obj = NULL;
			}
		}
	}
	return prevObjectIdGCCallback ? prevObjectIdGCCallback(cx, status) : JS_TRUE;
}


DEFINE_FUNCTION_FAST( ObjectToId ) {

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_OBJECT( JL_FARG(1) );
	JSObject *obj;
	obj = JSVAL_TO_OBJECT( JL_FARG(1) );

	ObjId *freeSlot;
	freeSlot = NULL;
	for ( ObjId *it = objIdList, *end = objIdList + objectIdAllocated; it < end; ++it ) {

		if ( it->obj == obj )
			return UIntToJsval(cx, it->id, JL_FRVAL);
		if ( !freeSlot && it->id == 0 )
			freeSlot = it;
	}

	if ( !freeSlot ) {

		unsigned int prevAlloced = objectIdAllocated;

		if ( !objIdList ) {

			prevObjectIdGCCallback = JS_SetGCCallback(cx, ObjectIdGCCallback);
			objectIdAllocated = 32;
		} else {

			objectIdAllocated *= 2;
		}
		objIdList = (ObjId*)JS_realloc(cx, objIdList, sizeof(ObjId) * objectIdAllocated); // (TBD) free objIdList at the end !
		freeSlot = objIdList + prevAlloced;
		memset(freeSlot, 0, (objectIdAllocated - prevAlloced) * sizeof(ObjId)); // init only new slots
	}

	freeSlot->id = ++lastObjectId;
	freeSlot->obj = obj;

	return UIntToJsval(cx, lastObjectId, JL_FRVAL);
	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $INT $INAME( id )
  Returns the object with the identifier _id_ or $UNDEF if the identifier do not exist or the object has been GCed.
  It is up to you to keep a reference to the object if you want to keep it through GC cycles.
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

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_NUMBER( JL_FARG(1) );

	unsigned int id;
	JL_CHK( JsvalToUInt(cx, JL_FARG(1), &id) );

	if ( id > 0 && id <= lastObjectId ) {

		for ( ObjId *it = objIdList, *end = objIdList + objectIdAllocated; it < end; ++it ) {

			if ( it->id == id ) {

				*JL_FRVAL = OBJECT_TO_JSVAL( it->obj );
				return JS_TRUE;
			}
		}
	}

	*JL_FRVAL = JSVAL_VOID;
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

	if ( JSVAL_IS_BOOLEAN(JL_FARG(1)) ) {

		*JL_FRVAL = JSVAL_TRUE;
		return JS_TRUE;
	}

	if ( JSVAL_IS_PRIMITIVE(JL_FARG(1)) ) {

		*JL_FRVAL = JSVAL_FALSE;
		return JS_TRUE;
	}

	*JL_FRVAL = JL_GetClass(JSVAL_TO_OBJECT(JL_FARG(1))) == JL_GetStandardClass(cx, JSProto_Boolean) ? JSVAL_TRUE : JSVAL_FALSE;
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME()
  Returns $TRUE if the value is a number.
**/
DEFINE_FUNCTION_FAST( IsNumber ) {

	if ( JSVAL_IS_NUMBER(JL_FARG(1)) ) {

		*JL_FRVAL = JSVAL_TRUE;
		return JS_TRUE;
	}

	if ( JSVAL_IS_PRIMITIVE(JL_FARG(1)) ) {

		*JL_FRVAL = JSVAL_FALSE;
		return JS_TRUE;
	}

	*JL_FRVAL = JL_GetClass(JSVAL_TO_OBJECT(JL_FARG(1))) == JL_GetStandardClass(cx, JSProto_Number) ? JSVAL_TRUE : JSVAL_FALSE;
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME()
  Returns $TRUE if the value is a primitive ( null or not an object ).
**/
DEFINE_FUNCTION_FAST( IsPrimitive ) {

	*JL_FRVAL = JSVAL_IS_PRIMITIVE(JL_FARG(1)) ? JSVAL_TRUE : JSVAL_FALSE;
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

	*JL_FRVAL = VALUE_IS_FUNCTION(cx, JL_FARG(1)) ? JSVAL_TRUE : JSVAL_FALSE;
	return JS_TRUE;
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

	JL_S_ASSERT_ARG(1);
	JSXDRState *xdr;
	xdr = JS_XDRNewMem(cx, JSXDR_ENCODE);
	JL_CHK( xdr );
	//if (unlikely( JsvalIsScript(cx, JL_FARG(1)) )) {

	//	JSScript *script = (JSScript*)JL_GetPrivate(cx, JSVAL_TO_OBJECT(JL_FARG(1)));
	//	JL_S_ASSERT_RESOURCE(script);
	//	JL_CHK( JS_XDRScript(xdr, &script) );
	//} else {

		JL_CHK( JS_XDRValue(xdr, &JL_FARG(1)) );
	//}
	uint32 length;
	void *buffer;
	buffer = JS_XDRMemGetData(xdr, &length);
	JL_S_ASSERT( buffer != NULL, "Invalid xdr data." );
	JL_CHK( JL_NewBlobCopyN(cx, buffer, length, JL_FRVAL) );
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

	JL_S_ASSERT_ARG(1);

	JSXDRState *xdr;
	xdr = JS_XDRNewMem(cx, JSXDR_DECODE);
	JL_CHK( xdr );
	const char *buffer;
	unsigned int length;
	JL_CHK( JsvalToStringAndLength(cx, &JL_FARG(1), &buffer, &length) );
	JS_XDRMemSetData(xdr, (void*)buffer, length); // safe de-const cast: we are JSXDR_DECODE from the buffer.
	JL_CHK( JS_XDRValue(xdr, JL_FRVAL) );
	//JL_CHK( JS_XDRScript(xdr, JL_FRVAL) );
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

	JL_S_ASSERT_ARG(1);
	const char *message;
	JL_CHK( JsvalToString(cx, &JL_FARG(1), &message) );
	JL_CHK( JS_ReportWarning(cx, "%s", message) );
	*JL_FRVAL = JSVAL_VOID;
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

	JL_S_ASSERT_ARG_RANGE(1,2);
	bool assert;
	JL_CHK( JsvalToBool(cx, JL_ARG(1), &assert) );
	if ( !assert ) {

		const char *message;
		if ( JL_ARG_ISDEF(2) )
			JL_CHK( JsvalToString(cx, &JL_ARG(2), &message) );
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
 $INT $INAME()
  Performs an unconditional garbage collection in the JS memory pool.
**/
DEFINE_FUNCTION_FAST( CollectGarbage ) {

	long gcBytesDiff = cx->runtime->gcBytes;
	JS_GC( cx );
	gcBytesDiff = cx->runtime->gcBytes - gcBytesDiff;

	*JL_FRVAL = INT_TO_JSVAL(gcBytesDiff);
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $INT $INAME()
  Performs a conditional garbage collection of JS objects, doubles, and strings that are no longer needed by a script executing.
  This offers the JavaScript engine an opportunity to perform garbage collection if needed.
**/
DEFINE_FUNCTION_FAST( MaybeCollectGarbage ) {

	long gcBytesDiff = cx->runtime->gcBytes;
	JS_MaybeGC( cx );
	gcBytesDiff = cx->runtime->gcBytes - gcBytesDiff;

	*JL_FRVAL = INT_TO_JSVAL(gcBytesDiff);
	return JS_TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $REAL $INAME( time )
  Suspends the execution of the current program during _time_ milliseconds.
**/
DEFINE_FUNCTION_FAST( Sleep ) {

	JL_S_ASSERT_ARG(1);
	unsigned int time;
	JL_CHK( JsvalToUInt(cx, JL_FARG(1), &time) );
	SleepMilliseconds(time);
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
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

	return DoubleToJsval(cx, AccurateTimeCounter(), JL_FRVAL);
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

	JL_S_ASSERT_ARG(2);

	unsigned int count;
	JL_CHK( JsvalToUInt(cx, JL_FARG(2), &count) );
	if ( count == 0 ) {

		*JL_FRVAL = JS_GetEmptyStringValue(cx);
		return JS_TRUE;
	}

	const char *buf;
	unsigned int len;
	JL_CHK( JsvalToStringAndLength(cx, &JL_FARG(1), &buf, &len) ); // warning: GC on the returned buffer !

	if ( len == 0 ) {

		*JL_FRVAL = JS_GetEmptyStringValue(cx);
		return JS_TRUE;
	}

	if ( count == 1 ) {

		*JL_FRVAL = STRING_TO_JSVAL( JS_ValueToString(cx, JL_FARG(1)) ); // force string conversion because we must return a string.
		return JS_TRUE;
	}

	unsigned int newLen;
	newLen = len * count;

	char *newBuf;
	newBuf = (char *)JS_malloc(cx, newLen +1);
	JL_CHK( newBuf );
	newBuf[newLen] = '\0';

	if ( len == 1 ) {

		memset(newBuf, *buf, newLen);
	} else {

		char *tmp = newBuf;
		unsigned int i, j;
		for ( i=0; i<count; i++ )
			for ( j=0; j<len; j++ )
				*(tmp++) = buf[j];
	}

	JSString *jsstr;
	jsstr = JS_NewString(cx, newBuf, newLen);
	JL_CHK( jsstr );
	*JL_FRVAL = STRING_TO_JSVAL( jsstr );
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
	JL_CHK( GetConfigurationValueById(cx, JLID(cx, stdout), &fval) );
	*JL_FRVAL = JSVAL_VOID;
	if ( JsvalIsFunction(cx, fval) )
		return JS_CallFunctionValue(cx, JS_GetGlobalObject(cx), fval, JL_ARGC, JL_FARGV, &fval);
	return JS_TRUE;
	JL_BAD;
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

	JSObject *obj = JL_FOBJ;

	JL_S_ASSERT_ARG_RANGE(1, 2);

	bool useAndSaveCompiledScripts;
	useAndSaveCompiledScripts = !JL_FARG_ISDEF(2) || JL_FARG(2) == JSVAL_TRUE;
	const char *filename;
	JL_CHK( JsvalToString(cx, &JL_FARG(1), &filename) );

	uint32 oldopts;
	oldopts = JS_SetOptions(cx, JS_GetOptions(cx) | JSOPTION_COMPILE_N_GO); // JSOPTION_COMPILE_N_GO is properly removed in JLLoadScript if needed.
	JSScript *script;
	script = JLLoadScript( cx, obj, filename, useAndSaveCompiledScripts, useAndSaveCompiledScripts );
	JS_SetOptions(cx, oldopts);
	JL_CHK( script );

	JSTempValueRooter tvr;
	JSObject *scrobj;
	scrobj = JS_NewScriptObject(cx, script);
	JS_PUSH_TEMP_ROOT_OBJECT(cx, scrobj, &tvr);
	JSBool ok;
	ok = JS_ExecuteScript(cx, obj, script, JL_FRVAL); // Doc: On successful completion, rval is a pointer to a variable that holds the value from the last executed expression statement processed in the script.
	JS_POP_TEMP_ROOT(cx, &tvr);
	JL_CHK( ok );

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

	JLSemaphoreHandler semEnd;
	unsigned int maxExecutionTime;
	JSContext *cx;
	jsval queryFunctionValue;
};

JSBool SandboxMaxOperationCallback(JSContext *cx) {

	JSObject *branchLimitExceptionObj = JS_NewObject(cx, JL_CLASS(OperationLimit), NULL, NULL);
	JL_CHK( branchLimitExceptionObj );
	JS_SetPendingException(cx, OBJECT_TO_JSVAL( branchLimitExceptionObj ));
	JL_BAD;
}

JLThreadFuncDecl SandboxWatchDogThreadProc(void *threadArg) {

	JSContext *scx = (JSContext*)threadArg;
	SandboxContextPrivate *pv = (SandboxContextPrivate*)JS_GetContextPrivate(scx);

	//	SleepMilliseconds(pv->maxExecutionTime);
	JLAcquireSemaphore(pv->semEnd, pv->maxExecutionTime); // used as a breakable Sleep. This avoids to Cancel the thread

	JS_TriggerOperationCallback(scx);
	JLThreadExit();
	return 0;
}

JSBool SandboxQueryFunction(JSContext *scx, uintN argc, jsval *vp) {

	SandboxContextPrivate *pv = (SandboxContextPrivate*)JS_GetContextPrivate(scx);
	JSContext *cx = pv->cx; // needed to send errors in the right context.
	if ( JSVAL_IS_VOID( pv->queryFunctionValue ) ) {

		*JL_FRVAL = JSVAL_VOID;
	} else {

		JL_CHK( JS_CallFunctionValue(scx, JL_FOBJ, pv->queryFunctionValue, JL_ARGC, JL_FARGV, JL_FRVAL) );
		JL_CHKM( JSVAL_IS_PRIMITIVE(*JL_FRVAL), "Only primitive value can be returned." );
	}
	return JS_TRUE;
	JL_BAD;
}

DEFINE_FUNCTION_FAST( SandboxEval ) {

	JSContext *scx = NULL;

	JL_S_ASSERT_ARG_RANGE(1, 3);

	scx = JS_NewContext(JS_GetRuntime(cx), 8192L); // see host/host.cpp
	JL_CHK( scx );
	JS_SetOptions(scx, JS_GetOptions(cx) | JSOPTION_JIT | JSOPTION_DONT_REPORT_UNCAUGHT | JSOPTION_COMPILE_N_GO | JSOPTION_RELIMIT); // new options are based on host's options. cf. moz bz#490616

	JSObject *globalObject;
	globalObject = JS_NewObject(scx, JL_CLASS(Sandbox), NULL, NULL);
	JL_CHK( globalObject );
	*JL_FRVAL = OBJECT_TO_JSVAL(globalObject); // GC protection

	SandboxContextPrivate pv;
	pv.cx = cx;

	if ( JL_FARG_ISDEF(2) ) {

		JL_S_ASSERT_FUNCTION( JL_FARG(2) );
		pv.queryFunctionValue = JL_FARG(2);
		JL_CHK( JS_DefineFunction(scx, globalObject, "Query", (JSNative)SandboxQueryFunction, 1, JSFUN_FAST_NATIVE | JSPROP_PERMANENT | JSPROP_READONLY) );
	} else {

		pv.queryFunctionValue = JSVAL_VOID;
	}

	if ( JL_FARG_ISDEF(3) )
		JL_CHK( JsvalToUInt(cx, JL_FARG(3), &pv.maxExecutionTime) );
	else
		pv.maxExecutionTime = 1000; // default value

	JSString *jsstr;
	jsstr = JS_ValueToString(cx, JL_FARG(1));
	JL_CHK( jsstr );
	uintN srclen;
	srclen = JL_GetStringLength(jsstr);
	jschar *src;
	src = JS_GetStringChars(jsstr);

	JSOperationCallback prev;
	prev = JS_SetOperationCallback(scx, SandboxMaxOperationCallback);

	JS_SetGlobalObject(scx, globalObject);
	JSStackFrame *fp;
	fp = JS_GetScriptedCaller(cx, NULL);
	JL_CHK( fp );
	JSScript *script;
	script = JS_GetFrameScript(cx, fp);
	JL_CHK( script );
	const char *filename;
	filename = JS_GetScriptFilename(cx, script);
	jsbytecode *pc;
	pc = JS_GetFramePC(cx, fp);
	uintN lineno;
	lineno = JS_PCToLineNumber(cx, script, pc);

	JS_SetContextPrivate(scx, &pv);

	pv.semEnd = JLCreateSemaphore(0);

	JLThreadHandler sandboxWatchDogThread;
	sandboxWatchDogThread = JLThreadStart(SandboxWatchDogThreadProc, scx);
	if ( !JLThreadOk(sandboxWatchDogThread) ) {

		char reason[1024];
		JLLastSysetmErrorMessage(reason, sizeof(reason));
		JL_REPORT_ERROR_NUM(cx, JLSMSG_OS_ERROR, reason);
	}

	JSBool ok;
	ok = JS_EvaluateUCScript(scx, globalObject, src, srclen, filename, lineno, JL_FRVAL);

	JL_CHK( JLReleaseSemaphore(pv.semEnd) );

	JL_CHK( JLWaitThread(sandboxWatchDogThread) );
	JL_CHK( JLFreeThread(&sandboxWatchDogThread) );
	JL_CHK( JLFreeSemaphore(&pv.semEnd) );

	prev = JS_SetOperationCallback(scx, prev);
	JL_S_ASSERT( prev == SandboxMaxOperationCallback, "Invalid SandboxMaxOperationCallback handler." );

	JL_CHK( JS_DeleteProperty(cx, globalObject, "Query") );

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
  Returns $TRUE if _statementString_ is a valid Javascript statement.
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

	JL_S_ASSERT_ARG(1);
	const char *buffer;
	unsigned int length;
	JL_CHK( JsvalToStringAndLength(cx, &JL_ARG(1), &buffer, &length) );
	JL_CHK( BoolToJsval(cx, JS_BufferIsCompilableUnit(cx, obj, buffer, length) == JS_TRUE, rval) );
	return JS_TRUE;
	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Stop the execution of the program. This is a ungraceful way to finish a program and should only be used in critical cases.
**/
DEFINE_FUNCTION_FAST( Halt ) {

	JL_REPORT_ERROR_NUM(cx, JLSMSG_PROGRAM_STOPPED);
bad:	
	return JS_FALSE;
}


/**doc
=== Static properties ===
**/



/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME $READONLY
  Is the filename of the script being executed.
  $H note
   The current filename is also available using: `StackFrameInfo(stackSize-1).filename` (see jsdebug module)
**/
DEFINE_PROPERTY( currentFilename ) {

	JSStackFrame *fp = JL_CurrentStackFrame(cx);
	if ( fp == NULL ) {

		*vp = JSVAL_VOID;
		return JS_TRUE;
	}
	JSScript *script = JS_GetFrameScript(cx, fp);
	if ( script == NULL ) {

		*vp = JSVAL_VOID;
		return JS_TRUE;
	}

	const char *filename = JS_GetScriptFilename(cx, script);
	JL_CHK( StringToJsval(cx, filename, vp) );
	return JS_TRUE;
	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME
  if $TRUE if the current function is being called as a constructor.
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

JSGCCallback prevJSGCCallback = NULL; // (TBD) restore the previous callback at the end (on REMOVE_CLASS ?)

JSBool VetoingGCCallback(JSContext *cx, JSGCStatus status) {

	// doc. JSGC_BEGIN: Start of GC. The callback may prevent GC from starting by returning JS_FALSE.
	//      But even if the callback returns JS_TRUE, the garbage collector may determine that GC is not necessary,
	//      in which case the other three callbacks are skipped.
	if ( status == JSGC_BEGIN )
		return JS_FALSE;
	return prevJSGCCallback ? prevJSGCCallback(cx, status) : JS_TRUE;
}

DEFINE_PROPERTY_SETTER( disableGarbageCollection ) {

	// <shaver>	you could install a vetoing callback!
	// <crowder>	oh, true
	bool disableGC;
	JL_CHK( JsvalToBool(cx, *vp, &disableGC) );
	if ( disableGC ) {

		JSGCCallback tmp = JS_SetGCCallback(cx, VetoingGCCallback);
		if ( tmp != VetoingGCCallback )
			prevJSGCCallback = tmp;
	} else {

		JSGCCallback tmp = JS_SetGCCallback(cx, prevJSGCCallback);
		if ( tmp != VetoingGCCallback )
			JS_SetGCCallback(cx, tmp);
	}
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( disableGarbageCollection ) {

	JSGCCallback cb = JS_SetGCCallback(cx, NULL);
	JS_SetGCCallback(cx, cb);
	*vp = cb == VetoingGCCallback ? JSVAL_TRUE : JSVAL_FALSE;
	return JS_TRUE;
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

JSBool testProp(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION_FAST( Test ) {

	return JS_TRUE;
}

#endif // _DEBUG


CONFIGURE_STATIC

	REVISION(JL_SvnRevToInt("$Revision$"))

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION_FAST( Expand )
		FUNCTION_FAST( InternString )
		FUNCTION_FAST( Seal )
		FUNCTION_FAST( Clear )
		FUNCTION_FAST( SetScope )
//		FUNCTION_FAST( GetCurrentScope )
//		FUNCTION( HideProperties )
		FUNCTION_FAST_ARGC( SetPropertyEnumerate, 3 )
		FUNCTION_FAST_ARGC( SetPropertyReadonly, 3 )

		FUNCTION_FAST( Exec )
		FUNCTION_FAST(SandboxEval)
		FUNCTION( IsStatementValid )
		FUNCTION_FAST( StringRepeat )
		FUNCTION_FAST( Print )
		FUNCTION_FAST( Sleep )
		FUNCTION_FAST( TimeCounter )
		FUNCTION_FAST( CollectGarbage )
		FUNCTION_FAST( MaybeCollectGarbage )
		FUNCTION_FAST( ObjectToId )
		FUNCTION_FAST( IdToObject )
		FUNCTION_FAST_ARGC( IsBoolean, 1 )
		FUNCTION_FAST_ARGC( IsNumber, 1 )
		FUNCTION_FAST_ARGC( IsPrimitive, 1 )
		FUNCTION_FAST_ARGC( IsFunction, 1 )
//		FUNCTION_FAST_ARGC( IsVoid, 1 ) // value === undefined is better
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
		PROPERTY_READ( currentFilename )
		PROPERTY_READ( isConstructing )
		PROPERTY( disableGarbageCollection )
//		PROPERTY( processPriority )
#ifdef _DEBUG
//		PROPERTY( testProp )
#endif // _DEBUG
	END_STATIC_PROPERTY_SPEC

END_STATIC
