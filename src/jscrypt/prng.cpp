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
#include "prng.h"


/**doc
$CLASS_HEADER
 This class is used to create pseudo random number generators objects.
**/
BEGIN_CLASS( Prng )


DEFINE_FINALIZE() {

	PrngPrivate *privateData = (PrngPrivate *)JS_GetPrivate( cx, obj );
	if ( privateData != NULL ) {

		privateData->prng.done( &privateData->state );
		free( privateData );
	}
}

/**doc
 * $INAME( prngName )
  Constructs a pseudo random number generator object using the given algorithm.
  $H arguments
   $ARG string prngName: is a string that contains the name of the Asymmetric Cipher algorithm:
    * yarrow
    * fortuna
    * rc4
    * sprng
    * sober128
**/
DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();

	J_S_ASSERT_ARG_MIN( 1 );

	const char *prngName;
	J_CHK( JsvalToString(cx, argv[0], &prngName) );

	int prngIndex = find_prng(prngName);
	J_S_ASSERT_1( prngIndex != -1, "prng %s is not available", prngName );

	PrngPrivate *privateData = (PrngPrivate*)malloc( sizeof(PrngPrivate) );
	J_S_ASSERT_ALLOC( privateData );

	privateData->prng = prng_descriptor[prngIndex];

	J_S_ASSERT_1( privateData->prng.test() == CRYPT_OK, "%s prng test failed.", prngName );

	int err;
	err = privateData->prng.start( &privateData->state );
	if (err != CRYPT_OK)
		return ThrowCryptError(cx,err);
	err = privateData->prng.ready( &privateData->state );
	if (err != CRYPT_OK)
		return ThrowCryptError(cx,err);
	JS_SetPrivate( cx, obj, privateData );
	return JS_TRUE;
}

/**doc
=== Methods ===
**/


/**doc
 * $STR $INAME( size )
  Returns _size_ bytes of pseudo-random data.
  ===== example: =====
  {{{
  LoadModule('jsstd');
  LoadModule('jscrypt');
  var myGen = new Prng('yarrow');
  myGen.AutoEntropy(128); // give more entropy
  Print(HexEncode(myGen(100))); // prints random data
  }}}
**/
DEFINE_FUNCTION( Call ) {

	JSObject *thisObj = JSVAL_TO_OBJECT(argv[-2]); // get 'this' object of the current object ...
	// (TBD) check JS_InstanceOf( cx, thisObj, &NativeProc, NULL )

	J_S_ASSERT_ARG_MIN( 1 );
	J_S_ASSERT_CLASS( thisObj, _class );
	PrngPrivate *privateData = (PrngPrivate *)JS_GetPrivate( cx, thisObj );
	J_S_ASSERT_RESOURCE( privateData );

	unsigned long readCount;
	J_JSVAL_TO_INT32( argv[0], readCount );

	char *pr = (char*)JS_malloc( cx, readCount );
	J_S_ASSERT_ALLOC( pr );
	unsigned long hasRead = privateData->prng.read( (unsigned char*)pr, readCount, &privateData->state );
	J_S_ASSERT( hasRead == readCount, "unable to read prng." );

	J_CHK( J_NewBlob( cx, pr, hasRead, rval ) );

	return JS_TRUE;
}

/**doc
 * $VOID $INAME( data )
  Add _data_ as entropy (randomness) to the current prng.
**/
DEFINE_FUNCTION( AddEntropy ) {

	J_S_ASSERT_ARG_MIN( 1 );
	J_S_ASSERT_THIS_CLASS();
	PrngPrivate *privateData = (PrngPrivate *)JS_GetPrivate( cx, obj );
	J_S_ASSERT_RESOURCE( privateData );

	const char *entropy;
	size_t entropyLength;
	J_CHK( JsvalToStringAndLength(cx, argv[0], &entropy, &entropyLength) );

	int err;
	err = privateData->prng.add_entropy( (const unsigned char *)entropy, entropyLength, &privateData->state );
	if ( err != CRYPT_OK )
		return ThrowCryptError(cx, err);
	err = privateData->prng.ready(&privateData->state);
	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err);
	return JS_TRUE;
}

/**doc
 * $VOID $INAME( size )
  Automaticaly add _size_ bits of entropy to the current prng.
**/
DEFINE_FUNCTION( AutoEntropy ) {

	J_S_ASSERT_ARG_MIN( 1 );
	J_S_ASSERT_CLASS( obj, _class );
	PrngPrivate *privateData = (PrngPrivate *)JS_GetPrivate( cx, obj );
	J_S_ASSERT_RESOURCE( privateData );
	int32 bits;
	J_JSVAL_TO_INT32( argv[0], bits );
	int err = rng_make_prng( bits, find_prng(privateData->prng.name), &privateData->state, NULL );
	if ( err != CRYPT_OK )
		return ThrowCryptError(cx, err);
	return JS_TRUE;
}


/**doc
=== Static properties ===
**/

/**doc
 * $DATA $INAME
  is the current state of the prng.
**/
DEFINE_PROPERTY( stateGetter ) {

	J_S_ASSERT_CLASS( obj, _class );
	PrngPrivate *privateData = (PrngPrivate *)JS_GetPrivate( cx, obj );
	J_S_ASSERT_RESOURCE( privateData );

	unsigned long size = privateData->prng.export_size;
	char *stateData = (char*)JS_malloc(cx, size);
	unsigned long stateLength = size;
	int err = privateData->prng.pexport((unsigned char *)stateData, &stateLength, &privateData->state);
	if ( err != CRYPT_OK )
		return ThrowCryptError(cx, err);
	J_S_ASSERT( stateLength == size, "Invalid export size." );

	J_CHK( J_NewBlob(cx, stateData, size, vp) );
	return JS_TRUE;
}

DEFINE_PROPERTY( stateSetter ) {

	J_S_ASSERT_CLASS( obj, _class );
	PrngPrivate *privateData = (PrngPrivate *)JS_GetPrivate( cx, obj );
	J_S_ASSERT_RESOURCE( privateData );

	const char *stateData;
	size_t stateLength;
	J_CHK( JsvalToStringAndLength(cx, *vp, &stateData, &stateLength) );
	J_S_ASSERT( stateLength == privateData->prng.export_size, "Invalid import size." );
	int err = privateData->prng.pimport((unsigned char *)stateData, stateLength, &privateData->state);
	if ( err != CRYPT_OK )
		return ThrowCryptError(cx, err);
	return JS_TRUE;
}


/**doc
 * $STR $INAME $READONLY
  is the name of the current prng.
**/
DEFINE_PROPERTY( name ) {

	J_S_ASSERT_CLASS( obj, _class );
	PrngPrivate *privateData = (PrngPrivate *)JS_GetPrivate( cx, obj );
	J_S_ASSERT_RESOURCE( privateData );

	*vp = STRING_TO_JSVAL( JS_NewStringCopyZ(cx,privateData->prng.name) );
	return JS_TRUE;
}

/**doc
 * $OBJ $INAME $READONLY
  Contains the list of all available prng and their feature. The list is a javascript object that map cipher names (key) with another object (value) that contain information.
**/
DEFINE_PROPERTY( list ) {

	if ( *vp == JSVAL_VOID ) {

		JSObject *list = JS_NewObject( cx, NULL, NULL, NULL );
		J_S_ASSERT_ALLOC( list );
		*vp = OBJECT_TO_JSVAL(list);
		jsval value;
		int i;
		LTC_MUTEX_LOCK(&ltc_prng_mutex);
		for (i=0; i<TAB_SIZE; i++)
			if ( prng_is_valid(i) == CRYPT_OK ) {

				value = JSVAL_ONE;
				JS_SetProperty( cx, list, prng_descriptor[i].name, &value );
			}
		LTC_MUTEX_UNLOCK(&ltc_prng_mutex);
	}
	return JS_TRUE;
}


CONFIGURE_CLASS

	HAS_CONSTRUCTOR
	HAS_FINALIZE
	HAS_CALL

	BEGIN_FUNCTION_SPEC
		FUNCTION( AddEntropy )
		FUNCTION( AutoEntropy )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ( name )
		PROPERTY( state )
	END_PROPERTY_SPEC

	BEGIN_STATIC_FUNCTION_SPEC
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_READ_STORE( list )
	END_STATIC_PROPERTY_SPEC

	HAS_PRIVATE

END_CLASS
