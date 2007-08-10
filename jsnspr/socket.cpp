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

#include <pprio.h> // nspr/include/nspr/private

#include "nsprError.h"
#include "socket.h"

#include <string.h>

#include "../common/jsNativeInterface.h"

#include "file.h"

static bool NativeInterfaceReadFile( void *pv, unsigned char *buf, unsigned int *amount ) {

	PRFileDesc *fd = (PRFileDesc *)pv;
	PRInt32 status;
	// because this socket class is non-blocking and NativeInterfaceReadFile cannot manage non-blocking socket, 
	// we simulate blocking socket using poll() function. the maximum timeout is 10 seconds.
	// (TBD) check again !
	PRPollDesc desc;
	desc.fd = fd;
	desc.in_flags = PR_POLL_READ;
	desc.out_flags = 0;
	status = PR_Poll( &desc, 1, PR_MillisecondsToInterval(10000) ); // wait for data
	if ( status == -1 )
		return false;
	PRInt32 tmp = *amount;
	status = PR_Recv( fd, buf, tmp, 0, PR_INTERVAL_NO_WAIT );
	*amount = tmp;
	if ( status == -1 )
		return false;
	*amount = status;
	return true;
}

BEGIN_CLASS( Socket )

DEFINE_FINALIZE() {

	PRFileDesc *fd = (PRFileDesc*)JS_GetPrivate( cx, obj );
	if ( fd != NULL ) { // check if not already closed

		PRStatus status = PR_Close( fd ); // what to do on error ??
		if ( status != PR_SUCCESS )
			JS_ReportError( cx, "a socket descriptor cannot be closed while Finalize" );
		JS_SetPrivate( cx, obj, NULL );
	}
}


// FastCGI need descriptor 0: PRInt32 osfd = fd->secret->md.osfd;
//	  PRFileDesc *fd = PR_AllocFileDesc(0, PR_GetTCPMethods());
//   ...
//   PR_FreeFileDesc(PRFileDesc *fd);
//   -- OR --
// PR_ImportTCPSocket(0)


DEFINE_CONSTRUCTOR() {
	
	RT_ASSERT_CONSTRUCTING( &classSocket );

	int descType = PR_DESC_SOCKET_TCP; // default
	if ( argc >= 1 )
		RT_JSVAL_TO_INT32( argv[0], descType );

	bool useOSFD = ( argc >= 2 );
	PRInt32 osfd;
	if ( useOSFD )
		RT_JSVAL_TO_INT32( argv[1], osfd );

	PRFileDesc *fd;
	switch ( descType ) {
		case PR_DESC_SOCKET_TCP:
			if ( useOSFD )
				fd = PR_ImportTCPSocket(osfd);
			else
				fd = PR_NewTCPSocket();
			break;
		case PR_DESC_SOCKET_UDP:
			if ( useOSFD )
				fd = PR_ImportUDPSocket(osfd);
			else
				fd = PR_NewUDPSocket();
			break;
		default:
			REPORT_ERROR( "Invalid socket type." );
	}
	if ( fd == NULL )
		return ThrowNSPRError( cx, PR_GetError() );
	PRSocketOptionData sod = { PR_SockOpt_Nonblocking, PR_TRUE };
	PRStatus status = PR_SetSocketOption(fd, &sod); // Make the socket nonblocking
	if ( status != PR_SUCCESS )
		return ThrowNSPRError( cx, PR_GetError() );
	JS_SetPrivate( cx, obj, fd );
	return JS_TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// Graceful Shutdown, Linger Options, and Socket Closure
//   http://windowssdk.msdn.microsoft.com/en-us/library/ms738547.aspx
// PRLinger ... PR_Close
//   http://developer.mozilla.org/en/docs/PRLinger
DEFINE_FUNCTION( Close ) {

	PRFileDesc *fd = (PRFileDesc*)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( fd );

	RemoveNativeInterface(cx, obj, NI_READ_RESOURCE );

/*
man page:
	Sets or gets the SO_LINGER option. The argument is a linger structure.
	When enabled, a close(2) or shutdown(2) will not return until all queued messages for the socket have been successfully sent
	or the linger timeout has been reached. Otherwise, the call returns immediately and the closing is done in the background.
	When the socket is closed as part of exit(2), it always lingers in the background.

MSDN: (http://windowssdk.msdn.microsoft.com/en-us/library/ms737582.aspx)
	Enabling SO_LINGER with a nonzero time-out interval on a nonblocking socket is not recommended. In this case,
	the call to closesocket will fail with an error of WSAEWOULDBLOCK if the close operation cannot be completed immediately.
	If closesocket fails with WSAEWOULDBLOCK the socket handle is still valid, and a disconnect is not initiated.
	The application must call closesocket again to close the socket.

Closing non-blocking network sockets
	http://www.squid-cache.org/mail-archive/squid-dev/199805/0065.html


status = PR_Shutdown( fd, PR_SHUTDOWN_BOTH ); // is this compatible with linger ??
if (status != PR_SUCCESS) // need to check PR_WOULD_BLOCK_ERROR ???
	return ThrowNSPRError( cx, PR_GetError() );
*/
	PRStatus status;
	status = PR_Close( fd ); // cf. linger option
	if (status != PR_SUCCESS) {

		PRErrorCode errorCode = PR_GetError();
		if ( errorCode != PR_WOULD_BLOCK_ERROR ) // non-blocking socket  +  linger > 0  +  pr_close  =  non-fatal error PR_WOULD_BLOCK_ERROR
			return ThrowNSPRError( cx, errorCode );
	}
	JS_SetPrivate( cx, obj, NULL );
	JS_ClearScope( cx, obj ); // help to clear readable, writable, exception
	return JS_TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// arg[0] =  false: SHUTDOWN_RCV | true: SHUTDOWN_SEND | else it will SHUTDOWN_BOTH
DEFINE_FUNCTION( Shutdown ) {

	PRFileDesc *fd = (PRFileDesc*)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( fd );
	PRShutdownHow how = PR_SHUTDOWN_BOTH; // default
	if ( argc >= 1 )
		if ( argv[0] == JSVAL_FALSE )
			how = PR_SHUTDOWN_RCV;
		else if (argv[0] == JSVAL_TRUE )
			how = PR_SHUTDOWN_SEND;
	if ( how == PR_SHUTDOWN_BOTH || how == PR_SHUTDOWN_RCV )
		RemoveNativeInterface(cx, obj, NI_READ_RESOURCE);
	PRStatus status;
	status = PR_Shutdown( fd, how ); // is this compatible with linger ??
	if (status != PR_SUCCESS) // need to check PR_WOULD_BLOCK_ERROR ???
		return ThrowNSPRError( cx, PR_GetError() );
	return JS_TRUE;
}

DEFINE_FUNCTION( Bind ) {

	RT_ASSERT_ARGC( 1 ); // need port number (at least)
	PRFileDesc *fd = (PRFileDesc*)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( fd );
	PRStatus status;
	uint16 port;
	RT_JSVAL_TO_INT32( argv[0], port );
	PRNetAddr addr;
	status = PR_InitializeNetAddr(PR_IpAddrAny, port, &addr); // Initializes or reinitializes a network address(
	if ( status != PR_SUCCESS )
		return ThrowNSPRError( cx, PR_GetError() );

	if ( argc >= 2 ) { // if we have a second argument and this argument is not undefined
		
		char *hostName;
		RT_JSVAL_TO_STRING( argv[1], hostName );

//		if ( hostName[0] != '\0' ) { // else, by default: PR_IpAddrAny ( see PR_InitializeNetAddr )

//	if ( strcmp( hostName, "localhost" ) == 0 )
//			addr.inet.ip = PR_INADDR_LOOPBACK;

		status = PR_StringToNetAddr( hostName, &addr ); // see PR_GetHostByName
		if ( status != PR_SUCCESS )
			return ThrowNSPRError( cx, PR_GetError() );
//		}
	} // else addr.inet.ip = PR_htonl(PR_INADDR_ANY)

	status = PR_Bind(fd, &addr);
	if ( status != PR_SUCCESS )
		return ThrowNSPRError( cx, PR_GetError() );
	return JS_TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//Q. PR_ConnectContinue() returns PR_SUCCESS whereas the server socket didn't PR_Accept() the connection.
//A. This behavior is reasonable if you have called PR_Listen on the
//   server socket.  After PR_Listen socket is called, the OS will
//   accept the connection for you as long as the listen queue is
//   not full.  This is why the client side gets a connection success
//   indication before the server side has called PR_Accept.
//   Since your call sequence shows that the SERVER called Listen()
//   and the SERVER socket was readable, the above is what happened.
//    Wan-Teh
DEFINE_FUNCTION( Listen ) {

	PRFileDesc *fd = (PRFileDesc*)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( fd );
	PRStatus status;
	PRIntn backlog = 8; // too low ??
	if ( argc >= 1 )
		RT_JSVAL_TO_INT32( argv[0], backlog );
	status = PR_Listen( fd, backlog );
	if ( status != PR_SUCCESS )
		return ThrowNSPRError( cx, PR_GetError() );
	return JS_TRUE;
}

///////////////////////////////////////////////////////////////////////////////
DEFINE_FUNCTION( Accept ) {

	PRFileDesc *fd = (PRFileDesc*)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( fd );
	PRFileDesc *newFd = PR_Accept( fd, NULL, PR_INTERVAL_NO_TIMEOUT ); // PR_INTERVAL_NO_WAIT ? ignored ?
	if ( newFd == NULL )
		return ThrowNSPRError( cx, PR_GetError() );
	SetNativeInterface(cx, obj, NI_READ_RESOURCE, (FunctionPointer)NativeInterfaceReadFile, newFd);
	JSObject *object = JS_NewObject( cx, &classSocket, NULL, NULL );
	JS_SetPrivate( cx, object, newFd );
	*rval = OBJECT_TO_JSVAL( object );
	return JS_TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// Non-blocking BSD socket connections
//   http://cr.yp.to/docs/connect.html
//
// Why client is writable before server accept the connection ? :
//http://groups.google.fr/group/comp.protocols.tcp-ip/browse_thread/thread/a30f95414eb63c62/cbcdc9f240f2eb6b?lnk=st&q=socket+writable+%2Bbefore+accept&rnum=4&hl=fr#cbcdc9f240f2eb6b
//	The connection is accomplished independently of the application, at
//	the TCP level, (calling accept() just makes an already-established
//	connection available to the application). The client side descriptor
//	becomes writeable when the three-way handshake has completed.
//
//	Since the client side has no way of telling when the server has called
//	accept() it can't wait for the server to do that before making the
//	descriptor writeable.
DEFINE_FUNCTION( Connect ) {

	RT_ASSERT_ARGC( 2 )
	PRFileDesc *fd = (PRFileDesc*)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( fd );

	JSString *jsstr = JS_ValueToString( cx, argv[0] );
	argv[0] = STRING_TO_JSVAL( jsstr );
	char *hostName = JS_GetStringBytes( jsstr );

	uint16 intval;
	JS_ValueToUint16( cx, argv[1], &intval );
	PRUint16 port = intval;

	PRStatus status;

	char netdb_buf[PR_NETDB_BUF_SIZE];
	PRHostEnt he;
	status = PR_GetHostByName( hostName, netdb_buf, sizeof(netdb_buf), &he );

	if ( status != PR_SUCCESS )
		return ThrowNSPRError( cx, PR_GetError() );

	PRNetAddr addr;
	PRIntn next_index = 0;

	next_index = PR_EnumerateHostEnt(next_index, &he, port, &addr); // data is valid until return is -1
	if ( next_index == -1 )
		return ThrowNSPRError( cx, PR_GetError() );

	status = PR_Connect( fd, &addr , /*PRIntervalTime*/ PR_INTERVAL_NO_WAIT ); // timeout is ignored in nonblocking mode
	if ( status != PR_SUCCESS ) {

		PRErrorCode errorCode = PR_GetError();
		if ( errorCode != PR_IN_PROGRESS_ERROR ) // not nonblocking-error
			return ThrowNSPRError( cx, errorCode );
	}
	// see 	PR_GetConnectStatus or PR_ConnectContinue INSTEAD ???

	SetNativeInterface(cx, obj, NI_READ_RESOURCE, (FunctionPointer)NativeInterfaceReadFile, fd);

	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}


///////////////////////////////////////////////////////////////////////////////
DEFINE_FUNCTION( Send ) {

	RT_ASSERT_ARGC( 1 )
	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( fd );

	JSString *jsstr = JS_ValueToString( cx, argv[0] );
	argv[0] = STRING_TO_JSVAL( jsstr ); // protect from GC

	PRInt32 length = JS_GetStringLength( jsstr );
	void *data = JS_GetStringBytes( jsstr );

	PRInt32 result = PR_Send( fd, data, length, 0 /*must always be zero*/, PR_INTERVAL_NO_WAIT ); // timeout is ignored in nonblocking mode

	//If a partial write occurred, send() returns the number of bytes that were
	//written.  If it returns -1 with errno == EWOULDBLOCK, *no* bytes were
	//written.

	//"With a nonblocking TCP socket, if there is no room at all in the socket
	//send buffer, return is made immediately with an error of EWOULDBLOCK.
	//If there is some room in the socket send buffer, the return value will
	//be the number of bytes that the kernel was able to copy into the buffer."
	//
	//page 398, UNIX Network Programming, Second Edition, W. Richard Stevens


	// return value : A positive number indicates the number of bytes successfully sent. If the parameter fd is a blocking socket, this number must always equal amount.
	// PR_Write(), PR_Send(), PR_Writev() in blocking mode block until the entire buffer is sent. In nonblocking mode, they cannot block, so they may return with just sending part of the buffer.


	//	printf( "%d<%d ?", byteSent, length ); // 		PR_WOULD_BLOCK_ERROR;
	if ( result == -1 ) {

		PRErrorCode errorCode = PR_GetError();

		if ( errorCode == PR_WOULD_BLOCK_ERROR ) {

			*rval = argv[0]; // if there is no room at all in the socket send buffer, return is made immediately with an error of EWOULDBLOCK.  Then the datas need to be send again later
			return JS_TRUE;
		}
		return ThrowNSPRError( cx, errorCode );
	}

	if ( result < length ) {

		JSString *jssRemaining = JS_NewDependentString( cx, jsstr, result, length - result ); // JSString jssRemaining = JS_NewStringCopyN( cx, data + result, length - result );
		*rval = STRING_TO_JSVAL(jssRemaining);
	}

	*rval = JS_GetEmptyStringValue( cx );
	return JS_TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// possible optimization: loop on PR_Recv while res > 0
DEFINE_FUNCTION( Recv ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( fd );


	//If PR_Poll() reports that the socket is readable (i.e., PR_POLL_READ is set in out_flags),
	//and PR_Available() returns 0, this means that the socket connection is closed.
	//see http://www.mozilla.org/projects/nspr/tech-notes/nonblockingio.html

	// the following code is needed when IE is connected then closed. does not happens with Moz.
	// else PR_Recv will return with -1 and error -5962 ( PR_BUFFER_OVERFLOW_ERROR (WSAEMSGSIZE for win32) )
	PRInt32 available = PR_Available( fd );
	if ( available == -1 )
		return ThrowNSPRError( cx, PR_GetError() );

	if ( available == 0 ) {

		*rval = JS_GetEmptyStringValue( cx );
		return JS_TRUE;
	}

	PRInt32 amount;
	if ( argc >= 1 ) {
		int32 val;
		JS_ValueToInt32( cx, argv[0], &val );
		amount = val;
		if ( amount > available )
			amount = available;
	} else
		amount = available;

	char *buf = (char*)JS_malloc( cx, amount +1 );
	buf[amount] = 0; // useful ? remember that datas can contain '\0' char !

	PRInt32 res = PR_Recv( fd, buf, amount, 0 /*must always be zero*/, PR_INTERVAL_NO_WAIT );
	if (res == -1) { // failure. The reason for the failure can be obtained by calling PR_GetError.

		JS_free( cx, buf );
		return ThrowNSPRError( cx, PR_GetError() );
	}

	JSString *str = JS_NewString( cx, (char*)buf, res );
	RT_ASSERT_ALLOC( str );
	*rval = STRING_TO_JSVAL(str);
	return JS_TRUE;
}

/*
///////////////////////////////////////////////////////////////////////////////
//
DEFINE_FUNCTION( TransmitFile ) { // WORKS ONLY ON BLOCKING SOCKET !!!

	RT_ASSERT_ARGC( 1 );
	PRFileDesc *socketFd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( socketFd );

	RT_ASSERT_OBJECT( argv[0] );
	JSObject *fileObj = JSVAL_TO_OBJECT( argv[0] );
	RT_ASSERT_CLASS( fileObj, &classFile );
	PRFileDesc *fileFd = (PRFileDesc*)JS_GetPrivate( cx, fileObj );
	RT_ASSERT_RESOURCE( fileFd );

	PRTransmitFileFlags flag = PR_TRANSMITFILE_KEEP_OPEN;
	if ( argc >= 2 ) {

		JSBool b;
		RT_CHECK_CALL( JS_ValueToBoolean(cx, argv[1], &b) );
		if ( b == JS_TRUE )
			flag = PR_TRANSMITFILE_CLOSE_SOCKET;
	}
	char *headers = NULL;
	int headerLength = 0;
	if ( argc >= 3 )
		RT_JSVAL_TO_STRING_AND_LENGTH( argv[2], headers, headerLength );
//	PRInt32 bytes = PR_TransmitFile( socketFd, fileFd, headers, headerLength, flag, PR_INTERVAL_NO_WAIT ); // PR_MillisecondsToInterval(10000)

	PRInt32 bytes = PR_TransmitFile( socketFd, fileFd, NULL, 0, PR_TRANSMITFILE_CLOSE_SOCKET, PR_INTERVAL_NO_TIMEOUT );
	if ( bytes == -1 )
		return ThrowNSPRError( cx, PR_GetError() );

	*rval = INT_TO_JSVAL( bytes ); // (TBD) secure this
	return JS_TRUE;
}
*/

///////////////////////////////////////////////////////////////////////////////
enum { linger = PR_SockOpt_Linger, noDelay = PR_SockOpt_NoDelay, reuseAddr = PR_SockOpt_Reuseaddr, keepAlive = PR_SockOpt_Keepalive, recvBufferSize = PR_SockOpt_RecvBufferSize, sendBufferSize = PR_SockOpt_SendBufferSize };

///////////////////////////////////////////////////////////////////////////////
DEFINE_PROPERTY( OptionSetter ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( fd );

	PRSocketOptionData sod;
	sod.option = (PRSockOption)JSVAL_TO_INT( id );

	switch ( sod.option ) {
		case PR_SockOpt_Linger: { // http://developer.mozilla.org/en/docs/PRLinger
				uint32 timeout;
				JS_ValueToECMAUint32( cx, *vp, &timeout );
				if ( timeout >= 0 ) {
					sod.value.linger.polarity = PR_TRUE;
					sod.value.linger.linger = PR_MillisecondsToInterval(timeout);
				} else {
					sod.value.linger.polarity = PR_FALSE;
				}
			} break;
		case PR_SockOpt_NoDelay: {
			JSBool boolValue;
			JS_ValueToBoolean( cx, *vp, &boolValue );
			sod.value.no_delay = ( boolValue == JS_TRUE );
		} break;
		case PR_SockOpt_Reuseaddr: {
			JSBool boolValue;
			JS_ValueToBoolean( cx, *vp, &boolValue );
			sod.value.reuse_addr = ( boolValue == JS_TRUE );
		} break;
		case PR_SockOpt_Keepalive: {
			JSBool boolValue;
			JS_ValueToBoolean( cx, *vp, &boolValue );
			sod.value.keep_alive = ( boolValue == JS_TRUE );
		} break;
		case PR_SockOpt_RecvBufferSize: {
			uint32 size;
			JS_ValueToECMAUint32( cx, *vp, &size );
			sod.value.recv_buffer_size = size;
		} break;
		case PR_SockOpt_SendBufferSize: {
			uint32 size;
			JS_ValueToECMAUint32( cx, *vp, &size );
			sod.value.send_buffer_size = size;
		} break;

	}
	if ( PR_SetSocketOption(fd, &sod) == PR_FAILURE )
		return ThrowNSPRError( cx, PR_GetError() );
	return JS_TRUE;
}

///////////////////////////////////////////////////////////////////////////////
DEFINE_PROPERTY( OptionGetter ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( fd );

	PRSocketOptionData sod;
	sod.option = (PRSockOption)JSVAL_TO_INT( id );
	if ( PR_GetSocketOption(fd, &sod) == PR_FAILURE )
		return ThrowNSPRError( cx, PR_GetError() );
	switch ( sod.option ) {
		case PR_SockOpt_Linger:
			if ( sod.value.linger.polarity == PR_TRUE )
				JS_NewNumberValue( cx, PR_IntervalToMilliseconds(sod.value.linger.linger), vp );
			else
				*vp = JSVAL_ZERO;
			break;
		case PR_SockOpt_NoDelay:
			*vp = sod.value.no_delay == PR_TRUE ? JSVAL_TRUE : JSVAL_FALSE;
			break;
		case PR_SockOpt_Reuseaddr:
			*vp = sod.value.reuse_addr == PR_TRUE ? JSVAL_TRUE : JSVAL_FALSE;
			break;
		case PR_SockOpt_Keepalive:
			*vp = sod.value.keep_alive == PR_TRUE ? JSVAL_TRUE : JSVAL_FALSE;
			break;
		case PR_SockOpt_RecvBufferSize:
			*vp = INT_TO_JSVAL(sod.value.recv_buffer_size);
			break;
		case PR_SockOpt_SendBufferSize:
			*vp = INT_TO_JSVAL(sod.value.send_buffer_size);
			break;
	}
	return JS_TRUE;
}

DEFINE_PROPERTY( peerName ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( fd );

	PRStatus status;
	PRNetAddr peerAddr;
	status = PR_GetPeerName( fd, &peerAddr );
	if ( status == PR_FAILURE )
		return ThrowNSPRError( cx, PR_GetError() );

	char buf[16]; // If addr is an IPv4 address, size needs to be at least 16. If addr is an IPv6 address, size needs to be at least 46.
	PR_NetAddrToString( &peerAddr, buf, sizeof(buf) );
	*vp = STRING_TO_JSVAL( JS_NewStringCopyZ( cx, buf ) );
	return JS_TRUE;
}

DEFINE_PROPERTY( sockName ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( fd );

	PRStatus status;
	PRNetAddr peerAddr;
	status = PR_GetSockName( fd, &peerAddr );
	if ( status == PR_FAILURE )
		return ThrowNSPRError( cx, PR_GetError() );

	char buf[16]; // If addr is an IPv4 address, size needs to be at least 16. If addr is an IPv6 address, size needs to be at least 46.
	PR_NetAddrToString( &peerAddr, buf, sizeof(buf) );
	*vp = STRING_TO_JSVAL( JS_NewStringCopyZ( cx, buf ) );
	return JS_TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// http://developer.mozilla.org/en/docs/PR_GetConnectStatus
// After PR_Connect on a nonblocking socket fails with PR_IN_PROGRESS_ERROR,
// you may wait for the connection to complete by calling PR_Poll on the socket with the in_flags PR_POLL_WRITE | PR_POLL_EXCEPT.
// When PR_Poll returns, call PR_GetConnectStatus on the socket to determine whether the nonblocking connect has succeeded or failed.
DEFINE_PROPERTY( connectContinue ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( fd );

	PRStatus status;
	PRPollDesc desc = { fd, PR_POLL_WRITE | PR_POLL_EXCEPT, 0 };

	PRInt32 result = PR_Poll( &desc, 1, PR_INTERVAL_NO_WAIT ); // this avoid to store out_flags from the previous poll
	if ( result == -1 )
		return ThrowNSPRError( cx, PR_GetError() );

	if ( result == 0 ) {

		JS_ReportError( cx, "no connection are pending" );
		return JS_FALSE;
	}

	// this can help ? : http://lxr.mozilla.org/seamonkey/search?string=PR_ConnectContinue
	// source: http://lxr.mozilla.org/seamonkey/source/nsprpub/pr/src/io/prsocket.c#287
	status = PR_ConnectContinue( fd, desc.out_flags ); // If the nonblocking connect has successfully completed, PR_ConnectContinue returns PR_SUCCESS
//	printf( "status: %d\n", status );

	if ( status != PR_SUCCESS )
		*vp = PR_GetError() == PR_IN_PROGRESS_ERROR ? JSVAL_VOID : JSVAL_FALSE;
	else
		*vp = JSVAL_TRUE;

	return JS_TRUE;
}

DEFINE_PROPERTY( connectionClosed ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( fd );

	//If PR_Poll() reports that the socket is readable (i.e., PR_POLL_READ is set in out_flags),
	//and PR_Available() returns 0, this means that the socket connection is closed.
	//see http://www.mozilla.org/projects/nspr/tech-notes/nonblockingio.html

	PRInt32 available = PR_Available( fd );
	if ( available == -1 )
		return ThrowNSPRError( cx, PR_GetError() );

	if ( available == 0 ) {

		// socket is readable ?
		PRPollDesc desc;
		desc.fd = fd;
		desc.in_flags = PR_POLL_READ;
		desc.out_flags = 0;

		PRInt32 result = PR_Poll( &desc, 1, PR_INTERVAL_NO_WAIT );
		if ( result == -1 ) // error
			return ThrowNSPRError( cx, PR_GetError() );

//		printf("out_flags: %x\n", desc.out_flags );
//		printf("result: %x\n", result );

		if ( result == 1 && (desc.out_flags & PR_POLL_READ) != 0 ) {

			*vp = JSVAL_TRUE; // socket is closed
			return JS_TRUE;
		}
	}

	*vp = JSVAL_FALSE;
	return JS_TRUE;
}

CONFIGURE_CLASS

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION( Close )
		FUNCTION( Shutdown )
		FUNCTION( Bind )
		FUNCTION( Listen )
		FUNCTION( Accept )
		FUNCTION( Connect )
		FUNCTION( Send )
		FUNCTION( Recv )
//		FUNCTION( TransmitFile )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
// PR SocketOption
		PROPERTY_SWITCH( linger, Option )
		PROPERTY_SWITCH( noDelay, Option )
		PROPERTY_SWITCH( reuseAddr, Option )
		PROPERTY_SWITCH( keepAlive, Option )
		PROPERTY_SWITCH( recvBufferSize, Option )
		PROPERTY_SWITCH( sendBufferSize, Option )
		PROPERTY_READ( peerName )
		PROPERTY_READ( sockName )
		PROPERTY_READ( connectContinue )
		PROPERTY_READ( connectionClosed )
	END_PROPERTY_SPEC

	BEGIN_CONST_DOUBLE_SPEC
		CONST_DOUBLE(TCP, PR_DESC_SOCKET_TCP)
		CONST_DOUBLE(UDP, PR_DESC_SOCKET_UDP)
	END_CONST_DOUBLE_SPEC

	HAS_PRIVATE
END_CLASS



/*

winsock:
	http://www.sockets.com/winsock.htm




post to mozilla.dev.tech.nspr

Issue with NON-BLOCKING sockets

Hi,
My issue is that :
PR_ConnectContinue() returns PR_SUCCESS whereas the server socket didn't PR_Accept() the connection.

[poll]
SERVER - Listen()
CLIENT - Connect()
[poll]
SERVER - readable ( but PR_Accept() is not called )
CLIENT - writable
CLIENT - PR_ConnectContinue() == PR_SUCCESS   <-------------- I should get PR_FAILURE & PR_GetError() == PR_IN_PROGRESS_ERROR  no ???
SERVER - readable
SERVER - readable
SERVER - readable
SERVER - readable
SERVER - readable
SERVER - readable
...

Do you have any explication ??

Thanks

Franck



Ps:
  source code: http://jslibs.googlecode.com/svn/trunk/jsnspr/nsprSocket.cpp
  test case: http://jslibs.googlecode.com/svn/trunk/tests/cs.js

*/
