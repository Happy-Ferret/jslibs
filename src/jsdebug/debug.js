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
	
		
	print( propertiesList([]) );
	
	halt();

	var m = privateMemoryUsage;

	!function() {
		stringRepeat('a', 0);
	}();

	var err = privateMemoryUsage - m
	
	m = privateMemoryUsage;

	!function() {
		stringRepeat('a', 3000000);
	}();
	m = privateMemoryUsage - m;
	
	print(m-err, '\n');


/*
		var length = 1024*1024;
		var times = 3;

		function genMem() {

			collectGarbage();
			var data = [];
		
			for ( var i = 0; i < times; ++i ) {
			
				data.push( Blob(stringRepeat('a', length)) );
				collectGarbage();
			}
			Blob(stringRepeat('a', length));
			collectGarbage();
			return data;
		}
		
		var mem = privateMemoryUsage;
		genMem();
		
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


print( uneval( propertiesList({}) ));
//testDebug();


throw 0;


var s = { a:1 };

var bValue;

Object.defineProperty(s, "b", {get : function(){ return bValue; },  
                               set : function(newValue){ bValue = newValue; },  
                               enumerable : true,  
                               configurable : true});  

print( uneval(propertiesInfo(s)) );



halt();


function prop(obj, prev, lvl) {

	if ( lvl > 6 )
		return;
	if ( isPrimitive(obj) )
		return;

	for each ( name in propertiesList(obj) ) {
		
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


halt();

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
myTask.request();
myTask.response();

onNewScript = function( filename, lineno, script, fct) {

	print( 'onNewScript ', filename, lineno, script, fct, '\n' );
}


var list = [];

eval('function test() {	list[1]; } ');

collectGarbage();


propertiesInfo(list);


halt();

for each ( var item in propertiesList(global) )
	String( item );

for each ( var item in propertiesInfo(global) )
	String( propertiesInfo(item.object) );




halt();

exec('debugger.js');

!function() {
exec('testForDebugger.js');
}();

collectGarbage();

debugger;



	/*
print('processTime ', processTime, '\n');



eval('function toto() {\
var test = -1; \
}');


var myTest;
!function() {
var x = 2;
myTest = function (arg) {
	
	try {
	throw 123;
	} catch(ex){}
	return arg+x;
}
	
}()


print( disassembleScript('debug.js', 3) );

*/