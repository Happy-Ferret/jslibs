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

#include "rsa.h"
#include "prng.h"
#include "hash.h"

#include <tommath.h>

struct RsaPrivate {
	rsa_key key;
};

BEGIN_CLASS( Rsa )


DEFINE_FINALIZE() {

	RsaPrivate *rsaPrivate = (RsaPrivate *)JS_GetPrivate( cx, obj );
	if ( rsaPrivate == NULL )
		return;
	rsa_free(&rsaPrivate->key);
	free(rsaPrivate);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// prng, keysize
DEFINE_CONSTRUCTOR() {

	RT_ASSERT( JS_IsConstructing(cx), RT_ERROR_NEED_CONSTRUCTION );
	RsaPrivate *rsaPrivate = (RsaPrivate *)malloc( sizeof(RsaPrivate) );
	RT_ASSERT( rsaPrivate != NULL, RT_ERROR_OUT_OF_MEMORY );
	JS_SetPrivate( cx, obj, rsaPrivate );
	return JS_TRUE;
}

DEFINE_FUNCTION( CreateKeys ) {

	RT_ASSERT_ARGC( 2 );
	RT_ASSERT_CLASS( obj, _class );

	JSObject *objPrng JSVAL_TO_OBJECT(argv[0]);
	RT_ASSERT_CLASS( objPrng, &classPrng );

	PrngPrivate *prngPrivate = (PrngPrivate *)JS_GetPrivate( cx, objPrng );
	RT_ASSERT( prngPrivate != NULL, "invalid prng." );

	int32 keySize;
	RT_JSVAL_TO_INT32( argv[1], keySize );

	int prngIndex = find_prng(prngPrivate->prng.name);
	RT_ASSERT_1( prngIndex != -1, "prng %s is not available", prngPrivate->prng.name );

	RsaPrivate *rsaPrivate = (RsaPrivate *)JS_GetPrivate(cx, obj);

	int e = 65537; // typical values are 3, 17, 257 and 65537
	int err = rsa_make_key( &prngPrivate->state, prngIndex, keySize/8, e, &rsaPrivate->key );
	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err); // (TBD) should free rsaPrivate or Finalize is called ?

	return JS_TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DEFINE_FUNCTION( Encrypt ) {

	RT_ASSERT_ARGC( 3 );

	RT_ASSERT_CLASS( obj, _class );
	RsaPrivate *privateData = (RsaPrivate *)JS_GetPrivate( cx, obj );

	prng_state *prngState;
	int prngIndex;

	if ( JSVAL_IS_OBJECT(argv[0]) && JS_GET_CLASS(cx, JSVAL_TO_OBJECT(argv[0])) == &classPrng ) {

		JSObject *objPrng = JSVAL_TO_OBJECT(argv[0]);
		PrngPrivate *prngPrivate = (PrngPrivate *)JS_GetPrivate(cx, objPrng);
		RT_ASSERT_RESOURCE( prngPrivate );
		prngIndex = find_prng(prngPrivate->prng.name); // needed !
		prngState = &prngPrivate->state;
	} else {

		char *prngName;
		RT_JSVAL_TO_STRING( argv[0], prngName );
		prngIndex = find_prng(prngName);
		RT_ASSERT_1( prngIndex != -1, "prng %s is not available", prngName );
		prngState = NULL;
	}

	char *hashName;
	RT_JSVAL_TO_STRING( argv[1], hashName );
	int hashIndex = find_hash(hashName);
	RT_ASSERT_1( hashIndex != -1, "hash %s is not available", hashName );


	char *in;
	int inLength;
	RT_JSVAL_TO_STRING_AND_LENGTH( argv[2], in, inLength );

	unsigned long outLength = inLength + mp_unsigned_bin_size((mp_int*)privateData->key.N) - hash_descriptor[hashIndex].hashsize - 2; // length = message + (modulus length - 2 * hash size - 2)

	char *out = (char *)JS_malloc( cx, outLength );
	RT_ASSERT_ALLOC( out );

	// lparam doc: The lparam variable is an additional system specific tag that can be applied to the encoding.
	// This is useful to identify which system encoded the message. If no variance is desired then lparam can be set to NULL.
	unsigned char *lparam = NULL; 
	unsigned long lparamlen = 0;

	int err;
	err = rsa_encrypt_key( (const unsigned char *)in, inLength, (unsigned char *)out, &outLength, lparam, lparamlen, prngState, prngIndex, hashIndex, &privateData->key );
	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err); // (TBD) free rsaPrivate ?

	JSString *jssOut = JS_NewString( cx, out, outLength );
	RT_ASSERT_ALLOC( jssOut );
	*rval = STRING_TO_JSVAL( jssOut );

	return JS_TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DEFINE_FUNCTION( Decrypt ) {

	RT_ASSERT_ARGC( 2 );

	RT_ASSERT_CLASS( obj, _class );
	RsaPrivate *privateData = (RsaPrivate *)JS_GetPrivate( cx, obj );

	char *hashName;
	RT_JSVAL_TO_STRING( argv[0], hashName );
	int hashIndex = find_hash(hashName);
	RT_ASSERT_1( hashIndex != -1, "hash %s is not available", hashName );

	char *in;
	int inLength;
	RT_JSVAL_TO_STRING_AND_LENGTH( argv[1], in, inLength );

//	int modulusSize = mp_unsigned_bin_size((mp_int*)privateData->key.N);
//	unsigned long outLength = inLength - ( modulusSize - hash_descriptor[hashIndex].hashsize - 2 );
//	if ( outLength < modulusSize )
//		outLength = modulusSize;

	unsigned long outLength = inLength;

	char *out = (char *)JS_malloc( cx, outLength +1 );
	out[outLength] = '\0';
	RT_ASSERT_ALLOC( out );

	unsigned char *lparam = NULL;
	unsigned long lparamlen = 0;

	int res; // validity of data

	int err;
	err = rsa_decrypt_key( (const unsigned char *)in, inLength, (unsigned char *)out, &outLength, lparam, lparamlen, hashIndex, &res, &privateData->key );
	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err); // (TBD) free rsaPrivate ?

	// doc: if all went well pt == pt2, l2 == 16, res == 1
	if ( res != 1 ) {
		
		JS_free(cx, out);
		*rval = JSVAL_VOID;
		return JS_TRUE;
	}

	JSString *jssOut = JS_NewString( cx, out, outLength );
	RT_ASSERT( jssOut != NULL, "unable to create the string." );
	*rval = STRING_TO_JSVAL(jssOut);

	return JS_TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DEFINE_PROPERTY( keySetter ) {

	RT_ASSERT_CLASS( obj, _class );
	RsaPrivate *privateData = (RsaPrivate *)JS_GetPrivate( cx, obj );
	RT_ASSERT( privateData != NULL, RT_ERROR_NOT_INITIALIZED );

	char *key;
	int keyLength;
	RT_JSVAL_TO_STRING_AND_LENGTH( *vp, key, keyLength );

	int err;
	if ( (err=rsa_import( (const unsigned char *)key, keyLength, &privateData->key )) != CRYPT_OK )
		return ThrowCryptError(cx, err); // (TBD) free rsaPrivate ?

	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DEFINE_PROPERTY( keyGetter ) {

	RT_ASSERT_CLASS( obj, _class );
	RsaPrivate *privateData = (RsaPrivate *)JS_GetPrivate( cx, obj );
	RT_ASSERT( privateData != NULL, RT_ERROR_NOT_INITIALIZED );

	JSBool jsErr;
	int32 type;
	jsErr = JS_ValueToInt32( cx, id, &type );
	RT_ASSERT( jsErr == JS_TRUE, "unable to get what to do." );

	char key[4096];
	unsigned long keyLength = sizeof(key);

	int err;
	if ( (err=rsa_export( (unsigned char *)key, &keyLength, type, &privateData->key )) != CRYPT_OK )
		return ThrowCryptError(cx, err); // (TBD) free rsaPrivate ?

	JSString *jssKey = JS_NewStringCopyN( cx, key, keyLength );
	RT_ASSERT( jssKey != NULL, "unable to create the key string." );
	*vp = STRING_TO_JSVAL(jssKey);

	return JS_TRUE;
}

enum {
	privateKey = PK_PRIVATE,
	publicKey = PK_PUBLIC
};

CONFIGURE_CLASS

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION( CreateKeys )
		FUNCTION( Encrypt )
		FUNCTION( Decrypt )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_SWITCH( privateKey, key )
		PROPERTY_SWITCH( publicKey, key )
	END_PROPERTY_SPEC

	BEGIN_STATIC_FUNCTION_SPEC
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
	END_STATIC_PROPERTY_SPEC

	HAS_PRIVATE

END_CLASS


/****************************************************************

We let
p = 61 	— first prime number (to be kept secret or deleted securely)
q = 53 	— second prime number (to be kept secret or deleted securely)
n = pq = 3233 	— modulus (to be made public)
e = 17 	— public exponent (to be made public)
d = 2753 	— private exponent (to be kept secret)

The public key is (e, n). The private key is d. The encryption function is:

    encrypt(m) = me mod n = m17 mod 3233

where m is the plaintext. The decryption function is:

    decrypt(c) = cd mod n = c2753 mod 3233

where c is the ciphertext.

To encrypt the plaintext value 123, we calculate

    encrypt(123) = 12317 mod 3233 = 855

To decrypt the ciphertext value 855, we calculate

    decrypt(855) = 8552753 mod 3233 = 123


*/
