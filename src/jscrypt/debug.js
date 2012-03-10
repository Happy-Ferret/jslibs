// loadModule('jsstd');  loadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { print( id, ':', uneval(args), '\n' ) } };  exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  halt();

loadModule('jsstd'); exec('../common/tools.js'); runQATests('jscrypt -exclude jstask'); throw 0;


loadModule('jsstd');
loadModule('jscrypt');

var cr = new Cipher("ECB", "cast5", "my");

halt();


var md5 = new Hash('md5');
print( hexEncode(md5('qwe')), '\n' );

var md5 = new Hash('md5');
print( hexEncode(md5('qw')), '\n' );
md5.process('e');
print( hexEncode(md5.done()), '\n' );

halt();


var rnd = new Prng('yarrow');
rnd(1);


try {

		var acName = 'katja';
		var acKeySize = 1024;

		var rnd = new Prng('yarrow');
//		rnd.AutoEntropy(128); // give more entropy
		
		var ac = new AsymmetricCipher( acName, 'sha512', rnd );
		ac.createKeys( acKeySize, true );
		var acPubKey = ac.publicKey;
		print( ac.keySize, '\n' );
		
		var ac2 = new AsymmetricCipher( acName, 'sha512', rnd );

		ac2.publicKey = acPubKey;

		print( ac2.keySize, '\n' );

		ac2.blockLength;
		var plainText = stringRepeat(' ', ac2.blockLength );
		print( 'len ', plainText.length, '\n' );
		
		var enc = ac2.encrypt( plainText );
//		var res = ac.Decrypt( enc );
//		print ( res == plainText, '\n' );
		
//		print('Rise an error:\n');
//		var res = ac2.Decrypt( ac.Encrypt( plainText ) );


} catch(ex) {
	
	print( ex.const, '\n' );
	print( ex.lineNumber, '\n' );
}

	
halt();



		var rnd = new Prng('fortuna');
		rnd.autoEntropy(128); // give more entropy
		var plainText = 'totqutyvq8wyetvq7ewryt9vq78yrt987wveyrt98v7weyr9tv87wery9twev87y9r78o';
		plainText = plainText;
		var rsa = new AsymmetricCipher('rsa', 'md5', rnd );
		rsa.createKeys( 1024 );
		var rsaPubKey = rsa.publicKey;
		var rsa1 = new AsymmetricCipher( 'rsa', 'md5', rnd );
		rsa1.publicKey = rsaPubKey;
		var rsaEncryptedData = rsa1.encrypt( plainText );
		var res = rsa.decrypt( rsaEncryptedData );
		print ( res == plainText )

		
		
halt();



		var fortuna = new Prng('fortuna');
		fortuna.autoEntropy(123); // give more entropy
		//Alice
		var alice = new AsymmetricCipher('dsa', 'sha1', fortuna);
		alice.createKeys(64);

print( alice.keySize, '\n' );

		var publicKey = alice.publicKey;
		//Bob
		var bob = new AsymmetricCipher('dsa', 'sha1', fortuna);
		bob.publicKey = publicKey;

print( bob.keySize, '\n' );
		
		
halt();	



		var fortuna = new Prng('fortuna');
		fortuna.autoEntropy(123); // give more entropy

		var alice = new AsymmetricCipher('dsa', 'sha1', fortuna);
		alice.createKeys(64);
		print( alice.keySize, '\n' );
		
		var publicKey = alice.publicKey;

		var bob = new AsymmetricCipher('dsa', 'sha1', fortuna);
		bob.publicKey = publicKey;

//		print( bob.keySize, '\n' );


halt();

var h1 = new Hash("md5")("some data");

var h2 = new Hash("md5");
h2.process("some data");
h2 = h2.done();

print( h1 == h2 );


halt();

var md5 = new Hash('md5');
md5.process('foobarxxx');
print(hexEncode(md5.done(), '\n'));
print( '\n' );


print( Cipher.list.toSource() );
print( '\n' );
print( Hash.list.toSource() );
print( '\n' );
print( Prng.list.toSource() );
print( '\n' );


var IV = "123";
var cr = new Cipher('CFB', "cast5", "my  key of  16B ", IV );
var encryptedText = cr.encrypt('my  dataof  16B ');
var cr = new Cipher('CFB', "cast5", "my  key of  16B ", IV );
print( cr.decrypt(encryptedText) );
print( '\n' );

function createString(count) {

	var s = '';
	while (s.length < count)
		s += 'x';
	return s;
}


var fortuna = new Prng('fortuna');
fortuna.autoEntropy(123); // give more entropy

//Alice
var alice = new AsymmetricCipher('dsa', 'sha1', fortuna);
alice.createKeys(128);

var publicKey = alice.publicKey;

//Bob
var bob = new AsymmetricCipher('dsa', 'sha1', fortuna);
bob.publicKey = publicKey;
print('blockLength = '+bob.blockLength, '\n' );
print('keySize = '+bob.keySize, '\n' );


var encryptedData = bob.encrypt(createString(bob.blockLength));
print('OK!\n');
var encryptedData = bob.encrypt(createString(bob.blockLength)+'z');


//Alice
print( alice.decrypt(encryptedData), '\n' );



halt();



try {

	var r = new Prng('fortuna');
	r.autoEntropy(128); // give more entropy

	
	var plainText = 'totqutyvq8wyetvq7ewryt9vq78yrt987wveyrt98v7weyr9tv87wery9twev87y9r78o';
	plainText = plainText;

	var rsa = new Rsa();
	rsa.createKeys( r, 1024 );
	
	var rsaPubKey = rsa.publicKey;
	
	var rsa1 = new Rsa();
	rsa1.publicKey = rsaPubKey;
	var rsaEncryptedData = rsa1.encryptKey( r ,'md5', plainText );
	

	print( plainText == rsa.decryptKey( 'md5', rsaEncryptedData ) , '\n' );

	var hkey = new Hash("sha256")('this is a secret key');
	print( hexEncode(hkey), '\n');
	
	
	var key = hkey;
	var IV = r(Crypt.blockLength('blowfish'));

// encrypt:
	var crypt = new Crypt( 'ctr', 'blowfish', key, IV );
	var plainText = 'secret string';
	var cipherData = crypt.encrypt(plainText);




// decrypt:
	crypt.IV = IV;
	var decipheredData = crypt.decrypt( cipherData );
	print( 'decrypted data: '+decipheredData, '\n' );
	
	
} catch( ex if ex instanceof CryptError ) {
	print( ex.text );
} catch( ex ) {
	throw(ex);
}
