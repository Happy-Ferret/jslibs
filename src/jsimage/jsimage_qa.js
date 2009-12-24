LoadModule('jsimage');

/// load an image [d]

	var texture = new Jpeg(new File('R0010235.JPG').Open(File.RDONLY)).Load().Trim([10,10,20,20]);
	// (TBD) ( texture.width+'x'+texture.height+'x'+texture.channels, '\n' );
