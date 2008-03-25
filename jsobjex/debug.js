LoadModule('jsstd');
LoadModule('jsobjex');

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
