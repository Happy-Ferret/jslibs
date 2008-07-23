LoadModule('jsstd');
LoadModule('jsio');


//Stringify( Stream('1234') );

Stringify( { __proto__: Stream('1234') } );

Halt();




Halt();


var buf = new Buffer('abcdefghi');


function myStream() {
	this.Read = function(amount) {
		
		return buf.Read(2);
	}
}

var stream1 = new myStream();

Print( Stringify(stream1).quote() );