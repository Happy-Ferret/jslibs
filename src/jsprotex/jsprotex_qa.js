LoadModule('jsprotex');

/// back buffer size [ft]

	var bump = new Texture(100, 100, 1);
	bump.Resize( 300, 300 );
	bump.Convolution([1]);
	bump.Normals();
	
	