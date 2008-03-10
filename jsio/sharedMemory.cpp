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
#include "../common/jsNativeInterface.h"

#include <pprio.h> // nspr/include/nspr/private

#include "error.h"

#include "sharedMemory.h"

#define RESERVED_LENGTH 4

struct ClassPrivate {

	char name[MAX_PATH];
	PRSharedMemory *shm;
	void *mem;
	unsigned int size;
	PRSem *accessSem;
};


BEGIN_CLASS( SharedMemory )

DEFINE_FINALIZE() {

	ClassPrivate *pv = (ClassPrivate*)JS_GetPrivate(cx, J_OBJ);
	if ( pv != NULL ) {

		PRStatus status;
		status = PR_WaitSemaphore( pv->accessSem );

		(*(int*)pv->mem)--;
		bool isLast = (*(int*)pv->mem == 0); // true if we are the last user of the shared memory
		status = PR_DetachSharedMemory(pv->shm, pv->mem);


		status = PR_PostSemaphore(pv->accessSem);
		status = PR_CloseSemaphore(pv->accessSem);

		if ( isLast ) {
		
			status = PR_DeleteSharedMemory(pv->name);

			char semName[MAX_PATH];
			strcpy(semName, pv->name);
			strcat(semName, "_jsiosem");

			status = PR_DeleteSemaphore(semName);
		}

		free(pv->name);
		JS_SetPrivate(cx, J_OBJ, NULL);
	}
}


DEFINE_CONSTRUCTOR() {

	RT_ASSERT_CONSTRUCTING( _class );
	RT_ASSERT_ARGC( 2 );

	PRStatus status;

	char *name;
	RT_JSVAL_TO_STRING( J_ARG(1), name );

	PRSize size;
	RT_JSVAL_TO_INT32( J_ARG(2), size );

	PRUintn mode = PR_IRUSR|PR_IWUSR | PR_IRWXG|PR_IRGRP; // read write permission for owner and group

	char semName[MAX_PATH];
	strcpy(semName, name);
	strcat(semName, "_jsiosem");

	PRSem *isCreation = PR_OpenSemaphore(semName, PR_SEM_EXCL, 0, 0); // fail if already exists
	PRSem *accessSem = PR_OpenSemaphore(semName, PR_SEM_CREATE, mode, 1); // If PR_SEM_CREATE is not specified, the third and fourth arguments are ignored.
	if ( accessSem == NULL )
		return ThrowIoError(cx);

	status = PR_WaitSemaphore( accessSem );
	if ( status != PR_SUCCESS )
		return ThrowIoError(cx);


	PRSharedMemory *shm = PR_OpenSharedMemory( name, size + RESERVED_LENGTH, PR_SHM_CREATE, mode );
	if ( shm == NULL )
		return ThrowIoError(cx);

	void *mem = PR_AttachSharedMemory(shm, 0); // PR_SHM_READONLY
	if ( mem == NULL )
		return ThrowIoError(cx);

	ClassPrivate *pv = (ClassPrivate*)malloc( sizeof(ClassPrivate) );
	RT_ASSERT_ALLOC( pv );

	strcpy(pv->name, name);
	pv->shm = shm;
	pv->mem = mem;
	pv->size = size + RESERVED_LENGTH;
	pv->accessSem = accessSem;

	if ( isCreation )
		(*(int*)pv->mem) = 0;

	(*(int*)pv->mem)++;

	status = PR_PostSemaphore( accessSem );
	if ( status != PR_SUCCESS )
		return ThrowIoError(cx);

	RT_CHECK_CALL( JS_SetPrivate(cx, J_OBJ, pv) );

	return JS_TRUE;
}


DEFINE_FUNCTION( Write ) {

	RT_ASSERT_ARGC( 1 );

	char *data;
	int dataLength;
	RT_JSVAL_TO_STRING_AND_LENGTH( J_ARG(1), data, dataLength );

	PRSize offset = 0;
	if ( J_ARG_ISDEF(2) )
		RT_JSVAL_TO_INT32( J_ARG(2), offset );

	ClassPrivate *pv = (ClassPrivate*)JS_GetPrivate(cx, J_OBJ);
	RT_ASSERT_RESOURCE( pv );

	PRStatus status;
	status = PR_WaitSemaphore( pv->accessSem );
	if ( status != PR_SUCCESS )
		return ThrowIoError(cx);

	memmove(	(char *)pv->mem + RESERVED_LENGTH + offset, data, dataLength );

	status = PR_PostSemaphore( pv->accessSem );
	if ( status != PR_SUCCESS )
		return ThrowIoError(cx);

	return JS_TRUE;
}


DEFINE_FUNCTION( Read ) {

	RT_ASSERT_ARGC( 1 );

	int dataLength;
	RT_JSVAL_TO_INT32( J_ARG(1), dataLength );

	PRSize offset = 0;
	if ( J_ARG_ISDEF(2) )
		RT_JSVAL_TO_INT32( J_ARG(2), offset );

	ClassPrivate *pv = (ClassPrivate*)JS_GetPrivate(cx, J_OBJ);
	RT_ASSERT_RESOURCE( pv );

	char *data = (char*)JS_malloc(cx, dataLength +1);

	PRStatus status;
	status = PR_WaitSemaphore( pv->accessSem );
	if ( status != PR_SUCCESS )
		return ThrowIoError(cx);

	memmove(	data, (char *)pv->mem + RESERVED_LENGTH + offset, dataLength );

	status = PR_PostSemaphore( pv->accessSem );
	if ( status != PR_SUCCESS )
		return ThrowIoError(cx);

	
	JSString *jss = JS_NewString(cx, data, dataLength);
	RT_ASSERT_ALLOC( jss );

	*J_RVAL = STRING_TO_JSVAL( jss );

	return JS_TRUE;
}


CONFIGURE_CLASS

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION( Read )
		FUNCTION( Write )
	END_FUNCTION_SPEC

	HAS_PRIVATE

END_CLASS
