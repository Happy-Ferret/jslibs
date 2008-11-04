LoadModule('jsstd');

Print( Id, '\n' );
this.Id = 123;
Print( Id, '\n' );



Halt(); ////////////////////////////////////////////////////////////////////////////////////////////////////////////


var code = "var db = {}; db.testFun = function() { return 123 }; Print( Test(db, 'testFun') )"
var func = new Function(code);
func();
//eval(code)



Halt(); ////////////////////////////////////////////////////////////////////////////////////////////////////////////

var b = new Blob('ABCDEF');
var s = 'ABCDEF'

Print( b.substr(-7,2) );

Halt();

Print( eval( new String('({add:123})') ) );

Halt();

b.prop1 = 11;
b.prop2 = 22;
b.prop3 = 33;
b.prop4 = 44;
b.prop5 = 55;
b.prop6 = 66;
b.prop7 = 77;

Print( b.concat, '\n' );


Print( b.replace, '\n' );

Print( b.replace('ab','cd'), '\n' );


Print( b.prop5, '\n' );

Print( b instanceof String, '\n' );

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
