// he l
LoadModule('jsstd');
LoadModule('jsio');


function Cap(s) {
	
	return s[0].toUpperCase() + s.substr(1);
}


function CreateWorld() {

	var w = global.arguments[0].substr(-8,5);
	return Cap(w);
}


var regexp = /\/\/ (.*)/;
var file = new File( arguments[0] );
var result = regexp( file.content );

var str = result[1];

str = str.split('');
str[0] = str[0].toUpperCase();
str[2] = 'l';
str = str.join('');

str += String.fromCharCode(111) + ' ';

str += CreateWorld();


Print(str,'!\n');

//Print('Press Enter key to continue...');
//File.stdin.Read();
