LoadModule('jsstd');

Print( '\n * testing IdOf ...\n' );

var obj1 = {};
var obj2 = {};

Print( IdOf(obj1.constructor), '\n' );
Print( IdOf(obj1.constructor), '\n' );


Print( '\n * testing Buffer ...\n' );

var buf = new Buffer();
buf.Write('1');
buf.Write('2');
buf.Write('3');
buf.Write('4');

buf.Clear();

Print( buf.length, '\n' );

buf.Write('4');
buf.Write('45');

Print( buf.length, '\n' );

//Halt();
Print( '\n * testing SetScope ...\n' );

function foo() {

	var data = 55;

	function bar() {

		Print(data,'\n');
	}
	
	var old = SetScope(bar,{data:7});
	bar();
	var old = SetScope(bar,old);
	bar();
}

foo();

Print( '\n * testing HideProperties ...\n' );

var obj = { aaa:123, bbb:'xxx' };

for ( var p in obj )
  Print( p + ' = ' + obj[p], '\n' );

HideProperties( obj, 'aaa' );
  Print( '...\n' );

for ( var p in obj )
  Print( p + ' = ' + obj[p], '\n' );


Print( '\n * testingIdOf ...\n' );

Print( IdOf({}), '\n' );
Print( IdOf({}), '\n' );
	
Print( '\n * testing HideProperties ...\n' );

var obj = { a:1, b:2, c:3 }

HideProperties( obj, 'b' );

for ( var i in obj )
	Print( i +':'+obj[i],'\n');



Print( '\n * testing Clear ...\n' );

var toto = [ 1,3,5 ];
toto.i = 6;
Clear(toto);
Print( 'toto:'+toto.length, '\n' );


Print( '\n * testing Buffer ...\n' );


var toto = 'rrr';

var buf = new Buffer();
buf.onunderflow = function(buf) { Print( ' ? ' ); buf.Write(toto) }
buf.Write('1234');
buf.Write('5');
buf.Write('');
buf.Write('6789');

CollectGarbage();
var s = buf.Read(5);
s += 'X'
CollectGarbage();
buf.Unread(s);
s += 'Y'
CollectGarbage();

Print( buf.Read(100) );

buf.Write('1234');
buf.Write('5');
buf.Write('');
buf.Write('6789');

Print( buf.Read(undefined), '\n' );
Print( buf.Read(undefined), '\n' );
Print( buf.Read(undefined), '\n' );
Print( buf.Read(undefined), '\n' );
Print( buf.Read(undefined), '\n' );


/*
Print( '['+buf.length +']');
Print( buf.Read(3) );
Print( '['+buf.length +']');
Print( buf.Read(4) );
Print( '['+buf.length +']');
Print( buf.Read(1) );
Print( '['+buf.length +']');
Print( buf.Read(1) );
Print( '['+buf.length +']');
Print( buf.Read(10) );
Print( buf.Read(10) );
*/


/*
var t = <text>
this is
a multiline

text
</text>

Print(t);

function test() {

	var o = { title:'My HTML Page', titi:1234, toString:function() { return Expand( this.text, this ) } };
	o.text = '<html><title>$(title)</title>\n'
	
	
	Print( o )
}

test();


var obj = {};

Exec( 'test.js' );


Print( 'obj.a : ' + obj.a, '\n');
Print( 'obj.b : ' + obj.b, '\n');
Print( 'obj.c : ' + obj.c, '\n');
*/