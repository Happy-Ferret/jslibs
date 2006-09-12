exec('deflib.js');
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


function SendHttpHeaders( s, status, headers ) {

	var buf = 'HTTP/1.1' + SP + status + SP + statusText[status] + CRLF;
	
	for ( var [h,v] in headers )
		buf += h + ': ' + v + CRLF;
	s.Send( buf + CRLF );
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



function SocketWriterHelper(s,starved) {
	
	var _chunkQueue = [];

	function feed() {
	
		for ( var i=0; i<arguments.length; i++ )
			_chunkQueue.push(arguments[i]);
	}
	
	function Next() {
		
		if (!starved(feed))
			Next = function() {
				delete s.writable;
				CloseConnection(s);
			}
	}
	
	s.writable = function(s) {
		
		if ( _chunkQueue.length ) {
			var data = _chunkQueue.shift();
			log.Write( data );
			s.Send(data);
		} else
			Next();
	}
}


function Connection(s) {

	var status,headers;

	function ProcessRequest( status, headers, body ) {
	
		var file = new File( root + status.path );
		if ( !file.exist ) {
		
			var message = 'file not found';
			SendHttpHeaders( s, 404, {'Content-Length':message.length, 'Content-Type':'text/plain'} );
			s.Send(message);
			return;
		}		
		
		file.Open( File.RDONLY );

		var respondeHeaders = {};
		respondeHeaders['Content-Type'] = mimeType[status.path.substr(status.path.lastIndexOf(DOT)+1)] || 'text/html; charset=iso-8859-1';
//		respondeHeaders['Content-Length'] = file.available;
		respondeHeaders['Transfer-Encoding'] = 'chunked';
		respondeHeaders['Content-Encoding'] = 'deflate';
		respondeHeaders['Connection'] = 'Keep-Alive';
		SendHttpHeaders( s, 200, respondeHeaders );

		var deflate = new Z(Z.DEFLATE);

		SocketWriterHelper( s, function(write) {

			var data = file.Read(Z.idealInputLength);
			data = deflate(data);

			if ( data.length ) {
				write( data.length.toString(16) + CRLF, data, CRLF );
				return true;
			}
			write( '0' + CRLF + CRLF );
			file.Close();
			return false;
		});
	}

	var _buffer = '';
	var _state = ReadHeaders;
	
	s.readable = function() {

		_buffer += s.Recv();
		_state();
		if ( s.connectionClosed )
			CloseConnection(s);
	}	
	
	function ReadHeaders() {
		
		_state = ReadHeaders;

		var eoh = _buffer.indexOf( CRLF+CRLF );
		if ( eoh == -1 )
			return;

		[status,headers] = ParseHeaders( _buffer.substr(0,eoh+4) );
		_buffer = _buffer.substr(eoh+4);

		status.peerName = s.peerName;
		
		switch (status.method) {
			case 'GET':
				ProcessRequest( status, headers, '' );
				break;
				
			case 'POST':
				var connection = headers['connection'];
				if ( connection == 'close' )
					ReadBodyClose();
				else
					ReadBodyLength();
				break;
		}
	}

	function ReadBodyClose() {
		
		_state = ReadBodyClose;

		if ( !s.connectionClosed )
			return;
		CloseConnection(s);
		ProcessRequest( status, headers, _buffer );
	}

	function ReadBodyLength() {

		_state = ReadBodyLength;

		var length = Number(headers['content-length']);
		
		if ( _buffer.length < length )
			return;

		ProcessRequest( status, headers, _buffer.substring(0,length) );
		_buffer = _buffer.substring(length);
		ReadHeaders();
	}
}




try {

	var serverSocket = new Socket();

	serverSocket.readable = function() { // after Listen, readable mean incoming connexion

		var clientSocket = serverSocket.Accept();
//		clientSocket.noDelay = true;
		Connection(clientSocket);
		list.push(clientSocket);
	}

	serverSocket.Listen( 80, undefined, 10 );
	list.push(serverSocket);
	for(;!endSignal;) {
		Poll(list,timeout.Next() || 500);
		timeout.Process();
	}
	print('end.');

} catch ( ex if ex instanceof NSPRError ) {
	print( ex.text + ' ('+ex.code+')', '\n' );
} catch(ex) {
	throw(ex);
}


/*

Key Differences between HTTP/1.0 and HTTP/1.1:
	http://www.research.att.com/~bala/papers/h0vh1.html

http 1.1 rfc
	http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html

*/