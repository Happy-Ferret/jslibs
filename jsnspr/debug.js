LoadModule('jsstd');
LoadModule('jsnspr');

try {
/*
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
*/


var end = false;
var soc = new Socket();
soc.Connect( 'www.google.com', 80 );
Print('Connecting...');

soc.writable = function(s) {
	
	Print('accepted`\n');
	delete soc.writable;
	s.Send('GET / HTTP/1.0\r\n\r\n');
}

soc.readable = function(s) {

	var data = s.Recv();
	
	if ( data.length == 0 ) {
		end = true;
		delete soc.readable;
	} else
		Print( data );
}


while(!endSignal && !end )
	Poll([soc],1000);


} catch ( ex if ex instanceof NSPRError ) {
	Print( 'NSPRError: ' + ex.text, '\n' );
} catch( ex ) {
	throw ex;
}
