LoadModule('jsstd');
LoadModule('jsio');

try {


Print('\n * testing stdout \n');

	var f = File.stdout;
	f.Write(new Date());

Print('\n * testing simple file write \n');

	var f = new File('test.txt');
	f.Open( File.RDWR | File.CREATE_FILE );
	f.Write(new Date());
	f.Close();

Print('\n * testing TCP socket \n');

	var dlist = []; //descriptor list

	var step = 0;

	var server = new Socket( Socket.TCP );
	server.nonblocking = true;
	server.Bind(80, '127.0.0.1');
	server.Listen();
	server.readable = function(s) {
		
		var soc = s.Accept();
		dlist.push(soc);
		soc.readable = function(s) {

			var data = s.Read();
			Print(data);
			if ( !data.length )
				delete s.readable;
		}
	}
	
	var client = new Socket( Socket.TCP );
	
	client.Connect('127.0.0.1', 80);
	
	dlist.push(client);
	dlist.push(server);
	
	var i = 0;
	while(++i < 15) {
		
		Print('.\n');
		Poll(dlist,100);
		step++;
		if ( !(step%5) ) {
		
			client.Write('test');
		}
	}
	

} catch ( ex if ex instanceof IoError ) {
	Print( 'IoError: ' + ex.text, '\n' );
} catch( ex ) {
	throw ex;
}
