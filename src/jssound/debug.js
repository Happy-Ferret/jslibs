var loadModule = host.loadModule;
// loadModule('jsstd');  loadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { print( id, ':', uneval(args), '\n' ) } };  exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  halt();

loadModule('jsstd'); exec('../common/tools.js'); runQATests('jssound'); throw 0;


loadModule('jsstd');
loadModule('jsio');
loadModule('jsaudio');
loadModule('jssound');


//	var audioFileName = QA.cx.item.path + '/41_30secOgg-q0_a.wav';
	var audioFileName = '.' + '/41_30secOgg-q0_a.wav';

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
	
	var stream = file;
	var decoder = new SoundFileDecoder( stream );
	print( decoder.read().frames );
	file.close();
	
	

throw 0;

var audioFileName = './41_30secOgg-q0.ogg';



var file = new File(audioFileName).open('r');

var stream = { read: function(n) { return ile.read(n); } }

var decoder = new OggVorbisDecoder( stream );
var block = decoder.read();
file.close();


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



