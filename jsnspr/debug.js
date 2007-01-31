LoadModule('jsstd');
LoadModule('jsnspr');

try {

	var f = new File('test.txt');
	f.Open( File.RDWR | File.CREATE_FILE  );
	f.Write('test');
	f.Close();

	File.stdout.Write('stdout test !\n');
	
	
	Print( 'PATH = ' + GetEnv('PATH')  ,'\n' );
	Print( 'asdfasdfas = ' + GetEnv('asdfasdfas')  ,'\n' );
	Print( 'Host name = ' + HostName() ,'\n' );


} catch ( ex if ex instanceof NSPRError ) {
	Print( 'NSPRError: ' + ex.text, '\n' );
} catch( ex ) {
	throw ex;
}
