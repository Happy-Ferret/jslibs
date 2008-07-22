({

	NativeInterface: function(QA) {

		var stream = new Stream('456');
		QA.ASSERT( !isNaN(stream._NI_StreamRead), true, 'NativeInterface system' )
	},

	NativeInterfaceSecurity: function(QA) {

		var stream = new Stream('456');
		var prev = stream._NI_StreamRead;
		stream._NI_StreamRead = 123456;
		QA.ASSERT( stream._NI_StreamRead, prev, 'NativeInterface security' )
		stream._NI_StreamRead = 987654;
		QA.ASSERT( stream._NI_StreamRead, prev, 'NativeInterface security' )
	},

	BString_construct_with_data: function(QA) {

		LoadModule('jsstd');
//		LoadModule('jslang');
		
		var bstr = new BString('98765');
		QA.ASSERT( String(bstr), '98765', 'string value' );
	},
	
	BString_not_constructed: function(QA) {
		
		LoadModule('jsstd');
//		LoadModule('jslang');

		var bstr = BString('98765');
		QA.ASSERT( String(bstr), '98765', 'string value' );


		var v = BString('123');
		QA.ASSERT( typeof v, 'object', 'variable type' );
		QA.ASSERT( String(v), '123', 'content' );
	},

	BString_to_string: function(QA) {

		LoadModule('jsstd');
//		LoadModule('jslang');
		
		QA.ASSERT( BString('345').toString(), '345', 'string value' );
	},

	BString_valueOf: function(QA) {

		LoadModule('jsstd');
//		LoadModule('jslang');

		QA.ASSERT( BString('567').valueOf(), '567', 'value of' );
	},

	BString: function(QA) {
		
		LoadModule('jsstd');
//		LoadModule('jslang');
	
		var b = new BString();
		b = b.Add( 'ABCD' );
		QA.ASSERT( String(b), 'ABCD', 'content' );
		QA.ASSERT( b.length, 4, 'length' );
		QA.ASSERT( b[0], 'A', 'first item' );
		QA.ASSERT( b[3], 'D', 'last item' );

//		QA.ASSERT( 2 in b, true, 'in operator' ); // cf. DEFINE_NEW_RESOLVE
		QA.ASSERT( b[2], 'C', '[] operator' );

//		QA.ASSERT( 4 in b, false, 'in operator' ); // cf. DEFINE_NEW_RESOLVE
		QA.ASSERT( b[4], undefined, '[] operator' );

		QA.ASSERT( b[4], undefined, 'after last item' );

		b = b.Add('XYZ');
		QA.ASSERT( b.length, 7, 'length' );
		QA.ASSERT( b[6], 'Z', 'last item' );
		QA.ASSERT( b.toString(), 'ABCDXYZ', 'toString' );
		QA.ASSERT( String(b.valueOf()), 'ABCDXYZ', 'valueof' );
//		b.Set();
//		QA.ASSERT( b.length, 0, 'length' );
	},

	BStringErrors: function(QA) {
	
		var b = new BString();
		b.Add( 'ABCD' );
		QA.ASSERT_EXCEPTION( function() b[5] = 'X', RangeError, 'set an out-of-range item' );
	},

	BStringSubstr: function(QA) {
	
		LoadModule('jsstd');
//		LoadModule('jslang');

		var b = new BString();
		b = b.Add( 'ABCDEF' );
		QA.ASSERT( ''+b.Substr(0), 'ABCDEF', 'substr' );
		QA.ASSERT( ''+b.Substr(1), 'BCDEF', 'substr' );
		QA.ASSERT( ''+b.Substr(2,3), 'CDE', 'substr' );
		QA.ASSERT( ''+b.Substr(-2,2), 'EF', 'substr' );
		QA.ASSERT( ''+b.Substr(-2,3), 'EF', 'substr' );
		QA.ASSERT( ''+b.Substr(0,6), 'ABCDEF', 'substr' );
		QA.ASSERT( ''+b.Substr(0,7), 'ABCDEF', 'substr' );
		QA.ASSERT( ''+b.Substr(0,-2), '', 'substr' );
		QA.ASSERT( ''+b.Substr(6), '', 'substr' );
		QA.ASSERT( ''+b.Substr(-6), 'ABCDEF', 'substr' );
		QA.ASSERT( ''+b.Substr(-7,2), '', 'substr' );
	},


	_BStringSetter: function(QA) { // this test do not have any sense with immutable objects
	
		LoadModule('jsstd');
//		LoadModule('jslang');

		var b = new BString();
		b.Add( 'ABCDEF' );
		b[0] = 'X';
		QA.ASSERT( ''+b.Substr(0,1), 'X', 'setter' );
		b[5] = 'W';
		QA.ASSERT( ''+b[5], 'W', 'setted value' );
		b[5] = 'W';
		QA.ASSERT( String(b), 'XBCDEW', 'setter' );
		QA.ASSERT_EXCEPTION( function() { b[-1] = 'Y'; }, Error, 'out of range' );
		QA.ASSERT_EXCEPTION( function() { b[6] = 'Z'; }, Error, 'out of range' );
	},
	
	
	BStringEquality: function(QA) {
		
		LoadModule('jsstd');
//		LoadModule('jslang');

		var b = new BString();
		b = b.Add( 'ABCDEF' );

		QA.ASSERT( b == 'ABCDEF', true, 'string and BString equality' )
		QA.ASSERT( 'ABCDEF' == b, true, 'string and BString equality' )

		QA.ASSERT( b === 'ABCDEF', false, 'string and BString equality and same type' )
	},


	_BStringSelfReference: function(QA) { // this test do not have any sense with immutable objects

		LoadModule('jsstd');
//		LoadModule('jslang');

		var a = new BString();
		a.Set();
		a.Set( 'ABCDEF' );
		a.Add(a);
		QA.ASSERT( ''+a, 'ABCDEFABCDEF', 'self add' )
	
		var b = new BString();
		b.Set( 'ABCDEF' );
		b.Add( '12345' );
		b.Set(b);
		QA.ASSERT( ''+b, 'ABCDEF12345', 'self set' )

		var c = new BString();
		c.Set('');
		c.Add(c);
		QA.ASSERT( ''+c, '', 'self add empty' )

		var d = new BString();
		d.Set('123456');
		d.Set(d.Substr(0,3));
		QA.ASSERT( ''+d, '123', 'self substr' )
	},

	SimpleStreamTest: function(QA) {	

		var bstring = new BString("1234567");
		var stream = Stream(bstring);
		QA.ASSERT( stream.source, bstring, 'source object' )

		QA.ASSERT( stream.position, 0, 'initial stream position' )
		QA.ASSERT( String(stream.Read(3)), '123', 'stream Read()' )
		QA.ASSERT( stream.position, 3, 'stream position after Read()' )
		QA.ASSERT( String(stream.Read(0)), '', 'read 0 bytes on the stream' )
		QA.ASSERT( stream.position, 3, 'stream position after Read(0)' )
		QA.ASSERT( String(stream.Read(100)), '4567', 'read more than the stream size' )
		QA.ASSERT( stream.position, 7, 'stream position after reading more than the stream size' )

		stream.position = 0;
		QA.ASSERT( String(stream.Read(bstring.length)), '1234567', 'read the exact length' )
	},

	StreamAdd: function(QA) {

		var bstring = new BString("1234");
		var stream = Stream(bstring);
		bstring = bstring.Add('ABC');
		QA.ASSERT( String(stream.Read(3)), '123', 'stream Read()' )
		bstring = bstring.Add('DEF');
		QA.ASSERT( bstring.length, 10, 'stream source length' )
		QA.ASSERT( String(stream.Read(3)), '4', 'stream Read()' )
		QA.ASSERT( stream.position, 4, 'stream position' )

		var s1 = Stream(bstring);
		QA.ASSERT( s1.position, 0, 'stream position' )
		QA.ASSERT( s1.available, 10, 'stream source length' )
		QA.ASSERT( String(s1.Read(3)), '123', 'stream Read()' )
	},

	
	JavascriptStream: function(QA) {
		
		var buf = new Buffer();
		buf.Write('abcdefghi');

		function myStream() {
		
			this.Read = function(amount) {
				
				return buf.Read(2);
			}
		}
		
		QA.ASSERT( Stringify( new myStream() ), 'abcdefghi', 'force string conversion' );
	},
	
	
	Stringify: function(QA) {
	
		QA.ASSERT( 'test', Stringify('test'), 'force string conversion' );
	}

	
})
