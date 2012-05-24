loadModule('jsprotex');

/// back buffer size []

	var bump = new Texture(100, 100, 1);
	bump.resize( 300, 300 );
	bump.convolution([1]);
	bump.normals();


/// crash 1 [p]
	
	QA.ASSERTOP( function() new Texture(), 'ex', RangeError );
	QA.ASSERTOP( function() new Texture(100, 100, 5), 'ex', RangeError );


/// basic test 1 [p]

	var tmp = new Texture(10, 10, 4)
	tmp.set('red');
	QA.ASSERT(tmp.getPixelAt(0,0).join(','), '1,0,0,1', 'red color' );
	tmp.free();
