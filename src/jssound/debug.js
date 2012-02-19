// loadModule('jsstd');  loadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { print( id, ':', uneval(args), '\n' ) } };  exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  Halt();

loadModule('jsstd');
loadModule('jsio');
loadModule('jsaudio');
loadModule('jssound');

Oal.Open();

var dec = new SoundFileDecoder( new File('41_30secOgg-q0.wav').Open('r') );
//var dec = new OggVorbisDecoder( new File('41_30secOgg-q0.ogg').Open('r') );
//var dec = new SoundFileDecoder( new File('break3.wav').Open('r') );

var block = dec.Read(100000);

//var [left, right] = SplitChannels(block);


print( block.bits, '\n' );
print( block.channels, '\n' );
print( block.rate, '\n' );
print( block.frames, '\n' );

var b = Oal.Buffer(block);
var src = Oal.GenSource();
Oal.SourceQueueBuffers(src, b);
Oal.PlaySource(src);

Sleep( 2000 );

Halt();

var t0 = IntervalNow();

do {
	var block = dec.Read(100000);
	print( 'frames: '+block.frames, '\n' );
} while(block);

print( IntervalNow() - t0, '\n' );



