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
var block = dec.Read(100000);

Print( dec.bits, '\n' );
Print( dec.channels, '\n' );
Print( dec.rate, '\n' );
Print( dec.frames, '\n' );

var [left, right] = SplitChannels(block);

var src = Oal.GenSource();
Oal.PlaySource(src);
var b = Oal.Buffer(left);
Oal.SourceQueueBuffers(src, b);

Sleep( 2000 );



Halt();

var t0 = IntervalNow();

do {
	var block = dec.Read(100000);
	Print( 'frames: '+block.frames, '\n' );
} while(block);

Print( IntervalNow() - t0, '\n' );



