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

#define MODE_CTR "CTR"
#define MODE_CFB "CFB"

enum CryptMode {
	mode_ctr,
	mode_cfb
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
DEFINE_CONSTRUCTOR() {

//	RT_ASSERT_THIS_CLASS(); <- done in RT_ASSERT_CONSTRUCTING
	RT_ASSERT_CONSTRUCTING(_class)
	RT_ASSERT_ARGC( 4 );

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

   int numRounds = 0;
	if ( argc >= 5 ) {

		RT_JSVAL_TO_INT32( numRounds, argv[4] );
	}

	CipherPrivate *privateData = (CipherPrivate*)malloc( sizeof(CipherPrivate) );
	RT_ASSERT_ALLOC( privateData );

	int cipherIndex = find_cipher(cipherName);
	RT_ASSERT_1( cipherIndex != -1, "cipher %s is not registred", cipherName );

	ltc_cipher_descriptor *cipher = &cipher_descriptor[cipherIndex];
	privateData->descriptor = cipher;
	RT_ASSERT_1( cipher->test() == CRYPT_OK, "%s cipher test failed.", cipherName );

//	RT_ASSERT_1( IVLength == cipher.block_length, "IV must have the same size as cipher block length (%d bytes)", cipher.block_length );

	int err;
	if ( strcasecmp( modeName, MODE_CTR ) == 0 ) {

		privateData->mode = mode_ctr;
		symmetric_CTR *psctr = (symmetric_CTR *)malloc( sizeof(symmetric_CTR) );
		RT_ASSERT_ALLOC( psctr );
		if ((err = ctr_start( cipherIndex, (const unsigned char *)IV, (const unsigned char *)key, keyLength, numRounds, CTR_COUNTER_LITTLE_ENDIAN, psctr )) != CRYPT_OK)
			return ThrowCryptError(cx, err); // (TBD) free privateData and psctr
		privateData->symmetric_XXX = psctr;
	} else if ( strcasecmp( modeName, MODE_CFB ) == 0 ) {

		privateData->mode = mode_cfb;
		symmetric_CFB *symmetric_XXX = (symmetric_CFB *)malloc( sizeof(symmetric_CFB) );
		RT_ASSERT_ALLOC( symmetric_XXX );
		if ((err = cfb_start( cipherIndex, (const unsigned char *)IV, (const unsigned char *)key, keyLength, numRounds, symmetric_XXX )) != CRYPT_OK)
			return ThrowCryptError(cx, err); // (TBD) free privateData and psctr
		privateData->symmetric_XXX = symmetric_XXX;
	} else
		RT_ASSERT_1( false, "unsupported %s mode.", modeName );

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

	char *ct = (char *)JS_malloc( cx, ptLength );
	RT_ASSERT_ALLOC( ct );

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
DEFINE_FUNCTION( Decrypt ) {

	RT_ASSERT_ARGC( 1 );
	RT_ASSERT_CLASS( obj, _class );
	CipherPrivate *privateData = (CipherPrivate *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( privateData );

	char *ct;
	int ctLength;
	RT_JSVAL_TO_STRING_AND_LENGTH( argv[0], ct, ctLength );

	char *pt = (char *)JS_malloc( cx, ctLength );
	RT_ASSERT_ALLOC( pt );

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
DEFINE_PROPERTY( IVGetter ) {

	RT_ASSERT_CLASS( obj, _class );
	CipherPrivate *privateData = (CipherPrivate *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( privateData );

	char *IV = NULL;
	unsigned long IVLength;

	int err;
	switch ( privateData->mode ) {
		case mode_ctr: {
			symmetric_CTR *psctr = (symmetric_CTR *)privateData->symmetric_XXX;
			IVLength = psctr->blocklen;
			IV = (char*)JS_malloc( cx, IVLength );
			RT_ASSERT_ALLOC( IV );
			if ( (err=ctr_getiv( (unsigned char *)IV, &IVLength, psctr )) != CRYPT_OK )
				return ThrowCryptError(cx, err);
			break;
		}
		case mode_cfb: {
			symmetric_CFB *psctr = (symmetric_CFB *)privateData->symmetric_XXX;
			IVLength = psctr->blocklen;
			IV = (char*)JS_malloc( cx, IVLength );
			RT_ASSERT_ALLOC( IV );
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
DEFINE_PROPERTY( list ) {

	JSObject *list = JS_NewObject( cx, NULL, NULL, NULL );
	RT_ASSERT( list != NULL, "unable to create cipher list." );

	const char *name;
	int keySize;
	int i;
	LTC_MUTEX_LOCK(&ltc_cipher_mutex);
	for (i=0; i<TAB_SIZE; i++)
		if ( cipher_is_valid(i) ) {
		  //JS_DefineProperty(cx, list, cipherName, JSVAL_TRUE, NULL, NULL, 0);
			keySize = INT_MAX;
			name = cipher_descriptor[i].name;
			cipher_descriptor[i].keysize(&keySize);
			jsval value = INT_TO_JSVAL( keySize );
			JS_SetProperty( cx, list, name, &value );
		}
	LTC_MUTEX_UNLOCK(&ltc_cipher_mutex);

	*vp = OBJECT_TO_JSVAL(list);
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
		PROPERTY_READ( list )
	END_STATIC_PROPERTY_SPEC

	HAS_PRIVATE

END_CLASS


/****************************************************************

CTR ( Counter CryptMode )
	http://en.wikipedia.org/wiki/Counter_mode (fr: http://fr.wikipedia.org/wiki/CryptMode_d%27op%C3%A9ration_%28cryptographie%29 )

*/
