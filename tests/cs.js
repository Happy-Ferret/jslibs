Exec('deflib.js');
LoadModule('jsnspr');

var dlist = []; //descriptor list

var step = 0;

try {


// SERVER //

	var serverSocket = new Socket();
	serverSocket.exception = function(s) {
		
		Print('SERVER: exception','\n');
	}
	
	serverSocket.readable = function(s) { // after Listen, readable mean incoming connexion

		Print('SERVER-SOCKET: readable','\n');
		if ( step>20 ) {
			 var incomingClient = s.Accept(); Print('  - Accept()','\n');
//		incomingClient.Send('123'); Print('  - Send()','\n');
//		incomingClient.Close(); Print('  - Close()','\n');
			dlist.push(incomingClient);
			incomingClient.readable = function(s) {
			
				Print('SERVER-CONNECTION - readable:'+s.Recv(), '\n' );
			}
		}
		
	}

	serverSocket.Listen( 80, '127.0.0.1' );	Print('SERVER-SOCKET','\n'); Print('  - Listen()','\n');
	dlist.push(serverSocket);


// CLIENT //

	var clientSocket = new Socket();
	clientSocket.exception = function(s) {

		Print('CLIENT: exception','\n');
	}
	
	clientSocket.readable = function(s) {

		Print('CLIENT - readable','\n');
	}
	
	clientSocket.writable = function(s) {

		Print('CLIENT: writable','\n');
		
		var datas = '<DATA...111001111011111110101110001011011111101011>';
		s.Send(datas); Print('  - sending datas: '+datas,'\n');
//		delete s.writable;
	}

	clientSocket.Connect( 'localhost', 80 ); Print('CLIENT','\n'); Print('  - Connect()','\n');
	dlist.push(clientSocket);
	
//

	while(!endSignal) {
		
		Print('.\n');
		Poll(dlist,100);
		Sleep(100); // to avoid my console being flood
		step++;
	}
	
} catch ( ex if ex instanceof NSPRError ) { 
	Print( ex.text + ' ('+ex.code+')', '\n' );
} catch (ex) {
	throw(ex);
}