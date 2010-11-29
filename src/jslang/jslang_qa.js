LoadModule('jsstd');

/// ReadOnlyGlobalClasses [ftrm]

		var bstr = Blob;
		Blob = null;
		QA.ASSERT( Blob, bstr, 'Blob integrity' );

		var bstr = Blob;
		Blob = function() {}
		QA.ASSERT( Blob, bstr, 'Blob integrity' );

		var bstr1 = Blob;
		delete global.Blob;
		QA.ASSERT( Blob, bstr1, 'Blob integrity' );


/// NativeInterface [ftrm]

		var stream = new Stream('456');
		QA.ASSERT( !isNaN(stream._NI_StreamRead), true, 'NativeInterface system' )


/// NativeInterface security [ftrm]

		var stream = new Stream('456');
		var prev = stream._NI_StreamRead;
		stream._NI_StreamRead = 123456;
		QA.ASSERT( stream._NI_StreamRead, prev, 'NativeInterface security' )
		stream._NI_StreamRead = 987654;
		QA.ASSERT( stream._NI_StreamRead, prev, 'NativeInterface security' )


/// Blob vs String [rmtf]

	function tests(CHK, T) {

		CHK( T('abcd').split('',0) );
		CHK( T('abcd').split('',1) );
		CHK( T('abcd').split('',2) );
		CHK( T('abcd').split('',3) );
		CHK( T('abcd').split('',4) );
		CHK( T('abcd').split('',5) );

		CHK( T('a').split('',0) );
		CHK( T('a').split('',1) );
		CHK( T('a').split('',2) );

		CHK( T('').split('',0) );
		CHK( T('').split('',1) );
		CHK( T('').split('',2) );

		CHK( T('').split('') );
		CHK( T('1').split('') );
		CHK( T('').split('1') );
		CHK( T('1').split('1') );

		CHK( T('abcd').split('') );
		CHK( T('').split('') );
	//	CHK( T('12').split(0,1) );
	//	CHK( T('12').split(2, undefined) );
		CHK( T('abcd').split('abcd') );
		CHK( T('ababababab').split('a', 3) );
		CHK( T('12').split(undefined) );
		CHK( T('12').split('a') );
		CHK( T('1a23aa').split('a') );

		CHK( T('aaa|bbb|ccc').split('|') );
		CHK( T('aaa||bbb||ccc').split('|') );
		CHK( T('aaa||bbb||ccc').split('||') );
	}

	var res1 = [];
	var res2 = [];
	tests( function(r) res1.push(r), String );
	tests( function(r) res2.push(r), Blob );

	QA.ASSERT( uneval(res1), uneval(res2), 'all split() test' )


/// Blob iterator object [rmtf]

	var tmp = '';
	var b = new Blob('ABC123');
	for ( c in b )
		tmp += c;
	QA.ASSERT( tmp, '012345', 'for loop over Blob' )

	var tmp = '';
	var b = new Blob('ABC123');
	for each ( c in b )
		tmp += c;
	QA.ASSERT( tmp, 'ABC123', 'for-each  loop over Blob' )


/// Blob equality []
		var b1 = new Blob('abc');
		var b2 = Blob('abc');
		QA.ASSERT_STR( b1, b2, 'Blob equality' )


/// Blob memory usage []

		var length = 1024*1024;
		var times = 3;
		QA.GC();
		var data = [];
	
		var mem0 = privateMemoryUsage;
		for ( var i = 0; i < times; ++i ) {
		
			data.push( Blob(StringRepeat('a', length)) );
			QA.GC();
		}
		
		var mem = (privateMemoryUsage - mem0) / length / times;
		QA.ASSERT( mem > 1 && mem < 1.02, true, 'Blob memory usage ('+mem+') (test not available on all platforms)' );


/// Blob toSource and Uneval [mrtf]

		var b = new Blob('test');
		QA.ASSERT( b.toSource(), '"test"', 'blob.toSource()' )
		QA.ASSERT( uneval(b), '"test"', 'uneval(blob)' )


/// Blob BufferGet NativeInterface [ftrm]

		var buffer = new Buffer();
		buffer.Write('123');
		var blob = buffer.Read(2);
		QA.ASSERT( blob.length, 2, 'buffer.Read() length' )
//		QA.ASSERT( blob instanceof Blob, true, 'buffer.Read() returns a Blob' )
		QA.ASSERT( typeof blob, 'string', 'typeof blob' );
//		QA.ASSERT( ('_NI_BufferGet' in blob), true, 'returned Blob has _NI_BufferGet' );
//		QA.ASSERT( blob._NI_BufferGet, true, 'returned Blob _NI_BufferGet is active' );


/// Mutated Blob BufferGet NativeInterface [ftrm d]

		var b = new Blob('123');
		b.replace; // force mutation
		QA.ASSERT(  b instanceof String, true, 'blob is a String' )
		QA.ASSERT(  b._NI_BufferGet, true, 'mutated blob has _NI_BufferGet' )


/// Blob mutation preserves BufferGet NativeInterface [ftrm d]

		var b = new Blob();
		QA.ASSERT( b._NI_BufferGet, true, 'constructed Blob has _NI_BufferGet enabled' );
		b.replace('bcde', '123'); // force Blob mutation
		QA.ASSERT( b instanceof String, true, 'Blob has mutated to String' );
		QA.ASSERT( '_NI_BufferGet' in b, true, 'mutated Blob has _NI_BufferGet' );
		QA.ASSERT( b._NI_BufferGet, true, 'mutated Blob has _NI_BufferGet' );


/// Blob mutation keep properties [ftrm d]
	
		var b = new Blob('abcdef');
		b.prop1 = 11; b.prop2 = 22; b.prop3 = 33; b.prop4 = 44; b.prop5 = 55; b.prop6 = 66; b.prop7 = 77;
		b.replace; // force mutation
		QA.ASSERT( b instanceof Blob, false, 'is not Blob' )
		QA.ASSERT( b instanceof String, true, 'is String' )
		QA.ASSERT( b.prop1+b.prop2+b.prop3+b.prop4+b.prop5+b.prop6+b.prop7, 308, 'properties are preserved' )


/// Blob no mutation on concat [ftrm]

		var b1 = new Blob('abcdef');
		var b2 = b1.concat('123');
		QA.ASSERT_STR( b2, 'abcdef123', 'concat result' )
		
		QA.ASSERT( b1 instanceof Blob, true, 'is Blob' )
		QA.ASSERT( b1 instanceof String, false, 'is not String' )

		QA.ASSERT_TYPE( b2, Blob )
		QA.ASSERT( b2 instanceof String, false, 'is not String' )


/// Blob mutation on replace [ftrm d]

		var b1 = new Blob('abcdef');
		var b2 = b1.replace('bcde', '123');

		QA.ASSERT_STR( b2, 'a123f', 'replace result' )
		
		QA.ASSERT( b1 instanceof Blob, false, 'is not Blob' )
		QA.ASSERT_TYPE( b1, String )

		QA.ASSERT_TYPE( b2, 'string' )


/// Blob avoid mutation [ftrm]

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


/// Blob misc mutation tests [ftrm d]

	var b = Blob('123')
	b.toUpperCase();
	QA.ASSERT( b instanceof String, true );
	QA.ASSERT( b.constructor, String );


/// Blob mutation reliability [ftrm d]

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


/// Blob API [ftrm]
		
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
		QA.ASSERT( Blob('12341234').lastIndexOf(''), 8, 'Blob lastIndexOf' )
		
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

		QA.ASSERT( Blob('123').charAt(undefined), '1', 'Blob charAt' )
		QA.ASSERT( Blob('123').charAt(NaN), '1', 'Blob charAt' )
		QA.ASSERT( Blob('123').charAt(-Infinity), '', 'Blob charAt' )
		QA.ASSERT( Blob('123').charAt(Infinity), '', 'Blob charAt' )
		QA.ASSERT( Blob('123').charAt('2'), '3', 'Blob charAt' )
		QA.ASSERT( Blob('123').charAt(''), '1', 'Blob charAt' )
		
		
		var s = 'Ab \0c';
		var b = Blob(s);
		
		for ( var j = -2; j < s.length+2; j++ ) {
		
			QA.ASSERT( b.indexOf(s[j]), s.indexOf(s[j]), 'Blob/String indexOf('+s[j]+')' )
			for ( var i = -10; i <= 10; i++ )		
				QA.ASSERT( b.indexOf(s[j],i), s.indexOf(s[j],i), 'Blob/String indexOf('+s[j]+','+i+')' )
		}
		
		for ( var j = -2; j < s.length+2; j++ ) {
		
			QA.ASSERT( b.lastIndexOf(s[j]), s.lastIndexOf(s[j]), 'Blob/String lastIndexOf('+s[j]+')' )
			for ( var i = -10; i <= 10; i++ )
				QA.ASSERT( b.lastIndexOf(s[j],i), s.lastIndexOf(s[j],i), 'Blob/String lastIndexOf('+s[j]+','+i+')' )
		}

		for ( var j = -2; j < s.length+2; j++ )
			QA.ASSERT( b.charAt(j), s.charAt(j), 'Blob/String charAt('+j+')' )

		for ( var j = -2; j < s.length+2; j++ )
			QA.ASSERT( b.charCodeAt(j), s.charCodeAt(j), 'Blob/String charCodeAt('+j+')' )


/// Blob and String comparaison (substr, substring) [tr]

	function argGenerator(count, argList) {

		 var len = argList.length;
		 var pos = Math.pow(len, count);
		 var arg = new Array(count);
		 while (pos--) {

			  var tmp = pos;
			  for (var i = 0; i < count; i++) {

					var r = tmp % len;
					tmp = (tmp - r) / len;
					arg[i] = argList[r];
			  }
			  yield arg;
		 }
	}

	var s = "abcdefgh";
	var b = Blob(s);
	
	var argGen = argGenerator(2, [undefined, NaN, - Infinity, -2147483649, -2147483648, -65535, -1000, -100, -10, -3, -2.5, -2, -1, -0.6, -0.5, -0.4, 0, 0.4, 0.5, 0.6, 1, 2, 2.5, 3, 10, 100, 1000, + Infinity, "", " ", "0", "1"]);
	try {
		 for (;;) {
		 
				var args = argGen.next();
//				Print( 'substr('+args+')\n' );
				QA.ASSERT_STR( s.substr.apply(s, args), b.substr.apply(b, args), 'substr('+args.toSource()+')' );
				QA.ASSERT_STR( s.substring.apply(s, args), b.substring.apply(b, args), 'substring('+args.toSource()+')' );
		 }
	} catch (ex if ex instanceof StopIteration) {}



/// Blob construct with data [ftrm]
		
		var bstr = new Blob('98765');
		QA.ASSERT( String(bstr), '98765', 'string value' );


/// Blob not constructed [ftrm]

		var bstr = Blob('98765');
		QA.ASSERT( String(bstr), '98765', 'string value' );

		var v = Blob('123');
		QA.ASSERT( typeof v, 'object', 'variable type' );
		QA.ASSERT( String(v), '123', 'content' );


/// Blob toString [ftrm]

		QA.ASSERT( Blob('345').toString(), '345', 'string value' );


/// Blob valueOf [ftrm]

		QA.ASSERT( Blob('567').valueOf(), '567', 'value of' );


/// Blob misc [ftrm]
	
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


// /// Blob errors [ftrm]
//
//		var b = new Blob();
//		b.concat( 'ABCD' ); // ????
//		QA.ASSERT_EXCEPTION( function() b[5] = 'X', Error, 'set an out-of-range item' );


/// Blob concat [ftrm]

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


/// Blob setter [ftrm]

		var b = new Blob('ABCDEF');
		
//		QA.ASSERT_EXCEPTION( function() {
//			b[0] = 'X'
//		}, Error, '"Cannot modify immutable objects" exception' );
		
//		QA.ASSERT_STR( b.substr(0,1), 'X', 'setter' );

//		QA.ASSERT_EXCEPTION( function() {
//			b[5] = 'W';
//		}, Error, '"Cannot modify immutable objects" exception' );
			
//		QA.ASSERT_STR( b[5], 'W', 'setted value' );
		
//		QA.ASSERT_EXCEPTION( function() {
//			b[5] = 'W';
//		}, Error, '"Cannot modify immutable objects" exception' );
		
//		QA.ASSERT( String(b), 'XBCDEW', 'setter' );
//		QA.ASSERT_EXCEPTION( function() { b[-1] = 'Y'; }, Error, 'out of range' );
//		QA.ASSERT_EXCEPTION( function() { b[6] = 'Z'; }, Error, 'out of range' );


/// Blob boolean test [ftrm]
	
		QA.ASSERT( !!Blob(''), false, 'empty Blob cast to boolean' );
		QA.ASSERT( !!Blob('x'), true, 'non-empty Blob cast to boolean' );
		QA.ASSERT( !!Blob(''), !!(''), 'empty Blob cast to boolean' );
		QA.ASSERT( !!Blob('a'), !!('a'), 'empty Blob cast to boolean' );


/// Blob equality operator [ftrm]

		var b = new Blob('ABCDEF');

		QA.ASSERT( b == 'ABCDEF', true, 'string and Blob equality' )
		QA.ASSERT( 'ABCDEF' == b, true, 'string and Blob equality' )

		QA.ASSERT( b === 'ABCDEF', false, 'string and Blob equality and same type' )

		QA.ASSERT( (Blob('abc') == Blob('abc')), true, 'Blob == Blob' )
		QA.ASSERT( (Blob('abc') == Blob('xyz')), false, 'Blob == Blob' )


/// Blob and String similarity [ftrm]

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


/// Stream [ftrm]

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


/// Stream add [ftrm]

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


/// non-native Stream [ftrm]

		var buf = new Buffer();
		buf.Write('abcdefghi');

		function myStream() {
		
			this.Read = function(amount) {
				
				return buf.Read(2);
			}
		}
		
		QA.ASSERT( Stringify( new myStream() ), 'abcdefghi', 'force string conversion' );



/// another non-native Stream [ftrm]

		var buf = new Buffer();
		buf.Write('abcdefghijklmnopqrstuvwxyz');
		var res = Stringify({ Read:function(n) { return buf.Read(3); } });
		QA.ASSERT_STR( res, 'abcdefghijklmnopqrstuvwxyz', 'stringified stream' );


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


/// blob serialization [ftrm d]

		var b = Blob("my blob");
		b.aPropertyOfMyBlob = 12345;
		var xdrData = XdrEncode(b);
		var val = XdrDecode(xdrData);
		QA.ASSERT( val.length, 7, 'blob length' );
		QA.ASSERT( val.aPropertyOfMyBlob, 12345, 'blob property' );
		QA.ASSERT_STR( val, "my blob", 'blob content' );


/// blob toSource [ftrm]

	new Blob('123').toSource();
//	new Blob('123').__proto__.toSource();

	var b = new Blob('123');
	new Blob().__proto__.toSource.call(b);


/// map serialization [ftrm d]

		var obj = {a:1, b:2, c:3};
		var m = Map(obj);
		var xdrData = XdrEncode(m);
		var val = XdrDecode(xdrData);
		QA.ASSERT_STR( uneval(obj), uneval(val), 'map content' );


/// Stringify blob [ft]

	var b = new Blob('123');
	var c = Stringify(b);

	QA.ASSERT_STR( c , 123, 'Stringified blob' );



/// Handle object [ft]

	var h = Handle;
	QA.ASSERT_TYPE(h, Handle, 'handle object type');
	QA.ASSERT_STR(h, '[Handle ????]', 'handle type string');
	QA.ASSERT(h.constructor, Math.constructor, 'constructor test');
	QA.ASSERT(h.prototype, Math.prototype, 'prototype test');
	QA.ASSERT(Handle instanceof Object, true, 'instance test');



/// Test InheritFrom() [ft d]

	function A() {}
	function B() {}
	A.prototype = new B;
	var a = new A;
	var b = new B;
	
	QA.ASSERT(InheritFrom(a, A), false, 'InheritFrom is not instanceof a,B');
	QA.ASSERT(InheritFrom(a, B), true, 'InheritFrom test a,B');
	QA.ASSERT(InheritFrom(b, B), false, 'InheritFrom is not instanceof b,A');
	QA.ASSERT(InheritFrom(b, A), false, 'InheritFrom test b,A');
	QA.ASSERT(InheritFrom(a, b), false, 'InheritFrom test a,b');
	QA.ASSERT(InheritFrom(b, a), false, 'InheritFrom test b,a');
	QA.ASSERT(InheritFrom(A, B), false, 'InheritFrom test A,B');
	QA.ASSERT(InheritFrom(B, A), false, 'InheritFrom test B,A');


/// ProcessEvents() [t]

	var timeout = TimeoutEvents(123);
	var t = TimeCounter();
	ProcessEvents(timeout);
	t = TimeCounter() - t;
	QA.ASSERT( t > 122 && t < 130, true, 'TimeoutEvents time ('+t+')' );
	QA.ASSERT_EXCEPTION( function() ProcessEvents(timeout), Error, 'ProcessEvents reused' );
