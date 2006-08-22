exec('deflib.js');
LoadModule('jsnspr');

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

function SendHttpResponse( s, status, headers, data ) {

	var statusText = { 200:'OK', 404:'NOT FOUND' };
	var buf = 'HTTP/1.1 ' + status + ' ' + statusText[status] + '\r\n';
	
	headers['Connection'] = 'Keep-Alive';
	headers['Keep-Alive'] = 'timeout=15, max=100';
	headers['Accept-Ranges'] = 'bytes';
	headers['Content-Length'] = data.length;
	for ( var [h,v] in headers )
		buf += h + ': ' + v + '\r\n';
	buf += '\r\n';
	s.Send( buf + data );
}


var list = [];

function Client(s) {

//	timeout.Add( 1000, function() { print('timeout-\n'); } );
	
	function closeConnexion() {

		print('localy close!\n');
		s.Close();
		list.splice( list.indexOf(s), 1 );
	}
	
	s.readable = function() {
		
//		delete s.readable;
 		var buf = s.Recv();
	  
		if ( buf.length == 0 ) {
		
			print('[remotly-closed]\n');
			s.Close();
			list.splice( list.indexOf(s), 1 );
			return;
		}

		var lines = buf.split('\r\n');
		var status = lines[0].split(' ');
		var method = status[0];
		var uri = status[1];
		var proto = status[2];

		print( method + ' ' + uri + '\n');

		s.writable = function() {
		
			delete s.writable;
			SendHttpResponse( s, 200, { 'Content-Type':'text/html' }, buf );
		}

	}
}

try {

	var serverSocket = new Socket();
	serverSocket.recvBufferSize = 10000;

	serverSocket.readable = function() { // after Listen, readable mean incoming connexion

		var clientSocket = serverSocket.Accept();
		var client = new Client(clientSocket);
		list.push(clientSocket);
	}



	serverSocket.Listen( '', 80 );
	list.push(serverSocket);
	for(;;) {
		Poll(list,timeout.Next() || 1000);
		timeout.Process();
	}

} catch ( ex if ex instanceof NSPRError ) { 
	print( ex.text + ' ('+ex.code+')', '\n' );
}