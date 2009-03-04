LoadModule('jsio');
LoadModule('jsstd');
LoadModule('jsdebug');

var dbg = new Debugger();

dbg.onBreak = function( scriptname, line, scope, orig, frame ) {

	//Print( 'onBreak(' + Array.slice(arguments)+') ' );
		
	Print( '@ '+scriptname+':'+line );
	Print( ' orig='+orig );
	Print( ' lineno='+dbg.Stack(frame).lineno );
	Print( ' '+dbg.Stack(frame).this );
	
	Print( '\n' )
	return Debugger.CONTINUE;
}

Exec('debug1.js');

dbg.ToggleBreakpoint( true, 'debug1.js', 14 );

toto();
toto.call('myThis');

xx();

Print('\n');
