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
!function() {

	function Match(v) Array.indexOf(arguments,v,1)-1;
	function Switch(i) arguments[++i];
	
	function ValToString(val) {
		
		function Crop(str, len) str.length > len ? str.substr(0, len)+'...' : str;
		
		try {
			switch ( typeof(val) ) {
				
				case 'string':
					return '"'+Crop(val,16)+'"';
				case 'number':
					return val;
				case 'object':
					if ( val === null )
						return 'null';
					if ( val instanceof Array )
						return '['+Crop(val.join(','),16)+']';
					return val.constructor.name+'{'+Crop(val.toString(),16)+'}';
			}
			return String(val);
		} catch (ex){}
		return '???';
	}

	function OriginToString( breakOrigin ) {

		with (Debugger) {
			var pos = Match(breakOrigin, FROM_BREAKPOINT, FROM_STEP, FROM_STEP_OVER, FROM_STEP_THROUGH, FROM_STEP_OUT, FROM_THROW, FROM_ERROR, FROM_DEBUGGER, FROM_EXECUTE, FROM_CALL);
			return Switch(pos, 'breakpoint', 'step', 'stepover', 'stepthrough', 'stepout', 'throw', 'error', 'debugger', 'execute', 'call');
		}
	}

	function SimpleHTTPServer(port, bind) {
		
		var pendingRequestList = [], serverSocket = new Socket(), socketList = [serverSocket];
		function CloseSocket(s) {
		
			s.Close();
			socketList.splice( socketList.indexOf(s), 1 );
		}
		serverSocket.readable = function() {

			var clientSocket = serverSocket.Accept();
			socketList.push(clientSocket);
			clientSocket.data = '';
			clientSocket.readable = function(s) {

 				var buf = s.Read();
				if ( !buf )
					return CloseSocket(s);
				s.data += buf;
				var eoh = s.data.indexOf('\r\n\r\n');
				if ( eoh == -1 )
					return;
				var contentLength = /^content-length: ?(.*)$/im(s.data)||[][1];
				s.data = s.data.substring(eoh+4);
				(function(s) {

					if ( s.data.length < contentLength ) {
				
						s.readable = arguments.callee;
 						var buf = s.Read();
						if ( !buf )
							return CloseSocket(s);						
 						s.data += buf;
						return;
					}
					delete s.readable;
					s.Shutdown(false);
					pendingRequestList.push([s.data, function(response) {
						
						if ( s.connectionClosed )
							return CloseSocket(s);
						s.Write('HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\n'+response);
						s.Shutdown(true);
						serverSocket.linger = 1000;
						CloseSocket(s)
						return response;
					}]);
				})(s);
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

	dbg.breakOnDebuggerKeyword = true;
	dbg.breakOnError = true;
	dbg.breakOnException = false;
	dbg.breakOnExecute = false;

	var breakpointList = {};
	var cookie = { __proto__:null };

	function Action(id) { this.valueOf = function() id }

	var debuggerApi = {

		Ping: function() {
			
			return true;
		},

		State: function() {
		
			with (this)
				return { filename:filename, lineno:lineno, breakOrigin:OriginToString(breakOrigin), stackFrameIndex:stackFrameIndex, hasException:hasException, exception:exception, rval:ValToString(rval) };
		},

		GetConfiguration: function() {
		
			return { breakOnError:dbg.breakOnError, breakOnException:dbg.breakOnException, breakOnDebuggerKeyword:dbg.breakOnDebuggerKeyword };
		},

		SetConfiguration: function( configuration ) {
		
			for ( var [item, value] in Iterator(configuration) )
				switch ( item ) {
					case 'breakOnError':
					case 'breakOnException':
					case 'breakOnDebuggerKeyword':
						dbg[item] = value;
						break;
			}
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
		
		AddBreakpoint: function(filename, lineno, condition) {
			
			if ( dbg.GetActualLineno(filename, lineno) != lineno )
				return false;
			dbg.ToggleBreakpoint(true, filename, lineno);
			var list = (filename in breakpointList) ? breakpointList[filename] : (breakpointList[filename] = {});
			list[lineno] = condition;
			return true;
		},

		RemoveBreakpoint: function(filename, lineno) {

			if ( dbg.GetActualLineno(filename, lineno) != lineno )
				return false;
			dbg.ToggleBreakpoint(false, filename, lineno);
			delete breakpointList[filename][lineno];
			return true;
		},

		BreakpointList: function(filename) {
			
			return breakpointList[filename] || {};
		},

		GetStack: function() {
			
			var stack = [];
			for ( var i = 0; i <= this.stackFrameIndex; i++ ) {
			
				var frameInfo = dbg.StackFrame(i);
				var contextInfo;
				if ( frameInfo.isEval )
					contextInfo = '(eval)';
				else if ( frameInfo.callee && frameInfo.callee.constructor == Function )
					contextInfo = (frameInfo.callee.name || '?') + '('+[ ValToString(arg) for each ( arg in frameInfo.argv ) ].join(',')+')';
				else
					contextInfo = '';

				with (frameInfo)
					stack.push({ index:i, filename:filename, lineno:lineno, isNative:isNative, contextInfo:contextInfo });
			}
			return stack;
		},

		Eval: function(code, stackFrameIndex) {

			return dbg.EvalInStackFrame(code, stackFrameIndex == undefined ? this.stackFrameIndex : stackFrameIndex );
		},
		
		ExpressionInfo: function(path, stackFrameIndex) {

			try {
			
				var val = dbg.EvalInStackFrame('('+path.shift()+')', stackFrameIndex == undefined ? this.stackFrameIndex : stackFrameIndex );
				val = path.reduce(function(prev, cur) prev[cur], val);
				return { childList:val instanceof Object ? [ name for ( name in val) ] : [], string:String(val), source:uneval(val) };
			} catch (ex) {
			
				return { childList:[], string:String(ex), source:'' };
			}
		},
		
		DefinitionLocation: function(identifier, stackFrameIndex) {
			
			var value = dbg.EvalInStackFrame(identifier, stackFrameIndex == undefined ? this.stackFrameIndex : stackFrameIndex );
			return dbg.DefinitionLocation(value);
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
				default:
					throw Error('Invalid debugger action');
			}
		}
	}

	dbg.onBreak = function( filename, lineno, scope, breakOrigin, stackFrameIndex, hasException, exception, rval ) {
		
		if ( breakOrigin == Debugger.FROM_BREAKPOINT && filename in breakpointList ) {

			var condition = breakpointList[filename][lineno];
			
			if ( condition && condition != 'true' )  {
				try {
					var test = dbg.EvalInStackFrame(condition, stackFrameIndex);
				} catch(ex) {}
				if ( !test )
					return Debugger.DO_CONTINUE;
			}
		}
		
		var breakContext = { filename:filename, lineno:lineno, scope:scope, breakOrigin:breakOrigin, stackFrameIndex:stackFrameIndex, hasException:hasException, exception:exception, rval:rval };
		for(;;) {

			var res, [req, responseFunction] = server.GetNextRequest();
			try {
			
				req = eval('('+req+')');
				res = debuggerApi[req[0]].apply(breakContext, req[1]);
			} catch(ex) {
			
				res = ex;
			}
			
			if ( res instanceof Action ) {

				responseFunction();
				return Number(res);
			}
			responseFunction(uneval(res));
		}
	}
	
	global.__dbg = dbg; // avoid CG
}();
