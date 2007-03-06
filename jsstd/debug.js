LoadModule('jsstd');

var toto = 'rrr';

var buf = new Buffer();
buf.onunderflow = function(buf) { Print( ' ? ' ); buf.Write(toto) }
buf.Write('1234');
buf.Write('5');
buf.Write('');
buf.Write('6789');

Print( buf.Read(100) );

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