	loadModule('jsimage');

/// load an image

	loadModule('jsio');
	var image = decodeJpegImage(new File(QA.cx.item.path + '/Patern_test.jpg').open(File.RDONLY));
	QA.ASSERT(image.width, 463);
	QA.ASSERT(image.height, 399);
	QA.ASSERT(image.channels, 3);
	QA.ASSERT(image.data.byteLength, 554211);

/// encode PNG image

	loadModule('jsio');
	var image = decodeJpegImage(new File(QA.cx.item.path + '/Patern_test.jpg').open(File.RDONLY));
	var png = encodePngImage(image, 0);
	QA.ASSERT(png.byteLength, 555562);
	var png = encodePngImage(image, 9);
	QA.ASSERT(png.byteLength, 97356);
