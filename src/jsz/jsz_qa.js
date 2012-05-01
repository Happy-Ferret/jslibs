loadModule('jsz');

/// empty operation [p]

		var deflate = new Z(Z.DEFLATE);
		var inflate = new Z(Z.INFLATE);
		inflate(deflate());

/// no erport on GC [p]

		var deflater = new Z(Z.DEFLATE, 9);
		var compressedText = deflater('xxx');

/// deflate ratio 1 [p]
		
		var uncompressezText = 'jjjjjjjjjjjssssssssssssssssssslllllliiiiiiiibbsssssssssssssss';
		var level = 9; // 0..9
		var compressedText = new Z(Z.DEFLATE, level)(uncompressezText, true);
		var ratio = (100*compressedText.byteLength/uncompressezText.length).toFixed(2)+'%';
		QA.ASSERT( ratio, '37.70%', 'Bad compression ratio' );


/// deflate ratio 2 [p]
		
		var uncompressezText = 'zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz';

		var deflater = new Z(Z.DEFLATE, 9);
		QA.ASSERT( deflater.idle, true, 'idle' );
		var compressedText = deflater(uncompressezText);
		QA.ASSERT( deflater.idle, false, 'idle' );
		compressedText = join([compressedText, deflater(uncompressezText)]);
		QA.ASSERT( deflater.idle, false, 'idle' );
		compressedText = join([compressedText, deflater(uncompressezText, true)]); //		compressedText += deflater();
		QA.ASSERT( deflater.idle, true, 'idle' );

		QA.ASSERT( compressedText.length, deflater.lengthOut, 'compressed text length' );
		QA.ASSERT( deflater.adler32, 2686765097, 'adler32' );
		QA.ASSERT( deflater.lengthIn, 3 * uncompressezText.length, 'lengthIn property' );

/// inflate object reusability [p]

		var data = 'clear data';
		var deflatedData = new Z(Z.DEFLATE)(data, true);
		var inflater = new Z(Z.INFLATE);

		QA.ASSERT_STR( inflater(deflatedData,true), data, '1st complete inflate' );
		QA.ASSERT_STR( inflater(deflatedData,true), data, '2nd complete inflate' );
		QA.ASSERT_STR( inflater(deflatedData,true), data, '3rd complete inflate' );
		

/// deflate object reusability [p]

		var data = 'clear data';
		var deflatedData = new Z(Z.DEFLATE)(data, true);
		var deflater = new Z(Z.DEFLATE);

		QA.ASSERT_STR( deflater(data,true), deflatedData, '1st complete deflate' );
		QA.ASSERT_STR( deflater(data,true), deflatedData, '2nd complete deflate' );
		QA.ASSERT_STR( deflater(data,true), deflatedData, '3rd complete deflate' );
		QA.ASSERT( deflater(data,true).length, deflatedData.length, '4rd complete deflate length' );

/// inflate / deflate [p]
		
		var deflate = new Z(Z.DEFLATE);
		var inflate = new Z(Z.INFLATE);
		var source = 'x';
		var str = deflate(source, true);	
		QA.ASSERT_STR( str, "x\x9C\xAB\0\0\0y\0y", 'deflate result' );
		var result = inflate(str, true);
		QA.ASSERT_STR( result, source, 'inflate result' );


/// inflate / deflate 2 [p]

		var deflate = new Z(Z.DEFLATE);
		var inflate = new Z(Z.INFLATE);
		var source = QA.randomString(10000);
		var str = deflate(source, true);	
		var result = inflate(str, true);
		QA.ASSERT_STR( result, source, 'inflate result' );


/// inflate / deflate huge []

		var deflate = new Z(Z.DEFLATE);
		var inflate = new Z(Z.INFLATE);

		for ( var i = 0; i < 200; i++ )
			inflate(deflate(QA.randomString(10000)));
		inflate(deflate());
		
		QA.ASSERT( deflate.lengthIn, 2000000 , 'input length' );
		QA.ASSERT( deflate.lengthIn, inflate.lengthOut, 'in and out length' );
		QA.ASSERT( deflate.adler32, inflate.adler32, 'adler32 match' );


/// end of data methods [p]

		var source = QA.randomString(10000);

		var deflate1 = new Z(Z.DEFLATE);
		deflate1(source, true);
		
		var deflate2 = new Z(Z.DEFLATE);
		deflate2(source);
		deflate2();
		
		QA.ASSERT( deflate1.lengthOut <= deflate2.lengthOut, true, 'lengthOut' );
		QA.ASSERT( deflate1.adler32, deflate2.adler32 , 'adler32' );
