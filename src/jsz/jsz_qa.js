LoadModule('jsz');

/// deflate ratio 1 [ftr]
		
		var uncompressezText = 'jjjjjjjjjjjssssssssssssssssssslllllliiiiiiiibbsssssssssssssss';
		var level = 9; // 0..9
		var compressedText = new Z(Z.DEFLATE, level)(uncompressezText, true);
		var ratio = (100*compressedText.length/uncompressezText.length).toFixed(2)+'%';
		QA.ASSERT( ratio, '37.70%', 'Bad compression ratio' );


/// deflate ratio 2 [ftr]
		
		var uncompressezText = 'zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz';

		var deflater = new Z(Z.DEFLATE, 9);
		var compressedText = deflater(uncompressezText);
		compressedText += deflater(uncompressezText);
//		QA.ASSERT( deflater.eof, false, 'eof before the end' );
		compressedText += deflater(uncompressezText, true); //		compressedText += deflater();
//		QA.ASSERT( deflater.eof, true, 'eof after the end' );
		QA.ASSERT( compressedText.length, deflater.lengthOut, 'compressed text length' );
		QA.ASSERT( deflater.adler32, 2686765097, 'adler32' );
		QA.ASSERT( deflater.lengthIn, 420, 'lengthIn property' );
		QA.ASSERT( deflater.lengthOut, 28, 'lengthOut property' );


/// inflate object reusability [ftr]

		var data = 'clear data';
		var deflatedData = new Z(Z.DEFLATE)(data, true);
		var inflater = new Z(Z.INFLATE);

		QA.ASSERT( inflater(data,true), data, '1st complete inflate' );
		QA.ASSERT( inflater(data,true), data, '2nd complete inflate' );
		QA.ASSERT( inflater(data,true), data, '3rd complete inflate' );


/// deflate object reusability [ftr]

		var data = 'clear data';
		var deflatedData = new Z(Z.DEFLATE)(data, true);
		var deflater = new Z(Z.DEFLATE);

		QA.ASSERT( deflater(data,true), deflatedData, '1st complete deflate' );
		QA.ASSERT( deflater(data,true), deflatedData, '2nd complete deflate' );
		QA.ASSERT( deflater(data,true), deflatedData, '3rd complete deflate' );


/// inflate / deflate [ftr]
		
		var deflate = new Z(Z.DEFLATE);
		var inflate = new Z(Z.INFLATE);
		var source = 'x';
		var str = deflate(source, true);	
		QA.ASSERT_STR( str, "x\x9C\xAB\0\0\0y\0y", 'deflate result' );
		var result = inflate(str, true);
		QA.ASSERT_STR( result, source, 'inflate result' );


/// inflate / deflate 2 [fr]

		var deflate = new Z(Z.DEFLATE);
		var inflate = new Z(Z.INFLATE);
		var source = QA.RandomString(10000);
		var str = deflate(source, true);	
		var result = inflate(str, true);
		QA.ASSERT_STR( result, source, 'inflate result' );


/// inflate / deflate huge [r]

		var deflate = new Z(Z.DEFLATE);
		var inflate = new Z(Z.INFLATE);

		for ( var i = 0; i < 200; i++ )
			inflate(deflate(QA.RandomString(10000)));
		inflate(deflate());
		
		QA.ASSERT( deflate.lengthIn, 2000000 , 'input length' );
		QA.ASSERT( deflate.lengthIn, inflate.lengthOut, 'in and out length' );
		QA.ASSERT( deflate.adler32, inflate.adler32, 'adler32 match' );


/// end of data methods [ftr]

		var source = QA.RandomString(10000);

		var deflate1 = new Z(Z.DEFLATE);
		deflate1(source, true);
		
		var deflate2 = new Z(Z.DEFLATE);
		deflate2(source);
		deflate2();
		
		QA.ASSERT( deflate1.lengthOut <= deflate2.lengthOut, true, 'lengthOut' );
		QA.ASSERT( deflate1.adler32, deflate2.adler32 , 'adler32' );


