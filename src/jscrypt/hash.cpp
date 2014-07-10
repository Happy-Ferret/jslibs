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


struct HashPrivate {
	ltc_hash_descriptor *descriptor;
	hash_state state;
	unsigned long inputLength;
	bool isValid;
};


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
 This class is used to create block Hash objects.
**/
BEGIN_CLASS( Hash )

DEFINE_FINALIZE() {

	if ( jl::Host::getHost(fop->runtime()).hostRuntime().skipCleanup() )
		return;
	HashPrivate *pv = (HashPrivate *)js::GetObjectPrivate(obj);
	jl_free(pv); // NULL is supported but is quite rare.
}

/**doc
$TOC_MEMBER $INAME
 $INAME( hashName )
  Creates a new hash.
  _hashName_ is a string that contains the name of the hash:
   * whirlpool
   * sha512
   * sha384
   * sha256
   * sha224
   * sha1
   * md5
   * md4
   * md2
   * tiger
   * rmd128
   * rmd160
   * rmd256
   * rmd320  
   * chc_hash
  $H note
   chc_hash is a special hash that allows to create a hash from a cipher (Cipher Hash Construction).
   See CipherHash() static function.
**/
DEFINE_CONSTRUCTOR() {

	JL_DEFINE_ARGS;

	HashPrivate *pv = NULL;
	jl::BufString hashName;

	JL_ASSERT_ARGC_MIN( 1 );
	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

	JL_CHK( jl::getValue(cx, JL_ARG(1), &hashName) );

	int hashIndex;
	hashIndex = find_hash(hashName);
	JL_ASSERT( hashIndex != -1, E_STR("hash"), E_NAME(hashName), E_NOTFOUND );

	pv = (HashPrivate*)jl_malloc(sizeof(HashPrivate));
	JL_CHK( pv );

	pv->descriptor = &hash_descriptor[hashIndex];

	JL_ASSERT( pv->descriptor->test() == CRYPT_OK, E_LIB, E_STR("libtomcrypt"), E_INTERNAL, E_SEP, E_STR(hashName), E_STR("test"), E_FAILURE );

	int err;
	err = pv->descriptor->init(&pv->state);
	if ( err != CRYPT_OK )
		return ThrowCryptError(cx, err);
	pv->inputLength = 0;
	pv->isValid = true;

	JL_SetPrivate(JL_OBJ, pv);
	return true;

bad:
	jl_free(pv);
	return false;
}


/* *doc
$TOC_MEMBER $INAME
 $DATA $INAME( data )
  This is the call operator of the object. It simplifies the usage of hashes and allows a digest calculation in one go.
  When called with a string as argument, it processes a block of memory though the hash.
  In this case, the function returns the hash of the whole given data.
  $H beware
   Using this methode to compute a digest automaticaly resets previous state let by Init(), Process() or Done().
  $H example
  {{{
  loadModule('jsstd');
  loadModule('jscrypt');
  var md5 = new Hash('md5');
  print( hexEncode( md5('foobar') ) ); // prints: 3858F62230AC3C915F300C664312C63F
  }}}
** /
DEFINE_CALL() {

	JL_DEFINE_ARGS;

	JLData in;

	JL_DEFINE_CALL_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_STRING(1);

	HashPrivate *pv;
	pv = (HashPrivate *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	int err;

	uint8_t *out;
	out = JL_NewBuffer(cx, pv->descriptor->hashsize, JL_RVAL);
	JL_CHK( out );

	JL_CHK( jl::getValue(cx, JL_ARG(1), &in) );

	err = pv->descriptor->init(&pv->state);
	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err);

	err = pv->descriptor->process(&pv->state, (const unsigned char *)in.GetConstStr(), (unsigned long)in.Length());
	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err);

	err = pv->descriptor->done(&pv->state, (unsigned char*)out);
	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err);

	return true;
	JL_BAD;
}
*/

/**doc
=== Methods ===
**/

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( [data = undefined] )
  Process a block of data though the hash. If _data_ is ommitted or undefined, the function does nothing.
**/
DEFINE_FUNCTION( write ) {

	JL_DEFINE_ARGS;

	jl::BufString in;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC(1);

	HashPrivate *pv;
	pv = (HashPrivate *)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv && pv->isValid );

	if ( JL_ARG_ISDEF(1) ) {
		
		JL_ASSERT_ARG_IS_STRING(1);
		JL_CHK( jl::getValue(cx, JL_ARG(1), &in) );

		unsigned long length = in.length();

		int err;
		err = pv->descriptor->process(&pv->state, in.toData<const uint8_t *>(), length); // Process a block of memory though the hash
		if ( err != CRYPT_OK )
			return ThrowCryptError(cx, err);

		pv->inputLength += length;
	}

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $DATA $INAME()
  Terminate the hash and get the digest in a binary format.
  $H example
  {{{
  loadModule('jsstd');
  loadModule('jscrypt');
  var md5 = new Hash('md5');
  md5.process('foo');
  md5.process('bar');
  print( hexEncode( md5.done(), '\n' ) ); // prints: 3858F62230AC3C915F300C664312C63F
  }}}
**/
DEFINE_FUNCTION( done ) {

	JL_DEFINE_ARGS;

	jl::BufBase buffer;

	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC(0);

	HashPrivate *pv;
	pv = (HashPrivate *)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv && pv->isValid );
	//unsigned long outLength;
	//outLength = pv->descriptor->hashsize;
	
	//uint8_t *out;
	//out = JL_NewBuffer(cx, outLength, JL_RVAL);

	buffer.alloc(pv->descriptor->hashsize, true);
	JL_ASSERT_ALLOC(buffer);
	
	//JL_CHK( out );
	int err;
	err = pv->descriptor->done(&pv->state, buffer.data()); // Terminate the hash to get the digest
	if ( err != CRYPT_OK )
		return ThrowCryptError(cx, err);
	
	pv->isValid = false;

	JL_CHK( BlobCreate(cx, buffer, JL_RVAL) );

	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Resets the hash state.
**/
DEFINE_FUNCTION( reset ) {

	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC(0);

	HashPrivate *pv;
	pv = (HashPrivate *)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	int err;
	err = pv->descriptor->init(&pv->state); // Initialize the hash state
	if ( err != CRYPT_OK )
		return ThrowCryptError(cx, err);
	pv->inputLength = 0;
	pv->isValid = true;

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
=== Properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $STR $INAME $READONLY
  Name of the current hash.
**/
DEFINE_PROPERTY_GETTER( name ) {

	JL_DEFINE_PROP_ARGS;

	JL_ASSERT_THIS_INSTANCE();
	HashPrivate *pv;
	pv = (HashPrivate *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	JL_CHK( jl::setValue( cx, JL_RVAL, jl::CStrSpec( pv->descriptor->name ) ) );
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  Input block size in octets.
**/
DEFINE_PROPERTY_GETTER( blockSize ) {

	JL_DEFINE_PROP_ARGS;

	JL_ASSERT_THIS_INSTANCE();
	HashPrivate *pv;
	pv = (HashPrivate *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	JL_RVAL.setInt32(pv->descriptor->blocksize);
	return true;
	JL_BAD;
}	

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  Size of the digest in octets.
**/
DEFINE_PROPERTY_GETTER( length ) {

	JL_DEFINE_PROP_ARGS;

	JL_ASSERT_THIS_INSTANCE();
	HashPrivate *pv;
	pv = (HashPrivate *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	JL_RVAL.setInt32(pv->descriptor->hashsize);
	return true;
	JL_BAD;
}	

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  Length of the processed data.
**/
DEFINE_PROPERTY_GETTER( inputLength ) {

	JL_DEFINE_PROP_ARGS;

	JL_ASSERT_THIS_INSTANCE();
	HashPrivate *pv;
	pv = (HashPrivate *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	return jl::setValue(cx, vp, pv->inputLength);
	JL_BAD;
}	

/**doc
=== Static functions ===
**/

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( cipherName )
   Initialize the CHC (chc_hash) state with _cipherName_ cipher.
   $LF
   An addition to the suite of hash functions is the Cipher Hash Construction or CHC mode.
   In this mode applicable block ciphers (such as AES) can be turned into hash functions that other functions can use.
   In particular this allows a cryptosystem to be designed using very few moving parts.   
**/
DEFINE_FUNCTION( cipherHash ) {

	JL_DEFINE_ARGS;

	jl::BufString cipherName;

	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN(1);

	JL_CHK( jl::getValue(cx, JL_ARG(1), &cipherName) );
	int cipherIndex;
	cipherIndex = find_cipher(cipherName);
	JL_ASSERT( cipherIndex >= 0, E_STR("cipher"), E_NAME(cipherName), E_NOTFOUND );

	int err;
	if ((err = chc_register(cipherIndex)) != CRYPT_OK)
		return ThrowCryptError(cx, err);

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}

/**doc
=== Static properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $OBJ $INAME $READONLY
  Contains the list of all available hash and their feature. The list is a javascript object that map hash names (key) with another object (value) that contain information.
  $H example
  {{{
  loadModule('jsstd');
  loadModule('jscrypt');
  print('hash list: ' + Hash.list.toSource() );
  }}}
**/
DEFINE_PROPERTY_GETTER( list ) {

	JL_DEFINE_PROP_ARGS;

	JS::RootedObject list(cx, JL_NewObj(cx));
	JS::RootedValue value(cx);
	int i;
	LTC_MUTEX_LOCK(&ltc_hash_mutex);
	for ( i=0; hash_is_valid(i) == CRYPT_OK; i++ ) {

		JS::RootedObject desc(cx, JL_NewObj(cx));
		JL_CHK( jl::setProperty(cx, list, hash_descriptor[i].name, desc) );
		JL_CHK( jl::setProperty(cx, desc, "hashSize", hash_descriptor[i].hashsize) );
		JL_CHK( jl::setProperty(cx, desc, "blockSize", hash_descriptor[i].blocksize) );
	}
	LTC_MUTEX_UNLOCK(&ltc_hash_mutex);

	JL_RVAL.setObject(*list);
	return jl::StoreProperty(cx, obj, id, vp, true); // create the list and store it once for all.
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision$"))
	HAS_PRIVATE
	HAS_CONSTRUCTOR
	HAS_FINALIZE
//	HAS_CALL

	BEGIN_FUNCTION_SPEC
		FUNCTION_ARGC( reset, 0 )
		FUNCTION_ARGC( write, 1 ) // previously process()
		FUNCTION_ARGC( done, 0 )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_GETTER( name )
		PROPERTY_GETTER( blockSize )
		PROPERTY_GETTER( length )
		PROPERTY_GETTER( inputLength )
	END_PROPERTY_SPEC

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION( cipherHash )
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_GETTER( list )
	END_STATIC_PROPERTY_SPEC

END_CLASS
