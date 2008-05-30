({

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


		var v = BString().Set('123');
		QA.ASSERT( typeof v, 'object', 'variable type' );
		QA.ASSERT( String(v), '123', 'content' );
	},

	BString_to_string: function(QA) {

		LoadModule('jsstd');
//		LoadModule('jslang');
		
		QA.ASSERT( BString().Set('345').toString(), '345', 'string value' );
	},

	BString_valueOf: function(QA) {

		LoadModule('jsstd');
//		LoadModule('jslang');

		QA.ASSERT( BString().Set('567').valueOf(), '567', 'value of' );
	},

	BString: function(QA) {
		
		LoadModule('jsstd');
//		LoadModule('jslang');
	
		var b = new BString();
		b.Add( 'ABCD' );
		QA.ASSERT( String(b), 'ABCD', 'content' );
		QA.ASSERT( b.length, 4, 'length' );
		QA.ASSERT( b[0], 'A', 'first item' );
		QA.ASSERT( b[3], 'D', 'last item' );
		QA.ASSERT( 2 in b, true, 'in operator' ); // cf. DEFINE_NEW_RESOLVE
		QA.ASSERT( 4 in b, false, 'in operator' );
		QA.ASSERT( b[4], undefined, 'after last item' );
		b.Add('XYZ');
		QA.ASSERT( b.length, 7, 'length' );
		QA.ASSERT( b[6], 'Z', 'last item' );
		QA.ASSERT( b.toString(), 'ABCDXYZ', 'toString' );
		QA.ASSERT( String(b.valueOf()), 'ABCDXYZ', 'valueof' );
		b.Set();
		QA.ASSERT( b.length, 0, 'length' );
	},


	BStringSubstr: function(QA) {
	
		LoadModule('jsstd');
//		LoadModule('jslang');

		var b = new BString();
		b.Add( 'ABCDEF' );
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


	BStringSetter: function(QA) {
	
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
		b.Add( 'ABCDEF' );

		QA.ASSERT( b == 'ABCDEF', true, 'string and BString equality' )
		QA.ASSERT( 'ABCDEF' == b, true, 'string and BString equality' )

		QA.ASSERT( b === 'ABCDEF', false, 'string and BString equality and same type' )
	},


	BStringSelfReference: function(QA) {

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

		QA.ASSERT( stream.position, 0, 'initial stream position' )
		QA.ASSERT( stream.Read(3), '123', 'stream Read()' )
		QA.ASSERT( stream.position, 3, 'stream position after Read()' )
		QA.ASSERT( stream.Read(0), '', 'read 0 bytes on the stream' )
		QA.ASSERT( stream.position, 3, 'stream position after Read(0)' )
		QA.ASSERT( stream.Read(100), '4567', 'read more than the stream size' )
		QA.ASSERT( stream.position, 7, 'stream position after reading more than the stream size' )

		stream.position = 0;
		QA.ASSERT( stream.Read(bstring.length), '1234567', 'read the exact length' )


	}
	
})
