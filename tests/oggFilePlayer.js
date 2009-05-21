LoadModule('jsio');
LoadModule('jsstd');
LoadModule('jssound');
LoadModule('jsaudio');

var filename = arguments[1] || '41_30secOgg-q0.ogg';

Print('playing '+filename, '\n' );

var decoder = new OggVorbisDecoder(new File(filename).Open(File.RDONLY));

Oal.Open();
var src = Oal.GenSource();

var pcm, decodeSize = 512;
while ( (pcm = decoder.Read(decodeSize)) && !endSignal ) {

	Oal.SourceQueueBuffers(src, Oal.Buffer(pcm));
	if ( Oal.GetSourceInteger(src, Oal.SOURCE_STATE) == Oal.INITIAL )
		Oal.PlaySource(src);
	decodeSize = pcm.frames * 2;
}

var totalTime = decoder.frames/decoder.rate;
var currentTimeOffset = Oal.GetSourceReal(src, Oal.SEC_OFFSET);
for ( i = 0; !endSignal && i < totalTime - currentTimeOffset; i++ )
	Sleep(1000);

Oal.Close();
