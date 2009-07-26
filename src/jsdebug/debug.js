LoadModule('jsstd');
LoadModule('jsdebug');
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
