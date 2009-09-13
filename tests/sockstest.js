LoadModule('jsstd');
LoadModule('jsio');


// http://tools.ietf.org/html/rfc1928
// http://en.wikipedia.org/wiki/SOCKS
function SocksString(ip, port) {
	
	var p = new Pack(new Buffer(), true);
	p.WriteInt(4,1);
	p.WriteInt(1,1);
	p.WriteInt(80,2);
	var ip = ip.split('.');
	p.WriteInt(Number(ip[0]), 1);
	p.WriteInt(Number(ip[1]), 1);
	p.WriteInt(Number(ip[2]), 1);
	p.WriteInt(Number(ip[3]), 1);
	p.buffer.Write("test user ID\0");
	return p.buffer.Read();
}

function ParseSocksString(stream) {


}



try {

	var dlist = []; // descriptor list (sockets)

	var clientSocket = new Socket();
	dlist.push(clientSocket); // add the socket to the descriptor list

	clientSocket.readable = function(s) { // this event is fired when data are available or the socket is closed.

		var data = s.Read();
		if ( data == undefined ) { // end of connection.

			Print('CLIENT - server has disconnected\n' );
			dlist.splice(dlist.indexOf(s), 1); // remove the socket from the descriptor list.
			s.Close();
			return;
		}				
		Print('CLIENT - receiving data: '+uneval(data)+'\n' );
		
		Print('CLIENT - closing the connection\n' );
//		s.Close();
	}
	
	clientSocket.writable = function(s) { // this event is fired when the socket is ready to send data.
		
		
		s.Write(SocksString('209.85.229.103', 80));
		s.Write('GET\r\n\r\n');
		
		delete s.writable; // stop
	}

	Print('CLIENT - connecting...\n');
	clientSocket.Connect( 'localhost', 3128 );
	
	
	// EVENT LOOP
	

	while( !endSignal ) { // ctrl-c
		
		Print('.\n');
		Poll(dlist, 500); // wait and dispatch network events.
	}



} catch ( ex if ex instanceof IoError ) {

	Print( ex.fileName+':'+ex.lineNumber+' '+ex.text + ' ('+ex.code+')', '\n' );
}
