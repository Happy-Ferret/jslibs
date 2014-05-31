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
$SVN_REVISION $Revision: 3533 $
**/
BEGIN_CLASS( Cipher )

ALWAYS_INLINE void
FinalizeCipher( JS::HandleObject obj, bool wipe ) {

	CipherPrivate *pv = (CipherPrivate *)JL_GetPrivate( obj );
	if ( pv ) {

		size_t size;
		IFDEBUG( size = 0 );
		int err;
		switch ( pv->mode ) {
			case mode_ecb:
				size = sizeof(symmetric_ECB);
				err = ecb_done( (symmetric_ECB *)pv->symmetric_XXX );
				break;
			case mode_cfb:
				size = sizeof(symmetric_CFB);
				err = cfb_done( (symmetric_CFB *)pv->symmetric_XXX );
				break;
			case mode_ofb:
				size = sizeof(symmetric_OFB);
				err = ofb_done( (symmetric_OFB *)pv->symmetric_XXX );
				break;
			case mode_cbc:
				size = sizeof(symmetric_CBC);
				err = cbc_done( (symmetric_CBC *)pv->symmetric_XXX );
				break;
			case mode_ctr:
				size = sizeof(symmetric_CTR);
				err = ctr_done( (symmetric_CTR *)pv->symmetric_XXX );
				break;
			case mode_lrw:
				size = sizeof(symmetric_LRW);
				err = lrw_done( (symmetric_LRW *)pv->symmetric_XXX );
				break;
			case mode_f8:
				size = sizeof(symmetric_F8);
				err = f8_done( (symmetric_F8 *)pv->symmetric_XXX );
				break;
			default:
				ASSERT(false);
		}
	//	if (err != CRYPT_OK)
	//		return ThrowCryptError(cx, err);
		if ( wipe ) {

			zeromem(pv->symmetric_XXX, size);
			jl_free(pv->symmetric_XXX);
			zeromem(pv, sizeof(CipherPrivate));
			jl_free(pv);
		} else {
		
			jl_free(pv->symmetric_XXX);
			jl_free(pv);
		}
	}
}

DEFINE_FINALIZE() {

	if ( jl::Host::getHost(fop->runtime()).hostRuntime().skipCleanup() )
		return;

	JS::RootedObject o(fop->runtime(), obj);
	FinalizeCipher(o, false);
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
	
	JL_DEFINE_ARGS;

	CipherPrivate *pv = NULL; // see. bad label
	jl::BufString modeName, cipherName, key, IV, optarg;

	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;
	JL_ASSERT_ARGC_MIN( 3 );

	JL_CHK( jl::getValue(cx, JL_ARG(1), &modeName) ); // warning: GC on the returned buffer !

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
		JL_ERR( E_ARG, E_NUM(1), E_INVALID, E_SEP, E_NAME(modeName), E_NOTSUPPORTED );

	JL_CHK( jl::getValue(cx, JL_ARG(2), &cipherName) ); // warning: GC on the returned buffer !
	JL_CHK( jl::getValue(cx, JL_ARG(3), &key) ); // warning: GC on the returned buffer !

	if ( argc >= 4 && !JL_ARG(4).isUndefined() )
		JL_CHK( jl::getValue(cx, JL_ARG(4), &IV) ); // warning: GC on the returned buffer !
	else
		IV.setEmpty();

	if ( argc >= 5 && !JL_ARG(5).isUndefined() )
		JL_CHK( jl::getValue(cx, JL_ARG(5), &optarg) ); // warning: GC on the returned buffer !

   int numRounds;
   numRounds = 0; // default value, us a default number of rounds.
	if ( argc >= 6 && !JL_ARG(6).isUndefined() )
		JL_CHK( jl::getValue(cx, JL_ARG(6), &numRounds) );

	pv = (CipherPrivate*)jl_malloc(sizeof(CipherPrivate));
	JL_CHK( pv );
	pv->symmetric_XXX = NULL; // see. bad: label

	pv->mode = mode;

	int cipherIndex;
	cipherIndex = find_cipher(cipherName);
	JL_ASSERT( cipherIndex != -1, E_STR("cipher"), E_NAME(cipherName), E_NOTFOUND );

	ltc_cipher_descriptor *cipher;
	cipher = &cipher_descriptor[cipherIndex];

	pv->descriptor = cipher;
	JL_ASSERT( cipher->test() == CRYPT_OK, E_LIB, E_STR("libtomcrypt"), E_INTERNAL, E_SEP, E_STR(cipherName), E_STR("test"), E_FAILURE );

	int err;
	switch ( mode ) {
		case mode_ecb: {
			pv->symmetric_XXX = jl_malloc(sizeof(symmetric_ECB));
			JL_ASSERT_ALLOC( pv->symmetric_XXX );
			JL_ASSERT_RANGE( (int)key.length(), cipher->min_key_length, cipher->max_key_length, "key.length" );
//			JL_ASSERT( IV == NULL, "Initialization vector is invalid for this mode." );
			JL_ASSERT_WARN( !optarg, E_ARG, E_NUM(5), E_IGNORED );
			err = ecb_start( cipherIndex, key.toData<const uint8_t*>(), (int)key.length(), numRounds, (symmetric_ECB *)pv->symmetric_XXX );
			break;
		}
		case mode_cfb: {
			pv->symmetric_XXX = jl_malloc(sizeof(symmetric_CFB));
			JL_ASSERT_ALLOC( pv->symmetric_XXX );
			JL_ASSERT_RANGE( (int)key.length(), cipher->min_key_length, cipher->max_key_length, "key.length" );
//			JL_ASSERT( IVLength == cipher->block_length, "This cipher require a IV length of %d", cipher->block_length );
			JL_ASSERT_WARN( !optarg, E_ARG, E_NUM(5), E_IGNORED );
			err = cfb_start( cipherIndex, IV.toData<const uint8_t*>(), key.toData<const uint8_t*>(), (int)key.length(), numRounds, (symmetric_CFB *)pv->symmetric_XXX );
			break;
		}
		case mode_ofb: {
			pv->symmetric_XXX = jl_malloc(sizeof(symmetric_OFB));
			JL_ASSERT_ALLOC( pv->symmetric_XXX );
			JL_ASSERT_RANGE( (int)key.length(), cipher->min_key_length, cipher->max_key_length, "key.length" );
//			JL_ASSERT( IVLength == cipher->block_length, "This cipher require a IV length of %d", cipher->block_length );
			JL_ASSERT_WARN( !optarg, E_ARG, E_NUM(5), E_IGNORED );
			err = ofb_start( cipherIndex, IV.toData<const uint8_t*>(), key.toData<const uint8_t*>(), (int)key.length(), numRounds, (symmetric_OFB *)pv->symmetric_XXX );
			break;
		}
		case mode_cbc: {
			pv->symmetric_XXX = jl_malloc(sizeof(symmetric_CBC));
			JL_ASSERT_ALLOC( pv->symmetric_XXX );
			JL_ASSERT_RANGE( (int)key.length(), cipher->min_key_length, cipher->max_key_length, "key.length" );
//			JL_ASSERT( IVLength == cipher->block_length, "This cipher require a IV length of %d", cipher->block_length );
			JL_ASSERT_WARN( !optarg, E_ARG, E_NUM(5), E_IGNORED );
			err = cbc_start( cipherIndex, IV.toData<const uint8_t*>(), key.toData<const uint8_t*>(), (int)key.length(), numRounds, (symmetric_CBC *)pv->symmetric_XXX );
			break;
		}
		case mode_ctr: {
			pv->symmetric_XXX = jl_malloc(sizeof(symmetric_CTR));
			JL_ASSERT_ALLOC( pv->symmetric_XXX );
			JL_ASSERT_RANGE( (int)key.length(), cipher->min_key_length, cipher->max_key_length, "key.length" );
//			JL_ASSERT( IVLength == cipher->block_length, "This cipher require a IV length of %d", cipher->block_length );
			JL_ASSERT_WARN( !optarg, E_ARG, E_NUM(5), E_IGNORED );
			err = ctr_start( cipherIndex, IV.toData<const uint8_t*>(), key.toData<const uint8_t*>(), (int)key.length(), numRounds, CTR_COUNTER_LITTLE_ENDIAN, (symmetric_CTR *)pv->symmetric_XXX );
			break;
		}
		case mode_lrw: {
			pv->symmetric_XXX = jl_malloc(sizeof(symmetric_LRW));
			JL_ASSERT_ALLOC( pv->symmetric_XXX );
			JL_ASSERT_RANGE( (int)key.length(), cipher->min_key_length, cipher->max_key_length, "key.length" );
//			JL_ASSERT( IVLength == cipher->block_length, "This cipher require a IV length of %d", cipher->block_length );
			JL_ASSERT( optarg && optarg.length() == key.length(), E_ARG, E_NUM(5), E_SEP, E_STR("tweak"), E_LENGTH, E_NUM((int)key.length()) );
			err = lrw_start( cipherIndex, IV.toData<const uint8_t*>(), key.toData<const uint8_t*>(), (int)key.length(), optarg.toData<const uint8_t*>(), numRounds, (symmetric_LRW *)pv->symmetric_XXX );
			break;
		}
		case mode_f8: {
			pv->symmetric_XXX = jl_malloc(sizeof(symmetric_F8));
			JL_ASSERT_ALLOC( pv->symmetric_XXX );
			JL_ASSERT_RANGE( (int)key.length(), cipher->min_key_length, cipher->max_key_length, "key.length" );
//			JL_ASSERT( IVLength == cipher->block_length, "This cipher require a IV length of %d", cipher->block_length );
			JL_ASSERT( optarg.lengthOrZero(), E_ARG, E_NUM(5), E_COMMENT("salt"), E_REQUIRED );
			err = f8_start( cipherIndex, IV.toData<const uint8_t*>(), key.toData<const uint8_t*>(), (int)key.length(), optarg.toDataOrNull<const uint8_t*>(), (int)optarg.lengthOrZero(), numRounds, (symmetric_F8 *)pv->symmetric_XXX );
			break;
		}
		default:
			IFDEBUG( err = 0 );
			ASSERT(false);
	}

	if ( err != CRYPT_OK )
		JL_CHK( ThrowCryptError(cx, err) );

	JL_SetPrivate(JL_OBJ, pv);
	return true;

bad:
	if ( pv ) {
		
		jl_free(pv->symmetric_XXX); // free a NULL pointer is legal.
		jl_free(pv);
	}
	return false;
}

/**doc
=== Methods ===
**/


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Cleanup and free internal data.
  $H note
   This object may contain sensitive data.
**/
DEFINE_FUNCTION( wipe ) {

	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();

	FinalizeCipher(JL_OBJ, true);
	JL_SetPrivate(JL_OBJ, NULL);
	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $DATA $INAME( data )
  Encrypts the given _data_ using the current cipher.
**/
DEFINE_FUNCTION( encrypt ) {

	JL_DEFINE_ARGS;

	jl::BufBase buffer;
	jl::BufString pt;

	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN( 1 );

	CipherPrivate *pv;
	pv = (CipherPrivate *)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	JL_CHK( jl::getValue(cx, JL_ARG(1), &pt) );

	//uint8_t *ct;
	//ct = JL_NewBuffer(cx, pt.Length(), JL_RVAL);
	//JL_CHK( ct );

	buffer.alloc(pt.length(), true);
	JL_ASSERT_ALLOC(buffer);

	int err;
	switch ( pv->mode ) {
		case mode_ecb:
			JL_ASSERT( buffer.allocSize() == (size_t)pv->descriptor->block_length, E_DATA, E_LENGTH, E_NUM(pv->descriptor->block_length) );
			err = ecb_encrypt( pt.toData<const uint8_t*>(), buffer.data(), buffer.allocSize(), (symmetric_ECB *)pv->symmetric_XXX );
			break;
		case mode_cfb:
			err = cfb_encrypt( pt.toData<const uint8_t*>(), buffer.data(), buffer.allocSize(), (symmetric_CFB *)pv->symmetric_XXX );
			break;
		case mode_ofb:
			err = ofb_encrypt( pt.toData<const uint8_t*>(), buffer.data(), buffer.allocSize(), (symmetric_OFB *)pv->symmetric_XXX );
			break;
		case mode_cbc:
			JL_ASSERT( buffer.allocSize() == (size_t)pv->descriptor->block_length, E_DATA, E_LENGTH, E_NUM(pv->descriptor->block_length) );
			err = cbc_encrypt( pt.toData<const uint8_t*>(), buffer.data(), buffer.allocSize(), (symmetric_CBC *)pv->symmetric_XXX );
			break;
		case mode_ctr:
			err = ctr_encrypt( pt.toData<const uint8_t*>(), buffer.data(), buffer.allocSize(), (symmetric_CTR *)pv->symmetric_XXX );
			break;
		case mode_lrw:
			JL_ASSERT( buffer.allocSize() == (size_t)pv->descriptor->block_length, E_DATA, E_LENGTH, E_NUM(pv->descriptor->block_length) );
			err = lrw_encrypt( pt.toData<const uint8_t*>(), buffer.data(), buffer.allocSize(), (symmetric_LRW *)pv->symmetric_XXX );
			break;
		case mode_f8:
			err = f8_encrypt( pt.toData<const uint8_t*>(), buffer.data(), buffer.allocSize(), (symmetric_F8 *)pv->symmetric_XXX );
			break;
		default:
			IFDEBUG( err = 0 );
			ASSERT(false);
	}

	if ( err != CRYPT_OK )
		return ThrowCryptError(cx, err);

	JL_CHK( BlobCreate(cx, buffer, JL_RVAL) );

	//ct[pt.Length()] = '\0';
	//JL_CHK( JL_NewBlob( cx, ct, pt.Length(), JL_RVAL ) );

//	JL_CHK( JL_NativeToJsval(cx, ct, pt.Length(), JL_RVAL ) );
//	jl_free(ct);

	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $DATA $INAME( data )
  Decrypts the given _data_ using the current cipher.
**/
DEFINE_FUNCTION( decrypt ) {

	JL_DEFINE_ARGS;

	jl::BufBase buffer;
	jl::BufString ct;

	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN( 1 );

	CipherPrivate *pv;
	pv = (CipherPrivate *)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

//	const char *ct;
//	size_t ctLength;
//	JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(1), &ct, &ctLength) ); // warning: GC on the returned buffer !
	JL_CHK( jl::getValue(cx, JL_ARG(1), &ct) );

	//uint8_t *pt;
	//pt = JL_NewBuffer(cx, ct.Length(), JL_RVAL);
	//JL_CHK( pt );
	buffer.alloc(ct.length(), true);
	JL_ASSERT_ALLOC(buffer);

	int err;
	switch ( pv->mode ) {
		case mode_ecb:
			err = ecb_decrypt( ct.toData<const uint8_t*>(), buffer.data(), ct.length(), (symmetric_ECB *)pv->symmetric_XXX );
			break;
		case mode_cfb:
			err = cfb_decrypt( ct.toData<const uint8_t*>(), buffer.data(), ct.length(), (symmetric_CFB *)pv->symmetric_XXX );
			break;
		case mode_ofb:
			err = ofb_decrypt( ct.toData<const uint8_t*>(), buffer.data(), ct.length(), (symmetric_OFB *)pv->symmetric_XXX );
			break;
		case mode_cbc:
			err = cbc_decrypt( ct.toData<const uint8_t*>(), buffer.data(), ct.length(), (symmetric_CBC *)pv->symmetric_XXX );
			break;
		case mode_ctr:
			err = ctr_decrypt( ct.toData<const uint8_t*>(), buffer.data(), ct.length(), (symmetric_CTR *)pv->symmetric_XXX );
			break;
		case mode_lrw:
			err = lrw_decrypt( ct.toData<const uint8_t*>(), buffer.data(), ct.length(), (symmetric_LRW *)pv->symmetric_XXX );
			break;
		case mode_f8:
			err = f8_decrypt( ct.toData<const uint8_t*>(), buffer.data(), ct.length(), (symmetric_F8 *)pv->symmetric_XXX );
			break;
		default:
			IFDEBUG( err = 0 );
			ASSERT(false);
	}

	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err);

	//pt[ct.Length()] = '\0';
	//JL_CHK( JL_NewBlob( cx, pt, ct.Length(), JL_RVAL ) );

	JL_CHK( BlobCreate(cx, buffer, JL_RVAL) );
	return true;
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
DEFINE_PROPERTY_GETTER( blockLength ) {

	JL_DEFINE_PROP_ARGS;

	JL_ASSERT_THIS_INSTANCE();

	CipherPrivate *pv;
	pv = (CipherPrivate *)JL_GetPrivate( obj );
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	JL_RVAL.setInt32(pv->descriptor->block_length);
	return true;
	JL_BAD;
}

/*
/ **doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
** /
DEFINE_PROPERTY( keySize ) {

	JL_ASSERT_INSTANCE( obj, _class );
	CipherPrivate *pv;
	pv = (CipherPrivate *)JL_GetPrivate( obj );
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	int size = 0;
	int err = pv->descriptor->keysize(&size);
	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err);
	*vp = INT_TO_JSVAL( size );
	return true;
	JL_BAD;
}
*/

/**doc
$TOC_MEMBER $INAME
 $STR $INAME $READONLY
  Is the name of the current cipher.
**/
DEFINE_PROPERTY_GETTER( name ) {

	JL_DEFINE_PROP_ARGS;

	JL_ASSERT_THIS_INSTANCE();

	CipherPrivate *pv;
	pv = (CipherPrivate *)JL_GetPrivate( obj );
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	JSString *jsstr;
	jsstr = JS_NewStringCopyZ(cx, pv->descriptor->name);
	JL_CHK( jsstr );
	JL_RVAL.setString(jsstr);
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME
  Set or get the current initialization vector of the cipher.
**/
DEFINE_PROPERTY_SETTER( IV ) {

	JL_DEFINE_PROP_ARGS;

	jl::BufString IV;

	JL_ASSERT_THIS_INSTANCE();

	CipherPrivate *pv;
	pv = (CipherPrivate *)JL_GetPrivate( obj );
	JL_ASSERT_THIS_OBJECT_STATE( pv );

//	const char *IV;
//	size_t IVLength;
//	JL_CHK( JL_JsvalToStringAndLength(cx, vp, &IV, &IVLength) );
	JL_CHK( jl::getValue(cx, JL_RVAL, &IV) );

	int err;
	switch ( pv->mode ) {
		case mode_ecb:
			JL_WARN( E_VALUE, E_IGNORED, E_SEP, E_STR("IV"), E_NOTSUPPORTED, E_COMMENT("ECB") );
			break;
		case mode_cfb: {
			symmetric_CFB *tmp = (symmetric_CFB *)pv->symmetric_XXX;
			JL_ASSERT( IV.length() == (size_t)tmp->blocklen, E_STR("IV"), E_LENGTH, E_NUM(tmp->blocklen) );
			err = cfb_setiv( IV.toData<const uint8_t *>(), IV.length(), tmp );
			if (err != CRYPT_OK)
				return ThrowCryptError(cx, err);
			break;
		}
		case mode_ofb: {
			symmetric_OFB *tmp = (symmetric_OFB *)pv->symmetric_XXX;
			JL_ASSERT( IV.length() == (size_t)tmp->blocklen, E_STR("IV"), E_LENGTH, E_NUM(tmp->blocklen) );
			err = ofb_setiv( IV.toData<const uint8_t *>(), IV.length(), tmp );
			if (err != CRYPT_OK)
				return ThrowCryptError(cx, err);
			break;
		}
		case mode_cbc: {
			symmetric_CBC *tmp = (symmetric_CBC *)pv->symmetric_XXX;
			JL_ASSERT( IV.length() == (size_t)tmp->blocklen, E_STR("IV"), E_LENGTH, E_NUM(tmp->blocklen) );
			err = cbc_setiv( IV.toData<const uint8_t *>(), IV.length(), tmp );
			if (err != CRYPT_OK)
				return ThrowCryptError(cx, err);
			break;
		}
		case mode_ctr: {
			symmetric_CTR *tmp = (symmetric_CTR *)pv->symmetric_XXX;
			JL_ASSERT( IV.length() == (size_t)tmp->blocklen, E_STR("IV"), E_LENGTH, E_NUM(tmp->blocklen) );
			err = ctr_setiv( IV.toData<const uint8_t *>(), IV.length(), tmp );
			if (err != CRYPT_OK)
				return ThrowCryptError(cx, err);
			break;
		}
		case mode_lrw: {
			symmetric_LRW *tmp = (symmetric_LRW *)pv->symmetric_XXX;
			JL_ASSERT( IV.length() == 16, E_STR("IV"), E_LENGTH, E_NUM(16) ); // (TBD) 16 == tmp->blocklen ???
			err = lrw_setiv( IV.toData<const uint8_t *>(), IV.length(), tmp );
			if (err != CRYPT_OK)
				return ThrowCryptError(cx, err);
			break;
		}
		case mode_f8: {
			symmetric_F8 *tmp = (symmetric_F8 *)pv->symmetric_XXX;
			JL_ASSERT( IV.length() == (size_t)tmp->blocklen, E_STR("IV"), E_LENGTH, E_NUM(tmp->blocklen) );
			err = f8_setiv( IV.toData<const uint8_t *>(), IV.length(), tmp );
			if (err != CRYPT_OK)
				return ThrowCryptError(cx, err);
			break;
		}
		default:
			ASSERT(false);
	}

	return true;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( IV ) {

	JL_DEFINE_PROP_ARGS;

	jl::BufString IV;

	JL_ASSERT_THIS_INSTANCE();

	CipherPrivate *pv;
	pv = (CipherPrivate *)JL_GetPrivate( obj );
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	//uint8_t *IV;
	//IV = NULL;

	unsigned long IVLength;

	int err;
	switch ( pv->mode ) {

		case mode_ecb:
			JL_WARN( E_STR("IV"), E_NOTSUPPORTED, E_COMMENT("ECB") );
			JL_RVAL.setUndefined();
			return true;
		case mode_cfb: {
			symmetric_CFB *tmp = (symmetric_CFB *)pv->symmetric_XXX;
			IVLength = tmp->blocklen;
			//IV = JL_NewBuffer(cx, IVLength, vp);
			//JL_CHK( IV );
			err = cfb_getiv( IV.data(), &IVLength, tmp );
			break;
		}
		case mode_ofb: {
			symmetric_OFB *tmp = (symmetric_OFB *)pv->symmetric_XXX;
			IVLength = tmp->blocklen;
			IV.alloc(IVLength);
			JL_ASSERT_ALLOC( IV );
			//IV = JL_NewBuffer(cx, IVLength, vp);
			//JL_CHK( IV );
			err = ofb_getiv( IV.data(), &IVLength, tmp );
			break;
		}
		case mode_cbc: {
			symmetric_CBC *tmp = (symmetric_CBC *)pv->symmetric_XXX;
			IVLength = tmp->blocklen;
			IV.alloc(IVLength);
			JL_ASSERT_ALLOC( IV );
			//IV = JL_NewBuffer(cx, IVLength, vp);
			//JL_CHK( IV );
			err = cbc_getiv( IV.data(), &IVLength, tmp );
			break;
		}
		case mode_ctr: {
			symmetric_CTR *tmp = (symmetric_CTR *)pv->symmetric_XXX;
			IVLength = tmp->blocklen;
			IV.alloc(IVLength);
			JL_ASSERT_ALLOC( IV );
//			IV = JL_NewBuffer(cx, IVLength, vp);
//			JL_CHK( IV );
			err = ctr_getiv( IV.data(), &IVLength, tmp );
			break;
		}
		case mode_lrw: {
			symmetric_LRW *tmp = (symmetric_LRW *)pv->symmetric_XXX;
			IVLength = 16;
			IV.alloc(IVLength);
			JL_ASSERT_ALLOC( IV );
//			IV = JL_NewBuffer(cx, IVLength, vp);
//			JL_CHK( IV );
			err = lrw_getiv( IV.data(), &IVLength, tmp );
			break;
		}
		case mode_f8: {
			symmetric_F8 *tmp = (symmetric_F8 *)pv->symmetric_XXX;
			IVLength = tmp->blocklen;
			IV.alloc(IVLength);
			JL_ASSERT_ALLOC( IV );
//			IV = JL_NewBuffer(cx, IVLength, vp);
//			JL_CHK( IV );
			err = f8_getiv( IV.data(), &IVLength, tmp );
			break;
		}
		default:
			IFDEBUG( IVLength = 0 );
			IFDEBUG( err = 0 );
			ASSERT(false);
	}

	if ( err != CRYPT_OK )
		return ThrowCryptError(cx, err);

	//IV[IVLength] = '\0';
	//JL_CHK( JL_NewBlob( cx, IV, IVLength, vp ) );

	IV.setUsed(IVLength);
	JL_CHK( BlobCreate(cx, IV, JL_RVAL) );

	return true;
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
DEFINE_PROPERTY_GETTER( list ) {

	JL_DEFINE_PROP_ARGS;

	JS::RootedObject list(cx, JL_NewObj(cx));
	int i;
	LTC_MUTEX_LOCK(&ltc_cipher_mutex);
	for ( i = 0; cipher_is_valid(i) == CRYPT_OK; ++i ) {

		JS::RootedObject desc(cx, JL_NewObj(cx));
		JL_CHK( jl::setProperty(cx, list, cipher_descriptor[i].name, desc) );
		JL_CHK( jl::setProperty(cx, desc, "minKeyLength", cipher_descriptor[i].min_key_length) );
		JL_CHK( jl::setProperty(cx, desc, "maxKeyLength", cipher_descriptor[i].max_key_length) );
		JL_CHK( jl::setProperty(cx, desc, "blockLength", cipher_descriptor[i].block_length) );
		JL_CHK( jl::setProperty(cx, desc, "defaultRounds", cipher_descriptor[i].default_rounds) );
	}
	LTC_MUTEX_UNLOCK(&ltc_cipher_mutex);

	JL_RVAL.setObject(*list);
	return jl::StoreProperty(cx, obj, id, vp, true); // create the list and store it once for all.
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision: 3533 $"))
	HAS_PRIVATE
	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION( wipe )
		FUNCTION( encrypt )
		FUNCTION( decrypt )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY( IV )
		PROPERTY_GETTER( blockLength )
//		PROPERTY_GETTER( keySize )
		PROPERTY_GETTER( name )
	END_PROPERTY_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_GETTER( list )
	END_STATIC_PROPERTY_SPEC

END_CLASS


/****************************************************************

CTR ( Counter CryptMode )
	http://en.wikipedia.org/wiki/Counter_mode (fr: http://fr.wikipedia.org/wiki/CryptMode_d%27op%C3%A9ration_%28cryptographie%29 )

*/
