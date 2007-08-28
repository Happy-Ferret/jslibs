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
		return ThrowIoError( cx, PR_GetError() );

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
		how = PR_SHUTDOWN_BOTH; // default
	if ( how == PR_SHUTDOWN_BOTH || how == PR_SHUTDOWN_RCV )
		RemoveNativeInterface(cx, obj, NI_READ_RESOURCE);
	PRStatus status;
	status = PR_Shutdown( fd, how ); // is this compatible with linger ??
	if (status != PR_SUCCESS) // need to check PR_WOULD_BLOCK_ERROR ???
		return ThrowIoError( cx, PR_GetError() );
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
		return ThrowIoError( cx, PR_GetError() );

	if ( argc >= 2 ) { // if we have a second argument and this argument is not undefined
		
		char *hostName;
		RT_JSVAL_TO_STRING( argv[1], hostName );

//		if ( hostName[0] != '\0' ) { // else, by default: PR_IpAddrAny ( see PR_InitializeNetAddr )

//	if ( strcmp( hostName, "localhost" ) == 0 )
//			addr.inet.ip = PR_INADDR_LOOPBACK;

		status = PR_StringToNetAddr( hostName, &addr ); // see PR_GetHostByName
		if ( status != PR_SUCCESS )
			return ThrowIoError( cx, PR_GetError() );
//		}
	} // else addr.inet.ip = PR_htonl(PR_INADDR_ANY)

	status = PR_Bind(fd, &addr);
	if ( status != PR_SUCCESS )
		return ThrowIoError( cx, PR_GetError() );
	return JS_TRUE;
}


DEFINE_FUNCTION( Listen ) {

	PRFileDesc *fd = (PRFileDesc*)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( fd );
	PRStatus status;
	PRIntn backlog;
	if ( argc >= 1 )
		RT_JSVAL_TO_INT32( argv[0], backlog );
	else
		backlog = 8; // too low ??
	status = PR_Listen(fd, backlog);
	if ( status != PR_SUCCESS )
		return ThrowIoError( cx, PR_GetError() );
	return JS_TRUE;
}


///////////////////////////////////////////////////////////////////////////////
DEFINE_FUNCTION( Accept ) {

	PRFileDesc *fd = (PRFileDesc*)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( fd );
	PRFileDesc *newFd = PR_Accept( fd, NULL, PR_INTERVAL_NO_TIMEOUT ); // cf. PR_INTERVAL_NO_WAIT and nonblocking socket
	if ( newFd == NULL )
		return ThrowIoError( cx, PR_GetError() );
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

	char *hostName;
	RT_JSVAL_TO_STRING( argv[0], hostName );

	PRUint16 port;
	RT_JSVAL_TO_INT32( argv[1], port );

	PRStatus status;

	char netdb_buf[PR_NETDB_BUF_SIZE];
	PRHostEnt he;
	status = PR_GetHostByName( hostName, netdb_buf, sizeof(netdb_buf), &he );

	if ( status != PR_SUCCESS )
		return ThrowIoError( cx, PR_GetError() );

	PRNetAddr addr;
	PRIntn next_index = 0;

	next_index = PR_EnumerateHostEnt(next_index, &he, port, &addr); // data is valid until return is -1
	if ( next_index == -1 )
		return ThrowIoError( cx, PR_GetError() );

	status = PR_Connect( fd, &addr , /*PRIntervalTime*/ PR_INTERVAL_NO_TIMEOUT ); // timeout is ignored in nonblocking mode ( cf. PR_INTERVAL_NO_WAIT )
	if ( status != PR_SUCCESS ) {

		PRErrorCode errorCode = PR_GetError();
		if ( errorCode != PR_IN_PROGRESS_ERROR ) // not nonblocking-error
			return ThrowIoError( cx, errorCode );
	}
	// see 	PR_GetConnectStatus or PR_ConnectContinue INSTEAD ???

	SetNativeInterface(cx, obj, NI_READ_RESOURCE, (FunctionPointer)NativeInterfaceReadDescriptor, fd);

	*rval = OBJECT_TO_JSVAL(obj);
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

	PRInt32 bytes = PR_TransmitFile( socketFd, fileFd, NULL, 0, flag, PR_INTERVAL_NO_TIMEOUT );
	if ( bytes == -1 )
		return ThrowIoError( cx, PR_GetError() );

	*rval = INT_TO_JSVAL( bytes ); // (TBD) secure this
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
   nonblocking = PR_SockOpt_Nonblocking
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
		case PR_SockOpt_Nonblocking: {
			JSBool boolValue;
			JS_ValueToBoolean( cx, *vp, &boolValue );
			sod.value.non_blocking = ( boolValue == JS_TRUE );
		} break;

	}
	if ( PR_SetSocketOption(fd, &sod) == PR_FAILURE )
		return ThrowIoError( cx, PR_GetError() );
	return JS_TRUE;
}

///////////////////////////////////////////////////////////////////////////////
DEFINE_PROPERTY( OptionGetter ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( fd );

	PRSocketOptionData sod;
	sod.option = (PRSockOption)JSVAL_TO_INT( id );
	if ( PR_GetSocketOption(fd, &sod) == PR_FAILURE )
		return ThrowIoError( cx, PR_GetError() );
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
		case PR_SockOpt_Nonblocking:
			*vp = sod.value.non_blocking == PR_TRUE ? JSVAL_TRUE : JSVAL_FALSE;
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
		return ThrowIoError( cx, PR_GetError() );

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
		return ThrowIoError( cx, PR_GetError() );

	char buf[16]; // If addr is an IPv4 address, size needs to be at least 16. If addr is an IPv6 address, size needs to be at least 46.
	PR_NetAddrToString( &peerAddr, buf, sizeof(buf) );
	*vp = STRING_TO_JSVAL( JS_NewStringCopyZ( cx, buf ) );
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
		PROPERTY_SWITCH( nonblocking, Option )
		PROPERTY_READ( peerName )
		PROPERTY_READ( sockName )
//		PROPERTY_READ( connectContinue )
//		PROPERTY_READ( connectionClosed )
	END_PROPERTY_SPEC

	BEGIN_CONST_DOUBLE_SPEC
		CONST_DOUBLE(TCP, PR_DESC_SOCKET_TCP)
		CONST_DOUBLE(UDP, PR_DESC_SOCKET_UDP)
	END_CONST_DOUBLE_SPEC

END_CLASS
