loadModule('jsimage');


/// invalid png image

	QA.ASSERTOP( function() decodePngImage(new Stream('xxxxxxxxx')), 'ex', InternalError, 'invalid png file' );


/// png memory leak

	QA.ASSERTOP( function() decodePngImage(), 'ex', RangeError, 'invalid decodePngImage arguemnt count' );


/// invalid jpeg image

	QA.ASSERTOP( function() decodeJpegImage(new Stream('xxxxxxxxx')), 'ex', InternalError, 'invalid jpeg file' );


/// jpeg memory leak

	QA.ASSERTOP( function() decodeJpegImage(), 'ex', RangeError, 'invalid decodeJpegImage arguemnt count' );


/// decompress jpeg image

	loadModule('jsio');
	var image = decodeJpegImage(new File(QA.cx.item.path + '/Patern_test.jpg').open(File.RDONLY));
	QA.ASSERT(image.width, 463);
	QA.ASSERT(image.height, 399);
	QA.ASSERT(image.channels, 3);
	QA.ASSERT(image.data.byteLength, 554211);


/// encode png image

	loadModule('jsio');
	var image = decodeJpegImage(new File(QA.cx.item.path + '/Patern_test.jpg').open(File.RDONLY));
	var png = encodePngImage(image, 0);
	QA.ASSERT(png.byteLength, 555562);
	var png = encodePngImage(image, 9);
	QA.ASSERT(png.byteLength, 97356);
