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

#include "semaphore.h"


BEGIN_CLASS( Semaphore )


struct ClassPrivate {
	char name[PATH_MAX +1];
	PRSem *semaphore;
	bool owner;
};


DEFINE_FINALIZE() {

	ClassPrivate *pv = (ClassPrivate*)JS_GetPrivate(cx, J_OBJ);
	if ( pv != NULL ) {

		PRStatus status;
		status = PR_CloseSemaphore(pv->semaphore);

		if ( pv->owner )
			status = PR_DeleteSemaphore(pv->name);

		free(pv->name);
		JS_SetPrivate(cx, J_OBJ, NULL);
	}
}



DEFINE_CONSTRUCTOR() {

	RT_ASSERT_CONSTRUCTING( _class );
	RT_ASSERT_ARGC( 2 );

	const char *name;
	size_t nameLength;
	RT_JSVAL_TO_STRING_AND_LENGTH( J_ARG(1), name, nameLength );

	RT_ASSERT( nameLength < PATH_MAX, "Semaphoer name too long." );

	PRUintn count = 0;
	if ( J_ARG_ISDEF(2) )
		RT_JSVAL_TO_INT32( J_ARG(2), count );

	PRUintn mode = PR_IRUSR | PR_IWUSR; // read write permission for owner.
	if ( J_ARG_ISDEF(3) )
		RT_JSVAL_TO_INT32( J_ARG(3), mode );

	bool isCreation = true;
	PRSem *semaphore = PR_OpenSemaphore(name, PR_SEM_EXCL | PR_SEM_CREATE, mode, count); // fail if already exists

	if ( semaphore == NULL ) {

		semaphore = PR_OpenSemaphore(name, 0, 0, 0); // If PR_SEM_CREATE is not specified, the third and fourth arguments are ignored.
		if ( semaphore == NULL )
			return ThrowIoError(cx);
		isCreation = false;
	}

	ClassPrivate *pv = (ClassPrivate*)malloc( sizeof(ClassPrivate) );
	RT_ASSERT_ALLOC( pv );

	strcpy( pv->name, name );
	pv->semaphore = semaphore;
	pv->owner = isCreation;

	RT_CHECK_CALL( JS_SetPrivate(cx, J_OBJ, pv) );

	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Wait ) {

	ClassPrivate *pv = (ClassPrivate*)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE( pv );

	PRStatus status;
	status = PR_WaitSemaphore( pv->semaphore );
	if ( status != PR_SUCCESS )
		return ThrowIoError(cx);

	return JS_TRUE;
}

DEFINE_FUNCTION_FAST( Post ) {

	ClassPrivate *pv = (ClassPrivate*)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE( pv );
	
	PRStatus status;
	status = PR_PostSemaphore( pv->semaphore );
	if ( status != PR_SUCCESS )
		return ThrowIoError(cx);

	return JS_TRUE;
}

CONFIGURE_CLASS

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST( Wait )
		FUNCTION_FAST( Post )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
	END_PROPERTY_SPEC

	HAS_PRIVATE

END_CLASS