LoadModule('jsstd');



var str = new BString('123');
var b = new Buffer();
b.Write(str);
str.Add('456');
b.Write('7');


Print( b.Skip(4) );
	