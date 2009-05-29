/* ***** BEGIN LICENSE BLOCK *****
 * Version: GNU GPL 2.0
 *
 * The contents of this file are subject to the
 * GNU General Public License Version 2.0; you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at
 * http://www.gnu.org/licenses/gpl.html
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 * ***** END LICENSE BLOCK ***** */

#include "stdafx.h"
#include "asymmetricCipher.h"

#include "prng.h"
#include "hash.h"

#include <tommath.h>

#include <math.h>

enum AsymmetricCipher {
	rsa,
	ecc,
	dsa,
};

union AsymmetricKey {
	rsa_key rsaKey;
	ecc_key eccKey;
	dsa_key dsaKey;
};

struct AsymmetricCipherPrivate {
	AsymmetricCipher cipher;
	AsymmetricKey key;
	bool hasKey;
	ltc_pkcs_1_paddings padding;
	ltc_hash_descriptor *hashDescriptor;
};


JSBool SlotGetPrng(JSContext *cx, JSObject *obj, int *prngIndex, prng_state **prngState) {

	jsval prngVal;
	JL_CHK( JS_GetReservedSlot(cx, obj, ASYMMETRIC_CIPHER_PRNG_SLOT, &prngVal) );
	JL_S_ASSERT_OBJECT(	prngVal );
	JL_S_ASSERT_CLASS( JSVAL_TO_OBJECT(prngVal), classPrng );
	PrngPrivate *prngPrivate;
	prngPrivate = (PrngPrivate *)JL_GetPrivate(cx, JSVAL_TO_OBJECT(prngVal));
	JL_S_ASSERT_RESOURCE( prngPrivate );
	*prngState = &prngPrivate->state;
	*prngIndex = find_prng(prngPrivate->prng.name);
	JL_S_ASSERT_1( *prngIndex != -1, "prng %s is not available.", prngPrivate->prng.name );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( AsymmetricCipher )


DEFINE_FINALIZE() {

	AsymmetricCipherPrivate *pv = (AsymmetricCipherPrivate *)JL_GetPrivate(cx, obj);
	if ( pv == NULL )
		return;
	if ( pv->hasKey ) {

		switch ( pv->cipher ) {
			case rsa:
				rsa_free( &pv->key.rsaKey );
				break;
			case ecc:
				ecc_free( &pv->key.eccKey );
				break;
			case dsa:
				dsa_free( &pv->key.dsaKey );
				break;
		}
	}
	zeromem(pv, sizeof(AsymmetricCipherPrivate)); // safe clean
	JS_free(cx, pv);
}

/**doc
$TOC_MEMBER $INAME
 $INAME( cipherName, hashName [, prngObject] [, PKCSVersion = 1_OAEP] )
  Creates a new Asymmetric Cipher object.
  $H arguments
   $ARG $STR cipherName: is a string that contains the name of the Asymmetric Cipher algorithm:
    * rsa
    * ecc
    * dsa
   $ARG $STR hashName: is the hash that will be used to create the PSS (Probabilistic Signature Scheme) encoding. It should be the same as the hash used to hash the message being signed. See Hash class for available names.
   $ARG $OBJ prngObject: is an instantiated Prng object. Its current state will be used for key creation, data encryption/decryption, data signature/signature check. This argument can be ommited if you aim to decrypt data only.
   $ARG $STR PKCSVersion: is a string that contains the padding version used by RSA to encrypt/decrypd data:
    * 1_OAEP (for PKCS #1 v2.1 encryption)
    * 1_V1_5 (for PKCS #1 v1.5 encryption)
    If omitted, the default value is 1_OAEP.
    Only RSA use this argument.
**/
DEFINE_CONSTRUCTOR() { // ( cipherName, hashName [, prngObject] [, PKCSVersion] )

	JL_S_ASSERT_CONSTRUCTING();
	JL_S_ASSERT_THIS_CLASS();
	JL_S_ASSERT_ARG_MIN( 3 );

	const char *asymmetricCipherName;
	JL_CHK( JsvalToString(cx, &argv[0], &asymmetricCipherName) );

	AsymmetricCipher asymmetricCipher;
	if ( strcasecmp( asymmetricCipherName, "RSA" ) == 0 )
		asymmetricCipher = rsa;
	else if ( strcasecmp( asymmetricCipherName, "ECC" ) == 0 )
		asymmetricCipher = ecc;
	else if ( strcasecmp( asymmetricCipherName, "DSA" ) == 0 )
		asymmetricCipher = dsa;
	else
		JL_REPORT_ERROR_1("Invalid asymmetric cipher %s.", asymmetricCipherName);

	AsymmetricCipherPrivate *pv;
	pv = (AsymmetricCipherPrivate *)JS_malloc(cx, sizeof(AsymmetricCipherPrivate));
	JL_CHK( pv );

	pv->cipher = asymmetricCipher;

	const char *hashName;
	JL_CHK( JsvalToString(cx, &argv[1], &hashName) );
	int hashIndex;
	hashIndex = find_hash(hashName);
	JL_S_ASSERT_1( hashIndex != -1, "hash %s is not available.", hashName );
	pv->hashDescriptor = &hash_descriptor[hashIndex];

	if ( argc >= 3 ) {

		JL_S_ASSERT_OBJECT(	argv[2] );
		JL_S_ASSERT_CLASS( JSVAL_TO_OBJECT(argv[2]), classPrng );
		JL_CHK( JS_SetReservedSlot(cx, obj, ASYMMETRIC_CIPHER_PRNG_SLOT, argv[2]) );
	} else {

		JL_CHK( JS_SetReservedSlot(cx, obj, ASYMMETRIC_CIPHER_PRNG_SLOT, JSVAL_VOID) );
	}

	pv->padding = LTC_LTC_PKCS_1_OAEP;

	if ( argc >= 4 && !JSVAL_IS_VOID( argv[3] ) ) {

		const char *paddingName;
		JL_CHK( JsvalToString(cx, &argv[3], &paddingName) );

		if ( strcmp(paddingName, "1_OAEP") == 0 ) {

			pv->padding = LTC_LTC_PKCS_1_OAEP;
		} else if ( strcmp(paddingName, "1_V1_5") == 0 ) {

			pv->padding = LTC_LTC_PKCS_1_V1_5;
		} else
			JL_REPORT_ERROR("Invalid padding version.");
	}

	pv->hasKey = false;
	JL_SetPrivate( cx, obj, pv );
	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Methods ===
**/

/**doc
$TOC_MEMBER $INAME
 $INAME( keySize )
  Create RSA public and private keys.
  $LF
  _keySize_ is the size of the key in bits (the value of _keySize_ is the modulus size).
  $H note
   supported RSA keySize: from 1024 to 4096 bits
   $LF
   supported ECC keySize: 112, 128, 160, 192, 224, 256, 384, 521, 528 bits
   $LF
   supported DSA keySize (Bits of Security): ???, 80, 120, 140, 160, ??? bits
**/
DEFINE_FUNCTION( CreateKeys ) { // ( bitsSize )

	JL_S_ASSERT_CLASS( obj, _class );
	JL_S_ASSERT_ARG_MIN( 1 );
	AsymmetricCipherPrivate *pv;
	pv = (AsymmetricCipherPrivate *)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( pv );

	prng_state *prngState;
	int prngIndex;
	JL_CHK( SlotGetPrng(cx, obj, &prngIndex, &prngState) );

	unsigned int keySize;
	JL_CHK( JsvalToUInt(cx, argv[0], &keySize) );

	int err;
	err = -1; // default
	switch ( pv->cipher ) {
		case rsa: {
			int e = 65537; // typical values are 3, 17, 257 and 65537
			int modulusSize = keySize / 8; // Bytes
			err = rsa_make_key( prngState, prngIndex, modulusSize, e, &pv->key.rsaKey );
			break;
		}
		case ecc: {
			int modulusSize = keySize / 8; // Bytes
			err = ecc_make_key( prngState, prngIndex, modulusSize, &pv->key.eccKey );
			break;
		}
		case dsa: {
			//Bits of Security / group size / modulus size
			//80 20 128 (2^7)
			//120 30 256 (2^8)
			//140 35 384
			//160 40 512 (2^9)
			// Max diff between group and modulus size in bytes: MDSA_DELTA (512)
			// Max DSA group size in bytes (default allows 4k-bit groups): MDSA_MAX_GROUP 512
			int groupSize = keySize / 4; // Bytes
			int modulusSize = 1 << (keySize / 40 + 5); // Bytes
			err = dsa_make_key( prngState, prngIndex, groupSize, modulusSize, &pv->key.dsaKey );

			//int stat = 0; // default: failed
			//dsa_verify_key(&pv->key.dsaKey, &stat);
			//if (stat != 1) // If the result is stat = 1 the DSA key is valid (as far as valid mathematics are concerned).
			//	JL_REPORT_ERROR("Invalid key.");
			break;
		}
		default:
			JL_REPORT_ERROR("Invalid case.");
	}
	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err);

	pv->hasKey = true;
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $DATA $INAME( data [, lparam] )
  This function returns the encrypted _data_ using a previously created or imported public key.
  $LF
  _data_ is the string to encrypt (usualy cipher keys).
**/
DEFINE_FUNCTION( Encrypt ) { // ( data [, lparam] )

	JL_S_ASSERT_CLASS( obj, _class );
	JL_S_ASSERT_ARG_MIN( 1 );
	AsymmetricCipherPrivate *pv;
	pv = (AsymmetricCipherPrivate *)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( pv );
	JL_S_ASSERT( pv->hasKey, "No key found." );

	prng_state *prngState;
	int prngIndex;
	JL_CHK( SlotGetPrng(cx, obj, &prngIndex, &prngState) );

	int hashIndex;
	hashIndex = find_hash(pv->hashDescriptor->name);

	const char *in;
	size_t inLength;
	JL_CHK( JsvalToStringAndLength( cx, &argv[0], &in, &inLength ) );

	char out[4096];
	unsigned long outLength;
	outLength = sizeof(out);
	int err;
	err = -1; // default
	switch ( pv->cipher ) {
		case rsa: {
			// lparam doc: The lparam variable is an additional system specific tag that can be applied to the encoding.
			// This is useful to identify which system encoded the message. If no variance is desired then lparam can be set to NULL.
			unsigned char *lparam = NULL; // default: lparam not used
			unsigned long lparamlen = 0;
			if (argc >= 2 && !JSVAL_IS_VOID( argv[1] ))
				JL_CHK( JsvalToStringAndLength(cx, &argv[1], &in, &inLength) );
			err = rsa_encrypt_key_ex( (unsigned char *)in, inLength, (unsigned char *)out, &outLength, lparam, lparamlen, prngState, prngIndex, hashIndex, pv->padding, &pv->key.rsaKey ); // ltc_mp.rsa_me()
			break;
		}
		case ecc: {
			err = ecc_encrypt_key( (unsigned char *)in, inLength, (unsigned char *)out, &outLength, prngState, prngIndex, hashIndex, &pv->key.eccKey );
			break;
		}
		case dsa: {
			err = dsa_encrypt_key( (unsigned char *)in, inLength, (unsigned char *)out, &outLength, prngState, prngIndex, hashIndex, &pv->key.dsaKey );
			break;
		}
		default:
			JL_REPORT_ERROR("Invalid case.");
	}
	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err);

	JL_CHK( JL_NewBlobCopyN(cx, out, outLength, rval) );
	zeromem(out, sizeof(out)); // safe clear

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $DATA $INAME( encryptedData [, lparam] )
  This function decrypts the given _encryptedData_ using a previously created or imported private key.
  $LF
  _encryptedData_ is the string that has to be decrypted (usualy cipher keys).
  $H note
   The lparam variable is an additional system specific tag that can be applied to the encoding.
   This is useful to identify which system encoded the message.
   If no variance is desired then lparam can be ignored or set to $UNDEF.
   $LF
   If it does not match what was used during encoding this function will not decode the packet.
   $LF
   When performing v1.5 RSA decryption, the hash and lparam parameters are totally ignored.
**/
DEFINE_FUNCTION( Decrypt ) { // ( encryptedData [, lparam] )

	JL_S_ASSERT_CLASS( obj, _class );
	JL_S_ASSERT_ARG_MIN( 1 );
	AsymmetricCipherPrivate *pv;
	pv = (AsymmetricCipherPrivate *)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( pv );
	JL_S_ASSERT( pv->hasKey, "No key found." );

	const char *in;
	size_t inLength;
	JL_CHK( JsvalToStringAndLength(cx, &argv[0], &in, &inLength) );

	char out[4096];
	unsigned long outLength;
	outLength = sizeof(out);

	int err;
	err = -1; // default
	switch ( pv->cipher ) {
		case rsa: {

			int hashIndex = find_hash(pv->hashDescriptor->name);

			const char *lparam = NULL; // default: lparam not used
			size_t lparamlen = 0;
			if (argc >= 2 && !JSVAL_IS_VOID( argv[1] ))
				JL_CHK( JsvalToStringAndLength(cx, &argv[1], &lparam, &lparamlen) );

			int stat = 0; // default: failed
			err = rsa_decrypt_key_ex( (unsigned char *)in, inLength, (unsigned char *)out, &outLength, (const unsigned char *)lparam, lparamlen, hashIndex, pv->padding, &stat, &pv->key.rsaKey );
			// doc: if all went well pt == pt2, l2 == 16, res == 1
			if ( err == CRYPT_OK && stat != 1 ) {

				*rval = JSVAL_VOID;
				return JS_TRUE;
			}
			break;
		}
		case ecc: {
			err = ecc_decrypt_key( (unsigned char *)in, inLength, (unsigned char *)out, &outLength, &pv->key.eccKey );
			break;
		}
		case dsa: {
			err = dsa_decrypt_key( (unsigned char *)in, inLength, (unsigned char *)out, &outLength, &pv->key.dsaKey );
			break;
		}
		default:
			JL_REPORT_ERROR("Invalid case.");
	}

	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err);

	out[outLength] = '\0';

	JL_CHK( JL_NewBlobCopyN( cx, out, outLength, rval ) );
	zeromem(out, sizeof(out));

	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $STR $INAME( data [, saltLength] )
  This function returns the signature of the given _data_.
  Because this process is slow, this function usualy used to sign a small amount of data, like hash digest.
  $LF
  _saltLength_ is only used with RSA signatures. (default value is 16)
**/
DEFINE_FUNCTION( Sign ) { // ( data [, saltLength] )

	JL_S_ASSERT_CLASS( obj, _class );
	JL_S_ASSERT_ARG_MIN( 1 );
	AsymmetricCipherPrivate *pv;
	pv = (AsymmetricCipherPrivate *)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( pv );

	JL_S_ASSERT( pv->hasKey, "No key found." );

	prng_state *prngState;
	int prngIndex;
	JL_CHK( SlotGetPrng(cx, obj, &prngIndex, &prngState) );

	const char *in;
	size_t inLength;
	JL_CHK( JsvalToStringAndLength(cx, &argv[0], &in, &inLength) );

	char out[4096];
	unsigned long outLength;
	outLength = sizeof(out);

	int err;
	err = -1; // default
	switch ( pv->cipher ) {
		case rsa: {
			int hashIndex = find_hash(pv->hashDescriptor->name);
			// A good saltLength default value is between 8 and 16 octets. Strictly, it must be small than modulus len - hLen - 2 where modulus len is the size of the RSA modulus (in octets), and hLen is the length of the message digest produced by the chosen hash.
			// int saltLength = 16; // OR saltLength = mp_unsigned_bin_size((mp_int*)(pv->key.rsaKey.N)) - hash_descriptor[hashIndex].hashsize - 2  -1;
			int saltLength = RSA_SIGN_DEFAULT_SALT_LENGTH;
			if ( argc >= 2 && !JSVAL_IS_VOID( argv[1] ) )
				JL_CHK( JsvalToInt(cx, argv[1], &saltLength) );

			err = rsa_sign_hash_ex( (unsigned char *)in, inLength, (unsigned char *)out, &outLength, LTC_LTC_PKCS_1_PSS, prngState, prngIndex, hashIndex, saltLength, &pv->key.rsaKey );
			break;
		}
		case ecc: {
			err = ecc_sign_hash( (unsigned char *)in, inLength, (unsigned char *)out, &outLength, prngState, prngIndex, &pv->key.eccKey );
			break;
		}
		case dsa: {
			err = dsa_sign_hash( (unsigned char *)in, inLength, (unsigned char *)out, &outLength, prngState, prngIndex, &pv->key.dsaKey );
			break;
		}
		default:
			JL_REPORT_ERROR("Invalid case.");
	}

	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err);

	JL_CHK( JL_NewBlobCopyN( cx, out, outLength, rval ) );
	zeromem(out, sizeof(out));

	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME( data, signature [, saltLength] )
  This function returns $TRUE if the _data_ match the data used to create the _signature_.
  $LF
  _saltLength_ is only used with RSA signatures. (default value is 16)
**/
DEFINE_FUNCTION( VerifySignature ) { // ( data, signature [, saltLength] )

	JL_S_ASSERT_CLASS( obj, _class );
	JL_S_ASSERT_ARG_MIN( 2 );
	AsymmetricCipherPrivate *pv;
	pv = (AsymmetricCipherPrivate *)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( pv );
	JL_S_ASSERT( pv->hasKey, "No key found." );

	const char *data;
	size_t dataLength;
	JL_CHK( JsvalToStringAndLength(cx, &argv[0], &data, &dataLength ) ); // warning: GC on the returned buffer !

	const char *sign;
	size_t signLength;
	JL_CHK( JsvalToStringAndLength(cx, &argv[1], &sign, &signLength) ); // warning: GC on the returned buffer !

	int stat;
	stat = 0; // default: failed
	int err;
	err = -1; // default: Invalid error code
	switch ( pv->cipher ) {
		case rsa: {
			int saltLength = RSA_SIGN_DEFAULT_SALT_LENGTH; // default
			if ( argc >= 3 && !JSVAL_IS_VOID( argv[2] ) )
				JL_CHK( JsvalToInt(cx, argv[2], &saltLength) );

			int hashIndex = find_hash(pv->hashDescriptor->name);

			rsa_verify_hash_ex( (unsigned char *)sign, signLength, (unsigned char *)data, dataLength, LTC_LTC_PKCS_1_PSS, hashIndex, saltLength, &stat, &pv->key.rsaKey );
			break;
		}
		case ecc: {
			ecc_verify_hash( (unsigned char *)sign, signLength, (unsigned char *)data, dataLength, &stat, &pv->key.eccKey );
			break;
		}
		case dsa: {
			dsa_verify_hash( (unsigned char *)sign, signLength, (unsigned char *)data, dataLength, &stat, &pv->key.dsaKey );
			break;
		}
		default:
			JL_REPORT_ERROR("Invalid case.");
	}

	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err);

	*rval = BOOLEAN_TO_JSVAL( stat == 1 ? JSVAL_TRUE : JSVAL_FALSE );
	return JS_TRUE;
	JL_BAD;
}


/**doc
=== Properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  is the maximum length of data that can be processed at once.
**/
DEFINE_PROPERTY( blockLength ) {

	JL_S_ASSERT_CLASS( obj, _class );
	AsymmetricCipherPrivate *pv;
	pv = (AsymmetricCipherPrivate *)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( pv );
	JL_S_ASSERT( pv->hasKey, "No key found." );
	int blockLength;
	switch ( pv->cipher ) {
		case rsa:
			blockLength = mp_unsigned_bin_size((mp_int*)(pv->key.rsaKey.N)) - 2 * pv->hashDescriptor->hashsize - 2; // Ok !
			break;
		case ecc:
			blockLength = pv->hashDescriptor->hashsize; // seems OK
			break;
		case dsa:
			blockLength = pv->hashDescriptor->hashsize; // seems OK
			break;
		default:
			JL_REPORT_ERROR("Invalid case.");
	}
	*vp = INT_TO_JSVAL( blockLength );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  is the size of the current key.
**/
DEFINE_PROPERTY( keySize ) {

	JL_S_ASSERT_CLASS( obj, _class );
	AsymmetricCipherPrivate *pv;
	pv = (AsymmetricCipherPrivate *)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( pv );
	JL_S_ASSERT( pv->hasKey, "No key found." );
	int keySize;
	switch ( pv->cipher ) {
		case rsa:
			keySize = mp_unsigned_bin_size((mp_int*)(pv->key.rsaKey.N)) * 8; // Ok !
			break;
		case ecc:
			keySize = mp_unsigned_bin_size((mp_int*)(pv->key.eccKey.pubkey.x)) * 8; // Ok !
			break;
		case dsa:
//			keySize = mp_unsigned_bin_size((mp_int*)(pv->key.dsaKey.x)) * 8;
			keySize = mp_unsigned_bin_size((mp_int*)(pv->key.dsaKey.y)) * 8; // ??? seems to randomly failed.
			break;
		default:
			JL_REPORT_ERROR("Invalid case.");
	}
	*vp = INT_TO_JSVAL( keySize );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR *privateKey*
  The private key encoded using PKCS #1. (Public Key Cryptographic Standard #1 v2.0 padding)

 * $STR *publicKey*
  The public key encoded using PKCS #1. (Public Key Cryptographic Standard #1 v2.0 padding)
**/

DEFINE_PROPERTY( keySetter ) {

	JL_S_ASSERT_CLASS( obj, _class );
	AsymmetricCipherPrivate *pv;
	pv = (AsymmetricCipherPrivate *)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( pv );

	int type;
	JL_CHK( JsvalToInt(cx, id, &type) );

	const char *key;
	size_t keyLength;
	JL_CHK( JsvalToStringAndLength(cx, vp, &key, &keyLength) );

	int err;
	err = -1; // default
	switch ( pv->cipher ) {
		case rsa:
			err = rsa_import( (unsigned char *)key, keyLength, &pv->key.rsaKey );
			JL_S_ASSERT( pv->key.rsaKey.type == type, "Invalid key type." );
			break;
		case ecc:
			err = ecc_import( (unsigned char *)key, keyLength, &pv->key.eccKey );
			JL_S_ASSERT( pv->key.rsaKey.type == type, "Invalid key type." );
			break;
		case dsa: {
			err = dsa_import( (unsigned char *)key, keyLength, &pv->key.dsaKey );
			JL_S_ASSERT( pv->key.rsaKey.type == type, "Invalid key type." );
			//int stat = 0;
			//dsa_verify_key(&pv->key.dsaKey, &stat);
			//if (stat != 1) // If the result is stat = 1 the DSA key is valid (as far as valid mathematics are concerned).
			//	JL_REPORT_ERROR("Invalid key.");
			break;
		}
		default:
			JL_REPORT_ERROR("Invalid case.");
	}
	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err);

	pv->hasKey = true;
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY( keyGetter ) {

	JL_S_ASSERT_CLASS( obj, _class );
	AsymmetricCipherPrivate *pv;
	pv = (AsymmetricCipherPrivate *)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( pv );
	JL_S_ASSERT( pv->hasKey, "No key found." );

	JSBool jsErr;
	int32 type;
	jsErr = JS_ValueToInt32(cx, id, &type);
	JL_S_ASSERT( jsErr == JS_TRUE, "Invalid operation." );

	char key[4096];
	unsigned long keyLength;
	keyLength = sizeof(key);

	int err;
	err = -1; // default
	switch ( pv->cipher ) {
		case rsa:
			err = rsa_export( (unsigned char *)key, &keyLength, type, &pv->key.rsaKey );
			break;
		case ecc:
			err = ecc_export( (unsigned char *)key, &keyLength, type, &pv->key.eccKey );
			break;
		case dsa:
			err = dsa_export( (unsigned char *)key, &keyLength, type, &pv->key.dsaKey );
			break;
		default:
			JL_REPORT_ERROR("Invalid case.");
	}
	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err);

	JL_CHK( JL_NewBlobCopyN(cx, key, keyLength, vp) );
	zeromem(key, sizeof(key)); // safe clean

	return JS_TRUE;
	JL_BAD;
}


enum {
	publicKey = PK_PUBLIC,
	privateKey = PK_PRIVATE,
};


CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION( CreateKeys )
		FUNCTION( Encrypt )
		FUNCTION( Decrypt )
		FUNCTION( Sign )
		FUNCTION( VerifySignature )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ( blockLength )
		PROPERTY_READ( keySize )
		PROPERTY_SWITCH( privateKey, key )
		PROPERTY_SWITCH( publicKey, key )
	END_PROPERTY_SPEC

	HAS_PRIVATE
	HAS_RESERVED_SLOTS( 1 )

END_CLASS

/**doc
=== Example ===
Data (or key) encryption using RSA:
{{{
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
}}}
**/
