LoadModule('jsstd');
LoadModule('jscrypt');

try {


var cr = new Cipher('CFB', "blowfish", "test_key" );


var encryptedText = cr.Encrypt('123456');

Print( cr.Decrypt(encryptedText) );


} catch(ex if ex instanceof CryptError) {

	
	Print( ex.code+' - '+ex.text );
}