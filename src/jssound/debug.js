// loadModule('jsstd');  loadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { print( id, ':', uneval(args), '\n' ) } };  exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  halt();

loadModule('jsstd');
loadModule('jsio');
loadModule('jsaudio');
loadModule('jssound');

Oal.open();

var dec = new SoundFileDecoder( new File('41_30secOgg-q0.wav').open('r') );
//var dec = new OggVorbisDecoder( new File('41_30secOgg-q0.ogg').Open('r') );
//var dec = new SoundFileDecoder( new File('break3.wav').Open('r') );

var block = dec.read(100000);

//var [left, right] = SplitChannels(block);


print( block.bits, '\n' );
print( block.channels, '\n' );
print( block.rate, '\n' );
print( block.frames, '\n' );

var b = Oal.Buffer(block);
var src = Oal.genSource();
Oal.sourceQueueBuffers(src, b);
Oal.playSource(src);

sleep( 2000 );

halt();

var t0 = intervalNow();

do {
	var block = dec.read(100000);
	print( 'frames: '+block.frames, '\n' );
} while(block);

print( intervalNow() - t0, '\n' );



