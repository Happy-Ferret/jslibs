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
LoadModule('jsdebug');
!function() {
	
	function Try(fct) { try { return fct() } catch(ex) { return ex } }

	function Match(v) Array.indexOf(arguments,v,1)-1;
	function Switch(i) arguments[++i];
	
	var ValToString = function(val) {
		
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

	function SimpleHTTPServer(port, bind) {
		
		var pendingRequestList = [], serverSocket = new Socket(), socketList = [serverSocket];
		function CloseSocket(s) {

			s.Close();
			socketList.splice( socketList.indexOf(s), 1 );
		}
	
		function ProcessRequest(s) {

			var buf = s.Read();
			if ( buf == undefined )
				return CloseSocket(s);
			s.data += buf;
			var eoh = s.data.indexOf('\r\n\r\n');
			if ( eoh == -1 )
				return;
			var contentLength = (/^content-length: ?(.*)$/im(s.data)||[])[1];
			s.data = s.data.substring(eoh+4);
			(function(s) {

				if ( s.data.length < contentLength ) {
			
					s.readable = arguments.callee;
					var buf = s.Read();
					if ( buf == undefined )
						return CloseSocket(s);						
					s.data += buf;
					return;
				}
			
				delete s.readable;
				pendingRequestList.push([s.data.substring(0, contentLength), function(response) {
					
					if ( s.connectionClosed )
						return CloseSocket(s);
					if ( response == undefined )
						s.Write('HTTP/1.1 204 No Content\r\n\r\n');
					else
						s.Write('HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: '+response.length+'\r\n\r\n'+response);
					s.readable = ProcessRequest;
					return true;
				}]);
				s.data = s.data.substring(contentLength);
			})(s);
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

	var _breakpointList = {};
	var _cookie = { __proto__:null };
	var _time = TimeCounter();
	var _reset = [];
	var _breakContext;

	function OriginToString( breakOrigin ) {

		with (Debugger)
			var pos = Match(breakOrigin, FROM_BREAKPOINT, FROM_STEP, FROM_STEP_OVER, FROM_STEP_THROUGH, FROM_STEP_OUT, FROM_THROW, FROM_ERROR, FROM_DEBUGGER, FROM_EXECUTE, FROM_CALL);
		return Switch(pos, 'breakpoint', 'step', 'stepover', 'stepthrough', 'stepout', 'throw', 'error', 'debugger', 'execute', 'call');
	}

	function Action(id) { this.valueOf = function() id }
	function Asynchronus(fct) { this.fct = fct }

	var debuggerApi = {

		Ping: function() true,

		State: function() {
		
			with (_breakContext)
				return { filename:filename, lineno:lineno, breakOrigin:OriginToString(breakOrigin), stackFrameIndex:stackFrameIndex, hasException:hasException, exception:exception, rval:ValToString(rval), enteringFunction:enteringFunction, time:time };
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
		
		SetValToStringFilter: function(functionSource) {
			
			ValToString = eval('('+functionSource+')');
		},
		
		SetCookie: function(name, data) {
			
			_cookie[name] = data;
		},
		
		GetCookie: function(name) {
			
			return _cookie[name];
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
			var list = (filename in _breakpointList) ? _breakpointList[filename] : (_breakpointList[filename] = {});
			list[lineno] = condition;
			return true;
		},

		RemoveBreakpoint: function(filename, lineno) {

			if ( dbg.GetActualLineno(filename, lineno) != lineno )
				return false;
			dbg.ToggleBreakpoint(false, filename, lineno);
			delete _breakpointList[filename][lineno];
			return true;
		},

		BreakpointList: function(filename) {
			
			return _breakpointList[filename] || {};
		},
		
		GetStack: function() {
			
			var stack = [];
			for ( var i = 0; i <= _breakContext.stackFrameIndex; i++ ) {
			
				var frameInfo = dbg.StackFrame(i);
				var contextInfo;
				if ( frameInfo.isEval )
					contextInfo = '(eval)';
				else if ( frameInfo.callee && frameInfo.callee.constructor == Function )
					contextInfo = (frameInfo.callee.name || '?') + '('+[ ValToString(arg) for each ( arg in frameInfo.argv ) ].join(' , ')+')';
				else
					contextInfo = '';

				with (frameInfo)
					stack.push({ index:i, filename:filename, lineno:lineno, isNative:isNative, contextInfo:contextInfo, variables:[n for ( n in variables )] });
			}
			return stack;
		},

		Eval: function(code, stackFrameIndex) {

			return dbg.EvalInStackFrame(code, stackFrameIndex == undefined ? _breakContext.stackFrameIndex : stackFrameIndex );
		},
		
		ExpressionInfo: function(path, stackFrameIndex, ChildListOnly) {
			
			try {
				var val = dbg.EvalInStackFrame('('+path.shift()+')', stackFrameIndex == undefined ? _breakContext.stackFrameIndex : stackFrameIndex );
				val = path.reduce(function(prev, cur) prev[cur], val);
				var isObj = typeof(val) == 'object';
				if ( ChildListOnly )
					return isObj ? [ name for ( name in val) ] : undefined;
				else
					return { isObj:isObj, string:Try(function() ValToString(val)), source:Try(function() uneval(val)) };
			} catch(ex) {
				return { isObj:false, string:String(ex), source:'' };
			}
		},
		
/*
		ContextInfo: function(path, stackFrameIndex) {
			
			var context = { stack:dbg.StackFrame(stackFrameIndex == undefined ? context.stackFrameIndex : stackFrameIndex), rval:context.rval };
			var val = context[path.shift()];
			val = path.reduce(function(prev, cur) prev[cur], val);
			return { childList:typeof(val) == 'object' ? [ name for ( name in val) ] : [], string:Try(function() ValToString(val)), source:Try(function() uneval(val)) };
		},
*/

		DefinitionLocation: function(identifier, stackFrameIndex) {
			
			var value = dbg.EvalInStackFrame(identifier, stackFrameIndex == undefined ? _breakContext.stackFrameIndex : stackFrameIndex );
			return dbg.DefinitionLocation(value);
		},
		
		Shutdown: function() {

			dbg.breakOnDebuggerKeyword = false;
			dbg.breakOnError = false;
			dbg.breakOnException = false;
			dbg.breakOnExecute = false;
			delete dbg.onBreak;
			delete global.__dbg;
			return new Action(Debugger.DO_CONTINUE);
		},

		Step: function() new Action(Debugger.DO_STEP),
		StepOver: function() new Action(Debugger.DO_STEP_OVER),
		StepThrough: function() new Action(Debugger.DO_STEP_THROUGH),
		StepOut: function() new Action(Debugger.DO_STEP_OUT),
		Continue: function() new Action(Debugger.DO_CONTINUE),
		Goto: function(filename, lineno) {
		
			if ( _breakContext.filename == filename && _breakContext.lineno == lineno )
				return;
			if ( dbg.HasBreakpoint(filename, lineno) )
				return new Action(Debugger.DO_CONTINUE);
			var lineno = dbg.ToggleBreakpoint(true, filename, lineno);
			_reset.push(function() dbg.ToggleBreakpoint(false, filename, lineno));
			return new Action(Debugger.DO_CONTINUE);
		},
		
		TraceTo: function(aimFilename, aimLineno) new Asynchronus(function(responseFunction) {
		
			with (_breakContext)
				var trace = [[filename, lineno, stackFrameIndex]]; // the current line
			var prevBreakFct = dbg.onBreak;
			dbg.onBreak = function(filename, lineno, scope, breakOrigin, stackFrameIndex, hasException, exception, rval) {

				if ( filename != aimFilename || lineno != aimLineno ) {
				
					trace.push([filename, lineno, stackFrameIndex]);
					return Debugger.DO_STEP;
				}
			
				dbg.onBreak = prevBreakFct;
				responseFunction(uneval(trace));
				return dbg.onBreak.apply(this, arguments);
			}
			return new Action(Debugger.DO_STEP);
		}),
	}
	
	dbg.onBreak = function(filename, lineno, scope, breakOrigin, stackFrameIndex, hasException, exception, rval, enteringFunction) {
		
		var tmpTime = TimeCounter();
		while (_reset.length) _reset.shift()();
		
		if ( breakOrigin == Debugger.FROM_BREAKPOINT && filename in _breakpointList ) {

			var condition = _breakpointList[filename][lineno];
			if ( condition && condition != 'true' ) {
			
				try {
					var test = dbg.EvalInStackFrame(condition, stackFrameIndex);
				} catch(ex) {}
				if ( !test )
					return Debugger.DO_CONTINUE;
			}
		}
		
		_breakContext = { filename:filename, lineno:lineno, scope:scope, breakOrigin:breakOrigin, stackFrameIndex:stackFrameIndex, hasException:hasException, exception:exception, rval:rval, enteringFunction:enteringFunction, time:tmpTime - _time };
		for(;;) {

			var res, [req, responseFunction] = server.GetNextRequest();
			try {

				req = eval('('+req+')');
				res = debuggerApi[req[0]].apply(debuggerApi, req[1]);
			} catch(ex) {
				
				res = ex;
			}
			
			if ( res instanceof Asynchronus ) {
			
				res = res.fct.call(cx, responseFunction);
				responseFunction = function(){}
			}

			if ( res instanceof Action ) {
				
				responseFunction();
				res = Number(res);
				_time = TimeCounter();
				return res;
			}
			
			responseFunction(uneval(res));
		}
	}

	global.__dbg = dbg; // avoid CG
}();
