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


DECLARE_CLASS( Descriptor )
DECLARE_CLASS( File )
DECLARE_CLASS( Socket )


JSBool NativeInterfaceStreamRead( JSContext *cx, JSObject *obj, char *buf, size_t *amount ) {

	JL_S_ASSERT( JL_InheritFrom(cx, obj, JL_CLASS(Descriptor)), "Invalid descriptor object." );
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
	res = PR_Read(fd, buf, *amount > PR_INT32_MAX ? PR_INT32_MAX : (PRInt32)*amount); // like recv() with PR_INTERVAL_NO_TIMEOUT
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
	JL_CHK( JL_GetReservedSlot(cx, obj, SLOT_JSIO_DESCRIPTOR_IMPORTED, &imported) );
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
DEFINE_FUNCTION( Close ) {

	JL_DEFINE_FUNCTION_OBJ;

	PRFileDesc *fd = (PRFileDesc*)JL_GetPrivate(cx, obj);

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
	JL_SetPrivate(cx, obj, NULL);
	JL_CHK( SetStreamReadInterface(cx, obj, NULL) );
	//	JS_ClearScope( cx, obj ); // (TBD) check if this can help to clear readable, writable, exception ?
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


JSBool ReadToJsval( JSContext *cx, PRFileDesc *fd, uint32_t amount, jsval *rval ) {

	uint8_t *buf = (uint8_t*)JS_malloc(cx, amount +1);
	JL_CHK( buf );

	PRInt32 res;
	res = PR_Read(fd, buf, amount); // like recv() with PR_INTERVAL_NO_TIMEOUT

	if (likely( res > 0 )) { // a positive number indicates the number of bytes actually read in.

		if (unlikely( JL_MaybeRealloc(amount, res) )) {

			buf = (uint8_t*)JS_realloc(cx, buf, res +1); // realloc the string using its real size
			JL_CHK( buf );
		}
		
		buf[res] = 0;
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
//			*rval = JL_GetEmptyStringValue(cx);
		return JS_TRUE;
	}

	if (unlikely( res == -1 )) { // failure. The reason for the failure can be obtained by calling PR_GetError.

		JS_free( cx, buf );
		switch ( PR_GetError() ) { // see Write() for details about errors
			case PR_WOULD_BLOCK_ERROR:
				*rval = JL_GetEmptyStringValue(cx); // mean no data available, but connection still established.
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
			return JS_TRUE;
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
		*rval = JL_GetEmptyStringValue(cx);
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
DEFINE_FUNCTION( Read ) {

	JL_DEFINE_FUNCTION_OBJ;
	PRFileDesc *fd = (PRFileDesc *)JL_GetPrivate(cx, JL_OBJ);
	JL_S_ASSERT_RESOURCE( fd );

	if (likely( JL_ARGC == 0 )) {

		PRInt32 available = PR_Available( fd ); // (TBD) use PRInt64 available = PR_Available64(fd);
		if (likely( available != -1 )) // we can use the 'available' information (possible reason: PR_NOT_IMPLEMENTED_ERROR)
			JL_CHK( ReadToJsval(cx, fd, available, JL_RVAL) ); // may block !
		else // 'available' is not usable with this fd type, then we use a buffered read (ie. read while there is someting to read)
			JL_CHK( ReadAllToJsval(cx, fd, JL_RVAL) );
	} else { // amount value is NOT provided, then try to read all

		if (likely( !JSVAL_IS_VOID(JL_ARG(1)) )) {

			PRInt32 amount;
			JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &amount) );
			JL_CHK( ReadToJsval(cx, fd, amount, JL_RVAL) ); // (TBD) check if it is good to call it even if amount is 0.
		} else {

			JL_CHK( ReadAllToJsval(cx, fd, JL_RVAL) ); // may block ! (TBD) check if this case is useful.
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
DEFINE_FUNCTION( Write ) {

	JLStr str;
	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_ARG( 1 );
	PRFileDesc *fd;
	fd = (PRFileDesc *)JL_GetPrivate(cx, JL_OBJ);
	JL_S_ASSERT_RESOURCE( fd );
	size_t sentAmount;

//	size_t len;
//	const char *str;
//	JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(1), &str, &len) );

	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &str) );

	JL_S_ASSERT( str.Length() <= PR_INT32_MAX, "Too many data." );
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

	// (TBD) try to detect if the return value will be used else just return.
	//	js_Disassemble1(cx, JL_CurrentStackFrame(cx)->script, JL_CurrentStackFrame(cx)->regs->pc +3, 0, false, stdout); // 00000:  setgvar "test"

	if (likely( sentAmount == str.Length() )) {

		*JL_RVAL = JL_GetEmptyStringValue(cx); // nothing remains
		return JS_TRUE;
	}

	if (unlikely( sentAmount < str.Length() )) {

		//*rval = STRING_TO_JSVAL( JS_NewDependentString(cx, JSVAL_TO_STRING( JL_ARG(1) ), sentAmount, len - sentAmount) ); // return unsent data // (TBD) use Blob ?
//		size_t remaining;
//		remaining = str.Length() - sentAmount;
//		buffer = JS_malloc(cx, remaining +1);
//		JL_CHK( buffer );
//		memcpy(buffer, str.GetConstStr() + sentAmount, remaining);
//		((char*)buffer)[remaining] = '\0';
//		JL_CHKB( JL_NewBlob(cx, buffer, remaining, JL_RVAL), bad_free ); // (TBD) keep the type: return a string if the JL_ARG(1) is a sring.
//		JL_CHK( , bad_free ); // (TBD) keep the type: return a string if the JL_ARG(1) is a sring.
 		return JL_NewBlobCopyN(cx, str.GetConstStr() + sentAmount, str.Length() - sentAmount, JL_RVAL);
	}

	*JL_RVAL = JL_ARG(1); // nothing has been sent

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Sync any buffered data for a fd to its backing device.
**/
DEFINE_FUNCTION( Sync ) {

	JL_DEFINE_FUNCTION_OBJ;

	PRFileDesc *fd = (PRFileDesc *)JL_GetPrivate(cx, JL_OBJ);
	JL_S_ASSERT_RESOURCE( fd );
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

	JL_S_ASSERT_THIS_INSTANCE();
	PRFileDesc *fd;
	fd = (PRFileDesc *)JL_GetPrivate( cx, obj );
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

	return JL_NativeToJsval(cx, available, vp);
/*
	if ( available <= JSVAL_INT_MAX )
		*vp = INT_TO_JSVAL( (jsint)available );
	else
		JL_CHK( JL_NewNumberValue(cx, (jsdouble)available, vp) );
*/
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  see constants below.
**/
DEFINE_PROPERTY_GETTER( type ) {

	JL_S_ASSERT_THIS_INSTANCE();
	PRFileDesc *fd;
	fd = (PRFileDesc *)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( fd );
	*vp = INT_TO_JSVAL( (jsint)PR_GetDescType(fd) );
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

	JL_S_ASSERT_THIS_INSTANCE();
	PRFileDesc *fd;
	fd = (PRFileDesc *)JL_GetPrivate( cx, obj );
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
// 					JL_CHK( JL_SetReservedSlot(cx, JL_OBJ, SLOT_JSIO_DESCRIPTOR_EOF, JSVAL_TRUE) );
DEFINE_PROPERTY( eof ) {

	return JL_GetReservedSlot(cx, obj, SLOT_JSIO_DESCRIPTOR_EOF, vp);
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

	switch (type) {
		case PR_DESC_FILE:
			fd = PR_ImportFile(osfd);
			descriptorObject = JS_NewObjectWithGivenProto(cx, JL_CLASS(File), JL_PROTOTYPE(cx, File), NULL); // (TBD) check if proto is needed !
			break;
		case PR_DESC_SOCKET_TCP:
			fd = PR_ImportTCPSocket(osfd);
			descriptorObject = JS_NewObjectWithGivenProto(cx, JL_CLASS(Socket), JL_PROTOTYPE(cx, Socket), NULL); // (TBD) check if proto is needed !
			break;
		case PR_DESC_SOCKET_UDP:
			fd = PR_ImportUDPSocket(osfd);
			descriptorObject = JS_NewObjectWithGivenProto(cx, JL_CLASS(Socket), JL_PROTOTYPE(cx, Socket), NULL); // (TBD) check if proto is needed !
			break;
		case PR_DESC_PIPE:
			fd = PR_ImportPipe(osfd);
			descriptorObject = JS_NewObjectWithGivenProto(cx, JL_CLASS(File), JL_PROTOTYPE(cx, File), NULL); // (TBD) check if proto is needed !
			break;
		default:
			JL_REPORT_ERROR("Invalid descriptor type.");
	}
	if ( fd == NULL )
		return ThrowIoError(cx);

	JL_CHK( descriptorObject );
	JL_SetPrivate(cx, descriptorObject, (void*)fd);
	JL_CHK( JL_SetReservedSlot(cx, descriptorObject, SLOT_JSIO_DESCRIPTOR_IMPORTED, JSVAL_TRUE) );

	*JL_RVAL = OBJECT_TO_JSVAL(descriptorObject);
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

	*bp = !JSVAL_IS_PRIMITIVE(*v) && JL_InheritFrom(cx, JSVAL_TO_OBJECT(*v), JL_THIS_CLASS);
	return JS_TRUE;
}

CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_HAS_INSTANCE // see issue#52

	BEGIN_FUNCTION_SPEC
		FUNCTION( Close )
		FUNCTION( Read )
		FUNCTION( Write )
		FUNCTION( Sync )
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

	BEGIN_CONST_INTEGER_SPEC
		CONST_INTEGER( DESC_FILE		,PR_DESC_FILE )
		CONST_INTEGER( DESC_SOCKET_TCP	,PR_DESC_SOCKET_TCP )
		CONST_INTEGER( DESC_SOCKET_UDP	,PR_DESC_SOCKET_UDP )
		CONST_INTEGER( DESC_LAYERED		,PR_DESC_LAYERED )
		CONST_INTEGER( DESC_PIPE		,PR_DESC_PIPE )
	END_CONST_INTEGER_SPEC

END_CLASS
