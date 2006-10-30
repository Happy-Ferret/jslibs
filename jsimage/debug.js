LoadModule('jsstd');
LoadModule('jsimage');
LoadModule('jsnspr');

var f = new File('test.png');
f.Open( File.RDONLY );
var img = new Image(f);
Print( img.width+'x'+img.height+'x'+img.channels, '\n' );
Print( img.Load().length , '\n' );
