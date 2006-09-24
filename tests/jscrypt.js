Exec('deflib.js');
LoadModule('jscrypt');


var cr = new Cipher("blowfish","test_key");

cr.encrypt('123456');