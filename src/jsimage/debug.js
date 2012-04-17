var loadModule = host.loadModule;
// loadModule('jsstd');  loadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { print( id, ':', uneval(args), '\n' ) } };  exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  halt();

loadModule('jsstd'); exec('../common/tools.js'); runQATests('jsimage'); throw 0;

loadModule('jsstd');
loadModule('jsimage');
loadModule('jsio');

/*
var f = new File('pngtest.png');
f.open( File.RDONLY );
var img = new Png(f);
print( img.width+'x'+img.height+'x'+img.channels, '\n' );
print( img.load().length , '\n' );
*/

var texture = decodeJpegImage(new File('Patern_test.jpg').open( File.RDONLY ));
print( texture.width+'x'+texture.height+'x'+texture.channels, '\n' );

var texture = decodePngImage(new File('battlesuitbj6.png').open());
print( texture.width+'x'+texture.height+'x'+texture.channels, '\n' );

