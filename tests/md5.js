LoadModule('jsstd');
LoadModule('jscrypt');

var str = arguments[1] || 'The quick brown fox jumps over the lazy dog';

var md5 = new Hash('md5');
Print( 'md5("'+str+'") = '+HexEncode(md5(str)) );
