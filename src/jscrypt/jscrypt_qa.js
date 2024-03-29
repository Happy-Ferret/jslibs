	loadModule('jscrypt');


/// CryptError.text [p]
	
	String( CryptError.text );
	QA.ASSERT_STR( String( CryptError.prototype.text ), 'undefined' );


/// MD5 digest [p]

	var md5 = new Hash('md5');
	md5.write('foobarxxx');
	QA.ASSERT_STR( md5.done(), "\b\t\xAA\xC0\x10\xB1G\xA8\xC71'}*\x80\x07\xBF", 'digest integrity' );


/// MD5 digest + undefined [p]

	var md5 = new Hash('md5');
	md5.write('foobarxxx');
	md5.write(undefined);
	QA.ASSERT_STR( md5.done(), "\b\t\xAA\xC0\x10\xB1G\xA8\xC71'}*\x80\x07\xBF", 'digest integrity' );


/// Cipher, Hash, Prng list [p]
	
	function count(o) {

		var n = 0;
		for (var p in o) {

			n += Object.prototype.hasOwnProperty.call(o, p);
		}
		return n;
	}


	QA.ASSERT( count(Cipher.list), 22, 'length of Cipher.list' );
	QA.ASSERT( Cipher.list.toSource().length, 1686, 'length of Cipher.list.toString' );

	QA.ASSERT( count(Hash.list), 15, 'length of Cipher.list' );
	QA.ASSERT( Hash.list.toSource().length, 535, 'length of Hash.list.toString' );

	QA.ASSERT( count(Prng.list), 5, 'length of Cipher.list' );
	QA.ASSERT( Prng.list.toSource().length, 51, 'length of Prng.list.toString' );


/// DSA asymmetric crypt and decrypt []
		
	var fortuna = new Prng('fortuna');
	fortuna.autoEntropy(123); // give more entropy
	//Alice
	var alice = new AsymmetricCipher('dsa', 'sha1', fortuna);
	alice.createKeys(64);
	QA.ASSERT( alice.keySize, 64, 'Asymmetric Cipher key size (alice)' );
	var publicKey = alice.publicKey;
	//Bob
	var bob = new AsymmetricCipher('dsa', 'sha1', fortuna);
	bob.publicKey = publicKey;
	QA.ASSERT( bob.blockLength, 20, 'Asymmetric Cipher block length' );
	QA.ASSERT( bob.keySize, 64, 'Asymmetric Cipher key size (bob)' );
	var secretMessage = QA.randomString(bob.blockLength);
	var encryptedData = bob.encrypt( secretMessage );
	//Alice
	QA.ASSERT_STR( alice.decrypt(encryptedData), secretMessage, 'data integrity' );


/// RSA asymmetric crypt and decrypt []

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

	QA.ASSERT_STR( plainText, rsa.decrypt( rsaEncryptedData ), 'data integrity' );

	rsa.wipe();


/// QA.RandomString []

	var data = QA.randomString(300000);
	QA.ASSERT(data.length, 300000, 'random data length');


/// Cipher 1 [p]
		
	var data = QA.randomString(30000);
	var IV = QA.randomString(100);
	var cr = new Cipher('CFB', "cast5", "my  key of  16B ", IV );
	var encryptedText = cr.encrypt(data);
	var cr = new Cipher('CFB', "cast5", "my  key of  16B ", IV );

	QA.ASSERT_STR( cr.decrypt(encryptedText), data, 'crypy/decript with cast5 cipher using CFB mode' );

	cr.wipe();


/// Cipher 2 [p]

	var hkey = new Hash("sha256")('this is a secret key');
	QA.ASSERTOP(hexEncode(hkey), '==', '4B6D4A517F0BB6AA58A412C5B0F68240046B68A6F4703CCB7D7D04320F0D459C');

	var key = hkey;
	var IV = QA.randomString(Cipher.list['blowfish'].blockLength);

	// encrypt
	var crypt = new Cipher( 'ctr', 'blowfish', key, IV );
	var plainText = 'secret string';
	var cipherData = crypt.encrypt(plainText);

	// decrypt
	crypt.IV = IV;
	var decipheredData = crypt.decrypt( cipherData );
	QA.ASSERT_STR( decipheredData, plainText );


/// Key size []

	var fortuna = new Prng('fortuna');
	fortuna.autoEntropy(123); // give more entropy

	var ecc = new AsymmetricCipher('ecc', 'md5', fortuna );
	ecc.createKeys( AsymmetricCipher.ECC_MAX_KEYSIZE );
	QA.ASSERT( ecc.keySize, AsymmetricCipher.ECC_MAX_KEYSIZE, 'ecc key size' );

	var dsa = new AsymmetricCipher('dsa', 'sha1', fortuna);
	dsa.createKeys(64);
	QA.ASSERT( dsa.keySize, 64, 'dsa key size' );

	var rsa = new AsymmetricCipher('rsa', 'md5', fortuna );
	rsa.createKeys( 1024 );
	QA.ASSERT( rsa.keySize, 1024, 'rsa key size' );


/// crash 1

	var cr = new Cipher("CFB", "cast5", "my  key of  16B ", "xxxxxxx");
	var encryptedText = cr.encrypt("secret text");
	var cr = new Cipher("CFB", "cast5", "my  key of  16B ", IV);
	var IV;


/// crash 2

	var cr = new Cipher("CFB", "cast5", "my  key of  16B ", IV);
	var IV;


/// hash validity check

	var md5 = new Hash('md5');

	md5.write('e');
	QA.ASSERTOP( function() md5.done(), 'ex', undefined );
	QA.ASSERTOP( function() md5.done(), 'ex', Error );
