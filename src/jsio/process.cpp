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

	PRProcess *process;
	process = (PRProcess*)JL_GetPrivate(cx, obj);
	if ( process )
		PR_DetachProcess(process); // may crash ?
}

#include <direct.h>

/**doc
$TOC_MEMBER $INAME
 $VAL $INAME( path , [ argv ] , [ stdioRedirect = true ]  )
  This function starts a new process optionaly using the JavaScript Array _argv_ for arguments or _undefined_ for no arguments.
  $H note
   The new process inherits the environment of the parent process.
  $H exemple
  {{{
  var p = new Process( 'c:\\windows\\System32\\cmd.exe', ['/c', 'dir', 'c:'] );
  p.Wait();
  Print( p.stdout.Read() );
  }}}
**/
DEFINE_CONSTRUCTOR() {

	JLStr path;
	JL_S_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT( !JL_ARG_ISDEF(2) || JL_IsArray(cx, JL_ARG(2)), "Invalid 2nd argument" );

	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &path) );

	PRProcessAttr *psattr;
	psattr = PR_NewProcessAttr();
	JL_CHK( psattr );

	int processArgc;
	const char **processArgv;
	if ( JL_ARG_ISDEF(2) ) {

		JSIdArray *idArray;
		idArray = JS_Enumerate( cx, JSVAL_TO_OBJECT(JL_ARG(2)) ); // make a kind of auto-ptr for this
		processArgc = idArray->length +1; // +1 is argv[0]
		processArgv = (const char**)alloca(sizeof(const char**) * (processArgc +1)); // +1 because the NULL list terminator.
		JL_S_ASSERT_ALLOC( processArgv );

		for ( int i=0; i<processArgc -1; i++ ) { // -1 because argv[0]

			JLStr tmp;
			jsval propVal;
			JL_CHK( JS_IdToValue(cx, idArray->vector[i], &propVal) );
			JL_CHK( JS_GetElement(cx, JSVAL_TO_OBJECT(JL_ARG(2)), JSVAL_TO_INT(propVal), &propVal) ); // (TBD) optimize
			JL_CHK( JL_JsvalToNative(cx, propVal, &tmp) ); // warning: GC on the returned buffer !
			processArgv[i+1] = tmp.GetStrZOwnership();
		}
		JS_DestroyIdArray( cx, idArray ); // (TBD) free it on error too !
	} else {

		processArgc = 0 +1; // +1 is argv[0]
		processArgv = (const char**)alloca(sizeof(const char**) * (processArgc +1)); // +1 is NULL
		JL_S_ASSERT_ALLOC( processArgv );
	}

	processArgv[0] = path.GetConstStrZ();
	processArgv[processArgc] = NULL;

	bool stdioRedirect;
	if ( JL_ARG_ISDEF(3) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &stdioRedirect) );
	else
		stdioRedirect = true;

	PRFileDesc *stdin_child, *stdout_child, *stderr_child;
	PRFileDesc *stdin_parent, *stdout_parent, *stderr_parent;
	IFDEBUG( stdin_child = stdout_child = stderr_child = stdin_parent = stdout_parent = stderr_parent = 0 ); // avoid "potentially uninitialized local variable" warning

	if ( stdioRedirect ) {

		JL_CHKB( PR_CreatePipe(&stdin_parent, &stdin_child) == PR_SUCCESS, bad_throw );
		JL_CHKB( PR_CreatePipe(&stdout_parent, &stdout_child) == PR_SUCCESS, bad_throw );
		JL_CHKB( PR_CreatePipe(&stderr_parent, &stderr_child) == PR_SUCCESS, bad_throw );

		PR_ProcessAttrSetStdioRedirect(psattr, PR_StandardInput, stdin_child);
		PR_ProcessAttrSetStdioRedirect(psattr, PR_StandardOutput, stdout_child);
		PR_ProcessAttrSetStdioRedirect(psattr, PR_StandardError, stderr_child);
	}

	//	JL_CHKB( PR_ProcessAttrSetCurrentDirectory(psattr, buf) == PR_SUCCESS, bad_throw );
	// PR_ProcessAttrSetInheritableFD

	// cf. bug 113095 -  PR_CreateProcess reports success even when it fails to create the process. (https://bugzilla.mozilla.org/show_bug.cgi?id=113095)
	// workaround: check the rights and execution flag before runiong the file
	PRProcess *process;
	process = PR_CreateProcess(path, (char * const *)processArgv, NULL, psattr); // (TBD) avoid cast to (char * const *)

	PR_DestroyProcessAttr(psattr);

	if ( JL_ARG_ISDEF(2) ) // see GetStrZOwnership
		for ( int i = 0; i < processArgc - 1; ++i )
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
	JL_SetPrivate(cx, obj, (void*)process);

	if ( stdioRedirect ) {

		JSObject *fdInObj;
		fdInObj = JS_NewObjectWithGivenProto( cx, JL_CLASS(Pipe), JL_PROTOTYPE(cx, Pipe), NULL );
		JL_CHK( JL_SetReservedSlot(cx, obj, SLOT_PROCESS_STDIN, OBJECT_TO_JSVAL(fdInObj)) );
		JL_SetPrivate( cx, fdInObj, stdin_parent );

		JSObject *fdOutObj;
		fdOutObj = JS_NewObjectWithGivenProto( cx, JL_CLASS(Pipe), JL_PROTOTYPE(cx, Pipe), NULL );
		JL_CHK( JL_SetReservedSlot(cx, obj, SLOT_PROCESS_STDOUT, OBJECT_TO_JSVAL(fdOutObj)) );
		JL_SetPrivate( cx, fdOutObj, stdout_parent );

		JSObject *fdErrObj;
		fdErrObj = JS_NewObjectWithGivenProto( cx, JL_CLASS(Pipe), JL_PROTOTYPE(cx, Pipe), NULL );
		JL_CHK( JL_SetReservedSlot(cx, obj, SLOT_PROCESS_STDERR, OBJECT_TO_JSVAL(fdErrObj)) );
		JL_SetPrivate( cx, fdErrObj, stderr_parent );
	}

	return JS_TRUE;

bad_throw:
	ThrowIoError(cx);
bad:
	return JS_FALSE;
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
DEFINE_FUNCTION( Wait ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_CLASS( obj, JL_THIS_CLASS );
	PRProcess *process;
	process = (PRProcess*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(process);
	PRInt32 exitValue;
	JL_CHK( PR_WaitProcess(process, &exitValue) == PR_SUCCESS );
	JL_SetPrivate(cx, obj, NULL);
	JL_CHK( JL_NativeToJsval(cx, exitValue, JL_RVAL) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME()
  This function detaches the process. A detached process does not need to be and cannot be reaped.
**/
DEFINE_FUNCTION( Detach ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_CLASS( obj, JL_THIS_CLASS );
	PRProcess *process;
	process = (PRProcess*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(process);
	JL_CHK( PR_DetachProcess(process) == PR_SUCCESS );
	JL_SetPrivate(cx, obj, NULL); // On return, the value of process becomes an invalid pointer and should not be passed to other functions.
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME()
  Terminates the process.
**/
DEFINE_FUNCTION( Kill ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_CLASS( obj, JL_THIS_CLASS );
	PRProcess *process;
	process = (PRProcess*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(process);
	JL_CHK( PR_KillProcess(process) == PR_SUCCESS );
	JL_SetPrivate(cx, obj, NULL); // Invalidates the current process pointer.
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
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
DEFINE_PROPERTY( stdin ) {

	JL_S_ASSERT_CLASS( obj, JL_THIS_CLASS );
	JL_CHK( JL_GetReservedSlot(cx, obj, SLOT_PROCESS_STDIN, vp) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $TYPE Pipe $INAME()
  Is the stdout pipe to the running process.
**/
DEFINE_PROPERTY( stdout ) {

	JL_S_ASSERT_CLASS( obj, JL_THIS_CLASS );
	JL_CHK( JL_GetReservedSlot(cx, obj, SLOT_PROCESS_STDOUT, vp) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $TYPE Pipe $INAME()
  Is the stderr pipe to the running process.
**/
DEFINE_PROPERTY( stderr ) {

	JL_S_ASSERT_CLASS( obj, JL_THIS_CLASS );
	JL_CHK( JL_GetReservedSlot(cx, obj, SLOT_PROCESS_STDERR, vp) );
	return JS_TRUE;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))

	HAS_PRIVATE
	HAS_RESERVED_SLOTS( 3 )

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION( Wait )
		FUNCTION( Detach )
		FUNCTION( Kill )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ( stdin )
		PROPERTY_READ( stdout )
		PROPERTY_READ( stderr )
	END_PROPERTY_SPEC

END_CLASS
