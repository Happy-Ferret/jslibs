loadModule('jssound');


/// chunked stream

	loadModule('jsio');
	var audioFileName = QA.cx.item.path + '/41_30secOgg-q0.ogg';
	var data = new File(audioFileName).content;

	var stream = { pos:0, reqIndex:0 };
	stream.read = function(amount) {

		if ( this.reqIndex != undefined && ++this.reqIndex % 2 == 0 )
			return '';

		amount = Math.min(amount, data.byteLength - this.pos);
		var tmp = new Int8Array(data, this.pos, amount);
		this.pos += amount;
		return tmp;
	}

	var decoder = new OggVorbisDecoder( stream );
	var block = decoder.read();
	QA.ASSERT( block.frames, 310080, 'frames' );
	var block = decoder.read();
	QA.ASSERT( block.frames, 319104, 'frames' );
	stream.reqIndex = undefined;
	var block = decoder.read();
	QA.ASSERT( block.frames, 693817, 'frames' );
	var block = decoder.read();
	QA.ASSERT( block, undefined, 'no more frames' );


/// decode ogg using scripted stream

	loadModule('jsio');
	var audioFileName = QA.cx.item.path + '/41_30secOgg-q0.ogg';

	var data = new File(audioFileName).content;

	var stream = { pos:0 };

	stream.read = function(amount) {

		amount = Math.min(amount, data.byteLength - this.pos);
		var tmp = new Int8Array(data, this.pos, amount);
		this.pos += amount;
		return tmp;
	}

	var decoder = new OggVorbisDecoder( stream );
	var block = decoder.read();

	QA.ASSERT( block.frames, 1323001, 'frames' );



/// read partial sample

	loadModule('jsio');
	var audioFileName = QA.cx.item.path + '/41_30secOgg-q0.ogg';
	var data = new File(audioFileName).content;

	data = new Int8Array(data, 0, data.byteLength - 1);

	var decoder = new OggVorbisDecoder( new Stream(data) );
	var block = decoder.read();

	QA.ASSERT( block.frames, 1310016, 'frames' );



/// decode ogg using the Stream object

	loadModule('jsio');
	var audioFileName = QA.cx.item.path + '/41_30secOgg-q0.ogg';
	var data = new File(audioFileName).content;

	var decoder = new OggVorbisDecoder( new Stream(data) );
	var block = decoder.read();

	QA.ASSERT( block.frames, 1323001, 'frames' );


/// invalid stream object

	QA.IS_UNSAFE || QA.ASSERTOP( function() { new OggVorbisDecoder({}) }, 'ex', Error );
	QA.IS_UNSAFE || QA.ASSERTOP( function() { new SoundFileDecoder({}) }, 'ex', Error );


/// invalid arg cound []

	QA.IS_UNSAFE || QA.ASSERTOP( function() { new SoundFileDecoder() }, 'ex', RangeError );
	QA.IS_UNSAFE || QA.ASSERTOP( function() { new OggVorbisDecoder() }, 'ex', RangeError );


/// wav stream decoding error

	loadModule('jsio');
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

	var decoder = new SoundFileDecoder( stream ); // if trouble, check initial ierr value.
	QA.ASSERTOP( function() { decoder.read() }, 'ex', ReferenceError );
	file.close();


/// wav stream decoding
	
	loadModule('jsio');
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


/// wav file decoding more EOF

	loadModule('jsio');
	var audioFileName = QA.cx.item.path + '/41_30secOgg-q0_a.wav';
	var file = new File(audioFileName).open('r');
	var decoder = new SoundFileDecoder( file );

	QA.ASSERT( decoder.read(203539 + 1000).frames, 203539, 'frames' );
	QA.ASSERT( decoder.read(0).frames, 0, 'frames' );
	QA.ASSERT( decoder.read(), undefined, 'frames' );
	file.close();


/// wav file decoding EOF test

	loadModule('jsio');
	var audioFileName = QA.cx.item.path + '/41_30secOgg-q0_a.wav';
	var file = new File(audioFileName).open('r');
	var decoder = new SoundFileDecoder( file );

	QA.ASSERT( decoder.read(203539).frames, 203539, 'frames' );
	QA.ASSERT( decoder.read(), undefined, 'frames' );
	file.close();


/// wav file decoding 0 frames

	loadModule('jsio');
	var audioFileName = QA.cx.item.path + '/41_30secOgg-q0_a.wav';
	var file = new File(audioFileName).open('r');
	var decoder = new SoundFileDecoder( file );
	QA.ASSERT( decoder.read(0).frames, 0, 'frames' );
	QA.ASSERT( decoder.read(0).rate, 44100, 'rate' );
	file.close();


/// wav file decoding n frames

	loadModule('jsio');
	var audioFileName = QA.cx.item.path + '/41_30secOgg-q0_a.wav';
	var file = new File(audioFileName).open('r');
	var decoder = new SoundFileDecoder( file );
	QA.ASSERT( decoder.read(1000).frames, 1000, 'frames' );
	file.close();


/// wav file decoding all frames

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



/// ogg decoding non-opened file []

	loadModule('jsio');
	var file = new File(QA.cx.item.path + '/41_30secOgg-q0.ogg');
	QA.IS_UNSAFE || QA.ASSERTOP( function() { new OggVorbisDecoder( file ) }, 'ex', Error );


/// ogg decoding non-closed file []

	loadModule('jsio');
	var file = new File(QA.cx.item.path + '/41_30secOgg-q0.ogg');
	file.open('r');
	file.close();
	QA.IS_UNSAFE || QA.ASSERTOP( function() { new OggVorbisDecoder( file ) }, 'ex', Error );


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

