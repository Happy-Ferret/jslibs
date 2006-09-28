LoadModule('jsstd');
LoadModule('jsnspr');

var file = new File('file_test.txt');

if ( file.exist ) {
	file.Open( File.RDONLY );
	print( 'file content:\n'+file.Read() );
	file.Close();
}

try {

file.Open( File.CREATE_FILE + File.RDWR + File.APPEND );
file.Write('this is a test\n');
print( 'current file pos: '+file.Seek() );
//file.Write('x');
file.Close();

} catch ( ex if ex instanceof NSPRError ) { print( ex.text ) }
