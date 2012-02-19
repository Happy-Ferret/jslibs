// loadModule('jsstd');  loadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { print( id, ':', uneval(args), '\n' ) } };  exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  Halt();

loadModule('jsstd');
loadModule('jsimage');
loadModule('jsio');

/*
var f = new File('pngtest.png');
f.Open( File.RDONLY );
var img = new Png(f);
print( img.width+'x'+img.height+'x'+img.channels, '\n' );
print( img.Load().length , '\n' );
*/

//var texture = DecodeJpegImage(new File('battleship_1280x1024.jpg').Open( File.RDONLY ));
//print( texture.width+'x'+texture.height+'x'+texture.channels, '\n' );

var texture = DecodePngImage(new File('battlesuitbj6.png').Open());
print( texture.width+'x'+texture.height+'x'+texture.channels, '\n' );

