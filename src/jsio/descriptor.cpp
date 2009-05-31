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

	JL_S_ASSERT( JL_InheritFrom(cx, obj, classDescriptor), "Invalid descriptor object." );
//	JL_S_ASSERT_CLASS(obj, classDescriptor);

	PRFileDesc *fd;
	fd = (PRFileDesc*)JL_GetPrivate(cx, obj); // (PRFileDesc *)pv;
	JL_S_ASSERT_RESOURCE(fd);

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

	JL_S_ASSERT( fd != NULL, "The descriptor is closed." ); // see PublicApiRules (http://code.google.com/p/jslibs/wiki/PublicApiRules)
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
	//	JS_ClearScope( cx, obj ); // help to clear readable, writable, exception ?
	JL_CHK( SetStreamReadInterface(cx, JL_FOBJ, NULL) );
	return JS_TRUE;
	JL_BAD;
}


JSBool ReadToJsval( JSContext *cx, PRFileDesc *fd, int amount, jsval *rval ) {

	char *buf = (char*)JS_malloc(cx, amount +1);
	JL_CHK( buf );

	PRInt32 res;
	res = PR_Read(fd, buf, amount);

	if (likely( res > 0 )) {

		if (unlikely( JL_MaybeRealloc(amount, res) )) {

			buf = (char*)JS_realloc(cx, buf, res +1); // realloc the string using its real size
			JL_CHK( buf );
		}
		buf[res] = '\0';
		JL_CHKB( JL_NewBlob(cx, buf, res, rval), bad_free );
		return JS_TRUE;
	} else
	if (res == 0) {

		JS_free( cx, buf );
		*rval = JSVAL_VOID; // end of file/socket
		return JS_TRUE;
	} else
	if (unlikely( res == -1 )) { // failure. The reason for the failure can be obtained by calling PR_GetError.

		JS_free( cx, buf );
		if (unlikely( PR_GetError() != PR_WOULD_BLOCK_ERROR ))
			JL_CHK( ThrowIoError(cx) );
		*rval = JS_GetEmptyStringValue(cx); // mean no data available, but connection still established.
		return JS_TRUE;
	}

bad_free:
	JS_free(cx, buf);
	JL_BAD;
}


JSBool ReadAllToJsval(JSContext *cx, PRFileDesc *fd, jsval *rval ) {

	Buffer buf;
	BufferInitialize(&buf, bufferTypeChunk, bufferGrowTypeNoGuess);
	PRInt32 currentReadLength = 1024;
	for (;;) {

		PRInt32 res = PR_Read(fd, BufferNewChunk(&buf, currentReadLength), currentReadLength);
		if (likely( res > 0 )) {

			BufferConfirm(&buf, res);
			if ( res < currentReadLength )
				break;
		} else
		if ( res == 0 ) { // end of file is reached or the network connection is closed.

			if ( BufferGetLength(&buf) > 0 ) // we reach eof BUT we have read some data.
				break; // no error, no data received, we cannot reach currentReadLength

			BufferFinalize(&buf);
			*rval = JSVAL_VOID;
			return JS_TRUE;
		} else
		if (unlikely( res == -1 )) { // failure. The reason for the failure can be obtained by calling PR_GetError.

			if ( PR_GetError() != PR_WOULD_BLOCK_ERROR )
				JL_CHK( ThrowIoError(cx) );
			break; // no error, no data received, we cannot reach currentReadLength
		}

//		if ( res == currentReadLength ) {
//
			if ( currentReadLength < 32768 )
				currentReadLength *= 2;
//		} else {
//
//			if ( currentReadLength > 1024 )
//				currentReadLength /= 2;
//		}
	}

	if ( BufferGetLength(&buf) == 0 ) { // no data but NOT end of file/socket

		BufferFinalize(&buf);
		*rval = JS_GetEmptyStringValue(cx);
		return JS_TRUE;
	}

	*BufferNewChunk(&buf, 1) = '\0';
	BufferConfirm(&buf, 1);
	JL_CHK( JL_NewBlob(cx, BufferGetDataOwnership(&buf), BufferGetLength(&buf) -1, rval) );

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

		PRInt32 available = PR_Available( fd );
		if (likely( available != -1 )) // we can use the 'available' information
			JL_CHK( ReadToJsval(cx, fd, available, JL_FRVAL) ); // may block !
		else // 'available' is not usable with this fd type, then we use a buffered read (ie. read while there is someting to read)
			JL_CHK( ReadAllToJsval(cx, fd, JL_FRVAL) );

	} else { // amount value is NOT provided, then try to read all

		if (likely( !JSVAL_IS_VOID(JL_FARG(1)) )) {

			PRInt32 amount;
			JL_CHK( JsvalToInt(cx, JL_FARG(1), &amount) );
			JL_CHK( ReadToJsval(cx, fd, amount, JL_FRVAL) ); // (TBD) check if it is good to call it even if amount is 0.
		} else {

			JL_CHK( ReadAllToJsval(cx, fd, JL_FRVAL) ); // may block !
		}
	}

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( data )
  If the whole data cannot be written, Write returns that have not be written.
**/
DEFINE_FUNCTION_FAST( Write ) {

	JL_S_ASSERT_ARG_MIN( 1 );
	PRFileDesc *fd;
	fd = (PRFileDesc *)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE( fd );
	const char *str;
	size_t len;
	JL_CHK( JsvalToStringAndLength(cx, &JL_FARG(1), &str, &len) );

	size_t sentAmount;

	PRInt32 res;
	res = PR_Write( fd, str, len );
	if (unlikely( res == -1 )) {

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
*/ // find a better solution !?

		if ( errCode != PR_WOULD_BLOCK_ERROR )
			return ThrowIoError(cx);
		sentAmount = 0;
	} else {

		sentAmount = res;
	}

	// (TBD) try to detect if the return value will be used else just return.
	//	js_Disassemble1(cx, JL_CurrentStackFrame(cx)->script, JL_CurrentStackFrame(cx)->regs->pc +3, 0, false, stdout); // 00000:  setgvar "test"

	char *buffer;

	if (likely( sentAmount == len )) {

		*JL_FRVAL = JS_GetEmptyStringValue(cx); // nothing remains
	} else
	if (unlikely( sentAmount < len )) {

		//*rval = STRING_TO_JSVAL( JS_NewDependentString(cx, JSVAL_TO_STRING( JL_ARG(1) ), sentAmount, len - sentAmount) ); // return unsent data // (TBD) use Blob ?

		buffer = (char*)JS_malloc(cx, len - sentAmount +1);
		JL_CHK( buffer );
		buffer[len - sentAmount] = '\0';
		memcpy(buffer, str, len - sentAmount);
		JL_CHKB( JL_NewBlob(cx, buffer, len - sentAmount, JL_FRVAL), bad_free );
	} else
	if ( sentAmount == 0 ) {

		*JL_FRVAL = JL_FARG(1); // nothing has been sent
	}

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
	*JL_FRVAL = JSVAL_VOID;
	JL_CHKB( PR_Sync(fd) == PR_SUCCESS, bad_ioerror );
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
	JL_CHKB( available != -1, bad_ioerror );

	if ( available <= JSVAL_INT_MAX )
		*vp = INT_TO_JSVAL(available);
	else
		JL_CHK( JS_NewNumberValue(cx, (jsdouble)available, vp) );

	return JS_TRUE;
bad_ioerror:
	ThrowIoError(cx);
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
