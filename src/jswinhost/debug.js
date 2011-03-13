// LoadModule('jsstd');  LoadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { Print( id, ':', uneval(args), '\n' ) } };  Exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  Halt();

throw 'error test';

while ( !endSignal )
	ProcessEvents( EndSignalEvents() );

//LoadModule('jsshell');
//MessageBox(isfirstInstance);


//var cons = new Console();
//stdout = cons.Write;
//Print('123');
