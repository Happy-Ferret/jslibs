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
var _dbg = (function() {

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
					socketList.splice(socketList.indexOf(s), 1);
					return true;
				}]);
			}
		}

		serverSocket.Bind(port, bind);
		serverSocket.Listen();

		this.GetNextRequest = function() {

			while( !pendingRequestList.length )
				Poll(socketList, 100);
			return pendingRequestList.shift();
		}	
	}

	var server = new SimpleHTTPServer(8009, '127.0.0.1');

	var dbg = new Debugger();
	var breakpointList = {};
	var cookie = { __proto__:null };

	function Action(id) {

		this.valueOf = function() id;
	}

	var debuggerApi = {

		Ping: function() {
			
			return true;
		},

		State: function() {
		
			with (this)
				return { filename:filename, lineno:lineno, breakOrigin:OriginToString(breakOrigin), stackFrameIndex:stackFrameIndex, hasException:hasException, exception:exception };
		},

		SetCookie: function(name, data) {
			
			cookie[name] = data;
		},
		
		GetCookie: function(name) {
			
			return cookie[name];
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
		
		HasBreakpoint: function(filename, lineno) {
		
			var actualLineno = dbg.GetActualLineno(filename, lineno);
			var loc = filename+':'+actualLineno;
			return loc in breakpointList;
		},
		
		AddBreakpoint: function(filename, lineno) {
		
			var actualLineno = dbg.ToggleBreakpoint(true, filename, lineno);
			breakpointList[filename+':'+actualLineno] = undefined;
			return actualLineno;
		},

		RemoveBreakpoint: function(filename, lineno) {
		
			var actualLineno = dbg.ToggleBreakpoint(false, filename, lineno);
			delete breakpointList[filename+':'+actualLineno];
			return actualLineno;
		},

		SetBreakpointCondition: function(filename, lineno, condition) {
		
			var actualLineno = dbg.GetActualLineno(filename, lineno);
			var loc = filename+':'+actualLineno;
			if ( !(loc in breakpointList) )
				throw Error('Invalid breakpoint ('+loc+')');
			breakpointList[loc] = condition;
		},
		
		GetBreakpointCondition: function(filename, lineno) {
		
			var actualLineno = dbg.GetActualLineno(filename, lineno);
			var loc = filename+':'+actualLineno;
			if ( !(loc in breakpointList) )
				throw Error('Invalid breakpoint ('+loc+')');
			return breakpointList[loc];
		},

		BreakpointList: function(filename) {
			
			if ( filename == undefined )
				return [ let (p = b.lastIndexOf(':') ) [b.substring(0, p), b.substring(p+1), c ] for each ( [b, c] in Iterator(breakpointList) ) ];
			else
				return [ [b.substring(filename.length+1), c] for ( [b, c] in Iterator(breakpointList) ) if (b.indexOf(filename+':') == 0) ];
		},
		
		GetStack: function() {
			
			var stack = [];
			for ( var i = 0; i <= this.stackFrameIndex; i++ ) {
			
				var frameInfo = dbg.StackFrame(i);
				var contextName;
				if ( frameInfo.isEval )
					contextName = '(eval)';
				else if ( frameInfo.callee && frameInfo.callee.constructor == Function )
					contextName = frameInfo.callee.name + '()';
				else
					contextName = '';

				with (frameInfo)
					stack.unshift({ filename:filename, lineno:lineno, isNative:isNative, contextName:contextName });
			}
			return stack;
		},

		Eval: function(code, stackFrameIndex) {

			try {
			
				return dbg.EvalInStackFrame(code, stackFrameIndex == undefined ? this.stackFrameIndex : stackFrameIndex );
			} catch (ex) {
			
				return ex;
			}
		},
		
		DefinitionLocation: function(identifier, stackFrameIndex) {
			
			try {
				var value = dbg.EvalInStackFrame(identifier, stackFrameIndex == undefined ? this.stackFrameIndex : stackFrameIndex );
				return dbg.DefinitionLocation(value);
			} catch (ex) {
			
				return ex;
			}
		},

		Action: function(name) {
			
			switch (name) {
				case 'step':
					return new Action(Debugger.DO_STEP);
				case 'stepover':
					return new Action(Debugger.DO_STEP_OVER);
				case 'stepthrough':
					return new Action(Debugger.DO_STEP_THROUGH);
				case 'stepout':
					return new Action(Debugger.DO_STEP_OUT);
				case 'continue':
					return new Action(Debugger.DO_CONTINUE);
			}
		}
	}

	dbg.onBreak = function( filename, lineno, scope, breakOrigin, stackFrameIndex, hasException, exception ) {
		
		if ( breakOrigin == Debugger.FROM_BREAKPOINT ) {

			var condition = breakpointList[filename+':'+lineno];
			if ( condition )  {
				try {
					var test = dbg.EvalInStackFrame(condition, stackFrameIndex);
				} catch(ex) {}
				if ( !test )
					return Debugger.DO_CONTINUE;
			}
		}
		
		var breakContext = { filename:filename, lineno:lineno, scope:scope, breakOrigin:breakOrigin, stackFrameIndex:stackFrameIndex, hasException:hasException, exception:exception };
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
	
	return dbg;
})();
