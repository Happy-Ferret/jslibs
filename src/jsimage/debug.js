// LoadModule('jsstd');  LoadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { Print( id, ':', uneval(args), '\n' ) } };  Exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  Halt();

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

//var texture = DecodeJpegImage(new File('battleship_1280x1024.jpg').Open( File.RDONLY ));
//Print( texture.width+'x'+texture.height+'x'+texture.channels, '\n' );

var texture = DecodePngImage(new File('Tremulous.png').Open( File.RDONLY ));
Print( texture.width+'x'+texture.height+'x'+texture.channels, '\n' );

