#include "stdafx.h"

#define XP_WIN
#include <jsapi.h>

#include "rsa.h"
#include "prng.h"

#include <tomcrypt.h>

#include "cryptError.h"

#include "../common/jshelper.h"


struct RsaPrivate {
	rsa_key key;
};

void rsa_Finalize(JSContext *cx, JSObject *obj);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSClass rsa_class = { "Rsa", JSCLASS_HAS_PRIVATE,
	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, rsa_Finalize
};

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

	RsaPrivate *rsaPrivate = (RsaPrivate *)malloc( sizeof(RsaPrivate) );

	int err;
	if ((err=rsa_make_key( &prngPrivate->state, prngIndex, keySize/8, 65537, &rsaPrivate->key )) != CRYPT_OK)
		return ThrowCryptError(cx, err); // [TBD] free privateData and psctr

	return JS_TRUE;
}



//

/*
	char *modeName;
	RT_JSVAL_TO_STRING( argv[0], modeName );

	char *cipherName;
	RT_JSVAL_TO_STRING( argv[1], cipherName );

	char *key;
	int keyLength;
	RT_JSVAL_TO_STRING_AND_LENGTH( argv[2], key, keyLength );

	char *IV;
	int IVLength;
	RT_JSVAL_TO_STRING_AND_LENGTH( argv[3], IV, IVLength );

	PrivateData *privateData = (PrivateData*)malloc( sizeof(PrivateData) );
	RT_ASSERT( privateData != NULL, RT_ERROR_OUT_OF_MEMORY );

	int cipherIndex = find_cipher(cipherName);
	RT_ASSERT_1( cipherIndex != -1, "cipher %s is not registred", cipherName );

	ltc_cipher_descriptor cipher = cipher_descriptor[cipherIndex];

	RT_ASSERT_1( cipher.test() == rsa_OK, "%s cipher test failed.", cipherName );

//	RT_ASSERT_1( IVLength == cipher.block_length, "IV must have the same size as cipher block length (%d bytes)", cipher.block_length );

	int err;
	if ( _stricmp( modeName, MODE_CTR ) == 0 ) {

		privateData->mode = mode_ctr;
		symmetric_CTR *psctr = (symmetric_CTR *)malloc( sizeof(symmetric_CTR) );
		RT_ASSERT( psctr != NULL, RT_ERROR_OUT_OF_MEMORY );
		if ((err = ctr_start( cipherIndex, (const unsigned char *)IV, (const unsigned char *)key, keyLength, 0, CTR_COUNTER_LITTLE_ENDIAN, psctr )) != rsa_OK)
			return ThrowrsaError(cx, err); // [TBD] free privateData and psctr
		privateData->symmetric_XXX = psctr;
	} else if ( _stricmp( modeName, MODE_CFB ) == 0 ) {

		privateData->mode = mode_cfb;
		symmetric_CFB *symmetric_XXX = (symmetric_CFB *)malloc( sizeof(symmetric_CFB) );
		RT_ASSERT( symmetric_XXX != NULL, RT_ERROR_OUT_OF_MEMORY );
		if ((err = cfb_start( cipherIndex, (const unsigned char *)IV, (const unsigned char *)key, keyLength, 0, symmetric_XXX )) != rsa_OK)
			return ThrowrsaError(cx, err); // [TBD] free privateData and psctr
		privateData->symmetric_XXX = symmetric_XXX;
	} else
		RT_ASSERT_1( false, "unsupported %s mode.", modeName );

	JS_SetPrivate( cx, obj, privateData );
*/
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool rsa_encrypt(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT_ARGC( 1 );
	RT_ASSERT_CLASS( obj, &rsa_class );

/*
	PrivateData *privateData = (PrivateData *)JS_GetPrivate( cx, obj );
	RT_ASSERT( privateData != NULL, RT_ERROR_NOT_INITIALIZED );

	char *pt;
	int ptLength;
	RT_JSVAL_TO_STRING_AND_LENGTH( argv[0], pt, ptLength );

	char *ct = (char *)JS_malloc( cx, ptLength );
	RT_ASSERT( ct != NULL, RT_ERROR_OUT_OF_MEMORY );

	int err;
	switch ( privateData->mode ) {

		case mode_ctr:
			if ( (err = ctr_enrsa( (const unsigned char *)pt, (unsigned char *)ct, ptLength, (symmetric_CTR *)privateData->symmetric_XXX )) != rsa_OK )
				return ThrowrsaError(cx, err);
			break;
		case mode_cfb:
			if ( (err = cfb_enrsa( (const unsigned char *)pt, (unsigned char *)ct, ptLength, (symmetric_CFB *)privateData->symmetric_XXX )) != rsa_OK )
				return ThrowrsaError(cx, err);
			break;
		default:
			RT_ASSERT( false, "unsupported mode." );
	}

	JSString *jssCt = JS_NewString( cx, ct, ptLength );
	RT_ASSERT( jssCt != NULL, "unable to create the cipher string." );
	*rval = STRING_TO_JSVAL(jssCt);

*/
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool rsa_decrypt(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT_ARGC( 1 );
	RT_ASSERT_CLASS( obj, &rsa_class );
	
/*	
	PrivateData *privateData = (PrivateData *)JS_GetPrivate( cx, obj );
	RT_ASSERT( privateData != NULL, RT_ERROR_NOT_INITIALIZED );

	char *ct;
	int ctLength;
	RT_JSVAL_TO_STRING_AND_LENGTH( argv[0], ct, ctLength );

	char *pt = (char *)JS_malloc( cx, ctLength );
	RT_ASSERT( pt != NULL, RT_ERROR_OUT_OF_MEMORY );

	int err;
	switch ( privateData->mode ) {

		case mode_ctr:
			if ( (err = ctr_dersa( (const unsigned char *)ct, (unsigned char *)pt, ctLength, (symmetric_CTR *)privateData->symmetric_XXX )) != rsa_OK )
				return ThrowrsaError(cx, err);
			break;
		case mode_cfb:
			if ( (err = cfb_dersa( (const unsigned char *)ct, (unsigned char *)pt, ctLength, (symmetric_CFB *)privateData->symmetric_XXX )) != rsa_OK )
				return ThrowrsaError(cx, err);
			break;
		default:
			RT_ASSERT( false, "unsupported mode." );
	}

	JSString *jssCt = JS_NewString( cx, pt, ctLength );
	RT_ASSERT( jssCt != NULL, "unable to create the cipher string." );
	*rval = STRING_TO_JSVAL(jssCt);

*/
	return JS_TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSFunctionSpec rsa_FunctionSpec[] = { // *name, call, nargs, flags, extra
 { "Encrypt"        , rsa_encrypt     , 0, 0, 0 },
 { "Decrypt"        , rsa_decrypt     , 0, 0, 0 },
 { 0 }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool rsa_setter_IV(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
/*
	RT_ASSERT_CLASS( obj, &rsa_class );
	PrivateData *privateData = (PrivateData *)JS_GetPrivate( cx, obj );
	RT_ASSERT( privateData != NULL, RT_ERROR_NOT_INITIALIZED );

	char *IV;
	int IVLength;
	RT_JSVAL_TO_STRING_AND_LENGTH( *vp, IV, IVLength );

	int err;
	switch ( privateData->mode ) {
		case mode_ctr:
			if ( (err=ctr_setiv( (const unsigned char *)IV, IVLength, (symmetric_CTR *)privateData->symmetric_XXX )) != rsa_OK )
				return ThrowrsaError(cx, err);
			break;
		case mode_cfb:
			if ( (err=cfb_setiv( (const unsigned char *)IV, IVLength, (symmetric_CFB *)privateData->symmetric_XXX )) != rsa_OK )
				return ThrowrsaError(cx, err);
			break;
		default:
			RT_ASSERT( false, "unsupported mode." );
	}
*/
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool rsa_getter_IV(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
/*
	RT_ASSERT_CLASS( obj, &rsa_class );
	PrivateData *privateData = (PrivateData *)JS_GetPrivate( cx, obj );
	RT_ASSERT( privateData != NULL, RT_ERROR_NOT_INITIALIZED );

	char *IV;
	unsigned long IVLength;

	int err;
	switch ( privateData->mode ) {
		case mode_ctr: {
			symmetric_CTR *psctr = (symmetric_CTR *)privateData->symmetric_XXX;
			IVLength = psctr->blocklen;
			IV = (char*)JS_malloc( cx, IVLength );
			RT_ASSERT( IV != NULL, RT_ERROR_OUT_OF_MEMORY );
			if ( (err=ctr_getiv( (unsigned char *)IV, &IVLength, psctr )) != rsa_OK )
				return ThrowrsaError(cx, err);
			break;
		}
		case mode_cfb: {
			symmetric_CFB *psctr = (symmetric_CFB *)privateData->symmetric_XXX;
			IVLength = psctr->blocklen;
			IV = (char*)JS_malloc( cx, IVLength );
			RT_ASSERT( IV != NULL, RT_ERROR_OUT_OF_MEMORY );
			if ( (err=cfb_getiv( (unsigned char *)IV, &IVLength, psctr )) != rsa_OK )
				return ThrowrsaError(cx, err);
			break;
		}
		default:
			RT_ASSERT( false, "unsupported mode." );
	}

	JSString *jssIV = JS_NewString( cx, IV, IVLength );
	RT_ASSERT( jssIV != NULL, "unable to create the IV string." );
	*vp = STRING_TO_JSVAL(jssIV);
*/
	return JS_TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSPropertySpec rsa_PropertySpec[] = { // *name, tinyid, flags, getter, setter
	{ "IV"            , 0, JSPROP_PERMANENT, rsa_getter_IV, rsa_setter_IV },
  { 0 }
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool rsa_static_blockLength(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
/*
	RT_ASSERT_ARGC( 1 );
	char *cipherName;
	RT_JSVAL_TO_STRING( argv[0], cipherName );

	int cipherIndex = find_cipher(cipherName);
	RT_ASSERT_1( cipherIndex != -1, "cipher %s is not registred", cipherName );

	*rval = INT_TO_JSVAL(cipher_descriptor[cipherIndex].block_length);
*/
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSFunctionSpec rsa_static_FunctionSpec[] = { // *name, call, nargs, flags, extra
	{ "BlockLength"        , rsa_static_blockLength  , 0, 0, 0 },
	{ 0 }
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool rsa_static_getter_cipherList(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSPropertySpec rsa_static_PropertySpec[] = { // *name, tinyid, flags, getter, setter
	{ "cipherList"   , 0, JSPROP_PERMANENT|JSPROP_READONLY, rsa_static_getter_cipherList , NULL },
	{ 0 }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSObject *rsaInitClass( JSContext *cx, JSObject *obj ) {

	return JS_InitClass( cx, obj, NULL, &rsa_class, rsa_construct, 0, rsa_PropertySpec, rsa_FunctionSpec, rsa_static_PropertySpec, rsa_static_FunctionSpec );
}


/****************************************************************

*/