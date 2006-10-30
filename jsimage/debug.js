LoadModule('jsstd');
LoadModule('jsimage');
LoadModule('jsnspr');

var f = new File('pngtest.png');
f.Open( File.RDONLY );
var img = new Image(f);
img.Load();