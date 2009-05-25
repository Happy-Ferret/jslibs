LoadModule('jsstd');
LoadModule('jsio');

try {

	var dlist = []; // descriptor list (sockets)


	// SERVER

	var serverSocket = new Socket(); // the rendezvous socket.
	serverSocket.nonblocking = true;
	dlist.push(serverSocket); // add the socket to the descriptor list.

	Print('SERVER - listening...\n');
	serverSocket.Bind(80, '127.0.0.1'); // listen port 80 on 127.0.0.1 interface only.
	serverSocket.Listen();

	serverSocket.readable = function(s) { // after Listen(), readable mean incoming connexion.

		Print('SERVER - accepting connexion\n');
		var incomingClient = s.Accept();
		dlist.push(incomingClient); // add the socket to the descriptor list.
		
		incomingClient.readable = function(s) { // this event is fired when data are available or the socket is closed.

			var data = s.Read();
			if ( data == undefined ) { // end of connection.
				
				Print('SERVER - client has disconnected\n' );
				dlist.splice(dlist.indexOf(s), 1); // remove the socket from the descriptor list.
				s.Close();
				return;
			}				

			Print('SERVER - receiving data: '+data+'\n' );
			if ( data == 'PING' ) {
				
				Print('SERVER - replying "PONG"\n' );
				s.Write('PONG');
			}
		}
	}


	// CLIENT

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
		Print('CLIENT - receiving data: '+data+'\n' );
		
		Print('CLIENT - closing the connection\n' );
		s.Close();
	}
	
	clientSocket.writable = function(s) { // this event is fired when the socket is ready to send data.
		
		Print('CLIENT - sending "PING" to the server.\n');
		s.Write('PING');
		delete s.writable; // stop
	}

	Print('CLIENT - connecting...\n');
	clientSocket.Connect( 'localhost', 80 );
	
	
	// MAIN
	
	Print( 'Press ctrl-c to exit.\n' );

	while( !endSignal ) { // ctrl-c
		
		Print('.\n');
		Poll(dlist, 500); // wait and dispatch network events.
	}
	
} catch ( ex if ex instanceof IoError ) {

	Print( ex.fileName+':'+ex.lineNumber+' '+ex.text + ' ('+ex.code+')', '\n' );
}
