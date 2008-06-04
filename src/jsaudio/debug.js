LoadModule('jslang');
LoadModule('jsio');
LoadModule('jssound');
LoadModule('jsaudio');

var f = new File('../jssound/41_30secOgg-q0.ogg');
f.Open( File.RDONLY );
var pcm = DecodeOggVorbis(f);
Oal.PlaySound(pcm);
