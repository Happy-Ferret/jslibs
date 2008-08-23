LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jssound');

/*
var file = new File('41_30secOgg-q0.wav');
file.Open('r');
var dec = new SoundFileDecoder( file );
*/

var file = new File('41_30secOgg-q0.ogg');
file.Open('r');
var dec = new OggVorbisDecoder( file );


Print( dec.bits, '\n' );
Print( dec.channels, '\n' );
Print( dec.rate, '\n' );
Print( dec.frames, '\n' );


var t0 = IntervalNow();

do {
	var block = dec.Read(100000);
	Print( 'frames: '+block.frames, '\n' );
} while(block);

Print( IntervalNow() - t0, '\n' );



