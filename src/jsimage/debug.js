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


var texture = new Jpeg(new File('R0010235.JPG').Open( File.RDONLY )).Load().Trim([10,10,20,20]);
Print( texture.width+'x'+texture.height+'x'+texture.channels, '\n' );


var texture = new Png(new File('calendar2.png').Open( File.RDONLY )).Load();
Print( texture.width+'x'+texture.height+'x'+texture.channels, '\n' );
