LoadModule('jsnspr');
LoadModule('jsstd');
LoadModule('jsfastcgi');

while (Accept() >= 0) {

	try {
		var res = Exec( GetParam('SCRIPT_FILENAME') );
	} catch(ex) {
		Log( ex.line );
	}
}
