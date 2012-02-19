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

!function() {

	if ( '__dbg' in global )
		return;
	global.__dbg = undefined;
	SetPropertyEnumerate(global, '__dbg', false);
	
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
						return '['+Crop(val.join(','),16)+'] (#'+val.length+')';
					return val.constructor.name+'{'+Crop(val.toString(),16)+'}';
			}
			return String(val);
		} catch (ex){}
		return '???';
	}

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

	var server = new SimpleHTTPServer(8009, '127.0.0.1');

	var dbg = new Debugger();
	
	dbg.excludedFileList = [ currentFilename ];

	dbg.breakOnDebuggerKeyword = true;
	dbg.breakOnError = true;
	dbg.breakOnException = false;
	dbg.breakOnexecute = false;
	dbg.interruptCounterLimit = 0;

	var _breakpointList = {};
	var _cookie = { __proto__:null };
	var _time = TimeCounter();
	var _reset = [];
	var _breakContext;
	var _stdout = '';
	var _stderr = '';
	var _prevStdout = _host.stdout;
	var _prevStderr = _host.stdout;
	_host.stdout = function() { _stdout += Array.slice(arguments).join(''); return _prevStdout.apply(this, arguments) }
	_host.stderr = function() { _stderr += Array.slice(arguments).join(''); return _prevStderr.apply(this, arguments) }

	function OriginToString( breakOrigin ) {

		with (Debugger)
			var pos = Match(breakOrigin, FROM_INTERRUPT, FROM_BREAKPOINT, FROM_STEP, FROM_STEP_OVER, FROM_STEP_THROUGH, FROM_STEP_OUT, FROM_THROW, FROM_ERROR, FROM_DEBUGGER, FROM_EXECUTE, FROM_CALL);
		return Switch(pos, 'interrupt', 'breakpoint', 'step', 'stepover', 'stepthrough', 'stepout', 'throw', 'error', 'debugger', 'execute', 'call');
	}

	function Action(id) { throw id }
	function Asynchronus(fct) { throw fct }

	var debuggerApi = {

		Ping: function() true,
		
		StdOut: function() {
			
			var tmp = _stdout;
			_stdout = '';
			return tmp;
		},

		StdErr: function() {
			
			var tmp = _stderr;
			_stderr = '';
			return tmp;
		},

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
			
			return name in _cookie ? _cookie[name] : undefined;
		},
		
		GetSource: function(filename) {
		
			return new File(filename).content;
		},
		
		GetScriptList: function() {
		
			return scriptFilenameList; // (TBD) remove excludedFileList from scriptFilenameList.
		},
		
		GetActualLineno: function(filename, lineno) {
		
			return GetActualLineno(filename, lineno);
		},
		
		AddBreakpoint: function(filename, lineno, condition) {
			
			if ( GetActualLineno(filename, lineno) != lineno )
				return false;
			dbg.ToggleBreakpoint(true, filename, lineno);
			var list = (filename in _breakpointList) ? _breakpointList[filename] : (_breakpointList[filename] = {});
			list[lineno] = condition;
			return true;
		},

		RemoveBreakpoint: function(filename, lineno) {

			if ( GetActualLineno(filename, lineno) != lineno )
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
			
				var frameInfo = StackFrameInfo(i);
				var contextInfo;
				if ( frameInfo.isEval )
					contextInfo = '(eval)';
				else if ( frameInfo.callee && frameInfo.callee.constructor == Function )
					contextInfo = (frameInfo.callee.name || '?') + '('+[ ValToString(arg) for each ( arg in frameInfo.argv ) ].join(' , ')+')';
				else
					contextInfo = '';

				with (frameInfo)
					stack.push({ index:i, filename:filename, lineno:lineno, isNative:isNative, contextInfo:contextInfo });
			}
			return stack;
		},

		Eval: function(code, stackFrameIndex) {

			return EvalInStackFrame(code, stackFrameIndex == undefined ? _breakContext.stackFrameIndex : stackFrameIndex );
		},
		
		ExpressionInfo: function(path, stackFrameIndex, ChildListOnly) {
			
			stackFrameIndex = stackFrameIndex == undefined ? _breakContext.stackFrameIndex : stackFrameIndex;
			var val, expression = path.shift();
			try {
				if ( expression[0] != '.' ) {

					val = EvalInStackFrame('('+expression+')', stackFrameIndex );
				} else {

					val = { stack:StackFrameInfo(stackFrameIndex), rval:_breakContext.rval }[expression.substr(1)];
				}
				val = path.reduce(function(prev, cur) prev[cur], val);
				if ( ChildListOnly )
					return IsPrimitive(val) ? undefined : [ { name:v.name, enumerate:v.enumerate, readonly:v.readonly, permanent:v.permanent } for each ( v in PropertiesInfo(val, true) ) ];
				else
					return { isObj:!IsPrimitive(val), string:Try(function() ValToString(val)), source:Try(function() uneval(val)) };

			} catch(ex) {
				return { isObj:false, string:String(ex), source:'' };
			}
		},
		
		DefinitionLocation: function(identifier, stackFrameIndex) {
			
			var value = EvalInStackFrame(identifier, stackFrameIndex == undefined ? _breakContext.stackFrameIndex : stackFrameIndex );
			return DefinitionLocation(value);
		},
		
		DisassembleScript: function(filename, lineno) {
			
			var asm = DisassembleScript(filename, lineno);
			if ( !asm )
				return 'Disassembly is only available in DEBUG mode\n';
			asm = [ let (r = /(\d+): *(\d+) *(.*)/(line)) r ? r[2]+' '+r[3] : '' for each ( line in asm.split('\n') ) ].join('\n');
			return 'Script at '+filename+':'+lineno+'\n'+asm+'\n';
		},
		
		Shutdown: function() {

			dbg.breakOnDebuggerKeyword = false;
			dbg.breakOnError = false;
			dbg.breakOnException = false;
			dbg.breakOnexecute = false;
			delete dbg.onBreak;
			dbg.ClearBreakpoints();
			delete global.__dbg;
			_host.stdout = _prevStdout;
			_host.stdout = _prevStderr;
			Action(Debugger.DO_CONTINUE);
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
				Action(Debugger.DO_CONTINUE);
			lineno = dbg.ToggleBreakpoint(true, filename, lineno);
			_reset.push(function() dbg.ToggleBreakpoint(false, filename, lineno));
			Action(Debugger.DO_CONTINUE);
		},
		
		TraceTo: function(aimFilename, aimLineno) Asynchronus(function(responseFunction) {
		
			var time;
			with (_breakContext)
				var trace = [[filename, lineno, stackFrameIndex, '?.???']]; // the current line
			var prevBreakFct = dbg.onBreak;
			dbg.onBreak = function(filename, lineno, breakOrigin, stackFrameIndex) {

				if ( filename != aimFilename || lineno != aimLineno ) {
				
					trace.push([filename, lineno, stackFrameIndex, (TimeCounter() - time).toFixed(3)]);
					time = TimeCounter();
					return Debugger.DO_STEP;
				}
				dbg.onBreak = prevBreakFct;
				responseFunction(uneval(trace));
				return dbg.onBreak.apply(dbg, arguments);
			}
			time = TimeCounter();
			Action(Debugger.DO_STEP);
		})
	}
	
	dbg.onBreak = function(filename, lineno, breakOrigin, stackFrameIndex, hasException, exception, rval, enteringFunction) {
		
//		if ( breakOrigin == Debugger.FROM_INTERRUPT && !server.HasPendingRequest() )
//			return Debugger.DO_CONTINUE;
		
		var tmpTime = TimeCounter();
		while (_reset.length) _reset.shift()();
		
		if ( breakOrigin == Debugger.FROM_BREAKPOINT && filename in _breakpointList ) {

			var condition = _breakpointList[filename][lineno];
			if ( condition && condition != 'true' ) {
			
				try {
					var test = EvalInStackFrame(condition, stackFrameIndex);
				} catch(ex) {}
				if ( !test )
					return Debugger.DO_CONTINUE;
			}
		}
		
		_breakContext = { filename:filename, lineno:lineno, breakOrigin:breakOrigin, stackFrameIndex:stackFrameIndex, hasException:hasException, exception:exception, rval:rval, enteringFunction:enteringFunction, time:tmpTime - _time };
		for(;;) {

			var res = undefined, [req, responseFunction] = server.GetNextRequest();
			
			try {
				try {

					req = eval('('+req+')');
					res = debuggerApi[req[0]].apply(debuggerApi, req[1]);
					responseFunction(uneval(res));
				} catch ( fct if IsFunction(fct) ) {
					
					var tmp = responseFunction;
					responseFunction = function(){}
					fct.call(debuggerApi, tmp);
				}
			} catch ( action if !isNaN(action) ) {

				responseFunction();
				_time = TimeCounter();
				return action;
			}  catch ( ex ) {
				
				responseFunction(uneval(ex));
			}
		}
	}

	if ( global.arguments[0] == currentFilename ) { // debugger.js is used like: jshost jsdebugger.js programToDebug.js
		
		dbg.breakOnFirstexecute = true;
		var prog = global.arguments.splice(1,1);
		exec(prog, false);
	}

	global.__dbg = dbg; // avoid CG
}();
