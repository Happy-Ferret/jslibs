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

#include <../host/host2.h>"

#ifdef XP_WIN
#include <Psapi.h>

	#pragma comment(lib,"Psapi.lib") // need for currentMemoryUsage()
	#include <Psapi.h>
	#pragma comment(lib,"pdh.lib") // need for performance counters usage
	#include <pdh.h>
	#include <PDHMsg.h>

#endif // XP_WIN


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

typedef struct {
	const jschar *chars;
	size_t count;
	JSString *root;
} ExpandChunk;




DEFINE_FUNCTION( expand ) {

	JL_DEFINE_ARGS;

	// look at jslang: ::join()

	jl::Stack<ExpandChunk, jl::StaticAllocMedium> stack;
	JLData srcStr;
	bool hasMapFct;
	JS::RootedValue value(cx);
	
	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARGC_RANGE(1, 2);

	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &srcStr) );

	if ( JL_ValueIsCallable(cx, JL_ARG(2)) ) {
			
		hasMapFct = true;
	} else {

		hasMapFct = false;
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

			key = js_strchr_limit(key, L('$'), srcEnd);
			if ( key == NULL || ++key == srcEnd ) {

				++stack;
				stack->chars = txt;
				stack->count = srcEnd - txt;
				total += stack->count;
				goto assemble;
			}
		} while ( *key != L('(') );

		if ( key ) {

			size_t txtLen = key - 1 - txt;
			if ( txtLen > 0 ) {
				
				++stack;
				stack->chars = txt;
				stack->count = txtLen;
				total += txtLen;
			}

			++key;
			keyEnd = js_strchr_limit(key, L(')'), srcEnd);
			
			if ( keyEnd ) {

				txt = keyEnd+1;
				

				if ( hasMapFct ) {

					JL_CHK( JL_NativeToJsval(cx, key, keyEnd - key, &value) );
					JL_CHK( JL_CallFunctionVA(cx, JL_OBJ, JL_ARG(2), &value, value) );
				} else if ( JL_ARG(2).isObject() ) {

					JS::RootedObject tmpObj(cx, &JL_ARG(2).toObject());
					JL_CHK( JS_GetUCProperty(cx, tmpObj, key, keyEnd - key, &value) );
				} else {
					
					continue;
				}

				if ( value.isNullOrUndefined() )
					continue;

				++stack;

				if ( !value.isString() ) { // 'convert to string' and 'root new string' if necessary.

					stack->root = JS::ToString(cx, value);
					JL_CHK( stack->root );
					JL_CHK( JS_AddStringRoot(cx, &stack->root) );
					stack->chars = JS_GetStringCharsAndLength(cx, stack->root, &stack->count);
			} else {

					stack->root = NULL;
					stack->chars = JS_GetStringCharsAndLength(cx, value.toString(), &stack->count);
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
		js_strncpy(tmp, stack->chars, stack->count);
		if ( stack->root != NULL )
			JS_RemoveStringRoot(cx, &stack->root);
	}

	JSString *jsstr;
	jsstr = JL_NewUCString(cx, res, total);
	JL_CHK( jsstr );
	args.rval().setString(jsstr);

	return true;

bad:
	while ( stack ) {

		if ( stack->root != NULL )
			JS_RemoveStringRoot(cx, &stack->root);
		--stack;
	}
	return false;
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
		
		JL_RVAL.setUndefined();
		return true;
	}

	unsigned int i;
	for ( i = 1; i < argc; i += 2 )
		
		if ( JS_SameValue(cx, JL_ARGV[0], JL_ARGV[i]) ) { // see also JS_StrictlyEqual
			
			*JL_RVAL = JL_ARGV[i+1];
			return true;
		}

	if ( i > argc ) {

		*JL_RVAL = JL_ARGV[argc-1];
		return true;
	}

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}
*/

/**doc
$TOC_MEMBER $INAME
 $VAL $INAME( value, caseArray, resultArray [, defaultResult] )
**/
DEFINE_FUNCTION( switchCase ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_RANGE( 3, 4 );
	JL_ASSERT_ARG_IS_ARRAY(2);
	JL_ASSERT_ARG_IS_ARRAY(3);

	{

	JS::RootedObject caseArray(cx, &JL_ARG(2).toObject());
	unsigned caseArrayLength;
	JL_CHK( JS_GetArrayLength(cx, caseArray, &caseArrayLength) );

	unsigned i;
	for ( i = 0; i < caseArrayLength; ++i ) {
	
		JL_CHK( JL_GetElement(cx, caseArray, i, JL_RVAL) );
		
		bool same;
		JL_CHK( JS_SameValue(cx, JL_ARG(1), JL_RVAL, &same) );
		if ( same ) {
			JS::RootedObject resultArrayObj(cx, &JL_ARG(3).toObject());
			return JL_GetElement(cx, resultArrayObj, i, JL_RVAL);
		}
	}

	JL_RVAL.set( argc >= 4 ? JL_ARG(4) : JSVAL_VOID);
	
	}

	return true;
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
	
	JL_DEFINE_ARGS;
	
	JL_ASSERT_ARGC(1);

	{

	JS::RootedString str(cx);
	if ( JL_ARG(1).isString() )
		str = JL_ARG(1).toString();
	else
		str = JS::ToString(cx, JL_ARG(1));
	JL_CHK( str );
	str = JS_InternJSString(cx, str);
	JL_CHK( str );
	JL_RVAL.setString(str);
	return true;
	}
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
	
	JL_DEFINE_ARGS;

	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_OBJECT(1);

	{

	JS::RootedObject objToFreeze(cx, &JL_ARG(1).toObject());
	//JL_CHK( JS_ValueToObject(cx, JL_ARG(1), &obj) );
	JL_RVAL.setUndefined();
	return JS_DeepFreezeObject(cx, objToFreeze);
	
	}

	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $INT $INAME( obj )
  Returns the number of own properties of an object.
**/
DEFINE_FUNCTION( countProperties ) {

	JL_DEFINE_ARGS;

	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_OBJECT(1);

	JSIdArray *arr;
	arr = JS_Enumerate(cx, &JL_ARG(1).toObject());
	JL_RVAL.setInt32(JS_IdArrayLength(cx, arr));
	JS_DestroyIdArray(cx, arr);

	return true;
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

	JL_DEFINE_ARGS;

	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_OBJECT(1);
	
	{
	
	JS::RootedObject argObj(cx, &JL_ARG(1).toObject());
	JS::RootedId id(cx);

	JSIdArray *list;
	list = JS_Enumerate(cx, argObj); // JS_NewPropertyIterator, JS_NextProperty ?
	JL_CHK(list);

    bool junk;
	for ( int i = 0; i < JS_IdArrayLength(cx, list); ++i ) {

		id.set(JS_IdArrayGet(cx, list, i));
		return JS_DeletePropertyById2(cx, argObj, id, &junk);
	}
	JS_DestroyIdArray(cx, list);

	JL_RVAL.setUndefined();
	
	}

	return true;
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
	return true;
	JL_BAD;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( text )
  Report the given _text_ as warning. The warning is reported on the stderr. Warnings are ignored in unsafeMode.
**/
DEFINE_FUNCTION( warning ) {

	JL_DEFINE_ARGS;

	JLData str;
	JL_ASSERT_ARGC(1);

//	const char *message;

	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &str) );
	JL_CHK( JS_ReportWarning(cx, "%s", str.GetConstStrZ()) );
	
	JL_RVAL.setUndefined();
	return true;
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
   The error output can be redirected by redefining the host.stderr function. see the print() function.
**/
DEFINE_FUNCTION( assert ) {

	if ( !JL_IS_SAFE )
		return true;

	JL_DEFINE_ARGS;

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
		return false;
	}

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $INT $INAME()
  Performs an unconditional garbage collection in the JS memory pool.
**/
DEFINE_FUNCTION( collectGarbage ) {

	JL_DEFINE_ARGS;

	JL_IGNORE(argc);

	size_t gcBytesDiff = JS_GetGCParameter(JL_GetRuntime(cx), JSGC_BYTES);
	JS_GC(JL_GetRuntime(cx));
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

	JL_DEFINE_ARGS;

	JL_IGNORE(argc);

	size_t gcBytesDiff = JS_GetGCParameter(JL_GetRuntime(cx), JSGC_BYTES);
	JS_MaybeGC( cx );
	gcBytesDiff = JS_GetGCParameter(JL_GetRuntime(cx), JSGC_BYTES) - gcBytesDiff;
	return JL_NativeToJsval(cx, gcBytesDiff, JL_RVAL);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $REAL $INAME( time [, accurate] )
  Suspends the execution of the current program during _time_ milliseconds. If _accurate_ is true, the precision of the pause is preferred to the performance of the function (CPU usage).
**/
DEFINE_FUNCTION( sleep ) {

	JL_DEFINE_ARGS;

	JL_ASSERT_ARGC_RANGE(1, 2);
	
	bool accurate;
	if ( JL_ARG_ISDEF(2) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &accurate) );
	else
		accurate = false;

	unsigned int time;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &time) );

	if ( accurate )
		jl::SleepMillisecondsAccurate(time);
	else
		jl::SleepMilliseconds(time);
	
	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $REAL $INAME()
  Returns the current value of a high-resolution time counter in millisecond.
  The returned value is a relative time value.
  $H beware
   This function is not reliable over a long period of time. For long period of time, use Date.now()
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

	JL_DEFINE_ARGS;

	JL_IGNORE(argc);
	return JL_NativeToJsval(cx, jl::AccurateTimeCounter(), JL_RVAL);
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

	JL_DEFINE_ARGS;

	JLData str;

	JL_ASSERT_ARGC(2);

	size_t count;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &count) );
	if ( count == 0 ) {

		JL_RVAL.set(JL_GetEmptyStringValue(cx));
		return true;
	}
	if ( count == 1 ) {

		JL_RVAL.setString( JS::ToString(cx, JL_ARG(1)) ); // force string conversion because we must return a string.
		return true;
	}

	size_t len;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &str) );
	len = str.Length();

	if ( len == 0 ) {

		JL_RVAL.set(JL_GetEmptyStringValue(cx));
		return true;
	}

	size_t newLen;
	newLen = len * count;

	jschar *newBuf;
	newBuf = static_cast<jschar*>(jl_malloc(sizeof(jschar) * (newLen +1)));
	JL_ASSERT_ALLOC( newBuf );
	newBuf[newLen] = 0;
	
	const jschar *buf;
	buf = str.GetConstWStr();

	jschar *tmp;
	tmp = newBuf;
	size_t i;
	for ( i = 0; i < count; ++i ) {

		jl::memcpy(tmp, buf, len * sizeof(jschar));
		tmp += len;
	}

	JSString *jsstr;
	JL_updateMallocCounter(cx, sizeof(jschar) * newLen);
	jsstr = JL_NewUCString(cx, newBuf, newLen);
	JL_CHK( jsstr );
	JL_RVAL.setString(jsstr);
	return true;
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
   The output can be redirected by redefining the host.stdout function.
   $H example 1
   {{{
   loadModule('jsstd');
   print('Hello', 'World'); // prints: HelloWorld
   }}}
   $H example 2
   {{{
   loadModule('jsstd');
   print('foo\n'); // prints: foo
   host.stdout = function() {}
   print('bar\n'); // prints nothing
   }}}
**/
DEFINE_FUNCTION( print ) {

	JL_DEFINE_ARGS;

	// print() => host->stdout() => JSDefaultStdoutFunction() => pv->hostStdOut()

	JS::RootedValue fval(cx);
	JL_CHK( JS_GetPropertyById(cx, jl::Host::getHost(cx).hostObject(), JLID(cx, stdout), &fval) );
	JL_RVAL.setUndefined();
	if (likely( JL_ValueIsCallable(cx, fval) )) {

		//return JS_CallFunctionValue(cx, JL_GetGlobal(cx), fval, JL_ARGC, JS_ARGV(cx,vp), fval.address());
		return JS_CallFunctionValue(cx, JS::NullPtr(), fval, args._jsargs, &fval);
	}
	return true;
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

	JL_DEFINE_ARGS;

	JLData fileName;
//	JSObject *scriptObjRoot;
	
	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARGC_RANGE(1, 2);

	bool useAndSaveCompiledScripts;
	useAndSaveCompiledScripts = !JL_ARG_ISDEF(2) || JL_ARG(2).isTrue();
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &fileName) );

	{

	JS::RootedScript script(cx, JL_LoadScript(cx, JL_OBJ, fileName, jl::ENC_UNKNOWN, useAndSaveCompiledScripts, useAndSaveCompiledScripts));
	JL_CHK( script );

	// doc: On successful completion, rval is a pointer to a variable that holds the value from the last executed expression statement processed in the script.
	JL_CHK( JS_ExecuteScript(cx, JL_OBJ, script, JL_RVAL.address()) );
	
	}

	return true;
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

/**qa
	QA.ASSERTOP( function() { new OperationLimit }, 'ex', TypeError, 'unconstructability' );
	QA.ASSERTOP( function() { (function f(){f();})(); }, 'ex', InternalError, 'too much recursion' );
	QA.ASSERTOP( function() { sandboxEval('for (var i=0; i<10000000000; ++i);', undefined, 250) }, 'ex', OperationLimit, 'OperationLimit detection' );
	QA.ASSERTOP( OperationLimit, '!typeof', 'undefined' );
	QA.ASSERTOP( OperationLimit.constructor, '===', StopIteration.constructor );
	QA.ASSERTOP( OperationLimit.prototype, '===', StopIteration.prototype );

// OperationLimit instance test
	try {
		sandboxEval('for (var i=0; i<10000000000; ++i);', undefined, 10);
	} catch(ex) {

		QA.ASSERTOP( ex, 'instanceof', OperationLimit, 'check OperationLimit instance' );
		QA.ASSERTOP( ex.constructor, '===', Object, 'check OperationLimit instance constructor' );
		QA.ASSERTOP( ex.__proto__, '===', OperationLimit, 'check OperationLimit instance prototype' );
	}

// not-a-constructor test
	try { new OperationLimit() } catch (ex1) {

		var message1 = ex1.message.replace('OperationLimit', 'XXX');
		try { new StopIteration() } catch (ex2) {

			var message2 = ex2.message.replace('StopIteration', 'XXX');
			QA.ASSERTOP( message1, '==', message2 );
		}
	}
**/

BEGIN_CLASS( OperationLimit )

DEFINE_HAS_INSTANCE() {

	JL_IGNORE(obj, cx);
//	*bp = !JSVAL_IS_PRIMITIVE(vp) && JL_GetClass(JSVAL_TO_OBJECT(vp)) == JL_THIS_CLASS;
	*bp = JL_ValueIsClass(cx, vp, JL_THIS_CLASS);
	return true;
}

CONFIGURE_CLASS

	HAS_HAS_INSTANCE

END_CLASS


// source: http://mxr.mozilla.org/mozilla/source/js/src/js.c
static bool
sandbox_resolve(JSContext *cx, JS::Handle<JSObject*> obj, JS::Handle<jsid> id, unsigned flags, JS::MutableHandleObject objp) {

	bool resolved;
	if ( (flags & JSRESOLVE_ASSIGNING) == 0 ) {

		if ( !JS_ResolveStandardClass(cx, obj, id, &resolved) )
			return false;

		if ( !resolved && id == JLID(cx, Reflect) ) { // JSID_IS_ATOM(id, CLASS_ATOM(cx, Reflect))
			
			if ( !JS_InitReflect(cx, obj) )
				return false;
			resolved = true;
		}

		if ( !resolved && id == JLID(cx, Debugger) ) {

			if ( !JS_DefineDebuggerObject(cx, obj) ) // doc: https://developer.mozilla.org/en/SpiderMonkey/JS_Debugger_API_Guide
				return false;
			resolved = true;
		}

		if ( resolved ) {

			objp.set(obj);
			return true;
		}
	}
	objp.set(NULL);
	return true;
}

static JSClass sandbox_class = {
    "Sandbox",
    JSCLASS_NEW_RESOLVE | JSCLASS_GLOBAL_FLAGS,
    JS_PropertyStub,   JS_DeletePropertyStub,
    JS_PropertyStub,   JS_StrictPropertyStub,
    JS_EnumerateStub, (JSResolveOp)sandbox_resolve,
    JS_ConvertStub,    NULL
};

class SandboxContextPrivate {
public:

	SandboxContextPrivate(JSContext *cx) : queryFunctionValue(cx) {
	}

	JLSemaphoreHandler semEnd;
	unsigned int maxExecutionTime;
	volatile bool expired;
	JS::PersistentRootedValue queryFunctionValue; //jsval queryFunctionValue;
	JSOperationCallback prevOperationCallback;
};

bool SandboxMaxOperationCallback(JSContext *cx) {

	SandboxContextPrivate *pv = (SandboxContextPrivate*)JS_GetContextPrivate(cx);
	if ( pv->expired && !JL_IsExceptionPending(cx) ) {

		JS::RootedValue branchLimitExceptionVal(cx);
		JSOperationCallback tmp = JS_SetOperationCallback(JL_GetRuntime(cx), NULL);

		const jl::ProtoCache::Item* cpc = jl::Host::getHost(cx).getCachedClassProto(JL_CLASS_NAME(OperationLimit));
		ASSERT( cpc );

		JSCompartment *oldCompartment;

		oldCompartment = JS_EnterCompartment(cx, cpc->proto);
		JL_CHK( oldCompartment );
		JSObject *branchLimitExceptionObj;
		branchLimitExceptionObj = JL_NewObjectWithGivenProto(cx, cpc->clasp, cpc->proto);
		
		branchLimitExceptionVal.setObject(*branchLimitExceptionObj);
		JS_SetPendingException(cx, branchLimitExceptionVal);
		JS_LeaveCompartment(cx, oldCompartment);
		JL_CHK( branchLimitExceptionObj );
		JS_SetOperationCallback(JL_GetRuntime(cx), tmp);
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
		JS_TriggerOperationCallback(JL_GetRuntime(cx));
	}
	JLThreadExit(0);
	return 0;
}

bool SandboxQueryFunction(JSContext *cx, unsigned argc, jsval *vp) {

	JL_DEFINE_ARGS;

	SandboxContextPrivate *pv = (SandboxContextPrivate*)JS_GetContextPrivate(cx);
	if ( pv->queryFunctionValue.get().isUndefined() ) {

		JL_RVAL.setUndefined();
	} else {

		JL_CHK( JS_CallFunctionValue(cx, args.thisObj(), pv->queryFunctionValue, args._jsargs, JL_RVAL) );
		JL_CHKM( JL_RVAL.isPrimitive(), E_RETURNVALUE, E_TYPE, E_TY_PRIMITIVE );
	}
	return true;
	JL_BAD;
}

DEFINE_FUNCTION( sandboxEval ) {

	JL_DEFINE_ARGS;

	SandboxContextPrivate pv(cx);

	JL_ASSERT_ARGC_RANGE(1, 3);

	JSString *jsstr;
	jsstr = JS::ToString(cx, JL_ARG(1));
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


	const char *filename;
	unsigned lineno;
	
	{
	JS::AutoFilename autoFilename;
	JL_CHK( JS::DescribeScriptedCaller(cx, &autoFilename, &lineno) );
	filename = autoFilename.get();
	}

	pv.expired = false;
	pv.semEnd = JLSemaphoreCreate(0);
	JLThreadHandler sandboxWatchDogThread;
	sandboxWatchDogThread = JLThreadStart(SandboxWatchDogThreadProc, cx);
	if ( !JLThreadOk(sandboxWatchDogThread) )
		return JL_ThrowOSError(cx);

	void *prevCxPrivate;
	prevCxPrivate = JS_GetContextPrivate(cx);
	JS_SetContextPrivate(cx, &pv);

	{

	JS::RootedObject globalObj(cx, JS_NewGlobalObject(cx, &sandbox_class, NULL, JS::DontFireOnNewGlobalHook));
	JSAutoCompartment ac(cx, globalObj);

	if ( pv.queryFunctionValue != JSVAL_VOID ) {

		JL_CHK( JS_WrapValue(cx, &pv.queryFunctionValue) );
		JL_CHK( JS_DefineFunction(cx, globalObj, "query", SandboxQueryFunction, 1, JSPROP_PERMANENT | JSPROP_READONLY) );
	}

	JS_SetNativeStackQuota(JL_GetRuntime(cx), 32000);

	pv.prevOperationCallback = JS_SetOperationCallback(JL_GetRuntime(cx), SandboxMaxOperationCallback);

	JS_FireOnNewGlobalObject(cx, globalObj);



	bool ok;
	ok = JS_EvaluateUCScript(cx, globalObj, src, srclen, filename, lineno, JL_RVAL);

	JSOperationCallback tmp;
	tmp = JS_SetOperationCallback(JL_GetRuntime(cx), pv.prevOperationCallback);
	ASSERT( tmp == SandboxMaxOperationCallback );

	JS_SetNativeStackQuota(JL_GetRuntime(cx), 0); // (TBD) any way to restore the previous value ?

	JLSemaphoreRelease(pv.semEnd);
	JLThreadWait(sandboxWatchDogThread);
	JLThreadFree(&sandboxWatchDogThread);
	JLSemaphoreFree(&pv.semEnd);

	JS_SetContextPrivate(cx, prevCxPrivate);

	// JL_CHK( JS_DeleteProperty(scx, globalObject, "query") ); // (TBD) needed ? also check of the deletion is successful using JS_HasProperty...

	if ( ok )
		return JS_WrapValue(cx, JL_RVAL);

	ASSERT( JL_IsExceptionPending(cx) ); // JL_REPORT_ERROR_NUM( JLSMSG_RUNTIME_ERROR, "exception is expected");

	}

	JL_BAD;
}

/*

	SandboxContextPrivate pv;
	pv.cx = cx;

	if ( JL_ARG_ISDEF(2) ) {

		JL_ASSERT_ARG_IS_CALLABLE(2);
		pv.queryFunctionValue = JL_ARG(2);
		JL_CHK( JS_DefineFunction(scx, globalObject, "query", SandboxQueryFunction, 1, JSPROP_PERMANENT | JSPROP_READONLY) );
	} else {

		pv.queryFunctionValue = JSVAL_VOID;
	}

	if ( JL_ARG_ISDEF(3) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &pv.maxExecutionTime) );
	else
		pv.maxExecutionTime = 1000; // default value

	JSString *jsstr;
	jsstr = JS::ToString(cx, JL_ARG(1));
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
	JS::RootedScript script;
	script = JS_GetFrameScript(cx, fp);
	JL_CHK( script );
	const char *filename;
	filename = JS_GetScriptFilename(cx, script);
	jsbytecode *pc;
	pc = JS_GetFramePC(cx, fp);
	unsigned lineno;
	lineno = JS_PCToLineNumber(cx, script, pc);

	JS_SetContextPrivate(scx, &pv);

	pv.semEnd = JLSemaphoreCreate(0);

	JLThreadHandler sandboxWatchDogThread;
	sandboxWatchDogThread = JLThreadStart(SandboxWatchDogThreadProc, scx);
	if ( !JLThreadOk(sandboxWatchDogThread) )
		return JL_ThrowOSError(cx);

//	JSCrossCompartmentCall *call = JS_EnterCrossCompartmentCall(cx, globalObject);

	bool ok;
	ok = JS_EvaluateUCScript(scx, globalObject, src, (unsigned)srclen, filename, lineno, JL_RVAL);

//	JS_LeaveCrossCompartmentCall(call);

	JLSemaphoreRelease(pv.semEnd);

	JLThreadWait(sandboxWatchDogThread);
	JLThreadFree(&sandboxWatchDogThread);
	JLSemaphoreFree(&pv.semEnd);

	prev = JS_SetOperationCallback(scx, prev);
	JL_ASSERT( prev == SandboxMaxOperationCallback, "Invalid SandboxMaxOperationCallback handler." );

	JL_CHK( JS_DeleteProperty(scx, globalObject, "query") ); // (TBD) also check of the deletion is successful using JS_HasProperty...

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

	JL_DEFINE_ARGS;

	JLData str;
	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARGC(1);

	//const char *buffer;
	//size_t length;
	//JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(1), &buffer, &length) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &str) );
	JL_CHK( JL_NativeToJsval(cx, JS_BufferIsCompilableUnit(cx, JL_OBJ, str.GetConstStr(), str.Length()), JL_RVAL) );
	return true;
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
	return false;
}


#ifdef XP_UNIX
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

struct LinuxProcInfo {
	int pid; // %d
	char comm[400]; // %s
	char state; // %c
	int ppid; // %d
	int pgrp; // %d
	int session; // %d
	int tty; // %d
	int tpgid; // %d
	unsigned int flags; // %u
	unsigned int minflt; // %u
	unsigned int cminflt; // %u
	unsigned int majflt; // %u
	unsigned int cmajflt; // %u
	int utime; // %d
	int stime; // %d
	int cutime; // %d
	int cstime; // %d
	int counter; // %d
	int priority; // %d
	unsigned int timeout; // %u
	unsigned int itrealvalue; // %u
	int starttime; // %d
	unsigned int vsize; // %u
	unsigned int rss; // %u
	unsigned int rlim; // %u
	unsigned int startcode; // %u
	unsigned int endcode; // %u
	unsigned int startstack; // %u
	unsigned int kstkesp; // %u
	unsigned int kstkeip; // %u
	int signal; // %d
	int blocked; // %d
	int sigignore; // %d
	int sigcatch; // %d
	unsigned int wchan; // %u
};

bool GetProcInfo( pid_t pid, LinuxProcInfo *pinfo ) {

	char path[128];
	char data[512];
	snprintf(path, sizeof(path), "/proc/%d/stat", pid);
	int fd = open(path, O_RDONLY);
	//lseek(fd,0,SEEK_SET);
	int rd = read(fd, data, sizeof(data));
	close(fd);
	data[rd] = '\0';

	sscanf(data, "%d %s %c %d %d %d %d %d %u %u %u %u %u %d %d %d %d %d %d %u %u %d %u %u %u %u %u %u %u %u %d %d %d %d %u",
		&pinfo->pid, // %d
		pinfo->comm, // %s
		&pinfo->state, // %c
		&pinfo->ppid, // %d
		&pinfo->pgrp, // %d
		&pinfo->session, // %d
		&pinfo->tty, // %d
		&pinfo->tpgid, // %d
		&pinfo->flags, // %u
		&pinfo->minflt, // %u
		&pinfo->cminflt, // %u
		&pinfo->majflt, // %u
		&pinfo->cmajflt, // %u
		&pinfo->utime, // %d
		&pinfo->stime, // %d
		&pinfo->cutime, // %d
		&pinfo->cstime, // %d
		&pinfo->counter, // %d
		&pinfo->priority, // %d
		&pinfo->timeout, // %u
		&pinfo->itrealvalue, // %u
		&pinfo->starttime, // %d
		&pinfo->vsize, // %u
		&pinfo->rss, // %u
		&pinfo->rlim, // %u
		&pinfo->startcode, // %u
		&pinfo->endcode, // %u
		&pinfo->startstack, // %u
		&pinfo->kstkesp, // %u
		&pinfo->kstkeip, // %u
		&pinfo->signal, // %d
		&pinfo->blocked, // %d
		&pinfo->sigignore, // %d
		&pinfo->sigcatch, // %d
		&pinfo->wchan // %u
	);
	return true;
}

#endif // XP_UNIX


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $INAME
  TBD
**/
DEFINE_PROPERTY_GETTER( currentMemoryUsage ) {

	JL_IGNORE( id, obj );

	uint32_t bytes;

#if defined(XP_WIN)

	PROCESS_MEMORY_COUNTERS_EX pmc;
	BOOL status = ::GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
	if ( !status )
		return JL_ThrowOSError(cx);
	bytes = pmc.WorkingSetSize; // same value as "windows task manager" "mem usage"

#elif defined(XP_UNIX)

	// VmRSS in /proc/self/status

	LinuxProcInfo pinfo;
	GetProcInfo(getpid(), &pinfo);
	bytes = pinfo.vsize;

#else

	JL_WARN( E_API, E_NOTIMPLEMENTED );

#endif

	JL_CHK( JL_NewNumberValue(cx, bytes, vp) );
	return true;
	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $INAME
  TBD
**/
DEFINE_PROPERTY_GETTER( peakMemoryUsage ) {

	JL_IGNORE( id, obj );

#if defined(XP_WIN)

/*
	DWORD  dwMin, dwMax;
	HANDLE hProcess;
	hProcess = GetCurrentProcess();
	if (!GetProcessWorkingSetSize(hProcess, &dwMin, &dwMax))
		JL_REPORT_ERROR("GetProcessWorkingSetSize failed (%d)\n", GetLastError());
//	printf("Minimum working set: %lu Kbytes\n", dwMin/1024);
//	printf("Maximum working set: %lu Kbytes\n", dwMax/1024);
	bytes = dwMax;
	//cf. GetProcessWorkingSetSizeEx
*/

	PROCESS_MEMORY_COUNTERS_EX pmc;
	BOOL status = ::GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
	if ( !status )
		return JL_ThrowOSError(cx);

	JL_CHK( JL_NewNumberValue(cx, pmc.PeakWorkingSetSize, vp) ); // same value as "windows task manager" "peak mem usage"
	return true;
#else

	// see:
	// 1/ max rss field in  /proc/self/status
	// 2/ getrusage

	JL_WARN( E_API, E_NOTIMPLEMENTED );

#endif

	vp.setUndefined();
	return true;
	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $INAME
  TBD
**/

int Compare( const void * Val1, const void * Val2 )
{
    if ( *(PDWORD)Val1 == *(PDWORD)Val2 )
    return 0;

    return *(PDWORD)Val1 > *(PDWORD)Val2 ? 1 : -1;
}

DEFINE_PROPERTY_GETTER( privateMemoryUsage ) {

	JL_IGNORE( id, obj );

#if defined(XP_WIN)

/*
	// see also. http://www.codeproject.com/KB/cpp/XPWSPrivate.aspx (Calculate Memory (Working Set - Private) Programmatically in Windows XP/2000)

	PROCESS_MEMORY_COUNTERS_EX pmc;
	BOOL status = ::GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
	if ( !status )
		return JL_ThrowOSError(cx);
	// doc:
	//   pmc.PrivateUsage
	//     The current amount of memory that cannot be shared with other processes, in bytes. Private bytes include memory that is committed and marked MEM_PRIVATE,
	//     data that is not mapped, and executable pages that have been written to.
	//   pmc.WorkingSetSize
	//     same value as "windows task manager" "mem usage"
	return JL_NewNumberValue(cx, pmc.PrivateUsage, vp);
*/

/* source: http://www.codeproject.com/Articles/87529/Calculate-Memory-Working-Set-Private-Programmatica
*/
	DWORD dWorkingSetPages[1024 * 128]; // hold the working set information get from QueryWorkingSet()
	DWORD dPageSize = 0x1000;

	DWORD dSharedPages = 0;
	DWORD dPrivatePages = 0; 
	DWORD dPageTablePages = 0;

	HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId() );
	if ( !hProcess )
		goto bad;

	__try {

		if ( !QueryWorkingSet(hProcess, dWorkingSetPages, sizeof(dWorkingSetPages)) )
			__leave;

		DWORD dPages = dWorkingSetPages[0];
		qsort(&dWorkingSetPages[1], dPages, sizeof(DWORD), Compare);

		for ( DWORD i = 1; i <= dPages; i++ ) {

			DWORD dCurrentPageStatus = 0; 
			DWORD dCurrentPageAddress;
			DWORD dNextPageAddress;
			DWORD dNextPageFlags;
			DWORD dPageAddress = dWorkingSetPages[i] & 0xFFFFF000;
			DWORD dPageFlags = dWorkingSetPages[i] & 0x00000FFF;

			while ( i <= dPages ) { // iterate all pages

				dCurrentPageStatus++;

				if ( i == dPages ) // if last page
					break;

				dCurrentPageAddress = dWorkingSetPages[i] & 0xFFFFF000;
				dNextPageAddress = dWorkingSetPages[i+1] & 0xFFFFF000;
				dNextPageFlags = dWorkingSetPages[i+1] & 0x00000FFF;

				// decide whether iterate further or exit (this is non-contiguous page or have different flags) 
				if ( (dNextPageAddress == (dCurrentPageAddress + dPageSize)) && (dNextPageFlags == dPageFlags) )
					++i;
				else
					break;
			}

			if ( (dPageAddress < 0xC0000000) || (dPageAddress > 0xE0000000) ) {
	            
				if ( dPageFlags & 0x100 ) // this is shared one
					dSharedPages += dCurrentPageStatus;
				else // private one
					dPrivatePages += dCurrentPageStatus;
			} else {
	        
				dPageTablePages += dCurrentPageStatus; // page table region
			}
		} 
		return JL_NewNumberValue(cx, (dPages - dSharedPages) * dPageSize, vp);
	} __finally {

		CloseHandle( hProcess );
	}

#else


	// get VmSize in /proc/self/status

	JL_WARN( E_API, E_NOTIMPLEMENTED );

	*vp = JSVAL_VOID;
	return true;

#endif
	JL_BAD;
}




/**doc
=== Static properties ===
**/


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME $READONLY
  Is the filename of the script being executed.
  $H note
   The current filename is also available using: `StackFrameInfo(stackSize-1).filename` (see jsdebug module)
**/
/**qa
	QA.ASSERTOP( currentFilename, 'typeof', 'string' );
	QA.ASSERTOP( currentFilename, '!=', '' );
**/
DEFINE_PROPERTY_GETTER( currentFilename ) {
	
	JL_IGNORE(id, obj);

	const char *filename;
	JL_CHK( JL_GetCurrentLocation(cx, &filename, NULL) );
	JL_CHK( JL_NativeToJsval(cx, filename, vp) );
	return true;
	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME $READONLY
  Is the line number of the script being executed.
**/
/**qa
	QA.ASSERTOP( currentLineNumber, 'typeof', 'number' );
**/
DEFINE_PROPERTY_GETTER( currentLineNumber ) {
	
	JL_IGNORE(id, obj);

	unsigned lineno;
	JL_CHK( JL_GetCurrentLocation(cx, NULL, &lineno) );
	JL_CHK( JL_NativeToJsval(cx, lineno, vp) );
	return true;
	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Is the amount of CPU time (milliseconds) that the process has executed.
**/
DEFINE_PROPERTY_GETTER( processTime ) {

	JL_IGNORE( id, obj );

#if defined(XP_WIN)

	FILETIME creationTime, exitTime, kernelTime, userTime;
	BOOL status = ::GetProcessTimes(GetCurrentProcess(), &creationTime, &exitTime, &kernelTime, &userTime);
	if ( !status )
		return JL_ThrowOSError(cx);

	ULARGE_INTEGER user;
    ULARGE_INTEGER kernel;
    kernel.LowPart  = kernelTime.dwLowDateTime;
    kernel.HighPart = kernelTime.dwHighDateTime;
    user.LowPart  = userTime.dwLowDateTime;
    user.HighPart = userTime.dwHighDateTime;
	return JL_NativeToJsval(cx, (kernel.QuadPart + user.QuadPart) / (double)10000, vp);

#else

	// see "CPU currently used by current process:" in http://stackoverflow.com/questions/63166/how-to-determine-cpu-and-memory-consumption-from-inside-a-process
	// /proc/cpuinfo
	// times()

	JL_WARN( E_API, E_NOTIMPLEMENTED );

#endif

	vp.setUndefined();
	return true;
	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Is the current CPU usage in percent.
**/
DEFINE_PROPERTY_GETTER( cpuLoad ) {

	JL_IGNORE( id, obj );

#if defined(XP_WIN)

  static PDH_STATUS status;
  static PDH_FMT_COUNTERVALUE value;
  static HQUERY query;
  static HCOUNTER counter;
  static DWORD ret;
  static bool notInit = true;

	if ( notInit ) {

		status = PdhOpenQuery(NULL, NULL, &query);
		if ( status != ERROR_SUCCESS )
			return JL_ThrowOSErrorCode(cx, status, "pdh.dll");
		status = PdhAddCounter(query, TEXT("\\Processor(_Total)\\% Processor Time"), NULL, &counter); // A total of ALL CPU's in the system
		if ( status != ERROR_SUCCESS )
			return JL_ThrowOSErrorCode(cx, status, "pdh.dll");
		status = PdhCollectQueryData(query); // No error checking here
		if ( status != ERROR_SUCCESS )
			return JL_ThrowOSErrorCode(cx, status, "pdh.dll");
		notInit = false;
	}

	status = PdhCollectQueryData(query);
	if ( status != ERROR_SUCCESS )
		return JL_ThrowOSErrorCode(cx, status, "pdh.dll");

	status = PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, &ret, &value);
	if ( status != ERROR_SUCCESS && status != PDH_CALC_NEGATIVE_DENOMINATOR )
		return JL_ThrowOSErrorCode(cx, status, "pdh.dll");

	return JL_NativeToJsval(cx, value.doubleValue, vp);

#else

	// see:
	// 1/ in http://stackoverflow.com/questions/63166/how-to-determine-cpu-and-memory-consumption-from-inside-a-process
	// 2/ sysconf(_SC_NPROCESSORS_ONLN)


	JL_WARN( E_API, E_NOTIMPLEMENTED );

#endif

	vp.setUndefined();
	return true;
	JL_BAD;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**doc
$TOC_MEMBER $INAME
 $INT $INAME
**/
DEFINE_PROPERTY_GETTER( cpuId ) {

	JL_IGNORE(id, obj);

	if ( !vp.isUndefined() )
		return true;

	jl::CpuInfo_t info;
	jl::CPUInfo(info);
	return JL_NativeToJsval(cx, info, sizeof(info), vp);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

bool testProp(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	JL_IGNORE(vp, id, obj, cx);

	return true;
//	JL_BAD;
}


DEFINE_FUNCTION( jsstdTest ) {

	JL_IGNORE(vp, argc, cx);


	void *test = &ModuleInit;


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
	double t = jl::AccurateTimeCounter();
	for ( int i = 0; i < 100000; ++i )
		JL_Push(cx, arr, &val); 
	t = jl::AccurateTimeCounter() - t;
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



	return true;
	JL_BAD;
}

#endif // _DEBUG


DEFINE_INIT() {

	JL_IGNORE(proto, cs);

	INIT_CLASS( OperationLimit );
	return true;
	JL_BAD;
}


CONFIGURE_STATIC

	REVISION(jl::SvnRevToInt("$Revision$"))

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
		PROPERTY_GETTER( currentMemoryUsage )
		PROPERTY_GETTER( peakMemoryUsage )
		PROPERTY_GETTER( privateMemoryUsage )
		PROPERTY_GETTER( processTime )
		PROPERTY_GETTER( cpuLoad )
		PROPERTY_GETTER( cpuId )

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
