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
#include "hash.h"

struct HashPrivate {

	ltc_hash_descriptor *descriptor;
	hash_state state;
	int inputLength;
};


/**doc
$CLASS_HEADER
 This class is used to create block Hash objects.
**/
BEGIN_CLASS( Hash )

DEFINE_FINALIZE() {

	HashPrivate *privateData = (HashPrivate *)JS_GetPrivate( cx, obj );
	if ( privateData != NULL ) {

		free(privateData);
	}
}

/**doc
 * $INAME( hashName )
  Creates a new hash 
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
  ===== note: =====
  chc_hash is a special hash that allows to create a hash from a cipher (Cipher Hash Construction).
  See CipherHash() static function.
**/
DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();
	J_S_ASSERT_ARG_MIN( 1 );

	const char *hashName;
	J_CHK( JsvalToString(cx, &argv[0], &hashName) );

	int hashIndex;
	hashIndex = find_hash(hashName);
	J_S_ASSERT_1( hashIndex != -1, "hash %s is not available", hashName );

	HashPrivate *privateData;
	privateData = (HashPrivate*)malloc( sizeof(HashPrivate) );
	J_S_ASSERT_ALLOC( privateData );

	privateData->descriptor = &hash_descriptor[hashIndex];

	J_S_ASSERT_1( privateData->descriptor->test() == CRYPT_OK, "%s hash test failed.", hashName );

	int err;
	if ( (err = privateData->descriptor->init(&privateData->state)) != CRYPT_OK )
		return ThrowCryptError(cx, err);
	privateData->inputLength = 0;

	JS_SetPrivate( cx, obj, privateData );

	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Methods ===
**/

/**doc
 * $INAME()
  Initialize the hash state.
**/
DEFINE_FUNCTION( Init ) {

	J_S_ASSERT_CLASS( obj, _class );
	J_S_ASSERT_ARG_MAX( 0 );

	HashPrivate *privateData;
	privateData = (HashPrivate *)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( privateData );

	int err;
	err = privateData->descriptor->init(&privateData->state); // Initialize the hash state
	if ( err != CRYPT_OK )
		return ThrowCryptError(cx, err);
	privateData->inputLength = 0;

	return JS_TRUE;
	JL_BAD;
}


/**doc
 * $VOID $INAME( data )
  Process a block of data though the hash.
**/
DEFINE_FUNCTION( Process ) {

	J_S_ASSERT_CLASS( obj, _class );
	J_S_ASSERT_ARG_MIN( 1 );
	J_S_ASSERT_STRING( argv[0] );

	HashPrivate *privateData;
	privateData = (HashPrivate *)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( privateData );

	int err;
	const char *in;
	size_t inLength;
	J_CHK( JsvalToStringAndLength(cx, &argv[0], &in, &inLength) );

	err = privateData->descriptor->process(&privateData->state, (const unsigned char *)in, inLength); // Process a block of memory though the hash
	if ( err != CRYPT_OK )
		return ThrowCryptError(cx, err);

	privateData->inputLength += inLength;

	return JS_TRUE;
	JL_BAD;
}


/**doc
 * $DATA $INAME()
  Terminate the hash and get the digest in a binary format.
  ===== example: =====
  {{{
  LoadModule('jsstd');
  LoadModule('jscrypt');
  var md5 = new Hash('md5');
  md5.Process('foo');
  md5.Process('bar');
  Print( HexEncode( md5.Done(), '\n' ) ); // prints: 3858F62230AC3C915F300C664312C63F
  }}}
**/
DEFINE_FUNCTION( Done ) {

	J_S_ASSERT_CLASS( obj, _class );
	HashPrivate *privateData;
	privateData = (HashPrivate *)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( privateData );

	unsigned long outLength;
	outLength = privateData->descriptor->hashsize;
	char *out;
	out = (char *)JS_malloc( cx, outLength );
	J_S_ASSERT_ALLOC( out );
	int err;
	err = privateData->descriptor->done(&privateData->state, (unsigned char*)out); // Terminate the hash to get the digest
	if ( err != CRYPT_OK )
		return ThrowCryptError(cx, err);

	J_CHK( J_NewBlob( cx, out, outLength, rval ) );

	return JS_TRUE;
	JL_BAD;
}


/**doc
 * $DATA $INAME( data )
  This is the call operator of the object. It simplifies the usage of hashes and allows a digest calculation in one call only.
  When called with a string as argument, it Process a block of memory though the hash
  Compute the hash until the function is called without arguments or end is <true>. In this case, the function returns the hash of the whole given data.
  ===== beware: =====
  Using this methode to compute a digest automaticaly resets previous state let by Init(), Process() or Done().
  ===== example: =====
  {{{
  LoadModule('jsstd');
  LoadModule('jscrypt');
  var md5 = new Hash('md5');
  Print( HexEncode( md5('foobar') ) ); // prints: 3858F62230AC3C915F300C664312C63F
  }}}
**/
DEFINE_CALL() {

	JSObject *thisObj = JSVAL_TO_OBJECT(argv[-2]); // get 'this' object of the current object ...
	J_S_ASSERT_CLASS( thisObj, _class );
	
	J_S_ASSERT_ARG_MIN( 1 );
	J_S_ASSERT_STRING( argv[0] );

	HashPrivate *privateData;
	privateData = (HashPrivate *)JS_GetPrivate( cx, thisObj );
	J_S_ASSERT_RESOURCE( privateData );

	int err;

	unsigned long outLength;
	outLength = privateData->descriptor->hashsize;
	char *out;
	out = (char *)JS_malloc( cx, outLength );
	J_S_ASSERT_ALLOC( out );

	const char *in;
	size_t inLength;
	J_CHK( JsvalToStringAndLength(cx, &argv[0], &in, &inLength) );

	err = privateData->descriptor->init(&privateData->state);
	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err);

	err = privateData->descriptor->process(&privateData->state, (const unsigned char *)in, inLength);
	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err);

	err = privateData->descriptor->done(&privateData->state, (unsigned char*)out);
	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err);

	err = privateData->descriptor->init(&privateData->state);
	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err);
	privateData->inputLength = 0;

	J_CHK( J_NewBlob( cx, out, outLength, rval ) );

	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Properties ===
**/

/**doc
 * $STR $INAME $READONLY
  Name of the current hash.
**/
DEFINE_PROPERTY( name ) {

	J_S_ASSERT_CLASS( obj, _class );
	HashPrivate *privateData;
	privateData = (HashPrivate *)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( privateData );
	JSString *jsstr;
	jsstr = JS_NewStringCopyZ(cx, privateData->descriptor->name );
	J_S_ASSERT_ALLOC( jsstr );
	*vp = STRING_TO_JSVAL( jsstr );
	return JS_TRUE;
	JL_BAD;
}

/**doc
 * $INT $INAME $READONLY
  Input block size in octets.
**/
DEFINE_PROPERTY( blockSize ) {

	J_S_ASSERT_CLASS( obj, _class );
	HashPrivate *privateData;
	privateData = (HashPrivate *)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( privateData );
	*vp = INT_TO_JSVAL( privateData->descriptor->blocksize );
	return JS_TRUE;
	JL_BAD;
}	

/**doc
 * $INT $INAME $READONLY
  Size of the digest in octets.
**/
DEFINE_PROPERTY( length ) {

	J_S_ASSERT_CLASS( obj, _class );
	HashPrivate *privateData;
	privateData = (HashPrivate *)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( privateData );
	*vp = INT_TO_JSVAL( privateData->descriptor->hashsize );
	return JS_TRUE;
	JL_BAD;
}	

/**doc
 * $INT $INAME $READONLY
  Length of the processed data.
**/
DEFINE_PROPERTY( inputLength ) {

	J_S_ASSERT_CLASS( obj, _class );
	HashPrivate *privateData;
	privateData = (HashPrivate *)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( privateData );
	*vp = INT_TO_JSVAL( 	privateData->inputLength );
	return JS_TRUE;
	JL_BAD;
}	

/**doc
=== Static functions ===
**/

/**doc
 * $INAME( cipherName )
   Initialize the CHC (chc_hash) state with _cipherName_ cipher.
    = =
   An addition to the suite of hash functions is the Cipher Hash Construction or CHC mode.
   In this mode applicable block ciphers (such as AES) can be turned into hash functions that other functions can use.
   In particular this allows a cryptosystem to be designed using very few moving parts.   
**/
DEFINE_FUNCTION( CipherHash ) {

	J_S_ASSERT_CLASS( obj, _class );
	J_S_ASSERT_ARG_MIN(1);
	const char *cipherName;
	J_CHK( JsvalToString(cx, &argv[0], &cipherName) );
	int cipherIndex;
	cipherIndex = find_cipher(cipherName);
	J_S_ASSERT_1( cipherIndex >= 0, "Cipher not found: %s", cipherName );
	int err;
	if ((err = chc_register(cipherIndex)) != CRYPT_OK)
		return ThrowCryptError(cx, err);
	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Static properties ===
**/

/**doc
 * $OBJ $INAME $READONLY
  Contains the list of all available hash and their feature. The list is a javascript object that map hash names (key) with another object (value) that contain information.
  ===== example: =====
  {{{
  LoadModule('jsstd');
  LoadModule('jscrypt');
  Print('hash list: ' + Hash.list.toSource() );
  }}}
**/
DEFINE_PROPERTY( list ) {

	if ( JSVAL_IS_VOID( *vp ) ) {

		JSObject *list = JS_NewObject( cx, NULL, NULL, NULL );
		J_S_ASSERT_ALLOC( list );
		*vp = OBJECT_TO_JSVAL(list);
		jsval value;
		int i;
		LTC_MUTEX_LOCK(&ltc_hash_mutex);
		for (i=0; i<TAB_SIZE; i++)
			if ( hash_is_valid(i) == CRYPT_OK ) {

				JSObject *desc = JS_NewObject( cx, NULL, NULL, NULL );
				value = OBJECT_TO_JSVAL(desc);
				JS_SetProperty( cx, list, hash_descriptor[i].name, &value );

				value = INT_TO_JSVAL( hash_descriptor[i].hashsize );
				JS_SetProperty( cx, desc, "hashSize", &value );
				value = INT_TO_JSVAL( hash_descriptor[i].blocksize );
				JS_SetProperty( cx, desc, "blockSize", &value );
			}
		LTC_MUTEX_UNLOCK(&ltc_hash_mutex);
	}
	return JS_TRUE;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(SvnRevToInt("$Revision$"))
	HAS_CONSTRUCTOR
	HAS_FINALIZE
	HAS_CALL

	BEGIN_FUNCTION_SPEC
		FUNCTION( Init )
		FUNCTION( Process )
		FUNCTION( Done )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ( name )
		PROPERTY_READ( blockSize )
		PROPERTY_READ( length )
		PROPERTY_READ( inputLength )
	END_PROPERTY_SPEC

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION( CipherHash )
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_READ_STORE( list )
	END_STATIC_PROPERTY_SPEC

	HAS_PRIVATE

END_CLASS
