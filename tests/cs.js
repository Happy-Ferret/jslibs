exec('deflib.js');
LoadModule('jsnspr');

var dlist = []; //descriptor list

var step = 0;

try {


// SERVER //

	var serverSocket = new Socket();
	serverSocket.exception = function(s) {
		
		print('SERVER: exception','\n');
	}
	
	serverSocket.readable = function(s) { // after Listen, readable mean incoming connexion

		print('SERVER-SOCKET: readable','\n');
		if ( step>100 ) {
			 var incomingClient = s.Accept(); print('  - Accept()','\n');
//		incomingClient.Send('123'); print('  - Send()','\n');
//		incomingClient.Close(); print('  - Close()','\n');
			dlist.push(incomingClient);
			incomingClient.readable = function(s) {
			
				print('SERVER-CONNECTION - readable:'+s.Recv(), '\n' );
			}
		}
		
	}

	serverSocket.Listen( 80, '127.0.0.1' );	print('SERVER-SOCKET','\n'); print('  - Listen()','\n');
	dlist.push(serverSocket);


// CLIENT //

	var clientSocket = new Socket();
	clientSocket.exception = function(s) {

		print('CLIENT: exception','\n');
	}
	
	clientSocket.readable = function(s) {

		print('CLIENT - readable','\n');
	}
	
	clientSocket.writable = function(s) {

		print('CLIENT: writable','\n');
		
		var datas = '<DATA...111001111011111110101110001011011111101011>';
		s.Send(datas); print('  - sending datas: '+datas,'\n');
//		delete s.writable;
	}

	clientSocket.Connect( 'localhost', 80 ); print('CLIENT','\n'); print('  - Connect()','\n');
	dlist.push(clientSocket);
	
//

	while(!endSignal) {
		
		print('.\n');
		Poll(dlist,100);
		Sleep(100); // to avoid my console being flood
		step++;
	}
	
} catch ( ex if ex instanceof NSPRError ) { 
	print( ex.text + ' ('+ex.code+')', '\n' );
}