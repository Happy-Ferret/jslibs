loadModule('jsstd');
loadModule('jsio');
loadModule('jsfastcgi');

function log(data) {
	
	var f = new File('c:\\myLog.txt');
	f.open( File.WRONLY | File.CREATE_FILE | File.APPEND );
	f.write(data);
	f.close();
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

	var fd = Descriptor.import(FCGI_LISTENSOCK_FILENO, Descriptor.DESC_PIPE);

	function responder( request, writer ) {
		
		writer('Content-type: text/html\r\n\r\n<html><body>test ok.</body></html>');
	}


	for (;;) { 
	
//		while( !IsReadable(fd, 100) );
		var data = fd.read(FCGI_HEADER_LEN);
		log( 'IsReadable:'+isReadable(fd)+' - headers (size:'+data.length+'):');
		if ( data.length == 0 ) {
			sleep(200);

//			fd.Close();
//			fd = Descriptor.Import(FCGI_LISTENSOCK_FILENO, Descriptor.DESC_PIPE);
			continue;
		}
		
		var header = parseHeader(data);
		log( header.toSource() + '\n' );
		switch ( header.type ) {
			case fcgi.GET_VALUES:
				break;
			case fcgi.BEGIN_REQUEST:
				var request = { params:{}, data:'' };
				requests[header.requestId] = request;
				parseBeginRequestBody( fd.read(header.contentLength), request );
				break;
			case fcgi.ABORT_REQUEST:
				break;
			case fcgi.PARAMS:
				parsePairs( fd.read(header.contentLength), requests[header.requestId].params );
				break;
			case fcgi.STDIN:
				if ( header.contentLength != 0 )
					requests[header.requestId].data += fd.read(header.contentLength);
				else {

					var status = responder( requests[header.requestId], function(data) {

						fd.write( makeHeader(fcgi.STDOUT, header.requestId, data.length) );
						fd.write( data );
					});
					fd.write( makeHeader(fcgi.STDOUT, header.requestId, 0) );
					var endBody = makeEndRequestBody( status||0, protocolStatus.REQUEST_COMPLETE );
					fd.write( makeHeader(fcgi.END_REQUEST, header.requestId, endBody.length) );
					fd.write( endBody );
					
					if ( !requests[header.requestId].keepConn )
						fd.close();
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

	var f = Descriptor.import(0, Descriptor.DESC_PIPE);
	var data = f.read();
	new File('c:\\myLog.txt').content = data;
	

} catch ( ex if ex instanceof IoError ) {
	
	new File('c:\\myLog.txt').content = 'IoError: ' + ex.text, '\n';

} catch( ex ) {
	throw ex;
}
*/

/*
loadModule('jsnspr');
loadModule('jsstd');
loadModule('jsfastcgi');

//new File('c:\\fcgi_process_log.txt').content += 'start\n';

while ( accept() >= 0 ) {
	
	try {
	
		exec(getParam('SCRIPT_FILENAME'));
	} catch(ex) {
	
		var errorMessage = ex.name + ': ' + ex.message + ' (' + ex.fileName + ':' + ex.lineNumber + ')';
		log( errorMessage );
		write( 'Status: 500\r\n\r\n' + errorMessage );
	}
}

new File('c:\\fcgi_process_log.txt').content += 'end\n';
*/