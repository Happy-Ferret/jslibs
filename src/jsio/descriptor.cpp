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

// open: 	SetNativeInterface(cx, obj, ...
// close: 	RemoveNativeInterface(cx, obj, NI_READ_RESOURCE );


JSBool NativeInterfaceStreamRead( JSContext *cx, JSObject *obj, char *buf, size_t *amount ) {

	J_S_ASSERT( InheritFrom(cx, obj, classDescriptor), "Invalid descriptor object." );
//	J_S_ASSERT_CLASS(obj, classDescriptor);

	PRFileDesc *fd;
	fd = (PRFileDesc*)JS_GetPrivate(cx, obj); // (PRFileDesc *)pv;
	J_S_ASSERT_RESOURCE(fd);

	PRInt32 ret;
	PRPollDesc desc;
	desc.fd = fd;
	desc.in_flags = PR_POLL_READ;
	desc.out_flags = 0;
	ret = PR_Poll( &desc, 1, PR_SecondsToInterval(10) ); // wait 10 seconds for data
	if ( ret == -1 ) // if PR_Poll is not compatible with the file descriptor, just ignore the error ?
		return ThrowIoError(cx);

	if ( ret == 0 ) { // timeout

		*amount = 0;
		return JS_TRUE; // no data, but it is not an error.
	}

	ret = PR_Read(fd, buf, *amount);
	if ( ret == -1 ) {

		PRErrorCode errorCode = PR_GetError();
		if ( errorCode != PR_WOULD_BLOCK_ERROR )// if non-blocking descriptor, this is a non-fatal error
			return ThrowIoError(cx); // real error
		*amount = 0;
		return JS_TRUE; // no data yet, but it is not an error.
	}

	if ( ret == 0 ) { // end of file is reached or the network connection is closed.
		
		// (TBD) something to do ?
	}

	*amount = ret;
	return JS_TRUE;
	JL_BAD;
}


void FinalizeDescriptor(JSContext *cx, JSObject *obj) {

	PRFileDesc *fd = (PRFileDesc*)JS_GetPrivate( cx, obj );
	if ( fd != NULL ) { // check if not already closed

		jsval imported;
		J_CHK( JS_GetReservedSlot(cx, obj, SLOT_JSIO_DESCRIPTOR_IMPORTED, &imported) );
		if ( imported != JSVAL_TRUE ) { // Descriptor was inported, then do not close it

			PRStatus status = PR_Close( fd ); // what to do on error ??
			if (status != PR_SUCCESS) {

				PRErrorCode errorCode = PR_GetError();
				if ( errorCode != PR_WOULD_BLOCK_ERROR ) // if non-blocking descriptor, this is a non-fatal error
					JS_ReportError( cx, "A descriptor cannot be closed while Finalize." );
			}
			J_CHK( JS_SetPrivate( cx, obj, NULL ) );
		}
	}
bad:
	return;
}

/**doc fileIndex:top
$CLASS_HEADER
**/
BEGIN_CLASS( Descriptor )


DEFINE_CONSTRUCTOR() {

	J_REPORT_ERROR( J__ERRMSG_NO_CONSTRUCT ); // BUT constructor must be defined
	JL_BAD;
}

/**doc
=== Methods ===
**/

/**doc
 * *Close*()
  Close the descriptor.
**/
DEFINE_FUNCTION( Close ) {

	PRFileDesc *fd = (PRFileDesc*)JS_GetPrivate( cx, obj );
	J_S_ASSERT( fd != NULL, "file is closed." );
	PRStatus status;
	status = PR_Close( fd );
	if ( status != PR_SUCCESS ) {

		PRErrorCode errorCode = PR_GetError();
		if ( errorCode != PR_WOULD_BLOCK_ERROR ) // if non-blocking descriptor, this is a non-fatal error
			return ThrowIoError(cx);
	}
	J_CHK( JS_SetPrivate( cx, obj, NULL ) );
//	JS_ClearScope( cx, obj ); // help to clear readable, writable, exception
//	J_CHK( SetStreamReadInterface(cx, obj, NULL) );
	J_CHK( SetStreamReadInterface(cx, obj, NULL) );
	return JS_TRUE;
	JL_BAD;
}


JSBool ReadToJsval( JSContext *cx, PRFileDesc *fd, int amount, jsval *rval ) {

	char *buf = (char*)JS_malloc(cx, amount +1);
	J_S_ASSERT_ALLOC(buf);
	buf[amount] = '\0';

	PRInt32 res;
	res = PR_Read(fd, buf, amount);
	if (res == -1) { // failure. The reason for the failure can be obtained by calling PR_GetError.

		JS_free( cx, buf );

		PRErrorCode errCode = PR_GetError();
		if ( errCode != PR_WOULD_BLOCK_ERROR )
			J_CHK( ThrowIoError(cx) );
		*rval = JS_GetEmptyStringValue(cx);
		return JS_TRUE;
	}

	if (res == 0) {

		JS_free( cx, buf );
		*rval = JSVAL_VOID; // end of file/socket
		return JS_TRUE;
	}

	if ( MaybeRealloc(amount, res) ) {

		buf = (char*)JS_realloc(cx, buf, res + 1); // realloc the string using its real size
		J_S_ASSERT_ALLOC(buf);
	}

	J_CHKB( J_NewBlob(cx, buf, res, rval), bad_free );

	return JS_TRUE;
bad_free:
	JS_free(cx, buf);
	JL_BAD;
}


JSBool ReadAllToJsval(JSContext *cx, PRFileDesc *fd, jsval *rval ) {

	Buffer buf;
	BufferInitialize(&buf, bufferTypeChunk, bufferGrowTypeNoGuess);
	PRInt32 currentReadLength = 512;
	for (;;) {

		if ( currentReadLength < 16384 )
			currentReadLength *= 2;

		PRInt32 res = PR_Read(fd, BufferNewChunk(&buf, currentReadLength), currentReadLength);
		if ( res > 0 ) {

			if ( res < currentReadLength ) {

				BufferUnused(&buf, currentReadLength - res);
				break;
			}
		} else if ( res == -1 ) { // failure. The reason for the failure can be obtained by calling PR_GetError.

			if ( PR_GetError() != PR_WOULD_BLOCK_ERROR )
				J_CHK( ThrowIoError(cx) );
			break; // no error, no data received, we cannot reach currentReadLength
		} else if ( res == 0 ) { // end of file/socket

			if ( BufferGetLength(&buf) > 0 ) { // we reach eof BUT we have read some data.

				break; // no error, no data received, we cannot reach currentReadLength
			} else {

				BufferFinalize(&buf);
				*rval = JSVAL_VOID;
				return JS_TRUE;
			}
		}
	}

	if ( BufferGetLength(&buf) == 0 ) { // PR_WOULD_BLOCK_ERROR and NOT end of file/socket

		BufferFinalize(&buf);
		*rval = JS_GetEmptyStringValue(cx);
		return JS_TRUE;
	}

	size_t length = BufferGetLength(&buf);
	char *jsData = (char*)JS_malloc(cx, length +1);
	BufferCopyData(&buf, jsData, length);
	jsData[length] = '\0';
	J_CHK( J_NewBlob( cx, jsData, length, rval ) );

	BufferFinalize(&buf);
	return JS_TRUE;
bad:
	BufferFinalize(&buf);
	return JS_FALSE;
}


/**doc
 * $VAL *Read*( [amount] )
  Read _amount_ bytes of data from the current descriptor. If _amount_ is ommited, the whole available data is read.
  If the descriptor is exhausted (eof or disconnected), this function returns _undefined_.
  $H beware	
   This function returns a Blob or a string literal as empty string.
**/
DEFINE_FUNCTION( Read ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( fd );

	if ( J_ARG_ISDEF(1) ) {

		PRInt32 amount;
		J_CHK( JsvalToInt(cx, J_ARG(1), &amount) );

//		if ( amount == 0 ) // (TBD) check if it is good to use this ( even if amount is 0, we must call Read
//			*rval = JS_GetEmptyStringValue(cx);
//		else

		J_CHK( ReadToJsval(cx, fd, amount, rval) );

	} else { // amount value is NOT provided, then try to read all

		PRInt32 available = PR_Available( fd );
		if ( available != -1 ) // we can use the 'available' information
			J_CHK( ReadToJsval(cx, fd, available, rval) );
		else // 'available' is not usable with this fd type, then we used a buffered read ( aka read while there is someting to read )
			J_CHK( ReadAllToJsval(cx, fd, rval) );
	}
	return JS_TRUE;
	JL_BAD;
}


/**doc
 * $STR *Write*( data )
  If the whole data cannot be written, Write returns that have not be written.
**/
DEFINE_FUNCTION( Write ) {

	J_S_ASSERT_ARG_MIN( 1 );
	PRFileDesc *fd;
	fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	J_S_ASSERT_RESOURCE( fd );
	const char *str;
	size_t len;
	J_CHK( JsvalToStringAndLength(cx, &J_ARG(1), &str, &len) );

	size_t sentAmount;

	PRInt32 res;
	res = PR_Write( fd, str, len );
	if ( res == -1 ) {

		PRErrorCode errCode = PR_GetError();
/*
		if ( errCode == PR_CONNECT_RESET_ERROR ) { // 10054

			// WSAECONNRESET (error 10054)
			// Connection reset by peer. An existing connection
			// was forcibly closed by the remote host. This
			// normally results if the peer application on the
			// remote host is suddenly stopped, the host is
			// rebooted, or the remote host uses a hard close
			// (see setsockopt for more information on the
			// SO_LINGER option on the remote socket.) This
			// error may also result if a connection was broken
			// due to keep-alive activity detecting a failure
			// while one or more operations are in progress.
			// Operations that were in progress fail with
			// WSAENETRESET. Subsequent operations fail with
			// WSAECONNRESET.

			*rval = JSVAL_VOID; // TCP connection reset by peer
			return JS_TRUE;
		}
*/ // find a better solution !

		if ( errCode != PR_WOULD_BLOCK_ERROR )
			return ThrowIoError(cx);
		sentAmount = 0;
	} else
		sentAmount = res;

	char *buffer;
	if ( sentAmount < len ) {
		//*rval = STRING_TO_JSVAL( JS_NewDependentString(cx, JSVAL_TO_STRING( J_ARG(1) ), sentAmount, len - sentAmount) ); // return unsent data // (TBD) use Blob ?
		
		buffer = (char*)JS_malloc(cx, len - sentAmount +1);
		J_S_ASSERT_ALLOC(buffer);
		buffer[len - sentAmount] = '\0';
		memcpy(buffer, str, len - sentAmount);
		J_CHKB( J_NewBlob(cx, buffer, len - sentAmount, rval), bad_free );
	} else
	if ( sentAmount == 0 ) {

		*rval = J_ARG(1); // nothing has been sent
	} else {

		*rval = JS_GetEmptyStringValue(cx); // nothing remains
	}

	return JS_TRUE;
bad_free:
	JS_free(cx, buffer);
	JL_BAD;
}


/**doc
 * *Sync*()
  Sync any buffered data for a fd to its backing device.
**/
DEFINE_FUNCTION( Sync ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	J_S_ASSERT_RESOURCE( fd );
	J_CHKB( PR_Sync(fd) == PR_SUCCESS, bad_ioerror );
	return JS_TRUE;
bad_ioerror:
	ThrowIoError(cx);
	JL_BAD;
}


/**doc
=== Properties ===
**/

/**doc
 * *available* $READONLY
  Determine the amount of data in bytes available for reading on the descriptor.
 **/
DEFINE_PROPERTY( available ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	J_S_ASSERT_RESOURCE( fd );

	PRInt64 available;
	available = PR_Available64( fd ); // For a normal file, these are the bytes beyond the current file pointer.
	J_CHKB( available != -1, bad_ioerror );

	if ( available <= JSVAL_INT_MAX )
		*vp = INT_TO_JSVAL(available);
	else
		J_CHK( JS_NewNumberValue(cx, (jsdouble)available, vp) );

	return JS_TRUE;
bad_ioerror:
	ThrowIoError(cx);
	JL_BAD;
}


/**doc
 * *type* $READONLY
  see constants.
**/
DEFINE_PROPERTY( type ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	J_S_ASSERT_RESOURCE( fd );
	*vp = INT_TO_JSVAL( (int)PR_GetDescType(fd) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
 * *closed* $READONLY
  Is true if the file descriptor has been closed.
  ===== beware: =====
   Do not confuse with disconnected.
**/
DEFINE_PROPERTY( closed ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	*vp = BOOLEAN_TO_JSVAL( fd == NULL );
	return JS_TRUE;
}

/**doc
=== Static functions ===
**/

/**doc
 * *Import*( nativeDescriptor, type )
**/
DEFINE_FUNCTION( Import ) {

	J_S_ASSERT_ARG_MIN(2);
	//int stdfd;
	//J_CHK( JsvalToInt(cx, J_ARG(1), &stdfd) );

	PRInt32 osfd;
	J_CHK( JsvalToInt(cx, J_ARG(1), &osfd) );

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
	//		//J_REPORT_ERROR("Unsupported standard handle.");
	//}

	int descType;
	J_CHK( JsvalToInt(cx, J_ARG(2), &descType) );
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
			J_REPORT_ERROR("Invalid descriptor type.");
	}
	if ( fd == NULL )
		return ThrowIoError(cx);

	J_S_ASSERT_ALLOC( descriptorObject );
	J_CHK( JS_SetPrivate(cx, descriptorObject, (void*)fd) );
	J_CHK( JS_SetReservedSlot(cx, descriptorObject, SLOT_JSIO_DESCRIPTOR_IMPORTED, JSVAL_TRUE) );

	*rval = OBJECT_TO_JSVAL(descriptorObject);
	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Constants ===
 * *`DESC_FILE`*

 * *`DESC_SOCKET_TCP`*

 * *`DESC_SOCKET_UDP`*

 * *`DESC_LAYERED`*

 * *`DESC_PIPE`*
**/

/**doc
=== Native Interface ===
 *NIStreamRead*: Read the file as a stream.
**/


CONFIGURE_CLASS

	HAS_CONSTRUCTOR

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
