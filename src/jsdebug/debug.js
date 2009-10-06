// LoadModule('jsstd');  LoadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { Print( id, ':', uneval(args), '\n' ) } };  Exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  Halt();

LoadModule('jsstd');
LoadModule('jsdebug');

Exec('debugger.js');

!function() {
Exec('testForDebugger.js');
}();

CollectGarbage();

debugger;



	/*
Print('processTime ', processTime, '\n');



eval('function toto() {\
var test = -1; \
}');


var MyTest;
!function() {
var x = 2;
MyTest = function (arg) {
	
	try {
	throw 123;
	} catch(ex){}
	return arg+x;
}
	
}()


Print( DisassembleScript('debug.js', 3) );

*/