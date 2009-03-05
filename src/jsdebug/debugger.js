LoadModule('jsio');
LoadModule('jsdebug');

function HTTPServer(port) {
	
	var self = this;
	var pendingRequestList = [];
	var socketList = [];
	var serverSocket = new Socket();
	socketList.push(serverSocket);
	serverSocket.readable = function() {

		var clientSocket = serverSocket.Accept();
		socketList.push(clientSocket);
		clientSocket.data = '';
		clientSocket.readable = function(s) {
			
 			var buf = s.Read();
			if ( !buf ) {
			
				s.Close();
				socketList.splice( socketList.indexOf(s), 1 );
				return;
			}
			s.data += buf;
			if ( s.data.indexOf('\r\n\r\n') == -1 )
				return;
			
			var request = /^GET \/(?:\?(.*?)|()) HTTP/(s.data)[1];
			
			var responseFunction = function(response) {
				
				s.Write('HTTP/1.0 200 OK\r\nconnection: close\r\n\r\n'+response);
				s.linger = 500;
				s.Shutdown();
				s.Close();
				socketList.splice( socketList.indexOf(s), 1 );
			}
			pendingRequestList.push([request, responseFunction]);
		}
	}

	serverSocket.Bind(port);
	serverSocket.Listen();

	this.GetNextRequest = function() {

		while( pendingRequestList.length == 0 )
			Poll(socketList, 100);
		return pendingRequestList.shift();
	}	
}

var server = new HTTPServer(8009);

var dbg = new Debugger();

function Match(v) Array.indexOf(arguments,v,1)-1;
function Switch(i) arguments[++i];

function OriginToString( breakOrigin ) {

	with (Debugger) {
		var pos = Match(breakOrigin, FROM_BREAKPOINT, FROM_STEP, FROM_THROW, FROM_ERROR, FROM_DEBUGGER);
		return Switch(pos, 'breakpoint', 'step', 'throw', 'error', 'debugger');
	}
}

dbg.onBreak = function( filename, line, scope, breakOrigin, frameLevel ) {
	
//	Print( 'break at '+filename+':'+line+' because '+breakOrigin , '\n');

	var action;
	while ( !action ) {
	
		var response;
		Print( 'wait...\n');
		var [request, responseFunction] = server.GetNextRequest();
		Print( 'has request \n');
		
//		Print( 'request: '+request , '\n');
		
		var [, key, val] = /([^=]+)=?(.*)/(request);
		switch (key) {
			case 'state':
				response = ({ filename:filename, line:line, breakOrigin:OriginToString(breakOrigin) }).toSource();
				break;
			case 'getSource':
				response = new File(val).content;
				break;
			case 'getScriptList':
				response = dbg.scriptList.toSource();
				break;
			case 'action':
				switch (val) {
					case 'continue':
						action = Debugger.CONTINUE;
						break;
					case 'step':
						action = Debugger.STEP;
						break;
					case 'stepover':
						action = Debugger.STEP_OVER;
						break;
					case 'stepout':
						action = Debugger.STEP_OUT;
						break;
				}
				break;
		}
		responseFunction(response);
	}
	return action;
}
