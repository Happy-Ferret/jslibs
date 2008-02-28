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
#include "crypt.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void crypt_Finalize(JSContext *cx, JSObject *obj) {

	CryptPrivate *privateData = (CryptPrivate *)JS_GetPrivate( cx, obj );
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

	CryptPrivate *privateData = (CryptPrivate*)malloc( sizeof(CryptPrivate) );
	RT_ASSERT( privateData != NULL, RT_ERROR_OUT_OF_MEMORY );

	int cipherIndex = find_cipher(cipherName);
	RT_ASSERT_1( cipherIndex != -1, "cipher %s is not registred", cipherName );

	ltc_cipher_descriptor cipher = cipher_descriptor[cipherIndex];

	RT_ASSERT_1( cipher.test() == CRYPT_OK, "%s cipher test failed.", cipherName );

//	RT_ASSERT_1( IVLength == cipher.block_length, "IV must have the same size as cipher block length (%d bytes)", cipher.block_length );

	int err;
	if ( strcasecmp( modeName, MODE_CTR ) == 0 ) {

		privateData->mode = mode_ctr;
		symmetric_CTR *psctr = (symmetric_CTR *)malloc( sizeof(symmetric_CTR) );
		RT_ASSERT( psctr != NULL, RT_ERROR_OUT_OF_MEMORY );
		if ((err = ctr_start( cipherIndex, (const unsigned char *)IV, (const unsigned char *)key, keyLength, 0, CTR_COUNTER_LITTLE_ENDIAN, psctr )) != CRYPT_OK)
			return ThrowCryptError(cx, err); // (TBD) free privateData and psctr
		privateData->symmetric_XXX = psctr;
	} else if ( strcasecmp( modeName, MODE_CFB ) == 0 ) {

		privateData->mode = mode_cfb;
		symmetric_CFB *symmetric_XXX = (symmetric_CFB *)malloc( sizeof(symmetric_CFB) );
		RT_ASSERT( symmetric_XXX != NULL, RT_ERROR_OUT_OF_MEMORY );
		if ((err = cfb_start( cipherIndex, (const unsigned char *)IV, (const unsigned char *)key, keyLength, 0, symmetric_XXX )) != CRYPT_OK)
			return ThrowCryptError(cx, err); // (TBD) free privateData and psctr
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
	CryptPrivate *privateData = (CryptPrivate *)JS_GetPrivate( cx, obj );
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
	CryptPrivate *privateData = (CryptPrivate *)JS_GetPrivate( cx, obj );
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
	CryptPrivate *privateData = (CryptPrivate *)JS_GetPrivate( cx, obj );
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
	CryptPrivate *privateData = (CryptPrivate *)JS_GetPrivate( cx, obj );
	RT_ASSERT( privateData != NULL, RT_ERROR_NOT_INITIALIZED );

	char *IV = NULL;
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
JSBool crypt_static_getter_cipherList(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	JSObject *list = JS_NewObject( cx, NULL, NULL, NULL );
	RT_ASSERT( list != NULL, "unable to create cipher list." );

	const char *cipherName;
	int keySize;
	int x;
	LTC_MUTEX_LOCK(&ltc_cipher_mutex);
	for (x = 0; x < TAB_SIZE; x++)
		if ( (cipherName = cipher_descriptor[x].name) != NULL) {
		  //JS_DefineProperty(cx, list, cipherName, JSVAL_TRUE, NULL, NULL, 0);
			keySize = INT_MAX;
			cipher_descriptor[x].keysize(&keySize);
			jsval value = INT_TO_JSVAL( keySize );
			JS_SetProperty( cx, list, cipherName, &value );
		}
	LTC_MUTEX_UNLOCK(&ltc_cipher_mutex);

	*vp = OBJECT_TO_JSVAL(list);
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSPropertySpec crypt_static_PropertySpec[] = { // *name, tinyid, flags, getter, setter
	{ "cipherList"   , 0, JSPROP_PERMANENT|JSPROP_READONLY, crypt_static_getter_cipherList , NULL },
	{ 0 }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSClass crypt_class = { "Crypt", JSCLASS_HAS_PRIVATE,
	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, crypt_Finalize
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSObject *cryptInitClass( JSContext *cx, JSObject *obj ) {

	return JS_InitClass( cx, obj, NULL, &crypt_class, crypt_construct, 0, crypt_PropertySpec, crypt_FunctionSpec, crypt_static_PropertySpec, crypt_static_FunctionSpec );
}


/****************************************************************

CTR ( Counter CryptMode )
	http://en.wikipedia.org/wiki/Counter_mode (fr: http://fr.wikipedia.org/wiki/CryptMode_d%27op%C3%A9ration_%28cryptographie%29 )

*/
