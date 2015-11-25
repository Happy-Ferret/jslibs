<b>If something seems wrong or incomplete, please enter <a href='#commentform.md'>a comment at the bottom of this page</a>.</b><br /><br />- [source](http://jslibs.googlecode.com/svn/trunk/./src/jsstd/) - [main](JSLibs.md) - [QA](http://jslibs.googlecode.com/svn/trunk/./src/jsstd/qa.js) -
# jsstd module #





---

## jsstd static members ##
- [top](#jsstd_module.md) -
[de static.cpp?r=2420 revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsstd/Copie) -

### Static functions ###

#### <font color='white' size='1'><b>Expand</b></font> ####
> <sub>string</sub> <b>Expand</b>( str [[.md](.md), obj] )
> > Return an expanded string using key/value stored in _obj_.
> > <br />
> > If _obj_ is omitted, the current object is used to look for key/value.
> > ##### example: #####
```
  function Test() {
   this.Expand = Expand;
   this.a = 123;
  }
  Print( new Test().Expand('$(a)') );
```
> > ##### note: #####
> > > undefined values are ignored in the resulting string.

> > ##### example: #####
```
  Expand(' $(h) $(w)', { h:'Hello', w:'World' }); // returns "Hello World"
```

#### <font color='white' size='1'><b>InternString</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>InternString</b>( string )
> > Make an interned string, a string that is automatically shared with other code that needs a string with the same value.

#### <font color='white' size='1'><b>Seal</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Seal</b>( obj [[.md](.md) , recursively  ] )
> > Prevents all write access to the object, either to add a new property, delete an existing property, or set the value or attributes of an existing property.
> > If _recursively_ is true, the function seal any non-null objects in the graph connected to obj's slots.

#### <font color='white' size='1'><b>Clear</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Clear</b>( obj )
> > Removes all properties and elements from _obj_ in a single operation.

#### <font color='white' size='1'><b>SetScope</b></font> ####

> <sub>Object</sub> <b>SetScope</b>( obj, scopeObject )
> > Set the scope object of _obj_.
> > ##### example 1: #####
```
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
```
> > prints:
> > > `55`
> > > `7`
> > > `55`


> ##### example 2: #####
```
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
```

#### <font color='white' size='1'><b>SetPropertyEnumerate</b></font> ####
> <font color='gray' size='1'><sub>void</sub></font> <b>SetPropertyEnumerate</b>( object, propertyName, polarity )
> > Show/Hide a property to for-in loop.
> > ##### note: #####
> > > Using this function may change the order of the properties within the object.

> > ##### example: #####
```
  var obj = { a:1, b:2, c:3 };
  for ( var p in obj )
   Print(p, ', '); // prints: a, b, c

  SetPropertyEnumerate(obj, 'b', false);
  for ( var p in obj )
   Print(p, ', '); // prints: a, c

  SetPropertyEnumerate(obj, 'b', true);
  for ( var p in obj )
   Print(p, ', '); // prints: a, c, b
```

#### <font color='white' size='1'><b>SetPropertyReadonly</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>SetPropertyReadonly</b>( object, propertyName, polarity )
> > Make a property not settable. Any attempt to modify a read-only property fail silently.
> > ##### example: #####
```
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
```

#### <font color='white' size='1'><b>ObjectToId</b></font> ####

> <sub>integer</sub> <b>ObjectToId</b>( object )
> > Returns an integer value that is a unique identifier of the object _object_ .
> > ##### example: #####
```
  var myObj = {};
  Print( ObjectToId(myObj), '\n' );
```

#### <font color='white' size='1'><b>IdToObject</b></font> ####

> <sub>integer</sub> <b>IdToObject</b>( id )
> > Returns the object with the identifier _id_ or undefined if the identifier do not exist or the object has been GCed.
> > ##### example 1: #####
```
  var myObj = {};
  Print( IdToObject(ObjectToId(myObj)) === myObj, '\n' ); // prints true
```
> > ##### example 2: #####
```
  var id = ObjectToId({});
  Print( IdToObject(id), '\n' ); // prints: [object Object]
  CollectGarbage();
  Print( IdToObject(id), '\n' ); // prints: undefined
```

#### <font color='white' size='1'><b>IsBoolean</b></font> ####

> <sub>boolean</sub> <b>IsBoolean</b>()
> > Returns _true_ if the value is a boolean.

#### <font color='white' size='1'><b>IsPrimitive</b></font> ####

> <sub>boolean</sub> <b>IsPrimitive</b>()
> > Returns _true_ if the value is a primitive ( null or not an object ).

#### <font color='white' size='1'><b>IsFunction</b></font> ####

> <sub>boolean</sub> <b>IsFunction</b>()
> > Returns _true_ if the value is a function.

#### <font color='white' size='1'><b>IsVoid</b></font> ####

> <sub>boolean</sub> <b>IsVoid</b>()
> > Returns _true_ if the value is undefined (ie. void 0).
> > ##### example: #####
```
  Print( IsVoid(undefined) ); // prints: true
```

#### <font color='white' size='1'><b>XdrEncode</b></font> ####

> <sub>Blob</sub> <b>XdrEncode</b>( value )
> > Encode (serialize) a JavaScript value into an XDR (eXternal Data Representation) blob.
> > ##### note: #####
> > > All JavaScript values cannot be encoded into XDR. If the function failed to encode a value, an error is raised. The Map object can help you to encode Object and Array.

#### <font color='white' size='1'><b>XdrDecode</b></font> ####

> <sub>value</sub> <b>XdrDecode</b>( xdrBlob )
> > Decode (deserialize) XDR (eXternal Data Representation) blob to a JavaScript value.
> > ##### <font color='red'>beware</font>: #####
> > > Decoding malformed XDR data can lead the program to crash. This may be a security issue.

#### <font color='white' size='1'><b>Warning</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Warning</b>( text )
> > Report the given _text_ as warning. The warning is reported on the stderr. Warnings ignored in unsafeMode.

#### <font color='white' size='1'><b>Assert</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Assert</b>( expression [[.md](.md), failureMessage ] )
> > If the argument expression compares equal to zero, the failureMessage is written to the standard error device and the program stops its execution.
> > ##### example: #####
```
  var foo = ['a', 'b', 'c'];
  <b>Assert</b>( i >= 0 || i < 3, 'Invalid value.' );
  Print( foo[i] );
```
> > ##### note: #####
> > > The error output can be redirected by redefining the _configuration.stderr function. see the Print() function._

#### <font color='white' size='1'><b>CollectGarbage</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>CollectGarbage</b>()
> > Performs garbage collection in the JS memory pool.

#### <font color='white' size='1'><b>MaybeCollectGarbage</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>MaybeCollectGarbage</b>()
> > Performs a conditional garbage collection of JS objects, doubles, and strings that are no longer needed by a script executing.
> > This offers the JavaScript engine an opportunity to perform garbage collection if needed.

#### <font color='white' size='1'><b>TimeCounter</b></font> ####

> <sub>real</sub> <b>TimeCounter</b>()
> > Returns the current value of a high-resolution time counter in millisecond.
> > The returned value is a relative time value.

#### <font color='white' size='1'><b>StringRepeat</b></font> ####

> <sub>string</sub> <b>StringRepeat</b>( $STR str, $INT count )
> > Returns the string that is _count_ times _str_.
> > ##### example: #####
```
  Print( StringRepeat('foo', 3) ); // prints: foofoofoo
```

#### <font color='white' size='1'><b>Print</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Print</b>( value1 [[.md](.md), value2 [[.md](.md), ...]] )
> > Display _val_ to the output (the screen by default).
> > ##### example: #####
```
  Print( 'Hello', ' ', 'World', '\n' ); // prints: Hello World
```
> > ##### note: #####
> > > The output can be redirected by redefining the _configuration.stdout function.
> > > ##### example: #####
```
   LoadModule('jsstd');
   Print('foo\n'); // prints: foo
   _configuration.stdout = function() {}
   Print('bar\n'); // prints nothing
```_

#### <font color='white' size='1'><b>Exec</b></font> ####

> <sub>value</sub> <b>Exec</b>( fileName [[.md](.md), useAndSaveCompiledScript = true] )
> > Executes the script specified by _fileName_.
> > If _useAndSaveCompiledScript_ is true, the function load and save a compiled version (using XDR format) of the script on the disk ( adding 'xrd' to _fileName_ ).
> > If the compiled file is not found, the uncompiled version is used instead.
> > ##### return value: #####
> > > returns the last evaluated statement of the script.

> > ##### example: #####
```
  var foo = Exec('constants.js'); // loads constants.js or constants.jsxrd if available.
```

#### <font color='white' size='1'><b>SandboxEval</b></font> ####

> <sub>value</sub> <b>SandboxEval</b>( scriptCode [[.md](.md) , queryCallback ] [[.md](.md) , maxExecutionTime = 1000 ] )
> > Evaluates the JavaScript code in a sandbox with a new set of standard classes (Object, Math, ...).
> > ##### arguments: #####
      1. <sub>string</sub> _scriptCode_: the unsecured script code to be executed.
      1. <sub>`UN`</sub> _queryCallback_: this function may be called by the unsecured script to query miscellaneous information to the host script. For security reasons, the function can only return primitive values (no objects).
      1. <sub>integer</sub> _maxExecutionTime_: if defined, an OperationLimit exception is thrown when _maxExecutionTime_ milliseconds are elapsed.
> > ##### return value: #####
> > > the value of the last-executed expression statement processed in the script.

> > ##### example 1: #####
```
  function QueryCallback(val) {
   return val;
  }
  var res = SandboxEval('1 + 2 + Query(3)', QueryCallback);
  Print( res ); // prints: 6
```
> > ##### example 2: #####
```
  var res = SandboxEval('Math');
  Print( res == Math ); // prints: false
```
> > ##### example 3: #####
> > > abort very-long-running scripts.
```
  try {
   var res = SandboxEval('while (true);', undefined, 1000);
  } catch (ex if ex instanceof OperationLimit) {
   Print( 'script execution too long !' );
  }
```

#### <font color='white' size='1'><b>IsStatementValid</b></font> ####

> <sub>boolean</sub> <b>IsStatementValid</b>( statementString )
> > Returns true if _statementString_ is a valid Javascript statement.
> > The intent is to support interactive compilation, accumulate lines in a buffer until IsStatementValid returns true, then pass it to an eval.
> > This function is useful to write an interactive console.

#### <font color='white' size='1'><b>Halt</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Halt</b>()
> > Stop the execution of the program. This is a ungraceful way to finish a program and should only be used in critical cases.

### Static properties ###

#### <font color='white' size='1'><b>isConstructing</b></font> ####

> <sub>boolean</sub> <b>isConstructing</b>
> > Determines whether or not the function currently executing was called as a constructor.

#### <font color='white' size='1'><b>disableGarbageCollection</b></font> ####

> <sub>boolean</sub> <b>disableGarbageCollection</b>
> > Set to _true_, this property desactivates the garbage collector.



---

## jsstd static members ##
- [top](#jsstd_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsstd/static.cpp?r=2577) -

### Static functions ###

#### <font color='white' size='1'><b>Expand</b></font> ####

> <sub>string</sub> <b>Expand</b>( str [[.md](.md), obj | function] )
> > Return an expanded string using key/value stored in _obj_.
> > <br />
> > If _obj_ is omitted, the current object is used to look for key/value.
> > ##### example: #####
```
  function Test() {
   this.Expand = Expand;
   this.a = 123;
  }
  Print( new Test().Expand('$(a)') );
```
> > ##### note: #####
> > > undefined values are ignored in the resulting string.

> > ##### example: #####
```
  Expand(' $(h) $(w)', { h:'Hello', w:'World' }); // returns "Hello World"
```

#### <font color='white' size='1'><b>InternString</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>InternString</b>( string )
> > Make an interned string, a string that is automatically shared with other code that needs a string with the same value. Use this function with care.

#### <font color='white' size='1'><b>Seal</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Seal</b>( obj [[.md](.md) , recursively  ] )
> > Prevent modification of object fields. ie. all write access to the object, either to add a new property, delete an existing property, or set the value or attributes of an existing property.
> > If _recursively_ is true, the function seal any non-null objects in the graph connected to obj's slots.
> > ##### example: #####
```
  LoadModule('jsstd');

  var obj = { a:1 };
  obj.b = 2;
  Seal(obj);
  obj.c = 3; // Error: obj.c is read-only
```

#### <font color='white' size='1'><b>Clear</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Clear</b>( obj )
> > Remove all properties associated with an object.
> > ##### note: #####
> > > <b>Clear</b> removes all of obj's own properties, except the special proto and parent properties, in a single operation. Properties belonging to objects on obj's prototype chain are not affected.

> > ##### example: #####
```
  LoadModule('jsstd');

  var obj = { a:1, b:[2,3,4], c:{} };
  Print( uneval(obj) ); // prints: ({a:1, b:[2, 3, 4], c:{}})

  Clear(obj);
  Print( uneval(obj) ); // prints: ({})
```

#### <font color='white' size='1'><b>SetScope</b></font> ####

> <sub>Object</sub> <b>SetScope</b>( obj, scopeObject )
> > Set the scope object of _obj_. Use this function with care.
> > ##### example 1: #####
```
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
```
> > prints:
> > > `55` <br />
> > > `7` <br />
> > > `55` <br />

> > ##### example 2: #####
```
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
```

#### <font color='white' size='1'><b>SetPropertyEnumerate</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>SetPropertyEnumerate</b>( object, propertyName, polarity )
> > Show/Hide a property to for-in loop.
> > ##### note: #####
> > > Using this function may change the order of the properties within the object.

> > ##### example: #####
```
  var obj = { a:1, b:2, c:3 };
  for ( var p in obj )
   Print(p, ', '); // prints: a, b, c

  SetPropertyEnumerate(obj, 'b', false);
  for ( var p in obj )
   Print(p, ', '); // prints: a, c

  SetPropertyEnumerate(obj, 'b', true);
  for ( var p in obj )
   Print(p, ', '); // prints: a, c, b
```

#### <font color='white' size='1'><b>SetPropertyReadonly</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>SetPropertyReadonly</b>( object, propertyName, polarity )
> > Make a property not settable. Any attempt to modify a read-only property fail silently.
> > ##### example: #####
```
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
```

#### <font color='white' size='1'><b>ObjectToId</b></font> ####

> <sub>integer</sub> <b>ObjectToId</b>( object )
> > Returns an integer value that is a unique identifier of the object _object_ .
> > ##### example: #####
```
  var myObj = {};
  Print( ObjectToId(myObj), '\n' );
```

#### <font color='white' size='1'><b>IdToObject</b></font> ####

> <sub>integer</sub> <b>IdToObject</b>( id )
> > Returns the object with the identifier _id_ or undefined if the identifier do not exist or the object has been GCed. It is up to you to keep a reference to the object if you want to keep it through GC cycles.
> > ##### example 1: #####
```
  var myObj = {};
  Print( IdToObject( ObjectToId(myObj) ) === myObj ); // prints true
```
> > ##### example 2: #####
```
  var id = ObjectToId({});
  Print( IdToObject(id) ); // prints: [object Object]
  CollectGarbage();
  Print( IdToObject(id) ); // prints: undefined
```

#### <font color='white' size='1'><b>IsBoolean</b></font> ####

> <sub>boolean</sub> <b>IsBoolean</b>()
> > Returns _true_ if the value is a boolean.

#### <font color='white' size='1'><b>IsPrimitive</b></font> ####

> <sub>boolean</sub> <b>IsPrimitive</b>()
> > Returns _true_ if the value is a primitive ( null or not an object ).

#### <font color='white' size='1'><b>IsFunction</b></font> ####

> <sub>boolean</sub> <b>IsFunction</b>()
> > Returns _true_ if the value is a function.

#### <font color='white' size='1'><b>IsVoid</b></font> ####

> <sub>boolean</sub> <b>IsVoid</b>()
> > Returns _true_ if the value is undefined (ie. void 0).
> > ##### example: #####
```
  Print( IsVoid(undefined) ); // prints: true
```

#### <font color='white' size='1'><b>XdrEncode</b></font> ####

> <sub>Blob</sub> <b>XdrEncode</b>( value )
> > Encode (serialize) a JavaScript value into an XDR (eXternal Data Representation) blob.
> > ##### note: #####
> > > All JavaScript values cannot be encoded into XDR. If the function failed to encode a value, an error is raised. The Map object can help you to encode Object and Array.

#### <font color='white' size='1'><b>XdrDecode</b></font> ####

> <sub>value</sub> <b>XdrDecode</b>( xdrBlob )
> > Decode (deserialize) XDR (eXternal Data Representation) blob to a JavaScript value.
> > ##### <font color='red'>beware</font>: #####
> > > Decoding malformed XDR data can lead the program to crash. This may be an important security issue. Decode only trusted data.

#### <font color='white' size='1'><b>Warning</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Warning</b>( text )
> > Report the given _text_ as warning. The warning is reported on the stderr. Warnings ignored in unsafeMode.

#### <font color='white' size='1'><b>Assert</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Assert</b>( expression [[.md](.md), failureMessage ] )
> > If the argument expression compares equal to zero, the failureMessage is written to the standard error device and the program stops its execution.
> > ##### example: #####
```
  var foo = ['a', 'b', 'c'];
  <b>Assert</b>( i >= 0 || i < 3, 'Invalid value.' );
  Print( foo[i] );
```
> > ##### note: #####
> > > The error output can be redirected by redefining the _configuration.stderr function. see the Print() function._

#### <font color='white' size='1'><b>CollectGarbage</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>CollectGarbage</b>()
> > Performs garbage collection in the JS memory pool.

#### <font color='white' size='1'><b>MaybeCollectGarbage</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>MaybeCollectGarbage</b>()
> > Performs a conditional garbage collection of JS objects, doubles, and strings that are no longer needed by a script executing.
> > This offers the JavaScript engine an opportunity to perform garbage collection if needed.

#### <font color='white' size='1'><b>Sleep</b></font> ####

> <sub>real</sub> <b>Sleep</b>( time )
> > Suspends the execution of the current program during _time_ milliseconds.

#### <font color='white' size='1'><b>TimeCounter</b></font> ####

> <sub>real</sub> <b>TimeCounter</b>()
> > Returns the current value of a high-resolution time counter in millisecond.
> > The returned value is a relative time value.
> > ##### example: #####
```
  LoadModule('jsstd');
  LoadModule('jsio');
  Print( 't0: '+TimeCounter(), '\n' ); // prints: 1743731894.4259675
  Print( 't1: '+TimeCounter(), '\n' ); // prints: 1743731896.1083043
  Sleep(100);
  Print( 't2: '+TimeCounter(), '\n' ); // prints: 1743732003.6174989
```

#### <font color='white' size='1'><b>StringRepeat</b></font> ####

> <sub>string</sub> <b>StringRepeat</b>( $STR str, $INT count )
> > Returns the string that is _count_ times _str_.
> > ##### example: #####
```
  Print( StringRepeat('foo', 3) ); // prints: foofoofoo
```

#### <font color='white' size='1'><b>Print</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Print</b>( value1 [[.md](.md), value2 [[.md](.md), ...]] )
> > Display _val_ to the output (the screen by default).
> > ##### example: #####
```
  Print( 'Hello', ' ', 'World', '\n' ); // prints: Hello World
```
> > ##### note: #####
> > > The output can be redirected by redefining the _configuration.stdout function.
> > > ##### example 1: #####
```
   LoadModule('jsstd');
   Print('Hello', 'World'); // prints: HelloWorld
```
> > > ##### example 2: #####
```
   LoadModule('jsstd');
   Print('foo\n'); // prints: foo
   _configuration.stdout = function() {}
   Print('bar\n'); // prints nothing
```_

#### <font color='white' size='1'><b>Exec</b></font> ####

> <sub>value</sub> <b>Exec</b>( fileName [[.md](.md), useAndSaveCompiledScript = true] )
> > Executes the script specified by _fileName_.
> > If _useAndSaveCompiledScript_ is true, the function load and save a compiled version (using XDR format) of the script on the disk ( adding 'xrd' to _fileName_ ).
> > If the compiled file is not found, the uncompiled version is used instead.
> > ##### return value: #####
> > > returns the last evaluated statement of the script.

> > ##### example: #####
```
  var foo = Exec('constants.js'); // loads constants.js or constants.jsxrd if available.
```

#### <font color='white' size='1'><b>SandboxEval</b></font> ####

> <sub>value</sub> <b>SandboxEval</b>( scriptCode [[.md](.md) , queryCallback ] [[.md](.md) , maxExecutionTime = 1000 ] )
> > Evaluates the JavaScript code in a sandbox with a new set of standard classes (Object, Math, ...).
> > ##### arguments: #####
      1. <sub>string</sub> _scriptCode_: the unsecured script code to be executed.
      1. <sub>`UN`</sub> _queryCallback_: this function may be called by the unsecured script to query miscellaneous information to the host script. For security reasons, the function can only return primitive values (no objects).
      1. <sub>integer</sub> _maxExecutionTime_: if defined, an OperationLimit exception is thrown when _maxExecutionTime_ milliseconds are elapsed.
> > ##### return value: #####
> > > the value of the last-executed expression statement processed in the script.

> > ##### example 1: #####
```
  function QueryCallback(val) {
   return val;
  }
  var res = SandboxEval('1 + 2 + Query(3)', QueryCallback);
  Print( res ); // prints: 6
```
> > ##### example 2: #####
```
  var res = SandboxEval('Math');
  Print( res == Math ); // prints: false
```
> > ##### example 3: #####
> > > abort very-long-running scripts.
```
  try {
   var res = SandboxEval('while (true);', undefined, 1000);
  } catch (ex if ex instanceof OperationLimit) {
   Print( 'script execution too long !' );
  }
```

#### <font color='white' size='1'><b>IsStatementValid</b></font> ####

> <sub>boolean</sub> <b>IsStatementValid</b>( statementString )
> > Returns true if _statementString_ is a valid Javascript statement.
> > The intent is to support interactive compilation, accumulate lines in a buffer until IsStatementValid returns true, then pass it to an eval.
> > This function is useful to write an interactive console.
> > ##### example: #####
```
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
```

#### <font color='white' size='1'><b>Halt</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Halt</b>()
> > Stop the execution of the program. This is a ungraceful way to finish a program and should only be used in critical cases.

### Static properties ###

#### <font color='white' size='1'><b>isConstructing</b></font> ####

> <sub>boolean</sub> <b>isConstructing</b>
> > Determines whether or not the function currently executing was called as a constructor.

#### <font color='white' size='1'><b>disableGarbageCollection</b></font> ####

> <sub>boolean</sub> <b>disableGarbageCollection</b>
> > Set to _true_, this property desactivates the garbage collector.


---

## class jsstd::Buffer ##
- [top](#jsstd_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsstd/buffer.cpp?r=2557) -

> Buffer class is a simple buffer that allows arbitrary length input and output.

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####
> <i><b>constructor</b></i>( [[.md](.md)source] )
> > Constructs a Buffer object.
> > ##### arguments: #####
      1. <sub>streamObject</sub> _source_: any object that supports NIStreamRead interface. The Buffer uses this object when its length is less than the requested amount of data.
> > > ##### <font color='red'>beware</font>: #####
> > > > When used by the Buffer, the source MUST return the exact or less data than the required size else the remaining is lost (not stored in the buffer).

> > > ##### note: #####
> > > > You can use the Write() function in the source like:
```
    var buf = new Buffer({ Read:function(count) { buf.Write('some data') } });
```


> ##### example 1: #####
```
  var buf1 = new Buffer(Stream('456'));
  buf1.Write('123');
  Print( buf1.Read(6) ); // prints: '123456'
```

> ##### example 2: #####
```
  var buf1 = new Buffer(Stream('123'));
  Print( buf1.Read(1) ,'\n'); // prints: '1'
  Print( buf1.Read(1) ,'\n'); // prints: '2'
  Print( buf1.Read(1) ,'\n'); // prints: '3'
```

> ##### example 3: #####
```
  var buf2 = new Buffer(new function() {
   this.Read = function(count) StringRepeat('x',count);
  });
  Print( buf2.Read(6) ); // prints: 'xxxxxx'
```

> ##### example 4: #####
> > Create a long chain ( pack << buffer << buffer << stream )
```
  var p = new Pack(new Buffer(new Buffer(Stream('\x12\x34'))));
  Print( (p.ReadInt(2, false, true)).toString(16) ); // prints: '1234'
```

### Methods ###

#### <font color='white' size='1'><b>Clear</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Clear</b>()
> > Empty the whole buffer.

#### <font color='white' size='1'><b>Write</b></font> ####

> <b>Write</b>( data [[.md](.md), length] )
  * <font color='gray' size='1'><sub>void</sub></font> <b>Write</b>( buffer )
> > Add _data_ in the buffer. If _length_ is used, only the first _length_ bytes of _data_ are added.
> > The second form allow to add another whole buffer in the current buffer.

#### <font color='white' size='1'><b>Match</b></font> ####

> <sub>boolean</sub> <b>Match</b>( str [[.md](.md), consume = false ] )
> > Check if the given string _str_ matchs to the next data in the buffer.
> > ##### arguments: #####
      1. <sub>string</sub> _str_
      1. <sub>boolean</sub> _consume_: if false, just check if it match without consuming data, else, read and check.
> > ##### return value: #####
> > > true if it matchs, else false.

#### <font color='white' size='1'><b>Read</b></font> ####

> <sub>string</sub> <b>Read</b>( [[.md](.md) amount ] )
> > Read _amount_ data in the buffer. If _amount_ is omited, The whole buffer is returned.
> > ##### <font color='red'>beware</font>: #####
> > > This function returns a Blob or a string literal as empty string.

> > <br />
> > If _amount_ == undefined, an arbitrary (ideal) amount of data is returned. Use this when you don't know how many data you have to read.
> > ##### example: #####
```
  var chunk = buffer.Read(undefined);
```
> > ##### note: #####
> > The read operation never blocks, even if the requested amount of data is greater than the buffer length.

#### <font color='white' size='1'><b>Skip</b></font> ####

> <b>Skip</b>( length )
> > Skip _length_ bytes of data from the buffer.

#### <font color='white' size='1'><b>ReadUntil</b></font> ####

> <sub>string</sub> <b>ReadUntil</b>( boundaryString [[.md](.md), skip] )
> > Reads the buffer until it match the _boundaryString_, else it returns _undefined_.
> > If _skip_ argument is _true_, the _boundaryString_ is skiped from the buffer.

#### <font color='white' size='1'><b>IndexOf</b></font> ####

> <sub>integer</sub> <b>IndexOf</b>( string )
> > Find _string_ in the buffer and returns the offset of the first letter. If not found, this function returns -1.

#### <font color='white' size='1'><b>Unread</b></font> ####

> <sub>string</sub> <b>Unread</b>( _data_ )
> > Insert _data_ at the begining of the buffer. This function can undo a read operation. The returned value is _data_.
> > ##### example: #####
```
  function Peek(len) {

    return buffer.Unread( buffer.Read(len) );
  }
```

#### <font color='white' size='1'><b><i>toString</i></b></font> ####

> <sub>string</sub> <b><i>toString</i></b>()
> > Converts the whole content of the buffer to a string.
> > ##### note: #####
> > > The buffer is not modified.

#### <font color='white' size='1'><i><b><a href='N.md'>N</a> operator</b></i></font> ####

> <sub>char</sub> <i><b><a href='N.md'>N</a> operator</b></i>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Used to access the character in the _N\_th position where_N_is a positive integer between 0 and one less than the value of length._

### Properties ###

#### <font color='white' size='1'><b>length</b></font> ####

> <sub>integer</sub> <b>length</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Is the current length of the buffer.

### Native Interface ###
  * **NIStreamRead**

### example 1 ###
```
 var buf = new Buffer();
 buf.Write('1234');
 buf.Write('5');
 buf.Write('');
 buf.Write('6789');
 Print( buf.Read() );
```

### example 2 ###
```
 var buf = new Buffer();
 buf.Write('0123456789');
 Print( buf.Read(4) );
 Print( buf.Read(1) );
 Print( buf.Read(1) );
 Print( buf.Read(4) );
```


### example 3 ###

> Buffered read from a stream.
```
 function ReadFromFile() {

  Print('*** read from the file\n');
  return StringRepeat('x',5);
 }

 var buf = new Buffer({ Read:function() { buf.Write(ReadFromFile()); }})

 for ( var i=0; i<15; i++ )
  Print( buf.Read(1), '\n' )
```


---

## class jsstd::Map ##
- [top](#jsstd_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsstd/map.cpp?r=2555) -
> Map is an unsorted associative container that associates strings with values.
> ##### note: #####
    * No two elements have the same key.
    * Map can store any string value as a key (even reserved special strings like proto, parent, toString, ...).
    * Constructing the object is not mendatory. Calling Map() will return a new Map object;

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####
> <i><b>constructor</b></i>( [[.md](.md) obj ] )
> > Creates a new Map object.
> > ##### arguments: #####
      1. <sub>Object</sub> _obj_: if given, _obj_ is converted into a Map object (Array is also an object). No recursion is made.

### example 1 ###
```
 var m = new Map();
 m.a = 1;
 m.b = 2;
 m.c = 3;

 Print( m.c ); // prints: 3
```

### example 2 ###
```
 var m = new Map([1,2,3,4]);
 Print( [k+'='+v for ([k,v] in Iterator(m))] ); // prints: 3=4,2=3,1=2,0=1
```


---

## class jsstd::ObjEx ##
- [top](#jsstd_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsstd/objex.cpp?r=2555) -

> This class give the ability to spy properties changes. One can listen for add, del, set and set events on the object.
> It is also possible to store an hidden auxiliary value that can be access using ObjEx.Aux( _ObjEx object_ ) static function.

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####
> <i><b>constructor</b></i>( [[.md](.md)addCallback], [[.md](.md)delCallback], [[.md](.md)getCallback], [[.md](.md)setCallback] [[.md](.md), auxObject] )
> > Constructs a ObjEx object.
> > ##### arguments: #####
      1. <sub>`UN`</sub> _addCallback_: called when the property is added to the object.
      1. <sub>`UN`</sub> _delCallback_: called when the property is deleted form the object.
      1. <sub>`UN`</sub> _getCallback_: called when the property is read form the object.
      1. <sub>`UN`</sub> _setCallback_: called when the property is changed. This include when the property is added.
      1. <sub>value</sub> _auxObject_:
> > ##### note: #####
> > > addCallback, delCallback, getCallback, setCallback: can be set to the _undefined_ value.

> > ##### behavior: #####
> > > addCallback, delCallback, getCallback, setCallback functions are called according to the operation done on the object.
> > > These callback functions are called with the following arguments:
        1. # <sub>string</sub> _propertyName_ : the name of the property being handled.
        1. # <sub>value</sub> _propertyValue_ : the value of the property being handled.
        1. # <sub>value</sub> _auxObject_ : the _auxObject_ provided to the constructor.
        1. # <sub>integer</sub> _callbackIndex_ : an integer that has the folowing value: 0 for addCallback, 1 for delCallback, 2 for getCallback, 3 for setCallback.

> > ##### note: #####
> > > addCallback callback function is called only when the property is being added, in opposition to _setCallback_ that is called each time the value is changed.

### Static functions ###

#### <font color='white' size='1'><b>Aux</b></font> ####

> <b>Aux</b>( objex [[.md](.md), newAux] )
> > Returns the _auxObject_ stored in the _objex_.
> > If newAux is given, it replaces the current auxiliary value of _objex_.

### Example ###
```
function addCallback( name, value ) {

   Print('adding ' + name + ' = ' + value);
}

var obj = new ObjEx( addCallback, undefined, undefined, undefined, null );

obj.foo = 123;
obj.foo = 456;
```
prints:
<pre>
adding foo = 123<br>
</pre>

### Example ###


> http://jsircbot.googlecode.com/svn/trunk/dataObject.js



---

## class jsstd::Pack ##
- [top](#jsstd_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsstd/pack.cpp?r=2555) -
> Pack is a class that helps to convert binary data into Integer, Real or String and to write an integer in a binary data string.
> The Pack class manages the system endian or network endian.

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####
> <i><b>constructor</b></i>( buffer )
> > Constructs a Pack object from a Buffer object. This is the only way to read or write binary data.
> > ##### arguments: #####
      1. <sub>Buffer</sub> _buffer_

### Methods ###

#### <font color='white' size='1'><b>ReadInt</b></font> ####

> <sub>integer</sub> <b>ReadInt</b>( size, [[.md](.md)isSigned = false], [[.md](.md)isNetworkEndian = false] )
> > Read an integer on the current stream. cf. systemIntSize property.

#### <font color='white' size='1'><b>WriteInt</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>WriteInt</b>( intValue, [[.md](.md)isSigned = false [[.md](.md), isNetworkEndian = false]] )
> > Write an integer to the current stream. cf. systemIntSize property.

#### <font color='white' size='1'><b>ReadReal</b></font> ####

> <sub>real</sub> <b>ReadReal</b>( size )
> > Read a 4-byte single precision real (float) or a 8-byte double precision real (double) on the current stream.

#### <font color='white' size='1'><b>ReadString</b></font> ####

> <sub>string</sub> <b>ReadString</b>( length )
> > Read a string of the specifiex _length_ on the current stream.

### Properties ###

#### <font color='white' size='1'><b>buffer</b></font> ####

> <sub>Buffer</sub> **buffer**
> > Is the current Buffer object.

### Static properties ###

#### <font color='white' size='1'><b>systemIntSize</b></font> ####

> <b>systemIntSize</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Is the size (in Byte) of a system int.

#### <font color='white' size='1'><b>systemBigEndian</b></font> ####

> <b>systemBigEndian</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Is _true_ if the system endian is BigEndian else is `ALSE`.


---

- [top](#jsstd_module.md) - [main](JSLibs.md) -
