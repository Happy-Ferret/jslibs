LoadModule('jsstd');
LoadModule('jsio');

Print('Type "exit" to exit', '\n');

global.__defineGetter__('quit', Halt);
global.__defineGetter__('exit', Halt);
global.__defineGetter__('bye', Halt);

for (;;) {
	
	Print('js> ');

	var code = '';
	do {
	
		code += File.stdin.Read();
	} while( !IsStatementValid( code ) );

	try {

		var res = eval( code );
	} catch(ex) {

		Print( ex, '\n' );
	}

	if ( res != undefined )
		Print( res, '\n' );
}