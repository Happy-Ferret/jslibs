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
#include <jslibsModule.h>
#include "jsstd.h"

#include <jsvalserializer.h>

DECLARE_CLASS( OperationLimit )


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
  expand('$(h) $(xxx) $(w)', { h:'Hello', w:'World' }); // returns "Hello  World"
  }}}
  $H example 2
  {{{
  expand('$(foo)-$(bar)', function(id) '<'+id+'>' ); // returns "<foo>-<bar>"
  }}}
**/
DEFINE_FUNCTION( expand ) {

	// look at jslang: ::join()
	typedef struct {
		const jschar *chars;
		size_t count;
		JSString *root;
	} Chunk;

	jl::Stack<Chunk, jl::StaticAllocMedium> stack;
	JLData srcStr;

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARGC_RANGE(1, 2);

	jsval value;

	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &srcStr) );

	JSObject *mapObj = NULL;
	jsval *mapFct = NULL;
	
	if ( JL_ARG_ISDEF(2) ) {
	
		if ( JL_ValueIsCallable(cx, JL_ARG(2)) )
			mapFct = &JL_ARG(2);
		else
		if ( !JSVAL_IS_PRIMITIVE(JL_ARG(2)) )
			mapObj = JSVAL_TO_OBJECT(JL_ARG(2));
	}

	ptrdiff_t total;

	const jschar *src, *srcEnd;
	src = srcStr.GetConstWStr();
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

				if ( value.isNullOrUndefined() )
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
DEFINE_FUNCTION( switchCase ) {

	JL_ASSERT_ARGC_MIN( 1 );

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
DEFINE_FUNCTION( switchCase ) {

	JL_ASSERT_ARGC_RANGE( 3, 4 );
	JL_ASSERT_ARG_IS_ARRAY(2);
	JL_ASSERT_ARG_IS_ARRAY(3);

	JSObject *caseArray;
	jsuint caseArrayLength;
	caseArray = JSVAL_TO_OBJECT(JL_ARG(2));
	JL_CHK( JS_GetArrayLength(cx, caseArray, &caseArrayLength) );

	jsuint i;
	for ( i = 0; i < caseArrayLength; ++i ) {
	
		JL_CHK( JL_GetElement(cx, caseArray, i, JL_RVAL) );
		
		JSBool same;
		JL_CHK( JS_SameValue(cx, JL_ARG(1), *JL_RVAL, &same) );
		if ( same )
			return JL_GetElement(cx, JSVAL_TO_OBJECT(JL_ARG(3)), i, JL_RVAL);
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
DEFINE_FUNCTION( internString ) {
	
	JL_IGNORE(argc);

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
  loadModule('jsstd');

  var obj = { a:1 };
  obj.b = 2;
  seal(obj);
  obj.c = 3; // Error: obj.c is read-only
}}}
**/
DEFINE_FUNCTION( deepFreezeObject ) {

	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_OBJECT(1);
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
DEFINE_FUNCTION( countProperties ) {

	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_OBJECT(1);

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
  loadModule('jsstd');

  var obj = { a:1, b:[2,3,4], c:{} };
  print( uneval(obj) ); // prints: ({a:1, b:[2, 3, 4], c:{}})

  clearObject(obj);
  print( uneval(obj) ); // prints: ({})
  }}}
**/
DEFINE_FUNCTION( clearObject ) {

	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_OBJECT(1);

	JSObject *argObj;
	argObj = JSVAL_TO_OBJECT(JL_ARG(1));

	JSIdArray *list;
	list = JS_Enumerate(cx, argObj); // JS_NewPropertyIterator, JS_NextProperty ?
	JL_CHK(list);

	for ( jsint i = 0; i < list->length; ++i )
		JL_CHK( JS_DeletePropertyById(cx, argObj, list->vector[i]) );
	JS_DestroyIdArray(cx, list);

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* beware:
     once an object is exposed to a script, the object's parent must not change. The JavaScript engine relies on this invariant.
	 JS_SetParent has no way to check that this is the case, but nonetheless, applications must not call JS_SetParent on an object that has already been exposed to a script.
	 If an application does this, the behavior is undefined.
	 (see MDN doc.)

/ **doc
$TOC_MEMBER $INAME
 $OBJ $INAME( obj, scopeObject )
  Set the scope object of _obj_. Use this function with care.
  $H example 1
  {{{
  function foo() {

   var data = 55;
   function bar() { print( data, '\n' ); }
   bar();
   var old = setScope( bar, {data:7} );
   bar();
   var old = setScope( bar, old );
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
  loadModule('jsstd');

  function bind(fun, obj) {

   setScope(obj, setScope(fun, obj));
  }


  function MyClass() {

   this.test = function() {

    foo();
   }

   this.foo = function() {

    print('foo\n');
   }

   bind(this.test, this);
  }

  var myObj = new MyClass();
  myObj.test(); // prints: foo
  var fct = myObj.test;
  fct(); // prints: foo
  }}}
** /
DEFINE_FUNCTION( setScope ) {

	JL_ASSERT_ARGC(2);
	JSObject *o, *p;
	JL_CHK( JS_ValueToObject(cx, JL_ARG(1), &o) ); // o = JSVAL_TO_OBJECT(JL_ARG(1));
	JL_CHK( JS_ValueToObject(cx, JL_ARG(2), &p) ); // p = JSVAL_TO_OBJECT(JL_ARG(2));
	*JL_RVAL = OBJECT_TO_JSVAL( JS_GetParent(o) );
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
  print( ObjectToId(myObj), '\n' );
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

			if ( it->obj && JS_IsAboutToBeFinalized(it->obj) ) {

				it->id = 0;
				it->obj = NULL;
			}
		}
	}
	return mpv->prevObjectIdGCCallback ? mpv->prevObjectIdGCCallback(cx, status) : JS_TRUE;
}


DEFINE_FUNCTION( objectToId ) {

	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_OBJECT(1);
	JSObject *obj;
	obj = JSVAL_TO_OBJECT( JL_ARG(1) );

	ModulePrivate *mpv = (ModulePrivate*)JL_GetModulePrivate(cx, _moduleId);

	ObjId *freeSlot;
	freeSlot = NULL;
	for ( ObjId *it = mpv->objIdList, *end = mpv->objIdList + mpv->objectIdAllocated; it < end; ++it ) {

		if ( it->obj == obj ) // safer alternative (but slower): JS_StrictlyEqual()
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
  print( IdToObject( objectToId(myObj) ) === myObj ); // prints true
  }}}
  $H example 2
  {{{
  var id = objectToId({});
  print( idToObject(id) ); // prints: [object Object]
  collectGarbage();
  print( idToObject(id) ); // prints: undefined
  }}}
**/
DEFINE_FUNCTION( idToObject ) {

	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_INTEGER_NUMBER(1);

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
 $VOID $INAME( text )
  Report the given _text_ as warning. The warning is reported on the stderr. Warnings are ignored in unsafeMode.
**/
DEFINE_FUNCTION( warning ) {

	JLData str;
	JL_ASSERT_ARGC(1);
//	const char *message;

	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &str) );
	JL_CHK( JS_ReportWarning(cx, "%s", str.GetConstStrZ()) );
	
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
  print( foo[i] );
  }}}
  $H note
   The error output can be redirected by redefining the _host.stderr function. see the print() function.
**/
DEFINE_FUNCTION( assert ) {

	if ( !JL_IS_SAFE )
		return JS_TRUE;

	JL_ASSERT_ARGC_RANGE(1,2);

	// see. js_DecompileValueGenerator  (http://infomonkey.cdleary.com/questions/144/how-to-get-the-script-text-code-at-runtime)

	bool assert;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &assert) );
	if ( !assert ) {

		JLData str;
		if ( JL_ARG_ISDEF(2) )
			JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &str) );
		else
			str = JLData("Assertion failed.", true);
		
		JS_ReportError( cx, "%s", str.GetConstStrZ());
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
DEFINE_FUNCTION( collectGarbage ) {

	JL_IGNORE(argc);

	size_t gcBytesDiff = JS_GetGCParameter(JL_GetRuntime(cx), JSGC_BYTES);
	JS_GC( cx );
	gcBytesDiff = JS_GetGCParameter(JL_GetRuntime(cx), JSGC_BYTES) - gcBytesDiff;
	return JL_NativeToJsval(cx, gcBytesDiff, JL_RVAL);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $INT $INAME()
  Performs a conditional garbage collection of JS objects, doubles, and strings that are no longer needed by a script executing.
  This offers the JavaScript engine an opportunity to perform garbage collection if needed.
**/
DEFINE_FUNCTION( maybeCollectGarbage ) {

	JL_IGNORE(argc);

	size_t gcBytesDiff = JS_GetGCParameter(JL_GetRuntime(cx), JSGC_BYTES);
	JS_MaybeGC( cx );
	gcBytesDiff = JS_GetGCParameter(JL_GetRuntime(cx), JSGC_BYTES) - gcBytesDiff;
	return JL_NativeToJsval(cx, gcBytesDiff, JL_RVAL);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $REAL $INAME( time )
  Suspends the execution of the current program during _time_ milliseconds.
**/
DEFINE_FUNCTION( sleep ) {

	JL_ASSERT_ARGC(1);
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
  loadModule('jsstd');
  loadModule('jsio');
  print( 't0: '+timeCounter(), '\n' );
  print( 't1: '+timeCounter(), '\n' );
  sleep(100);
  print( 't2: '+timeCounter(), '\n' );
  }}}
  prints:
  {{{
  t0: 0
  t1: 0.15615914588863317
  t2: 100.02473070050955
  }}}
**/
DEFINE_FUNCTION( timeCounter ) {

	JL_IGNORE(argc);

	return JL_NativeToJsval(cx, AccurateTimeCounter(), JL_RVAL);
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $STR $INAME( $STR str, $INT count )
  Returns the string that is _count_ times _str_.
  $H example
  {{{
  print( stringRepeat('foo', 3) ); // prints: foofoofoo
  }}}
**/
DEFINE_FUNCTION( stringRepeat ) {

	JLData str;

	JL_ASSERT_ARGC(2);

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
	JL_ASSERT_ALLOC( newBuf );
	newBuf[newLen] = 0;
	
	const jschar *buf;
	buf = str.GetConstWStr();

	jschar *tmp = newBuf;
	size_t i;
	for ( i = 0; i < count; ++i ) {

		memcpy(tmp, buf, len * sizeof(jschar));
		tmp += len;
	}

	JSString *jsstr;
	JL_updateMallocCounter(cx, sizeof(jschar) * newLen);
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
  print( 'Hello', ' ', 'World', '\n' ); // prints: Hello World
  }}}
  $H note
   The output can be redirected by redefining the _host.stdout function.
   $H example 1
   {{{
   loadModule('jsstd');
   print('Hello', 'World'); // prints: HelloWorld
   }}}
   $H example 2
   {{{
   loadModule('jsstd');
   print('foo\n'); // prints: foo
   _host.stdout = function() {}
   print('bar\n'); // prints nothing
   }}}
**/
DEFINE_FUNCTION( print ) {

	// print() => _host->stdout() => JSDefaultStdoutFunction() => pv->hostStdOut()
	jsval fval;
	JL_CHK( GetHostObjectValue(cx, JLID(cx, stdout), &fval) );
	*JL_RVAL = JSVAL_VOID;
	if (likely( JL_ValueIsCallable(cx, fval) ))
		return JS_CallFunctionValue(cx, JL_GetGlobal(cx), fval, JL_ARGC, JL_ARGV, &fval);
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
  var foo = exec('constants.js'); // loads constants.js or constants.jsxrd if available.
  }}}
**/
// function copied from mozilla/js/src/js.c
DEFINE_FUNCTION( exec ) {

	JLData str;
//	JSObject *scriptObjRoot;
	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARGC_RANGE(1, 2);

	bool useAndSaveCompiledScripts;
	useAndSaveCompiledScripts = !JL_ARG_ISDEF(2) || JL_ARG(2) == JSVAL_TRUE;
//	const char *filename;
//	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &filename) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &str) );

	uint32_t oldopts;
	oldopts = JS_SetOptions(cx, JS_GetOptions(cx) | JSOPTION_COMPILE_N_GO); // JSOPTION_COMPILE_N_GO is properly removed in JLLoadScript if needed.
	JSScript *script;
	script = JL_LoadScript(cx, obj, str, ENC_UNKNOWN, useAndSaveCompiledScripts, useAndSaveCompiledScripts);
	JS_SetOptions(cx, oldopts);
	JL_CHK( script );

//	scriptObjRoot = JS_NewScriptObject(cx, script);
	
	JSBool ok;
	ok = JS_ExecuteScript(cx, obj, script, JL_RVAL); // Doc: On successful completion, rval is a pointer to a variable that holds the value from the last executed expression statement processed in the script.
//	JS_DestroyScript(cx, script);
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
  function queryCallback(val) {

   return val;
  }
  var res = sandboxEval('1 + 2 + Query(3)', QueryCallback);
  print( res ); // prints: 6
  }}}
  $H example 2
  {{{
  var res = sandboxEval('Math');
  print( res == Math ); // prints: false
  }}}
  $H example 3
   abort very-long-running scripts.
  {{{
  try {

   var res = sandboxEval('while (true);', undefined, 1000);
  } catch (ex if ex instanceof OperationLimit) {

   print( 'script execution too long !' );
  }
  }}}
**/


BEGIN_CLASS( OperationLimit )

/*
DEFINE_CONSTRUCTOR() {

	JL_DEFINE_CONSTRUCTOR_OBJ;
	return JS_TRUE;
}
*/

/*
DEFINE_HAS_INSTANCE() {

	JL_IGNORE(obj);

	*bp = !JSVAL_IS_PRIMITIVE(*v) && JL_InheritFrom(cx, JSVAL_TO_OBJECT(*v), JL_THIS_CLASS);
	return JS_TRUE;
}
*/
CONFIGURE_CLASS

//	HAS_CONSTRUCTOR
	IS_INCONSTRUCTIBLE

//	HAS_HAS_INSTANCE // see issue#52

END_CLASS



// source: http://mxr.mozilla.org/mozilla/source/js/src/js.c
static JSBool
sandbox_resolve(JSContext *cx, JSObject *obj, jsid id, uintN flags, JSObject **objp) {

	JSBool resolved;
	if ( (flags & JSRESOLVE_ASSIGNING) == 0 ) {

		if ( !JS_ResolveStandardClass(cx, obj, id, &resolved) )
			return JS_FALSE;

		if ( !resolved && id == JLID(cx, Reflect) ) { // JSID_IS_ATOM(id, CLASS_ATOM(cx, Reflect))
			
			if ( !JS_InitReflect(cx, obj) )
				return JS_FALSE;
			resolved = JS_TRUE;
		}

		if ( !resolved && id == JLID(cx, Debugger) ) {

			if ( !JS_DefineDebuggerObject(cx, obj) ) // doc: https://developer.mozilla.org/en/SpiderMonkey/JS_Debugger_API_Guide
				return JS_FALSE;
			resolved = JS_TRUE;
		}

		if ( resolved ) {

			*objp = obj;
			return JS_TRUE;
		}
	}
	*objp = NULL;
	return JS_TRUE;
}


static JSClass sandbox_class = {
    "Sandbox",
    JSCLASS_NEW_RESOLVE | JSCLASS_GLOBAL_FLAGS,
    JS_PropertyStub,   JS_PropertyStub,
    JS_PropertyStub,   JS_StrictPropertyStub,
    JS_EnumerateStub, (JSResolveOp)sandbox_resolve,
    JS_ConvertStub,    NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};


struct SandboxContextPrivate {

	JLSemaphoreHandler semEnd;
	unsigned int maxExecutionTime;
	volatile bool expired;
	jsval queryFunctionValue;
	JSOperationCallback prevOperationCallback;
};


JSBool SandboxMaxOperationCallback(JSContext *cx) {

	SandboxContextPrivate *pv = (SandboxContextPrivate*)JS_GetContextPrivate(cx);
	if ( pv->expired && !JL_IsExceptionPending(cx) ) {

		JSOperationCallback tmp = JS_SetOperationCallback(cx, NULL);
		const ClassProtoCache *cpc = JL_GetCachedClassProto(JL_GetHostPrivate(cx), JL_CLASS_NAME(OperationLimit));
		ASSERT( cpc );
		JSCrossCompartmentCall *ccc;
		ccc = JS_EnterCrossCompartmentCall(cx, cpc->proto);
		JL_CHK( ccc );
		JSObject *branchLimitExceptionObj = JL_NewObjectWithGivenProto(cx, cpc->clasp, cpc->proto, NULL);
		JS_SetPendingException(cx, OBJECT_TO_JSVAL( branchLimitExceptionObj ));
		JS_LeaveCrossCompartmentCall(ccc);
		JL_CHK( branchLimitExceptionObj );
		JS_SetOperationCallback(cx, tmp);
		JL_BAD;
	}
	return pv->prevOperationCallback(cx);
}


JLThreadFuncDecl SandboxWatchDogThreadProc(void *threadArg) {

	JSContext *cx = (JSContext*)threadArg;
	SandboxContextPrivate *pv = (SandboxContextPrivate*)JS_GetContextPrivate(cx);
	int st = JLSemaphoreAcquire(pv->semEnd, pv->maxExecutionTime); // used as a breakable Sleep. This avoids to Cancel the thread
	if ( st == JLTIMEOUT ) {
		
		pv->expired = true;
		JS_TriggerOperationCallback(cx);
	}
	JLThreadExit(0);
	return 0;
}


JSBool SandboxQueryFunction(JSContext *cx, uintN argc, jsval *vp) {

	SandboxContextPrivate *pv = (SandboxContextPrivate*)JS_GetContextPrivate(cx);
	if ( JSVAL_IS_VOID( pv->queryFunctionValue ) ) {

		*JL_RVAL = JSVAL_VOID;
	} else {

		JSObject *obj = JS_THIS_OBJECT(cx, vp);
		ASSERT( obj != NULL );
		JL_CHK( JS_CallFunctionValue(cx, obj, pv->queryFunctionValue, JL_ARGC, JL_ARGV, JL_RVAL) );
		JL_CHKM( JSVAL_IS_PRIMITIVE(*JL_RVAL), E_RETURNVALUE, E_TYPE, E_TY_PRIMITIVE );
	}
	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION( sandboxEval ) {

	JL_ASSERT_ARGC_RANGE(1, 3);

	SandboxContextPrivate pv;

	JSString *jsstr;
	jsstr = JS_ValueToString(cx, JL_ARG(1));
	JL_CHK( jsstr );
	size_t srclen;
	const jschar *src;
	src = JS_GetStringCharsAndLength(cx, jsstr, &srclen);

	if ( JL_ARG_ISDEF(2) ) {

		JL_ASSERT_ARG_IS_CALLABLE(2);
		pv.queryFunctionValue = JL_ARG(2);
	} else {

		pv.queryFunctionValue = JSVAL_VOID;
	}

	if ( JL_ARG_ISDEF(3) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &pv.maxExecutionTime) );
	else
		pv.maxExecutionTime = 1000; // default value


	JSScript *script;
	unsigned lineno;
	JL_CHK( JS_DescribeTopFrame(cx, &script, &lineno) );
	const char *filename;
	filename = JS_GetScriptFilename(cx, script);

	pv.expired = false;
	pv.semEnd = JLSemaphoreCreate(0);
	JLThreadHandler sandboxWatchDogThread;
	sandboxWatchDogThread = JLThreadStart(SandboxWatchDogThreadProc, cx);
	if ( !JLThreadOk(sandboxWatchDogThread) )
		return JL_ThrowOSError(cx);

	void *prevCxPrivate = JS_GetContextPrivate(cx);
	JS_SetContextPrivate(cx, &pv);

	JSObject *globalObj;
	globalObj = JS_NewCompartmentAndGlobalObject(cx, &sandbox_class, NULL);

	JSCrossCompartmentCall *ccc;
	ccc = JS_EnterCrossCompartmentCall(cx, globalObj);
	JL_CHK( ccc );

	if ( pv.queryFunctionValue != JSVAL_VOID ) {

		JL_CHK( JS_WrapValue(cx, &pv.queryFunctionValue) );
		JL_CHK( JS_DefineFunction(cx, globalObj, "Query", SandboxQueryFunction, 1, JSPROP_PERMANENT | JSPROP_READONLY) );
	}

	//JS_SetNativeStackQuota(cx, 50000); // (TBD) seems to be optional ?
	// JS_SetScriptStackQuota(cx, JS_DEFAULT_SCRIPT_STACK_QUOTA / 16); ?

	pv.prevOperationCallback = JS_SetOperationCallback(cx, SandboxMaxOperationCallback);

	JSBool ok;
	ok = JS_EvaluateUCScript(cx, globalObj, src, srclen, filename, lineno, vp);

	JSOperationCallback tmp;
	tmp = JS_SetOperationCallback(cx, pv.prevOperationCallback);
	ASSERT( tmp == SandboxMaxOperationCallback );

	JS_LeaveCrossCompartmentCall(ccc);

	JLSemaphoreRelease(pv.semEnd);
	JLThreadWait(sandboxWatchDogThread, NULL);
	JLThreadFree(&sandboxWatchDogThread);
	JLSemaphoreFree(&pv.semEnd);

	JS_SetContextPrivate(cx, prevCxPrivate);

	// JL_CHK( JS_DeleteProperty(scx, globalObject, "Query") ); // (TBD) needed ? also check of the deletion is successful using JS_HasProperty...

	if ( ok )
		return JS_WrapValue(cx, vp);
	ASSERT( JL_IsExceptionPending(cx) ); // JL_REPORT_ERROR_NUM( JLSMSG_RUNTIME_ERROR, "exception is expected");
	JL_BAD;
}

/*

	SandboxContextPrivate pv;
	pv.cx = cx;

	if ( JL_ARG_ISDEF(2) ) {

		JL_ASSERT_ARG_IS_CALLABLE(2);
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
	if ( !JLThreadOk(sandboxWatchDogThread) )
		return JL_ThrowOSError(cx);

//	JSCrossCompartmentCall *call = JS_EnterCrossCompartmentCall(cx, globalObject);

	JSBool ok;
	ok = JS_EvaluateUCScript(scx, globalObject, src, (uintN)srclen, filename, lineno, JL_RVAL);

//	JS_LeaveCrossCompartmentCall(call);

	JLSemaphoreRelease(pv.semEnd);

	JLThreadWait(sandboxWatchDogThread, NULL);
	JLThreadFree(&sandboxWatchDogThread);
	JLSemaphoreFree(&pv.semEnd);

	prev = JS_SetOperationCallback(scx, prev);
	JL_ASSERT( prev == SandboxMaxOperationCallback, "Invalid SandboxMaxOperationCallback handler." );

	JL_CHK( JS_DeleteProperty(scx, globalObject, "Query") ); // (TBD) also check of the deletion is successful using JS_HasProperty...

	if (!ok) {

		jsval ex;
		if ( JS_GetPendingException(scx, &ex) )
			JS_SetPendingException(cx, ex);
		else
			JS_ReportError(cx, "Unexpected error.");
	}

	JS_DestroyContextNoGC(scx);
	return ok;
*/



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME( statementString )
  Returns $TRUE if _statementString_ is a valid Javascript statement.
  The intent is to support interactive compilation, accumulate lines in a buffer until IsStatementValid returns true, then pass it to an eval.
  This function is useful to write an interactive console.
  $H example
  {{{
  loadModule('jsstd');
  loadModule('jsio');

  global.__defineGetter__('quit', halt);

  for (;;) {

   print('js> ');

   var code = '';
   do {

    code += File.stdin.read();
   } while( !isStatementValid( code ) );

   try {

    var res = eval( code );
   } catch(ex) {

    print( ex, '\n' );
   }

   if ( res != undefined )
    print( res, '\n' );
  }
  }}}
**/
DEFINE_FUNCTION( isStatementValid ) {

	JLData str;
	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARGC(1);

	//const char *buffer;
	//size_t length;
	//JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(1), &buffer, &length) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &str) );
	JL_CHK( JL_NativeToJsval(cx, JS_BufferIsCompilableUnit(cx, JS_FALSE, obj, str.GetConstStr(), str.Length()) == JS_TRUE, JL_RVAL) );
	return JS_TRUE;
	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Stop the execution of the program. This is a ungraceful way to finish a program and should only be used in critical cases.
**/
DEFINE_FUNCTION( halt ) {

	JL_IGNORE(vp, argc);

	JL_ERR(E_STR("the program has been stopped"));
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
/**qa
	QA.ASSERT_EQ( 'typeof', currentFilename, 'string' );
	QA.ASSERT_EQ( '!=', currentFilename, '' );
**/
DEFINE_PROPERTY_GETTER( currentFilename ) {
	
	JL_IGNORE(id, obj);

	JSScript *script;
	JL_CHK( JS_DescribeTopFrame(cx, &script, NULL) );
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
/**qa
	QA.ASSERT_EQ( 'typeof', currentLineNumber, 'number' );
**/
DEFINE_PROPERTY_GETTER( currentLineNumber ) {
	
	JL_IGNORE(id, obj);

	unsigned lineno;
	JL_CHK( JS_DescribeTopFrame(cx, NULL, &lineno) );
	JL_CHK( JL_NativeToJsval(cx, lineno, vp) );
	return JS_TRUE;
	JL_BAD;
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

	JL_IGNORE(id, obj, strict);

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

	JL_IGNORE(id, obj);

	JSGCCallback cb = JS_SetGCCallback(cx, NULL);
	JS_SetGCCallback(cx, cb);
	*vp = BOOLEAN_TO_JSVAL( cb == VetoingGCCallback );
	return JS_TRUE;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $INT $INAME
**/
DEFINE_PROPERTY_GETTER( CPUID ) {

	JL_IGNORE(id, obj);

	if ( !JSVAL_IS_VOID(*vp) )
		return JS_TRUE;

	JLCpuInfo_t info;
	JLCpuInfo(info);
	return JL_NativeToJsval(cx, info, sizeof(info), vp);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

JSBool testProp(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	JL_IGNORE(vp, id, obj, cx);

	return JS_TRUE;
//	JL_BAD;
}



DEFINE_FUNCTION( jsstdTest ) {

	JL_IGNORE(vp, argc, cx);


/*
	JSObject *obj = JS_NewCompartmentAndGlobalObject(cx, &sandbox_class, NULL);
    JL_CHK( JS_WrapObject(cx, &obj) );
	*JL_RVAL = OBJECT_TO_JSVAL(obj);
*/


/*
	JSXDRState *xdr1 = JS_XDRNewMem(cx, JSXDR_ENCODE);
	JS_XDRValue(xdr1, &JL_ARG(1));
	uint32_t length;
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
	*JL_RVAL = STRING_TO_JSVAL( JL_NewUCString(cx, str, 1) );
*/	


/*
	JLData str;
	JL_JsvalToNative(cx, JL_ARG(1), &str);
	JSObject *scriptObj;

	//	scriptObj = JS_CompileFile(cx, JL_GetGlobal(cx), str.GetConstStrZ());

	//	scriptObj = JL_LoadScript(cx, JL_GetGlobal(cx), str, false, false);

	size_t scriptFileSize;
	int scriptFile;
	scriptFile = open(str.GetConstStrZ(), O_RDONLY | O_BINARY | O_SEQUENTIAL);

	scriptFileSize = lseek(scriptFile, 0, SEEK_END);
	lseek(scriptFile, 0, SEEK_SET); // see tell(scriptFile);
	char * scriptBuffer = (char*)jl_malloca(scriptFileSize);
	int res;
	res = read(scriptFile, (void*)scriptBuffer, (unsigned int)scriptFileSize);
	close(scriptFile);

	scriptObj = JS_CompileScript(cx, JL_GetGlobal(cx), scriptBuffer, scriptFileSize, str.GetConstStrZ(), 1);
*/



	return JS_TRUE;
	JL_BAD;
}

#endif // _DEBUG


DEFINE_INIT() {

	JL_IGNORE(proto, sc);

	INIT_CLASS( OperationLimit );
	return JS_TRUE;
	JL_BAD;
}


CONFIGURE_STATIC

	REVISION(JL_SvnRevToInt("$Revision$"))

	HAS_INIT

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION_ARGC( expand, 2 )
		FUNCTION_ARGC( switchCase, 4 )
		FUNCTION_ARGC( internString, 1 )
		FUNCTION_ARGC( deepFreezeObject, 1 )
		FUNCTION_ARGC( countProperties, 1 )
		FUNCTION_ARGC( clearObject, 1 )
		FUNCTION_ARGC( exec, 2 )
		FUNCTION_ARGC( sandboxEval, 3 )
		FUNCTION_ARGC( isStatementValid, 1 )
		FUNCTION_ARGC( stringRepeat, 2 )
		FUNCTION_ARGC( print, 1 ) // ...
		FUNCTION_ARGC( sleep, 1 )
		FUNCTION_ARGC( timeCounter, 0 )
		FUNCTION_ARGC( collectGarbage, 0 )
		FUNCTION_ARGC( maybeCollectGarbage, 0 )
		FUNCTION_ARGC( objectToId, 1 )
		FUNCTION_ARGC( idToObject, 1 )
		FUNCTION_ARGC( warning, 1 )
		FUNCTION_ARGC( assert, 2 )
		FUNCTION_ARGC( halt, 0 )
#ifdef _DEBUG
		FUNCTION( jsstdTest )
#endif // _DEBUG
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_GETTER( currentFilename )
		PROPERTY_GETTER( currentLineNumber )
		PROPERTY( disableGarbageCollection )
		PROPERTY_GETTER( CPUID )
#ifdef _DEBUG
//		PROPERTY( jsstdPropTest )
#endif // _DEBUG
	END_STATIC_PROPERTY_SPEC

END_STATIC



///////////////////////////////////////////////////////////////////////////////
// trash

/*
jschar wstrstr(const jschar *src, const jschar *srcEnd, const jschar *sub, const jschar *subEnd) {

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
