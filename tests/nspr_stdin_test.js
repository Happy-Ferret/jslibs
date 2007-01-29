LoadModule('jsstd');
LoadModule('jsnspr');

try {
	
	var chunk;
	while ( chunk = File.stdin.Read(5) )
			Print ( chunk );
	

} catch ( ex if ex instanceof NSPRError ) {

	Print( 'NSPRError: ' + ex.text, '\n' );
} catch( ex ) {

	throw ex;
}