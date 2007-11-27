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
#include "../common/jsNativeInterface.h"

#include "error.h"
#include "descriptor.h"
#include "file.h"

#define MAX_IP_STRING 39 // IPv4 & IPv6

BEGIN_CLASS( Socket )


DEFINE_FINALIZE() {

	FinalizeDescriptor(cx, obj); // defined in descriptor.cpp
}

DEFINE_CONSTRUCTOR() {
	
	RT_ASSERT_CONSTRUCTING( _class );

	int descType;
	if ( argc >= 1 )
		RT_JSVAL_TO_INT32( argv[0], descType );
	else
		descType = PR_DESC_SOCKET_TCP; // default

	PRFileDesc *fd;
	
	if ( descType == PR_DESC_SOCKET_TCP )
		fd = PR_NewTCPSocket();
	else if ( descType == PR_DESC_SOCKET_UDP )
		fd = PR_NewUDPSocket();
	else
		REPORT_ERROR( "Invalid socket type." );

	if ( fd == NULL )
		return ThrowIoError(cx);

	RT_CHECK_CALL( JS_SetPrivate( cx, obj, fd ) );
	return JS_TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// arg[0] =  false: SHUTDOWN_RCV | true: SHUTDOWN_SEND | else it will SHUTDOWN_BOTH
DEFINE_FUNCTION( Shutdown ) {

	PRFileDesc *fd = (PRFileDesc*)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( fd );

	PRShutdownHow how;
	if ( argc >= 1 )
		if ( argv[0] == JSVAL_FALSE )
			how = PR_SHUTDOWN_RCV;
		else if (argv[0] == JSVAL_TRUE )
			how = PR_SHUTDOWN_SEND;
		else
			REPORT_ERROR("Invalid Shutdown case.");
	else
		how = PR_SHUTDOWN_BOTH; // default

	if ( how == PR_SHUTDOWN_BOTH || how == PR_SHUTDOWN_RCV )
		RemoveNativeInterface(cx, obj, NI_READ_RESOURCE);
	if (PR_Shutdown( fd, how ) != PR_SUCCESS) // is this compatible with linger ?? need to check PR_WOULD_BLOCK_ERROR ???
		return ThrowIoError(cx);

	return JS_TRUE;
}


DEFINE_FUNCTION( Bind ) {

	RT_ASSERT_ARGC( 1 ); // need port number (at least)
	PRFileDesc *fd = (PRFileDesc*)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( fd );

	PRNetAddr addr;
	PRUint16 port;
	RT_JSVAL_TO_INT32( argv[0], port );

	if ( argc >= 2 && argv[1] != JSVAL_VOID ) { // if we have a second argument and this argument is not undefined

		char *host;
		RT_JSVAL_TO_STRING( argv[1], host );

		if ( PR_StringToNetAddr(host, &addr) != PR_SUCCESS )
			return ThrowIoError(cx);

		if ( PR_InitializeNetAddr(PR_IpAddrNull, port, &addr) != PR_SUCCESS )
			return ThrowIoError(cx);
	} else {
		
		if ( PR_InitializeNetAddr(PR_IpAddrAny, port, &addr) != PR_SUCCESS )
			return ThrowIoError(cx);
	}

	if ( PR_Bind(fd, &addr) != PR_SUCCESS ) {
		
		PRErrorCode errorCode = PR_GetError();
		switch (errorCode) {
			case PR_ADDRESS_IN_USE_ERROR: // do not failed but return false
				*rval = JSVAL_FALSE;
				break;
			default:
				return ThrowIoErrorArg(cx, errorCode, PR_GetOSError());
		}
	} else {
		
		*rval = JSVAL_TRUE; // no error, return true
	}
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
	PRIntn backlog;
	if ( argc >= 1 )
		RT_JSVAL_TO_INT32( argv[0], backlog );
	else
		backlog = 8; // too low ??
	if ( PR_Listen(fd, backlog) != PR_SUCCESS )
		return ThrowIoError(cx);
	return JS_TRUE;
}


///////////////////////////////////////////////////////////////////////////////
DEFINE_FUNCTION( Accept ) {

	PRFileDesc *fd = (PRFileDesc*)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( fd );

	PRIntervalTime connectTimeout;
	if ( argc >= 1 ) {

		PRUint32 timeoutInMilliseconds;
		RT_JSVAL_TO_INT32( argv[0], timeoutInMilliseconds );
		connectTimeout = PR_MillisecondsToInterval(timeoutInMilliseconds);
	} else
		connectTimeout = PR_INTERVAL_NO_TIMEOUT;

	PRFileDesc *newFd = PR_Accept( fd, NULL, connectTimeout );
	if ( newFd == NULL )
		return ThrowIoError(cx);
	SetNativeInterface(cx, obj, NI_READ_RESOURCE, (FunctionPointer)NativeInterfaceReadDescriptor, newFd);
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

	char *host;
	RT_JSVAL_TO_STRING( argv[0], host );

	PRUint16 port;
	RT_JSVAL_TO_INT32( argv[1], port );

	PRIntervalTime connectTimeout;
	if ( argc >= 3 ) {

		PRUint32 timeoutInMilliseconds;
		RT_JSVAL_TO_INT32( argv[2], timeoutInMilliseconds );
		connectTimeout = PR_MillisecondsToInterval(timeoutInMilliseconds);
	} else
		connectTimeout = PR_INTERVAL_NO_TIMEOUT;

	PRNetAddr addr;

	if ( PR_StringToNetAddr(host, &addr) == PR_SUCCESS ) {

		if ( PR_InitializeNetAddr(PR_IpAddrNull, port, &addr) != PR_SUCCESS )
			return ThrowIoError(cx);
	} else {

		char netdbBuf[PR_NETDB_BUF_SIZE];
		PRHostEnt hostEntry;
		PRIntn hostIndex;

		if ( PR_GetHostByName( host, netdbBuf, sizeof(netdbBuf), &hostEntry ) != PR_SUCCESS )
			return ThrowIoError(cx);

		hostIndex = 0;
		hostIndex = PR_EnumerateHostEnt(hostIndex, &hostEntry, port, &addr); // data is valid until return is 0 or -1
		if ( hostIndex == -1 )
			return ThrowIoError(cx);
	}

	if ( PR_Connect( fd, &addr, connectTimeout ) != PR_SUCCESS ) { // Doc: timeout is ignored in nonblocking mode ( cf. PR_INTERVAL_NO_WAIT )

		PRErrorCode errorCode = PR_GetError();
		switch (errorCode) {
			case PR_CONNECT_TIMEOUT_ERROR:
				*rval = JSVAL_FALSE;
				return JS_TRUE;
			case PR_IN_PROGRESS_ERROR: // not nonblocking-error
				break;
			default:
				return ThrowIoError(cx);
		}
	}
	// see 	PR_GetConnectStatus or PR_ConnectContinue INSTEAD ???

	SetNativeInterface(cx, obj, NI_READ_RESOURCE, (FunctionPointer)NativeInterfaceReadDescriptor, fd);
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}



DEFINE_FUNCTION( SendTo ) {
		
	RT_ASSERT_ARGC( 3 );

	PRFileDesc *fd;
	if ( JS_GET_CLASS(cx, obj) == _class )
		fd = (PRFileDesc*)JS_GetPrivate( cx, obj );
	else
		fd = PR_NewUDPSocket(); // allow to use SendTo as static function
	RT_ASSERT_RESOURCE( fd );

	char *host;
	RT_JSVAL_TO_STRING( argv[0], host );

	PRUint16 port;
	RT_JSVAL_TO_INT32( argv[1], port );

	PRNetAddr addr;

	if ( PR_StringToNetAddr(host, &addr) == PR_SUCCESS ) {

		if ( PR_InitializeNetAddr(PR_IpAddrNull, port, &addr) != PR_SUCCESS )
			return ThrowIoError(cx);
	} else {

		char netdbBuf[PR_NETDB_BUF_SIZE];
		PRHostEnt hostEntry;
		PRIntn hostIndex;

		if ( PR_GetHostByName( host, netdbBuf, sizeof(netdbBuf), &hostEntry ) != PR_SUCCESS )
			return ThrowIoError(cx);

		hostIndex = 0;
		hostIndex = PR_EnumerateHostEnt(hostIndex, &hostEntry, port, &addr); // data is valid until return is 0 or -1
		if ( hostIndex == -1 )
			return ThrowIoError(cx);
	}

	char *str;
	int len;
	RT_JSVAL_TO_STRING_AND_LENGTH( argv[2], str, len );

	PRInt32 res = PR_SendTo(fd, str, len, 0, &addr, PR_INTERVAL_NO_TIMEOUT );

	PRInt32 sentAmount;
	if ( res == -1 ) {

		PRErrorCode errCode = PR_GetError();
		if ( errCode != PR_WOULD_BLOCK_ERROR )
			return ThrowIoError(cx);
		sentAmount = 0;
	} else
		sentAmount = res;

	if ( sentAmount < len )
		*rval = STRING_TO_JSVAL( JS_NewDependentString(cx, JSVAL_TO_STRING( argv[2] ), sentAmount, len - sentAmount) ); // return unsent data
	else if ( sentAmount == 0 )
		*rval = argv[2]; // nothing has been sent
	else
		*rval = JS_GetEmptyStringValue(cx); // nothing remains

	if ( JS_GET_CLASS(cx, obj) != _class )
		PR_Close(fd);

	return JS_TRUE;
}



DEFINE_FUNCTION( RecvFrom ) {
		
//	PRFileDesc *fd = (PRFileDesc*)JS_GetPrivate( cx, obj );
//	RT_ASSERT_RESOURCE( fd );
	PRFileDesc *fd;
	if ( JS_GET_CLASS(cx, obj) == _class )
		fd = (PRFileDesc*)JS_GetPrivate( cx, obj );
	else
		fd = PR_NewUDPSocket(); // allow to use SendTo as static function
	RT_ASSERT_RESOURCE( fd );



	PRInt64 available = PR_Available64( fd );
	if ( available == -1 )
		return ThrowIoError(cx);

	if ( available == 0 ) {

		*rval = JS_GetEmptyStringValue(cx);
		return JS_TRUE;
	}

	char *buffer = (char *)JS_malloc(cx, available+1);
	RT_ASSERT_ALLOC( buffer );
	buffer[available] = '\0';

	PRNetAddr addr;
	PRInt32 res = PR_RecvFrom(fd, buffer, available, 0, &addr, PR_INTERVAL_NO_TIMEOUT );

	if ( res == -1 ) {

		PRErrorCode errCode = PR_GetError();
		if ( errCode != PR_WOULD_BLOCK_ERROR ) {

			JS_free(cx, buffer);
			return ThrowIoError(cx);
		}
	}
	
//	RT_ASSERT( res == available, "Invalid size" );

	JSString *strBuffer = JS_NewString(cx, buffer, res);
	RT_ASSERT_ALLOC( strBuffer ); // (TBD) else free buffer

	char peerName[46]; // If addr is an IPv4 address, size needs to be at least 16. If addr is an IPv6 address, size needs to be at least 46.
	PRStatus status = PR_NetAddrToString(&addr, peerName, sizeof(peerName)); // Converts a character string to a network address.
	if ( status != PR_SUCCESS ) {

		JS_free(cx, buffer);
		return ThrowIoError(cx);
	}

	JSString *strPeerName = JS_NewStringCopyZ(cx, peerName);
	RT_ASSERT_ALLOC(strPeerName); // (TBD) else free buffer ? ( or, this is a fatal error, program should stop )

	int port = PR_NetAddrInetPort(&addr);

	jsval arrayItems[] = { STRING_TO_JSVAL(strBuffer), STRING_TO_JSVAL(strPeerName), INT_TO_JSVAL(port) };
	JSObject *arrayObject = JS_NewArrayObject(cx, sizeof(arrayItems), arrayItems );
	RT_ASSERT_ALLOC( arrayObject ); // (TBD) else free buffer
	*rval = OBJECT_TO_JSVAL( arrayObject );

	if ( JS_GET_CLASS(cx, obj) != _class )
		PR_Close(fd);


	return JS_TRUE;
}



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

		bool closeAfterTransmit;
		RT_JSVAL_TO_BOOL( argv[1], closeAfterTransmit );
		if ( closeAfterTransmit )
			flag = PR_TRANSMITFILE_CLOSE_SOCKET;
	}

	char *headers = NULL;
	int headerLength = 0;
	if ( argc >= 3 )
		RT_JSVAL_TO_STRING_AND_LENGTH( argv[2], headers, headerLength );

	PRIntervalTime connectTimeout;
	if ( argc >= 4 ) {

		PRUint32 timeoutInMilliseconds;
		RT_JSVAL_TO_INT32( argv[3], timeoutInMilliseconds );
		connectTimeout = PR_MillisecondsToInterval(timeoutInMilliseconds);
	} else
		connectTimeout = PR_INTERVAL_NO_TIMEOUT;

	PRInt32 bytes = PR_TransmitFile( socketFd, fileFd, NULL, 0, flag, connectTimeout );
	if ( bytes == -1 )
		return ThrowIoError(cx);

	RT_CHECK_CALL( JS_NewNumberValue(cx, bytes, rval) );
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

	PRPollDesc desc = { fd, PR_POLL_WRITE | PR_POLL_EXCEPT, 0 };

	PRInt32 result = PR_Poll( &desc, 1, PR_INTERVAL_NO_WAIT ); // this avoid to store out_flags from the previous poll
	if ( result == -1 )
		return ThrowIoError(cx);

	if ( result == 0 ) {

		JS_ReportError( cx, "no connection are pending" );
		return JS_FALSE;
	}

	// this can help ? : http://lxr.mozilla.org/seamonkey/search?string=PR_ConnectContinue
	// source: http://lxr.mozilla.org/seamonkey/source/nsprpub/pr/src/io/prsocket.c#287
	// example: http://www.google.com/codesearch?hl=en&q=+PR_ConnectContinue+show:Su0oyYj9cVc:HRv_2Hg8Bm0:u0BRTcINCf8&sa=N&cd=5&ct=rc&cs_p=http://archive.mozilla.org/pub/mozilla.org/mozilla/releases/mozilla1.1a/src/mozilla-source-1.1a.tar.bz2&cs_f=mozilla/netwerk/base/src/nsSocketTransport.cpp#l926

	if ( PR_ConnectContinue( fd, desc.out_flags ) != PR_SUCCESS ) { // If the nonblocking connect has successfully completed, PR_ConnectContinue returns PR_SUCCESS

		if ( PR_GetError() == PR_IN_PROGRESS_ERROR ) {

			*vp = JSVAL_VOID; // Operation is still in progress
		} else {

			*vp = JSVAL_FALSE; // Connection refused, ...
		}
	} else {

		*vp = JSVAL_TRUE; // We are connected.
	}

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
		return ThrowIoError(cx);

	if ( available == 0 ) {

		// socket is readable ?
		PRPollDesc desc = { fd, PR_POLL_READ, 0 };
		PRInt32 result = PR_Poll( &desc, 1, PR_INTERVAL_NO_WAIT );
		if ( result == -1 ) // error
			return ThrowIoError(cx);

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


///////////////////////////////////////////////////////////////////////////////
enum { 
	linger = PR_SockOpt_Linger, 
	noDelay = PR_SockOpt_NoDelay, 
	reuseAddr = PR_SockOpt_Reuseaddr, 
	keepAlive = PR_SockOpt_Keepalive, 
	recvBufferSize = PR_SockOpt_RecvBufferSize, 
	sendBufferSize = PR_SockOpt_SendBufferSize,
	maxSegment = PR_SockOpt_MaxSegment,
   nonblocking = PR_SockOpt_Nonblocking,
	broadcast = PR_SockOpt_Broadcast,
	multicastLoopback = PR_SockOpt_McastLoopback
};

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
		case PR_SockOpt_MaxSegment: {
			uint32 size;
			JS_ValueToECMAUint32( cx, *vp, &size );
			sod.value.max_segment = size;
		} break;
		case PR_SockOpt_Nonblocking: {
			JSBool boolValue;
			JS_ValueToBoolean( cx, *vp, &boolValue );
			sod.value.non_blocking = ( boolValue == JS_TRUE );
		} break;
		case PR_SockOpt_Broadcast: {
			JSBool boolValue;
			JS_ValueToBoolean( cx, *vp, &boolValue );
			sod.value.broadcast = ( boolValue == JS_TRUE );
		} break;
		case PR_SockOpt_McastLoopback: {
			JSBool boolValue;
			JS_ValueToBoolean( cx, *vp, &boolValue );
			sod.value.mcast_loopback = ( boolValue == JS_TRUE );
		} break;

	}
	if ( PR_SetSocketOption(fd, &sod) != PR_SUCCESS )
		return ThrowIoError(cx);
	return JS_TRUE;
}

///////////////////////////////////////////////////////////////////////////////
DEFINE_PROPERTY( OptionGetter ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( fd );

	PRSocketOptionData sod;
	sod.option = (PRSockOption)JSVAL_TO_INT( id );
	if ( PR_GetSocketOption(fd, &sod) != PR_SUCCESS )
		return ThrowIoError(cx);
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
		case PR_SockOpt_MaxSegment:
			*vp = INT_TO_JSVAL(sod.value.max_segment);
			break;
		case PR_SockOpt_Nonblocking:
			*vp = sod.value.non_blocking == PR_TRUE ? JSVAL_TRUE : JSVAL_FALSE;
			break;
		case PR_SockOpt_Broadcast:
			*vp = sod.value.broadcast == PR_TRUE ? JSVAL_TRUE : JSVAL_FALSE;
			break;
		case PR_SockOpt_McastLoopback:
			*vp = sod.value.mcast_loopback == PR_TRUE ? JSVAL_TRUE : JSVAL_FALSE;
			break;
	}
	return JS_TRUE;
}

DEFINE_PROPERTY( peerName ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( fd );
	PRNetAddr peerAddr;
	if ( PR_GetPeerName(fd, &peerAddr) != PR_SUCCESS )
		return ThrowIoError(cx);
	char buf[MAX_IP_STRING + 1];
	if ( PR_NetAddrToString(&peerAddr, buf, sizeof(buf)) != PR_SUCCESS )
		return ThrowIoError(cx);
	*vp = STRING_TO_JSVAL( JS_NewStringCopyZ( cx, buf ) );
	return JS_TRUE;
}

DEFINE_PROPERTY( peerPort ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( fd );
	PRNetAddr peerAddr;
	if ( PR_GetPeerName(fd, &peerAddr) != PR_SUCCESS )
		return ThrowIoError(cx);
	*vp = INT_TO_JSVAL( PR_ntohs(PR_NetAddrInetPort(&peerAddr)) );
	return JS_TRUE;
}

DEFINE_PROPERTY( sockName ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( fd );
	PRNetAddr sockAddr;
	if ( PR_GetSockName( fd, &sockAddr ) != PR_SUCCESS )
		return ThrowIoError(cx);
	char buf[MAX_IP_STRING + 1];
	if ( PR_NetAddrToString( &sockAddr, buf, sizeof(buf) ) != PR_SUCCESS )
		return ThrowIoError(cx);
	*vp = STRING_TO_JSVAL( JS_NewStringCopyZ( cx, buf ) );
	return JS_TRUE;
}

DEFINE_PROPERTY( sockPort ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( fd );
	PRNetAddr sockAddr;
	if ( PR_GetSockName( fd, &sockAddr ) != PR_SUCCESS )
		return ThrowIoError(cx);
	*vp = INT_TO_JSVAL( PR_ntohs(PR_NetAddrInetPort(&sockAddr)) );
	return JS_TRUE;
}


DEFINE_FUNCTION( GetHostsByName ) {

	RT_ASSERT_ARGC( 1 )

	char *host;
	RT_JSVAL_TO_STRING( argv[0], host );

//	PRUint16 port;
//	RT_JSVAL_TO_INT32( argv[1], port );

	char netdbBuf[PR_NETDB_BUF_SIZE];
	PRHostEnt hostEntry;
	PRNetAddr addr;
	
	if ( PR_GetHostByName( host, netdbBuf, sizeof(netdbBuf), &hostEntry ) != PR_SUCCESS )
		return ThrowIoError(cx);

	JSObject *addrJsObj = JS_NewArrayObject(cx, 0, NULL);
	RT_ASSERT_ALLOC( addrJsObj );

	int index = 0;
	PRIntn hostIndex = 0;
	char addrStr[MAX_IP_STRING + 1];

	for (;;) {

		hostIndex = PR_EnumerateHostEnt(hostIndex, &hostEntry, 0, &addr);
		if ( hostIndex == -1 )
			return ThrowIoError(cx);
		if ( hostIndex == 0 )
			break;
		if ( PR_NetAddrToString(&addr, addrStr, sizeof(addrStr)) != PR_SUCCESS )
			return ThrowIoError(cx);
		JSString *str = JS_NewStringCopyZ(cx, addrStr);
		RT_ASSERT_ALLOC( str );
		JS_DefineElement(cx, addrJsObj, index++, STRING_TO_JSVAL(str), NULL, NULL, JSPROP_ENUMERATE);
	}
	*rval = OBJECT_TO_JSVAL(addrJsObj);
	return JS_TRUE;
}


CONFIGURE_CLASS

	HAS_PROTOTYPE( prototypeDescriptor )

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	HAS_PRIVATE
	HAS_RESERVED_SLOTS( 1 )

	BEGIN_FUNCTION_SPEC
		FUNCTION( Shutdown )
		FUNCTION( Bind )
		FUNCTION( Listen )
		FUNCTION( Accept )
		FUNCTION( Connect )
		FUNCTION( SendTo )
		FUNCTION( RecvFrom )
		FUNCTION( TransmitFile )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
// PR SocketOption
		PROPERTY_SWITCH( linger, Option )
		PROPERTY_SWITCH( noDelay, Option )
		PROPERTY_SWITCH( reuseAddr, Option )
		PROPERTY_SWITCH( keepAlive, Option )
		PROPERTY_SWITCH( recvBufferSize, Option )
		PROPERTY_SWITCH( sendBufferSize, Option )
		PROPERTY_SWITCH( maxSegment, Option )
		PROPERTY_SWITCH( nonblocking, Option )
		PROPERTY_SWITCH( broadcast, Option )
		PROPERTY_SWITCH( multicastLoopback, Option )
		PROPERTY_READ( peerName )
		PROPERTY_READ( peerPort )
		PROPERTY_READ( sockName )
		PROPERTY_READ( sockPort )
		PROPERTY_READ( connectContinue )
		PROPERTY_READ( connectionClosed )
	END_PROPERTY_SPEC

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION( SendTo )
		FUNCTION( RecvFrom )
		FUNCTION( GetHostsByName )
	END_STATIC_FUNCTION_SPEC

	BEGIN_CONST_DOUBLE_SPEC
		CONST_DOUBLE(TCP, PR_DESC_SOCKET_TCP)
		CONST_DOUBLE(UDP, PR_DESC_SOCKET_UDP)
	END_CONST_DOUBLE_SPEC

END_CLASS

/*

Five pitfalls of Linux sockets programming:
	http://www.ibm.com/developerworks/linux/library/l-sockpit/index.html


About closing socket:

		Graceful Shutdown, Linger Options, and Socket Closure
		  http://windowssdk.msdn.microsoft.com/en-us/library/ms738547.aspx
		PRLinger ... PR_Close
		  http://developer.mozilla.org/en/docs/PRLinger

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

About sending data:

		If a partial write occurred, send() returns the number of bytes that were
		written.  If it returns -1 with errno == EWOULDBLOCK, *no* bytes were
		written.

		"With a nonblocking TCP socket, if there is no room at all in the socket
		send buffer, return is made immediately with an error of EWOULDBLOCK.
		If there is some room in the socket send buffer, the return value will
		be the number of bytes that the kernel was able to copy into the buffer."

		page 398, UNIX Network Programming, Second Edition, W. Richard Stevens

		return value : A positive number indicates the number of bytes successfully sent. If the parameter fd is a blocking socket, this number must always equal amount.
		PR_Write(), PR_Send(), PR_Writev() in blocking mode block until the entire buffer is sent. In nonblocking mode, they cannot block, so they may return with just sending part of the buffer.


About receiving data:

		If PR_Poll() reports that the socket is readable (i.e., PR_POLL_READ is set in out_flags),
		and PR_Available() returns 0, this means that the socket connection is closed.
		see http://www.mozilla.org/projects/nspr/tech-notes/nonblockingio.html

		the following code is needed when IE is connected then closed. does not happens with Moz.
		else PR_Recv will return with -1 and error -5962 ( PR_BUFFER_OVERFLOW_ERROR (WSAEMSGSIZE for win32) )


Miscellaneous:


		winsock: http://www.sockets.com/winsock.htm

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


		Ps:
		  source code: http://jslibs.googlecode.com/svn/trunk/jsnspr/nsprSocket.cpp
		  test case: http://jslibs.googlecode.com/svn/trunk/tests/cs.js


*/