LoadModule('jsstd');
LoadModule('jsimage');
LoadModule('jsnspr');

/*
var f = new File('pngtest.png');
f.Open( File.RDONLY );
var img = new Png(f);
Print( img.width+'x'+img.height+'x'+img.channels, '\n' );
Print( img.Load().length , '\n' );
*/

var f = new File('testimg.jpg');
f.Open( File.RDONLY );
var img = new Jpeg(f);

Print( img.width+'x'+img.height+'x'+img.channels, '\n' );
Print( img.Load().length , '\n' );

