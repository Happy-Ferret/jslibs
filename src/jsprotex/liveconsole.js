/* ***** BEGIN LICENSE BLOCK *****
 * Version: GNU GPL 2.0
 *
 * The contents of this file are subject to the
 * GNU General Public License Version 2.0; you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at
 * http://www.gnu.org/licenses/gpl.html
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 * ***** END LICENSE BLOCK ***** */


LoadModule('jsio');
LoadModule('jsstd');
LoadModule('jsz');
LoadModule('jscrypt');

function SimpleHTTPServer(port, bind, basicAuth) {

	var pendingRequestList = [], serverSocket = new Socket(), socketList = [serverSocket], deflate = new Z(Z.DEFLATE, Z.BEST_SPEED);

	function CloseSocket(s) {

		s.Close();
		socketList.splice(socketList.indexOf(s), 1);
	}

	function ProcessRequest(s) {

		var buf = s.Read();
		if ( buf == undefined )
			return CloseSocket(s);
		s.data += buf;
		var eoh = s.data.indexOf('\r\n\r\n');
		if ( eoh == -1 )
			return undefined;
		var headers = s.data.substring(0, eoh);
		var contentLength = (/^content-length: ?(.*)$/im(headers)||[])[1];
		var authorization = (/^authorization: ?Basic (.*)$/im(headers)||[])[1];

		s.data = s.data.substring(eoh+4);
		(function(s) {

			if ( s.data.length < contentLength ) {

				s.readable = arguments.callee;
				var buf = s.Read();
				if ( buf == undefined )
					return CloseSocket(s);						
				s.data += buf;
				return undefined;
			}

			if ( basicAuth && authorization != Base64Encode(basicAuth) ) {

				Sleep(2000);
				s.Write('HTTP/1.1 401 Authorization Required\r\nWWW-Authenticate: Basic realm="debugger"\r\nContent-Length: 0\r\n\r\n');
				return undefined;
			}

			delete s.readable;
			pendingRequestList.push([s.data.substring(0, contentLength), function(response) {

				if ( s.connectionClosed )
					return CloseSocket(s);
				if ( response == undefined )
					s.Write('HTTP/1.1 204 No Content\r\n\r\n');
				else {
					var head = 'HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n';
					if ( response.length >= 1460 ) {

						response = deflate(response, true);
						head += 'Content-Encoding: deflate\r\n';
					}
					s.Write(head + 'Content-Length: '+response.length+'\r\n\r\n' + response);
				}
				s.readable = ProcessRequest;
				return true;
			}]);
			s.data = s.data.substring(contentLength);
			return undefined;
		})(s);
		return undefined;
	}

	serverSocket.readable = function() {

		var clientSocket = serverSocket.Accept();
		socketList.push(clientSocket);
		clientSocket.data = '';
		clientSocket.readable = ProcessRequest;
	}

	serverSocket.nonblocking = true;
	serverSocket.Bind(port, bind);
	serverSocket.Listen();
	this.HasPendingRequest = function() {

		Poll(socketList, 0);
		return pendingRequestList.length > 0;
	}		

	this.GetNextRequest = function() {

		while( !endSignal && pendingRequestList.length == 0 )
			Poll(socketList, 100);
		return pendingRequestList.shift();
	}	
}


function RemoteMessageServer( post, ip ) {

	var _this = this;
	var server = new SimpleHTTPServer(8007, '127.0.0.1');
	var pendingResponseFunction;
	
	this.Poll = function() {

		if ( server.HasPendingRequest() ) {
		
			if ( pendingResponseFunction )
				pendingResponseFunction();
			var req;
			[req, pendingResponseFunction] = server.GetNextRequest();
Print('receive '+req, '\n');

			_this.onMessage && _this.onMessage(req);
			return true;
		}
		return false;
	}
	
	this.Send = function( message ) {
		
		if ( !pendingResponseFunction ) {
			
			var req;
			[req, pendingResponseFunction] = server.GetNextRequest();
			_this.onMessage && _this.onMessage(req);
		}
Print('send '+message, '\n');
		pendingResponseFunction(message); // liveconsole.js:146: TypeError: pendingResponseFunction is not a function
		pendingResponseFunction = undefined;
	}
}


function RemoteCall( remoteMessage ) {

	var _this = this;
	this.__noSuchMethod__ = function() {

		remoteMessage.Send(uneval(Array.slice(arguments)));
	}

	remoteMessage.onMessage = function( message ) {

		if ( !message )
			return;
		var call = eval('('+message+')');
		_this.api[call[0]].apply(_this.api, call[1]);
	}
}



var live = new function() {

	var server = new SimpleHTTPServer(8008, '127.0.0.1');

	var uiExpr = /\/\*\*ui(?:([^]*?))?\*\*\//g;

	var codeFunction = function(){};

	this.LiveCode = function() {

		try {
			
			codeFunction();
		} catch(ex) {
			
			rc.ReportError(ex);
		}
	}

	var api = {

		SetCode: function(code) {

			var userInterfaceCode = (uiExpr(code)|[])[1];
			rc.SetUserInterface(userInterfaceCode);
			try {
				
				codeFunction = new Function(code);
			} catch(ex) {
				
				rc.ReportError(ex);
			}
		},

		UpdateVariables: function(variables) {

			for ( var name in variables )
				global[name] = variables[name];
		}
	}
	
	var rms = new RemoteMessageServer(8008, '127.0.0.1');
	var rc = new RemoteCall(rms);
	rc.api = api;
	
	this.Poll = function() {
		
		rms.Poll();
	}

}

while ( !endSignal ) {
	
	
	live.Poll();
	Sleep(500);
}


/*
try {
	
} catch(ex) { Print(ex.fileName+':'+ex.lineNumber+' '+ex+' ', '\n') }
*/
