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


JSBool SharedMemoryBufferGet( JSContext *cx, JSObject *obj, JLStr *str ) {

	ClassPrivate *pv = (ClassPrivate*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( pv );
	MemHeader *mh;
	mh = (MemHeader*)pv->mem;
//	*buf = (char *)pv->mem + sizeof(MemHeader);
//	*size = mh->currentDataLength;
	*str = JLStr(((const char *)pv->mem) + sizeof(MemHeader), mh->currentDataLength, false);

	return JS_TRUE;
	JL_BAD;
}


JSBool CloseSharedMemory( JSContext *cx, JSObject *obj ) {

	ClassPrivate *pv = (ClassPrivate*)JL_GetPrivate(cx, JL_OBJ);
	JL_S_ASSERT_RESOURCE(pv);

	JL_CHKB( PR_WaitSemaphore( pv->accessSem ) == PR_SUCCESS, bad_ioerror );

	MemHeader *mh;
	mh = (MemHeader*)pv->mem;

	bool isLast;
	isLast = (mh->accessCount == 0);
	mh->accessCount--;

	JL_CHKB( PR_DetachSharedMemory(pv->shm, pv->mem) == PR_SUCCESS, bad_ioerror );
	JL_CHKB( PR_CloseSharedMemory(pv->shm) == PR_SUCCESS, bad_ioerror );

	JL_CHKB( PR_PostSemaphore(pv->accessSem) == PR_SUCCESS, bad_ioerror );
	JL_CHKB( PR_CloseSemaphore(pv->accessSem) == PR_SUCCESS, bad_ioerror );

	if ( isLast ) {

		JL_CHKB( PR_DeleteSharedMemory(pv->name) == PR_SUCCESS, bad_ioerror );
		char semName[PATH_MAX];
		strcpy(semName, pv->name);
		strcat(semName, SEMAPHORE_EXTENSION);
		JL_CHKB( PR_DeleteSemaphore(semName) == PR_SUCCESS, bad_ioerror );
	}

	JS_free(cx, pv);
	JL_SetPrivate(cx, JL_OBJ, NULL);

	return JS_TRUE;
bad_ioerror:
	ThrowIoError(cx);
	JL_BAD;
}


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
 This class manages shared memory between two or more process.
**/
BEGIN_CLASS( SharedMemory )

DEFINE_FINALIZE() {

	if ( !JL_GetPrivate(cx, JL_OBJ) )
		return;
	CloseSharedMemory(cx, obj);
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

	JLStr name;
	JL_S_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

	JL_S_ASSERT_ARG_MIN( 2 );

	size_t size;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &size) );

	unsigned int mode;
	mode = PR_IRUSR | PR_IWUSR; // read write permission for owner.
	if ( JL_ARG_ISDEF(3) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &mode) );

	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &name) );

	char semName[PATH_MAX];
	strcpy(semName, name);
	strcat(semName, SEMAPHORE_EXTENSION);

	bool isCreation;
	isCreation = true;
	PRSem *accessSem;
	accessSem = PR_OpenSemaphore(semName, PR_SEM_EXCL | PR_SEM_CREATE, mode, 1); // fail if already exists

	if ( accessSem == NULL ) {

		accessSem = PR_OpenSemaphore(semName, 0, 0, 0); // If PR_SEM_CREATE is not specified, the third and fourth arguments are ignored.
		JL_CHKB( accessSem != NULL, bad_ioerror );
		isCreation = false;
	}

	JL_CHKB( PR_WaitSemaphore( accessSem ) == PR_SUCCESS, bad_ioerror );

	PRSharedMemory *shm;
	shm = PR_OpenSharedMemory( name, size + sizeof(MemHeader), PR_SHM_CREATE, mode );
	JL_CHKB( shm != NULL, bad_ioerror ); // PR_SHM_READONLY

	void *mem;
	mem = PR_AttachSharedMemory(shm, 0);
	JL_CHKB( mem != NULL, bad_ioerror ); // PR_SHM_READONLY

	ClassPrivate *pv;
	pv = (ClassPrivate*)JS_malloc(cx, sizeof(ClassPrivate));
	JL_CHK( pv );

	strcpy(pv->name, name);
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

	JL_CHKB( PR_PostSemaphore( accessSem ) == PR_SUCCESS, bad_ioerror );
	JL_SetPrivate(cx, obj, pv);
	JL_CHK( SetBufferGetInterface(cx, obj, SharedMemoryBufferGet) );

	return JS_TRUE;

bad_ioerror:
	ThrowIoError(cx);
	JL_BAD;
}


/**doc
=== Methods ===
**/

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( data [, offset] )
  Write _data_ at _offset_ in the shared memory.
**/
DEFINE_FUNCTION( Write ) {

	JLStr data;
	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_ARG_MIN( 1 );

	ClassPrivate *pv;
	pv = (ClassPrivate*)JL_GetPrivate(cx, JL_OBJ);
	JL_S_ASSERT_RESOURCE( pv );

	size_t offset;
	offset = 0;
	if ( JL_ARG_ISDEF(2) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &offset) );

//	const char *data;
//	size_t dataLength;
//	JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(1), &data, &dataLength) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &data) );
	
	JL_S_ASSERT( sizeof(MemHeader) + offset + data.Length() <= pv->size, "SharedMemory too small to hold the given data." );

	JL_CHK( Lock(cx, pv) );

	MemHeader *mh;
	mh = (MemHeader*)pv->mem;
	if ( offset + data.Length() > mh->currentDataLength )
		mh->currentDataLength = offset + data.Length();
	memmove( (char *)pv->mem + sizeof(MemHeader) + offset, data.GetConstStr(), data.Length() ); // doc. Use memmove to handle overlapping regions.

	JL_CHK( Unlock(cx, pv) );

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( length [, offset] )
  Read _length_ bytes from _offset_ in the shared memory.
**/
DEFINE_FUNCTION( Read ) {

	JL_DEFINE_FUNCTION_OBJ;

	ClassPrivate *pv = (ClassPrivate*)JL_GetPrivate(cx, JL_OBJ);
	JL_S_ASSERT_RESOURCE( pv );

	size_t offset;
	offset = 0;
	if ( JL_ARG_ISDEF(2) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &offset) );

	JL_CHK( Lock(cx, pv) );
	MemHeader *mh;
	mh = (MemHeader*)pv->mem;

	size_t dataLength;
	if ( JL_ARG_ISDEF(1) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &dataLength) );
	else
		dataLength = mh->currentDataLength;

	char *data;
	data = (char*)JS_malloc(cx, dataLength +1);
	JL_CHK( data );

	memmove( data, (char *)pv->mem + sizeof(MemHeader) + offset, dataLength );

	JL_CHK( Unlock(cx, pv) );

	data[dataLength] = '\0';
	JL_CHK( JL_NewBlob( cx, data, dataLength, JL_RVAL ) );

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Clears the content of the shared memory.
**/
DEFINE_FUNCTION( Clear ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_ARG_MIN( 1 );

	ClassPrivate *pv;
	pv = (ClassPrivate*)JL_GetPrivate(cx, JL_OBJ);
	JL_S_ASSERT_RESOURCE( pv );

	JL_CHK( Lock(cx, pv) );
	MemHeader *mh;
	mh = (MemHeader*)pv->mem;
	mh->currentDataLength = 0;
	memset( (char *)pv->mem + sizeof(MemHeader), 0, pv->size - sizeof(MemHeader) );
	JL_CHK( Unlock(cx, pv) );

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Close the shared memory.
**/
DEFINE_FUNCTION( Close ) {

	JL_DEFINE_FUNCTION_OBJ;
	ClassPrivate *pv = (ClassPrivate*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( pv );
	JL_CHK( CloseSharedMemory(cx, obj) );

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
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

	ClassPrivate *pv = (ClassPrivate*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( pv );

	if ( JSVAL_IS_VOID( *vp ) ) {

		JL_CHK( Lock(cx, pv) );
		MemHeader *mh = (MemHeader*)pv->mem;
		mh->currentDataLength = 0;
		memset( (char *)pv->mem + sizeof(MemHeader), 0, pv->size - sizeof(MemHeader) );
		JL_CHK( Unlock(cx, pv) );
	} else {

		JLStr data;
//		const char *data;
//		size_t dataLength;
//		JL_CHK( JL_JsvalToStringAndLength(cx, vp, &data, &dataLength) );
		JL_CHK( JL_JsvalToNative(cx, *vp, &data) );

		JL_S_ASSERT( sizeof(MemHeader) + data.Length() <= pv->size, "SharedMemory too small to hold the given data." );

		JL_CHK( Lock(cx, pv) );

		MemHeader *mh = (MemHeader*)pv->mem;
		if ( data.Length() > mh->currentDataLength )
			mh->currentDataLength = data.Length();
		memmove( (char *)pv->mem + sizeof(MemHeader), data.GetConstStr(), data.Length() ); // doc. Use memmove to handle overlapping regions.

		JL_CHK( Unlock(cx, pv) );
	}
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( content ) {

	ClassPrivate *pv = (ClassPrivate*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( pv );

	JL_CHK( Lock(cx, pv) );

	MemHeader *mh;
	mh = (MemHeader*)pv->mem;

	size_t dataLength;
	dataLength = mh->currentDataLength;
	char *data;
	data = (char*)JS_malloc(cx, dataLength +1);
	JL_CHK( data );

	memmove( data, (char *)pv->mem + sizeof(MemHeader), dataLength );

	JL_CHK( Unlock(cx, pv) );

	data[dataLength] = '\0';
	JL_CHK( JL_NewBlob( cx, data, dataLength, vp ) );

	return JS_TRUE;
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

	ClassPrivate *pv = (ClassPrivate*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( pv );

	JL_CHK( Lock(cx, pv) );
	MemHeader *mh = (MemHeader*)pv->mem;

	JSXDRState *xdr = JS_XDRNewMem(cx, JSXDR_ENCODE);
	JL_S_ASSERT( xdr, "Unable to create XDR encoder." );

	JL_CHK( JS_XDRValue( xdr, vp ) );

	uint32 length;
	void *buffer = JS_XDRMemGetData( xdr, &length );
	JL_S_ASSERT( buffer, "Unable to create XDR data." );

	memmove( (char*)pv->mem + sizeof(MemHeader), buffer, length );

	mh->currentDataLength = length;

	JS_XDRDestroy( xdr );

	JL_CHK( Unlock(cx, pv) );

	return JS_TRUE;
}


DEFINE_PROPERTY( xdrGetter ) {

	ClassPrivate *pv = (ClassPrivate*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( pv );

	JL_CHK( Lock(cx, pv) );
	MemHeader *mh = (MemHeader*)pv->mem;

	JSXDRState *xdr = JS_XDRNewMem(cx, JSXDR_DECODE);
	JL_S_ASSERT( xdr, "Unable to create XDR decoder." );

	JS_XDRMemSetData( xdr, (char*)pv->mem + sizeof(MemHeader), mh->currentDataLength );

	JL_CHK( JS_XDRValue(xdr, vp) );

	JS_XDRMemSetData(xdr, NULL, 0);
	JS_XDRDestroy(xdr);

	JL_CHK( Unlock(cx, pv) );
	return JS_TRUE;
}
*/

/**doc
=== Native Interface ===
 * *NIBufferGet*
  This object provide a BufferGet native interface an can be used in any function that support this interface. For example a Stream object.
**/

CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION( Read )
		FUNCTION( Write )
		FUNCTION( Clear )
		FUNCTION( Close )
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
