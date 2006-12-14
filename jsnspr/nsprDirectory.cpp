#define XP_WIN
#include <jsapi.h>
#include <nspr.h>

#include "nsprError.h"
#include "nsprDirectory.h"

void Directory_Finalize(JSContext *cx, JSObject *obj) {

	PRDir *dd = (PRDir *)JS_GetPrivate( cx, obj );
	if ( dd != NULL ) {

		PRStatus status = PR_CloseDir( dd ); // what to do on error ??
		if ( status == PR_FAILURE )
			JS_ReportError( cx, "a directory descriptor cannot be closed while Finalize" );
		JS_SetPrivate( cx, obj, NULL );
	}
}


JSClass Directory_class = {
	"Directory", JSCLASS_HAS_PRIVATE | JSCLASS_HAS_RESERVED_SLOTS(1),
	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Directory_Finalize
};


JSBool Directory_construct(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

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


JSBool Directory_open(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	jsval jsvalDirectoryName;
	JS_GetReservedSlot( cx, obj, 0, &jsvalDirectoryName );

	if ( jsvalDirectoryName == JSVAL_VOID ) {

		JS_ReportError( cx, "unable to get the directory name" );
		return JS_FALSE;
	}

	JSString *jsStringDirectoryName = JS_ValueToString( cx, jsvalDirectoryName );
	if ( jsStringDirectoryName == NULL ) {

		JS_ReportError( cx, "unable to get the directory name" );
		return JS_FALSE;
	}

	jsvalDirectoryName = STRING_TO_JSVAL( jsStringDirectoryName ); // protect form GC ??? ( is this ok, is this needed, ... ? )
	char *directoryName = JS_GetStringBytes( jsStringDirectoryName );


	PRDir *dd = PR_OpenDir( directoryName );

	if ( dd == NULL )
		return ThrowNSPRError( cx, PR_GetError() ); // ??? Doc do not say it is possible to read PR_GetError after an error on PR_OpenDir !!!
		// (TBD) check

	JS_SetPrivate( cx, obj, dd );

	return JS_TRUE;
}


JSBool Directory_close(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	PRDir *dd = (PRDir *)JS_GetPrivate( cx, obj );
	if ( dd == NULL ) {

		JS_ReportError( cx, "directory is closed" );
		return JS_FALSE;
	}
	PRStatus status = PR_CloseDir( dd );
	if ( status == PR_FAILURE )
		return ThrowNSPRError( cx, PR_GetError() );
	JS_SetPrivate( cx, obj, NULL );
	return JS_TRUE;
}




JSBool Directory_read(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	PRDir *dd = (PRDir *)JS_GetPrivate( cx, obj );
	if ( dd == NULL ) {

		JS_ReportError( cx, "directory is closed" );
		return JS_FALSE;
	}

	PRDirFlags flags = PR_SKIP_NONE;
	if ( argc >= 1 ) {
		int32 tmp;
		JS_ValueToInt32( cx, argv[0], &tmp );
		flags = (PRDirFlags)tmp;
	}

	PRDirEntry *dirEntry = PR_ReadDir( dd, flags );
	if ( dirEntry == NULL ) {
	
		PRErrorCode errorCode = PR_GetError();
		if ( errorCode == PR_NO_MORE_FILES_ERROR ) {
			*rval = JSVAL_VOID;
			return JS_TRUE;
		} else
			return ThrowNSPRError( cx, errorCode );
	}

	*rval = STRING_TO_JSVAL(JS_NewStringCopyZ( cx, dirEntry->name ));
	return JS_TRUE;
}


JSBool Directory_make(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	jsval jsvalDirectoryName;
	JS_GetReservedSlot( cx, obj, 0, &jsvalDirectoryName );
	if ( jsvalDirectoryName == JSVAL_VOID ) {

		JS_ReportError( cx, "unable to get the directory name" );
		return JS_FALSE;
	}

	JSString *jsStringDirectoryName = JS_ValueToString( cx, jsvalDirectoryName );
	if ( jsStringDirectoryName == NULL ) {

		JS_ReportError( cx, "unable to get the directory name" );
		return JS_FALSE;
	}

	jsvalDirectoryName = STRING_TO_JSVAL( jsStringDirectoryName ); // protect form GC ??? ( is this ok, is this needed, ... ? )
	char *directoryName = JS_GetStringBytes( jsStringDirectoryName );

	PRIntn mode = 0666;
	PRStatus status = PR_MkDir( directoryName, mode );
	if ( status == PR_FAILURE )
		return ThrowNSPRError( cx, PR_GetError() );
	return JS_TRUE;
}


JSBool Directory_remove(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	jsval jsvalDirectoryName;
	JS_GetReservedSlot( cx, obj, 0, &jsvalDirectoryName );

	if ( jsvalDirectoryName == JSVAL_VOID ) {

		JS_ReportError( cx, "unable to get the directory name" );
		return JS_FALSE;
	}

	JSString *jsStringDirectoryName = JS_ValueToString( cx, jsvalDirectoryName );
	if ( jsStringDirectoryName == NULL ) {

		JS_ReportError( cx, "unable to get the directory name" );
		return JS_FALSE;
	}


	jsvalDirectoryName = STRING_TO_JSVAL( jsStringDirectoryName ); // protect form GC ??? ( is this ok, is this needed, ... ? )
	char *directoryName = JS_GetStringBytes( jsStringDirectoryName );

	PRIntn mode = 0666;
	PRStatus status = PR_RmDir( directoryName ); // PR_RmDir removes the directory specified by the pathname name. The directory must be empty. If the directory is not empty, PR_RmDir fails and PR_GetError returns the error code PR_DIRECTORY_NOT_EMPTY_ERROR.
	if ( status == PR_FAILURE ) {
		
		PRErrorCode errorCode = PR_GetError();
		if ( errorCode == PR_DIRECTORY_NOT_EMPTY_ERROR ) 
			*rval = JSVAL_FALSE;
		else
			return ThrowNSPRError( cx, errorCode );
	}
	return JS_TRUE;
}


JSFunctionSpec Directory_FunctionSpec[] = { // { *name, call, nargs, flags, extra }
 { "Open"     , Directory_open   , 0, 0, 0 },
 { "Close"    , Directory_close  , 0, 0, 0 },
 { "Read"     , Directory_read   , 0, 0, 0 },
 { "Make"     , Directory_make   , 0, 0, 0 },
 { "Remove"   , Directory_remove , 0, 0, 0 },
 { 0 }
};


JSBool Directory_getter_name( JSContext *cx, JSObject *obj, jsval id, jsval *vp ) {

	JS_GetReservedSlot( cx, obj, 0, vp );
	return JS_TRUE;
}


JSPropertySpec Directory_PropertySpec[] = { // *name, tinyid, flags, getter, setter
	{ "name", 0, JSPROP_PERMANENT|JSPROP_READONLY, Directory_getter_name, NULL },
  { 0 }
};

    
JSBool Directory_static_setConst( JSContext *cx, JSObject *obj, jsval id, jsval *vp ) {

	*vp = id;
	return JS_TRUE;
}


JSPropertySpec Directory_static_PropertySpec[] = { // *name, tinyid, flags, getter, setter
// PR_ReadDir flags
	{ "SKIP_NONE"     ,PR_SKIP_NONE     ,JSPROP_SHARED | JSPROP_PERMANENT | JSPROP_READONLY, Directory_static_setConst, NULL },
	{ "SKIP_DOT"		,PR_SKIP_DOT      ,JSPROP_SHARED | JSPROP_PERMANENT | JSPROP_READONLY, Directory_static_setConst, NULL },
	{ "SKIP_DOT_DOT"  ,PR_SKIP_DOT_DOT  ,JSPROP_SHARED | JSPROP_PERMANENT | JSPROP_READONLY, Directory_static_setConst, NULL },
	{ "SKIP_BOTH"     ,PR_SKIP_BOTH     ,JSPROP_SHARED | JSPROP_PERMANENT | JSPROP_READONLY, Directory_static_setConst, NULL },
	{ "SKIP_HIDDEN"   ,PR_APPEND        ,JSPROP_SHARED | JSPROP_PERMANENT | JSPROP_READONLY, Directory_static_setConst, NULL },
//
	{ 0 }
};



JSObject *InitDirectoryClass( JSContext *cx, JSObject *obj ) {

	return JS_InitClass( cx, obj, NULL, &Directory_class, Directory_construct, 1, Directory_PropertySpec, Directory_FunctionSpec, Directory_static_PropertySpec, NULL );
}
