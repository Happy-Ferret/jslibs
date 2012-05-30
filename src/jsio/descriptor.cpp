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
DECLARE_CLASS( Socket )


JSBool InitPollDesc( JSContext *cx, jsval descVal, PRPollDesc *pollDesc );
JSBool PollDescNotify( JSContext *cx, jsval descVal, PRPollDesc *pollDesc, int index );


JSBool NativeInterfaceStreamRead( JSContext *cx, JSObject *obj, char *buf, size_t *amount ) {

	JL_ASSERT_INHERITANCE(obj, JL_CLASS(Descriptor));
	JL_ASSERT( *amount <= PR_INT32_MAX );

	PRInt32 res;
	PRFileDesc *fd;
	fd = (PRFileDesc*)JL_GetPrivate(obj); // (PRFileDesc *)pv;
	JL_ASSERT_OBJECT_STATE(fd, JL_CLASS_NAME(Descriptor));

	PRPollDesc desc;
	desc.fd = fd;
	desc.in_flags = PR_POLL_READ;
	desc.out_flags = 0;

	// PR_Poll works with non-blocking sockets.  In fact, PR_Poll is intended to be used with non-blocking sockets.
	// PR_Poll blocks until the events you're interested in have occurred.
	// The fact that the socket is in non-blocking mode affects PR_Recv, PR_Send, etc., but doesn't affect PR_Poll.

	PRIntervalTime timeout;
	JL_CHK( GetTimeoutInterval(cx, obj, &timeout) );

	res = PR_Poll(&desc, 1, timeout);
	if ( res == -1 ) // if PR_Poll is not compatible with the file descriptor, just ignore the error ?
		return ThrowIoError(cx);

	if ( res == 0 ) { // timeout, no data

		*amount = 0;
		return JS_TRUE; // no data, but it is not an error.
	}

	// PR_Read doc:
	// * a positive number indicates the number of bytes actually read in.
	// * 0 means end of file is reached or the network connection is closed.
	// * -1 indicates a failure. The reason for the failure is obtained by calling PR_GetError().
	res = PR_Read(fd, buf, (PRInt32)*amount); // like recv() with PR_INTERVAL_NO_TIMEOUT
	if ( res == -1 ) {

		PRErrorCode err = PR_GetError();
		switch ( err ) { // see Write() for details about errors

			// try to catch runtime errors
			case PR_WOULD_BLOCK_ERROR: // The operation would have blocked
			case PR_SOCKET_SHUTDOWN_ERROR: // Socket shutdown
			case PR_CONNECT_ABORTED_ERROR: // Connection aborted
			case PR_CONNECT_RESET_ERROR: // TCP connection reset by peer
			case PR_CONNECT_TIMEOUT_ERROR: // Connection attempt timed out
				*amount = 0;
				return JS_TRUE;
			default:
				// try to catch logic errors
				return ThrowIoError(cx);
		}
	}

	*amount = res;
	return JS_TRUE;
	JL_BAD;
}



void FinalizeDescriptor(JSFreeOp *, JSObject *obj) {

	PRFileDesc *fd = (PRFileDesc*)JL_GetPrivate( obj );
	if ( !fd ) // check if not already closed
		return;

	jsval imported;
	JL_CHK( JL_GetReservedSlot( obj, SLOT_JSIO_DESCRIPTOR_IMPORTED, &imported) );
	if ( imported == JSVAL_TRUE ) // Descriptor was inported, then do not close it
		return;

	PRStatus status;
	status = PR_Close(fd); // what to do on error ??
	if ( status != PR_SUCCESS ) {

		if ( PR_GetError() != PR_WOULD_BLOCK_ERROR ) { // if non-blocking descriptor, this is a non-fatal error

			//JL_WARN( E_NAME(JL_CLASS_NAME(Descriptor)), E_FIN ); // "A descriptor cannot be closed while Finalize." // (TBD) send to log !
		}
	}

bad:
	return;
}


/**doc fileIndex:top
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( Descriptor )

/**doc
=== Methods ===
**/

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Close the descriptor.
  Using this function on a socket may cause an abortive shutdown (as opposed to a gracefully shutdown).
**/
DEFINE_FUNCTION( close ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INHERITANCE();
	JL_ASSERT_ARGC(0);

	*JL_RVAL = JSVAL_VOID;

	PRFileDesc *fd;
	fd = (PRFileDesc*)JL_GetPrivate(obj);

	JL_ASSERT_WARN( fd, E_NAME(JL_THIS_CLASS_NAME), E_CLOSED ); // see PublicApiRules (http://code.google.com/p/jslibs/wiki/PublicApiRules)
	if ( !fd )
		return JS_TRUE;

	PRStatus status;
	status = PR_Close(fd);
	if ( status != PR_SUCCESS ) {

		if ( PR_GetError() != PR_WOULD_BLOCK_ERROR ) // if non-blocking descriptor, this is a non-fatal error
			return ThrowIoError(cx);
	}
	JL_SetPrivate( obj, NULL);
	JL_CHK( SetStreamReadInterface(cx, obj, NULL) );

	return JS_TRUE;
	JL_BAD;
}



/*
void* JSBufferAlloc(void * opaqueAllocatorContext, size_t size) {

	return JS_malloc((JSContext*)opaqueAllocatorContext, size);
}

void JSBufferFree(void * opaqueAllocatorContext, void* address) {

	JS_free((JSContext*)opaqueAllocatorContext, address);
}

void* JSBufferRealloc(void * opaqueAllocatorContext, void* address, size_t size) {

	return JS_realloc((JSContext*)opaqueAllocatorContext, address, size);
}


JSBool ReadAllToJsval(JSContext *cx, PRFileDesc *fd, jsval *rval ) {

	jl::Buffer buf;
	BufferInitialize(&buf, jl::bufferTypeChunk, jl::bufferGrowTypeNoGuess, cx, JSBufferAlloc, JSBufferRealloc, JSBufferFree);
	PRInt32 currentReadLength = 1024;
	for (;;) {

		PRInt32 res = PR_Read(fd, BufferNewChunk(&buf, currentReadLength), currentReadLength); // like recv() with PR_INTERVAL_NO_TIMEOUT
		if (likely( res > 0 )) { // doc: a positive number indicates the number of bytes actually read in.

			BufferConfirm(&buf, res);
			if ( res < currentReadLength )
				break;
		} else
		if ( res == 0 ) { // doc: 0 means end of file is reached or the network connection is closed.

			if ( BufferGetLength(&buf) > 0 ) // we reach eof BUT we have read some data.
				break;
			ASSERT( currentReadLength > 0 ); // else it is not necessarily the end of the file.
			// no error, no data received, we cannot reach currentReadLength
			BufferFinalize(&buf);
			*rval = JSVAL_VOID;
			return JS_TRUE;
		} else
		if ( res == -1 ) { // doc: -1 indicates a failure. The reason for the failure can be obtained by calling PR_GetError.

			if ( BufferGetLength(&buf) > 0 ) // an error has occured, but we want to keep the current data, error will rise in the next call.
				break;

			switch ( PR_GetError() ) { // see Write() for details about errors
				case PR_WOULD_BLOCK_ERROR: // The operation would have blocked (non-fatal error)
					break;
				case PR_CONNECT_ABORTED_ERROR: // Connection aborted
//				case PR_SOCKET_SHUTDOWN_ERROR: // Socket shutdown
				case PR_CONNECT_RESET_ERROR: // TCP connection reset by peer

					if ( BufferGetLength(&buf) > 0 ) // even on connection failure, do not lost last reveived data.
						break; // for-loop

					BufferFinalize(&buf);
					*rval = JSVAL_VOID;
					return JS_TRUE;
				default:
					JL_CHK( ThrowIoError(cx) );
			}
			break; // no error, no data received, we cannot reach currentReadLength
		}

		if ( currentReadLength < 32768 && res == currentReadLength )
			currentReadLength *= 2;
	}

	if ( BufferGetLength(&buf) == 0 ) { // no data but NOT end of file/socket

		BufferFinalize(&buf);
		JL_CHK( JL_NewEmptyBuffer(cx, rval) );
		return JS_TRUE;
	}

	JL_CHK( JL_NewBufferGetOwnership(cx, BufferGetDataOwnership(&buf), BufferGetLength(&buf), rval) );

	BufferFinalize(&buf);
	return JS_TRUE;

bad:
	BufferFinalize(&buf);
	return JS_FALSE;
}
*/



/**doc
$TOC_MEMBER $INAME
 $VAL $INAME( [amount] )
  Read _amount_ bytes of data from the current descriptor. If _amount_ is ommited, the available data is read (not all the data).
  If the descriptor is exhausted (eof or disconnected), this function returns _undefined_.
  If the descriptor is a blocking socket and _amount_ is set to the value 0 value, the call blocks until data is available.
  See stringify() to read the whole content of a file descriptor.
  $H return
   This function may returns a ArrayBuffer or undefined.
  $H example
  {{{
  var loadModule = host.loadModule;
  loadModule('jsstd');
  loadModule('jsio');

  var soc = new Socket();
  soc.connect('www.google.com', 80);
  soc.write('GET\r\n\r\n');
  print( soc.read(undefined), '\n' );
  }}}
**/
DEFINE_FUNCTION( read ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INHERITANCE();

	PRFileDesc *fd;
	fd = (PRFileDesc*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( fd );

	uint32_t amount;
	PRInt32 res;

	if ( JL_ARG_ISDEF(1) ) {

		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &amount) );
	} else {

		// doc:
		//   Determine the amount of data in bytes available for reading in the given file or socket.
		//   Returns a -1 and the reason for the failure can be retrieved via PR_GetError().
		//   PR_Available64 works on normal files and sockets. PR_Available does not work with pipes on Win32 platforms.
		PRInt64 available;
		available = PR_Available64( fd );

		if ( available == 0 ) {

			res = PR_Read(fd, NULL, 0);
			ASSERT( res == 0 || res == -1 );
			available = PR_Available64( fd );
		}

		if ( available == 0 ) {

			amount = 1; // don't use 0 to avoid |res == amount == 0| case and wrongly return empty instead of undefined
		} else
		if ( available == -1 ) { // API not available

			ASSERT( PR_GetError() == PR_NOT_IMPLEMENTED_ERROR ); // (TBD) else handle errors properly
			amount = 4096;
		} else {

			amount = (uint32_t)available;
		}
	}


	uint8_t *buf;
	buf = JL_NewBuffer(cx, amount, JL_RVAL);
	JL_ASSERT_ALLOC( buf );

	//PRIntervalTime timeout;
	//GetTimeoutInterval(cx, JL_OBJ, &timeout);
	//res = PR_Recv(fd, buf, amount, 0, timeout);

	res = PR_Read(fd, buf, amount); // like recv() with PR_INTERVAL_NO_TIMEOUT

	if (unlikely( res == -1 )) { // failure. The reason for the failure can be obtained by calling PR_GetError.

		switch ( PR_GetError() ) { // see Write() for details about errors
			case PR_WOULD_BLOCK_ERROR:

				if ( amount > 0 )
					JL_CHK( JL_ChangeBufferLength(cx, JL_RVAL, 0) ); // mean no data available, but connection still established.
				return JS_TRUE;

			case PR_CONNECT_ABORTED_ERROR: // Connection aborted
				//Network dropped connection on reset.
				//  The connection has been broken due to keep-alive activity detecting a failure while the operation was in progress.
				//  It can also be returned by setsockopt if an attempt is made to set SO_KEEPALIVE on a connection that has already failed.
				//Software caused connection abort.
				//  An established connection was aborted by the software in your host computer, possibly due to a data transmission time-out or protocol error.
				//Connection timed out.
				//  A connection attempt failed because the connected party did not properly respond after a period of time,
				//  or the established connection failed because the connected host has failed to respond.

			//case PR_SOCKET_SHUTDOWN_ERROR: // Socket shutdown
				//Cannot send after socket shutdown.
				//  A request to send or receive data was disallowed because the socket had already been shut down in that direction with a previous shutdown call.
				//  By calling shutdown a partial close of a socket is requested, which is a signal that sending or receiving, or both have been discontinued.

			case PR_CONNECT_RESET_ERROR: // TCP connection reset by peer
				//Connection reset by peer.
				//  An existing connection was forcibly closed by the remote host.
				//  This normally results if the peer application on the remote host is suddenly stopped, the host is rebooted,
				//  the host or remote network interface is disabled, or the remote host uses a hard close (see setsockopt for more information on the SO_LINGER option on the remote socket).
				//  This error may also result if a connection was broken due to keep-alive activity detecting a failure while one or more operations are in progress.
				//  Operations that were in progress fail with WSAENETRESET. Subsequent operations fail with WSAECONNRESET.
				JL_CHK( JL_FreeBuffer(cx, JL_RVAL) );
				*JL_RVAL = JSVAL_VOID;
				return JS_TRUE;

			default:
				return ThrowIoError(cx);
		}
	}

	if ( (uint32_t)res == amount ) { // also handle |res == amount == 0|

		return JS_TRUE;
	}

	if ( res == 0 ) { // doc: 0 means end of file is reached or the network connection is closed.

		JL_CHK( JL_FreeBuffer(cx, JL_RVAL) );
		*JL_RVAL = JSVAL_VOID;
		return JS_TRUE;
	}

	if ( res > 0 ) { // doc: a positive number indicates the number of bytes actually read in.

		buf = JL_ChangeBufferLength(cx, JL_RVAL, res);
		JL_CHK( buf );
		return JS_TRUE;
	}

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( data )
  If the whole data cannot be written, Write returns that have not be written.
  If the descriptor is disconnected (socket only), this function returns _undefined_.
**/
DEFINE_FUNCTION( write ) {

	JLData str;

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INHERITANCE();
	JL_ASSERT_ARGC( 1 );

	PRFileDesc *fd;
	fd = (PRFileDesc *)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( fd );
	size_t sentAmount;

	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &str) );

	ASSERT( str.Length() <= PR_INT32_MAX );

	PRInt32 res;
	res = PR_Write( fd, str.GetConstStr(), (PRInt32)str.Length() );
	if (unlikely( res == -1 )) {

		switch ( PR_GetError() ) {
			case PR_WOULD_BLOCK_ERROR: // The operation would have blocked.
				sentAmount = 0;
				break;
//			case PR_NOT_CONNECTED_ERROR: // Network file descriptor is not connected.

			case PR_CONNECT_ABORTED_ERROR: // Connection aborted
				//Berkeley description:
				//A connection abort was caused internal to your host machine. The software caused
				//a connection abort because there is no space on the socket’s queue and the socket
				// cannot receive further connections.
				//
				//WinSock description:
				//Partly the same as Berkeley. The error can occur when the local network system aborts
				//a connection. This would occur if WinSock aborts an established connection after data
				//retransmission fails  (receiver never acknowledges data sent on a datastream socket).
				//
				//TCP/IP scenario:
				//A connection will timeout if the local system doesn’t receive an (ACK)nowledgement for
				//data sent.  It would also timeout if a (FIN)ish TCP packet is not ACK’d
				//(and even if the FIN is ACK’d, it will eventually timeout if a FIN is not returned).
				// source: http://www.chilkatsoft.com/p/p_299.asp

//			case PR_SOCKET_SHUTDOWN_ERROR: // Socket shutdown
				// Cannot send after socket shutdown.
				// A request to send or receive data was disallowed because the socket had already been shut down in that direction with a previous shutdown call.
				// By calling shutdown a partial close of a socket is requested, which is a signal that sending or receiving, or both have been discontinued.
				// source: msdn, Winsock Error Codes

			case PR_CONNECT_RESET_ERROR: // TCP connection reset by peer
				// Connection reset by peer. An existing connection was forcibly closed by the remote host. This normally results if the peer application on the remote host is suddenly stopped,
				// the host is rebooted, or the remote host uses a hard close (see setsockopt for more information on the SO_LINGER option on the remote socket.) This error may also result if
				// a connection was broken due to keep-alive activity detecting a failure while one or more operations are in progress. Operations that were in progress fail with WSAENETRESET.
				// Subsequent operations fail with WSAECONNRESET.
				// source: msdn, Winsock Error Codes

				*JL_RVAL = JSVAL_VOID;
				return JS_TRUE;
			default:
				return ThrowIoError(cx);
		}
	} else {

		sentAmount = res;
	}

	ASSERT( sentAmount <= str.Length() );

	if (likely( sentAmount == str.Length() )) { // nothing remains

		JL_CHK( JL_NewEmptyBuffer(cx, JL_RVAL) );
	} else if ( sentAmount == 0 ) { // nothing has been sent

		if ( JSVAL_IS_STRING( JL_ARG(1) ) ) { // optimization (string are immutable)

			*JL_RVAL = JL_ARG(1);
		} else {

			JL_CHK( str.GetArrayBuffer(cx, JL_RVAL) );
		}
	} else { // return unsent data

		JL_CHK( JL_NewBufferCopyN(cx, str.GetConstStr() + sentAmount, str.Length() - sentAmount, JL_RVAL) );
	}

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Sync any buffered data for a fd to its backing device.
**/
DEFINE_FUNCTION( sync ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INHERITANCE();
	JL_ASSERT_ARGC(0);

	PRFileDesc *fd;
	fd = (PRFileDesc *)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( fd );
	JL_CHKB( PR_Sync(fd) == PR_SUCCESS, bad_ioerror );

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;

bad_ioerror:
	ThrowIoError(cx);
	JL_BAD;
}


/**doc
=== Properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Determine the amount of data in bytes available for reading on the descriptor.
**/
DEFINE_PROPERTY_GETTER( available ) {

	JL_IGNORE(id);

	PRFileDesc *fd;
	JL_ASSERT_THIS_INHERITANCE();

	fd = (PRFileDesc *)JL_GetPrivate( obj );
	JL_ASSERT_THIS_OBJECT_STATE( fd ); //	JL_ASSERT_THIS_INSTANCE();

	PRInt64 available;
	available = PR_Available64( fd ); // For a normal file, these are the bytes beyond the current file pointer.
	if ( available == -1 ) {
		// (TBD) understand when it is possible to return 'undefined' instead of throwing an exception
//		switch ( PR_GetError() ) {
//			case PR_NOT_IMPLEMENTED_ERROR:
//				*vp = JSVAL_VOID;
//				return JS_TRUE;
//		}
		return ThrowIoError(cx);
	}

	return JL_NativeToJsval(cx, available, vp);
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  see constants below.
**/
DEFINE_PROPERTY_GETTER( type ) {

	JL_IGNORE(id);

	JL_ASSERT_THIS_INHERITANCE();

	PRFileDesc *fd;
	fd = (PRFileDesc *)JL_GetPrivate( obj );
	JL_ASSERT_THIS_OBJECT_STATE( fd ); //	JL_ASSERT_THIS_INSTANCE();
	*vp = INT_TO_JSVAL( (int)PR_GetDescType(fd) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Is $TRUE if the file descriptor is closed.
  $H beware
   Do not confuse with disconnected.$LF eg. A socket descriptor can be open but disconnected.
**/
DEFINE_PROPERTY_GETTER( closed ) {

	JL_IGNORE(id);

	JL_ASSERT_THIS_INHERITANCE();

	PRFileDesc *fd;
	fd = (PRFileDesc *)JL_GetPrivate( obj );
	*vp = BOOLEAN_TO_JSVAL( fd == NULL );
	return JS_TRUE;
	JL_BAD;
}

/* *doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Is $TRUE if the end of the file has been reached or the socket has been disconnected.
**/
/*
// 					JL_CHK( JL_SetReservedSlot( JL_OBJ, SLOT_JSIO_DESCRIPTOR_EOF, JSVAL_TRUE) );
DEFINE_PROPERTY( eof ) {

	return JL_GetReservedSlot( obj, SLOT_JSIO_DESCRIPTOR_EOF, vp);
}
*/

/**doc
=== Static functions ===
**/

/**doc
$TOC_MEMBER $INAME
 $INAME( nativeDescriptor, type )
**/
DEFINE_FUNCTION( import ) {

	JL_ASSERT_ARGC_MIN(2);
	//int stdfd;
	//JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &stdfd) );

	PRInt32 osfd;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &osfd) );

	//switch (stdfd) {
	//	case 0:
	//		osfd = PR_FileDesc2NativeHandle(PR_STDIN);
	//		break;
	//	case 1:
	//		osfd = PR_FileDesc2NativeHandle(PR_STDOUT);
	//		break;
	//	case 2:
	//		osfd = PR_FileDesc2NativeHandle(PR_STDERR);
	//		break;
	//	default:
	//		osfd = stdfd;
	//		//JL_REPORT_ERROR("Unsupported standard handle.");
	//}

	int descType;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &descType) );
	PRDescType type;
	type = (PRDescType)descType;

	PRFileDesc *fd;
	JSObject *descriptorObject;

	switch ( type ) {
		case PR_DESC_FILE:
			fd = PR_ImportFile(osfd);
			descriptorObject = JL_NewObjectWithGivenProto(cx, JL_CLASS(File), JL_CLASS_PROTOTYPE(cx, File), NULL);
			break;
		case PR_DESC_SOCKET_TCP:
			fd = PR_ImportTCPSocket(osfd);
			descriptorObject = JL_NewObjectWithGivenProto(cx, JL_CLASS(Socket), JL_CLASS_PROTOTYPE(cx, Socket), NULL);
			break;
		case PR_DESC_SOCKET_UDP:
			fd = PR_ImportUDPSocket(osfd);
			descriptorObject = JL_NewObjectWithGivenProto(cx, JL_CLASS(Socket), JL_CLASS_PROTOTYPE(cx, Socket), NULL);
			break;
		case PR_DESC_LAYERED:
			JL_ERR(E_THISOPERATION, E_NOTSUPPORTED);
			break;
		case PR_DESC_PIPE:
			fd = PR_ImportPipe(osfd);
			descriptorObject = JL_NewObjectWithGivenProto(cx, JL_CLASS(Pipe), JL_CLASS_PROTOTYPE(cx, Pipe), NULL);
			break;
		default:
			JL_ERR( E_MODULE, E_INTERNAL );
	}
	if ( fd == NULL )
		return ThrowIoError(cx);

	JL_CHK( descriptorObject );
	JL_SetPrivate( descriptorObject, (void*)fd);
	JL_CHK( JL_SetReservedSlot( descriptorObject, SLOT_JSIO_DESCRIPTOR_IMPORTED, JSVAL_TRUE) );

	*JL_RVAL = OBJECT_TO_JSVAL(descriptorObject);
	return JS_TRUE;
	JL_BAD;
}




/**doc
$TOC_MEMBER $INAME
 $INT $INAME( _descriptorArray_ )
  Passively waits for a descriptor event through the processEvents function.
  $H example:
{{{
(TBD)
}}}
**/

struct IOProcessEvent {

	ProcessEvent pe;

	int fdCount;
	PRPollDesc *pollDesc;
	jsval *descVal;
	PRInt32 pollResult;
};

S_ASSERT( offsetof(IOProcessEvent, pe) == 0 );


static JSBool IOPrepareWait( volatile ProcessEvent *pe, JSContext *cx, JSObject *obj ) {

	IOProcessEvent *upe = (IOProcessEvent*)pe;

	jsval fdArrayVal;
	JSObject *fdArrayObj;
	unsigned fdCount;

	JL_CHK( GetHandleSlot(cx, OBJECT_TO_JSVAL(obj), 0, &fdArrayVal) );
	fdArrayObj = JSVAL_TO_OBJECT(fdArrayVal);
	JL_CHK( JS_GetArrayLength(cx, fdArrayObj, &fdCount) );

	upe->pollDesc = (PRPollDesc*)jl_malloc(sizeof(PRPollDesc) * (1 + fdCount)); // pollDesc[0] is the event fd
	JL_ASSERT_ALLOC( upe->pollDesc );

	upe->descVal = (jsval*)jl_malloc(sizeof(jsval) * (fdCount));
	JL_ASSERT_ALLOC( upe->descVal );

	JL_updateMallocCounter(cx, (sizeof(PRPollDesc) + sizeof(jsval)) * fdCount); // approximately (pollDesc + descVal)

	JsioPrivate *mpv;
	mpv = (JsioPrivate*)JL_GetModulePrivate(cx, _moduleId);
	if ( mpv->peCancel == NULL ) {

		mpv->peCancel = PR_NewPollableEvent();
		if ( mpv->peCancel == NULL )
			return ThrowIoError(cx);
	}

	upe->pollDesc[0].fd = mpv->peCancel;
	upe->pollDesc[0].in_flags = PR_POLL_READ;
	upe->pollDesc[0].out_flags = 0;

	upe->fdCount = fdCount; // count excluding peCancel

	// (TBD) find a better solution to root fdArray content
	// (TBD) use AutoValueVector avr(cx); avr.reserve(16); avr.append(val);
	JSObject *rootedValues;
	rootedValues = JS_NewArrayObject(cx, fdCount, NULL);
	JL_CHK( SetHandleSlot(cx, OBJECT_TO_JSVAL(obj), 1, OBJECT_TO_JSVAL(rootedValues)) );

	jsval *descriptor;
	for ( unsigned int i = 0; i < fdCount; ++i ) {

		descriptor = &upe->descVal[i]; // get the slot addr
		JL_CHK( JL_GetElement(cx, fdArrayObj, i, descriptor) ); // read the item
		JL_CHK( JL_SetElement(cx, rootedValues, i, descriptor) ); // root the item
		JL_CHK( InitPollDesc(cx, *descriptor, &upe->pollDesc[1 + i]) ); // upe->pollDesc[0] is reserved for mpv->peCancel
	}

	return JS_TRUE;
	JL_BAD;
}

static void IOStartWait( volatile ProcessEvent *pe ) {

	IOProcessEvent *upe = (IOProcessEvent*)pe;

	upe->pollResult = PR_Poll(upe->pollDesc, 1 + upe->fdCount, PR_INTERVAL_NO_TIMEOUT); // 1 is the PollableEvent
}

static bool IOCancelWait( volatile ProcessEvent *pe ) {

	IOProcessEvent *upe = (IOProcessEvent*)pe;

	PRStatus st;
	st = PR_SetPollableEvent(upe->pollDesc[0].fd); // cancel the poll
	ASSERT( st == PR_SUCCESS );
	st = PR_WaitForPollableEvent(upe->pollDesc[0].fd); // resets the event. doc. blocks the calling thread until the pollable event is set, and then it atomically unsets the pollable event before it returns.
	ASSERT( st == PR_SUCCESS );

	return true;
}

static JSBool IOEndWait( volatile ProcessEvent *pe, bool *hasEvent, JSContext *cx, JSObject *obj ) {

	JL_IGNORE(obj);

	IOProcessEvent *upe = (IOProcessEvent*)pe;

	if ( upe->pollResult == -1 )
		JL_CHK( ThrowIoError(cx) );

	*hasEvent = upe->pollResult > 0 && !( upe->pollDesc[0].out_flags & PR_POLL_READ );

	if ( !*hasEvent ) // optimization
		goto end;

	for ( int i = 0; i < upe->fdCount; ++i )
		JL_CHK( PollDescNotify(cx, upe->descVal[i], &upe->pollDesc[1 + i], i) );

end:
	jl_free(upe->pollDesc);
	jl_free(upe->descVal);
	return JS_TRUE;

bad:
	jl_free(upe->pollDesc);
	jl_free(upe->descVal);
	return JS_FALSE;
}


static void IOWaitFinalize( void* ) {

//	IOProcessEvent *upe = (IOProcessEvent*)data;
}


DEFINE_FUNCTION( events ) {

	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_ARRAY(1);

	IOProcessEvent *upe;
	JL_CHK( HandleCreate(cx, JLHID(pev), &upe, IOWaitFinalize, JL_RVAL) );
	upe->pe.prepareWait = IOPrepareWait;
	upe->pe.startWait = IOStartWait;
	upe->pe.cancelWait = IOCancelWait;
	upe->pe.endWait = IOEndWait;

	JL_CHK( SetHandleSlot(cx, *JL_RVAL, 0, JL_ARG(1)) );

	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME()
  Returns $TRUE if the descriptor can be read without blocking.
  Use _timeout_ property to manage time limit for completion of this operation.
  If _timeout_ property is $UNDEF, returns the result immediately.
**/
DEFINE_FUNCTION( isReadable ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INHERITANCE();
	JL_ASSERT_ARGC_RANGE(0, 1);

	PRFileDesc *fd;
	fd = (PRFileDesc *)JL_GetPrivate(JL_OBJ); //beware: fd == NULL is supported !

/*
	PRIntervalTime prTimeout;
	if ( JL_ARG_ISDEF(1) ) {

		PRUint32 timeout;
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &timeout) );
		prTimeout = PR_MillisecondsToInterval(timeout);
	} else {

		prTimeout = PR_INTERVAL_NO_WAIT; //PR_INTERVAL_NO_TIMEOUT;
	}
*/

	PRPollDesc desc;
	desc.fd = fd;
	desc.in_flags = PR_POLL_READ;
	desc.out_flags = 0;

	PRIntervalTime timeout;
	JL_CHK( GetTimeoutInterval(cx, JL_OBJ, &timeout, PR_INTERVAL_NO_WAIT) );

	PRInt32 result;
	result = PR_Poll(&desc, 1, timeout);
	if ( result == -1 ) // error
		return ThrowIoError(cx);
	*JL_RVAL = BOOLEAN_TO_JSVAL( result == 1 && (desc.out_flags & PR_POLL_READ) != 0 );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME()
  Returns $TRUE if the descriptor can be write without blocking.
  Use _timeout_ property to manage time limit for completion of this operation.
  If _timeout_ property is $UNDEF, returns the result immediately.
**/
DEFINE_FUNCTION( isWritable ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INHERITANCE();
	JL_ASSERT_ARGC_RANGE(0, 1);

	PRFileDesc *fd;
	fd = (PRFileDesc *)JL_GetPrivate(JL_OBJ); //beware: fd == NULL is supported !

/*
	PRIntervalTime prTimeout;
	if ( JL_ARG_ISDEF(1) ) {

		PRUint32 timeout;
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &timeout) );
		prTimeout = PR_MillisecondsToInterval(timeout);
	} else {

		prTimeout = PR_INTERVAL_NO_WAIT; //PR_INTERVAL_NO_TIMEOUT;
	}
*/

	PRPollDesc desc;
	desc.fd = fd;
	desc.in_flags = PR_POLL_WRITE;
	desc.out_flags = 0;

	PRIntervalTime timeout;
	JL_CHK( GetTimeoutInterval(cx, JL_OBJ, &timeout, PR_INTERVAL_NO_WAIT) );

	PRInt32 result;
	result = PR_Poll(&desc, 1, timeout);
	if ( result == -1 ) // error
		return ThrowIoError(cx);
	*JL_RVAL = BOOLEAN_TO_JSVAL( result == 1 && (desc.out_flags & PR_POLL_WRITE) != 0 );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INAME
  Set the timeout value (in milliseconds) for blocking operations. +Infinity mean no timeout. 0 mean no wait.
  A value of $UNDEF mean that desctiptor will use the default behavior.
**/
DEFINE_PROPERTY_SETTER( timeout ) {

	JL_IGNORE(id, strict);
	JL_ASSERT_THIS_INHERITANCE();

	if ( !JSVAL_IS_VOID( *vp ) ) {

		PRIntervalTime timeout;
		if ( *vp == JSVAL_ZERO ) {

			timeout = PR_INTERVAL_NO_WAIT;
		} else
		if ( JL_ValueIsPInfinity(cx, *vp) ) {

			timeout = PR_INTERVAL_NO_TIMEOUT;
		} else {

			PRUint32 milli;
			JL_CHK( JL_JsvalToNative(cx, *vp, &milli) );
			timeout = PR_MillisecondsToInterval(milli);
		}
		JL_CHK( JL_NativeToJsval(cx, timeout, vp) );
	}

	JL_CHK( JL_SetReservedSlot( obj, SLOT_JSIO_DESCRIPTOR_TIMEOUT, *vp) );
	JL_CHK( jl::StoreProperty(cx, obj, id, vp, false) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
=== Constants ===
 $CONST DESC_FILE

 $CONST DESC_SOCKET_TCP

 $CONST DESC_SOCKET_UDP

 $CONST DESC_LAYERED

 $CONST DESC_PIPE

**/

/**doc
=== Native Interface ===
 * *NIStreamRead*
  Read the file as a stream.
**/


/**qa

**/
CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision$"))

	IS_UNCONSTRUCTIBLE

	HAS_PRIVATE // unused. Just avoid print(Descriptor.available); to crash

	BEGIN_FUNCTION_SPEC
		FUNCTION( close )
		FUNCTION( read )
		FUNCTION( write )
		FUNCTION( sync )
		FUNCTION( isReadable )
		FUNCTION( isWritable )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_GETTER( available )
		PROPERTY_GETTER( type )
		PROPERTY_GETTER( closed )
		PROPERTY_SETTER( timeout )
	END_PROPERTY_SPEC

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION( import )
		FUNCTION( events )
	END_STATIC_FUNCTION_SPEC

	BEGIN_CONST
		CONST_INTEGER( DESC_FILE       ,PR_DESC_FILE )
		CONST_INTEGER( DESC_SOCKET_TCP ,PR_DESC_SOCKET_TCP )
		CONST_INTEGER( DESC_SOCKET_UDP ,PR_DESC_SOCKET_UDP )
		CONST_INTEGER( DESC_LAYERED    ,PR_DESC_LAYERED )
		CONST_INTEGER( DESC_PIPE       ,PR_DESC_PIPE )
	END_CONST

END_CLASS
