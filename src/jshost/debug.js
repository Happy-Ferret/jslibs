LoadModule('jsstd');
LoadModule('jsio');


var stream = new Stream('456');
Print( stream._NI_StreamRead, '\n' )



Halt();


var buf = new Buffer('abcdefghi');


function myStream() {
	this.Read = function(amount) {
		
		return buf.Read(2);
	}
}

var stream1 = new myStream();

Print( Stringify(stream1).quote() );