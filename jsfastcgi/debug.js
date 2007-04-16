LoadModule('jsstd');
LoadModule('jsnspr');
LoadModule('jsfastcgi');

var buffer = new Buffer();
buffer.Write( new File('test.txt').content );


var stdinBuffer = new Buffer();

for ( var i = 0; i < 4; i++ ) { 

	var header = ParseHeader( buffer.Read(8) );
	Print( 'type:',header.type, '\n' );
	Print( 'id:',header.requestId, '\n' );
	switch ( header.type ) {
		case 1: // FCGI_BEGIN_REQUEST
			ParseBeginRequest( buffer.Read(header.contentLength), header );
			break;
		case 2: // FCGI_ABORT_REQUEST
			break;
		case 4: // FCGI_PARAMS
			header.param || (header.param={});
			ParseParams( buffer.Read(header.contentLength), header.param );
			break;
		case 5: // FCGI_STDIN
			stdinBuffer.Write( buffer.Read(header.contentLength) );
			break;
	}
	Print( '[remain ' + buffer.length + ']', '\n' );
}


MakeStdoutHeader( id, length );



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

