loadModule('jsprotex');

/// back buffer size [t]

	var bump = new Texture(100, 100, 1);
	bump.resize( 300, 300 );
	bump.convolution([1]);
	bump.normals();
	
	
