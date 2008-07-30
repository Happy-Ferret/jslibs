({

	ReadOnlyGlobalClasses: function(QA) {

		var bstr = BString;
		BString = null;
		QA.ASSERT( BString, bstr, 'BString integrity' );

		var bstr = BString;
		BString = function() {}
		QA.ASSERT( BString, bstr, 'BString integrity' );

		var bstr1 = BString;
		delete global.BString;
		QA.ASSERT( BString, bstr1, 'BString integrity' );
	},

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

	BStringAPI: function(QA) {

		LoadModule('jsstd');
		
		QA.ASSERT( BString('123456').indexOf(''), 0, 'BString indexOf' )
		QA.ASSERT( BString('123456').indexOf('1'), 0, 'BString indexOf' )
		QA.ASSERT( BString('123456').indexOf('2'), 1, 'BString indexOf' )
		QA.ASSERT( BString('123456').indexOf('6'), 5, 'BString indexOf' )
		QA.ASSERT( BString('123456').indexOf('7'), -1, 'BString indexOf' )
		QA.ASSERT( BString('12341234').indexOf('2',1), 1, 'BString indexOf' )
		QA.ASSERT( BString('12341234').indexOf('2',2), 5, 'BString indexOf' )
		QA.ASSERT( BString('123').indexOf('3',-10), 2, 'BString indexOf' )
		QA.ASSERT( BString('123').indexOf('3',10), -1, 'BString indexOf' )
		QA.ASSERT( BString('1231234').indexOf('1234'), 3, 'BString indexOf' )
		QA.ASSERT( BString('12345').indexOf('12345'), 0, 'BString indexOf' )

		QA.ASSERT( BString('123').lastIndexOf('3', 100), 2, 'BString lastIndexOf' )
		QA.ASSERT( BString('123').lastIndexOf('3',-100), -1, 'BString lastIndexOf' )
		QA.ASSERT( BString('123456').lastIndexOf('123456'), 0, 'BString lastIndexOf' )
		QA.ASSERT( BString('12341234').lastIndexOf('2'), 5, 'BString lastIndexOf' )
		QA.ASSERT( BString('123').lastIndexOf('1',0), 0, 'BString lastIndexOf' )
		QA.ASSERT( BString('123456').lastIndexOf('12'), 0, 'BString lastIndexOf' )
		QA.ASSERT( BString('123456').lastIndexOf('56'), 4, 'BString lastIndexOf' )
		QA.ASSERT( BString('0123456789').lastIndexOf('567', 4), -1, 'BString lastIndexOf' )
		QA.ASSERT( BString('0123456789').lastIndexOf('56789', 5), 5, 'BString lastIndexOf' )
		QA.ASSERT( BString('12341234').lastIndexOf('1234'), 4, 'BString lastIndexOf' )
		QA.ASSERT( BString('12341234').lastIndexOf('1234', 4), 4, 'BString lastIndexOf' )
		QA.ASSERT( BString('12341234').lastIndexOf('1234', 3), 0, 'BString lastIndexOf' )
		
		QA.ASSERT( isNaN( BString('').charCodeAt(0) ), true, 'BString charCodeAt' )
		QA.ASSERT( BString('1').charCodeAt(0), 49, 'BString charCodeAt' )
		QA.ASSERT( BString('1').charCodeAt(), 49, 'BString charCodeAt' )
		QA.ASSERT( isNaN( BString('1').charCodeAt(-1) ), true, 'BString charCodeAt' )
		QA.ASSERT( isNaN( BString('1').charCodeAt(1) ), true, 'BString charCodeAt' )
		
		QA.ASSERT( BString('').charAt(0), '', 'BString charAt' )
		QA.ASSERT( BString('1').charAt(0), '1', 'BString charAt' )
		QA.ASSERT( BString('1').charAt(), '1', 'BString charAt' )
		QA.ASSERT( BString('1').charAt(-1), '', 'BString charAt' )
		QA.ASSERT( BString('1').charAt(1), '', 'BString charAt' )
		
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
		b = b.concat( 'ABCD' );
		QA.ASSERT( String(b), 'ABCD', 'content' );
		QA.ASSERT( b.length, 4, 'length' );
		QA.ASSERT( b[0], 'A', 'first item' );
		QA.ASSERT( b[3], 'D', 'last item' );

//		QA.ASSERT( 2 in b, true, 'in operator' ); // cf. DEFINE_NEW_RESOLVE
		QA.ASSERT( b[2], 'C', '[] operator' );

//		QA.ASSERT( 4 in b, false, 'in operator' ); // cf. DEFINE_NEW_RESOLVE
		QA.ASSERT( b[4], undefined, '[] operator' );

		QA.ASSERT( b[4], undefined, 'after last item' );

		b = b.concat('XYZ');
		QA.ASSERT( b.length, 7, 'length' );
		QA.ASSERT( b[6], 'Z', 'last item' );
		QA.ASSERT( b.toString(), 'ABCDXYZ', 'toString' );
		QA.ASSERT( String(b.valueOf()), 'ABCDXYZ', 'valueof' );
//		b.Set();
//		QA.ASSERT( b.length, 0, 'length' );
	},

	BStringErrors: function(QA) {
	
		var b = new BString();
		b.concat( 'ABCD' ); // ????
		QA.ASSERT_EXCEPTION( function() b[5] = 'X', Error, 'set an out-of-range item' );
	},


	BString_concat: function(QA) {

		var b = BString('123');
		var res = b.concat(b,BString('456'),789,'abc');
		QA.ASSERT_STR( res, '123123456789abc', 'BString concat' );
	},


	BString_substr: function(QA) {
	
		LoadModule('jsstd');
//		LoadModule('jslang');

		var b = new BString();
		b = b.concat( 'ABCDEF' );
		QA.ASSERT( ''+b.substr(0), 'ABCDEF', 'substr' );
		QA.ASSERT( ''+b.substr(1), 'BCDEF', 'substr' );
		QA.ASSERT( ''+b.substr(2,3), 'CDE', 'substr' );
		QA.ASSERT( ''+b.substr(-2,2), 'EF', 'substr' );
		QA.ASSERT( ''+b.substr(-2,3), 'EF', 'substr' );
		QA.ASSERT( ''+b.substr(0,6), 'ABCDEF', 'substr' );
		QA.ASSERT( ''+b.substr(0,7), 'ABCDEF', 'substr' );
		QA.ASSERT( ''+b.substr(0,-2), '', 'substr' );
		QA.ASSERT( ''+b.substr(6), '', 'substr' );
		QA.ASSERT( ''+b.substr(-6), 'ABCDEF', 'substr' );
		QA.ASSERT( ''+b.substr(-7,2), '', 'substr' );
	},


	_BStringSetter: function(QA) { // this test do not have any sense with immutable objects
	
		LoadModule('jsstd');
//		LoadModule('jslang');

		var b = new BString('ABCDEF');
		b[0] = 'X';
		QA.ASSERT( ''+b.substr(0,1), 'X', 'setter' );
		b[5] = 'W';
		QA.ASSERT( ''+b[5], 'W', 'setted value' );
		b[5] = 'W';
		QA.ASSERT( String(b), 'XBCDEW', 'setter' );
		QA.ASSERT_EXCEPTION( function() { b[-1] = 'Y'; }, Error, 'out of range' );
		QA.ASSERT_EXCEPTION( function() { b[6] = 'Z'; }, Error, 'out of range' );
	},
	
	BString_boolean: function(QA) {
	
		QA.ASSERT( !!BString(''), true, 'empty BString cast to boolean' );
		QA.ASSERT( !!BString('x'), true, 'non-empty BString cast to boolean' );
	
//		QA.ASSERT( !!BString(''), !!(''), 'empty BString cast to boolean' );
//		QA.ASSERT( !!BString('a'), !!('a'), 'empty BString cast to boolean' );
	},
	
	BStringEquality: function(QA) {
		
		LoadModule('jsstd');

		var b = new BString('ABCDEF');

		QA.ASSERT( b == 'ABCDEF', true, 'string and BString equality' )
		QA.ASSERT( 'ABCDEF' == b, true, 'string and BString equality' )

		QA.ASSERT( b === 'ABCDEF', false, 'string and BString equality and same type' )

		QA.ASSERT( (BString('abc') == BString('abc')), true, 'BString == BString' )
		QA.ASSERT( (BString('abc') == BString('xyz')), false, 'BString == BString' )
	},

	BStringStringSimilarity: function(QA) {

//		QA.ASSERT( (new BString('abc') == new BString('abc')), (new String('abc') == new String('abc')), 'new a == a' );
//		QA.ASSERT( (new BString('abc') != new BString('abc')), (new String('abc') != new String('abc')), 'new a != a' );
		QA.ASSERT( (new BString('abc') == new BString('xxx')), (new String('abc') == new String('xxx')), 'new a == b' );
		QA.ASSERT( (new BString('abc') != new BString('xxx')), (new String('abc') != new String('xxx')), 'new a != b' );
		QA.ASSERT( (new BString('abc') === new BString('abc')), (new String('abc') === new String('abc')), 'new a === a' );
		QA.ASSERT( (new BString('abc') !== new BString('abc')), (new String('abc') !== new String('abc')), 'new a !== a' );
		QA.ASSERT( (new BString('abc') === new BString('xxx')), (new String('abc') === new String('xxx')), 'new a === b' );
		QA.ASSERT( (new BString('abc') !== new BString('xxx')), (new String('abc') !== new String('xxx')), 'new a !== b' );
//		QA.ASSERT( (new BString()), (new String()), 'new ()' );
//		QA.ASSERT( (new BString('')), (new String('')), 'new ()' );
		QA.ASSERT( (!!new BString('')), (!!new String('')), 'new !!""' );
		QA.ASSERT( (!!new BString('abc')), (!!new String('abc')), 'new !!a' );


		QA.ASSERT( (BString('abc') == BString('abc')), (String('abc') == String('abc')), 'a == a' );
		QA.ASSERT( (BString('abc') != BString('abc')), (String('abc') != String('abc')), 'a != a' );
		QA.ASSERT( (BString('abc') == BString('xxx')), (String('abc') == String('xxx')), 'a == b' );
		QA.ASSERT( (BString('abc') != BString('xxx')), (String('abc') != String('xxx')), 'a != b' );
//?		QA.ASSERT( (BString('abc') === BString('abc')), (String('abc') === String('abc')), 'a === a' );
//?		QA.ASSERT( (BString('abc') !== BString('abc')), (String('abc') !== String('abc')), 'a !== a' );
		QA.ASSERT( (BString('abc') === BString('xxx')), (String('abc') === String('xxx')), 'a === b' );
		QA.ASSERT( (BString('abc') !== BString('xxx')), (String('abc') !== String('xxx')), 'a !== b' );
//		QA.ASSERT( (BString()), (String()), '()' );
//?		QA.ASSERT( (BString('')), (String('')), '("")' );
//		QA.ASSERT( (!!BString('')), (!!String('')), '!!""' );
		QA.ASSERT( (!!BString('abc')), (!!String('abc')), '!!a' );

//		QA.ASSERT( (BString(undefined)), (String(undefined)), '(undefined)' );
		QA.ASSERT( (BString('') == ''), (String('') == ''), '== ""' );
		QA.ASSERT( (BString('abc') == 'abc'), (String('abc') == 'abc'), ' Str(a) == "a" ' );
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
		d.Set(d.substr(0,3));
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
		bstring = bstring.concat('ABC');
		QA.ASSERT( String(stream.Read(3)), '123', 'stream Read()' )
		bstring = bstring.concat('DEF');
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
		
		var len = 0;
		var b = new Buffer();
		for ( var i = 0; i < 500; i++ ) {
			
			len += i;
			b.Write(QA.RandomString(i));
		}

		QA.ASSERT( b.length, len, 'buffer length' );
		
		var s = Stringify(b);

		QA.ASSERT( s.length, len, 'string length' );

	}

	
})
