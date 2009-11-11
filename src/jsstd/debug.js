LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jsdebug');


var o = new ObjEx();

Print( o instanceof ObjEx );


Halt(); //////////////////////////////


var ids = '';
var i = 0;
var res = Expand('ab$(c)$(d)e$(f)g$(h)ij', function(id) { ids+=id; return i++ } );
Print( (res +'-'+ ids), ' == ', 'ab01e2g3ij-cdfh', '\n' );
Halt(); //////////////////////////////


var s = new Buffer();
for ( var i = 0; i < 2; i++ )
	s.constructor;


Halt(); //////////////////////////////



	var o = {
		Read: function(size) { return undefined }
	}

	var b = new Buffer(o);
	Print( uneval( b.Read(10) ) );
	
		
Halt(); //////////////////////////////



function setCallback() {
	
  Print( Array.slice(arguments).toSource(), '\n' );
}

var obj = new ObjEx( undefined, undefined, undefined, setCallback );

obj.foo = 123;
obj.foo = 456;
obj.foo = 789;

ObjEx.Aux(obj, {});


Halt();

function addCallback( name, value ) {
	
  Print('adding ' + name + ' = ' + value, '\n');
}

var obj = new ObjEx( addCallback, undefined, undefined, undefined, null );

obj.foo = 123;
obj.foo = 456;


/*
function Error(text) {
	throw(text);
}
var api = new ObjEx( undefined,undefined,undefined, function(name, value) this[name] ? Error('API Already defined') : value );

api.toto = function() Print('ok\n');
api.toto();
api.toto = function() Print('ok2\n');
*/

function dump() {
	Print( Array.slice(arguments).toSource(), '\n' )
}

var data = new ObjEx( dump, dump, dump, dump, null );

data.aaa = 111;
data.bbb = 222;

for ( var k in data ) {
  Print( k, '\n' )
}

//Print( data.constructor );






Halt(); /////////////////////////////////////////////////////////////////////


  var obj = { a:11, b:22, c:33 };
  for ( var p in obj )
   Print(p, ', '); // prints: a, b, c
  
  SetPropertyEnumerate(obj, 'b', false);
  for ( var p in obj )
   Print(p, ', '); // prints: a, c

  SetPropertyEnumerate(obj, 'b', true);
  for ( var p in obj )
   Print(p, ', '); // prints: a, c


Halt(); /////////////////////////////////////////////////////////////////////

var root = [];

for ( var i = 0; i<500; i++ ) {
	
	var obj = {};
	ObjectToId(obj);
	if ( Math.random() > 0.5 )
		root.push(obj);
}
		
CollectGarbage();

var root = [];

for ( var i = 0; i<500; i++ ) {
	
	var obj = {};
	ObjectToId(obj);
	if ( Math.random() > 0.5 )
		root.push(obj);
}

CollectGarbage();

for ( var i = 0; i < 5000; i++ )
	Print( IdToObject(i) == undefined ? '.' : 'o' );





Halt(); /////////////////////////////////////////////////////////////////////

function test2() {

	var a = { a:'a'};
	var b = { b:'b'};
	var c = { c:'c'};

	Print( ObjectToId(a), '\n' );
	Print( ObjectToId(b), '\n' );
	Print( ObjectToId(c), '\n' );
	Print( ObjectToId(c), '\n' );
	Print( ObjectToId(b), '\n' );
	Print( ObjectToId(a), '\n' );

}

CollectGarbage();

test2();

Print( IdToObject(1), '\n' );
Print( IdToObject(2), '\n' );
Print( IdToObject(3), '\n' );
Print( IdToObject(4), '\n' );

CollectGarbage();
Print( '\n' );

Print( IdToObject(1), '\n' );
Print( IdToObject(2), '\n' );
Print( IdToObject(3), '\n' );
Print( IdToObject(4), '\n' );

test2();

Print( IdToObject(4), '\n' );


Halt(); /////////////////////////////////////////////////////////////////////



Print('test');

var m = {__proto__:null};
m.__parent__ = 123;
m.foo = 456;

delete m.__parent__;
delete m.foo;

Print(m.__parent__); //123
Print(m.foo); // undefiend


Print('\n\n');

//Halt(); /////////////////////////////////////////////////////////////////////

var f = new Function("	Print( SandboxEval('Math') == Math )")

f()


//Halt(); /////////////////////////////////////////////////////////////////////

var xdrData = XdrEncode(Blob("testxxx"));

var val = XdrDecode(xdrData);

Print( val.length );


//Halt(); /////////////////////////////////////////////////////////////////////

var m = new Map([1,2,3,4]);


Print( [k+'='+v for ([k,v] in Iterator(m))] );

Halt(); //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


function mySecurityCheck() true;

function MySecureFile(name) {

	if ( !mySecurityCheck(name) )
		throw 'Invalid access'; 

	var f = new File(name);
	
	this.Open = function() {
		
		return f.Open.apply(f, arguments);
	}
	
	this.Read = function() {
	
		return f.Read.apply(f, arguments);
	}
}


var globalCx = { File:MySecureFile };
SetScope(globalCx, null); // very important because globalCx object is created in this global scope, and inherit from it !
var res = Sandbox.Eval('var f = new File("debug.js"); f.Open("r"); f.Read(50);', globalCx );

Print( res+'...\n' );



Halt();

var obj1 = { name:"Object 1", a:1,  b:2, c:3, d:4, e:5, f:6, g:7 };

function test() {

	var obj2 = new String();

	var x = obj1;

	Test( obj1, obj2 );

	Print( 'swap done.', "\n" );

//	Print( 'obj1.name=', obj1.name, "\n" );
//	Print( 'obj2.name=', obj2.name, "\n" );

}

var toto = function(){}
toto.prototype = obj1;
var t = new toto;

CollectGarbage();

test()

Print( 'obj1.length=', obj1.length, "\n" );


Halt();


var b = new Buffer();
b.Write('123');

b.toString();
Print( b.toString() )

Halt();


var b = new Buffer();
b.Write('');
b.Write('123');
b.Write('');
b.Write('456');
b.Write('');
b.Write('789');
b.Write('');

Print(   b[-1]+b[0]+b[1]+b[2]+b[3]+b[4]+b[5]+b[6]+b[7]+b[8]+b[9]+b[10]  , '\n' );


Halt();


var b1 = new Buffer();
b1.Write('1');
b1.Write('');
b1.Write('1');
b1.Write('1');

var b2 = new Buffer();
b2.Write('aaa');
b2.Write(b1);
b2.Write('bbb');
b2.Write(b1);

Print( b2);


Halt();

 function ReadFromFile() {

  Print('*** read from the file\n');
  return StringRepeat('x',5);
 }
 
 var buf = new Buffer({ Read:function() { buf.Write(ReadFromFile()); }})

 for ( var i=0; i<15; i++ )
  Print( buf.Read(1), '\n' )





Halt()


		var buf2 = new Buffer({
			Read: function(count) { return 'xcv' }
		});

		Print( buf2.length, '\n' );
		Print( buf2.Read(100), '\n' );
		Print( buf2.length, '\n' );



Halt();
		var stream = new Stream('456');
		
		Print( '_NI_StreamRead:', stream._NI_StreamRead, '\n' )
		

		var buf1 = new Buffer('123');
		buf1.source = stream;
		
		
		Print( buf1.Read(6) )



Halt()

/*
var p = new Pack(new Buffer());
p.buffer.source = new Buffer();
p.buffer.source.source = Stream('\x12\x34');
Print( (p.ReadInt(2, false, true)).toString(16) );

*/

		var toto = 'Zz';

		var buf = new Buffer();
//		buf.onunderflow = function(buf) { buf.Write(toto) }
		buf.source = { Read:function(count) { return toto } };
		buf.Write('1234');
		buf.Write('5');
		buf.Write('');
		buf.Write('6789');


		var s = buf.Read(5);
		s += 'X'

		buf.Unread(s);
		s += 'Y'


Print(	buf.Read(30) )

		buf.Write('1234');
		buf.Write('5');
		buf.Write('');
		buf.Write('6789');
