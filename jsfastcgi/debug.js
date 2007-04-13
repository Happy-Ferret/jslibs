LoadModule('jsstd');
LoadModule('jsnspr');
LoadModule('jsfastcgi');

var fcgidata = new File('test.txt').content;
var record = ParseRecord(fcgidata);

Print( record.contentLength, '\n' );

for each ( var i in record.params )
	Print( i, '\n' );


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

