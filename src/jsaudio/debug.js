LoadModule('jsio');
LoadModule('jsstd');
LoadModule('jssound');
LoadModule('jsaudio');

var f = new File('41_30secOgg-q0.ogg');
f.Open( File.RDONLY );
var pcm = DecodeOggVorbis(f);
Oal.PlaySound(pcm);
