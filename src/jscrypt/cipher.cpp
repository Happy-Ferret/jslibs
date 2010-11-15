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
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( Cipher )

DEFINE_FINALIZE() {

	CipherPrivate *pv = (CipherPrivate *)JL_GetPrivate( cx, obj );
	if ( !pv )
		return;

	int err;
	switch ( pv->mode ) {
		case mode_ecb:
			err = ecb_done( (symmetric_ECB *)pv->symmetric_XXX );
			break;
		case mode_cfb:
			err = cfb_done( (symmetric_CFB *)pv->symmetric_XXX );
			break;
		case mode_ofb:
			err = ofb_done( (symmetric_OFB *)pv->symmetric_XXX );
			break;
		case mode_cbc:
			err = cbc_done( (symmetric_CBC *)pv->symmetric_XXX );
			break;
		case mode_ctr:
			err = ctr_done( (symmetric_CTR *)pv->symmetric_XXX );
			break;
		case mode_lrw:
			err = lrw_done( (symmetric_LRW *)pv->symmetric_XXX );
			break;
		case mode_f8:
			err = f8_done( (symmetric_F8 *)pv->symmetric_XXX );
			break;
	}
//	if (err != CRYPT_OK)
//		return ThrowCryptError(cx, err);
	JS_free(cx, pv->symmetric_XXX);
	JS_free(cx, pv);
}

/**doc
$TOC_MEMBER $INAME
 $INAME( modeName, cipherName, key, [IV], [arg], [rounds] )
  Constructs a Cipher object that use _cipherName_ algorithm for performing encryption and decryption.
  $H arguments
   $ARG $STR modeName: is the block cipher modes of operation:
    * ECB (Electronic codebook)
    * CFB (Cipher feedback)
    * OFB (Output Feedback)
    * CBC (Cipher Block Chaining)
    * CTR (CounTeR)
    * LRW
    * F8
   $ARG $STR cipherName: is the name of the cipher algorithm used for data encryption and decryption:
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
   $ARG $STR key: is the encryption/decryption key.
   $ARG $STR IV:
    _IV_ is the first initialization vector:
    The IV value is the initialization vector to be used with the cipher.
    You must fill the IV yourself and it is assumed they are the same length as the block size of the cipher you choose.
    It is important that the IV be random for each unique message you want to encrypt.
    $H beware
     This argument is invalid in ECB block mode.
   $ARG $STR arg: is either the tweak key for the LRW mode or the salt value for the F8 mode. In other modes _arg_ must be undefined.
    $H beware
     In LRW mode, the tweak value must have the same length as the _key_.
   $ARG $INT rounds: is the number of rounds to do with the current sipher. If the argument is omitted, a default value is used.
**/
// mode, cipher, key, IV
DEFINE_CONSTRUCTOR() {

	CipherPrivate *pv = NULL; // see. bad label

	JL_S_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;
	JL_S_ASSERT_ARG_MIN( 3 );

	const char *modeName;
	JL_CHK( JL_JsvalToCVal(cx, JL_ARG(1), &modeName) ); // warning: GC on the returned buffer !

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
		JL_REPORT_ERROR("Invalid mode %s", modeName);

	const char *cipherName;
	JL_CHK( JL_JsvalToCVal(cx, JL_ARG(2), &cipherName) ); // warning: GC on the returned buffer !

	const char *key;
	size_t keyLength;
	JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(3), &key, &keyLength) ); // warning: GC on the returned buffer !

	const char *IV;
	IV = NULL;
	size_t IVLength;
	IVLength = 0;
	if ( argc >= 4 && !JSVAL_IS_VOID( JL_ARG(4) ) )
		JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(4), &IV, &IVLength ) ); // warning: GC on the returned buffer !

	const char *optarg;
	optarg = NULL;
	size_t optargLength;
	optargLength = 0;
	if ( argc >= 5 && !JSVAL_IS_VOID( JL_ARG(5) ) )
		JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(5), &optarg, &optargLength ) ); // warning: GC on the returned buffer !

   int numRounds;
   numRounds= 0; // default value, us a default number of rounds.
	if ( argc >= 6 && !JSVAL_IS_VOID( JL_ARG(6) ) )
		JL_CHK( 	JL_JsvalToCVal(cx, JL_ARG(6), &numRounds) );

	pv = (CipherPrivate*)JS_malloc(cx, sizeof(CipherPrivate));
	JL_CHK( pv );

	pv->symmetric_XXX = NULL; // see. bad label
	pv->mode = mode;

	int cipherIndex;
	cipherIndex = find_cipher(cipherName);
	JL_S_ASSERT( cipherIndex != -1, "cipher %s is not available", cipherName );
	ltc_cipher_descriptor *cipher;
	cipher = &cipher_descriptor[cipherIndex];

	pv->descriptor = cipher;
	JL_S_ASSERT( cipher->test() == CRYPT_OK, "%s cipher test failed.", cipherName );

	int err;
	switch ( mode ) {
		case mode_ecb: {
			pv->symmetric_XXX = JS_malloc(cx, sizeof(symmetric_ECB));
			JL_S_ASSERT_ALLOC( pv->symmetric_XXX );
			JL_S_ASSERT( keyLength >= (size_t)cipher->min_key_length && keyLength <= (size_t)cipher->max_key_length, "Invalid key length (need [%d,%d]  bytes)", cipher->min_key_length, cipher->max_key_length );
//			JL_S_ASSERT( IV == NULL, "Initialization vector is invalid for this mode." );
			JL_S_ASSERT( optarg == NULL, "invalid 'arg' argument for this mode." );
			err = ecb_start( cipherIndex, (unsigned char *)key, (int)keyLength, numRounds, (symmetric_ECB *)pv->symmetric_XXX );
			break;
		}
		case mode_cfb: {
			pv->symmetric_XXX = JS_malloc(cx, sizeof(symmetric_CFB));
			JL_S_ASSERT_ALLOC( pv->symmetric_XXX );
			JL_S_ASSERT( keyLength >= (size_t)cipher->min_key_length && keyLength <= (size_t)cipher->max_key_length, "Invalid key length (need [%d,%d]  bytes)", cipher->min_key_length, cipher->max_key_length );
//			JL_S_ASSERT( IVLength == cipher->block_length, "This cipher require a IV length of %d", cipher->block_length );
			JL_S_ASSERT( optarg == NULL, "invalid 'arg' argument for this mode." );
			err = cfb_start( cipherIndex, (unsigned char *)IV, (unsigned char *)key, (int)keyLength, numRounds, (symmetric_CFB *)pv->symmetric_XXX );
			break;
		}
		case mode_ofb: {
			pv->symmetric_XXX = JS_malloc(cx, sizeof(symmetric_OFB));
			JL_S_ASSERT_ALLOC( pv->symmetric_XXX );
			JL_S_ASSERT( keyLength >= (size_t)cipher->min_key_length && keyLength <= (size_t)cipher->max_key_length, "Invalid key length (need [%d,%d]  bytes)", cipher->min_key_length, cipher->max_key_length );
//			JL_S_ASSERT( IVLength == cipher->block_length, "This cipher require a IV length of %d", cipher->block_length );
			JL_S_ASSERT( optarg == NULL, "invalid 'arg' argument for this mode." );
			err = ofb_start( cipherIndex, (unsigned char *)IV, (unsigned char *)key, (int)keyLength, numRounds, (symmetric_OFB *)pv->symmetric_XXX );
			break;
		}
		case mode_cbc: {
			pv->symmetric_XXX = JS_malloc(cx, sizeof(symmetric_CBC));
			JL_S_ASSERT_ALLOC( pv->symmetric_XXX );
			JL_S_ASSERT( keyLength >= (size_t)cipher->min_key_length && keyLength <= (size_t)cipher->max_key_length, "Invalid key length (need [%d,%d]  bytes)", cipher->min_key_length, cipher->max_key_length );
//			JL_S_ASSERT( IVLength == cipher->block_length, "This cipher require a IV length of %d", cipher->block_length );
			JL_S_ASSERT( optarg == NULL, "invalid 'arg' argument for this mode." );
			err = cbc_start( cipherIndex, (unsigned char *)IV, (unsigned char *)key, (int)keyLength, numRounds, (symmetric_CBC *)pv->symmetric_XXX );
			break;
		}
		case mode_ctr: {
			pv->symmetric_XXX = JS_malloc(cx, sizeof(symmetric_CTR));
			JL_S_ASSERT_ALLOC( pv->symmetric_XXX );
			JL_S_ASSERT( keyLength >= (size_t)cipher->min_key_length && keyLength <= (size_t)cipher->max_key_length, "Invalid key length (need [%d,%d]  bytes)", cipher->min_key_length, cipher->max_key_length );
//			JL_S_ASSERT( IVLength == cipher->block_length, "This cipher require a IV length of %d", cipher->block_length );
			JL_S_ASSERT( optarg == NULL, "invalid 'arg' argument for this mode." );
			err = ctr_start( cipherIndex, (unsigned char *)IV, (unsigned char *)key, (int)keyLength, numRounds, CTR_COUNTER_LITTLE_ENDIAN, (symmetric_CTR *)pv->symmetric_XXX );
			break;
		}
		case mode_lrw: {
			pv->symmetric_XXX = JS_malloc(cx, sizeof(symmetric_LRW));
			JL_S_ASSERT_ALLOC( pv->symmetric_XXX );
			JL_S_ASSERT( keyLength >= (size_t)cipher->min_key_length && keyLength <= (size_t)cipher->max_key_length, "Invalid key length (need [%d,%d]  bytes)", cipher->min_key_length, cipher->max_key_length );
//			JL_S_ASSERT( IVLength == cipher->block_length, "This cipher require a IV length of %d", cipher->block_length );
			JL_S_ASSERT( optargLength == keyLength, "The tweak length must be %d bytes length (key size)", (int)keyLength );
			err = lrw_start( cipherIndex, (unsigned char *)IV, (unsigned char *)key, (int)keyLength, (unsigned char *)optarg, numRounds, (symmetric_LRW *)pv->symmetric_XXX );
			break;
		}
		case mode_f8: {
			pv->symmetric_XXX = JS_malloc(cx, sizeof(symmetric_F8));
			JL_S_ASSERT_ALLOC( pv->symmetric_XXX );
			JL_S_ASSERT( keyLength >= (size_t)cipher->min_key_length && keyLength <= (size_t)cipher->max_key_length, "Invalid key length (need [%d,%d]  bytes)", cipher->min_key_length, cipher->max_key_length );
//			JL_S_ASSERT( IVLength == cipher->block_length, "This cipher require a IV length of %d", cipher->block_length );
			JL_S_ASSERT( optargLength > 0, "This mode need the salt argument" );
			err = f8_start( cipherIndex, (unsigned char *)IV, (unsigned char *)key, (int)keyLength, (unsigned char *)optarg, (int)optargLength, numRounds, (symmetric_F8 *)pv->symmetric_XXX );
			break;
		}
		default:
			JL_REPORT_ERROR("Invalid mode %u", mode);
	}

	if (err != CRYPT_OK) {
		
		ThrowCryptError(cx, err);
		goto bad;
	}

	JL_SetPrivate( cx, obj, pv );
	return JS_TRUE;

bad:
	if ( pv ) {
		
		jl_free(pv);
		if ( pv->symmetric_XXX )
			jl_free(pv->symmetric_XXX);
	}
	return JS_FALSE;
}

/**doc
=== Methods ===
**/

/**doc
$TOC_MEMBER $INAME
 $DATA $INAME( data )
  Encrypts the given _data_ using the current cipher.
**/
DEFINE_FUNCTION( Encrypt ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_CLASS( obj, JL_THIS_CLASS );
	JL_S_ASSERT_ARG_MIN( 1 );

	CipherPrivate *pv;
	pv = (CipherPrivate *)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( pv );

	const char *pt;
	size_t ptLength;
	JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(1), &pt, &ptLength) ); // warning: GC on the returned buffer !

	char *ct;
	ct = (char *)JS_malloc( cx, ptLength +1 );
	JL_CHK( ct );
	ct[ptLength] = '\0';

	int err;
	switch ( pv->mode ) {
		case mode_ecb:
			JL_S_ASSERT( ptLength == (size_t)pv->descriptor->block_length, "This mode require a %d bytes block of data", pv->descriptor->block_length );
			err = ecb_encrypt( (const unsigned char *)pt, (unsigned char *)ct, (unsigned long)ptLength, (symmetric_ECB *)pv->symmetric_XXX );
			break;
		case mode_cfb:
			err = cfb_encrypt( (const unsigned char *)pt, (unsigned char *)ct, (unsigned long)ptLength, (symmetric_CFB *)pv->symmetric_XXX );
			break;
		case mode_ofb:
			err = ofb_encrypt( (const unsigned char *)pt, (unsigned char *)ct, (unsigned long)ptLength, (symmetric_OFB *)pv->symmetric_XXX );
			break;
		case mode_cbc:
			JL_S_ASSERT( ptLength == (size_t)pv->descriptor->block_length, "This mode require a %d bytes block of data", pv->descriptor->block_length );
			err = cbc_encrypt( (const unsigned char *)pt, (unsigned char *)ct, (unsigned long)ptLength, (symmetric_CBC *)pv->symmetric_XXX );
			break;
		case mode_ctr:
			err = ctr_encrypt( (const unsigned char *)pt, (unsigned char *)ct, (unsigned long)ptLength, (symmetric_CTR *)pv->symmetric_XXX );
			break;
		case mode_lrw:
			JL_S_ASSERT( ptLength == (size_t)pv->descriptor->block_length, "This mode require a %d bytes block of data", pv->descriptor->block_length );
			err = lrw_encrypt( (const unsigned char *)pt, (unsigned char *)ct, (unsigned long)ptLength, (symmetric_LRW *)pv->symmetric_XXX );
			break;
		case mode_f8:
			err = f8_encrypt( (const unsigned char *)pt, (unsigned char *)ct, (unsigned long)ptLength, (symmetric_F8 *)pv->symmetric_XXX );
			break;
		default:
			JL_REPORT_ERROR("Invalid mode %u", pv->mode);
	}

	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err);

	JL_CHK( JL_NewBlob( cx, ct, ptLength, JL_RVAL ) );

	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $DATA $INAME( data )
  Decrypts the given _data_ using the current cipher.
**/
DEFINE_FUNCTION( Decrypt ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_CLASS( obj, JL_THIS_CLASS );
	JL_S_ASSERT_ARG_MIN( 1 );

	CipherPrivate *pv;
	pv = (CipherPrivate *)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( pv );

	const char *ct;
	size_t ctLength;
	JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(1), &ct, &ctLength) ); // warning: GC on the returned buffer !

	char *pt;
	pt = (char *)JS_malloc( cx, ctLength +1 );
	JL_CHK( pt );
	pt[ctLength] = '\0';

	int err;
	switch ( pv->mode ) {
		case mode_ecb:
			err = ecb_decrypt( (const unsigned char *)ct, (unsigned char *)pt, (unsigned long)ctLength, (symmetric_ECB *)pv->symmetric_XXX );
			break;
		case mode_cfb:
			err = cfb_decrypt( (const unsigned char *)ct, (unsigned char *)pt, (unsigned long)ctLength, (symmetric_CFB *)pv->symmetric_XXX );
			break;
		case mode_ofb:
			err = ofb_decrypt( (const unsigned char *)ct, (unsigned char *)pt, (unsigned long)ctLength, (symmetric_OFB *)pv->symmetric_XXX );
			break;
		case mode_cbc:
			err = cbc_decrypt( (const unsigned char *)ct, (unsigned char *)pt, (unsigned long)ctLength, (symmetric_CBC *)pv->symmetric_XXX );
			break;
		case mode_ctr:
			err = ctr_decrypt( (const unsigned char *)ct, (unsigned char *)pt, (unsigned long)ctLength, (symmetric_CTR *)pv->symmetric_XXX );
			break;
		case mode_lrw:
			err = lrw_decrypt( (const unsigned char *)ct, (unsigned char *)pt, (unsigned long)ctLength, (symmetric_LRW *)pv->symmetric_XXX );
			break;
		case mode_f8:
			err = f8_decrypt( (const unsigned char *)ct, (unsigned char *)pt, (unsigned long)ctLength, (symmetric_F8 *)pv->symmetric_XXX );
			break;
		default:
			JL_REPORT_ERROR("Invalid mode %u", pv->mode);
	}

	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err);

	JL_CHK( JL_NewBlob( cx, pt, ctLength, JL_RVAL ) );

	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  Is the block length of the current cipher.
**/
DEFINE_PROPERTY( blockLength ) {

	JL_S_ASSERT_CLASS( obj, JL_THIS_CLASS );
	CipherPrivate *pv;
	pv = (CipherPrivate *)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( pv );
	*vp = INT_TO_JSVAL( pv->descriptor->block_length );
	return JS_TRUE;
	JL_BAD;
}

/*
/ **doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
** /
DEFINE_PROPERTY( keySize ) {

	JL_S_ASSERT_CLASS( obj, _class );
	CipherPrivate *pv;
	pv = (CipherPrivate *)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( pv );
	int size = 0;
	int err = pv->descriptor->keysize(&size);
	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err);
	*vp = INT_TO_JSVAL( size );
	return JS_TRUE;
	JL_BAD;
}
*/

/**doc
$TOC_MEMBER $INAME
 $STR $INAME $READONLY
  Is the name of the current cipher.
**/
DEFINE_PROPERTY( name ) {

	JL_S_ASSERT_CLASS( obj, JL_THIS_CLASS );
	CipherPrivate *pv;
	pv = (CipherPrivate *)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( pv );
	JSString *jsstr;
	jsstr = JS_NewStringCopyZ(cx, pv->descriptor->name);
	JL_CHK( jsstr );
	*vp = STRING_TO_JSVAL( jsstr );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME
  Set or get the current initialization vector of the cipher.
**/
DEFINE_PROPERTY( IVSetter ) {

	JL_S_ASSERT_CLASS( obj, JL_THIS_CLASS );
	CipherPrivate *pv;
	pv = (CipherPrivate *)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( pv );

	const char *IV;
	size_t IVLength;
	JL_CHK( JL_JsvalToStringAndLength(cx, vp, &IV, &IVLength) );

	int err;
	switch ( pv->mode ) {
		case mode_ecb:
			JL_REPORT_ERROR("No IV in ECB mode");
		case mode_cfb: {
			symmetric_CFB *tmp = (symmetric_CFB *)pv->symmetric_XXX;
			JL_S_ASSERT( IVLength == (size_t)tmp->blocklen, "This cipher require a IV length of %d", tmp->blocklen );
			err = cfb_setiv( (const unsigned char *)IV, (unsigned long)IVLength, tmp );
			if (err != CRYPT_OK)
				return ThrowCryptError(cx, err);
			break;
		}
		case mode_ofb: {
			symmetric_OFB *tmp = (symmetric_OFB *)pv->symmetric_XXX;
			JL_S_ASSERT( IVLength == (size_t)tmp->blocklen, "This cipher require a IV length of %d", tmp->blocklen );
			err = ofb_setiv( (const unsigned char *)IV, (unsigned long)IVLength, tmp );
			if (err != CRYPT_OK)
				return ThrowCryptError(cx, err);
			break;
		}
		case mode_cbc: {
			symmetric_CBC *tmp = (symmetric_CBC *)pv->symmetric_XXX;
			JL_S_ASSERT( IVLength == (size_t)tmp->blocklen, "This cipher require a IV length of %d", tmp->blocklen );
			err = cbc_setiv( (const unsigned char *)IV, (unsigned long)IVLength, tmp );
			if (err != CRYPT_OK)
				return ThrowCryptError(cx, err);
			break;
		}
		case mode_ctr: {
			symmetric_CTR *tmp = (symmetric_CTR *)pv->symmetric_XXX;
			JL_S_ASSERT( IVLength == (size_t)tmp->blocklen, "This cipher require a IV length of %d", tmp->blocklen );
			err = ctr_setiv( (const unsigned char *)IV, (unsigned long)IVLength, tmp );
			if (err != CRYPT_OK)
				return ThrowCryptError(cx, err);
			break;
		}
		case mode_lrw: {
			symmetric_LRW *tmp = (symmetric_LRW *)pv->symmetric_XXX;
			JL_S_ASSERT( IVLength == 16, "This cipher require a IV length of %d", 16 );
			err = lrw_setiv( (const unsigned char *)IV, (unsigned long)IVLength, tmp );
			if (err != CRYPT_OK)
				return ThrowCryptError(cx, err);
			break;
		}
		case mode_f8: {
			symmetric_F8 *tmp = (symmetric_F8 *)pv->symmetric_XXX;
			JL_S_ASSERT( IVLength == (size_t)tmp->blocklen, "This cipher require a IV length of %d", tmp->blocklen );
			err = f8_setiv( (const unsigned char *)IV, (unsigned long)IVLength, tmp );
			if (err != CRYPT_OK)
				return ThrowCryptError(cx, err);
			break;
		}
		default:
			JL_REPORT_ERROR("Invalid mode %u", pv->mode);
	}

	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( IVGetter ) {

	JL_S_ASSERT_CLASS( obj, JL_THIS_CLASS );
	CipherPrivate *pv;
	pv = (CipherPrivate *)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( pv );

	char *IV;
	IV = NULL;
	unsigned long IVLength;

	int err;
	switch ( pv->mode ) {

		case mode_ecb:
			JL_REPORT_ERROR("No IV in ECB mode");
		case mode_cfb: {
			symmetric_CFB *tmp = (symmetric_CFB *)pv->symmetric_XXX;
			IVLength = tmp->blocklen;
			IV = (char*)JS_malloc( cx, IVLength );
			JL_CHK( IV );
			err = cfb_getiv( (unsigned char *)IV, &IVLength, tmp );
			break;
		}
		case mode_ofb: {
			symmetric_OFB *tmp = (symmetric_OFB *)pv->symmetric_XXX;
			IVLength = tmp->blocklen;
			IV = (char*)JS_malloc( cx, IVLength );
			JL_CHK( IV );
			err = ofb_getiv( (unsigned char *)IV, &IVLength, tmp );
			break;
		}
		case mode_cbc: {
			symmetric_CBC *tmp = (symmetric_CBC *)pv->symmetric_XXX;
			IVLength = tmp->blocklen;
			IV = (char*)JS_malloc( cx, IVLength );
			JL_CHK( IV );
			err = cbc_getiv( (unsigned char *)IV, &IVLength, tmp );
			break;
		}
		case mode_ctr: {
			symmetric_CTR *tmp = (symmetric_CTR *)pv->symmetric_XXX;
			IVLength = tmp->blocklen;
			IV = (char*)JS_malloc( cx, IVLength );
			JL_CHK( IV );
			err = ctr_getiv( (unsigned char *)IV, &IVLength, tmp );
			break;
		}
		case mode_lrw: {
			symmetric_LRW *tmp = (symmetric_LRW *)pv->symmetric_XXX;
			IVLength = 16;
			IV = (char*)JS_malloc( cx, IVLength );
			JL_CHK( IV );
			err = lrw_getiv( (unsigned char *)IV, &IVLength, tmp );
			break;
		}
		case mode_f8: {
			symmetric_F8 *tmp = (symmetric_F8 *)pv->symmetric_XXX;
			IVLength = tmp->blocklen;
			IV = (char*)JS_malloc( cx, IVLength );
			JL_CHK( IV );
			err = f8_getiv( (unsigned char *)IV, &IVLength, tmp );
			break;
		}
		default:
			JL_REPORT_ERROR("Invalid mode %u", pv->mode);
	}
	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err);

	JL_CHK( JL_NewBlob( cx, IV, IVLength, vp ) );

	return JS_TRUE;
	JL_BAD;
}


/**doc
=== Static properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $OBJ $INAME $READONLY
  Contains the list of all available ciphers and their feature. The list is a javascript object that map cipher names (key) with another object (value) that contain information.
**/
DEFINE_PROPERTY( list ) {

	js::AutoObjectRooter tvr(cx, JS_NewObject(cx, NULL, NULL, NULL)); // (TBD) remove this workaround. cf. bz495422 || bz397177

	int i;
	jsval tmp;
	LTC_MUTEX_LOCK(&ltc_cipher_mutex);
	for ( i = 0; cipher_is_valid(i) == CRYPT_OK; ++i ) {

		JSObject *desc = JS_NewObject( cx, NULL, NULL, NULL );
		tmp = OBJECT_TO_JSVAL( desc );
		JS_SetProperty( cx, tvr.object(), cipher_descriptor[i].name, &tmp );

		tmp = INT_TO_JSVAL( cipher_descriptor[i].min_key_length );
		JS_SetProperty( cx, desc, "minKeyLength", &tmp );
		tmp = INT_TO_JSVAL( cipher_descriptor[i].max_key_length );
		JS_SetProperty( cx, desc, "maxKeyLength", &tmp );
		tmp = INT_TO_JSVAL( cipher_descriptor[i].block_length );
		JS_SetProperty( cx, desc, "blockLength", &tmp );
		tmp = INT_TO_JSVAL( cipher_descriptor[i].default_rounds );
		JS_SetProperty( cx, desc, "defaultRounds", &tmp );
	}
	LTC_MUTEX_UNLOCK(&ltc_cipher_mutex);

	*vp = OBJECT_TO_JSVAL(tvr.object());
	return JL_StoreProperty(cx, obj, id, vp, true);
}


CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION( Encrypt )
		FUNCTION( Decrypt )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY( IV )
		PROPERTY_READ( blockLength )
//		PROPERTY_READ( keySize )
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
