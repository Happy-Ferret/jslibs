
LoadModule('jsstd');
LoadModule('jsdebug');

function toto( o ) {

	o.i++;
	
}


Trap( 8, 'Print(Locate(0))');


toto({i:1});
