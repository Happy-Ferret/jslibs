// LoadModule('jsstd');  LoadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { Print( id, ':', uneval(args), '\n' ) } };  Exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  Halt();

LoadModule('jsstd');
LoadModule('jsio');

LoadModule('jsfont');
LoadModule('jsoglft');

var f = new Font('c:\\windows\\fonts\\arial.ttf');

DrawText(f, "test");
