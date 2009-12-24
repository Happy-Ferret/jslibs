// LoadModule('jsstd');  LoadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { Print( id, ':', uneval(args), '\n' ) } };  Exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  Halt();

LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jsiconv');
LoadModule('jswinshell');

	var conv = new Iconv('UTF-8', 'UCS-2-INTERNAL', false, true);
	Print( conv.invalidChar, '?', 'default invalidChar' );
	ASSERT_EXCEPTION( function() { conv.invalidChar = '???' }, TypeError, 'invalid invalidChar' );
	conv.invalidChar = '.';
	Print( conv.invalidChar, '.', 'new invalidChar' );



Halt(); //////////////////////////////////////////

	var conv = new Iconv('UCS-2-INTERNAL', 'UTF-8', true, false);
	var result = conv(String.fromCharCode(0x20AC));
	Print(result, '\n');


Halt(); //////////////////////////////////////////


Print( Iconv.version );

var conv = new Iconv('UCS-2-INTERNAL', 'ISO-8859-1', true, false); // source is not wide, dest is wide

var src = [ String.fromCharCode(c) for each ( c in [256, 300, 65000] ) ].join('');

Print( src.quote(), '\n' );

var res = conv(src);

Print( String.charCodeAt(res[0]) );

