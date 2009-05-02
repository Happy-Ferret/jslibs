LoadModule('jsstd');
LoadModule('jsdebug');

gcZeal = 2;  

var o = Test(function(a1, a2, a3) {
	
	
	Print(a1);
	return 1;
});

Print(o);

Halt();



/*
LoadModule('jsdebug');
var o = { x:5 }
Print(o.hasOwnProperty('x')); // true
Print(o.hasOwnProperty('__parent__')); // true
Print(o.hasOwnProperty('constructor')); // false
Print('\n');
// PropertiesInfo (using JS_GetPropertyDescArray or JS_PropertyIterator)
Print( [ p+'='+v.enumerate for each ( [p,v] in Iterator(PropertiesInfo(o)) ) ].join('\n') ); // ['x']
Print('\n');
Halt();
*/


Exec('debugger.js');
Exec('debug1.js');
Debug1();
Print('Done.\n');