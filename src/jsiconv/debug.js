loadModule('jsstd'); 
loadModule('jsiconv'); 

	var conv = new Iconv('UTF-8', 'ISO-8859-1', true, false); // source is not wide, dest is wide
	var res = conv('été');
	//QA.ASSERT( res.length, 3, 'UC string length' );
	print( res.length, '\n' );


throw 0;


loadModule('jsstd'); exec('../common/tools.js');
//var QA = FakeQAApi;
//runLocalQAFile();
//runJsircbot(false); throw 0;
runQATests('-rep 1 -exclude jstask jsiconv');


// loadModule('jsstd');  loadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { print( id, ':', uneval(args), '\n' ) } };  exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  halt();

loadModule('jsstd');
loadModule('jsio');
loadModule('jsiconv');
loadModule('jswinshell');

	
	var conv = new Iconv(consoleCodepage, Iconv.jsUC, false, true);
	print( consoleCodepage )


	
halt(); //////////////////////////////////////////
	

	var conv = new Iconv('UTF-8', 'UCS-2-INTERNAL', false, true);
	print( conv.invalidChar, '?', 'default invalidChar' );
	ASSERT_EXCEPTION( function() { conv.invalidChar = '???' }, TypeError, 'invalid invalidChar' );
	conv.invalidChar = '.';
	print( conv.invalidChar, '.', 'new invalidChar' );



halt(); //////////////////////////////////////////

	var conv = new Iconv('UCS-2-INTERNAL', 'UTF-8', true, false);
	var result = conv(String.fromCharCode(0x20AC));
	print(result, '\n');


halt(); //////////////////////////////////////////


print( Iconv.version );

var conv = new Iconv('UCS-2-INTERNAL', 'ISO-8859-1', true, false); // source is not wide, dest is wide

var src = [ String.fromCharCode(c) for each ( c in [256, 300, 65000] ) ].join('');

print( src.quote(), '\n' );

var res = conv(src);

print( String.charCodeAt(res[0]) );

