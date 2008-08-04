LoadModule('jsstd');

var obj1 = { name:"Object 1", a:1,  b:2, c:3, d:4, e:5, f:6, g:7 };




function test() {

	var obj2 = new String();

	var x = obj1;

	Test( obj1, obj2 );

	Print( 'swap done.', "\n" );

//	Print( 'obj1.name=', obj1.name, "\n" );
//	Print( 'obj2.name=', obj2.name, "\n" );

}

var toto = function(){}
toto.prototype = obj1;
var t = new toto;

CollectGarbage();

test()

Print( 'obj1.length=', obj1.length, "\n" );


Halt();


var b = new Buffer();
b.Write('123');

b.toString();
Print( b.toString() )

Halt();


var b = new Buffer();
b.Write('');
b.Write('123');
b.Write('');
b.Write('456');
b.Write('');
b.Write('789');
b.Write('');

Print(   b[-1]+b[0]+b[1]+b[2]+b[3]+b[4]+b[5]+b[6]+b[7]+b[8]+b[9]+b[10]  , '\n' );


Halt();


var b1 = new Buffer();
b1.Write('1');
b1.Write('');
b1.Write('1');
b1.Write('1');

var b2 = new Buffer();
b2.Write('aaa');
b2.Write(b1);
b2.Write('bbb');
b2.Write(b1);

Print( b2);


Halt();

 function ReadFromFile() {

  Print('*** read from the file\n');
  return StringRepeat('x',5);
 }
 
 var buf = new Buffer({ Read:function() { buf.Write(ReadFromFile()); }})

 for ( var i=0; i<15; i++ )
  Print( buf.Read(1), '\n' )





Halt()


		var buf2 = new Buffer({
			Read: function(count) { return 'xcv' }
		});

		Print( buf2.length, '\n' );
		Print( buf2.Read(100), '\n' );
		Print( buf2.length, '\n' );



Halt();
		var stream = new Stream('456');
		
		Print( '_NI_StreamRead:', stream._NI_StreamRead, '\n' )
		

		var buf1 = new Buffer('123');
		buf1.source = stream;
		
		
		Print( buf1.Read(6) )



Halt()

/*
var p = new Pack(new Buffer());
p.buffer.source = new Buffer();
p.buffer.source.source = Stream('\x12\x34');
Print( (p.ReadInt(2, false, true)).toString(16) );

*/

		var toto = 'Zz';

		var buf = new Buffer();
//		buf.onunderflow = function(buf) { buf.Write(toto) }
		buf.source = { Read:function(count) { return toto } };
		buf.Write('1234');
		buf.Write('5');
		buf.Write('');
		buf.Write('6789');


		var s = buf.Read(5);
		s += 'X'

		buf.Unread(s);
		s += 'Y'


Print(	buf.Read(30) )

		buf.Write('1234');
		buf.Write('5');
		buf.Write('');
		buf.Write('6789');