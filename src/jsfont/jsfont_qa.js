loadModule('jsfont');

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

	var f = new Font(QA.cx.item.path + '/fqsytfvusytfviyqsdv.otf');
