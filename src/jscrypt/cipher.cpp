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

/**doc fileIndex:top **/

/**doc
$CLASS_HEADER
**/
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

/**doc
 * $INAME( modeName, cipherName, key, [IV], [arg], [rounds] )
  Constructs a Cipher object that use _cipherName_ algorithm for performing encryption and decryption.
  $H arguments
   $ARG string modeName: is the block cipher modes of operation:
    * ECB (Electronic codebook)
    * CFB (Cipher feedback)
    * OFB (Output Feedback)
    * CBC (Cipher Block Chaining)
    * CTR (CounTeR)
    * LRW
    * F8
   $ARG string cipherName: is the name of the cipher algorithm used for data encryption and decryption:
    * blowfish
    * rc5
    * rc6
    * rc2
    * saferp
    * safer_k64, safer_k128, safer_sk64, safer_sk128
    * rijndael, aes
    * rijndael_enc, aes_enc
    * xtea
    * twofish
    * des, des3
    * cast5
    * noekeon
    * skipjack
    * khazad
    * anubis
    * kseed
    * kasumi
   $ARG string key: is the encryption/decryption key.
   $ARG string IV:
    _IV_ is the first initialization vector:
    The IV value is the initialization vector to be used with the cipher.
    You must fill the IV yourself and it is assumed they are the same length as the block size of the cipher you choose.
    It is important that the IV be random for each unique message you want to encrypt.
    $H beware
     This argument is invalid in ECB block mode.
   $ARG string arg: is either the tweak key for the LRW mode or the salt value for the F8 mode. In other modes _arg_ must be undefined.
    $H beware
     In LRW mode, the tweak value must have the same length as the _key_.
   $ARG integer rounds: is the number of rounds to do with the current sipher. If the argument is omitted, a default value is used.
**/
// mode, cipher, key, IV
DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();
	J_S_ASSERT_ARG_MIN( 3 );

	const char *modeName;
	J_CHK( JsvalToString(cx, &argv[0], &modeName) ); // warning: GC on the returned buffer !

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
		J_REPORT_ERROR_1("Invalid mode %s", modeName);

	const char *cipherName;
	J_CHK( JsvalToString(cx, &argv[1], &cipherName) ); // warning: GC on the returned buffer !

	const char *key;
	size_t keyLength;
	J_CHK( JsvalToStringAndLength(cx, &argv[2], &key, &keyLength) ); // warning: GC on the returned buffer !

	const char *IV = NULL;
	size_t IVLength = 0;
	if ( argc >= 4 && !JSVAL_IS_VOID( argv[3] ) )
		J_CHK( JsvalToStringAndLength(cx, &argv[3], &IV, &IVLength ) ); // warning: GC on the returned buffer !

	const char *optarg = NULL;
	size_t optargLength = 0;
	if ( argc >= 5 && !JSVAL_IS_VOID( argv[4] ) )
		J_CHK( JsvalToStringAndLength(cx, &argv[4], &optarg, &optargLength ) ); // warning: GC on the returned buffer !

   int numRounds = 0; // default value, us a default number of rounds.
	if ( argc >= 6 && !JSVAL_IS_VOID( argv[5] ) )
		J_CHK( JsvalToInt(cx, argv[5], &numRounds) );

	CipherPrivate *privateData = (CipherPrivate*)malloc( sizeof(CipherPrivate) );
	J_S_ASSERT_ALLOC( privateData );

	privateData->mode = mode;

	int cipherIndex = find_cipher(cipherName);
	J_S_ASSERT_1( cipherIndex != -1, "cipher %s is not available", cipherName );
	ltc_cipher_descriptor *cipher = &cipher_descriptor[cipherIndex];

	privateData->descriptor = cipher;
	J_S_ASSERT_1( cipher->test() == CRYPT_OK, "%s cipher test failed.", cipherName );

	int err;
	switch ( mode ) {
		case mode_ecb: {
			privateData->symmetric_XXX = malloc( sizeof(symmetric_ECB) );
			J_S_ASSERT_ALLOC( privateData->symmetric_XXX );
			J_S_ASSERT_2( (int)keyLength >= cipher->min_key_length && (int)(int)keyLength <= cipher->max_key_length, "Invalid key length (need [%d,%d]  bytes)", cipher->min_key_length, cipher->max_key_length );
//			J_S_ASSERT( IV == NULL, "Initialization vector is invalid for this mode." );
			J_S_ASSERT( optarg == NULL, "invalid 'arg' argument for this mode." );
			err = ecb_start( cipherIndex, (unsigned char *)key, (int)keyLength, numRounds, (symmetric_ECB *)privateData->symmetric_XXX );
			break;
		}
		case mode_cfb: {
			privateData->symmetric_XXX = malloc( sizeof(symmetric_CFB) );
			J_S_ASSERT_ALLOC( privateData->symmetric_XXX );
			J_S_ASSERT_2( (int)keyLength >= cipher->min_key_length && (int)keyLength <= cipher->max_key_length, "Invalid key length (need [%d,%d]  bytes)", cipher->min_key_length, cipher->max_key_length );
//			J_S_ASSERT_1( IVLength == cipher->block_length, "This cipher require a IV length of %d", cipher->block_length );
			J_S_ASSERT( optarg == NULL, "invalid 'arg' argument for this mode." );
			err = cfb_start( cipherIndex, (unsigned char *)IV, (unsigned char *)key, (int)keyLength, numRounds, (symmetric_CFB *)privateData->symmetric_XXX );
			break;
		}
		case mode_ofb: {
			privateData->symmetric_XXX = malloc( sizeof(symmetric_OFB) );
			J_S_ASSERT_ALLOC( privateData->symmetric_XXX );
			J_S_ASSERT_2( (int)keyLength >= cipher->min_key_length && (int)keyLength <= cipher->max_key_length, "Invalid key length (need [%d,%d]  bytes)", cipher->min_key_length, cipher->max_key_length );
//			J_S_ASSERT_1( IVLength == cipher->block_length, "This cipher require a IV length of %d", cipher->block_length );
			J_S_ASSERT( optarg == NULL, "invalid 'arg' argument for this mode." );
			err = ofb_start( cipherIndex, (unsigned char *)IV, (unsigned char *)key, (int)keyLength, numRounds, (symmetric_OFB *)privateData->symmetric_XXX );
			break;
		}
		case mode_cbc: {
			privateData->symmetric_XXX = malloc( sizeof(symmetric_CBC) );
			J_S_ASSERT_ALLOC( privateData->symmetric_XXX );
			J_S_ASSERT_2( (int)keyLength >= cipher->min_key_length && (int)keyLength <= cipher->max_key_length, "Invalid key length (need [%d,%d]  bytes)", cipher->min_key_length, cipher->max_key_length );
//			J_S_ASSERT_1( IVLength == cipher->block_length, "This cipher require a IV length of %d", cipher->block_length );
			J_S_ASSERT( optarg == NULL, "invalid 'arg' argument for this mode." );
			err = cbc_start( cipherIndex, (unsigned char *)IV, (unsigned char *)key, (int)keyLength, numRounds, (symmetric_CBC *)privateData->symmetric_XXX );
			break;
		}
		case mode_ctr: {
			privateData->symmetric_XXX = malloc( sizeof(symmetric_CTR) );
			J_S_ASSERT_ALLOC( privateData->symmetric_XXX );
			J_S_ASSERT_2( (int)keyLength >= cipher->min_key_length && (int)keyLength <= (size_t)cipher->max_key_length, "Invalid key length (need [%d,%d]  bytes)", cipher->min_key_length, cipher->max_key_length );
//			J_S_ASSERT_1( IVLength == cipher->block_length, "This cipher require a IV length of %d", cipher->block_length );
			J_S_ASSERT( optarg == NULL, "invalid 'arg' argument for this mode." );
			err = ctr_start( cipherIndex, (unsigned char *)IV, (unsigned char *)key, (int)keyLength, numRounds, CTR_COUNTER_LITTLE_ENDIAN, (symmetric_CTR *)privateData->symmetric_XXX );
			break;
		}
		case mode_lrw: {
			privateData->symmetric_XXX = malloc( sizeof(symmetric_LRW) );
			J_S_ASSERT_ALLOC( privateData->symmetric_XXX );
			J_S_ASSERT_2( (int)keyLength >= cipher->min_key_length && (int)keyLength <= cipher->max_key_length, "Invalid key length (need [%d,%d]  bytes)", cipher->min_key_length, cipher->max_key_length );
//			J_S_ASSERT_1( IVLength == cipher->block_length, "This cipher require a IV length of %d", cipher->block_length );
			J_S_ASSERT_1( optargLength == (int)keyLength, "The tweak length must be %d bytes length (key size)", (int)keyLength );
			err = lrw_start( cipherIndex, (unsigned char *)IV, (unsigned char *)key, (int)keyLength, (unsigned char *)optarg, numRounds, (symmetric_LRW *)privateData->symmetric_XXX );
			break;
		}
		case mode_f8: {
			privateData->symmetric_XXX = malloc( sizeof(symmetric_F8) );
			J_S_ASSERT_ALLOC( privateData->symmetric_XXX );
			J_S_ASSERT_2( (int)keyLength >= cipher->min_key_length && (int)keyLength <= cipher->max_key_length, "Invalid key length (need [%d,%d]  bytes)", cipher->min_key_length, cipher->max_key_length );
//			J_S_ASSERT_1( IVLength == cipher->block_length, "This cipher require a IV length of %d", cipher->block_length );
			J_S_ASSERT( optargLength > 0, "This mode need the salt argument" );
			err = f8_start( cipherIndex, (unsigned char *)IV, (unsigned char *)key, (int)keyLength, (unsigned char *)optarg, optargLength, numRounds, (symmetric_F8 *)privateData->symmetric_XXX );
			break;
		}
	}

	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err); // (TBD) free privateData and psctr
	JS_SetPrivate( cx, obj, privateData );
	return JS_TRUE;
}

/**doc
=== Methods ===
**/

/**doc
 * $DATA $INAME( data )
  Encrypts the given _data_ using the current cipher.
**/
DEFINE_FUNCTION( Encrypt ) {

	J_S_ASSERT_THIS_CLASS();
	J_S_ASSERT_ARG_MIN( 1 );
	CipherPrivate *privateData = (CipherPrivate *)JS_GetPrivate( cx, obj );
	J_S_ASSERT_RESOURCE( privateData );

	const char *pt;
	size_t ptLength;
	J_CHK( JsvalToStringAndLength(cx, &argv[0], &pt, &ptLength) ); // warning: GC on the returned buffer !

	char *ct = (char *)JS_malloc( cx, ptLength +1 );
	J_S_ASSERT_ALLOC( ct );
	ct[ptLength] = '\0';

	int err;
	switch ( privateData->mode ) {
		case mode_ecb:
			J_S_ASSERT_1( ptLength == privateData->descriptor->block_length, "This mode require a %d bytes block of data", privateData->descriptor->block_length );
			err = ecb_encrypt( (const unsigned char *)pt, (unsigned char *)ct, ptLength, (symmetric_ECB *)privateData->symmetric_XXX );
			break;
		case mode_cfb:
			err = cfb_encrypt( (const unsigned char *)pt, (unsigned char *)ct, ptLength, (symmetric_CFB *)privateData->symmetric_XXX );
			break;
		case mode_ofb:
			err = ofb_encrypt( (const unsigned char *)pt, (unsigned char *)ct, ptLength, (symmetric_OFB *)privateData->symmetric_XXX );
			break;
		case mode_cbc:
			J_S_ASSERT_1( ptLength == privateData->descriptor->block_length, "This mode require a %d bytes block of data", privateData->descriptor->block_length );
			err = cbc_encrypt( (const unsigned char *)pt, (unsigned char *)ct, ptLength, (symmetric_CBC *)privateData->symmetric_XXX );
			break;
		case mode_ctr:
			err = ctr_encrypt( (const unsigned char *)pt, (unsigned char *)ct, ptLength, (symmetric_CTR *)privateData->symmetric_XXX );
			break;
		case mode_lrw:
			J_S_ASSERT_1( ptLength == privateData->descriptor->block_length, "This mode require a %d bytes block of data", privateData->descriptor->block_length );
			err = lrw_encrypt( (const unsigned char *)pt, (unsigned char *)ct, ptLength, (symmetric_LRW *)privateData->symmetric_XXX );
			break;
		case mode_f8:
			err = f8_encrypt( (const unsigned char *)pt, (unsigned char *)ct, ptLength, (symmetric_F8 *)privateData->symmetric_XXX );
			break;
	}

	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err);

	J_CHK( J_NewBlob( cx, ct, ptLength, rval ) );

	return JS_TRUE;
}

/**doc
 * $DATA $INAME( data )
  Decrypts the given _data_ using the current cipher.
**/
DEFINE_FUNCTION( Decrypt ) {

	J_S_ASSERT_ARG_MIN( 1 );
	J_S_ASSERT_CLASS( obj, _class );
	CipherPrivate *privateData = (CipherPrivate *)JS_GetPrivate( cx, obj );
	J_S_ASSERT_RESOURCE( privateData );

	const char *ct;
	size_t ctLength;
	J_CHK( JsvalToStringAndLength(cx, &argv[0], &ct, &ctLength) ); // warning: GC on the returned buffer !

	char *pt = (char *)JS_malloc( cx, ctLength +1 );
	J_S_ASSERT_ALLOC( pt );
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

	J_CHK( J_NewBlob( cx, pt, ctLength, rval ) );

	return JS_TRUE;
}

/**doc
=== Properties ===
**/

/**doc
 * $INT $INAME $READONLY
  Is the block length of the current cipher.
**/
DEFINE_PROPERTY( blockLength ) {

	J_S_ASSERT_CLASS( obj, _class );
	CipherPrivate *privateData = (CipherPrivate *)JS_GetPrivate( cx, obj );
	J_S_ASSERT_RESOURCE( privateData );
	*vp = INT_TO_JSVAL( privateData->descriptor->block_length );
	return JS_TRUE;
}


/**doc
 * $INT $INAME $READONLY
  Is the key size of the current cipher.
**/
DEFINE_PROPERTY( keySize ) {

	J_S_ASSERT_CLASS( obj, _class );
	CipherPrivate *privateData = (CipherPrivate *)JS_GetPrivate( cx, obj );
	J_S_ASSERT_RESOURCE( privateData );
	*vp = INT_TO_JSVAL( privateData->descriptor->keysize );
	return JS_TRUE;
}


/**doc
 * $STR $INAME $READONLY
  Is the name of the current cipher.
**/
DEFINE_PROPERTY( name ) {

	J_S_ASSERT_CLASS( obj, _class );
	CipherPrivate *privateData = (CipherPrivate *)JS_GetPrivate( cx, obj );
	J_S_ASSERT_RESOURCE( privateData );
	JSString *jsstr = JS_NewStringCopyZ(cx, privateData->descriptor->name);
	J_S_ASSERT_ALLOC( jsstr );
	*vp = STRING_TO_JSVAL( jsstr );
	return JS_TRUE;
}


/**doc
 * $STR $INAME
  Set or get the current initialization vector of the cipher.
**/
DEFINE_PROPERTY( IVSetter ) {

	J_S_ASSERT_CLASS( obj, _class );
	CipherPrivate *privateData = (CipherPrivate *)JS_GetPrivate( cx, obj );
	J_S_ASSERT_RESOURCE( privateData );

	const char *IV;
	size_t IVLength;
	J_CHK( JsvalToStringAndLength(cx, vp, &IV, &IVLength) );

	int err;
	switch ( privateData->mode ) {
		case mode_ecb:
			J_REPORT_ERROR("No IV in ECB mode");
		case mode_cfb: {
			symmetric_CFB *tmp = (symmetric_CFB *)privateData->symmetric_XXX;
			J_S_ASSERT_1( IVLength == tmp->blocklen, "This cipher require a IV length of %d", tmp->blocklen );
			err = cfb_setiv( (const unsigned char *)IV, IVLength, tmp );
			if (err != CRYPT_OK)
				return ThrowCryptError(cx, err);
			break;
		}
		case mode_ofb: {
			symmetric_OFB *tmp = (symmetric_OFB *)privateData->symmetric_XXX;
			J_S_ASSERT_1( IVLength == tmp->blocklen, "This cipher require a IV length of %d", tmp->blocklen );
			err = ofb_setiv( (const unsigned char *)IV, IVLength, tmp );
			if (err != CRYPT_OK)
				return ThrowCryptError(cx, err);
			break;
		}
		case mode_cbc: {
			symmetric_CBC *tmp = (symmetric_CBC *)privateData->symmetric_XXX;
			J_S_ASSERT_1( IVLength == tmp->blocklen, "This cipher require a IV length of %d", tmp->blocklen );
			err = cbc_setiv( (const unsigned char *)IV, IVLength, tmp );
			if (err != CRYPT_OK)
				return ThrowCryptError(cx, err);
			break;
		}
		case mode_ctr: {
			symmetric_CTR *tmp = (symmetric_CTR *)privateData->symmetric_XXX;
			J_S_ASSERT_1( IVLength == tmp->blocklen, "This cipher require a IV length of %d", tmp->blocklen );
			err = ctr_setiv( (const unsigned char *)IV, IVLength, tmp );
			if (err != CRYPT_OK)
				return ThrowCryptError(cx, err);
			break;
		}
		case mode_lrw: {
			symmetric_LRW *tmp = (symmetric_LRW *)privateData->symmetric_XXX;
			J_S_ASSERT_1( IVLength == 16, "This cipher require a IV length of %d", 16 );
			err = lrw_setiv( (const unsigned char *)IV, IVLength, tmp );
			if (err != CRYPT_OK)
				return ThrowCryptError(cx, err);
			break;
		}
		case mode_f8: {
			symmetric_F8 *tmp = (symmetric_F8 *)privateData->symmetric_XXX;
			J_S_ASSERT_1( IVLength == tmp->blocklen, "This cipher require a IV length of %d", tmp->blocklen );
			err = f8_setiv( (const unsigned char *)IV, IVLength, tmp );
			if (err != CRYPT_OK)
				return ThrowCryptError(cx, err);
			break;
		}
	}

	return JS_TRUE;
}

DEFINE_PROPERTY( IVGetter ) {

	J_S_ASSERT_CLASS( obj, _class );
	CipherPrivate *privateData = (CipherPrivate *)JS_GetPrivate( cx, obj );
	J_S_ASSERT_RESOURCE( privateData );

	char *IV = NULL;
	unsigned long IVLength;

	int err;
	switch ( privateData->mode ) {

		case mode_ecb:
			J_REPORT_ERROR("No IV in ECB mode");
		case mode_cfb: {
			symmetric_CFB *tmp = (symmetric_CFB *)privateData->symmetric_XXX;
			IVLength = tmp->blocklen;
			IV = (char*)JS_malloc( cx, IVLength );
			J_S_ASSERT_ALLOC( IV );
			err = cfb_getiv( (unsigned char *)IV, &IVLength, tmp );
			break;
		}
		case mode_ofb: {
			symmetric_OFB *tmp = (symmetric_OFB *)privateData->symmetric_XXX;
			IVLength = tmp->blocklen;
			IV = (char*)JS_malloc( cx, IVLength );
			J_S_ASSERT_ALLOC( IV );
			err = ofb_getiv( (unsigned char *)IV, &IVLength, tmp );
			break;
		}
		case mode_cbc: {
			symmetric_CBC *tmp = (symmetric_CBC *)privateData->symmetric_XXX;
			IVLength = tmp->blocklen;
			IV = (char*)JS_malloc( cx, IVLength );
			J_S_ASSERT_ALLOC( IV );
			err = cbc_getiv( (unsigned char *)IV, &IVLength, tmp );
			break;
		}
		case mode_ctr: {
			symmetric_CTR *tmp = (symmetric_CTR *)privateData->symmetric_XXX;
			IVLength = tmp->blocklen;
			IV = (char*)JS_malloc( cx, IVLength );
			J_S_ASSERT_ALLOC( IV );
			err = ctr_getiv( (unsigned char *)IV, &IVLength, tmp );
			break;
		}
		case mode_lrw: {
			symmetric_LRW *tmp = (symmetric_LRW *)privateData->symmetric_XXX;
			IVLength = 16;
			IV = (char*)JS_malloc( cx, IVLength );
			J_S_ASSERT_ALLOC( IV );
			err = lrw_getiv( (unsigned char *)IV, &IVLength, tmp );
			break;
		}
		case mode_f8: {
			symmetric_F8 *tmp = (symmetric_F8 *)privateData->symmetric_XXX;
			IVLength = tmp->blocklen;
			IV = (char*)JS_malloc( cx, IVLength );
			J_S_ASSERT_ALLOC( IV );
			err = f8_getiv( (unsigned char *)IV, &IVLength, tmp );
			break;
		}
	}
	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err);

	J_CHK( J_NewBlob( cx, IV, IVLength, vp ) );

	return JS_TRUE;
}


/**doc
=== Static properties ===
**/

/**doc
 * $OBJ $INAME $READONLY
  Contains the list of all available ciphers and their feature. The list is a javascript object that map cipher names (key) with another object (value) that contain information.
**/
DEFINE_PROPERTY( list ) {

	if ( JSVAL_IS_VOID( *vp ) ) {

		JSObject *list = JS_NewObject( cx, NULL, NULL, NULL );
		J_S_ASSERT_ALLOC( list );
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
