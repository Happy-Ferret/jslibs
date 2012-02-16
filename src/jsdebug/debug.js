	LoadModule('jsdebug');
	LoadModule('jsstd');

function a() {

function b() {

}

}



throw 0;



if ( 0 ) {

	LoadModule('jsdebug');
	LoadModule('jsstd');
	
		
	Print( PropertiesList([]) );
	
	Halt();

	var m = privateMemoryUsage;

	!function() {
		StringRepeat('a', 0);
	}();

	var err = privateMemoryUsage - m
	
	m = privateMemoryUsage;

	!function() {
		StringRepeat('a', 3000000);
	}();
	m = privateMemoryUsage - m;
	
	Print(m-err, '\n');


/*
		var length = 1024*1024;
		var times = 3;

		function GenMem() {

			CollectGarbage();
			var data = [];
		
			for ( var i = 0; i < times; ++i ) {
			
				data.push( Blob(StringRepeat('a', length)) );
				CollectGarbage();
			}
			Blob(StringRepeat('a', length));
			CollectGarbage();
			return data;
		}
		
		var mem = privateMemoryUsage;
		GenMem();
		
		mem = (privateMemoryUsage-mem) / (length*times);
		
		Print( mem, '\n' ); // 1.009
*/

	throw 0;
}

// LoadModule('jsstd');  LoadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { Print( id, ':', uneval(args), '\n' ) } };  Exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  Halt();

LoadModule('jsstd');
LoadModule('jsdebug');
//LoadModule('jsio');
//LoadModule('jssqlite');


Print( uneval( PropertiesList({}) ));
//TestDebug();


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