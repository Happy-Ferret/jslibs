loadModule('jsfont');

/// jsfont exported API

	QA.ASSERTOP( host, 'has', '_jsfontModulePrivate' );
	var prev = host._jsfontModulePrivate;
	host._jsfontModulePrivate = 123;
	QA.ASSERTOP( host._jsfontModulePrivate, '==', prev );


/// simple test

	var f = new Font(QA.cx.item.path + '/AnarchySans.otf');
	f.size = 32;
	f.verticalPadding = 10;
	f.horizontalPadding = 10;
	f.letterSpacing = 5;
	
	var image = f.drawString('Hello World', true);
	QA.ASSERTOP(image.width, '==', 280, 'text width');
	QA.ASSERTOP(image.height, '==', 65, 'text height');
	QA.ASSERTOP(image.channels, '==', 1, 'channels');
	QA.ASSERTOP(image.data.byteLength, '==', 18200, 'data size');


/// font not found

	QA.ASSERTOP( function() new Font(QA.cx.item.path + '/XXXXXXXXXX.otf'), 'ex', Error );


/// font small image

	var f = new Font(QA.cx.item.path + '/AnarchySans.otf');

	f.size = 0;
	var image = f.drawString('Hello World', true);
	QA.ASSERTOP(image.width, '==', 0, 'text width');
	QA.ASSERTOP(image.height, '==', 0, 'text height');

	f.size = 1;
	var image = f.drawString('Hello World', true);
	QA.ASSERTOP(image.width, '==', 7, 'text width');
	QA.ASSERTOP(image.height, '==', 3, 'text height');

	f.size = 2;
	var image = f.drawString('Hello World', true);
	QA.ASSERTOP(image.width, '==', 13, 'text width');
	QA.ASSERTOP(image.height, '==', 4, 'text height');
