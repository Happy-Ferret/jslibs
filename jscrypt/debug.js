LoadModule('jsstd');
LoadModule('jscrypt');


var md5 = new Hash('md5');
md5.Process('foobarxxx');
Print(HexEncode(md5.Done(), '\n'));
Print( '\n' );


Print( Cipher.list.toSource() );
Print( '\n' );
Print( Hash.list.toSource() );
Print( '\n' );
Print( Prng.list.toSource() );
Print( '\n' );


var IV = "123";
var cr = new Cipher('CFB', "cast5", "my  key of  16B ", IV );
var encryptedText = cr.Encrypt('my  dataof  16B ');
var cr = new Cipher('CFB', "cast5", "my  key of  16B ", IV );
Print( cr.Decrypt(encryptedText) );
Print( '\n' );


Halt();



try {

	var r = new Prng('fortuna');
	r.AutoEntropy(128); // give more entropy

	
	var plainText = 'totqutyvq8wyetvq7ewryt9vq78yrt987wveyrt98v7weyr9tv87wery9twev87y9r78o';
	plainText = plainText;

	var rsa = new Rsa();
	rsa.CreateKeys( r, 1024 );
	
	var rsaPubKey = rsa.publicKey;
	
	var rsa1 = new Rsa();
	rsa1.publicKey = rsaPubKey;
	var rsaEncryptedData = rsa1.EncryptKey( r ,'md5', plainText );
	

	Print( plainText == rsa.DecryptKey( 'md5', rsaEncryptedData ) , '\n' );

	var hkey = new Hash("sha256")('this is a secret key');
	Print( HexEncode(hkey), '\n');
	
	
	var key = hkey;
	var IV = r(Crypt.BlockLength('blowfish'));

// encrypt:
	var crypt = new Crypt( 'ctr', 'blowfish', key, IV );
	var plainText = 'secret string';
	var cipherData = crypt.Encrypt(plainText);




// decrypt:
	crypt.IV = IV;
	var decipheredData = crypt.Decrypt( cipherData );
	Print( 'decrypted data: '+decipheredData, '\n' );
	
	
} catch( ex if ex instanceof CryptError ) {
	Print( ex.text );
} catch( ex ) {
	throw(ex);
}
