LoadModule('jsfastcgi');

var cgiVariables = [
"SERVER_SOFTWARE",
"SERVER_NAME",
"GATEWAY_INTERFACE",
"SERVER_PROTOCOL",
"SERVER_PORT",
"REQUEST_METHOD",
"PATH_INFO",
"PATH_TRANSLATED",
"SCRIPT_NAME",
"QUERY_STRING",
"REMOTE_HOST",
"REMOTE_ADDR",
"AUTH_TYPE",
"REMOTE_USER",
"REMOTE_IDENT",
"CONTENT_TYPE",
"CONTENT_LENGTH" ];


while (Accept() >= 0) {

	Write( "Content-type: text/plain\r\n\r\n" ); // cgi response: http://www.ietf.org/rfc/rfc3875

	Write('CGI/1.1 variables:\n');
	for ( var i in cgiVariables )
		Write( '    ' + cgiVariables[i] + ' = ' + GetParam(cgiVariables[i]) + '\n' );
	Write('HTTP_* variables:\n');
	Write( '    ' + 'HTTP_USER_AGENT = ' + GetParam('HTTP_USER_AGENT') + '\n' );

	Write('Data:\n');
	var str;
	Write('[');
	while ( str = Read(1024) )
		Write( str );
	Write(']');
}


/*
LoadModule('jsstd');
LoadModule('jsnspr');
LoadModule('jsfastcgi');


//var p = ParsePairs( MakePairs({aaa:123444, bbb:'test'}) ); // test
//Print( p.bbb.length );
//throw 0;

var buffer = new Buffer();
buffer.Write( new File('test.txt').content );

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
	RESPONDER       :1,
	FCGI_AUTHORIZER :2,
	FCGI_FILTER     :3
};

var protocolStatus = {
	REQUEST_COMPLETE :0,
	CANT_MPX_CONN    :1,
	OVERLOADED       :2,
	UNKNOWN_ROLE     :3
};

var stdinBuffer = new Buffer();

for ( var i = 0; i < 4; i++ ) { 

	var header = ParseHeader( buffer.Read(8) ); // FCGI_HEADER_LEN
	Print( 'type:',header.type, '\n' );
	Print( 'id:',header.requestId, '\n' );
	switch ( header.type ) {
		case fcgi.GET_VALUES:
			break;
		case fcgi.BEGIN_REQUEST:
			ParseBeginRequestBody( buffer.Read(header.contentLength), header );
			break;
		case fcgi.ABORT_REQUEST:
			break;
		case fcgi.PARAMS:
			header.param || (header.param={});
			ParsePairs( buffer.Read(header.contentLength), header.param );
			break;
		case fcgi.STDIN:
			stdinBuffer.Write( buffer.Read(header.contentLength) );
			break;
	}
	Print( '[remain ' + buffer.length + ']', '\n' );
}


//MakeHeader( fcgi.STDOUT, id, length );




// http://www.fastcgi.com/devkit/doc/fcgi-spec.html


/*

LoadModule('jsnspr');

var data = File.stdin.Read(10000); // spec: ... FCGI_LISTENSOCK_FILENO equals STDIN_FILENO.
var result;

//var hex = [];
//for each ( var c in data )
//	hex.push( '0x'+c.charCodeAt(0).toString(16) );
//result = 'char fcgidata[] = { '+(hex.join(', '))+' };';

var hex = [];
for each ( var c in data ) {

	var h = c.charCodeAt().toString(16);
	hex.push( '\\x'+(h.length==1?'0':'')+h );
}
result = 'var fcgidata = \''+(hex.join(''))+'\';';


new File('C:\\PERSO\\fcgitest\\test.txt').content = result;

*/

