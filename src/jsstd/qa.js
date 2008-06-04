({


	BufferTest1: function(QA) {
		
		LoadModule('jsstd');

		var b = new Buffer();
		b.Write('aaa');
		b.Write('');
		b.Write('bbXb');
		b.Write('');
		b.Write('ccc');
		b.Read(2);
		b.ReadUntil('X');
		b.Skip(2);
		b.Match('cc');
		b.Unread('ddd');
		b.Match('ddd');
		b.Match('ZZZ');
		b.ReadUntil('ZZZ');
		b.Read(1000);
		b.Write('eeee');
		b.Write('ffff');
		b.Write('gggg');
		var t = b.Read();

		QA.ASSERT( t, 'eeeeffffgggg', 'buffer match' );
	},


	BufferUnderflow: function(QA) {
		
		LoadModule('jsstd');

		var toto = 'Zz';

		var buf = new Buffer();
		buf.onunderflow = function(buf) { buf.Write(toto) }
		buf.Write('1234');
		buf.Write('5');
		buf.Write('');
		buf.Write('6789');

		QA.GC();
		var s = buf.Read(5);
		s += 'X'
		QA.GC();
		buf.Unread(s);
		s += 'Y'
		QA.GC();

		QA.ASSERT( buf.Read(30), '12345X6789ZzZzZzZzZzZzZzZzZzZz', 'Read(30)' );

		buf.Write('1234');
		buf.Write('5');
		buf.Write('');
		buf.Write('6789');
		
		QA.ASSERT( buf.Read(undefined), '1234', 'undefined read' );
		QA.ASSERT( buf.Read(7), '56789Zz', 'read(7)' );
		QA.ASSERT( buf.Read(1), 'Z', 'read(1)' );
		QA.ASSERT( buf.Read(), 'z', 'read()' );
	},


	BufferSimpleRead: function(QA) {
	
		var b = new Buffer();
		b.Write('123');
		QA.ASSERT( b.Read(1), '1', 'buffer read' );	
	},


	BufferIndexOf: function(QA) {
	
		var b = new Buffer();
		b.Write('abcd');
		b.Write('e');
		b.Write('');
		b.Write('');
		b.Write('fghij');
		QA.ASSERT( b.IndexOf('def'), 3, 'buffer read' );	
	},
	
	
	BufferReadUntil: function(QA) {
	
		var buf = new Buffer('xxx');
		buf.Write('aaa');
		buf.Write('bb1');
		buf.Write('14ccc');
		var buf2 = new Buffer(buf);
		buf2.Write('buffer2');
		QA.ASSERT( buf2.ReadUntil('114'), 'xxxaaabb', 'ReadUntil' );
		QA.ASSERT( typeof buf2, 'object', 'buffer type' );
		QA.ASSERT( String(buf2), 'cccbuffer2', 'remaining' );
	},


	BufferMisc: function(QA) {

		var buf = new Buffer();
		buf.Write('12345');
		buf.Write('6');
		buf.Write('');
		buf.Write('789');
		QA.ASSERT( buf.Read(2), '12', 'Read(int)' );
		buf.Clear();
		QA.ASSERT( buf.length, 0, 'empty length' );
		QA.ASSERT( buf.Read(), '', 'empty content' );
		buf.Write('4');
		buf.Write('45');
		QA.ASSERT( buf.length, 3, 'content length' );
	},


	PackEndianTest: function(QA) {
		
		LoadModule('jsstd');
	
		if ( Pack.systemBigEndian == false ) {
		
			var buf = new Buffer();
			var pack = new Pack(buf);
			var v = 0x71727374;
			pack.WriteInt(v, 4, false);
			QA.ASSERT( buf.Read(), 'tsrq', 'check stored data' );
		} else {
		
			report('this test is missing');
		}
	},


	PackReadInt: function(QA) {

		var buf = new Buffer();
		buf.Write('\xAA\xBB\xCC\xDD');
		var pack = new Pack(buf);
		QA.ASSERT( pack.ReadInt(4, false, true).toString(16), 'aabbccdd', 'ReadInt' );
	},	
	
	
	PackReadString: function(QA) {

		var buf = new Buffer();
		buf.Write('\xAA\xBB\xCC\xDD');
		var pack = new Pack(buf);
		QA.ASSERT( pack.ReadString(4), "\xAA\xBB\xCC\xDD", 'ReadString' );
	},


	Pack: function(QA) {

		LoadModule('jsstd');

		var buf = new Buffer();
		var pack = new Pack(buf);

		var v = 12345678;

		pack.WriteInt(v, 4, true);
		QA.ASSERT( pack.buffer.length, 4, 'buffer length' );
		QA.ASSERT( v, pack.ReadInt(4, true), 'data validity' );

		pack.WriteInt(v, 4);
		QA.ASSERT( v, pack.ReadInt(4, false), 'data validity' );

		pack.WriteInt(v, 4, false);
		QA.ASSERT( v, pack.ReadInt(4), 'data validity' );

		v = 65432;
		pack.WriteInt(v, 2);
		QA.ASSERT( v, pack.ReadInt(2), 'data validity' );
	},
	
	GarbageCollector: function(QA) {
		
		LoadModule('jsdebug');
		LoadModule('jsstd');
		
		var str = QA.RandomString(1024*1024);
		
		disableGarbageCollection = true;
		CollectGarbage();
	
		for ( var i = 0; i < 4; i++ )
			str += str;
			
//		QA.ASSERT( gcBytes, str.length, 'lot of allocated memory' );
		
		CollectGarbage();
	},
	
	IdOf: function(QA) {
		
		var t1 = IdOf('uroiquwyeroquiwyeroquwyeroquwyeroquwreyouqwhelqwfvqlwefvlqwhjvlqwefjvlqw12412h5oiu2f34hovcu312gofuiv3124gfovi23gfvo23y4gfov234yugfvo23ufgvwperiughweoigh23oi7o2h3vg7o2374o23g74hgo32i74gho23i7ghov237ihg');
		var xx = '23y2378vg239784gf293v87gfv293874gfv932847gfv';
		var t2 = IdOf('uroiquwyeroquiwyeroquwyeroquwyeroquwreyouqwhelqwfvqlwefvlqwhjvlqwefjvlqw12412h5oiu2f34hovcu312gofuiv3124gfovi23gfvo23y4gfov234yugfvo23ufgvwperiughweoigh23oi7o2h3vg7o2374o23g74hgo32i74gho23i7ghov237ihg');
		QA.ASSERT( t1, t2, 'IdOf on string' );
		QA.ASSERT( IdOf(32), 65, 'IdOf on integer' );
		
		var o = {};
		var p = {};
		QA.ASSERT( IdOf(o.constructor), IdOf(o.constructor), 'object constructor index' );
	},
	
	
	HideProperties: function(QA) {
	
		var o = { a:1, b:2, c:3, d:4 };
		HideProperties(o, 'b', 'c');
		QA.ASSERT( o.b, 2, 'do not delete' );
		QA.ASSERT( [p for each (p in o)].join(','), '1,4', 'visible properties' );
	},
	
	SetScope: function(QA) {

		var data = 55;
		function bar() { QA.ASSERT( data, 7, 'modified scope' ); }
		var old = SetScope( bar, {data:7, QA:QA} );
		bar();
	},
	
	
	Expand: function(QA) {
	
		QA.ASSERT( Expand(' $(h) $(w)', { h:'Hello', w:'World' }), ' Hello World', 'expanding a string' );
	},
	
	ExpandToString: function(QA) {

		var o = { title:'My HTML Page', titi:1234, toString:function() { return Expand( this.text, this ) } };
		o.text = '<html><title>$(title)</title>\n'
		QA.ASSERT( String(o), '<html><title>My HTML Page</title>\n', 'expand string' );
	},

	ExecXDR: function(QA) {
		
		LoadModule('jsio');
	
		var f = new File('qa_exec_test.js');
		f.content = '(1234)';
		
		var res = Exec(f.name, true);
		QA.ASSERT( res, 1234, 'Exec return value' );

		var fxdr = new File('qa_exec_test.jsxdr');
		QA.ASSERT( fxdr.exist, true, 'XDR file exist' );
		
		f.Delete();

		QA.ASSERT( f.exist, false, 'do not have source file' );

		var res = Exec(f.name, true);
		QA.ASSERT( res, 1234, 'Exec using XDR file' );

		fxdr.Delete();
		QA.ASSERT( fxdr.exist, false, 'XDR file is deleted' );
		
		try {
			
			var res = Exec(f.name, false);
			QA.FAILED('Exec do not detect missing file');
			
		} catch(ex) {
			
			QA.ASSERT( ex.constructor, Error, 'Exec exception' );
			QA.ASSERT( ex.message.substr(0,21), 'Unable to load Script', 'error message' );
		}
	},
	
	
	Clear: function(QA) {
	
		var o = { x:5, y:6, z:7 };
		QA.ASSERT( 'z' in o, true, 'has z property' );
		Clear(o);
		QA.ASSERT( 'z' in o, false, 'property z is cleared' );
	},


	ClearOnAnArray: function(QA) {
	
		var o = { x:5, y:6, z:7 };
		QA.ASSERT( 'z' in o, true, 'has z property' );
		Clear(o);
		QA.ASSERT( 'z' in o, false, 'property z is cleared' );
	},
	

	Seal: function(QA) {
		
		var o = { a:1 };
		Seal(o);
		
		try {
			
			o.a = 123;
			QA.FAILED('seal do not work');
			
		} catch(ex) {
			
			QA.ASSERT( ex.constructor, Error, 'object access exception' );
			QA.ASSERT( ex.message, 'o.a is read-only', 'error message' );
		}
	},
	
	
	IsStatementValid: function(QA) {
		
		QA.ASSERT( IsStatementValid( 'for ( var i; i<10; i++ )' ), false, 'invalid statement' );
		QA.ASSERT( IsStatementValid( 'for ( var i; i<10; i++ );' ), true, 'valid statement' );
		QA.ASSERT( IsStatementValid( '{a,b,c} = { a:1, b:2, c:3 }' ), true, 'valid statement' );
	},

	
	StrChr: function(QA) {

		var str1 = StrSet('y', 100);
		QA.ASSERT( [ c for each ( c in str1 ) if (c == 'y') ].length, 100, 'all chars are good' );
		
		var str = StrSet('x', 10000);
		QA.ASSERT( str.length, 10000, 'string length' );
		QA.ASSERT( str[0], 'x', 'first char' );
		QA.ASSERT( str[9999], 'x', 'last char' );
	},
	
	
	MultiLineStringUsingE4X: function(QA) {
				
		var t = <text>
		this is
		a multiline

		text
		</text>
		
		QA.ASSERT( typeof t, 'xml', 'text type' );
		QA.ASSERT( String(t), "\n\t\tthis is\n\t\ta multiline\n\n\t\ttext\n\t\t", 'text' );
	}
	
	
	
})