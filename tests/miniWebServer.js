Exec('deflib.js');
LoadModule('jsnspr');
LoadModule('jsz');

const CR = '\r';
const LF = '\n';
const CRLF = CR+LF;
const SP = ' ';
const DOT = '.';

const mimeType = {
	html:'text/html',
	gif:'image/gif',
	jpeg:'image/jpeg',
	jpg:'image/jpeg',
	jpe:'image/jpeg'
};

const statusText = { 
	200:'OK', 
	404:'Not Found' 
};


var log = new function() {

	var _time0 = IntervalNow();
	var _file = new File('miniWebServer.log');
	_file.Open( File.CREATE_FILE + File.WRONLY + File.APPEND );
	
	this.Write = function( data ) {
		
		var t = IntervalNow() - _time0;
		_file.Write( data );
	}
	
	this.WriteLn = function( data ) {
		
		this.Write( data + LF );
	}

	this.Close = function() {
		
		_file.Close();
	}
}


var timeout = new function() {

	var _min;
	var _tlist = {};
	this.Add = function( time, func ) {
	
		var when = IntervalNow() + time;
		while( _tlist[when] ) when++; // avoid same time
		_tlist[when] = func;
		if ( when < _min )
			_min = when;
		return when;
	}
	
	this.Remove = function(when) {
		
		if ( when == _min )
			_min = Number.POSITIVE_INFINITY;
		delete _tlist[when];
	}
	
	this.Next = function() {
		
		_min = Number.POSITIVE_INFINITY;
		for ( var w in _tlist )
			if ( w < _min )
				_min = w;
		return _min == Number.POSITIVE_INFINITY ? undefined : _min - IntervalNow();
	}

	this.Process = function() {
		
		var now = IntervalNow();
		if ( _min > now )
			return;
		for ( var [w,f] in _tlist )
			if ( w <= now ) {
				f();
				delete _tlist[w];
			}
	}
}

var root='.';

var list = [];


function CreateHttpHeaders( status, headers ) {

	var buf = 'HTTP/1.1' + SP + status + SP + statusText[status] + CRLF;
	for ( var [h,v] in headers )
		buf += h + ': ' + v + CRLF;
	return buf + CRLF;
}

function CloseConnection(s) {

	s.Close();
	list.splice( list.indexOf(s), 1 );
}


function ParseHeaders(buffer) {
	
	log.WriteLn(buffer);

	var lines = buffer.split(CRLF);
	var [method,url,proto] = lines[0].split(SP);
	var [path,query] = url.split('?');
	var status = { method:method, url:url, proto:proto, path:path, query:query };
	var headers = {};
	for ( var i=1; lines[i].length; i++ ) {
		var [name,value] = lines[i].split(': ');
		headers[name.toLowerCase()] = value;
	}
	return [ status, headers ];
}



function SocketWriterHelper(s,starved,end) {
	
	var _chunkQueue = [];

	function feed() {
	
		for ( var i=0; i<arguments.length; i++ )
			_chunkQueue.push(arguments[i]);
	}
	
	function Next() {
		
		if (!starved(feed)) // continue or exit ?
			Next = function() {
				
				delete s.writable
				end && end();
			}
	}
	
	s.writable = function(s) {
		
		if ( _chunkQueue.length ) {
		
			var unsent = s.Send(_chunkQueue.shift());
			unsent.length && _chunkQueue.unshift(unsent);				
		} else
			Next();
	}
}




/*
	function ProcessRequest( status, headers, body ) {
	
		var file = new File( root + status.path );
		if ( !file.exist || file.info.type != File.FILE_FILE ) {
		
			var message = 'file not found';
			SendHttpHeaders( s, 404, {'Content-Length':message.length, 'Content-Type':'text/plain'} );
			s.Send(message);
			return;
		}

		var respondeHeaders = {};
		respondeHeaders['Content-Type'] = mimeType[status.path.substr(status.path.lastIndexOf(DOT)+1)] || 'text/html; charset=iso-8859-1';

		respondeHeaders['Transfer-Encoding'] = 'chunked';
		respondeHeaders['Connection'] = 'Keep-Alive';

		var ContentEncoding = function(data) {
			
			return data; // identity filter
		}
		
		if ( headers['accept-encoding'].indexOf('deflate') != -1 ) {
		
			respondeHeaders['Content-Encoding'] = 'deflate';
			var deflate = new Z(Z.DEFLATE);
			ContentEncoding = function(data) {

				return deflate(data);
			}
		}
		
		SendHttpHeaders( s, 200, respondeHeaders );
		file.Open( File.RDONLY );

		SocketWriterHelper( s, function(write) {

			var data = file.Read(Z.idealInputLength);
			data = ContentEncoding(data);

			if ( data.length ) {
			
				write( data.length.toString(16) + CRLF, data, CRLF );
				return true; // continue
			}
			write( '0' + CRLF + CRLF );
			file.Close();
			return false;
		});
	}
*/


function Identity(arg) { return arg; }
function Noop() {}


function ProcessRequest( status, headers, output ) {

	var data = '';

	var file = new File( root + status.path );
	if ( !file.exist || file.info.type != File.FILE_FILE ) {

		var message = 'file not found';
		output(CreateHttpHeaders( 404, {'Content-Length':message.length, 'Content-Type':'text/plain'} ));
		output(message);
		return Noop;
	}


	return function(chunk) {
		
		if ( !chunk ) {

			var respondeHeaders = {};
			respondeHeaders['Content-Type'] = mimeType[status.path.substr(status.path.lastIndexOf(DOT)+1)] || 'text/html; charset=iso-8859-1';
			respondeHeaders['Transfer-Encoding'] = 'chunked';
			respondeHeaders['Connection'] = 'Keep-Alive';

			var ContentEncoding = Identity;

			if ( headers['accept-encoding'].indexOf('deflate') != -1 ) {

				respondeHeaders['Content-Encoding'] = 'deflate';
				var deflate = new Z(Z.DEFLATE);
				ContentEncoding = function(data) {

					return deflate(data);
				}
			}

			output( CreateHttpHeaders( 200, respondeHeaders ) );
			
			file.Open( File.RDONLY );

			output( function(chunks) {

				var data = file.Read(Z.idealInputLength);
				data = ContentEncoding(data);

				if ( data.length ) {

					chunks.push( data.length.toString(16) + CRLF, data, CRLF );
					return true; // continue
				}
				file.Close();
				return false;
			});
			
			output( '0' + CRLF + CRLF );
		}
	}
}




function Connection(s) {

	var _output = [];
	
	function ProcessOutput() {
	
		if ( _output.length ) {

			var dataInfo = _output[0];
			if ( dataInfo instanceof Function ) {
				var chunks = [];
				dataInfo(chunks) || _output.shift(); // ''exhausted'' function
				if ( chunks.length == 0 )
					return; // cannot shift here because the next item may be a Function
				_output = chunks.concat(_output);
			} 
			s.Send(_output.shift());
		} else
			delete(s.writable);
	}
	
	function Output( dataInfo ) {
		
		_output.push(dataInfo);
		s.writable = ProcessOutput;
	}



	var _input = '';

	function ProcessHeaders() {
	
		_input && Next('');
		s.readable = function() { 
			
			var data = s.Recv();
			Next( data );
			data.length || CloseConnection(s);
		}


		function Next(chunk) {

			_input += chunk;
			var eoh = _input.indexOf( CRLF+CRLF );				
			if ( eoh == -1 )
				return;
			var [status,headers] = ParseHeaders( _input.substr(0,eoh+4) );
			status.peerName = s.peerName;
			
			_input = _input.substr(eoh+4);
			if( status.method == 'POST' )
				ProcessBody( status, headers );
			else
				ProcessRequest( status, headers, Output )('');
		}
	}

	
	function ProcessBody( status, headers ) {
	
		var processRequest = ProcessRequest( status, headers, Output );
		var length = headers['content-length'];
		_input && Next('');

		s.readable = function() { 
			
			var data = s.Recv();
			Next( data );
			data.length || CloseConnection(s);
		}

		function Next(chunk) {

			_input += chunk;
	
			if ( length == undefined ) {
				
				processRequest(_input);
				_input = '';
				return;
			}

			if ( _input.length < length ) {
				
				length -= _input.length;
				processRequest(_input);
				_input = '';
			} else {
			
				processRequest(_input.substr(0,length));
				_input = _input.substr(length);
				processRequest();
				ProcessHeaders();
			}
		}
	}
	
	ProcessHeaders();
}



try {

	var serverSocket = new Socket();

	serverSocket.readable = function() { // after Listen, readable mean incoming connexion

		var clientSocket = serverSocket.Accept();
		Connection(clientSocket);
		list.push(clientSocket);
	}

	serverSocket.Listen( 80, undefined, 100 );
	list.push(serverSocket);
	for(;!endSignal;) {
		Poll(list,timeout.Next() || 500);
		timeout.Process();
	}
	Print('end.');

} catch ( ex if ex instanceof NSPRError ) {
	Print( ex.text + ' ('+ex.code+')', '\n' );
} catch(ex) {
	throw(ex);
}


/*

Key Differences between HTTP/1.0 and HTTP/1.1:
	http://www.research.att.com/~bala/papers/h0vh1.html

http 1.1 rfc
	http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html

*/