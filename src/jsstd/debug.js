LoadModule('jsstd');

var buf = new Buffer('xxx');
buf.Write('aaa');
buf.Write('bb1');
buf.Write('14ccc');
var buf2 = new Buffer(buf);
buf2.Write('buffer2');

Print( String(buf2) );

