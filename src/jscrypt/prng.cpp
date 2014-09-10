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


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
 This class is used to create pseudo random number generators objects.
**/
BEGIN_CLASS( Prng )


DEFINE_FINALIZE() {

	if ( jl::HostRuntime::getJLRuntime( fop->runtime() ).skipCleanup() )
		return;

	PrngPrivate *pv = (PrngPrivate *)JL_GetPrivateFromFinalize(obj);
	if ( !pv )
		return;

	pv->prng.done(&pv->state);
	jl_free(pv);
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

	PrngPrivate *pv = NULL;

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_MIN( 1 );
	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

	{

		jl::StrData prngName(cx);

		JL_CHK( jl::getValue(cx, JL_ARG(1), &prngName) );

		int prngIndex;
		prngIndex = find_prng(prngName.toStrZ());
		JL_ASSERT( prngIndex != -1, E_STR("PRNG"), E_NAME(prngName), E_NOTFOUND );

		pv = (PrngPrivate*)jl_malloc(sizeof(PrngPrivate));
		JL_CHK( pv );

		pv->prng = prng_descriptor[prngIndex];
		JL_ASSERT( pv->prng.test() == CRYPT_OK, E_LIB, E_STR("libtomcrypt"), E_INTERNAL, E_SEP, E_STR(prngName), E_STR("test"), E_FAILURE );

		int err;
		err = pv->prng.start( &pv->state );
		if (err != CRYPT_OK)
			return ThrowCryptError(cx,err);

		err = pv->prng.ready( &pv->state );
		if (err != CRYPT_OK)
			return ThrowCryptError(cx,err);
	
	}

	JL_SetPrivate(JL_OBJ, pv);
	return true;
bad:
	jl_free(pv);
	return false;
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
  loadModule('jsstd');
  loadModule('jscrypt');
  var myGen = new Prng('yarrow');
  myGen.autoEntropy(128); // give more entropy
  print(hexEncode(myGen.read(100))); // prints random data
  }}}
**/
/**qa
  loadModule('jscrypt');
  var myGen = new Prng('yarrow');
  myGen.autoEntropy(128);
  QA.ASSERT( myGen.read(100).byteLength, 100 );
**/
DEFINE_FUNCTION( read ) {

	JL_DEFINE_ARGS;
	
	jl::BufBase buffer;

	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_RANGE(0,1);

	PrngPrivate *pv;
	pv = (PrngPrivate *)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	unsigned long readCount, actualRead;

	if ( JL_ARG_ISDEF(1) )
		JL_CHK( jl::getValue(cx, JL_ARG(1), &readCount) );
	else
		readCount = 1024;

	//uint8_t *pr;
	//pr = JL_NewBuffer(cx, readCount, JL_RVAL);
	//JL_CHK( pr );

	buffer.alloc(readCount, true);
	JL_ASSERT_ALLOC( buffer );

	actualRead = pv->prng.read(buffer.data(), readCount, &pv->state);
	JL_CHKM( actualRead == readCount, E_DATA, E_CREATE );

	JL_CHK( BlobCreate(cx, buffer, JL_RVAL) );
	
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( data )
  Add _data_ as entropy (randomness) to the current prng.
**/
DEFINE_FUNCTION( addEntropy ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN( 1 );

	PrngPrivate *pv;
	pv = (PrngPrivate *)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	
	{

		jl::StrData entropy(cx);
		JL_CHK( jl::getValue(cx, JL_ARG(1), &entropy) );

		int err;
		err = pv->prng.add_entropy( entropy.toBytes(), entropy.length(), &pv->state );
		if ( err != CRYPT_OK )
			return ThrowCryptError(cx, err);

		err = pv->prng.ready(&pv->state);
		if ( err != CRYPT_OK )
			return ThrowCryptError(cx, err);
	}

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( size )
  Automaticaly add _size_ bits of entropy to the current prng.
**/
DEFINE_FUNCTION( autoEntropy ) {

	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN( 1 );

	PrngPrivate *pv;
	pv = (PrngPrivate *)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	unsigned int bits;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &bits) );
	int err;
	err = rng_make_prng( bits, find_prng(pv->prng.name), &pv->state, NULL );
	if ( err != CRYPT_OK )
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
 $DATA $INAME
  is the current state of the prng.
**/
DEFINE_PROPERTY_GETTER( state ) {

	JL_DEFINE_PROP_ARGS;

	jl::BufBase buffer;

	JL_ASSERT_THIS_INSTANCE();

	PrngPrivate *pv;
	pv = (PrngPrivate *)JL_GetPrivate( obj );
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	unsigned long size;
	size = pv->prng.export_size;
	
	//uint8_t *stateData;
	//stateData = JL_NewBuffer(cx, size, vp);
	//JL_CHK( stateData );

	buffer.alloc(size);
	JL_ASSERT_ALLOC(buffer);

	unsigned long stateLength;
	stateLength = size;
	int err;
	err = pv->prng.pexport(buffer.data(), &stateLength, &pv->state);
	if ( err != CRYPT_OK )
		return ThrowCryptError(cx, err);
	
	JL_CHKM( stateLength == size, E_LIB, E_INTERNAL, E_SEP, E_STR("state"), E_LENGTH, E_NUM(size) );

	buffer.setUsed(stateLength);

	JL_CHK( BlobCreate(cx, buffer, JL_RVAL) );

	return true;
	JL_BAD;
}


DEFINE_PROPERTY_SETTER( state ) {

	JL_DEFINE_PROP_ARGS;
	JL_ASSERT_THIS_INSTANCE();

	PrngPrivate *pv;
	pv = (PrngPrivate *)JL_GetPrivate( obj );
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	{

		jl::StrData state(cx);
		JL_CHK( jl::getValue(cx, JL_RVAL, &state) );
		JL_CHKM( state.length() == (size_t)pv->prng.export_size, E_VALUE, E_LENGTH, E_NUM(pv->prng.export_size) );

		int err;
		err = pv->prng.pimport( state.toBytes(), state.length(), &pv->state );
		if ( err != CRYPT_OK )
			return ThrowCryptError(cx, err);
	
	}

	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME $READONLY
  is the name of the current prng.
**/
DEFINE_PROPERTY_GETTER( name ) {

	JL_DEFINE_PROP_ARGS;

	JL_ASSERT_THIS_INSTANCE();

	PrngPrivate *pv;
	pv = (PrngPrivate *)JL_GetPrivate( obj );
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	JL_CHK( jl::setValue( cx, JL_RVAL, pv->prng.name ) );
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $OBJ $INAME $READONLY
  Contains the list of all available prng and their feature. The list is a javascript object that map cipher names (key) with another object (value) that contain information.
**/
DEFINE_PROPERTY_GETTER( list ) {

	JL_DEFINE_PROP_ARGS;

	JS::RootedObject listObj(cx, JL_NewObj(cx));
	
	JS::RootedValue value(cx);
	int i;
	LTC_MUTEX_LOCK(&ltc_prng_mutex);
	for ( i=0; prng_is_valid(i) == CRYPT_OK; i++ ) {

		JL_CHK( jl::setProperty(cx, listObj, prng_descriptor[i].name, 1) );
	}
	LTC_MUTEX_UNLOCK(&ltc_prng_mutex);

	JL_RVAL.setObject(*listObj);
	return jl::StoreProperty(cx, obj, id, vp, true); // create the list and store it once for all.
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision$"))
	HAS_PRIVATE
	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION_ARGC( addEntropy, 1 )
		FUNCTION_ARGC( autoEntropy, 1 )
		FUNCTION_ARGC( read, 1 )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_GETTER( name )
		PROPERTY( state )
	END_PROPERTY_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_GETTER( list )
	END_STATIC_PROPERTY_SPEC

END_CLASS
