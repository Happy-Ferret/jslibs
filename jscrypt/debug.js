Exec('deflib.js');
LoadModule('jscrypt');

try {

	var hkey = new Hash("sha256")('this is a secret key',true);

	
	var r = new Prng('yarrow');
	r.AddEntropy('something random');
	r.AutoEntropy(128); // give more entropy
	
	var key = hkey;
	var IV = r(Crypt.BlockLength('twofish'));

// encrypt:
	var crypt = new Crypt( 'ctr', 'twofish', key, IV );
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
