LoadModule('jsstd');
LoadModule('jsstd');
LoadModule('jsio');

var f = new Function("\
		var buf = new Buffer();\
		buf.Write('abcdefghi');\
		function myStream() {\
			this.Read = function(amount) {\
				return buf.Read(2);\
			}\
		}\
	Print( buf, Stringify( new myStream() ) )\
");	

f();
		
		
Halt();

	_configuration.stderr = function(){}

	var s = "abcdefgh";
	var b = Blob(s);
	
/*
	Print('expected: '+b.substring( 100, " " ), '\n');
	b.substring( 1000, " " );
  throw 0;
*/


	function argGenerator(count, argList) {

		 var len = argList.length;
		 var pos = Math.pow(len, count);
		 var arg = new Array(count);
		 while (pos--) {

			  var tmp = pos;
			  for (var i = 0; i < count; i++) {

					var r = tmp % len;
					tmp = (tmp - r) / len;
					arg[i] = argList[r];
			  }
			  yield arg;
		 }
	}

	var argGen = argGenerator(2, [undefined, NaN, - Infinity, -1000, -100, -10, -3, -2.5, -2, -1, -0.6, -0.5, -0.4, 0, 0.4, 0.5, 0.6, 1, 2, 2.5, 3, 10, 100, 1000, + Infinity, "", " "]);
	try {
		 for (;;) {
		 
				var args = argGen.next();
				if ( s.substring.apply(s, args) != b.substring.apply(b, args) ) {
				
					Print( 'substring('+args.toSource()+')\n' );
					throw 0;
				}
		 }
	} catch (ex if ex instanceof StopIteration) {}

Print( 'done');
throw 2


var b = Blob("testxxx");
b.foo = 12345;
var xdrData = XdrEncode(b);
var val = XdrDecode(xdrData);
Print( val.length, '\n' );
Print( val.foo, '\n' );
Print( val, '\n' );

Halt(); ////////////////////////////////////////////////////////////////////////////////////////////////////////////


var code = "var db = {}; db.testFun = function() { return 123 }; Print( Test(db, 'testFun') )"
var func = new Function(code);
func();
//eval(code)



Halt(); ////////////////////////////////////////////////////////////////////////////////////////////////////////////

var b = new Blob('ABCDEF');
var s = 'ABCDEF'

Print( b.substr(-7,2) );

Halt();

Print( eval( new String('({add:123})') ) );

Halt();

b.prop1 = 11;
b.prop2 = 22;
b.prop3 = 33;
b.prop4 = 44;
b.prop5 = 55;
b.prop6 = 66;
b.prop7 = 77;

Print( b.concat, '\n' );


Print( b.replace, '\n' );

Print( b.replace('ab','cd'), '\n' );


Print( b.prop5, '\n' );

Print( b instanceof String, '\n' );

Halt();


b = b.concat( 'ABCD' );

Print( ''+b );


//Print( ''+b );


Halt();



//Stringify(new Buffer());
/*
var b = new Blob();
b._NI_BufferGet = 123;
Print( b._NI_BufferGet );
*/

var o = { _NI_BufferGet:123 }
Stringify(o);



Halt();



Print( new String('abc') == new String('abc') );

Halt();


Stringify( { __proto__: Stream('1234') } );

Halt();


var buf = new Buffer('abcdefghi');


function myStream() {
	this.Read = function(amount) {
		
		return buf.Read(2);
	}
}

var stream1 = new myStream();

Print( Stringify(stream1).quote() );
