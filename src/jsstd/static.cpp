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
#include "jsstd.h"
#include "jslibsModule.h"


#include "jsxdrapi.h"
#include "jscntxt.h"
//#include "jsnum.h"
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
  Return an expanded string using key/value stored in _obj_ or the falue computed by a function.
  $H note
   $UNDEF or null values are ignored in the resulting string.
  $H example 1
  {{{
  Expand('$(h) $(xxx) $(w)', { h:'Hello', w:'World' }); // returns "Hello  World"
  }}}
  $H example 2
  {{{
  Expand('$(foo)-$(bar)', function(id) '<'+id+'>' ); // returns "<foo>-<bar>"
  }}}
**/
DEFINE_FUNCTION( Expand ) {

	JL_DEFINE_FUNCTION_OBJ;

	typedef struct {
		const jschar *chars;
		size_t count;
		JSString *root;
	} Chunk;

	jl::Stack<Chunk, jl::StaticAllocMedium> stack;
	jsval value;

	JLStr srcStr;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &srcStr) );

	JSObject *mapObj = NULL;
	jsval *mapFct = NULL;
	
	if ( JL_ARG_ISDEF(2) ) {
	
		if ( JL_IsFunction(cx, JL_ARG(2)) )
			mapFct = &JL_ARG(2);
		else
		if ( !JSVAL_IS_PRIMITIVE(JL_ARG(2)) )
			mapObj = JSVAL_TO_OBJECT(JL_ARG(2));
	}

	ssize_t total;

	const jschar *src, *srcEnd;
	src = srcStr.GetConstJsStr();
	srcEnd = src + srcStr.Length();

	const jschar *txt, *key, *keyEnd;

	total = 0;
	txt = src;
	for (;;) {

		key = txt;
		do {

			key = wmemchr(key, L'$', srcEnd-key);
			if ( key == NULL || ++key == srcEnd ) {

				++stack;
				stack->chars = txt;
				stack->count = srcEnd - txt;
				total += stack->count;
				goto assemble;
			}
		} while ( *key != L'(' );

		if ( key ) {

			size_t txtLen = key - 1 - txt;
			if ( txtLen > 0 ) {
				
				++stack;
				stack->chars = txt;
				stack->count = txtLen;
				total += txtLen;
			}

			++key;
			keyEnd = wmemchr(key, L')', srcEnd-key);
			
			if ( keyEnd ) {

				txt = keyEnd+1;
				
				if ( mapObj ) {

					JL_CHK( JS_GetUCProperty(cx, mapObj, key, keyEnd - key, &value) );
				} else
				if ( mapFct ) {

					JL_CHK( JL_NativeToJsval(cx, key, keyEnd - key, &value) );
					JL_CHK( JS_CallFunctionValue(cx, obj, *mapFct, 1, &value, &value) );
				} else {

					continue;
				}

				if ( js::Valueify(value).isNullOrUndefined() )
					continue;

				++stack;

				if ( !JSVAL_IS_STRING(value) ) { // 'convert to string' and 'root new string' if necessary.

					stack->root = JS_ValueToString(cx, value);
					JL_CHK( stack->root );
					JL_CHK( JS_AddStringRoot(cx, &stack->root) );
					stack->chars = JS_GetStringCharsAndLength(cx, stack->root, &stack->count);
			} else {

					stack->root = NULL;
					stack->chars = JS_GetStringCharsAndLength(cx, JSVAL_TO_STRING(value), &stack->count);
				}

				total += stack->count;
			} else {

				goto assemble;
			}
		}
	}

assemble:
	jschar *res, *tmp;
	res = (jschar*)JS_malloc(cx, total * sizeof(jschar) + 2);
	JL_CHK( res );
	res[total] = 0;

	tmp = res + total;
	
	for ( ; stack; --stack ) {

		tmp -= stack->count;
		wmemcpy(tmp, stack->chars, stack->count);
		if ( stack->root != NULL )
			JS_RemoveStringRoot(cx, &stack->root);
	}

	JSString *jsstr;
	jsstr = JL_NewUCString(cx, res, total);
	JL_CHK( jsstr );
	*JL_RVAL = STRING_TO_JSVAL( jsstr );

	return JS_TRUE;

bad:
	while ( stack ) {

		if ( stack->root != NULL )
			JS_RemoveStringRoot(cx, &stack->root);
		--stack;
	}
	return JS_FALSE;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
/ **doc
$TOC_MEMBER $INAME
 $VAL $INAME( value, case1, result1, case2, result2, ..., caseN, resultN, defaultResult )
  Based on _value_, returns _resultN_ value for the matching _caseN_, or _defaultResult_ if sothing match.
** /
DEFINE_FUNCTION( SwitchCase ) {

	JL_S_ASSERT_ARG_MIN( 1 );

	if ( argc <= 2 ) {
		
		*JL_RVAL = JSVAL_VOID;
		return JS_TRUE;
	}

	unsigned int i;
	for ( i = 1; i < argc; i += 2 )
		
		if ( JS_SameValue(cx, JL_ARGV[0], JL_ARGV[i]) ) { // see also JS_StrictlyEqual
			
			*JL_RVAL = JL_ARGV[i+1];
			return JS_TRUE;
		}

	if ( i > argc ) {

		*JL_RVAL = JL_ARGV[argc-1];
		return JS_TRUE;
	}

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}
*/

/**doc
$TOC_MEMBER $INAME
 $VAL $INAME( value, caseArray, resultArray [, defaultResult] )
**/
DEFINE_FUNCTION( SwitchCase ) {

	JL_S_ASSERT_ARG_RANGE( 3, 4 );

	JL_S_ASSERT_ARRAY( JL_ARG(2) );
	JL_S_ASSERT_ARRAY( JL_ARG(3) );

	JSObject *caseArray;
	jsuint caseArrayLength;
	caseArray = JSVAL_TO_OBJECT(JL_ARG(2));
	JL_CHK( JS_GetArrayLength(cx, caseArray, &caseArrayLength) );

	jsuint i;
	for ( i = 0; i < caseArrayLength; ++i ) {
	
		JL_CHK( JS_GetElement(cx, caseArray, i, JL_RVAL) );
		
		JSBool same;
		JL_CHK( JS_SameValue(cx, JL_ARG(1), *JL_RVAL, &same) );
		if ( same )
			return JS_GetElement(cx, JSVAL_TO_OBJECT(JL_ARG(3)), i, JL_RVAL);
	}

	*JL_RVAL = argc >= 4 ? JL_ARG(4) : JSVAL_VOID;
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
DEFINE_FUNCTION( InternString ) {

	JSString *jsstr = JS_ValueToString(cx, vp[2]);
	JL_CHK( jsstr );

	size_t length;
	const jschar *chars;
	chars = JS_GetStringCharsAndLength(cx, jsstr, &length);
	JL_CHK( chars );
	JL_CHK( JS_InternUCStringN(cx, chars, length) );
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( obj )
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
DEFINE_FUNCTION( DeepFreezeObject ) {

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_OBJECT( JL_ARG(1) );
	//JL_CHK( JS_ValueToObject(cx, JL_ARG(1), &obj) );
	*JL_RVAL = JSVAL_VOID;
	return JS_DeepFreezeObject(cx, JSVAL_TO_OBJECT(JL_ARG(1)));
	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $INT $INAME( obj )
  Returns the number of own properties of an object.
**/
DEFINE_FUNCTION( CountProperties ) {

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_OBJECT( JL_ARG(1) );

	JSIdArray *arr;
	arr = JS_Enumerate(cx, JSVAL_TO_OBJECT(JL_ARG(1)));
	*JL_RVAL = INT_TO_JSVAL(arr->length);
	JS_DestroyIdArray(cx, arr);

	return JS_TRUE;
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

  ClearObject(obj);
  Print( uneval(obj) ); // prints: ({})
  }}}
**/
DEFINE_FUNCTION( ClearObject ) {

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_OBJECT( JL_ARG(1) );
	JS_ClearScope(cx, JSVAL_TO_OBJECT( JL_ARG(1) ));
	
	*JL_RVAL = JSVAL_VOID;
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
DEFINE_FUNCTION( SetScope ) {

	JL_S_ASSERT_ARG(2);
	JSObject *o, *p;
	JL_CHK( JS_ValueToObject(cx, JL_ARG(1), &o) ); // o = JSVAL_TO_OBJECT(JL_ARG(1));
	JL_CHK( JS_ValueToObject(cx, JL_ARG(2), &p) ); // p = JSVAL_TO_OBJECT(JL_ARG(2));
	*JL_RVAL = OBJECT_TO_JSVAL( JS_GetParent(cx, o) );
	JL_CHK( JS_SetParent(cx, o, p) );
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

//unsigned int lastObjectId = 0;
//unsigned int objectIdAllocated = 0;

//JSGCCallback prevObjectIdGCCallback = NULL;

JSBool ObjectIdGCCallback(JSContext *cx, JSGCStatus status) {

	ModulePrivate *mpv = (ModulePrivate*)JL_GetModulePrivate(cx, _moduleId);
	if ( status == JSGC_MARK_END ) {

		for ( ObjId *it = mpv->objIdList, *end = mpv->objIdList + mpv->objectIdAllocated; it < end; ++it ) {

			if ( it->obj && JS_IsAboutToBeFinalized(cx, it->obj) ) {

				it->id = 0;
				it->obj = NULL;
			}
		}
	}
	return mpv->prevObjectIdGCCallback ? mpv->prevObjectIdGCCallback(cx, status) : JS_TRUE;
}


DEFINE_FUNCTION( ObjectToId ) {

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_OBJECT( JL_ARG(1) );
	JSObject *obj;
	obj = JSVAL_TO_OBJECT( JL_ARG(1) );

	ModulePrivate *mpv = (ModulePrivate*)JL_GetModulePrivate(cx, _moduleId);

	ObjId *freeSlot;
	freeSlot = NULL;
	for ( ObjId *it = mpv->objIdList, *end = mpv->objIdList + mpv->objectIdAllocated; it < end; ++it ) {

		if ( it->obj == obj )
			return JL_NativeToJsval(cx, it->id, JL_RVAL);
		if ( !freeSlot && it->id == 0 )
			freeSlot = it;
	}

	if ( !freeSlot ) {

		unsigned int prevAlloced = mpv->objectIdAllocated;

		if ( !mpv->objIdList ) {

			mpv->prevObjectIdGCCallback = JS_SetGCCallback(cx, ObjectIdGCCallback);
			mpv->objectIdAllocated = 32;
		} else {

			mpv->objectIdAllocated *= 2;
		}
		mpv->objIdList = (ObjId*)jl_realloc(mpv->objIdList, sizeof(ObjId) * mpv->objectIdAllocated); // (TBD) free mpv->objIdList at the end !
		freeSlot = mpv->objIdList + prevAlloced;
		memset(freeSlot, 0, (mpv->objectIdAllocated - prevAlloced) * sizeof(ObjId)); // init only new slots
	}

	freeSlot->id = ++mpv->lastObjectId;
	freeSlot->obj = obj;

	return JL_NativeToJsval(cx, mpv->lastObjectId, JL_RVAL);
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
DEFINE_FUNCTION( IdToObject ) {

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_NUMBER( JL_ARG(1) );

	unsigned int id;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &id) );

	ModulePrivate *mpv = (ModulePrivate*)JL_GetModulePrivate(cx, _moduleId);

	if ( id > 0 && id <= mpv->lastObjectId ) {

		for ( ObjId *it = mpv->objIdList, *end = mpv->objIdList + mpv->objectIdAllocated; it < end; ++it ) {

			if ( it->id == id ) {

				*JL_RVAL = OBJECT_TO_JSVAL( it->obj );
				return JS_TRUE;
			}
		}
	}

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME()
  Returns $TRUE if the value is a boolean value or object.
**/
DEFINE_FUNCTION( IsBoolean ) {

	if ( JSVAL_IS_BOOLEAN(JL_ARG(1)) ) {

		*JL_RVAL = JSVAL_TRUE;
		return JS_TRUE;
	}

	if ( JSVAL_IS_PRIMITIVE(JL_ARG(1)) ) {

		*JL_RVAL = JSVAL_FALSE;
		return JS_TRUE;
	}

	*JL_RVAL = JL_GetClass(JSVAL_TO_OBJECT(JL_ARG(1))) == JL_GetStandardClassByKey(cx, JSProto_Boolean) ? JSVAL_TRUE : JSVAL_FALSE;
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME()
  Returns $TRUE if the value is a number value or object.
**/
DEFINE_FUNCTION( IsNumber ) {

	if ( JSVAL_IS_NUMBER(JL_ARG(1)) ) {

		*JL_RVAL = JSVAL_TRUE;
		return JS_TRUE;
	}

	if ( JSVAL_IS_PRIMITIVE(JL_ARG(1)) ) {

		*JL_RVAL = JSVAL_FALSE;
		return JS_TRUE;
	}

	*JL_RVAL = JL_GetClass(JSVAL_TO_OBJECT(JL_ARG(1))) == JL_GetStandardClassByKey(cx, JSProto_Number) ? JSVAL_TRUE : JSVAL_FALSE;
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME()
  Returns $TRUE if the value is a primitive ( null or not an object ).
**/
DEFINE_FUNCTION( IsPrimitive ) {

	*JL_RVAL = JSVAL_IS_PRIMITIVE(JL_ARG(1)) ? JSVAL_TRUE : JSVAL_FALSE;
	return JS_TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME()
  Returns $TRUE if the value is a function.
**/
DEFINE_FUNCTION( IsFunction ) {

	*JL_RVAL = VALUE_IS_FUNCTION(cx, JL_ARG(1)) ? JSVAL_TRUE : JSVAL_FALSE;
	return JS_TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME()
  Returns $TRUE if the value is a generator.
**/
DEFINE_FUNCTION( IsGenerator ) {

	if ( !JSVAL_IS_OBJECT(JL_ARG(1)) ) {

		*JL_RVAL = JSVAL_FALSE;
		return JS_TRUE;
	}

	jsval prop;
	JL_CHK( JS_GetPropertyById(cx, JSVAL_TO_OBJECT(JL_ARG(1)), JL_ATOMJSID(cx, iterator), &prop) );

	*JL_RVAL = VALUE_IS_FUNCTION(cx, prop) ? JSVAL_TRUE : JSVAL_FALSE;
	return JS_TRUE;
	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( text )
  Report the given _text_ as warning. The warning is reported on the stderr. Warnings are ignored in unsafeMode.
**/
DEFINE_FUNCTION( Warning ) {

	JLStr str;
	JL_S_ASSERT_ARG(1);
//	const char *message;

	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &str) );
	JL_CHK( JS_ReportWarning(cx, "%s", str.GetConstStr()) );
	
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( expression [, failureMessage ] )
  If the argument expression compares equal to zero, the failureMessage is written to the standard error device and the program stops its execution. Asserts are ignored in unsafeMode.
  $H example
  {{{
  var foo = ['a', 'b', 'c'];
  $INAME( i >= 0 || i < 3, 'Invalid value.' );
  Print( foo[i] );
  }}}
  $H note
   The error output can be redirected by redefining the _host.stderr function. see the Print() function.
**/
DEFINE_FUNCTION( Assert ) {

	if ( !JL_IS_SAFE )
		return JS_TRUE;

	JL_S_ASSERT_ARG_RANGE(1,2);

	// see. js_DecompileValueGenerator  (http://infomonkey.cdleary.com/questions/144/how-to-get-the-script-text-code-at-runtime)

	bool assert;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &assert) );
	if ( !assert ) {

		JLStr str;
		if ( JL_ARG_ISDEF(2) )
			JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &str) );
		else
			str = JLStr("Assertion failed.", true);
		
		JS_ReportError( cx, "%s", str.GetConstStr());
		return JS_FALSE;
	}

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $INT $INAME()
  Performs an unconditional garbage collection in the JS memory pool.
**/
DEFINE_FUNCTION( CollectGarbage ) {

	size_t gcBytesDiff = cx->runtime->gcBytes;
	JS_GC( cx );
	gcBytesDiff = cx->runtime->gcBytes - gcBytesDiff;
	return JL_NativeToJsval(cx, gcBytesDiff, JL_RVAL);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $INT $INAME()
  Performs a conditional garbage collection of JS objects, doubles, and strings that are no longer needed by a script executing.
  This offers the JavaScript engine an opportunity to perform garbage collection if needed.
**/
DEFINE_FUNCTION( MaybeCollectGarbage ) {

	size_t gcBytesDiff = cx->runtime->gcBytes;
	JS_MaybeGC( cx );
	gcBytesDiff = cx->runtime->gcBytes - gcBytesDiff;
	return JL_NativeToJsval(cx, gcBytesDiff, JL_RVAL);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $REAL $INAME( time )
  Suspends the execution of the current program during _time_ milliseconds.
**/
DEFINE_FUNCTION( Sleep ) {

	JL_S_ASSERT_ARG(1);
	unsigned int time;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &time) );
	SleepMilliseconds(time);
	
	*JL_RVAL = JSVAL_VOID;
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
  Print( 't0: '+TimeCounter(), '\n' );
  Print( 't1: '+TimeCounter(), '\n' );
  Sleep(100);
  Print( 't2: '+TimeCounter(), '\n' );
  }}}
  prints:
  {{{
  t0: 0
  t1: 0.15615914588863317
  t2: 100.02473070050955
  }}}
**/
DEFINE_FUNCTION( TimeCounter ) {

	return JL_NativeToJsval(cx, AccurateTimeCounter(), JL_RVAL);
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
DEFINE_FUNCTION( StringRepeat ) {

	JLStr str;

	JL_S_ASSERT_ARG(2);

	size_t count;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &count) );
	if ( count == 0 ) {

		*JL_RVAL = JL_GetEmptyStringValue(cx);
		return JS_TRUE;
	}
	if ( count == 1 ) {

		*JL_RVAL = STRING_TO_JSVAL( JS_ValueToString(cx, JL_ARG(1)) ); // force string conversion because we must return a string.
		return JS_TRUE;
	}

	size_t len;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &str) );
	len = str.Length();

	if ( len == 0 ) {

		*JL_RVAL = JL_GetEmptyStringValue(cx);
		return JS_TRUE;
	}

	size_t newLen;
	newLen = len * count;

	jschar *newBuf;
	newBuf = static_cast<jschar*>(jl_malloc(sizeof(jschar) * (newLen +1)));
	JL_S_ASSERT_ALLOC( newBuf );
	newBuf[newLen] = 0;
	
	const jschar *buf;
	buf = str.GetConstJsStr();

	jschar *tmp = newBuf;
	size_t i;
	for ( i = 0; i < count; ++i ) {

		memcpy(tmp, buf, len * sizeof(jschar));
		tmp += len;
	}

	JSString *jsstr;
	jsstr = JL_NewUCString(cx, newBuf, newLen);
	JL_CHK( jsstr );
	*JL_RVAL = STRING_TO_JSVAL( jsstr );
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
   The output can be redirected by redefining the _host.stdout function.
   $H example 1
   {{{
   LoadModule('jsstd');
   Print('Hello', 'World'); // prints: HelloWorld
   }}}
   $H example 2
   {{{
   LoadModule('jsstd');
   Print('foo\n'); // prints: foo
   _host.stdout = function() {}
   Print('bar\n'); // prints nothing
   }}}
**/
DEFINE_FUNCTION( Print ) {

	// Print() => _host->stdout() => JSDefaultStdoutFunction() => pv->hostStdOut()

	jsval fval;
	JL_CHK( GetHostObjectValue(cx, JLID(cx, stdout), &fval) );
	*JL_RVAL = JSVAL_VOID;
	if (likely( JL_IsFunction(cx, fval) ))
		return JS_CallFunctionValue(cx, JL_GetGlobalObject(cx), fval, JL_ARGC, JL_ARGV, &fval);
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
DEFINE_FUNCTION( Exec ) {

	JLStr str;
	JSObject *scriptObjRoot;
	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_ARG_RANGE(1, 2);

	bool useAndSaveCompiledScripts;
	useAndSaveCompiledScripts = !JL_ARG_ISDEF(2) || JL_ARG(2) == JSVAL_TRUE;
//	const char *filename;
//	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &filename) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &str) );

	uint32 oldopts;
	oldopts = JS_SetOptions(cx, JS_GetOptions(cx) | JSOPTION_COMPILE_N_GO); // JSOPTION_COMPILE_N_GO is properly removed in JLLoadScript if needed.
	JSScript *script;
	script = JL_LoadScript(cx, obj, str, useAndSaveCompiledScripts, useAndSaveCompiledScripts);
	JS_SetOptions(cx, oldopts);
	JL_CHK( script );

	scriptObjRoot = JS_NewScriptObject(cx, script);
	
	JSBool ok;
	ok = JS_ExecuteScript(cx, obj, script, JL_RVAL); // Doc: On successful completion, rval is a pointer to a variable that holds the value from the last executed expression statement processed in the script.
	JS_DestroyScript(cx, script);
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

	JSObject *branchLimitExceptionObj = JS_NewObjectWithGivenProto(cx, JL_CLASS(OperationLimit), JL_PROTOTYPE(cx, OperationLimit), NULL);
	JL_CHK( branchLimitExceptionObj );
	JS_SetPendingException(cx, OBJECT_TO_JSVAL( branchLimitExceptionObj ));
	JL_BAD;
}

JLThreadFuncDecl SandboxWatchDogThreadProc(void *threadArg) {

	JSContext *scx = (JSContext*)threadArg;
	SandboxContextPrivate *pv = (SandboxContextPrivate*)JS_GetContextPrivate(scx);

	//	SleepMilliseconds(pv->maxExecutionTime);
	JLSemaphoreAcquire(pv->semEnd, pv->maxExecutionTime); // used as a breakable Sleep. This avoids to Cancel the thread

	JS_TriggerOperationCallback(scx);
	JLThreadExit(0);
	return 0;
}

JSBool SandboxQueryFunction(JSContext *scx, uintN argc, jsval *vp) {

	SandboxContextPrivate *pv = (SandboxContextPrivate*)JS_GetContextPrivate(scx);
	JSContext *cx = pv->cx; // needed to send errors in the right context.
	if ( JSVAL_IS_VOID( pv->queryFunctionValue ) ) {

		*JL_RVAL = JSVAL_VOID;
	} else {

		JL_CHK( JS_CallFunctionValue(scx, JS_THIS_OBJECT(scx, vp), pv->queryFunctionValue, JL_ARGC, JL_ARGV, JL_RVAL) );
		JL_CHKM( JSVAL_IS_PRIMITIVE(*JL_RVAL), "Only primitive value can be returned." );
	}
	return JS_TRUE;
	JL_BAD;
}

DEFINE_FUNCTION( SandboxEval ) {

	JSContext *scx = NULL;

	JL_S_ASSERT_ARG_RANGE(1, 3);

	scx = JS_NewContext(JL_GetRuntime(cx), 8192L); // see host/host.cpp
	JL_CHK( scx );
	JS_SetOptions(scx, JS_GetOptions(cx) | JSOPTION_JIT | JSOPTION_METHODJIT | JSOPTION_DONT_REPORT_UNCAUGHT | JSOPTION_COMPILE_N_GO | JSOPTION_RELIMIT); // new options are based on host's options. cf. moz bz#490616

	JSObject *globalObject;
	globalObject = JS_NewGlobalObject(scx, JL_CLASS(Sandbox));
	//	globalObject = JS_NewCompartmentAndGlobalObject(cx, JL_CLASS(Sandbox), NULL);
	JL_CHK( globalObject );
	*JL_RVAL = OBJECT_TO_JSVAL(globalObject); // GC protection

	SandboxContextPrivate pv;
	pv.cx = cx;

	if ( JL_ARG_ISDEF(2) ) {

		JL_S_ASSERT_FUNCTION( JL_ARG(2) );
		pv.queryFunctionValue = JL_ARG(2);
		JL_CHK( JS_DefineFunction(scx, globalObject, "Query", SandboxQueryFunction, 1, JSPROP_PERMANENT | JSPROP_READONLY) );
	} else {

		pv.queryFunctionValue = JSVAL_VOID;
	}

	if ( JL_ARG_ISDEF(3) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &pv.maxExecutionTime) );
	else
		pv.maxExecutionTime = 1000; // default value

	JSString *jsstr;
	jsstr = JS_ValueToString(cx, JL_ARG(1));
	JL_CHK( jsstr );

//	size_t srclen;
//	srclen = JL_GetStringLength(jsstr);
//	jschar *src;
//	src = JS_GetStringChars(jsstr);
	size_t srclen;
	const jschar *src;
	src = JS_GetStringCharsAndLength(cx, jsstr, &srclen);

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

	pv.semEnd = JLSemaphoreCreate(0);

	JLThreadHandler sandboxWatchDogThread;
	sandboxWatchDogThread = JLThreadStart(SandboxWatchDogThreadProc, scx);
	if ( !JLThreadOk(sandboxWatchDogThread) ) {

		char reason[1024];
		JLLastSysetmErrorMessage(reason, sizeof(reason));
		JL_REPORT_ERROR_NUM(cx, JLSMSG_OS_ERROR, reason);
	}

	JSBool ok;
	ok = JS_EvaluateUCScript(scx, globalObject, src, (uintN)srclen, filename, lineno, JL_RVAL);

	JLSemaphoreRelease(pv.semEnd);

	JLThreadWait(sandboxWatchDogThread, NULL);
	JLThreadFree(&sandboxWatchDogThread);
	JLSemaphoreFree(&pv.semEnd);

	prev = JS_SetOperationCallback(scx, prev);
	JL_S_ASSERT( prev == SandboxMaxOperationCallback, "Invalid SandboxMaxOperationCallback handler." );

	JL_CHK( JS_DeleteProperty(scx, globalObject, "Query") );

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

	JLStr str;
	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_ARG(1);

	//const char *buffer;
	//size_t length;
	//JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(1), &buffer, &length) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &str) );
	JL_CHK( JL_NativeToJsval(cx, JS_BufferIsCompilableUnit(cx, obj, str.GetConstStr(), str.Length()) == JS_TRUE, JL_RVAL) );
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
	JL_CHK( JL_NativeToJsval(cx, filename, vp) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME $READONLY
  Is the line number of the script being executed.
**/
DEFINE_PROPERTY( currentLineNumber ) {
	
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

	uintN lineno;
	lineno = JS_PCToLineNumber(cx, script, JS_GetFramePC(cx, fp));

	JL_CHK( JL_NativeToJsval(cx, lineno, vp) );
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

	*vp = BOOLEAN_TO_JSVAL( JS_IsConstructing(cx, vp) );
	return JS_TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME
  Set to $TRUE, this property desactivates the garbage collector.
**/

//JSGCCallback prevJSGCCallback = NULL; // (TBD) restore the previous callback at the end (on REMOVE_CLASS ?)

JSBool VetoingGCCallback(JSContext *cx, JSGCStatus status) {

	ModulePrivate *mpv = (ModulePrivate*)JL_GetModulePrivate(cx, _moduleId);
	// doc. JSGC_BEGIN: Start of GC. The callback may prevent GC from starting by returning JS_FALSE.
	//      But even if the callback returns JS_TRUE, the garbage collector may determine that GC is not necessary,
	//      in which case the other three callbacks are skipped.
	if ( status == JSGC_BEGIN )
		return JS_FALSE;
	return mpv->prevJSGCCallback ? mpv->prevJSGCCallback(cx, status) : JS_TRUE;
}

DEFINE_PROPERTY_SETTER( disableGarbageCollection ) {

	// <shaver>	you could install a vetoing callback!
	// <crowder>	oh, true
	ModulePrivate *mpv = (ModulePrivate*)JL_GetModulePrivate(cx, _moduleId);
	bool disableGC;
	JL_CHK( JL_JsvalToNative(cx, *vp, &disableGC) );
	if ( disableGC ) {

		JSGCCallback tmp = JS_SetGCCallback(cx, VetoingGCCallback);
		if ( tmp != VetoingGCCallback )
			mpv->prevJSGCCallback = tmp;
	} else {

		JSGCCallback tmp = JS_SetGCCallback(cx, mpv->prevJSGCCallback);
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
//	JL_BAD;
}


DEFINE_FUNCTION( Test ) {
/*
	JSXDRState *xdr1 = JS_XDRNewMem(cx, JSXDR_ENCODE);
	JS_XDRValue(xdr1, &JL_ARG(1));
	uint32 length;
	void *buffer;
	buffer = JS_XDRMemGetData(xdr1, &length);
	
	JSXDRState *xdr2 = JS_XDRNewMem(cx, JSXDR_DECODE);
	JS_XDRMemSetData(xdr2, buffer, length);
	JS_XDRValue(xdr2, JL_RVAL);
	JS_XDRMemSetData(xdr2, NULL, 0);
	JS_XDRDestroy(xdr2);

	JS_XDRDestroy(xdr1);
*/
/*
	JSObject *arr = JS_NewArrayObject(cx, 0, NULL);
	jsval val;
	val = JSVAL_ONE;
	double t = AccurateTimeCounter();
	for ( int i = 0; i < 100000; ++i )
		JL_Push(cx, arr, &val); 
	t = AccurateTimeCounter() - t;
	printf( "%f\n", t );
*/
/*
	jschar *str = (jschar*)jl_malloc(2 * sizeof(jschar));
	str[1] = 0;
	*JL_RVAL = STRING_TO_JSVAL( JS_NewUCString(cx, str, 1) );
*/	
	
	for ( int i = 0; i < 100; i++ ) {
		char *data = (char*)jl_calloc(1, 10 * sizeof(char));
		JL_NewBlob(cx, data, 10, JL_RVAL);
	}

	return JS_TRUE;
}

#endif // _DEBUG


CONFIGURE_STATIC

	REVISION(JL_SvnRevToInt("$Revision$"))

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION( Expand )
		FUNCTION_ARGC( SwitchCase, 4 )
		FUNCTION( InternString )
		FUNCTION( DeepFreezeObject )
		FUNCTION( CountProperties )
		FUNCTION( ClearObject )
		FUNCTION( SetScope )
//		FUNCTION( GetCurrentScope )
//		FUNCTION( HideProperties )
//		FUNCTION_ARGC( SetPropertyEnumerate, 3 )
//		FUNCTION_ARGC( SetPropertyReadonly, 3 )

		FUNCTION( Exec )
		FUNCTION( SandboxEval )
		FUNCTION( IsStatementValid )
		FUNCTION( StringRepeat )
		FUNCTION( Print )
		FUNCTION( Sleep )
		FUNCTION( TimeCounter )
		FUNCTION( CollectGarbage )
		FUNCTION( MaybeCollectGarbage )
		FUNCTION( ObjectToId )
		FUNCTION( IdToObject )
		FUNCTION_ARGC( IsBoolean, 1 )
		FUNCTION_ARGC( IsNumber, 1 )
		FUNCTION_ARGC( IsPrimitive, 1 )
		FUNCTION_ARGC( IsFunction, 1 )
		FUNCTION_ARGC( IsGenerator, 1 )
//		FUNCTION_ARGC( IsVoid, 1 ) // value === undefined is better
		FUNCTION( Warning )
		FUNCTION( Assert )
		FUNCTION( Halt )
//		FUNCTION( StrSet )
#ifdef _DEBUG
		FUNCTION( Test )
#endif // _DEBUG
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_READ( currentFilename )
		PROPERTY_READ( currentLineNumber )
		PROPERTY_READ( isConstructing )
		PROPERTY( disableGarbageCollection )
//		PROPERTY( processPriority )
#ifdef _DEBUG
//		PROPERTY( testProp )
#endif // _DEBUG
	END_STATIC_PROPERTY_SPEC

END_STATIC



///////////////////////////////////////////////////////////////////////////////
// trash

/*
static jschar wstrstr(const jschar *src, const jschar *srcEnd, const jschar *sub, const jschar *subEnd) {

	size_t subLen = subEnd - sub;
	for (;;) {

		const jschar *pos = wmemchr(src, sub[0], srcEnd - src);
		if ( pos == NULL || srcEnd - pos < subLen )
			return NULL;
		if ( wmemcmp(pos, sub, subLen) == 0 )
			return pos;
		src = pos;
	}
}
*/
