var loadModule = host.loadModule;
// loadModule('jsstd');  loadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { print( id, ':', uneval(args), '\n' ) } };  exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  halt();

loadModule('jsstd'); exec('../common/tools.js'); runQATests('jsimage'); throw 0;

loadModule('jsstd');
loadModule('jsimage');
loadModule('jsio');


	var image = { width:0, height:0, channels:1, data:'' };

	encodeJpegImage(image, 100);



throw 0;

	loadModule('jsio');
	var image = decodePngImage(new File('.' + '/z09n2c08.png').open(File.RDONLY));

	var jpeg = encodeJpegImage(image, 50);
//	var jpeg = encodeJpegImage(image, 100);

	print( jpeg.byteLength, ' ', 1154,  '\n' );

	new File('test.jpeg').content = jpeg;


print( 'end' );


/*
var f = new File('pngtest.png');
f.open( File.RDONLY );
var img = new Png(f);
print( img.width+'x'+img.height+'x'+img.channels, '\n' );
print( img.load().length , '\n' );
*/

/*
var texture = decodeJpegImage(new File('Patern_test.jpg').open( File.RDONLY ));
print( texture.width+'x'+texture.height+'x'+texture.channels, '\n' );

var texture = decodePngImage(new File('battlesuitbj6.png').open());
print( texture.width+'x'+texture.height+'x'+texture.channels, '\n' );
*/
