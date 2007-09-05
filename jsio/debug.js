LoadModule('jsstd');
LoadModule('jsio');



try {

Print( Socket.GetHostsByName('www.google.com') );


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


Print('\n * testing TCP socket \n');

	var dlist = []; //descriptor list

	var step = 0;

	var server = new Socket( Socket.TCP );
	server.Bind(80, '127.0.0.1');
	server.Listen();
	
	server.readable = function(s) {
		
		var soc = s.Accept();
		dlist.push(soc);
		soc.readable = function(s) {

			var data = s.Read();
			
			Print('recv '+data+' from '+s.sockPort);
			if ( !data.length )
				delete s.readable;
		}
	}
	
	var client = new Socket( Socket.TCP );
	client.Connect('127.0.0.1', 80);
	
	Print('seg: '+client.maxSegment);
	
	dlist.push(client);
	dlist.push(server);
	
	var i = 0;
	while(++i < 10) {
		
		Print('.\n');
		Poll(dlist,100);
		step++;
		if ( !(step%5) ) {
		
			client.Write('test');
		}
	}
	


Print('\n * testing UDP socket \n');

	var data = '1234';
//	for ( var i = 0; i < 8192; i++, data += 'x' );

	var step = 0;

	var s1 = new Socket( Socket.UDP );
	s1.reuseAddr = true;
	s1.Connect('127.0.0.1', 9999);
	
	
	var s2 = new Socket( Socket.UDP );
//	s2.nonblocking = true;
	s2.Bind(9999);
	s2.readable = function(s) {
	
		Print( 'port:'+s.sockPort+'\n' );
		Print( s.Read().length );
	}
	var dlist = [s1,s2]; //descriptor list

	var i = 0;
	while(++i < 10) {
		
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
