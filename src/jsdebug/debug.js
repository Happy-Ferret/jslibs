// LoadModule('jsstd');  LoadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { Print( id, ':', uneval(args), '\n' ) } };  Exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  Halt();

LoadModule('jsstd');
LoadModule('jsdebug');
//LoadModule('jsio');
//LoadModule('jssqlite');

throw 0;


var s = { a:1 };

var bValue;

Object.defineProperty(s, "b", {get : function(){ return bValue; },  
                               set : function(newValue){ bValue = newValue; },  
                               enumerable : true,  
                               configurable : true});  

Print( uneval(PropertiesInfo(s)) );



Halt();


function prop(obj, prev, lvl) {

	if ( lvl > 6 )
		return;
	if ( IsPrimitive(obj) )
		return;

	for each ( name in PropertiesList(obj) ) {
		
		Print( (prev+'.'+name).quote(), ':\n' );
		var v;
		try {
			v = obj[name];
		} catch(ex) {
			
			Print(ex, '\n');
		}
		prop(v, prev+'.'+name, lvl+1);
	}
}

prop(global, 'global', 0);


Halt();

LoadModule('jsio');
LoadModule('jscrypt');
LoadModule('jssqlite');
LoadModule('jsode');
LoadModule('jssound');
LoadModule('jsz');
LoadModule('jsimage');
LoadModule('jsgraphics');
LoadModule('jstask');
LoadModule('jstrimesh');
LoadModule('jsiconv');
LoadModule('jsfont');


var myTask = new Task(function() { LoadModule('jsdebug') } );
myTask.Request();
myTask.Response();

onNewScript = function( filename, lineno, script, fct) {

	Print( 'onNewScript ', filename, lineno, script, fct, '\n' );
}


var list = [];

eval('function Test() {	list[1]; } ');

CollectGarbage();


PropertiesInfo(list);


Halt();

for each ( var item in PropertiesList(global) )
	String( item );

for each ( var item in PropertiesInfo(global) )
	String( PropertiesInfo(item.object) );




Halt();

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