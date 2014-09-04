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


DECLARE_CLASS( Descriptor )
DECLARE_CLASS( Pipe )
DECLARE_CLASS( Process )

#define SLOT_PROCESS_STDIN 0
#define SLOT_PROCESS_STDOUT 1
#define SLOT_PROCESS_STDERR 2

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( Process )

DEFINE_FINALIZE() {

	JL_IGNORE(fop);

	PRProcess *process;
	process = (PRProcess*)JL_GetPrivateFromFinalize(obj);
	if ( process )
		PR_DetachProcess(process); // may crash ?
}

#ifdef WIN
	#include <direct.h>
#endif

/**doc
$TOC_MEMBER $INAME
 $VAL $INAME( path , [ argv ] , [ currentDir = undefined ], [ stdioRedirect = true ]  )
  This function starts a new process optionaly using the JavaScript Array _argv_ for arguments or _undefined_ for no arguments.
  $H note
   The new process inherits the environment of the parent process.
  $H exemple
  {{{
  var p = new Process( 'c:\\windows\\System32\\cmd.exe', ['/c', 'dir', 'c:'] );
  p.wait();
  print( p.stdout.read() );
  }}}
**/
DEFINE_CONSTRUCTOR() {

	PRProcessAttr *psattr = NULL;
	PRProcess *process = NULL;

	JL_DEFINE_ARGS;
	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;
	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT( !JL_ARG_ISDEF(2) || jl::isArrayLike(cx, JL_ARG(2)), E_ARG, E_NUM(2), E_TYPE, E_TY_ARRAY );

	{

		JS::AutoCheckCannotGC nogc;
		jl::BufString path;
		jl::BufString currentDir;

		JL_CHK( jl::getValue(cx, JL_ARG(1), &path) );

		uint32_t processArgc;
		const char **processArgv;
		if ( JL_ARG_ISDEF(2) ) {

			JS::RootedObject argObj(cx, &JL_ARG(2).toObject());
			JL_CHK( JS_GetArrayLength(cx, argObj, &processArgc) );
			processArgc++; // +1 is argv[0]
			processArgv = (const char**)alloca(sizeof(char**) * (processArgc +1)); // +1 because the NULL list terminator.
			JL_ASSERT_ALLOC( processArgv );

			JS::AutoCheckCannotGC nogc;
			for ( uint32_t i = 1; i < processArgc; ++i ) {

				jl::BufString tmp;

				//JL_CHK( JL_GetElement(cx, argObj, i -1, &propVal) ); // -1 because 0 is reserved to argv[0]
				//JL_CHK( jl::getValue(cx, propVal, &tmp) ); // warning: GC on the returned buffer !

				jl::getElement(cx, argObj, i - 1, &tmp);

				processArgv[i] = tmp.toStringZ<char*>();
			}
		} else {

			processArgc = 0 +1; // +1 is argv[0]
			processArgv = (const char**)alloca(sizeof(char**) * (processArgc +1)); // +1 is NULL
			JL_ASSERT_ALLOC( processArgv );
		}

		processArgv[0] = path.toStringZ<const char *>();
		processArgv[processArgc] = NULL;

		if ( JL_ARG_ISDEF(3) )
			JL_CHK( jl::getValue(cx, JL_ARG(3), &currentDir) );

		bool stdioRedirect;
		if ( JL_ARG_ISDEF(4) )
			JL_CHK( jl::getValue(cx, JL_ARG(4), &stdioRedirect) );
		else
			stdioRedirect = true;

		PRFileDesc *stdin_child, *stdout_child, *stderr_child;
		PRFileDesc *stdin_parent, *stdout_parent, *stderr_parent;
		IFDEBUG( stdin_child = stdout_child = stderr_child = stdin_parent = stdout_parent = stderr_parent = NULL ); // avoid "potentially uninitialized local variable" warning

		if ( stdioRedirect || currentDir ) {

			psattr = PR_NewProcessAttr();
			JL_CHKB( psattr, bad_throw );

			if ( currentDir ) {
			
				JL_CHKB( PR_ProcessAttrSetCurrentDirectory(psattr, currentDir) == PR_SUCCESS, bad_throw );
			}

			if ( stdioRedirect ) {

				JL_CHKB( PR_CreatePipe(&stdin_parent, &stdin_child) == PR_SUCCESS, bad_throw );
				JL_CHKB( PR_CreatePipe(&stdout_parent, &stdout_child) == PR_SUCCESS, bad_throw );
				JL_CHKB( PR_CreatePipe(&stderr_parent, &stderr_child) == PR_SUCCESS, bad_throw );
				PR_ProcessAttrSetStdioRedirect(psattr, PR_StandardInput, stdin_child);
				PR_ProcessAttrSetStdioRedirect(psattr, PR_StandardOutput, stdout_child);
				PR_ProcessAttrSetStdioRedirect(psattr, PR_StandardError, stderr_child);
			}
		}

		//	JL_CHKB( PR_ProcessAttrSetCurrentDirectory(psattr, buf) == PR_SUCCESS, bad_throw );
		// PR_ProcessAttrSetInheritableFD

		// cf. bug 113095 -  PR_CreateProcess reports success even when it fails to create the process. (https://bugzilla.mozilla.org/show_bug.cgi?id=113095)
		// workaround: check the rights and execution flag before runiong the file
		process = PR_CreateProcess(processArgv[0], (char *const *)processArgv, NULL, psattr);

		//printf("***[%p - %s - %d]\n", processArgv[0], processArgv[0], processArgv[0][0]);

		if ( psattr != NULL ) {
	
			PR_DestroyProcessAttr(psattr);
			psattr = NULL;
		}

		if ( JL_ARG_ISDEF(2) ) // see GetStrZOwnership
			for ( uint32_t i = 0; i < processArgc - 1; ++i )
				jl_free( const_cast<char*>(processArgv[i+1]) );

		//free(processArgv); // alloca do not need free

		if ( stdioRedirect ) {

			JL_CHKB( PR_Close(stderr_child) == PR_SUCCESS, bad_throw );
			JL_CHKB( PR_Close(stdout_child) == PR_SUCCESS, bad_throw );
			JL_CHKB( PR_Close(stdin_child) == PR_SUCCESS, bad_throw );

			if ( !process ) {

				JL_CHKB( PR_Close(stderr_parent) == PR_SUCCESS, bad_throw );
				JL_CHKB( PR_Close(stdout_parent) == PR_SUCCESS, bad_throw );
				JL_CHKB( PR_Close(stdin_parent) == PR_SUCCESS, bad_throw );
			}
		}

		JL_CHKB( process != NULL, bad_throw );

		if ( stdioRedirect ) {

			JS::RootedValue tmp(cx);

			JS::RootedObject fdInObj(cx, jl::newObjectWithGivenProto( cx, JL_CLASS(Pipe), JL_CLASS_PROTOTYPE(cx, Pipe)));
			tmp.setObject(*fdInObj);
			JL_CHK( JL_SetReservedSlot(JL_OBJ, SLOT_PROCESS_STDIN, tmp) );
			JL_SetPrivate(  fdInObj, stdin_parent );

			JS::RootedObject fdOutObj(cx, jl::newObjectWithGivenProto( cx, JL_CLASS(Pipe), JL_CLASS_PROTOTYPE(cx, Pipe)));
			tmp.setObject(*fdOutObj);
			JL_CHK( JL_SetReservedSlot(JL_OBJ, SLOT_PROCESS_STDOUT, tmp) );
			JL_SetPrivate(  fdOutObj, stdout_parent );

			JS::RootedObject fdErrObj(cx, jl::newObjectWithGivenProto( cx, JL_CLASS(Pipe), JL_CLASS_PROTOTYPE(cx, Pipe)));
			tmp.setObject(*fdErrObj);
			JL_CHK( JL_SetReservedSlot(JL_OBJ, SLOT_PROCESS_STDERR, tmp) );
			JL_SetPrivate(  fdErrObj, stderr_parent );
		}

		JL_SetPrivate(JL_OBJ, process);
	
	}

	return true;

bad_throw:
	ThrowIoError(cx);

bad:
	if ( psattr != NULL )
		PR_DestroyProcessAttr(psattr);
	if ( process)
		PR_DetachProcess(process); 
	return false;
}


/**doc
=== Methods ===
**/


/**doc
$TOC_MEMBER $INAME
 $INT $INAME()
  The function waits the end of the nondetached process and returns its exit code. This function will fail if the process has beed detached.
  $H note
   This function will wait endless if stdin, stdout or stderr pipes are not read (emptied).
  $H note
   In bash, `true;echo $?` prints `0` and `false;echo $?` prints `1`
**/
DEFINE_FUNCTION( wait ) {

	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INSTANCE();

	PRProcess *process;
	process = (PRProcess*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(process);
	PRInt32 exitValue;
	JL_CHK( PR_WaitProcess(process, &exitValue) == PR_SUCCESS );
	JL_SetPrivate(JL_OBJ, NULL);
	JL_CHK( jl::setValue(cx, JL_RVAL, exitValue) );
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME()
  This function detaches the process. A detached process does not need to be and cannot be reaped.
**/
DEFINE_FUNCTION( detach ) {

	JL_IGNORE( argc );

	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INSTANCE();

	PRProcess *process;
	process = (PRProcess*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(process);
	JL_CHK( PR_DetachProcess(process) == PR_SUCCESS );
	JL_SetPrivate(JL_OBJ, NULL); // On return, the value of process becomes an invalid pointer and should not be passed to other functions.
	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME()
  Terminates the process.
**/
DEFINE_FUNCTION( kill ) {

	JL_IGNORE( argc );

	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INSTANCE();

	PRProcess *process;
	process = (PRProcess*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(process);
	JL_CHK( PR_KillProcess(process) == PR_SUCCESS );
	JL_SetPrivate(JL_OBJ, NULL); // Invalidates the current process pointer.
	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
=== Properties ===
**/


/**doc
$TOC_MEMBER $INAME
 $TYPE Pipe $INAME()
  Is the stdin pipe to the running process.
**/
DEFINE_PROPERTY_GETTER( stdin ) {

	JL_DEFINE_PROP_ARGS;
	JL_ASSERT_THIS_INSTANCE();

	JL_CHK( JL_GetReservedSlot(JL_OBJ, SLOT_PROCESS_STDIN, JL_RVAL) );
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $TYPE Pipe $INAME()
  Is the stdout pipe to the running process.
**/
DEFINE_PROPERTY_GETTER( stdout ) {

	JL_DEFINE_PROP_ARGS;
	JL_ASSERT_THIS_INSTANCE();

	JL_CHK( JL_GetReservedSlot(JL_OBJ, SLOT_PROCESS_STDOUT, JL_RVAL) );

	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $TYPE Pipe $INAME()
  Is the stderr pipe to the running process.
**/
DEFINE_PROPERTY_GETTER( stderr ) {

	JL_DEFINE_PROP_ARGS;
	JL_ASSERT_THIS_INSTANCE();

	JL_CHK( JL_GetReservedSlot(JL_OBJ, SLOT_PROCESS_STDERR, JL_RVAL) );
	return true;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision$"))

	HAS_PRIVATE
	HAS_RESERVED_SLOTS( 3 )

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION( wait )
		FUNCTION( detach )
		FUNCTION( kill )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_GETTER( stdin )
		PROPERTY_GETTER( stdout )
		PROPERTY_GETTER( stderr )
	END_PROPERTY_SPEC

END_CLASS
