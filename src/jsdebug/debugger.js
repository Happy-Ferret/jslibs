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

var dbg = new Debugger();
var breakpointList = {};

function Action(id) {

	this.valueOf = function() id;
}

var debuggerApi = {

	Ping: function() {
		
		return true;
	},

	State: function() {

		return { filename:this.filename, line:this.line, breakOrigin:OriginToString(this.breakOrigin), hasException:this.hasException, exception:this.exception };
	},

	Eval: function(code) {

		try {

			function tmp(c) eval(c);
			var prevScope = SetScope(tmp, this.scope);
			var res = tmp(code);
			SetScope(tmp, prevScope);
			return res;
		} catch (ex) {
		
			return ex;
		}
	},
	
	GetSource: function(filename) {
	
		return new File(filename).content;
	},
	
	GetScriptList: function() {
	
		return dbg.scriptList;
	},
	
	GetActualLineno: function(filename, lineno) {
	
		return dbg.GetActualLineno(filename, lineno);
	},
	
	Breakpoint: function(polarity, filename, lineno) {
	
		var actualLineno = dbg.ToggleBreakpoint(polarity, filename, lineno);
		if (polarity)
			breakpointList[filename+':'+actualLineno] = true;
		else
			delete breakpointList[filename+':'+actualLineno];
		return actualLineno;
	},
	
	BreakpointList: function() {
		
		return breakpointList;
	},

	Action: function(name) {
		
		switch (name) {
			case 'step':
				return new Action(Debugger.STEP);
			case 'stepover':
				return new Action(Debugger.STEP_OVER);
			case 'stepthrough':
				return new Action(Debugger.STEP_THROUGH);
			case 'stepout':
				return new Action(Debugger.STEP_OUT);
			case 'continue':
				return new Action(Debugger.CONTINUE);
		}
	}
}

dbg.onBreak = function( filename, line, scope, breakOrigin, frameLevel, hasException, exception ) {
	
	var breakContext = { filename:filename, line:line, scope:scope, breakOrigin:breakOrigin, frameLevel:frameLevel, hasException:hasException, exception:exception };
	for(;;) {

		var res, [req, responseFunction] = server.GetNextRequest();
		try {
			req = eval('('+req+')');
			res = debuggerApi[req[0]].apply(breakContext, req[1]);
		} catch(ex) {
			res = ex;
		}
		responseFunction(uneval(res));
		if ( res instanceof Action )
			return res;
	}
}
