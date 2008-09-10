LoadModule('jsstd');

/// ReadOnlyGlobalClasses [ftr]

		var bstr = Blob;
		Blob = null;
		QA.ASSERT( Blob, bstr, 'Blob integrity' );

		var bstr = Blob;
		Blob = function() {}
		QA.ASSERT( Blob, bstr, 'Blob integrity' );

		var bstr1 = Blob;
		delete global.Blob;
		QA.ASSERT( Blob, bstr1, 'Blob integrity' );


/// NativeInterface [ftr]

		var stream = new Stream('456');
		QA.ASSERT( !isNaN(stream._NI_StreamRead), true, 'NativeInterface system' )


/// NativeInterface security [ftr]

		var stream = new Stream('456');
		var prev = stream._NI_StreamRead;
		stream._NI_StreamRead = 123456;
		QA.ASSERT( stream._NI_StreamRead, prev, 'NativeInterface security' )
		stream._NI_StreamRead = 987654;
		QA.ASSERT( stream._NI_StreamRead, prev, 'NativeInterface security' )


/// Blob BufferGet NativeInterface

		LoadModule('jsstd');
		var buffer = new Buffer();
		buffer.Write('123');
		var blob = buffer.Read(2);
		QA.ASSERT( blob.length, 2, 'buffer.Read() length' )
		QA.ASSERT( blob instanceof Blob, true, 'buffer.Read() returns a Blob' )
		QA.ASSERT( '_NI_BufferGet' in blob, true, 'returned Blob has _NI_BufferGet' );
		QA.ASSERT( blob._NI_BufferGet, true, 'returned Blob _NI_BufferGet is active' );


/// Mutated Blob BufferGet NativeInterface

		var b = new Blob('123');
		b.replace; // force mutation
		QA.ASSERT(  b instanceof String, true, 'blob is a String' )
		QA.ASSERT(  b._NI_BufferGet, true, 'mutated blob has _NI_BufferGet' )
		
		


/// Blob mutation preserves BufferGet NativeInterface

		var b = new Blob();
		QA.ASSERT( b._NI_BufferGet, true, 'constructed Blob has _NI_BufferGet enabled' );
		b.replace('bcde', '123'); // force Blob mutation
		QA.ASSERT( b instanceof String, true, 'Blob has mutated to String' );
		QA.ASSERT( '_NI_BufferGet' in b, true, 'mutated Blob has _NI_BufferGet' );
		QA.ASSERT( b._NI_BufferGet, false, 'mutated Blob _NI_BufferGet is disactivated' );


/// Blob mutation keep properties [ftr]
	
		var b = new Blob('abcdef');
		b.prop1 = 11; b.prop2 = 22; b.prop3 = 33; b.prop4 = 44; b.prop5 = 55; b.prop6 = 66; b.prop7 = 77;
		b.replace; // force mutation
		QA.ASSERT( b instanceof Blob, false, 'is not Blob' )
		QA.ASSERT( b instanceof String, true, 'is String' )
		QA.ASSERT( b.prop1+b.prop2+b.prop3+b.prop4+b.prop5+b.prop6+b.prop7, 308, 'properties are preserved' )


/// Blob no mutation on concat [ftr]

		var b1 = new Blob('abcdef');
		var b2 = b1.concat('123');
		QA.ASSERT_STR( b2, 'abcdef123', 'concat result' )
		
		QA.ASSERT( b1 instanceof Blob, true, 'is Blob' )
		QA.ASSERT( b1 instanceof String, false, 'is not String' )

		QA.ASSERT_TYPE( b2, Blob )
		QA.ASSERT( b2 instanceof String, false, 'is not String' )


/// Blob mutation on replace [ftr]

		var b1 = new Blob('abcdef');
		var b2 = b1.replace('bcde', '123');

		QA.ASSERT_STR( b2, 'a123f', 'replace result' )
		
		QA.ASSERT( b1 instanceof Blob, false, 'is not Blob' )
		QA.ASSERT_TYPE( b1, String )

		QA.ASSERT_TYPE( b2, 'string' )


/// Blob avoid mutation [ftr]

		var b = new Blob('123');
		QA.ASSERT( b.length, 3, 'Blob length' );
		QA.ASSERT_TYPE( b, Blob )

		var b = new Blob('123');
		QA.ASSERT( b.toString(), '123', 'Blob toString' );
		QA.ASSERT_TYPE( b, Blob )

		var b = new Blob('123');
		QA.ASSERT( b.valueOf(), '123', 'Blob valueOf' );
		QA.ASSERT_TYPE( b, Blob )

		var b = new Blob('123');
		b.constructor;
		b.__proto__;
		QA.ASSERT_TYPE( b, Blob )


		// now all at once
		
		var b = new Blob('123');
		QA.ASSERT( b.length, 3, 'Blob length' );
		QA.ASSERT( b.toString(), '123', 'Blob toString' );
		QA.ASSERT( b.valueOf(), '123', 'Blob valueOf' );
		b.constructor;
		b.__proto__;
		QA.ASSERT( b == '123', true );
		QA.ASSERT( b == Blob('123'), true );
		QA.ASSERT( b == new Blob('123'), true );
		QA.ASSERT_TYPE( b, Blob )
		
		
		var c = Blob('abcd');
		c.replace = function() {}
		c.replace();
		QA.ASSERT_TYPE( c, Blob )

		
		var d = Blob('abcd');
		d.__proto__ = { replace: function() {} }
		QA.ASSERT( d instanceof String, false )
		d.replace();
		QA.ASSERT( d instanceof String, false )


		var e = Blob('abcd');
		e.__proto__ = { __proto__:e.__proto__, replace: function() {} }
		QA.ASSERT( e instanceof Blob, true )
		QA.ASSERT( e instanceof String, false )
		e.replace();
		QA.ASSERT( e instanceof Blob, true )
		QA.ASSERT( e instanceof String, false )
		e.match;
		QA.ASSERT( e instanceof Blob, true )
		QA.ASSERT( e instanceof String, false )


/// Blob mutation reliability [ftr]

		var b = new Blob('123');
		b.replace;
		
		QA.ASSERT( b == '123', true, 'string comparaison' );
		QA.ASSERT( b === '123', false, 'string and type comparaison' );

		QA.ASSERT( b.length, 3, 'string length' );

		var b1 = Blob('123');
		QA.ASSERT_STR( b == b1, false, 'String == Blob' );
		QA.ASSERT_STR( b1 == b, false, 'Blob == String' ); // see EQUALITY hook
		QA.ASSERT_TYPE( b1, Blob )

		QA.ASSERT_TYPE( b, String )


/// Blob API [ftr]
		
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


/// Blob construct with data [ftr]
		
		var bstr = new Blob('98765');
		QA.ASSERT( String(bstr), '98765', 'string value' );


/// Blob not constructed [ftr]

		var bstr = Blob('98765');
		QA.ASSERT( String(bstr), '98765', 'string value' );

		var v = Blob('123');
		QA.ASSERT( typeof v, 'object', 'variable type' );
		QA.ASSERT( String(v), '123', 'content' );


/// Blob toString [ftr]

		QA.ASSERT( Blob('345').toString(), '345', 'string value' );


/// Blob valueOf [ftr]

		QA.ASSERT( Blob('567').valueOf(), '567', 'value of' );


/// Blob misc [ftr]
	
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


/// Blob errors [ftr]

		var b = new Blob();
		b.concat( 'ABCD' ); // ????
		QA.ASSERT_EXCEPTION( function() b[5] = 'X', Error, 'set an out-of-range item' );


/// Blob concat [ftr]

		var b = Blob('123');
		var res = b.concat(b,Blob('456'),789,'abc');
		QA.ASSERT_STR( res, '123123456789abc', 'Blob concat' );


/// Blob substr [ftr]

		var b = new Blob('ABCDEF');
		var s = 'ABCDEF'

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
		QA.ASSERT( ''+b.substr(-7,2), 'AB', 'substr' );

		for ( var i = -10; i <= 10; i++ ) {

			QA.ASSERT( ''+b.substr(i)   ,s.substr(i)   , 'substr('+i+')' );
			for ( var j = -10; j <= 10; j++ )
				QA.ASSERT( ''+b.substr(i,j)   ,s.substr(i,j)   , 'substr('+i+','+j+')' );
		}

		QA.ASSERT( (b instanceof Blob) && !(b instanceof String), true, 'b is a Blob, not a String' );


/// Blob substring [ftr]

		var b = new Blob('ABCDEF');
		var s = 'ABCDEF'

		QA.ASSERT( ''+b.substring(-6)   ,s.substring(-6)   , 'substring' );
		QA.ASSERT( ''+b.substring(-1)   ,s.substring(-1)   , 'substring' );
		QA.ASSERT( ''+b.substring(0)    ,s.substring(0)    , 'substring' );
		QA.ASSERT( ''+b.substring(1)    ,s.substring(1)    , 'substring' );
		QA.ASSERT( ''+b.substring(2)    ,s.substring(2)    , 'substring' );
		QA.ASSERT( ''+b.substring(5)    ,s.substring(5)    , 'substring' );
		QA.ASSERT( ''+b.substring(6)    ,s.substring(6)    , 'substring' );
		QA.ASSERT( ''+b.substring(7)    ,s.substring(7)    , 'substring' );
		QA.ASSERT( ''+b.substring(2,3)  ,s.substring(2,3)  , 'substring' );
		QA.ASSERT( ''+b.substring(-2,2) ,s.substring(-2,2) , 'substring' );
		QA.ASSERT( ''+b.substring(-2,3) ,s.substring(-2,3) , 'substring' );
		QA.ASSERT( ''+b.substring(0,6)  ,s.substring(0,6)  , 'substring' );
		QA.ASSERT( ''+b.substring(0,7)  ,s.substring(0,7)  , 'substring' );
		QA.ASSERT( ''+b.substring(0,-2) ,s.substring(0,-2) , 'substring' );
		QA.ASSERT( ''+b.substring(-7,2) ,s.substring(-7,2) , 'substring' );

		for ( var i = -10; i <= 10; i++ ) {

			QA.ASSERT( ''+b.substring(i)   ,s.substring(i)   , 'substring('+i+')' );
			for ( var j = -10; j <= 10; j++ )
				QA.ASSERT( ''+b.substring(i,j)   ,s.substring(i,j)   , 'substring('+i+','+j+')' );
		}

		QA.ASSERT( (b instanceof Blob) && !(b instanceof String), true, 'b is a Blob, not a String' );


/// Blob setter [ftrd]

		var b = new Blob('ABCDEF');
		b[0] = 'X';
		QA.ASSERT( ''+b.substr(0,1), 'X', 'setter' );
		b[5] = 'W';
		QA.ASSERT( ''+b[5], 'W', 'setted value' );
		b[5] = 'W';
		QA.ASSERT( String(b), 'XBCDEW', 'setter' );
		QA.ASSERT_EXCEPTION( function() { b[-1] = 'Y'; }, Error, 'out of range' );
		QA.ASSERT_EXCEPTION( function() { b[6] = 'Z'; }, Error, 'out of range' );


/// Blob boolean test [ftr]
	
		QA.ASSERT( !!Blob(''), true, 'empty Blob cast to boolean' );
		QA.ASSERT( !!Blob('x'), true, 'non-empty Blob cast to boolean' );
//		QA.ASSERT( !!Blob(''), !!(''), 'empty Blob cast to boolean' );
//		QA.ASSERT( !!Blob('a'), !!('a'), 'empty Blob cast to boolean' );


/// Blob equality operator [ftr]

		var b = new Blob('ABCDEF');

		QA.ASSERT( b == 'ABCDEF', true, 'string and Blob equality' )
		QA.ASSERT( 'ABCDEF' == b, true, 'string and Blob equality' )

		QA.ASSERT( b === 'ABCDEF', false, 'string and Blob equality and same type' )

		QA.ASSERT( (Blob('abc') == Blob('abc')), true, 'Blob == Blob' )
		QA.ASSERT( (Blob('abc') == Blob('xyz')), false, 'Blob == Blob' )


/// Blob and String similarity [ftr]

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


/// Stream [ftr]

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


/// Stream add [ftr]

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


/// non-native Stream [ftr]

		var buf = new Buffer();
		buf.Write('abcdefghi');

		function myStream() {
		
			this.Read = function(amount) {
				
				return buf.Read(2);
			}
		}
		
		QA.ASSERT( Stringify( new myStream() ), 'abcdefghi', 'force string conversion' );


/// Stringify function [ftr]
	
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

