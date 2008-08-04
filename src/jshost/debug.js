LoadModule('jsstd');
LoadModule('jsio');



var b = new Blob();

Print( b.replace, '\n' );
Print( b.replace('ab','cd'), '\n' );


Halt();


b = b.concat( 'ABCD' );

Print( ''+b );


//Print( ''+b );


Halt();



//Stringify(new Buffer());
/*
var b = new Blob();
b._NI_BufferGet = 123;
Print( b._NI_BufferGet );
*/

var o = { _NI_BufferGet:123 }
Stringify(o);



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