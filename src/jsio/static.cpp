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

#include "static.h"

#include "descriptor.h"
#include "pipe.h"

/**doc fileIndex:topmost **/

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_STATIC


/**doc
=== Static functions ===
**/

/**doc
$TOC_MEMBER $INAME
 $INT $INAME( _descriptorArray_ [, _timeout_ = undefined ] )
  This function listen for a readable, writable or exception event on each descriptor in _descriptorArray_.
  When an event occurs, the function tries to call the corresponding property (function) on the descriptor.
  $LF
  The function returns the number of events that occurs or 0 if the function timed out.
  $H note
   The property is NOT cleared by the function.
   If you want to stop receiving an event, you have to remove the property by yourself. eg. `delete socket1.writable`.
  $LF
  The second argument, _timeout_, defines the number of milliseconds the function has to wait before returning if no event has occured.
  _timeout_ can be undefined OR omited, in this case, no timeout is used.
  $H beware
   No timeout means that the function will wait endless for events.
  $H example
  {{{
  socket1.readable = function(soc) { Print( soc.Recv(); ) }
  var count = Poll( [socket1, socket2], 1000 );
  if ( count == 0 )
    Print( 'Nothing has been recived' );
  }}}
**/
DEFINE_FUNCTION( Poll ) {

	PRInt32 result;

	// NSPR Poll Method:
	//   http://www.mozilla.org/projects/nspr/tech-notes/poll-method.html

	// http://developer.mozilla.org/en/docs/PR_Poll

	J_S_ASSERT_ARG_MIN( 1 );

	J_S_ASSERT_ARRAY( J_ARG(1) );
	JSIdArray *idArray;
	idArray = JS_Enumerate( cx, JSVAL_TO_OBJECT(J_ARG(1)) ); // make a kind of auto-ptr for this

	PRIntervalTime pr_timeout;
	if ( J_ARG_ISDEF(2) ) {

		PRUint32 tmp;
		J_CHK( JsvalToUInt(cx, J_ARG(2), &tmp) );
		pr_timeout = PR_MillisecondsToInterval(tmp);
	} else {

		pr_timeout = PR_INTERVAL_NO_TIMEOUT;
	}

	if ( idArray->length == 0 ) { // optimization

		JS_DestroyIdArray(cx, idArray);
		result = PR_Poll( NULL, 0, pr_timeout ); // we can replace this by a delay, but the function is Pool, not Sleep
		if ( result == -1 )
			return ThrowIoError(cx);
		*rval = JSVAL_ZERO;
		return JS_TRUE;
	}

	PRPollDesc staticPollDesc[32];
	PRPollDesc *pollDesc;
	pollDesc = staticPollDesc; // Optimization to avoid dynamic allocation when it is possible

	if ( idArray->length > (signed)(sizeof(staticPollDesc) / sizeof(staticPollDesc[0])) )
		pollDesc = (PRPollDesc*) malloc(idArray->length * sizeof(PRPollDesc));

	jsint i;
	for ( i = 0; i < idArray->length; i++ ) {

		jsval propVal;
		J_CHK( JS_IdToValue(cx, idArray->vector[i], &propVal ));
		J_CHK( JS_GetElement(cx, JSVAL_TO_OBJECT(J_ARG(1)), JSVAL_TO_INT(propVal), &propVal ));
		J_S_ASSERT_OBJECT( propVal );
		JSObject *fdObj = JSVAL_TO_OBJECT( propVal );
		J_S_ASSERT( InheritFrom(cx, fdObj, classDescriptor), J__ERRMSG_INVALID_CLASS );
		PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, fdObj );
//		J_S_ASSERT_RESOURCE( fd ); // fd == NULL is supported !

		pollDesc[i].fd = fd; // fd is A pointer to a PRFileDesc object representing a socket or a pollable event.  This field can be set to NULL to indicate to PR_Poll that this PRFileDesc object should be ignored.
		pollDesc[i].in_flags = 0;
		pollDesc[i].out_flags = 0;
		jsval prop;

		if ( JS_GetProperty( cx, fdObj, "writable", &prop ) == JS_FALSE )
			goto failed;
		if ( !JSVAL_IS_VOID( prop ) )
			pollDesc[i].in_flags |= PR_POLL_WRITE;

		if ( JS_GetProperty( cx, fdObj, "readable", &prop ) == JS_FALSE )
			goto failed;
		if ( !JSVAL_IS_VOID( prop ) )
			pollDesc[i].in_flags |= PR_POLL_READ;

		if ( JS_GetProperty( cx, fdObj, "hangup", &prop ) == JS_FALSE )
			goto failed;
		if ( !JSVAL_IS_VOID( prop ) )
			pollDesc[i].in_flags |= PR_POLL_HUP;

		if ( JS_GetProperty( cx, fdObj, "exception", &prop ) == JS_FALSE )
			goto failed;
		if ( !JSVAL_IS_VOID( prop ) )
			pollDesc[i].in_flags |= PR_POLL_EXCEPT;

		if ( JS_GetProperty( cx, fdObj, "error", &prop ) == JS_FALSE )
			goto failed;
		if ( !JSVAL_IS_VOID( prop ) )
			pollDesc[i].in_flags |= PR_POLL_ERR;
	}

	result = PR_Poll( pollDesc, idArray->length, pr_timeout );
	if ( result == -1 ) {  // failed. see PR_GetError()

		ThrowIoError(cx); // returns later
		goto failed;
	}

	if ( result > 0 ) { // has event(s)

		for ( i = 0; i < idArray->length; i++ ) {

			jsval arrayItem;
			J_CHK( JS_IdToValue(cx, idArray->vector[i], &arrayItem) );
			J_CHK( JS_GetElement(cx, JSVAL_TO_OBJECT(J_ARG(1)), JSVAL_TO_INT(arrayItem), &arrayItem) );
			if ( JSVAL_IS_VOID( arrayItem ) ) // socket has been removed from the list while js func "poll()" is runing
				continue;
			JSObject *fdObj = JSVAL_TO_OBJECT( arrayItem ); //JS_ValueToObject
			jsval prop, ret;
			PRInt16 outFlag = pollDesc[i].out_flags;
			jsval cbArgv[2] = { arrayItem, (outFlag & PR_POLL_HUP) ? JSVAL_TRUE : JSVAL_FALSE }; // fd, hup_flag

			if ( outFlag & PR_POLL_ERR ) {

				JS_GetProperty( cx, fdObj, "error", &prop );
				if ( JS_TypeOfValue( cx, prop ) == JSTYPE_FUNCTION )
					if ( JS_CallFunctionValue( cx, fdObj, prop, sizeof(cbArgv)/sizeof(*cbArgv), cbArgv, &ret ) == JS_FALSE ) // JS_CallFunction() DO NOT WORK !!!
						goto failed;
			}
			if (JS_IsExceptionPending(cx))
				goto failed;

			if ( outFlag & PR_POLL_EXCEPT ) {

				JS_GetProperty( cx, fdObj, "exception", &prop );
				if (JS_TypeOfValue( cx, prop ) == JSTYPE_FUNCTION )
					if ( JS_CallFunctionValue( cx, fdObj, prop, sizeof(cbArgv)/sizeof(*cbArgv), cbArgv, &ret ) == JS_FALSE ) // JS_CallFunction() DO NOT WORK !!!
						goto failed;
			}
			if (JS_IsExceptionPending(cx))
				goto failed;

			if ( outFlag & PR_POLL_HUP ) {

				JS_GetProperty( cx, fdObj, "hangup", &prop );
				if ( JS_TypeOfValue( cx, prop ) == JSTYPE_FUNCTION )
					if ( JS_CallFunctionValue( cx, fdObj, prop, sizeof(cbArgv)/sizeof(*cbArgv), cbArgv, &ret ) == JS_FALSE ) // JS_CallFunction() DO NOT WORK !!!
						goto failed;
			}
			if (JS_IsExceptionPending(cx))
				goto failed;

			if ( outFlag & PR_POLL_READ ) {

				JS_GetProperty( cx, fdObj, "readable", &prop );
				if ( JS_TypeOfValue( cx, prop ) == JSTYPE_FUNCTION )
					if ( JS_CallFunctionValue( cx, fdObj, prop, sizeof(cbArgv)/sizeof(*cbArgv), cbArgv, &ret ) == JS_FALSE ) // JS_CallFunction() DO NOT WORK !!!
						goto failed;
			}
			if (JS_IsExceptionPending(cx))
				goto failed;

			if ( outFlag & PR_POLL_WRITE ) {

				JS_GetProperty( cx, fdObj, "writable", &prop );
				if ( JS_TypeOfValue( cx, prop ) == JSTYPE_FUNCTION )
					if ( JS_CallFunctionValue( cx, fdObj, prop, sizeof(cbArgv)/sizeof(*cbArgv), cbArgv, &ret ) == JS_FALSE ) // JS_CallFunction() DO NOT WORK !!!
						goto failed;
			}
			if (JS_IsExceptionPending(cx))
				goto failed;
		}
	}

	*rval = INT_TO_JSVAL( result );
	if ( pollDesc != staticPollDesc )
		free(pollDesc);
	JS_DestroyIdArray(cx, idArray);
	return JS_TRUE;

failed: // goto is the cheaper solution
	if ( pollDesc != staticPollDesc )
		free(pollDesc);
	JS_DestroyIdArray( cx, idArray );
	return JS_FALSE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME( _descriptor_ )
  Returns true if the _descriptor_ can be read without blocking.
**/
DEFINE_FUNCTION( IsReadable ) {

	J_S_ASSERT_ARG_MIN( 1 );

	JSObject *descriptorObj;
	descriptorObj = JSVAL_TO_OBJECT( J_ARG(1) );
	J_S_ASSERT( InheritFrom(cx, descriptorObj, classDescriptor), J__ERRMSG_INVALID_CLASS );
	PRFileDesc *fd;
	fd = (PRFileDesc *)JS_GetPrivate( cx, descriptorObj );
//	J_S_ASSERT_RESOURCE( fd ); // fd == NULL is supported !

	PRIntervalTime prTimeout;
	if ( J_ARG_ISDEF(2) ) {

		PRUint32 timeout;
		J_CHK( JsvalToUInt(cx, J_ARG(2), &timeout) );
		prTimeout = PR_MillisecondsToInterval(timeout);
	} else
		prTimeout = PR_INTERVAL_NO_WAIT; //PR_INTERVAL_NO_TIMEOUT;

	PRPollDesc desc; // = { fd, PR_POLL_READ, 0 };
	desc.fd = fd;
	desc.in_flags = PR_POLL_READ;
	desc.out_flags = 0;

	PRInt32 result;
	result = PR_Poll( &desc, 1, prTimeout );
	if ( result == -1 ) // error
		return ThrowIoError(cx);
	*rval = ( result == 1 && (desc.out_flags & PR_POLL_READ) != 0 ) ? JSVAL_TRUE : JSVAL_FALSE;
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME( _descriptor_ )
  Returns true if the _descriptor_ can be write without blocking.
**/
DEFINE_FUNCTION( IsWritable ) {

	J_S_ASSERT_ARG_MIN( 1 );

	JSObject *descriptorObj;
	descriptorObj = JSVAL_TO_OBJECT( J_ARG(1) );
	J_S_ASSERT( InheritFrom(cx, descriptorObj, classDescriptor), J__ERRMSG_INVALID_CLASS );
	PRFileDesc *fd;
	fd = (PRFileDesc *)JS_GetPrivate( cx, descriptorObj );
//	J_S_ASSERT_RESOURCE( fd ); // fd == NULL is supported !

	PRIntervalTime prTimeout;
	if ( J_ARG_ISDEF(2) ) {

		PRUint32 timeout;
		J_CHK( JsvalToUInt(cx, J_ARG(2), &timeout) );
		prTimeout = PR_MillisecondsToInterval(timeout);
	} else
		prTimeout = PR_INTERVAL_NO_WAIT; //PR_INTERVAL_NO_TIMEOUT;

	PRPollDesc desc; // = { fd, PR_POLL_WRITE, 0 };
	desc.fd = fd;
	desc.in_flags = PR_POLL_WRITE;
	desc.out_flags = 0;

	PRInt32 result;
	result = PR_Poll( &desc, 1, prTimeout );
	if ( result == -1 ) // error
		return ThrowIoError(cx);
	*rval = ( result == 1 && (desc.out_flags & PR_POLL_WRITE) != 0 ) ? JSVAL_TRUE : JSVAL_FALSE;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME()
  Returns the milliseconds value of NSPR's free-running interval timer.
**/
DEFINE_FUNCTION( IntervalNow ) {

	PRUint32 interval = PR_IntervalToMilliseconds( PR_IntervalNow() );
	JS_NewNumberValue( cx, interval, rval );
	return JS_TRUE;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME()
  Returns the microseconds value of NSPR's free-running interval timer.
**/
DEFINE_FUNCTION_FAST( UIntervalNow ) {

	PRUint32 interval = PR_IntervalToMicroseconds( PR_IntervalNow() );
	JS_NewNumberValue( cx, interval, &JS_RVAL(cx, vp) );
	return JS_TRUE;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( _milliseconds_ )
  Sleeps _milliseconds_ milliseconds.
**/
DEFINE_FUNCTION( Sleep ) {

	uint32 timeout;
	J_CHK( JS_ValueToECMAUint32( cx, J_ARG(1), &timeout ) );
	PR_Sleep( PR_MillisecondsToInterval(timeout) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( name )
  Retrieve the value of the given environment variable.
**/
DEFINE_FUNCTION( GetEnv ) {

	J_S_ASSERT_ARG_MIN(1);
	const char *name;
	J_CHK( JsvalToString(cx, &J_ARG(1), &name) );
	char* value;
	value = PR_GetEnv(name); // If the environment variable is not defined, the function returns NULL.
	if ( value != NULL ) { // this will cause an 'undefined' return value

//		JSString *jsstr = JS_NewExternalString(cx, (jschar*)value, strlen(value), JS_AddExternalStringFinalizer(NULL)); only works with unicode strings
		JSString *jsstr = JS_NewStringCopyZ(cx,value);
		J_S_ASSERT_ALLOC( jsstr );
		*rval = STRING_TO_JSVAL(jsstr);
	}
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( size )
  Provides, depending on platform, a random value.
  The length of the random value is dependent on platform and the platform's ability to provide a random value at that moment.
  $H beware
   Calls to $INAME() may use a lot of CPU on some platforms.
   Some platforms may block for up to a few seconds while they accumulate some noise.
   Busy machines generate lots of noise, but care is advised when using $INAME() frequently in your application.
  $H underlying API
   [http://developer.mozilla.org/en/docs/PR_GetRandomNoise NSPR API]
**/
DEFINE_FUNCTION( GetRandomNoise ) {

	J_S_ASSERT_ARG_MIN( 1 );
	J_S_ASSERT_INT( J_ARG(1) );
	PRSize rndSize;
	rndSize = JSVAL_TO_INT( J_ARG(1) );
	void *buf;
	buf = (void*)JS_malloc(cx, rndSize);
	J_S_ASSERT_ALLOC( buf );
	PRSize size;
	size = PR_GetRandomNoise(buf, rndSize);
	if ( size <= 0 ) {

		JS_free(cx, buf);
		J_REPORT_ERROR( "PR_GetRandomNoise is not implemented on this platform." );
	}

	J_CHK( J_NewBlob( cx, buf, size, rval ) );

	return JS_TRUE;
	JL_BAD;
}

/*
**      PR_ntohs        16 bit conversion from network to host
**      PR_ntohl        32 bit conversion from network to host
**      PR_ntohll       64 bit conversion from network to host
**      PR_htons        16 bit conversion from host to network
**      PR_htonl        32 bit conversion from host to network
**      PR_ntonll       64 bit conversion from host to network


DEFINE_FUNCTION( hton ) {

	J_S_ASSERT_ARG_MIN( 1 );

	PRUint32 val;
	J_JSVAL_TO_UINT32( J_ARG(1), val );

	val = PR_ntohl(val);

	if (

	PR_htonll
	return JS_TRUE;
}

*/



/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( semaphoreName )
  Tests the value of the semaphore.
  If the value of the semaphore is > 0, the value of the semaphore is decremented and the function returns.
  If the value of the semaphore is 0, the function blocks until the value becomes > 0, then the semaphore is decremented and the function returns.
  $H note
   The "test and decrement" operation is performed atomically.
**/
DEFINE_FUNCTION_FAST( WaitSemaphore ) {

	J_S_ASSERT_ARG_MIN( 1 );

	PRUintn mode;
	mode = PR_IRUSR | PR_IWUSR; // read write permission for owner.
	if ( J_FARG_ISDEF(2) )
		J_CHK( JsvalToUInt(cx, J_FARG(2), &mode) );

	const char *name;
	size_t nameLength;
	J_CHK( JsvalToStringAndLength(cx, &J_FARG(1), &name, &nameLength) );

	bool isCreation;
	isCreation = true;
	PRSem *semaphore;
	semaphore = PR_OpenSemaphore(name, PR_SEM_EXCL | PR_SEM_CREATE, mode, 1); // fail if already exists

	if ( semaphore == NULL ) {

		semaphore = PR_OpenSemaphore(name, 0, 0, 0); // If PR_SEM_CREATE is not specified, the third and fourth arguments are ignored.
		if ( semaphore == NULL )
			return ThrowIoError(cx);
		isCreation = false;
	}

	PRStatus status;
	status = PR_WaitSemaphore( semaphore );
	if ( status != PR_SUCCESS )
		return ThrowIoError(cx);

	status = PR_CloseSemaphore(semaphore);
	if ( status != PR_SUCCESS )
		return ThrowIoError(cx);

	if ( isCreation ) {

		status = PR_DeleteSemaphore(name);
		if ( status != PR_SUCCESS )
			return ThrowIoError(cx);
	}

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( semaphoreName )
  Increments the value of a specified semaphore.
**/
DEFINE_FUNCTION_FAST( PostSemaphore ) {

	J_S_ASSERT_ARG_MIN( 1 );

	const char *name;
	size_t nameLength;
	J_CHK( JsvalToStringAndLength(cx, &J_FARG(1), &name, &nameLength) );

	PRSem *semaphore;
	semaphore = PR_OpenSemaphore(name, 0, 0, 0);

	if ( semaphore != NULL ) {

		PRStatus status;
		status = PR_PostSemaphore( semaphore );
		if ( status != PR_SUCCESS )
			return ThrowIoError(cx);

		status = PR_CloseSemaphore(semaphore);
		if ( status != PR_SUCCESS )
			return ThrowIoError(cx);
	}

	return JS_TRUE;
	JL_BAD;
}



/*doc
$TOC_MEMBER $INAME
 $VAL $INAME( path [, argv [, waitExit ]] )
  This function starts a new process optionaly using the JavaScript Array _argv_ for arguments or _undefined_ for no arguments.
  If _waitExit_ is true, the function waits the end of the process and returns its exit code.
  If _waitExit_ is not true, the function immediately returns an array that contains an input pipe and an output pipe to the current process stdin and stdout.
  $H example
  {{{
  var [stdin, stdout] = CreateProcess( 'c:\\windows\\System32\\cmd.exe', ['/c', 'dir', 'c:\\'], false );
  Sleep(100);
  Print( stdout.Read(), '\n' );
  stdin.Close();
  stdout.Close();
  }}}
**/
/*
// Doc. http://www.mozilla.org/projects/nspr/reference/html/prprocess.html#24535
DEFINE_FUNCTION_FAST( CreateProcess ) {

	const char **processArgv = NULL; // keep on top

	J_S_ASSERT_ARG_MIN( 1 );

	int processArgc;
	if ( J_FARG_ISDEF(2) && JSVAL_IS_OBJECT(J_FARG(2)) && JS_IsArrayObject( cx, JSVAL_TO_OBJECT(J_FARG(2)) ) == JS_TRUE ) {

		JSIdArray *idArray;
		idArray = JS_Enumerate( cx, JSVAL_TO_OBJECT(J_FARG(2)) ); // make a kind of auto-ptr for this
		processArgc = idArray->length +1; // +1 is argv[0]
		processArgv = (const char**)malloc(sizeof(const char**) * (processArgc +1)); // +1 is NULL
		J_S_ASSERT_ALLOC( processArgv );

		for ( int i=0; i<processArgc -1; i++ ) { // -1 because argv[0]

			jsval propVal;
			J_CHK( JS_IdToValue(cx, idArray->vector[i], &propVal ));
			J_CHK( JS_GetElement(cx, JSVAL_TO_OBJECT(J_FARG(2)), JSVAL_TO_INT(propVal), &propVal )); // (TBD) optimize

			const char *tmp;
			J_CHK( JsvalToString(cx, &propVal, &tmp) ); // warning: GC on the returned buffer !
			processArgv[i+1] = tmp;
		}
		JS_DestroyIdArray( cx, idArray );
	} else {

		processArgc = 0 +1; // +1 is argv[0]
		processArgv = (const char**)malloc(sizeof(const char**) * (processArgc +1)); // +1 is NULL
	}

	const char *path;
	J_CHK( JsvalToString(cx, &J_FARG(1), &path) );

	processArgv[0] = path;
	processArgv[processArgc] = NULL;

	bool waitEnd;
	waitEnd = false;
	if ( J_FARG_ISDEF(3) )
		J_CHK( JsvalToBool(cx, J_FARG(3), &waitEnd) );

	PRFileDesc *stdout_child, *stdout_parent, *stdin_child, *stdin_parent;
	J_CHKB( PR_CreatePipe(&stdout_parent, &stdout_child) == PR_SUCCESS, bad_throw );
	J_CHKB( PR_CreatePipe(&stdin_parent, &stdin_child) == PR_SUCCESS, bad_throw );

	PRProcessAttr *psattr;
	psattr = PR_NewProcessAttr();

	PR_ProcessAttrSetStdioRedirect(psattr, PR_StandardInput, stdin_child);
	PR_ProcessAttrSetStdioRedirect(psattr, PR_StandardOutput, stdout_child);

	PRProcess *process;
	process = PR_CreateProcess(path, (char * const *)processArgv, NULL, psattr); // (TBD) avoid cast to (char * const *)
	PR_DestroyProcessAttr(psattr);

	J_CHKB( PR_Close(stdin_child) == PR_SUCCESS, bad_throw );
	J_CHKB( PR_Close(stdout_child) == PR_SUCCESS, bad_throw );
	J_CHKB( process != NULL, bad_throw );

	if ( waitEnd ) {

		PRInt32 exitValue;
		J_CHKB( PR_WaitProcess( process, &exitValue ) == PR_SUCCESS, bad_throw );
		*J_FRVAL = INT_TO_JSVAL( exitValue );
	} else {

//		status = PR_DetachProcess(process);
//		if ( status != PR_SUCCESS )
//			return ThrowIoError(cx);

		JSObject *fdin = JS_NewObject( cx, classPipe, NULL, NULL );
		J_CHK( JS_SetPrivate( cx, fdin, stdin_parent ) );

		JSObject *fdout = JS_NewObject( cx, classPipe, NULL, NULL );
		J_CHK( JS_SetPrivate( cx, fdout, stdout_parent ) );

		J_CHK( ReserveStreamReadInterface(cx, fdout) );
		J_CHK( SetStreamReadInterface(cx, fdout, NativeInterfaceStreamRead) );

		jsval vector[2];
		vector[0] = OBJECT_TO_JSVAL( fdin );
		vector[1] = OBJECT_TO_JSVAL( fdout );
		JSObject *arrObj = JS_NewArrayObject(cx, 2, vector);
		*J_FRVAL = OBJECT_TO_JSVAL( arrObj );
	}


	if ( processArgv )
		free(processArgv);
	return JS_TRUE;
bad_throw:
	ThrowIoError(cx);
bad:
	if ( processArgv )
		free(processArgv);
	return JS_FALSE;
}
*/


/**doc
=== Static properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $STR $INAME $READONLY
  Is the host name with the domain name (if any).
**/
DEFINE_PROPERTY( hostName ) {

	char tmp[SYS_INFO_BUFFER_LENGTH];
	/* doc:
			Suppose the name of the host is configured as "foo.bar.com".
			If PR_SI_HOSTNAME is specified, "foo" is returned.
			If PR_SI_HOSTNAME_UNTRUNCATED is specified, "foo.bar.com" is returned.
			On the other hand, if the name of the host is configured as just "foo",
			both PR_SI_HOSTNAME and PR_SI_HOSTNAME_UNTRUNCATED return "foo".
	*/
	PRStatus status = PR_GetSystemInfo( PR_SI_HOSTNAME_UNTRUNCATED, tmp, sizeof(tmp) );
	if ( status != PR_SUCCESS )
		return ThrowIoError(cx);
	JSString *jsstr = JS_NewStringCopyZ(cx,tmp);
	J_S_ASSERT_ALLOC( jsstr );
	*vp = STRING_TO_JSVAL(jsstr);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  Is the amount of physical RAM in the system in bytes.
**/
DEFINE_PROPERTY( physicalMemorySize ) {

	PRUint64 mem = PR_GetPhysicalMemorySize();
	J_CHK( JS_NewNumberValue(cx, (jsdouble)mem, vp) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $OBJ $INAME $READONLY
  Returns an object that contains the following properties:
   * $STR *architecture*: x86, ...
   * $STR *name*: Linux, Windows_NT, ...
   * $STR *release*: 2.6.22.18, 5.1, ...
  $H example
  {{{
  LoadModule('jsstd');
  LoadModule('jsio');
  Print( systemInfo.toSource() );
  }}}
  prints:
   * on coLinux: {{{ ({architecture:"x86", name:"Linux", release:"2.6.22.18-co-0.7.3"}) }}}
   * on WinXP: {{{ ({architecture:"x86", name:"Windows_NT", release:"5.1"}) }}}
**/
DEFINE_PROPERTY( systemInfo ) {

	if ( JSVAL_IS_VOID( *vp ) ) {

		char tmp[SYS_INFO_BUFFER_LENGTH];

		JSObject *info = JS_NewObject(cx, NULL, NULL, NULL);
		J_S_ASSERT_ALLOC( info );
		*vp = OBJECT_TO_JSVAL( info );

		PRStatus status;
//		jsval tmpVal;
		JSString *jsstr;

		// (TBD) these properties must be read-only !!

		status = PR_GetSystemInfo( PR_SI_ARCHITECTURE, tmp, sizeof(tmp) );
		if ( status != PR_SUCCESS )
			return ThrowIoError(cx);
		jsstr = JS_NewStringCopyZ(cx,tmp);
		J_S_ASSERT_ALLOC( jsstr );
//		tmpVal = STRING_TO_JSVAL(jsstr);
//		JS_SetProperty(cx, info, "architecture", &tmpVal);
		J_CHK( JS_DefineProperty(cx, info, "architecture", STRING_TO_JSVAL(jsstr), NULL, NULL, JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT) );

		status = PR_GetSystemInfo( PR_SI_SYSNAME, tmp, sizeof(tmp) );
		if ( status != PR_SUCCESS )
			return ThrowIoError(cx);
		jsstr = JS_NewStringCopyZ(cx,tmp);
		J_S_ASSERT_ALLOC( jsstr );
//		tmpVal = STRING_TO_JSVAL(jsstr);
//		JS_SetProperty(cx, info, "name", &tmpVal);
		J_CHK( JS_DefineProperty(cx, info, "name", STRING_TO_JSVAL(jsstr), NULL, NULL, JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT) );

		status = PR_GetSystemInfo( PR_SI_RELEASE, tmp, sizeof(tmp) );
		if ( status != PR_SUCCESS )
			return ThrowIoError(cx);
		jsstr = JS_NewStringCopyZ(cx,tmp);
		J_S_ASSERT_ALLOC( jsstr );
//		tmpVal = STRING_TO_JSVAL(jsstr);
//		JS_SetProperty(cx, info, "release", &tmpVal);
		J_CHK( JS_DefineProperty(cx, info, "release", STRING_TO_JSVAL(jsstr), NULL, NULL, JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT) );
	}

	return JS_TRUE;
	JL_BAD;
}




/**doc
$TOC_MEMBER $INAME
 $INT $INAME
  Is the current process priority among the following values:
   * `-1`: low
   * ` 0`: normal
   * ` 1`: high
   * ` 2`: urgent
**/
DEFINE_PROPERTY( processPriorityGetter ) {

	PRThreadPriority priority = PR_GetThreadPriority(PR_GetCurrentThread());
	int priorityValue;
	switch (priority) {
		case PR_PRIORITY_LOW:
			priorityValue = -1;
			break;
		case PR_PRIORITY_NORMAL:
			priorityValue = 0;
			break;
		case PR_PRIORITY_HIGH:
			priorityValue = 1;
			break;
		case PR_PRIORITY_URGENT:
			priorityValue = 2;
			break;
		default:
			J_REPORT_ERROR( "Invalid thread priority." );
	}
	*vp = INT_TO_JSVAL( priorityValue );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( processPrioritySetter ) {

	int priorityValue;
	J_CHK( JsvalToInt(cx, *vp, &priorityValue) );
	PRThreadPriority priority;
	switch (priorityValue) {
		case -1:
			priority = PR_PRIORITY_LOW;
			break;
		case 0:
			priority = PR_PRIORITY_NORMAL;
			break;
		case 1:
			priority = PR_PRIORITY_HIGH;
			break;
		case 2:
			priority = PR_PRIORITY_URGENT;
			break;
		default:
			J_REPORT_ERROR( "Invalid thread priority." );
	}
	PRThread *thread;
	thread = PR_GetCurrentThread();
	PR_SetThreadPriority( thread, priority );
	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $STR $INAME
  Gets/sets the current working directory.
**/
DEFINE_PROPERTY_GETTER( currentWorkingDirectory ) {

	char buf[PATH_MAX];

#ifdef XP_WIN
	_getcwd(buf, sizeof(buf));
#else // XP_WIN
	getcwd(buf, sizeof(buf));
#endif // XP_WIN

	JSString *str = JS_NewStringCopyZ(cx, buf);
	J_S_ASSERT_ALLOC( str );
	*vp = STRING_TO_JSVAL( str );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( currentWorkingDirectory ) {

	const char *buf;
	J_CHK( JsvalToString(cx, vp, &buf ) );

#ifdef XP_WIN
	_chdir(buf);
#else // XP_WIN
	chdir(buf);
#endif // XP_WIN

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME $READONLY
  Is the os's path separator.
**/
DEFINE_PROPERTY( pathSeparator ) {

	if ( *vp == JSVAL_VOID ) {

		JSString *str = JS_InternString(cx, PATH_SEPARATOR_STRING);
		J_S_ASSERT_ALLOC( str );
		*vp = STRING_TO_JSVAL( str );
	}
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $STR $INAME $READONLY
  Is the os's list separator.
  $H example
  {{{
  Print( GetEnv('PATH').split(listSeparator) )
  }}}
**/
DEFINE_PROPERTY( listSeparator ) {

	if ( *vp == JSVAL_VOID ) {

		JSString *str = JS_InternString(cx, LIST_SEPARATOR_STRING);
		J_S_ASSERT_ALLOC( str );
		*vp = STRING_TO_JSVAL( str );
	}
	return JS_TRUE;
	JL_BAD;
}


CONFIGURE_STATIC

	REVISION(SvnRevToInt("$Revision$"))
	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION( Poll ) // Do not turn it in FAST NATIVE because we need a stack frame for debuging
		FUNCTION( IsReadable )
		FUNCTION( IsWritable )
		FUNCTION( IntervalNow )
		FUNCTION_FAST( UIntervalNow )
		FUNCTION( Sleep )
		FUNCTION( GetEnv )
		FUNCTION( GetRandomNoise )
		FUNCTION_FAST( WaitSemaphore )
		FUNCTION_FAST( PostSemaphore )
//		FUNCTION_FAST( CreateProcess )
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_READ( hostName )
		PROPERTY_READ( physicalMemorySize )
		PROPERTY_READ_STORE( systemInfo )
		PROPERTY( processPriority )
		PROPERTY( currentWorkingDirectory )
		PROPERTY_READ_STORE( pathSeparator )
		PROPERTY_READ_STORE( listSeparator )
	END_STATIC_PROPERTY_SPEC

END_STATIC
