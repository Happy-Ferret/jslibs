LoadModule('jslang');
LoadModule('jsstd');

Print( BString );

var b = new BString();

b.Add('abc');
b.Add(b);

Print(b);