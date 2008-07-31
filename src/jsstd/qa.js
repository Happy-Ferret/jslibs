({

	StringRepeat: function(QA) {
			
		LoadModule('jsstd');

		QA.ASSERT( StringRepeat( '', 0 ), '', '0 x empty' );
		QA.ASSERT( StringRepeat( '', 1 ), '', '1 x empty' );
		QA.ASSERT( StringRepeat( '', 10 ), '', '10 x empty' );
		
		QA.ASSERT( StringRepeat( 'a', 0 ), '', '0 x 1 char' );
		QA.ASSERT( StringRepeat( 'a', 1 ), 'a', '1 x 1 char' );
		QA.ASSERT( StringRepeat( 'a', 10 ), 'aaaaaaaaaa', '10 x 1 char' );
		
		QA.ASSERT( StringRepeat( 'abc', 0 ), '', '0 x string' );
		QA.ASSERT( StringRepeat( 'abc', 1 ), 'abc', '1 x string' );
		QA.ASSERT( StringRepeat( 'abc', 10 ), 'abcabcabcabcabcabcabcabcabcabc', '10 x string' );
	},
	

	BufferAccess: function(QA) {

		var b = new Buffer();
		b.Write('');
		b.Write('123');
		b.Write('');
		b.Write('456');
		b.Write('');
		b.Write('789');
		b.Write('');
		
		var res = b[-1]+b[0]+b[1]+b[2]+b[3]+b[4]+b[5]+b[6]+b[7]+b[8]+b[9]+b[10];

		QA.ASSERT( res.length, 9, 'resulting length' );
		QA.ASSERT( res, '123456789', 'resulting string' );
		QA.ASSERT( b.toString(), res, 'resulting string' );
	},


	EmptyBuffer: function(QA) {

		LoadModule('jsstd');
	
		var b = new Buffer();
		QA.ASSERT( typeof b.Read(), 'string', 'empty buffer read' );

		QA.ASSERT( b.Read(), '', 'empty buffer read' );

		b.Write('a');
		QA.ASSERT( typeof b.Read(), 'object', 'buffer read' );
	
		b.Write('a');
		QA.ASSERT( b.Read() instanceof Blob, true, 'buffer read' );
	},


	BufferTest1: function(QA) {
		
		LoadModule('jsstd');

		var b = new Buffer();
		b.Read(0);
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
		b.Read(0);
		var t = b.Read();

		QA.ASSERT_STR( t, 'eeeeffffgggg', 'buffer match' );
	},

	BufferTest2: function(QA) {
		
		LoadModule('jsstd');

		var b = new Buffer();
		b.Write("abcdefghi");
		b.Read(2);
		b.Read(2);
		b.Read(2);
		b.Read(2);
		QA.ASSERT_STR( b.Read(2), 'i', 'buffer match' );
	},

	BufferUnderflow: function(QA) {
		
		LoadModule('jsstd');
		
		var times = 0;
		var toto = 'Zz';

		var buf = new Buffer({ Read:function(count) { times++; buf.Write(toto) } });

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

		QA.ASSERT( buf.length, 10, 'length before read 30' );

		QA.ASSERT_STR( buf.Read(30), '12345X6789ZzZzZzZzZzZzZzZzZzZz', 'Read(30)' );

		QA.ASSERT( buf.length, 0, 'length after read 30' );

		QA.ASSERT( times, 10, 'times .source stream has been called' );

		buf.Write('1234');
		buf.Write('5');
		buf.Write('');
		buf.Write('6789');
		
		QA.ASSERT( buf.Read(undefined), '1234', 'undefined read' );
		QA.ASSERT_STR( buf.Read(7), '56789Zz', 'read(7)' );
		QA.ASSERT_STR( buf.Read(1), 'Z', 'read(1)' );
		QA.ASSERT_STR( buf.Read(), 'z', 'read()' );
	},



	BufferCopy: function(QA) {

		var b1 = new Buffer();
		b1.Write('1');
		b1.Write('');
		b1.Write('1');
		b1.Write('1');

		QA.ASSERT( b1.length, 3, 'source buffer length' );

		var b2 = new Buffer();
		b2.Write('aaa');

		b2.Write(b1);
		QA.ASSERT( b1.length, 3, 'source buffer length' );

		b2.Write('bbb');
		b2.Write(b1);
		QA.ASSERT( b1.length, 3, 'source buffer length' );

		QA.ASSERT_STR( b2.toString(), 'aaa111bbb111', 'buffer content' );	
		
		QA.ASSERT_STR( b1.toString(), '111', 'source buffer content' );	
	},
	
	BufferSimpleRead: function(QA) {
	
		var b = new Buffer();
		b.Write('123');
		QA.ASSERT_STR( b.Read(1), '1', 'buffer read' );	
	},


	Buffer_toString: function(QA) {
	
		var b = new Buffer();
		b.Write('12345');
		b.Write('678');
		b.Write('9');
		b.Write('');
		QA.ASSERT_STR( b, '123456789', 'buffer toString' );	
	},


	Buffer_toString_consumption: function(QA) {

		var b = new Buffer();
		b.Write('123');
		
		var str1 = b.toString();
		var str2 = b.toString();

		QA.ASSERT( str2.length, str1.length, 'toString Buffer consumption' );
		QA.ASSERT( str1, '123', 'first toString result' );
		QA.ASSERT( str2, '123', 'second toString result' );
	},
	
	Buffer_valueOf: function(QA) {
	
		var b = new Buffer();
		b.Write('12345');
		b.Write('678');
		b.Write('9');
		b.Write('');
		QA.ASSERT_STR( b.valueOf(), '123456789', 'buffer valueOf' );	
	},
	

	Buffer_valuesStore: function(QA) {
	
		var b = new Buffer();
		b.Write({ toString:function(){return '01'} });
		b.Write([2,3]);
		b.Write(4567);
		b.Write(8.9);
		QA.ASSERT_STR( b.Read(), '012,345678.9', 'buffer read stored values' );	
	},


	Buffer_valuesStore: function(QA) {
	
		var b = new Buffer();
		b.Write({ toString:function(){return '01'} });
		b.Write([2,3]);
		b.Write(4567);
		b.Write(8.9);
		QA.ASSERT_STR( b.Read(3), '012', 'buffer read stored values' );	
		QA.ASSERT_STR( b.Read(3), ',34', 'buffer read stored values' );	
		QA.ASSERT_STR( b.Read(2), '56', 'buffer read stored values' );	
		QA.ASSERT_STR( b.Read(3), '78.', 'buffer read stored values' );	
		QA.ASSERT_STR( b.Read(1), '9', 'buffer read stored values' );	
	},

	Buffer_andBlob: function(QA) {

		var str = new Blob('123');
		var b = new Buffer();
		b.Write(str);
		str.concat('456');
		b.Write('7');
		QA.ASSERT_STR( b.Read(), '1237', 'buffer containing a Blob' );
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
	
		var buf = new Buffer();
		buf.Write('xxx');

		buf.Write('aaa');
		buf.Write('bb1');
		buf.Write('14ccc');
		buf.Write('buffer2');
		QA.ASSERT_STR( buf.ReadUntil('114'), 'xxxaaabb', 'ReadUntil' );
		QA.ASSERT( typeof buf, 'object', 'buffer type' );
		QA.ASSERT_STR( buf, 'cccbuffer2', 'remaining' );
	},


	BufferMisc: function(QA) {

		var buf = new Buffer();
		buf.Write('12345');
		buf.Write('6');
		buf.Write('');
		buf.Write('789');
		QA.ASSERT_STR( buf.Read(2), '12', 'Read(int)' );
		buf.Clear();
		QA.ASSERT( buf.length, 0, 'empty length' );
		QA.ASSERT( buf.Read(), '', 'empty content' );
		buf.Write('4');
		buf.Write('45');
		QA.ASSERT( buf.length, 3, 'content length' );
	},
	
	BufferMissingUnRoot: function(QA) {

		var buf = new Buffer();
		buf.Write('1234');
		buf.Read(4);
	},
	
	BufferSource: function(QA) {

		var buf = new Buffer(Stream('456'));
		buf.Write('123');


		QA.ASSERT( buf.length, 3, 'length' );
		QA.ASSERT_STR( buf.Read(6), '123456', 'read' );

		var buf1 = new Buffer(Stream('456'));
		buf1.Write('123');
		
		QA.ASSERT( buf1.length, 3, 'length' );
		QA.ASSERT_STR( buf1.Read(6), '123456', 'read' );

		var buf2 = new Buffer({
			Read: function(count) { return StringRepeat('x',count) }
		});
		buf2.Write('123');
		QA.ASSERT( buf2.length, 3, 'length' );
		QA.ASSERT_STR( buf2.Read(6), '123xxx', 'read' );


		var buf3 = new Buffer({
			Read: function(count) { buf3.Write( StringRepeat('x',count) ) }
		});
		buf3.Write('123');
		QA.ASSERT( buf3.length, 3, 'length' );
		QA.ASSERT_STR( buf3.Read(6), '123xxx', 'read' );
	},

	PackEndianTest: function(QA) {
		
		LoadModule('jsstd');
	
		if ( Pack.systemBigEndian == false ) {
		
			var buf = new Buffer();
			var pack = new Pack(buf);
			var v = 0x71727374;
			pack.WriteInt(v, 4, false);
			QA.ASSERT_STR( buf.Read(), 'tsrq', 'check stored data' );
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
		QA.ASSERT_STR( pack.ReadString(4), "\xAA\xBB\xCC\xDD", 'ReadString' );
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

		QA.ASSERT( Expand('', { h:'Hello', w:'World' }), '', 'expanding an empty string' );

		QA.ASSERT( Expand('Hello World'), 'Hello World', 'expanding a simple string' );

		QA.ASSERT( Expand(' $(h) $(w)', { h:'Hello', w:'World' }), ' Hello World', 'expanding a string' );

		QA.ASSERT( Expand(' $(h) $(w', { h:'Hello', w:'World' }), ' Hello ', 'expanding a bugous string' );

		QA.ASSERT( Expand(' $(h) $(', { h:'Hello', w:'World' }), ' Hello ', 'expanding a bugous string' );

		QA.ASSERT( Expand(' $(h) $', { h:'Hello', w:'World' }), ' Hello $', 'expanding a string' );

		QA.ASSERT( Expand(' $(h)', { h:'Hello', w:'World' }), ' Hello', 'expanding a string' );
		
		var obj = {
			toString: function() {
				return 'Hello';
			}
		}
		QA.ASSERT( Expand('$(h)', { h:obj }), 'Hello', 'expanding a string' );
		
		var obj1 = {
			
			Expand:Expand,
			x:123,
			y:'456',
		}
		QA.ASSERT( obj1.Expand('$(x)$(y)'), '123456', 'expanding a string using this as map' );
		
		var o = { title:'My HTML Page', titi:1234, toString:function() { return Expand( this.text, this ) } };
		o.text = '<html><title>$(title)</title>\n'
		QA.ASSERT_STR( o, '<html><title>My HTML Page</title>\n', 'expand string using this object' );
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

		var str1 = StringRepeat('y', 100);
		QA.ASSERT( [ c for each ( c in str1 ) if (c == 'y') ].length, 100, 'all chars are good' );
		
		var str = StringRepeat('x', 10000);
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
		QA.ASSERT_STR( t, "\n\t\tthis is\n\t\ta multiline\n\n\t\ttext\n\t\t", 'text' );
	}
	
	
})
