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

#define XP_WIN
#include <jsapi.h>
#include <nspr.h>

#include "nsprError.h"
#include "nsprFile.h"

#include "../common/jshelper.h"
#include "../common/jsNativeInterface.h"


void File_Finalize(JSContext *cx, JSObject *obj) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	if ( fd != NULL ) {

		PRStatus status = PR_Close( fd ); // what to do on error ??
		if ( status == PR_FAILURE )
			JS_ReportError( cx, "a file descriptor cannot be closed while Finalize" );
		JS_SetPrivate( cx, obj, NULL );
	}
}


JSClass File_class = {
	"File", JSCLASS_HAS_PRIVATE | JSCLASS_HAS_RESERVED_SLOTS(1),
	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, File_Finalize
};


JSBool File_construct(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	if ( !JS_IsConstructing(cx) ) {

		JS_ReportError( cx, "need to be construct" );
		return JS_FALSE;
	}

	if ( argc < 1 ) {

		JS_ReportError( cx, "missing argument" );
		return JS_FALSE;
	}

	JS_SetReservedSlot( cx, obj, 0, argv[0] );
	return JS_TRUE;
}


static bool NativeInterfaceReadFile( void *pv, unsigned char *buf, unsigned int *amount ) {

	PRInt32 status = PR_Read( (PRFileDesc *)pv, buf, *amount );
	if ( status == -1 )
		return false;
	*amount = status;
	return true;
}


JSBool File_open(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	if ( argc < 1 ) {

		JS_ReportError( cx, "missing argument" );
		return JS_FALSE;
	}

	jsval jsvalFileName;
//	JS_GetProperty( cx, obj, "fileName", &jsvalFileName );
	JS_GetReservedSlot( cx, obj, 0, &jsvalFileName );
	if ( jsvalFileName == JSVAL_VOID ) {

		JS_ReportError( cx, "unable to get the file name" );
		return JS_FALSE;
	}

	JSString *jsStringFileName = JS_ValueToString( cx, jsvalFileName );
	if ( jsStringFileName == NULL ) {

		JS_ReportError( cx, "unable to get the file name" );
		return JS_FALSE;
	}
	jsvalFileName = STRING_TO_JSVAL( jsStringFileName ); // protect form GC ??? ( is this ok, is this needed, ... ? )
	char *fileName = JS_GetStringBytes( jsStringFileName );

	PRIntn flags;
	int32 tmp;
	JS_ValueToInt32( cx, argv[0], &tmp );
	flags = tmp;

	PRFileDesc *fd;

	PRIntn mode = 0666;
	fd = PR_Open( fileName, flags, mode ); // The mode parameter is currently applicable only on Unix platforms.
	if ( fd == NULL )
		return ThrowNSPRError( cx, PR_GetError() );

	JS_SetPrivate( cx, obj, fd );
	SetNativeInterface(cx, obj, NI_READ_RESOURCE, (FunctionPointer)NativeInterfaceReadFile, fd);
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}


JSBool File_close(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	if ( fd == NULL ) {

		JS_ReportError( cx, "file is closed" );
		return JS_FALSE;
	}

	RemoveNativeInterface(cx, obj, NI_READ_RESOURCE );
	PRStatus status = PR_Close( fd );
	if ( status == PR_FAILURE )
		return ThrowNSPRError( cx, PR_GetError() );
	JS_SetPrivate( cx, obj, NULL );
	return JS_TRUE;
}

JSBool File_read(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	if ( fd == NULL ) {

		JS_ReportError( cx, "file is closed" );
		return JS_FALSE;
	}


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

	JSString *str = JS_NewString( cx, (char*)buf, res );
	if (str == NULL) {

		JS_ReportError( cx, "JS_NewString error" );
		return JS_FALSE;
	}

	*rval = STRING_TO_JSVAL(str); // GC protection is ok with this ?
	return JS_TRUE;
}


JSBool File_write(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	if ( argc < 1 ) {

		JS_ReportError( cx, "missing argument" );
		return JS_FALSE;
	}

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	if ( fd == NULL ) {

		JS_ReportError( cx, "file is closed" );
		return JS_FALSE;
	}

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

	if ( bytesSent < length ) {

		JS_ReportError( cx, "unable to send all datas" );
		return JS_FALSE;
	}
	return JS_TRUE;
}


JSBool File_seek(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	if ( fd == NULL ) {

		JS_ReportError( cx, "file is closed" );
		return JS_FALSE;
	}

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


JSBool File_sync(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	if ( fd == NULL ) {

		JS_ReportError( cx, "file is closed" );
		return JS_FALSE;
	}

	PRStatus status = PR_Sync(fd);
	if ( status == PR_FAILURE )
		return ThrowNSPRError( cx, PR_GetError() );

	return JS_TRUE;
}

JSFunctionSpec File_FunctionSpec[] = { // { *name, call, nargs, flags, extra }
 { "Open"     , File_open   , 0, 0, 0 },
 { "Close"    , File_close  , 0, 0, 0 },
 { "Read"     , File_read   , 0, 0, 0 },
 { "Write"    , File_write  , 0, 0, 0 },
 { "Seek"     , File_seek   , 0, 0, 0 },
 { "Sync"     , File_sync   , 0, 0, 0 },
 { 0 }
};


JSBool File_getter_eof( JSContext *cx, JSObject *obj, jsval id, jsval *vp ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	if ( fd == NULL ) {

		JS_ReportError( cx, "file is closed" );
		return JS_FALSE;
	}

	PRInt32 available = PR_Available( fd ); // For a normal file, these are the bytes beyond the current file pointer.
	if ( available == -1 )
		return ThrowNSPRError( cx, PR_GetError() );

	*vp = BOOLEAN_TO_JSVAL(available == 0);
	return JS_TRUE;
}


JSBool File_getter_name( JSContext *cx, JSObject *obj, jsval id, jsval *vp ) {

	JS_GetReservedSlot( cx, obj, 0, vp );
	return JS_TRUE;
}


JSBool File_getter_available( JSContext *cx, JSObject *obj, jsval id, jsval *vp ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	if ( fd == NULL ) {

		JS_ReportError( cx, "file is closed" );
		return JS_FALSE;
	}

	PRInt32 available = PR_Available( fd ); // For a normal file, these are the bytes beyond the current file pointer.
	if ( available == -1 )
		return ThrowNSPRError( cx, PR_GetError() );

	*vp = INT_TO_JSVAL(available);

	return JS_TRUE;
}


JSBool File_getter_exist( JSContext *cx, JSObject *obj, jsval id, jsval *vp ) {

	jsval jsvalFileName;
//	JS_GetProperty( cx, obj, "fileName", &jsvalFileName );
	JS_GetReservedSlot( cx, obj, 0, &jsvalFileName );

	if ( jsvalFileName == JSVAL_VOID ) {

		JS_ReportError( cx, "unable to get the file name" );
		return JS_FALSE;
	}

	JSString *jsStringFileName = JS_ValueToString( cx, jsvalFileName );
	if ( jsStringFileName == NULL ) {

		JS_ReportError( cx, "unable to get the file name" );
		return JS_FALSE;
	}
	jsvalFileName = STRING_TO_JSVAL( jsStringFileName ); // protect form GC ??? ( is this ok, is this needed, ... ? )

	char *fileName = JS_GetStringBytes( jsStringFileName );

	PRStatus status = PR_Access( fileName, PR_ACCESS_EXISTS );
	*vp = BOOLEAN_TO_JSVAL( status == PR_SUCCESS );

	return JS_TRUE;
}



JSBool File_getter_info( JSContext *cx, JSObject *obj, jsval id, jsval *vp ) {

	jsval jsvalFileName;
//	JS_GetProperty( cx, obj, "fileName", &jsvalFileName );
	JS_GetReservedSlot( cx, obj, 0, &jsvalFileName );

	if ( jsvalFileName == JSVAL_VOID ) {

		JS_ReportError( cx, "unable to get the file name" );
		return JS_FALSE;
	}

	JSString *jsStringFileName = JS_ValueToString( cx, jsvalFileName );
	if ( jsStringFileName == NULL ) {

		JS_ReportError( cx, "unable to get the file name" );
		return JS_FALSE;
	}
	jsvalFileName = STRING_TO_JSVAL( jsStringFileName ); // protect form GC ??? ( is this ok, is this needed, ... ? )

	char *fileName = JS_GetStringBytes( jsStringFileName );

	PRFileInfo fileInfo;
	PRStatus status;

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	if ( fd == NULL )
		status = PR_GetFileInfo( fileName, &fileInfo );
	else
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

JSPropertySpec File_PropertySpec[] = { // *name, tinyid, flags, getter, setter
	{ "eof"       , 0, JSPROP_SHARED | JSPROP_PERMANENT|JSPROP_READONLY, File_getter_eof       , NULL },
	{ "name"      , 0, JSPROP_SHARED | JSPROP_PERMANENT|JSPROP_READONLY, File_getter_name      , NULL },
	{ "available" , 0, JSPROP_SHARED | JSPROP_PERMANENT|JSPROP_READONLY, File_getter_available , NULL },
	{ "exist"     , 0, JSPROP_SHARED | JSPROP_PERMANENT|JSPROP_READONLY, File_getter_exist     , NULL },
	{ "info"      , 0, JSPROP_SHARED | JSPROP_PERMANENT|JSPROP_READONLY, File_getter_info      , NULL },
  { 0 }
};


JSBool File_static_setConst( JSContext *cx, JSObject *obj, jsval id, jsval *vp ) {

	int32 i;
	JS_ValueToInt32( cx, id, &i );
	*vp = INT_TO_JSVAL(i);
	return JS_TRUE;
}


JSBool File_static_standard( JSContext *cx, JSObject *obj, jsval id, jsval *vp ) {
	
	if ( *vp == JSVAL_VOID ) {

		int32 i;
		JS_ValueToInt32( cx, id, &i );

		JSObject *obj = JS_NewObject(cx, &File_class, NULL, NULL );
		*vp = OBJECT_TO_JSVAL( obj ); // GC protection ?

		PRFileDesc *fd = PR_GetSpecialFD( (PRSpecialFD)i); // beware: cast !
		if ( fd == NULL )
			return ThrowNSPRError( cx, PR_GetError() );
		JS_SetPrivate( cx, obj, fd );
	}
}




JSPropertySpec File_static_PropertySpec[] = { // *name, tinyid, flags, getter, setter

	{ "stdin"    , PR_StandardInput,  JSPROP_PERMANENT|JSPROP_READONLY, File_static_standard, NULL },
	{ "stdout"   , PR_StandardOutput, JSPROP_PERMANENT|JSPROP_READONLY, File_static_standard, NULL },
	{ "stderr"   , PR_StandardError,  JSPROP_PERMANENT|JSPROP_READONLY, File_static_standard, NULL },


// PR_Open flags
	{ "RDONLY"        ,PR_RDONLY      ,JSPROP_SHARED | JSPROP_PERMANENT|JSPROP_READONLY, File_static_setConst, NULL },
	{ "WRONLY"			,PR_WRONLY      ,JSPROP_SHARED | JSPROP_PERMANENT|JSPROP_READONLY, File_static_setConst, NULL },
	{ "RDWR"				,PR_RDWR        ,JSPROP_SHARED | JSPROP_PERMANENT|JSPROP_READONLY, File_static_setConst, NULL },
	{ "CREATE_FILE"	,PR_CREATE_FILE ,JSPROP_SHARED | JSPROP_PERMANENT|JSPROP_READONLY, File_static_setConst, NULL },
	{ "APPEND"			,PR_APPEND      ,JSPROP_SHARED | JSPROP_PERMANENT|JSPROP_READONLY, File_static_setConst, NULL },
	{ "TRUNCATE"		,PR_TRUNCATE    ,JSPROP_SHARED | JSPROP_PERMANENT|JSPROP_READONLY, File_static_setConst, NULL },
	{ "SYNC"				,PR_SYNC        ,JSPROP_SHARED | JSPROP_PERMANENT|JSPROP_READONLY, File_static_setConst, NULL },
	{ "EXCL"				,PR_EXCL        ,JSPROP_SHARED | JSPROP_PERMANENT|JSPROP_READONLY, File_static_setConst, NULL },
// PRSeekWhence enum
	{ "SEEK_SET"		,PR_SEEK_SET    ,JSPROP_SHARED | JSPROP_PERMANENT|JSPROP_READONLY, File_static_setConst, NULL },
	{ "SEEK_CUR"		,PR_SEEK_CUR    ,JSPROP_SHARED | JSPROP_PERMANENT|JSPROP_READONLY, File_static_setConst, NULL },
	{ "SEEK_END"		,PR_SEEK_END    ,JSPROP_SHARED | JSPROP_PERMANENT|JSPROP_READONLY, File_static_setConst, NULL },
// PRFileType enum
	{ "FILE_FILE"		 ,PR_FILE_FILE         ,JSPROP_SHARED | JSPROP_PERMANENT|JSPROP_READONLY, File_static_setConst, NULL },
	{ "FILE_DIRECTORY" ,PR_FILE_DIRECTORY    ,JSPROP_SHARED | JSPROP_PERMANENT|JSPROP_READONLY, File_static_setConst, NULL },
	{ "FILE_OTHER"		 ,PR_FILE_OTHER        ,JSPROP_SHARED | JSPROP_PERMANENT|JSPROP_READONLY, File_static_setConst, NULL },
//
	{ 0 }
};

JSObject *InitFileClass( JSContext *cx, JSObject *obj ) {

	PR_STDIO_INIT()

	JSObject *fileClass = JS_InitClass( cx, obj, NULL, &File_class, File_construct, 1, File_PropertySpec, File_FunctionSpec, File_static_PropertySpec, NULL );
//	JS_DefineConstDoubles( cx, fileClass, &File_const);
	return fileClass;
}
