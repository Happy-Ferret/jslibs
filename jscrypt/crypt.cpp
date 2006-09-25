#include "stdafx.h"

#define XP_WIN
#include <jsapi.h>

#include "crypt.h"

#include <tomcrypt.h>

#include "cryptError.h"

#include "../common/jshelper.h"

#define MODE_CTR "CTR"
#define MODE_CFB "CFB"

enum Mode {
	mode_ctr,
	mode_cfb
};

struct PrivateData {

	Mode mode;
	void *symmetric_XXX;
};

void crypt_Finalize(JSContext *cx, JSObject *obj);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSClass crypt_class = { "Crypt", JSCLASS_HAS_PRIVATE,
	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, crypt_Finalize
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void crypt_Finalize(JSContext *cx, JSObject *obj) {

	PrivateData *privateData = (PrivateData *)JS_GetPrivate( cx, obj );
	if ( privateData == NULL )
		return;

	switch ( privateData->mode ) {
		case mode_ctr:
			ctr_done((symmetric_CTR *)privateData->symmetric_XXX);
			break;
		case mode_cfb:
			cfb_done((symmetric_CFB *)privateData->symmetric_XXX);
			break;
	}

	free(privateData->symmetric_XXX);
	free(privateData);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// mode, cipher, key, IV
JSBool crypt_construct(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT( JS_IsConstructing(cx), RT_ERROR_NEED_CONSTRUCTION );
	RT_ASSERT_ARGC( 4 );
	RT_ASSERT_CLASS( obj, &crypt_class );

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

	RT_ASSERT_1( IVLength == cipher.block_length, "IV must have the same size as cipher block length (%d bytes)", cipher.block_length );

	int err;
	if ( _stricmp( modeName, MODE_CTR ) == 0 ) {

		privateData->mode = mode_ctr;
		symmetric_CTR *psctr = (symmetric_CTR *)malloc( sizeof(symmetric_CTR) );
		RT_ASSERT( psctr != NULL, RT_ERROR_OUT_OF_MEMORY );
		if ((err = ctr_start( cipherIndex, (const unsigned char *)IV, (const unsigned char *)key, keyLength, 0, CTR_COUNTER_LITTLE_ENDIAN, psctr )) != CRYPT_OK)
			return ThrowCryptError(cx, err); // [TBD] free privateData and psctr
		privateData->symmetric_XXX = psctr;
	} else if ( _stricmp( modeName, MODE_CFB ) == 0 ) {

		privateData->mode = mode_cfb;
		symmetric_CFB *symmetric_XXX = (symmetric_CFB *)malloc( sizeof(symmetric_CFB) );
		RT_ASSERT( symmetric_XXX != NULL, RT_ERROR_OUT_OF_MEMORY );
		if ((err = cfb_start( cipherIndex, (const unsigned char *)IV, (const unsigned char *)key, keyLength, 0, symmetric_XXX )) != CRYPT_OK)
			return ThrowCryptError(cx, err); // [TBD] free privateData and psctr
		privateData->symmetric_XXX = symmetric_XXX;
	} else
		RT_ASSERT_1( false, "unsupported %s mode.", modeName );

	JS_SetPrivate( cx, obj, privateData );

	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool crypt_encrypt(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT_ARGC( 1 );
	RT_ASSERT_CLASS( obj, &crypt_class );
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
			if ( (err = ctr_encrypt( (const unsigned char *)pt, (unsigned char *)ct, ptLength, (symmetric_CTR *)privateData->symmetric_XXX )) != CRYPT_OK )
				return ThrowCryptError(cx, err);
			break;
		case mode_cfb:
			if ( (err = cfb_encrypt( (const unsigned char *)pt, (unsigned char *)ct, ptLength, (symmetric_CFB *)privateData->symmetric_XXX )) != CRYPT_OK )
				return ThrowCryptError(cx, err);
			break;
		default:
			RT_ASSERT( false, "unsupported mode." );
	}

	JSString *jssCt = JS_NewString( cx, ct, ptLength );
	RT_ASSERT( jssCt != NULL, "unable to create the cipher string." );
	*rval = STRING_TO_JSVAL(jssCt);

	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool crypt_decrypt(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT_ARGC( 1 );
	RT_ASSERT_CLASS( obj, &crypt_class );
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
			if ( (err = ctr_decrypt( (const unsigned char *)ct, (unsigned char *)pt, ctLength, (symmetric_CTR *)privateData->symmetric_XXX )) != CRYPT_OK )
				return ThrowCryptError(cx, err);
			break;
		case mode_cfb:
			if ( (err = cfb_decrypt( (const unsigned char *)ct, (unsigned char *)pt, ctLength, (symmetric_CFB *)privateData->symmetric_XXX )) != CRYPT_OK )
				return ThrowCryptError(cx, err);
			break;
		default:
			RT_ASSERT( false, "unsupported mode." );
	}

	JSString *jssCt = JS_NewString( cx, pt, ctLength );
	RT_ASSERT( jssCt != NULL, "unable to create the cipher string." );
	*rval = STRING_TO_JSVAL(jssCt);

	return JS_TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSFunctionSpec crypt_FunctionSpec[] = { // *name, call, nargs, flags, extra
 { "Encrypt"        , crypt_encrypt     , 0, 0, 0 },
 { "Decrypt"        , crypt_decrypt     , 0, 0, 0 },
 { 0 }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool crypt_setter_IV(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	RT_ASSERT_CLASS( obj, &crypt_class );
	PrivateData *privateData = (PrivateData *)JS_GetPrivate( cx, obj );
	RT_ASSERT( privateData != NULL, RT_ERROR_NOT_INITIALIZED );

	char *IV;
	int IVLength;
	RT_JSVAL_TO_STRING_AND_LENGTH( *vp, IV, IVLength );

	int err;
	switch ( privateData->mode ) {
		case mode_ctr:
			if ( (err=ctr_setiv( (const unsigned char *)IV, IVLength, (symmetric_CTR *)privateData->symmetric_XXX )) != CRYPT_OK )
				return ThrowCryptError(cx, err);
			break;
		case mode_cfb:
			if ( (err=cfb_setiv( (const unsigned char *)IV, IVLength, (symmetric_CFB *)privateData->symmetric_XXX )) != CRYPT_OK )
				return ThrowCryptError(cx, err);
			break;
		default:
			RT_ASSERT( false, "unsupported mode." );
	}

	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool crypt_getter_IV(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	RT_ASSERT_CLASS( obj, &crypt_class );
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
			if ( (err=ctr_getiv( (unsigned char *)IV, &IVLength, psctr )) != CRYPT_OK )
				return ThrowCryptError(cx, err);
			break;
		}
		case mode_cfb: {
			symmetric_CFB *psctr = (symmetric_CFB *)privateData->symmetric_XXX;
			IVLength = psctr->blocklen;
			IV = (char*)JS_malloc( cx, IVLength );
			RT_ASSERT( IV != NULL, RT_ERROR_OUT_OF_MEMORY );
			if ( (err=cfb_getiv( (unsigned char *)IV, &IVLength, psctr )) != CRYPT_OK )
				return ThrowCryptError(cx, err);
			break;
		}
		default:
			RT_ASSERT( false, "unsupported mode." );
	}

	JSString *jssIV = JS_NewString( cx, IV, IVLength );
	RT_ASSERT( jssIV != NULL, "unable to create the IV string." );
	*vp = STRING_TO_JSVAL(jssIV);

	return JS_TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSPropertySpec crypt_PropertySpec[] = { // *name, tinyid, flags, getter, setter
	{ "IV"            , 0, JSPROP_PERMANENT, crypt_getter_IV, crypt_setter_IV },
  { 0 }
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool crypt_static_blockLength(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	RT_ASSERT_ARGC( 1 );
	char *cipherName;
	RT_JSVAL_TO_STRING( argv[0], cipherName );

	int cipherIndex = find_cipher(cipherName);
	RT_ASSERT_1( cipherIndex != -1, "cipher %s is not registred", cipherName );

	*rval = INT_TO_JSVAL(cipher_descriptor[cipherIndex].block_length);

	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSFunctionSpec crypt_static_FunctionSpec[] = { // *name, call, nargs, flags, extra
	{ "BlockLength"        , crypt_static_blockLength  , 0, 0, 0 },
	{ 0 }
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool crypt_static_getter_myStatic(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

  return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSPropertySpec crypt_static_PropertySpec[] = { // *name, tinyid, flags, getter, setter
//	{ "myStatic"                , 0, JSPROP_PERMANENT|JSPROP_READONLY, crypt_static_getter_myStatic         , NULL },
  { 0 }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSObject *cryptInitClass( JSContext *cx, JSObject *obj ) {


#ifdef RIJNDAEL
  register_cipher (&aes_desc);
#endif
#ifdef BLOWFISH
  register_cipher (&blowfish_desc);
#endif
#ifdef XTEA
  register_cipher (&xtea_desc);
#endif
#ifdef RC5
  register_cipher (&rc5_desc);
#endif
#ifdef RC6
  register_cipher (&rc6_desc);
#endif
#ifdef SAFERP
  register_cipher (&saferp_desc);
#endif
#ifdef TWOFISH
  register_cipher (&twofish_desc);
#endif
#ifdef SAFER
  register_cipher (&safer_k64_desc);
  register_cipher (&safer_sk64_desc);
  register_cipher (&safer_k128_desc);
  register_cipher (&safer_sk128_desc);
#endif
#ifdef RC2
  register_cipher (&rc2_desc);
#endif
#ifdef DES
  register_cipher (&des_desc);
  register_cipher (&des3_desc);
#endif
#ifdef CAST5
  register_cipher (&cast5_desc);
#endif
#ifdef NOEKEON
  register_cipher (&noekeon_desc);
#endif
#ifdef SKIPJACK
  register_cipher (&skipjack_desc);
#endif
#ifdef KHAZAD
  register_cipher (&khazad_desc);
#endif
#ifdef ANUBIS
  register_cipher (&anubis_desc);
#endif

// if register_cipher failed but the cipher will not be used, it is acceptable because further check is done on find_cipher call


/*
	// you must register a cipher before you use it
	if ( register_cipher(&blowfish_desc) == -1 ) {

		JS_ReportError( cx, "Unable to register Blowfish cipher." );
		return JS_FALSE;
	}
*/

	return JS_InitClass( cx, obj, NULL, &crypt_class, crypt_construct, 0, crypt_PropertySpec, crypt_FunctionSpec, crypt_static_PropertySpec, crypt_static_FunctionSpec );
}


/****************************************************************

CTR ( Counter Mode )
	http://en.wikipedia.org/wiki/Counter_mode (fr: http://fr.wikipedia.org/wiki/Mode_d%27op%C3%A9ration_%28cryptographie%29 )

*/