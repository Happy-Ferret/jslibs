LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jsfastcgi');

function log(data) {
	
	var f = new File('c:\\myLog.txt');
	f.Open( File.WRONLY | File.CREATE_FILE | File.APPEND );
	f.Write(data);
	f.Close();
}

	var fcgi = { 
		BEGIN_REQUEST     : 1,
		ABORT_REQUEST     : 2,
		END_REQUEST       : 3,
		PARAMS            : 4,
		STDIN             : 5,
		STDOUT            : 6,
		STDERR            : 7,
		DATA              : 8,
		GET_VALUES        : 9,
		GET_VALUES_RESULT :10,
		UNKNOWN_TYPE      :11
	};

	var role = {
		RESPONDER  :1,
		AUTHORIZER :2,
		FILTER     :3
	};

	var protocolStatus = {
		REQUEST_COMPLETE :0,
		CANT_MPX_CONN    :1,
		OVERLOADED       :2,
		UNKNOWN_ROLE     :3
	};

	const FCGI_HEADER_LEN = 8;
	const FCGI_LISTENSOCK_FILENO = 0;
	
	
	// FCGI_WEB_SERVER_ADDRS
	
	

	log( '\n------START------\n' );


	var requests = {};


// Typical Protocol Message Flow: http://www.fastcgi.com/devkit/doc/fcgi-spec.html#S3.5 


//	var fd = File.stdin;


try {

	var fd = Descriptor.Import(FCGI_LISTENSOCK_FILENO, Descriptor.DESC_PIPE);

	function Responder( request, Writer ) {
		
		Writer('Content-type: text/html\r\n\r\n<html><body>test ok.</body></html>');
	}


	for (;;) { 
	
//		while( !IsReadable(fd, 100) );
		var data = fd.Read(FCGI_HEADER_LEN);
		log( 'IsReadable:'+IsReadable(fd)+' - headers (size:'+data.length+'):');
		if ( data.length == 0 ) {
			Sleep(200);

//			fd.Close();
//			fd = Descriptor.Import(FCGI_LISTENSOCK_FILENO, Descriptor.DESC_PIPE);
			continue;
		}
		
		var header = ParseHeader(data);
		log( header.toSource() + '\n' );
		switch ( header.type ) {
			case fcgi.GET_VALUES:
				break;
			case fcgi.BEGIN_REQUEST:
				var request = { params:{}, data:'' };
				requests[header.requestId] = request;
				ParseBeginRequestBody( fd.Read(header.contentLength), request );
				break;
			case fcgi.ABORT_REQUEST:
				break;
			case fcgi.PARAMS:
				ParsePairs( fd.Read(header.contentLength), requests[header.requestId].params );
				break;
			case fcgi.STDIN:
				if ( header.contentLength != 0 )
					requests[header.requestId].data += fd.Read(header.contentLength);
				else {

					var status = Responder( requests[header.requestId], function(data) {

						fd.Write( MakeHeader(fcgi.STDOUT, header.requestId, data.length) );
						fd.Write( data );
					});
					fd.Write( MakeHeader(fcgi.STDOUT, header.requestId, 0) );
					var endBody = MakeEndRequestBody( status||0, protocolStatus.REQUEST_COMPLETE );
					fd.Write( MakeHeader(fcgi.END_REQUEST, header.requestId, endBody.length) );
					fd.Write( endBody );
					
					if ( !requests[header.requestId].keepConn )
						fd.Close();
					delete requests[header.requestId];
				}
				break;
		}			
	}
} catch ( ex ) {

	var errorMessage = ex.name + ': ' + ex.message + ' (' + ex.fileName + ':' + ex.lineNumber + ')';
	log(errorMessage);
}


/*
try {

	var f = Descriptor.Import(0, Descriptor.DESC_PIPE);
	var data = f.Read();
	new File('c:\\myLog.txt').content = data;
	

} catch ( ex if ex instanceof IoError ) {
	
	new File('c:\\myLog.txt').content = 'IoError: ' + ex.text, '\n';

} catch( ex ) {
	throw ex;
}
*/

/*
LoadModule('jsnspr');
LoadModule('jsstd');
LoadModule('jsfastcgi');

//new File('c:\\fcgi_process_log.txt').content += 'start\n';

while ( Accept() >= 0 ) {
	
	try {
	
		Exec(GetParam('SCRIPT_FILENAME'));
	} catch(ex) {
	
		var errorMessage = ex.name + ': ' + ex.message + ' (' + ex.fileName + ':' + ex.lineNumber + ')';
		Log( errorMessage );
		Write( 'Status: 500\r\n\r\n' + errorMessage );
	}
}

new File('c:\\fcgi_process_log.txt').content += 'end\n';
*/