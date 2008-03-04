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
	RT_CHECK_CALL( JS_GetReservedSlot(cx, obj, ASYMMETRIC_CIPHER_PRNG_SLOT, &prngVal) );
	RT_ASSERT_OBJECT(	prngVal );
	RT_ASSERT_CLASS( JSVAL_TO_OBJECT(prngVal), &classPrng );
	PrngPrivate *prngPrivate = (PrngPrivate *)JS_GetPrivate(cx, JSVAL_TO_OBJECT(prngVal));
	RT_ASSERT_RESOURCE( prngPrivate );
	*prngState = &prngPrivate->state;
	*prngIndex = find_prng(prngPrivate->prng.name);
	RT_ASSERT_1( *prngIndex != -1, "prng %s is not available.", prngPrivate->prng.name );
	return JS_TRUE;
}


BEGIN_CLASS( AsymmetricCipher )


DEFINE_FINALIZE() {

	AsymmetricCipherPrivate *pv = (AsymmetricCipherPrivate *)JS_GetPrivate(cx, obj);
	if ( pv == NULL )
		return;
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
	zeromem(pv, sizeof(AsymmetricCipherPrivate)); // safe clean
	free(pv);
}


DEFINE_CONSTRUCTOR() { // ( cipherName, hashName [, prngObject] [, PKCSVersion] )

	RT_ASSERT_CONSTRUCTING(_class)
	RT_ASSERT_ARGC( 3 );

	char *asymmetricCipherName;
	RT_JSVAL_TO_STRING( argv[0], asymmetricCipherName );

	AsymmetricCipher asymmetricCipher;
	if ( strcasecmp( asymmetricCipherName, "RSA" ) == 0 )
		asymmetricCipher = rsa;
	else if ( strcasecmp( asymmetricCipherName, "ECC" ) == 0 )
		asymmetricCipher = ecc;
	else if ( strcasecmp( asymmetricCipherName, "DSA" ) == 0 )
		asymmetricCipher = dsa;
	else
		REPORT_ERROR_1("Invalid asymmetric cipher %s.", asymmetricCipherName);

	AsymmetricCipherPrivate *pv = (AsymmetricCipherPrivate *)malloc( sizeof(AsymmetricCipherPrivate) );
	RT_ASSERT_ALLOC( pv );

	pv->cipher = asymmetricCipher;

	char *hashName;
	RT_JSVAL_TO_STRING( argv[1], hashName );
	int hashIndex = find_hash(hashName);
	RT_ASSERT_1( hashIndex != -1, "hash %s is not available.", hashName );
	pv->hashDescriptor = &hash_descriptor[hashIndex];

	if ( argc >= 3 ) {

		RT_ASSERT_OBJECT(	argv[2] );
		RT_ASSERT_CLASS( JSVAL_TO_OBJECT(argv[2]), &classPrng );
		RT_CHECK_CALL( JS_SetReservedSlot(cx, obj, ASYMMETRIC_CIPHER_PRNG_SLOT, argv[2]) );
	} else {

		RT_CHECK_CALL( JS_SetReservedSlot(cx, obj, ASYMMETRIC_CIPHER_PRNG_SLOT, JSVAL_VOID) );
	}

	pv->padding = LTC_LTC_PKCS_1_OAEP;

	if ( argc >= 4 && argv[3] != JSVAL_VOID ) {

		char *paddingName;
		RT_JSVAL_TO_STRING(argv[3], paddingName);

		if ( strcmp(paddingName, "1_OAEP") == 0 ) {

			pv->padding = LTC_LTC_PKCS_1_OAEP;
		} else if ( strcmp(paddingName, "1_V1_5") == 0 ) {

			pv->padding = LTC_LTC_PKCS_1_V1_5;
		} else
			REPORT_ERROR("Invalid padding version.");
	}

	pv->hasKey = false;
	JS_SetPrivate( cx, obj, pv );
	return JS_TRUE;
}


DEFINE_FUNCTION( CreateKeys ) { // ( bitsSize )

	RT_ASSERT_ARGC( 1 );
	RT_ASSERT_CLASS( obj, _class );
	AsymmetricCipherPrivate *pv = (AsymmetricCipherPrivate *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE( pv );

	prng_state *prngState;
	int prngIndex;
	RT_CHECK_CALL( SlotGetPrng(cx, obj, &prngIndex, &prngState) );

	int32 keySize;
	RT_JSVAL_TO_INT32( argv[0], keySize );

	int err = -1; // default
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
			// Max diff between group and modulus size in bytes: 512
			// Max DSA group size in bytes (default allows 4k-bit groups): 512
			int groupSize = keySize/4; // Bytes
			int modulusSize = 1<<(keySize / 40 + 5); // Bytes
			err = dsa_make_key( prngState, prngIndex, groupSize, modulusSize, &pv->key.dsaKey );
			//int stat = 0; // default: failed
			//dsa_verify_key(&pv->key.dsaKey, &stat);
			//if (stat != 1) { // If the result is stat = 1 the DSA key is valid (as far as valid mathematics are concerned).
			//}
			break;			 
		}
		default:
			REPORT_ERROR("Invalid case.");
	}
	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err); // (TBD) free something ? 

	pv->hasKey = true;
	return JS_TRUE;
}



DEFINE_FUNCTION( Encrypt ) { // ( data [, lparam] )

	RT_ASSERT_ARGC( 1 );
	RT_ASSERT_CLASS( obj, _class );
	AsymmetricCipherPrivate *pv = (AsymmetricCipherPrivate *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( pv );
	RT_ASSERT( pv->hasKey, "No key found." );

	prng_state *prngState;
	int prngIndex;
	RT_CHECK_CALL( SlotGetPrng(cx, obj, &prngIndex, &prngState) );

	int hashIndex = find_hash(pv->hashDescriptor->name);

	char *in;
	int inLength;
	RT_JSVAL_TO_STRING_AND_LENGTH( argv[0], in, inLength );

	char out[4096];
	unsigned long outLength = sizeof(out);
	int err = -1; // default
	switch ( pv->cipher ) {
		case rsa: {
			// lparam doc: The lparam variable is an additional system specific tag that can be applied to the encoding.
			// This is useful to identify which system encoded the message. If no variance is desired then lparam can be set to NULL.
			unsigned char *lparam = NULL; // default: lparam not used
			unsigned long lparamlen = 0;
			if (argc >= 2 && argv[1] != JSVAL_VOID)
				RT_JSVAL_TO_STRING_AND_LENGTH(argv[1], in, inLength);
			err = rsa_encrypt_key_ex( (unsigned char *)in, inLength, (unsigned char *)out, &outLength, lparam, lparamlen, prngState, prngIndex, hashIndex, pv->padding, &pv->key.rsaKey ); // ltc_mp.rsa_me()
			break;			 
		}
		case ecc: {
			RT_ASSERT_ALLOC( out );
			err = ecc_encrypt_key( (unsigned char *)in, inLength, (unsigned char *)out, &outLength, prngState, prngIndex, hashIndex, &pv->key.eccKey );
			break;			 
		}
		case dsa: {
			err = dsa_encrypt_key( (unsigned char *)in, inLength, (unsigned char *)out, &outLength, prngState, prngIndex, hashIndex, &pv->key.dsaKey );
			break;			 
		}
		default:
			REPORT_ERROR("Invalid case.");
	}
	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err); // (TBD) free something ?
	JSString *jssOut = JS_NewStringCopyN(cx, out, outLength);
	zeromem(out, sizeof(out)); // safe clear
	RT_ASSERT_ALLOC( jssOut );
	*rval = STRING_TO_JSVAL( jssOut );
	return JS_TRUE;
}


DEFINE_FUNCTION( Decrypt ) { // ( encryptedData [, lparam] )

	RT_ASSERT_ARGC( 1 );
	RT_ASSERT_CLASS( obj, _class );
	AsymmetricCipherPrivate *pv = (AsymmetricCipherPrivate *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( pv );
	RT_ASSERT( pv->hasKey, "No key found." );

	char *in;
	int inLength;
	RT_JSVAL_TO_STRING_AND_LENGTH(argv[0], in, inLength);

	char out[4096];
	unsigned long outLength = sizeof(out);

	int err = -1; // default
	switch ( pv->cipher ) {
		case rsa: {
			
			int hashIndex = find_hash(pv->hashDescriptor->name);

			unsigned char *lparam = NULL; // default: lparam not used
			unsigned long lparamlen = 0;
			if (argc >= 2 && argv[1] != JSVAL_VOID)
				RT_JSVAL_TO_STRING_AND_LENGTH( argv[1], in, inLength );

			int stat = 0; // default: failed
			err = rsa_decrypt_key_ex( (unsigned char *)in, inLength, (unsigned char *)out, &outLength, lparam, lparamlen, hashIndex, pv->padding, &stat, &pv->key.rsaKey );
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
			REPORT_ERROR("Invalid case.");
	}
		
	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err); // (TBD) free something ?

	out[outLength] = '\0';
	JSString *jssOut = JS_NewStringCopyN( cx, out, outLength +1 );
	zeromem(out, sizeof(out));
	RT_ASSERT_ALLOC( jssOut );
	*rval = STRING_TO_JSVAL(jssOut);
	return JS_TRUE;
}


DEFINE_FUNCTION( Sign ) { // ( data [, saltLength] )

	RT_ASSERT_ARGC( 1 );
	RT_ASSERT_CLASS( obj, _class );
	AsymmetricCipherPrivate *pv = (AsymmetricCipherPrivate *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( pv );

	RT_ASSERT( pv->hasKey, "No key found." );

	prng_state *prngState;
	int prngIndex;
	RT_CHECK_CALL( SlotGetPrng(cx, obj, &prngIndex, &prngState) );

	char *in;
	int inLength;
	RT_JSVAL_TO_STRING_AND_LENGTH( argv[0], in, inLength );

	char out[4096];
	unsigned long outLength = sizeof(out);

	int err = -1; // default
	switch ( pv->cipher ) {
		case rsa: {
			int hashIndex = find_hash(pv->hashDescriptor->name);
			// A good saltLength default value is between 8 and 16 octets. Strictly, it must be small than modulus len - hLen - 2 where modulus len is the size of the RSA modulus (in octets), and hLen is the length of the message digest produced by the chosen hash.
			// int saltLength = 16; // OR saltLength = mp_unsigned_bin_size((mp_int*)(pv->key.rsaKey.N)) - hash_descriptor[hashIndex].hashsize - 2  -1;
			int saltLength = RSA_SIGN_DEFAULT_SALT_LENGTH;
			if ( argc >= 2 && argv[1] != JSVAL_VOID )
				RT_JSVAL_TO_INT32(argv[1], saltLength);

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
			REPORT_ERROR("Invalid case.");
	}

	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err); // (TBD) free something ?
	JSString *jssOut = JS_NewStringCopyN( cx, out, outLength );
	zeromem(out, sizeof(out));
	RT_ASSERT_ALLOC( jssOut );
	*rval = STRING_TO_JSVAL(jssOut);
	return JS_TRUE;
}


DEFINE_FUNCTION( VerifySignature ) { // ( data, signature [, saltLength] )

	RT_ASSERT_ARGC( 2 );
	RT_ASSERT_CLASS( obj, _class );
	AsymmetricCipherPrivate *pv = (AsymmetricCipherPrivate *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( pv );
	RT_ASSERT( pv->hasKey, "No key found." );

	char *data;
	int dataLength;
	RT_JSVAL_TO_STRING_AND_LENGTH( argv[0], data, dataLength );

	char *sign;
	int signLength;
	RT_JSVAL_TO_STRING_AND_LENGTH( argv[1], sign, signLength );

	int stat = 0; // default: failed
	int err = -1; // default: Invalid error code
	switch ( pv->cipher ) {
		case rsa: {
			int saltLength = RSA_SIGN_DEFAULT_SALT_LENGTH; // default
			if ( argc >= 3 && argv[2] != JSVAL_VOID )
				RT_JSVAL_TO_INT32(argv[2], saltLength);

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
			REPORT_ERROR("Invalid case.");
	}

	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err); // (TBD) free something ?

	*rval = STRING_TO_JSVAL( stat == 1 ? JSVAL_TRUE : JSVAL_FALSE );
	return JS_TRUE;
}


DEFINE_PROPERTY( blockLength ) {

	RT_ASSERT_CLASS( obj, _class );
	AsymmetricCipherPrivate *pv = (AsymmetricCipherPrivate *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( pv );
	RT_ASSERT( pv->hasKey, "No key found." );
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
			REPORT_ERROR("Invalid case.");
	}
	*vp = INT_TO_JSVAL( blockLength );
	return JS_TRUE;
}


DEFINE_PROPERTY( keySize ) {

	RT_ASSERT_CLASS( obj, _class );
	AsymmetricCipherPrivate *pv = (AsymmetricCipherPrivate *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( pv );
	RT_ASSERT( pv->hasKey, "No key found." );
	int keySize;
	switch ( pv->cipher ) {
		case rsa:
			keySize = mp_unsigned_bin_size((mp_int*)(pv->key.rsaKey.N)) * 8; // Ok !
			break;
		case ecc:
			keySize = mp_unsigned_bin_size((mp_int*)(pv->key.eccKey.pubkey.x)) * 8; // Ok !
			break;
		case dsa:
			keySize = mp_unsigned_bin_size((mp_int*)(pv->key.dsaKey.y)) * 8; // ???
			break;
		default:
			REPORT_ERROR("Invalid case.");
	}
	*vp = INT_TO_JSVAL( keySize );
	return JS_TRUE;
}


DEFINE_PROPERTY( keySetter ) {

	RT_ASSERT_CLASS( obj, _class );
	AsymmetricCipherPrivate *pv = (AsymmetricCipherPrivate *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( pv );

	char *key;
	int keyLength;
	RT_JSVAL_TO_STRING_AND_LENGTH( *vp, key, keyLength );

	int type;
	RT_JSVAL_TO_INT32( id, type );

	int err = -1; // default
	switch ( pv->cipher ) {
		case rsa:
			err = rsa_import( (unsigned char *)key, keyLength, &pv->key.rsaKey );
			RT_ASSERT( pv->key.rsaKey.type == type, "Invalid key type." );
			break;
		case ecc:
			err = ecc_import( (unsigned char *)key, keyLength, &pv->key.eccKey );
			RT_ASSERT( pv->key.rsaKey.type == type, "Invalid key type." );
			break;
		case dsa:
			err = dsa_import( (unsigned char *)key, keyLength, &pv->key.dsaKey );
			RT_ASSERT( pv->key.rsaKey.type == type, "Invalid key type." );
			//dsa_verify_key(
			break;
		default:
			REPORT_ERROR("Invalid case.");
	}
	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err); // (TBD) free something ?

	pv->hasKey = true;
	return JS_TRUE;
}


DEFINE_PROPERTY( keyGetter ) {

	RT_ASSERT_CLASS( obj, _class );
	AsymmetricCipherPrivate *pv = (AsymmetricCipherPrivate *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( pv );
	RT_ASSERT( pv->hasKey, "No key found." );

	JSBool jsErr;
	int32 type;
	jsErr = JS_ValueToInt32(cx, id, &type);
	RT_ASSERT( jsErr == JS_TRUE, "Invalid operation." );

	char key[4096];
	unsigned long keyLength = sizeof(key);

	int err = -1; // default
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
			REPORT_ERROR("Invalid case.");
	}
	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err); // (TBD) free something ?

	JSString *jssKey = JS_NewStringCopyN(cx, key, keyLength);
	RT_ASSERT( jssKey != NULL, "unable to create the key string." );
	*vp = STRING_TO_JSVAL(jssKey);
	zeromem(key, sizeof(key)); // safe clean
	return JS_TRUE;
}


enum {
	publicKey = PK_PUBLIC,
	privateKey = PK_PRIVATE,
};


CONFIGURE_CLASS

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
