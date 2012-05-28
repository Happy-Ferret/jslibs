loadModule('jsimage');

/// png decode an invalid image

	QA.ASSERTOP( function() decodePngImage(new Stream('xxxxxxxxx')), 'ex', InternalError, 'invalid png file' );


/// png memory leak

	QA.IS_UNSAFE || QA.ASSERTOP( function() decodePngImage(), 'ex', RangeError, 'invalid decodePngImage arguemnt count' );


/// invalid jpeg image []

	QA.ASSERTOP( function() decodeJpegImage(new Stream('xxxxxxxxx')), 'ex', InternalError, 'invalid jpeg file' );


/// jpeg memory leak

	QA.IS_UNSAFE || QA.ASSERTOP( function() decodeJpegImage(), 'ex', RangeError, 'invalid decodeJpegImage arguemnt count' );


/// png decode an compressed image [p]

	loadModule('jsio');
	var image = decodePngImage(new File(QA.cx.item.path + '/z09n2c08.png').open(File.RDONLY));
	QA.ASSERT(image.width, 32);
	QA.ASSERT(image.height, 32);
	QA.ASSERT(image.channels, 3);
	QA.ASSERT(image.data.byteLength, 3072);


/// png decode an image (alpha)

	loadModule('jsio');
	var image = decodePngImage(new File(QA.cx.item.path + '/basn6a08.png').open(File.RDONLY));
	QA.ASSERT(image.width, 32);
	QA.ASSERT(image.height, 32);
	QA.ASSERT(image.channels, 4);
	QA.ASSERT(image.data.byteLength, 4096);


/// jpeg decode an image

	loadModule('jsio');
	var image = decodeJpegImage(new File(QA.cx.item.path + '/Patern_test.jpg').open(File.RDONLY));
	QA.ASSERT(image.width, 463);
	QA.ASSERT(image.height, 399);
	QA.ASSERT(image.channels, 3);
	QA.ASSERT(image.data.byteLength, 554211);


/// png encode an image

	loadModule('jsio');
	var image = decodeJpegImage(new File(QA.cx.item.path + '/Patern_test.jpg').open(File.RDONLY));
	
	var png = encodePngImage(image, 0);
	QA.ASSERT(png.byteLength, 555562);
	var png = encodePngImage(image, 9);
	QA.ASSERT(png.byteLength, 97356);


/// jpeg encode an image

	loadModule('jsio');
	var image = decodePngImage(new File(QA.cx.item.path + '/z09n2c08.png').open(File.RDONLY));

	var jpeg = encodeJpegImage(image, 0);
	QA.ASSERT(jpeg.byteLength, 650);
	var jpeg = encodeJpegImage(image, 100);
	QA.ASSERT(jpeg.byteLength, 1154);


/// jpeg encode an unsupported image

	var TYPE_UINT8 = 1;
	var image = { width:0, height:0, channels:3, type:TYPE_UINT8, data:'' };
	
	QA.ASSERTOP( function() encodeJpegImage(image, 100), 'ex', InternalError );


/// jpeg encode an invalid image

	var image = { width:0, height:0, channels:0, data:'' };
	QA.ASSERTOP( function() encodeJpegImage(image, 100), 'ex', Error );


/// jpeg encode the smallest image

	var TYPE_UINT8 = 1;
	var image = { width:1, height:1, channels:1, type:TYPE_UINT8, data:'X' };
	encodeJpegImage(image, 0);


/// jpeg encode invalid argument

	var image = { width:1, height:1, channels:1, data:'X' };
	QA.IS_UNSAFE || QA.ASSERTOP( function() encodeJpegImage(image, 0, 999), 'ex', RangeError );
