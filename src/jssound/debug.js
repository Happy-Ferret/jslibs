LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jssound');

var file = new File('41_30secOgg-q0.wav');
var buffer = file.content;
var stream = new Stream(buffer);
var pcm = DecodeSound(stream);
Print('sample length: '+pcm.length, '\n');
