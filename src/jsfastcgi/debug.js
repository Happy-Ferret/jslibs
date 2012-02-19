// loadModule('jsstd');  loadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { print( id, ':', uneval(args), '\n' ) } };  exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  Halt();

loadModule('jsstd');
loadModule('jsfastcgi');

getParam();
//write();
log();

print( URLDecode( 'a+a+a+a' ), '\n' );



/*
loadModule('jsstd');
loadModule('jsnspr');
loadModule('jsfastcgi');


//var p = ParsePairs( MakePairs({aaa:123444, bbb:'test'}) ); // test
//print( p.bbb.length );
//throw 0;

var buffer = new Buffer();
buffer.write( new File('test.txt').content );

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

	var header = parseHeader( buffer.read(8) ); // FCGI_HEADER_LEN
	print( 'type:',header.type, '\n' );
	print( 'id:',header.requestId, '\n' );
	switch ( header.type ) {
		case fcgi.GET_VALUES:
			break;
		case fcgi.BEGIN_REQUEST:
			parseBeginRequestBody( buffer.read(header.contentLength), header );
			break;
		case fcgi.ABORT_REQUEST:
			break;
		case fcgi.PARAMS:
			header.param || (header.param={});
			parsePairs( buffer.read(header.contentLength), header.param );
			break;
		case fcgi.STDIN:
			stdinBuffer.write( buffer.read(header.contentLength) );
			break;
	}
	print( '[remain ' + buffer.length + ']', '\n' );
}


//makeHeader( fcgi.STDOUT, id, length );




// http://www.fastcgi.com/devkit/doc/fcgi-spec.html


/*

loadModule('jsnspr');

var data = File.stdin.read(10000); // spec: ... FCGI_LISTENSOCK_FILENO equals STDIN_FILENO.
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

