// LoadModule('jsstd');  LoadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { Print( id, ':', uneval(args), '\n' ) } };  Exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  Halt();

LoadModule('jsstd');
LoadModule('jsiconv');

Print( Iconv.version );

var conv = new Iconv('UCS-2-INTERNAL', 'ISO-8859-1', true, false); // source is not wide, dest is wide

var src = [ String.fromCharCode(c) for each ( c in [256, 300, 65000] ) ].join('');

Print( src.quote(), '\n' );

var res = conv(src);

Print( String.charCodeAt(res[0]) );

