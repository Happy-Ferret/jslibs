LoadModule('jsstd');
Exec('debugger.js');

/*
LoadModule('jsdebug');
var o = { x:5 }
Print( o.hasOwnProperty('x') ); // true
Print( o.hasOwnProperty('__parent__') ); // true
Print( o.hasOwnProperty('constructor') ); // false

// PropertiesList uses JS_PropertyIterator
Print(uneval( PropertiesList(o.__proto__) )); // ['x']

Halt();
*/

Exec('debug1.js');

Debug1();

Print('Done.\n');
