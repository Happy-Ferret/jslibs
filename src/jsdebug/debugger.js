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
LoadModule('jsdebug');

function Match(v) Array.indexOf(arguments,v,1)-1;
function Switch(i) arguments[++i];

function OriginToString( breakOrigin ) {

	with (Debugger) {
		var pos = Match(breakOrigin, FROM_BREAKPOINT, FROM_STEP, FROM_THROW, FROM_ERROR, FROM_DEBUGGER);
		return Switch(pos, 'breakpoint', 'step', 'throw', 'error', 'debugger');
	}
}

function SimpleHTTPServer(port, bind) {
	
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
			pendingRequestList.push([decodeURIComponent(/^GET \/\?(?:(.*?)|()) HTTP/(s.data)[1]), function(response) {
				
				if ( s.connectionClosed )
					return false;
				s.Write('HTTP/1.0 200 OK\r\ncontent-type: text/plain\r\nconnection: close\r\n\r\n'+response);
				s.linger = 2000;
				s.Shutdown();
				s.Close();
				socketList.splice( socketList.indexOf(s), 1 );
				return true;
			}]);
		}
	}

	serverSocket.Bind(port, bind);
	serverSocket.Listen();

	this.GetNextRequest = function() {

		while( pendingRequestList.length == 0 )
			Poll(socketList, 100);
		return pendingRequestList.shift();
	}	
}

var server = new SimpleHTTPServer(8009, '127.0.0.1');

/*
function Dump( data, tab ) {

    tab = tab||'';
    if ( data === null ) return 'null';
    if ( data === undefined ) return 'undefined';
    if ( typeof(data) == 'string' || data instanceof String || data instanceof Blob ) return '"' + data + '"';
    if ( typeof(data) == 'number' || data instanceof Number ) return data;
    if ( data instanceof Function ) return data.toSource().substr(1,50) + '...';
    if ( data instanceof Date ) return data;
    if ( data instanceof XML ) return data.toXMLString();
    if ( data instanceof Object && data.__iterator__ ) return data;
    if ( data instanceof Object ) {
   
       var name = data.constructor != Object && data.constructor != Array ? '|'+(data.constructor.name||'?')+'|' : '';
       var newTab = tab+'  '
       var propList = '';
        for ( var p in data )
            propList += newTab+p+':'+arguments.callee( data[p], newTab )+'\n';
      
       var isArray = data instanceof Array;
       return name + (isArray? '[' : '{') + (propList? '\n'+propList+tab : '') + (isArray? ']' : '}') + '\n';
    }
    return data;
}
*/

var dbg = new Debugger();
var breakpointList = {};

dbg.onBreak = function( filename, line, scope, breakOrigin, frameLevel, hasException, exception ) {

	var action;
	do {

		var response = '';
		var [req, responseFunction] = server.GetNextRequest();
		req = eval(req);

		switch (req[0]) {
			case 'state':
				response = { filename:filename, line:line, breakOrigin:OriginToString(breakOrigin), hasException:hasException, exception:exception };
				break;
			case 'eval':
				try {

					function tmp(code) eval(code);
					var prevScope = SetScope(tmp, scope);
					response = tmp(req[1]);
					SetScope(tmp, prevScope);
				} catch (ex) {
				
					response = ex;
				}				
				break;
			case 'getSource':
				response = new File(req[1]).content;
				break;
			case 'getScriptList':
				response = dbg.scriptList;
				break;
			case 'breakpoint':
				response = dbg.ToggleBreakpoint( req[1], req[2], req[3] );
				if ( req[1] )
					breakpointList[req[2]+':'+response] = true;
				else
					delete breakpointList[req[2]+':'+response];
				break;
			case 'breakpointList':
				response = breakpointList;
				break;
			case 'action':
				switch (req[1]) {
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
		responseFunction(uneval(response));
	} while ( !action );
	return action;
}
