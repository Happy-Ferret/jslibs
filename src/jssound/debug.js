LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jssound');

var file = new File('41_30secOgg-q0.wav');

//var buffer = file.content;
//var stream = new Stream(buffer);
//var pcm = DecodeSound(stream);
//Print('sample length: '+pcm.length, '\n');

//Print( typeof buffer, '\n' );
//Print( buffer._NI_BufferRead, '\n' );

file.Open('r');
var dec = new SoundFileDecoder( file );

var block = dec.Read();

Print( dec.bits, '\n' );
Print( dec.channels, '\n' );
Print( dec.rate, '\n' );
Print( block.frames, '\n' );
