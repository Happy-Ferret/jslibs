loadModule('jsiconv');

/// basic tests [rfmt]

	QA.ASSERT( new Iconv('UTF-8', 'ISO-8859-1')('é').length, 2, '"é" char length in UTF-8' );

	var conv = new Iconv('UTF-8', 'ISO-8859-1');
	var invConv = new Iconv('ISO-8859-1', 'UTF-8');
	var converted = conv('été');
	var result = invConv(converted);
	QA.ASSERT_STR( result, 'été', 'string convertion' );


	var conv = new Iconv('UTF-8', 'ISO-8859-1');
	var invConv = new Iconv('ISO-8859-1', 'UTF-8');
	var converted = conv('été');
	var result = '';
	for each ( var c in converted )
		result += invConv(c);
	QA.ASSERT_STR( result, 'été', 'char by char convertion' );


/// invalid sequense detection [rfmt]

	var utf8 = (new Iconv('UTF-8', 'ISO-8859-1'))('été').split('');
	utf8.splice(1,0,'x'); // insert an invalid multybyte sequense
	utf8 = utf8.join('');
	var res = (new Iconv('ISO-8859-1', 'UTF-8'))(utf8);
	
	QA.ASSERT_STR( res[0], '?', 'invalid sequense char' )
	QA.ASSERT_STR( res[2], '?', 'invalid sequense char' )


/// identity [rmt]

	var ui = new Iconv('UTF-8', 'ISO-8859-1');
	var iu = new Iconv('ISO-8859-1', 'UTF-8');
	
	for ( var cc = 0; cc < 256; ++cc ) {
		
		var c0 = String.fromCharCode(cc);
		var c1 = iu(ui(c0));
		QA.ASSERT_STR( c0, c1, 'char '+cc );
	}


/// encoding list [rfmt]

	var list = Iconv.list;
//	QA.ASSERT( list.length, 387, 'encoding count' );
	QA.ASSERT( list.indexOf('UTF-8'), 11, 'has UTF-8' );
	QA.ASSERT( list.indexOf('ISO-8859-1'), 48, 'has ISO-8859-1' );


/// test javascrit unicode char [rmtf]

	var conv = new Iconv(Iconv.jsUC, 'ISO-8859-1', true, true);
	var res = conv('été');
	
	QA.ASSERT( res.length, 6, 'string length' );
	QA.ASSERT( String.charCodeAt(res[0]), 233, 'the char' );


/// store UTF-8 to JS unicode [rmtf]

	var conv = new Iconv('UTF-8', 'ISO-8859-1', true, false); // source is not wide, dest is wide
	var res = conv('é');
	
	QA.ASSERT( res.length, 1, 'UC string length' );
	QA.ASSERT( String.charCodeAt(res[0]), 43459, '"é" is (c3 a9)' );

	var res = conv('éééé');
	QA.ASSERT( res.length, 4, 'UC string length' );
	QA.ASSERT( String.charCodeAt(res[0]), 43459, '"é" is (c3 a9)' );
	QA.ASSERT( String.charCodeAt(res[1]), 43459, '"é" is (c3 a9)' );
	QA.ASSERT( String.charCodeAt(res[2]), 43459, '"é" is (c3 a9)' );
	QA.ASSERT( String.charCodeAt(res[3]), 43459, '"é" is (c3 a9)' );


/// store UTF-16 to JS string and unicode string [rmtf]

	var conv = new Iconv('UTF-16', 'ISO-8859-1', false, false);
	var res = conv('abc');
	QA.ASSERT( res.length, 8, 'non-UC string length' );

	var conv = new Iconv('UTF-16', 'ISO-8859-1', true, false);
	var res = conv('abc');
	QA.ASSERT( res.length, 4, 'UC string length' );


/// store UTF-8 to JS string and unicode string [rmtf]

	var conv = new Iconv('UTF-8', 'ISO-8859-1', false, false); // source is not wide, dest is wide
	var res = conv('été');
	QA.ASSERT( res.length, 5, 'UC string length' );
	
	
	//var conv = new Iconv('UTF-8', 'ISO-8859-1', true, false); // source is not wide, dest is wide
	//var res = conv('été');
	//QA.ASSERT( res.length, 3, 'UC string length' );


/// store UCS-2le to JS unicode (1) [rmtf]
	
	var conv = new Iconv(Iconv.jsUC, 'ISO-8859-1', true, false); // source is not wide, dest is wide
	var res = conv('été');
	QA.ASSERT( res.length, 3, '"été" UC string length' );

	QA.ASSERT( res, 'été', 'string test' );


/// store UCS-2le to JS unicode (2) [rmtf]

	var conv = new Iconv(Iconv.jsUC, 'ISO-8859-1', true, false); // source is not wide, dest is wide

//	var src = "\u007A\u6C34";
	var src = "\u007A\u006C";
	var res = conv(src);

	QA.ASSERT( res.length, 2, 'check the resulting string length' );
	QA.ASSERT_STR( res, "zl", 'check the resulting string' );

/// store UCS-2le to JS unicode (3) [rmtf]

  var utf8str = "\xC3\xA9t\xC3\xA9 \xC3\xA0 la plage"; // été à la plage
  var conv = new Iconv(Iconv.jsUC, 'UTF-8', true, false); // source is not wide (8bit), dest is wide (16bit)
  var result = conv(utf8str);
  QA.ASSERT_STR( result, 'été à la plage', 'check the intermediate' );
  var conv = new Iconv('ISO-8859-1', Iconv.jsUC, false, true); // source is not wide (8bit), dest is wide (16bit)
  QA.ASSERT_STR( conv(result), 'été à la plage', 'check the resulting string' );


/// invalid char [rmtf]

	var conv = new Iconv('UTF-8', Iconv.jsUC, false, true);
	QA.ASSERT_STR( conv.invalidChar, '?', 'default invalidChar' );
	QA.ASSERT_EXCEPTION( function() { conv.invalidChar = '???' }, RangeError, 'invalid invalidChar' );
	conv.invalidChar = '.';
	QA.ASSERT_STR( conv.invalidChar, '.', 'new invalidChar' );


/// invalid multibyte sequence 1 [rmtf]

	var conv = new Iconv('850', 'UTF-8');
	
	conv.invalidChar = ' ';
	var result = conv('\xC3');
	QA.ASSERT( conv.hasIncompleteSequence, true, 'incomplete sequence' );
	conv();
	QA.ASSERT( conv.hasIncompleteSequence, false, 'reset' );

	var result = conv('\xC3');
	QA.ASSERT( conv.hasIncompleteSequence, true, 'first part' );
	result += conv('\xA9');
	QA.ASSERT( conv.hasIncompleteSequence, false, 'last part' );


/// invalid multibyte sequence 2 [rmtf]

	var conv = new Iconv('ISO-8859-1', 'UTF-8');
	var result = conv('\xC3');
	QA.ASSERT( conv.hasIncompleteSequence, true, 'first part' );
	result += conv('\xC3\xA9ZzZ');
	QA.ASSERT( conv.hasIncompleteSequence, false, 'last part' );
	QA.ASSERT_STR( result, '?éZzZ', 'last part' );
