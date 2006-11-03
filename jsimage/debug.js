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


var texture = new Jpeg(new File('shiphull.jpg').Open( File.RDONLY )).Load().Trim([10,10,20,20]);

Print( texture.width+'x'+texture.height+'x'+texture.channels, '\n' );

