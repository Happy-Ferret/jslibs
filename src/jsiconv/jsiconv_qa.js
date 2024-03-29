loadModule('jsiconv');

/// basic tests [p]

	QA.ASSERT( new Iconv('UTF-8', 'ISO-8859-1').process('�').length, 2, '"�" char length in UTF-8' );

	var conv = new Iconv('UTF-8', 'ISO-8859-1');
	var invConv = new Iconv('ISO-8859-1', 'UTF-8');
	var converted = conv.process('�t�');
	var result = invConv.process(converted);
	QA.ASSERT_STR( result, '�t�', 'string convertion' );


	var conv = new Iconv('UTF-8', 'ISO-8859-1');
	var invConv = new Iconv('ISO-8859-1', 'UTF-8');
	var converted = conv.process('�t�');
	var result = '';
	for each ( var c in converted )
		result += invConv.process(c);
	QA.ASSERT_STR( result, '�t�', 'char by char convertion' );


/// invalid sequense detection [p]

	var utf8 = (new Iconv('UTF-8', 'ISO-8859-1')).process('�t�').split('');
	utf8.splice(1,0,'x'); // insert an invalid multybyte sequense
	utf8 = utf8.join('');
	var res = (new Iconv('ISO-8859-1', 'UTF-8')).process(utf8);
	
	QA.ASSERT_STR( res[0], '?', 'invalid sequense char' )
	QA.ASSERT_STR( res[2], '?', 'invalid sequense char' )


/// identity [p]

	var ui = new Iconv('UTF-8', 'ISO-8859-1');
	var iu = new Iconv('ISO-8859-1', 'UTF-8');
	
	for ( var cc = 0; cc < 256; ++cc ) {
		
		var c0 = String.fromCharCode(cc);
		var c1 = iu.process(ui.process(c0));
		QA.ASSERT_STR( c0, c1, 'char '+cc );
	}


/// encoding list [p]

	var list = Iconv.list;
//	QA.ASSERT( list.length, 387, 'encoding count' );
	QA.ASSERT( list.indexOf('UTF-8'), 11, 'has UTF-8' );
	QA.ASSERT( list.indexOf('ISO-8859-1'), 48, 'has ISO-8859-1' );


/// test javascrit unicode char [p]

	var conv = new Iconv(Iconv.jsUC, 'ISO-8859-1', true, true);
	var res = conv.process('�t�');
	
	QA.ASSERT( res.length, 6, 'string length' );
	QA.ASSERT( String.charCodeAt(res[0]), 233, 'the char' );


/// store UTF-8 to JS unicode [p]

	var conv = new Iconv('UTF-8', 'ISO-8859-1', true, false); // source is not wide, dest is wide
	var res = conv.process('�');
	
	QA.ASSERT( res.length, 1, 'UC string length' );
	QA.ASSERT( String.charCodeAt(res[0]), 43459, '"�" is (c3 a9)' );

	var res = conv.process('����');
	QA.ASSERT( res.length, 4, 'UC string length' );
	QA.ASSERT( String.charCodeAt(res[0]), 43459, '"�" is (c3 a9)' );
	QA.ASSERT( String.charCodeAt(res[1]), 43459, '"�" is (c3 a9)' );
	QA.ASSERT( String.charCodeAt(res[2]), 43459, '"�" is (c3 a9)' );
	QA.ASSERT( String.charCodeAt(res[3]), 43459, '"�" is (c3 a9)' );


/// store UTF-16 to JS string and unicode string [p]

	var conv = new Iconv('UTF-16', 'ISO-8859-1', false, false);
	var res = conv.process('abc');
	QA.ASSERT( res.length, 8, 'non-UC string length' );

	var conv = new Iconv('UTF-16', 'ISO-8859-1', true, false);
	var res = conv.process('abc');
	QA.ASSERT( res.length, 4, 'UC string length' );


/// store UTF-8 to JS string and unicode string [p]

	var conv = new Iconv('UTF-8', 'ISO-8859-1', false, false); // source is not wide, dest is wide
	var res = conv.process('�t�');
	QA.ASSERT( res.length, 5, 'UC string length' );
	
	
	//var conv = new Iconv('UTF-8', 'ISO-8859-1', true, false); // source is not wide, dest is wide
	//var res = conv('�t�');
	//QA.ASSERT( res.length, 3, 'UC string length' );


/// store UCS-2le to JS unicode (1) [p]
	
	var conv = new Iconv(Iconv.jsUC, 'ISO-8859-1', true, false); // source is not wide, dest is wide
	var res = conv.process('�t�');
	QA.ASSERT( res.length, 3, '"�t�" UC string length' );

	QA.ASSERT( res, '�t�', 'string test' );


/// store UCS-2le to JS unicode (2) [p]

	var conv = new Iconv(Iconv.jsUC, 'ISO-8859-1', true, false); // source is not wide, dest is wide

//	var src = "\u007A\u6C34";
	var src = "\u007A\u006C";
	var res = conv.process(src);

	QA.ASSERT( res.length, 2, 'check the resulting string length' );
	QA.ASSERT_STR( res, "zl", 'check the resulting string' );

/// store UCS-2le to JS unicode (3) [p]

  var utf8str = "\xC3\xA9t\xC3\xA9 \xC3\xA0 la plage"; // �t� � la plage
  var conv = new Iconv(Iconv.jsUC, 'UTF-8', true, false); // source is not wide (8bit), dest is wide (16bit)
  var result = conv.process(utf8str);
  QA.ASSERT_STR( result, '�t� � la plage', 'check the intermediate' );
  var conv = new Iconv('ISO-8859-1', Iconv.jsUC, false, true); // source is not wide (8bit), dest is wide (16bit)
  QA.ASSERT_STR( conv.process(result), '�t� � la plage', 'check the resulting string' );


/// invalid char [p]

	var conv = new Iconv('UTF-8', Iconv.jsUC, false, true);
	QA.ASSERT_STR( conv.invalidChar, '?', 'default invalidChar' );
	conv.invalidChar = 'X';
	conv.invalidChar = '???';
	QA.ASSERTOP( conv.invalidChar, '==', '?', 'invalid invalidChar' );
	conv.invalidChar = '.';
	QA.ASSERT_STR( conv.invalidChar, '.', 'new invalidChar' );


/// invalid multibyte sequence 1 [p]

	var conv = new Iconv('850', 'UTF-8');
	
	conv.invalidChar = ' ';
	var result = conv.process('\xC3');
	QA.ASSERT( conv.hasIncompleteSequence, true, 'incomplete sequence' );
	conv.process();
	QA.ASSERT( conv.hasIncompleteSequence, false, 'reset' );

	var result = conv.process('\xC3');
	QA.ASSERT( conv.hasIncompleteSequence, true, 'first part' );
	result += conv.process('\xA9');
	QA.ASSERT( conv.hasIncompleteSequence, false, 'last part' );


/// invalid multibyte sequence 2 [p]

	var conv = new Iconv('ISO-8859-1', 'UTF-8');
	var result = conv.process('\xC3');
	QA.ASSERT( conv.hasIncompleteSequence, true, 'first part' );
	result += conv.process('\xC3\xA9ZzZ');
	QA.ASSERT( conv.hasIncompleteSequence, false, 'last part' );
	QA.ASSERT_STR( result, '?�ZzZ', 'last part' );
