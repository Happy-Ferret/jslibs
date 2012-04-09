loadModule('jssound');

/// wav stream decoding error

	var audioFileName = QA.cx.item.path + '/41_30secOgg-q0_a.wav';
	var file = new File(audioFileName).open('r');
	
	var ierr = 13;

	var stream = {
		read: function(n) {
			if ( !--ierr ) zzz(); 
			return file.read(n);
		},
		get position() {
			return file.position;
		},
		set position(pos) {
			file.position = pos;
		},
		get available() {
			return file.available;
		}
	}
	var decoder = new SoundFileDecoder( stream );
	QA.ASSERTOP( function() { decoder.read() }, 'ex', ReferenceError );
	file.close();


/// wav stream decoding

	var audioFileName = QA.cx.item.path + '/41_30secOgg-q0_a.wav';
	var file = new File(audioFileName).open('r');
	var stream = {
		read: function(n) {
			return file.read(n);
		},
		get position() {
			return file.position;
		},
		set position(pos) {
			file.position = pos;
		},
		get available() {
			return file.available;
		}
	}
	var decoder = new SoundFileDecoder( stream );
	QA.ASSERT( decoder.read().frames, 203539, 'frames' );
	file.close();


/// wav file decoding

	loadModule('jsio');
	var audioFileName = QA.cx.item.path + '/41_30secOgg-q0_a.wav';
	var file = new File(audioFileName).open('r');
	var decoder = new SoundFileDecoder( file );
	QA.ASSERT( decoder.read().frames, 203539, 'frames' );
	file.close();


/// ogg decoding read() error

	var stream = { read: function(n) { zervazfvzafv; } }
	QA.ASSERTOP( function() { new OggVorbisDecoder( stream ) }, 'ex', ReferenceError );


/// ogg stream decoding

	loadModule('jsio');

	var audioFileName = QA.cx.item.path + '/41_30secOgg-q0.ogg';
	
	var file = new File(audioFileName).open('r');

	var stream = { read: function(n) { return file.read(n); } }

	var decoder = new OggVorbisDecoder( stream );
	var block = decoder.read();
	
	file.close();
	
	QA.ASSERT( block.frames, 1323001, 'frames' );



/// ogg decoding stream error

	loadModule('jsio');

	var audioFileName = QA.cx.item.path + '/41_30secOgg-q0.ogg';
	
	var file = new File(audioFileName).open('r');

	var ierr = 2;
	var stream = { read: function(n) { print('.'); if ( !--ierr ) zzz(); return file.read(n) } }

	var decoder = new OggVorbisDecoder( stream );

	QA.ASSERTOP( function() { decoder.read() }, 'ex', ReferenceError );

	file.close();



/// ogg file decoding

	loadModule('jsio');

	var audioFileName = QA.cx.item.path + '/41_30secOgg-q0.ogg';

	var file = new File(audioFileName).open('r');
	var decoder = new OggVorbisDecoder( file );
	var block = decoder.read(1000);
	file.close();

	QA.ASSERT( block.bits, 16, 'bits' );
	QA.ASSERT( block.channels, 2, 'channels' );
	QA.ASSERT( block.rate, 44100, 'rate' );
	QA.ASSERT( block.frames, 1000, 'frames' );



/// ogg whole file decoding

	loadModule('jsio');

	var audioFileName = QA.cx.item.path + '/41_30secOgg-q0.ogg';

	var file = new File(audioFileName).open('r');
	var decoder = new OggVorbisDecoder( file );
	var block = decoder.read();
	file.close();

	QA.ASSERT( block.bits, 16, 'bits' );
	QA.ASSERT( block.channels, 2, 'channels' );
	QA.ASSERT( block.rate, 44100, 'rate' );
	QA.ASSERT( block.frames, 1323001, 'frames' );



/// ogg decoding closed file

	loadModule('jsio');
	var file = new File(QA.cx.item.path + '/41_30secOgg-q0.ogg');
	file.close();
	QA.ASSERTOP( function() { new OggVorbisDecoder( file ) }, 'ex', Error );



/// split audio data

	loadModule('jsio');

	var audioFileName = QA.cx.item.path + '/41_30secOgg-q0.ogg';

	var file = new File(audioFileName).open('r');
	var decoder = new OggVorbisDecoder( file );
	var block = decoder.read(1000);
	file.close();

	var [left, right] = splitChannels(block);

	QA.ASSERT( left.bits, 16, 'bits' );
	QA.ASSERT( left.channels, 1, 'channels' );
	QA.ASSERT( left.rate, 44100, 'rate' );
	QA.ASSERT( left.frames, 1000, 'frames' );
	
	QA.ASSERT( right.bits, 16, 'bits' );
	QA.ASSERT( right.channels, 1, 'channels' );
	QA.ASSERT( right.rate, 44100, 'rate' );
	QA.ASSERT( right.frames, 1000, 'frames' );
