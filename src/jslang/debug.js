LoadModule('jslang');
LoadModule('jsstd');

Print( BString, '\n' );

var b = new BString('123');

b[3] = '4';

var stream = Stream(b);

Print( b.length, '\n' );
Print( stream.Read(10), '\n' );

