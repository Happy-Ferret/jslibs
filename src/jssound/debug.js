// LoadModule('jsstd');  LoadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { Print( id, ':', uneval(args), '\n' ) } };  Exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  Halt();

LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jsaudio');
LoadModule('jssound');



//var dec = new SoundFileDecoder( new File('41_30secOgg-q0.wav').Open('r') );
//var dec = new OggVorbisDecoder( new File('41_30secOgg-q0.ogg').Open('r') );
var dec = new SoundFileDecoder( new File('break3.wav').Open('r') );

var block = dec.Read(100000);

//var [left, right] = SplitChannels(block);


Print( block.bits, '\n' );
Print( block.channels, '\n' );
Print( block.rate, '\n' );
Print( block.frames, '\n' );

var b = Oal.Buffer(block);
var src = Oal.GenSource();
Oal.SourceQueueBuffers(src, b);
Oal.PlaySource(src);

Sleep( 2000 );

Halt();

var t0 = IntervalNow();

do {
	var block = dec.Read(100000);
	Print( 'frames: '+block.frames, '\n' );
} while(block);

Print( IntervalNow() - t0, '\n' );



