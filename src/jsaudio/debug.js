LoadModule('jsio');
LoadModule('jsstd');
LoadModule('jssound');
LoadModule('jsaudio');

var decoder = new OggVorbisDecoder(new File('41_30secOgg-q0.ogg').Open( File.RDONLY ));

var src = Oal.GenSource();

var pcm;
while ( pcm = decoder.Read(10000) ) {
	
	var b = Oal.Buffer(pcm);
	Oal.SourceQueueBuffers(src, b);
	if ( Oal.GetSourceInteger(src, Oal.SOURCE_STATE) == Oal.INITIAL )
		Oal.PlaySource(src);
};

var totalTime = decoder.frames/decoder.rate;
var currentTimeOffset = Oal.GetSourceReal(src, Oal.SEC_OFFSET);

Sleep( 1000 * (totalTime - currentTimeOffset) );

