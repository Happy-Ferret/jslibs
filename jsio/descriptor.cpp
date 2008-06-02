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

#include "../jslang/streamapi.h"

#include <pprio.h> // nspr/include/nspr/private
#include <string.h>

#include "descriptor.h"
#include "file.h"
#include "socket.h"

// open: 	SetNativeInterface(cx, obj, ...
// close: 	RemoveNativeInterface(cx, obj, NI_READ_RESOURCE );


JSBool NativeInterfaceStreamRead( JSContext *cx, JSObject *obj, char *buf, size_t *amount ) {

	PRFileDesc *fd = (PRFileDesc*)JS_GetPrivate(cx, obj); // (PRFileDesc *)pv;
	J_S_ASSERT_RESOURCE(fd);

	PRInt32 ret;
	PRPollDesc desc = { fd, PR_POLL_READ, 0 };
	ret = PR_Poll( &desc, 1, PR_SecondsToInterval(10) ); // wait 10 seconds for data
	if ( ret == -1 ) // if PR_Poll is not compatible with the file descriptor, just ignore the error ?
		return ThrowIoError(cx); // returns later	

	if ( ret == 0 ) { // timeout

		*amount = 0;
		return JS_TRUE; // no error, but no data
	}

	ret = PR_Read(fd, buf, *amount);
	if ( ret == -1 ) {

		PRErrorCode errorCode = PR_GetError();
		if ( errorCode != PR_WOULD_BLOCK_ERROR )// if non-blocking descriptor, this is a non-fatal error
			return ThrowIoError(cx); // real error			

		*amount = 0;
		return JS_TRUE; // no error, but no data
	}
	
	if ( ret == 0 ) { // end of file / socket
		
		// (TBD) ?
	}

	*amount = ret;
	return JS_TRUE;
}

extern NIStreamRead pNativeInterfaceStreamRead = NativeInterfaceStreamRead;


void FinalizeDescriptor(JSContext *cx, JSObject *obj) {

	PRFileDesc *fd = (PRFileDesc*)JS_GetPrivate( cx, obj );
	if ( fd != NULL ) { // check if not already closed
		
		jsval imported;
		JS_GetReservedSlot(cx, obj, SLOT_JSIO_DESCRIPTOR_IMPORTED, &imported);
		if ( imported != JSVAL_TRUE ) { // Descriptor was inported, then do not close it

			PRStatus status = PR_Close( fd ); // what to do on error ??
			if (status != PR_SUCCESS) {

				PRErrorCode errorCode = PR_GetError();
				if ( errorCode != PR_WOULD_BLOCK_ERROR ) // if non-blocking descriptor, this is a non-fatal error
					JS_ReportError( cx, "A descriptor cannot be closed while Finalize." );
			}
			JS_SetPrivate( cx, obj, NULL );
		}
	}
}


BEGIN_CLASS( Descriptor )


DEFINE_CONSTRUCTOR() {

	J_REPORT_ERROR( J__ERRMSG_NO_CONSTRUCT ); // BUT constructor must be defined
}


DEFINE_FUNCTION( Close ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	RT_ASSERT( fd != NULL, "file is closed." );
	PRStatus status = PR_Close( fd );
	if (status != PR_SUCCESS) {

		PRErrorCode errorCode = PR_GetError();
		if ( errorCode != PR_WOULD_BLOCK_ERROR ) // if non-blocking descriptor, this is a non-fatal error
			return ThrowIoError(cx);
	}
	JS_SetPrivate( cx, obj, NULL );
//	JS_ClearScope( cx, obj ); // help to clear readable, writable, exception
//	J_CHECK_CALL( SetStreamReadInterface(cx, obj, NULL) );
	J_CHK( SetStreamReadInterface(cx, obj, NULL) );
	return JS_TRUE;
}


JSBool ReadToJsval(JSContext *cx, PRFileDesc *fd, int amount, jsval *rval ) {

	char *buf = (char*)JS_malloc( cx, amount + 1 );
	RT_ASSERT_ALLOC(buf);
	buf[amount] = '\0';
// (TBD) use BString
//	JSObject bstringObj = NewBString(cx, buf, amount);
//	*rval = OBJECT_TO_JSVAL(bstringObj);

	PRInt32 res = PR_Read( fd, buf, amount );

	if (res == -1) { // failure. The reason for the failure can be obtained by calling PR_GetError.

		JS_free( cx, buf );

		PRErrorCode errCode = PR_GetError();
		if ( errCode != PR_WOULD_BLOCK_ERROR )
			return ThrowIoError(cx);
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
		RT_ASSERT_ALLOC(buf);
	}

	JSString *str = JS_NewString(cx, buf, res);
	RT_ASSERT_ALLOC(str);
	*rval = STRING_TO_JSVAL(str); // GC protection is ok with this ?
	return JS_TRUE;
}


JSBool ReadAllToJsval(JSContext *cx, PRFileDesc *fd, jsval *rval ) {

	int totalLength = 0;
	int chunkListTotalLength = 32; // initial value (chunks), will evolve at runtime
	int chunkListContentLength = 0;
	char **chunkList = (char **)malloc(chunkListTotalLength * sizeof(char*));
	int currentReadLength = 1024;

	PRInt32 receivedAmount;
	do {
		if ( chunkListContentLength >= chunkListTotalLength ) {

			chunkListTotalLength *= 2;
			chunkList = (char**)realloc(chunkList, chunkListTotalLength * sizeof(char*));
		}
		//	currentReadLength = currentReadLength < 16384 ? 2048 + 1024 * chunkListContentLength : 16384; // 2048, 3072, 4096, 5120, ..., 16384
		char *chunk = (char *)malloc(sizeof(int) + currentReadLength);  // chunk format: int + data ...
		chunkList[chunkListContentLength++] = chunk;

		PRInt32 res = PR_Read( fd, chunk + sizeof(int), currentReadLength ); // chunk + sizeof(int) gives the position where the data can be written. Size to read is currentReadLength
		
		if ( res > 0 ) {

			receivedAmount = res;
		} else if ( res == -1 ) { // failure. The reason for the failure can be obtained by calling PR_GetError.

			PRErrorCode errCode = PR_GetError();
			if ( errCode != PR_WOULD_BLOCK_ERROR ) {

				while ( chunkListContentLength )
					free(chunkList[--chunkListContentLength]);
				free(chunkList);
				return ThrowIoError(cx);
			}
			free(chunkList[--chunkListContentLength]); // cancel the last chunk
			receivedAmount = 0;
			break; // no error, no data received, we cannot reach currentReadLength
		} else if ( res == 0 ) { // end of file/socket
			
			if ( totalLength > 0 ) { // we reach eof BUT we have read some data.

				free(chunkList[--chunkListContentLength]); // cancel the last chunk
				receivedAmount = 0;
				break; // no error, no data received, we cannot reach currentReadLength
			} else {

				free(chunkList[--chunkListContentLength]); // cancel the last chunk
				free(chunkList);
				*rval = JSVAL_VOID;
				return JS_TRUE;
			}
		}
		*(int*)chunk = receivedAmount;
		totalLength += receivedAmount;
	} while ( receivedAmount == currentReadLength );


	if ( totalLength == 0 ) { // PR_WOULD_BLOCK_ERROR and NOT end of file/socket

		while ( chunkListContentLength )
			free(chunkList[--chunkListContentLength]);
		free(chunkList);
		*rval = JS_GetEmptyStringValue(cx);
		return JS_TRUE;
	}

// (TBD) use BString
	char *jsData = (char*)JS_malloc(cx, totalLength + 1);
	jsData[totalLength] = '\0';
	char *ptr = jsData + totalLength; // starts from the end
	while ( chunkListContentLength ) {

		char *chunk = chunkList[--chunkListContentLength];
		int chunkLength = *(int*)chunk;
		ptr -= chunkLength;
		memcpy(ptr, chunk + sizeof(int), chunkLength);
		free(chunk);
	}
	free(chunkList);
	JSString *jsstr = JS_NewString(cx, jsData, totalLength);
	RT_ASSERT_ALLOC(jsstr);
	*rval = STRING_TO_JSVAL( jsstr );
	return JS_TRUE;
}


DEFINE_FUNCTION( Read ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate(cx, obj);
	RT_ASSERT_RESOURCE( fd );

	if ( J_ARG_ISDEF(1) ) {
		
		PRInt32 amount;
		RT_JSVAL_TO_INT32( J_ARG(1), amount );

//		if ( amount == 0 ) // (TBD) check if it is good to use this ( even if amount is 0, we must call Read
//			*rval = JS_GetEmptyStringValue(cx);
//		else
			
		RT_CHECK_CALL( ReadToJsval(cx, fd, amount, rval) );

	} else { // amount value is NOT provided, then try to read all

		PRInt32 available = PR_Available( fd );
		if ( available != -1 ) // we can use the 'available' information
			RT_CHECK_CALL( ReadToJsval(cx, fd, available, rval) );
		else // 'available' is not usable with this fd type, then we used a buffered read ( aka read while there is someting to read )
			RT_CHECK_CALL( ReadAllToJsval(cx, fd, rval) );
	}
	return JS_TRUE;
}


DEFINE_FUNCTION( Write ) {

	RT_ASSERT_ARGC( 1 );
	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( fd );
	const char *str;
	size_t len;
	RT_JSVAL_TO_STRING_AND_LENGTH( J_ARG(1), str, len );

	size_t sentAmount;

	PRInt32 res = PR_Write( fd, str, len );
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

	if ( sentAmount < len )
		*rval = STRING_TO_JSVAL( JS_NewDependentString(cx, JSVAL_TO_STRING( J_ARG(1) ), sentAmount, len - sentAmount) ); // return unsent data
	else if ( sentAmount == 0 )
		*rval = J_ARG(1); // nothing has been sent
	else
		*rval = JS_GetEmptyStringValue(cx); // nothing remains
	return JS_TRUE;
}


DEFINE_FUNCTION( Sync ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( fd );

	PRStatus status = PR_Sync(fd);
	if ( status == PR_FAILURE )
		return ThrowIoError(cx);

	return JS_TRUE;
}


DEFINE_PROPERTY( available ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( fd );

	PRInt64 available = PR_Available64( fd ); // For a normal file, these are the bytes beyond the current file pointer.
	if ( available == -1 )
		return ThrowIoError(cx);

	if ( available <= JSVAL_INT_MAX )
		*vp = INT_TO_JSVAL(available);
	else
		JS_NewNumberValue(cx, (jsdouble)available, vp);

	return JS_TRUE;
}


DEFINE_PROPERTY( type ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( fd );
	*vp = INT_TO_JSVAL( (int)PR_GetDescType(fd) );
	return JS_TRUE;
}


DEFINE_FUNCTION( Import ) {

	RT_ASSERT_ARGC(2);
	int stdfd;
	RT_JSVAL_TO_INT32( J_ARG(1), stdfd );
	PRInt32 osfd;
	switch (stdfd) {
		case 0:
			osfd = PR_FileDesc2NativeHandle(PR_STDIN);
			break;
		case 1:
			osfd = PR_FileDesc2NativeHandle(PR_STDOUT);
			break;
		case 2:
			osfd = PR_FileDesc2NativeHandle(PR_STDERR);
			break;
		default:
			REPORT_ERROR("Unsupported standard handle.");
	}

	int tmp;
	RT_JSVAL_TO_INT32( J_ARG(2), tmp );
	PRDescType type = (PRDescType)tmp;

	PRFileDesc *fd;
	JSObject *descriptorObject;

	switch (type) {
		case PR_DESC_FILE:
			fd = PR_ImportFile(osfd);
			descriptorObject = JS_NewObject(cx, &classFile, NULL, NULL); // (TBD) check if proto is needed !
			break;
		case PR_DESC_SOCKET_TCP:
			fd = PR_ImportTCPSocket(osfd);
			descriptorObject = JS_NewObject(cx, &classSocket, NULL, NULL); // (TBD) check if proto is needed !
			break;
		case PR_DESC_SOCKET_UDP:
			fd = PR_ImportUDPSocket(osfd);
			descriptorObject = JS_NewObject(cx, &classSocket, NULL, NULL); // (TBD) check if proto is needed !
			break;
		case PR_DESC_PIPE:
			fd = PR_ImportPipe(osfd);
			descriptorObject = JS_NewObject(cx, &classFile, NULL, NULL); // (TBD) check if proto is needed !
			break;
		default:
			REPORT_ERROR("Invalid descriptor type.");
	}
	if ( fd == NULL )
		return ThrowIoError(cx);

	RT_ASSERT_ALLOC( descriptorObject ); 
	RT_CHECK_CALL( JS_SetPrivate(cx, descriptorObject, (void*)fd) );
	RT_CHECK_CALL( JS_SetReservedSlot(cx, descriptorObject, SLOT_JSIO_DESCRIPTOR_IMPORTED, JSVAL_TRUE) );

	*rval = OBJECT_TO_JSVAL(descriptorObject);
	return JS_TRUE;
}


DEFINE_PROPERTY( closed ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	*vp = BOOLEAN_TO_JSVAL( fd == NULL );
	return JS_TRUE;
}


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
