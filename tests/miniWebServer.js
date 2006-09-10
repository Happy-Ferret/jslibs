exec('deflib.js');
LoadModule('jsnspr');
LoadModule('jsz');

const CRLF = '\r\n';
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
		_file.Write( t + ' : ' +data );
	}
	
	this.WriteLn = function( data ) {
		
		this.Write( data + '\n' );
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

var root='./miniWebServerRoot';

function SendHttpHeaders( s, status, headers ) {

	var buf = 'HTTP/1.1' + SP + status + SP + statusText[status] + CRLF;
	
	for ( var [h,v] in headers )
		buf += h + ': ' + v + CRLF;
	s.Send( buf + CRLF );
}


var list = [];

function Client(s) {

	var _buffer = '';
	var _status = { peerName:s.peerName };
	var _headers = {};

	function CloseConnexion() {

		s.Close();
		list.splice( list.indexOf(s), 1 );
	}

	function ParseHeaders(buffer) {
	
		var lines = buffer.split(CRLF);
		var [method,url,proto] = lines[0].split(SP);
		var [path,query] = url.split('?');
		for ( var i=1; lines[i].length; i++ ) {
			var [name,value] = lines[i].split(': ');
			_headers[name] = value;
		}
		_status.method = method;
		_status.url = url;
		_status.proto = proto;
		_status.path = path;
		_status.query = query;
		
		print( _status.toSource(), '\n' );
		
	}

	function ParseBody( data ) {

		if ( _status.method == 'GET' ) {


			var file = new File( root + _status.path );
			if ( file.exist ) {

				file.Open( File.RDONLY );
				var respondeHeaders = {};
				respondeHeaders['Content-Length'] = file.available;
				respondeHeaders['Connection'] = 'Keep-Alive';
				SendHttpHeaders( s, 200, respondeHeaders );

				s.writable = function() {

					var data = file.Read();
					if ( data.length == 0 ) {

						file.Close();
						delete s.writable;
						if ( respondeHeaders['Connection'] == 'Close' ) {

							list.splice( list.indexOf(s), 1 );
							s.Close();
						}
						return;
					}

					s.Send( data );
				}

			
			} else {

				var message = 'file not found';
				SendHttpHeaders( s, 404, {'Content-Length':message.length, 'Content-Type':'text/plain'} );
				s.Send(message);
			}
		}
	}

	s.readable = function() {
		
		var buf = s.Recv();
		if ( buf.length == 0 ) {
			CloseConnexion();
			return;
		}
		_buffer += buf;
		var eoh = _buffer.indexOf( CRLF+CRLF );
		if ( eoh == -1 ) 
			return;

		eoh += 4;
		ParseHeaders( _buffer.substr(0,eoh) );
		ParseBody( _buffer.substr(eoh) );
		s.readable = function() {

			var buf = s.Recv();
			if ( buf.length == 0 ) {
				CloseConnexion();
				return;
			}
			ParseBody( buf );
		}
	}
	
/*

		var deflate = ( headers['Accept-Encoding'].indexOf('deflate') != -1 );

//		SendHttpResponse( s, 200, { 'Content-Type':'text/html' }, buf );

		var file = new File( root + path );
		if ( file.exist ) {
		
			file.Open( File.RDONLY );

			var respondeHeaders = {};
			respondeHeaders['Content-Type'] = mimeType[path.substr(path.lastIndexOf(DOT)+1)] || 'text/html; charset=iso-8859-1';
//			respondeHeaders['Keep-Alive'] = 'timeout=15, max=100';
//			respondeHeaders['Accept-Ranges'] = 'bytes';
			
			if ( deflate ) {
				respondeHeaders['Content-Encoding'] = 'deflate';
				respondeHeaders['Connection'] = 'Close';
				var z = new Z(Z.DEFLATE,9);
			} else {
				respondeHeaders['Content-Length'] = file.available;
				respondeHeaders['Connection'] = 'Keep-Alive';
			}
			

			SendHttpHeaders( s, 200, respondeHeaders );

			s.writable = function() {
				
				var data = file.Read();

				if ( deflate )
					data = z( data );
				
				if ( data.length == 0 ) {

					file.Close();
					delete s.writable;
					if ( respondeHeaders['Connection'] == 'Close' ) {
						
						list.splice( list.indexOf(s), 1 );
						s.Close();
					}
					return;
				}
				
				s.Send( data );
			}
			
		} else {

			var message = 'file not found';
			SendHttpHeaders( s, 404, {'Content-Length':message.length, 'Content-Type':'text/plain'} );
			s.Send(message);
		}

	}
*/	
	
}

//try {

	var serverSocket = new Socket();

	serverSocket.readable = function() { // after Listen, readable mean incoming connexion

		var clientSocket = serverSocket.Accept();
		var client = new Client(clientSocket);
//		client.noDelay = true;
		list.push(clientSocket);
	}

	serverSocket.Listen( 80 );
	list.push(serverSocket);
	for(;!endSignal;) {
		Poll(list,timeout.Next() || 1000);
		timeout.Process();
	}
	print('end.');

//} catch ( ex if ex instanceof NSPRError ) { 
//	print( ex.text + ' ('+ex.code+')', '\n' );
//}