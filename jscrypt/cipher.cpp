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
#include "cipher.h"

enum CryptMode {
	mode_ecb,
	mode_cfb,
	mode_ofb,
	mode_cbc,
	mode_ctr,
	mode_lrw,
	mode_f8,
};

struct CipherPrivate {

	ltc_cipher_descriptor *descriptor;
	CryptMode mode;
	void *symmetric_XXX;
};


BEGIN_CLASS( Cipher )

DEFINE_FINALIZE() {

	CipherPrivate *privateData = (CipherPrivate *)JS_GetPrivate( cx, obj );
	if ( privateData == NULL ) // is already finished ?
		return;
	
	int err;
	switch ( privateData->mode ) {
		case mode_ecb:
			err = ecb_done( (symmetric_ECB *)privateData->symmetric_XXX );
			break;
		case mode_cfb:
			err = cfb_done( (symmetric_CFB *)privateData->symmetric_XXX );
			break;
		case mode_ofb:
			err = ofb_done( (symmetric_OFB *)privateData->symmetric_XXX );
			break;
		case mode_cbc:
			err = cbc_done( (symmetric_CBC *)privateData->symmetric_XXX );
			break;
		case mode_ctr:
			err = ctr_done( (symmetric_CTR *)privateData->symmetric_XXX );
			break;
		case mode_lrw:
			err = lrw_done( (symmetric_LRW *)privateData->symmetric_XXX );
			break;
		case mode_f8:
			err = f8_done( (symmetric_F8 *)privateData->symmetric_XXX );
			break;
	}
//	if (err != CRYPT_OK)
//		return ThrowCryptError(cx, err);
	free(privateData->symmetric_XXX);
	free(privateData);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// mode, cipher, key, IV
DEFINE_CONSTRUCTOR() {

	RT_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();
	RT_ASSERT_ARGC( 3 );

	char *modeName;
	RT_JSVAL_TO_STRING( argv[0], modeName );

	CryptMode mode;
	if ( strcasecmp( modeName, "ECB" ) == 0 )
		mode = mode_ecb;
	else if ( strcasecmp( modeName, "CFB" ) == 0 )
		mode = mode_cfb;
	else if ( strcasecmp( modeName, "OFB" ) == 0 )
		mode = mode_ofb;
	else if ( strcasecmp( modeName, "CBC" ) == 0 )
		mode = mode_cbc;
	else if ( strcasecmp( modeName, "CTR" ) == 0 )
		mode = mode_ctr;
	else if ( strcasecmp( modeName, "LRW" ) == 0 )
		mode = mode_lrw;
	else if ( strcasecmp( modeName, "F8" ) == 0 )
		mode = mode_f8;
	else
		REPORT_ERROR_1("Invalid mode %s", modeName);

	char *cipherName;
	RT_JSVAL_TO_STRING( argv[1], cipherName );

	char *key;
	int keyLength;
	RT_JSVAL_TO_STRING_AND_LENGTH( argv[2], key, keyLength );

	char *IV = NULL;
	int IVLength = 0;
	if ( argc >= 4 && argv[3] != JSVAL_VOID )
		RT_JSVAL_TO_STRING_AND_LENGTH( argv[3], IV, IVLength );

	char *optarg = NULL;
	int optargLength = 0;
	if ( argc >= 5 && argv[4] != JSVAL_VOID )
		RT_JSVAL_TO_STRING_AND_LENGTH( argv[4], optarg, optargLength );

   int numRounds = 0; // default value, us a default number of rounds.
	if ( argc >= 6 && argv[5] != JSVAL_VOID )
		RT_JSVAL_TO_INT32( numRounds, argv[5] );

	CipherPrivate *privateData = (CipherPrivate*)malloc( sizeof(CipherPrivate) );
	RT_ASSERT_ALLOC( privateData );

	privateData->mode = mode;

	int cipherIndex = find_cipher(cipherName);
	RT_ASSERT_1( cipherIndex != -1, "cipher %s is not available", cipherName );
	ltc_cipher_descriptor *cipher = &cipher_descriptor[cipherIndex];

	privateData->descriptor = cipher;
	RT_ASSERT_1( cipher->test() == CRYPT_OK, "%s cipher test failed.", cipherName );

	int err;
	switch ( mode ) {
		case mode_ecb: {
			privateData->symmetric_XXX = malloc( sizeof(symmetric_ECB) );
			RT_ASSERT_ALLOC( privateData->symmetric_XXX );
			RT_ASSERT_2( keyLength >= cipher->min_key_length && keyLength <= cipher->max_key_length, "Invalid key length (need [%d,%d]  bytes)", cipher->min_key_length, cipher->max_key_length );
//			RT_ASSERT( IV == NULL, "Initialization vector is invalid for this mode." );
			RT_ASSERT( optarg == NULL, "invalid 'arg' argument for this mode." );
			err = ecb_start( cipherIndex, (unsigned char *)key, keyLength, numRounds, (symmetric_ECB *)privateData->symmetric_XXX );
			break;
		}
		case mode_cfb: {
			privateData->symmetric_XXX = malloc( sizeof(symmetric_CFB) );
			RT_ASSERT_ALLOC( privateData->symmetric_XXX );
			RT_ASSERT_2( keyLength >= cipher->min_key_length && keyLength <= cipher->max_key_length, "Invalid key length (need [%d,%d]  bytes)", cipher->min_key_length, cipher->max_key_length );
//			RT_ASSERT_1( IVLength == cipher->block_length, "This cipher require a IV length of %d", cipher->block_length );
			RT_ASSERT( optarg == NULL, "invalid 'arg' argument for this mode." );
			err = cfb_start( cipherIndex, (unsigned char *)IV, (unsigned char *)key, keyLength, numRounds, (symmetric_CFB *)privateData->symmetric_XXX );
			break;
		}
		case mode_ofb: {
			privateData->symmetric_XXX = malloc( sizeof(symmetric_OFB) );
			RT_ASSERT_ALLOC( privateData->symmetric_XXX );
			RT_ASSERT_2( keyLength >= cipher->min_key_length && keyLength <= cipher->max_key_length, "Invalid key length (need [%d,%d]  bytes)", cipher->min_key_length, cipher->max_key_length );
//			RT_ASSERT_1( IVLength == cipher->block_length, "This cipher require a IV length of %d", cipher->block_length );
			RT_ASSERT( optarg == NULL, "invalid 'arg' argument for this mode." );
			err = ofb_start( cipherIndex, (unsigned char *)IV, (unsigned char *)key, keyLength, numRounds, (symmetric_OFB *)privateData->symmetric_XXX );
			break;
		}
		case mode_cbc: {
			privateData->symmetric_XXX = malloc( sizeof(symmetric_CBC) );
			RT_ASSERT_ALLOC( privateData->symmetric_XXX );
			RT_ASSERT_2( keyLength >= cipher->min_key_length && keyLength <= cipher->max_key_length, "Invalid key length (need [%d,%d]  bytes)", cipher->min_key_length, cipher->max_key_length );
//			RT_ASSERT_1( IVLength == cipher->block_length, "This cipher require a IV length of %d", cipher->block_length );
			RT_ASSERT( optarg == NULL, "invalid 'arg' argument for this mode." );
			err = cbc_start( cipherIndex, (unsigned char *)IV, (unsigned char *)key, keyLength, numRounds, (symmetric_CBC *)privateData->symmetric_XXX );
			break;
		}
		case mode_ctr: {
			privateData->symmetric_XXX = malloc( sizeof(symmetric_CTR) );
			RT_ASSERT_ALLOC( privateData->symmetric_XXX );
			RT_ASSERT_2( keyLength >= cipher->min_key_length && keyLength <= cipher->max_key_length, "Invalid key length (need [%d,%d]  bytes)", cipher->min_key_length, cipher->max_key_length );
//			RT_ASSERT_1( IVLength == cipher->block_length, "This cipher require a IV length of %d", cipher->block_length );
			RT_ASSERT( optarg == NULL, "invalid 'arg' argument for this mode." );
			err = ctr_start( cipherIndex, (unsigned char *)IV, (unsigned char *)key, keyLength, numRounds, CTR_COUNTER_LITTLE_ENDIAN, (symmetric_CTR *)privateData->symmetric_XXX );
			break;
		}
		case mode_lrw: {
			privateData->symmetric_XXX = malloc( sizeof(symmetric_LRW) );
			RT_ASSERT_ALLOC( privateData->symmetric_XXX );
			RT_ASSERT_2( keyLength >= cipher->min_key_length && keyLength <= cipher->max_key_length, "Invalid key length (need [%d,%d]  bytes)", cipher->min_key_length, cipher->max_key_length );
//			RT_ASSERT_1( IVLength == cipher->block_length, "This cipher require a IV length of %d", cipher->block_length );
			RT_ASSERT_1( optargLength == keyLength, "The tweak length must be %d bytes length (key size)", keyLength );
			err = lrw_start( cipherIndex, (unsigned char *)IV, (unsigned char *)key, keyLength, (unsigned char *)optarg, numRounds, (symmetric_LRW *)privateData->symmetric_XXX );
			break;
		}
		case mode_f8: {
			privateData->symmetric_XXX = malloc( sizeof(symmetric_F8) );
			RT_ASSERT_ALLOC( privateData->symmetric_XXX );
			RT_ASSERT_2( keyLength >= cipher->min_key_length && keyLength <= cipher->max_key_length, "Invalid key length (need [%d,%d]  bytes)", cipher->min_key_length, cipher->max_key_length );
//			RT_ASSERT_1( IVLength == cipher->block_length, "This cipher require a IV length of %d", cipher->block_length );
			RT_ASSERT( optargLength > 0, "This mode need the salt argument" );
			err = f8_start( cipherIndex, (unsigned char *)IV, (unsigned char *)key, keyLength, (unsigned char *)optarg, optargLength, numRounds, (symmetric_F8 *)privateData->symmetric_XXX );
			break;
		}
	}

	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err); // (TBD) free privateData and psctr
	JS_SetPrivate( cx, obj, privateData );
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DEFINE_FUNCTION( Encrypt ) {

	RT_ASSERT_THIS_CLASS();
	RT_ASSERT_ARGC( 1 );
	CipherPrivate *privateData = (CipherPrivate *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( privateData );

	char *pt;
	int ptLength;
	RT_JSVAL_TO_STRING_AND_LENGTH( argv[0], pt, ptLength );

	char *ct = (char *)JS_malloc( cx, ptLength +1 );
	RT_ASSERT_ALLOC( ct );
	ct[ptLength] = '\0';

	int err;
	switch ( privateData->mode ) {
		case mode_ecb:
			RT_ASSERT_1( ptLength == privateData->descriptor->block_length, "This mode require a %d bytes block of data", privateData->descriptor->block_length );
			err = ecb_encrypt( (const unsigned char *)pt, (unsigned char *)ct, ptLength, (symmetric_ECB *)privateData->symmetric_XXX );
			break;
		case mode_cfb:
			err = cfb_encrypt( (const unsigned char *)pt, (unsigned char *)ct, ptLength, (symmetric_CFB *)privateData->symmetric_XXX );
			break;
		case mode_ofb:
			err = ofb_encrypt( (const unsigned char *)pt, (unsigned char *)ct, ptLength, (symmetric_OFB *)privateData->symmetric_XXX );
			break;
		case mode_cbc:
			RT_ASSERT_1( ptLength == privateData->descriptor->block_length, "This mode require a %d bytes block of data", privateData->descriptor->block_length );
			err = cbc_encrypt( (const unsigned char *)pt, (unsigned char *)ct, ptLength, (symmetric_CBC *)privateData->symmetric_XXX );
			break;
		case mode_ctr:
			err = ctr_encrypt( (const unsigned char *)pt, (unsigned char *)ct, ptLength, (symmetric_CTR *)privateData->symmetric_XXX );
			break;
		case mode_lrw:
			RT_ASSERT_1( ptLength == privateData->descriptor->block_length, "This mode require a %d bytes block of data", privateData->descriptor->block_length );
			err = lrw_encrypt( (const unsigned char *)pt, (unsigned char *)ct, ptLength, (symmetric_LRW *)privateData->symmetric_XXX );
			break;
		case mode_f8:
			err = f8_encrypt( (const unsigned char *)pt, (unsigned char *)ct, ptLength, (symmetric_F8 *)privateData->symmetric_XXX );
			break;
	}

	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err);

	JSString *jssCt = JS_NewString( cx, ct, ptLength );
	RT_ASSERT_ALLOC( jssCt );
	*rval = STRING_TO_JSVAL(jssCt);
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DEFINE_FUNCTION( Decrypt ) {

	RT_ASSERT_ARGC( 1 );
	RT_ASSERT_CLASS( obj, _class );
	CipherPrivate *privateData = (CipherPrivate *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( privateData );

	char *ct;
	int ctLength;
	RT_JSVAL_TO_STRING_AND_LENGTH( argv[0], ct, ctLength );

	char *pt = (char *)JS_malloc( cx, ctLength +1 );
	RT_ASSERT_ALLOC( pt );
	pt[ctLength] = '\0';

	int err;
	switch ( privateData->mode ) {
		case mode_ecb:
			err = ecb_decrypt( (const unsigned char *)ct, (unsigned char *)pt, ctLength, (symmetric_ECB *)privateData->symmetric_XXX );
			break;
		case mode_cfb:
			err = cfb_decrypt( (const unsigned char *)ct, (unsigned char *)pt, ctLength, (symmetric_CFB *)privateData->symmetric_XXX );
			break;
		case mode_ofb:
			err = ofb_decrypt( (const unsigned char *)ct, (unsigned char *)pt, ctLength, (symmetric_OFB *)privateData->symmetric_XXX );
			break;
		case mode_cbc:
			err = cbc_decrypt( (const unsigned char *)ct, (unsigned char *)pt, ctLength, (symmetric_CBC *)privateData->symmetric_XXX );
			break;
		case mode_ctr:
			err = ctr_decrypt( (const unsigned char *)ct, (unsigned char *)pt, ctLength, (symmetric_CTR *)privateData->symmetric_XXX );
			break;
		case mode_lrw:
			err = lrw_decrypt( (const unsigned char *)ct, (unsigned char *)pt, ctLength, (symmetric_LRW *)privateData->symmetric_XXX );
			break;
		case mode_f8:
			err = f8_decrypt( (const unsigned char *)ct, (unsigned char *)pt, ctLength, (symmetric_F8 *)privateData->symmetric_XXX );
			break;
	}

	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err);

	JSString *jssCt = JS_NewString( cx, pt, ctLength );
	RT_ASSERT_ALLOC( jssCt );
	*rval = STRING_TO_JSVAL(jssCt);
	return JS_TRUE;
}


DEFINE_PROPERTY( blockLength ) {

	RT_ASSERT_CLASS( obj, _class );
	CipherPrivate *privateData = (CipherPrivate *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( privateData );
	*vp = INT_TO_JSVAL( privateData->descriptor->block_length );
	return JS_TRUE;
}


DEFINE_PROPERTY( keySize ) {

	RT_ASSERT_CLASS( obj, _class );
	CipherPrivate *privateData = (CipherPrivate *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( privateData );
	*vp = INT_TO_JSVAL( privateData->descriptor->keysize );
	return JS_TRUE;
}


DEFINE_PROPERTY( name ) {

	RT_ASSERT_CLASS( obj, _class );
	CipherPrivate *privateData = (CipherPrivate *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( privateData );
	JSString *jsstr = JS_NewStringCopyZ(cx, privateData->descriptor->name);
	RT_ASSERT_ALLOC( jsstr );
	*vp = STRING_TO_JSVAL( jsstr );
	return JS_TRUE;
}


DEFINE_PROPERTY( IVSetter ) {

	RT_ASSERT_CLASS( obj, _class );
	CipherPrivate *privateData = (CipherPrivate *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( privateData );

	char *IV;
	int IVLength;
	RT_JSVAL_TO_STRING_AND_LENGTH( *vp, IV, IVLength );

	int err;
	switch ( privateData->mode ) {
		case mode_ecb:
			REPORT_ERROR("No IV in ECB mode");
		case mode_cfb: {
			symmetric_CFB *tmp = (symmetric_CFB *)privateData->symmetric_XXX;
			RT_ASSERT_1( IVLength == tmp->blocklen, "This cipher require a IV length of %d", tmp->blocklen );
			err = cfb_setiv( (const unsigned char *)IV, IVLength, tmp );
			if (err != CRYPT_OK)
				return ThrowCryptError(cx, err);
			break;
		}
		case mode_ofb: {
			symmetric_OFB *tmp = (symmetric_OFB *)privateData->symmetric_XXX;
			RT_ASSERT_1( IVLength == tmp->blocklen, "This cipher require a IV length of %d", tmp->blocklen );
			err = ofb_setiv( (const unsigned char *)IV, IVLength, tmp );
			if (err != CRYPT_OK)
				return ThrowCryptError(cx, err);
			break;
		}
		case mode_cbc: {
			symmetric_CBC *tmp = (symmetric_CBC *)privateData->symmetric_XXX;
			RT_ASSERT_1( IVLength == tmp->blocklen, "This cipher require a IV length of %d", tmp->blocklen );
			err = cbc_setiv( (const unsigned char *)IV, IVLength, tmp );
			if (err != CRYPT_OK)
				return ThrowCryptError(cx, err);
			break;
		}
		case mode_ctr: {
			symmetric_CTR *tmp = (symmetric_CTR *)privateData->symmetric_XXX;
			RT_ASSERT_1( IVLength == tmp->blocklen, "This cipher require a IV length of %d", tmp->blocklen );
			err = ctr_setiv( (const unsigned char *)IV, IVLength, tmp );
			if (err != CRYPT_OK)
				return ThrowCryptError(cx, err);
			break;
		}
		case mode_lrw: {
			symmetric_LRW *tmp = (symmetric_LRW *)privateData->symmetric_XXX;
			RT_ASSERT_1( IVLength == 16, "This cipher require a IV length of %d", 16 );
			err = lrw_setiv( (const unsigned char *)IV, IVLength, tmp );
			if (err != CRYPT_OK)
				return ThrowCryptError(cx, err);
			break;
		}
		case mode_f8: {
			symmetric_F8 *tmp = (symmetric_F8 *)privateData->symmetric_XXX;
			RT_ASSERT_1( IVLength == tmp->blocklen, "This cipher require a IV length of %d", tmp->blocklen );
			err = f8_setiv( (const unsigned char *)IV, IVLength, tmp );
			if (err != CRYPT_OK)
				return ThrowCryptError(cx, err);
			break;
		}
	}

	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DEFINE_PROPERTY( IVGetter ) {

	RT_ASSERT_CLASS( obj, _class );
	CipherPrivate *privateData = (CipherPrivate *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( privateData );

	char *IV = NULL;
	unsigned long IVLength;

	int err;
	switch ( privateData->mode ) {

		case mode_ecb:
			REPORT_ERROR("No IV in ECB mode");
		case mode_cfb: {
			symmetric_CFB *tmp = (symmetric_CFB *)privateData->symmetric_XXX;
			IVLength = tmp->blocklen;
			IV = (char*)JS_malloc( cx, IVLength );
			RT_ASSERT_ALLOC( IV );
			err = cfb_getiv( (unsigned char *)IV, &IVLength, tmp );
			break;
		}
		case mode_ofb: {
			symmetric_OFB *tmp = (symmetric_OFB *)privateData->symmetric_XXX;
			IVLength = tmp->blocklen;
			IV = (char*)JS_malloc( cx, IVLength );
			RT_ASSERT_ALLOC( IV );
			err = ofb_getiv( (unsigned char *)IV, &IVLength, tmp );
			break;
		}
		case mode_cbc: {
			symmetric_CBC *tmp = (symmetric_CBC *)privateData->symmetric_XXX;
			IVLength = tmp->blocklen;
			IV = (char*)JS_malloc( cx, IVLength );
			RT_ASSERT_ALLOC( IV );
			err = cbc_getiv( (unsigned char *)IV, &IVLength, tmp );
			break;
		}
		case mode_ctr: {
			symmetric_CTR *tmp = (symmetric_CTR *)privateData->symmetric_XXX;
			IVLength = tmp->blocklen;
			IV = (char*)JS_malloc( cx, IVLength );
			RT_ASSERT_ALLOC( IV );
			err = ctr_getiv( (unsigned char *)IV, &IVLength, tmp );
			break;
		}
		case mode_lrw: {
			symmetric_LRW *tmp = (symmetric_LRW *)privateData->symmetric_XXX;
			IVLength = 16;
			IV = (char*)JS_malloc( cx, IVLength );
			RT_ASSERT_ALLOC( IV );
			err = lrw_getiv( (unsigned char *)IV, &IVLength, tmp );
			break;
		}
		case mode_f8: {
			symmetric_F8 *tmp = (symmetric_F8 *)privateData->symmetric_XXX;
			IVLength = tmp->blocklen;
			IV = (char*)JS_malloc( cx, IVLength );
			RT_ASSERT_ALLOC( IV );
			err = f8_getiv( (unsigned char *)IV, &IVLength, tmp );
			break;
		}
	}
	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err);
	JSString *jssIV = JS_NewString(cx, IV, IVLength);
	RT_ASSERT( jssIV != NULL, "unable to create the IV string." );
	*vp = STRING_TO_JSVAL(jssIV);
	return JS_TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DEFINE_PROPERTY( list ) {

	if ( *vp == JSVAL_VOID ) {

		JSObject *list = JS_NewObject( cx, NULL, NULL, NULL );
		RT_ASSERT_ALLOC( list );
		*vp = OBJECT_TO_JSVAL(list);
		jsval value;
		int i;
		LTC_MUTEX_LOCK(&ltc_cipher_mutex);
		for (i=0; i<TAB_SIZE; i++)
			if ( cipher_is_valid(i) == CRYPT_OK ) {

				JSObject *desc = JS_NewObject( cx, NULL, NULL, NULL );
				value = OBJECT_TO_JSVAL(desc);
				JS_SetProperty( cx, list, cipher_descriptor[i].name, &value );

				value = INT_TO_JSVAL( cipher_descriptor[i].min_key_length );
				JS_SetProperty( cx, desc, "minKeyLength", &value );
				value = INT_TO_JSVAL( cipher_descriptor[i].max_key_length );
				JS_SetProperty( cx, desc, "maxKeyLength", &value );
				value = INT_TO_JSVAL( cipher_descriptor[i].block_length );
				JS_SetProperty( cx, desc, "blockLength", &value );
				value = INT_TO_JSVAL( cipher_descriptor[i].default_rounds );
				JS_SetProperty( cx, desc, "defaultRounds", &value );
			}
		LTC_MUTEX_UNLOCK(&ltc_cipher_mutex);
	}
	return JS_TRUE;
}


CONFIGURE_CLASS

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION( Encrypt )
		FUNCTION( Decrypt )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY( IV )
		PROPERTY_READ( blockLength )
		PROPERTY_READ( keySize )
		PROPERTY_READ( name )
	END_PROPERTY_SPEC

	BEGIN_STATIC_FUNCTION_SPEC
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_READ_STORE( list )
	END_STATIC_PROPERTY_SPEC

	HAS_PRIVATE

END_CLASS


/****************************************************************

CTR ( Counter CryptMode )
	http://en.wikipedia.org/wiki/Counter_mode (fr: http://fr.wikipedia.org/wiki/CryptMode_d%27op%C3%A9ration_%28cryptographie%29 )

*/
