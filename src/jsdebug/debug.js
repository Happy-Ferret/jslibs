LoadModule('jsstd');
LoadModule('jsdebug');

var dbg = new Debugger();

/*
dbg.onInterrupt = function( scriptname, line, scope ) {

	Print( 'onInterrupt - name: '+scriptname + ' line: '+line, ' i='+i, '\n' )
	return 3;
}
*/

dbg.onNewScript = function(fileName, line, length, depth, fname, script) {

	Print( 'onNewScript - fileName: '+fileName + ' line: '+line + ' length: '+length + ' depth: '+depth + ' fname: '+fname )
//	Print( ' script: '+script )
	Print( '\n' );
		
}

Exec('test.js');

Print('\n');


var i = 0;

i++;
i++;
i++;

function inner() {
	
	i += 100;

	i++;
}

i++;

debugger;

	inner(); i++;

for ( var j=0; j<10; j++ ) {
	i++;
}

i++;
i++;
i++;
