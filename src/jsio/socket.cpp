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

#include "jsio.h"


DECLARE_CLASS( Descriptor )
DECLARE_CLASS( File )


#define MAX_IP_STRING 39 // IPv4 & IPv6

/**doc
$CLASS_HEADER Descriptor
$SVN_REVISION $Revision$
 Socket class is used to create a non-blocking TCP socket.
**/
BEGIN_CLASS( Socket )


DEFINE_FINALIZE() {

	FinalizeDescriptor(cx, obj); // defined in descriptor.cpp
}

/**doc
$TOC_MEMBER $INAME
 $INAME( [type = Socket.TCP] )
  Type can be Socket.TCP or Socket.UDP.
**/
DEFINE_CONSTRUCTOR() {

	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

	int descType;
	if ( JL_ARG_ISDEF(1) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &descType) );
	else
		descType = PR_DESC_SOCKET_TCP; // default

	PRFileDesc *fd;

	if ( descType == PR_DESC_SOCKET_TCP )
		fd = PR_NewTCPSocket();
	else if ( descType == PR_DESC_SOCKET_UDP )
		fd = PR_NewUDPSocket();
	else
		JL_ERR( E_ARG, E_NUM(1), E_INVALID );

	if ( fd == NULL )
		return ThrowIoError(cx);

	JL_SetPrivate( cx, obj, fd );
	JL_CHK( ReserveStreamReadInterface(cx, obj) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Methods ===
**/

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( [ what ] )
  Shut down part of a full-duplex connection on a socket.
  $LF
  if _what_ is $FALSE, further receives will be disallowed.
  $LF
  if _what_ is $TRUE, further sends will be disallowed.
  $LF
  if _what_ is ommited or $UNDEF, further sends and receives will be disallowed
**/
DEFINE_FUNCTION( Shutdown ) { // arg[0] =  false: SHUTDOWN_RCV | true: SHUTDOWN_SEND | else it will SHUTDOWN_BOTH

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();

	PRFileDesc *fd = (PRFileDesc*)JL_GetPrivate( cx, obj );
	JL_ASSERT_THIS_OBJECT_STATE( fd );

	PRShutdownHow how;
	if ( JL_ARG_ISDEF(1) )
		if ( JL_ARG(1) == JSVAL_FALSE )
			how = PR_SHUTDOWN_RCV;
		else if (JL_ARG(1) == JSVAL_TRUE )
			how = PR_SHUTDOWN_SEND;
		else
			JL_ERR( E_ARG, E_NUM(1), E_INVALID );
	else
		how = PR_SHUTDOWN_BOTH; // default

	if ( how == PR_SHUTDOWN_RCV )
		JL_CHK( SetStreamReadInterface(cx, obj, NULL) );

	if (PR_Shutdown( fd, how ) != PR_SUCCESS) // is this compatible with linger ?? need to check PR_WOULD_BLOCK_ERROR ???
		return ThrowIoError(cx);

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME( [port] [, ip] )
  When a new socket is created, it has no address bound to it.
  Bind assigns the specified address (also known as name) to the socket.
  _ip_ is the address (interface) to which the socket will be bound.
  If _address_ is ommited, any address is will match.
  If _port_ is $UNDEF, the socket is not bind to any port.
  $H return value
   $FALSE if the address is already is in use otherwise $TRUE.
  $H example 1
  {{{
  var server = new Socket();
  server.Bind(8099, '127.0.0.1');
  server.Listen();
  }}}
  $H example 2
  {{{
  var client = new Socket();
  client.Bind(0, '192.168.0.1');
  client.Connect('127.0.0.1', 8099);
  }}}
**/
DEFINE_FUNCTION( Bind ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();

	JL_ASSERT_ARGC_MIN( 1 ); // need port number (at least)

	PRFileDesc *fd;
	fd = (PRFileDesc*)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( fd );

	PRNetAddr addr;
	PRUint16 port;
	if ( JL_ARG_ISDEF(1) ) {

		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &port) );
	} else {

		port = 0; // doc. If you do not care about the TCP/UDP port assigned to the socket, set the inet.port field of PRNetAddr to 0.
	}

	if ( JL_ARG_ISDEF(2) ) { // if we have a second argument and this argument is not undefined

		JLStr host;
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &host) );

		if ( PR_StringToNetAddr(host, &addr) != PR_SUCCESS )
			return ThrowIoError(cx);

		if ( PR_InitializeNetAddr(PR_IpAddrNull, port, &addr) != PR_SUCCESS )
			return ThrowIoError(cx);
	} else {

		if ( PR_InitializeNetAddr(PR_IpAddrAny, port, &addr) != PR_SUCCESS )
			return ThrowIoError(cx);
	}

	if ( PR_Bind(fd, &addr) != PR_SUCCESS ) {

		if ( PR_GetError() == PR_ADDRESS_IN_USE_ERROR ) { // do not failed but return false

			*JL_RVAL = JSVAL_FALSE;
			return JS_TRUE;
		}
		return ThrowIoError(cx);
	}

	*JL_RVAL = JSVAL_TRUE; // no error, return true
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( [ backlogSize = 8 ] )
   Listen for connections on a socket.
   _backlogSize_ specifies the maximum length of the queue of pending connections.
**/

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

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();

	PRFileDesc *fd = (PRFileDesc*)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( fd );
	PRIntn backlog;
	if ( JL_ARG_ISDEF(1) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &backlog) );
	else
		backlog = 8; // too low ??

	if ( PR_Listen(fd, backlog) != PR_SUCCESS )
		return ThrowIoError(cx);

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE Socket $INAME()
  Accept a connection on a socket.
  This function returns a connected jsio::Socket.
  Use _timeout_ property to manage time limit for completion of this operation.
**/
DEFINE_FUNCTION( Accept ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARG_COUNT(0);

	PRFileDesc *fd = (PRFileDesc*)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( fd );

/*
	PRIntervalTime timeout;
	if ( JL_ARG_ISDEF(1) ) {

		PRUint32 timeoutInMilliseconds;
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &timeoutInMilliseconds) );
		timeout = PR_MillisecondsToInterval(timeoutInMilliseconds);
	} else {

		timeout = PR_INTERVAL_NO_TIMEOUT;
	}
*/

	PRIntervalTime timeout;
	JL_CHK( GetTimeoutInterval(cx, obj, &timeout) );

	PRFileDesc *newFd;
	newFd = PR_Accept(fd, NULL, timeout);
	if ( newFd == NULL )
		return ThrowIoError(cx);

	JSObject *object;
	object = JS_NewObjectWithGivenProto(cx, JL_CLASS(Socket), JL_PROTOTYPE(cx, Socket), NULL);
	JL_SetPrivate(cx, object, newFd);
//	JL_CHK( JL_SetReservedSlot(cx, descriptorObject, SLOT_JSIO_DESCRIPTOR_IMPORTED, JSVAL_FALSE) );

	// JL_CHK( ReserveStreamReadInterface(cx, object) );
	JL_CHK( SetStreamReadInterface(cx, object, NativeInterfaceStreamRead) );

	*JL_RVAL = OBJECT_TO_JSVAL( object );

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME( host, port )
  Initiate a connection on a socket.
  Use _timeout_ property to manage time limit for completion of this operation.
  $H return value
   $FALSE if the connection times out $TRUE, otherwise $TRUE.
**/

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

	JLStr host;
	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARG_COUNT(2);

	PRFileDesc *fd;
	fd = (PRFileDesc*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( fd );

	PRUint16 port;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &port) );

/*
	PRIntervalTime connectTimeout;
	if ( JL_ARG_ISDEF(3) ) {

		PRUint32 timeoutInMilliseconds;
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &timeoutInMilliseconds) );
		connectTimeout = PR_MillisecondsToInterval(timeoutInMilliseconds);
	} else
		connectTimeout = PR_INTERVAL_NO_TIMEOUT;
*/

//	const char *host;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &host) );

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

	PRIntervalTime timeout;
	JL_CHK( GetTimeoutInterval(cx, obj, &timeout) );

	if ( PR_Connect(fd, &addr, timeout) != PR_SUCCESS ) { // Doc: timeout is ignored in nonblocking mode ( cf. PR_INTERVAL_NO_WAIT )

		switch ( PR_GetError() ) {
			case PR_CONNECT_TIMEOUT_ERROR:
			case PR_IO_TIMEOUT_ERROR:
				*JL_RVAL = JSVAL_FALSE;
				return JS_TRUE;
			case PR_IN_PROGRESS_ERROR: // After a nonblocking connect is initiated with PR_Connect() (which fails with PR_IN_PROGRESS_ERROR), ...see connectContinue
				break;
			default:
				return ThrowIoError(cx);
		}
	}
	// see 	PR_GetConnectStatus or PR_ConnectContinue INSTEAD ???

	JL_CHK( SetStreamReadInterface(cx, obj, NativeInterfaceStreamRead) );
	*JL_RVAL = JSVAL_TRUE;
	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $STR $INAME( host, port, string )
  Send a specified number of bytes from an unconnected socket.
  See. Static functions.
**/
DEFINE_FUNCTION( SendTo ) {

	JLStr host, str;
	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();

	JL_ASSERT_ARGC_MIN( 3 );

	PRFileDesc *fd;
	if ( JL_GetClass(obj) == JL_THIS_CLASS ) {
		
		fd = (PRFileDesc*)JL_GetPrivate(cx, obj);
		JL_ASSERT_THIS_OBJECT_STATE( fd );
	} else {

		fd = PR_NewUDPSocket(); // allow to use SendTo as static function
		if ( fd == NULL )
			return ThrowIoError(cx);
	}

	PRUint16 port;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &port) );

//	const char *host;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &host) );

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

//	const char *str;
//	size_t len;
//	JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(3), &str, &len) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &str) );
	ASSERT( str.Length() <= PR_INT32_MAX );

	PRInt32 res;
	res = PR_SendTo(fd, str.GetConstStr(), (PRInt32)str.Length(), 0, &addr, PR_INTERVAL_NO_TIMEOUT );

	size_t sentAmount;
	if ( res == -1 ) {

		PRErrorCode errCode = PR_GetError();
		if ( errCode != PR_WOULD_BLOCK_ERROR )
			return ThrowIoError(cx);
		sentAmount = 0;
	} else
		sentAmount = res;

	if ( sentAmount < str.Length() )
		*JL_RVAL = STRING_TO_JSVAL( JS_NewDependentString(cx, JSVAL_TO_STRING( JL_ARG(3) ), sentAmount, str.Length() - sentAmount) ); // return unsent data
	else if ( sentAmount == 0 )
		*JL_RVAL = JL_ARG(3); // nothing has been sent
	else
		*JL_RVAL = JL_GetEmptyStringValue(cx); // nothing remains

	if ( JL_GetClass(obj) != JL_THIS_CLASS )
		PR_Close(fd);

	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $STR $INAME()
  Receive all data from socket which may or may not be connected.
  See. Static functions.
**/
DEFINE_FUNCTION( RecvFrom ) {

	char *buffer = NULL;

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();


//	JL_ASSERT_INSTANCE( obj, _class );

	PRFileDesc *fd;
	if ( JL_GetClass(obj) == JL_THIS_CLASS ) {

		fd = (PRFileDesc*)JL_GetPrivate(cx, obj);
		JL_ASSERT_THIS_OBJECT_STATE( fd );
	} else {

		fd = PR_NewUDPSocket(); // allow to use RecvFrom as static function // 		//JL_CHKM( fd, "Unable to create the UDP socket." );
		if ( fd == NULL )
			return ThrowIoError(cx);
	}

	PRInt64 available;
	available = PR_Available64( fd );
	if ( available == -1 )
		return ThrowIoError(cx);

	buffer = (char *)JS_malloc(cx, (size_t)available +1); // (TBD) optimize this if  available == 0 !!
	JL_CHK( buffer );

	PRNetAddr addr;
	PRInt32 res;
	res = PR_RecvFrom(fd, buffer, (PRInt32)available, 0, &addr, PR_INTERVAL_NO_TIMEOUT);
	if (unlikely( res == -1 )) {
		goto bad_ex;
/*
		JS_free(cx, buffer);
			return ThrowIoError(cx);
		tmp = JL_GetEmptyStringValue(cx);
		JL_CHK( JS_SetElement(cx, arrayObject, 0, &tmp));
*/
	}

	char peerName[47]; // If addr is an IPv4 address, size needs to be at least 16. If addr is an IPv6 address, size needs to be at least 46.

	JSObject *arrayObject;
	arrayObject = JS_NewArrayObject(cx, 3, NULL);
	JL_CHK( arrayObject ); // (TBD) else free buffer
	*JL_RVAL = OBJECT_TO_JSVAL( arrayObject );

	JL_CHKB( PR_NetAddrToString(&addr, peerName, sizeof(peerName)) == PR_SUCCESS, bad_ex ); // Converts a character string to a network address.

	jsval tmp;
	JL_CHK( JL_NativeToJsval(cx, peerName, &tmp) );
	JL_CHK( JS_SetElement(cx, arrayObject, 1, &tmp) );

	PRUint16 port;
	port = PR_NetAddrInetPort(&addr);

	JL_CHK( JL_NativeToJsval(cx, port, &tmp) );
	JL_CHK( JS_SetElement(cx, arrayObject, 2, &tmp) );

	if (likely( res > 0 )) {

		buffer[res] = '\0';
		JL_CHK( JL_NewBlob( cx, buffer, res, &tmp ) );
		JL_CHK( JS_SetElement(cx, arrayObject, 0, &tmp) );
		return JS_TRUE;
	} else
	if ( res == 0 ) {

		JS_free(cx, buffer);
		tmp = JSVAL_VOID;
		JL_CHK( JS_SetElement(cx, arrayObject, 0, &tmp) );
	}

	return JS_TRUE;

bad_ex:
	ThrowIoError(cx);
bad:
	JS_free(cx, buffer); // JS_free NULL pointer is legal.
	return JS_FALSE;
}



/**doc
$TOC_MEMBER $INAME
 $INT $INAME( fileDescriptor [, close [, headers]] )
  Sends a complete file pointed by _fileDescriptor_ across a socket.
  Use _timeout_ property to manage time limit for completion of this operation.
  $LF
  _headers_ is a string that contains the headers to send across the socket prior to sending the file.
  $LF
  Optionally, _close_ flag specifies that transmitfile should close the socket after sending the data.
  $LF
  _timeout_ is the time limit for completion of the transmit operation.
  $H return value
   The number of bytes successfully written, including both the headers and the file. 
  $H note
   This function only works with blocking sockets.
**/
DEFINE_FUNCTION( TransmitFile ) { // WORKS ONLY ON BLOCKING SOCKET !!!

	JLStr headers;
	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_RANGE(1,3);

	PRFileDesc *socketFd;
	socketFd = (PRFileDesc *)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( socketFd );

	JL_ASSERT_ARG_IS_OBJECT(1);
	JSObject *fileObj;
	fileObj = JSVAL_TO_OBJECT( JL_ARG(1) );
	JL_ASSERT_INSTANCE( fileObj, JL_CLASS(File) );
	PRFileDesc *fileFd;
	fileFd = (PRFileDesc*)JL_GetPrivate( cx, fileObj );
	JL_ASSERT_OBJECT_STATE( fileFd, JL_CLASS_NAME(File) );

	PRTransmitFileFlags flag;
	flag = PR_TRANSMITFILE_KEEP_OPEN;
	if ( JL_ARG_ISDEF(2) ) {

		bool closeAfterTransmit;
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &closeAfterTransmit) );
		if ( closeAfterTransmit )
			flag = PR_TRANSMITFILE_CLOSE_SOCKET;
	}

/*
	PRIntervalTime connectTimeout;
	if ( JL_ARG_ISDEF(4) ) {

		PRUint32 timeoutInMilliseconds;
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(4), &timeoutInMilliseconds) );
		connectTimeout = PR_MillisecondsToInterval(timeoutInMilliseconds);
	} else
		connectTimeout = PR_INTERVAL_NO_TIMEOUT;
*/

//	const char *headers;
//	headers = NULL;
//	size_t headerLength;
	if ( JL_ARG_ISDEF(3) ) {

//		JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(3), &headers, &headerLength) );
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &headers) );
		ASSERT( headers.Length() <= PR_INT32_MAX );
	}

	PRIntervalTime timeout;
	JL_CHK( GetTimeoutInterval(cx, JL_OBJ, &timeout) );

	PRInt32 bytes;
	bytes = PR_TransmitFile(socketFd, fileFd, headers.GetStrConstOrNull(), (PRInt32)headers.LengthOrZero(), flag, timeout);
	if ( bytes == -1 )
		return ThrowIoError(cx);

	//JL_CHK( JL_NewNumberValue(cx, bytes, JL_RVAL) );
	JL_CHK( JL_NativeToJsval(cx, bytes, JL_RVAL) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
=== Properties ===
**/


/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Test if a nonblocking connect has completed.
  Is $TRUE if the socket is connected.
  $LF
  Is $UNDEF if the operation is still in progress.
  $LF
  Is $FALSE the nonblocking connect has failed.
**/

// http://developer.mozilla.org/en/docs/PR_GetConnectStatus
// After PR_Connect on a nonblocking socket fails with PR_IN_PROGRESS_ERROR,
// you may wait for the connection to complete by calling PR_Poll on the socket with the in_flags PR_POLL_WRITE | PR_POLL_EXCEPT.
// When PR_Poll returns, call PR_GetConnectStatus on the socket to determine whether the nonblocking connect has succeeded or failed.
DEFINE_PROPERTY_GETTER( connectContinue ) {

	PRFileDesc *fd = (PRFileDesc *)JL_GetPrivate( cx, obj );
	JL_ASSERT_THIS_OBJECT_STATE( fd );

	PRPollDesc desc;
	desc.fd = fd;
	desc.in_flags = PR_POLL_WRITE | PR_POLL_EXCEPT;
	desc.out_flags = 0;

	PRInt32 result;
	result = PR_Poll( &desc, 1, PR_INTERVAL_NO_WAIT ); // this avoid to store out_flags from the previous poll
	if ( result == -1 )
		return ThrowIoError(cx);

	JL_ASSERT( result != 0, E_THISOBJ, E_STATE );

	// this can help ? : http://lxr.mozilla.org/seamonkey/search?string=PR_ConnectContinue
	// source: http://lxr.mozilla.org/seamonkey/source/nsprpub/pr/src/io/prsocket.c#287
	// example: http://www.google.com/codesearch?hl=en&q=+PR_ConnectContinue+show:Su0oyYj9cVc:HRv_2Hg8Bm0:u0BRTcINCf8&sa=N&cd=5&ct=rc&cs_p=http://archive.mozilla.org/pub/mozilla.org/mozilla/releases/mozilla1.1a/src/mozilla-source-1.1a.tar.bz2&cs_f=mozilla/netwerk/base/src/nsSocketTransport.cpp#l926

	if ( PR_ConnectContinue(fd, desc.out_flags) == PR_SUCCESS ) { // If the nonblocking connect has successfully completed, PR_ConnectContinue returns PR_SUCCESS

		*vp = JSVAL_TRUE; // We are connected.
		return JS_TRUE;
	}

	if ( PR_GetError() == PR_IN_PROGRESS_ERROR ) {

		*vp = JSVAL_VOID; // Operation is still in progress
		return JS_TRUE;
	}
	// else, the nonblocking connect has failed with this error code.
	// (TBD) check for PR_CONNECT_REFUSED_ERROR error ?
	*vp = JSVAL_FALSE; // Connection refused, ...

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Check if the socket connection is closed.
**/
DEFINE_PROPERTY_GETTER( connectionClosed ) {

	PRFileDesc *fd = (PRFileDesc *)JL_GetPrivate( cx, obj );
	JL_ASSERT_THIS_OBJECT_STATE( fd );

	//If PR_Poll() reports that the socket is readable (i.e., PR_POLL_READ is set in out_flags),
	//and PR_Available() returns 0, this means that the socket connection is closed.
	//see http://www.mozilla.org/projects/nspr/tech-notes/nonblockingio.html

	// socket is readable ?
	PRPollDesc desc;
	desc.fd = fd;
	desc.in_flags = PR_POLL_READ;
	desc.out_flags = 0;
	PRInt32 result;
	result = PR_Poll( &desc, 1, PR_INTERVAL_NO_WAIT );
	if ( result == -1 ) // error
		return ThrowIoError(cx);
	if ( result == 1 && (desc.out_flags & PR_POLL_READ) ) {

		PRInt32 available;
		available = PR_Available( fd );
		if ( available == -1 )
			return ThrowIoError(cx);
		if ( available == 0 ) {

			*vp = JSVAL_TRUE; // socket is closed
			return JS_TRUE;
		}
	}

	*vp = JSVAL_FALSE;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER linger
 $INT *linger*
  The time in milliseconds to linger on close if data present.
  A value of zero means no linger.

$TOC_MEMBER noDelay
 $BOOL *noDelay*
  Don't delay send to coalesce packets.

$TOC_MEMBER reuseAddr
 $BOOL *reuseAddr*
  Allow local address reuse.

$TOC_MEMBER keepAlive
 $BOOL *keepAlive*
  Keep connections alive.

$TOC_MEMBER recvBufferSize
 $INT *recvBufferSize*
  Receive buffer size.

$TOC_MEMBER sendBufferSize
 $INT *sendBufferSize*
  Send buffer size.

$TOC_MEMBER maxSegment
 $INT *maxSegment*
  Maximum segment size.

$TOC_MEMBER nonblocking
 $BOOL *nonblocking*
  Non-blocking (network) I/O.

$TOC_MEMBER broadcast
 $BOOL *broadcast*
  Enable broadcast.

$TOC_MEMBER multicastLoopback
 $BOOL *multicastLoopback*
  IP multicast loopback.
**/

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

DEFINE_PROPERTY_SETTER( Option ) {

	PRFileDesc *fd = (PRFileDesc *)JL_GetPrivate( cx, obj );
	JL_ASSERT_THIS_OBJECT_STATE( fd );

	PRSocketOptionData sod;
	sod.option = (PRSockOption)JSID_TO_INT( id );

	switch ( sod.option ) {
		case PR_SockOpt_Linger: { // http://developer.mozilla.org/en/docs/PRLinger
				unsigned int timeout;
				JL_CHK( JL_JsvalToNative(cx, *vp, &timeout) );
//				JS_ValueToECMAUint32( cx, *vp, &timeout );
				if ( timeout > 0 ) {
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
		default:;
	}
	if ( PR_SetSocketOption(fd, &sod) != PR_SUCCESS )
		return ThrowIoError(cx);
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( Option ) {

	PRFileDesc *fd = (PRFileDesc *)JL_GetPrivate( cx, obj );
	JL_ASSERT_THIS_OBJECT_STATE( fd );

	PRSocketOptionData sod;
	sod.option = (PRSockOption)JSID_TO_INT( id );
	if ( PR_GetSocketOption(fd, &sod) != PR_SUCCESS )
		return ThrowIoError(cx);
	switch ( sod.option ) {
		case PR_SockOpt_Linger:
			if ( sod.value.linger.polarity == PR_TRUE )
				JL_CHK( JL_NativeToJsval( cx, PR_IntervalToMilliseconds(sod.value.linger.linger), vp ) );
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
//			*vp = INT_TO_JSVAL(sod.value.recv_buffer_size);
			JL_CHK( JL_NativeToJsval(cx, sod.value.recv_buffer_size, vp) );
			break;
		case PR_SockOpt_SendBufferSize:
//			*vp = INT_TO_JSVAL((int)sod.value.send_buffer_size);
			JL_CHK( JL_NativeToJsval(cx, sod.value.send_buffer_size, vp) );
			break;
		case PR_SockOpt_MaxSegment:
//			*vp = INT_TO_JSVAL(sod.value.max_segment);
			JL_CHK( JL_NativeToJsval(cx, sod.value.max_segment, vp) );
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
		default:;
	}
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME $READONLY
  Get name of the connected peer.
  Return the network address for the connected peer socket or $UNDEF if the socket is not connected
**/
DEFINE_PROPERTY_GETTER( peerName ) {

	PRFileDesc *fd = (PRFileDesc *)JL_GetPrivate( cx, obj );
	JL_ASSERT_THIS_OBJECT_STATE( fd );
	PRNetAddr peerAddr;
	if ( PR_GetPeerName(fd, &peerAddr) != PR_SUCCESS ) {
		
		if ( PR_GetError() == PR_NOT_CONNECTED_ERROR ) {

			*vp = JSVAL_VOID;
			return JS_TRUE;
		}
		return ThrowIoError(cx);
	}
	char buf[MAX_IP_STRING + 1];
	if ( PR_NetAddrToString(&peerAddr, buf, sizeof(buf)) != PR_SUCCESS )
		return ThrowIoError(cx);
	*vp = STRING_TO_JSVAL( JS_NewStringCopyZ( cx, buf ) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  Get port of the connected peer.
  Return the port for the connected peer socket or $UNDEF if the socket is not connected.
**/
DEFINE_PROPERTY_GETTER( peerPort ) {

	PRFileDesc *fd = (PRFileDesc *)JL_GetPrivate( cx, obj );
	JL_ASSERT_THIS_OBJECT_STATE( fd );
	PRNetAddr peerAddr;
	if ( PR_GetPeerName(fd, &peerAddr) != PR_SUCCESS ) {

		if ( PR_GetError() == PR_NOT_CONNECTED_ERROR ) {

			*vp = JSVAL_VOID;
			return JS_TRUE;
		}
		return ThrowIoError(cx);
	}
	*vp = INT_TO_JSVAL( PR_ntohs(PR_NetAddrInetPort(&peerAddr)) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $STR $INAME $READONLY
  Get socket name.
  Return the network address for this socket.
**/
DEFINE_PROPERTY_GETTER( sockName ) {

	PRFileDesc *fd = (PRFileDesc *)JL_GetPrivate( cx, obj );
	JL_ASSERT_THIS_OBJECT_STATE( fd );
	PRNetAddr sockAddr;
	if ( PR_GetSockName( fd, &sockAddr ) != PR_SUCCESS )
		return ThrowIoError(cx);
	char buf[MAX_IP_STRING + 1];
	if ( PR_NetAddrToString( &sockAddr, buf, sizeof(buf) ) != PR_SUCCESS )
		return ThrowIoError(cx);
	*vp = STRING_TO_JSVAL( JS_NewStringCopyZ( cx, buf ) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  Get socket port.
  Return the port for this socket.
**/
DEFINE_PROPERTY_GETTER( sockPort ) {

	PRFileDesc *fd = (PRFileDesc *)JL_GetPrivate( cx, obj );
	JL_ASSERT_THIS_OBJECT_STATE( fd );
	PRNetAddr sockAddr;
	if ( PR_GetSockName( fd, &sockAddr ) != PR_SUCCESS )
		return ThrowIoError(cx);
	*vp = INT_TO_JSVAL( PR_ntohs(PR_NetAddrInetPort(&sockAddr)) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
=== Static functions ===
**/

/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME( hostName )
  Lookup a host by name (DNS lookup) and returns the results in a javascript array.
  {{{
  Print( GetHostsByName('localhost')[0] ); // prints: 127.0.0.1
  }}}
**/
DEFINE_FUNCTION( GetHostsByName ) {

	JLStr host;
	JL_ASSERT_ARGC_MIN( 1 );

	char netdbBuf[PR_NETDB_BUF_SIZE];
	PRHostEnt hostEntry;
	PRNetAddr addr;

	JSObject *addrJsObj;
	addrJsObj = JS_NewArrayObject(cx, 0, NULL);
	JL_CHK( addrJsObj );
	*JL_RVAL = OBJECT_TO_JSVAL(addrJsObj);

	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &host) );

	if ( PR_GetHostByName( host, netdbBuf, sizeof(netdbBuf), &hostEntry ) != PR_SUCCESS ) {

		if ( PR_GetError() == PR_DIRECTORY_LOOKUP_ERROR )
			return JS_TRUE;
		goto bad_throw;
	}

	int index;
	index = 0;
	PRIntn hostIndex;
	hostIndex = 0;
	char addrStr[MAX_IP_STRING + 1];

	for (;;) {

		hostIndex = PR_EnumerateHostEnt(hostIndex, &hostEntry, 0, &addr);
		if ( hostIndex == 0 )
			break;
		JL_CHKB( hostIndex != -1, bad_throw );
		JL_CHKB( PR_NetAddrToString(&addr, addrStr, sizeof(addrStr)) == PR_SUCCESS, bad_throw ); // memory leak
		jsval tmp;
		JL_CHK( JL_NativeToJsval(cx, addrStr, &tmp) );
		JL_CHK( JS_DefineElement(cx, addrJsObj, index++, tmp, NULL, NULL, JSPROP_ENUMERATE) );
	}
	return JS_TRUE;

bad_throw:
	ThrowIoError(cx);
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME( hostAddr )
  Lookup a name by host (reverse DNS lookup) and returns the results in a javascript array.
  $H example 1
{{{
 Print( GetHostsByAddr('127.0.0.1')[0] ); // prints: localhost
}}}
  $H example 2
{{{
 function ReverseLookup( ip ) {
  try {
   
   return Socket.GetHostsByAddr(ip)[0];
  } catch ( ex if ex instanceof IoError ) {
   
   return undefined; // not found
  }
}
}}}
**/
DEFINE_FUNCTION( GetHostsByAddr ) {

	JLStr addr;
	JL_ASSERT_ARG_COUNT( 1 );

	//const char *addr; // MAX_IP_STRING
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &addr) );

	PRNetAddr netaddr;
	if ( PR_StringToNetAddr(addr, &netaddr) != PR_SUCCESS )
		return ThrowIoError(cx);

	char buffer[PR_NETDB_BUF_SIZE * 2];

	PRHostEnt hostent;
	if ( PR_GetHostByAddr(&netaddr, buffer, sizeof(buffer), &hostent) != PR_SUCCESS )
		return ThrowIoError(cx);

	JSObject *hostJsObj;
	hostJsObj = JS_NewArrayObject(cx, 0, NULL);
	JL_CHK( hostJsObj );
	*JL_RVAL = OBJECT_TO_JSVAL(hostJsObj);

	int index;
	index = 0;

	jsval tmp;

	JL_CHK( JL_NativeToJsval(cx, hostent.h_name, &tmp) );
	JL_CHK( JS_DefineElement(cx, hostJsObj, index++, tmp, NULL, NULL, JSPROP_ENUMERATE) );
	
	if ( hostent.h_aliases == NULL )
		return JS_TRUE;

	for ( int i = 0; hostent.h_aliases[i]; ++i ) {

		JL_CHK( JL_NativeToJsval(cx, hostent.h_aliases[i], &tmp) );
		JL_CHK( JS_DefineElement(cx, hostJsObj, index++, tmp, NULL, NULL, JSPROP_ENUMERATE) );
	}

	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER SendTo
 SendTo
  see Socket::SendTo

$TOC_MEMBER RecvFrom
 RecvFrom
  see Socket::RecvFrom
**/


/**doc
=== Constants ===
**/

/**doc
 $CONST TCP

 $CONST UDP

**/


/**doc
=== Native Interface ===
 * *NIStreamRead*
**/

/**doc
=== Intresting lecture ===
 # [http://www.ibm.com/developerworks/linux/library/l-sockpit/ Five pitfalls of Linux sockets programming]
**/


CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_PROTOTYPE( Descriptor )

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	HAS_PRIVATE
	HAS_RESERVED_SLOTS( 2 ) // SLOT_JSIO_DESCRIPTOR_IMPORTED, SLOT_JSIO_DESCRIPTOR_TIMEOUT

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
		PROPERTY_GETTER( peerName )
		PROPERTY_GETTER( peerPort )
		PROPERTY_GETTER( sockName )
		PROPERTY_GETTER( sockPort )
		PROPERTY_GETTER( connectContinue )
		PROPERTY_GETTER( connectionClosed )
	END_PROPERTY_SPEC

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION( SendTo )
		FUNCTION( RecvFrom )
		FUNCTION( GetHostsByName )
		FUNCTION_ARGC( GetHostsByAddr, 1 )
	END_STATIC_FUNCTION_SPEC

	BEGIN_CONST_INTEGER_SPEC
		CONST_INTEGER(TCP, PR_DESC_SOCKET_TCP)
		CONST_INTEGER(UDP, PR_DESC_SOCKET_UDP)
	END_CONST_INTEGER_SPEC

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
