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


BEGIN_CLASS( Hash )

DEFINE_FINALIZE() {

	HashPrivate *privateData = (HashPrivate *)JS_GetPrivate( cx, obj );
	if ( privateData != NULL ) {

		free(privateData);
	}
}

DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();
	RT_ASSERT_ARGC( 1 );

	char *hashName;
	RT_JSVAL_TO_STRING( argv[0], hashName );

	int hashIndex = find_hash(hashName);
	RT_ASSERT_1( hashIndex != -1, "hash %s is not available", hashName );

	HashPrivate *privateData = (HashPrivate*)malloc( sizeof(HashPrivate) );
	RT_ASSERT_ALLOC( privateData );

	privateData->descriptor = &hash_descriptor[hashIndex];

	RT_ASSERT_1( privateData->descriptor->test() == CRYPT_OK, "%s hash test failed.", hashName );

	int err;
	if ( (err = privateData->descriptor->init(&privateData->state)) != CRYPT_OK )
		return ThrowCryptError(cx, err);
	privateData->inputLength = 0;

	JS_SetPrivate( cx, obj, privateData );

	return JS_TRUE;
}


DEFINE_FUNCTION( Init ) {

	RT_ASSERT_THIS_CLASS();
	RT_ASSERT_ARGC_MAX( 0 );

	HashPrivate *privateData = (HashPrivate *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE( privateData );

	int err;
	err = privateData->descriptor->init(&privateData->state); // Initialize the hash state
	if ( err != CRYPT_OK )
		return ThrowCryptError(cx, err);
	privateData->inputLength = 0;

	return JS_TRUE;
}


DEFINE_FUNCTION( Process ) {

	RT_ASSERT_THIS_CLASS();
	RT_ASSERT_ARGC( 1 );
	RT_ASSERT_STRING( argv[0] );

	HashPrivate *privateData = (HashPrivate *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE( privateData );

	int err;
	char *in;
	int inLength;
	RT_JSVAL_TO_STRING_AND_LENGTH( argv[0], in, inLength );

	err = privateData->descriptor->process(&privateData->state, (const unsigned char *)in, inLength); // Process a block of memory though the hash
	if ( err != CRYPT_OK )
		return ThrowCryptError(cx, err);

	privateData->inputLength += inLength;

	return JS_TRUE;
}


DEFINE_FUNCTION( Done ) {

	HashPrivate *privateData = (HashPrivate *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE( privateData );

	unsigned long outLength = privateData->descriptor->hashsize;
	char *out = (char *)JS_malloc( cx, outLength );
	RT_ASSERT_ALLOC( out );
	int err;
	err = privateData->descriptor->done(&privateData->state, (unsigned char*)out); // Terminate the hash to get the digest
	if ( err != CRYPT_OK )
		return ThrowCryptError(cx, err);
	JSString *jssHashData = JS_NewString( cx, out, outLength );
	RT_ASSERT( jssHashData != NULL, "unable to create the hash string." );
	*rval = STRING_TO_JSVAL(jssHashData);
	return JS_TRUE;
}


DEFINE_FUNCTION( Call ) {

	JSObject *thisObj = JSVAL_TO_OBJECT(argv[-2]); // get 'this' object of the current object ...
	// (TBD) check JS_InstanceOf( cx, thisObj, &NativeProc, NULL )
	RT_ASSERT_CLASS( thisObj, _class );
	
	RT_ASSERT_ARGC( 1 );
	RT_ASSERT_STRING( argv[0] );

	HashPrivate *privateData = (HashPrivate *)JS_GetPrivate( cx, thisObj );
	J_S_ASSERT_RESOURCE( privateData );

	int err;
	
	char *in;
	int inLength;
	RT_JSVAL_TO_STRING_AND_LENGTH( argv[0], in, inLength );
	unsigned long outLength = privateData->descriptor->hashsize;
	char *out = (char *)JS_malloc( cx, outLength );
	RT_ASSERT_ALLOC( out );

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

	JSString *jssHashData = JS_NewString( cx, out, outLength );
	RT_ASSERT_ALLOC( jssHashData );
	*rval = STRING_TO_JSVAL(jssHashData);
	return JS_TRUE;
}

DEFINE_PROPERTY( name ) {

	RT_ASSERT_CLASS( obj, _class );
	HashPrivate *privateData = (HashPrivate *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE( privateData );
	JSString *jsstr = JS_NewStringCopyZ(cx, privateData->descriptor->name );
	RT_ASSERT_ALLOC( jsstr );
	*vp = STRING_TO_JSVAL( jsstr );
	return JS_TRUE;
}

DEFINE_PROPERTY( blockSize ) {

	RT_ASSERT_CLASS( obj, _class );
	HashPrivate *privateData = (HashPrivate *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE( privateData );
	*vp = INT_TO_JSVAL( privateData->descriptor->blocksize );
	return JS_TRUE;
}	

DEFINE_PROPERTY( length ) {

	RT_ASSERT_CLASS( obj, _class );
	HashPrivate *privateData = (HashPrivate *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE( privateData );
	*vp = INT_TO_JSVAL( privateData->descriptor->hashsize );
	return JS_TRUE;
}	

DEFINE_PROPERTY( inputLength ) {

	RT_ASSERT_CLASS( obj, _class );
	HashPrivate *privateData = (HashPrivate *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE( privateData );
	*vp = INT_TO_JSVAL( 	privateData->inputLength );
	return JS_TRUE;
}	

DEFINE_FUNCTION( CipherHash ) {

	RT_ASSERT_ARGC(1);
	char *cipherName;
	RT_JSVAL_TO_STRING( argv[0], cipherName );
	int cipherIndex = find_cipher(cipherName);
	RT_ASSERT_1( cipherIndex >= 0, "Cipher not found: %s", cipherName );
	int err;
	if ((err = chc_register(cipherIndex)) != CRYPT_OK)
		return ThrowCryptError(cx, err);
	return JS_TRUE;
}

DEFINE_PROPERTY( list ) {

	if ( *vp == JSVAL_VOID ) {

		JSObject *list = JS_NewObject( cx, NULL, NULL, NULL );
		RT_ASSERT_ALLOC( list );
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
}


CONFIGURE_CLASS

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
