({

	MD5digest: function(QA) {
	
		LoadModule('jscrypt');

		var md5 = new Hash('md5');
		md5.Process('foobarxxx');
		QA.ASSERT_STR( md5.Done(), "\b\t\xAA\xC0\x10\xB1G\xA8\xC71'}*\x80\x07\xBF", 'digest integrity' );
	},


	CipherHashPrngList: function(QA) {

		LoadModule('jscrypt');
	
		QA.ASSERT( Cipher.list.toSource().length, 1609, 'length of Cipher.list' );
		QA.ASSERT( Hash.list.toSource().length, 535, 'length of Hash.list' );
		QA.ASSERT( Prng.list.toSource().length, 51, 'length of Prng.list' );
	},


	DSAAsymmetricCryptDecrypt: function(QA) {
		
		LoadModule('jscrypt');
		var fortuna = new Prng('fortuna');
		fortuna.AutoEntropy(123); // give more entropy
		//Alice
		var alice = new AsymmetricCipher('dsa', 'sha1', fortuna);
		alice.CreateKeys(64);
		var publicKey = alice.publicKey;
		//Bob
		var bob = new AsymmetricCipher('dsa', 'sha1', fortuna);
		bob.publicKey = publicKey;
		QA.ASSERT( bob.blockLength, 20, 'Asymmetric Cipher block length' );
		QA.ASSERT( bob.keySize, 512, 'Asymmetric Cipher key size' );
		var secretMessage = QA.RandomString(bob.blockLength);
		var encryptedData = bob.Encrypt( secretMessage );
		//Alice
		QA.ASSERT_STR( alice.Decrypt(encryptedData), secretMessage, 'data integrity' );
	},


	RSAAsymmetricCryptDecrypt: function(QA) {

		LoadModule('jscrypt');

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

		QA.ASSERT_STR( plainText, rsa.Decrypt( rsaEncryptedData ), 'data integrity' );
	},

	Cipher1: function(QA) {

		LoadModule('jscrypt');
		
		var data = QA.RandomString(300000);
		
		var IV = QA.RandomString(100);
		var cr = new Cipher('CFB', "cast5", "my  key of  16B ", IV );
		var encryptedText = cr.Encrypt(data);
		var cr = new Cipher('CFB', "cast5", "my  key of  16B ", IV );
		
		QA.ASSERT_STR( cr.Decrypt(encryptedText), data, 'crypy/decript with Cast5 cipher using CFB mode' );
	},
	

	Cipher2: function(QA) {

		LoadModule('jscrypt');
/*
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
*/
	}


})