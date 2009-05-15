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

#include "descriptor.h"
#include "pipe.h"
#include "process.h"

#define SLOT_PROCESS_STDIN 0
#define SLOT_PROCESS_STDOUT 1
#define SLOT_PROCESS_STDERR 2

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 2212 $
**/
BEGIN_CLASS( Process )

DEFINE_FINALIZE() {

	PRProcess *process;
	process = (PRProcess*)JS_GetPrivate(cx, obj);
	if ( process )
		PR_DetachProcess(process); // may crash ?
}


/**doc
$TOC_MEMBER $INAME
 $VAL $INAME( path [ , argv ] )
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

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();
	J_S_ASSERT_ARG_MIN(1);
	J_S_ASSERT( !J_ARG_ISDEF(2) || JsvalIsArray(cx, J_ARG(2)), "Invalid 2nd argument" );

	const char *path;
	J_CHK( JsvalToString(cx, &J_ARG(1), &path) );

	int processArgc;
	const char **processArgv;
	if ( J_ARG_ISDEF(2) ) {

		JSIdArray *idArray;
		idArray = JS_Enumerate( cx, JSVAL_TO_OBJECT(J_ARG(2)) ); // make a kind of auto-ptr for this
		processArgc = idArray->length +1; // +1 is argv[0]
		processArgv = (const char**)malloc(sizeof(const char**) * (processArgc +1)); // +1 because the NULL list terminator.
		J_S_ASSERT_ALLOC( processArgv );

		for ( int i=0; i<processArgc -1; i++ ) { // -1 because argv[0]

			jsval propVal;
			J_CHK( JS_IdToValue(cx, idArray->vector[i], &propVal) );
			J_CHK( JS_GetElement(cx, JSVAL_TO_OBJECT(J_ARG(2)), JSVAL_TO_INT(propVal), &propVal) ); // (TBD) optimize
			const char *tmp;
			J_CHK( JsvalToString(cx, &propVal, &tmp) ); // warning: GC on the returned buffer !
			processArgv[i+1] = tmp;
		}
		JS_DestroyIdArray( cx, idArray ); // (TBD) free it on error too !
	} else {

		processArgc = 0 +1; // +1 is argv[0]
		processArgv = (const char**)malloc(sizeof(const char**) * (processArgc +1)); // +1 is NULL
		J_S_ASSERT_ALLOC( processArgv );
	}

	processArgv[0] = path;
	processArgv[processArgc] = NULL;

	PRFileDesc *stdin_child, *stdout_child, *stderr_child;
	PRFileDesc *stdin_parent, *stdout_parent, *stderr_parent;

	J_CHKB( PR_CreatePipe(&stdin_parent, &stdin_child) == PR_SUCCESS, bad_throw );
	J_CHKB( PR_CreatePipe(&stdout_parent, &stdout_child) == PR_SUCCESS, bad_throw );
	J_CHKB( PR_CreatePipe(&stderr_parent, &stderr_child) == PR_SUCCESS, bad_throw );

	PRProcessAttr *psattr;
	psattr = PR_NewProcessAttr();

	PR_ProcessAttrSetStdioRedirect(psattr, PR_StandardInput, stdin_child);
	PR_ProcessAttrSetStdioRedirect(psattr, PR_StandardOutput, stdout_child);
	PR_ProcessAttrSetStdioRedirect(psattr, PR_StandardError, stderr_child);
	// PR_ProcessAttrSetCurrentDirectory ?

	// cf. bug 113095 -  PR_CreateProcess reports success even when it fails to create the process. (https://bugzilla.mozilla.org/show_bug.cgi?id=113095)
	// workaround: check the rights and execution flag before runiong the file
	PRProcess *process;
	process = PR_CreateProcess(path, (char * const *)processArgv, NULL, psattr); // (TBD) avoid cast to (char * const *)

	PR_DestroyProcessAttr(psattr);
	free(processArgv);

	J_CHKB( PR_Close(stderr_child) == PR_SUCCESS, bad_throw );
	J_CHKB( PR_Close(stdout_child) == PR_SUCCESS, bad_throw );
	J_CHKB( PR_Close(stdin_child) == PR_SUCCESS, bad_throw );
	if ( !process ) {

		J_CHKB( PR_Close(stderr_parent) == PR_SUCCESS, bad_throw );
		J_CHKB( PR_Close(stdout_parent) == PR_SUCCESS, bad_throw );
		J_CHKB( PR_Close(stdin_parent) == PR_SUCCESS, bad_throw );
	}
	J_CHKB( process != NULL, bad_throw );
	J_CHK( JS_SetPrivate(cx, obj, (void*)process) );

	JSObject *fdInObj;
	fdInObj = JS_NewObject( cx, classPipe, NULL, NULL );
	J_CHK( JS_SetReservedSlot(cx, obj, SLOT_PROCESS_STDIN, OBJECT_TO_JSVAL(fdInObj)) );
	J_CHK( JS_SetPrivate( cx, fdInObj, stdin_parent ) );

	JSObject *fdOutObj;
	fdOutObj = JS_NewObject( cx, classPipe, NULL, NULL );
	J_CHK( JS_SetReservedSlot(cx, obj, SLOT_PROCESS_STDOUT, OBJECT_TO_JSVAL(fdOutObj)) );
	J_CHK( JS_SetPrivate( cx, fdOutObj, stdout_parent ) );

	JSObject *fdErrObj;
	fdErrObj = JS_NewObject( cx, classPipe, NULL, NULL );
	J_CHK( JS_SetReservedSlot(cx, obj, SLOT_PROCESS_STDERR, OBJECT_TO_JSVAL(fdErrObj)) );
	J_CHK( JS_SetPrivate( cx, fdErrObj, stderr_parent ) );

	return JS_TRUE;

bad_throw:
	ThrowIoError(cx);
bad:
	return JS_FALSE;
}



/**doc
$TOC_MEMBER $INAME
 $INT $INAME()
  The function waits the end of the nondetached process and returns its exit code. This function will fail if the process has beed detached.
  $H note
   In bash, `true;echo $?` prints `0` and `false;echo $?` prints `1`
**/
DEFINE_FUNCTION_FAST( Wait ) {

	J_S_ASSERT_CLASS( J_FOBJ, _class );
	PRProcess *process;
	process = (PRProcess*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(process);
	PRInt32 exitValue;
	J_CHK( PR_WaitProcess(process, &exitValue) == PR_SUCCESS );
	J_CHK( JS_SetPrivate(cx, J_FOBJ, NULL) );
	J_CHK( IntToJsval(cx, exitValue, J_FRVAL) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME()
  This function detaches the process. A detached process does not need to be and cannot be reaped.
**/
DEFINE_FUNCTION_FAST( Detach ) {

	J_S_ASSERT_CLASS( J_FOBJ, _class );
	PRProcess *process;
	process = (PRProcess*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(process);
	J_CHK( PR_DetachProcess(process) == PR_SUCCESS );
	J_CHK( JS_SetPrivate(cx, J_FOBJ, NULL) ); // On return, the value of process becomes an invalid pointer and should not be passed to other functions.
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME()
  Terminates the process.
**/
DEFINE_FUNCTION_FAST( Kill ) {

	J_S_ASSERT_CLASS( J_FOBJ, _class );
	PRProcess *process;
	process = (PRProcess*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(process);
	J_CHK( PR_KillProcess(process) == PR_SUCCESS );
	J_CHK( JS_SetPrivate(cx, J_FOBJ, NULL) ); // Invalidates the current process pointer.
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE Pipe $INAME()
  Is the stdin pipe to the running process.
**/
DEFINE_PROPERTY( stdin ) {

	J_S_ASSERT_CLASS( obj, _class );
	J_CHK( JS_GetReservedSlot(cx, obj, SLOT_PROCESS_STDIN, vp) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $TYPE Pipe $INAME()
  Is the stdout pipe to the running process.
**/
DEFINE_PROPERTY( stdout ) {

	J_S_ASSERT_CLASS( obj, _class );
	J_CHK( JS_GetReservedSlot(cx, obj, SLOT_PROCESS_STDOUT, vp) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $TYPE Pipe $INAME()
  Is the stderr pipe to the running process.
**/
DEFINE_PROPERTY( stderr ) {

	J_S_ASSERT_CLASS( obj, _class );
	J_CHK( JS_GetReservedSlot(cx, obj, SLOT_PROCESS_STDERR, vp) );
	return JS_TRUE;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(SvnRevToInt("$Revision: 2212 $"))

	HAS_PRIVATE
	HAS_RESERVED_SLOTS( 3 )

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST( Wait )
		FUNCTION_FAST( Detach )
		FUNCTION_FAST( Kill )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ( stdin )
		PROPERTY_READ( stdout )
		PROPERTY_READ( stderr )
	END_PROPERTY_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
	END_STATIC_PROPERTY_SPEC

	BEGIN_CONST_DOUBLE_SPEC
	END_CONST_DOUBLE_SPEC

END_CLASS
