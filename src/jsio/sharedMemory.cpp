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
#include <string.h>

//#include <jsxdrapi.h>

#include "sharedMemory.h"

#define SEMAPHORE_EXTENSION "_sem"


struct ClassPrivate {
	char name[PATH_MAX +1];
	PRSharedMemory *shm;
	void *mem;
	unsigned int size;
	PRSem *accessSem;
};


struct MemHeader {
	unsigned int currentDataLength;
	unsigned int accessCount;
};


JSBool Lock( JSContext *cx, ClassPrivate *pv ) {

	PRStatus status = PR_WaitSemaphore( pv->accessSem );
	if ( status != PR_SUCCESS )
		return ThrowIoError(cx);
	return JS_TRUE;
}

JSBool Unlock( JSContext *cx, ClassPrivate *pv ) {

	PRStatus status = PR_PostSemaphore( pv->accessSem );
	if ( status != PR_SUCCESS )
		return ThrowIoError(cx);
	return JS_TRUE;
}


static JSBool BufferGet( JSContext *cx, JSObject *obj, const char **buf, size_t *size ) {

	ClassPrivate *pv = (ClassPrivate*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	MemHeader *mh = (MemHeader*)pv->mem;
	*buf = (char *)pv->mem + sizeof(MemHeader);
	*size = mh->currentDataLength;
	return JS_TRUE;
}


JSBool CloseSharedMemory( JSContext *cx, JSObject *obj ) {

	ClassPrivate *pv = (ClassPrivate*)JS_GetPrivate(cx, J_OBJ);

	PRStatus status;
	status = PR_WaitSemaphore( pv->accessSem );
	
	MemHeader *mh = (MemHeader*)pv->mem;

	bool isLast = (mh->accessCount == 0);
	mh->accessCount--;

	status = PR_DetachSharedMemory(pv->shm, pv->mem);
	status = PR_PostSemaphore(pv->accessSem);
	status = PR_CloseSemaphore(pv->accessSem);

	if ( isLast ) {
	
		status = PR_DeleteSharedMemory(pv->name);
		char semName[PATH_MAX];
		strcpy(semName, pv->name);
		strcat(semName, SEMAPHORE_EXTENSION);
		status = PR_DeleteSemaphore(semName);
	}

	free(pv->name);
	JS_SetPrivate(cx, J_OBJ, NULL);

	return JS_TRUE;
}


/**doc
$CLASS_HEADER
 This class manages shared memory between two or more process.
**/
BEGIN_CLASS( SharedMemory )

DEFINE_FINALIZE() {

	if ( JS_GetPrivate(cx, J_OBJ) != NULL ) {
		
		CloseSharedMemory(cx, obj);
	}
}

// doc.
// The unix implementation may use SysV IPC shared memory, Posix
// shared memory, or memory mapped files; the filename may used to
// define the namespace. On Windows, the name is significant, but
// there is no file associated with name.


/**doc
 * $INAME( name, size [, mode] )
  Creates a named shared memory area of _size_ bytes using _mode_ linux-like rights.
**/
DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();
	J_S_ASSERT_ARG_MIN( 2 );

	PRSize size;
	J_JSVAL_TO_UINT32( J_ARG(2), size );

	PRUintn mode = PR_IRUSR | PR_IWUSR; // read write permission for owner.
	if ( J_ARG_ISDEF(3) )
		J_JSVAL_TO_UINT32( J_ARG(3), mode );

	const char *name;
	J_CHK( JsvalToString(cx, &J_ARG(1), &name) );

	char semName[PATH_MAX];
	strcpy(semName, name);
	strcat(semName, SEMAPHORE_EXTENSION);

	bool isCreation = true;
	PRSem *accessSem = PR_OpenSemaphore(semName, PR_SEM_EXCL | PR_SEM_CREATE, mode, 1); // fail if already exists

	if ( accessSem == NULL ) {

		accessSem = PR_OpenSemaphore(semName, 0, 0, 0); // If PR_SEM_CREATE is not specified, the third and fourth arguments are ignored.
		if ( accessSem == NULL )
			return ThrowIoError(cx);
		isCreation = false;
	}

	PRStatus status;
	status = PR_WaitSemaphore( accessSem );
	if ( status != PR_SUCCESS )
		return ThrowIoError(cx);

	PRSharedMemory *shm = PR_OpenSharedMemory( name, size + sizeof(MemHeader), PR_SHM_CREATE, mode );
	if ( shm == NULL )
		return ThrowIoError(cx);

	void *mem = PR_AttachSharedMemory(shm, 0); // PR_SHM_READONLY
	if ( mem == NULL )
		return ThrowIoError(cx);

	ClassPrivate *pv = (ClassPrivate*)malloc( sizeof(ClassPrivate) );
	J_S_ASSERT_ALLOC( pv );

	strcpy(pv->name, name);
	pv->shm = shm;
	pv->mem = mem;
	pv->size = size + sizeof(MemHeader);
	pv->accessSem = accessSem;

	MemHeader *mh = (MemHeader*)pv->mem;

	if ( isCreation ) {
		mh->accessCount = 0;
		mh->currentDataLength = 0;
	} else
		mh->accessCount++;

	status = PR_PostSemaphore( accessSem );
	if ( status != PR_SUCCESS )
		return ThrowIoError(cx);

	J_CHK( JS_SetPrivate(cx, obj, pv) );

	J_CHK( SetBufferGetInterface(cx, obj, BufferGet) );

	return JS_TRUE;
}


/**doc
=== Methods ===
**/

/**doc
 * $INAME( data [, offset] )
  Write _data_ at _offset_ in the shared memory.
**/
DEFINE_FUNCTION_FAST( Write ) {

	J_S_ASSERT_ARG_MIN( 1 );
	ClassPrivate *pv = (ClassPrivate*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE( pv );

	PRSize offset = 0;
	if ( J_FARG_ISDEF(2) )
		J_JSVAL_TO_UINT32( J_FARG(2), offset );

	const char *data;
	size_t dataLength;
	J_CHK( JsvalToStringAndLength(cx, &J_FARG(1), &data, &dataLength) );

	J_S_ASSERT( sizeof(MemHeader) + offset + dataLength <= pv->size, "SharedMemory too small to hold the given data." );

	J_CHK( Lock(cx, pv) );

	MemHeader *mh = (MemHeader*)pv->mem;
	if ( offset + dataLength > mh->currentDataLength )
		mh->currentDataLength = offset + dataLength;
	memmove(	(char *)pv->mem + sizeof(MemHeader) + offset, data, dataLength );

	J_CHK( Unlock(cx, pv) );

	return JS_TRUE;
}


/**doc
 * $STR $INAME( length [, offset] )
  Read _length_ bytes from _offset_ in the shared memory.
**/
DEFINE_FUNCTION_FAST( Read ) {

	ClassPrivate *pv = (ClassPrivate*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE( pv );

	unsigned int offset = 0;
	if ( J_FARG_ISDEF(2) )
		J_JSVAL_TO_UINT32( J_FARG(2), offset );

	J_CHK( Lock(cx, pv) );
	MemHeader *mh = (MemHeader*)pv->mem;
	
	unsigned int dataLength;
	if ( J_FARG_ISDEF(1) )
		J_JSVAL_TO_UINT32( J_FARG(1), dataLength );
	else
		dataLength = mh->currentDataLength;

	char *data = (char*)JS_malloc(cx, dataLength +1);
	data[dataLength] = '\0';

	memmove(	data, (char *)pv->mem + sizeof(MemHeader) + offset, dataLength );

	J_CHK( Unlock(cx, pv) );

	J_CHK( J_NewBlob( cx, data, dataLength, J_FRVAL ) );
	
	return JS_TRUE;
}


/**doc
 * $INAME()
  Clears the content of the shared memory.
**/
DEFINE_FUNCTION_FAST( Clear ) {

	J_S_ASSERT_ARG_MIN( 1 );
	ClassPrivate *pv = (ClassPrivate*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE( pv );

	J_CHK( Lock(cx, pv) );
	MemHeader *mh = (MemHeader*)pv->mem;
	mh->currentDataLength = 0;
	memset( (char *)pv->mem + sizeof(MemHeader), 0, pv->size - sizeof(MemHeader) );
	J_CHK( Unlock(cx, pv) );

	return JS_TRUE;
}


/**doc
 * $INAME()
  Close the shared memory.
**/
DEFINE_FUNCTION_FAST( Close ) {

	ClassPrivate *pv = (ClassPrivate*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE( pv );
	J_CHK( CloseSharedMemory(cx, J_FOBJ) );
	return JS_TRUE;
}


/**doc
=== Properties ===
**/

/**doc
 * $STR $INAME
  Read or write the whole content of the shared memory. Setting <undefined> as value clears the memory area.
**/
DEFINE_PROPERTY( contentSetter ) {

	ClassPrivate *pv = (ClassPrivate*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );

	if ( JSVAL_IS_VOID( *vp ) ) {

		J_CHK( Lock(cx, pv) );
		MemHeader *mh = (MemHeader*)pv->mem;
		mh->currentDataLength = 0;
		memset( (char *)pv->mem + sizeof(MemHeader), 0, pv->size - sizeof(MemHeader) );
		J_CHK( Unlock(cx, pv) );
	} else {

		const char *data;
		size_t dataLength;
		J_CHK( JsvalToStringAndLength(cx, vp, &data, &dataLength) );

		J_S_ASSERT( sizeof(MemHeader) + dataLength <= pv->size, "SharedMemory too small to hold the given data." );

		J_CHK( Lock(cx, pv) );

		MemHeader *mh = (MemHeader*)pv->mem;
		if ( dataLength > mh->currentDataLength )
			mh->currentDataLength = dataLength;
		memmove(	(char *)pv->mem + sizeof(MemHeader), data, dataLength );

		J_CHK( Unlock(cx, pv) );
	}
	return JS_TRUE;
}


DEFINE_PROPERTY( contentGetter ) {

	ClassPrivate *pv = (ClassPrivate*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );

	J_CHK( Lock(cx, pv) );

	MemHeader *mh = (MemHeader*)pv->mem;

	unsigned int dataLength = mh->currentDataLength;
	char *data = (char*)JS_malloc(cx, dataLength +1);
	data[dataLength] = '\0';

	memmove(	data, (char *)pv->mem + sizeof(MemHeader), dataLength );

	J_CHK( Unlock(cx, pv) );

	J_CHK( J_NewBlob( cx, data, dataLength, vp ) );

	return JS_TRUE;
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

	ClassPrivate *pv = (ClassPrivate*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );

	J_CHK( Lock(cx, pv) );
	MemHeader *mh = (MemHeader*)pv->mem;

	JSXDRState *xdr = JS_XDRNewMem(cx, JSXDR_ENCODE);
	J_S_ASSERT( xdr, "Unable to create XDR encoder." );

	J_CHK( JS_XDRValue( xdr, vp ) );

	uint32 length;
	void *buffer = JS_XDRMemGetData( xdr, &length );
	J_S_ASSERT( buffer, "Unable to create XDR data." );

	memmove( (char*)pv->mem + sizeof(MemHeader), buffer, length );

	mh->currentDataLength = length;

	JS_XDRDestroy( xdr );

	J_CHK( Unlock(cx, pv) );

	return JS_TRUE;
}


DEFINE_PROPERTY( xdrGetter ) {

	ClassPrivate *pv = (ClassPrivate*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );

	J_CHK( Lock(cx, pv) );
	MemHeader *mh = (MemHeader*)pv->mem;
	
	JSXDRState *xdr = JS_XDRNewMem(cx, JSXDR_DECODE);
	J_S_ASSERT( xdr, "Unable to create XDR decoder." );

	JS_XDRMemSetData( xdr, (char*)pv->mem + sizeof(MemHeader), mh->currentDataLength );

	J_CHK( JS_XDRValue(xdr, vp) );

	JS_XDRMemSetData(xdr, NULL, 0);
	JS_XDRDestroy(xdr);

	J_CHK( Unlock(cx, pv) );
	return JS_TRUE;
}
*/

/**doc
=== Native Interface ===
 *NIBufferGet*
**/

CONFIGURE_CLASS

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST( Read )
		FUNCTION_FAST( Write )
		FUNCTION_FAST( Clear )
		FUNCTION_FAST( Close )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY( content )
//		PROPERTY( xdr )
	END_PROPERTY_SPEC

	HAS_PRIVATE

END_CLASS

/**doc
=== Exemple ===
{{{
LoadModule('jsstd');
LoadModule('jsio');

var mem1 = new SharedMemory( 'mytest', 100 );
mem1.Write('foo');

var mem2 = new SharedMemory( 'mytest', 100 );
Print( mem2.Read(3), '\n' );
}}}
**/
