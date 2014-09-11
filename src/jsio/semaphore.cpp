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
#include "../jslang/handlePub.h"


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 3533 $
 This class manages interprocess communication semaphores using a counting semaphore model similar to that which is provided in Unix and Windows platforms.
**/
BEGIN_CLASS( Semaphore )


struct ClassPrivate {
	char name[PATH_MAX];
	PRSem *semaphore;
	bool owner;
};


DEFINE_FINALIZE() {

	ClassPrivate *pv = (ClassPrivate*)JL_GetPrivateFromFinalize(obj);
	if ( !pv )
		return;

	PRStatus status;
	status = PR_CloseSemaphore(pv->semaphore);

	if ( pv->owner )
		status = PR_DeleteSemaphore(pv->name);

	JS_freeop(fop, pv);
}


/**doc
$TOC_MEMBER $INAME
 $INAME( name, [ count = 0 ] [, mode = Semaphore.IRUSR | Semaphore.IWUSR ] )
  Create or open a named semaphore with the specified name. If the named semaphore doesn't exist, the named semaphore is created.
  $H exemple
  {{{
  TBD
  }}}
**/
DEFINE_CONSTRUCTOR() {

	ClassPrivate *pv = NULL;

	JL_DEFINE_ARGS;
	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;
	JL_ASSERT_ARGC_MIN( 1 );

	PRUintn count;
	if ( JL_ARG_ISDEF(2) )
		JL_CHK( jl::getValue(cx, JL_ARG(2), &count) );
	else
		count = 0;

	PRUintn mode;
	if ( JL_ARG_ISDEF(3) )
		JL_CHK( jl::getValue(cx, JL_ARG(3), &mode) );
	else
		mode = PR_IRUSR | PR_IWUSR; // read write permission for owner.

	{

		jl::StrData name(cx);

		JL_CHK( jl::getValue(cx, JL_ARG(1), &name) );
		JL_ASSERT( name.length() < PATH_MAX, E_ARG, E_NUM(1), E_MAX, E_NUM(PATH_MAX) );

		pv = (ClassPrivate*)JS_malloc(cx, sizeof(ClassPrivate));
		JL_CHK( pv );
		pv->semaphore = NULL;

		pv->owner = true;
		pv->semaphore = PR_OpenSemaphore(name, PR_SEM_EXCL | PR_SEM_CREATE, mode, count); // fail if already exists
		if ( pv->semaphore == NULL ) {

			pv->semaphore = PR_OpenSemaphore(name, 0, 0, 0); // If PR_SEM_CREATE is not specified, the third and fourth arguments are ignored.
			if ( pv->semaphore == NULL )
				return ThrowIoError(cx);
			pv->owner = false;
		}

		jl::memcpy(pv->name, name.toBytes(), name.length());
		pv->name[name.length()] = '\0';
	
	}

	JL_SetPrivate(JL_OBJ, pv);
	return true;

bad:
	if ( pv ) {
		if ( pv->semaphore )
			PR_CloseSemaphore(pv->semaphore);
		JS_free(cx, pv);
	}
	return false;
}

/**doc
=== Methods ===
**/


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  If the value of the semaphore is > 0, decrement the value and return. If the value is 0, sleep until the value becomes > 0, then decrement the value and return. The "test and decrement" operation is performed atomically.
**/
DEFINE_FUNCTION( wait ) {

	JL_IGNORE( argc );

	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INSTANCE();

	ClassPrivate *pv;
	pv = (ClassPrivate*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	PRStatus status;
	status = PR_WaitSemaphore( pv->semaphore );
	if ( status != PR_SUCCESS )
		return ThrowIoError(cx);

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Increment the value of the named semaphore by 1.
**/
DEFINE_FUNCTION( post ) {

	JL_IGNORE( argc );

	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INSTANCE();

	ClassPrivate *pv;
	pv = (ClassPrivate*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	PRStatus status;
	status = PR_PostSemaphore( pv->semaphore );
	if ( status != PR_SUCCESS )
		return ThrowIoError(cx);

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( name ) {

	JL_DEFINE_PROP_ARGS;
	ClassPrivate *pv;
	pv = (ClassPrivate*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	JL_CHK( jl::setValue(cx, JL_RVAL, pv->name) );
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( isOwner ) {

	JL_DEFINE_PROP_ARGS;
	ClassPrivate *pv;
	pv = (ClassPrivate*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	JL_RVAL.setBoolean(pv->owner);
	return true;
	JL_BAD;
}


/**doc
=== Constants ===
**/

/**doc
  $CONST IRWXU
   read, write, execute/search by owner
  $CONST IRUSR
   read permission, owner
  $CONST IWUSR
   write permission, owner
  $CONST IXUSR
   execute/search permission, owner
  $CONST IRWXG
   read, write, execute/search by group
  $CONST IRGRP
   read permission, group
  $CONST IWGRP
   write permission, group
  $CONST IXGRP
   execute/search permission, group
  $CONST IRWXO
   read, write, execute/search by others
  $CONST IROTH
   read permission, others
  $CONST IWOTH
   write permission, others
  $CONST IXOTH
   execute/search permission, others
**/




class SemProcessEvent : public ProcessEvent2 {

	PRSem *_sem;
	bool _hasEvent;

	bool prepareWait(JSContext *cx, JS::HandleObject obj) {

		return true;
	}

	void startWait() {

		_hasEvent = false;
		PRStatus st = PR_WaitSemaphore(_sem);
		ASSERT( st == PR_SUCCESS );
		_hasEvent = true;
	}

	bool cancelWait() {

		return false;
	}

	bool endWait(bool *hasEvent, JSContext *cx, JS::HandleObject obj) {
		
		*hasEvent = _hasEvent;
	
		if ( !*hasEvent )
			return true;

		JS::RootedValue fct(cx, getSlot(0));
		if ( !fct.isUndefined() ) {

			JS::RootedValue calleeThis(cx, getSlot(1));
			JL_CHK( jl::callNoRval(cx, calleeThis, fct) );
		}

		return true;
		JL_BAD;
	}
public:

	void setSemaphore(PRSem *sem) {

		_sem = sem;
	}
};




DEFINE_FUNCTION( events ) {

	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INHERITANCE();
	JL_ASSERT_ARGC_RANGE(0, 1);

	SemProcessEvent *upe = new SemProcessEvent();
	JL_CHK( HandleCreate(cx, upe, JL_RVAL) );

	ClassPrivate *pv;
	pv = (ClassPrivate*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	upe->setSemaphore(pv->semaphore);

	if ( JL_ARG_ISDEF(1) ) {

		JL_ASSERT_ARG_IS_CALLABLE(1);
		upe->setSlot(0, JL_ARG(1));
		upe->setSlot(1, JL_OBJVAL);
	}

	return true;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision: 3533 $"))
	HAS_CONSTRUCTOR
	HAS_FINALIZE
	HAS_PRIVATE

	BEGIN_FUNCTION_SPEC
		FUNCTION( wait )
		FUNCTION( post )
		FUNCTION_ARGC( events, 1 )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_GETTER( name )
		PROPERTY_GETTER( isOwner )
	END_PROPERTY_SPEC

	BEGIN_CONST
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

	END_CONST

END_CLASS
