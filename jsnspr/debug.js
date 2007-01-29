LoadModule('jsstd');
LoadModule('jsnspr');

try {

Print ( '-----' +File.stdin.Read(5));
Print ( '-----' +File.stdin.Read(5));


} catch ( ex if ex instanceof NSPRError ) {

	Print( 'NSPRError: ' + ex.text, '\n' );
} catch( ex ) {

	throw ex;
}