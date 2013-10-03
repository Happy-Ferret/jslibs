var loadModule = host.loadModule;
// loadModule('jsstd');  loadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { print( id, ':', uneval(args), '\n' ) } };  exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  halt();
//loadModule('jsstd'); exec('../common/tools.js'); runQATests('jssound'); throw 0; // -inlineOnly


loadModule('jsstd');
loadModule('jsio');
loadModule('jssound');


var audioFileName = './41_30secOgg-q0.ogg';

var data = new File(audioFileName).content;

var data = new File('tmpFile').content;


var stream = new Stream(data);

var decoder = new OggVorbisDecoder( stream );

var block = decoder.read();

print( block.frames, '\n' );


throw 0;


loadModule('jsstd');
loadModule('jsio');
loadModule('jssound');


	loadModule('jsio');
	var audioFileName = '.' + '/41_30secOgg-q0_a.wav';
	var file = new File(audioFileName).open('r');
	var decoder = new SoundFileDecoder( file );


throw 0;


var audioFileName = './41_30secOgg-q0.ogg';

var data = new File(audioFileName).content;

var stream = new Stream(data);

var decoder = new OggVorbisDecoder( stream );

var block = decoder.read();

print( block.frames, '\n' );


throw 0;

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



