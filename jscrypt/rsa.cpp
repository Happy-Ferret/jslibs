#include "stdafx.h"

#include "rsa.h"
#include "prng.h"
#include "hash.h"

#include <tomcrypt.h>
#include <tommath.h>

#include "cryptError.h"

#include "../common/jshelper.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void rsa_Finalize(JSContext *cx, JSObject *obj) {

	RsaPrivate *rsaPrivate = (RsaPrivate *)JS_GetPrivate( cx, obj );
	if ( rsaPrivate == NULL )
		return;

	rsa_free(&rsaPrivate->key);
	free(rsaPrivate);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// prng, keysize
JSBool rsa_construct(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT( JS_IsConstructing(cx), RT_ERROR_NEED_CONSTRUCTION );
	RsaPrivate *rsaPrivate = (RsaPrivate *)malloc( sizeof(RsaPrivate) );
	RT_ASSERT( rsaPrivate != NULL, RT_ERROR_OUT_OF_MEMORY );
	JS_SetPrivate( cx, obj, rsaPrivate );

	return JS_TRUE;
}


JSBool rsa_createKeys(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT_ARGC( 2 );
	RT_ASSERT_CLASS( obj, &rsa_class );
	
	JSObject *objPrng JSVAL_TO_OBJECT(argv[0]);
	RT_ASSERT_CLASS( objPrng, &prng_class );

	PrngPrivate *prngPrivate = (PrngPrivate *)JS_GetPrivate( cx, objPrng );
	RT_ASSERT( prngPrivate != NULL, "invalid prng." );
	
	int32 keySize;
	RT_JSVAL_TO_INT32( argv[1], keySize );

	int prngIndex = find_prng(prngPrivate->prng.name);
	RT_ASSERT_1( prngIndex != -1, "prng %s is not registred", prngPrivate->prng.name );

	RsaPrivate *rsaPrivate = (RsaPrivate *)JS_GetPrivate( cx, obj );

	int err;
	if ((err=rsa_make_key( &prngPrivate->state, prngIndex, keySize/8, 65537, &rsaPrivate->key )) != CRYPT_OK)
		return ThrowCryptError(cx, err); // [TBD] should free rsaPrivate or Finalize is called ?

	return JS_TRUE;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool rsa_encryptKey(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT_ARGC( 3 );

	RT_ASSERT_CLASS( obj, &rsa_class );
	RsaPrivate *privateData = (RsaPrivate *)JS_GetPrivate( cx, obj );

	JSObject *objPrng = JSVAL_TO_OBJECT(argv[0]);
	RT_ASSERT_CLASS( objPrng, &prng_class );
	PrngPrivate *prngPrivate = (PrngPrivate *)JS_GetPrivate( cx, objPrng );
	RT_ASSERT( prngPrivate != NULL, "prng is not initialized." );
	int prngIndex = find_prng(prngPrivate->prng.name);
	RT_ASSERT_1( prngIndex != -1, "prng %s is not registred", prngPrivate->prng.name );

	char *hashName;
	RT_JSVAL_TO_STRING( argv[1], hashName );
	int hashIndex = find_hash(hashName);
	RT_ASSERT_1( hashIndex != -1, "hash %s is not registred", hashName );

/*
	JSObject *objHash = JSVAL_TO_OBJECT(argv[1]);
	RT_ASSERT_CLASS( objHash, &hash_class );
	HashPrivate *hashPrivate = (HashPrivate *)JS_GetPrivate( cx, objHash );
	RT_ASSERT( hashPrivate != NULL, "hash is not initialized." );
	int hashIndex = find_hash(hashPrivate->hash.name);
	RT_ASSERT_1( hashIndex != -1, "hash %s is not registred", hashPrivate->hash.name );
*/

	char *in;
	int inLength;
	RT_JSVAL_TO_STRING_AND_LENGTH( argv[2], in, inLength );

	unsigned long outLength = inLength + mp_unsigned_bin_size((mp_int*)privateData->key.N) - hash_descriptor[hashIndex].hashsize - 2; // length = message + (modulus length - 2 * hash size - 2)

	char *out = (char *)JS_malloc( cx, outLength );
	RT_ASSERT( out != NULL, RT_ERROR_OUT_OF_MEMORY );

	int err;
	if ( (err=rsa_encrypt_key( (const unsigned char *)in, inLength, (unsigned char *)out, &outLength, NULL, 0, &prngPrivate->state, prngIndex, hashIndex, &privateData->key )) != CRYPT_OK )
		return ThrowCryptError(cx, err); // [TBD] free rsaPrivate ?

	JSString *jssOut = JS_NewString( cx, out, outLength );
	RT_ASSERT( jssOut != NULL, "unable to create the string." );
	*rval = STRING_TO_JSVAL(jssOut);

	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool rsa_decryptKey(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT_ARGC( 2 );

	RT_ASSERT_CLASS( obj, &rsa_class );
	RsaPrivate *privateData = (RsaPrivate *)JS_GetPrivate( cx, obj );

	char *hashName;
	RT_JSVAL_TO_STRING( argv[0], hashName );
	int hashIndex = find_hash(hashName);
	RT_ASSERT_1( hashIndex != -1, "hash %s is not registred", hashName );

/*
	JSObject *objHash = JSVAL_TO_OBJECT(argv[0]);
	RT_ASSERT_CLASS( objHash, &hash_class );
	HashPrivate *hashPrivate = (HashPrivate *)JS_GetPrivate( cx, objHash );
	RT_ASSERT( hashPrivate != NULL, "hash is not initialized." );
	int hashIndex = find_hash(hashPrivate->hash.name);
	RT_ASSERT_1( hashIndex != -1, "hash %s is not registred", hashPrivate->hash.name );
*/

	char *in;
	int inLength;
	RT_JSVAL_TO_STRING_AND_LENGTH( argv[1], in, inLength );

//	int modulusSize = mp_unsigned_bin_size((mp_int*)privateData->key.N);
//	unsigned long outLength = inLength - ( modulusSize - hash_descriptor[hashIndex].hashsize - 2 );
//	if ( outLength < modulusSize )
//		outLength = modulusSize;

	unsigned long outLength = inLength;

	char *out = (char *)JS_malloc( cx, outLength );
	RT_ASSERT( out != NULL, RT_ERROR_OUT_OF_MEMORY );

	int stat;

	int err;
	if ( (err=rsa_decrypt_key( (const unsigned char *)in, inLength, (unsigned char *)out, &outLength, NULL, 0, hashIndex, &stat, &privateData->key )) != CRYPT_OK )
		return ThrowCryptError(cx, err); // [TBD] free rsaPrivate ?

	// RT_ASSERT( stat == 1, "invalid decryption" );
	if ( stat != 1 ) {

		*rval = JSVAL_VOID;
		return JS_TRUE;
	}

	JSString *jssOut = JS_NewString( cx, out, outLength );
	RT_ASSERT( jssOut != NULL, "unable to create the string." );
	*rval = STRING_TO_JSVAL(jssOut);

	return JS_TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSFunctionSpec rsa_FunctionSpec[] = { // *name, call, nargs, flags, extra
 { "CreateKeys"        , rsa_createKeys },
 { "EncryptKey"        , rsa_encryptKey },
 { "DecryptKey"        , rsa_decryptKey },
 { 0 }
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool rsa_setter_key(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	RT_ASSERT_CLASS( obj, &rsa_class );
	RsaPrivate *privateData = (RsaPrivate *)JS_GetPrivate( cx, obj );
	RT_ASSERT( privateData != NULL, RT_ERROR_NOT_INITIALIZED );

	char *key;
	int keyLength;
	RT_JSVAL_TO_STRING_AND_LENGTH( *vp, key, keyLength );

	int err;
	if ( (err=rsa_import( (const unsigned char *)key, keyLength, &privateData->key )) != CRYPT_OK )
		return ThrowCryptError(cx, err); // [TBD] free rsaPrivate ?

	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool rsa_getter_key(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	RT_ASSERT_CLASS( obj, &rsa_class );
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
		return ThrowCryptError(cx, err); // [TBD] free rsaPrivate ?

	JSString *jssKey = JS_NewStringCopyN( cx, key, keyLength );
	RT_ASSERT( jssKey != NULL, "unable to create the key string." );
	*vp = STRING_TO_JSVAL(jssKey);

	return JS_TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSPropertySpec rsa_PropertySpec[] = { // *name, tinyid, flags, getter, setter
	{ "privateKey"     , PK_PRIVATE, JSPROP_PERMANENT, rsa_getter_key , rsa_setter_key },
	{ "publicKey"      , PK_PUBLIC , JSPROP_PERMANENT, rsa_getter_key , rsa_setter_key  },
	{ 0 }
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool rsa_static_blockLength(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSFunctionSpec rsa_static_FunctionSpec[] = { // *name, call, nargs, flags, extra
//	{ "BlockLength"        , rsa_static_blockLength  , 0, 0, 0 },
	{ 0 }
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool rsa_static_getter_cipherList(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSPropertySpec rsa_static_PropertySpec[] = { // *name, tinyid, flags, getter, setter
//	{ "cipherList"   , 0, JSPROP_PERMANENT|JSPROP_READONLY, rsa_static_getter_cipherList , NULL },
	{ 0 }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSClass rsa_class = { "Rsa", JSCLASS_HAS_PRIVATE,
	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, rsa_Finalize
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSObject *rsaInitClass( JSContext *cx, JSObject *obj ) {

	return JS_InitClass( cx, obj, NULL, &rsa_class, rsa_construct, 0, rsa_PropertySpec, rsa_FunctionSpec, rsa_static_PropertySpec, rsa_static_FunctionSpec );
}


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