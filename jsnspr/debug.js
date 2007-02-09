LoadModule('jsstd');
LoadModule('jsnspr');

try {

	var f = new File('test.txt');
	f.Open( File.RDWR | File.CREATE_FILE  );
	f.Write('test');
	f.Close();

	f.name = 'toto.txt'
	f.Delete();

	File.stdout.Write('stdout test !\n');

	Print( 'NSPR version = ' + NSPRVersion()  ,'\n' );
	
	
	Print( 'PATH = ' + GetEnv('PATH')  ,'\n' );
	Print( 'asdfasdfas = ' + GetEnv('asdfasdfas')  ,'\n' );
	Print( 'Host name = ' + HostName() ,'\n' );
	
	Print( 'has xxx directory: ' + new Directory('xxx').exist, '\n' );
	Print( 'has .svn directory: ' + new Directory('.svn').exist, '\n' );


} catch ( ex if ex instanceof NSPRError ) {
	Print( 'NSPRError: ' + ex.text, '\n' );
} catch( ex ) {
	throw ex;
}
