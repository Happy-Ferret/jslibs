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
#include <jslibsModule.h>
#include "../jslang/handlePub.h"


DECLARE_CLASS( Descriptor )
DECLARE_CLASS( Pipe )
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

bool InitPollDesc( JSContext *cx, JS::HandleValue descVal, PRPollDesc *pollDesc ) {

	pollDesc->in_flags = 0;
	pollDesc->out_flags = 0;

	if ( descVal.isPrimitive() ) {

		pollDesc->fd = NULL; // indicate to PR_Poll that this PRFileDesc object should be ignored.
		return true;
	}

	JS::RootedObject fdObj(cx, &descVal.toObject());
	JL_ASSERT_INHERITANCE( fdObj, JL_CLASS(Descriptor) );

	// doc:
	//   fd is a pointer to a PRFileDesc object representing a socket or a pollable event. This field can be set to NULL to indicate to PR_Poll that this PRFileDesc object should be ignored.
	pollDesc->fd = (PRFileDesc*)JL_GetPrivate(fdObj); // beware: fd == NULL may be NULL !

	bool has;
	JL_CHK( JS_HasPropertyById(cx, fdObj, JLID(cx, writable), &has) );
	if ( has )
		pollDesc->in_flags |= PR_POLL_WRITE;

	JL_CHK( JS_HasPropertyById(cx, fdObj, JLID(cx, readable), &has) );
	if ( has )
		pollDesc->in_flags |= PR_POLL_READ;

	JL_CHK( JS_HasPropertyById(cx, fdObj, JLID(cx, hangup), &has) );
	if ( has )
		pollDesc->in_flags |= PR_POLL_HUP;

	JL_CHK( JS_HasPropertyById(cx, fdObj, JLID(cx, exception), &has) );
	if ( has )
		pollDesc->in_flags |= PR_POLL_EXCEPT;

	JL_CHK( JS_HasPropertyById(cx, fdObj, JLID(cx, error), &has) );
	if ( has )
		pollDesc->in_flags |= PR_POLL_ERR;

	return true;
	JL_BAD;
}


bool PollDescNotify( JSContext *cx, JS::HandleValue descVal, PRPollDesc *pollDesc, int index ) {

	if ( !descVal.isObject() )
		return true;

	PRInt16 outFlag = pollDesc->out_flags;

	JS::RootedValue tmp(cx);
	JS::RootedValue arg1(cx, JS::Int32Value(index));
	JS::RootedValue arg2(cx, JS::BooleanValue(outFlag & PR_POLL_HUP));
	JS::RootedObject fdObj(cx, &descVal.toObject());
	JL_ASSERT_INHERITANCE(fdObj, JL_CLASS(Descriptor));

	if ( outFlag & PR_POLL_ERR ) {

		JL_CHK( JS_GetProperty( cx, fdObj, "error", &tmp ) ); // (TBD) use JS_HasPropertyById
		if ( jl::isCallable(cx, tmp) )
			JL_CHK( jl::call(cx, fdObj, tmp, &tmp, descVal, arg1, arg2) );
	}

	if ( outFlag & PR_POLL_EXCEPT ) {

		JL_CHK( JS_GetProperty( cx, fdObj, "exception", &tmp ) );
		if ( jl::isCallable(cx, tmp) )
			JL_CHK( jl::call(cx, fdObj, tmp, &tmp, descVal, arg1, arg2) );
	}

	if ( outFlag & PR_POLL_HUP ) {

		JL_CHK( JS_GetProperty( cx, fdObj, "hangup", &tmp ) );
		if ( jl::isCallable(cx, tmp) )
			JL_CHK( jl::call(cx, fdObj, tmp, &tmp, descVal, arg1, arg2) );
	}

	if ( outFlag & PR_POLL_READ ) {

		JL_CHK( JS_GetProperty( cx, fdObj, "readable", &tmp ) );
		if ( jl::isCallable(cx, tmp) )
			JL_CHK( jl::call(cx, fdObj, tmp, &tmp, descVal, arg1, arg2) );
	}

	if ( outFlag & PR_POLL_WRITE ) {

		JL_CHK( JS_GetProperty( cx, fdObj, "writable", &tmp ) );
		if ( jl::isCallable(cx, tmp) )
			JL_CHK( jl::call(cx, fdObj, tmp, &tmp, descVal, arg1, arg2) );
	}

	return true;
	JL_BAD;
}

// see processEvents( descriptor.events )...
/* *doc
$TOC_MEMBER $INAME
 $INT $INAME( _descriptorArray_ [, _timeout_ = undefined ] )
  This function listen for a readable, writable or exception event on each descriptor in _descriptorArray_.
  When an event occurs, the function tries to call the corresponding property (function) on the descriptor.
  $LF
  The function returns the number of events that occurs or 0 if the function timed out.
  $H note
   The event callback property is NOT cleared by the function.
   If you want to stop receiving an event, you have to remove the property by yourself. eg. `delete socket1.writable`.
  $LF
  The second argument, _timeout_, defines the number of milliseconds the function has to wait before returning if no event has occured.
  _timeout_ can be undefined OR omited, in this case, no timeout is used.
  $H beware
   No timeout means that the function will endless wait for events.
  $H example
  {{{
  socket1.readable = function(soc) {
   print( soc.Recv() )
  }
  var count = poll( [socket1, socket2], 1000 );
  if ( count == 0 )
    print( 'Nothing has been recived' );
  }}}
** /
DEFINE_FUNCTION( poll ) {

	PRInt32 result;
	PRIntervalTime pr_timeout;
	unsigned i;
	PRPollDesc *pollDesc = NULL;
	jsval *props = NULL;

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_RANGE( 1, 2 );
	JL_ASSERT_ARG_IS_ARRAY(1);

	JS::RootedObject fdArrayObj(cx);
	fdArrayObj = JSVAL_TO_OBJECT( JL_ARG(1) );

	if ( JL_ARG_ISDEF(2) && !JL_ValueIsPInfinity(cx, JL_ARG(2)) ) {

		PRUint32 tmpint;
		JL_CHKB( jl::getValue(cx, JL_ARG(2), &tmpint), bad1 );
		pr_timeout = PR_MillisecondsToInterval(tmpint);
	} else {

		pr_timeout = PR_INTERVAL_NO_TIMEOUT;
	}

	unsigned propsCount;
	JL_CHKB( JS_GetArrayLength(cx, fdArrayObj, &propsCount), bad1 );

	if ( propsCount == 0 ) { // optimization

		result = PR_Poll(NULL, 0, pr_timeout); // we can replace this by a delay, but the function is Pool, not Sleep.
		if ( result == -1 )
			return ThrowIoError(cx);
		JL_RVAL.setInt32(0);
		return true;
	}

	pollDesc = (PRPollDesc*)jl_malloca(sizeof(PRPollDesc) * propsCount);
	JL_ASSERT_ALLOC( pollDesc );

	props = (jsval*)jl_malloca(sizeof(jsval) * propsCount);
	JL_ASSERT_ALLOC( props );

	memset(props, 0, sizeof(jsval) * propsCount); // needed because JS_PUSH_TEMP_ROOT
	ASSERT( JSVAL_IS_PRIMITIVE(*props) );

	{
		JS::AutoArrayRooter tvr(cx, propsCount, props);
		for ( i = 0; i < propsCount; ++i ) {

			JL_CHK( JL_GetElement(cx, fdArrayObj, i, props[i]) );
			JL_CHK( InitPollDesc(cx, props[i], &pollDesc[i]) );
		}

		result = PR_Poll(pollDesc, propsCount, pr_timeout); // http://www.mozilla.org/projects/nspr/tech-notes/poll-method.html / http://developer.mozilla.org/en/docs/PR_Poll
		if ( result == -1 ) // failed. see PR_GetError()
			JL_CHK( ThrowIoError(cx) ); // returns later

		if ( result > 0 ) // has event(s)
			for ( i = 0; i < propsCount; ++i )
				JL_CHK( PollDescNotify(cx,props[i], &pollDesc[i], i) );

		*JL_RVAL = INT_TO_JSVAL( result );
	}
	jl_freea(props);
	jl_freea(pollDesc);
	return true;
bad:
	jl_freea(props);
	jl_freea(pollDesc);
bad1:
	return false;
}
*/

/**doc
$TOC_MEMBER $INAME
 $INT $INAME()
  Returns the milliseconds value of NSPR's free-running interval timer.
**/
DEFINE_FUNCTION( intervalNow ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC(0);

	// (TBD) Check if it may wrap around in about 12 hours. Is it related to the data type ???
	JL_CHK( jl::setValue(cx, JL_RVAL, PR_IntervalToMilliseconds(PR_IntervalNow())) );
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( [ _milliseconds_ = 0 ] )
  Sleeps _milliseconds_ milliseconds.
**/
DEFINE_FUNCTION( sleep ) {

	PRUint32 timeout;
	JL_DEFINE_ARGS;
	if ( JL_ARG_ISDEF(1) )
		JL_CHK( jl::getValue(cx, JL_ARG(1), &timeout) );
	else
		timeout = 0;
	PR_Sleep( PR_MillisecondsToInterval(timeout) );

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( name )
  Retrieve the value of the given environment variable.
**/
DEFINE_FUNCTION( getEnv ) {

	jl::BufString name;
	
	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC(1);

	JL_CHK( jl::getValue(cx, JL_ARG(1), &name) );
	char* value;
	value = PR_GetEnv(name); // If the environment variable is not defined, the function returns NULL.

	if ( value == NULL || *value == '\0' ) { // this will cause an 'undefined' return value

		JL_RVAL.setUndefined();
	} else {

		JL_CHK( jl::setValue(cx, JL_RVAL, value) );
	}
	return true;
	JL_BAD;
}


/**
$TOC_MEMBER $INAME
 $STR $INAME( name, value )
  Set, unset or change an environment variable.
**/
/*
DEFINE_FUNCTION( setEnv ) {

doc:
	The caller must ensure that the string passed
	to PR_SetEnv() is persistent. That is: The string should
	not be on the stack, where it can be overwritten

	JL_ASSERT_ARGC_MIN(1);

	const char *name, *value;
	JL_CHK( jl::getValue(cx, &JL_ARG(1), &name) );
	JL_CHK( jl::getValue(cx, &JL_ARG(2), &value) );

	PRStatus status = PR_SetEnv...

	return true;
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
DEFINE_FUNCTION( getRandomNoise ) {

	jl::BufBase buffer;

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_MIN( 1 );
	JL_ASSERT_ARG_IS_INTEGER(1);

	PRSize amount;
	amount = JL_ARG(1).toInt32();
	//uint8_t *buf;
	//buf = JL_NewBuffer(cx, rndSize, JL_RVAL);
	//JL_CHK( buf );

	buffer.alloc(amount, true);
	JL_ASSERT_ALLOC( buffer );

	PRSize res;
	res = PR_GetRandomNoise(buffer.data(), amount);
	if ( res == 0 && amount > 0 ) {

		JL_ERR( E_FUNC, E_NOTIMPLEMENTED );
	}

	JL_CHK( BlobCreate(cx, buffer, JL_RVAL) );

	return true;
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

	JL_ASSERT_ARGC_MIN( 1 );

	PRUint32 val;
	J_JSVAL_TO_UINT32( JL_ARG(1), val );

	val = PR_ntohl(val);

	if (

	PR_htonll
	return true;
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
DEFINE_FUNCTION( waitSemaphore ) {

	jl::BufString name;
	
	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_MIN( 1 );

	PRUintn mode;
	mode = PR_IRUSR | PR_IWUSR; // read write permission for owner.
	if ( JL_ARG_ISDEF(2) )
		JL_CHK( jl::getValue(cx, JL_ARG(2), &mode) );

//	const char *name;
//	size_t nameLength;
//	JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(1), &name, &nameLength) );
	JL_CHK( jl::getValue(cx, JL_ARG(1), &name) );

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

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( semaphoreName )
  Increments the value of a specified semaphore.
**/
DEFINE_FUNCTION( postSemaphore ) {

	jl::BufString name;
	
	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_MIN( 1 );

//	const char *name;
//	size_t nameLength;
//	JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(1), &name, &nameLength) );
	JL_CHK( jl::getValue(cx, JL_ARG(1), &name) );

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

	JL_RVAL.setUndefined();
	return true;
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
  var [stdin, stdout] = createProcess( 'c:\\windows\\System32\\cmd.exe', ['/c', 'dir', 'c:\\'], false );
  sleep(100);
  print( stdout.Read(), '\n' );
  stdin.close();
  stdout.close();
  }}}
**/
/*
// Doc. http://www.mozilla.org/projects/nspr/reference/html/prprocess.html#24535
DEFINE_FUNCTION( createProcess ) {

	const char **processArgv = NULL; // keep on top

	JL_ASSERT_ARGC_MIN( 1 );

	int processArgc;
	if ( JL_ARG_ISDEF(2) && JSVAL_IS_OBJECT(JL_ARG(2)) && JL_IsArrayObject( cx, JSVAL_TO_OBJECT(JL_ARG(2)) ) == true ) {

		JSIdArray *idArray;
		idArray = JS_Enumerate( cx, JSVAL_TO_OBJECT(JL_ARG(2)) ); // make a kind of auto-ptr for this
		processArgc = idArray->length +1; // +1 is argv[0]
		processArgv = (const char**)jl_malloc(sizeof(const char**) * (processArgc +1)); // +1 is NULL
		JL_ASSERT_ALLOC( processArgv );

		for ( int i=0; i<processArgc -1; i++ ) { // -1 because argv[0]

			jsval propVal;
			JL_CHK( JS_IdToValue(cx, idArray->vector[i], &propVal ));
			JL_CHK( JL_GetElement(cx, JSVAL_TO_OBJECT(JL_ARG(2)), propVal.toInt32(), &propVal )); // (TBD) optimize

			const char *tmp;
			JL_CHK( jl::getValue(cx, &propVal, &tmp) ); // warning: GC on the returned buffer !
			processArgv[i+1] = tmp;
		}
		JS_DestroyIdArray( cx, idArray );
	} else {

		processArgc = 0 +1; // +1 is argv[0]
		processArgv = (const char**)jl_malloc(sizeof(const char**) * (processArgc +1)); // +1 is NULL
	}

	const char *path;
	JL_CHK( jl::getValue(cx, &JL_ARG(1), &path) );

	processArgv[0] = path;
	processArgv[processArgc] = NULL;

	bool waitEnd;
	waitEnd = false;
	if ( JL_ARG_ISDEF(3) )
		JL_CHK( jl::getValue(cx, JL_ARG(3), &waitEnd) );

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
		*JL_RVAL = INT_TO_JSVAL( exitValue );
	} else {

//		status = PR_DetachProcess(process);
//		if ( status != PR_SUCCESS )
//			return ThrowIoError(cx);

		JSObject *fdin = JS_NewObject( cx, classPipe, NULL, NULL );
		JL_SetPrivate(  fdin, stdin_parent );

		JSObject *fdout = JS_NewObject( cx, classPipe, NULL, NULL );
		JL_SetPrivate(  fdout, stdout_parent );

		JL_CHK( ReserveStreamReadInterface(cx, fdout) );
		JL_CHK( SetStreamReadInterface(cx, fdout, NativeInterfaceStreamRead) );

		jsval vector[2];
		vector[0] = OBJECT_TO_JSVAL( fdin );
		vector[1] = OBJECT_TO_JSVAL( fdout );
		JSObject *arrObj = JS_NewArrayObject(cx, 2, vector);
		*JL_RVAL = OBJECT_TO_JSVAL( arrObj );
	}


	if ( processArgv )
		jl_free(processArgv);
	return true;
bad_throw:
	ThrowIoError(cx);
bad:
	if ( processArgv )
		jl_free(processArgv);
	return false;
}
*/



/**doc
$TOC_MEMBER $INAME
 $REAL $INAME( path )
  Returns the available storage space (in bytes) at the given path.
  $H example
  {{{
  print( availableSpace('/var') );
  }}}
**/
DEFINE_FUNCTION( availableSpace ) {

	jl::BufString path;
	
	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_MIN( 1 );
	
	JL_CHK( jl::getValue(cx, JL_ARG(1), &path) );

	double available;

#ifdef WIN
	ULARGE_INTEGER freeBytesAvailable;
	BOOL res = ::GetDiskFreeSpaceEx(path, &freeBytesAvailable, NULL, NULL);
	if ( res == 0 )
		jl::throwOSError(cx);
	available = (double)freeBytesAvailable.QuadPart;
#else // now for UNIX and  MAC ?
	struct statvfs fsd;
	if ( statvfs(path, &fsd) < 0 )
		jl::throwOSError(cx);
	available = (double)fsd.f_bsize * (double)fsd.f_bavail;
#endif

	JL_RVAL.setDouble(available);

	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( descriptor, [baudRate], [byteSize], [parity], [stopBits] )
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
 f.open(File.RDWR);
 configureSerialPort(f, 9600);
 f.write('A');
 f.read(1);
 }}}
**/
DEFINE_FUNCTION( configureSerialPort ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_RANGE(1,2);
	JL_ASSERT_ARG_IS_OBJECT(1);
	
	PRFileDesc *fd;

	{
	JS::RootedObject fileObj(cx);
	fileObj = &JL_ARG(1).toObject();
	JL_ASSERT_INHERITANCE( fileObj, JL_CLASS(File) );

	fd = (PRFileDesc *)JL_GetPrivate(fileObj);
	}

	JL_ASSERT( fd, E_THISOPERATION, E_INVALID, E_SEP, E_NAME(JL_CLASS_NAME(File)), E_CLOSED );

#ifdef WIN

	DWORD baudRate;
	BYTE byteSize, parity, stopBits;

	HANDLE fh = (HANDLE)PR_FileDesc2NativeHandle(fd);

	BOOL status;
	DCB dcb;
	COMMTIMEOUTS commTimeouts;

	status = GetCommState(fh, &dcb);
	if ( status == 0 )
		jl::throwOSError(cx);

	status = GetCommTimeouts(fh, &commTimeouts);
	if ( status == 0 )
		jl::throwOSError(cx);
	// (TBD) manage commTimeouts


	if ( JL_ARG_ISDEF(2) ) {

		JL_CHK( jl::getValue(cx, JL_ARG(2), &baudRate) );
		dcb.BaudRate = baudRate;
	}

	if ( JL_ARG_ISDEF(3) ) {

		JL_CHK( jl::getValue(cx, JL_ARG(3), &byteSize) );
		dcb.ByteSize = byteSize;
	}

	if ( JL_ARG_ISDEF(4) ) {

		JL_CHK( jl::getValue(cx, JL_ARG(4), &parity) );
		dcb.Parity = parity;
	}

	if ( JL_ARG_ISDEF(5) ) {

		JL_CHK( jl::getValue(cx, JL_ARG(5), &stopBits) );
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
		jl::throwOSError(cx);
#endif // WIN

#ifdef UNIX

//	struct termios tio;
//	tcgetattr(
	// (TBD) see http://www.comptechdoc.org/os/linux/programming/c/linux_pgcserial.html

#endif // UNIX


	JL_RVAL.setUndefined();
	return true;
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
DEFINE_PROPERTY_GETTER( hostName ) {

	JL_DEFINE_PROP_ARGS;

	char tmp[SYS_INFO_BUFFER_LENGTH];
	// doc:
	//   Suppose the name of the host is configured as "foo.bar.com".
	//   If PR_SI_HOSTNAME is specified, "foo" is returned. If PR_SI_HOSTNAME_UNTRUNCATED is specified, "foo.bar.com" is returned.
	//   On the other hand, if the name of the host is configured as just "foo",	both PR_SI_HOSTNAME and PR_SI_HOSTNAME_UNTRUNCATED return "foo".
	PRStatus status = PR_GetSystemInfo(PR_SI_HOSTNAME_UNTRUNCATED, tmp, COUNTOF(tmp));
	if ( status != PR_SUCCESS )
		return ThrowIoError(cx);
	JL_CHK( jl::setValue(cx, JL_RVAL, tmp) );
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME $READONLY
  Is the architecture of the system (eg. x86).
**/
DEFINE_PROPERTY_GETTER( architecture ) {

	JL_DEFINE_PROP_ARGS;

	char tmp[SYS_INFO_BUFFER_LENGTH];
	if (PR_GetSystemInfo(PR_SI_ARCHITECTURE, tmp, COUNTOF(tmp)) != PR_SUCCESS)
		return ThrowIoError(cx);
	JL_CHK( jl::setValue(cx, JL_RVAL, tmp) );
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $STR $INAME $READONLY
  Is the name of the system (eg. Windows_NT).
**/
DEFINE_PROPERTY_GETTER( systemName ) {

	JL_DEFINE_PROP_ARGS;

	char tmp[SYS_INFO_BUFFER_LENGTH];
	if (PR_GetSystemInfo(PR_SI_SYSNAME, tmp, COUNTOF(tmp)) != PR_SUCCESS)
		return ThrowIoError(cx);
	JL_CHK( jl::setValue(cx, JL_RVAL, tmp) );
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $STR $INAME $READONLY
  Is the release number of the system (eg. 2.6.22.18, 5.1, ...).
**/
DEFINE_PROPERTY_GETTER( systemRelease ) {

	JL_DEFINE_PROP_ARGS;

	char tmp[SYS_INFO_BUFFER_LENGTH];
	if (PR_GetSystemInfo(PR_SI_RELEASE, tmp, COUNTOF(tmp)) != PR_SUCCESS)
		return ThrowIoError(cx);
	JL_CHK( jl::setValue(cx, JL_RVAL, tmp) );
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME
  Is the number of processors (CPUs available in a symmetric multiprocessing system).
**/
DEFINE_PROPERTY_GETTER( numberOfProcessors ) {

	JL_DEFINE_PROP_ARGS;

	PRInt32 count = PR_GetNumberOfProcessors();
	if ( count < 0 ) {

		JL_WARN( E_FUNC, E_NOTIMPLEMENTED );
		count = 1;
	}
	JL_CHK( jl::setValue(cx, JL_RVAL, count) );
	//	JL_CHK( jl::StoreProperty(cx, obj, id, vp, true) ); // may change ??
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  Is the amount of physical RAM in the system in bytes.
**/
DEFINE_PROPERTY_GETTER( physicalMemorySize ) {

	JL_DEFINE_PROP_ARGS;

	JL_CHK( jl::setValue(cx, JL_RVAL, PR_GetPhysicalMemorySize()) );
	return true;
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
DEFINE_PROPERTY_GETTER( processPriority ) {

	JL_DEFINE_PROP_ARGS;

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
			IFDEBUG( priorityValue = 0 );
			ASSERT(false);
	}
	vp.setInt32(priorityValue);
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( processPriority ) {

	JL_DEFINE_PROP_ARGS;

	int priorityValue;
	JL_CHK( jl::getValue(cx, vp, &priorityValue) );
	PRThreadPriority priority;
	switch ( priorityValue ) {
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
			priority = PR_PRIORITY_NORMAL;
			JL_WARN( E_VALUE, E_RANGE, E_INTERVAL_NUM(-1, 2) );
	}

	PR_SetThreadPriority(PR_GetCurrentThread(), priority);
	return true;
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

	JL_DEFINE_PROP_ARGS;

	TCHAR buf[PATH_MAX];
#ifdef WIN
//	_getcwd(buf, COUNTOF(buf));
	::GetCurrentDirectory(COUNTOF(buf), buf);
#else
	getcwd(buf, COUNTOF(buf)); // need  #include <direct.h>
#endif // WIN
	JL_CHK( jl::setValue( cx, vp, buf ) );
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( currentDirectory ) {

	JL_DEFINE_PROP_ARGS;

	jl::BufString dir;
	JL_CHK( jl::getValue(cx, vp, &dir ) );
#ifdef WIN
//	_chdir(dir);
	::SetCurrentDirectory(dir);
#else
	chdir(dir);
#endif // WIN
	return true;
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
DEFINE_PROPERTY_GETTER( directorySeparator ) {

	JL_DEFINE_PROP_ARGS;

	char tmp = PR_GetDirectorySeparator();
	JL_CHK( jl::setValue(cx, JL_RVAL, jl::strSpec(&tmp, 1)) );
	JL_CHK( jl::StoreProperty(cx, obj, id, vp, true) );
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $STR $INAME $READONLY
  Get the host' path separator.
  $H example
  {{{
  print( getEnv('PATH').split($INAME) )
  }}}
**/
DEFINE_PROPERTY_GETTER( pathSeparator ) {

	JL_DEFINE_PROP_ARGS;

	const char tmp = PR_GetPathSeparator();
	JL_CHK( jl::setValue(cx, JL_RVAL, jl::strSpec(&tmp, 1)) );
	JL_CHK( jl::StoreProperty(cx, obj, id, vp, true) );
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Hold the current version of NSPR.
**/
DEFINE_PROPERTY_GETTER( version ) {

	JL_DEFINE_PROP_ARGS;

	JL_CHK( jl::setValue(cx, JL_RVAL, PR_VERSION) );
	JL_CHK( jl::StoreProperty(cx, obj, id, vp, true) );
	return true;
	JL_BAD;
}


#if defined(DEBUG) // || 1
#define HAS_JSIOTEST
#endif

#ifdef HAS_JSIOTEST

#if !DEBUG
#pragma message ( "WARNING: test API available in non-debug mode !" )
#endif

#include "jlalloc.h"

DEFINE_FUNCTION( jsioTest ) {

	JL_IGNORE( argc, cx );
	JL_DEFINE_ARGS;



/*
	const char **processArgv;

	processArgv = (const char **)malloc(100);
	
	processArgv[0] = "C:\\Windows\\system32\\cmd.exe";
	processArgv[1] = NULL;

	const char *path = "C:\\Windows\\system32\\cmd.exe";

	PRProcessAttr *psattr;
	psattr = PR_NewProcessAttr();


	PRProcess *process;

	printf("***[%p - %s - %d]\n", processArgv[0], processArgv[0], processArgv[0][0]);

	process = PR_CreateProcess(path, (char *const *)processArgv, NULL, psattr);

	printf("***[%p - %s - %d]\n", processArgv[0], processArgv[0], processArgv[0][0]);
*/


/*
	JSObject *o = JL_NewObj(cx);


	
	jsid idStrOne;
	JS_ValueToId(cx, STRING_TO_JSVAL( JS_NewStringCopyZ(cx, "1") ), &idStrOne);

	jsid idIntOne;
	JS_ValueToId(cx, INT_TO_JSVAL(1), &idIntOne);

	

	jsval tmp;

	tmp = INT_TO_JSVAL(6);
	JS_SetPropertyById(cx, o, idIntOne, &tmp);

	tmp = INT_TO_JSVAL(5);
	JS_SetPropertyById(cx, o, idStrOne, &tmp);


	*JL_RVAL = OBJECT_TO_JSVAL(o);
	return true;
*/

/*
	jsval jsv[65535];

	double err = jl::AccurateTimeCounter();
	double t0 = jl::AccurateTimeCounter();
	t0 = t0 - (t0 - err);

//	for ( int i = 0; i < COUNTOF(jsv); i++ )
//		JS_AddRoot(cx, &jsv[i]);

//	for ( int i = 0; i < 65535; i++ )
//		JS_RemoveRoot(cx, &jsv[i]);

	JSObject *arr = JS_NewArrayObject(cx, 0, NULL);
	jsval v = JSVAL_ONE;
	for ( int i = 0; i < COUNTOF(jsv); i++ )
		JL_SetElement(cx, arr, i, &v);

	double t = jl::AccurateTimeCounter() - t0;
*/

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}
#endif // HAS_JSIOTEST


CONFIGURE_STATIC

	REVISION(jl::SvnRevToInt("$Revision$"))
	BEGIN_STATIC_FUNCTION_SPEC

		#ifdef HAS_JSIOTEST
		FUNCTION( jsioTest )
		#endif // HAS_JSIOTEST

//		FUNCTION( poll ) // Do not turn it in FAST NATIVE because we need a stack frame for debuging
//		FUNCTION( iOEvents ) // moved do Descriptor.Events()
		FUNCTION( intervalNow )
//		FUNCTION( uIntervalNow )
		FUNCTION( sleep )
		FUNCTION( getEnv )
		FUNCTION( getRandomNoise )
		FUNCTION( waitSemaphore )
		FUNCTION( postSemaphore )
//		FUNCTION( createProcess )
		FUNCTION( availableSpace )
		FUNCTION( configureSerialPort )
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_GETTER( hostName )
		PROPERTY_GETTER( architecture )
		PROPERTY_GETTER( systemName )
		PROPERTY_GETTER( systemRelease )
		PROPERTY_GETTER( numberOfProcessors )
		PROPERTY_GETTER( physicalMemorySize )
		PROPERTY( processPriority )
		PROPERTY( currentDirectory )
		PROPERTY_GETTER( directorySeparator )
		PROPERTY_GETTER( pathSeparator )
//		PROPERTY_GETTER( version )
	END_STATIC_PROPERTY_SPEC

END_STATIC
