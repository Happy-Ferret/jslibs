loadModule('jsstd');

/// sandboxEval

	sandboxEval('[].a');

/// exec error

	var filename = 'jsqa_' + QA.randomString(8);
	new File(filename).content = '\n\n\n\nfunction();\n';
	
//	var errorMsg = '';
//	var tmp = host.stderr;
//	host.stderr = function(txt) {
//		
//		errorMsg += txt;
//	}
	
	QA.ASSERTOP(function() { exec(filename, false) }, 'ex', SyntaxError, 'exec handle compilation errors');

	new File(filename).content = undefined;
	
//	host.stderr = tmp;
//	QA.ASSERT( errorMsg.indexOf('function()') != -1, true, 'correct error message' );
	

/// Count object properties [p]

	QA.ASSERT( countProperties({}), 0, 'test with 0 properties' );
	QA.ASSERT( countProperties({a:1}), 1, 'test with 1 properties' );
	QA.ASSERT( countProperties({a:1, b:2}), 2, 'test with 2 properties' );

	var obj = {a:1, b:2};
	delete obj.a;
	delete obj.b;
	QA.ASSERT( countProperties(obj), 0, 'test with deleted properties' );

/// SwitchCase function [p]

	QA.ASSERT( switchCase( 1, [1, '1'], ['num', 'str'] ), 'num', 'SwitchCase type' );
	QA.ASSERT( switchCase( '1', [1, '1'], ['num', 'str'] ), 'str', 'SwitchCase type' );

	QA.ASSERT( switchCase( 2, [1, '1'], ['num', 'str'] ), undefined, 'SwitchCase not found' );
	QA.ASSERT( switchCase( 2, [1, '1'], ['num', 'str'], 'def' ), 'def', 'SwitchCase default value' );


/// print returns undefined [p]
		QA.ASSERT( print(), undefined, 'print return value' );


/// StringRepeat function [p]

		QA.ASSERT( stringRepeat( '', 0 ), '', '0 x empty' );
		QA.ASSERT( stringRepeat( '', 1 ), '', '1 x empty' );
		QA.ASSERT( stringRepeat( '', 10 ), '', '10 x empty' );
		
		QA.ASSERT( stringRepeat( 'a', 0 ), '', '0 x 1 char' );
		QA.ASSERT( stringRepeat( 'a', 1 ), 'a', '1 x 1 char' );
		QA.ASSERT( stringRepeat( 'a', 10 ), 'aaaaaaaaaa', '10 x 1 char' );
		
		QA.ASSERT( stringRepeat( 'abc', 0 ), '', '0 x string' );
		QA.ASSERT( stringRepeat( 'abc', 1 ), 'abc', '1 x string' );
		QA.ASSERT( stringRepeat( 'abc', 10 ), 'abcabcabcabcabcabcabcabcabcabc', '10 x string' );

		QA.ASSERT( stringRepeat( '\u1234', 2 ), '\u1234\u1234', 'UC string' );

/// big StringRepeat []

		QA.ASSERT( stringRepeat( '123', 1024*1024 ).length, 3*1024*1024, '3MB' );


/// Garbage collector []
		
//		loadModule('jsdebug');
		
		var str = QA.randomString(1024*1024);
		
		QA.gc();
	
		for ( var i = 0; i < 4; i++ )
			str += str;
			
//		QA.ASSERT( gcBytes, str.length, 'lot of allocated memory' );
		
		QA.gc();


/// hide properties [p]
	
		var o = { a:1, b:2, c:3, d:4 };

		//setPropertyEnumerate(o, 'b', false);
		Object.defineProperty(o, 'b', {enumerable : false});  
		//setPropertyEnumerate(o, 'c', false);
		Object.defineProperty(o, 'c', {enumerable : false});  
		
		QA.ASSERT( o.b, 2, 'do not delete' );
		QA.ASSERT( [p for each (p in o)].join(','), '1,4', 'visible properties' );


/// SetScope function [pd]

		var data = 55;
		function bar() { QA.ASSERT( data, 7, 'modified scope' ); }
		var old = setScope( bar, {data:7, QA:QA} );
		bar();



/// Expand function [p]

		QA.ASSERT( expand('\u1234', {} ), '\u1234', 'unicode' );
		QA.ASSERT( expand('$(\u1234)', { '\u1234':'ok' } ), 'ok', 'unicode key' );
		QA.ASSERT( expand('$(foo)', { foo:'\u1234' } ), '\u1234', 'unicode value' );

		QA.ASSERT( expand('abcde', {} ), 'abcde', 'no expand' );
		QA.ASSERT( expand('ab$()cde', {} ), 'abcde', 'empty expand' );
		QA.ASSERT( expand('ab$(xx)cde', {} ), 'abcde', 'unknown expand' );
		QA.ASSERT( expand('$(foo)', {foo:'$(foo)'} ), '$(foo)', 'kind of escape for $()' );
		QA.ASSERT( expand('', { h:'Hello', w:'World' }), '', 'expanding an empty string' );
		
		QA.ASSERT( expand('Hello World'), 'Hello World', 'expanding a simple string' );
		
		QA.ASSERT( expand(' $(h) $(w)', { h:'Hello', w:'World' }), ' Hello World', 'expanding a string' );
		QA.ASSERT( expand(' $(h) $(w', { h:'Hello', w:'World' }), ' Hello ', 'expanding a bugous string' );
		QA.ASSERT( expand(' $(h) $(', { h:'Hello', w:'World' }), ' Hello ', 'expanding a bugous string' );
		QA.ASSERT( expand(' $(h) $', { h:'Hello', w:'World' }), ' Hello $', 'expanding a string' );
		QA.ASSERT( expand(' $(h)', { h:'Hello', w:'World' }), ' Hello', 'expanding a string' );

		QA.ASSERT( expand('$(c)'), '', 'expanding a string' );
		QA.ASSERT( expand('$(c)a'), 'a', 'expanding a string' );
		QA.ASSERT( expand('a$(c'), 'a', 'expanding a string' );
		QA.ASSERT( expand('a$()c'), 'ac', 'expanding a string' );
		QA.ASSERT( expand('a$(b)c'), 'ac', 'expanding a string' );
		QA.ASSERT( expand('a$(b)c', {}), 'ac', 'expanding a string' );
		QA.ASSERT( expand('a$(b)c', { b:'' }), 'ac', 'expanding a string' );
		QA.ASSERT( expand('$(b)c', { b:'' }), 'c', 'expanding a string' );
		QA.ASSERT( expand('a$(b)', { b:'' }), 'a', 'expanding a string' );
		QA.ASSERT( expand('a$()', { b:'' }), 'a', 'expanding a string' );
		QA.ASSERT( expand('$()a', { b:'' }), 'a', 'expanding a string' );
		QA.ASSERT( expand('$(b)$(b)$(b)', { b:'' }), '', 'expanding a string' );
		QA.ASSERT( expand('$(b)$(b)$(b)', { b:'x' }), 'xxx', 'expanding a string' );
		QA.ASSERT( expand('$(b)$(b$(b)', { b:'x' }), 'x', 'expanding a string' );
		QA.ASSERT( expand('$(b)$(b$(b)'), '', 'expanding a string' );

	
		QA.ASSERT( expand('a$(b()c)d', function(key) key), 'ab(c)d', 'expanding a string' );
		QA.ASSERT( expand('$(b)$(b$(b)', function(key) key), 'bb$(b', 'expanding a string' );
		QA.ASSERT( expand('$(b)$(b$(b', function(key) key), 'b', 'expanding a string' );
		QA.ASSERT( expand('$()', function(key) key), '', 'expanding a string' );
		QA.ASSERT( expand('a$(b)c$(d)e', function(key) key), 'abcde', 'expanding a string' );
		QA.ASSERT( expand('$(b)c$(d)', function(key) key), 'bcd', 'expanding a string' );
		QA.ASSERT( expand('$(a)', function(key) undefined), '', 'expanding a string' );
		QA.ASSERT( expand('$(a)', function(key) null), '', 'expanding a string' );
		QA.ASSERT( expand('', function(key) key), '', 'expanding a string' );
		QA.ASSERT( expand('', function(key) key), '', 'expanding a string' );

		QA.ASSERTOP( function() expand('$()', function(key) { throw 123 }), 'ex', 123, 'Expand function throw exception' );

		var obj = {
			toString: function() {
				return 'Hello';
			}
		}
		QA.ASSERT( expand('$(h)', { h:obj }), 'Hello', 'expanding a string' );
		
		var obj1 = {
			
			expand:expand,
			x:123,
			y:'456',
		}
		QA.ASSERT( obj1.expand('$(x)$(y)', obj1), '123456', 'expanding a string using this as map' );
		
		var o = { title:'My HTML Page', titi:1234, toString:function() { return expand( this.text, this ) } };
		o.text = '<html><title>$(title)</title>\n'
		QA.ASSERT_STR( o, '<html><title>My HTML Page</title>\n', 'expand string using this object' );

/// expand bug1 [p]

	var source = 'gl_LightSource[$(xxx)';
	QA.ASSERT_STR( expand(source, { xxx:1 }), 'gl_LightSource[1' );
		
		
/// Big expand []
		
	var exp = stringRepeat('$(X)', 1000);
	for ( var i = 0 ; i < 10; ++i )
		expand(exp, {X:'123'});


/// Expand using a callback function [p]

		var ids = '';
		var i = 0;
		var res = expand('ab$(c)$(d)e$(f)g$(h)ij', function(id) { ids+=id; return i++ } );
		QA.ASSERT_STR( res +'-'+ ids, 'ab01e2g3ij-cdfh', 'Expand result is correct' );


/// exec error [p]

		QA.ASSERTOP( function() exec('e654ser65t'), 'ex', ReferenceError, 'exec unknown file' );

		
/// exec basic test [p]
		
		loadModule('jsio');
		var f = new File('qa_exec_test.js');
		f.content = '((1234))';
		var res = exec('qa_exec_test.js', false);
		QA.ASSERT_STR( res, eval( stringify(f.content) ), 'content validity' );

		var res = exec.call(this, 'qa_exec_test.js', false);
		QA.ASSERT_STR( res, eval( stringify(f.content) ), 'content validity' );

		f.content = undefined;


/// exec using XDR [p]
		
		loadModule('jsio');
	
		var f = new File('qa_exec_test.js');
		f.content = '(1234)';
		
		var res = exec(f.name, true);
		QA.ASSERT( res, 1234, 'exec return value' );

		var fxdr = new File('qa_exec_test.jsxdr');
		QA.ASSERT( fxdr.exist, true, 'XDR file exist' );
		
		f.delete();

		QA.ASSERT( f.exist, false, 'do not have source file' );

		var res = exec(f.name, true);
		QA.ASSERT( res, 1234, 'exec using XDR file' );

		fxdr.delete();
		QA.ASSERT( fxdr.exist, false, 'XDR file is deleted' );
		
		try {
			
			var res = exec(f.name, false);
			QA.FAILED('exec do not detect missing file');
			
		} catch(ex) {
			
			QA.ASSERT( ex.constructor, ReferenceError, 'exec exception' );
			QA.ASSERT( ex.message.indexOf('xdr cannot be found') != -1, true, 'error message' );
		}


/// XDR serialization [pd]

		var s = new Script('/./');
		QA.ASSERTOP( function() xdrEncode(s), 'ex', TypeError );
		//var res = XdrDecode(XdrEncode(s));
		//QA.ASSERT_STR( s.toSource(), res.toSource(), 'XDR->unXDR a Script' );

		QA.ASSERT( xdrDecode(xdrEncode(Map({a:1, b:2, c:3}))).a, 1, 'XDR->unXDR a Map' );
		QA.ASSERT( xdrDecode(xdrEncode(Map({a:1, b:2, c:3}))).b, 2, 'XDR->unXDR a Map' );
		QA.ASSERT( xdrDecode(xdrEncode(Map({a:1, b:2, c:3}))).c, 3, 'XDR->unXDR a Map' );

		QA.ASSERT( xdrDecode(xdrEncode('6r54aze6r5')), '6r54aze6r5', 'XDR->unXDR a string' );
		QA.ASSERT( xdrDecode(xdrEncode(1234567)), 1234567, 'XDR->unXDR a number' );
		QA.ASSERT( xdrDecode(xdrEncode(1.234567)), 1.234567, 'XDR->unXDR a double' );


/// Clear function [p]

		var o = { x:5, y:6, z:7 };
		QA.ASSERT( 'z' in o, true, 'has z property' );
		clearObject(o);
		QA.ASSERT( 'z' in o, false, 'property z is cleared' );


/// Clear on an array [p]
	
		var o = { x:5, y:6, z:7 };
		QA.ASSERT( 'z' in o, true, 'has z property' );
		clearObject(o);
		QA.ASSERT( 'z' in o, false, 'property z is cleared' );



/// Seal function [p]
		
		var o = { a:1, b:{c:2} };
		deepFreezeObject(o);
		o.a = 5;
		QA.ASSERT( o.a, 1, 'freezed object' );
		o.b.c = 6;
		QA.ASSERT( o.b.c, 2, 'deep freezed object' );


/// IsStatementValid  function [p]
		
		QA.ASSERT( isStatementValid( 'for ( var i; i<10; i++ )' ), false, 'invalid statement' );
		QA.ASSERT( isStatementValid( 'for ( var i; i<10; i++ );' ), true, 'valid statement' );
		QA.ASSERT( isStatementValid( '{a,b,c} = { a:1, b:2, c:3 }' ), true, 'valid statement' );


/// StrChr function [p]

		var str1 = stringRepeat('y', 100);
		QA.ASSERT( [ c for each ( c in str1 ) if (c == 'y') ].length, 100, 'all chars are good' );
		
		var str = stringRepeat('x', 10000);
		QA.ASSERT( str.length, 10000, 'string length' );
		QA.ASSERT( str[0], 'x', 'first char' );
		QA.ASSERT( str[9999], 'x', 'last char' );


/// Sandbox global objects [p]
	
		var res = sandboxEval('Math');
		QA.ASSERT( res.toString(), Math.toString(), 'Math object' );
		QA.ASSERT( res == Math, false, 'Global objects' );


/// Sandbox external access [p]

		loadModule('jsio');
		var res = sandboxEval('typeof File');
		QA.ASSERT( res == typeof File, false, 'forbidden File class access' );
		var res = sandboxEval('typeof loadModule');
		QA.ASSERT( res == typeof loadModule, false, 'forbidden loadModule function access' );


/// Sandbox basic query [p]

	QA.ASSERTOP( sandboxEval('', function() 123), '===', undefined );
	QA.ASSERTOP( sandboxEval('typeof query'), '===', 'undefined' );
	QA.ASSERTOP( sandboxEval('query()', function() 123), '===', 123 );
	

/// Sandbox Query []

		var res = Function("var v = 567; return sandboxEval('query()', function(val) v)")();
		QA.ASSERT( res, 567, 'SandboxEval result using Function( query function )' );

		var res = sandboxEval('query(123)', function(val) val + 1 );
		QA.ASSERT( res, 124, 'SandboxEval result using query function' );
		
		var obj = {};
		QA.ASSERTOP( function() { sandboxEval('query()', function(val) obj) }, 'ex', 'TypeError', 'Query returns a non-primitive value');

//		var obj = { abc: 321 };
//		QA.ASSERT( SandboxEval('query()', function(val) obj).abc, 321, 'Query return value');


/// Disabled GC [d]

		var prev = disableGarbageCollection;
		disableGarbageCollection = true;
		QA.ASSERT( disableGarbageCollection, true, 'GC is disabled' );
		
		QA.gc();
		var mem0 = privateMemoryUsage;
		var str = stringRepeat('x', 1000000);
		str = undefined;
		QA.gc();
		var mem1 = privateMemoryUsage;
		QA.ASSERT( mem1 >= mem0 + 1000000, true, 'without GC' );
		
		disableGarbageCollection = prev;

/*	
		QA.gc();
		var mem0 = privateMemoryUsage;
		var str = stringRepeat('x', 1000000);
		str = undefined;
		QA.gc();
		var mem1 = privateMemoryUsage;
		QA.ASSERT( Math.abs(mem1/mem0) < 1.1, true, 'with GC' );
*/

/// Sandbox misc [p]

	var s = sandboxEval('var a = Math.abs(-123); a');
	QA.ASSERT( s, 123, 'abs' );


/// Sandbox watchdog []
	
	QA.ASSERTOP( function() { sandboxEval('for (var i=0; i<10000000000; ++i);', undefined, 250) }, 'ex', OperationLimit, 'OperationLimit detection' );


/// OperationLimit is not extensible []

	try {
		
		sandboxEval('for (var i = 0; i < 100000; ++i);', undefined, 0);
		QA.FAILED('OperationLimit exception not thrown');
	} catch (ex) {

		QA.ASSERTOP( function() { 'use strict'; ex.__proto__.test = 1 }, 'ex', TypeError, 'OperationLimit extensibility' );
	}


/// Sandbox stack overflow []

	QA.ASSERTOP( function() { sandboxEval('(function f(){f();})();') }, 'ex', 'InternalError', 'Stack overflow detection' );


/// exec function []
	
	var filename = QA.randomString(10);
	new File(filename).content = '_exectest++';
	
	_exectest = 5;
	exec(filename);
	QA.ASSERT( _exectest, 6, 'exec an expression script (1)' );
	exec(filename);
	QA.ASSERT( _exectest, 7, 'exec an expression script (2)' );
		
	new File(filename).content = undefined;
	QA.ASSERT( new File(filename).content, undefined, 'exec etest file deletion' );
	new File(filename+'xdr').content = undefined;

	delete global._exectest;


/// warning messages [p]

	if ( QA.IS_UNSAFE ) // warning messages are disabled in unsafe mode
		return;
		
	var buffer = '';
	var prev = host.stderr;
	host.stderr = function(chunk) buffer += chunk;
	try {
		warning('testing warning messages');
	} catch (ex) {}
	host.stderr = prev;
	
	//print(buffer.length)
	QA.ASSERT_STR( buffer.indexOf('warning: testing warning messages') != -1, true, 'stderr redirection result' ); 

