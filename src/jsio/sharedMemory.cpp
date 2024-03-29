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

DECLARE_CLASS(SharedMemory)

#define SEMAPHORE_EXTENSION "_sem"

struct ClassPrivate {
	char name[PATH_MAX +1];
	PRSharedMemory *shm;
	void *mem;
	size_t size;
	PRSem *accessSem;
};


struct MemHeader {
	size_t currentDataLength;
	int accessCount;
};


bool Lock( JSContext *cx, ClassPrivate *pv ) {

	PRStatus status = PR_WaitSemaphore( pv->accessSem );
	if ( status != PR_SUCCESS )
		return ThrowIoError(cx);
	return true;
}

bool Unlock( JSContext *cx, ClassPrivate *pv ) {

	PRStatus status = PR_PostSemaphore( pv->accessSem );
	if ( status != PR_SUCCESS )
		return ThrowIoError(cx);
	return true;
}


bool SharedMemoryBufferGet( JSContext *cx, JS::HandleObject obj, jl::BufString *str ) {

	ClassPrivate *pv = (ClassPrivate*)JL_GetPrivate(obj);
	JL_ASSERT_OBJECT_STATE( pv, JL_CLASS_NAME(SharedMemory) );
	MemHeader *mh;
	mh = (MemHeader*)pv->mem;
//	*buf = (char *)pv->mem + sizeof(MemHeader);
//	*size = mh->currentDataLength;
	//*str = JLData(((const char *)pv->mem) + sizeof(MemHeader), false, mh->currentDataLength);
	str->get(((const char *)pv->mem) + sizeof(MemHeader), mh->currentDataLength, false);

	return true;
	JL_BAD;
}

template <js::AllowGC allowGC>
bool
CloseSharedMemory( typename js::MaybeRooted<JSObject*, allowGC>::HandleType obj ) {

	ClassPrivate *pv = (ClassPrivate*)JL_GetPrivateFromFinalize(obj);

	JL_CHK( PR_WaitSemaphore( pv->accessSem ) == PR_SUCCESS );

	MemHeader *mh;
	mh = (MemHeader*)pv->mem;

	bool isLast;
	isLast = (mh->accessCount == 0);
	mh->accessCount--;

	JL_CHK( PR_DetachSharedMemory(pv->shm, pv->mem) == PR_SUCCESS );
	JL_CHK( PR_CloseSharedMemory(pv->shm) == PR_SUCCESS );

	JL_CHK( PR_PostSemaphore(pv->accessSem) == PR_SUCCESS );
	JL_CHK( PR_CloseSemaphore(pv->accessSem) == PR_SUCCESS );

	if ( isLast ) {

		JL_CHK( PR_DeleteSharedMemory(pv->name) == PR_SUCCESS );
		char semName[PATH_MAX];
		jl::strcpy( semName, pv->name );
		jl::strcat( semName, SEMAPHORE_EXTENSION );
		JL_CHK( PR_DeleteSemaphore(semName) == PR_SUCCESS );
	}

	jl_free(pv);

	return true;
	JL_BAD;
}


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
 This class manages shared memory between two or more process.
**/
BEGIN_CLASS( SharedMemory )

DEFINE_FINALIZE() {

	JL_IGNORE( fop );

	if ( JL_GetPrivateFromFinalize(obj) )
		CloseSharedMemory<js::NoGC>(obj);
}

// doc.
// The unix implementation may use SysV IPC shared memory, Posix
// shared memory, or memory mapped files; the filename may used to
// define the namespace. On Windows, the name is significant, but
// there is no file associated with name.


/**doc
$TOC_MEMBER $INAME
 $INAME( name, size [, mode] )
  Creates a named shared memory area of _size_ bytes using _mode_ linux-like rights.
**/
DEFINE_CONSTRUCTOR() {

	ClassPrivate *pv = NULL;
	PRSem *accessSem = NULL;
	PRSharedMemory *shm = NULL;
	
	JL_DEFINE_ARGS;
	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;
	JL_ASSERT_ARGC_RANGE(2, 3);

	size_t size;
	JL_CHK( jl::getValue(cx, JL_ARG(2), &size) );

	unsigned int mode;
	if ( JL_ARG_ISDEF(3) )
		JL_CHK( jl::getValue(cx, JL_ARG(3), &mode) );
	else
		mode = PR_IRUSR | PR_IWUSR; // read write permission for owner.

	{

		jl::StrData name(cx);

		JL_CHK( jl::getValue(cx, JL_ARG(1), &name) );

		char semName[PATH_MAX];
		jl::strcpy( semName, name );
		jl::strcat( semName, SEMAPHORE_EXTENSION );

		bool isCreation;
		isCreation = true;
		accessSem = PR_OpenSemaphore(semName, PR_SEM_EXCL | PR_SEM_CREATE, mode, 1); // fail if already exists

		if ( accessSem == NULL ) {

			accessSem = PR_OpenSemaphore(semName, 0, 0, 0); // If PR_SEM_CREATE is not specified, the third and fourth arguments are ignored.
			JL_CHKB( accessSem != NULL, bad_ioerror );
			isCreation = false;
		}

		JL_CHKB( PR_WaitSemaphore( accessSem ) == PR_SUCCESS, bad_ioerror );

		shm = PR_OpenSharedMemory( name, size + sizeof(MemHeader), PR_SHM_CREATE, mode );
		JL_CHKB( shm != NULL, bad_ioerror ); // PR_SHM_READONLY

		void *mem;
		mem = PR_AttachSharedMemory(shm, 0);
		JL_CHKB( mem != NULL, bad_ioerror ); // PR_SHM_READONLY

		pv = (ClassPrivate*)jl_malloc(sizeof(ClassPrivate));
		JL_CHK( pv );

		jl::strcpy( pv->name, name );

		pv->shm = shm;
		pv->mem = mem;
		pv->size = size + sizeof(MemHeader);
		pv->accessSem = accessSem;

		MemHeader *mh;
		mh = (MemHeader*)pv->mem;

		if ( isCreation ) {

			mh->accessCount = 0;
			mh->currentDataLength = 0;
		} else {

			mh->accessCount++;
		}

	}

	JL_CHKB( PR_PostSemaphore( accessSem ) == PR_SUCCESS, bad_ioerror );
	JL_CHK( jl::setBufferGetInterface(cx, JL_OBJ, SharedMemoryBufferGet) );

	JL_SetPrivate(JL_OBJ, pv);
	return true;

bad_ioerror:
	ThrowIoError(cx);

bad:
	if ( pv ) {
		if ( accessSem )
			PR_CloseSemaphore(accessSem);
		if ( shm )
			PR_CloseSharedMemory(shm);
		jl_free(pv);
	}
	return false;
}


/**doc
=== Methods ===
**/

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( data [, offset] )
  Write _data_ at _offset_ in the shared memory.
**/
DEFINE_FUNCTION( write ) {


	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_MIN( 1 );

	ClassPrivate *pv;
	pv = (ClassPrivate*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	size_t offset;
	offset = 0;
	if ( JL_ARG_ISDEF(2) )
		JL_CHK( jl::getValue(cx, JL_ARG(2), &offset) );

	{

		jl::StrData data(cx);
		JL_CHK( jl::getValue(cx, JL_ARG(1), &data) );

		JL_ASSERT( sizeof(MemHeader) + offset + data.length() <= pv->size, E_DATASIZE, E_MAX, E_NUM(pv->size - sizeof(MemHeader) - offset) ); // JL_ASSERT( sizeof(MemHeader) + offset + data.length() <= pv->size, "SharedMemory too small to hold the given data." );

		JL_CHK( Lock(cx, pv) );

		MemHeader *mh;
		mh = (MemHeader*)pv->mem;
		if ( offset + data.length() > mh->currentDataLength )
			mh->currentDataLength = offset + data.length();
		memmove( (char *)pv->mem + sizeof(MemHeader) + offset, data, data.length() ); // doc. Use memmove to handle overlapping regions.
	
	}

	JL_CHK( Unlock(cx, pv) );

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( length [, offset] )
  Read _length_ bytes from _offset_ in the shared memory.
**/
DEFINE_FUNCTION( read ) {

//	jl::BufString buffer;

	JL_DEFINE_ARGS;
	JL_ASSERT_THIS_INSTANCE();

	ClassPrivate *pv;
	pv = (ClassPrivate*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	size_t offset;
	offset = 0;
	if ( JL_ARG_ISDEF(2) )
		JL_CHK( jl::getValue(cx, JL_ARG(2), &offset) );

	JL_CHK( Lock(cx, pv) );
	MemHeader *mh;
	mh = (MemHeader*)pv->mem;

	size_t dataLength;
	if ( JL_ARG_ISDEF(1) )
		JL_CHK( jl::getValue(cx, JL_ARG(1), &dataLength) );
	else
		dataLength = mh->currentDataLength;

	//uint8_t *data;
	//data = (char*)jl_malloc(dataLength +1);
	//data = JL_NewBuffer(cx, dataLength, JL_RVAL);
	//JL_CHK( data );

	//memmove( data, (char *)pv->mem + sizeof(MemHeader) + offset, dataLength ); // With memcpy, the destination cannot overlap the source at all. With memmove it can.

	BlobCreateCopy(cx, (char*)pv->mem + sizeof(MemHeader) + offset, dataLength, JL_RVAL);

	JL_CHK( Unlock(cx, pv) );

	//data[dataLength] = '\0';
	//JL_CHK( JL_NewBlob( cx, data, dataLength, JL_RVAL ) );

	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Clears the content of the shared memory.
**/
DEFINE_FUNCTION( clear ) {

	JL_DEFINE_ARGS;
		JL_ASSERT_ARGC_MIN( 1 );

	ClassPrivate *pv;
	pv = (ClassPrivate*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	JL_CHK( Lock(cx, pv) );
	MemHeader *mh;
	mh = (MemHeader*)pv->mem;
	mh->currentDataLength = 0;
	memset( (char *)pv->mem + sizeof(MemHeader), 0, pv->size - sizeof(MemHeader) );
	JL_CHK( Unlock(cx, pv) );

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Close the shared memory.
**/
DEFINE_FUNCTION( close ) {

	JL_IGNORE( argc );

	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INSTANCE();

	ClassPrivate *pv;
	pv = (ClassPrivate*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	if ( !CloseSharedMemory<js::CanGC>(JL_OBJ) )
		return ThrowIoError(cx);
	JL_SetPrivate( JL_OBJ, NULL);

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
=== Properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $STR $INAME
  Read or write the whole content of the shared memory. Setting _undefined_ as value clears the memory area.
**/
DEFINE_PROPERTY_SETTER( content ) {

	JL_IGNORE( strict, id );

	ClassPrivate *pv = (ClassPrivate*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	if ( vp.isUndefined() ) {

		JL_CHK( Lock(cx, pv) );
		MemHeader *mh = (MemHeader*)pv->mem;
		mh->currentDataLength = 0;
		memset( (char *)pv->mem + sizeof(MemHeader), 0, pv->size - sizeof(MemHeader) );
		JL_CHK( Unlock(cx, pv) );
	} else {

		jl::StrData data(cx);
		JL_CHK( jl::getValue(cx, vp, &data) );

		JL_ASSERT( sizeof(MemHeader) + data.length() <= pv->size, E_DATASIZE, E_MAX, E_NUM(pv->size - sizeof(MemHeader)) ); //JL_ASSERT( sizeof(MemHeader) + data.length() <= pv->size, "SharedMemory too small to hold the given data." );

		JL_CHK( Lock(cx, pv) );

		MemHeader *mh = (MemHeader*)pv->mem;
		if ( data.length() > mh->currentDataLength )
			mh->currentDataLength = data.length();
		memmove( (char *)pv->mem + sizeof(MemHeader), data, data.length() ); // doc. Use memmove to handle overlapping regions.

		JL_CHK( Unlock(cx, pv) );
	}
	return true;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( content ) {

	JL_DEFINE_PROP_ARGS;

	ClassPrivate *pv = (ClassPrivate*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	JL_CHK( Lock(cx, pv) );

	MemHeader *mh;
	mh = (MemHeader*)pv->mem;

	//size_t dataLength;
	//dataLength = mh->currentDataLength;
	
	//data = (char*)jl_malloc(dataLength +1);
	//data = JL_NewBuffer(cx, dataLength, vp);
	//JL_CHK( data );
	//memmove( data, (char *)pv->mem + sizeof(MemHeader), dataLength );

	JL_CHK( BlobCreateCopy(cx, (char*)pv->mem + sizeof(MemHeader), mh->currentDataLength, JL_RVAL) );

	JL_CHK( Unlock(cx, pv) );

	//data[dataLength] = '\0';
	//JL_CHK( JL_NewBlob( cx, data, dataLength, vp ) );

	return true;
	JL_BAD;
}

/*
TypeError: can't XDR class Array
...
<soubok>	why Object class do not have XDR support ?
<shaver>	because serializing object state is a very complex problem
<shaver>	and not necessary for serialization of script
<soubok>	ok thanks, I didn't know XDR was only for scripts
<shaver>	that was why it was implemented

DEFINE_PROPERTY( xdrSetter ) {

	ClassPrivate *pv = (ClassPrivate*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	JL_CHK( Lock(cx, pv) );
	MemHeader *mh = (MemHeader*)pv->mem;

	JSXDRState *xdr = JS_XDRNewMem(cx, JSXDR_ENCODE);
	JL_ASSERT( xdr, "Unable to create XDR encoder." );

	JL_CHK( JS_XDRValue( xdr, vp ) );

	uint32_t length;
	void *buffer = JS_XDRMemGetData( xdr, &length );
	JL_ASSERT( buffer, "Unable to create XDR data." );

	memmove( (char*)pv->mem + sizeof(MemHeader), buffer, length );

	mh->currentDataLength = length;

	JS_XDRDestroy( xdr );

	JL_CHK( Unlock(cx, pv) );

	return true;
}


DEFINE_PROPERTY( xdrGetter ) {

	ClassPrivate *pv = (ClassPrivate*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	JL_CHK( Lock(cx, pv) );
	MemHeader *mh = (MemHeader*)pv->mem;

	JSXDRState *xdr = JS_XDRNewMem(cx, JSXDR_DECODE);
	JL_ASSERT( xdr, "Unable to create XDR decoder." );

	JS_XDRMemSetData( xdr, (char*)pv->mem + sizeof(MemHeader), mh->currentDataLength );

	JL_CHK( JS_XDRValue(xdr, vp) );

	JS_XDRMemSetData(xdr, NULL, 0);
	JS_XDRDestroy(xdr);

	JL_CHK( Unlock(cx, pv) );
	return true;
}
*/

/**doc
=== Native Interface ===
 * *NIBufferGet*
  This object provide a BufferGet native interface an can be used in any function that support this interface. For example a Stream object.
**/

CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision$"))
	HAS_PRIVATE

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION( read )
		FUNCTION( write )
		FUNCTION( clear )
		FUNCTION( close )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY( content )
//		PROPERTY( xdr )
	END_PROPERTY_SPEC

END_CLASS

/**doc
=== Exemple ===
{{{
loadModule('jsstd');
loadModule('jsio');

var mem1 = new SharedMemory( 'mytest', 100 );
mem1.write('foo');

var mem2 = new SharedMemory( 'mytest', 100 );
print( mem2.read(3), '\n' );
}}}
**/
