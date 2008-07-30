LoadModule('jsstd');
LoadModule('jsio');


var b = BString('123');

Print( b.concat(b,BString('456'),789,'abc') );


Halt();


Print( new String('abc') == new String('abc') );

Halt();


Stringify( { __proto__: Stream('1234') } );

Halt();


var buf = new Buffer('abcdefghi');


function myStream() {
	this.Read = function(amount) {
		
		return buf.Read(2);
	}
}

var stream1 = new myStream();

Print( Stringify(stream1).quote() );