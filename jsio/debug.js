LoadModule('jsstd');
LoadModule('jsio');

try {


/*
var f = new File('toto.txt');
f.Open( "w+" );
File.stdin.Read();

f.Close();

Halt();	
	

Print('\n * testing stdout \n');

	var f = File.stdout;
	f.Write(new Date());

Print('\n * testing simple file write \n');

	var f = new File('test.txt');
	f.Open( File.RDWR | File.CREATE_FILE );
	f.Write(new Date());
	f.Close();
*/

Print('\n * testing UDP socket \n');


	var data = '';
	for ( var i = 0; i < 8193; i++, data += 'x' );

	var step = 0;

	var s1 = new Socket( Socket.UDP );
	s1.Connect('127.0.0.1', 1);
	
	var s2 = new Socket( Socket.UDP );
//	s2.nonblocking = true;
	s2.Bind(1);
	s2.readable = function(s) {
	
		Print( s.Read().length );
	}
	var dlist = [s1,s2]; //descriptor list

	var i = 0;
	while(++i < 15) {
		
		Print('.\n');
		Poll(dlist,100);
		step++;
		if ( !(step % 5) ) {
		
			s1.Write(data);
		}
	}



} catch ( ex if ex instanceof IoError ) {
	Print( 'IoError: ' + ex.text, '\n' );
} catch( ex ) {
	throw ex;
}
