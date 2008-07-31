({

	ReadOnlyGlobalClasses: function(QA) {

		var bstr = Blob;
		Blob = null;
		QA.ASSERT( Blob, bstr, 'Blob integrity' );

		var bstr = Blob;
		Blob = function() {}
		QA.ASSERT( Blob, bstr, 'Blob integrity' );

		var bstr1 = Blob;
		delete global.Blob;
		QA.ASSERT( Blob, bstr1, 'Blob integrity' );
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

	BlobAPI: function(QA) {

		LoadModule('jsstd');
		
		QA.ASSERT( Blob('123456').indexOf(''), 0, 'Blob indexOf' )
		QA.ASSERT( Blob('123456').indexOf('1'), 0, 'Blob indexOf' )
		QA.ASSERT( Blob('123456').indexOf('2'), 1, 'Blob indexOf' )
		QA.ASSERT( Blob('123456').indexOf('6'), 5, 'Blob indexOf' )
		QA.ASSERT( Blob('123456').indexOf('7'), -1, 'Blob indexOf' )
		QA.ASSERT( Blob('12341234').indexOf('2',1), 1, 'Blob indexOf' )
		QA.ASSERT( Blob('12341234').indexOf('2',2), 5, 'Blob indexOf' )
		QA.ASSERT( Blob('123').indexOf('3',-10), 2, 'Blob indexOf' )
		QA.ASSERT( Blob('123').indexOf('3',10), -1, 'Blob indexOf' )
		QA.ASSERT( Blob('1231234').indexOf('1234'), 3, 'Blob indexOf' )
		QA.ASSERT( Blob('12345').indexOf('12345'), 0, 'Blob indexOf' )

		QA.ASSERT( Blob('123').lastIndexOf('3', 100), 2, 'Blob lastIndexOf' )
		QA.ASSERT( Blob('123').lastIndexOf('3',-100), -1, 'Blob lastIndexOf' )
		QA.ASSERT( Blob('123456').lastIndexOf('123456'), 0, 'Blob lastIndexOf' )
		QA.ASSERT( Blob('12341234').lastIndexOf('2'), 5, 'Blob lastIndexOf' )
		QA.ASSERT( Blob('123').lastIndexOf('1',0), 0, 'Blob lastIndexOf' )
		QA.ASSERT( Blob('123456').lastIndexOf('12'), 0, 'Blob lastIndexOf' )
		QA.ASSERT( Blob('123456').lastIndexOf('56'), 4, 'Blob lastIndexOf' )
		QA.ASSERT( Blob('0123456789').lastIndexOf('567', 4), -1, 'Blob lastIndexOf' )
		QA.ASSERT( Blob('0123456789').lastIndexOf('56789', 5), 5, 'Blob lastIndexOf' )
		QA.ASSERT( Blob('12341234').lastIndexOf('1234'), 4, 'Blob lastIndexOf' )
		QA.ASSERT( Blob('12341234').lastIndexOf('1234', 4), 4, 'Blob lastIndexOf' )
		QA.ASSERT( Blob('12341234').lastIndexOf('1234', 3), 0, 'Blob lastIndexOf' )
		
		QA.ASSERT( isNaN( Blob('').charCodeAt(0) ), true, 'Blob charCodeAt' )
		QA.ASSERT( Blob('1').charCodeAt(0), 49, 'Blob charCodeAt' )
		QA.ASSERT( Blob('1').charCodeAt(), 49, 'Blob charCodeAt' )
		QA.ASSERT( isNaN( Blob('1').charCodeAt(-1) ), true, 'Blob charCodeAt' )
		QA.ASSERT( isNaN( Blob('1').charCodeAt(1) ), true, 'Blob charCodeAt' )
		
		QA.ASSERT( Blob('').charAt(0), '', 'Blob charAt' )
		QA.ASSERT( Blob('1').charAt(0), '1', 'Blob charAt' )
		QA.ASSERT( Blob('1').charAt(), '1', 'Blob charAt' )
		QA.ASSERT( Blob('1').charAt(-1), '', 'Blob charAt' )
		QA.ASSERT( Blob('1').charAt(1), '', 'Blob charAt' )
		
	},

	Blob_construct_with_data: function(QA) {

		LoadModule('jsstd');
//		LoadModule('jslang');
		
		var bstr = new Blob('98765');
		QA.ASSERT( String(bstr), '98765', 'string value' );
	},
	
	Blob_not_constructed: function(QA) {
		
		LoadModule('jsstd');
//		LoadModule('jslang');

		var bstr = Blob('98765');
		QA.ASSERT( String(bstr), '98765', 'string value' );


		var v = Blob('123');
		QA.ASSERT( typeof v, 'object', 'variable type' );
		QA.ASSERT( String(v), '123', 'content' );
	},

	Blob_to_string: function(QA) {

		LoadModule('jsstd');
//		LoadModule('jslang');
		
		QA.ASSERT( Blob('345').toString(), '345', 'string value' );
	},

	Blob_valueOf: function(QA) {

		LoadModule('jsstd');
//		LoadModule('jslang');

		QA.ASSERT( Blob('567').valueOf(), '567', 'value of' );
	},
	
	Blob: function(QA) {
		
		LoadModule('jsstd');
//		LoadModule('jslang');
	
		var b = new Blob();
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

	BlobErrors: function(QA) {
	
		var b = new Blob();
		b.concat( 'ABCD' ); // ????
		QA.ASSERT_EXCEPTION( function() b[5] = 'X', Error, 'set an out-of-range item' );
	},


	Blob_concat: function(QA) {

		var b = Blob('123');
		var res = b.concat(b,Blob('456'),789,'abc');
		QA.ASSERT_STR( res, '123123456789abc', 'Blob concat' );
	},


	Blob_substr: function(QA) {
	
		LoadModule('jsstd');
//		LoadModule('jslang');

		var b = new Blob();
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


	Blob_str_property: function(QA) {

		var b = Blob('123');
		QA.ASSERT_TYPE( b.str, 'string', '.str is String' );
	},


	_BlobSetter: function(QA) { // this test do not have any sense with immutable objects
	
		LoadModule('jsstd');
//		LoadModule('jslang');

		var b = new Blob('ABCDEF');
		b[0] = 'X';
		QA.ASSERT( ''+b.substr(0,1), 'X', 'setter' );
		b[5] = 'W';
		QA.ASSERT( ''+b[5], 'W', 'setted value' );
		b[5] = 'W';
		QA.ASSERT( String(b), 'XBCDEW', 'setter' );
		QA.ASSERT_EXCEPTION( function() { b[-1] = 'Y'; }, Error, 'out of range' );
		QA.ASSERT_EXCEPTION( function() { b[6] = 'Z'; }, Error, 'out of range' );
	},
	
	Blob_boolean: function(QA) {
	
		QA.ASSERT( !!Blob(''), true, 'empty Blob cast to boolean' );
		QA.ASSERT( !!Blob('x'), true, 'non-empty Blob cast to boolean' );
	
//		QA.ASSERT( !!Blob(''), !!(''), 'empty Blob cast to boolean' );
//		QA.ASSERT( !!Blob('a'), !!('a'), 'empty Blob cast to boolean' );
	},
	
	BlobEquality: function(QA) {
		
		LoadModule('jsstd');

		var b = new Blob('ABCDEF');

		QA.ASSERT( b == 'ABCDEF', true, 'string and Blob equality' )
		QA.ASSERT( 'ABCDEF' == b, true, 'string and Blob equality' )

		QA.ASSERT( b === 'ABCDEF', false, 'string and Blob equality and same type' )

		QA.ASSERT( (Blob('abc') == Blob('abc')), true, 'Blob == Blob' )
		QA.ASSERT( (Blob('abc') == Blob('xyz')), false, 'Blob == Blob' )
	},

	BlobStringSimilarity: function(QA) {

//		QA.ASSERT( (new Blob('abc') == new Blob('abc')), (new String('abc') == new String('abc')), 'new a == a' );
//		QA.ASSERT( (new Blob('abc') != new Blob('abc')), (new String('abc') != new String('abc')), 'new a != a' );
		QA.ASSERT( (new Blob('abc') == new Blob('xxx')), (new String('abc') == new String('xxx')), 'new a == b' );
		QA.ASSERT( (new Blob('abc') != new Blob('xxx')), (new String('abc') != new String('xxx')), 'new a != b' );
		QA.ASSERT( (new Blob('abc') === new Blob('abc')), (new String('abc') === new String('abc')), 'new a === a' );
		QA.ASSERT( (new Blob('abc') !== new Blob('abc')), (new String('abc') !== new String('abc')), 'new a !== a' );
		QA.ASSERT( (new Blob('abc') === new Blob('xxx')), (new String('abc') === new String('xxx')), 'new a === b' );
		QA.ASSERT( (new Blob('abc') !== new Blob('xxx')), (new String('abc') !== new String('xxx')), 'new a !== b' );
//		QA.ASSERT( (new Blob()), (new String()), 'new ()' );
//		QA.ASSERT( (new Blob('')), (new String('')), 'new ()' );
		QA.ASSERT( (!!new Blob('')), (!!new String('')), 'new !!""' );
		QA.ASSERT( (!!new Blob('abc')), (!!new String('abc')), 'new !!a' );


		QA.ASSERT( (Blob('abc') == Blob('abc')), (String('abc') == String('abc')), 'a == a' );
		QA.ASSERT( (Blob('abc') != Blob('abc')), (String('abc') != String('abc')), 'a != a' );
		QA.ASSERT( (Blob('abc') == Blob('xxx')), (String('abc') == String('xxx')), 'a == b' );
		QA.ASSERT( (Blob('abc') != Blob('xxx')), (String('abc') != String('xxx')), 'a != b' );
//?		QA.ASSERT( (Blob('abc') === Blob('abc')), (String('abc') === String('abc')), 'a === a' );
//?		QA.ASSERT( (Blob('abc') !== Blob('abc')), (String('abc') !== String('abc')), 'a !== a' );
		QA.ASSERT( (Blob('abc') === Blob('xxx')), (String('abc') === String('xxx')), 'a === b' );
		QA.ASSERT( (Blob('abc') !== Blob('xxx')), (String('abc') !== String('xxx')), 'a !== b' );
//		QA.ASSERT( (Blob()), (String()), '()' );
//?		QA.ASSERT( (Blob('')), (String('')), '("")' );
//		QA.ASSERT( (!!Blob('')), (!!String('')), '!!""' );
		QA.ASSERT( (!!Blob('abc')), (!!String('abc')), '!!a' );

//		QA.ASSERT( (Blob(undefined)), (String(undefined)), '(undefined)' );
		QA.ASSERT( (Blob('') == ''), (String('') == ''), '== ""' );
		QA.ASSERT( (Blob('abc') == 'abc'), (String('abc') == 'abc'), ' Str(a) == "a" ' );
	},


	_BlobSelfReference: function(QA) { // this test do not have any sense with immutable objects

		LoadModule('jsstd');
//		LoadModule('jslang');

		var a = new Blob();
		a.Set();
		a.Set( 'ABCDEF' );
		a.Add(a);
		QA.ASSERT( ''+a, 'ABCDEFABCDEF', 'self add' )
	
		var b = new Blob();
		b.Set( 'ABCDEF' );
		b.Add( '12345' );
		b.Set(b);
		QA.ASSERT( ''+b, 'ABCDEF12345', 'self set' )

		var c = new Blob();
		c.Set('');
		c.Add(c);
		QA.ASSERT( ''+c, '', 'self add empty' )

		var d = new Blob();
		d.Set('123456');
		d.Set(d.substr(0,3));
		QA.ASSERT( ''+d, '123', 'self substr' )
	},

	SimpleStreamTest: function(QA) {	

		var blob = new Blob("1234567");
		var stream = Stream(blob);
		QA.ASSERT( stream.source, blob, 'source object' )

		QA.ASSERT( stream.position, 0, 'initial stream position' )
		QA.ASSERT( String(stream.Read(3)), '123', 'stream Read()' )
		QA.ASSERT( stream.position, 3, 'stream position after Read()' )
		QA.ASSERT( String(stream.Read(0)), '', 'read 0 bytes on the stream' )
		QA.ASSERT( stream.position, 3, 'stream position after Read(0)' )
		QA.ASSERT( String(stream.Read(100)), '4567', 'read more than the stream size' )
		QA.ASSERT( stream.position, 7, 'stream position after reading more than the stream size' )

		stream.position = 0;
		QA.ASSERT( String(stream.Read(blob.length)), '1234567', 'read the exact length' )
	},

	StreamAdd: function(QA) {

		var blob = new Blob("1234");
		var stream = Stream(blob);
		blob = blob.concat('ABC');
		QA.ASSERT( String(stream.Read(3)), '123', 'stream Read()' )
		blob = blob.concat('DEF');
		QA.ASSERT( blob.length, 10, 'stream source length' )
		QA.ASSERT( String(stream.Read(3)), '4', 'stream Read()' )
		QA.ASSERT( stream.position, 4, 'stream position' )

		var s1 = Stream(blob);
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
