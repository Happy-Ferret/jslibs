LoadModule('jsstd');


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
