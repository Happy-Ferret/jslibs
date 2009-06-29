LoadModule('jsstd');
LoadModule('jsimage');
LoadModule('jsio');

/*
var f = new File('pngtest.png');
f.Open( File.RDONLY );
var img = new Png(f);
Print( img.width+'x'+img.height+'x'+img.channels, '\n' );
Print( img.Load().length , '\n' );
*/


var texture = DecodePngImage(new File('Tremulous.png').Open( File.RDONLY ));
Print( texture.width+'x'+texture.height+'x'+texture.channels, '\n' );

