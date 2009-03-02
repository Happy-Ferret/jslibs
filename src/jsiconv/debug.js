LoadModule('jsstd');
LoadModule('jsiconv');


Print( new Iconv('UTF-8', 'ISO-8859-1')('é').length );

/*
for ( var i = 0; i < 255; i++ ) {

	Print( conv(String.fromCharCode(i)).length ,'   ' );
}
*/


Halt();

var conv = new Iconv('UTF-8', 'ISO-8859-1');
var invConv = new Iconv('ISO-8859-1', 'UTF-8');
var converted = conv('été');
var result = invConv(converted);
Print( result == 'été','\n' );


var conv = new Iconv('UTF-8', 'ISO-8859-1');
var invConv = new Iconv('ISO-8859-1', 'UTF-8');
var converted = conv('été');
var result = '';
for each ( var c in converted )
	result += invConv(c);
Print( result == 'été','\n' );



/*
var utf8 = (new Iconv('UTF-8', 'ISO-8859-1'))('été').split('');
utf8.splice(1,0,'x'); // insert an invalid multybyte sequense
utf8 = utf8.join('');
Print( (new Iconv('ISO-8859-1', 'UTF-8'))(utf8), '\n' )
*/

//var conv = new Iconv('UCS-2-INTERNAL', 'ISO-8859-1', true, true);
//Print( conv('été') );
