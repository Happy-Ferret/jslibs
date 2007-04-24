LoadModule('jsstd');
LoadModule('jsobjex');

function addCallback( name, value ) {
	
  Print('adding ' + name + ' = ' + value, '\n');
}

var obj = new ObjEx( addCallback, undefined, undefined, undefined, null );

obj.foo = 123;
obj.foo = 456;