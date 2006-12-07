LoadModule('jsstd');


var t = <text>
this is
a multiline

text
</text>

Print(t);

function test() {

	var o = { title:'My HTML Page', titi:1234, Expand:Expand };
	
	
	Print( o.Expand('<html><title>$(title)</title>\n' ) )
}

test();