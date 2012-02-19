// loadModule('jsstd');  loadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { print( id, ':', uneval(args), '\n' ) } };  exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  Halt();
//loadModule('jsstd'); exec('../common/tools.js'); RunQATests('-rep 4 jscrypt');

loadModule('jsstd');
loadModule('jscrypt');

var cr = new Cipher("ECB", "cast5", "my");

Halt();


var md5 = new Hash('md5');
print( HexEncode(md5('qwe')), '\n' );

var md5 = new Hash('md5');
print( HexEncode(md5('qw')), '\n' );
md5.Process('e');
print( HexEncode(md5.Done()), '\n' );

Halt();


var rnd = new Prng('yarrow');
rnd(1);


try {

		var acName = 'katja';
		var acKeySize = 1024;

		var rnd = new Prng('yarrow');
//		rnd.AutoEntropy(128); // give more entropy
		
		var ac = new AsymmetricCipher( acName, 'sha512', rnd );
		ac.CreateKeys( acKeySize, true );
		var acPubKey = ac.publicKey;
		print( ac.keySize, '\n' );
		
		var ac2 = new AsymmetricCipher( acName, 'sha512', rnd );

		ac2.publicKey = acPubKey;

		print( ac2.keySize, '\n' );

		ac2.blockLength;
		var plainText = StringRepeat(' ', ac2.blockLength );
		print( 'len ', plainText.length, '\n' );
		
		var enc = ac2.Encrypt( plainText );
//		var res = ac.Decrypt( enc );
//		print ( res == plainText, '\n' );
		
//		print('Rise an error:\n');
//		var res = ac2.Decrypt( ac.Encrypt( plainText ) );


} catch(ex) {
	
	print( ex.const, '\n' );
	print( ex.lineNumber, '\n' );
}

	
Halt();



		var rnd = new Prng('fortuna');
		rnd.AutoEntropy(128); // give more entropy
		var plainText = 'totqutyvq8wyetvq7ewryt9vq78yrt987wveyrt98v7weyr9tv87wery9twev87y9r78o';
		plainText = plainText;
		var rsa = new AsymmetricCipher('rsa', 'md5', rnd );
		rsa.CreateKeys( 1024 );
		var rsaPubKey = rsa.publicKey;
		var rsa1 = new AsymmetricCipher( 'rsa', 'md5', rnd );
		rsa1.publicKey = rsaPubKey;
		var rsaEncryptedData = rsa1.Encrypt( plainText );
		var res = rsa.Decrypt( rsaEncryptedData );
		print ( res == plainText )

		
		
Halt();



		var fortuna = new Prng('fortuna');
		fortuna.AutoEntropy(123); // give more entropy
		//Alice
		var alice = new AsymmetricCipher('dsa', 'sha1', fortuna);
		alice.CreateKeys(64);

print( alice.keySize, '\n' );

		var publicKey = alice.publicKey;
		//Bob
		var bob = new AsymmetricCipher('dsa', 'sha1', fortuna);
		bob.publicKey = publicKey;

print( bob.keySize, '\n' );
		
		
Halt();	



		var fortuna = new Prng('fortuna');
		fortuna.AutoEntropy(123); // give more entropy

		var alice = new AsymmetricCipher('dsa', 'sha1', fortuna);
		alice.CreateKeys(64);
		print( alice.keySize, '\n' );
		
		var publicKey = alice.publicKey;

		var bob = new AsymmetricCipher('dsa', 'sha1', fortuna);
		bob.publicKey = publicKey;

//		print( bob.keySize, '\n' );


Halt();

var h1 = new Hash("md5")("some data");

var h2 = new Hash("md5");
h2.Process("some data");
h2 = h2.Done();

print( h1 == h2 );


Halt();

var md5 = new Hash('md5');
md5.Process('foobarxxx');
print(HexEncode(md5.Done(), '\n'));
print( '\n' );


print( Cipher.list.toSource() );
print( '\n' );
print( Hash.list.toSource() );
print( '\n' );
print( Prng.list.toSource() );
print( '\n' );


var IV = "123";
var cr = new Cipher('CFB', "cast5", "my  key of  16B ", IV );
var encryptedText = cr.Encrypt('my  dataof  16B ');
var cr = new Cipher('CFB', "cast5", "my  key of  16B ", IV );
print( cr.Decrypt(encryptedText) );
print( '\n' );

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
print('blockLength = '+bob.blockLength, '\n' );
print('keySize = '+bob.keySize, '\n' );


var encryptedData = bob.Encrypt(createString(bob.blockLength));
print('OK!\n');
var encryptedData = bob.Encrypt(createString(bob.blockLength)+'z');


//Alice
print( alice.Decrypt(encryptedData), '\n' );



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
	

	print( plainText == rsa.DecryptKey( 'md5', rsaEncryptedData ) , '\n' );

	var hkey = new Hash("sha256")('this is a secret key');
	print( HexEncode(hkey), '\n');
	
	
	var key = hkey;
	var IV = r(Crypt.BlockLength('blowfish'));

// encrypt:
	var crypt = new Crypt( 'ctr', 'blowfish', key, IV );
	var plainText = 'secret string';
	var cipherData = crypt.Encrypt(plainText);




// decrypt:
	crypt.IV = IV;
	var decipheredData = crypt.Decrypt( cipherData );
	print( 'decrypted data: '+decipheredData, '\n' );
	
	
} catch( ex if ex instanceof CryptError ) {
	print( ex.text );
} catch( ex ) {
	throw(ex);
}
