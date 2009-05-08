LoadModule('jsstd');
LoadModule('jscrypt');

		var fortuna = new Prng('fortuna');
		fortuna.AutoEntropy(123); // give more entropy

		var alice = new AsymmetricCipher('dsa', 'sha1', fortuna);
		alice.CreateKeys(64);
		Print( alice.keySize, '\n' );
		
		var publicKey = alice.publicKey;

		var bob = new AsymmetricCipher('dsa', 'sha1', fortuna);
		bob.publicKey = publicKey;

//		Print( bob.keySize, '\n' );


Halt();

var h1 = new Hash("md5")("some data");

var h2 = new Hash("md5");
h2.Process("some data");
h2 = h2.Done();

Print( h1 == h2 );


Halt();

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

function createString(count) {

	var s = '';
	while (s.length < count)
		s += 'x';
	return s;
}


var fortuna = new Prng('fortuna');
fortuna.AutoEntropy(123); // give more entropy

//Alice
var alice = new AsymmetricCipher('dsa', 'sha1', fortuna);
alice.CreateKeys(128);

var publicKey = alice.publicKey;

//Bob
var bob = new AsymmetricCipher('dsa', 'sha1', fortuna);
bob.publicKey = publicKey;
Print('blockLength = '+bob.blockLength, '\n' );
Print('keySize = '+bob.keySize, '\n' );


var encryptedData = bob.Encrypt(createString(bob.blockLength));
Print('OK!\n');
var encryptedData = bob.Encrypt(createString(bob.blockLength)+'z');


//Alice
Print( alice.Decrypt(encryptedData), '\n' );



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
