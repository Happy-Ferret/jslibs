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


loadModule('jsio');
loadModule('jsstd');
loadModule('jsz');
loadModule('jscrypt');
loadModule('jsdebug');

function isNotEmpty(obj) {

	for ( var tmp in obj )
		return true;
	return false;
}

function SimpleHTTPServer(port, bind, basicAuth) {

	var pendingRequestList = [], serverSocket = new Socket(), socketList = [serverSocket], deflate = new Z(Z.DEFLATE, Z.BEST_SPEED);

	function closeSocket(s) {

		s.close();
		socketList.splice(socketList.indexOf(s), 1);
	}

	function processRequest(s) {

		var buf = s.read();
		if ( buf == undefined )
			return closeSocket(s);
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
				var buf = s.read();
				if ( buf == undefined )
					return closeSocket(s);						
				s.data += buf;
				return undefined;
			}

			if ( basicAuth && authorization != base64Encode(basicAuth) ) {

				sleep(2000);
				s.write('HTTP/1.1 401 Authorization Required\r\nWWW-Authenticate: Basic realm="debugger"\r\nContent-length: 0\r\n\r\n');
				return undefined;
			}

			delete s.readable;
			pendingRequestList.push([s.data.substring(0, contentLength), function(response) {

				if ( s.connectionClosed )
					return closeSocket(s);
				if ( response == undefined )
					s.write('HTTP/1.1 204 No Content\r\n\r\n');
				else {
					var head = 'HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n';
					if ( response.length >= 1460 ) {

						response = deflate(response, true);
						head += 'Content-encoding: deflate\r\n';
					}
					s.write(head + 'Content-length: '+response.length+'\r\n\r\n' + response);
				}
				s.readable = processRequest;
				return true;
			}]);
			s.data = s.data.substring(contentLength);
			return undefined;
		})(s);
		return undefined;
	}

	serverSocket.readable = function() {

		var clientSocket = serverSocket.accept();
		socketList.push(clientSocket);
		clientSocket.data = '';
		clientSocket.readable = processRequest;
	}

	serverSocket.nonblocking = true;
	serverSocket.bind(port, bind);
	serverSocket.listen();

	this.hasPendingRequest = function() {

		poll(socketList, 0);
		return pendingRequestList.length > 0;
	}		

	this.getNextRequest = function() {

		while( !host.endSignal && pendingRequestList.length == 0 )
			poll(socketList, 100);
		return pendingRequestList.shift();
	}	
}

function RemoteMessageServer( port, ip ) {

	var _this = this;
	var server = new SimpleHTTPServer(port, ip);
	var pendingResponseFunction;
	
	this.poll = function() {

		if ( !server.hasPendingRequest() )
			return false;
		if ( pendingResponseFunction )
			pendingResponseFunction();
		var req;
		[req, pendingResponseFunction] = server.getNextRequest();
		_this.onMessage && _this.onMessage(req);
		return true;
	}
	
	this.send = function( message ) {
		
		var req;
		if ( !pendingResponseFunction )
			[req, pendingResponseFunction] = server.getNextRequest();
		pendingResponseFunction(message);
		pendingResponseFunction = undefined;
		if ( req )
			_this.onMessage && _this.onMessage(req);
	}
}


function RemoteCall( remoteMessage ) {

	var _this = this;
	this.__noSuchMethod__ = function() {

		remoteMessage.send(uneval(Array.slice(arguments)));
	}

	remoteMessage.onMessage = function( message ) {

		if ( !message )
			return;
		var call = eval('('+message+')');
		_this.api[call[0]].apply(_this.api, call[1]);
	}
}

var live = new function() {

//	var server = new SimpleHTTPServer(8008, '127.0.0.1');

	var initExpr = /\/\*init (?:([^]*?))?\*\//;
	var uiExpr = /\/\*ui (?:([^]*?))?\*\//;

	var initData;
	var userInterfaceCode;
	var codeLocation, codeFunction, lastValidCodeFunection;
	
	var watchList = [];
	var updatedVariables = {};

	
	this.Function = function() {

		if ( !codeFunction )
			return undefined;
		
		try {
			
			var res = codeFunction.apply(this, arguments);
			lastValidCodeFunection = codeFunction;
			return res;
		} catch(ex) {
			
			rc.reportError('Runtime error: '+ex+' (line '+(ex.lineNumber-codeLocation)+')');
			codeFunction = lastValidCodeFunection;
		}
		return undefined;
	}
	
	var api = {

		setCode: function(code) {

			initData = (initExpr(code)||[''])[1];
			
			var tmp = (uiExpr(code)||[''])[1];
			if ( tmp != userInterfaceCode ) {
				
				userInterfaceCode = tmp;
				rc.setUserInterface(userInterfaceCode);
			}
			
			var previousValidFunction = codeFunction;
			try {
				
				[,codeLocation] = locate();
				codeFunction = new Function(code);
			} catch(ex) {
				
				rc.reportError('Compilation error: '+ex+' (line '+(ex.lineNumber-codeLocation)+')');
				codeFunction = previousValidFunction;
			}
		},

		setVariables: function(variables) {

			for ( var name in variables ) {
			
				global[name] = variables[name];
				delete updatedVariables[name];
			}
		},
		
		watchVariables: function(variableList) {
			
			for each ( var name in watchList )
				global.unwatch(name);
			watchList = variableList;
			for each ( var name in watchList )
				global.watch(name, function(id, oldval, newval) {
					
					if ( newval !== oldval )
						updatedVariables[id] = true;
					return newval;
				});
			
			var state = {};
			for each ( var name in variableList )
				state[name] = global[name];
			rc.setVariables(state);
			updatedVariables = {};
		},
		
		init: function() {
			
			var tmp = eval('('+initData+')');
			for ( var name in tmp )
				if ( name in global )
					global[name] = tmp[name];
		}
	}
	
	var rms = new RemoteMessageServer(8007, '127.0.0.1');
	var rc = new RemoteCall(rms);
	rc.api = api;
	
	this.poll = function() {
		
		while ( rms.poll() );

		if ( isNotEmpty( updatedVariables ) ) {
			
			var state = {};
			for ( var name in updatedVariables )
				state[name] = global[name];
			rc.setVariables(state);
			updatedVariables = {};
		}
	}
}


// test part

while ( !host.endSignal ) {

// /*ui slider({min:10, max:20, step:1}); button({ name:'go'});  hr */
	live.poll();
	live.Function('test');
	sleep(10);
}

