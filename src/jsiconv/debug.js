LoadModule('jsstd');
LoadModule('jsiconv');

var conv = new Iconv('UTF-8', 'ISO-8859-1');
var invConv = new Iconv('ISO-8859-1', 'UTF-8');

var converted = conv('été');


var result = '';

for each ( var c in converted ) {

	result += invConv(c);
}

Print( result ,'\n' );


