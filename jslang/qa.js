({

	BString: function(QA) {
		
		LoadModule('jsstd');
	
		var b = new BString();
		b.Add( 'ABCD' );
		QA.ASSERT( String(b), 'ABCD', 'content' );
		QA.ASSERT( b.length, 4, 'length' );
		QA.ASSERT( b[0], 'A', 'first item' );
		QA.ASSERT( b[3], 'D', 'last item' );
		QA.ASSERT( 2 in b, true, 'in operator' );
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
		
		var b = new BString();
		b.Add( 'ABCDEF' );

		QA.ASSERT( b == 'ABCDEF', true, 'string and BString equality' )
		QA.ASSERT( 'ABCDEF' == b, true, 'string and BString equality' )

		QA.ASSERT( b === 'ABCDEF', false, 'string and BString equality and same type' )
	},


	BStringSelfReference: function(QA) {

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
	}
	
})
