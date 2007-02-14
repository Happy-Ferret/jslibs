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

#include "nsprError.h"
#include "nsprFile.h"

#include "../common/jshelper.h"
#include "../common/jsNativeInterface.h"


static bool NativeInterfaceReadFile( void *pv, unsigned char *buf, unsigned int *amount ) {

	PRInt32 status = PR_Read( (PRFileDesc *)pv, buf, *amount );
	if ( status == -1 )
		return false;
	*amount = status;
	return true;
}


BEGIN_CLASS( File )

DEFINE_FINALIZE() {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	if ( fd != NULL ) {

		PRStatus status = PR_Close( fd ); // what to do on error ??
		if ( status == PR_FAILURE )
			JS_ReportError( cx, "a file descriptor cannot be closed while Finalize" );
		JS_SetPrivate( cx, obj, NULL );
	}
}

DEFINE_CONSTRUCTOR() {

	RT_ASSERT_CONSTRUCTING( &classFile );
	RT_ASSERT_ARGC(1);
	JS_SetReservedSlot( cx, obj, SLOT_NSPR_FILE_NAME, argv[0] );
	return JS_TRUE;
}

DEFINE_FUNCTION( Open ) {

	RT_ASSERT_ARGC(1);

	jsval jsvalFileName;
	JS_GetReservedSlot( cx, obj, SLOT_NSPR_FILE_NAME, &jsvalFileName );
	RT_ASSERT_DEFINED( jsvalFileName );
	char *fileName;
	RT_JSVAL_TO_STRING(jsvalFileName, fileName);

	PRIntn flags;
	int32 tmp;
	JS_ValueToInt32( cx, argv[0], &tmp );
	flags = tmp;

	PRIntn mode = 0666;

	PRFileDesc *fd = PR_Open( fileName, flags, mode ); // The mode parameter is currently applicable only on Unix platforms.
	if ( fd == NULL )
		return ThrowNSPRError( cx, PR_GetError() );
	JS_SetPrivate( cx, obj, fd );
	SetNativeInterface(cx, obj, NI_READ_RESOURCE, (FunctionPointer)NativeInterfaceReadFile, fd);
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}


DEFINE_FUNCTION( Close ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	RT_ASSERT( fd != NULL, "file is closed." );

	RemoveNativeInterface(cx, obj, NI_READ_RESOURCE );
	PRStatus status = PR_Close( fd );
	if ( status == PR_FAILURE )
		return ThrowNSPRError( cx, PR_GetError() );
	JS_SetPrivate( cx, obj, NULL );
	return JS_TRUE;
}

DEFINE_FUNCTION( Read ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	RT_ASSERT( fd != NULL, "file is closed." );

	PRInt32 amount;
	if ( argc >= 1 && argv[0] != JSVAL_VOID ) {

		int32 val;
		JS_ValueToInt32( cx, argv[0], &val );
		amount = val;

/* PR_Available fails with PRSpecialFD
		if ( amount > available )
			amount = available;
*/
	} else  { // no amount specified : read the whole file

		PRInt32 available = PR_Available(fd); // For a normal file, these are the bytes beyond the current file pointer.
		if ( available == -1 )
			return ThrowNSPRError( cx, PR_GetError() );
		amount = available;
	}

	char *buf = (char*)JS_malloc( cx, amount +1 );
	RT_ASSERT_ALLOC(buf);

	buf[amount] = 0; // (TBD) check if useful: PR_Read can read binary data !

	PRInt32 res = PR_Read( fd, buf, amount );
	if (res == -1) { // failure. The reason for the failure can be obtained by calling PR_GetError.

		JS_free( cx, buf );
		return ThrowNSPRError( cx, PR_GetError() );
	}

	if ( res == 0 ) {

		JS_free( cx, buf );
		*rval = JS_GetEmptyStringValue(cx); // (TBD) check if it is realy faster.
		return JS_TRUE;
	}

	if ( argc >= 2 ) {

		JSBool doRealloc;
		JS_ValueToBoolean(cx, argv[1], &doRealloc );
		if ( doRealloc == JS_TRUE ) {
			
			buf = (char*)JS_realloc(cx, buf, res); // realloc the string using its real size
			RT_ASSERT_ALLOC(buf);
		}
	}

	JSString *str = JS_NewString( cx, (char*)buf, res );
	RT_ASSERT_ALLOC(str);

	*rval = STRING_TO_JSVAL(str); // GC protection is ok with this ?
	return JS_TRUE;
}

DEFINE_FUNCTION( Write ) {

	RT_ASSERT_ARGC(1);

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	RT_ASSERT( fd != NULL, "file is closed." );

	JSString *jsstr = JS_ValueToString( cx, argv[0] );
	argv[0] = STRING_TO_JSVAL( jsstr ); // protect from GC

	PRInt32 length = JS_GetStringLength( jsstr );

	if ( argc >= 2 ) { // length is specified

		int32 userLength;
		JS_ValueToInt32( cx, argv[1], &userLength );

		if ( userLength <= length && userLength >= 0 ) // security
			length = userLength;
	}

	void *buf = JS_GetStringBytes( jsstr );

	PRInt32 bytesSent = PR_Write( fd, buf, length );

	if ( bytesSent == -1 )
		return ThrowNSPRError( cx, PR_GetError() );
	
	RT_ASSERT( bytesSent = length, "unable to send all datas" );
	return JS_TRUE;
}

DEFINE_FUNCTION( Seek ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	RT_ASSERT( fd != NULL, "file is closed." );

	PRInt64 offset = 0; // default is arg is missing
	if ( argc >= 1 ) {

		jsdouble doubleOffset;
		JS_ValueToNumber( cx, argv[0], &doubleOffset );
		offset = doubleOffset;
	}

	PRSeekWhence whence = PR_SEEK_CUR; // default is arg is missing
	if ( argc >= 2 ) {

		int32 tmp;
		JS_ValueToInt32( cx, argv[1], &tmp );
		whence = (PRSeekWhence)tmp;
	}

	PRInt64 ret = PR_Seek64( fd, offset, whence );
	if ( ret == -1 )
		return ThrowNSPRError( cx, PR_GetError() );

	jsdouble newLocation = ret;
	JS_NewDoubleValue( cx, newLocation, rval );
	return JS_TRUE;
}

DEFINE_FUNCTION( Sync ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	RT_ASSERT( fd != NULL, "file is closed." ); // 	RT_ASSERT_RESOURCE(fd);

	PRStatus status = PR_Sync(fd);
	if ( status == PR_FAILURE )
		return ThrowNSPRError( cx, PR_GetError() );

	return JS_TRUE;
}


DEFINE_FUNCTION( Delete ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	RT_ASSERT( fd == NULL, "Cannot delete an open file." );
	jsval jsvalFileName;
	JS_GetReservedSlot( cx, obj, SLOT_NSPR_FILE_NAME, &jsvalFileName );
	RT_ASSERT_DEFINED( jsvalFileName );
	char *fileName;
	RT_JSVAL_TO_STRING(jsvalFileName, fileName);
	
	PRStatus status = PR_Delete(fileName);
	if ( status != PR_SUCCESS )
		return ThrowNSPRError( cx, PR_GetError() );

	return JS_TRUE;
}


DEFINE_PROPERTY( available ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	RT_ASSERT( fd != NULL, "file is closed." );

	PRInt32 available = PR_Available( fd ); // For a normal file, these are the bytes beyond the current file pointer.
	if ( available == -1 )
		return ThrowNSPRError( cx, PR_GetError() );

	*vp = INT_TO_JSVAL(available);

	return JS_TRUE;
}

DEFINE_PROPERTY( eof ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	RT_ASSERT( fd != NULL, "file is closed." );

	PRInt32 available = PR_Available( fd ); // For a normal file, these are the bytes beyond the current file pointer.
	if ( available == -1 )
		return ThrowNSPRError( cx, PR_GetError() );

	*vp = BOOLEAN_TO_JSVAL(available == 0);
	return JS_TRUE;
}

DEFINE_PROPERTY( nameGetter ) {

	JS_GetReservedSlot( cx, obj, SLOT_NSPR_FILE_NAME, vp );
	return JS_TRUE;
}


DEFINE_PROPERTY( nameSetter ) {

	RT_ASSERT_DEFINED( *vp );

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	RT_ASSERT( fd == NULL, "Cannot rename an open file.");
	jsval jsvalFileName;
	JS_GetReservedSlot( cx, obj, SLOT_NSPR_FILE_NAME, &jsvalFileName );
	RT_ASSERT_DEFINED( jsvalFileName );

	char *fromFileName, *toFileName;
	RT_JSVAL_TO_STRING(jsvalFileName, fromFileName);
	RT_JSVAL_TO_STRING(*vp, toFileName);

	PRStatus status = PR_Rename(fromFileName, toFileName); // is status == PR_FILE_EXISTS_ERROR ...
	if ( status != PR_SUCCESS )
		return ThrowNSPRError( cx, PR_GetError() );

	JS_SetReservedSlot( cx, obj, SLOT_NSPR_FILE_NAME, *vp );
	return JS_TRUE;
}


DEFINE_PROPERTY( exist ) {

	jsval jsvalFileName;
//	JS_GetProperty( cx, obj, "fileName", &jsvalFileName );
	JS_GetReservedSlot( cx, obj, SLOT_NSPR_FILE_NAME, &jsvalFileName );
	RT_ASSERT_DEFINED( jsvalFileName );
	char *fileName;
	RT_JSVAL_TO_STRING(jsvalFileName, fileName);
	PRStatus status = PR_Access( fileName, PR_ACCESS_EXISTS );
	*vp = BOOLEAN_TO_JSVAL( status == PR_SUCCESS );
	return JS_TRUE;
}


DEFINE_PROPERTY( info ) {

	PRFileInfo fileInfo;
	PRStatus status;

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	if ( fd == NULL ) {
		
		jsval jsvalFileName;
		JS_GetReservedSlot( cx, obj, SLOT_NSPR_FILE_NAME, &jsvalFileName );
		RT_ASSERT_DEFINED( jsvalFileName );
		char *fileName;
		RT_JSVAL_TO_STRING(jsvalFileName, fileName);
		
		status = PR_GetFileInfo( fileName, &fileInfo );
	} else
		status = PR_GetOpenFileInfo( fd, &fileInfo );

	if ( status != PR_SUCCESS )
		return ThrowNSPRError( cx, PR_GetError() ); // ??? Doc do not say it is possible to read PR_GetError after an error on PR_GetFileInfo !!!
		// (TBD) check

	JSObject *fileTypeObj = JS_NewObject( cx, NULL, NULL, NULL );
	*vp = OBJECT_TO_JSVAL( fileTypeObj );

	jsval jsvalType = INT_TO_JSVAL((int)fileInfo.type);
	JS_SetProperty( cx, fileTypeObj, "type", &jsvalType );

	jsval jsvalSize = INT_TO_JSVAL(fileInfo.size);
	JS_SetProperty( cx, fileTypeObj, "size", &jsvalSize );

	PRTime creationTime = fileInfo.creationTime;
	PRTime modifyTime = fileInfo.modifyTime;

	return JS_TRUE;
}


DEFINE_PROPERTY( standard ) {
	
	if ( *vp == JSVAL_VOID ) {

		int32 i;
		JS_ValueToInt32( cx, id, &i );

		JSObject *obj = JS_NewObject(cx, &classFile, NULL, NULL );
		*vp = OBJECT_TO_JSVAL( obj ); // GC protection ?

		PRFileDesc *fd = PR_GetSpecialFD( (PRSpecialFD)i); // beware: cast !
		if ( fd == NULL )
			return ThrowNSPRError( cx, PR_GetError() );
		JS_SetPrivate( cx, obj, fd );
	}
	return JS_TRUE;
}


CONFIGURE_CLASS

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION( Open )
		FUNCTION( Close )
		FUNCTION( Read )
		FUNCTION( Write )
		FUNCTION( Seek )
		FUNCTION( Sync )
		FUNCTION( Delete )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY( name )
		PROPERTY_READ( eof )
		PROPERTY_READ( available )
		PROPERTY_READ( exist )
		PROPERTY_READ( info )
	END_PROPERTY_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_CREATE( stdin , PR_StandardInput,  JSPROP_PERMANENT|JSPROP_READONLY, standard, NULL ) // (TBD) change this
		PROPERTY_CREATE( stdout, PR_StandardOutput, JSPROP_PERMANENT|JSPROP_READONLY, standard, NULL ) // (TBD) change this
		PROPERTY_CREATE( stderr, PR_StandardError,  JSPROP_PERMANENT|JSPROP_READONLY, standard, NULL ) // (TBD) change this
	END_STATIC_PROPERTY_SPEC

	BEGIN_CONST_DOUBLE_SPEC
// PR_Open flags
		CONST_DOUBLE( RDONLY			,PR_RDONLY )
		CONST_DOUBLE( WRONLY			,PR_WRONLY )
		CONST_DOUBLE( RDWR			,PR_RDWR )
		CONST_DOUBLE( CREATE_FILE	,PR_CREATE_FILE )
		CONST_DOUBLE( APPEND			,PR_APPEND )
		CONST_DOUBLE( TRUNCATE		,PR_TRUNCATE )
		CONST_DOUBLE( SYNC			,PR_SYNC )
		CONST_DOUBLE( EXCL			,PR_EXCL )
// PRSeekWhence enum
		CONST_DOUBLE( SEEK_SET			,PR_SEEK_SET )
		CONST_DOUBLE( SEEK_CUR			,PR_SEEK_CUR )
		CONST_DOUBLE( SEEK_END			,PR_SEEK_END )
// PRFileType enum
		CONST_DOUBLE( FILE_FILE			,PR_FILE_FILE )
		CONST_DOUBLE( FILE_DIRECTORY	,PR_FILE_DIRECTORY )
		CONST_DOUBLE( FILE_OTHER		,PR_FILE_OTHER )
	END_CONST_DOUBLE_SPEC

	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1)

END_CLASS
