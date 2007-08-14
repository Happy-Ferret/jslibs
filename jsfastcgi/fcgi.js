LoadModule('jsnspr');
LoadModule('jsstd');
LoadModule('jsfastcgi');

while ( Accept() >= 0 ) {
	
	try {
	
		Exec(GetParam('SCRIPT_FILENAME'));
	} catch(ex) {
	
		var errorMessage = ex.name + ': ' + ex.message + ' (' + ex.fileName + ':' + ex.lineNumber + ')';
		Log( errorMessage );
		Write( 'Status: 500\r\n\r\n' + errorMessage );
	}
}
