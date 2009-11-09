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
$SVN_REVISION $Revision$
 This class is used to create pseudo random number generators objects.
**/
BEGIN_CLASS( Prng )


DEFINE_FINALIZE() {

	PrngPrivate *pv = (PrngPrivate *)JL_GetPrivate( cx, obj );
	if ( !pv )
		return;

	pv->prng.done(&pv->state);
	JS_free(cx, pv);
}

/**doc
$TOC_MEMBER $INAME
 $INAME( prngName )
  Constructs a pseudo random number generator object using the given algorithm.
  $H arguments
   $ARG $STR prngName: is a string that contains the name of the Asymmetric Cipher algorithm:
    * yarrow
    * fortuna
    * rc4
    * sprng
    * sober128
**/
DEFINE_CONSTRUCTOR() {

	JL_S_ASSERT_CONSTRUCTING();
	JL_S_ASSERT_THIS_CLASS();

	JL_S_ASSERT_ARG_MIN( 1 );

	const char *prngName;
	JL_CHK( JsvalToString(cx, &argv[0], &prngName) );

	int prngIndex;
	prngIndex = find_prng(prngName);
	JL_S_ASSERT( prngIndex != -1, "prng %s is not available", prngName );

	PrngPrivate *pv;
	pv = (PrngPrivate*)JS_malloc(cx, sizeof(PrngPrivate));
	JL_CHK( pv );

	pv->prng = prng_descriptor[prngIndex];

	JL_S_ASSERT( pv->prng.test() == CRYPT_OK, "%s prng test failed.", prngName );

	int err;
	err = pv->prng.start( &pv->state );
	if (err != CRYPT_OK)
		return ThrowCryptError(cx,err);
	err = pv->prng.ready( &pv->state );
	if (err != CRYPT_OK)
		return ThrowCryptError(cx,err);
	JL_SetPrivate( cx, obj, pv );
	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Methods ===
**/


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( size )
  Returns _size_ bytes of pseudo-random data.
  $H example
  {{{
  LoadModule('jsstd');
  LoadModule('jscrypt');
  var myGen = new Prng('yarrow');
  myGen.AutoEntropy(128); // give more entropy
  Print(HexEncode(myGen(100))); // prints random data
  }}}
**/
DEFINE_CALL() {

	JSObject *thisObj = JSVAL_TO_OBJECT(argv[-2]); // get 'this' object of the current object ...
	JL_S_ASSERT_CLASS( thisObj, _class );

	JL_S_ASSERT_ARG_MIN( 1 );
	JL_S_ASSERT_CLASS( thisObj, _class );
	PrngPrivate *pv;
	pv = (PrngPrivate *)JL_GetPrivate( cx, thisObj );
	JL_S_ASSERT_RESOURCE( pv );

	unsigned int readCount;
	JL_CHK( JsvalToUInt(cx, argv[0], &readCount) );

	char *pr;
	pr = (char*)JS_malloc( cx, readCount );
	JL_CHK( pr );
	unsigned long hasRead;
	hasRead = pv->prng.read( (unsigned char*)pr, readCount, &pv->state );
	JL_S_ASSERT( hasRead == readCount, "unable to read prng." );

	JL_CHK( JL_NewBlob( cx, pr, hasRead, rval ) );

	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( data )
  Add _data_ as entropy (randomness) to the current prng.
**/
DEFINE_FUNCTION( AddEntropy ) {

	JL_S_ASSERT_CLASS( obj, _class );
	JL_S_ASSERT_ARG_MIN( 1 );
	PrngPrivate *pv;
	pv = (PrngPrivate *)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( pv );

	const char *entropy;
	size_t entropyLength;
	JL_CHK( JsvalToStringAndLength(cx, &argv[0], &entropy, &entropyLength) );

	int err;
	err = pv->prng.add_entropy( (const unsigned char *)entropy, entropyLength, &pv->state );
	if ( err != CRYPT_OK )
		return ThrowCryptError(cx, err);
	err = pv->prng.ready(&pv->state);
	if (err != CRYPT_OK)
		return ThrowCryptError(cx, err);
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( size )
  Automaticaly add _size_ bits of entropy to the current prng.
**/
DEFINE_FUNCTION( AutoEntropy ) {

	JL_S_ASSERT_CLASS( obj, _class );
	JL_S_ASSERT_ARG_MIN( 1 );
	PrngPrivate *pv;
	pv = (PrngPrivate *)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( pv );

	unsigned int bits;
	JL_CHK( JsvalToUInt(cx, argv[0], &bits) );
	int err;
	err = rng_make_prng( bits, find_prng(pv->prng.name), &pv->state, NULL );
	if ( err != CRYPT_OK )
		return ThrowCryptError(cx, err);
	return JS_TRUE;
	JL_BAD;
}


/**doc
=== Static properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $DATA $INAME
  is the current state of the prng.
**/
DEFINE_PROPERTY( stateGetter ) {

	JL_S_ASSERT_CLASS( obj, _class );
	PrngPrivate *pv;
	pv = (PrngPrivate *)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( pv );

	unsigned long size;
	size = pv->prng.export_size;
	char *stateData;
	stateData = (char*)JS_malloc(cx, size);
	unsigned long stateLength;
	stateLength = size;
	int err;
	err = pv->prng.pexport((unsigned char *)stateData, &stateLength, &pv->state);
	if ( err != CRYPT_OK )
		return ThrowCryptError(cx, err);
	JL_S_ASSERT( stateLength == size, "Invalid export size." );

	JL_CHK( JL_NewBlob(cx, stateData, size, vp) );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( stateSetter ) {

	JL_S_ASSERT_CLASS( obj, _class );
	PrngPrivate *pv;
	pv = (PrngPrivate *)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( pv );

	const char *stateData;
	unsigned int stateLength;
	JL_CHK( JsvalToStringAndLength(cx, vp, &stateData, &stateLength) );
	JL_S_ASSERT( stateLength == (unsigned)pv->prng.export_size, "Invalid import size." );
	int err;
	err = pv->prng.pimport((unsigned char *)stateData, stateLength, &pv->state);
	if ( err != CRYPT_OK )
		return ThrowCryptError(cx, err);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME $READONLY
  is the name of the current prng.
**/
DEFINE_PROPERTY( name ) {

	JL_S_ASSERT_CLASS( obj, _class );
	PrngPrivate *pv;
	pv = (PrngPrivate *)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( pv );

	*vp = STRING_TO_JSVAL( JS_NewStringCopyZ(cx,pv->prng.name) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $OBJ $INAME $READONLY
  Contains the list of all available prng and their feature. The list is a javascript object that map cipher names (key) with another object (value) that contain information.
**/
DEFINE_PROPERTY( list ) {

	JSTempValueRooter tvr;
	JS_PUSH_SINGLE_TEMP_ROOT(cx, JSVAL_NULL, &tvr); // (TBD) remove this workaround. cf. bz495422 || bz397177

	JSObject *list = JS_NewObject( cx, NULL, NULL, NULL );
	tvr.u.value = OBJECT_TO_JSVAL(list);
	jsval value;
	int i;
	LTC_MUTEX_LOCK(&ltc_prng_mutex);
	for (i=0; i<TAB_SIZE; i++)
		if ( prng_is_valid(i) == CRYPT_OK ) {

			value = JSVAL_ONE;
			JS_SetProperty( cx, list, prng_descriptor[i].name, &value );
		}
	LTC_MUTEX_UNLOCK(&ltc_prng_mutex);

	*vp = tvr.u.value;
	JS_POP_TEMP_ROOT(cx, &tvr);
	return JL_StoreProperty(cx, obj, id, vp, true);
}


CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
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
		PROPERTY_READ( list )
	END_STATIC_PROPERTY_SPEC

	HAS_PRIVATE

END_CLASS
