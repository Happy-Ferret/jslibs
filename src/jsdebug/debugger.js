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
	setPropertyEnumerate(global, '__dbg', false);
	
	function try(fct) { try { return fct() } catch(ex) { return ex } }
	function match(v) Array.indexOf(arguments,v,1)-1;
	function switch(i) arguments[++i];
	
	var valToString = function(val) {
		
		function crop(str, len) str.length > len ? str.substr(0, len)+'...' : str;
		
		try {
			switch ( typeof(val) ) {
				
				case 'string':
					return '"'+crop(val,16)+'"';
				case 'number':
					return val;
				case 'object':
					if ( val === null )
						return 'null';
					if ( val instanceof Array )
						return '['+crop(val.join(','),16)+'] (#'+val.length+')';
					return val.constructor.name+'{'+crop(val.toString(),16)+'}';
			}
			return String(val);
		} catch (ex){}
		return '???';
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
	var _time = timeCounter();
	var _reset = [];
	var _breakContext;
	var _stdout = '';
	var _stderr = '';
	var _prevStdout = host.stdout;
	var _prevStderr = host.stdout;
	host.stdout = function() { _stdout += Array.slice(arguments).join(''); return _prevStdout.apply(this, arguments) }
	host.stderr = function() { _stderr += Array.slice(arguments).join(''); return _prevStderr.apply(this, arguments) }

	function originToString( breakOrigin ) {

		with (Debugger)
			var pos = match(breakOrigin, FROM_INTERRUPT, FROM_BREAKPOINT, FROM_STEP, FROM_STEP_OVER, FROM_STEP_THROUGH, FROM_STEP_OUT, FROM_THROW, FROM_ERROR, FROM_DEBUGGER, FROM_EXECUTE, FROM_CALL);
		return switch(pos, 'interrupt', 'breakpoint', 'step', 'stepover', 'stepthrough', 'stepout', 'throw', 'error', 'debugger', 'execute', 'call');
	}

	function Action(id) { throw id }
	function asynchronus(fct) { throw fct }

	var debuggerApi = {

		ping: function() true,
		
		stdOut: function() {
			
			var tmp = _stdout;
			_stdout = '';
			return tmp;
		},

		stdErr: function() {
			
			var tmp = _stderr;
			_stderr = '';
			return tmp;
		},

		state: function() {
		
			with (_breakContext)
				return { filename:filename, lineno:lineno, breakOrigin:originToString(breakOrigin), stackFrameIndex:stackFrameIndex, hasException:hasException, exception:exception, rval:valToString(rval), enteringFunction:enteringFunction, time:time };
		},

		getConfiguration: function() {
		
			return { breakOnError:dbg.breakOnError, breakOnException:dbg.breakOnException, breakOnDebuggerKeyword:dbg.breakOnDebuggerKeyword };
		},

		setConfiguration: function( configuration ) {
		
			for ( var [item, value] in Iterator(configuration) )
				switch ( item ) {
					case 'breakOnError':
					case 'breakOnException':
					case 'breakOnDebuggerKeyword':
						dbg[item] = value;
						break;
			}
		},
		
		setValToStringFilter: function(functionSource) {
			
			valToString = eval('('+functionSource+')');
		},
		
		setCookie: function(name, data) {
			
			_cookie[name] = data;
		},
		
		getCookie: function(name) {
			
			return name in _cookie ? _cookie[name] : undefined;
		},
		
		getSource: function(filename) {
		
			return new File(filename).content;
		},
		
		getScriptList: function() {
		
			return scriptFilenameList; // (TBD) remove excludedFileList from scriptFilenameList.
		},
		
		getActualLineno: function(filename, lineno) {
		
			return getActualLineno(filename, lineno);
		},
		
		addBreakpoint: function(filename, lineno, condition) {
			
			if ( getActualLineno(filename, lineno) != lineno )
				return false;
			dbg.toggleBreakpoint(true, filename, lineno);
			var list = (filename in _breakpointList) ? _breakpointList[filename] : (_breakpointList[filename] = {});
			list[lineno] = condition;
			return true;
		},

		removeBreakpoint: function(filename, lineno) {

			if ( getActualLineno(filename, lineno) != lineno )
				return false;
			dbg.toggleBreakpoint(false, filename, lineno);
			delete _breakpointList[filename][lineno];
			return true;
		},

		breakpointList: function(filename) {
			
			return _breakpointList[filename] || {};
		},
		
		getStack: function() {
			
			var stack = [];
			for ( var i = 0; i <= _breakContext.stackFrameIndex; i++ ) {
			
				var frameInfo = stackFrameInfo(i);
				var contextInfo;
				if ( frameInfo.isEval )
					contextInfo = '(eval)';
				else if ( frameInfo.callee && frameInfo.callee.constructor == Function )
					contextInfo = (frameInfo.callee.name || '?') + '('+[ valToString(arg) for each ( arg in frameInfo.argv ) ].join(' , ')+')';
				else
					contextInfo = '';

				with (frameInfo)
					stack.push({ index:i, filename:filename, lineno:lineno, isNative:isNative, contextInfo:contextInfo });
			}
			return stack;
		},

		eval: function(code, stackFrameIndex) {

			return evalInStackFrame(code, stackFrameIndex == undefined ? _breakContext.stackFrameIndex : stackFrameIndex );
		},
		
		expressionInfo: function(path, stackFrameIndex, childListOnly) {
			
			stackFrameIndex = stackFrameIndex == undefined ? _breakContext.stackFrameIndex : stackFrameIndex;
			var val, expression = path.shift();
			try {
				if ( expression[0] != '.' ) {

					val = evalInStackFrame('('+expression+')', stackFrameIndex );
				} else {

					val = { stack:stackFrameInfo(stackFrameIndex), rval:_breakContext.rval }[expression.substr(1)];
				}
				val = path.reduce(function(prev, cur) prev[cur], val);
				if ( childListOnly )
					return isPrimitive(val) ? undefined : [ { name:v.name, enumerate:v.enumerate, readonly:v.readonly, permanent:v.permanent } for each ( v in propertiesInfo(val, true) ) ];
				else
					return { isObj:!isPrimitive(val), string:try(function() valToString(val)), source:try(function() uneval(val)) };

			} catch(ex) {
				return { isObj:false, string:String(ex), source:'' };
			}
		},
		
		definitionLocation: function(identifier, stackFrameIndex) {
			
			var value = evalInStackFrame(identifier, stackFrameIndex == undefined ? _breakContext.stackFrameIndex : stackFrameIndex );
			return definitionLocation(value);
		},
		
		disassembleScript: function(filename, lineno) {
			
			var asm = disassembleScript(filename, lineno);
			if ( !asm )
				return 'Disassembly is only available in DEBUG mode\n';
			asm = [ let (r = /(\d+): *(\d+) *(.*)/(line)) r ? r[2]+' '+r[3] : '' for each ( line in asm.split('\n') ) ].join('\n');
			return 'Script at '+filename+':'+lineno+'\n'+asm+'\n';
		},
		
		shutdown: function() {

			dbg.breakOnDebuggerKeyword = false;
			dbg.breakOnError = false;
			dbg.breakOnException = false;
			dbg.breakOnexecute = false;
			delete dbg.onBreak;
			dbg.clearBreakpoints();
			delete global.__dbg;
			host.stdout = _prevStdout;
			host.stdout = _prevStderr;
			Action(Debugger.DO_CONTINUE);
		},

		step: function() new Action(Debugger.DO_STEP),
		stepOver: function() new Action(Debugger.DO_STEP_OVER),
		stepThrough: function() new Action(Debugger.DO_STEP_THROUGH),
		stepOut: function() new Action(Debugger.DO_STEP_OUT),
		continue: function() new Action(Debugger.DO_CONTINUE),
		goto: function(filename, lineno) {
		
			if ( _breakContext.filename == filename && _breakContext.lineno == lineno )
				return;
			if ( dbg.hasBreakpoint(filename, lineno) )
				Action(Debugger.DO_CONTINUE);
			lineno = dbg.toggleBreakpoint(true, filename, lineno);
			_reset.push(function() dbg.toggleBreakpoint(false, filename, lineno));
			Action(Debugger.DO_CONTINUE);
		},
		
		traceTo: function(aimFilename, aimLineno) asynchronus(function(responseFunction) {
		
			var time;
			with (_breakContext)
				var trace = [[filename, lineno, stackFrameIndex, '?.???']]; // the current line
			var prevBreakFct = dbg.onBreak;
			dbg.onBreak = function(filename, lineno, breakOrigin, stackFrameIndex) {

				if ( filename != aimFilename || lineno != aimLineno ) {
				
					trace.push([filename, lineno, stackFrameIndex, (timeCounter() - time).toFixed(3)]);
					time = timeCounter();
					return Debugger.DO_STEP;
				}
				dbg.onBreak = prevBreakFct;
				responseFunction(uneval(trace));
				return dbg.onBreak.apply(dbg, arguments);
			}
			time = timeCounter();
			Action(Debugger.DO_STEP);
		})
	}
	
	dbg.onBreak = function(filename, lineno, breakOrigin, stackFrameIndex, hasException, exception, rval, enteringFunction) {
		
//		if ( breakOrigin == Debugger.FROM_INTERRUPT && !server.HasPendingRequest() )
//			return Debugger.DO_CONTINUE;
		
		var tmpTime = timeCounter();
		while (_reset.length) _reset.shift()();
		
		if ( breakOrigin == Debugger.FROM_BREAKPOINT && filename in _breakpointList ) {

			var condition = _breakpointList[filename][lineno];
			if ( condition && condition != 'true' ) {
			
				try {
					var test = evalInStackFrame(condition, stackFrameIndex);
				} catch(ex) {}
				if ( !test )
					return Debugger.DO_CONTINUE;
			}
		}
		
		_breakContext = { filename:filename, lineno:lineno, breakOrigin:breakOrigin, stackFrameIndex:stackFrameIndex, hasException:hasException, exception:exception, rval:rval, enteringFunction:enteringFunction, time:tmpTime - _time };
		for(;;) {

			var res = undefined, [req, responseFunction] = server.getNextRequest();
			
			try {
				try {

					req = eval('('+req+')');
					res = debuggerApi[req[0]].apply(debuggerApi, req[1]);
					responseFunction(uneval(res));
				} catch ( fct if isFunction(fct) ) {
					
					var tmp = responseFunction;
					responseFunction = function(){}
					fct.call(debuggerApi, tmp);
				}
			} catch ( action if !isNaN(action) ) {

				responseFunction();
				_time = timeCounter();
				return action;
			}  catch ( ex ) {
				
				responseFunction(uneval(ex));
			}
		}
	}

	if ( host.arguments[0] == currentFilename ) { // debugger.js is used like: jshost jsdebugger.js programToDebug.js
		
		dbg.breakOnFirstexecute = true;
		var prog = host.arguments.splice(1,1);
		exec(prog, false);
	}

	global.__dbg = dbg; // avoid CG
}();
