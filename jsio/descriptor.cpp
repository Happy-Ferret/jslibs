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

#include "error.h"
#include "descriptor.h"
#include "file.h"

BEGIN_CLASS( Descriptor )

DEFINE_FUNCTION( Close ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	RT_ASSERT( fd != NULL, "file is closed." );
	PRStatus st = PR_Close( fd );
	if ( st == PR_FAILURE )
		return ThrowIoError( cx, PR_GetError() );
	JS_SetPrivate( cx, obj, NULL );
	return JS_TRUE;
}

DEFINE_FUNCTION( Read ) {

	RT_ASSERT_ARGC( 1 );
	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( fd );
	PRInt32 amount;
	RT_JSVAL_TO_INT32( argv[0], amount );
	char *buf = (char*)JS_malloc( cx, amount +1 );
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


DEFINE_FUNCTION( Write ) {

	RT_ASSERT_ARGC(1);

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( fd );

	char *str;
	int len;
	RT_JSVAL_TO_STRING_AND_LENGTH( argv[0], str, len );

	PRInt32 bytesSent = PR_Write( fd, str, len );

	if ( bytesSent == -1 )
		return ThrowIoError( cx, PR_GetError() );

	if ( bytesSent < len )
		*rval = STRING_TO_JSVAL( JS_NewDependentString(cx, JSVAL_TO_STRING( argv[0] ), bytesSent, len-bytesSent) );
	else
		*rval = JS_GetEmptyStringValue(cx);

	return JS_TRUE;
}



DEFINE_FUNCTION( ReadAll ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( fd );

	int totalLength = 0;
	int chunkListTotalLength = 32; // initial value, will evolve at runtime
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



DEFINE_PROPERTY( type ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( fd );
	*vp = INT_TO_JSVAL( (int)PR_GetDescType(fd) );
	return JS_TRUE;
}



DEFINE_FUNCTION( ImportFile ) {

	RT_ASSERT_ARGC(1);
	PRInt32 osfd;
	RT_JSVAL_TO_INT32( argv[0], osfd );
	JSObject *descriptorObject = JS_NewObject(cx, &classFile, NULL, NULL); // (TBD) chack if proto is needed !
	RT_ASSERT_ALLOC( descriptorObject ); 
	PRFileDesc *fd = PR_ImportFile(osfd);
	if ( fd == NULL )
		return ThrowIoError( cx, PR_GetError() );
	RT_CHECK_CALL( JS_SetPrivate(cx, descriptorObject, (void*)fd) );
	RT_CHECK_CALL( JS_SetReservedSlot(cx, descriptorObject, SLOT_JSIO_DESCRIPTOR_IMPORTED, JSVAL_TRUE) );
	return JS_TRUE;
}

DEFINE_FUNCTION( ImportPipe ) {

	RT_ASSERT_ARGC(1);
	PRInt32 osfd;
	RT_JSVAL_TO_INT32( argv[0], osfd );
	JSObject *descriptorObject = JS_NewObject(cx, &classFile, NULL, NULL); // (TBD) chack if proto is needed !
	RT_ASSERT_ALLOC( descriptorObject ); 
	PRFileDesc *fd = PR_ImportPipe(osfd);
	if ( fd == NULL )
		return ThrowIoError( cx, PR_GetError() );
	RT_CHECK_CALL( JS_SetPrivate(cx, descriptorObject, (void*)fd) );
	RT_CHECK_CALL( JS_SetReservedSlot(cx, descriptorObject, SLOT_JSIO_DESCRIPTOR_IMPORTED, JSVAL_TRUE) );
	return JS_TRUE;
}


DEFINE_FUNCTION( ImportTCPSocket ) {

	RT_ASSERT_ARGC(1);
	PRInt32 osfd;
	RT_JSVAL_TO_INT32( argv[0], osfd );
	JSObject *descriptorObject = JS_NewObject(cx, &classFile, NULL, NULL); // (TBD) chack if proto is needed !
	RT_ASSERT_ALLOC( descriptorObject ); 
	PRFileDesc *fd = PR_ImportTCPSocket(osfd);
	if ( fd == NULL )
		return ThrowIoError( cx, PR_GetError() );
	RT_CHECK_CALL( JS_SetPrivate(cx, descriptorObject, (void*)fd) );
	RT_CHECK_CALL( JS_SetReservedSlot(cx, descriptorObject, SLOT_JSIO_DESCRIPTOR_IMPORTED, JSVAL_TRUE) );
	return JS_TRUE;
}


DEFINE_FUNCTION( ImportUDPSocket ) {

	RT_ASSERT_ARGC(1);
	PRInt32 osfd;
	RT_JSVAL_TO_INT32( argv[0], osfd );
	JSObject *descriptorObject = JS_NewObject(cx, &classFile, NULL, NULL); // (TBD) chack if proto is needed !
	RT_ASSERT_ALLOC( descriptorObject ); 
	PRFileDesc *fd = PR_ImportUDPSocket(osfd);
	if ( fd == NULL )
		return ThrowIoError( cx, PR_GetError() );
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
		PROPERTY_READ( type )
	END_PROPERTY_SPEC

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION( ImportFile )
		FUNCTION( ImportPipe )
		FUNCTION( ImportTCPSocket )
		FUNCTION( ImportUDPSocket )
		FUNCTION( Read )
		FUNCTION( Write )
	END_STATIC_FUNCTION_SPEC

	BEGIN_CONST_DOUBLE_SPEC
		CONST_DOUBLE( DESC_FILE			,PR_DESC_FILE )
		CONST_DOUBLE( DESC_SOCKET_TCP	,PR_DESC_SOCKET_TCP )
		CONST_DOUBLE( DESC_SOCKET_UDP ,PR_DESC_SOCKET_UDP )
		CONST_DOUBLE( DESC_LAYERED	   ,PR_DESC_LAYERED )
	END_CONST_DOUBLE_SPEC

END_CLASS
