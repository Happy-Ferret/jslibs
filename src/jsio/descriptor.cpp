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
#include <string.h>
#include <cstring>

#include "descriptor.h"
#include "file.h"
#include "socket.h"

#include "../common/buffer.h"
using namespace jl;

/*
template <class T>
class Terminate {
	Terminate(T predicate) {
	}
	~Terminate() {
		predicate();
	}
};
*/


JSBool NativeInterfaceStreamRead( JSContext *cx, JSObject *obj, char *buf, size_t *amount ) {

	JL_S_ASSERT( JL_InheritFrom(cx, obj, classDescriptor), "Invalid descriptor object." );
//	JL_S_ASSERT_CLASS(obj, classDescriptor);

	PRFileDesc *fd;
	fd = (PRFileDesc*)JL_GetPrivate(cx, obj); // (PRFileDesc *)pv;
	JL_S_ASSERT_RESOURCE(fd);

	PRInt32 res;
/* (TBD) not sure a Poll is realy needed here
	PRPollDesc desc;
	desc.fd = fd;
	desc.in_flags = PR_POLL_READ;
	desc.out_flags = 0;
	res = PR_Poll( &desc, 1, PR_SecondsToInterval(10) ); // wait 10 seconds for data
	if ( res == -1 ) // if PR_Poll is not compatible with the file descriptor, just ignore the error ?
		return ThrowIoError(cx);

	if ( res == 0 ) { // timeout, no data

		*amount = 0;
		return JS_TRUE; // no data, but it is not an error.
	}
*/
	res = PR_Read(fd, buf, *amount); // like recv() with PR_INTERVAL_NO_TIMEOUT
	if ( res == -1 ) {

		PRErrorCode errorCode = PR_GetError();
		if ( errorCode != PR_WOULD_BLOCK_ERROR )// if non-blocking descriptor, this is a non-fatal error
			return ThrowIoError(cx); // real error
		*amount = 0;
		return JS_TRUE; // no data yet, but it is not an error.
	}

	//if ( res == 0 ) { // end of file is reached or the network connection is closed.
	//	// (TBD) something else to do ?
	//}

	*amount = res;
	return JS_TRUE;
	JL_BAD;
}


void FinalizeDescriptor(JSContext *cx, JSObject *obj) {

	PRFileDesc *fd = (PRFileDesc*)JL_GetPrivate( cx, obj );
	if ( !fd ) // check if not already closed
		return;
	jsval imported;
	JL_CHK( JS_GetReservedSlot(cx, obj, SLOT_JSIO_DESCRIPTOR_IMPORTED, &imported) );
	if ( imported == JSVAL_TRUE ) // Descriptor was inported, then do not close it
		return;
	PRStatus status;
	status = PR_Close(fd); // what to do on error ??
	if ( status != PR_SUCCESS ) {

		if ( PR_GetError() != PR_WOULD_BLOCK_ERROR ) // if non-blocking descriptor, this is a non-fatal error
			JS_ReportWarning(cx, "A descriptor cannot be closed while Finalize.");
	}
	JL_SetPrivate(cx, obj, NULL);

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
**/
DEFINE_FUNCTION_FAST( Close ) {

	*JL_FRVAL = JSVAL_VOID;
	PRFileDesc *fd = (PRFileDesc*)JL_GetPrivate(cx, JL_FOBJ);

	JL_S_ASSERT( fd != NULL, "The descriptor is already closed." ); // see PublicApiRules (http://code.google.com/p/jslibs/wiki/PublicApiRules)
//	if ( !fd ) { // (TBD) apply jslibsAPIRules
//
//		JL_REPORT_WARNING( "The descriptor is closed." );
//		return JS_TRUE;
//	}

	PRStatus status;
	status = PR_Close(fd);
	if ( status != PR_SUCCESS ) {

		if ( PR_GetError() != PR_WOULD_BLOCK_ERROR ) // if non-blocking descriptor, this is a non-fatal error
			return ThrowIoError(cx);
	}
	JL_SetPrivate(cx, JL_FOBJ, NULL);
	JL_CHK( SetStreamReadInterface(cx, JL_FOBJ, NULL) );
	//	JS_ClearScope( cx, obj ); // (TBD) check if this can help to clear readable, writable, exception ?
	return JS_TRUE;
	JL_BAD;
}


JSBool ReadToJsval( JSContext *cx, PRFileDesc *fd, unsigned int amount, jsval *rval ) {

	void *buf = JS_malloc(cx, amount +1);
	JL_CHK( buf );

	PRInt32 res;
	res = PR_Read(fd, buf, amount); // like recv() with PR_INTERVAL_NO_TIMEOUT

	if (likely( res > 0 )) { // a positive number indicates the number of bytes actually read in.

		if (unlikely( JL_MaybeRealloc(amount, res) )) {

			buf = (char*)JS_realloc(cx, buf, res +1); // realloc the string using its real size
			JL_CHK( buf );
		}
		((char*)buf)[res] = '\0';
		JL_CHKB( JL_NewBlob(cx, buf, res, rval), bad_free );
		return JS_TRUE;
	}

	if ( res == 0 ) { // 0 means end of file is reached or the network connection is closed.

		JS_free( cx, buf );
		// requesting 0 bytes and receiving 0 bytes does not indicate eof.
		// BUT if amount is given by PR_Available and is 0 (see Read()), and the eof is reached, this function should return (void 0)
//		if (likely( amount != 0 )) 
			*rval = JSVAL_VOID;
//		else
//			*rval = JS_GetEmptyStringValue(cx);
		return JS_TRUE;
	}

	if (unlikely( res == -1 )) { // failure. The reason for the failure can be obtained by calling PR_GetError.

		JS_free( cx, buf );
		switch ( PR_GetError() ) { // see Write() for details about errors
			case PR_WOULD_BLOCK_ERROR:
				*rval = JS_GetEmptyStringValue(cx); // mean no data available, but connection still established.
				return JS_TRUE;
			case PR_CONNECT_ABORTED_ERROR: // Connection aborted
//			case PR_SOCKET_SHUTDOWN_ERROR: // Socket shutdown
			case PR_CONNECT_RESET_ERROR: // TCP connection reset by peer
				*rval = JSVAL_VOID;
				return JS_TRUE;
			default:
				return ThrowIoError(cx);
		}
	}

bad_free:
	JS_free(cx, buf);
	JL_BAD;
}


void* JSBufferAlloc(void * opaqueAllocatorContext, unsigned int size) {
	
	return JS_malloc((JSContext*)opaqueAllocatorContext, size);
}

void JSBufferFree(void * opaqueAllocatorContext, void* address) {

	JS_free((JSContext*)opaqueAllocatorContext, address);
}

void* JSBufferRealloc(void * opaqueAllocatorContext, void* address, unsigned int size) {

	return JS_realloc((JSContext*)opaqueAllocatorContext, address, size);
}

JSBool ReadAllToJsval(JSContext *cx, PRFileDesc *fd, jsval *rval ) {

	Buffer buf;
	BufferInitialize(&buf, bufferTypeChunk, bufferGrowTypeNoGuess);
	BufferSetAllocators(&buf, cx, JSBufferAlloc, JSBufferFree, JSBufferRealloc);
	PRInt32 currentReadLength = 1024;
	for (;;) {

		PRInt32 res = PR_Read(fd, BufferNewChunk(&buf, currentReadLength), currentReadLength); // like recv() with PR_INTERVAL_NO_TIMEOUT
		if (likely( res > 0 )) { // a positive number indicates the number of bytes actually read in.

			BufferConfirm(&buf, res);
			if ( res < currentReadLength )
				break;
		} else
		if ( res == 0 ) { // 0 means end of file is reached or the network connection is closed.

			if ( BufferGetLength(&buf) > 0 ) // we reach eof BUT we have read some data.
				break; // no error, no data received, we cannot reach currentReadLength

			BufferFinalize(&buf);
			*rval = JSVAL_VOID;
		} else
		if ( res == -1 ) { // -1 indicates a failure. The reason for the failure can be obtained by calling PR_GetError.
			
			switch ( PR_GetError() ) { // see Write() for details about errors
				case PR_WOULD_BLOCK_ERROR: // The operation would have blocked (non-fatal error)
					break;
				case PR_CONNECT_ABORTED_ERROR: // Connection aborted
//				case PR_SOCKET_SHUTDOWN_ERROR: // Socket shutdown
				case PR_CONNECT_RESET_ERROR: // TCP connection reset by peer
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
		*rval = JS_GetEmptyStringValue(cx);
		return JS_TRUE;
	}

	*BufferNewChunk(&buf, 1) = '\0';
	BufferConfirm(&buf, 1);
	JL_CHK( JL_NewBlob(cx, BufferGetDataOwnership(&buf), BufferGetLength(&buf) -1, rval) ); // -1 because '\0' is not a part of the data.

	BufferFinalize(&buf);
	return JS_TRUE;

bad:
	BufferFinalize(&buf);
	return JS_FALSE;
}


/**doc
$TOC_MEMBER $INAME
 $VAL $INAME( [amount] )
  Read _amount_ bytes of data from the current descriptor. If _amount_ is ommited, the whole available data is read.
  If the descriptor is exhausted (eof or disconnected), this function returns _undefined_.
  If the descriptor is a blocking socket and _amount_ is set to the value $UNDEF value, the call blocks until data is available.
  $H beware
   This function returns a Blob or a string literal as empty string.
  $H example
  {{{
  LoadModule('jsstd');
  LoadModule('jsio');

  var soc = new Socket();
  soc.Connect('www.google.com', 80);
  soc.Write('GET\r\n\r\n');
  Print( soc.Read(undefined), '\n' );
  }}}
**/
DEFINE_FUNCTION_FAST( Read ) {

	PRFileDesc *fd = (PRFileDesc *)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE( fd );

	if (likely( JL_ARGC == 0 )) {

		PRInt32 available = PR_Available( fd ); // (TBD) use PRInt64 available = PR_Available64(fd);
		if (likely( available != -1 )) // we can use the 'available' information (possible reason: PR_NOT_IMPLEMENTED_ERROR)
			JL_CHK( ReadToJsval(cx, fd, available, JL_FRVAL) ); // may block !
		else // 'available' is not usable with this fd type, then we use a buffered read (ie. read while there is someting to read)
			JL_CHK( ReadAllToJsval(cx, fd, JL_FRVAL) );
	} else { // amount value is NOT provided, then try to read all

		if (likely( !JSVAL_IS_VOID(JL_FARG(1)) )) {

			PRInt32 amount;
			JL_CHK( JsvalToInt(cx, JL_FARG(1), &amount) );
			JL_CHK( ReadToJsval(cx, fd, amount, JL_FRVAL) ); // (TBD) check if it is good to call it even if amount is 0.
		} else {

			JL_CHK( ReadAllToJsval(cx, fd, JL_FRVAL) ); // may block ! (TBD) check if this case is useful.
		}
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
DEFINE_FUNCTION_FAST( Write ) {

	JL_S_ASSERT_ARG( 1 );
	PRFileDesc *fd;
	fd = (PRFileDesc *)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE( fd );
	unsigned int len, sentAmount;
	const char *str;
	JL_CHK( JsvalToStringAndLength(cx, &JL_FARG(1), &str, &len) );

	PRInt32 res;
	res = PR_Write( fd, str, len );
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

				*JL_FRVAL = JSVAL_VOID;
				return JS_TRUE;
			default:
				return ThrowIoError(cx);
		}
	} else {

		sentAmount = res;
	}

	// (TBD) try to detect if the return value will be used else just return.
	//	js_Disassemble1(cx, JL_CurrentStackFrame(cx)->script, JL_CurrentStackFrame(cx)->regs->pc +3, 0, false, stdout); // 00000:  setgvar "test"

	void *buffer;

	if (likely( sentAmount == len )) {

		*JL_FRVAL = JS_GetEmptyStringValue(cx); // nothing remains
		return JS_TRUE;
	}

	if (unlikely( sentAmount < len )) {

		//*rval = STRING_TO_JSVAL( JS_NewDependentString(cx, JSVAL_TO_STRING( JL_ARG(1) ), sentAmount, len - sentAmount) ); // return unsent data // (TBD) use Blob ?
		unsigned int remaining;
		remaining = len - sentAmount;
		buffer = JS_malloc(cx, remaining +1);
		JL_CHK( buffer );
		((char*)buffer)[remaining] = '\0';
		memcpy(buffer, str, remaining);
		JL_CHKB( JL_NewBlob(cx, buffer, remaining, JL_FRVAL), bad_free );
 		return JS_TRUE;
	}

	*JL_FRVAL = JL_FARG(1); // nothing has been sent

	return JS_TRUE;
bad_free:
	JS_free(cx, buffer);
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Sync any buffered data for a fd to its backing device.
**/
DEFINE_FUNCTION_FAST( Sync ) {

	PRFileDesc *fd = (PRFileDesc *)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE( fd );
	JL_CHKB( PR_Sync(fd) == PR_SUCCESS, bad_ioerror );
	*JL_FRVAL = JSVAL_VOID;
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
DEFINE_PROPERTY( available ) {

	PRFileDesc *fd = (PRFileDesc *)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( fd );

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

	if ( available <= JSVAL_INT_MAX )
		*vp = INT_TO_JSVAL(available);
	else
		JL_CHK( JS_NewNumberValue(cx, (jsdouble)available, vp) );

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  see constants below.
**/
DEFINE_PROPERTY( type ) {

	PRFileDesc *fd = (PRFileDesc *)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( fd );
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
DEFINE_PROPERTY( closed ) {

	PRFileDesc *fd = (PRFileDesc *)JL_GetPrivate( cx, obj );
	*vp = BOOLEAN_TO_JSVAL( fd == NULL );
	return JS_TRUE;
}

/* *doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Is $TRUE if the end of the file has been reached or the socket has been disconnected.
**/
/*
// 					JL_CHK( JS_SetReservedSlot(cx, JL_FOBJ, SLOT_JSIO_DESCRIPTOR_EOF, JSVAL_TRUE) );
DEFINE_PROPERTY( eof ) {

	return JS_GetReservedSlot(cx, obj, SLOT_JSIO_DESCRIPTOR_EOF, vp);
}
*/

/**doc
=== Static functions ===
**/

/**doc
$TOC_MEMBER $INAME
 $INAME( nativeDescriptor, type )
**/
DEFINE_FUNCTION( Import ) {

	JL_S_ASSERT_ARG_MIN(2);
	//int stdfd;
	//JL_CHK( JsvalToInt(cx, JL_ARG(1), &stdfd) );

	PRInt32 osfd;
	JL_CHK( JsvalToInt(cx, JL_ARG(1), &osfd) );

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
	JL_CHK( JsvalToInt(cx, JL_ARG(2), &descType) );
	PRDescType type;
	type = (PRDescType)descType;

	PRFileDesc *fd;
	JSObject *descriptorObject;

	switch (type) {
		case PR_DESC_FILE:
			fd = PR_ImportFile(osfd);
			descriptorObject = JS_NewObject(cx, classFile, NULL, NULL); // (TBD) check if proto is needed !
			break;
		case PR_DESC_SOCKET_TCP:
			fd = PR_ImportTCPSocket(osfd);
			descriptorObject = JS_NewObject(cx, classSocket, NULL, NULL); // (TBD) check if proto is needed !
			break;
		case PR_DESC_SOCKET_UDP:
			fd = PR_ImportUDPSocket(osfd);
			descriptorObject = JS_NewObject(cx, classSocket, NULL, NULL); // (TBD) check if proto is needed !
			break;
		case PR_DESC_PIPE:
			fd = PR_ImportPipe(osfd);
			descriptorObject = JS_NewObject(cx, classFile, NULL, NULL); // (TBD) check if proto is needed !
			break;
		default:
			JL_REPORT_ERROR("Invalid descriptor type.");
	}
	if ( fd == NULL )
		return ThrowIoError(cx);

	JL_CHK( descriptorObject );
	JL_SetPrivate(cx, descriptorObject, (void*)fd);
	JL_CHK( JS_SetReservedSlot(cx, descriptorObject, SLOT_JSIO_DESCRIPTOR_IMPORTED, JSVAL_TRUE) );

	*rval = OBJECT_TO_JSVAL(descriptorObject);
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


DEFINE_HAS_INSTANCE() { // see issue#52

	// Check whether v is an instance of obj.
	*bp = !JSVAL_IS_PRIMITIVE(v) && JL_InheritFrom(cx, JSVAL_TO_OBJECT(v), JL_GetClass(obj));
	return JS_TRUE;
}

CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_HAS_INSTANCE // see issue#52

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST( Close )
		FUNCTION_FAST( Read )
		FUNCTION_FAST( Write )
		FUNCTION_FAST( Sync )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ( available )
		PROPERTY_READ( type )
		PROPERTY_READ( closed )
//		PROPERTY_READ( eof )
	END_PROPERTY_SPEC

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION( Import )
	END_STATIC_FUNCTION_SPEC

	BEGIN_CONST_DOUBLE_SPEC
		CONST_DOUBLE( DESC_FILE			,PR_DESC_FILE )
		CONST_DOUBLE( DESC_SOCKET_TCP	,PR_DESC_SOCKET_TCP )
		CONST_DOUBLE( DESC_SOCKET_UDP ,PR_DESC_SOCKET_UDP )
		CONST_DOUBLE( DESC_LAYERED	   ,PR_DESC_LAYERED )
		CONST_DOUBLE( DESC_PIPE       ,PR_DESC_PIPE )
	END_CONST_DOUBLE_SPEC

END_CLASS
