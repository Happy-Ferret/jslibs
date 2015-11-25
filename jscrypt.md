<b>If something seems wrong or incomplete, please enter <a href='#commentform.md'>a comment at the bottom of this page</a>.</b><br /><br />- [source](http://jslibs.googlecode.com/svn/trunk/./src/jscrypt/) - [main](JSLibs.md) - [QA](http://jslibs.googlecode.com/svn/trunk/./src/jscrypt/qa.js) -
# jscrypt module #

> jscrypt is a cryptographic toolkit that provides developers with a vast array of well known published block ciphers,
> one-way hash functions, chaining modes, pseudo-random number generators, public key cryptography and a plethora of other routines.
> The underlying native code of this module is based on [LibTom library](http://libtom.org/).




---

## jscrypt static members ##
- [top](#jscrypt_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jscrypt/misc.cpp?r=2555) -

### Static functions ###

#### <font color='white' size='1'><b>Base64Encode</b></font> ####
> <sub>string</sub> **Base64Encode**( string )
> > Encode the given _string_ using base64 encoding.

#### <font color='white' size='1'><b>Base64Decode</b></font> ####

> <sub>string</sub> **Base64Decode**( string )
> > Encode the given _string_ using base64 encoding.

#### <font color='white' size='1'><b>HexEncode</b></font> ####

> <sub>string</sub> **HexEncode**( string )
> > Encode the given _string_ using hexadecimal encoding.

#### <font color='white' size='1'><b>HexDecode</b></font> ####

> <sub>string</sub> **HexDecode**( string )
> > Decode the given _string_ using hexadecimal encoding.



---

## class jscrypt::Cipher ##
- [top](#jscrypt_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jscrypt/cipher.cpp?r=2557) -

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####

> <i><b>constructor</b></i>( modeName, cipherName, key, [[.md](.md)IV], [[.md](.md)arg], [[.md](.md)rounds] )
> > Constructs a Cipher object that use _cipherName_ algorithm for performing encryption and decryption.
> > ##### arguments: #####
      1. <sub>string</sub> _modeName_: is the block cipher modes of operation:
        * ECB (Electronic codebook)
        * CFB (Cipher feedback)
        * OFB (Output Feedback)
        * CBC (Cipher Block Chaining)
        * CTR (CounTeR)
        * LRW
        * F8
      1. <sub>string</sub> _cipherName_: is the name of the cipher algorithm used for data encryption and decryption:
        * blowfish
        * rc5
        * rc6
        * rc2
        * saferp
        * safer\_k64, safer\_k128, safer\_sk64, safer\_sk128
        * rijndael, aes
        * rijndael\_enc, aes\_enc
        * xtea
        * twofish
        * des, des3
        * cast5
        * noekeon
        * skipjack
        * khazad
        * anubis
        * kseed
        * kasumi
      1. <sub>string</sub> _key_: is the encryption/decryption key.
      1. <sub>string</sub> _IV_:
> > > > _IV_ is the first initialization vector:
> > > > The IV value is the initialization vector to be used with the cipher.
> > > > You must fill the IV yourself and it is assumed they are the same length as the block size of the cipher you choose.
> > > > It is important that the IV be random for each unique message you want to encrypt.
> > > > ##### <font color='red'>beware</font>: #####
> > > > > This argument is invalid in ECB block mode.
      1. <sub>string</sub> _arg_: is either the tweak key for the LRW mode or the salt value for the F8 mode. In other modes _arg_ must be undefined.

> > > > ##### <font color='red'>beware</font>: #####
> > > > > In LRW mode, the tweak value must have the same length as the _key_.
      1. <sub>integer</sub> _rounds_: is the number of rounds to do with the current sipher. If the argument is omitted, a default value is used.

### Methods ###

#### <font color='white' size='1'><b>Encrypt</b></font> ####

> <sub>bstring</sub> <b>Encrypt</b>( data )
> > Encrypts the given _data_ using the current cipher.

#### <font color='white' size='1'><b>Decrypt</b></font> ####

> <sub>bstring</sub> <b>Decrypt</b>( data )
> > Decrypts the given _data_ using the current cipher.

### Properties ###

#### <font color='white' size='1'><b>blockLength</b></font> ####

> <sub>integer</sub> <b>blockLength</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Is the block length of the current cipher.

#### <font color='white' size='1'><b>keySize</b></font> ####

> <sub>integer</sub> <b>keySize</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Is the key size of the current cipher.

#### <font color='white' size='1'><b>name</b></font> ####

> <sub>string</sub> <b>name</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Is the name of the current cipher.

#### <font color='white' size='1'><b>IV</b></font> ####

> <sub>string</sub> <b>IV</b>
> > Set or get the current initialization vector of the cipher.

### Static properties ###

#### <font color='white' size='1'><b>list</b></font> ####

> <sub>Object</sub> <b>list</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Contains the list of all available ciphers and their feature. The list is a javascript object that map cipher names (key) with another object (value) that contain information.


---

## class jscrypt::AsymmetricCipher ##
- [top](#jscrypt_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jscrypt/asymmetricCipher.cpp?r=2557) -

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####

> <i><b>constructor</b></i>( cipherName, hashName [[.md](.md), prngObject] [[.md](.md), PKCSVersion = 1\_OAEP] )
> > Creates a new Asymmetric Cipher object.
> > ##### arguments: #####
      1. <sub>string</sub> _cipherName_: is a string that contains the name of the Asymmetric Cipher algorithm:
        * rsa
        * ecc
        * dsa
      1. <sub>string</sub> _hashName_: is the hash that will be used to create the PSS (Probabilistic Signature Scheme) encoding. It should be the same as the hash used to hash the message being signed. See Hash class for available names.
      1. <sub>Object</sub> _prngObject_: is an instantiated Prng object. Its current state will be used for key creation, data encryption/decryption, data signature/signature check. This argument can be ommited if you aim to decrypt data only.
      1. <sub>string</sub> _PKCSVersion_: is a string that contains the padding version used by RSA to encrypt/decrypd data:
        * 1\_OAEP (for PKCS #1 v2.1 encryption)
        * 1\_V1\_5 (for PKCS #1 v1.5 encryption)
> > > > If omitted, the default value is 1\_OAEP.
> > > > Only RSA use this argument.

### Methods ###

#### <font color='white' size='1'><b>CreateKeys</b></font> ####

> <b>CreateKeys</b>( keySize )
> > Create RSA public and private keys.
> > <br />
> > _keySize_ is the size of the key in bits (the value of _keySize_ is the modulus size).
> > ##### note: #####
> > > supported RSA keySize: from 1024 to 4096 bits
> > > <br />
> > > supported ECC keySize: 112, 128, 160, 192, 224, 256, 384, 521, 528 bits
> > > <br />
> > > supported DSA keySize (Bits of Security): <b><font color='red'>???</font></b>, 80, 120, 140, 160, <b><font color='red'>???</font></b> bits

#### <font color='white' size='1'><b>Encrypt</b></font> ####

> <sub>bstring</sub> <b>Encrypt</b>( data [[.md](.md), lparam] )
> > This function returns the encrypted _data_ using a previously created or imported public key.
> > <br />
> > _data_ is the string to encrypt (usualy cipher keys).

#### <font color='white' size='1'><b>Decrypt</b></font> ####

> <sub>bstring</sub> <b>Decrypt</b>( encryptedData [[.md](.md), lparam] )
> > This function decrypts the given _encryptedData_ using a previously created or imported private key.
> > <br />
> > _encryptedData_ is the string that has to be decrypted (usualy cipher keys).
> > ##### note: #####
> > > The lparam variable is an additional system specific tag that can be applied to the encoding.
> > > This is useful to identify which system encoded the message.
> > > If no variance is desired then lparam can be ignored or set to _undefined_.
> > > <br />
> > > If it does not match what was used during encoding this function will not decode the packet.
> > > <br />
> > > When performing v1.5 RSA decryption, the hash and lparam parameters are totally ignored.

#### <font color='white' size='1'><b>Sign</b></font> ####

> <sub>string</sub> <b>Sign</b>( data [[.md](.md), saltLength] )
> > This function returns the signature of the given _data_.
> > Because this process is slow, this function usualy used to sign a small amount of data, like hash digest.
> > <br />
> > _saltLength_ is only used with RSA signatures. (default value is 16)

#### <font color='white' size='1'><b>VerifySignature</b></font> ####

> <sub>boolean</sub> <b>VerifySignature</b>( data, signature [[.md](.md), saltLength] )
> > This function returns _true_ if the _data_ match the data used to create the _signature_.
> > <br />
> > _saltLength_ is only used with RSA signatures. (default value is 16)

### Properties ###

#### <font color='white' size='1'><b>blockLength</b></font> ####

> <sub>integer</sub> <b>blockLength</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > is the maximum length of data that can be processed at once.

#### <font color='white' size='1'><b>keySize</b></font> ####

> <sub>integer</sub> <b>keySize</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > is the size of the current key.

#### <font color='white' size='1'><b>key</b></font> ####

> <sub>string</sub> **privateKey**
> > The private key encoded using PKCS #1. (Public Key Cryptographic Standard #1 v2.0 padding)

  * <sub>string</sub> **publicKey**
> > The public key encoded using PKCS #1. (Public Key Cryptographic Standard #1 v2.0 padding)

### Example ###
Data (or key) encryption using RSA:
```
LoadModule('jsstd');
LoadModule('jscrypt');
var fortuna = new Prng('fortuna');
fortuna.AutoEntropy(123); // give more entropy

//Alice
var alice = new AsymmetricCipher('RSA', 'md5', fortuna);
alice.CreateKeys(1024);
var publicKey = alice.publicKey;

//Bob
var bob = new AsymmetricCipher('RSA', 'md5', fortuna);
bob.publicKey = publicKey;
var encryptedData = bob.Encrypt('Alice, I love you !');

//Alice
Print( alice.Decrypt(encryptedData), '\n' );
```


---

## class jscrypt::Hash ##
- [top](#jscrypt_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jscrypt/hash.cpp?r=2557) -

> This class is used to create block Hash objects.

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####
> <i><b>constructor</b></i>( hashName )
> > Creates a new hash.
> > _hashName_ is a string that contains the name of the hash:
      * whirlpool
      * sha512
      * sha384
      * sha256
      * sha224
      * sha1
      * md5
      * md4
      * md2
      * tiger
      * rmd128
      * rmd160
      * rmd256
      * rmd320
      * chc\_hash
> > ##### note: #####
> > > chc\_hash is a special hash that allows to create a hash from a cipher (Cipher Hash Construction).
> > > See CipherHash() static function.

### Methods ###

#### <font color='white' size='1'><b>Init</b></font> ####

> <b>Init</b>()
> > Initialize the hash state.

#### <font color='white' size='1'><b>Process</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Process</b>( data )
> > Process a block of data though the hash.

#### <font color='white' size='1'><b>Done</b></font> ####

> <sub>bstring</sub> <b>Done</b>()
> > Terminate the hash and get the digest in a binary format.
> > ##### example: #####
```
  LoadModule('jsstd');
  LoadModule('jscrypt');
  var md5 = new Hash('md5');
  md5.Process('foo');
  md5.Process('bar');
  Print( HexEncode( md5.Done(), '\n' ) ); // prints: 3858F62230AC3C915F300C664312C63F
```

#### <font color='white' size='1'><i><b>call operator</b></i></font> ####

> <sub>bstring</sub> <i><b>call operator</b></i>( data )
> > This is the call operator of the object. It simplifies the usage of hashes and allows a digest calculation in one call only.
> > When called with a string as argument, it Process a block of memory though the hash
> > Compute the hash until the function is called without arguments or end is _true_. In this case, the function returns the hash of the whole given data.
> > ##### <font color='red'>beware</font>: #####
> > > Using this methode to compute a digest automaticaly resets previous state let by Init(), Process() or Done().

> > ##### example: #####
```
  LoadModule('jsstd');
  LoadModule('jscrypt');
  var md5 = new Hash('md5');
  Print( HexEncode( md5('foobar') ) ); // prints: 3858F62230AC3C915F300C664312C63F
```

### Properties ###

#### <font color='white' size='1'><b>name</b></font> ####

> <sub>string</sub> <b>name</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Name of the current hash.

#### <font color='white' size='1'><b>blockSize</b></font> ####

> <sub>integer</sub> <b>blockSize</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Input block size in octets.

#### <font color='white' size='1'><b>length</b></font> ####

> <sub>integer</sub> <b>length</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Size of the digest in octets.

#### <font color='white' size='1'><b>inputLength</b></font> ####

> <sub>integer</sub> <b>inputLength</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Length of the processed data.

### Static functions ###

#### <font color='white' size='1'><b>CipherHash</b></font> ####

> <b>CipherHash</b>( cipherName )
> > Initialize the CHC (chc\_hash) state with _cipherName_ cipher.
> > <br />
> > An addition to the suite of hash functions is the Cipher Hash Construction or CHC mode.
> > In this mode applicable block ciphers (such as AES) can be turned into hash functions that other functions can use.
> > In particular this allows a cryptosystem to be designed using very few moving parts.

### Static properties ###

#### <font color='white' size='1'><b>list</b></font> ####

> <sub>Object</sub> <b>list</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Contains the list of all available hash and their feature. The list is a javascript object that map hash names (key) with another object (value) that contain information.
> > ##### example: #####
```
  LoadModule('jsstd');
  LoadModule('jscrypt');
  Print('hash list: ' + Hash.list.toSource() );
```


---

## class jscrypt::Prng ##
- [top](#jscrypt_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jscrypt/prng.cpp?r=2557) -

> This class is used to create pseudo random number generators objects.

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####
> <i><b>constructor</b></i>( prngName )
> > Constructs a pseudo random number generator object using the given algorithm.
> > ##### arguments: #####
      1. <sub>string</sub> _prngName_: is a string that contains the name of the Asymmetric Cipher algorithm:
        * yarrow
        * fortuna
        * rc4
        * sprng
        * sober128

### Methods ###

#### <font color='white' size='1'><i><b>call operator</b></i></font> ####

> <sub>string</sub> <i><b>call operator</b></i>( size )
> > Returns _size_ bytes of pseudo-random data.
> > ##### example: #####
```
  LoadModule('jsstd');
  LoadModule('jscrypt');
  var myGen = new Prng('yarrow');
  myGen.AutoEntropy(128); // give more entropy
  Print(HexEncode(myGen(100))); // prints random data
```

#### <font color='white' size='1'><b>AddEntropy</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>AddEntropy</b>( data )
> > Add _data_ as entropy (randomness) to the current prng.

#### <font color='white' size='1'><b>AutoEntropy</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>AutoEntropy</b>( size )
> > Automaticaly add _size_ bits of entropy to the current prng.

### Static properties ###

#### <font color='white' size='1'><b>state</b></font> ####

> <sub>bstring</sub> <b>state</b>
> > is the current state of the prng.

#### <font color='white' size='1'><b>name</b></font> ####

> <sub>string</sub> <b>name</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > is the name of the current prng.

#### <font color='white' size='1'><b>list</b></font> ####

> <sub>Object</sub> <b>list</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Contains the list of all available prng and their feature. The list is a javascript object that map cipher names (key) with another object (value) that contain information.


---

## class jscrypt::CryptError ##
- [top](#jscrypt_module.md) -



---

- [top](#jscrypt_module.md) - [main](JSLibs.md) -