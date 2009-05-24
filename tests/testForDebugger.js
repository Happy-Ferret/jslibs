LoadModule('jsstd');

function Trace( msg ) {

	_configuration.stderr(msg, '\n');
}


function foo( obj ) {

	Trace('foo');
	
	var fooVar = obj.value;

	function fooInnert( count ) {
	
		Trace('fooInnert');

		Print('fooVar=', fooVar + count, '\n')
	}

	return fooInnert;
}


var ifoo = foo( { name:'test', value:123 } );

for ( var i = 0; i < 5; i++ ) {

	ifoo(i);
}


Print('Done.\n');
