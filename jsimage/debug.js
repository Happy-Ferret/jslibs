LoadModule('jsstd');
LoadModule('jsimage');
LoadModule('jsnspr');

var f = new File('pnglogo-blk-sml1.png');
f.Open( File.RDONLY );
var img = new Image(f);
Print( img.width+'x'+img.height+'x'+img.channels, '\n' );
Print( img.Load().length , '\n' );
