LoadModule('jslang');
LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jssound');

var f = new File('41_30secOgg-q0.ogg');
//f.Open( File.RDONLY );

//var pcm = DecodeOggVorbis(f);

//var pcm = DecodeOggVorbis(f.content);

var f = new File('41_30secOgg-q0.wav');
var pcm = DecodeSound(Stream(f.content));

Print(pcm.length, '\n');
