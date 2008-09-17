LoadModule('jsio');
LoadModule('jsstd');
LoadModule('jssound');
LoadModule('jsaudio');

Print('playing '+arguments[1] );

var decoder = new OggVorbisDecoder(new File(arguments[1]).Open( File.RDONLY ));

Oal.Open();
var src = Oal.GenSource();

var pcm, decodeSize = 1024;
while ( pcm = decoder.Read(decodeSize) ) {
	
	var b = Oal.Buffer(pcm);
	Oal.SourceQueueBuffers(src, b);
	if ( Oal.GetSourceInteger(src, Oal.SOURCE_STATE) == Oal.INITIAL )
		Oal.PlaySource(src);
	decodeSize *= 2;
}

var totalTime = decoder.frames/decoder.rate;
var currentTimeOffset = Oal.GetSourceReal(src, Oal.SEC_OFFSET);
Sleep( 1000 * (totalTime - currentTimeOffset) );

Oal.Close();
