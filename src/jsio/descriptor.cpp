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
#include "../jslang/blobPub.h"

DECLARE_CLASS( Descriptor )
DECLARE_CLASS( Pipe )
DECLARE_CLASS( File )
DECLARE_CLASS( Socket )


bool InitPollDesc( JSContext *cx, JS::HandleValue descVal, PRPollDesc *pollDesc );
bool PollDescNotify( JSContext *cx, JS::HandleValue descVal, PRPollDesc *pollDesc, int index );


bool NativeInterfaceStreamRead( JSContext *cx, JS::HandleObject obj, char *buf, size_t *amount ) {

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
		return true; // no data, but it is not an error.
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
				return true;
			default:
				// try to catch logic errors
				return ThrowIoError(cx);
		}
	}

	*amount = res;
	return true;
	JL_BAD;
}



void FinalizeDescriptor(JSFreeOp *fop, JSObject *obj) {

	PRFileDesc *fd = (PRFileDesc*)js::GetObjectPrivate(obj);
	if ( !fd ) // check if not already closed
		return;

	JS::RootedValue imported(fop->runtime(), JS_GetReservedSlot(obj, SLOT_JSIO_DESCRIPTOR_IMPORTED));

	if ( !imported.isUndefined() ) // Descriptor was inported, then do not close it
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

	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INHERITANCE();
	JL_ASSERT_ARGC(0);

	JL_RVAL.setUndefined();

	PRFileDesc *fd;
	fd = (PRFileDesc*)JL_GetPrivate(JL_OBJ);

	JL_ASSERT_WARN( fd, E_NAME(JL_THIS_CLASS_NAME), E_CLOSED ); // see PublicApiRules (http://code.google.com/p/jslibs/wiki/PublicApiRules)
	if ( !fd )
		return true;

	PRStatus status;
	status = PR_Close(fd);
	if ( status != PR_SUCCESS ) {

		if ( PR_GetError() != PR_WOULD_BLOCK_ERROR ) // if non-blocking descriptor, this is a non-fatal error
			return ThrowIoError(cx);
	}
	JL_SetPrivate(JL_OBJ, NULL);
	JL_CHK( SetStreamReadInterface(cx, JL_OBJ, NULL) );

	return true;
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


bool ReadAllToJsval(JSContext *cx, PRFileDesc *fd, jsval *rval ) {

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
			return true;
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
					return true;
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
		return true;
	}

	JL_CHK( JL_NewBufferGetOwnership(cx, BufferGetDataOwnership(&buf), BufferGetLength(&buf), rval) );

	BufferFinalize(&buf);
	return true;

bad:
	BufferFinalize(&buf);
	return false;
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

	jl::BufPartial buffer;

	JL_DEFINE_ARGS;
	JL_ASSERT_THIS_INHERITANCE();

	PRFileDesc *fd;
	fd = (PRFileDesc*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( fd );

	uint32_t amount;
	PRInt32 res;

	if ( JL_ARG_ISDEF(1) ) {

		JL_CHK( jl::getValue(cx, JL_ARG(1), &amount) );
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

			amount = 1; // don't use 0 to avoid |res == amount == 0| case and wrongly return empty instead of undefined !!!
		} else
		if ( available == -1 ) { // API not available

			ASSERT( PR_GetError() == PR_NOT_IMPLEMENTED_ERROR ); // (TBD) else handle errors properly
			amount = 4096;
		} else {

			amount = (uint32_t)available;
		}
	}


	//uint8_t *buf;
	//buf = JL_NewBuffer(cx, amount, JL_RVAL);
	//JL_ASSERT_ALLOC( buf );

	buffer.alloc(amount);
	JL_ASSERT_ALLOC( buffer.hasData() );


	//PRIntervalTime timeout;
	//GetTimeoutInterval(cx, JL_OBJ, &timeout);
	//res = PR_Recv(fd, buf, amount, 0, timeout);

	res = PR_Read(fd, buffer.data(), buffer.size()); // like recv() with PR_INTERVAL_NO_TIMEOUT

	if (unlikely( res == -1 )) { // failure. The reason for the failure can be obtained by calling PR_GetError.

		switch ( PR_GetError() ) { // see Write() for details about errors
			case PR_WOULD_BLOCK_ERROR:

				//if ( amount > 0 )
					//JL_CHK( JL_ChangeBufferLength(cx, JL_RVAL, 0) ); // mean no data available, but connection still established.
				return BlobCreateEmpty(cx, JL_RVAL);

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
				
				//JL_CHK( JL_FreeBuffer(cx, JL_RVAL) );
				JL_RVAL.setUndefined();
				return true;

			default:
				return ThrowIoError(cx);
		}
	}

	if ( amount == 0 )
		return BlobCreateEmpty(cx, JL_RVAL);

	if ( res == 0 ) { // doc: 0 means end of file is reached or the network connection is closed.

		JL_RVAL.setUndefined();
		return true;
	}

	if ( (uint32_t) res < amount )
		buffer.setUsed(res);

	return BlobCreate(cx, buffer, JL_RVAL);
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( data )
  If the whole data cannot be written, Write returns that have not be written.
  If the descriptor is disconnected (socket only), this function returns _undefined_.
**/
DEFINE_FUNCTION( write ) {

	jl::BufString str;

	JL_DEFINE_ARGS;
	JL_ASSERT_THIS_INHERITANCE();
	JL_ASSERT_ARGC( 1 );

	PRFileDesc *fd;
	fd = (PRFileDesc *)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( fd );
	size_t sentAmount;

	JL_CHK( jl::getValue(cx, JL_ARG(1), &str) );

	ASSERT( str.length() <= PR_INT32_MAX );

	PRInt32 res;
	res = PR_Write( fd, str.toData<const uint8_t*>(), (PRInt32)str.length() );
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

				JL_RVAL.setUndefined();
				return true;
			default:
				return ThrowIoError(cx);
		}
	} else {

		sentAmount = res;
	}

	ASSERT( sentAmount <= str.length() );

	if (likely( sentAmount == str.length() )) { // nothing remains

		//JL_CHK( JL_NewEmptyBuffer(cx, JL_RVAL) );
		JL_CHK( BlobCreateEmpty(cx, JL_RVAL) );
	} else if ( sentAmount == 0 ) { // nothing has been sent

		if ( JL_ARG(1).isString() ) { // optimization (string are immutable)

			JL_RVAL.set(JL_ARG(1));
		} else {

			//JL_CHK( str.GetArrayBuffer(cx, JL_RVAL) );
			JL_CHK( BlobCreate(cx, str.toData<uint8_t*>(), str.length(), JL_RVAL) );
		}
	} else { // return unsent data

		//JL_CHK( JL_NewBufferCopyN(cx, str.GetConstStr() + sentAmount, str.length() - sentAmount, JL_RVAL) );
		//JL_CHK( BlobCreateCopy(cx, str.GetConstStr() + sentAmount, str.length() - sentAmount, JL_RVAL) );

		if ( JL_ARG(1).isString() ) {
			
			JS::RootedString tmp(cx, JL_ARG(3).toString());
			JL_RVAL.setString(JS_NewDependentString(cx, tmp, sentAmount, str.length() - sentAmount));
		} else {
			
			size_t length = str.length() - sentAmount;
			void *data = jl_malloc(length);
			JL_ASSERT_ALLOC(data);
			jl::memcpy(data, str.toData<const uint8_t*>() + sentAmount, length);
			JL_RVAL.setObject(*JS_NewArrayBufferWithContents(cx, length, data));
		}
	}

	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Sync any buffered data for a fd to its backing device.
**/
DEFINE_FUNCTION( sync ) {

	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INHERITANCE();
	JL_ASSERT_ARGC(0);

	PRFileDesc *fd;
	fd = (PRFileDesc *)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( fd );
	JL_CHKB( PR_Sync(fd) == PR_SUCCESS, bad_ioerror );

	JL_RVAL.setUndefined();
	return true;

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

	JL_DEFINE_PROP_ARGS;

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
//				return true;
//		}
		return ThrowIoError(cx);
	}

	return jl::setValue(cx, JL_RVAL, available);
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  see constants below.
**/
DEFINE_PROPERTY_GETTER( type ) {

	JL_DEFINE_PROP_ARGS;

	JL_ASSERT_THIS_INHERITANCE();

	PRFileDesc *fd;
	fd = (PRFileDesc *)JL_GetPrivate( obj );
	JL_ASSERT_THIS_OBJECT_STATE( fd ); //	JL_ASSERT_THIS_INSTANCE();
	JL_RVAL.setInt32( (int32_t)PR_GetDescType(fd) );
	return true;
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

	JL_DEFINE_PROP_ARGS;

	JL_ASSERT_THIS_INHERITANCE();

	PRFileDesc *fd;
	fd = (PRFileDesc *)JL_GetPrivate( obj );
	JL_RVAL.setBoolean( fd == NULL );
	return true;
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

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_MIN(2);
	//int stdfd;
	//JL_CHK( jl::getValue(cx, JL_ARG(1), &stdfd) );

	PRInt32 osfd;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &osfd) );

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
	JL_CHK( jl::getValue(cx, JL_ARG(2), &descType) );
	PRDescType type;
	type = (PRDescType)descType;

	{
	PRFileDesc *fd;
	JS::RootedObject descriptorObject(cx);

	switch ( type ) {
		case PR_DESC_FILE:
			fd = PR_ImportFile(osfd);
			descriptorObject = jl::newObjectWithGivenProto(cx, JL_CLASS(File), JL_CLASS_PROTOTYPE(cx, File));
			break;
		case PR_DESC_SOCKET_TCP:
			fd = PR_ImportTCPSocket(osfd);
			descriptorObject = jl::newObjectWithGivenProto(cx, JL_CLASS(Socket), JL_CLASS_PROTOTYPE(cx, Socket));
			break;
		case PR_DESC_SOCKET_UDP:
			fd = PR_ImportUDPSocket(osfd);
			descriptorObject = jl::newObjectWithGivenProto(cx, JL_CLASS(Socket), JL_CLASS_PROTOTYPE(cx, Socket));
			break;
		case PR_DESC_LAYERED:
			JL_ERR(E_THISOPERATION, E_NOTSUPPORTED);
			break;
		case PR_DESC_PIPE:
			fd = PR_ImportPipe(osfd);
			descriptorObject = jl::newObjectWithGivenProto(cx, JL_CLASS(Pipe), JL_CLASS_PROTOTYPE(cx, Pipe));
			break;
		default:
			JL_ERR( E_MODULE, E_INTERNAL );
	}
	if ( fd == NULL )
		return ThrowIoError(cx);

	JL_CHK( descriptorObject );
	JL_SetPrivate( descriptorObject, (void*)fd);

	JL_CHK( JL_SetReservedSlot(descriptorObject, SLOT_JSIO_DESCRIPTOR_IMPORTED, JL_TRUE()) );

	JL_RVAL.setObject(*descriptorObject);
	}

	return true;
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

struct IOProcessEvent : public ProcessEvent2 {

	uint32_t _fdCount;
	PRPollDesc *_pollDesc;
	PRInt32 _pollResult;

	bool prepareWait(JSContext *cx, JS::HandleObject obj) {

		JS::RootedValue descriptor(cx);
		JS::RootedObject fdArrayObj(cx, &slot(0).toObject());
		JL_CHK( JS_GetArrayLength(cx, fdArrayObj, &_fdCount) );

		_pollDesc = (PRPollDesc*)jl_malloc(sizeof(PRPollDesc) * (1 + _fdCount)); // pollDesc[0] is the peCancel event fd, _fdCount excludes peCancel descriptior.
		JL_ASSERT_ALLOC( _pollDesc );

//		JL_updateMallocCounter(cx, (sizeof(PRPollDesc) /*+ sizeof(jsval)*/) * _fdCount); // approximately (pollDesc + descVal)

		JsioPrivate *mpv;
		mpv = (JsioPrivate*)jl::Host::getHost(cx).moduleManager().modulePrivate(moduleId());
		if ( mpv->peCancel == NULL ) {

			mpv->peCancel = PR_NewPollableEvent();
			if ( mpv->peCancel == NULL )
				return ThrowIoError(cx);
		}

		_pollDesc[0].fd = mpv->peCancel;
		_pollDesc[0].in_flags = PR_POLL_READ;
		_pollDesc[0].out_flags = 0;

		// in order to avoid a Descriptor being GC because it has been removed from the descriptor list (slot(0)), we just duplicate it.
		allocDynSlots(_fdCount);
		for ( uint32_t i = 0; i < _fdCount; ++i ) {

			JL_CHK( JL_GetElement(cx, fdArrayObj, i, &descriptor) ); // read the item
			hDynSlot(i).set(descriptor);
			JL_CHK( InitPollDesc(cx, descriptor, &_pollDesc[1 + i]) ); // _pollDesc[0] is reserved for mpv->peCancel
		}

		return true;
		JL_BAD;
	}

	void startWait() {

		_pollResult = PR_Poll(_pollDesc, 1 + _fdCount, PR_INTERVAL_NO_TIMEOUT); // 1 is the PollableEvent
	}

	bool cancelWait() {

		PRStatus st;
		st = PR_SetPollableEvent(_pollDesc[0].fd); // cancel the poll
		ASSERT( st == PR_SUCCESS );
		st = PR_WaitForPollableEvent(_pollDesc[0].fd); // resets the event. doc. blocks the calling thread until the pollable event is set, and then it atomically unsets the pollable event before it returns.
		ASSERT( st == PR_SUCCESS );
		return true;
	}

	bool endWait(bool *hasEvent, JSContext *cx, JS::HandleObject) {

		JLAutoPtr<PRPollDesc> pollDesc(_pollDesc);

		if ( _pollResult == -1 )
			JL_CHK( ThrowIoError(cx) );

		*hasEvent = _pollResult > 0 && !( _pollDesc[0].out_flags & PR_POLL_READ ); // has an event but not the cancel event

		if ( *hasEvent ) { // optimization
		
			for ( uint32_t i = 0; i < _fdCount; ++i ) {

				JL_CHK( PollDescNotify(cx, hDynSlot(i), &_pollDesc[1 + i], i) );
			}
		}
		return true;
		JL_BAD;
	}
};


DEFINE_FUNCTION( events ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_ARRAY(1);

	IOProcessEvent *upe = new IOProcessEvent();
	JL_CHK( HandleCreate(cx, upe, JL_RVAL) );

	upe->slot(0) = JL_ARG(1);

	return true;
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

	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INHERITANCE();
	JL_ASSERT_ARGC_RANGE(0, 1);

	PRFileDesc *fd;
	fd = (PRFileDesc *)JL_GetPrivate(JL_OBJ); //beware: fd == NULL is supported !

/*
	PRIntervalTime prTimeout;
	if ( JL_ARG_ISDEF(1) ) {

		PRUint32 timeout;
		JL_CHK( jl::getValue(cx, JL_ARG(1), &timeout) );
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
	JL_RVAL.setBoolean( result == 1 && (desc.out_flags & PR_POLL_READ) != 0 );
	return true;
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

	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INHERITANCE();
	JL_ASSERT_ARGC_RANGE(0, 1);

	PRFileDesc *fd;
	fd = (PRFileDesc *)JL_GetPrivate(JL_OBJ); //beware: fd == NULL is supported !

/*
	PRIntervalTime prTimeout;
	if ( JL_ARG_ISDEF(1) ) {

		PRUint32 timeout;
		JL_CHK( jl::getValue(cx, JL_ARG(1), &timeout) );
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
	JL_RVAL.setBoolean( result == 1 && (desc.out_flags & PR_POLL_WRITE) != 0 );
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INAME
  Set the timeout value (in milliseconds) for blocking operations. +Infinity mean no timeout. 0 mean no wait.
  A value of $UNDEF mean that desctiptor will use the default behavior.
**/
DEFINE_PROPERTY_SETTER( timeout ) {

	JL_DEFINE_PROP_ARGS;
	JL_ASSERT_THIS_INHERITANCE();

	if ( !JL_RVAL.isUndefined() ) {

		PRIntervalTime timeout;
		if ( (JS::Value)JL_RVAL == JSVAL_ZERO ) {

			timeout = PR_INTERVAL_NO_WAIT;
		} else
		if ( jl::isPInfinity(cx, JL_RVAL) ) {

			timeout = PR_INTERVAL_NO_TIMEOUT;
		} else {

			PRUint32 milli;
			JL_CHK( jl::getValue(cx, JL_RVAL, &milli) );
			timeout = PR_MillisecondsToInterval(milli);
		}
		JL_CHK( jl::setValue(cx, JL_RVAL, timeout) );
	}

	JL_CHK( JL_SetReservedSlot( obj, SLOT_JSIO_DESCRIPTOR_TIMEOUT, JL_RVAL) );
	JL_CHK( jl::StoreProperty(cx, obj, id, JL_RVAL, false) );
	return true;
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
