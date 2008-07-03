LoadModule('jsstd');


		var buf2 = new Buffer('123');
		buf2.source = {
			Read: function(count) { return 'xcv' }
		}

		Print( buf2.Read(6) );



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