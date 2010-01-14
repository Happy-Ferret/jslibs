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

#ifndef XP_WIN
#include <sys/statvfs.h>
#endif


#include "static.h"

#include "descriptor.h"
#include "pipe.h"

#include <pprio.h> // nspr/include/nspr/private

#include "../jslang/handlePub.h"


DECLARE_CLASS( File )

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

	// NSPR Poll Method:
	//   http://www.mozilla.org/projects/nspr/tech-notes/poll-method.html

	// http://developer.mozilla.org/en/docs/PR_Poll

	PRInt32 result;
	PRIntervalTime pr_timeout;
	jsval tmp, prop;
	jsint i;
	JSIdArray *arrayIds = NULL;
	PRPollDesc staticPollDesc[128]; // 1KB
	PRPollDesc *pollDesc = staticPollDesc;

	JL_S_ASSERT_ARG_MIN( 1 );
	JL_S_ASSERT_ARRAY( JL_ARG(1) );
	JSObject *fdArrayObj;
	fdArrayObj = JSVAL_TO_OBJECT(JL_ARG(1));

	if ( JL_ARG_ISDEF(2) && !JsvalIsPInfinity(cx, JL_ARG(2)) ) {

		PRUint32 tmp;
		JL_CHK( JsvalToUInt(cx, JL_ARG(2), &tmp) );
		pr_timeout = PR_MillisecondsToInterval(tmp);
	} else {

		pr_timeout = PR_INTERVAL_NO_TIMEOUT;
	}

	arrayIds = JS_Enumerate(cx, fdArrayObj);

	if ( arrayIds->length == 0 ) { // optimization

		JS_DestroyIdArray(cx, arrayIds);
		result = PR_Poll(NULL, 0, pr_timeout); // we can replace this by a delay, but the function is Pool, not Sleep
		if ( result == -1 )
			return ThrowIoError(cx);
		*rval = JSVAL_ZERO;
		return JS_TRUE;
	}

	// Optimization to avoid dynamic allocation when it is possible. There is also a potential use of alloca()
	if ( (unsigned)arrayIds->length > COUNTOF(staticPollDesc) )
		pollDesc = (PRPollDesc*)jl_malloc(arrayIds->length * sizeof(PRPollDesc));
	else
		pollDesc = staticPollDesc;

	for ( i = 0; i < arrayIds->length; ++i ) {

		JL_CHK( JS_GetElement(cx, fdArrayObj, JSID_TO_INT(arrayIds->vector[i]), &prop) );
		if ( JSVAL_IS_VOID( prop ) ) {

			pollDesc[i].fd = 0;
			pollDesc[i].in_flags = 0;
			pollDesc[i].out_flags = 0;
			continue;
		}
		JSObject *fdObj = JSVAL_TO_OBJECT( prop );
		JL_S_ASSERT_INHERITANCE(fdObj, JL_CLASS(Descriptor));

		pollDesc[i].fd = (PRFileDesc *)JL_GetPrivate(cx, fdObj); // fd is A pointer to a PRFileDesc object representing a socket or a pollable event.  This field can be set to NULL to indicate to PR_Poll that this PRFileDesc object should be ignored.
//		JL_S_ASSERT_RESOURCE( fd ); // beware: fd == NULL is supported !
		pollDesc[i].in_flags = 0;
		pollDesc[i].out_flags = 0;

		JL_CHK( JS_GetProperty( cx, fdObj, "writable", &prop ) );
		if ( JsvalIsFunction(cx, prop) )
			pollDesc[i].in_flags |= PR_POLL_WRITE;

		JL_CHK( JS_GetProperty( cx, fdObj, "readable", &prop ) );
		if ( JsvalIsFunction(cx, prop) )
			pollDesc[i].in_flags |= PR_POLL_READ;

		JL_CHK( JS_GetProperty( cx, fdObj, "hangup", &prop ) );
		if ( JsvalIsFunction(cx, prop) )
			pollDesc[i].in_flags |= PR_POLL_HUP;

		JL_CHK( JS_GetProperty( cx, fdObj, "exception", &prop ) );
		if ( JsvalIsFunction(cx, prop) )
			pollDesc[i].in_flags |= PR_POLL_EXCEPT;

		JL_CHK( JS_GetProperty( cx, fdObj, "error", &prop ) );
		if ( JsvalIsFunction(cx, prop) )
			pollDesc[i].in_flags |= PR_POLL_ERR;
	}

	result = PR_Poll(pollDesc, arrayIds->length, pr_timeout);
	if ( result == -1 ) // failed. see PR_GetError()
		JL_CHK( ThrowIoError(cx) ); // returns later

	if ( result == 0 ) { // has no event(s)

		*rval = JSVAL_ZERO;
		JS_DestroyIdArray(cx, arrayIds);
		if ( pollDesc != staticPollDesc )
			jl_free(pollDesc);
		return JS_TRUE;
	}

	jsval cbArgv[3];

	// the following protection against GC is not needed bacause all arguments are already safe (fd object, int, int)
//	memset(cbArgv, 0, sizeof(cbArgv));
//	JSTempValueRooter tvr;
//	JS_PUSH_TEMP_ROOT(cx, COUNTOF(cbArgv), cbArgv, &tvr);

	for ( i = 0; i < arrayIds->length; ++i ) {

		JL_CHK( JS_GetElement(cx, fdArrayObj, JSID_TO_INT(arrayIds->vector[i]), &prop) );
		if ( JSVAL_IS_VOID( prop ) ) // socket has been removed from the list while js func "poll()" is runing
			continue;
		JSObject *fdObj = JSVAL_TO_OBJECT( prop ); //JS_ValueToObject
		PRInt16 outFlag = pollDesc[i].out_flags;
		cbArgv[0] = prop;
		cbArgv[1] = ID_TO_VALUE(arrayIds->vector[i]);
		cbArgv[2] = (outFlag & PR_POLL_HUP) ? JSVAL_TRUE : JSVAL_FALSE;

		if ( outFlag & PR_POLL_ERR ) {

			JL_CHKB( JS_GetProperty( cx, fdObj, "error", &prop ), bad2 );
			if ( JsvalIsFunction(cx, prop) )
				JL_CHKB( JS_CallFunctionValue( cx, fdObj, prop, COUNTOF(cbArgv), cbArgv, &tmp ), bad2 );
		}

		if ( outFlag & PR_POLL_EXCEPT ) {

			JL_CHKB( JS_GetProperty( cx, fdObj, "exception", &prop ), bad2 );
			if ( JsvalIsFunction(cx, prop) )
				JL_CHKB( JS_CallFunctionValue( cx, fdObj, prop, COUNTOF(cbArgv), cbArgv, &tmp ), bad2 );
		}

		if ( outFlag & PR_POLL_HUP ) {

			JL_CHKB( JS_GetProperty( cx, fdObj, "hangup", &prop ), bad2 );
			if ( JsvalIsFunction(cx, prop) )
				JL_CHKB( JS_CallFunctionValue( cx, fdObj, prop, COUNTOF(cbArgv), cbArgv, &tmp ), bad2 );
		}

		if ( outFlag & PR_POLL_READ ) {

			JL_CHKB( JS_GetProperty( cx, fdObj, "readable", &prop ), bad2 );
			if ( JsvalIsFunction(cx, prop) )
				JL_CHKB( JS_CallFunctionValue( cx, fdObj, prop, COUNTOF(cbArgv), cbArgv, &tmp ), bad2 );
		}

		if ( outFlag & PR_POLL_WRITE ) {

			JL_CHKB( JS_GetProperty( cx, fdObj, "writable", &prop ), bad2 );
			if ( JsvalIsFunction(cx, prop) )
				JL_CHKB( JS_CallFunctionValue( cx, fdObj, prop, COUNTOF(cbArgv), cbArgv, &tmp ), bad2 );
		}
	} // for

	*rval = INT_TO_JSVAL( result );

//	JS_POP_TEMP_ROOT(cx, &tvr);
	JS_DestroyIdArray(cx, arrayIds);
	if ( pollDesc != staticPollDesc )
		jl_free(pollDesc);
	return JS_TRUE;
bad2:
//	JS_POP_TEMP_ROOT(cx, &tvr);
bad:
	if ( arrayIds )
		JS_DestroyIdArray(cx, arrayIds);
	if ( pollDesc != staticPollDesc )
		jl_free(pollDesc);
	return JS_FALSE;
}




struct MetaPollIOData {
	
	MetaPoll mp;

	PRPollDesc *pollDesc;
	PRInt32 pollResult;
	bool hasEvent;
};

JL_STATIC_ASSERT( offsetof(MetaPollIOData, mp) == 0 );

static void StartPoll( volatile MetaPoll *mp ) {

	MetaPollIOData *mpio = (MetaPollIOData*)mp;
	mpio->pollResult = PR_Poll(mpio->pollDesc, 1, PR_INTERVAL_NO_TIMEOUT);
	mpio->hasEvent = mpio->pollResult > 0 && !( mpio->pollDesc[0].out_flags & PR_POLL_READ );
}

static void CancelPoll( volatile MetaPoll *mp ) {

	MetaPollIOData *mpio = (MetaPollIOData*)mp;
	PRStatus status;
	status = PR_SetPollableEvent(mpio->pollDesc[0].fd); // cancel the poll
}

static JSBool EndPoll( volatile MetaPoll *mp, bool *hasEvent, JSContext *cx, JSObject *obj ) {

	MetaPollIOData *mpio = (MetaPollIOData*)mp;
	PRStatus status;
	status = PR_DestroyPollableEvent(mpio->pollDesc[0].fd);
	*hasEvent = mpio->hasEvent;

	jl_free(mpio->pollDesc);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_FUNCTION_FAST( MetaPollIO ) {
	
	MetaPollIOData *mpio;
	JL_CHK( CreateHandle(cx, 'poll', sizeof(MetaPollIOData), (void**)&mpio, NULL, JL_FRVAL) );
	mpio->mp.startPoll = StartPoll;
	mpio->mp.cancelPoll = CancelPoll;
	mpio->mp.endPoll = EndPoll;

	mpio->pollDesc = (PRPollDesc*)jl_malloc(sizeof(PRPollDesc) * 1);

	mpio->pollDesc[0].fd = PR_NewPollableEvent();
	mpio->pollDesc[0].in_flags = PR_POLL_READ;
	mpio->pollDesc[0].out_flags = 0;

	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME( _descriptor_ )
  Returns true if the _descriptor_ can be read without blocking.
**/
DEFINE_FUNCTION( IsReadable ) {

	JL_S_ASSERT_ARG_MIN( 1 );

	JSObject *descriptorObj;
	descriptorObj = JSVAL_TO_OBJECT( JL_ARG(1) );
	JL_S_ASSERT_INHERITANCE(descriptorObj, JL_CLASS(Descriptor));

	PRFileDesc *fd;
	fd = (PRFileDesc *)JL_GetPrivate( cx, descriptorObj );
//	JL_S_ASSERT_RESOURCE( fd ); // fd == NULL is supported !

	PRIntervalTime prTimeout;
	if ( JL_ARG_ISDEF(2) ) {

		PRUint32 timeout;
		JL_CHK( JsvalToUInt(cx, JL_ARG(2), &timeout) );
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
 $BOOL $INAME( descriptor )
  Returns $TRUE if the _descriptor_ can be write without blocking.
**/
DEFINE_FUNCTION( IsWritable ) {

	JL_S_ASSERT_ARG_MIN( 1 );

	JSObject *descriptorObj;
	descriptorObj = JSVAL_TO_OBJECT( JL_ARG(1) );
	JL_S_ASSERT_INHERITANCE(descriptorObj, JL_CLASS(Descriptor));
	PRFileDesc *fd;
	fd = (PRFileDesc *)JL_GetPrivate( cx, descriptorObj );
//	JL_S_ASSERT_RESOURCE( fd ); // fd == NULL is supported !

	PRIntervalTime prTimeout;
	if ( JL_ARG_ISDEF(2) ) {

		PRUint32 timeout;
		JL_CHK( JsvalToUInt(cx, JL_ARG(2), &timeout) );
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

	PRUint32 interval = PR_IntervalToMilliseconds( PR_IntervalNow() ); // (TBD) Check if it may wrap around in about 12 hours. Is it related to the data type ???
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
	JL_CHK( JS_ValueToECMAUint32( cx, JL_ARG(1), &timeout ) );
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

	JL_S_ASSERT_ARG_MIN(1);
	const char *name;
	JL_CHK( JsvalToString(cx, &JL_ARG(1), &name) );
	char* value;
	value = PR_GetEnv(name); // If the environment variable is not defined, the function returns NULL.

	if ( value == NULL || *value == '\0' ) { // this will cause an 'undefined' return value

		*JL_RVAL = JSVAL_VOID;
	} else {

//		JSString *jsstr = JS_NewExternalString(cx, (jschar*)value, strlen(value), JS_AddExternalStringFinalizer(NULL)); only works with unicode strings
		JSString *jsstr = JS_NewStringCopyZ(cx,value);
		JL_CHK( jsstr );
		*rval = STRING_TO_JSVAL(jsstr);
	}
	return JS_TRUE;
	JL_BAD;
}


/**
$TOC_MEMBER $INAME
 $STR $INAME( name, value )
  Set, unset or change an environment variable.
**/
/*
DEFINE_FUNCTION( SetEnv ) {

doc:
	The caller must ensure that the string passed
	to PR_SetEnv() is persistent. That is: The string should
	not be on the stack, where it can be overwritten

	JL_S_ASSERT_ARG_MIN(1);
	const char *name, *value;
	JL_CHK( JsvalToString(cx, &JL_ARG(1), &name) );
	JL_CHK( JsvalToString(cx, &JL_ARG(2), &value) );

	PRStatus status = PR_SetEnv...

	return JS_TRUE;
	JL_BAD;
}
*/

/**doc
$TOC_MEMBER $INAME
 $STR $INAME( size )
  Provides, depending on platform, a random value.
  The length of the random value is dependent on platform and the platform's ability to provide a random value at that moment.
  $H beware
   Calls to $INAME() may use a lot of CPU on some platforms.
   Some platforms may block for up to a few seconds while they accumulate some noise.
   Busy machines generate lots of noise, but care is advised when using $INAME() frequently in your application.
   $LF
   [http://developer.mozilla.org/en/docs/PR_GetRandomNoise NSPR API]
**/
DEFINE_FUNCTION( GetRandomNoise ) {

	JL_S_ASSERT_ARG_MIN( 1 );
	JL_S_ASSERT_INT( JL_ARG(1) );
	PRSize rndSize;
	rndSize = JSVAL_TO_INT( JL_ARG(1) );
	void *buf;
	buf = (void*)JS_malloc(cx, rndSize);
	JL_CHK( buf );
	PRSize size;
	size = PR_GetRandomNoise(buf, rndSize);
	if ( size <= 0 ) {

		JS_free(cx, buf);
		JL_REPORT_ERROR( "PR_GetRandomNoise is not implemented on this platform." );
	}

	JL_CHK( JL_NewBlob( cx, buf, size, rval ) );

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

	JL_S_ASSERT_ARG_MIN( 1 );

	PRUint32 val;
	J_JSVAL_TO_UINT32( JL_ARG(1), val );

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

	JL_S_ASSERT_ARG_MIN( 1 );

	PRUintn mode;
	mode = PR_IRUSR | PR_IWUSR; // read write permission for owner.
	if ( JL_FARG_ISDEF(2) )
		JL_CHK( JsvalToUInt(cx, JL_FARG(2), &mode) );

	const char *name;
	size_t nameLength;
	JL_CHK( JsvalToStringAndLength(cx, &JL_FARG(1), &name, &nameLength) );

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

	JL_S_ASSERT_ARG_MIN( 1 );

	const char *name;
	size_t nameLength;
	JL_CHK( JsvalToStringAndLength(cx, &JL_FARG(1), &name, &nameLength) );

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



/** doc
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

	JL_S_ASSERT_ARG_MIN( 1 );

	int processArgc;
	if ( JL_FARG_ISDEF(2) && JSVAL_IS_OBJECT(JL_FARG(2)) && JS_IsArrayObject( cx, JSVAL_TO_OBJECT(JL_FARG(2)) ) == JS_TRUE ) {

		JSIdArray *idArray;
		idArray = JS_Enumerate( cx, JSVAL_TO_OBJECT(JL_FARG(2)) ); // make a kind of auto-ptr for this
		processArgc = idArray->length +1; // +1 is argv[0]
		processArgv = (const char**)jl_malloc(sizeof(const char**) * (processArgc +1)); // +1 is NULL
		JL_S_ASSERT_ALLOC( processArgv );

		for ( int i=0; i<processArgc -1; i++ ) { // -1 because argv[0]

			jsval propVal;
			JL_CHK( JS_IdToValue(cx, idArray->vector[i], &propVal ));
			JL_CHK( JS_GetElement(cx, JSVAL_TO_OBJECT(JL_FARG(2)), JSVAL_TO_INT(propVal), &propVal )); // (TBD) optimize

			const char *tmp;
			JL_CHK( JsvalToString(cx, &propVal, &tmp) ); // warning: GC on the returned buffer !
			processArgv[i+1] = tmp;
		}
		JS_DestroyIdArray( cx, idArray );
	} else {

		processArgc = 0 +1; // +1 is argv[0]
		processArgv = (const char**)jl_malloc(sizeof(const char**) * (processArgc +1)); // +1 is NULL
	}

	const char *path;
	JL_CHK( JsvalToString(cx, &JL_FARG(1), &path) );

	processArgv[0] = path;
	processArgv[processArgc] = NULL;

	bool waitEnd;
	waitEnd = false;
	if ( JL_FARG_ISDEF(3) )
		JL_CHK( JsvalToBool(cx, JL_FARG(3), &waitEnd) );

	PRFileDesc *stdout_child, *stdout_parent, *stdin_child, *stdin_parent;
	JL_CHKB( PR_CreatePipe(&stdout_parent, &stdout_child) == PR_SUCCESS, bad_throw );
	JL_CHKB( PR_CreatePipe(&stdin_parent, &stdin_child) == PR_SUCCESS, bad_throw );

	PRProcessAttr *psattr;
	psattr = PR_NewProcessAttr();

	PR_ProcessAttrSetStdioRedirect(psattr, PR_StandardInput, stdin_child);
	PR_ProcessAttrSetStdioRedirect(psattr, PR_StandardOutput, stdout_child);

	PRProcess *process;
	process = PR_CreateProcess(path, (char * const *)processArgv, NULL, psattr); // (TBD) avoid cast to (char * const *)
	PR_DestroyProcessAttr(psattr);

	JL_CHKB( PR_Close(stdin_child) == PR_SUCCESS, bad_throw );
	JL_CHKB( PR_Close(stdout_child) == PR_SUCCESS, bad_throw );
	JL_CHKB( process != NULL, bad_throw );

	if ( waitEnd ) {

		PRInt32 exitValue;
		JL_CHKB( PR_WaitProcess( process, &exitValue ) == PR_SUCCESS, bad_throw );
		*JL_FRVAL = INT_TO_JSVAL( exitValue );
	} else {

//		status = PR_DetachProcess(process);
//		if ( status != PR_SUCCESS )
//			return ThrowIoError(cx);

		JSObject *fdin = JS_NewObject( cx, classPipe, NULL, NULL );
		JL_SetPrivate( cx, fdin, stdin_parent );

		JSObject *fdout = JS_NewObject( cx, classPipe, NULL, NULL );
		JL_SetPrivate( cx, fdout, stdout_parent );

		JL_CHK( ReserveStreamReadInterface(cx, fdout) );
		JL_CHK( SetStreamReadInterface(cx, fdout, NativeInterfaceStreamRead) );

		jsval vector[2];
		vector[0] = OBJECT_TO_JSVAL( fdin );
		vector[1] = OBJECT_TO_JSVAL( fdout );
		JSObject *arrObj = JS_NewArrayObject(cx, 2, vector);
		*JL_FRVAL = OBJECT_TO_JSVAL( arrObj );
	}


	if ( processArgv )
		jl_free(processArgv);
	return JS_TRUE;
bad_throw:
	ThrowIoError(cx);
bad:
	if ( processArgv )
		jl_free(processArgv);
	return JS_FALSE;
}
*/



/**doc
$TOC_MEMBER $INAME
 $REAL $INAME( path )
  Returns the available storage space (in bytes) at the given path.
  $H example
  {{{
  Print( AvailableSpace('/var') );
  }}}
**/
DEFINE_FUNCTION_FAST( AvailableSpace ) {

	JL_S_ASSERT_ARG_MIN( 1 );
	const char *path;
	JL_CHK( JsvalToString(cx, &JL_FARG(1), &path) );

	jsdouble available;

#ifdef XP_WIN
	ULARGE_INTEGER freeBytesAvailable;
	BOOL res = ::GetDiskFreeSpaceEx(path, &freeBytesAvailable, NULL, NULL);
	if ( res == 0 )
		JL_REPORT_ERROR("Unable to get the available space of %s.", path);
	available = freeBytesAvailable.QuadPart;
#else // now for XP_UNIX an MacOS ?
	struct statvfs fsd;
	if ( statvfs(path, &fsd) < 0 )
		JL_REPORT_ERROR("Unable to get the available space of %s.", path);
	available = (jsdouble)fsd.f_bsize * (jsdouble)fsd.f_bavail;
#endif // XP_WIN

	JL_CHK( JS_NewDoubleValue(cx, available, JL_FRVAL) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $UNDEF $INAME( descriptor, [baudRate], [byteSize], [parity], [stopBits] )
 baudRate: 
  110, 300, 600, 1200, 2400, 4800, 9600, 14400, 19200, 38400, 56000, 57600, 115200, 128000, 256000
 byteSize:
  4, 5, 6, 7, 8
 parity:
  0: No parity.
  1: Odd parity.
  2: Even parity.
  3: Mark parity.
  4: Space parity.
 stopBits:
  0: 1 stop bit.
  1: 1.5 stop bits.
  2: 2 stop bits.

 $H example
 {{{
 var f = new File( systemInfo.name == 'Linux' ? '/dev/ttyS0' : '//./com1' );
 f.Open(File.RDWR);
 ConfigureSerialPort(f, 9600);
 f.Write('A');
 f.Read(1);
 }}}
**/
DEFINE_FUNCTION_FAST( ConfigureSerialPort ) {
	
	JL_S_ASSERT_ARG_RANGE(1,2);
	JL_S_ASSERT_OBJECT( JL_FARG(1) );
	JSObject *fileObj = JSVAL_TO_OBJECT( JL_FARG(1) );
	JL_S_ASSERT_INHERITANCE( fileObj, JL_CLASS(File) );

	PRFileDesc *fd = (PRFileDesc *)JL_GetPrivate(cx, fileObj);
	JL_S_ASSERT( fd != NULL, "File is closed." );

	unsigned int baudRate, byteSize, parity, stopBits;


#ifdef XP_WIN
	HANDLE fh = (HANDLE)PR_FileDesc2NativeHandle(fd);
	
	BOOL status;
	DCB dcb;
	COMMTIMEOUTS commTimeouts;

	status = GetCommState(fh, &dcb);
	if ( status == 0 )
		JL_ThrowOSError(cx);
		
	status = GetCommTimeouts(fh, &commTimeouts);
	if ( status == 0 )
		JL_ThrowOSError(cx);
	// (TBD) manage commTimeouts


	if ( JL_FARG_ISDEF(2) ) {

		JL_CHK( JsvalToUInt(cx, JL_FARG(2), &baudRate) );
		dcb.BaudRate = baudRate;
	}

	if ( JL_FARG_ISDEF(3) ) {
		
		JL_CHK( JsvalToUInt(cx, JL_FARG(3), &byteSize) );
		dcb.ByteSize = byteSize;
	}
	
	if ( JL_FARG_ISDEF(4) ) {
		
		JL_CHK( JsvalToUInt(cx, JL_FARG(4), &parity) );
		dcb.Parity = parity;
	}

	if ( JL_FARG_ISDEF(5) ) {
		
		JL_CHK( JsvalToUInt(cx, JL_FARG(5), &stopBits) );
		dcb.StopBits = stopBits;
	}

	dcb.fBinary = TRUE;
	dcb.fDsrSensitivity = FALSE;
	dcb.fOutX = FALSE;
	dcb.fInX = FALSE;
	dcb.fNull = FALSE;
	dcb.fAbortOnError = TRUE;
	dcb.fOutxCtsFlow = FALSE;
	dcb.fOutxDsrFlow = FALSE;
	dcb.fDtrControl = DTR_CONTROL_DISABLE;
	dcb.fDsrSensitivity = FALSE;
	dcb.fRtsControl = RTS_CONTROL_DISABLE;
	dcb.fOutxCtsFlow = FALSE;
	dcb.fOutxCtsFlow = FALSE;

	status = SetCommState(fh, &dcb);
	if ( status == 0 )
		JL_ThrowOSError(cx);
#endif // XP_WIN

#ifdef XP_UNIX

	struct termios tio;
//	tcgetattr(
	// (TBD) see http://www.comptechdoc.org/os/linux/programming/c/linux_pgcserial.html

#endif // XP_UNIX


	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


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
	JL_CHK( jsstr );
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
	JL_CHK( JS_NewNumberValue(cx, (jsdouble)mem, vp) );
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

	char tmp[SYS_INFO_BUFFER_LENGTH];

	JSObject *info = JS_NewObject(cx, NULL, NULL, NULL);
	JL_CHK( info );
	*vp = OBJECT_TO_JSVAL( info );

	PRStatus status;
//		jsval tmpVal;
	JSString *jsstr;

	// (TBD) these properties must be read-only !!

	status = PR_GetSystemInfo( PR_SI_ARCHITECTURE, tmp, sizeof(tmp) );
	if ( status != PR_SUCCESS )
		return ThrowIoError(cx);
	jsstr = JS_NewStringCopyZ(cx,tmp);
	JL_CHK( jsstr );
//		tmpVal = STRING_TO_JSVAL(jsstr);
//		JS_SetProperty(cx, info, "architecture", &tmpVal);
	JL_CHK( JS_DefineProperty(cx, info, "architecture", STRING_TO_JSVAL(jsstr), NULL, NULL, JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT) );

	status = PR_GetSystemInfo( PR_SI_SYSNAME, tmp, sizeof(tmp) );
	if ( status != PR_SUCCESS )
		return ThrowIoError(cx);
	jsstr = JS_NewStringCopyZ(cx,tmp);
	JL_CHK( jsstr );
//		tmpVal = STRING_TO_JSVAL(jsstr);
//		JS_SetProperty(cx, info, "name", &tmpVal);
	JL_CHK( JS_DefineProperty(cx, info, "name", STRING_TO_JSVAL(jsstr), NULL, NULL, JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT) );

	status = PR_GetSystemInfo( PR_SI_RELEASE, tmp, sizeof(tmp) );
	if ( status != PR_SUCCESS )
		return ThrowIoError(cx);
	jsstr = JS_NewStringCopyZ(cx,tmp);
	JL_CHK( jsstr );
//		tmpVal = STRING_TO_JSVAL(jsstr);
//		JS_SetProperty(cx, info, "release", &tmpVal);
	JL_CHK( JS_DefineProperty(cx, info, "release", STRING_TO_JSVAL(jsstr), NULL, NULL, JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT) );

	return JL_StoreProperty(cx, obj, id, vp, true);
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
			JL_REPORT_ERROR( "Invalid thread priority." );
	}
	*vp = INT_TO_JSVAL( priorityValue );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( processPrioritySetter ) {

	int priorityValue;
	JL_CHK( JsvalToInt(cx, *vp, &priorityValue) );
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
			JL_REPORT_ERROR( "Invalid thread priority." );
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
  Is the number of processors (CPUs available in an SMP system).
**/
DEFINE_PROPERTY( numberOfProcessors ) {

	PRInt32 count = PR_GetNumberOfProcessors();
	if ( count < 0 )
		JL_REPORT_ERROR( "Unable to get the number of processors." );
	JL_CHK( IntToJsval(cx, count, vp) );
	return JL_StoreProperty(cx, obj, id, vp, true);
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME
  Gets or sets the current working directory.
  $H example
  {{{
  currentDirectory = '/home/foobar';
  }}}
**/
DEFINE_PROPERTY_GETTER( currentDirectory ) {

	char buf[PATH_MAX];
#ifdef XP_WIN
//	_getcwd(buf, sizeof(buf));
	::GetCurrentDirectory(COUNTOF(buf), buf);
#else // XP_WIN
	getcwd(buf, sizeof(buf));
#endif // XP_WIN
	JSString *str = JS_NewStringCopyZ(cx, buf);
	JL_CHK( str );
	*vp = STRING_TO_JSVAL( str );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( currentDirectory ) {

	const char *buf;
	JL_CHK( JsvalToString(cx, vp, &buf ) );
#ifdef XP_WIN
//	_chdir(buf);
	::SetCurrentDirectory(buf);
#else // XP_WIN
	chdir(buf);
#endif // XP_WIN
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME $READONLY
  Get the host' directory separator.
  $H example
  {{{
  var isRootDir = (currentDirectory == directorySeparator);
  }}}
**/
DEFINE_PROPERTY( directorySeparator ) {

	jschar sep = PR_GetDirectorySeparator();
	JSString *str = JS_InternUCStringN(cx, &sep, 1);
	JL_CHK( str );
	*vp = STRING_TO_JSVAL( str );
	return JL_StoreProperty(cx, obj, id, vp, true);
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $STR $INAME $READONLY
  Get the host' path separator.
  $H example
  {{{
  Print( GetEnv('PATH').split($INAME) )
  }}}
**/
DEFINE_PROPERTY( pathSeparator ) {

	jschar sep = PR_GetPathSeparator();
	JSString *str = JS_InternUCStringN(cx, &sep, 1);
	JL_CHK( str );
	*vp = STRING_TO_JSVAL( str );
	return JL_StoreProperty(cx, obj, id, vp, true);
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Hold the current version of NSPR.
**/
DEFINE_PROPERTY( version ) {

	*vp = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, PR_VERSION));
	return JL_StoreProperty(cx, obj, id, vp, true);
}


#ifdef DEBUG
JLThreadFuncDecl jsioTestThread( void *data ) {
	
	JLSemaphoreHandler sem = (JLMutexHandler)data;


	JLThreadExit();
	return 0;
}

DEFINE_FUNCTION( jsioTest ) {

	JLSemaphoreHandler sem = JLCreateSemaphore(2);

	JLAcquireSemaphore(sem, -1);
	JLAcquireSemaphore(sem, -1);

	JLThreadHandler thread = JLThreadStart(jsioTestThread, sem);


	return JS_TRUE;
	JL_BAD;
}
#endif // DEBUG


CONFIGURE_STATIC

	REVISION(JL_SvnRevToInt("$Revision$"))
	BEGIN_STATIC_FUNCTION_SPEC

		#ifdef DEBUG
		FUNCTION( jsioTest )
		#endif // DEBUG

		FUNCTION( Poll ) // Do not turn it in FAST NATIVE because we need a stack frame for debuging
		FUNCTION_FAST( MetaPollIO )
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
		FUNCTION_FAST( AvailableSpace )
		FUNCTION_FAST( ConfigureSerialPort )
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_READ( hostName )
		PROPERTY_READ( physicalMemorySize )
		PROPERTY_READ( systemInfo )
		PROPERTY( processPriority )
		PROPERTY_READ( numberOfProcessors )
		PROPERTY( currentDirectory )
		PROPERTY_READ( directorySeparator )
		PROPERTY_READ( pathSeparator )
		PROPERTY_READ( version )
	END_STATIC_PROPERTY_SPEC

END_STATIC
