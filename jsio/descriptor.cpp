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

#include <pprio.h> // nspr/include/nspr/private

#include "error.h"
#include "descriptor.h"
#include "file.h"

// open: 	SetNativeInterface(cx, obj, NI_READ_RESOURCE, (FunctionPointer)NativeInterfaceReadFile, fd);
// close: 	RemoveNativeInterface(cx, obj, NI_READ_RESOURCE );


bool NativeInterfaceReadDescriptor( void *pv, unsigned char *buf, unsigned int *amount ) {

	PRFileDesc *fd = (PRFileDesc *)pv;
	PRInt32 status;
	// because this socket class is non-blocking and NativeInterfaceReadFile cannot manage non-blocking socket, 
	// we simulate blocking socket using poll() function. the maximum timeout is 10 seconds.
	// (TBD) check again !
	PRPollDesc desc;
	desc.fd = fd;
	desc.in_flags = PR_POLL_READ;
	desc.out_flags = 0;
	status = PR_Poll( &desc, 1, PR_MillisecondsToInterval(10000) ); // wait for data
	if ( status == -1 ) // if PR_Poll is not compatible with the file descriptor, just ignore the error ?
		return false;

	if ( status == 0 ) { // The value 0 indicates the function timed out.

		*amount = 0;
		return true; // no error, but no data
	}
	PRInt32 tmp = *amount;
	status = PR_Recv( fd, buf, tmp, 0, PR_INTERVAL_NO_WAIT );
	*amount = tmp;
	if ( status == -1 )
		return false;
	*amount = status;
	return true;


/*
	PRInt32 status = PR_Read( (PRFileDesc *)pv, buf, *amount );
	if ( status == -1 )
		return false;
	*amount = status;
	return true;
*/
}


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


DEFINE_FUNCTION( Close ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	RT_ASSERT( fd != NULL, "file is closed." );
	PRStatus status = PR_Close( fd );
	if (status != PR_SUCCESS) {

		PRErrorCode errorCode = PR_GetError();
		if ( errorCode != PR_WOULD_BLOCK_ERROR ) // if non-blocking descriptor, this is a non-fatal error
			JS_ReportError( cx, "A descriptor cannot be closed." );
	}
	JS_SetPrivate( cx, obj, NULL );
	JS_ClearScope( cx, obj ); // help to clear readable, writable, exception
	RemoveNativeInterface(cx, obj, NI_READ_RESOURCE );

	return JS_TRUE;
}

JSBool ReadToJsval(JSContext *cx, PRFileDesc *fd, int amount, jsval *rval ) {

	char *buf = (char*)JS_malloc( cx, amount + 1 );
	RT_ASSERT_ALLOC(buf);
	buf[amount] = 0; // (TBD) check if useful: PR_Read can read binary data !

	PRInt32 res = PR_Read( fd, buf, amount );

	if (res == -1) { // failure. The reason for the failure can be obtained by calling PR_GetError.

		JS_free( cx, buf );
		return ThrowIoError( cx, PR_GetError() );
	}

	if (res == 0) {

		JS_free( cx, buf );
		*rval = JS_GetEmptyStringValue(cx); // (TBD) check if it is realy faster.
		return JS_TRUE;
	}

	JSString *str = JS_NewString( cx, (char*)buf, res );
	RT_ASSERT_ALLOC(str);
	*rval = STRING_TO_JSVAL(str); // GC protection is ok with this ?
	return JS_TRUE;
}


JSBool ReadAllToJsval(JSContext *cx, PRFileDesc *fd, jsval *rval ) {

	int totalLength = 0;
	int chunkListTotalLength = 32; // initial value (chunks), will evolve at runtime
	int chunkListContentLength = 0;
	char **chunkList = (char **)malloc(chunkListTotalLength * sizeof(char*));
	int currrentReadLength = 1024;

	PRInt32 res;
	do {
		if ( chunkListContentLength >= chunkListTotalLength ) {

			chunkListTotalLength *= 2;
			chunkList = (char**)realloc(chunkList, chunkListTotalLength * sizeof(char*));
		}
		//	currrentReadLength = currrentReadLength < 16384 ? 2048 + 1024 * chunkListContentLength : 16384; // 2048, 3072, 4096, 5120, ..., 16384
		char *chunk = (char *)malloc(sizeof(int) + currrentReadLength);  // chunk format: int + data ...
		chunkList[chunkListContentLength++] = chunk;
		res = PR_Read( fd, chunk + sizeof(int), currrentReadLength ); // chunk + sizeof(int) gives the position where the data can be written. Size to read is currrentReadLength
		if (res == -1) { // failure. The reason for the failure can be obtained by calling PR_GetError.

			while ( chunkListContentLength )
				free(chunkList[--chunkListContentLength]);
			free(chunkList);
			return ThrowIoError( cx, PR_GetError() );
		}
		*(int*)chunk = res;
		totalLength += res;

	} while ( res == currrentReadLength );

	char *jsData = (char*)JS_malloc(cx, totalLength +1);
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

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( fd );

	if ( argc >= 1 ) { // amount value is provided
		
		PRInt32 amount;
		RT_JSVAL_TO_INT32( argv[0], amount );
		RT_CHECK_CALL( ReadToJsval(cx, fd, amount, rval) );
	} else { // amount value is NOT provided, then try to read all

		PRInt32 available = PR_Available( fd );
		if ( available != -1 ) // we can use the 'available' information
			RT_CHECK_CALL( ReadToJsval(cx, fd, available, rval) )
		else
			RT_CHECK_CALL( ReadAllToJsval(cx, fd, rval) )
	}
	return JS_TRUE;
}


DEFINE_FUNCTION( Write ) {

	RT_ASSERT_ARGC(1);
	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( fd );
	char *str;
	int len;
	RT_JSVAL_TO_STRING_AND_LENGTH( argv[0], str, len );
	PRInt32 res = PR_Write( fd, str, len );
	if ( res == -1 )
		return ThrowIoError( cx, PR_GetError() );
	if ( res < len )
		*rval = STRING_TO_JSVAL( JS_NewDependentString(cx, JSVAL_TO_STRING( argv[0] ), res, len - res) );
	else
		*rval = JS_GetEmptyStringValue(cx);
	return JS_TRUE;
}


DEFINE_PROPERTY( available ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( fd );
	PRInt32 available = PR_Available( fd ); // For a normal file, these are the bytes beyond the current file pointer.
	if ( available == -1 )
		return ThrowIoError( cx, PR_GetError() );
	*vp = INT_TO_JSVAL(available);
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
	PRInt32 osfd;
	RT_JSVAL_TO_INT32( argv[0], osfd );
	int tmp;
	RT_JSVAL_TO_INT32( argv[1], tmp );
	PRDescType type = (PRDescType)tmp;
	PRFileDesc *fd;
	switch (type) {
		case PR_DESC_FILE:
			fd = PR_ImportFile(osfd);
			break;
		case PR_DESC_SOCKET_TCP:
			fd = PR_ImportTCPSocket(osfd);
			break;
		case PR_DESC_SOCKET_UDP:
			fd = PR_ImportUDPSocket(osfd);
			break;
		case PR_DESC_PIPE:
			fd = PR_ImportPipe(osfd);
			break;
		default:
			REPORT_ERROR("Invalid descriptor type.");
	}
	if ( fd == NULL )
		return ThrowIoError( cx, PR_GetError() );

	JSObject *descriptorObject = JS_NewObject(cx, &classFile, NULL, NULL); // (TBD) chack if proto is needed !
	RT_ASSERT_ALLOC( descriptorObject ); 
	RT_CHECK_CALL( JS_SetPrivate(cx, descriptorObject, (void*)fd) );
	RT_CHECK_CALL( JS_SetReservedSlot(cx, descriptorObject, SLOT_JSIO_DESCRIPTOR_IMPORTED, JSVAL_TRUE) );
	return JS_TRUE;
}


CONFIGURE_CLASS

	BEGIN_FUNCTION_SPEC
		FUNCTION( Close )
		FUNCTION( Read )
		FUNCTION( Write )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ( available )
		PROPERTY_READ( type )
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
