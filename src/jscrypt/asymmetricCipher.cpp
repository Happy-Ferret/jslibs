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

DECLARE_CLASS( Prng )


enum AsymmetricCipherType {
	rsa,
	ecc,
	dsa,
#ifdef MKAT
	katja,
#endif
};

union AsymmetricKey {
	rsa_key rsaKey;
	ecc_key eccKey;
	dsa_key dsaKey;
#ifdef MKAT
	katja_key katjaKey;
#endif
};

struct AsymmetricCipherPrivate {
	AsymmetricCipherType cipher;
	AsymmetricKey key;
	bool hasKey;
	ltc_pkcs_1_paddings padding;
	int hashIndex;
};


JSBool SlotGetPrng(JSContext *cx, JSObject *obj, int *prngIndex, prng_state **prngState) {

	jsval prngVal;
	JL_CHK( JL_GetReservedSlot( obj, ASYMMETRIC_CIPHER_PRNG_SLOT, &prngVal) );
	JL_ASSERT_OBJECT_STATE( JSVAL_IS_OBJECT(prngVal), JL_CLASS_NAME(Prng) );
	JL_ASSERT_INSTANCE( JSVAL_TO_OBJECT(prngVal), JL_CLASS(Prng) );
	PrngPrivate *prngPrivate;
	prngPrivate = (PrngPrivate *)JL_GetPrivate(JSVAL_TO_OBJECT(prngVal));
	JL_ASSERT_OBJECT_STATE( prngPrivate, JL_CLASS_NAME(Prng) );
	*prngState = &prngPrivate->state;
	*prngIndex = find_prng(prngPrivate->prng.name);
	JL_ASSERT( *prngIndex != -1, E_STR("PRNG"), E_NAME(prngPrivate->prng.name), E_NOTFOUND );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( AsymmetricCipher )

ALWAYS_INLINE void
FinalizeAsymmetricCipher( JSObject *obj, bool wipe ) {

	AsymmetricCipherPrivate *pv = (AsymmetricCipherPrivate*)JL_GetPrivate(obj);
	if ( pv ) {

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
			#ifdef MKAT
				case katja:
					katja_free( &pv->key.katjaKey );
					break;
			#endif
			}
		}

		if ( wipe )
			zeromem(pv, sizeof(AsymmetricCipherPrivate));
		jl_free(pv);
	}
}


DEFINE_FINALIZE() {

	if ( JL_GetHostPrivate(fop->runtime())->canSkipCleanup )
		return;
	FinalizeAsymmetricCipher(obj, false);
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
    * katja
   $ARG $STR hashName: is the hash that will be used to create the PSS (Probabilistic Signature Scheme) encoding. It should be the same as the hash used to hash the message being signed. See Hash class for available names.
   $ARG $OBJ prngObject: is an instantiated Prng object. Its current state will be used for key creation, data encryption/decryption, data signature/signature check. This argument can be ommited if you aim to decrypt data only.
   $ARG $STR PKCSVersion: is a string that contains the padding version used by RSA to encrypt/decrypd data:
    * 1_V1_5 (for PKCS#1 v1.5 padding)
    * 1_OAEP (for PKCS#1 v2.0 encryption padding)
    If omitted, the default value is 1_OAEP.
	 Only RSA use this argument.
    $H note
    When performing v1.5 encryption, the hash and lparam parameters are totally ignored.
**/
DEFINE_CONSTRUCTOR() { // ( cipherName [, hashName] [, prngObject] [, PKCSVersion] )

	AsymmetricCipherPrivate *pv = NULL;
	JLData asymmetricCipherName, hashName;

	JL_ASSERT_ARGC_MIN( 3 );
	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;


	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &asymmetricCipherName) );

	AsymmetricCipherType asymmetricCipher;
	if ( strcasecmp( asymmetricCipherName, "RSA" ) == 0 )
		asymmetricCipher = rsa;
	else if ( strcasecmp( asymmetricCipherName, "DSA" ) == 0 )
		asymmetricCipher = dsa;
	else if ( strcasecmp( asymmetricCipherName, "ECC" ) == 0 )
		asymmetricCipher = ecc;
#ifdef MKAT
	else if ( strcasecmp( asymmetricCipherName, "KATJA" ) == 0 )
		asymmetricCipher = katja;
#endif
	else
		JL_ERR( E_ARG, E_NUM(1), E_INVALID, E_SEP, E_NAME(asymmetricCipherName), E_NOTSUPPORTED );

	pv = (AsymmetricCipherPrivate *)jl_malloc(sizeof(AsymmetricCipherPrivate));
	JL_CHK( pv );

	pv->cipher = asymmetricCipher;

	if ( JL_ARG_ISDEF(2) ) {

		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &hashName) );
		pv->hashIndex = find_hash(hashName);
	} else {

		pv->hashIndex = -1;
	}

	if ( argc >= 3 ) {

		JL_ASSERT_ARG_IS_OBJECT(3);
		JL_ASSERT_INSTANCE( JSVAL_TO_OBJECT(JL_ARG(3)), JL_CLASS(Prng) );
		JL_CHK( JL_SetReservedSlot( obj, ASYMMETRIC_CIPHER_PRNG_SLOT, JL_ARG(3)) );
	} else {

		JL_CHK( JL_SetReservedSlot( obj, ASYMMETRIC_CIPHER_PRNG_SLOT, JSVAL_VOID) );
	}

	if ( asymmetricCipher == rsa ) {

		if ( JL_ARGC >= 4 && !JSVAL_IS_VOID( JL_ARG(4) ) ) {

			JLData paddingName;
			JL_CHK( JL_JsvalToNative(cx, JL_ARG(4), &paddingName) );

			if ( strcasecmp(paddingName, "1_OAEP") == 0 ) {
				pv->padding = LTC_LTC_PKCS_1_OAEP;
			} else
			if ( strcasecmp(paddingName, "1_V1_5") == 0 ) {
				pv->padding = LTC_LTC_PKCS_1_V1_5;
			} else
				JL_ERR( E_ARG, E_NUM(4), E_INVALID, E_SEP, E_NAME(paddingName), E_NOTSUPPORTED );
		} else {

			pv->padding = LTC_LTC_PKCS_1_OAEP; // default
		}
	} else {

		JL_ASSERT_ARGC_MAX( 3 );
	}

	pv->hasKey = false;

	JL_SetPrivate(obj, pv);
	return JS_TRUE;
bad:
	jl_free(pv);
	return JS_FALSE;
}

/**doc
=== Methods ===
**/


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Cleanup and free internal data.
  $H note
   This object may contain sensitive data.
**/
DEFINE_FUNCTION( wipe ) {

	JL_IGNORE(argc);

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	FinalizeAsymmetricCipher(obj, true);
	JL_SetPrivate( obj, NULL);
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( keySize [, verify] )
  Create public and private keys.
  $LF
  _keySize_ is the size of the key in bits (the modulus size). from AsymmetricCipher.[RSA|ECC|DSA|KATJA]_MIN_KEYSIZE to AsymmetricCipher.[RSA|ECC|DSA|KATJA]_MAX_KEYSIZE bits.
  $LF
  _verify_ is a boolean. If true, the key is verified. (DSA only)
  $H example:
{{{
var ac = new AsymmetricCipher( 'rsa', 'md5', new Prng('fortuna') );
ac.createKeys( AsymmetricCipher.RSA_MIN_KEYSIZE );
}}}
**/
DEFINE_FUNCTION( createKeys ) { // ( bitsSize )

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN( 1 );

	AsymmetricCipherPrivate *pv;
	pv = (AsymmetricCipherPrivate *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	prng_state *prngState;
	int prngIndex;
	JL_CHK( SlotGetPrng(cx, obj, &prngIndex, &prngState) );

	unsigned int keySize;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &keySize) );

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
			int modulusSize = keySize;
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
			break;
		}
#ifdef MKAT
		case katja: {
			int modulusSize = keySize / 8; // Bytes
			err = katja_make_key( prngState, prngIndex, modulusSize, &pv->key.katjaKey );
			break;
		}
#endif
		default:
			ASSERT(false);
	}
	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err);

	bool verify;
	if ( JL_ARG_ISDEF(2) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &verify) );
	else
		verify = false;

	if ( pv->cipher == dsa && verify ) {

		int stat;
		err = dsa_verify_key(&pv->key.dsaKey, &stat);
		// JL_CHKM( err == CRYPT_OK && stat == 1, E_LIB, E_STR("libtomcrypt"), E_INTERNAL, E_COMMENT("dsa_verify_key") ); // If the result is stat = 1 the DSA key is valid (as far as valid mathematics are concerned).
		JL_ASSERT( err == CRYPT_OK && stat == 1, E_LIB, E_STR("libtomcrypt"), E_INTERNAL, E_SEP, E_STR("DSA"), E_STR("test"), E_FAILURE );
	}

	pv->hasKey = true;

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $DATA $INAME( data [, lparam] )
  This function returns the encrypted _data_ using a previously created or imported public key.
  $LF
  _data_ is the string to encrypt (usualy cipher keys).
  $H note
   This function may throw CRYPT_INVALID_HASH or CRYPT_PK_INVALID_SIZE if the data length is greater that the hash size (see blockLength property);
**/
DEFINE_FUNCTION( encrypt ) { // ( data [, lparam] )

	unsigned long outLength = 4096;
	uint8_t *out = NULL;
	JLData in;

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN( 1 );

	AsymmetricCipherPrivate *pv;
	pv = (AsymmetricCipherPrivate *)JL_GetPrivate( obj );
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	JL_ASSERT( pv->hasKey, E_NAME("key"), E_DEFINED );

	prng_state *prngState;
	int prngIndex;
	JL_CHK( SlotGetPrng(cx, obj, &prngIndex, &prngState) );

//	const char *in;
//	size_t inLength;
//	JL_CHK( JL_JsvalToStringAndLength( cx, &JL_ARG(1), &in, &inLength ) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &in) );
	
	out = JL_DataBufferAlloc(cx, outLength);
	JL_ASSERT_ALLOC(out);

	int err;
	err = -1; // default
	switch ( pv->cipher ) {
		case rsa: {
			// lparam doc: The lparam variable is an additional system specific tag that can be applied to the encoding.
			// This is useful to identify which system encoded the message. If no variance is desired then lparam can be set to NULL.
//			unsigned char *lparam = NULL; // default: lparam not used
//			unsigned long lparamlen = 0;
//			if ( argc >= 2 && !JSVAL_IS_VOID( JL_ARG(2) ) )
//				JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(2), &in, &inLength) );
			JLData lparam;
			if ( argc >= 2 && !JSVAL_IS_VOID( JL_ARG(2) ) )
				JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &lparam) );
			// doc. When performing v1.5 encryption, the hash and lparam parameters are totally ignored and can be set to NULL or zero (respectively).
			err = rsa_encrypt_key_ex( (const unsigned char *)in.GetConstStr(), (unsigned long)in.Length(), out, &outLength, (const unsigned char *)lparam.GetStrConstOrNull(), lparam.LengthOrZero(), prngState, prngIndex, pv->hashIndex, pv->padding, &pv->key.rsaKey ); // ltc_mp.rsa_me()
			break;
		}
		case ecc: {
			err = ecc_encrypt_key( (const unsigned char *)in.GetConstStr(), (unsigned long)in.Length(), out, &outLength, prngState, prngIndex, pv->hashIndex, &pv->key.eccKey );
			break;
		}
		case dsa: {
			// if inlen > hash_descriptor[hash].hashsize => ERROR
			err = dsa_encrypt_key( (const unsigned char *)in.GetConstStr(), (unsigned long)in.Length(), out, &outLength, prngState, prngIndex, pv->hashIndex, &pv->key.dsaKey );
			break;
		}
#ifdef MKAT
		case katja: {
			JLData lparam;
			if ( argc >= 2 && !JSVAL_IS_VOID( JL_ARG(2) ) )
				JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &lparam) );
			err = katja_encrypt_key( (const unsigned char *)in.GetConstStr(), (unsigned long)in.Length(), out, &outLength, (const unsigned char *)lparam.GetStrConstOrNull(), lparam.LengthOrZero(), prngState, prngIndex, pv->hashIndex, &pv->key.katjaKey );
			break;
		}
#endif
		default:
			ASSERT(false);
	}

	if ( err != CRYPT_OK )
		JL_CHK( ThrowCryptError(cx, err) );

	JL_CHK( JL_NewBufferGetOwnership(cx, out, outLength, JL_RVAL) );
	return JS_TRUE;

bad:
	zeromem(out, outLength); // safe clear
	JL_DataBufferFree(cx, out);
	return JS_FALSE;
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
DEFINE_FUNCTION( decrypt ) { // ( encryptedData [, lparam] )

	unsigned long outLength = 4096;
	uint8_t *out = NULL;
	JLData in;

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN( 1 );

	AsymmetricCipherPrivate *pv;
	pv = (AsymmetricCipherPrivate *)JL_GetPrivate( obj );
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	JL_ASSERT( pv->hasKey, E_NAME("key"), E_DEFINED );

//	const char *in;
//	size_t inLength;
//	JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(1), &in, &inLength) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &in) );

	out = JL_DataBufferAlloc(cx, outLength);
	JL_ASSERT_ALLOC(out);

	int err;
	err = -1; // default
	switch ( pv->cipher ) {
		case rsa: {

			JLData lparam;
			if ( argc >= 2 && !JSVAL_IS_VOID( JL_ARG(2) ) )
				JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &lparam) );

			int stat = 0; // default: failed
			err = rsa_decrypt_key_ex( (const unsigned char *)in.GetConstStr(), (unsigned long)in.Length(), out, &outLength, (const unsigned char *)lparam.GetStrConstOrNull(), lparam.LengthOrZero(), pv->hashIndex, pv->padding, &stat, &pv->key.rsaKey );
			// doc: if all went well pt == pt2, l2 == 16, res == 1
			if ( err == CRYPT_OK && stat != 1 ) {

				*JL_RVAL = JSVAL_VOID;
				return JS_TRUE;
			}
			break;
		}
		case ecc: {
			err = ecc_decrypt_key( (const unsigned char *)in.GetConstStr(), (unsigned long)in.Length(), out, &outLength, &pv->key.eccKey );
			break;
		}
		case dsa: {
			err = dsa_decrypt_key( (const unsigned char *)in.GetConstStr(), (unsigned long)in.Length(), out, &outLength, &pv->key.dsaKey );
			break;
		}
#ifdef MKAT
		case katja: {

			JLData lparam;
			if ( argc >= 2 && !JSVAL_IS_VOID( JL_ARG(2) ) )
				JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &lparam) );

			int stat = 0; // default: failed
			err = katja_decrypt_key( (const unsigned char *)in.GetConstStr(), (unsigned long)in.Length(), out, &outLength, (const unsigned char *)lparam.GetStrConstOrNull(), lparam.LengthOrZero(), pv->hashIndex, &stat, &pv->key.katjaKey );
			// doc: if all went well pt == pt2, l2 == 16, res == 1
			if ( err == CRYPT_OK && stat != 1 ) {

				*JL_RVAL = JSVAL_VOID;
				return JS_TRUE;
			}
			break;
		}
#endif
		default:
			ASSERT(false);
	}

	if ( err != CRYPT_OK )
		JL_CHK( ThrowCryptError(cx, err) );

	JL_CHK( JL_NewBufferGetOwnership(cx, out, outLength, JL_RVAL) );
	return JS_TRUE;

bad:
	zeromem(out, outLength); // safe clear
	JL_DataBufferFree(cx, out);
	return JS_FALSE;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( data [, saltLength] )
  This function returns the signature of the given _data_.
  Because this process is slow, this function usualy used to sign a small amount of data, like hash digest.
  $LF
  _saltLength_ is only used with RSA signatures. (default value is 16)
**/
DEFINE_FUNCTION( sign ) { // ( data [, saltLength] )

	unsigned long outLength = 4096;
	uint8_t *out = NULL;
	JLData in;

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN( 1 );

	AsymmetricCipherPrivate *pv;
	pv = (AsymmetricCipherPrivate *)JL_GetPrivate( obj );
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	JL_ASSERT( pv->hasKey, E_NAME("key"), E_DEFINED );

	prng_state *prngState;
	int prngIndex;
	JL_CHK( SlotGetPrng(cx, obj, &prngIndex, &prngState) );

//	const char *in;
//	size_t inLength;
//	JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(1), &in, &inLength) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &in) );

	out = JL_DataBufferAlloc(cx, outLength);
	JL_ASSERT_ALLOC(out);

	int err;
	err = -1; // default
	switch ( pv->cipher ) {
		case rsa: {

			// A good saltLength default value is between 8 and 16 octets. Strictly, it must be small than modulus len - hLen - 2 where modulus len is the size of the RSA modulus (in octets), and hLen is the length of the message digest produced by the chosen hash.
			// int saltLength = 16; // OR saltLength = mp_unsigned_bin_size((mp_int*)(pv->key.rsaKey.N)) - hash_descriptor[hashIndex].hashsize - 2  -1;
			int saltLength = RSA_SIGN_DEFAULT_SALT_LENGTH;
			if ( argc >= 2 && !JSVAL_IS_VOID( JL_ARG(2) ) )
				JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &saltLength) );

			err = rsa_sign_hash_ex( (const unsigned char *)in.GetConstStr(), (unsigned long)in.Length(), out, &outLength, LTC_LTC_PKCS_1_PSS, prngState, prngIndex, pv->hashIndex, saltLength, &pv->key.rsaKey );
			break;
		}
		case ecc: {
			err = ecc_sign_hash( (const unsigned char *)in.GetConstStr(), (unsigned long)in.Length(), out, &outLength, prngState, prngIndex, &pv->key.eccKey );
			break;
		}
		case dsa: {
			err = dsa_sign_hash( (const unsigned char *)in.GetConstStr(), (unsigned long)in.Length(), out, &outLength, prngState, prngIndex, &pv->key.dsaKey );
			break;
		}
#ifdef MKAT
		case katja: {
			JL_ERR( E_THISOPERATION, E_NOTIMPLEMENTED );
			break;
		}
#endif
		default:
			ASSERT(false);
	}

	if ( err != CRYPT_OK )
		JL_CHK( ThrowCryptError(cx, err) );

	JL_CHK( JL_NewBufferGetOwnership(cx, out, outLength, JL_RVAL) );
	return JS_TRUE;

bad:
	zeromem(out, outLength); // safe clear
	JL_DataBufferFree(cx, out);
	return JS_FALSE;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME( data, signature [, saltLength] )
  This function returns $TRUE if the _data_ match the data used to create the _signature_.
  $LF
  _saltLength_ is only used with RSA signatures. (default value is 16)
**/
DEFINE_FUNCTION( verifySignature ) { // ( data, signature [, saltLength] )

	JLData data, sign;

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN( 2 );

	AsymmetricCipherPrivate *pv;
	pv = (AsymmetricCipherPrivate *)JL_GetPrivate( obj );
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	JL_ASSERT( pv->hasKey, E_NAME("key"), E_DEFINED );

//	const char *data;
//	size_t dataLength;
//	JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(1), &data, &dataLength ) ); // warning: GC on the returned buffer !
//	const char *sign;
//	size_t signLength;
//	JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(2), &sign, &signLength) ); // warning: GC on the returned buffer !
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &data) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &sign) );

	int stat;
	stat = 0; // default: failed
	int err;
	err = -1; // default: Invalid error code
	switch ( pv->cipher ) {
		case rsa: {
			int saltLength = RSA_SIGN_DEFAULT_SALT_LENGTH; // default
			if ( argc >= 3 && !JSVAL_IS_VOID( JL_ARG(3) ) )
				JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &saltLength) );

			rsa_verify_hash_ex( (const unsigned char *)sign.GetConstStr(), (unsigned long)sign.Length(), (const unsigned char *)data.GetConstStr(), (unsigned long)data.Length(), LTC_LTC_PKCS_1_PSS, pv->hashIndex, saltLength, &stat, &pv->key.rsaKey );
			break;
		}
		case ecc: {
			ecc_verify_hash( (const unsigned char *)sign.GetConstStr(), (unsigned long)sign.Length(), (const unsigned char *)data.GetConstStr(), (unsigned long)data.Length(), &stat, &pv->key.eccKey );
			break;
		}
		case dsa: {
			dsa_verify_hash( (const unsigned char *)sign.GetConstStr(), (unsigned long)sign.Length(), (const unsigned char *)data.GetConstStr(), (unsigned long)data.Length(), &stat, &pv->key.dsaKey );
			break;
		}
#ifdef MKAT
		case katja: {
			JL_ERR( E_THISOPERATION, E_NOTIMPLEMENTED );
			break;
		}
#endif
		default:
			ASSERT(false);
	}

	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err);

	*JL_RVAL = BOOLEAN_TO_JSVAL( stat == 1 );
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
DEFINE_PROPERTY_GETTER( blockLength ) {

	JL_IGNORE(id);

	JL_ASSERT_THIS_INSTANCE();
	AsymmetricCipherPrivate *pv;
	pv = (AsymmetricCipherPrivate *)JL_GetPrivate( obj );
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	JL_ASSERT( pv->hasKey, E_NAME("key"), E_DEFINED );

	int err;
	err = hash_is_valid(pv->hashIndex);
	if ( err != CRYPT_OK )
		return ThrowCryptError(cx, err);
	ltc_hash_descriptor *hashDescriptor = &hash_descriptor[pv->hashIndex];
	int blockLength;
	switch ( pv->cipher ) {
		case rsa:
			// blockLength = mp_unsigned_bin_size((mp_int*)(pv->key.rsaKey.N)) - 2 * hashDescriptor->hashsize - 2;
			blockLength = ltc_mp.unsigned_size(pv->key.rsaKey.N) - 2 * hashDescriptor->hashsize - 2; // (TBD) not the same rule if hash is not given (LTC_LTC_PKCS_1_V1_5 case)
			break;
		case ecc:
			blockLength = hashDescriptor->hashsize;
			break;
		case dsa:
			blockLength = hashDescriptor->hashsize;
			break;
#ifdef MKAT
		case katja: {

			// see katja_encrypt_key() and pkcs_1_oaep_encode() for details:
			unsigned long modulus_bitlen, modulus_len, hLen;
			modulus_bitlen = ltc_mp.count_bits(pv->key.katjaKey.N);
			modulus_bitlen = ((modulus_bitlen << 1) / 3);
			modulus_bitlen -= (modulus_bitlen & 7) + 8;
			modulus_len = (modulus_bitlen >> 3) + (modulus_bitlen & 7 ? 1 : 0);
			hLen = hashDescriptor->hashsize;
			blockLength = modulus_bitlen - 2*hLen - 2;
			//bool tmp = (2*hLen >= (modulus_len - 2));
			break;
		}
#endif
		default:
			IFDEBUG( blockLength = 0 );
			ASSERT(false);
	}
	*vp = INT_TO_JSVAL( blockLength );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  is the size of the current private key.
**/
DEFINE_PROPERTY_GETTER( keySize ) {

	JL_IGNORE(id);

	JL_ASSERT_THIS_INSTANCE();
	AsymmetricCipherPrivate *pv;
	pv = (AsymmetricCipherPrivate *)JL_GetPrivate( obj );
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	JL_ASSERT( pv->hasKey, E_NAME("key"), E_DEFINED );

	int keySize;
	switch ( pv->cipher ) {
		case rsa:
			keySize = ltc_mp.count_bits(pv->key.rsaKey.N);
			break;
		case ecc:
//			keySize = ltc_mp.count_bits(pv->key.eccKey.pubkey.x);
			keySize = ecc_get_size(&pv->key.eccKey); // doc. returns INT_MAX on error
			break;
		case dsa:
			keySize = ltc_mp.count_bits(pv->key.dsaKey.q) / 2; // or .dsaKey.x ???
			break;
#ifdef MKAT
		case katja:
			keySize = ltc_mp.count_bits(pv->key.katjaKey.N);
			break;
#endif
		default:
			IFDEBUG( keySize = 0 );
			ASSERT(false);
	}
	*vp = INT_TO_JSVAL( keySize );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR *privateKey*
  The private key.

 * $STR *publicKey*
  The public key.
**/

DEFINE_PROPERTY_SETTER( key ) {
	
	JL_IGNORE(strict);

	JLData key;

	JL_ASSERT_THIS_INSTANCE();
	AsymmetricCipherPrivate *pv;
	pv = (AsymmetricCipherPrivate *)JL_GetPrivate( obj );
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	int type;
	type = JSID_TO_INT(id);

//	const char *key;
//	size_t keyLength;
//	JL_CHK( JL_JsvalToStringAndLength(cx, vp, &key, &keyLength) );
	JL_CHK( JL_JsvalToNative(cx, *vp, &key) );

	int err;
	err = -1; // default
	switch ( pv->cipher ) {
		case rsa:
			err = rsa_import( (const unsigned char *)key.GetConstStr(), (unsigned long)key.Length(), &pv->key.rsaKey );
			JL_ASSERT( pv->key.rsaKey.type == type, E_VALUE, E_TYPE, E_NAME("RSA key") );
			break;
		case ecc:
			err = ecc_import( (const unsigned char *)key.GetConstStr(), (unsigned long)key.Length(), &pv->key.eccKey );
			JL_ASSERT( pv->key.eccKey.type == type, E_VALUE, E_TYPE, E_NAME("ECC key") );
			break;
		case dsa:
			err = dsa_import( (const unsigned char *)key.GetConstStr(), (unsigned long)key.Length(), &pv->key.dsaKey );
			JL_ASSERT( pv->key.dsaKey.type == type, E_VALUE, E_TYPE, E_NAME("DSA key") );
			//int stat = 0;
			//dsa_verify_key(&pv->key.dsaKey, &stat);
			//if (stat != 1) // If the result is stat = 1 the DSA key is valid (as far as valid mathematics are concerned).
			//	JL_REPORT_ERROR("Invalid key.");
			break;
#ifdef MKAT
		case katja:
			err = katja_import( (const unsigned char *)key.GetConstStr(), (unsigned long)key.Length(), &pv->key.katjaKey );
			JL_ASSERT( pv->key.katjaKey.type == type, E_VALUE, E_TYPE, E_NAME("KATJA key") );
			break;
#endif
		default:
			ASSERT(false);
	}
	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err);

	pv->hasKey = true;
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( key ) {

	unsigned long keyLength = 4096;
	uint8_t *key = NULL;

	JL_ASSERT_THIS_INSTANCE();
	AsymmetricCipherPrivate *pv;
	pv = (AsymmetricCipherPrivate *)JL_GetPrivate( obj );
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	JL_ASSERT( pv->hasKey, E_NAME("key"), E_DEFINED );

	int type;
	type = JSID_TO_INT(id);

	key = JL_DataBufferAlloc(cx, keyLength);

	int err;
	err = -1; // default
	switch ( pv->cipher ) {
		case rsa:
			err = rsa_export( key, &keyLength, type, &pv->key.rsaKey );
			break;
		case ecc:
			err = ecc_export( key, &keyLength, type, &pv->key.eccKey );
			break;
		case dsa:
			err = dsa_export( key, &keyLength, type, &pv->key.dsaKey );
			break;
#ifdef MKAT
		case katja:
			err = katja_export( key, &keyLength, type, &pv->key.katjaKey );
			break;
#endif
		default:
			ASSERT(false);
	}

	if ( err != CRYPT_OK )
		JL_CHK( ThrowCryptError(cx, err) );

	JL_CHK( JL_NewBufferGetOwnership(cx, key, keyLength, vp) );
	return JS_TRUE;

bad:
	zeromem(key, keyLength); // safe clear
	JL_DataBufferFree(cx, key);
	return JS_FALSE;
}


enum {
	publicKey = PK_PUBLIC,
	privateKey = PK_PRIVATE,
};


CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision$"))

	HAS_PRIVATE
	HAS_RESERVED_SLOTS( 1 )

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION( wipe )
		FUNCTION( createKeys )
		FUNCTION( encrypt )
		FUNCTION( decrypt )
		FUNCTION( sign )
		FUNCTION( verifySignature )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_GETTER( blockLength )
		PROPERTY_GETTER( keySize )
		PROPERTY_SWITCH( privateKey, key )
		PROPERTY_SWITCH( publicKey, key )
	END_PROPERTY_SPEC

	BEGIN_CONST
		#ifdef LTC_MRSA
		CONST_INTEGER(RSA_MIN_KEYSIZE, MIN_RSA_SIZE)
		CONST_INTEGER(RSA_MAX_KEYSIZE, MAX_RSA_SIZE)
		#endif
		#ifdef MKAT
		CONST_INTEGER(KAT_MIN_KEYSIZE, MIN_KAT_SIZE)
		CONST_INTEGER(KAT_MAX_KEYSIZE, MAX_KAT_SIZE)
		#endif
		#ifdef LTC_MECC
		CONST_INTEGER(ECC_MIN_KEYSIZE, 0)
		CONST_INTEGER(ECC_MAX_KEYSIZE, ECC_MAXSIZE)
		#endif
		#ifdef LTC_MDSA
		CONST_INTEGER(DSA_MIN_KEYSIZE, 0)
		CONST_INTEGER(DSA_MAX_KEYSIZE, LTC_MDSA_MAX_GROUP * 4) // see CreateKeys(): groupSize = keySize / 4;
		#endif
	END_CONST

END_CLASS

/**doc
=== Example ===
Data (or key) encryption using RSA:
{{{
loadModule('jsstd');
loadModule('jscrypt');
var fortuna = new Prng('fortuna');
fortuna.autoEntropy(123); // give more entropy

//Alice
var alice = new AsymmetricCipher('RSA', 'md5', fortuna);
alice.createKeys(1024);
var publicKey = alice.publicKey;

//Bob
var bob = new AsymmetricCipher('RSA', 'md5', fortuna);
bob.publicKey = publicKey;
var encryptedData = bob.encrypt('Alice, I love you !');

//Alice
print( alice.decrypt(encryptedData), '\n' );
}}}
**/
