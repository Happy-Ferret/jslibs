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


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
 This class manages interprocess communication semaphores using a counting semaphore model similar to that which is provided in Unix and Windows platforms.
**/
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


/**doc
 * $INAME( name, [ count = 0 ] [, mode = Semaphore.IRUSR | Semaphore.IWUSR ] )
  Create or open a named semaphore with the specified name. If the named semaphore doesn't exist, the named semaphore is created.
  $H exemple
  {{{
  TBD
  }}}
**/
DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();
	J_S_ASSERT_ARG_MIN( 1 );

	PRUintn count;
	count = 0;
	if ( J_ARG_ISDEF(2) )
		J_CHK( JsvalToUInt(cx, J_ARG(2), &count) );

	PRUintn mode;
	mode = PR_IRUSR | PR_IWUSR; // read write permission for owner.
	if ( J_ARG_ISDEF(3) )
		J_CHK( JsvalToUInt(cx, J_ARG(3), &mode) );

	const char *name;
	size_t nameLength;
	J_CHK( JsvalToStringAndLength(cx, &J_ARG(1), &name, &nameLength) );
	J_S_ASSERT( nameLength < PATH_MAX, "Semaphore name too long." );

	bool isCreation;
	isCreation = true;
	PRSem *semaphore;
	semaphore = PR_OpenSemaphore(name, PR_SEM_EXCL | PR_SEM_CREATE, mode, count); // fail if already exists

	if ( semaphore == NULL ) {

		semaphore = PR_OpenSemaphore(name, 0, 0, 0); // If PR_SEM_CREATE is not specified, the third and fourth arguments are ignored.
		if ( semaphore == NULL )
			return ThrowIoError(cx);
		isCreation = false;
	}

	ClassPrivate *pv;
	pv = (ClassPrivate*)malloc( sizeof(ClassPrivate) );
	J_S_ASSERT_ALLOC( pv );

//	strcpy( pv->name, name ); // (TBD) use memcpy instead ?
	memcpy(pv->name, name, nameLength);
	pv->name[nameLength] = '\0';

	pv->semaphore = semaphore;
	pv->owner = isCreation;

	J_CHK( JS_SetPrivate(cx, J_OBJ, pv) );

	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Methods ===
**/


/**doc
 * $VOID $INAME()
  If the value of the semaphore is > 0, decrement the value and return. If the value is 0, sleep until the value becomes > 0, then decrement the value and return. The "test and decrement" operation is performed atomically.
**/
DEFINE_FUNCTION_FAST( Wait ) {

	ClassPrivate *pv = (ClassPrivate*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE( pv );

	PRStatus status;
	status = PR_WaitSemaphore( pv->semaphore );
	if ( status != PR_SUCCESS )
		return ThrowIoError(cx);
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

/**doc
 * $VOID $INAME()
  Increment the value of the named semaphore by 1.
**/
DEFINE_FUNCTION_FAST( Post ) {

	ClassPrivate *pv = (ClassPrivate*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE( pv );

	PRStatus status;
	status = PR_PostSemaphore( pv->semaphore );
	if ( status != PR_SUCCESS )
		return ThrowIoError(cx);
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}



/**doc
=== Constants ===
**/

/**doc
  * Semaphore.`IRWXU`
   read, write, execute/search by owner 
  * Semaphore.`IRUSR`
   read permission, owner 
  * Semaphore.`IWUSR`
   write permission, owner 
  * Semaphore.`IXUSR`
   execute/search permission, owner 
  * Semaphore.`IRWXG`
   read, write, execute/search by group 
  * Semaphore.`IRGRP`
   read permission, group 
  * Semaphore.`IWGRP`
   write permission, group 
  * Semaphore.`IXGRP`
   execute/search permission, group 
  * Semaphore.`IRWXO`
   read, write, execute/search by others 
  * Semaphore.`IROTH`
   read permission, others 
  * Semaphore.`IWOTH`
   write permission, others 
  * Semaphore.`IXOTH`
   execute/search permission, others 
**/


CONFIGURE_CLASS

	REVISION(SvnRevToInt("$Revision$"))
	HAS_CONSTRUCTOR
	HAS_FINALIZE
	HAS_PRIVATE

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST( Wait )
		FUNCTION_FAST( Post )
	END_FUNCTION_SPEC

	BEGIN_CONST_INTEGER_SPEC
// PR_Open flags

		CONST_INTEGER( IRWXU, PR_IRWXU )
		CONST_INTEGER( IRUSR, PR_IRUSR )
		CONST_INTEGER( IWUSR, PR_IWUSR )
		CONST_INTEGER( IXUSR, PR_IXUSR )
		CONST_INTEGER( IRWXG, PR_IRWXG )
		CONST_INTEGER( IRGRP, PR_IRGRP )
		CONST_INTEGER( IWGRP, PR_IWGRP )
		CONST_INTEGER( IXGRP, PR_IXGRP )
		CONST_INTEGER( IRWXO, PR_IRWXO )
		CONST_INTEGER( IROTH, PR_IROTH )
		CONST_INTEGER( IWOTH, PR_IWOTH )
		CONST_INTEGER( IXOTH, PR_IXOTH )

	END_CONST_INTEGER_SPEC

END_CLASS
