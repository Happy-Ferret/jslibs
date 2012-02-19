	loadModule('jsdebug');
	loadModule('jsstd');

function a() {

function b() {

}

}



throw 0;



if ( 0 ) {

	loadModule('jsdebug');
	loadModule('jsstd');
	
		
	print( PropertiesList([]) );
	
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
	
	print(m-err, '\n');


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
		
		print( mem, '\n' ); // 1.009
*/

	throw 0;
}

// loadModule('jsstd');  loadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { print( id, ':', uneval(args), '\n' ) } };  exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  Halt();

loadModule('jsstd');
loadModule('jsdebug');
//loadModule('jsio');
//loadModule('jssqlite');


print( uneval( PropertiesList({}) ));
//TestDebug();


throw 0;


var s = { a:1 };

var bValue;

Object.defineProperty(s, "b", {get : function(){ return bValue; },  
                               set : function(newValue){ bValue = newValue; },  
                               enumerable : true,  
                               configurable : true});  

print( uneval(PropertiesInfo(s)) );



Halt();


function prop(obj, prev, lvl) {

	if ( lvl > 6 )
		return;
	if ( IsPrimitive(obj) )
		return;

	for each ( name in PropertiesList(obj) ) {
		
		print( (prev+'.'+name).quote(), ':\n' );
		var v;
		try {
			v = obj[name];
		} catch(ex) {
			
			print(ex, '\n');
		}
		prop(v, prev+'.'+name, lvl+1);
	}
}

prop(global, 'global', 0);


Halt();

loadModule('jsio');
loadModule('jscrypt');
loadModule('jssqlite');
loadModule('jsode');
loadModule('jssound');
loadModule('jsz');
loadModule('jsimage');
loadModule('jsgraphics');
loadModule('jstask');
loadModule('jstrimesh');
loadModule('jsiconv');
loadModule('jsfont');


var myTask = new Task(function() { loadModule('jsdebug') } );
myTask.Request();
myTask.Response();

onNewScript = function( filename, lineno, script, fct) {

	print( 'onNewScript ', filename, lineno, script, fct, '\n' );
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

exec('debugger.js');

!function() {
exec('testForDebugger.js');
}();

CollectGarbage();

debugger;



	/*
print('processTime ', processTime, '\n');



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


print( DisassembleScript('debug.js', 3) );

*/