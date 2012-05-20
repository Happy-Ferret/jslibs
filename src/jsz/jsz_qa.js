loadModule('jsz');

/// empty operation [p]

		var deflate = new Z(Z.DEFLATE);
		var inflate = new Z(Z.INFLATE);
		inflate.process(deflate.process());

/// no erport on GC [p]

		var deflater = new Z(Z.DEFLATE, 9);
		var compressedText = deflater.process('xxx');

/// deflate ratio 1 [p]
		
		var uncompressezText = 'jjjjjjjjjjjssssssssssssssssssslllllliiiiiiiibbsssssssssssssss';
		var level = 9; // 0..9
		var compressedText = new Z(Z.DEFLATE, level).process(uncompressezText, true);
		var ratio = (100 * compressedText.byteLength / uncompressezText.length).toFixed(2)+'%';
		QA.ASSERT( ratio, '37.70%', 'Bad compression ratio' );


/// deflate ratio 2 [p]
		
		var uncompressezText = 'zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz';

		var deflater = new Z(Z.DEFLATE, 9);
		QA.ASSERT( deflater.idle, true, 'idle' );
		var compressedText = deflater.process(uncompressezText);
		QA.ASSERT( deflater.idle, false, 'idle' );
		compressedText = join([compressedText, deflater.process(uncompressezText)]);
		QA.ASSERT( deflater.idle, false, 'idle' );
		compressedText = join([compressedText, deflater.process(uncompressezText, true)]); //		compressedText += deflater();
		QA.ASSERT( deflater.idle, true, 'idle' );

		QA.ASSERT( compressedText.length, deflater.lengthOut, 'compressed text length' );
		QA.ASSERT( deflater.adler32, 2686765097, 'adler32' );
		QA.ASSERT( deflater.lengthIn, 3 * uncompressezText.length, 'lengthIn property' );

/// inflate object reusability [p]

		var data = 'clear data';
		var deflatedData = new Z(Z.DEFLATE).process(data, true);
		var inflater = new Z(Z.INFLATE);

		QA.ASSERT_STR( inflater.process(deflatedData, true), data, '1st complete inflate' );
		QA.ASSERT_STR( inflater.process(deflatedData, true), data, '2nd complete inflate' );
		QA.ASSERT_STR( inflater.process(deflatedData, true), data, '3rd complete inflate' );
		

/// deflate object reusability [p]

	var data = 'clear data';
	var deflatedData = new Z(Z.DEFLATE).process(data, true);
	var deflater = new Z(Z.DEFLATE);

	QA.ASSERT_STR( deflater.process(data, true), deflatedData, '1st complete deflate' );
	QA.ASSERT_STR( deflater.process(data, true), deflatedData, '2nd complete deflate' );
	QA.ASSERT_STR( deflater.process(data, true), deflatedData, '3rd complete deflate' );
	QA.ASSERT( deflater.process(data, true).length, deflatedData.length, '4rd complete deflate length' );


/// inflate / deflate [p]
		
	var deflate = new Z(Z.DEFLATE);
	var inflate = new Z(Z.INFLATE);
	var source = 'x';
	var str = deflate.process(source, true);	
	QA.ASSERT_STR( str, "x\x9C\xAB\0\0\0y\0y", 'deflate result' );
	var result = inflate.process(str, true);
	QA.ASSERT_STR( result, source, 'inflate result' );


/// inflate / deflate 2 [p]

	var deflate = new Z(Z.DEFLATE);
	var inflate = new Z(Z.INFLATE);
	var source = QA.randomString(10000);
	var str = deflate.process(source, true);	
	var result = inflate.process(str, true);
	QA.ASSERT_STR( result, source, 'inflate result' );


/// inflate / deflate huge []

	var deflate = new Z(Z.DEFLATE);
	var inflate = new Z(Z.INFLATE);

	for ( var i = 0; i < 200; i++ )
		inflate.process(deflate.process(QA.randomString(10000)));
	inflate.process(deflate.process());

	QA.ASSERT( deflate.lengthIn, 2000000 , 'input length' );
	QA.ASSERT( deflate.lengthIn, inflate.lengthOut, 'in and out length' );
	QA.ASSERT( deflate.adler32, inflate.adler32, 'adler32 match' );


/// end of data methods [p]

	var source = QA.randomString(10000);

	var deflate1 = new Z(Z.DEFLATE);
	deflate1.process(source, true);

	var deflate2 = new Z(Z.DEFLATE);
	deflate2.process(source);
	deflate2.process();

	QA.ASSERT( deflate1.lengthOut <= deflate2.lengthOut, true, 'lengthOut' );
	QA.ASSERT( deflate1.adler32, deflate2.adler32 , 'adler32' );


/// zipfile date

	var f = new ZipFile('_qa_tmp.zip');
	f.open(ZipFile.CREATE);
	
	f.select('test1.txt');
	f.date = new Date(2008,6,4);
	f.write('');
	
	f.select('test2.txt');
	f.date = undefined;
	f.write('');
	
	f.select('test3.txt');
	var d = new Date();
	d.setMilliseconds(0);
	d.setSeconds(30);
	f.date = d;
	f.write('');
	
	f.close();

	//

	var g = new ZipFile('_qa_tmp.zip');
	g.open(ZipFile.READ);
	g.select('test1.txt');
	QA.ASSERT(+g.date, +new Date(2008, 6, 4));

	g.select('test2.txt');
	QA.ASSERT(g.date, undefined);

	g.select('test3.txt');
	QA.ASSERT(+g.date, +d);

	g.close();

	new File('_qa_tmp.zip').content = undefined;


/// zipfile full test

	var f = new ZipFile('_qa_tmp.zip');
	f.open(ZipFile.CREATE);
	f.level = 9;
	var d = new Date(2008,6,4);
	for ( var i = 0; i < 10; ++i ) {

		f.select('file'+i+'.txt');
		f.date = d;
		f.write('content data '+i+' '+stringRepeat(i, 10));
	}
	f.close();

	var g = new ZipFile('_qa_tmp.zip');
	g.open(ZipFile.READ);

	g.select('file9.txt');
	QA.ASSERT( g.filename, 'file9.txt');
	QA.ASSERT( stringify(g.read()), 'content data 9 9999999999');
	
	g.select('file9.txt');
	QA.ASSERT( stringify(g.read()), 'content data 9 9999999999');
	
	g.goTo(5);
	QA.ASSERT( g.filename, 'file5.txt');
	QA.ASSERT( stringify(g.read()), 'content data 5 5555555555');
	
	g.goNext();
	QA.ASSERT( stringify(g.read()), 'content data 6 6666666666');
	QA.ASSERT( g.read(), undefined);
	
	g.goFirst()
	QA.ASSERT( stringify(g), 'content data 0 0000000000' );
	QA.ASSERT( g.filename, 'file0.txt' );
	QA.ASSERT( g.level, 9 );
	QA.ASSERT( +g.date, +d );
	
	g.goTo(9);
	g.goNext();
	QA.ASSERT( g.eol, true );

	g.close();

	new File('_qa_tmp.zip').content = undefined;



/// zipfile multiple read

	var f = new ZipFile('_qa_tmp.zip');
	f.open(ZipFile.CREATE);
	f.select('toto/xxx.txt');
	f.date = new Date(2008,6,4);
	f.globalComment = 'a global comment';
	f.extra = 'extra field';
	f.write('123456789');
	f.close();

	var g = new ZipFile('_qa_tmp.zip');
	g.open(ZipFile.READ);
	g.select('toto/xxx.txt');
	
	QA.ASSERT( stringify(g.globalComment), 'a global comment' );
	QA.ASSERT( g.filename, 'toto/xxx.txt' );

	QA.ASSERT( stringify(g.read(0)), '' );
	QA.ASSERT( stringify(g.read(3)), '123' );
	QA.ASSERT( stringify(g.read(0)), '' );
	QA.ASSERT( stringify(g.read(1)), '4' );
	QA.ASSERT( stringify(g.read()), '56789' );
	QA.ASSERT( g.read(), undefined );

	QA.ASSERT( stringify(g.extra), 'extra field' );

	g.close();

	new File('_qa_tmp.zip').content = undefined;


/// zipfile not reusable after close

	var f = new ZipFile('_qa_tmp.zip');
	f.open(ZipFile.CREATE);
	f.write('');
	f.close();

	var g = new ZipFile('_qa_tmp.zip');
	g.open(ZipFile.READ);
	g.close();

	QA.ASSERTOP( function() g.open(ZipFile.READ), 'ex', Error );
	QA.ASSERTOP( function() g.close(), 'ex', Error );

	new File('_qa_tmp.zip').content = undefined;


/// zipfile stream

	var f = new ZipFile('_qa_tmp.zip');
	f.open(ZipFile.CREATE);
	f.select('test.txt');
	f.write('123456789');
	f.close();

	var g = new ZipFile('_qa_tmp.zip');
	g.open(ZipFile.READ);
	g.select('test.txt');
	g.read(1);
	QA.ASSERT( stringify(g), '23456789' );
	QA.ASSERT( stringify(g).length, 0 );
	g.close();

	new File('_qa_tmp.zip').content = undefined;
