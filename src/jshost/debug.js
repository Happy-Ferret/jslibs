// LoadModule('jsstd'); Exec('../common/tools.js');
// var QA = FakeQAApi;
// RunLocalQAFile();
// RunJsircbot(false);
// RunQATests('-rep 3 -exclude jstask');

// jslang_test(); throw 0;


LoadModule('jsstd');


while ( !endSignal )
	ProcessEvents( EndSignalEvents() ); // MetaPollTimeout(50)
Halt();


LoadModule('jsstd');
LoadModule('jstask');

var myTask = new Task(function(){
	LoadModule('jsstd');
	return 123;
});

myTask.Request();
Print( myTask.Response() );
Halt();



var t0 = TimeCounter();
var i = 0;
while ( i++ < 1000 )
	ProcessEvents( TimeoutEvents(0), TimeoutEvents(0), TimeoutEvents(0), TimeoutEvents(0), TimeoutEvents(0), TimeoutEvents(0), TimeoutEvents(0), TimeoutEvents(0) );

Print( (TimeCounter() - t0).toFixed(3), ' ', i,  '\n' );

Halt();


LoadModule('jsstd');
LoadModule('jsio');
descList = [];

while ( !endSignal )
	ProcessEvents( IOEvents(descList), EndSignalEvents() ); // MetaPollTimeout(50)

Halt();



var ev = MetaPoll( MetaPollTimeout(1), MetaPollTimeout(1), MetaPollTimeout(1), MetaPollTimeout(1), MetaPollTimeout(1), MetaPollTimeout(1), MetaPollTimeout(1), MetaPollTimeout(1), MetaPollTimeout(1), MetaPollTimeout(1) );
Print( 'events flags: '+ev.toString(2)+'\n' );

var ev = MetaPoll( MetaPollTimeout(1), MetaPollTimeout(1), MetaPollTimeout(1), MetaPollTimeout(1), MetaPollTimeout(1), MetaPollTimeout(1), MetaPollTimeout(1), MetaPollTimeout(1), MetaPollTimeout(1), MetaPollTimeout(1) );
Print( 'events flags: '+ev.toString(2)+'\n' );

Halt();

var t0 = +new Date();

while ( !endSignal ) {
	
	Print('.');
	MetaPoll( MetaPollTimeout(2000), MetaPollEndSignal(), MetaPollEndSignal() );
}

endSignal = false;

while ( !endSignal ) {
	
	Print('.');
//	MetaPoll( MetaPollTimeout(2000), MetaPollTimeout(500), MetaPollTimeout(1) , MetaPollTimeout(10) );
	MetaPoll( MetaPollTimeout(1), MetaPollEndSignal() );
}


var t = +new Date() - t0;



Print( t, '\n' );


Halt(); //////////////////////////////////////////////////////////////////////////////


function Test() {
	
	Print( TimeCounter() );
}

Test();

Print( uneval(Test).quote() );


Halt();

gcZeal = 2;

LoadModule('jsio');
LoadModule('jsstd');
while (!endSignal) {


	var buf = new Buffer();
	buf.Write('\xAA\xBB\xCC\xDD');
	var pack = new Pack(buf);
	pack.ReadInt(4, false, true).toString(16);

	var m = new Map();

	m.__proto__ = 'AAA', 
	m.__parent__ = 'BBB';
	m.toString = 'CCC';
	m.toSource = 'DDD';
	m.__count__ = 'EEE';

	var b = new Buffer();
	b.Write({ toString:function(){return '01'} });
	b.Write([2,3]);
	b.Write(4567);
	b.Write(8.9);
	b.Read()

	var res = Function("var v = 567; return SandboxEval('Query()', function(val) v)")();
	var res = SandboxEval('Query(123)', function(val) val + 1 );


	var res = SandboxEval('typeof File');
	var res = SandboxEval('typeof LoadModule');

	var res = SandboxEval('Math');

	var res = Function("var v = 567; return SandboxEval('Query()', function(val) v)")();
	var res = SandboxEval('Query(123)', function(val) val + 1 );
	var obj = {};
	(function() { SandboxEval('Query()', function(val) { obj })  })();
	
	Print('.');
}


LoadModule('jstask');
LoadModule('jsstd');
LoadModule('jsio');

LoadModule('jsstd');
LoadModule('jsz');
LoadModule('jstask');


Print( -2 % 2 );

		var list = [];
		function Add() {

			 list.push(arguments);
		}

		function Run() {

			 for each ( var item in list )
				  item[0]();
		}

		for ( var i = 0; i < 10; i++ )
			 Add(function(s) { });

		Run();

Halt(); //////////////////////////////

	var randomString = '';
	for ( var i = 0; i < 1024; ++i )
		randomString += Math.random().toString(36).substr(2);
   RandomString = function(length) { // [0-9A-Za-z]

        var data = '';
        while( data.length < length )
            data += randomString.substring( Math.random()*randomString.length, Math.random()*randomString.length );
        return data.substr(0, length);
   }


		var deflate = new Z(Z.DEFLATE);
		var inflate = new Z(Z.INFLATE);
		var source = RandomString(10000);
		var str = deflate(source, true);	
		var result = inflate(str, true);
		
		Print( result == source );

Halt(); //////////////////////////////


for ( var k = 0; k < 20; k++ ) {

	var myTask = new Task(function(){});
	for ( var i = 0; i < 1000; i++ )
		myTask.Request(StringRepeat('x', 1000));

	for ( var j = 0; j < 1000; j++ )
		myTask.Response();
}


Halt(); //////////////////////////////


	var inflate = new Z(Z.INFLATE);
	inflate();


for ( var i = 0; i < 500; i++ ) {

	Print( Expand("$(a)b$(c)d$(e)f$(g)g$(i)j$(k)l$(m)n$(o)p") );
//	Sleep(110);
}


Halt(); //////////////////////////////



var i = 0;
while ( !endSignal ) {
	
	i++;
	Expand("$(a)$()$()$()$()$()$()$()");

	i % 1000 || Print('.');
//	if ( i > 100000 )
//		break;
}
	
//	Print( (res +'-'+ ids), ' == ', 'ab01e2g3ij-cdfh', '\n' );
	
	
	
Halt(); //////////////////////////////




var b = new Blob('abcde');
for ( var i = 0; i < 2; i++ )
	b.constructor;

throw 0;



function TestCase( section, name, expected, result ) {}
var SECTION = '';

var x;

new TestCase( SECTION,     "x = new Blob(); x.charAt(NaN)",  "",     eval("x=new Blob();x.charAt(Number.NaN)") );
new TestCase( SECTION,     "x = new Blob(); x.charAt(Number.POSITIVE_INFINITY)",   "",     eval("x=new Blob();x.charAt(Number.POSITIVE_INFINITY)") );
new TestCase( SECTION,     "x = new Blob(); x.charAt(Number.NEGATIVE_INFINITY)",   "",     eval("x=new Blob();x.charAt(Number.NEGATIVE_INFINITY)") );

new TestCase( SECTION,     "x = new Blob(); x.charAt(0)",    "",     eval("x=new Blob();x.charAt(0)") );
new TestCase( SECTION,     "x = new Blob(); x.charAt(1)",    "",     eval("x=new Blob();x.charAt(1)") );
new TestCase( SECTION,     "x = new Blob(); x.charAt(-1)",   "",     eval("x=new Blob();x.charAt(-1)") );


new TestCase( SECTION,      "var MYOB = new Blob(1234567890); MYOB.charAt(0)",  "1",        eval("var MYOB = new Blob(1234567890); MYOB.charAt(0)") );
new TestCase( SECTION,      "var MYOB = new Blob(1234567890); MYOB.charAt(1)",  "2",        eval("var MYOB = new Blob(1234567890); MYOB.charAt(1)") );
new TestCase( SECTION,      "var MYOB = new Blob(1234567890); MYOB.charAt(2)",  "3",        eval("var MYOB = new Blob(1234567890); MYOB.charAt(2)") );
new TestCase( SECTION,      "var MYOB = new Blob(1234567890); MYOB.charAt(3)",  "4",        eval("var MYOB = new Blob(1234567890); MYOB.charAt(3)") );
new TestCase( SECTION,      "var MYOB = new Blob(1234567890); MYOB.charAt(4)",  "5",        eval("var MYOB = new Blob(1234567890); MYOB.charAt(4)") );
new TestCase( SECTION,      "var MYOB = new Blob(1234567890); MYOB.charAt(5)",  "6",        eval("var MYOB = new Blob(1234567890); MYOB.charAt(5)") );
new TestCase( SECTION,      "var MYOB = new Blob(1234567890); MYOB.charAt(6)",  "7",        eval("var MYOB = new Blob(1234567890); MYOB.charAt(6)") );
new TestCase( SECTION,      "var MYOB = new Blob(1234567890); MYOB.charAt(7)",  "8",        eval("var MYOB = new Blob(1234567890); MYOB.charAt(7)") );
new TestCase( SECTION,      "var MYOB = new Blob(1234567890); MYOB.charAt(8)",  "9",        eval("var MYOB = new Blob(1234567890); MYOB.charAt(8)") );
new TestCase( SECTION,      "var MYOB = new Blob(1234567890); MYOB.charAt(9)",  "0",        eval("var MYOB = new Blob(1234567890); MYOB.charAt(9)") );
new TestCase( SECTION,      "var MYOB = new Blob(1234567890); MYOB.charAt(10)",  "",       eval("var MYOB = new Blob(1234567890); MYOB.charAt(10)") );

new TestCase( SECTION,      "var MYOB = new Blob(1234567890); MYOB.charAt(Math.PI)",  "4",        eval("var MYOB = new Blob(1234567890); MYOB.charAt(Math.PI)") );

Print('OK');


Halt();
// very long GC:
var count = 1000000;
var v;

CollectGarbage();

var gc1 = gcBytes;

for ( i = 0; i < count; i++ ) {
}

var gc2 = gcBytes;


for ( i = 0; i < count; i++ ) {
	
	v = [1.1,2.2]
}


Print( (gcBytes - gc2 - (gc2-gc1))/count, '\n' );


Halt();


var f = new Function("\
	var buf = '';\
	_configuration.stderr = function(chunk) buf += chunk;\
	Warning('test');\
	delete _configuration.stderr;\
	if ( buf.indexOf('test') == -1 )\
		Print('error');\
	CollectGarbage();\
");


for ( var i=0; i<1000; i++ )
	f();


Halt();

//_configuration.stderr = function(){}

	var s = new Blob('this is a string object');
	s.substring(-Infinity, -Infinity);

//	eval("var s = new Blob('this is a string object'); s.substring(-Infinity, -Infinity)") );

		
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
