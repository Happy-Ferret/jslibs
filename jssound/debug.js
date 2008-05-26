LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jssound');

var f = new File('41_30secOgg-b64M.ogg');
f.Open( File.RDONLY );

DecodeVorbis(f);